#!/usr/bin/env bash
# Build the hw-cbmc submodule: the `ebmc` binary (verification backend) and
# the static libraries the SMART frontend adapter links against (WP2A).
#
# Usage: third_party/build-hw-cbmc.sh [make-jobs]
#
# This compiles most of CBMC and takes a while — in Docker it must live in
# its own cached layer (plan gotcha 17).
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
submodule="$here/hw-cbmc"
jobs="${1:-$(nproc)}"

if [ ! -d "$submodule/lib/cbmc/src" ]; then
    echo "error: nested cbmc submodule missing — run:" >&2
    echo "       git submodule update --init --recursive" >&2
    exit 1
fi

"$here/apply-patches.sh"

# CBMC's SAT backend is downloaded, not vendored.
if [ ! -d "$submodule/lib/cbmc/minisat-2.2.1" ]; then
    make -C "$submodule/lib/cbmc/src" minisat2-download
fi

make -C "$submodule/src" -j"$jobs"

"$submodule/src/ebmc/ebmc" --version
