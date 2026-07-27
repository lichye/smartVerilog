#include "Pipeline.h"

#include "../Version.h"

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
#include <mutex>
#include <random>
#include <set>
#include <sstream>
#include <unistd.h>

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

// Elapsed seconds since `since`, for the run-log's per-stage timings.
double seconds(std::chrono::steady_clock::time_point since) {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - since)
        .count();
}

namespace fs = std::filesystem;

std::string shellQuote(const std::string& text) { return "'" + text + "'"; }

std::string jsonEscape(const std::string& text) {
    std::string escaped;
    for (char c : text) {
        switch (c) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\n': escaped += "\\n"; break;
            case '\t': escaped += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) >= 0x20) escaped += c;
        }
    }
    return escaped;
}

// Append one JSON object to the run log. Line-buffered and flushed per record
// on purpose: the log has to survive the run being killed, which is precisely
// when it is worth reading.
//
// Every record carries the writer's pid. Reopening in append mode means a run
// whose workdir is deleted underneath it will recreate the file and keep
// writing, so two runs' records can end up interleaved in one log with two
// independent `t` timelines. Observed, and not detectable without this field.
// Group by "run" before reading any timing out of a log.
class RunLog {
public:
    explicit RunLog(std::string path)
        : path_(std::move(path)), pid_(static_cast<long>(::getpid())) {}

    void record(const std::string& fields) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::ofstream out(path_, std::ios::app);
        if (!out) return;
        out << "{\"run\":" << pid_ << ",\"t\":" << std::fixed
            << std::setprecision(3) << elapsed() << "," << fields << "}\n";
    }

private:
    double elapsed() const {
        return std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                             start_)
            .count();
    }
    std::string path_;
    long pid_;
    std::mutex mutex_;
    std::chrono::steady_clock::time_point start_ = std::chrono::steady_clock::now();
};

const char* statusName(BlockStatus status) {
    switch (status) {
        case BlockStatus::Verified: return "verified";
        case BlockStatus::NoAssertion: return "no-assertion";
        case BlockStatus::TimedOut: return "timed-out";
        case BlockStatus::Failed: return "failed";
    }
    return "unknown";
}

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

    // The module the invariants are about. The design is still elaborated,
    // simulated and checked as a whole from `top`; this only decides whose
    // signals become candidates and whose body the assertions are written
    // into. Pointing it at a submodule mines that block with its parent as
    // the environment.
    std::string mineModule = options_.getString("module");
    if (mineModule.empty()) mineModule = top;
    // A VCD scope is an instance (DFF_0); a property is written into a module
    // definition (dff). Candidate variables key off the former, injection off
    // the latter, and only the design knows the mapping.
    std::string injectModule = mineModule;

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

    // A VCD scope is an instance; a property is written into a module
    // definition. Ask the elaborator which module the named instance is, and
    // fall back to treating the name as a module when it is not an instance.
    if (mineModule != top) {
        const auto defined = simgen::definingModule(designFiles, top, mineModule,
                                                    "verilator", work.root());
        if (!defined.empty()) {
            injectModule = defined;
            say("[" + timestamp() + "] mining instance " + mineModule +
                ", writing into module " + injectModule);
        }
    }
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
        // Start fresh: a stale log from a previous run in the same workdir
        // would be worse than none.
        std::error_code error;
        fs::remove(work.runLogFile(), error);
    } catch (const std::exception& e) {
        std::cerr << "smart: cannot prepare " << work.root() << ": " << e.what()
                  << "\n";
        return ExitCode::Failure;
    }

    RunLog runLog(work.runLogFile());
    runLog.record("\"stage\":\"start\",\"version\":\"" + std::string(smart::version()) +
                  "\",\"top\":\"" + jsonEscape(top) +
                  "\",\"design\":\"" + jsonEscape(mainFile) +
                  "\",\"jobs\":" + std::to_string(options_.getInt("jobs")) +
                  ",\"core_timeout\":" +
                  std::to_string(options_.getInt("core_timeout")));

    for (const auto& warning : options_.warnings()) say("note: " + warning);

    // ---- simulate ------------------------------------------------------
    simgen::HarnessOptions harness;
    harness.cycles = static_cast<int>(options_.getInt("cycles"));
    harness.traces = static_cast<int>(options_.getInt("traces"));
    harness.seed = static_cast<unsigned>(options_.getInt("seed"));
    harness.clock = options_.getString("clock");
    harness.reset = parseResetOption(options_.getString("reset"));
    harness.firstIndex = 1;  // sim1.vcd .. simN.vcd, as the legacy tree had
    harness.simulationTimeoutSeconds =
        static_cast<int>(options_.getInt("simulation_timeout"));

    const auto simulatorName = options_.getString("simulator");
    if (simulatorName == "iverilog" || simulatorName == "icarus") {
        harness.simulator = simgen::Simulator::Icarus;
    } else if (simulatorName == "verilator" || simulatorName.empty()) {
        harness.simulator = simgen::Simulator::Verilator;
    } else {
        std::cerr << "smart: unknown simulator '" << simulatorName
                  << "' (known: verilator, iverilog)\n";
        return ExitCode::UserError;
    }

    const auto policyName = options_.getString("trace_policy");
    if (policyName == "fuzz") {
        if (harness.simulator != simgen::Simulator::Verilator) {
            std::cerr << "smart: --trace-policy fuzz needs the verilator "
                         "backend (the Icarus harness has no stimulus hook)\n";
            return ExitCode::UserError;
        }
        harness.policy = simgen::TracePolicy::Fuzz;
        harness.fuzzIterations =
            static_cast<int>(options_.getInt("fuzz_iterations"));
    } else if (policyName == "random" || policyName.empty()) {
        harness.policy = simgen::TracePolicy::Random;
    } else {
        std::cerr << "smart: unknown trace policy '" << policyName
                  << "' (known: random, fuzz)\n";
        return ExitCode::UserError;
    }

    say("[" + timestamp() + "] simulating " + std::to_string(harness.traces) +
        " traces x " + std::to_string(harness.cycles) + " cycles with " +
        simulatorName + ", " + policyName + " stimulus");
    try {
        // Simulate the design as it sits in the workdir, so the traces match
        // the sources the blocks reason about.
        std::vector<std::string> workdirDesign;
        for (const auto& file : designFiles)
            workdirDesign.push_back(work.verilogDir() + "/" +
                                    fs::path(file).filename().string());

        // The workdir root, not sim_src: runSimulations makes its own
        // sim_src/ underneath for the assume-stripped copies.
        const auto sim = simgen::runSimulations(info, workdirDesign, work.root(),
                                                work.simResultsDir(), harness);
        runLog.record("\"stage\":\"simulate\",\"simulator\":\"" +
                      jsonEscape(simulatorName) + "\",\"traces\":" +
                      std::to_string(harness.traces) + ",\"cycles\":" +
                      std::to_string(harness.cycles));
        if (sim.fuzz.ran) {
            say("[" + timestamp() + "] fuzz: " +
                std::to_string(sim.fuzz.statesSelected) + " states in " +
                std::to_string(harness.traces) + " traces (random baseline " +
                std::to_string(sim.fuzz.statesRandom) + ", " +
                std::to_string(sim.fuzz.statesSeen) + " seen over " +
                std::to_string(sim.fuzz.iterations) + " runs)");
            runLog.record("\"stage\":\"fuzz\",\"iterations\":" +
                          std::to_string(sim.fuzz.iterations) +
                          ",\"states_selected\":" +
                          std::to_string(sim.fuzz.statesSelected) +
                          ",\"states_seen\":" +
                          std::to_string(sim.fuzz.statesSeen) +
                          ",\"states_random\":" +
                          std::to_string(sim.fuzz.statesRandom));
        }
    } catch (const simgen::MissingToolError& e) {
        std::cerr << "smart: " << e.what() << "\n"
                  << "smart: the simulator is not on PATH. Either add it,\n"
                     "       e.g. from otherTools/oss-cad-suite/bin, or run "
                     "./install.sh;\n"
                     "       `smart --check-env` reports what is missing, and\n"
                     "       `--simulator iverilog` selects the other backend.\n";
        summary.workDirKept = true;
        return ExitCode::EnvironmentError;
    } catch (const std::exception& e) {
        std::cerr << "smart: simulation failed: " << e.what() << "\n";
        summary.workDirKept = true;
        return ExitCode::Failure;
    }

    // ---- pre-analysis --------------------------------------------------
    std::vector<std::string> variables;
    try {
        variables = candidateVariables(mineModule, work.simResultsDir());
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
    runLog.record("\"stage\":\"pre-analysis\",\"variables\":" +
                  std::to_string(variables.size()) + ",\"k\":" +
                  std::to_string(plan.k) + ",\"blocks\":" +
                  std::to_string(plan.threadBlocks + plan.initBlocks));

    // ---- synthesis rounds ----------------------------------------------
    BlockRunnerOptions runner;
    runner.executable = self_;
    runner.workDir = work.root();
    runner.topModule = top;
    runner.mineModule = mineModule;
    runner.injectModule = injectModule;
    runner.configPath = work.effectiveConfigFile();
    runner.logsDir = work.logsDir();
    runner.jobs = jobs;
    runner.coreTimeoutSeconds = static_cast<int>(options_.getInt("core_timeout"));

    const auto deadline =
        runStart + std::chrono::seconds(options_.getInt("timeout"));

    std::set<std::string> mined;
    int round = 0;
    int blockCounter = 0;

    // LTL mode mines once per latency, 0..ltl_depth: latency 0 gives plain
    // safety properties, and each step above it lets the synthesiser reach one
    // cycle further back. The block code has always understood the latency
    // argument (printLTLSygusPath); this is the loop around it.
    const long long ltlDepth =
        options_.getBool("ltl") ? options_.getInt("ltl_depth") : 0;

    for (long long latency = 0; latency <= ltlDepth; ++latency) {
    if (latency > 0) {
        say("[" + timestamp() + "] LTL latency " + std::to_string(latency) +
            "/" + std::to_string(ltlDepth));
        // Each latency starts from the full initial block set, as the legacy
        // flow did by re-running its pre-analysis per latency.
        std::error_code error;
        fs::remove_all(work.variablesDir(), error);
        fs::create_directories(work.variablesDir());
        writeInitialBlocks(variables, work.variablesDir(),
                           options_.getBool("blockified"),
                           options_.getDouble("block_size"), cores, rng);
    }

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
            job.latency = static_cast<int>(latency);
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

                // Why a block found nothing decides what to do about it, and
                // the three reasons want opposite responses. Measured on
                // s838/c880: infeasible 44-47%, refuted 0.5-29%, killed
                // 0-4%. Without this the log said "no-assertion" for all
                // three and the exit code had to be decoded by hand.
                //
                //   0    the block verified an assertion
                //   1    a candidate was synthesised and did not verify
                //   255  smart.cpp exit(-1): SyGuS found nothing, or cvc5 gave
                //        up on the problem
                //   127  the child never got as far as running
                //   -1   killed by a signal, i.e. it ran out of wall clock
                const char* reason =
                    result.exitCode == 0     ? "verified"
                    : result.exitCode == 1   ? "refuted"
                    : result.exitCode == 255 ? "infeasible"
                    : result.exitCode == 127 ? "setup-failed"
                    : result.exitCode < 0    ? "killed"
                                             : "other";

                std::ostringstream record;
                record << "\"reason\":\"" << reason << "\","
                       << "\"block\":\"" << jsonEscape(result.job.coreId)
                       << "\",\"round\":" << round
                       << ",\"latency\":" << result.job.latency
                       << ",\"pid\":" << result.pid
                       << ",\"status\":\"" << statusName(result.status)
                       << "\",\"exit\":" << result.exitCode
                       << ",\"seconds\":" << std::fixed << std::setprecision(2)
                       << result.seconds;
                if (result.killed) record << ",\"killed\":true";
                // The one that matters when a benchmark goes wrong: we killed
                // the group and something in it was still there afterwards.
                if (result.survivedKill) record << ",\"survived_kill\":true";
                if (!result.assertion.empty())
                    record << ",\"assertion\":\"" << jsonEscape(result.assertion)
                           << "\"";
                runLog.record(record.str());
                if (!quiet && (done % 10 == 0 || done == total))
                    std::cout << "[" << timestamp() << "]   " << done << "/" << total
                              << " blocks, " << mined.size() << " assertions"
                              << std::endl;
            });

        say("[" + timestamp() + "] round " + std::to_string(round) + " found " +
            std::to_string(foundThisRound) + " new assertions (" +
            std::to_string(mined.size()) + " total)");

        {
            std::size_t timedOut = 0, failed = 0, verified = 0;
            for (const auto& result : results) {
                if (result.status == BlockStatus::TimedOut) ++timedOut;
                else if (result.status == BlockStatus::Failed) ++failed;
                else if (result.status == BlockStatus::Verified) ++verified;
            }
            runLog.record("\"stage\":\"round\",\"round\":" + std::to_string(round) +
                          ",\"blocks\":" + std::to_string(jobsThisRound.size()) +
                          ",\"verified\":" + std::to_string(verified) +
                          ",\"timed_out\":" + std::to_string(timedOut) +
                          ",\"failed\":" + std::to_string(failed) +
                          ",\"new\":" + std::to_string(foundThisRound) +
                          ",\"total\":" + std::to_string(mined.size()));
        }

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
            const auto minStart = std::chrono::steady_clock::now();
            try {
                const auto minimised = mus::minimiseAssertions(
                    work.sygusResultFile(), work.sygusResultFile(),
                    static_cast<int>(options_.getInt("block_minimizer_timeout")));
                if (minimised.before != minimised.after)
                    say("[" + timestamp() + "] minimiser: " +
                        std::to_string(minimised.before) + " -> " +
                        std::to_string(minimised.after) + " invariants");
                runLog.record(
                    "\"stage\":\"minimiser\",\"round\":" + std::to_string(round) +
                    ",\"before\":" + std::to_string(minimised.before) +
                    ",\"after\":" + std::to_string(minimised.after) +
                    ",\"secs\":" + std::to_string(seconds(minStart)));
            } catch (const std::exception& e) {
                say("note: minimiser skipped: " + std::string(e.what()));
                runLog.record(
                    "\"stage\":\"minimiser\",\"round\":" + std::to_string(round) +
                    ",\"error\":\"" + jsonEscape(e.what()) + "\",\"secs\":" +
                    std::to_string(seconds(minStart)));
            }
        }

        const bool useMsa = options_.getBool("msa");
        const bool useRandom = options_.getBool("random") || !useMsa;

        std::vector<std::string> msaPool;
        if (useMsa) {
            const auto msaStart = std::chrono::steady_clock::now();
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
                runLog.record(
                    "\"stage\":\"msa\",\"round\":" + std::to_string(round) +
                    ",\"pool\":" + std::to_string(msaPool.size()) +
                    ",\"variables\":" + std::to_string(variables.size()) +
                    ",\"timed_out\":" + (found.timedOut ? "true" : "false") +
                    ",\"secs\":" + std::to_string(seconds(msaStart)));
            } catch (const std::exception& e) {
                say("note: MSA failed (" + std::string(e.what()) +
                    "), falling back to random blocks");
                runLog.record(
                    "\"stage\":\"msa\",\"round\":" + std::to_string(round) +
                    ",\"error\":\"" + jsonEscape(e.what()) + "\",\"secs\":" +
                    std::to_string(seconds(msaStart)));
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

    if (std::chrono::steady_clock::now() >= deadline) {
        if (latency < ltlDepth) say("global timeout reached; skipping the "
                                    "remaining LTL latencies");
        break;
    }
    }  // latency

    summary.rounds = round;
    summary.minedAssertions = mined.size();

    std::vector<std::string> assertions(mined.begin(), mined.end());

    // End-of-run minimisation: drop assertions the rest already imply. The
    // minimiser works on the `.sl` definitions, so the survivors have to be
    // mapped back to the Verilog strings we emit — runtime/CompareResult.txt
    // is that mapping, written by each block as
    //     <verilog assertion>:
    //     ( (define-fun inv (...) Bool <body>) )
    if (options_.getBool("minimizer") && options_.getBool("end_minimizer") &&
        assertions.size() > 1) {
        try {
            const auto reduced = work.runtime() + "/reducedResult.sl";
            const auto minimised = mus::minimiseAssertions(
                work.sygusResultFile(), reduced,
                static_cast<int>(options_.getInt("end_minimizer_timeout")));

            // Which define-fun bodies survived.
            std::set<std::string> keptBodies;
            for (const auto& line : minimised.keptDefinitions) {
                const auto open = line.find("Bool ");
                if (open == std::string::npos) continue;
                auto body = line.substr(open + 5);
                while (!body.empty() && (body.back() == ')' || body.back() == ' '))
                    body.pop_back();
                keptBodies.insert(body);
            }

            // Walk the mapping and keep only the assertions whose definition
            // survived. An assertion we cannot find a definition for is KEPT:
            // dropping something we failed to understand would silently weaken
            // the result.
            std::map<std::string, bool> survives;
            {
                std::ifstream in(work.compareResultFile());
                std::string line, current;
                while (std::getline(in, line)) {
                    if (!line.empty() && line.back() == ':' &&
                        line.find("define-fun") == std::string::npos) {
                        current = line.substr(0, line.size() - 1);
                        continue;
                    }
                    if (current.empty() || line.find("define-fun") == std::string::npos)
                        continue;
                    const auto open = line.find("Bool ");
                    if (open == std::string::npos) continue;
                    auto body = line.substr(open + 5);
                    while (!body.empty() && (body.back() == ')' || body.back() == ' '))
                        body.pop_back();
                    if (keptBodies.count(body) != 0) survives[current] = true;
                    else if (survives.find(current) == survives.end())
                        survives[current] = false;
                    current.clear();
                }
            }

            std::vector<std::string> kept;
            for (const auto& assertion : assertions) {
                const auto found = survives.find(assertion);
                if (found == survives.end() || found->second) kept.push_back(assertion);
            }

            if (!kept.empty() && kept.size() < assertions.size()) {
                say("[" + timestamp() + "] end minimiser: " +
                    std::to_string(assertions.size()) + " -> " +
                    std::to_string(kept.size()) + " assertions");
                runLog.record("\"stage\":\"end-minimiser\",\"before\":" +
                              std::to_string(assertions.size()) + ",\"after\":" +
                              std::to_string(kept.size()));
                assertions = kept;
            }
        } catch (const std::exception& e) {
            say("note: end minimiser skipped: " + std::string(e.what()));
        }
    }

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
        check.injectModule = injectModule;
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
        std::size_t refuted = 0, timedOutChecks = 0, errors = 0;
        for (const auto& result : checked) {
            switch (result.status) {
                case emit::CheckStatus::Verified: verified.push_back(result.assertion); break;
                case emit::CheckStatus::Refuted: ++refuted; break;
                case emit::CheckStatus::TimedOut: ++timedOutChecks; break;
                case emit::CheckStatus::Error: ++errors; break;
            }
        }
        // Record HOW they were proved, not just how many. A bounded run
        // emits properties that hold to `bound` only — sound to report, but
        // NOT invariants, and unsound to assume in someone else's proof.
        // Anything reading invariants.txt has to know which it got.
        runLog.record(std::string("\"stage\":\"check\",\"mode\":\"") +
                      (check.unbounded ? "k-induction" : "bounded") + "\",\"bound\":" +
                      (check.unbounded ? std::string("null")
                                       : std::to_string(check.bound)) +
                      ",\"checked\":" +
                      std::to_string(checked.size()) + ",\"verified\":" +
                      std::to_string(verified.size()) + ",\"refuted\":" +
                      std::to_string(refuted) + ",\"timed_out\":" +
                      std::to_string(timedOutChecks) + ",\"errors\":" +
                      std::to_string(errors));
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
        emit::writeAssertionFile(mainFile, output, injectModule, verified,
                                 options_.toJson());
    } catch (const std::exception& e) {
        std::cerr << "smart: cannot write " << output << ": " << e.what() << "\n";
        summary.workDirKept = true;
        return ExitCode::Failure;
    }

    summary.seconds =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - runStart)
            .count();
    runLog.record("\"stage\":\"done\",\"mined\":" +
                  std::to_string(summary.minedAssertions) + ",\"verified\":" +
                  std::to_string(summary.verifiedAssertions) + ",\"rounds\":" +
                  std::to_string(summary.rounds) + ",\"output\":\"" +
                  jsonEscape(output) + "\"");

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

    std::cout << "[ok]      smart     " << selfExecutable << "  " << smart::version()
              << "\n";
    std::cout << "jobs default: " << options.getInt("jobs") << "\n";
    return ok ? ExitCode::Success : ExitCode::EnvironmentError;
}

}  // namespace pipeline
}  // namespace smart
