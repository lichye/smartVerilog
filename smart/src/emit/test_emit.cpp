// Spec tests for the emitter and, above all, for the EBMC command builder.
// A malformed command does not announce itself: EBMC just reports the
// property unproven, and the assertion is silently dropped. That failure mode
// has already happened once here (a missing space glued `-D FORMAL` to the
// next argument), which is why it gets a test of its own (plan gotcha 7).

#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <unistd.h>

#include "AssertionWriter.h"

using namespace smart::emit;

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

bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

void testCommandBuilder() {
    CheckOptions options;
    options.topModule = "c17";
    options.bound = 10;

    const auto defaultCommand = ebmcCommand("design.sv", options);
    check(contains(defaultCommand, "--k-induction"),
          "the checker API defaults to k-induction");
    check(!contains(defaultCommand, "--bound"),
          "the default checker API does not silently use a bound");

    options.unbounded = false;
    const auto bounded = ebmcCommand("design.sv", options);
    check(contains(bounded, "'design.sv'"), "design file is quoted");
    check(contains(bounded, " -D FORMAL "),
          "-D FORMAL keeps a space on BOTH sides");
    check(contains(bounded, "--bound 10"), "bounded mode passes --bound");
    check(contains(bounded, "--top 'c17'"), "top module is passed");
    check(!contains(bounded, "--k-induction"), "bounded mode is not k-induction");

    options.unbounded = true;
    const auto unbounded = ebmcCommand("design.sv", options);
    check(contains(unbounded, "--k-induction"), "unbounded mode uses k-induction");
    check(!contains(unbounded, "--bound"), "and drops --bound");

    options.unbounded = false;
    options.extraFiles = {"pkg.sv", "sub.sv"};
    const auto multi = ebmcCommand("design.sv", options);
    check(contains(multi, "'pkg.sv'") && contains(multi, "'sub.sv'"),
          "extra design files are passed");

    options.topModule.clear();
    check(!contains(ebmcCommand("design.sv", options), "--top"),
          "no --top when there is no top module");

    options.ebmc = "tool's ebmc";
    const auto apostrophe = ebmcCommand("design's copy.sv", options);
    check(contains(apostrophe, "'tool'\"'\"'s ebmc'") &&
              contains(apostrophe, "'design'\"'\"'s copy.sv'"),
          "shell arguments preserve apostrophes");
}

void testInjection() {
    const std::string source =
        "// header\n"
        "module tiny_and(input a, input b, output y);\n"
        "  assign y = a & b;\n"
        "endmodule\n";

    const auto injected = injectAssertions(source, "tiny_and", {"y == (a & b)"});
    check(contains(injected, "assert property (y == (a & b));"),
          "assertion is injected");
    const auto assertPos = injected.find("assert property");
    const auto endPos = injected.find("endmodule");
    check(assertPos < endPos, "and lands before endmodule");
    check(contains(injected, "assign y = a & b;"), "the design is left intact");

    const auto none = injectAssertions(source, "tiny_and", {});
    check(none == source, "no assertions means no change at all");

    bool threw = false;
    try {
        injectAssertions(source, "not_here", {"x"});
    } catch (const std::exception&) {
        threw = true;
    }
    check(threw, "a missing module is an error, not a silent no-op");
}

void testMultipleModules() {
    // The assertions belong to the TOP module, which is not necessarily the
    // first one in the file — s27 and friends declare their submodules first.
    const std::string source =
        "module dff(input clk, output q);\n"
        "endmodule\n"
        "module top(input a, output b);\n"
        "  assign b = a;\n"
        "endmodule\n";

    const auto injected = injectAssertions(source, "top", {"b == a"});
    const auto assertPos = injected.find("assert property");
    const auto topPos = injected.find("module top");
    check(assertPos > topPos,
          "injection targets the named module, not the first one");
}

void testOutputGuaranteeLabels() {
    namespace fs = std::filesystem;
    const auto base = fs::temp_directory_path() /
                      ("smart-emit-test-" +
                       std::to_string(static_cast<long>(::getpid())));
    const auto input = base.string() + ".sv";
    const auto output = base.string() + "-out.sv";
    {
        std::ofstream source(input);
        source << "module top; endmodule\n";
    }

    const auto readOutput = [&] {
        std::ifstream in(output);
        std::ostringstream text;
        text << in.rdbuf();
        return text.str();
    };

    writeAssertionFile(input, output, "top", {"1'b1"}, "{}",
                       VerificationMode::KInduction, 10);
    check(contains(readOutput(), "k-induction-proved invariant"),
          "k-induction output is labelled as an invariant");

    writeAssertionFile(input, output, "top", {"1'b1"}, "{}",
                       VerificationMode::Bounded, 17);
    const auto bounded = readOutput();
    check(contains(bounded, "bounded-verified assertion") &&
              contains(bounded, "not an invariant") &&
              contains(bounded, "bound 17"),
          "bounded output states its weaker guarantee");

    writeAssertionFile(input, output, "top", {"1'b1"}, "{}",
                       VerificationMode::Unchecked, 10);
    const auto unchecked = readOutput();
    check(contains(unchecked, "unverified candidate assertion") &&
              contains(unchecked, "prove before use"),
          "unchecked output never claims formal verification");

    std::error_code error;
    fs::remove(input, error);
    fs::remove(output, error);
}

}  // namespace

int main() {
    testCommandBuilder();
    testInjection();
    testMultipleModules();
    testOutputGuaranteeLabels();

    if (failures != 0) {
        std::cout << failures << " emit test(s) failed\n";
        return 1;
    }
    std::cout << "all emit tests passed\n";
    return 0;
}
