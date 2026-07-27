#!/bin/bash
# Subset benchmark + mutation evaluation, one line per experiment.
CONFIGS=(smart block_msa block_msa_rand block_msa_mini)
BENCHMARKS=(c17 c432 c499 c880 c1355 s27 s298 s344 s349 s382 s386 s400
            s420 s444 s510 s641 s713 s820 s832 s838 s953 s1488)
mkdir -p /work/out /work/wk /work/ev
printf '%-16s %-8s %6s %6s %8s %6s\n' CONFIG DESIGN ASSERT TIME MD% MUTANTS
for cfg in "${CONFIGS[@]}"; do
  for b in "${BENCHMARKS[@]}"; do
    read -r c d v t md mu <<< "$(/work/eval_one.sh "$cfg" "$b" 2400 3600)"
    printf '%-16s %-8s %6s %5ss %8s %6s\n' "$c" "$d" "$v" "$t" "$md" "$mu"
  done
done
