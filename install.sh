#!/usr/bin/env bash
# One-off local setup for SMART. No container, no root: everything lands under
# otherTools/ and third_party/, both of which are gitignored.
#
#   ./install.sh          # fetch + build everything, then verify
#   ./install.sh --check  # just report what is missing
#
# What gets installed:
#   third_party/hw-cbmc   EBMC (verification) + the Verilog frontend we link
#   otherTools/oss-cad-suite  iverilog + vvp (simulation)
#   otherTools/cvc5       libcvc5 (SyGuS + MSA/MUS), headers and static libs
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$here"

OSS_CAD_SUITE_DATE="${OSS_CAD_SUITE_DATE:-2023-05-20}"
jobs="$(nproc)"

check_only=0
[ "${1:-}" = "--check" ] && check_only=1

missing=0
need() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "[MISSING] $1 — $2"
        missing=1
    else
        echo "[ok]      $1"
    fi
}

echo "== host toolchain =="
need g++ "a C++17 compiler"
need make "GNU make"
need flex "flex"
need bison "bison"
need git "git"
need curl "curl (cvc5 and CBMC download dependencies)"
need cmake "cmake >= 3.16 — 'python3 -m pip install --user cmake' works"
need python3 "python3 (cvc5's build scripts)"

# cvc5's build creates a virtual environment for its own tooling, and on
# Debian/Ubuntu `python3` alone does not provide that — the failure surfaces
# deep inside cvc5's cmake as "Could not create Python virtual environment".
if command -v python3 >/dev/null 2>&1; then
    if python3 -m venv --help >/dev/null 2>&1; then
        echo "[ok]      python3-venv"
    else
        echo "[MISSING] python3-venv — cvc5 cannot build without it"
        echo "          Debian/Ubuntu: apt-get install python3-venv"
        missing=1
    fi
fi

if [ "$missing" = 1 ] && [ "$check_only" = 0 ]; then
    echo
    echo "Install the missing tools above first; everything else is local."
    exit 2
fi

if [ "$check_only" = 1 ]; then
    for d in third_party/hw-cbmc/src/ebmc/ebmc otherTools/oss-cad-suite/bin/iverilog \
             otherTools/cvc5/include/cvc5/cvc5.h; do
        [ -e "$d" ] && echo "[ok]      $d" || { echo "[MISSING] $d"; missing=1; }
    done
    exit $missing
fi

echo
echo "== hw-cbmc (EBMC + Verilog frontend) =="
git submodule update --init --recursive
./third_party/build-hw-cbmc.sh "$jobs"

echo
echo "== iverilog (simulation) =="
mkdir -p otherTools
if [ ! -x otherTools/oss-cad-suite/bin/iverilog ]; then
    stamp="$(printf '%s' "$OSS_CAD_SUITE_DATE" | tr -d '-')"
    tarball="oss-cad-suite-linux-x64-${stamp}.tgz"
    (cd otherTools && \
     curl -L -o "$tarball" \
       "https://github.com/YosysHQ/oss-cad-suite-build/releases/download/${OSS_CAD_SUITE_DATE}/${tarball}" && \
     tar -xzf "$tarball")
else
    echo "already present"
fi

echo
echo "== libcvc5 (SyGuS + MSA/MUS) =="
./tools/build-cvc5.sh "$jobs"

echo
echo "== build smart =="
cmake -B build -S .
cmake --build build --target smart -j"$jobs"

echo
echo "== verify =="
# No PATH juggling here on purpose: the binary finds the tools this script
# just built, relative to itself. If that stops being true, this fails.
./build/smart --check-env
echo
echo "Done. Try:  ./build/smart Benchmark/user/tiny_and/tiny_and.sv"
