#!/usr/bin/env bash
# Apply SMART's local compatibility patches to the downloaded hw-cbmc tree.
# Idempotent: a patch that is already applied is skipped, not re-applied.
# See tools/patches/hw-cbmc/README.md for why each patch exists.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
submodule="$(cd "$here/.." && pwd)/.deps/hw-cbmc"
patches="$here/patches/hw-cbmc"

if [ ! -d "$submodule/src" ]; then
    echo "error: $submodule is missing — run ./install.sh" >&2
    exit 1
fi

shopt -s nullglob
for patch in "$patches"/*.patch; do
    name="$(basename "$patch")"
    if git -C "$submodule" apply --reverse --check "$patch" >/dev/null 2>&1; then
        echo "already applied: $name"
    elif git -C "$submodule" apply --check "$patch" >/dev/null 2>&1; then
        git -C "$submodule" apply "$patch"
        echo "applied:         $name"
    else
        echo "error: $name applies neither forward nor in reverse;" >&2
        echo "       the pinned dependency probably moved — refresh the patch." >&2
        exit 1
    fi
done
