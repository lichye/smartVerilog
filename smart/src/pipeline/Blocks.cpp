#include "Blocks.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <set>
#include <thread>
#include <stdexcept>

#include "Module.h"

namespace smart {
namespace pipeline {
namespace {
namespace fs = std::filesystem;
}

int subsetSize(std::size_t variableCount, double kSize) {
    if (variableCount == 0) return 0;
    const double k = 2.7 + kSize * std::log10(static_cast<double>(variableCount));
    int rounded = static_cast<int>(std::lround(k));
    if (rounded < 1) rounded = 1;
    return std::min<int>(rounded, static_cast<int>(variableCount));
}

int hardwareCores() {
    const unsigned cores = std::thread::hardware_concurrency();
    return cores == 0 ? 1 : static_cast<int>(cores);
}

int subsetCount(std::size_t variableCount, double blockSize, int cores) {
    const double n =
        blockSize * std::pow(static_cast<double>(variableCount), 0.9);
    return std::max(static_cast<int>(n), std::max(cores, 1));
}

std::vector<std::string> candidateVariables(const std::string& top,
                                            const std::string& simResultsDir) {
    Module module(top);
    module.addTracesfromDir(SIM, simResultsDir);

    std::vector<Signal>* signals = module.getAllSignals();
    std::set<std::string> unique;
    for (const auto& signal : *signals) unique.insert(signal.name);

    return std::vector<std::string>(unique.begin(), unique.end());
}

int writeBlocks(const std::vector<std::string>& variables, int count, int k,
                const std::string& variablesDir, const std::string& prefix,
                std::mt19937_64& rng, int startIndex) {
    if (variables.empty() || count <= 0 || k <= 0) return 0;

    fs::create_directories(variablesDir);
    const int subset = std::min<int>(k, static_cast<int>(variables.size()));

    std::vector<std::string> pool(variables);
    for (int i = 0; i < count; ++i) {
        // Sample without replacement, like random.sample.
        std::shuffle(pool.begin(), pool.end(), rng);

        const auto path =
            variablesDir + "/" + prefix + std::to_string(startIndex + i) + ".txt";
        std::ofstream out(path);
        if (!out) throw std::runtime_error("cannot write " + path);
        for (int j = 0; j < subset; ++j) out << pool[j] << "\n";
    }
    return count;
}

InitialBlockPlan writeInitialBlocks(const std::vector<std::string>& variables,
                                    const std::string& variablesDir,
                                    bool blockified, double blockSize, int cores,
                                    std::mt19937_64& rng) {
    InitialBlockPlan plan;
    const std::size_t V = variables.size();
    if (V == 0) return plan;

    if (!blockified) {
        // preAnalyzer.py's non-blockified branch. Its `Block_size` is the
        // configured one DOUBLED, and the subset size steps by design size;
        // both are reproduced as-is because they set how much work round one
        // does.
        const double doubledBlockSize = blockSize * 2.0;
        int initSubsetSize;
        int initCount;
        if (V >= 400) {
            initSubsetSize = 20;
            initCount = static_cast<int>(static_cast<double>(V) * doubledBlockSize);
        } else if (V > 10) {
            initSubsetSize = 5;
            initCount =
                static_cast<int>(static_cast<double>(V) * 2.0 * doubledBlockSize);
        } else {
            initSubsetSize = static_cast<int>(V) - 3;
            initCount =
                static_cast<int>(static_cast<double>(V) * 2.0 * doubledBlockSize);
        }
        if (initSubsetSize > 0 && initCount > 0) {
            plan.initSubsetSize = initSubsetSize;
            plan.initBlocks = writeBlocks(variables, initCount, initSubsetSize,
                                          variablesDir, "Init_", rng);
        }
    }

    // The `thread_*` subsets are written in every mode. preAnalyzer.py uses
    // literal 1.6/0.5 here rather than the configured k_size/Block_size —
    // those only take effect from the second round on (GenerateNewBlocks).
    plan.k = subsetSize(V, 1.6);
    plan.threadBlocks =
        writeBlocks(variables, subsetCount(V, 0.5, cores), plan.k, variablesDir,
                    "thread_", rng);
    return plan;
}

}  // namespace pipeline
}  // namespace smart
