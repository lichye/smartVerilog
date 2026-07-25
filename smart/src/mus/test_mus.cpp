// Spec tests for the MUS/MSA port. The properties that matter: which
// variables an assertion set leaves free, and that the minimiser drops only
// assertions the rest already imply.

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include "Mus.h"

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

std::string tempDir() {
    const char* dir = std::getenv("TMPDIR");
    return dir != nullptr ? dir : "/tmp";
}

std::string write(const std::string& name, const std::string& content) {
    const std::string path = tempDir() + "/smart_mus_" + name;
    std::ofstream out(path);
    out << content;
    return path;
}

void testUnderspecified() {
    // b is pinned by the assertion; a and c are not mentioned at all, so they
    // stay underspecified and are what the next round should draw from.
    const auto variables = write("vars.txt", "a\nb\nc\n");
    const auto sl = write("res.sl",
        "(define-fun inv ((a Bool) (b Bool)) Bool b)\n");

    const auto result = smart::mus::getMus(variables, sl, 30);
    check(result.underspecified.count("a") == 1, "an unconstrained variable is underspecified");
    check(result.underspecified.count("c") == 1, "so is one the assertions never mention");
    check(result.underspecified.count("b") == 0, "a pinned variable is not");
    std::remove(variables.c_str());
    std::remove(sl.c_str());
}

void testBitVectorBodies() {
    // Parsing has to cope with BitVec sorts and their operators, not just Bool.
    const auto variables = write("vars_bv.txt", "w\nx\n");
    const auto sl = write("res_bv.sl",
        "(define-fun inv ((w (_ BitVec 8)) (x Bool)) Bool "
        "(and x (bvult w (_ bv7 8))))\n");

    const auto result = smart::mus::getMus(variables, sl, 30);
    check(result.groups >= 1, "a bit-vector body parses and yields a group");
    std::remove(variables.c_str());
    std::remove(sl.c_str());
}

void testEmptyInputs() {
    const auto variables = write("vars_empty.txt", "a\nb\n");
    const auto sl = write("res_empty.sl", "");
    const auto result = smart::mus::getMus(variables, sl, 30);
    check(result.underspecified.size() == 2,
          "with no assertions every variable is underspecified");
    std::remove(variables.c_str());
    std::remove(sl.c_str());
}

void testMinimiser() {
    // The second line is implied by the first, so it should go; a third,
    // independent one must stay.
    const auto in = write("min_in.sl",
        "(define-fun inv ((a Bool) (b Bool)) Bool (and a b))\n"
        "(define-fun inv ((a Bool) (b Bool)) Bool a)\n"
        "(define-fun inv ((c Bool)) Bool c)\n");
    const auto out = write("min_out.sl", "");

    const auto result = smart::mus::minimiseAssertions(in, out, 30);
    check(result.before == 3, "all three definitions were read");
    check(result.after < result.before, "an implied assertion is dropped");
    check(result.after >= 2, "and the independent one is kept");

    std::ifstream check_out(out);
    std::string written((std::istreambuf_iterator<char>(check_out)),
                        std::istreambuf_iterator<char>());
    check(written.find("(and a b)") != std::string::npos,
          "the strongest assertion survives");
    std::remove(in.c_str());
    std::remove(out.c_str());
}

}  // namespace

int main() {
    testUnderspecified();
    testBitVectorBodies();
    testEmptyInputs();
    testMinimiser();

    if (failures != 0) {
        std::cout << failures << " mus test(s) failed\n";
        return 1;
    }
    std::cout << "all mus tests passed\n";
    return 0;
}
