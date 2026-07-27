#!/bin/bash
# One experiment: mine, then measure mutation detection on the result.
# Usage: eval_one.sh <config> <design> <mine_timeout> <eval_timeout>
cd /work/smartVerilog
# The evaluator is Python and finds its tools on PATH only. It also cannot
# tell "assertion violated" from "ebmc failed to run" — both are a non-zero
# exit — so a missing ebmc reports every mutant as detected, i.e. 100%.
export PATH="/work/smartVerilog/third_party/hw-cbmc/src/ebmc:/work/smartVerilog/otherTools/cvc5/bin:/work/smartVerilog/otherTools/oss-cad-suite/bin:$PATH"
command -v ebmc >/dev/null || { echo "FATAL: ebmc not on PATH"; exit 2; }
cfg=$1; b=$2; mto=${3:-2400}; eto=${4:-3600}
d=$(find Benchmark -type d -name "$b" | head -1)
[ -n "$d" ] && [ -f "$d/$b.sv" ] || { echo "$cfg $b MISSING"; exit 0; }

wk=/work/wk/${cfg}_${b}
out=$(timeout $mto ./build/smart "$d/$b.sv" --top "$b" --config "Config/$cfg.json" \
        --jobs 16 --core-timeout 60 --workdir "$wk" \
        --output "/work/out/${cfg}_${b}.sv" --keep-work -q 2>/dev/null \
      | grep 'verified assertion' | tail -1)
v=$(echo "$out" | grep -oE '^[0-9]+'); t=$(echo "$out" | sed -n 's/.*, \([0-9]*\)s ->.*/\1/p')
[ -z "$v" ] && { echo "$cfg $b MINE-FAIL"; exit 0; }

# The evaluator wants the legacy smart/ layout: its own directory, with
# invariants.txt, runtime/verilog/ and a benchmarks/ dir for the mutants.
ev=/work/ev/${cfg}_${b}
rm -rf "$ev"; mkdir -p "$ev/runtime/verilog" "$ev/src"
cp smart/evaluater.py "$ev/"
cp -r smart/src/python "$ev/src/"
cp "$wk/invariants.txt" "$ev/" 2>/dev/null
cp "$wk"/runtime/verilog/*.sv "$ev/runtime/verilog/" 2>/dev/null

# Mutants come from the fixed MutationBenchmark, never from a fresh
# mutation.py run: the generator picks its operators randomly, so two runs
# produce the same COUNT of mutants with different content (383 mutants,
# 294 of them different, measured on c880). Comparing detection rates across
# runs is only meaningful against one fixed set. setupMutants() skips
# generation when benchmarks/ is already populated.
if [ -d "/work/MB/$b/benchmarks" ]; then
    cp -r "/work/MB/$b/benchmarks" "$ev/benchmarks"
else
    echo "$cfg $b NO-FIXED-MUTANTS"; exit 0
fi

md=$( cd "$ev" && timeout $eto python3 evaluater.py "$b" 10 2>/dev/null \
      | grep -oE 'Mutation Detection\(MD\) rate: *[0-9.]+' | grep -oE '[0-9.]+$' )
mut=$(ls "$ev/benchmarks"/mutant_*.sv 2>/dev/null | grep -cv _assertion)
echo "$cfg $b ${v} ${t:-?} ${md:-EVAL-FAIL} ${mut:-0}"
