#include "Harness.h"

#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <thread>

#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

namespace smart {
namespace simgen {
namespace {

namespace fs = std::filesystem;

// Characters reserved for the +vcd= plusarg in the generated testbench.
// $value$plusargs right-aligns the string into the register, so a path longer
// than this silently loses its leading characters — and then vvp fails to
// open a path like "ome/user/..." . runSimulations checks against it.
constexpr int kVcdPathChars = 512;

int effectiveWidth(int width) { return width > 0 ? width : 1; }

std::string declRange(int width) {
    return effectiveWidth(width) > 1
               ? "[" + std::to_string(effectiveWidth(width) - 1) + ":0] "
               : "";
}

// One uniform draw over the signal's full range.
//
// `$random(seed)` returns 32 bits; assigning it to a narrower reg truncates,
// which is exactly a uniform draw over 0..2^W-1. Wider signals are filled
// with as many 32-bit draws as they need. `seed` is an inout argument, so
// successive calls advance one reproducible stream.
std::string randomDraw(int width) {
    const int w = effectiveWidth(width);
    const int chunks = (w + 31) / 32;
    if (chunks <= 1) return "$random(seed)";

    std::string expr = "{";
    for (int i = 0; i < chunks; ++i) {
        if (i) expr += ", ";
        expr += "$random(seed)";
    }
    return expr + "}";
}

const SignalSpec* specFor(const HarnessOptions& options, const std::string& name) {
    auto it = options.inputs.find(name);
    return it == options.inputs.end() ? nullptr : &it->second;
}

// The assignment that drives `name` for one draw, honouring config overrides.
std::string driveStatement(const std::string& name, int width,
                           const HarnessOptions& options,
                           const std::string& indent) {
    const SignalSpec* spec = specFor(options, name);

    if (spec != nullptr && !spec->values.empty()) {
        // Pick from an explicit list. $unsigned keeps the modulo positive.
        std::ostringstream os;
        os << indent << "case ($unsigned($random(seed)) % " << spec->values.size()
           << ")\n";
        for (size_t i = 0; i < spec->values.size(); ++i)
            os << indent << "  " << i << ": " << name << " = " << spec->values[i]
               << ";\n";
        os << indent << "endcase\n";
        return os.str();
    }

    if (spec != nullptr && spec->constant.has_value())
        return indent + name + " = " + std::to_string(*spec->constant) + ";\n";

    if (spec != nullptr && spec->maxValue.has_value())
        return indent + name + " = $unsigned($random(seed)) % " +
               std::to_string(*spec->maxValue + 1) + ";\n";

    return indent + name + " = " + randomDraw(width) + ";\n";
}

std::string run(const std::string& command, int& exitCode) {
    std::string output;
    std::array<char, 4096> buffer{};

    FILE* pipe = popen((command + " 2>&1").c_str(), "r");
    if (pipe == nullptr) throw std::runtime_error("cannot run: " + command);

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
        output += buffer.data();

    const int status = pclose(pipe);
    exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    return output;
}

// Run a command under a deadline, killing its whole process group if it
// overruns. A simulation that never terminates is not hypothetical: gate-level
// netlists driven with a random clock can enter a zero-delay loop and spin at
// one timestamp forever.
std::string runWithDeadline(const std::string& command, int seconds,
                            int& exitCode, bool& timedOut) {
    timedOut = false;
    if (seconds <= 0) return run(command, exitCode);

    const std::string tmp =
        fs::temp_directory_path().string() + "/smart-sim-" +
        std::to_string(::getpid()) + "-" + std::to_string(::rand());

    const pid_t pid = fork();
    if (pid < 0) { exitCode = -1; return "fork failed"; }
    if (pid == 0) {
        setpgid(0, 0);
        freopen(tmp.c_str(), "w", stdout);
        dup2(STDOUT_FILENO, STDERR_FILENO);
        execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
        _exit(127);
    }
    setpgid(pid, pid);

    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
    int status = 0;
    for (;;) {
        const pid_t waited = waitpid(pid, &status, WNOHANG);
        if (waited == pid) break;
        if (waited < 0) { exitCode = -1; break; }
        if (std::chrono::steady_clock::now() >= deadline) {
            kill(-pid, SIGKILL);
            waitpid(pid, &status, 0);
            timedOut = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    exitCode = timedOut ? -1 : (WIFEXITED(status) ? WEXITSTATUS(status) : -1);

    std::ifstream in(tmp);
    std::ostringstream text;
    if (in) text << in.rdbuf();
    std::error_code error;
    fs::remove(tmp, error);
    return text.str();
}

std::string quote(const std::string& s) { return "'" + s + "'"; }

bool onPath(const std::string& tool) {
    if (tool.find('/') != std::string::npos) return fs::exists(tool);
    const char* path = std::getenv("PATH");
    if (path == nullptr) return false;
    std::istringstream entries(path);
    std::string dir;
    while (std::getline(entries, dir, ':')) {
        std::error_code error;
        if (!dir.empty() && fs::exists(fs::path(dir) / tool, error)) return true;
    }
    return false;
}

}  // namespace

std::string renderTestbench(const frontend::ModuleInfo& info,
                            const HarnessOptions& options) {
    const auto clock = options.clock.empty() ? frontend::guessClock(info)
                                             : std::optional<std::string>(options.clock);
    const auto reset = clock ? (options.reset ? options.reset
                                              : frontend::guessReset(info, clock))
                             : std::optional<frontend::ResetInfo>();

    std::ostringstream os;
    os << "// Auto-generated by SMART (simgen/Harness.cpp) — do not edit.\n"
       << "// Design: " << info.top
       << "  clock: " << (clock ? *clock : "none (combinational)")
       << "  reset: " << (reset ? reset->signal : "none") << "\n"
       << "`timescale 1ns/1ps\n\n"
       << "module tb;\n"
       << "  integer seed;\n"
       << "  reg [8*" << kVcdPathChars << ":1] vcd_file;\n\n";

    // Every port gets a tb-side signal: inputs are driven, outputs observed.
    for (const auto& port : info.ports) {
        const bool driven = port.dir != "output";
        os << "  " << (driven ? "reg  " : "wire ") << declRange(port.width)
           << port.name << ";\n";
    }
    os << "\n";

    // Instance name == module name: the VCD scope the trace loader looks for.
    os << "  " << info.top << " " << info.top << " (\n";
    for (size_t i = 0; i < info.ports.size(); ++i) {
        os << "    ." << info.ports[i].name << "(" << info.ports[i].name << ")"
           << (i + 1 < info.ports.size() ? "," : "") << "\n";
    }
    os << "  );\n\n";

    os << "  integer cycle;\n\n";

    if (clock) {
        os << "  // 2ns period, matching the legacy cocotb harness.\n"
           << "  always #1 " << *clock << " = ~" << *clock << ";\n\n";
    }

    os << "  initial begin\n"
       << "    if (!$value$plusargs(\"seed=%d\", seed)) seed = " << options.seed
       << ";\n"
       << "    if (!$value$plusargs(\"vcd=%s\", vcd_file)) vcd_file = \"dump.vcd\";\n"
       << "    $dumpfile(vcd_file);\n"
       << "    $dumpvars(0, tb." << info.top << ");\n\n";

    if (clock) os << "    " << *clock << " = 0;\n";

    // anyconst: drawn once, before anything else runs.
    for (const auto& reg : info.freeRegs) {
        if (reg.kind != "anyconst") continue;
        os << driveStatement(info.top + "." + reg.name, reg.width, options, "    ");
    }

    std::vector<frontend::Port> driven;
    for (const auto& port : info.inputs()) {
        if (clock && port.name == *clock) continue;
        if (reset && port.name == reset->signal) continue;
        driven.push_back(port);
    }

    if (reset) {
        os << "\n    " << reset->signal << " = " << reset->active << ";\n"
           << "    repeat (" << reset->cycles << ") @(posedge " << *clock << ");\n"
           << "    " << reset->signal << " = " << (1 - reset->active) << ";\n";
    }

    os << "\n    for (cycle = 0; cycle < " << options.cycles << "; cycle = cycle + 1) begin\n";
    for (const auto& port : driven)
        os << driveStatement(port.name, port.width, options, "      ");
    for (const auto& reg : info.freeRegs) {
        if (reg.kind != "anyseq") continue;
        os << driveStatement(info.top + "." + reg.name, reg.width, options, "      ");
    }
    if (clock)
        os << "      @(posedge " << *clock << ");\n";
    else
        os << "      #10;\n";
    os << "    end\n\n"
       << "    $finish;\n"
       << "  end\n"
       << "endmodule\n";

    return os.str();
}

SimResult runSimulations(const frontend::ModuleInfo& info,
                         const std::vector<std::string>& designFiles,
                         const std::string& workDir,
                         const std::string& outputDir,
                         const HarnessOptions& options) {
    if (designFiles.empty())
        throw std::runtime_error("no design files to simulate");

    // Check before writing anything: "iverilog: command not found" buried in a
    // compiler log is a worse answer than saying which tool is missing.
    if (options.simulator == Simulator::Verilator) {
        if (!onPath(options.verilator)) throw MissingToolError(options.verilator);
    } else {
        for (const auto& tool : {options.iverilog, options.vvp})
            if (!onPath(tool)) throw MissingToolError(tool);
    }

    fs::create_directories(workDir);
    fs::create_directories(outputDir);

    const auto tbPath = fs::path(workDir) / "tb.sv";

    // Simulation runs on assume-STRIPPED copies: an `assume(...)` that the
    // random stimulus violates would fire as a runtime assertion under
    // iverilog and pollute the trace. The originals stay untouched for the
    // formal side (plan WP3 / WP7).
    const auto simSrcDir = fs::path(workDir) / "sim_src";
    fs::create_directories(simSrcDir);

    std::vector<std::string> strippedFiles;
    strippedFiles.reserve(designFiles.size());
    for (const auto& file : designFiles) {
        std::ifstream in(file);
        if (!in) throw std::runtime_error("cannot read " + file);
        std::ostringstream text;
        text << in.rdbuf();

        const auto stripped = simSrcDir / fs::path(file).filename();
        std::ofstream out(stripped);
        if (!out) throw std::runtime_error("cannot write " + stripped.string());
        out << frontend::stripAssumes(text.str());
        strippedFiles.push_back(stripped.string());
    }

    const bool useVerilator = options.simulator == Simulator::Verilator;
    const auto simPath = fs::path(workDir) /
                         (useVerilator ? "obj_dir/sim_" + info.top : "sim.vvp");

    SimResult result;
    int exitCode = 0;

    std::ostringstream compile;
    if (useVerilator) {
        // The C++ testbench replaces tb.sv; keep both names distinct so a
        // workdir from the other simulator is never mistaken for this one.
        const auto harnessPath = fs::path(workDir) / "sim_main.cpp";
        {
            std::ofstream out(harnessPath);
            if (!out) throw std::runtime_error("cannot write " + harnessPath.string());
            out << renderVerilatorHarness(info, options);
        }

        compile << options.verilator << " --cc --exe --build --trace"
                // We mine from the user's design; we do not lint it. Verilator
                // treats warnings as fatal by default, and real designs carry
                // things like an out-of-range part-select in a branch their
                // parameters make unreachable (axis_fifo). Warnings still land
                // in the log, they just do not stop the run.
                << " -Wno-fatal"
                << " -Wno-WIDTHEXPAND -Wno-WIDTHTRUNC -Wno-UNOPTFLAT"
                << " -Wno-CASEOVERLAP -Wno-MULTIDRIVEN -Wno-BLKANDNBLK"
                // Internal free registers are only reachable when they are
                // public; the old cocotb flow passed this unconditionally.
                << " --public-flat-rw"
                << " --Mdir " << quote((fs::path(workDir) / "obj_dir").string())
                << " --top-module " << info.top
                << " -o " << quote("sim_" + info.top);
        compile << " -I" << quote(simSrcDir.string());
        for (const auto& file : strippedFiles) compile << " " << quote(file);
        compile << " " << quote(harnessPath.string());
    } else {
        std::ofstream out(tbPath);
        if (!out) throw std::runtime_error("cannot write " + tbPath.string());
        out << renderTestbench(info, options);

        compile << options.iverilog << " -g2012 -o " << quote(simPath.string());
        // Both the stripped copies (for `include "sibling.sv"` between them)
        // and the original directory, for includes we did not copy.
        compile << " -I " << quote(simSrcDir.string());
        compile << " -I "
                << quote(fs::path(designFiles.front()).parent_path().string());
        for (const auto& file : strippedFiles) compile << " " << quote(file);
        compile << " " << quote(tbPath.string());
    }

    result.command = compile.str();
    const auto compileOutput = run(result.command, exitCode);
    if (exitCode != 0)
        throw std::runtime_error(std::string(useVerilator ? "verilator" : "iverilog") +
                                 " failed:\n" + compileOutput +
                                 "\ncommand: " + result.command);

    for (int i = 0; i < options.traces; ++i) {
        const auto vcdPath =
            fs::path(outputDir) /
            ("sim" + std::to_string(options.firstIndex + i) + ".vcd");
        if (vcdPath.string().size() >= kVcdPathChars)
            throw std::runtime_error(
                "VCD path is longer than the " + std::to_string(kVcdPathChars) +
                " characters the testbench reserves for it: " + vcdPath.string());

        std::ostringstream simulate;
        if (useVerilator)
            simulate << quote(simPath.string());
        else
            simulate << options.vvp << " " << quote(simPath.string());
        simulate << " +seed=" << (options.seed + i)
                 << " +vcd=" << quote(vcdPath.string());
        result.command = simulate.str();

        bool timedOut = false;
        const auto simOutput = runWithDeadline(
            result.command, options.simulationTimeoutSeconds, exitCode, timedOut);
        if (timedOut)
            throw std::runtime_error(
                "simulation did not finish within " +
                std::to_string(options.simulationTimeoutSeconds) +
                "s — the design may have been driven into a zero-delay loop.\n"
                "command: " + result.command);
        if (exitCode != 0)
            throw std::runtime_error("simulation failed:\n" + simOutput +
                                     "\ncommand: " + result.command);
        if (!fs::exists(vcdPath))
            throw std::runtime_error("simulation produced no VCD at " +
                                     vcdPath.string() + "\n" + simOutput);

        result.vcdPaths.push_back(vcdPath.string());
    }

    return result;
}

}  // namespace simgen
}  // namespace smart
