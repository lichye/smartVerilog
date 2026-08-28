#include "Harness.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <unistd.h>

using smart::simgen::StateSignal;
using smart::simgen::enumerateStateSignals;

namespace {

namespace fs = std::filesystem;

int failures = 0;

void check(bool condition, const std::string& what) {
    if (condition) {
        std::cout << "ok  " << what << "\n";
    } else {
        std::cout << "FAIL " << what << "\n";
        ++failures;
    }
}

const StateSignal* findSignal(const std::vector<StateSignal>& signals,
                              const std::string& name) {
    for (const auto& signal : signals)
        if (signal.name == name) return &signal;
    return nullptr;
}

}  // namespace

int main() {
    const auto scratch =
        fs::temp_directory_path() /
        ("smart-verilator-state-" + std::to_string(static_cast<long>(::getpid())));
    const auto xmlDir = scratch / "xml";
    fs::create_directories(xmlDir);

    // enumerateStateSignals reuses an existing Verilator XML file. Keeping the
    // fixture inline makes this a deterministic parser regression with no tool
    // dependency: aggregate dtypes must never reach the generated C++ casts.
    std::ofstream xml(xmlDir / "Vtop.xml");
    xml << R"xml(<verilator_xml>
  <netlist>
    <module name="top" topModule="1">
      <var name="clk" dir="input" dtype_id="1"/>
      <var name="out" dir="output" dtype_id="2"/>
      <var name="count" dtype_id="2"/>
      <var name="mem" dtype_id="3"/>
      <var name="too_wide" dtype_id="4"/>
      <var name="__Vtmp" dtype_id="1"/>
    </module>
    <typetable>
      <basicdtype id="1" name="logic"/>
      <basicdtype id="2" name="logic" left="7" right="0"/>
      <unpackarraydtype id="3" sub_dtype_id="2"/>
      <basicdtype id="4" name="logic" left="127" right="0"/>
    </typetable>
  </netlist>
</verilator_xml>)xml";
    xml.close();

    const auto signals = enumerateStateSignals(
        {"unused.sv"}, "top", "verilator-is-not-invoked", scratch.string());

    const auto* out = findSignal(signals, "out");
    const auto* count = findSignal(signals, "count");
    check(out != nullptr && out->isPort && out->width == 8,
          "a scalar output is enumerated with its width");
    check(count != nullptr && !count->isPort && count->width == 8,
          "a scalar internal signal is enumerated");
    check(findSignal(signals, "clk") == nullptr, "inputs are excluded");
    check(findSignal(signals, "mem") == nullptr,
          "unpacked arrays are excluded from scalar state hashing");
    check(findSignal(signals, "too_wide") == nullptr,
          "signals wider than the QData accessor are excluded");
    check(findSignal(signals, "__Vtmp") == nullptr,
          "Verilator temporaries are excluded");

    std::error_code error;
    fs::remove_all(scratch, error);

    if (failures != 0) {
        std::cout << failures << " Verilator state test(s) failed\n";
        return 1;
    }
    std::cout << "all Verilator state tests passed\n";
    return 0;
}
