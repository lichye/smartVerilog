// SVModule: SystemVerilog top-module model for the smart frontend.
//
// Behavioral spec: smart/src/python/gen_bench.py (frozen; see
// docs/PLAN-cpp-single-binary.md WP2). Every function here mirrors the
// semantics of its Python counterpart exactly — including quirks — and is
// held to byte parity by tools/parity_frontend.py across all repo
// benchmarks. If a quirk turns out to be a bug, fix gen_bench.py first and
// keep the two in lockstep until WP8 retires the Python.

#ifndef SMART_FRONTEND_SVMODULE_H
#define SMART_FRONTEND_SVMODULE_H

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace smart {
namespace frontend {

struct Port {
    std::string dir;   // "input" | "output" | "inout"
    int width;         // resolved bit width; -1 if unresolvable (Python None)
    std::string name;
};

struct FreeReg {
    std::string kind;  // "anyseq" | "anyconst"
    int width;         // -1 if unresolvable
    std::string name;
};

struct ResetInfo {
    std::string signal;
    int active;        // 1 = active high, 0 = active low
    int cycles;        // reset assertion length, default 2
};

struct ModuleInfo {
    std::string top;
    std::map<std::string, long long> params;  // insertion order not needed:
                                              // substitution is \b-bounded
    std::vector<Port> ports;
    std::vector<FreeReg> freeRegs;
    bool hasAssume = false;

    std::vector<Port> inputs() const;
};

// Parse module `top` out of SystemVerilog source `text`.
// Throws std::runtime_error("module '<top>' not found") when absent
// (mirrors gen_bench.parse_module raising ValueError).
ModuleInfo parseModule(const std::string& text, const std::string& top);

// Remove immediate `assume(...);` / `assume property (...);` statements,
// each replaced by a null statement `;` so `if (c) assume(x); else ...`
// stays legal. Operates on the raw (uncommented-stripped) text.
std::string stripAssumes(const std::string& text);

// Insert an `always @(posedge <clock>) begin assume(...); ... end` block
// (or `always @(*)` when clock is empty) before the top module's endmodule.
// Throws if the module is missing. No-op when assumes is empty.
std::string injectAssumes(const std::string& text, const std::string& top,
                          const std::vector<std::string>& assumes,
                          const std::string& clock = "");

// Clock/reset guessing. `configured` overrides when non-empty (clock) /
// engaged (reset). Candidate lists and active-level inference mirror
// gen_bench.CLOCK_NAMES / RESET_NAMES.
std::optional<std::string> guessClock(const ModuleInfo& info,
                                      const std::string& configured = "");
std::optional<ResetInfo> guessReset(const ModuleInfo& info,
                                    const std::optional<std::string>& clock,
                                    const std::optional<ResetInfo>& configured =
                                        std::nullopt);

// Serialize the full frontend view (module info + clock/reset guesses) as
// canonical JSON for the parity harness / --dump-frontend.
std::string dumpJson(const ModuleInfo& info);

}  // namespace frontend
}  // namespace smart

#endif  // SMART_FRONTEND_SVMODULE_H
