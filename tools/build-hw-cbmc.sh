#!/usr/bin/env bash
# Build the downloaded hw-cbmc checkout: the `ebmc` binary (verification backend) and
# the static libraries the SMART frontend adapter links against (WP2A).
#
# Usage: tools/build-hw-cbmc.sh [make-jobs]
#
# This compiles most of CBMC and takes a while.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
submodule="$(cd "$here/.." && pwd)/.deps/hw-cbmc"
patches_dir="$here/patches/hw-cbmc"
jobs="${1:-$(nproc)}"

if [ ! -d "$submodule/lib/cbmc/src" ]; then
    echo "error: nested cbmc dependency missing — run ./install.sh" >&2
    exit 1
fi

# Patches are needed only while compiling hw-cbmc.  Record which ones were
# already present so a normal `./install.sh` leaves the dependency clean rather
# than making every developer's checkout look modified.  Pre-existing manual
# patches are deliberately left alone.
shopt -s nullglob
patches=("$patches_dir"/*.patch)
pre_applied=()
for patch in "${patches[@]}"; do
    if git -C "$submodule" apply --reverse --check "$patch" >/dev/null 2>&1; then
        pre_applied+=("$patch")
    fi
done

restore_new_patches() {
    local patch known
    for patch in "${patches[@]}"; do
        known=0
        for existing in "${pre_applied[@]}"; do
            [ "$patch" = "$existing" ] && known=1 && break
        done
        if [ "$known" = 0 ] && \
           git -C "$submodule" apply --reverse --check "$patch" >/dev/null 2>&1; then
            git -C "$submodule" apply --reverse "$patch"
        fi
    done
}
trap restore_new_patches EXIT

"$here/apply-hw-cbmc-patches.sh"

# CBMC's SAT backend is downloaded, not vendored.
if [ ! -d "$submodule/lib/cbmc/minisat-2.2.1" ]; then
    make -C "$submodule/lib/cbmc/src" minisat2-download
fi

make -C "$submodule/src" -j"$jobs"

"$submodule/src/ebmc/ebmc" --version
