# Put this checkout's own tools on PATH.  Source it, do not run it:
#
#     source load_environment.sh
#
# SMART needs four external programs, and this repo carries all of them:
#
#   smart      build/                            the binary itself
#   ebmc       third_party/hw-cbmc/src/ebmc/     model checker (submodule)
#   cvc5       otherTools/cvc5/bin/              SyGuS + MSA solver
#   verilator  otherTools/oss-cad-suite/bin/     trace generation (iverilog too)
#
# Order matters in exactly one place: oss-cad-suite ships its own cvc5
# (1.0.1-dev), and ours (1.2.0) must win.  Sourcing this puts ours first.
#
# The `smart` binary already appends these directories to its own PATH at
# startup, so it runs correctly without this file.  What needs it is everything
# else: calling `ebmc` by hand, running smart/evaluater.py, poking at cvc5.

# --- locate the checkout, whatever directory you sourced this from ----------
if [ -n "${BASH_SOURCE[0]:-}" ]; then
    _smart_self="${BASH_SOURCE[0]}"
elif [ -n "${ZSH_VERSION:-}" ]; then
    _smart_self="${(%):-%x}"
else
    echo "load_environment.sh: source me from bash or zsh" >&2
    return 1 2>/dev/null || exit 1
fi

case "$0" in
    *load_environment.sh)
        echo "load_environment.sh: source me, do not run me:" >&2
        echo "    source load_environment.sh" >&2
        exit 1
        ;;
esac

SMART_ROOT="$(cd "$(dirname "$_smart_self")" && pwd)"
unset _smart_self
export SMART_ROOT

# --- prepend, skipping what is already there so re-sourcing is harmless -----
_smart_prepend() {
    [ -d "$1" ] || return 0
    case ":$PATH:" in
        *":$1:"*) return 0 ;;
    esac
    PATH="$1:$PATH"
}

# Prepended in reverse priority: the last one ends up first.
_smart_prepend "$SMART_ROOT/otherTools/oss-cad-suite/bin"
_smart_prepend "$SMART_ROOT/third_party/hw-cbmc/src/ebmc"
_smart_prepend "$SMART_ROOT/otherTools/cvc5/bin"   # must beat oss-cad-suite's
_smart_prepend "$SMART_ROOT/build"
export PATH
unset -f _smart_prepend

# --- report, and say what to do about anything missing ----------------------
_smart_report() {
    local name="$1" path="$2" version="$3" fix="$4"
    if [ -x "$path" ]; then
        printf '  %-10s %s\n' "$name" "$version"
    else
        printf '  %-10s MISSING -- %s\n' "$name" "$fix"
        return 1
    fi
}

echo "SMART environment from $SMART_ROOT"
_smart_missing=0
_smart_report smart "$SMART_ROOT/build/smart" \
    "$("$SMART_ROOT/build/smart" --version 2>/dev/null | head -1)" \
    "cmake -B build && cmake --build build --target smart -j\$(nproc)" || _smart_missing=1
_smart_report ebmc "$SMART_ROOT/third_party/hw-cbmc/src/ebmc/ebmc" \
    "$("$SMART_ROOT/third_party/hw-cbmc/src/ebmc/ebmc" --version 2>/dev/null | head -1)" \
    "./install.sh  (builds the hw-cbmc submodule)" || _smart_missing=1
_smart_report cvc5 "$SMART_ROOT/otherTools/cvc5/bin/cvc5" \
    "$("$SMART_ROOT/otherTools/cvc5/bin/cvc5" --version 2>/dev/null | head -1 | sed 's/This is //')" \
    "tools/build-cvc5.sh  (~30 min, one-off)" || _smart_missing=1
_smart_report verilator "$SMART_ROOT/otherTools/oss-cad-suite/bin/verilator" \
    "$("$SMART_ROOT/otherTools/oss-cad-suite/bin/verilator" --version 2>/dev/null | head -1)" \
    "./install.sh  (downloads oss-cad-suite)" || _smart_missing=1
_smart_report iverilog "$SMART_ROOT/otherTools/oss-cad-suite/bin/iverilog" \
    "$("$SMART_ROOT/otherTools/oss-cad-suite/bin/iverilog" -V 2>/dev/null | head -1)" \
    "./install.sh  (downloads oss-cad-suite)" || _smart_missing=1
unset -f _smart_report

if [ "$_smart_missing" -ne 0 ]; then
    echo
    echo "Something above is missing; the fix is on its line."
fi
unset _smart_missing

cat <<'EOF'

Check the toolchain the way smart itself sees it:
  smart --check-env

Try it:
  smart Benchmark/fmcad2025/s27/s27.sv          # -> s27_assertion.sv
  smart <design>.sv --config Config/block_msa_mini.json --jobs 16
  smart --help | less                           # every flag and config key

Keep the work directory to see what happened:
  smart <design>.sv --keep-work                 # -> smart-work-<top>/
  jq -c 'select(.stage)' smart-work-<top>/run-log.jsonl   # per-stage log
EOF
