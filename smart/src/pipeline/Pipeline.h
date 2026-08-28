// Pipeline: the stages of one `smart <design.sv>` run.
//
//   frontend -> workdir -> simulate -> pre-analysis -> synthesis rounds
//            -> final check -> emit
//
// This is the port of setup.py + smart.py + checker.py. It orchestrates only;
// every stage lives in its own component.

#ifndef SMART_PIPELINE_PIPELINE_H
#define SMART_PIPELINE_PIPELINE_H

#include <string>
#include <vector>

#include "Options.h"

namespace smart {
namespace pipeline {

// Exit codes are the CLI contract (plan §1): 0 success, 1 user error,
// 2 environment error, 3 pipeline failure. A run that verifies nothing is
// still a success — it reports zero assertions.
enum class ExitCode { Success = 0, UserError = 1, EnvironmentError = 2, Failure = 3 };

struct RunSummary {
    std::string top;
    std::string outputFile;
    std::size_t minedAssertions = 0;
    std::size_t verifiedAssertions = 0;
    bool finalChecked = true;
    bool finalUnbounded = true;
    int rounds = 0;
    double seconds = 0.0;
    std::string workDir;
    bool workDirKept = false;
};

class Pipeline {
public:
    Pipeline(Options options, std::string selfExecutable);

    ExitCode run(RunSummary& summary);

private:
    Options options_;
    std::string self_;
};

// `--check-env`: report the tools a run needs and whether they are usable.
// Returns EnvironmentError if anything required is missing.
ExitCode checkEnvironment(const Options& options, const std::string& selfExecutable);

}  // namespace pipeline
}  // namespace smart

#endif  // SMART_PIPELINE_PIPELINE_H
