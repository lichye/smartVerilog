#include "Pipeline.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <random>
#include <set>
#include <sstream>

#include "AssertionWriter.h"
#include "setups.h"
#include "BlockRunner.h"
#include "Blocks.h"
#include "Harness.h"
#include "Mus.h"
#include "HwcbmcFrontend.h"
#include "SVModule.h"
#include "WorkDir.h"

namespace smart {
namespace pipeline {
namespace {

namespace fs = std::filesystem;

std::string shellQuote(const std::string& text) { return "'" + text + "'"; }

std::string readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot read " + path);
    std::ostringstream text;
    text << in.rdbuf();
    return text.str();
}

void writeFile(const std::string& path, const std::string& content) {
    std::ofstream out(path);
    if (!out) throw std::runtime_error("cannot write " + path);
    out << content;
}

std::string timestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    std::ostringstream os;
    os << std::put_time(std::localtime(&time), "%H:%M:%S");
    return os.str();
}

// Which tool a name resolves to, or "" when it is not on PATH.
std::string which(const std::string& tool) {
    if (tool.find('/') != std::string::npos)
        return fs::exists(tool) ? tool : std::string();

    const char* path = std::getenv("PATH");
    if (path == nullptr) return {};
    std::istringstream entries(path);
    std::string dir;
    while (std::getline(entries, dir, ':')) {
        if (dir.empty()) continue;
        const auto candidate = fs::path(dir) / tool;
        std::error_code error;
        if (fs::exists(candidate, error)) return candidate.string();
    }
    return {};
}

std::string firstLineOf(const std::string& command) {
    std::array<char, 512> buffer{};
    FILE* pipe = popen((command + " 2>&1").c_str(), "r");
    if (pipe == nullptr) return {};
    std::string line;
    if (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
        line = buffer.data();
    pclose(pipe);
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
        line.pop_back();
    return line;
}

// Sibling .sv/.v files, which multi-file designs keep their submodules in.
// A sibling that declares the top module itself is skipped: `smart` writes
// <top>_assertion.sv beside its input, and a second run in the same directory
// must not treat its own previous output as part of the design.
std::vector<std::string> designFilesFor(const std::string& main,
                                        const std::string& top) {
    std::vector<std::string> files{main};
    const auto parent = fs::path(main).parent_path();
    std::vector<std::string> siblings;
    std::error_code error;
    for (const auto& entry :
         fs::directory_iterator(parent.empty() ? fs::path(".") : parent, error)) {
        if (!entry.is_regular_file()) continue;
        const auto extension = entry.path().extension().string();
        if (extension != ".sv" && extension != ".v") continue;
        if (fs::equivalent(entry.path(), fs::path(main), error)) continue;
        if (frontend::declaresModule(entry.path().string(), top)) continue;
        siblings.push_back(entry.path().string());
    }
    std::sort(siblings.begin(), siblings.end());
    files.insert(files.end(), siblings.begin(), siblings.end());
    return files;
}

std::optional<frontend::ResetInfo> parseResetOption(const std::string& text) {
    if (text.empty()) return std::nullopt;
    frontend::ResetInfo reset;
    const auto colon = text.find(':');
    reset.signal = text.substr(0, colon);
    reset.active = 1;
    reset.cycles = 2;
    if (colon != std::string::npos) reset.active = std::stoi(text.substr(colon + 1));
    return reset;
}

}  // namespace

Pipeline::Pipeline(Options options, std::string selfExecutable)
    : options_(std::move(options)), self_(std::move(selfExecutable)) {}

ExitCode Pipeline::run(RunSummary& summary) {
    const auto runStart = std::chrono::steady_clock::now();
    const bool quiet = options_.getInt("verbosity") < 0;
    // The reused core prints through printDebug(); keep it quiet unless asked.
    smartVerbosity = static_cast<int>(options_.getInt("verbosity"));
    const auto say = [&](const std::string& message) {
        if (!quiet) std::cout << message << std::endl;
    };

    // ---- input ---------------------------------------------------------
    const auto& inputs = options_.designFiles();
    if (inputs.empty()) {
        std::cerr << "smart: no design file given\n\n" << Options::usage();
        return ExitCode::UserError;
    }
    const std::string mainFile = inputs.front();
    if (!fs::exists(mainFile)) {
        std::cerr << "smart: no such file: " << mainFile << "\n";
        return ExitCode::UserError;
    }

    std::string top = options_.getString("top");
    if (top.empty()) top = fs::path(mainFile).stem().string();
    summary.top = top;

    std::vector<std::string> designFiles =
        inputs.size() > 1 ? inputs : designFilesFor(mainFile, top);

    // ---- frontend ------------------------------------------------------
    frontend::ModuleInfo info;
    frontend::HwcbmcOptions frontendOptions;
    frontendOptions.autoDiscoverSiblings = inputs.size() <= 1;
    for (std::size_t i = 1; i < inputs.size(); ++i)
        frontendOptions.extraFiles.push_back(inputs[i]);
    try {
        info = frontend::parseModuleFile(mainFile, top, frontendOptions);
    } catch (const std::exception& e) {
        std::cerr << "smart: " << e.what() << "\n";
        return ExitCode::UserError;
    }
    if (!frontendOptions.elaborated)
        say("warning: widths unresolved (" + frontendOptions.elaborationError + ")");

    if (info.ports.empty() && info.freeRegs.empty()) {
        std::cerr << "smart: module '" << top
                  << "' has no inputs or free registers to drive\n";
        return ExitCode::UserError;
    }

    // ---- workdir -------------------------------------------------------
    std::string workRoot = options_.getString("workdir");
    if (workRoot.empty()) workRoot = "smart-work-" + top;
    WorkDir work(fs::absolute(workRoot).string());
    summary.workDir = work.root();

    try {
        work.create();

        // runtime/verilog and runtime/formal hold the design as the checker
        // sees it: originals, plus any --assume the user asked for.
        const auto assumes = options_.getStringList("assume");
        for (const auto& file : designFiles) {
            auto source = readFile(file);
            if (!assumes.empty() &&
                fs::path(file).stem().string() == fs::path(mainFile).stem().string())
                source = frontend::injectAssumes(source, top, assumes,
                                                 options_.getString("clock"));
            const auto name = fs::path(file).filename().string();
            writeFile(work.verilogDir() + "/" + name, source);
            writeFile(work.formalDir() + "/" + name, source);
        }
        writeFile(work.effectiveConfigFile(), options_.toJson());
    } catch (const std::exception& e) {
        std::cerr << "smart: cannot prepare " << work.root() << ": " << e.what()
                  << "\n";
        return ExitCode::Failure;
    }

    for (const auto& warning : options_.warnings()) say("note: " + warning);

    // ---- simulate ------------------------------------------------------
    simgen::HarnessOptions harness;
    harness.cycles = static_cast<int>(options_.getInt("cycles"));
    harness.traces = static_cast<int>(options_.getInt("traces"));
    harness.seed = static_cast<unsigned>(options_.getInt("seed"));
    harness.clock = options_.getString("clock");
    harness.reset = parseResetOption(options_.getString("reset"));
    harness.firstIndex = 1;  // sim1.vcd .. simN.vcd, as the legacy tree had

    say("[" + timestamp() + "] simulating " + std::to_string(harness.traces) +
        " traces x " + std::to_string(harness.cycles) + " cycles");
    try {
        // Simulate the design as it sits in the workdir, so the traces match
        // the sources the blocks reason about.
        std::vector<std::string> workdirDesign;
        for (const auto& file : designFiles)
            workdirDesign.push_back(work.verilogDir() + "/" +
                                    fs::path(file).filename().string());

        simgen::runSimulations(info, workdirDesign, work.simSrcDir(),
                               work.simResultsDir(), harness);
    } catch (const std::exception& e) {
        std::cerr << "smart: simulation failed: " << e.what() << "\n"
                  << "smart: workdir kept at " << work.root() << "\n";
        summary.workDirKept = true;
        return ExitCode::Failure;
    }

    // ---- pre-analysis --------------------------------------------------
    std::vector<std::string> variables;
    try {
        variables = candidateVariables(top, work.simResultsDir());
    } catch (const std::exception& e) {
        std::cerr << "smart: cannot read the traces back: " << e.what() << "\n";
        summary.workDirKept = true;
        return ExitCode::Failure;
    }
    if (variables.empty()) {
        std::cerr << "smart: no candidate variables — the traces carry no signals "
                     "for module '"
                  << top << "'\n";
        summary.workDirKept = true;
        return ExitCode::Failure;
    }

    {
        std::ostringstream lines;
        for (const auto& variable : variables) lines << variable << "\n";
        writeFile(work.variablesFile(), lines.str());
    }

    const int jobs = static_cast<int>(options_.getInt("jobs"));
    std::mt19937_64 rng(static_cast<unsigned long long>(options_.getInt("seed")));
    const int cores = hardwareCores();
    const auto plan = writeInitialBlocks(variables, work.variablesDir(),
                                         options_.getBool("blockified"),
                                         options_.getDouble("block_size"), cores, rng);
    say("[" + timestamp() + "] " + std::to_string(variables.size()) +
        " candidate variables, k=" + std::to_string(plan.k) + ", " +
        std::to_string(plan.threadBlocks + plan.initBlocks) + " blocks");

    // ---- synthesis rounds ----------------------------------------------
    BlockRunnerOptions runner;
    runner.executable = self_;
    runner.workDir = work.root();
    runner.topModule = top;
    runner.configPath = work.effectiveConfigFile();
    runner.logsDir = work.logsDir();
    runner.jobs = jobs;
    runner.coreTimeoutSeconds = static_cast<int>(options_.getInt("core_timeout"));

    const auto deadline =
        runStart + std::chrono::seconds(options_.getInt("timeout"));

    std::set<std::string> mined;
    int round = 0;
    int blockCounter = 0;
    std::size_t lastMsaSize = std::numeric_limits<std::size_t>::max();
    long long stableRounds = 0;

    for (;;) {
        ++round;

        std::vector<BlockJob> jobsThisRound;
        std::vector<std::string> blockFiles;
        for (const auto& entry : fs::directory_iterator(work.variablesDir()))
            if (entry.is_regular_file()) blockFiles.push_back(entry.path().string());
        std::sort(blockFiles.begin(), blockFiles.end());

        for (const auto& file : blockFiles) {
            BlockJob job;
            job.variablesFile = file;
            job.coreId = std::to_string(++blockCounter);
            job.resultFile = work.resultDir() + "/result_" + job.coreId + ".txt";
            jobsThisRound.push_back(job);
        }

        if (jobsThisRound.empty()) {
            say("no blocks to run");
            break;
        }

        say("[" + timestamp() + "] round " + std::to_string(round) + ": " +
            std::to_string(jobsThisRound.size()) + " blocks on " +
            std::to_string(jobs) + " workers");

        std::size_t foundThisRound = 0;
        const auto results = runBlocks(
            jobsThisRound, runner, deadline,
            [&](const BlockResult& result, std::size_t done, std::size_t total) {
                if (result.status == BlockStatus::Verified &&
                    mined.insert(result.assertion).second)
                    ++foundThisRound;
                if (!quiet && (done % 10 == 0 || done == total))
                    std::cout << "[" << timestamp() << "]   " << done << "/" << total
                              << " blocks, " << mined.size() << " assertions"
                              << std::endl;
            });
        (void)results;

        say("[" + timestamp() + "] round " + std::to_string(round) + " found " +
            std::to_string(foundThisRound) + " new assertions (" +
            std::to_string(mined.size()) + " total)");

        if (!options_.getBool("blockified")) break;
        if (static_cast<long long>(foundThisRound) <= options_.getInt("threshold"))
            break;
        if (round >= options_.getInt("max_iterations")) break;
        if (std::chrono::steady_clock::now() >= deadline) {
            say("global timeout reached");
            break;
        }

        // --- next round's blocks (port of smart.py GenerateNewBlocks) -----
        //
        // Between rounds, drop the invariants the others already imply: a
        // smaller assertion set means a larger, more useful MSA.
        if (options_.getBool("block_minimizer")) {
            try {
                const auto minimised = mus::minimiseAssertions(
                    work.sygusResultFile(), work.sygusResultFile(),
                    static_cast<int>(options_.getInt("block_minimizer_timeout")));
                if (minimised.before != minimised.after)
                    say("[" + timestamp() + "] minimiser: " +
                        std::to_string(minimised.before) + " -> " +
                        std::to_string(minimised.after) + " invariants");
            } catch (const std::exception& e) {
                say("note: minimiser skipped: " + std::string(e.what()));
            }
        }

        const bool useMsa = options_.getBool("msa");
        const bool useRandom = options_.getBool("random") || !useMsa;

        std::vector<std::string> msaPool;
        if (useMsa) {
            try {
                const auto found = mus::getMus(
                    work.variablesFile(), work.sygusResultFile(),
                    static_cast<int>(options_.getInt("block_minimizer_timeout")));
                msaPool.assign(found.underspecified.begin(),
                               found.underspecified.end());
                say("[" + timestamp() + "] MSA: " + std::to_string(msaPool.size()) +
                    " of " + std::to_string(variables.size()) +
                    " variables still underspecified" +
                    (found.timedOut ? " (timed out, result is not minimal)" : ""));
            } catch (const std::exception& e) {
                say("note: MSA failed (" + std::string(e.what()) +
                    "), falling back to random blocks");
            }
        }

        // Stop when the MSA has stopped shrinking for msa_stable_depth rounds:
        // the same pool would produce the same blocks.
        if (useMsa && options_.getBool("msa_stable_end")) {
            if (msaPool.size() >= lastMsaSize) {
                if (++stableRounds >= options_.getInt("msa_stable_depth")) {
                    say("MSA stopped shrinking; stopping");
                    break;
                }
            } else {
                stableRounds = 0;
            }
            lastMsaSize = msaPool.size();
        }

        std::error_code error;
        fs::remove_all(work.variablesDir(), error);
        fs::create_directories(work.variablesDir());

        const auto& pool = msaPool.empty() ? variables : msaPool;
        const int k = subsetSize(pool.size(), options_.getDouble("k_size"));
        const int count = subsetCount(pool.size(),
                                      options_.getDouble("block_size"), cores);

        if (!msaPool.empty() && useRandom) {
            // Both strategies asked for: half the blocks from the MSA pool,
            // half from the full variable set.
            const int msaCount = count / 2;
            writeBlocks(msaPool, msaCount, k, work.variablesDir(), "thread_", rng);
            writeBlocks(variables, count - msaCount,
                        subsetSize(variables.size(), options_.getDouble("k_size")),
                        work.variablesDir(), "thread_", rng, msaCount);
        } else {
            writeBlocks(pool, count, k, work.variablesDir(), "thread_", rng);
        }
    }

    summary.rounds = round;
    summary.minedAssertions = mined.size();

    std::vector<std::string> assertions(mined.begin(), mined.end());
    {
        std::ostringstream lines;
        for (const auto& assertion : assertions) lines << assertion << "\n";
        writeFile(work.assertionsFile(), lines.str());
    }

    // ---- final check ---------------------------------------------------
    std::vector<std::string> verified;
    if (options_.getBool("checker") && !assertions.empty()) {
        emit::CheckOptions check;
        check.topModule = top;
        check.bound = static_cast<int>(options_.getInt("bound"));
        check.unbounded = options_.getBool("check_unbounded");
        check.timeoutSeconds = static_cast<int>(options_.getInt("check_timeout"));
        check.jobs = jobs;
        check.scratchDir = work.formalDir();
        for (std::size_t i = 1; i < designFiles.size(); ++i)
            check.extraFiles.push_back(work.verilogDir() + "/" +
                                       fs::path(designFiles[i]).filename().string());

        say("[" + timestamp() + "] checking " + std::to_string(assertions.size()) +
            " assertions against the original design");

        const auto checked = emit::checkAssertions(
            work.verilogDir() + "/" + fs::path(mainFile).filename().string(),
            assertions, check);
        for (const auto& result : checked)
            if (result.status == emit::CheckStatus::Verified)
                verified.push_back(result.assertion);
    } else {
        verified = assertions;
    }

    summary.verifiedAssertions = verified.size();
    {
        std::ostringstream lines;
        for (const auto& assertion : verified) lines << assertion << "\n";
        writeFile(work.invariantsFile(), lines.str());
    }

    // ---- emit ----------------------------------------------------------
    std::string output = options_.getString("output");
    if (output.empty()) {
        const auto parent = fs::path(mainFile).parent_path();
        output = (parent.empty() ? fs::path(".") : parent).string() + "/" + top +
                 "_assertion.sv";
    }
    summary.outputFile = output;

    try {
        emit::writeAssertionFile(mainFile, output, top, verified, options_.toJson());
    } catch (const std::exception& e) {
        std::cerr << "smart: cannot write " << output << ": " << e.what() << "\n";
        summary.workDirKept = true;
        return ExitCode::Failure;
    }

    summary.seconds =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - runStart)
            .count();

    if (options_.getBool("keep_work")) {
        summary.workDirKept = true;
    } else {
        work.remove();
    }
    return ExitCode::Success;
}

ExitCode checkEnvironment(const Options& options, const std::string& selfExecutable) {
    struct Tool {
        std::string name;
        std::string versionFlag;
        bool required;
    };
    const std::vector<Tool> tools = {{"iverilog", "-V", true},
                                     {"vvp", "-V", true},
                                     {"ebmc", "--version", true},
                                     {"cvc5", "--version", false}};

    bool ok = true;
    for (const auto& tool : tools) {
        const auto path = which(tool.name);
        if (path.empty()) {
            std::cout << "[MISSING] " << tool.name
                      << (tool.required ? "" : " (optional)") << "\n";
            if (tool.required) ok = false;
            continue;
        }
        std::cout << "[ok]      " << tool.name << "  " << path << "  "
                  << firstLineOf(shellQuote(path) + " " + tool.versionFlag) << "\n";
    }

    std::cout << "[ok]      smart     " << selfExecutable << "\n";
    std::cout << "jobs default: " << options.getInt("jobs") << "\n";
    return ok ? ExitCode::Success : ExitCode::EnvironmentError;
}

}  // namespace pipeline
}  // namespace smart
