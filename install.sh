#!/usr/bin/env bash
# One-off local setup for SMART. No container, no root: everything lands under
# .deps/ and otherTools/, both of which are gitignored.
#
#   ./install.sh          # fetch + build everything, then verify
#   ./install.sh --check  # just report what is missing
#
# What gets installed:
#   .deps/hw-cbmc         EBMC (verification) + the Verilog frontend we link
#   .deps/include         header-only build dependencies
#   otherTools/oss-cad-suite  Verilator (default) + optional iverilog/vvp
#   otherTools/cvc5       libcvc5 (SyGuS + MSA/MUS), headers and static libs
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$here"

OSS_CAD_SUITE_DATE="${OSS_CAD_SUITE_DATE:-2023-05-20}"
HWCBMC_REPO="https://github.com/diffblue/hw-cbmc.git"
HWCBMC_REV="9b402aa7cfb1157b0399f9e684bdfc1b505ed50d"
NLOHMANN_JSON_VERSION="3.11.3"
DEPS_DIR="$here/.deps"
HWCBMC_DIR="$DEPS_DIR/hw-cbmc"
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

# cvc5's code generators need these two modules. We install them for the user
# and build cvc5 with --no-pyvenv, so the python3-venv package (a separate
# install on Debian/Ubuntu) is NOT required — see tools/build-cvc5.sh.
if command -v python3 >/dev/null 2>&1; then
    if python3 -m pip --version >/dev/null 2>&1; then
        echo "[ok]      python3-pip"
    else
        echo "[MISSING] python3-pip — needed to fetch cvc5's build-time modules"
        echo "          Debian/Ubuntu: apt-get install python3-pip"
        missing=1
    fi
fi

if [ "$missing" = 1 ] && [ "$check_only" = 0 ]; then
    echo
    echo "Install the missing tools above first; everything else is local."
    exit 2
fi

if [ "$check_only" = 1 ]; then
    for d in .deps/hw-cbmc/src/ebmc/ebmc \
             otherTools/oss-cad-suite/bin/verilator; do
        [ -x "$d" ] && echo "[ok]      $d" || { echo "[MISSING] $d"; missing=1; }
    done
    d=otherTools/cvc5/include/cvc5/cvc5.h
    [ -e "$d" ] && echo "[ok]      $d" || { echo "[MISSING] $d"; missing=1; }
    d=.deps/include/nlohmann/json.hpp
    [ -e "$d" ] && echo "[ok]      $d" || { echo "[MISSING] $d"; missing=1; }
    for d in otherTools/oss-cad-suite/bin/iverilog \
             otherTools/oss-cad-suite/bin/vvp; do
        if [ -x "$d" ]; then
            echo "[ok]      $d (optional Icarus backend)"
        else
            echo "[optional] $d — only needed for --simulator iverilog"
        fi
    done
    exit $missing
fi

echo
echo "== hw-cbmc (EBMC + Verilog frontend) =="
mkdir -p "$DEPS_DIR"
if [ ! -e "$HWCBMC_DIR/.git" ]; then
    git init "$HWCBMC_DIR"
    git -C "$HWCBMC_DIR" remote add origin "$HWCBMC_REPO"
fi
if ! git -C "$HWCBMC_DIR" cat-file -e "${HWCBMC_REV}^{commit}" 2>/dev/null; then
    git -C "$HWCBMC_DIR" fetch --depth 1 origin "$HWCBMC_REV"
fi
git -C "$HWCBMC_DIR" checkout --detach "$HWCBMC_REV"
git -C "$HWCBMC_DIR" submodule sync --recursive
git -C "$HWCBMC_DIR" submodule update --init --recursive --depth 1
./tools/build-hw-cbmc.sh "$jobs"

echo
echo "== header dependencies =="
json_header="$DEPS_DIR/include/nlohmann/json.hpp"
if [ ! -e "$json_header" ]; then
    mkdir -p "$(dirname "$json_header")"
    curl --fail --location \
        --output "$json_header" \
        "https://github.com/nlohmann/json/releases/download/v${NLOHMANN_JSON_VERSION}/json.hpp"
else
    echo "already present: $json_header"
fi

echo
echo "== simulators (Verilator default, Icarus optional) =="
mkdir -p otherTools
if [ ! -x otherTools/oss-cad-suite/bin/verilator ]; then
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
