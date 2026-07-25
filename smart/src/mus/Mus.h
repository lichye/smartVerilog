// Mus: the MSA/MUS machinery and the assertion minimiser, on libcvc5.
//
// Port of minimal_satisfiable_assignment.py + minimise_assertions.py
// (+ the parts of utils.py they lean on). Behaviour, not code: the algorithms
// below are the same ones, expressed against the cvc5 C++ API.
//
// What it is for: after a round of synthesis, `runtime/SygusResult.sl` holds
// the invariants found so far. getMus() asks which variables those invariants
// still leave UNDERSPECIFIED — that set is the pool the next round's blocks
// are drawn from, which is what makes blockified mode converge instead of
// re-mining the same corner. minimiseAssertions() drops invariants that the
// others already imply, so the pool keeps shrinking.

#ifndef SMART_MUS_MUS_H
#define SMART_MUS_MUS_H

#include <map>
#include <set>
#include <string>
#include <vector>

namespace smart {
namespace mus {

struct MusResult {
    // Variables from `variablesFile` that the assertions do NOT pin down.
    std::set<std::string> underspecified;
    // A satisfying assignment for the ones they do, as printed by cvc5.
    std::map<std::string, std::string> assignments;
    // How many independent assertion groups were analysed, and whether any of
    // them ran out of time (the result is still usable, just less minimal).
    int groups = 0;
    bool timedOut = false;
};

// `variablesFile`: one variable name per line (runtime/variables.txt).
// `sygusResultFile`: the accumulated `.sl` define-funs (runtime/SygusResult.sl).
MusResult getMus(const std::string& variablesFile,
                 const std::string& sygusResultFile, int timeoutSeconds);

struct MinimiseResult {
    std::vector<std::string> keptDefinitions;  // the define-fun lines kept
    std::size_t before = 0;
    std::size_t after = 0;
};

// Rewrite `outSl` with the subset of `inSl`'s define-funs that implies all of
// them. In-place is allowed (inSl == outSl), which is how the pipeline uses it.
MinimiseResult minimiseAssertions(const std::string& inSl, const std::string& outSl,
                                  int timeoutSeconds);

}  // namespace mus
}  // namespace smart

#endif  // SMART_MUS_MUS_H
