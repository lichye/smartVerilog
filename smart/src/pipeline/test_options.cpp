// Spec tests for the option table (plan §1.1). The property under test is
// that config file and command line are equivalent and that the eleven
// shipped legacy configs keep working — including their misspelled keys and
// the one value whose type is a lie.

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Options.h"

using smart::pipeline::Options;

namespace {

int failures = 0;

void check(bool condition, const std::string& what) {
    if (condition) {
        std::cout << "ok  " << what << "\n";
    } else {
        std::cout << "FAIL " << what << "\n";
        ++failures;
    }
}

Options parse(std::vector<std::string> args) {
    std::vector<char*> argv;
    std::string program = "smart";
    argv.push_back(program.data());
    for (auto& arg : args) argv.push_back(arg.data());

    Options options;
    options.parseCommandLine(static_cast<int>(argv.size()), argv.data());
    return options;
}

std::string writeTemp(const std::string& name, const std::string& content) {
    const std::string path = std::string(std::getenv("TMPDIR") ? std::getenv("TMPDIR")
                                                               : "/tmp") +
                             "/smart_test_" + name;
    std::ofstream out(path);
    out << content;
    return path;
}

bool hasWarningContaining(const Options& options, const std::string& needle) {
    for (const auto& warning : options.warnings())
        if (warning.find(needle) != std::string::npos) return true;
    return false;
}

void testDefaults() {
    Options options;
    check(options.getInt("cycles") == 10, "default cycles is 10");
    check(options.getInt("traces") == 3, "default traces is 3");
    check(options.getInt("seed") == 42, "default seed is 42");
    check(options.getInt("bound") == 10, "default bound is 10");
    check(options.getInt("core_timeout") == 100, "default core timeout is 100");
    check(options.getInt("timeout") == 43200, "default global timeout is 43200");
    check(!options.getBool("blockified"), "plain SMART by default");
    check(options.getInt("jobs") >= 1, "jobs defaults to something runnable");
}

void testCommandLine() {
    auto options = parse({"design.sv", "--cycles", "25", "-j", "4", "--seed=7",
                          "--assume", "a>0", "--assume", "b<3", "--keep-work"});
    check(options.designFiles().size() == 1 &&
              options.designFiles()[0] == "design.sv",
          "positional design file");
    check(options.getInt("cycles") == 25, "--cycles N");
    check(options.getInt("jobs") == 4, "-j N");
    check(options.getInt("seed") == 7, "--seed=N");
    check(options.getStringList("assume").size() == 2, "--assume repeats");
    check(options.getBool("keep_work"), "bare boolean flag");

    auto negated = parse({"design.sv", "--no-keep-work"});
    check(!negated.getBool("keep_work"), "--no-<flag> turns a boolean off");

    auto strategy = parse({"design.sv", "--msa"});
    check(strategy.getBool("msa") && strategy.getBool("blockified"),
          "--msa implies --blockified");

    auto unbounded = parse({"design.sv", "--unbounded"});
    check(unbounded.getBool("check_unbounded") && unbounded.getBool("unbound_check"),
          "--unbounded sets both legacy switches");

    auto verbosity = parse({"design.sv", "-v", "-v", "-q"});
    check(verbosity.getInt("verbosity") == 1, "-v/-q accumulate");
}

void testFlatConfig() {
    const auto path = writeTemp("flat.json", R"({
        "cycles": 40,
        "traces": 5,
        "jobs": 8,
        "block_size": 0.25,
        "not_a_real_key": 1
    })");

    Options options;
    options.mergeJsonFile(path);
    check(options.getInt("cycles") == 40, "flat config sets cycles");
    check(options.getInt("traces") == 5, "flat config sets traces");
    check(options.getDouble("block_size") == 0.25, "flat config sets a double");
    check(hasWarningContaining(options, "not_a_real_key"),
          "unknown flat key is warned about, not fatal");
    std::remove(path.c_str());
}

void testLegacyConfig() {
    // The shape of the eleven shipped Config/*.json files, including the two
    // misspelled keys and Depth_Trace's prose value.
    const auto path = writeTemp("legacy.json", R"({
        "Workflow": {
            "SMART_Time_out": 1800,
            "Core_timeout": 55,
            "Blockified": true,
            "Checker": true,
            "Evaluation": true,
            "Log": true
        },
        "PreAnalysis_settings": {
            "Number_Trace": 7,
            "Depth_Trace": "Default is 10 and hard to change"
        },
        "Blockified_settings": {
            "Threadhold": 2,
            "MSA": true,
            "Random": false,
            "k_size": 1.9,
            "Block_size": 0.75,
            "MSA_stable_depth": 4
        },
        "SMART_settings": {
            "Refinement": true,
            "Refinement_depth": 9,
            "Nagative_state_number": 3,
            "unbound_check": false,
            "bounded_depth": 12
        },
        "Parallel_settings": { "max_threads": 16 },
        "Checker_settings": { "Check_unbounded": false, "bounded_depth": 20 },
        "Evaluation_settings": { "bounded_depth": 10 }
    })");

    Options options;
    options.mergeJsonFile(path);

    check(options.getInt("timeout") == 1800, "legacy Workflow.SMART_Time_out");
    check(options.getInt("core_timeout") == 55, "legacy Workflow.Core_timeout");
    check(options.getBool("blockified"), "legacy Workflow.Blockified");
    check(options.getInt("traces") == 7, "legacy Number_Trace");
    check(options.getInt("threshold") == 2, "misspelled Threadhold is understood");
    check(options.getInt("negative_state_number") == 3,
          "misspelled Nagative_state_number is understood");
    check(options.getDouble("k_size") == 1.9, "legacy k_size");
    check(options.getInt("jobs") == 16, "legacy max_threads maps to jobs");
    check(options.getInt("bound") == 20, "Checker_settings.bounded_depth -> bound");
    check(options.getInt("block_bound") == 12,
          "SMART_settings.bounded_depth -> block_bound");
    check(options.getInt("cycles") == 10,
          "Depth_Trace's prose value leaves cycles at its default");
    check(hasWarningContaining(options, "Depth_Trace"),
          "and says so rather than failing");
    check(hasWarningContaining(options, "Evaluation"),
          "evaluation keys are accepted and ignored with a notice");
    std::remove(path.c_str());
}

void testPrecedence() {
    const auto path = writeTemp("prec.json", R"({"cycles": 40, "jobs": 16})");

    auto options = parse({"design.sv", "--config", path, "--jobs", "4"});
    check(options.getInt("jobs") == 4, "CLI beats config file");
    check(options.getInt("cycles") == 40, "config file beats default");

    // Same, with the flag BEFORE --config: order on the command line must not
    // change the outcome.
    auto reordered = parse({"design.sv", "--jobs", "4", "--config", path});
    check(reordered.getInt("jobs") == 4, "flag order does not matter");
    std::remove(path.c_str());
}

void testShippedConfigs() {
    // Every config that ships with the repo must load without a fatal error.
    const std::vector<std::string> shipped = {
        "Config/smart.json",        "Config/block_msa.json",
        "Config/block_msa_mini.json", "Config/block_msa_rand.json",
        "Config/parallel.json",     "Config/Cores-2.json",
        "Config/Cores-4.json",      "Config/Cores-8.json",
        "Config/Cores-16.json",     "Config/Cores-32.json",
        "Config/Cores-64.json"};

    int loaded = 0;
    for (const auto& path : shipped) {
        std::ifstream probe(path);
        if (!probe) continue;  // run from elsewhere; skip rather than fail
        Options options;
        try {
            options.mergeJsonFile(path);
            ++loaded;
        } catch (const std::exception& e) {
            std::cout << "FAIL " << path << ": " << e.what() << "\n";
            ++failures;
        }
    }
    if (loaded > 0)
        check(loaded == static_cast<int>(shipped.size()),
              "all 11 shipped configs load");
    else
        std::cout << "skip shipped configs (not run from the repo root)\n";
}

void testRoundTrip() {
    auto options = parse({"design.sv", "--cycles", "33", "--msa"});
    const auto dumped = options.toJson();
    const auto path = writeTemp("roundtrip.json", dumped);

    Options reloaded;
    reloaded.mergeJsonFile(path);
    check(reloaded.getInt("cycles") == 33, "--dump-config output reloads");
    check(reloaded.getBool("msa") && reloaded.getBool("blockified"),
          "and carries the derived settings");
    check(reloaded.warnings().empty(),
          "the dump is clean: reloading it warns about nothing");
    std::remove(path.c_str());
}

}  // namespace

int main() {
    testDefaults();
    testCommandLine();
    testFlatConfig();
    testLegacyConfig();
    testPrecedence();
    testShippedConfigs();
    testRoundTrip();

    if (failures != 0) {
        std::cout << failures << " option test(s) failed\n";
        return 1;
    }
    std::cout << "all option tests passed\n";
    return 0;
}
