#include "Options.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <thread>

using json = nlohmann::json;

namespace smart {
namespace pipeline {
namespace {

long long defaultJobs() {
    const unsigned hardware = std::thread::hardware_concurrency();
    return hardware == 0 ? 1 : static_cast<long long>(hardware);
}

// The table. Adding a row here is all it takes to make an option reachable
// from BOTH the command line and a config file.
//
// legacyPath is the dotted location in the old nested schema. Two of those
// names are misspelled upstream (`Threadhold`, `Nagative_state_number`); they
// are spelled here exactly as they appear on disk, because the shipped
// Config/*.json files and the artifact scripts use them.
std::vector<OptionSpec> buildTable() {
    using T = OptionType;
    std::vector<OptionSpec> table = {
        // --- input / output --------------------------------------------
        {"top", "top", 0, T::String, std::string(""), "", "NAME",
         "top module name (default: file stem)"},
        {"output", "output", 'o', T::String, std::string(""), "", "FILE",
         "output file (default: <top>_assertion.sv next to the input)"},
        {"workdir", "workdir", 0, T::String, std::string(""), "", "DIR",
         "working directory (default: ./smart-work-<top>)"},
        {"keep_work", "keep-work", 0, T::Bool, false, "", "",
         "keep the working directory on success"},
        {"assume", "assume", 0, T::StringList, std::vector<std::string>{}, "",
         "EXPR", "extra assume, repeatable; formal side only"},

        // --- design / stimulus -----------------------------------------
        {"clock", "clock", 0, T::String, std::string(""), "", "NAME",
         "clock signal (default: guessed)"},
        {"reset", "reset", 0, T::String, std::string(""), "", "NAME[:0|1]",
         "reset signal and active level (default: guessed)"},
        {"cycles", "cycles", 0, T::Int, 10LL, "PreAnalysis_settings.Depth_Trace",
         "N", "trace depth"},
        {"traces", "traces", 0, T::Int, 3LL, "PreAnalysis_settings.Number_Trace",
         "N", "number of simulation traces"},
        {"seed", "seed", 0, T::Int, 42LL, "", "N",
         "base RNG seed; trace i uses seed+i"},
        // Verilator is the default because the published results were produced
        // with it, and because being cycle-based and 2-state keeps x/z out of
        // the traces and stops a randomly-clocked gate netlist spinning in a
        // zero-delay loop (s382, s444). A commercial simulator would slot in
        // here as another value.
        {"simulator", "simulator", 0, T::String, std::string("verilator"), "",
         "NAME", "trace generator: verilator | iverilog"},
        {"simulation_timeout", "simulation-timeout", 0, T::Int, 300LL, "", "SECS",
         "per-trace simulation limit; 0 disables"},
        // How the stimulus is chosen. `random` draws every input independently
        // every cycle. `fuzz` searches for sequences that visit more states,
        // keeping the SAME trace count and depth — depth is not the lever,
        // because more sampled cycles means more positive constraints per
        // SyGuS call, which has a fixed budget.
        {"trace_policy", "trace-policy", 0, T::String, std::string("random"), "",
         "NAME", "stimulus generation: random | fuzz"},
        {"fuzz_iterations", "fuzz-iterations", 0, T::Int, 500LL, "", "N",
         "simulations the fuzz policy may spend searching"},

        // --- verification ----------------------------------------------
        {"bound", "bound", 0, T::Int, 10LL, "Checker_settings.bounded_depth", "N",
         "EBMC bound for the final check"},
        {"block_bound", "", 0, T::Int, 10LL, "SMART_settings.bounded_depth", "N",
         "EBMC bound used inside synthesis blocks"},
        // Deciding whether a random state is REACHABLE is a different
        // question from checking a candidate, so it gets its own depth. The
        // default matches block_bound because deepening it alone was measured
        // to change nothing: on plena_data_flow_control_flat, reachability 40
        // with block_bound 10 found 0 assertions, exactly as reachability 10
        // did — while block_bound 40 found 2. Whatever the depth buys there,
        // it is bought in the candidate check, not in reachability.
        // -1 selects k-induction: sound, but often inconclusive, and an
        // inconclusive answer loses the negative example altogether.
        {"reachability_bound", "reachability-bound", 0, T::Int, 10LL, "", "N",
         "EBMC bound for the reachability check; -1 uses k-induction"},
        {"unbounded", "unbounded", 0, T::Bool, false, "", "",
         "use --k-induction instead of a bound"},
        {"check_unbounded", "", 0, T::Bool, false,
         "Checker_settings.Check_unbounded", "", "unbounded final check"},
        {"unbound_check", "", 0, T::Bool, false, "SMART_settings.unbound_check",
         "", "unbounded check inside blocks"},
        {"check_timeout", "check-timeout", 0, T::Int, 180LL, "", "SECS",
         "per-assertion EBMC timeout"},
        {"checker", "", 0, T::Bool, true, "Workflow.Checker", "",
         "run the final checker stage"},

        // --- synthesis --------------------------------------------------
        {"refinement", "", 0, T::Bool, true, "SMART_settings.Refinement", "",
         "CEGIS refinement inside a block"},
        {"refinement_depth", "refine", 0, T::Int, 5LL,
         "SMART_settings.Refinement_depth", "N", "CEGIS refinement depth"},
        {"negative_state_number", "", 0, T::Int, 1LL,
         "SMART_settings.Nagative_state_number", "N",
         "random negative states added per block"},
        {"sygus_timeout_ms", "sygus-timeout", 0, T::Int, 5000LL, "", "MS",
         "per-call SyGuS time limit"},
        {"sygus_subprocess", "sygus-subprocess", 0, T::Bool, false, "", "",
         "solve SyGuS by running the cvc5 binary instead of libcvc5"},

        // --- scheduling -------------------------------------------------
        {"jobs", "jobs", 'j', T::Int, defaultJobs(), "Parallel_settings.max_threads",
         "N", "parallel workers"},
        {"timeout", "timeout", 0, T::Int, 43200LL, "Workflow.SMART_Time_out", "SECS",
         "global wall-clock budget"},
        {"core_timeout", "core-timeout", 0, T::Int, 100LL, "Workflow.Core_timeout",
         "SECS", "per-block wall-clock budget"},

        // --- block strategy ---------------------------------------------
        //
        // Iterate with the MSA strategy by default. Measured over 22 designs
        // against the fixed MutationBenchmark mutants (2026-07-26):
        //
        //   config           MD mean  MD median  assertions  total time
        //   smart (one-shot)   67.3%      66.0%         202        312s
        //   blockified+MSA     81.9%      82.4%         326       4569s
        //
        // 14.6 points of mutation detection for 15x the wall clock. Detection
        // is the metric the tool exists to move, so it wins the default;
        // `--no-blockified` is there when the budget matters more.
        //
        // The random strategy is NOT the default despite a nominally equal MD
        // mean (82.14% vs 82.06% on the 20 designs where it finished): its
        // median is 1.15 points LOWER, it emits 6.6x the assertions (median
        // 1968 vs 298), takes 2.7x the time, and failed outright on the two
        // hardest designs (c880, c1355) where MSA scored 92.7% and 67.4%.
        {"blockified", "blockified", 0, T::Bool, true, "Workflow.Blockified", "",
         "iterate: mine, then re-block from what was found"},
        {"msa", "msa", 0, T::Bool, true, "Blockified_settings.MSA", "",
         "MSA block strategy (implies --blockified)"},
        {"random", "random", 0, T::Bool, false, "Blockified_settings.Random", "",
         "random block strategy (implies --blockified)"},
        {"k_size", "k-size", 0, T::Double, 1.6, "Blockified_settings.k_size", "F",
         "k = round(2.7 + k_size*log10(V))"},
        {"block_size", "block-size", 0, T::Double, 0.5,
         "Blockified_settings.Block_size", "F",
         "n = max(block_size*V^0.9, jobs)"},
        {"threshold", "", 0, T::Int, 0LL, "Blockified_settings.Threadhold", "N",
         "stop when a round finds <= this many assertions"},
        {"max_iterations", "", 0, T::Int, 100LL,
         "Blockified_settings.Max_iterations", "N", "max block rounds"},
        {"msa_stable_end", "", 0, T::Bool, true,
         "Blockified_settings.MSA_stable_end", "", "stop when the MSA stops shrinking"},
        {"msa_stable_depth", "", 0, T::Int, 3LL,
         "Blockified_settings.MSA_stable_depth", "N",
         "rounds without MSA shrinkage before stopping"},
        {"save_temp_assertions", "", 0, T::Bool, false,
         "Blockified_settings.Save_temp_assertions", "",
         "dump the assertion set after every round"},

        // --- minimiser ---------------------------------------------------
        //
        // On by default at the end, off by default between rounds.
        //
        // Off is a simplicity call, not a performance one. Over 22 designs
        // turning it off moved the mutation rate by +0.28pp on average and
        // +0.00pp at the median (7 better, 5 worse, 10 unchanged), and the
        // total time by 4610s -> 4655s. Both are inside the noise.
        //
        // What is measurable is that it does work for nothing: on c880 it cost
        // 40.7s, 4.8% of the run, to remove 0.3-0.5% of the assertions
        // (206->206, 621->620), because by then the set is already
        // irredundant. It only bites on small designs, where it does remove
        // 20-27% (s298) but the MSA it feeds costs 0.17s either way.
        //
        // An earlier version of this comment claimed +1.04pp from a single
        // c880 A/B. That did not survive the 22-design run.
        //
        // The end-of-run pass is a different trade: one call, and what it
        // shrinks is the artefact a person reads (s27: 49 -> 28).
        //
        // Config/*.json set both keys explicitly, so the historical presets
        // keep their own behaviour regardless of what these defaults say.
        {"minimizer", "", 0, T::Bool, true, "Workflow.Minimizer", "",
         "run the end-of-run assertion minimiser"},
        {"end_minimizer", "", 0, T::Bool, true, "Minimizer_settings.End_minimizer",
         "", "minimise the final assertion set"},
        {"block_minimizer", "", 0, T::Bool, false,
         "Minimizer_settings.Block_minimizer", "", "minimise between rounds"},
        {"end_minimizer_timeout", "", 0, T::Int, 300LL,
         "Minimizer_settings.End_Minimizer_timeout", "SECS", ""},
        {"block_minimizer_timeout", "", 0, T::Int, 300LL,
         "Minimizer_settings.Block_Minimizer_timeout", "SECS", ""},

        // --- LTL ----------------------------------------------------------
        {"ltl", "", 0, T::Bool, false, "Workflow.LTL", "", "LTL synthesis mode"},
        {"ltl_depth", "", 0, T::Int, 1LL, "LTL_settings.LTL_depth", "N",
         "LTL latency depth"},

        // --- reporting ------------------------------------------------------
        {"log", "", 0, T::Bool, true, "Workflow.Log", "", "write per-run logs"},
        {"verbosity", "", 0, T::Int, 0LL, "", "N",
         "-v raises, -q lowers"},
    };
    return table;
}

OptionValue parseScalar(const OptionSpec& spec, const std::string& text) {
    switch (spec.type) {
        case OptionType::Bool: {
            std::string lowered;
            std::transform(text.begin(), text.end(), std::back_inserter(lowered),
                           [](unsigned char c) { return std::tolower(c); });
            if (lowered == "1" || lowered == "true" || lowered == "yes" ||
                lowered == "on" || lowered.empty())
                return true;
            if (lowered == "0" || lowered == "false" || lowered == "no" ||
                lowered == "off")
                return false;
            throw std::runtime_error("not a boolean: " + text);
        }
        case OptionType::Int: {
            size_t consumed = 0;
            const long long value = std::stoll(text, &consumed);
            if (consumed != text.size())
                throw std::runtime_error("not an integer: " + text);
            return value;
        }
        case OptionType::Double: {
            size_t consumed = 0;
            const double value = std::stod(text, &consumed);
            if (consumed != text.size())
                throw std::runtime_error("not a number: " + text);
            return value;
        }
        case OptionType::String:
            return text;
        case OptionType::StringList:
            return std::vector<std::string>{text};
    }
    throw std::runtime_error("unreachable");
}

// Legacy values are not always the type their key implies: `Depth_Trace` ships
// as the string "Default is 10 and hard to change". Coerce what we can and
// report the rest rather than dying on a config that used to work.
std::optional<OptionValue> coerceJson(const OptionSpec& spec, const json& value,
                                      std::string& why) {
    try {
        switch (spec.type) {
            case OptionType::Bool:
                if (value.is_boolean()) return value.get<bool>();
                if (value.is_number_integer()) return value.get<long long>() != 0;
                break;
            case OptionType::Int:
                if (value.is_number_integer()) return value.get<long long>();
                if (value.is_number_float())
                    return static_cast<long long>(value.get<double>());
                if (value.is_string()) return parseScalar(spec, value.get<std::string>());
                break;
            case OptionType::Double:
                if (value.is_number()) return value.get<double>();
                if (value.is_string()) return parseScalar(spec, value.get<std::string>());
                break;
            case OptionType::String:
                if (value.is_string()) return value.get<std::string>();
                break;
            case OptionType::StringList:
                if (value.is_array()) {
                    std::vector<std::string> items;
                    for (const auto& item : value)
                        items.push_back(item.is_string() ? item.get<std::string>()
                                                         : item.dump());
                    return items;
                }
                if (value.is_string())
                    return std::vector<std::string>{value.get<std::string>()};
                break;
        }
    } catch (const std::exception& e) {
        why = e.what();
        return std::nullopt;
    }
    why = "value " + value.dump() + " does not fit this option's type";
    return std::nullopt;
}

json valueToJson(const OptionValue& value) {
    return std::visit([](const auto& held) -> json { return held; }, value);
}

}  // namespace

const std::vector<OptionSpec>& optionTable() {
    static const std::vector<OptionSpec> table = buildTable();
    return table;
}

const OptionSpec* findByKey(const std::string& key) {
    for (const auto& spec : optionTable())
        if (spec.key == key) return &spec;
    return nullptr;
}

Options::Options() {
    for (const auto& spec : optionTable()) {
        values_[spec.key] = spec.defaultValue;
        explicitlySet_[spec.key] = false;
    }
}

bool Options::getBool(const std::string& key) const {
    return std::get<bool>(values_.at(key));
}
long long Options::getInt(const std::string& key) const {
    return std::get<long long>(values_.at(key));
}
double Options::getDouble(const std::string& key) const {
    const auto& value = values_.at(key);
    if (std::holds_alternative<long long>(value))
        return static_cast<double>(std::get<long long>(value));
    return std::get<double>(value);
}
const std::string& Options::getString(const std::string& key) const {
    return std::get<std::string>(values_.at(key));
}
std::vector<std::string> Options::getStringList(const std::string& key) const {
    return std::get<std::vector<std::string>>(values_.at(key));
}

void Options::set(const std::string& key, OptionValue value) {
    values_[key] = std::move(value);
    explicitlySet_[key] = true;
}

bool Options::wasSetExplicitly(const std::string& key) const {
    auto it = explicitlySet_.find(key);
    return it != explicitlySet_.end() && it->second;
}

void Options::assign(const std::string& key, const std::string& text,
                     const std::string& origin) {
    const OptionSpec* spec = findByKey(key);
    if (spec == nullptr) throw std::runtime_error("unknown option " + key);

    try {
        if (spec->type == OptionType::StringList) {
            auto items = getStringList(key);
            items.push_back(text);
            set(key, items);
        } else {
            set(key, parseScalar(*spec, text));
        }
    } catch (const std::exception& e) {
        throw std::runtime_error(origin + ": " + e.what());
    }
}

void Options::applyFlat(const void* jsonPtr) {
    const auto& root = *static_cast<const json*>(jsonPtr);
    for (auto it = root.begin(); it != root.end(); ++it) {
        const OptionSpec* spec = findByKey(it.key());
        if (spec == nullptr) {
            warnings_.push_back("config: ignoring unknown key '" + it.key() + "'");
            continue;
        }
        std::string why;
        auto coerced = coerceJson(*spec, it.value(), why);
        if (!coerced) {
            warnings_.push_back("config: ignoring '" + it.key() + "': " + why);
            continue;
        }
        set(spec->key, *coerced);
    }
}

void Options::applyLegacy(const void* jsonPtr) {
    const auto& root = *static_cast<const json*>(jsonPtr);

    // Sections that only ever configured the retired Python layer.
    static const char* retiredSections[] = {"Evaluation_settings"};

    std::map<std::string, const OptionSpec*> byLegacyPath;
    for (const auto& spec : optionTable())
        if (!spec.legacyPath.empty()) byLegacyPath[spec.legacyPath] = &spec;

    for (auto section = root.begin(); section != root.end(); ++section) {
        if (!section.value().is_object()) {
            warnings_.push_back("legacy config: ignoring non-section '" +
                                section.key() + "'");
            continue;
        }
        const bool retired =
            std::find_if(std::begin(retiredSections), std::end(retiredSections),
                         [&](const char* name) { return section.key() == name; }) !=
            std::end(retiredSections);

        for (auto entry = section.value().begin(); entry != section.value().end();
             ++entry) {
            const std::string path = section.key() + "." + entry.key();

            if (path == "Workflow.Evaluation" || retired) {
                warnings_.push_back(
                    "legacy config: '" + path +
                    "' is accepted and ignored — mutation evaluation is external "
                    "Python now");
                continue;
            }

            auto found = byLegacyPath.find(path);
            if (found == byLegacyPath.end()) {
                warnings_.push_back("legacy config: ignoring unknown key '" + path +
                                    "'");
                continue;
            }
            std::string why;
            auto coerced = coerceJson(*found->second, entry.value(), why);
            if (!coerced) {
                warnings_.push_back("legacy config: ignoring '" + path + "': " + why);
                continue;
            }
            set(found->second->key, *coerced);
        }
    }
}

void Options::mergeJsonFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open config file " + path);

    json root;
    try {
        in >> root;
    } catch (const std::exception& e) {
        throw std::runtime_error("config file " + path + " is not valid JSON: " +
                                 e.what());
    }
    if (!root.is_object())
        throw std::runtime_error("config file " + path + " must contain an object");

    // The legacy schema is nested one level into named sections; the new one is
    // flat. Detect by looking for any known legacy section.
    static const char* legacySections[] = {
        "Workflow",         "PreAnalysis_settings", "Blockified_settings",
        "SMART_settings",   "Parallel_settings",    "Minimizer_settings",
        "LTL_settings",     "Checker_settings",     "Evaluation_settings"};
    const bool legacy =
        std::any_of(std::begin(legacySections), std::end(legacySections),
                    [&](const char* name) { return root.contains(name); });

    if (legacy) {
        warnings_.push_back("config: '" + path +
                            "' uses the legacy nested schema; it still works, but "
                            "--dump-config prints the new flat one");
        applyLegacy(&root);
    } else {
        applyFlat(&root);
    }
}

void Options::parseCommandLine(int argc, char** argv) {
    std::map<std::string, const OptionSpec*> byFlag;
    std::map<char, const OptionSpec*> byShortFlag;
    for (const auto& spec : optionTable()) {
        if (!spec.flag.empty()) byFlag[spec.flag] = &spec;
        if (spec.shortFlag != 0) byShortFlag[spec.shortFlag] = &spec;
    }

    // The config file is merged before any flag is applied, whatever its
    // position on the command line, so that CLI > file holds (plan §1.1).
    std::vector<std::string> pendingConfigs;
    std::vector<std::pair<const OptionSpec*, std::string>> pendingFlags;
    std::vector<std::pair<std::string, bool>> pendingBools;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        auto requireValue = [&](const std::string& name) -> std::string {
            if (i + 1 >= argc)
                throw std::runtime_error("missing value for " + name);
            return argv[++i];
        };

        if (arg == "--help" || arg == "-h") { help_ = true; continue; }
        if (arg == "--check-env") { checkEnv_ = true; continue; }
        if (arg == "--version") { version_ = true; continue; }
        if (arg == "--dump-config") { dumpConfig_ = true; continue; }
        if (arg == "--dump-frontend") { dumpFrontend_ = true; continue; }
        if (arg == "--config") { pendingConfigs.push_back(requireValue(arg)); continue; }
        if (arg == "-v") { pendingBools.emplace_back("__verbose", true); continue; }
        if (arg == "-q") { pendingBools.emplace_back("__quiet", true); continue; }

        if (arg.rfind("--", 0) == 0) {
            std::string name = arg.substr(2);
            std::string inlineValue;
            bool hasInlineValue = false;
            const auto equals = name.find('=');
            if (equals != std::string::npos) {
                inlineValue = name.substr(equals + 1);
                name = name.substr(0, equals);
                hasInlineValue = true;
            }

            // --no-<flag> turns a boolean off.
            bool negated = false;
            if (byFlag.count(name) == 0 && name.rfind("no-", 0) == 0 &&
                byFlag.count(name.substr(3)) != 0) {
                name = name.substr(3);
                negated = true;
            }

            auto found = byFlag.find(name);
            if (found == byFlag.end())
                throw std::runtime_error("unknown option --" + name);

            const OptionSpec* spec = found->second;
            if (spec->type == OptionType::Bool) {
                const std::string text =
                    hasInlineValue ? inlineValue : (negated ? "false" : "true");
                pendingFlags.emplace_back(spec, text);
            } else {
                pendingFlags.emplace_back(
                    spec, hasInlineValue ? inlineValue : requireValue(arg));
            }
            continue;
        }

        if (arg.size() >= 2 && arg[0] == '-') {
            const char shortFlag = arg[1];
            auto found = byShortFlag.find(shortFlag);
            if (found == byShortFlag.end())
                throw std::runtime_error("unknown option " + arg);
            const OptionSpec* spec = found->second;
            const std::string value =
                arg.size() > 2 ? arg.substr(2) : requireValue(arg);
            pendingFlags.emplace_back(spec, value);
            continue;
        }

        designFiles_.push_back(arg);
    }

    for (const auto& path : pendingConfigs) mergeJsonFile(path);

    for (const auto& [spec, text] : pendingFlags)
        assign(spec->key, text, "--" + spec->flag);

    long long verbosity = getInt("verbosity");
    for (const auto& [which, unused] : pendingBools) {
        (void)unused;
        verbosity += (which == "__verbose") ? 1 : -1;
    }
    if (!pendingBools.empty()) set("verbosity", verbosity);

    // --msa / --random on the COMMAND LINE pick a block strategy, and asking
    // for one is asking to iterate. This must not apply to the config keys:
    // the shipped configs set Blockified_settings.MSA while leaving
    // Workflow.Blockified false, and there the strategy only says which
    // blocks to draw IF the workflow iterates at all.
    const auto cameFromCommandLine = [&](const std::string& key) {
        return std::any_of(pendingFlags.begin(), pendingFlags.end(),
                           [&](const auto& entry) { return entry.first->key == key; });
    };
    if ((getBool("msa") && cameFromCommandLine("msa")) ||
        (getBool("random") && cameFromCommandLine("random")))
        set("blockified", true);

    // --unbounded is the user-facing spelling of the two legacy switches.
    if (getBool("unbounded")) {
        set("check_unbounded", true);
        set("unbound_check", true);
    }
}

std::string Options::toJson() const {
    json root = json::object();
    for (const auto& spec : optionTable())
        root[spec.key] = valueToJson(values_.at(spec.key));
    return root.dump(2);
}

std::string Options::usage() {
    std::ostringstream os;
    os << "Usage: smart <design.sv> [more.sv ...] [options]\n\n"
       << "Mines SystemVerilog assertions from random simulation traces and\n"
       << "emits the ones EBMC proves, as <top>_assertion.sv.\n\n"
       << "Actions:\n"
       << "  -h, --help              this message\n"
       << "      --version           print the version and exit\n"
       << "      --check-env         report tool versions and exit\n"
       << "      --dump-config       print the resolved configuration and exit\n"
       << "      --dump-frontend     print the parsed module as JSON and exit\n"
       << "      --config FILE       JSON config; new flat or legacy schema\n"
       << "  -v / -q                 more / less output\n\n"
       << "Any boolean flag below can be turned off with --no-<flag>, e.g.\n"
       << "--no-blockified for the one-shot mode (much faster, lower detection).\n\n"
       << "Options (every one is also a config key of the same name):\n";

    for (const auto& spec : optionTable()) {
        if (spec.flag.empty()) continue;
        std::string left = "  ";
        if (spec.shortFlag != 0) {
            left += std::string("-") + spec.shortFlag + ", ";
        } else {
            left += "    ";
        }
        left += "--" + spec.flag;
        if (!spec.metavar.empty()) left += " " + spec.metavar;
        os << left;
        if (left.size() < 26)
            os << std::string(26 - left.size(), ' ');
        else
            os << "\n" << std::string(26, ' ');
        os << spec.help << "\n";
    }

    os << "\nConfig-only keys (no flag): ";
    bool first = true;
    for (const auto& spec : optionTable()) {
        if (!spec.flag.empty()) continue;
        if (!first) os << ", ";
        os << spec.key;
        first = false;
    }
    os << "\n\nExit codes: 0 success, 1 user error, 2 environment error,\n"
       << "3 pipeline failure.\n";
    return os.str();
}

}  // namespace pipeline
}  // namespace smart
