// Spec tests for how the trace loader names signals across a hierarchy.
//
// The whole "mine the top without hand-flattening" feature is this one
// decision: which VCD scopes contribute candidate variables, and under what
// name. Both simulators the pipeline reads put the mined module inside a
// wrapper scope (Verilator's `TOP`, whose signals must NOT leak into the
// candidate set) and both nest instances below it, so the fixture below is
// shaped like a real trace rather than like a minimal one.

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "Blocks.h"

namespace fs = std::filesystem;
using smart::pipeline::candidateVariables;

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

// TOP > top > U0 > V0, in the shape VerilatedVcd writes: the wrapper scope
// carries the design's ports, and each instance repeats the ports it is
// connected to.
const char* kVcd = R"(
$timescale 1ps $end
 $scope module TOP $end
  $var wire  1 # clk $end
  $var wire  4 $ din [3:0] $end
  $scope module top $end
   $var wire  1 % clk $end
   $var wire  4 & din [3:0] $end
   $var wire  4 ' r [3:0] $end
   $scope module U0 $end
    $var wire  1 ( clk $end
    $var wire  4 ) q [3:0] $end
    $scope module V0 $end
     $var wire  4 * lq [3:0] $end
    $upscope $end
   $upscope $end
  $upscope $end
 $upscope $end
$enddefinitions $end
#0
1#
b0000 $
1%
b0000 &
b0000 '
1(
b0000 )
b0000 *
#5
0#
0%
0(
)";

std::string writeTraceDir() {
    const std::string root =
        (fs::temp_directory_path() / "smart_test_trace").string();
    fs::remove_all(root);
    fs::create_directories(root);
    std::ofstream out(root + "/sim1.vcd");
    out << kVcd;
    return root;
}

bool has(const std::vector<std::string>& names, const std::string& name) {
    return std::find(names.begin(), names.end(), name) != names.end();
}

void testFlatNaming(const std::string& dir) {
    const auto found = candidateVariables("top", dir, false);
    check(found.names == std::vector<std::string>({"clk", "din", "r"}),
          "leaf-scope mining sees the mined scope's own signals, bare");
    check(found.fromSubScopes == 0, "and reports nothing from below it");
}

void testHierarchicalNaming(const std::string& dir) {
    const auto found = candidateVariables("top", dir, true);
    check(found.names == std::vector<std::string>({"U0.V0.lq", "U0.clk", "U0.q",
                                                   "clk", "din", "r"}),
          "hierarchical mining adds every scope below, by instance path");
    check(found.fromSubScopes == 3, "three of the six came from below");
    // The wrapper the simulator wraps the design in is not part of the design.
    // Its signals share their leaf names with the top's, so letting them in
    // would silently duplicate every port.
    check(!has(found.names, "TOP.clk") && !has(found.names, "TOP.din"),
          "the simulator's TOP wrapper stays out: it is above the mined scope");
}

void testDepthIsNotTruncated(const std::string& dir) {
    const auto found = candidateVariables("top", dir, true);
    check(has(found.names, "U0.V0.lq"),
          "two levels of nesting keep both instance names");
    check(!has(found.names, "V0.lq") && !has(found.names, "lq"),
          "and are never shortened to something ambiguous");
}

void testMiningASubmoduleScope(const std::string& dir) {
    // --module names a scope partway down. Its own signals are the candidates,
    // named bare, exactly as before hierarchy existed.
    const auto found = candidateVariables("U0", dir, false);
    check(found.names == std::vector<std::string>({"clk", "q"}),
          "--module still mines one scope, with bare names");
}

}  // namespace

int main() {
    const auto dir = writeTraceDir();
    testFlatNaming(dir);
    testHierarchicalNaming(dir);
    testDepthIsNotTruncated(dir);
    testMiningASubmoduleScope(dir);
    fs::remove_all(dir);

    if (failures != 0) {
        std::cout << failures << " trace test(s) failed\n";
        return 1;
    }
    std::cout << "all trace tests passed\n";
    return 0;
}
