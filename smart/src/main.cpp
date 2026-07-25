// smart: mine formally verified SystemVerilog assertions from one design file.
//
//   smart cat.sv        ->  cat_assertion.sv
//
// The binary has a second, internal role: `smart --block ...` runs a single
// synthesis block. The pipeline re-execs itself that way instead of shelling
// out to a separate worker, so there is still exactly one binary to ship —
// see BlockRunner.h for why blocks are processes and not threads.

#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "BlockRunner.h"
#include "HwcbmcFrontend.h"
#include "Options.h"
#include "Pipeline.h"
#include "SVModule.h"

namespace {

namespace fs = std::filesystem;

// Absolute path to this executable, for re-exec.
std::string selfPath(const char* argv0) {
    std::error_code error;
    const auto viaProc = fs::read_symlink("/proc/self/exe", error);
    if (!error) return viaProc.string();
    return fs::absolute(argv0).string();
}

int dumpFrontend(const smart::pipeline::Options& options) {
    if (options.designFiles().empty()) {
        std::cerr << "smart: --dump-frontend needs a design file\n";
        return static_cast<int>(smart::pipeline::ExitCode::UserError);
    }
    const auto& file = options.designFiles().front();
    std::string top = options.getString("top");
    if (top.empty()) top = fs::path(file).stem().string();

    smart::frontend::HwcbmcOptions frontendOptions;
    const auto info = smart::frontend::parseModuleFile(file, top, frontendOptions);
    if (!frontendOptions.elaborated)
        std::cerr << "smart: widths unresolved: " << frontendOptions.elaborationError
                  << "\n";
    std::cout << smart::frontend::dumpJson(info) << "\n";
    return 0;
}

}  // namespace

int main(int argc, char* argv[]) {
    using namespace smart::pipeline;

    // Internal: run one block. Everything after --block is the block's own
    // argv, matching the standalone smart.out contract.
    if (argc >= 2 && std::string(argv[1]) == "--block") {
        std::vector<char*> blockArgv;
        blockArgv.push_back(argv[0]);
        for (int i = 2; i < argc; ++i) blockArgv.push_back(argv[i]);
        return runSmartBlock(static_cast<int>(blockArgv.size()), blockArgv.data());
    }

    Options options;
    try {
        options.parseCommandLine(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "smart: " << e.what() << "\n\n" << Options::usage();
        return static_cast<int>(ExitCode::UserError);
    }

    if (options.helpRequested()) {
        std::cout << Options::usage();
        return 0;
    }

    const auto self = selfPath(argv[0]);

    if (options.checkEnvRequested())
        return static_cast<int>(checkEnvironment(options, self));

    if (options.dumpConfigRequested()) {
        for (const auto& warning : options.warnings())
            std::cerr << "note: " << warning << "\n";
        std::cout << options.toJson() << "\n";
        return 0;
    }

    if (options.dumpFrontendRequested()) {
        try {
            return dumpFrontend(options);
        } catch (const std::exception& e) {
            std::cerr << "smart: " << e.what() << "\n";
            return static_cast<int>(ExitCode::UserError);
        }
    }

    RunSummary summary;
    ExitCode code;
    try {
        Pipeline pipeline(options, self);
        code = pipeline.run(summary);
    } catch (const std::exception& e) {
        std::cerr << "smart: " << e.what() << "\n";
        return static_cast<int>(ExitCode::Failure);
    }

    if (code == ExitCode::Success) {
        std::cout << summary.verifiedAssertions << " verified assertion"
                  << (summary.verifiedAssertions == 1 ? "" : "s") << " of "
                  << summary.minedAssertions << " mined in " << summary.rounds
                  << " round" << (summary.rounds == 1 ? "" : "s") << ", "
                  << static_cast<int>(summary.seconds) << "s -> "
                  << summary.outputFile << "\n";
        if (summary.workDirKept)
            std::cout << "workdir kept at " << summary.workDir << "\n";
    } else if (!summary.workDir.empty() && summary.workDirKept) {
        std::cerr << "smart: workdir kept at " << summary.workDir << "\n";
    }
    return static_cast<int>(code);
}
