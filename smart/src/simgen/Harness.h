// Harness: generate a Verilog testbench for a design and run it under
// Icarus Verilog to produce VCD traces (WP3). Replaces the cocotb + Verilator
// flow; see docs/PLAN-cpp-single-binary.md WP3 for why iverilog.
//
// Stimulus semantics are a port of gen_bench.render_sim_py — the frozen
// behavioural spec — not of its code: seed the RNG, draw each `(* anyconst *)`
// register once, hold reset for `reset.cycles`, then per cycle draw every
// driven input and every `(* anyseq *)` register and advance one clock.

#ifndef SMART_SIMGEN_HARNESS_H
#define SMART_SIMGEN_HARNESS_H

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "SVModule.h"

namespace smart {
namespace simgen {

// Per-signal stimulus override, mirroring gen_bench's `_spec_for`.
// Precedence: values > const > max > full width range.
struct SignalSpec {
    std::optional<long long> constant;
    std::vector<long long> values;
    std::optional<long long> maxValue;
};

struct HarnessOptions {
    int cycles = 10;          // trace depth (--cycles)
    int traces = 3;           // number of VCDs (--traces)
    unsigned seed = 42;       // trace i uses seed + i (--seed)
    int firstIndex = 0;       // VCDs are named sim<firstIndex + i>.vcd

    // Overrides keyed by signal name (config `inputs` section).
    std::map<std::string, SignalSpec> inputs;

    // Explicit clock / reset, otherwise guessed from the ModuleInfo.
    std::string clock;
    std::optional<frontend::ResetInfo> reset;

    // Tools; resolved on PATH when left empty.
    std::string iverilog = "iverilog";
    std::string vvp = "vvp";
};

// Render the testbench source. Pure function of (info, options) — no I/O —
// so it can be unit-tested and diffed.
//
// The DUT instance is named after the top module ON PURPOSE: the VCD scope
// name is the instance name, and the trace loader matches signals with
// `scope->name == <top>` (Trace::createSignal / Module::getAllSignals).
std::string renderTestbench(const frontend::ModuleInfo& info,
                            const HarnessOptions& options);

struct SimResult {
    std::vector<std::string> vcdPaths;  // one per trace, in seed order
    std::string command;                // last command run, for diagnostics
};

// Write the testbench next to `designFiles`, compile with iverilog, and run
// `options.traces` simulations into `<outputDir>/sim<i>.vcd`.
// `workDir` holds the generated tb.sv and the compiled simulation.
// Throws std::runtime_error with the tool output on failure.
SimResult runSimulations(const frontend::ModuleInfo& info,
                         const std::vector<std::string>& designFiles,
                         const std::string& workDir,
                         const std::string& outputDir,
                         const HarnessOptions& options);

}  // namespace simgen
}  // namespace smart

#endif  // SMART_SIMGEN_HARNESS_H
