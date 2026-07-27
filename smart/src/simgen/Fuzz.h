// Search for stimulus that visits more of the design than uniform random draws
// do.
//
// The motivating case is a control FSM whose interesting states sit behind a
// run of consecutive cycles holding one input steady. Drawing every input
// independently every cycle, the probability of such a run falls off
// exponentially in its length — on plena_data_flow_control_flat, completing one
// load needs eight consecutive cycles out of a 3-in-16 subset, about 1.5e-6 per
// starting position, so a random trace never gets there and no constraint can
// ever mention those states.
//
// What this does NOT do is make traces longer. Depth costs the synthesiser:
// more sampled cycles means more positive constraints per block, and the SyGuS
// call has a fixed time budget. The search runs many throwaway simulations and
// hands back the same number of sequences, of the same depth, as the random
// policy would have produced. Only their content changes.

#ifndef SMART_SIMGEN_FUZZ_H
#define SMART_SIMGEN_FUZZ_H

#include <string>
#include <vector>

namespace smart {
namespace simgen {

struct FuzzInput {
    std::string simulator;   // built model, e.g. <workdir>/obj_dir/sim_<top>
    std::string scratchDir;  // where candidate stimulus files are written

    // Bit widths of the values the harness consumes, in the order it reads
    // them: the anyconst registers once, then per cycle the driven ports
    // followed by the anyseq registers.
    std::vector<int> constWidths;
    std::vector<int> cycleWidths;

    int cycles = 10;
    int traces = 3;
    int iterations = 500;  // simulation budget for the search
    unsigned seed = 42;
    int timeoutSeconds = 300;
};

struct FuzzResult {
    // `traces` stimulus files, chosen to maximise the union of states reached.
    std::vector<std::string> stimulusFiles;
    int iterations = 0;      // simulations actually run
    int statesSelected = 0;  // union covered by the files above
    int statesSeen = 0;      // union over everything the search tried
    int statesRandom = 0;    // what the same budget of plain random draws got,
                             // measured on the first `traces` candidates
};

// Runs the search. Never throws for a failed simulation — a candidate that
// crashes or times out simply scores nothing.
FuzzResult fuzzStimulus(const FuzzInput& input);

}  // namespace simgen
}  // namespace smart

#endif  // SMART_SIMGEN_FUZZ_H
