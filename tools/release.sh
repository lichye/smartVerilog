#!/usr/bin/env bash
# Package a release tarball: the stripped `smart` binary, the EBMC it needs,
# and enough documentation to use them.
#
#   tools/release.sh [version]
#
# The binary links libcvc5 and the hw-cbmc/CBMC libraries statically, so the
# only runtime dependencies left are libstdc++, libm and the two external
# tools: `ebmc` (shipped here) and `iverilog` (not — it is a large suite, and
# the user is told where to get it).
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$here"

version="${1:-$(git describe --tags --always --dirty 2>/dev/null || echo dev)}"
name="smart-$version-linux-x86_64"
stage="$here/dist/$name"

[ -x build/smart ] || { echo "build first: cmake --build build --target smart"; exit 2; }
[ -x third_party/hw-cbmc/src/ebmc/ebmc ] || { echo "hw-cbmc is not built"; exit 2; }

rm -rf "$stage"
mkdir -p "$stage/bin"

install -m 0755 build/smart "$stage/bin/smart"
install -m 0755 third_party/hw-cbmc/src/ebmc/ebmc "$stage/bin/ebmc"
strip --strip-unneeded "$stage/bin/smart" "$stage/bin/ebmc" 2>/dev/null || true

install -m 0644 ReadMe.md "$stage/ReadMe.md"
cp -r Config "$stage/Config"

cat > "$stage/README-release.txt" <<'EOF'
SMART — mine formally verified SystemVerilog assertions.

  bin/smart design.sv        ->  design_assertion.sv

Put bin/ on your PATH, or run the binaries from here. `smart` needs two
external tools:

  ebmc       shipped in bin/ (built from the pinned hw-cbmc submodule)
  iverilog   NOT shipped — install Icarus Verilog, or the oss-cad-suite
             bundle: https://github.com/YosysHQ/oss-cad-suite-build

Check what the tool can see:

  bin/smart --check-env

Every option is also a config key of the same name; see `bin/smart --help`
and the example configurations in Config/.
EOF

tar -czf "dist/$name.tar.gz" -C "$here/dist" "$name"
echo "dist/$name.tar.gz"
ls -lh "dist/$name.tar.gz"
echo
echo "dynamic dependencies of the packaged binary:"
ldd "$stage/bin/smart" | sed 's/^/  /'
