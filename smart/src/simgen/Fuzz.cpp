#include "Fuzz.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <random>
#include <set>
#include <limits>
#include <sstream>

namespace smart {
namespace simgen {
namespace {

namespace fs = std::filesystem;

using Sequence = std::vector<long long>;

long long maxValue(int width) {
    if (width >= 63) return std::numeric_limits<long long>::max();
    return (1LL << width) - 1;
}

long long draw(std::mt19937_64& rng, int width) {
    return static_cast<long long>(rng() & static_cast<unsigned long long>(
                                             maxValue(width < 1 ? 1 : width)));
}

// A candidate is flat: the constant draws, then cycles x per-cycle draws. The
// harness reads it in exactly this order, so index arithmetic is the format.
Sequence randomSequence(std::mt19937_64& rng, const FuzzInput& input) {
    Sequence sequence;
    sequence.reserve(input.constWidths.size() +
                     input.cycleWidths.size() * input.cycles);
    for (int width : input.constWidths) sequence.push_back(draw(rng, width));
    for (int cycle = 0; cycle < input.cycles; ++cycle)
        for (int width : input.cycleWidths) sequence.push_back(draw(rng, width));
    return sequence;
}

void write(const Sequence& sequence, const std::string& path) {
    std::ofstream out(path);
    for (const auto value : sequence) out << value << "\n";
}

// The states a run visited, as the harness printed them: a count, then one
// line per state vector.
std::set<std::string> runCandidate(const FuzzInput& input,
                                   const Sequence& sequence, int index) {
    const auto stim = input.scratchDir + "/stim" + std::to_string(index) + ".txt";
    const auto states =
        input.scratchDir + "/states" + std::to_string(index) + ".txt";
    write(sequence, stim);

    std::ostringstream command;
    if (input.timeoutSeconds > 0)
        command << "timeout " << input.timeoutSeconds << " ";
    command << "'" << input.simulator << "' +nodump '+stim=" << stim
            << "' '+states=" << states << "' > /dev/null 2>&1";
    std::system(command.str().c_str());

    std::set<std::string> visited;
    std::ifstream in(states);
    std::string line;
    bool first = true;
    while (std::getline(in, line)) {
        if (first) {  // the count
            first = false;
            continue;
        }
        if (!line.empty()) visited.insert(line);
    }
    fs::remove(states);
    return visited;
}

// --- mutation ---------------------------------------------------------------

// Hold one signal steady across a span of cycles. This is the operator the
// whole thing exists for: independent per-cycle draws cannot produce a run of
// equal values at any useful length, and that run is exactly what walks a
// counter to its bound or keeps an FSM in a load sequence.
void holdSpan(Sequence& sequence, std::mt19937_64& rng, const FuzzInput& input) {
    if (input.cycleWidths.empty() || input.cycles < 2) return;
    const std::size_t base = input.constWidths.size();
    const std::size_t perCycle = input.cycleWidths.size();

    const std::size_t signal = rng() % perCycle;
    const int start = static_cast<int>(rng() % input.cycles);
    const int span = 1 + static_cast<int>(rng() % (input.cycles - start));
    const long long value = draw(rng, input.cycleWidths[signal]);

    for (int cycle = start; cycle < start + span && cycle < input.cycles; ++cycle)
        sequence[base + cycle * perCycle + signal] = value;
}

void randomisePositions(Sequence& sequence, std::mt19937_64& rng,
                        const FuzzInput& input) {
    if (sequence.empty()) return;
    const std::size_t base = input.constWidths.size();
    const std::size_t perCycle = input.cycleWidths.size();
    const int howMany = 1 + static_cast<int>(rng() % 4);
    for (int i = 0; i < howMany; ++i) {
        const std::size_t at = rng() % sequence.size();
        const int width = at < base
                              ? input.constWidths[at]
                              : input.cycleWidths[(at - base) % perCycle];
        sequence[at] = draw(rng, width);
    }
}

// Take a prefix from one parent and the rest from another: a sequence that
// reached somewhere interesting keeps its opening and explores from there.
Sequence splice(const Sequence& a, const Sequence& b, std::mt19937_64& rng) {
    if (a.size() != b.size() || a.empty()) return a;
    const std::size_t cut = rng() % a.size();
    Sequence child(a.begin(), a.begin() + static_cast<long>(cut));
    child.insert(child.end(), b.begin() + static_cast<long>(cut), b.end());
    return child;
}

}  // namespace

FuzzResult fuzzStimulus(const FuzzInput& input) {
    FuzzResult result;
    if (input.cycleWidths.empty() || input.traces <= 0) return result;

    std::error_code error;
    fs::create_directories(input.scratchDir, error);
    std::mt19937_64 rng(input.seed);

    struct Entry {
        Sequence sequence;
        std::set<std::string> states;
    };
    std::vector<Entry> corpus;
    std::set<std::string> everything;

    // Seed the corpus with plain random draws, and record what that alone
    // achieves so the run log can say whether the search was worth it.
    const int seeds = std::max(input.traces, 4);
    for (int i = 0; i < seeds && result.iterations < input.iterations; ++i) {
        Entry entry;
        entry.sequence = randomSequence(rng, input);
        entry.states = runCandidate(input, entry.sequence, ++result.iterations);
        everything.insert(entry.states.begin(), entry.states.end());
        corpus.push_back(std::move(entry));
    }
    {
        // The baseline is the union over the first `traces` random candidates,
        // which is what the random policy actually produces.
        std::set<std::string> baseline;
        for (int i = 0; i < input.traces && i < static_cast<int>(corpus.size()); ++i)
            baseline.insert(corpus[i].states.begin(), corpus[i].states.end());
        result.statesRandom = static_cast<int>(baseline.size());
    }

    while (result.iterations < input.iterations && !corpus.empty()) {
        const auto& parent = corpus[rng() % corpus.size()];
        Sequence child = parent.sequence;

        switch (rng() % 4) {
            case 0:
            case 1:
                holdSpan(child, rng, input);  // weighted: it is the useful one
                break;
            case 2:
                randomisePositions(child, rng, input);
                break;
            default:
                child = splice(child, corpus[rng() % corpus.size()].sequence, rng);
                break;
        }

        auto states = runCandidate(input, child, ++result.iterations);

        // Keep only what adds something. Without this the corpus fills with
        // equivalent sequences and the search stops moving.
        bool novel = false;
        for (const auto& state : states)
            if (everything.insert(state).second) novel = true;
        if (novel) corpus.push_back({std::move(child), std::move(states)});
    }
    result.statesSeen = static_cast<int>(everything.size());

    // Greedy set cover: repeatedly take the sequence adding the most states.
    std::set<std::string> covered;
    std::vector<bool> used(corpus.size(), false);
    for (int picked = 0; picked < input.traces; ++picked) {
        std::size_t best = corpus.size();
        std::size_t bestGain = 0;
        for (std::size_t i = 0; i < corpus.size(); ++i) {
            if (used[i]) continue;
            std::size_t gain = 0;
            for (const auto& state : corpus[i].states)
                if (covered.count(state) == 0) ++gain;
            if (gain > bestGain) {
                bestGain = gain;
                best = i;
            }
        }
        // Nothing adds anything new: fall back to any unused sequence so the
        // caller still gets the trace count it asked for.
        if (best == corpus.size())
            for (std::size_t i = 0; i < corpus.size(); ++i)
                if (!used[i]) {
                    best = i;
                    break;
                }
        if (best == corpus.size()) break;

        used[best] = true;
        covered.insert(corpus[best].states.begin(), corpus[best].states.end());

        const auto path = input.scratchDir + "/selected" +
                          std::to_string(result.stimulusFiles.size()) + ".txt";
        write(corpus[best].sequence, path);
        result.stimulusFiles.push_back(path);
    }
    result.statesSelected = static_cast<int>(covered.size());

    // The per-candidate files are scratch; the selected ones are not.
    for (int i = 1; i <= result.iterations; ++i)
        fs::remove(input.scratchDir + "/stim" + std::to_string(i) + ".txt", error);

    return result;
}

}  // namespace simgen
}  // namespace smart
