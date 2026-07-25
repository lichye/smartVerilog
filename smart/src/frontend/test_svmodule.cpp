// C++ port of smart/src/python/test_gen_bench.py (the 6 spec cases).
// Usage: test_svmodule <repo_root>
// Exits non-zero on the first failing assertion.

#include <cassert>
#include <fstream>
#include <iostream>
#include <regex>
#include <set>
#include <sstream>

#include "SVModule.h"

using namespace smart::frontend;

static std::string root;

static std::string read(const std::string& rel) {
    std::ifstream in(root + "/" + rel);
    if (!in) throw std::runtime_error("cannot open " + rel);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

#define CHECK(cond, msg)                                              \
    do {                                                              \
        if (!(cond)) {                                                \
            std::cerr << "FAIL: " << msg << " (" << #cond << ")\n";   \
            std::exit(1);                                             \
        }                                                             \
    } while (0)

static size_t countMatches(const std::string& text, const std::string& pat) {
    std::regex re(pat);
    return std::distance(std::sregex_iterator(text.begin(), text.end(), re),
                         std::sregex_iterator());
}

static void test_ansi_no_clock() {
    auto info = parseModule(read("Benchmark/user/tiny_and/tiny_and.sv"), "tiny_and");
    std::set<std::string> names;
    for (const auto& p : info.inputs()) names.insert(p.name);
    CHECK((names == std::set<std::string>{"a", "b"}), "tiny_and inputs");
    CHECK(!guessClock(info), "tiny_and has no clock");
    std::cout << "ok  ansi_no_clock (tiny_and)\n";
}

static void test_non_ansi() {
    auto info = parseModule(read("Benchmark/fmcad2025/s27/s27.sv"), "s27");
    std::set<std::string> names;
    for (const auto& p : info.inputs()) names.insert(p.name);
    CHECK(names.count("CK") && names.count("G0"), "s27 non-ANSI inputs");
    std::cout << "ok  non_ansi (s27)\n";
}

static void test_params_clock_reset() {
    auto info = parseModule(read("Benchmark/HWSpec/axis_fifo/axis_fifo.sv"),
                            "axis_fifo");
    const Port* tdata = nullptr;
    bool hasClk = false, hasRst = false;
    for (const auto& p : info.inputs()) {
        if (p.name == "clk") hasClk = true;
        if (p.name == "rst") hasRst = true;
        if (p.name == "s_axis_tdata") tdata = &p;
    }
    CHECK(hasClk && hasRst, "axis_fifo clk/rst present");
    auto clock = guessClock(info);
    auto reset = guessReset(info, clock);
    CHECK(clock && *clock == "clk", "clock guess");
    CHECK(reset && reset->signal == "rst" && reset->active == 1, "reset guess");
    CHECK(tdata && tdata->width == 8, "s_axis_tdata width from DATA_WIDTH=8");
    std::cout << "ok  params_clock_reset (axis_fifo)\n";
}

static void test_free_regs_and_assumes() {
    std::string text = read("artifact/CaseStudy/Input/nru_a/nru_a.sv");
    auto info = parseModule(text, "nru_a");
    std::set<std::pair<std::string, std::string>> kinds;
    for (const auto& r : info.freeRegs) kinds.insert({r.kind, r.name});
    CHECK(kinds.count({"anyconst", "attacker_hitmap"}), "anyconst reg");
    CHECK(kinds.count({"anyseq", "hitmap1"}), "anyseq reg");
    CHECK(info.hasAssume, "has_assume");

    std::string stripped = stripAssumes(text);
    CHECK(countMatches(stripped, R"(\bassume\s*\()") == 0, "assumes removed");
    CHECK(countMatches(stripped, "endmodule") == countMatches(text, "endmodule"),
          "endmodule preserved");
    CHECK(countMatches(stripped, R"(\bbegin\b)") == countMatches(text, R"(\bbegin\b)"),
          "begin/end structure preserved");
    std::cout << "ok  free_regs_and_assumes (nru_a)\n";
}

static void test_inject_assumes() {
    std::string text = read("Benchmark/user/tiny_and/tiny_and.sv");
    std::string out = injectAssumes(text, "tiny_and", {"!(a && b)"});
    CHECK(out.find("assume(!(a && b));") != std::string::npos, "assume injected");
    CHECK(out.find("assume") < out.find("endmodule"), "before endmodule");
    CHECK(countMatches(stripAssumes(out), R"(\bassume\s*\()") == 0,
          "strip round-trips the injection");
    std::cout << "ok  inject_assumes (tiny_and)\n";
}

static void test_if_assume_stays_legal() {
    std::string src =
        "module m(input a); always @(*) begin if (a) assume(a); "
        "else assume(!a); end endmodule";
    std::string stripped = stripAssumes(src);
    CHECK(std::regex_search(stripped, std::regex(R"(if \(a\) ;\s*else ;)")),
          "null statements in place of assumes: " + stripped);
    std::cout << "ok  if_assume_stays_legal\n";
}

int main(int argc, char** argv) {
    root = argc > 1 ? argv[1] : ".";
    test_ansi_no_clock();
    test_non_ansi();
    test_params_clock_reset();
    test_free_regs_and_assumes();
    test_inject_assumes();
    test_if_assume_stays_legal();
    std::cout << "all SVModule tests passed\n";
    return 0;
}
