# Findings log

Running record of defects, measurements and dead ends from the C++ rewrite and
the benchmarking that followed. The plan documents say what we set out to do;
this says what turned out to be true.

Rules for entries: state the symptom, the mechanism, the evidence, and the
status. A number without the command that produced it is not evidence. A dead
end is worth as much as a fix — it stops the next person repeating it.

---

## 1. Defects found and fixed

### 1.1 hw-cbmc drops single `(* attr *)` instances
**Symptom.** `(* anyseq *) reg x;` parses to an empty attribute list, so free
registers are invisible to the frontend.
**Mechanism.** In `src/verilog/parser.y`, the base case of `attr_spec_list` is
`attr_spec { init($$); }` — it allocates a fresh, empty list and never moves
`$1` into it. With two or more attributes only the trailing ones survive.
**Evidence.** Present in `ebmc-5.6`, `ebmc-6.0` and `main` (1a4ffdb).
`ebmc --show-parse` on a file with `(* anyseq *)` shows zero occurrences of
"attribute".
**Status.** Patched locally (`third_party/patches/`, applied at build time).
NOT yet sent upstream — one line, and upstream would fix it for everyone.

### 1.2 VCD parser segfaults on an empty `$dumpall`
**Symptom.** The trace loader crashes on any Icarus Verilog VCD.
**Mechanism.** `$dumpall` switches the flex scanner to `IN_VAL_CHANGES`, which
had no `$end` rule; iverilog writes an empty `$dumpall $end` at time 0, so the
`$end` fell through to flex's default rule.
**Evidence.** `vcd_signals` on any iverilog VCD, before the fix.
**Status.** Fixed in WP3 (`080433a`). Verilator never writes an empty dump
section, which is why it had never fired.

### 1.3 VCD values are not widened to the declared width
**Symptom.** An 8-bit signal reaches SyGuS as a Bool.
**Mechanism.** VCD value changes may omit leading bits (IEEE 1364 §18.2.1) and
iverilog omits them; the parser kept the literal bits, so `b0` on an 8-bit
signal became a 1-bit value and printed as `false` instead of `(_ bv0 8)`.
cvc5 then rejects the whole problem.
**Evidence.** nru_a 0 -> 13 assertions, axis_fifo 3 -> 17.
**Status.** Fixed in WP4 (`6a5ed60`). Verilator writes full-width values.

### 1.4 LTL constraints check one of the two states they print
**Symptom.** LTL synthesis contributes nothing; every LTL block dies with
`Symbol 'unknown_Bool' not declared as a variable`.
**Mechanism.** `createLTLConstraint` prints two states per constraint (index
and index+latency for a positive example) but only checked one for
definedness — the second check was in the source, commented out. An undefined
value prints as the placeholder token, which is not valid SyGuS.
**Evidence.** s27 with `ltl_depth: 1`: 36 -> 90 assertions, latency 1 having
contributed exactly 0 before. All 90 proved by an independent ebmc run.
**Status.** Fixed (`d2c64de`).

### 1.5 `isUndefined()` did not recognise Z
**Symptom.** Blocks die with the same placeholder-token error, outside LTL.
**Mechanism.** `toSyGusString()` prints x AND z as `unknown_Bool` /
`unknown_bits`, but `isUndefined()` only tested for x, so a z value passed the
check and its placeholder reached the file.
**Evidence.** 11 of 398 blocks on plena_data_flow_control_flat.
**Status.** Fixed (`650615d`).

### 1.6 Counterexample constraints were discarded wholesale
**Symptom.** CEGIS refinement re-proposes the same refuted candidate every
round.
**Mechanism.** EBMC leaves a signal X in a counterexample when the violation
does not depend on it. Any X made the constraint builder comment out the whole
constraint, discarding what the other signals in that state say.
**Evidence.** 294 of 398 blocks on plena, 0 of 86 on c17 (whose signals are
all one bit, so its counterexamples are always concrete).
**Status.** Fixed (`e2e8e0e`): undefined positions are filled with a concrete
zero. Sound — a state reachable for every value of a signal is reachable with
zero — and it rules the refuted candidate out, since the violation holds for
every value. Blocks reaching a candidate went 62 -> 113.

### 1.7 Build and packaging defects (clean-container findings)
Each of these was invisible on the development host and surfaced immediately
in a clean `ubuntu:22.04`:

| Symptom | Fix |
|---|---|
| `cmake --build build` produced everything except the tool | `smart` was EXCLUDE_FROM_ALL (`80842f3`) |
| `cd build && ./smart` -> "Is a directory" | CMake mirrored `smart/` into `build/smart/`; subdirectory build tree renamed (`a800800`) |
| cvc5 build fails deep in cmake with "Could not create Python virtual environment" | build with `--no-pyvenv` (`1b97e86`) |
| After install.sh, the binary cannot find the tools install.sh just built | binary appends its own checkout's tool directories to PATH (`dd849d8`) |
| Missing tool reported as a pipeline failure with a shell error | reported as an environment error, exit 2, naming the tool (`8d1e9dd`) |

---

## 2. Open defects

### ~~2.1 A block escaped its deadline by four hours~~ — FIXED
**Symptom.** During the subset benchmark (`--core-timeout 60`), a block
process was found alive after 4h45m:
```
1521878  04:45:16  build/smart --block /work/wk/block_msa_rand_c1355 ... 6918 ...
```
**Why it matters.** BlockRunner's deadline is the mechanism that replaced
`timeout 100 ./smart.out`; WP4 claims it kills the block's whole process
group. It demonstrably did not here. The orphan also burned CPU for the rest
of the benchmark, so **every timing in §4 is contaminated**, and any block that
should have been killed but was not may have contributed assertions it should
not have.
**Cause.** Not the deadline at all — the deadline works. The subset script ran
each experiment under `timeout 1800`, which killed the PARENT `smart` when a
slow config hit the cap. Its blocks were re-parented to init and nothing was
left to kill them. `block_msa_rand` on c499/c880/c1355 hit that cap, which is
exactly where the orphan came from.
**Fix.** Blocks set `PR_SET_PDEATHSIG` so the kernel kills them when their
parent dies, plus a `getppid() == 1` check to close the race where the parent
is already gone.
**Evidence.** 8 blocks running, parent killed, 0 alive four seconds later.
Separately, with `--core-timeout 1` on axis_fifo: 188 blocks, 188 killed, 0
genuine escapes.
**Caveat found while measuring.** The first version of the escape check cried
wolf on all 188 — a killed block's own children (cvc5, ebmc) linger as zombies
until init reaps them, and a zombie still answers `kill(-pgid, 0)`. The check
now waits before concluding.
**Status.** FIXED. The timings in §4.1 are still contaminated and that run
should be repeated.

### 2.2 s382 and s444 fail in every config
**Symptom.** ERR in all four configs of the subset run; a direct run has been
going >12 minutes with no output, so it hangs rather than errors.
**Status.** OPEN, under diagnosis. Note these are ISCAS-89 designs the
artifact ran successfully, so this is a regression, not a design limit.

### 2.3 `--msa` alone yields less than plain mode
Measured on the smoke matrix: 4 of 6 designs do worse with `--msa` than
without. Mechanism understood — blockified mode skips the wide `Init_*` blocks
of round one, exactly as preAnalyzer.py did — so it starts from a narrower
round. Mixing helps: s27 gives 83 with `--msa --random`, 36 plain, 28 with
`--msa`. Not a defect; a default worth revisiting deliberately.

---

## 3. Dead ends (measured, do not repeat)

### 3.1 Universally quantifying undefined counterexample positions
Says exactly what is known: `(constraint (forall ((_u0 (_ BitVec 3)))
(= (inv ... _u0 ...) true)))`. cvc5 accepts it. But the problems get so much
harder that **blocks returning any candidate at all fell from 139 to 9** on
plena. Concrete zero-filling (§1.6) says less and wins.

### 3.2 k-induction for the reachability check
The rigorous answer to "is this random state reachable", and it is what a
negative example needs. But what k-induction can prove unreachable is a strict
SUBSET of what a bounded check accepts — so it only ever removes negative
examples. Measured on plena: bound 10 -> 0 assertions, bound 40 -> 2,
k-induction -> 0. Kept as `reachability_bound: -1`, not the default.

Worth being precise about the stake: a wrong negative example costs search
time, not soundness. The final check proves every emitted assertion against
the unmodified design regardless.

### 3.3 Deepening the reachability bound alone
Hypothesis was that shallow reachability was plena's bottleneck. Attribution
run: reachability 40 with the candidate check left at 10 found 0, exactly as
reachability 10 did. It was `block_bound: 40` — the CANDIDATE check — that
found 2. The hypothesis was wrong and the default went back to 10 (`f5810ce`).

### 3.4 More simulation cycles for plena
40 cycles instead of 10: still 0 assertions.

### 3.5 plena_data_flow_control_flat is at the method's limit
The pre-rewrite pipeline (`d7ca79c`, cocotb + Verilator + Python) finds **1**
assertion from 418 blocks on this design. The new pipeline finds 0-2 depending
on configuration. The 0 is not a regression; the design is simply near the
edge of what this method extracts.

---

## 4. Benchmark data

### 4.1 Subset run (container, 2026-07-26)
4 configs x 22 designs, `--jobs 24 --core-timeout 60`, in a clean
`ubuntu:22.04` built from scratch by install.sh. Raw log: `/work/sub.log` in
the `smart-test` container.

77 of 88 succeeded. **`mined == verified` in every one of the 77** — nothing
that passed a block's own check was rejected by the final gate.

11 failed: s382 and s444 in all four configs (§2.2), plus c499/c880/c1355 in
`block_msa_rand` (likely the 1800s cap — that config took 776s on c432).

**Timings in that log are not trustworthy** — see §2.1.

### 4.2 Against the paper artifact
`artifact/PrecomputedRawData/` has all 88 subset experiments with verified
counts AND mutation-detection rates. Extracted to compare (77 comparable):

- ours higher in 32, lower in 45, never equal
- the direction is systematic per config, not noise:
  - `block_msa_rand`: ours much higher (s838 2102 vs 308, c432 1919 vs 68)
  - `block_msa` / `block_msa_mini`: ours generally lower (s1488 133 vs 1350)
  - `smart`: higher on combinational (c880 243 vs 71), lower on sequential

**Assertion count is not the metric.** The artifact's MD rates vary hugely at
similar counts (smart/c880 71 assertions at 30.3% MD; block_msa/c880 474 at
83.0%), so a count comparison says nothing on its own. The comparison that
matters is MD rate at equal or better coverage, and that needs the evaluator
run — **not yet done**, deliberately, because §2.1 contaminates the inputs.

---

## 5. Environment facts worth not rediscovering

- The VCD scanner is flex-generated **without** `%option reentrant` and keeps
  file-scope state. Blocks must not share a process. (`Trace` does hold a
  mutex around parsing, added before this rewrite.)
- CBMC reports internal invariant violations by **aborting the process**.
  `cbmc_invariants_should_throwt` turns them into exceptions; any code linking
  CBMC libraries should set it.
- oss-cad-suite bundles cvc5 1.0.1-dev and puts itself first on PATH. Ours
  must win — solver version changes what gets synthesised.
- cvc5's libpoly still declares `cmake_minimum_required(VERSION <3.5)`, which
  CMake 4 refuses. `CMAKE_POLICY_VERSION_MINIMUM=3.5` gets past it.
- The static cvc5 build's libgmp is not copied by `make install`.
- Legacy MSA code needs Python >= 3.10 (`match` statement). Only relevant if
  running the pre-rewrite pipeline for comparison.

---

## 5a. Run log

Every run writes `workdir/run-log.jsonl`, one JSON object per line, flushed as
it goes so it survives the run being killed:

```
{"t":0.000,"stage":"start","top":"c17","jobs":8,"core_timeout":100}
{"t":0.035,"stage":"simulate","traces":3,"cycles":10}
{"t":0.037,"stage":"pre-analysis","variables":11,"k":4,"blocks":86}
{"t":0.9,"block":"42","round":1,"pid":1234,"status":"verified","exit":0,"seconds":0.31,"assertion":"..."}
{"t":1.044,"stage":"round","round":1,"blocks":86,"verified":81,"timed_out":0,"failed":0,"new":50,"total":50}
{"t":1.186,"stage":"check","checked":50,"verified":50,"refuted":0,"timed_out":0,"errors":0}
{"t":1.187,"stage":"done","mined":50,"verified":50,"rounds":1}
```

Per-block records carry `killed` when the deadline expired and `survived_kill`
when the process group was still there afterwards — the field that would have
caught §2.1 the day it happened instead of a week later. The `check` record
separates refuted from timed-out from errored, which the summary line cannot.

## 6. Next

1. Diagnose and fix §2.1 (deadline escape) — everything downstream depends on
   it.
2. Diagnose §2.2 (s382/s444 hang).
3. Re-run the subset on a quiet machine.
4. Run the evaluator and compare MD rates with the artifact.
5. Send the hw-cbmc patch (§1.1) upstream.
