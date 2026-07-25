// HwcbmcFrontend: fills the ModuleInfo contract from hw-cbmc's Verilog
// frontend (WP2). This is the SHIPPING frontend; SVModule's regex parser is
// the frozen oracle it is checked against (tools/compare_frontends.py).
//
// Two hw-cbmc passes are needed and neither alone is enough:
//   * parse      — declaration order, `(* anyseq *)` / `(* anyconst *)`
//                  attributes, and whether the module contains assumes;
//   * typecheck  — resolved bit widths. The parse tree keeps ranges
//                  unevaluated (`[W-1:0]` stays an expression), the
//                  elaborated symbol table has real `unsignedbv` widths.
//
// Elaboration goes through langapi's `language_filest`, the same path ebmc
// uses, because it orders modules by dependency. Calling
// `verilog_languaget::typecheck` directly on the top module only works for
// designs with no submodules — anything that instantiates another module
// (s27's `dff`, the i2c cores) trips an invariant inside verilog_synthesis
// and ABORTS the process rather than returning an error.

#ifndef SMART_FRONTEND_HWCBMCFRONTEND_H
#define SMART_FRONTEND_HWCBMCFRONTEND_H

#include <string>
#include <vector>

#include "SVModule.h"

namespace smart {
namespace frontend {

struct HwcbmcOptions {
    // Additional design files to elaborate together with the main one (the
    // plan's multi-file case). The directory of the main file is scanned for
    // sibling `.sv`/`.v` files when `autoDiscoverSiblings` is set.
    std::vector<std::string> extraFiles;
    bool autoDiscoverSiblings = true;

    // `-I` paths for the Verilog preprocessor. The directory holding the
    // design is always searched, so `` `include "ibex_pkg.sv" `` resolves.
    std::vector<std::string> includePaths;
    // Extra `-D` defines, "NAME" or "NAME=VALUE".
    std::vector<std::string> defines;

    // Outputs. When elaboration fails the parse-tree-only view is still
    // returned, with widths unresolved (-1), so callers degrade the way the
    // regex oracle does instead of losing the module entirely.
    bool elaborated = true;
    std::string elaborationError;
};

// Parse `path` and return the ModuleInfo for module `top` (empty = the first
// module in the file). Throws std::runtime_error when the file cannot be
// parsed or `top` is absent — same contract as SVModule::parseModule.
ModuleInfo parseModuleFile(const std::string& path, const std::string& top,
                           HwcbmcOptions& options);

// Convenience overload for callers that do not need the diagnostics.
ModuleInfo parseModuleFile(const std::string& path, const std::string& top);

}  // namespace frontend
}  // namespace smart

#endif  // SMART_FRONTEND_HWCBMCFRONTEND_H
