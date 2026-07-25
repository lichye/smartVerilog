#!/usr/bin/env bash
# Build libcvc5 (headers + static libraries) into otherTools/cvc5.
#
# SMART links cvc5 for two things: solving the SyGuS problems in-process
# (WP6) and the MSA/MUS analysis (WP5). The released cvc5 zip ships only the
# solver BINARY, and the pip wheel ships a shared library with no headers, so
# there is no shortcut — the library has to be built once.
#
# Takes about half an hour on a many-core machine. One-off: the result is
# reused by every later build.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
version="${CVC5_VERSION:-1.2.0}"
jobs="${1:-$(nproc)}"
prefix="$here/otherTools/cvc5"
src="$here/otherTools/cvc5-src"

if [ -f "$prefix/include/cvc5/cvc5.h" ]; then
    echo "libcvc5 already present in $prefix"
    exit 0
fi

mkdir -p "$here/otherTools"
if [ ! -d "$src" ]; then
    git clone --depth 1 --branch "cvc5-$version" https://github.com/cvc5/cvc5.git "$src"
fi

# cvc5's build runs Python code generators that need these. Installing them
# for the user lets us pass --no-python-venv below, which is the difference
# between needing the python3-venv package and not.
python3 -m pip install --user --quiet pyparsing tomli || true

# libpoly, one of cvc5's downloaded dependencies, still declares
# cmake_minimum_required(VERSION <3.5), which CMake 4 refuses outright.
export CMAKE_POLICY_VERSION_MINIMUM=3.5

cd "$src"
# --no-pyvenv: cvc5 otherwise builds a virtual environment for its own
# code generators, which drags in the python3-venv package (a separate install
# on Debian/Ubuntu) purely as a build-time dependency of a dependency.
./configure.sh production --auto-download --static --no-static-binary \
    --no-pyvenv --prefix="$prefix"
make -C build -j"$jobs" install

# The static build links these in; the install does not copy them.
for lib in libgmp.a libgmpxx.a; do
    [ -f "build/deps/lib/$lib" ] && cp -a "build/deps/lib/$lib" "$prefix"/lib*/
done

echo "libcvc5 installed in $prefix"
"$prefix/bin/cvc5" --version | head -1
