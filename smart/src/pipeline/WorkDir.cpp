#include "WorkDir.h"

#include <filesystem>
#include <fstream>

namespace smart {
namespace pipeline {
namespace {
namespace fs = std::filesystem;
}

WorkDir::WorkDir(std::string root) : root_(std::move(root)) {}

std::string WorkDir::runtime() const { return root_ + "/runtime"; }
std::string WorkDir::verilogDir() const { return runtime() + "/verilog"; }
std::string WorkDir::formalDir() const { return runtime() + "/formal"; }
std::string WorkDir::simResultsDir() const { return runtime() + "/sim_results"; }
std::string WorkDir::smtResultsDir() const { return runtime() + "/smt_results"; }
std::string WorkDir::variablesDir() const { return runtime() + "/variables"; }
std::string WorkDir::variablesFile() const { return runtime() + "/variables.txt"; }
std::string WorkDir::sygusResultFile() const { return runtime() + "/SygusResult.sl"; }
std::string WorkDir::compareResultFile() const {
    return runtime() + "/CompareResult.txt";
}
std::string WorkDir::resultDir() const { return root_ + "/result"; }
std::string WorkDir::logsDir() const { return runtime() + "/logs"; }
std::string WorkDir::simSrcDir() const { return root_ + "/sim_src"; }
std::string WorkDir::effectiveConfigFile() const {
    return root_ + "/effective-config.json";
}
std::string WorkDir::assertionsFile() const { return root_ + "/assertions.txt"; }
std::string WorkDir::invariantsFile() const { return root_ + "/invariants.txt"; }

void WorkDir::create() const {
    for (const auto& dir : {runtime(), verilogDir(), formalDir(), simResultsDir(),
                            smtResultsDir(), variablesDir(), resultDir(), logsDir(),
                            simSrcDir()})
        fs::create_directories(dir);

    // The blocks append to this one from several processes; it has to exist
    // before the first of them starts.
    if (!fs::exists(sygusResultFile())) std::ofstream(sygusResultFile()).close();
}

void WorkDir::remove() const {
    std::error_code error;
    fs::remove_all(root_, error);
}

}  // namespace pipeline
}  // namespace smart
