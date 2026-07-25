// vcd_signals: list the signals the pipeline's own trace loader finds in a
// VCD, for the module it cares about.
//   vcd_signals <top> <vcd-dir>
//
// This is the WP3 acceptance instrument: it answers "does Module::
// addTracesfromDir see the same signal set from an iverilog VCD as it did
// from the cocotb/verilator one?" — using the real loader, not a re-reading
// of the file.

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "Module.h"

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: vcd_signals <top> <vcd-dir>\n";
        return 1;
    }

    const std::string top = argv[1];
    const std::string dir = argv[2];

    try {
        Module module(top);
        module.addTracesfromDir(TraceType::SIM, dir);

        auto* signals = module.getAllSignals();
        std::vector<std::string> names;
        names.reserve(signals->size());
        for (const auto& signal : *signals) {
            std::string entry = signal.name;
            if (signal.lindex >= 0 || signal.rindex >= 0)
                entry += "[" + std::to_string(signal.lindex) + ":" +
                         std::to_string(signal.rindex) + "]";
            names.push_back(entry);
        }
        std::sort(names.begin(), names.end());

        for (const auto& name : names) std::cout << name << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "vcd_signals: " << e.what() << "\n";
        return 1;
    }
}
