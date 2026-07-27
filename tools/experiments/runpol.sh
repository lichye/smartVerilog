#!/bin/bash
BENCH=(c17 c432 c499 c880 c1355 s27 s298 s344 s349 s382 s386 s400 s420 s444 s510 s641 s713 s820 s832 s838 s953 s1488)
echo "POLICY   DESIGN   ASSERT  TIME     MD% MUTANTS STATES RANDST"
for pol in random fuzz; do
  for b in "${BENCH[@]}"; do /work/eval_pol.sh "$pol" "$b"; done
done
