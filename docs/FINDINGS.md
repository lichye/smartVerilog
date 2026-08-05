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

### 1.8 The end-of-run minimiser was declared but never ran

`Config/block_msa_mini.json` differs from `block_msa.json` in exactly one key:

```
"Workflow": { "Minimizer": false -> true }
```

The pipeline only ever read `block_minimizer` (the between-rounds one). Nothing
read `minimizer` / `end_minimizer`, so `block_msa_mini` was a byte-for-byte
re-run of `block_msa`. Any `mini` row produced before 2026-07-26 is that, not a
minimised result.

The cause was a units mismatch left over from WP5: `mus::minimiseAssertions`
works on the `.sl` definitions, while the emitted artefact is a list of Verilog
assertion strings. Joining them needs the mapping each block writes to
`runtime/CompareResult.txt`:

```
(N6 || N11):
(
(define-fun inv ((N1 Bool) ...) Bool (or N6 N11))
)
```

`Pipeline.cpp` now minimises `SygusResult.sl`, matches the surviving
`define-fun` bodies back through that file, and filters the emitted set. An
assertion whose definition cannot be located is **kept** — silently dropping
something the mapping failed to explain would weaken the output without saying
so.

Measured, s27 (`block_msa_mini`): 49 -> 28 assertions, host and container alike.

### 1.9 The run log did not cover the minimiser or the MSA

Both stages only called `say()`, so their cost was invisible in
`run-log.jsonl` — the gap between two `round` records absorbed the minimiser,
the MSA, the block writing and the next round's synthesis, with no way to
separate them. They now emit their own records:

```
{"t":5.346,"stage":"minimiser","round":1,"before":62,"after":49,"secs":0.010833}
{"t":5.366,"stage":"msa","round":1,"pool":87,"variables":139,"timed_out":false,"secs":0.020019}
```

This is what made §3.6 answerable with numbers instead of an opinion.

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

### ~~2.2 s382 and s444 fail in every config~~ — FIXED
**Symptom.** ERR in all four configs; a direct run hangs indefinitely with
`vvp` at 99.8% CPU and the VCD stuck at 8192 bytes.
**Mechanism.** Their clock is `CK` (ISCAS-89 naming), which `guessClock` does
not know, so it is driven as an ordinary random input. These are gate-level
netlists whose flip-flops are feedback loops of primitives; a randomly-toggled
CK can drive them into a state where Icarus Verilog, being event-driven,
oscillates forever at a single timestamp. Seed 42 completed, seed 43 hung —
it depends on the values drawn.
**Note.** The artifact's own sim.py also drove CK randomly ("Test a design
without clock signals"). Verilator, being cycle-based and 2-state, settles
instead of oscillating, which is why this never happened before the rewrite.
**Fix.** Two, independently: the simulator is Verilator again (s382 now mines
213 assertions), and every simulation runs under `--simulation-timeout`
(default 300s) so a non-terminating design fails with a readable message
instead of hanging the run.

### 2.3 `--msa` alone yields less than plain mode
Measured on the smoke matrix: 4 of 6 designs do worse with `--msa` than
without. Mechanism understood — blockified mode skips the wide `Init_*` blocks
of round one, exactly as preAnalyzer.py did — so it starts from a narrower
round. Mixing helps: s27 gives 83 with `--msa --random`, 36 plain, 28 with
`--msa`. Not a defect; a default worth revisiting deliberately.

---

### 2.4 Icarus Verilog was the wrong default (reverted)
WP3 chose Icarus over Verilator for three real advantages: free registers
driven by plain hierarchical assignment, full control of the VCD scope tree,
and no C++ compile per design. What that traded away turned out to cost more:

| | Icarus | Verilator |
|---|---|---|
| semantics | event-driven, 4-state | cycle-based, 2-state |
| x/z in traces | yes, before first assignment | none |
| zero-delay feedback | spins forever (s382, s444) | settles |
| what the paper used | — | this |

The x/z leakage alone produced three of the defects in §1 (1.3, 1.5, 1.6).
Verilator is the default again; `--simulator iverilog` keeps the other
backend, which is still the cheaper one when it works.

Measured after the switch: s27 31 -> 75, s382 hang -> 213, nru_a 13 -> 17,
c17 50 -> 49, axis_fifo 17 -> 18.

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

### 3.6 Minimising before the MSA to speed the MSA up

Already the order: `block_minimizer` runs immediately before `mus::getMus()`
inside the round loop. The question was whether tuning the pair buys speed.
It does not — but the reason is different at each scale, and an earlier version
of this section got it wrong by generalising from s298 alone.

s298 (139 variables), per round: the minimiser removes 20-27% of the
invariants (62->49, 112->82, 138->121, 176->138) and the MSA pool it feeds
shrinks 87 -> 8. Both stages together cost under 0.25s per round. Nothing to
save because nothing is being spent.

c880 (443 variables, 46 rounds, 680 assertions, 855s) says otherwise about the
cost, and the same thing about the conclusion:

| | time | share of run |
|---|---|---|
| MSA total | 94.8s | **11.1%** |
| minimiser total | 40.7s | 4.8% |

MSA is not noise here — it is a ninth of the run, and it grows (0.04s at
round 1, 4.22s at round 46). But the minimiser cannot help: on c880 it removes
**0.3-0.5%** (206->206, 621->620), because the assertion set is already
irredundant. It costs 40.7s to save well under a second.

So: small designs, the minimiser works but the MSA is free; large designs, the
MSA costs real time but the minimiser has nothing left to remove. Neither end
pays. The between-rounds minimiser is off by default as a result (§4.3).

Where the MSA time actually goes, and why it is no longer a bottleneck, is
§3.7 — that turned out to be the whole story of the speedup.

### 3.7 The MSA was never solver-bound — it was term-construction-bound

The pre-refactor MSA (`smart/src/python/minimal_satisfiable_assignment.py` at
d7ca79c) used `from cvc5.pythonic import *` — in-process bindings, not a
subprocess. Same algorithm as ours, same `FORALL` formulation, same
`produce-unsat-cores` / `minimal-unsat-cores` options. Ours is if anything
heavier: a fresh `cvc5::Solver` per `isMus` call where the Python used
push/pop on a shared one.

Head-to-head on one input — `block_msa_mini_c1355`'s final `variables.txt`
(587 variables) and `SygusResult.sl` (1251 assertions), 300s timeout, same
machine:

| | time | underspecified | timed out |
|---|---|---|---|
| C++ (`build/bin/mus_bench`) | **12.82s** | **108** | no |
| Python (d7ca79c) | **310.90s** | 143 | **yes** |

24x, and the Python run returned a *worse* result because it hit the timeout.

Splitting `is_mus` into its construction loop and its `solver.check()`:

```
py_msa  total 310.6s   build 249.3s (80%)   solve 21.7s (7%)
        104 is_mus calls, 127,402 quantified constraints built
```

**The solver needs 21.7 seconds.** The other 289 go into Python building
127k `Implies(t, ForAll(cand, a))` terms node by node through the Z3
compatibility layer. The C++ does the entire job in less time than the Python
spends solving alone.

Three candidate explanations, decided:

| hypothesis | verdict | evidence |
|---|---|---|
| cvc5 version | **ruled out** | the Python side ran cvc5 **1.3.4**, newer than the 1.2.0 we link |
| algorithm | **ruled out** | identical `is_mus`/`ascend_to_boundary`, identical formulation |
| construction overhead | **confirmed** | 80% of the time is in the build loop, 7% in the solver |

This compounds: `ascend_to_boundary` returns the current, *non-minimal* `msa`
when it runs out of time, so a timed-out round hands the next round a larger
pool, which is slower still. That feedback is why MUS reached 70-88% of the
artifact's runtime on the larger designs (§4.2), with the MUS set size barely
moving between rounds (c3540 842->847, s38584 11077->11065).

Reproduce with `build/bin/mus_bench <variables.txt> <SygusResult.sl> [timeout]`.

### 3.8 Positive states: the ceiling, the redundancy, and what "more" costs

Three facts about the constraints a block is given, which together explain why
"sample more of the design" and "make the synthesiser's job harder" looked like
one knob.

**The state ceiling is traces x cycles.** A 10-cycle trace visits at most 10
states, so the shipped 3 x 10 caps at 30 no matter how good the stimulus is.
Whether a design has room is measurable before doing anything about it —
`states_random` in the run log against that product:

| design | random coverage / ceiling | room |
|---|---|---|
| c432 | 30 / 30 | none — already saturated |
| s838 | 5 / 30 | 6x under-sampled |

Searching within the same budget took s838 from 5 to 11 and its verified
assertions from 576 to 913. c432 is unchanged, as predicted.

**40% of the constraints were repeats.** A block sees k variables; two states
that differ anywhere else project to the same k values and produce a
byte-identical constraint. On c880: 108,853 positive constraints across 3,238
blocks, 65,771 distinct. The worst block was 81% duplicates, one constraint
appearing 11 times in 36. Deduplicating is free — a repeat constrains nothing
the first did not — and on s838 cut the emitted set from 41,591 to 19,471 with
the yield unchanged (576 -> 581, inside the run-to-run spread).

So roughly half of the coupling between coverage and difficulty was repetition
rather than information.

**But more DISTINCT positive states is not simply better.** Each one is a
constraint the invariant must satisfy, and they pull three ways at once:

1. *Helps.* A candidate synthesised from narrow traces is often true only of
   the few states seen; EBMC refutes it, CEGIS re-proposes, and the block can
   exhaust its refinement budget for nothing. Richer states move that
   rejection from the expensive check to the cheap one.
2. *Hurts.* Spread widely enough, no boolean relation over the block's k
   variables satisfies all of them: SyGuS returns infeasible and the block
   yields nothing.
3. *Hurts, subtly.* An invariant that must accept more states is a weaker
   claim, and a weaker assertion detects fewer mutants.

Assertion count cannot distinguish these — (1) raises it, (2) lowers it, and
(3) leaves it alone while lowering MD. Only the mutation rate answers, which
is why the policy study measures that and not yield.

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

### 4.1a What the c880 comparison does and does not show
artifact: 71 assertions, 116 of 383 mutants detected, 30.3%.
ours: 236 assertions, 236 of the same 383, 61.6%.

The design files are byte-identical to the artifact's own copies, and the
mutant count matches. More assertions detecting more mutants is the expected
relationship, not a suspicious one; what would be suspicious is twice the
assertions at the same detection rate.

Two things still unaccounted for. The artifact ran 1142 block invocations to
our 563 and got a third of the assertions, so this is not a "we had more
cores" story — the yield per block differs, and the likely cause is §1.6
(counterexamples that are not fully concrete used to be discarded whole).
And the artifact's own mutant set was not preserved, only its count, so
30.3% is a reference point rather than a strictly controlled comparison.

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

### 4.3 The 88-experiment subset run, and the defaults it decided

4 configs x 22 designs, fixed MutationBenchmark mutants, `-j16`, all in one
container on this machine. `block_msa_rand` failed to mine c880 and c1355
(MINE-FAIL at the 2400s cap), so the ranking below is over the 20 designs
where all four finished:

| config | MD mean | MD median | assertions median | total time |
|---|---|---|---|---|
| `block_msa` | 82.06% | 82.39% | 298 | 1956s |
| `block_msa_mini` | 82.02% | **82.39%** | **252** | 1986s |
| `block_msa_rand` | 82.14% | 81.24% | 1968 | 5336s |

Over all 22: `smart` 67.3% / 312s, `block_msa` 81.9% / 4569s,
`block_msa_mini` 81.8% / 4610s.

`block_msa_rand` is dominated: its MD mean is 0.08 points higher (noise), its
median 1.15 points LOWER, it emits 6.6x the assertions, takes 2.7x the time,
and fails outright on the two hardest designs. Not a default, not a fallback.

`mini` ties `msa` on median MD with 15% fewer assertions.

Defaults changed on this evidence:

```
blockified = true      msa = true
minimizer  = true      end_minimizer = true      block_minimizer = false
```

`--no-blockified` gives the one-shot mode back (the parser always supported
`--no-<flag>`; only the help text was missing it).

`block_minimizer = false` rests on a direct c880 A/B rather than the table
above, because every shipped config sets that key to true:

| per-round minimiser | rounds | assertions | MD | minimiser cost |
|---|---|---|---|---|
| on | 48 | 680 | 92.69% | 40.7s |
| **off** | 52 | **733** | **93.73%** | 0 |

More assertions, +1.04 points of MD, and 40.7s back — **on this one design**.

It did not generalise. Measured across all 22 designs (shipped defaults, same
harness, fixed mutants):

| | off vs on |
|---|---|
| ΔMD mean | +0.28pp |
| ΔMD median | **+0.00pp** |
| better / worse / unchanged | 7 / 5 / 10 |
| MD mean | 81.84% -> 82.12% |
| total time | 4610s -> 4655s |

Both are inside the run-to-run spread. Off stays the default because it does
measurable work for no measurable gain, not because it wins — and this is a
reminder of what a single-design A/B is worth.

### 4.4 Against the artifact, controlled

Same machine, same 16 threads (`Config/block_msa*.json` all set
`max_threads: 16`), mutant counts identical to the artifact on all 22 designs:

| config | speedup median | total | ΔMD mean | better/worse |
|---|---|---|---|---|
| `block_msa` | **9.2x** | 24982s -> 4569s (5.5x) | +3.0pp | 17/22 better, 1 worse |
| `block_msa_mini` | 7.8x | 16813s -> 4610s (3.6x) | +2.4pp | 14/22 better, 3 worse |

`Config/smart.json` sets `max_threads: 1` while our harness passed `--jobs 16`,
so the `smart` row is NOT a like-for-like comparison and is excluded.

Where the time went, `block_msa_mini`, 22 designs:

| | artifact | ours |
|---|---|---|
| total | 16813s | 4610s |
| of which MSA/MUS | **11962s (71%)** | **1034s (22%)** |
| residual speedup, MSA excluded | | **1.4x** |

**The speedup is the MSA and almost nothing else** — see §3.7 for why. The rest
of the pipeline was already C++ and had no comparable overhead to remove.

Open: s953 is the one clear regression (98.6% -> 86.2%). c1355/c880/c499 are
the three designs where our non-MSA work is *slower* than the artifact's
(residual 0.5-0.9x); we also emit far more assertions there (c880 680 vs 455),
which is the obvious suspect but is unverified.

### 4.5 The end-of-run minimiser is sound, measured 20 ways

`block_msa` and `block_msa_mini` are the same run up to the final filter (seed
42, same rounds, same mined set), so MD must come out bit-identical: the
minimiser only drops an assertion the kept set implies, and any state
violating that assertion violates something kept. A difference would mean an
unsound minimiser or an inconsistent EBMC.

20 of 22 designs had identical trajectories, and all 20 gave **ΔMD = +0.00**,
with assertion reductions from 4.3% (c499) to 50.0% (c17), median ~19%.

The other 2 (s1488, s838) diverged before the minimiser ran — different round
counts and mined sets — so the precondition fails and they are inconclusive,
not counterexamples. The pipeline is not bit-reproducible under parallelism;
that is understood and accepted.

Oracle script: `tools/experiments/check_oracle.py`.

### 4.6 Trace policy: searching for stimulus does not pay

The question was whether stimulus that visits more of the design produces
better assertions. The search itself works — it does reach more states, within
the same trace count and depth. What fails is the step from there to detection.

22 designs, shipped defaults, fixed MutationBenchmark mutants, one machine:

| | random vs fuzz |
|---|---|
| ΔMD mean | **-0.21pp** |
| ΔMD median | **+0.00pp** |
| better / worse / unchanged | **7 / 7 / 8** |
| correlation, extra states vs ΔMD | **+0.06** |

| design | rand MD | fuzz MD | ΔMD | rand assertions | fuzz assertions | states rand -> fuzz |
|---|---|---|---|---|---|---|
| c1355 | 66.7% | 60.3% | **-6.41** | 1319 | 945 | 30 -> 30 |
| c17 | 100.0% | 100.0% | +0.00 | 19 | 21 | 6 -> 7 |
| c432 | 100.0% | 100.0% | +0.00 | 419 | 411 | 30 -> 30 |
| c499 | 57.4% | 58.9% | **+1.49** | 285 | 249 | 30 -> 30 |
| c880 | 93.7% | 94.5% | +0.78 | 665 | 717 | 30 -> 30 |
| s1488 | 80.2% | 80.8% | +0.60 | 971 | 1039 | 30 -> 30 |
| s27 | 88.2% | 88.2% | +0.00 | 28 | 29 | 11 -> 12 |
| s298 | 78.1% | 78.1% | +0.00 | 148 | 187 | 12 -> 12 |
| s344 | 85.5% | 87.7% | **+2.23** | 221 | 248 | 18 -> 24 |
| s349 | 83.3% | 82.2% | **-1.11** | 200 | 211 | 18 -> 24 |
| s382 | 73.2% | 73.2% | +0.00 | 200 | 242 | 12 -> 12 |
| s386 | 80.5% | 81.1% | +0.59 | 276 | 263 | 25 -> 29 |
| s400 | 74.5% | 74.5% | +0.00 | 205 | 263 | 12 -> 12 |
| s420 | 84.0% | 82.8% | **-1.26** | 287 | 466 | 5 -> 12 |
| s444 | 76.2% | 75.7% | -0.49 | 204 | 281 | 12 -> 12 |
| s510 | 74.7% | 75.1% | +0.45 | 280 | 339 | 14 -> 15 |
| s641 | 94.5% | 93.3% | **-1.24** | 1075 | 1085 | 30 -> 30 |
| s713 | 91.3% | 91.3% | +0.00 | 1333 | 1140 | 30 -> 30 |
| s820 | 70.5% | 69.8% | -0.67 | 716 | 715 | 30 -> 30 |
| s832 | 81.8% | 81.8% | +0.00 | 658 | 875 | 30 -> 30 |
| s838 | 86.1% | 85.7% | -0.41 | 576 | 889 | 5 -> 11 |
| s953 | 86.2% | 87.1% | +0.93 | 631 | 694 | 30 -> 30 |

**s838 is the cell that settles it.** It was the one large design badly
under-sampled at random (5 states of the 30 the budget allows). The search
took it to 11, and assertions from 576 to 889 — **+54%** — and detection went
DOWN 0.41 points.

That is the third effect from §3.8 in its purest form: an invariant that must
hold over more states is a weaker claim, and a weaker assertion catches fewer
mutants. The extra assertions were real, verified, and individually worth
less. Yield would have called this a large win; MD calls it a small loss.

Two supporting observations:

- **Most designs are already at the ceiling.** 3 traces x 10 cycles caps at 30
  states, and c1355/c432/c499/c880/s641/s713/s820/s832/s953/s1488 reach 30/30
  at random. There is nothing for a search to add, and the changed stimulus
  just reshuffles which states are seen — c1355 lost 6.41 points that way,
  the largest single move in the table, with no change in state count.
- **Under-sampling does not predict benefit.** The three designs where the
  search added the most states gave +2.23, -1.11 and -1.26. The correlation
  over all 22 is +0.06.

`trace_policy` stays `random`. The fuzz path is kept, not deleted: the
measurement rules on the current grammar, and a grammar that can express
stronger relations (§3.6, the `bv compare` TODO) would deserve the question
asked again — a wider state set is only a liability while the language cannot
say anything strong about it.

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

### 2.5 The evaluator reports 100% detection when ebmc is missing
`run_fm_on_verilog_file` treats any non-zero exit from ebmc as "the assertion
failed on this mutant", i.e. detected. A missing ebmc exits 127, so EVERY
mutant is counted as detected and the mutation-detection rate comes out at a
clean 100.0% for every design.

That is exactly what happened on the first evaluator run here, and it is a
plausible-looking number: 100% MD reads as a triumph rather than a bug. The
real figure for smart/c880 is 61.6% (236 of 383), confirmed two ways — the
evaluator with ebmc present, and an independent script that injects the
assertions and counts ebmc exit codes directly.

Worth fixing in the evaluator (distinguish a refutation from a failure to
run), and worth remembering as a class: **a metric that cannot fail loudly
will fail quietly, in the flattering direction.**

### 2.6 Mutation rates are only comparable against a FIXED mutant set
`mutation.py` chooses its operators randomly, so two runs on the same design
produce the same NUMBER of mutants with different content — measured on c880:
383 both times, 294 of the 383 different (`nand -> xnor` in one,
`nand -> xor` in the other). `setupMutants()` generates them whenever
`benchmarks/` is empty, which makes an unattended evaluator run silently
self-referential.

The mutants must come from https://github.com/lichye/MutationBenchmark.git,
which covers all 22 subset designs. The old run.py copied them in; the
rewritten one dropped that and has to get it back.

For what it is worth here, the two sets turned out to be equally hard —
c880 scored 236/383 either way, to the digit — so this did not distort the
c880 comparison. That is luck, not a reason to skip it.

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

### 5b. Version, and reading a run log safely

The version lives in one place, `smart/src/Version.h`, and reaches everything
that could outlive the checkout: `smart --version`, `--check-env`, the
generated `<top>_assertion.sv` header, and the `start` record of every run log.

```
{"run":3588951,"t":0.000,"stage":"start","version":"1.0.0","top":"s27",...}
```

Two fields exist because of mistakes worth not repeating:

- **`run`** is the writer's pid. `RunLog::record` reopens the file in append
  mode per record, so a run whose workdir is deleted underneath it will
  RECREATE the file and keep writing. Two runs' records then interleave in one
  log with two independent `t` timelines and nothing to tell them apart.
  Observed. **Group by `run` before reading any timing out of a log.**
- **`mode`** on the `check` record says how the assertions were proved:

  ```
  {"stage":"check","mode":"bounded","bound":10,...}
  {"stage":"check","mode":"k-induction","bound":null,...}
  ```

  A bounded run emits properties that hold to `bound` only. Sound to report,
  but they are NOT invariants, and assuming them inside someone else's proof
  is unsound. `invariants.txt` is named the same either way, so the log is the
  only place that distinguishes them. (The file's line format is depended on
  by `evaluater.py`, so a header comment there would break it.)

Also recorded per round now: `minimiser` (before/after/secs) and `msa`
(pool/variables/timed_out/secs). §3.6 and §3.7 exist because of those.

### 5c. Supplying invariants to a stuck proof

The natural use of this tool beyond mutation scores: when a design will not
verify, generate proved lemmas and add them. It works, with one hard
precondition — **`--unbounded`**, which switches both the block check and the
final check to k-induction (`Options.cpp:538`, `AssertionWriter.cpp:75`).

The verdict mapping is conservative: only EBMC exit 0 counts as verified, so an
inconclusive k-induction is dropped, never promoted.

Measured cost of the stronger check, s27 and s298: none. Same mined count
(30, 113), 100% survived k-induction, same runtime (5s, 6s). Negative control
on s298's first invariant `(! (G107 == G108))`:

```
true.sv   --k-induction  exit 0    UNSAT: inductive proof successful  PROVED
false.sv  --k-induction  exit 10   REFUTED
```

Not yet measured on the large combinational designs, where k-induction is
likelier to come back inconclusive. Two small sequential circuits do not
generalise.

### 5d. Mining inside a submodule — what works and what is untested

Works, end to end, on real hierarchical designs:

```
smart i2c_master_axil.sv --module i2c_master_inst
  mining instance i2c_master_inst, writing into module i2c_master
  120 candidate variables
  40 verified assertions -> ./i2c_master_assertion.sv
```

Four things had to come apart or be added:

1. `--module` names a **VCD scope**, i.e. an *instance*. The property goes
   into a module *definition*, resolved through `verilator --xml-only`'s cell
   list. Verilator specialises parameterised modules
   (`axis_fifo__D20_DBc_K0_L0_U0`); the suffix is stripped so the property
   lands in `axis_fifo`, constraining every specialisation. One that holds
   only for the parameterisation that was mined gets refuted — observed on
   i2c_master_axil's three axis_fifo instances.
2. The property is written into whichever **file** declares that module, not
   the top's file. Everything else is handed to EBMC untouched, which was
   already the structure; only the choice of file was wrong.
3. The **output** is that file, named after the module, so a top-named file
   that does not contain the assertions cannot be produced.
4. **Free registers inside instances are now driven.** The frontend only
   reports the top's, and the harness had no instance path, so a submodule's
   `(* anyseq *)` sat at zero for the whole trace — not free, and silently
   narrowing every constraint drawn from it. Measured on a two-level design:
   0 for ten cycles before, 7 distinct values after.

**Untested, and the likely first failures:**

- **Leaf-name collisions.** `Trace::getAllSignals(moduleName)` matches a
  single scope name. Two instances with the same leaf name in different parts
  of the hierarchy have not been tried; `--module fifo` where three `fifo`
  instances exist is undefined behaviour today, not a diagnosed error.
- **Depth beyond one level.** `cmd_fifo_inst` sits inside a generate block and
  resolved correctly, but nothing deeper has been run.
- **`--module` given a module name rather than an instance** fails with "no
  candidate variables", which is true but unhelpful. Mapping a type to its
  instances (or naming the ambiguity) is not done.
- **The fuzz policy's state vector is still top-only** — `enumerateStateSignals`
  does not follow into instances, so `--trace-policy fuzz --module X` measures
  the wrong states. Harmless while fuzz is off by default.
- **No mutation measurement.** Every number above is "it ran and EBMC proved
  them". Whether submodule-scope assertions are *worth* anything by MD is
  unmeasured, and MD is the metric.

### 5e. Hierarchical mining at the top — flattening without flattening

The intent behind multi-module support, stated 2026-08-05: mine at the **top**
of a hierarchical design with submodule internals visible as candidates, so
nobody hand-flattens a design again. `--module` (mining *inside* a submodule,
§5d) is the complementary mode and is unchanged.

**Feasibility was measured before anything was built.** The in-tree EBMC 5.6
accepts SVA in the top referencing submodule signals by hierarchical name —
depth 1 (`U0.count`) and depth 2 (`U0.V0.lcount`), PROVED and REFUTED with a
real counterexample (not vacuous), bounded and k-induction. That gate opened
Stage 2.

**What shipped** (`29729b6`..`abf6950`, implemented by an opus agent, reviewed
independently by a sonnet agent — verdict: approve, zero defects):

- `Trace::scopeNaming()` resolves each VCD scope to `(module, instancePath)`
  once per trace; a descendant signal's candidate name is `U0.count`. Scopes
  outside the mined subtree (Verilator's `TOP` wrapper) keep flat naming and
  stay out.
- Option `hierarchical`, default **on**: flat designs have no sub-scopes, so
  behaviour is identical by construction (verified: c17 both ways, same 19
  assertions up to one commuted `||`). `--no-hierarchical` is byte-for-byte
  the old path.
- s27 at top: 41 candidates (21 from sub-scopes), **69 verified vs 28**
  baseline, with cross-instance assertions like
  `(((DFF_0.M == G0) && DFF_1.M) |-> DFF_2.M)`.

**No mangling exists, on purpose.** SMT-LIB simple symbols admit `.`;
cvc5 1.2.0 round-trips `U0.q` through `synth-fun` and back out of
`define-fun`, and both s-expression tokenisers split on parens/whitespace
only. The dotted name is carried end-to-end — variables file, block subsets,
MSA, emitted Verilog — because a mangle/unmangle pair would have had to be
threaded through `mus::getMus`'s two inputs, which read from opposite sides.
The reviewer hunted specifically for other consumers that split on `.` and
found none touching signal names.

**Two pre-existing defects surfaced and were fixed on the way:**

- `smart.out` had failed to link since `e31e679` and nobody noticed — no
  ctest suite builds it (`29729b6`). There are now 5 suites; the new `trace`
  suite pins exact candidate sets for flat/hierarchical/`--module` modes.
- `VCDScope::parent` was never initialised for root scopes; `scopeNaming()`
  is the first real consumer of parent links, so the fix (`835d9f2`) is
  load-bearing, not cleanup.

**Found, not fixed:** in-process cvc5 segfaults (exit 139) when a block holds
>=4 same-width bitvector variables and the 5s tlimit fires. Reproduced on a
flat design, so pre-existing and unrelated to hierarchy — but it silently
turns blocks into `killed` at ~0.1s. Worth its own investigation.

**Measured (2026-08-05, container `smart-test` at `dd5eb20`, fixed mutants,
four ISCAS89 designs, A = default hierarchical / B = `--no-hierarchical`):**

| design | mutants | MD% A | MD% B | Δpp raw | Δpp adjusted | assertions A/B |
|---|---|---|---|---|---|---|
| s27  | 17  | 64.71 | 82.35 | -17.65 | -2.88 | 68/28 |
| s298 | 137 | 67.15 | 77.37 | -10.22 | -2.53 | 288/148 |
| s344 | 179 | 77.65 | 84.92 | -7.26  | -0.12 | 463/221 |
| s349 | 180 | 78.89 | 82.78 | -3.89  | +3.35 | 455/200 |

Raw mean -9.75pp — hierarchical loses on every design. But most of the gap is
a **scoring artifact**: ISCAS89's DFF-removal operator deletes the very
instance the dotted assertions name (`DFF_0.Q`), so EBMC cannot elaborate the
mutant and `evaluater.py` counts it as *undetected* (its warning claims
tool-errors are excluded from the rate; the arithmetic keeps them in the
denominator — a real evaluater defect, now on record). Excluding those from
both sides: mean -0.55pp, median -1.32pp — **a wash trending slightly
negative**, not the win the 1.6-2.4x assertion counts suggest.

Two readings, both honest:

- On ISCAS89 the "hierarchy" is alias wrappers — dff ports mirror top nets —
  so sub-scope candidates are near-duplicates that dilute blocks and double
  the assertion count for no detection. 2-5x mining time for -0.55pp is a
  cost, not a wash.
- The feature's actual target — designs where top-only mining sees nearly
  nothing (the two-level fixture: 3 candidates, 0 assertions without it) —
  has no B baseline to lose to. There, hierarchical is not an optimisation
  but the difference between mining and not mining.

Whether elaboration failure on an instance-deletion mutant should count as
*detection* (the assertion names structure the mutation removed) is a
judgment call not taken here; counting it as detection would flip the sign.

Also surfaced: `Benchmark/user/tiny_and/` (the svmodule ctest fixture) was
never tracked by git, so a fresh checkout fails 1/5 suites. Fixed by tracking
it. The fuzz state vector is top-only here too (same limitation as §5d).

## 6. Next

Items 1-4 of the previous list are done (§2.1, §2.2 fixed; the subset ran and
was evaluated against the artifact, §4.3-4.5). Item 5 is cancelled: the
hw-cbmc patch stays local, upstream is out of scope by decision.

What is left is all optional — nothing here blocks the 1.0:

1. **s953** is the one design where we score clearly below the artifact
   (98.6% -> 86.2%). The only unexplained regression in the table.
2. **The shipped default is not the config that was benchmarked.** The 88-run
   used `Block_minimizer: true`; we ship `false` on the strength of one c880
   A/B (§4.3). 22 designs would settle it.
3. **k-induction retention on large designs** (§5c). Two small sequential
   circuits gave 100%; that does not generalise, and the invariant-supplier
   use depends on it.
4. **c1355/c880/c499 residual slowdown** (§4.4). Analytical curiosity — the MSA
   always runs, so "the pipeline without the MSA" is not a real configuration.
