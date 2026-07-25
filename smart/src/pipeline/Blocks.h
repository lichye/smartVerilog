// Blocks: candidate-variable bookkeeping and block generation.
//
// A "block" is one SyGuS problem over a subset of the design's variables.
// This is the port of preAnalyzer.py (the initial subsets) and of
// smart.py's GenerateNewBlocks() (the subsequent rounds), including their
// k/n formulas — those decide how much work a round does, so they are
// reproduced exactly rather than re-derived.

#ifndef SMART_PIPELINE_BLOCKS_H
#define SMART_PIPELINE_BLOCKS_H

#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace smart {
namespace pipeline {

// k = round(2.7 + k_size * log10(V)) — subset size.
int subsetSize(std::size_t variableCount, double kSize);

// n = max(block_size * V^0.9, cores) — number of subsets.
//
// The floor is the machine's core count, NOT --jobs: how many blocks a round
// attempts is a question about coverage, and -j should change how fast a run
// goes, not how much of the space it looks at. (This is also what the legacy
// pipeline did — it read os.cpu_count() here while taking its worker count
// from the config.)
int subsetCount(std::size_t variableCount, double blockSize, int cores);
int hardwareCores();

// Candidate variables for `top`: every signal the VCD traces carry for that
// module's scope.
//
// preAnalyzer.py instead intersected two regex sweeps — one over the VCD text,
// one over the module text. The result is the same set for every design in the
// repo, because a signal in the top scope is by construction declared in the
// module; the regex version merely also let through stray tokens that the
// keyword blocklist then had to remove. Reading the trace loader's own view
// of the VCD is both simpler and exactly what the blocks can actually use:
// smart.cpp drops any variable that is not a signal anyway.
std::vector<std::string> candidateVariables(const std::string& top,
                                            const std::string& simResultsDir);

// Write one file per block into `variablesDir`, named `<prefix><i>.txt`.
// Each holds `k` variables, one per line.
int writeBlocks(const std::vector<std::string>& variables, int count, int k,
                const std::string& variablesDir, const std::string& prefix,
                std::mt19937_64& rng, int startIndex = 0);

// The initial round's blocks, as preAnalyzer.py built them: `thread_*` subsets
// always, plus wider `Init_*` subsets when not running blockified.
struct InitialBlockPlan {
    int threadBlocks = 0;
    int initBlocks = 0;
    int k = 0;
    int initSubsetSize = 0;
};

InitialBlockPlan writeInitialBlocks(const std::vector<std::string>& variables,
                                    const std::string& variablesDir,
                                    bool blockified, double blockSize, int cores,
                                    std::mt19937_64& rng);

}  // namespace pipeline
}  // namespace smart

#endif  // SMART_PIPELINE_BLOCKS_H
