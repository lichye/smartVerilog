#!/bin/bash
# Mine with the shipped defaults under one trace policy, then score against the
# fixed MutationBenchmark mutants.  eval_pol.sh <policy> <design>
set -u
pol=$1; b=$2
cd /work/smartVerilog
export PATH="/work/smartVerilog/.deps/hw-cbmc/src/ebmc:/work/smartVerilog/otherTools/cvc5/bin:/work/smartVerilog/otherTools/oss-cad-suite/bin:$PATH"
command -v ebmc >/dev/null || { echo "FATAL: ebmc not on PATH"; exit 2; }
d=Benchmark/fmcad2025/$b
wk=/work/wk2/${pol}_${b}
rm -rf "$wk"
t0=$SECONDS
out=$(timeout 3000 ./build/smart "$d/$b.sv" --top "$b" --trace-policy "$pol" \
      --jobs 16 --core-timeout 60 --workdir "$wk" \
      --output /work/out2/${pol}_${b}.sv --keep-work -q 2>&1)
mine=$(( SECONDS - t0 ))
v=$(echo "$out" | grep -oE '^[0-9]+ verified' | grep -oE '^[0-9]+')
[ -z "$v" ] && { echo "$pol $b MINE-FAIL"; exit 0; }
ev=/work/ev2/${pol}_${b}
rm -rf "$ev"; mkdir -p "$ev/runtime/verilog" "$ev/src"
cp smart/evaluater.py "$ev/"; cp -r smart/src/python "$ev/src/"
cp "$wk/invariants.txt" "$ev/" 2>/dev/null
cp "$wk"/runtime/verilog/*.sv "$ev/runtime/verilog/" 2>/dev/null
[ -d "/work/MB/$b/benchmarks" ] || { echo "$pol $b NO-FIXED-MUTANTS"; exit 0; }
cp -r "/work/MB/$b/benchmarks" "$ev/benchmarks"
md=$( cd "$ev" && timeout 3600 python3 evaluater.py "$b" 10 2>/dev/null \
      | grep -oE 'Mutation Detection\(MD\) rate: *[0-9.]+' | grep -oE '[0-9.]+$' )
mut=$(ls "$ev/benchmarks"/mutant_*.sv 2>/dev/null | grep -cv _assertion)
st=$(grep -h '"stage":"fuzz"' "$wk/run-log.jsonl" 2>/dev/null | grep -oE '"states_selected":[0-9]+' | grep -oE '[0-9]+$')
sr=$(grep -h '"stage":"fuzz"' "$wk/run-log.jsonl" 2>/dev/null | grep -oE '"states_random":[0-9]+' | grep -oE '[0-9]+$')
printf "%-8s %-8s %6s %5ss %s %s %s %s\n" "$pol" "$b" "$v" "$mine" "${md:-EVAL-FAIL}" "$mut" "${st:-0}" "${sr:-0}"
