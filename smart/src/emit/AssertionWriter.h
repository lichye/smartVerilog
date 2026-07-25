// AssertionWriter: the final formal gate, and the file the user actually
// gets.
//
// checkAssertions() re-proves every mined assertion against the ORIGINAL
// design with EBMC, in parallel and under a timeout; only what survives is
// emitted. That the assertions were already verified inside their blocks is
// not enough — a block proves against its own narrowed view, and the whole
// point of this stage is that the shipped file is sound against the real
// design.
//
// writeAssertionFile() then injects the survivors into a copy of the user's
// file, ahead of the top module's `endmodule`, leaving everything else — the
// user's assumes, formatting, comments — exactly as it was.

#ifndef SMART_EMIT_ASSERTIONWRITER_H
#define SMART_EMIT_ASSERTIONWRITER_H

#include <string>
#include <vector>

namespace smart {
namespace emit {

struct CheckOptions {
    std::string ebmc = "ebmc";
    std::string topModule;
    int bound = 10;
    bool unbounded = false;   // --k-induction instead of a bound
    int timeoutSeconds = 180;
    int jobs = 1;
    std::string scratchDir;   // where the per-assertion copies are written
    std::vector<std::string> extraFiles;  // other design files to pass to EBMC
};

enum class CheckStatus { Verified, Refuted, TimedOut, Error };

struct CheckResult {
    std::string assertion;
    CheckStatus status = CheckStatus::Error;
    double seconds = 0.0;
};

// The EBMC command for one assertion. Split out because getting it wrong is
// silent — a missing space once turned `-D FORMAL` into part of a filename —
// and because it is the one piece here worth unit-testing (plan gotcha 7).
std::string ebmcCommand(const std::string& designFile, const CheckOptions& options);

// Inject `assertion` into `source` before the `endmodule` that closes module
// `top`. Returns the modified source.
std::string injectAssertions(const std::string& source, const std::string& top,
                             const std::vector<std::string>& assertions);

std::vector<CheckResult> checkAssertions(const std::string& designFile,
                                         const std::vector<std::string>& assertions,
                                         const CheckOptions& options);

// Write <output>: the original design with the verified assertions injected,
// preceded by a header naming the settings that produced them.
void writeAssertionFile(const std::string& designFile, const std::string& output,
                        const std::string& top,
                        const std::vector<std::string>& assertions,
                        const std::string& configJson);

}  // namespace emit
}  // namespace smart

#endif  // SMART_EMIT_ASSERTIONWRITER_H
