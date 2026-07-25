#!/usr/bin/env bash
# Final regression: every listed design in both block strategies, with each
# emitted file re-proved by an INDEPENDENT ebmc run — because "the tool says
# it verified them" is not evidence, and the whole promise of the output file
# is that a third party can check it.
#
#   tools/smoke.sh [jobs]
set -uo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$here"

jobs="${1:-16}"
work="${SMOKE_DIR:-$here/smoke-out}"
smart="$here/build/bin/smart"
ebmc="$here/third_party/hw-cbmc/src/ebmc/ebmc"

[ -x "$smart" ] || { echo "build the tool first: cmake --build build --target smart"; exit 2; }

# Put our own tools first (the suite ships an older cvc5).
export PATH="$here/third_party/hw-cbmc/src/ebmc:$here/otherTools/cvc5/bin:$here/otherTools/oss-cad-suite/bin:$PATH"

designs=(
    "Benchmark/user/tiny_and tiny_and"
    "Benchmark/fmcad2025/c17 c17"
    "Benchmark/fmcad2025/s27 s27"
    "Benchmark/fmcad2025/arb2 arb2"
    "Benchmark/HWSpec/axis_fifo axis_fifo"
    "artifact/CaseStudy/Input/nru_a nru_a"
)

rm -rf "$work"
mkdir -p "$work"
failures=0

printf '%-12s %-8s %10s %8s %s\n' DESIGN MODE ASSERTIONS TIME EBMC
for entry in "${designs[@]}"; do
    set -- $entry
    src="$1"; top="$2"
    [ -f "$here/$src/$top.sv" ] || { printf '%-12s %-8s %10s\n' "$top" "-" "no source"; continue; }

    for mode in plain msa; do
        flags=""
        [ "$mode" = msa ] && flags="--msa"

        dir="$work/${top}_${mode}"
        mkdir -p "$dir"
        cp "$here/$src"/*.sv "$dir"/ 2>/dev/null

        start=$(date +%s)
        count=$(cd "$dir" && timeout 3600 "$smart" "$top.sv" --jobs "$jobs" \
                    --core-timeout 60 $flags -q 2>"$dir/stderr.log" \
                    | grep -oE '^[0-9]+' | head -1)
        status=$?
        elapsed=$(( $(date +%s) - start ))
        count="${count:-0}"

        verdict="SKIP"
        if [ "$status" -ne 0 ]; then
            verdict="RUN-FAIL"
            failures=$((failures + 1))
        elif [ -f "$dir/${top}_assertion.sv" ]; then
            if "$ebmc" "$dir/${top}_assertion.sv" -D FORMAL --bound 10 --top "$top" \
                 >/dev/null 2>&1; then
                verdict="PASS"
            else
                verdict="EBMC-FAIL"
                failures=$((failures + 1))
            fi
        else
            verdict="NO-OUTPUT"
            failures=$((failures + 1))
        fi
        printf '%-12s %-8s %10s %7ss %s\n' "$top" "$mode" "$count" "$elapsed" "$verdict"
    done
done

echo
if [ "$failures" -eq 0 ]; then
    echo "smoke matrix clean"
else
    echo "$failures failure(s); artefacts kept in $work"
fi
exit $failures
