// WorkDir: the directory tree a run happens in.
//
// The layout mirrors the legacy `smart/runtime/` tree on purpose: the reused
// C++ core (smart.cpp, VerilogChecker) resolves several of these paths
// relative to its working directory, so the names are a contract, not a
// preference. Grep `runtime/` under smart/src/ before renaming anything here.

#ifndef SMART_PIPELINE_WORKDIR_H
#define SMART_PIPELINE_WORKDIR_H

#include <string>
#include <vector>

namespace smart {
namespace pipeline {

class WorkDir {
public:
    explicit WorkDir(std::string root);

    // Create the tree. Existing content is left alone so a run can be resumed
    // or inspected between stages.
    void create() const;

    const std::string& root() const { return root_; }

    std::string runtime() const;        // <root>/runtime
    std::string verilogDir() const;     // design sources, as the checker sees them
    std::string formalDir() const;      // per-assertion formal copies
    std::string simResultsDir() const;  // simN.vcd
    std::string smtResultsDir() const;  // EBMC counterexample traces
    std::string variablesDir() const;   // one file per block: its variable subset
    std::string variablesFile() const;  // every candidate variable, one per line
    std::string sygusResultFile() const;
    std::string compareResultFile() const;
    std::string resultDir() const;      // one file per block: its assertion
    std::string logsDir() const;
    std::string simSrcDir() const;      // assume-stripped copies for simulation
    std::string effectiveConfigFile() const;
    std::string assertionsFile() const;
    std::string invariantsFile() const;
    // One JSON object per line: every stage and every block, with timings and
    // outcomes. Written as the run goes, so it survives a crash or a kill.
    std::string runLogFile() const;

    // Remove the tree. Only ever called on success and without --keep-work.
    void remove() const;

private:
    std::string root_;
};

}  // namespace pipeline
}  // namespace smart

#endif  // SMART_PIPELINE_WORKDIR_H
