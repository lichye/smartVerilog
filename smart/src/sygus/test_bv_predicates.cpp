#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "SyGuSGenerater.h"
#include "SmtFunctionParser.h"
#include "VCDValue.hpp"

namespace fs = std::filesystem;

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

Signal bits(const std::string& name, int width) {
    return Signal{name, "top", SignalType::BITS, width - 1, 0};
}

Signal boolean(const std::string& name) {
    return Signal{name, "top", SignalType::BOOLEAN, 0, 0};
}

std::string readFile(const fs::path& path) {
    std::ifstream input(path);
    std::ostringstream contents;
    contents << input.rdbuf();
    return contents.str();
}

std::string render(const std::vector<Signal>& inputSignals,
                   const std::string& mode, int latency = 0) {
    const fs::path path =
        fs::temp_directory_path() /
        ("smart-bv-predicates-" + mode + "-" + std::to_string(latency) + ".sl");
    std::error_code error;
    fs::remove(path, error);

    SyGuSGenerater generator;
    auto signals = inputSignals;
    generator.setSignals(&signals);
    generator.setBvPredicateMode(mode);
    if (latency == 0)
        generator.printSysgusPath(path.string());
    else
        generator.printLTLSygusPath(path.string(), latency);

    const std::string result = readFile(path);
    fs::remove(path, error);
    return result;
}

std::string termBody(const std::string& grammar, const std::string& name,
                     int width) {
    const std::string start =
        "(" + name + " (_ BitVec " + std::to_string(width) + ") \n";
    const auto begin = grammar.find(start);
    if (begin == std::string::npos) return "";
    const auto end = grammar.find("\t)\n)\n", begin);
    if (end == std::string::npos) return "";
    return grammar.substr(begin, end + 5 - begin);
}

std::size_t occurrences(const std::string& text, const std::string& needle) {
    std::size_t count = 0;
    for (std::size_t at = 0; (at = text.find(needle, at)) != std::string::npos;
         at += needle.size())
        ++count;
    return count;
}

void testOffIsTheOldGrammar() {
    const std::vector<Signal> signals = {boolean("valid"), bits("count", 8),
                                         bits("limit", 8)};
    const fs::path fixture =
        fs::path(SMART_TEST_SOURCE_DIR) /
        "src/sygus/testdata/bv_predicates_off.sl";
    const std::string golden = readFile(fixture);
    check(!golden.empty(), "the committed off-grammar golden fixture is readable");
    check(golden.find("CmpBv") == std::string::npos &&
              golden.find("bvult") == std::string::npos &&
              golden.find("bvule") == std::string::npos,
          "the committed golden predates all controlled BitVec predicates");

    SyGuSGenerater defaultGenerator;
    auto defaultSignals = signals;
    defaultGenerator.setSignals(&defaultSignals);
    const fs::path path =
        fs::temp_directory_path() / "smart-bv-predicates-default.sl";
    defaultGenerator.printSysgusPath(path.string());
    const std::string defaultGrammar = readFile(path);
    std::error_code error;
    fs::remove(path, error);

    const std::string explicitOff = render(signals, "off");
    check(defaultGrammar == golden,
          "the constructor default matches the committed old grammar byte-for-byte");
    check(explicitOff == golden,
          "explicit off matches the committed old grammar byte-for-byte");
}

void testTwoSignalsOfOneWidth() {
    const std::string grammar =
        render({boolean("valid"), bits("count", 8), bits("limit", 8)},
               "unsigned");
    const std::string body = termBody(grammar, "CmpBv8", 8);
    check(!body.empty() && body.find("\tcount\n") != std::string::npos &&
              body.find("\tlimit\n") != std::string::npos &&
              body.find("valid") == std::string::npos,
          "CmpBv8 lists only the raw 8-bit signals");
    check(occurrences(grammar, "(= CmpBv8 CmpBv8)") == 1 &&
              occurrences(grammar, "(bvult CmpBv8 CmpBv8)") == 1 &&
              occurrences(grammar, "(bvule CmpBv8 CmpBv8)") == 1,
          "unsigned emits equality, bvult, and bvule exactly once per width");
}

void testWidthsStaySeparate() {
    const std::string grammar =
        render({bits("nibble_a", 4), bits("nibble_b", 4), bits("byte_a", 8),
                bits("byte_b", 8)},
               "unsigned");
    const std::string four = termBody(grammar, "CmpBv4", 4);
    const std::string eight = termBody(grammar, "CmpBv8", 8);
    check(four.find("nibble_a") != std::string::npos &&
              four.find("byte_a") == std::string::npos &&
              eight.find("byte_a") != std::string::npos &&
              eight.find("nibble_a") == std::string::npos,
          "mixed widths get separate raw-term nonterminals");
    check(grammar.find("CmpBv4 CmpBv8") == std::string::npos &&
              grammar.find("CmpBv8 CmpBv4") == std::string::npos,
          "no predicate compares different widths");
}

void testBooleanOnlyBlock() {
    const std::string grammar =
        render({boolean("ready"), boolean("valid")}, "unsigned");
    check(grammar.find("CmpBv") == std::string::npos &&
              grammar.find("bvult") == std::string::npos &&
              grammar.find("bvule") == std::string::npos,
          "a Boolean-only block emits no BitVec predicate grammar");
}

void testSingleSignalWidthIsSkipped() {
    const std::string grammar =
        render({bits("one_nibble", 4), bits("byte_a", 8), bits("byte_b", 8)},
               "unsigned");
    check(grammar.find("CmpBv4") == std::string::npos &&
              grammar.find("CmpBv8") != std::string::npos,
          "a width needs two raw signals before comparisons are generated");
}

void testLtlTermsDoNotMixTime() {
    const std::string grammar =
        render({bits("count", 8), bits("limit", 8)}, "unsigned", 2);
    const std::string current = termBody(grammar, "CmpBv8", 8);
    const std::string next = termBody(grammar, "CmpBvX8", 8);
    check(current.find("\tcount\n") != std::string::npos &&
              current.find("\tlimit\n") != std::string::npos &&
              current.find("_2") == std::string::npos,
          "LTL Atom uses only current-state BitVec names");
    check(next.find("\tcount_2\n") != std::string::npos &&
              next.find("\tlimit_2\n") != std::string::npos &&
              next.find("\tcount\n") == std::string::npos &&
              next.find("\tlimit\n") == std::string::npos,
          "LTL AtomX uses only latency-suffixed BitVec names");
    check(grammar.find("(bvult CmpBv8 CmpBv8)") != std::string::npos &&
              grammar.find("(bvult CmpBvX8 CmpBvX8)") != std::string::npos,
          "LTL current and next atoms reference their own nonterminals");
}

Value* vectorValue(unsigned value, int width) {
    auto* vector = new VCDBitVector();
    for (int bit = width - 1; bit >= 0; --bit)
        vector->push_back((value & (1U << bit)) == 0 ? VCD_0 : VCD_1);
    VCDValue encoded(vector);
    return new Value(&encoded, width);
}

void testCvc5SolvesGeneratedGrammar() {
    const fs::path path =
        fs::temp_directory_path() / "smart-bv-predicates-solve.sl";
    std::vector<std::unique_ptr<Value>> values;
    auto own = [&](unsigned value) {
        values.emplace_back(vectorValue(value, 8));
        return values.back().get();
    };

    std::string answer;
    {
        SyGuSGenerater generator;
        std::vector<Signal> signals = {bits("count", 8), bits("limit", 8)};
        generator.setSignals(&signals);
        generator.setBvPredicateMode("unsigned");
        generator.setKeepTempFiles(true);
        generator.addConstraints({{own(0)}, {own(1)}}, true);
        generator.addConstraints({{own(2)}, {own(1)}}, false);
        generator.printSysgusPath(path.string());
        answer = generator.runCVC5Sygus(path.string());
    }
    check(answer.find("define-fun inv") != std::string::npos,
          "cvc5 parses and solves a generated phase-1 grammar in process");
    std::error_code error;
    fs::remove(path, error);
}

void testLtlCvc5ResultParsesToSystemVerilog() {
    const fs::path path =
        fs::temp_directory_path() / "smart-bv-predicates-ltl-solve.sl";
    std::vector<std::unique_ptr<Value>> values;
    auto own = [&](unsigned value) {
        values.emplace_back(vectorValue(value, 8));
        return values.back().get();
    };

    std::string answer;
    {
        SyGuSGenerater generator;
        std::vector<Signal> signals = {bits("count", 8), bits("limit", 8)};
        generator.setSignals(&signals);
        generator.setBvPredicateMode("unsigned");
        generator.setKeepTempFiles(true);
        generator.addConstraints(
            {{own(0), own(0), own(0)}, {own(1), own(1), own(1)}}, true);
        generator.addConstraints({{own(2)}, {own(1)}}, false);
        generator.printLTLSygusPath(path.string(), 2);
        answer = generator.runCVC5Sygus(path.string());
    }

    bool parsed = false;
    try {
        SmtFunctionParser parser;
        std::unique_ptr<SygusExpr> expression(
            parser.parseSmtFunction(answer, true));
        auto* function = dynamic_cast<SygusFunction*>(expression.get());
        if (function != nullptr) {
            const std::string systemVerilog = function->getBodyVerilogExpr();
            parsed = systemVerilog.find("##2") != std::string::npos &&
                     systemVerilog.find("$unsigned(") != std::string::npos;
        }
    } catch (const std::exception&) {
        parsed = false;
    }
    check(parsed,
          "an LTL cvc5 define-fun parses to unsigned, latency-aware SV");
    std::error_code error;
    fs::remove(path, error);
}

void testGeneratorRejectsUnknownMode() {
    bool failed = false;
    try {
        SyGuSGenerater generator;
        generator.setBvPredicateMode("signed");
    } catch (const std::invalid_argument& e) {
        failed = std::string(e.what()).find("off, unsigned") != std::string::npos;
    }
    check(failed, "the generator rejects modes outside off and unsigned");
}

}  // namespace

int main() {
    testOffIsTheOldGrammar();
    testTwoSignalsOfOneWidth();
    testWidthsStaySeparate();
    testBooleanOnlyBlock();
    testSingleSignalWidthIsSkipped();
    testLtlTermsDoNotMixTime();
    testCvc5SolvesGeneratedGrammar();
    testLtlCvc5ResultParsesToSystemVerilog();
    testGeneratorRejectsUnknownMode();

    if (failures != 0) {
        std::cout << failures << " BitVec predicate test(s) failed\n";
        return 1;
    }
    std::cout << "all BitVec predicate tests passed\n";
    return 0;
}
