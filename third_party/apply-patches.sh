#!/usr/bin/env bash
# Apply the local patches in third_party/patches/ to the hw-cbmc submodule.
# Idempotent: a patch that is already applied is skipped, not re-applied.
# See third_party/patches/README.md for why each patch exists.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
submodule="$here/hw-cbmc"
patches="$here/patches"

if [ ! -d "$submodule/src" ]; then
    echo "error: $submodule is empty — run:" >&2
    echo "       git submodule update --init --recursive" >&2
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
        echo "       the submodule pin probably moved — refresh the patch." >&2
        exit 1
    fi
done
