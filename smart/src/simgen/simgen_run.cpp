// simgen_run: drive the WP3 harness standalone.
//   simgen_run <design.sv> [top] [--cycles N] [--traces N] [--seed N]
//              [--workdir DIR] [--out DIR] [--print-tb]
// WP4 folds this into the `smart` binary's simulation stage.

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "Harness.h"
#include "HwcbmcFrontend.h"

namespace fs = std::filesystem;

static std::string stem(const std::string& path) {
    return fs::path(path).stem().string();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: simgen_run <design.sv> [top] [--cycles N] "
                     "[--traces N] [--seed N] [--workdir DIR] [--out DIR] "
                     "[--print-tb]\n";
        return 1;
    }

    const std::string design = argv[1];
    std::string top;
    smart::simgen::HarnessOptions options;
    std::string workDir, outDir;
    bool printTb = false;

    for (int i = 2; i < argc; ++i) {
        const std::string arg = argv[i];
        auto next = [&]() -> std::string {
            if (i + 1 >= argc) throw std::runtime_error("missing value for " + arg);
            return argv[++i];
        };
        try {
            if (arg == "--cycles") options.cycles = std::stoi(next());
            else if (arg == "--traces") options.traces = std::stoi(next());
            else if (arg == "--seed") options.seed = std::stoul(next());
            else if (arg == "--workdir") workDir = next();
            else if (arg == "--out") outDir = next();
            else if (arg == "--print-tb") printTb = true;
            else if (arg.rfind("--", 0) == 0) {
                std::cerr << "simgen_run: unknown option " << arg << "\n";
                return 1;
            } else if (top.empty()) top = arg;
        } catch (const std::exception& e) {
            std::cerr << "simgen_run: " << e.what() << "\n";
            return 1;
        }
    }

    if (top.empty()) top = stem(design);
    if (workDir.empty()) workDir = "simgen-work-" + top;
    if (outDir.empty()) outDir = workDir + "/sim_results";

    try {
        smart::frontend::HwcbmcOptions frontendOptions;
        const auto info = smart::frontend::parseModuleFile(design, top, frontendOptions);
        if (!frontendOptions.elaborated) {
            std::cerr << "simgen_run: warning: widths unresolved ("
                      << frontendOptions.elaborationError << ")\n";
        }

        if (printTb) {
            std::cout << smart::simgen::renderTestbench(info, options);
            return 0;
        }

        // Elaborate the design together with its siblings, exactly as the
        // frontend did, so submodules resolve for the simulator too.
        std::vector<std::string> designFiles{design};
        for (const auto& entry : fs::directory_iterator(
                 fs::path(design).parent_path().empty()
                     ? fs::path(".")
                     : fs::path(design).parent_path())) {
            if (!entry.is_regular_file()) continue;
            const auto extension = entry.path().extension().string();
            if (extension != ".sv" && extension != ".v") continue;
            std::error_code same;
            if (fs::equivalent(entry.path(), fs::path(design), same))
                continue;
            designFiles.push_back(entry.path().string());
        }
        std::sort(designFiles.begin() + 1, designFiles.end());

        const auto result =
            smart::simgen::runSimulations(info, designFiles, workDir, outDir, options);

        for (const auto& vcd : result.vcdPaths) std::cout << vcd << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "simgen_run: " << e.what() << "\n";
        return 3;
    }
}
