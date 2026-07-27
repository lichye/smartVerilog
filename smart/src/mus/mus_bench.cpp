// Time getMus() on a saved (variables.txt, SygusResult.sl) pair.
//
// Exists to compare the C++ MSA against the pre-refactor Python one on the
// SAME input: the two differ in language and in nothing else that matters
// (same cvc5, same FORALL formulation, same unsat-core options), so a
// head-to-head on one input is the only way to attribute the speedup.
//
//   mus_bench <variables.txt> <SygusResult.sl> [timeout_seconds]
#include "Mus.h"

#include <chrono>
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "usage: mus_bench <variables.txt> <SygusResult.sl> "
                     "[timeout_seconds]\n";
        return 1;
    }
    const int timeout = argc > 3 ? std::atoi(argv[3]) : 300;

    const auto start = std::chrono::steady_clock::now();
    const auto result = smart::mus::getMus(argv[1], argv[2], timeout);
    const double seconds =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - start)
            .count();

    std::cout << "cpp_msa seconds=" << seconds
              << " underspecified=" << result.underspecified.size()
              << " groups=" << result.groups
              << " timed_out=" << (result.timedOut ? "true" : "false") << "\n";
    return 0;
}
