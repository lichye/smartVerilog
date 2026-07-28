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
#include <stdexcept>
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

// Which simulator generates the traces.
//
// Verilator is the default and what the published results were produced with.
// It is cycle-based and 2-state, which matters more than it sounds: Icarus
// Verilog's event-driven 4-state semantics put x and z into the traces (where
// they then disqualify constraints), and let a design with zero-delay feedback
// spin forever at a single timestamp — s382 and s444, gate-level netlists
// whose CK is driven randomly, hang under Icarus and do not under Verilator.
//
// Icarus stays available: it needs no C++ compile per design and drives
// internal free registers by plain hierarchical assignment.
enum class Simulator { Verilator, Icarus };

// How the stimulus is chosen. Random draws every input independently every
// cycle; Fuzz searches for sequences that reach more states, at the same trace
// count and depth. See Fuzz.h for why depth is not the lever.
enum class TracePolicy { Random, Fuzz };

// A signal the fuzzer scores state diversity over. `isPort` picks the
// accessor: ports are members of the model, internals live on the flattened
// root as <top>__DOT__<name>.
struct StateSignal {
    std::string name;
    bool isPort = false;
    int width = 1;
};

// Enumerate the top module's non-input signals with `verilator --xml-only`,
// which elaborates without compiling. Returns an empty vector if verilator
// cannot be run or the XML cannot be read — the caller then falls back to the
// ports it already knows.
// Map an instance name to the module that defines it, via
// `verilator --xml-only`'s cell list. A VCD scope is an INSTANCE (DFF_0),
// while a property is written into a module DEFINITION (dff), and the two are
// spelled differently. Returns an empty string when the name is not an
// instance — in which case it is already a module name.
std::string definingModule(const std::vector<std::string>& designFiles,
                           const std::string& top, const std::string& instance,
                           const std::string& verilator,
                           const std::string& scratchDir);

std::vector<StateSignal> enumerateStateSignals(
    const std::vector<std::string>& designFiles, const std::string& top,
    const std::string& verilator, const std::string& scratchDir);

// A free register that lives inside an instance rather than in the top
// module. `(* anyseq *)` means "drive this like an input" wherever it is
// written, and Verilator reaches it by its flattened path.
struct HierarchicalFreeReg {
    std::string kind;      // "anyseq" | "anyconst"
    int width = 1;
    std::string flatPath;  // <top>__DOT__<instance>__DOT__<name>
    std::string name;      // local name, for looking up an input spec
};

// Instances in the design, as `verilator --xml-only` reports them: the
// hierarchical path (i2c_master_axil.genblk1.cmd_fifo_inst) and the module it
// is an instance of, with Verilator's parameter-specialisation suffix removed.
struct Instance {
    std::string hierarchy;
    std::string module;
};
std::vector<Instance> enumerateInstances(
    const std::vector<std::string>& designFiles, const std::string& top,
    const std::string& verilator, const std::string& scratchDir);

struct FuzzOutcome {
    bool ran = false;
    int iterations = 0;
    int statesSelected = 0;
    int statesSeen = 0;
    int statesRandom = 0;
};

struct HarnessOptions {
    Simulator simulator = Simulator::Verilator;
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
    std::string verilator = "verilator";

    // What to hash when measuring state diversity. Empty means "outputs and
    // free registers only", which on a design with one output port is far too
    // coarse to steer a search.
    std::vector<StateSignal> stateSignals;

    // Free registers inside instances; the top's own are in ModuleInfo.
    std::vector<HierarchicalFreeReg> hierFreeRegs;

    TracePolicy policy = TracePolicy::Random;
    int fuzzIterations = 500;

    // A simulation that does not terminate must not hang the run: stimulus
    // that drives a design into a zero-delay loop otherwise spins at 100% CPU
    // forever. 0 disables the limit.
    int simulationTimeoutSeconds = 300;
};

// Render the testbench source. Pure function of (info, options) — no I/O —
// so it can be unit-tested and diffed.
//
// The DUT instance is named after the top module ON PURPOSE: the VCD scope
// name is the instance name, and the trace loader matches signals with
// `scope->name == <top>` (Trace::createSignal / Module::getAllSignals).
std::string renderTestbench(const frontend::ModuleInfo& info,
                            const HarnessOptions& options);

// The Verilator equivalent: a C++ testbench driving the model directly.
// Internal `(* anyseq *)` registers are reached through the flattened public
// names Verilator generates under --public-flat-rw (`rootp->top__DOT__name`).
std::string renderVerilatorHarness(const frontend::ModuleInfo& info,
                                   const HarnessOptions& options);

// Bit widths of the values the generated harness consumes, in the order it
// reads them: anyconst registers once, then per cycle the driven ports
// followed by the anyseq registers. The fuzzer needs this to build stimulus
// the harness will accept.
struct DrivenWidths {
    std::vector<int> constant;
    std::vector<int> perCycle;
};
DrivenWidths drivenWidths(const frontend::ModuleInfo& info,
                          const HarnessOptions& options);

struct SimResult {
    std::vector<std::string> vcdPaths;  // one per trace, in seed order
    std::string command;                // last command run, for diagnostics
    FuzzOutcome fuzz;                   // empty unless the fuzz policy ran
};

// Thrown when a tool the harness needs is not on PATH — distinct from a
// simulation that ran and failed, because the two want different answers from
// the user (install something vs. look at your design).
struct MissingToolError : std::runtime_error {
    explicit MissingToolError(const std::string& tool)
        : std::runtime_error(tool + " is not on PATH"), tool(tool) {}
    std::string tool;
};

// Write the testbench next to `designFiles`, compile with iverilog, and run
// `options.traces` simulations into `<outputDir>/sim<i>.vcd`.
// `workDir` holds the generated tb.sv and the compiled simulation.
// Throws MissingToolError when a tool is absent, std::runtime_error with the
// tool output when one runs and fails.
SimResult runSimulations(const frontend::ModuleInfo& info,
                         const std::vector<std::string>& designFiles,
                         const std::string& workDir,
                         const std::string& outputDir,
                         const HarnessOptions& options);

}  // namespace simgen
}  // namespace smart

#endif  // SMART_SIMGEN_HARNESS_H
