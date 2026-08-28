#!/usr/bin/env bash
# Package a release tarball: the stripped `smart` binary, the EBMC it needs,
# and enough documentation to use them.
#
#   tools/release.sh [version]
#
# The binary links libcvc5 and the hw-cbmc/CBMC libraries statically, so the
# default flow's external tools are `ebmc` (shipped here) and Verilator (not —
# it is a large suite). Icarus Verilog remains an optional simulation backend.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$here"

version="${1:-$(git describe --tags --always --dirty 2>/dev/null || echo dev)}"
name="smart-$version-linux-x86_64"
stage="$here/dist/$name"

[ -x build/smart ] || { echo "build first: cmake --build build --target smart"; exit 2; }
[ -x .deps/hw-cbmc/src/ebmc/ebmc ] || { echo "hw-cbmc is not built"; exit 2; }

tool_available() {
    command -v "$1" >/dev/null 2>&1 ||
        [ -x "$here/otherTools/oss-cad-suite/bin/$1" ]
}

tool_available verilator || {
    echo "verilator is required by the default simulator; install it or oss-cad-suite"
    exit 2
}
if tool_available iverilog && tool_available vvp; then
    echo "[ok] optional Icarus backend available"
else
    echo "[optional] Icarus backend unavailable; default Verilator flow is unaffected"
fi

rm -rf "$stage"
mkdir -p "$stage/bin"

install -m 0755 build/smart "$stage/bin/smart"
install -m 0755 .deps/hw-cbmc/src/ebmc/ebmc "$stage/bin/ebmc"
strip --strip-unneeded "$stage/bin/smart" "$stage/bin/ebmc" 2>/dev/null || true

install -m 0644 README.md "$stage/README.md"
cp -r Config "$stage/Config"

cat > "$stage/README-release.txt" <<'EOF'
SMART — mine formally verified SystemVerilog assertions.

  bin/smart design.sv        ->  design_assertion.sv

Put bin/ on your PATH, or run the binaries from here. The default flow uses:

  ebmc       shipped in bin/ (built from the pinned hw-cbmc dependency)
  verilator  NOT shipped — install Verilator, or the oss-cad-suite
             bundle: https://github.com/YosysHQ/oss-cad-suite-build

Icarus Verilog (`iverilog` + `vvp`) is an optional alternative selected with
`--simulator iverilog`; it is not required for the default Verilator flow.

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
