# SMART 1.0 — what changed and what it cost

A work record, not a paper. What was built, what it measured, what turned out
to be wrong, and what is still open. Numbers first throughout; anything stated
without one is flagged as such.

Setup: one machine, 4 configs × 22 designs (88 experiments), fixed
MutationBenchmark mutant set, `-j16`. Mutant counts match the artifact's on
all 22 designs. The artifact comparison is like-for-like — same machine, same
16 threads (`Config/block_msa*.json` all set `max_threads: 16`).

Current proof-strength policy (changed 2026-08-09): the final output gate uses
EBMC k-induction by default, while checks inside synthesis blocks remain
bounded for mining speed. The measurements below predate that default unless
they explicitly say k-induction; `--no-final-unbounded` restores the historical
bounded final check for reproduction only.

---

## 1. Input quality — what reaches the synthesiser

Five defects, all of which silently threw away or corrupted the constraints
handed to SyGuS.

| defect | measured effect |
|---|---|
| Counterexample constraints discarded if they held a **single** X | 294 of 398 blocks on plena. Blocks reaching a candidate **62 → 113** |
| `isUndefined()` did not recognise **Z** | 11 of 398 blocks on plena |
| VCD values not left-extended to the declared width | nru_a **0 → 13** assertions, axis_fifo **3 → 17** |
| LTL constraints checked one of the two states they printed | s27 **36 → 90** assertions |
| VCD parser segfaulted on an empty `$dumpall` | crash → runs complete |

The first is the largest. EBMC leaves a signal X in a counterexample when the
violation does not depend on it; any X made the constraint builder comment out
the **whole** constraint, discarding what every other signal in that state
said. CEGIS then re-proposed the same refuted candidate every round.

Undefined positions are now filled with a concrete zero. Sound — a state
reachable for every value of a signal is reachable with zero — and it rules
the refuted candidate out, because the violation holds for every value.

This is the most likely explanation for the higher per-block yield: on c880
with an identical variable count (V=443) and an identical Init/thread split,
we get roughly 3× the artifact's assertions per block. **Not yet isolated by
a controlled toggle.**

---

## 2. All C++ — and the speedup is one component

| config | speedup (median) | total time | ΔMD (mean) | better / worse |
|---|---|---|---|---|
| `block_msa` (CAV) | **9.2×** | 24982s → 4569s (5.5×) | **+3.0 pp** | **17 / 22 better, 1 worse** |
| `block_msa_mini` (default) | 7.8× | 16813s → 4610s (3.6×) | +2.4 pp | 14 / 22 better, 3 worse |

Not a speed-for-quality trade — detection rose on the large majority while
wall clock fell.

### Where the time went

`block_msa_mini`, all 22 designs:

| | artifact | ours |
|---|---|---|
| total | 16813s | 4610s |
| of which MSA / MUS | **11962s (71%)** | **1034s (22%)** |
| everything else | 4851s | 3576s |
| **residual speedup, MSA excluded** | | **1.4×** |

**The MSA is essentially the entire speedup.** The rest of the pipeline was
already C++ before and had no comparable overhead to remove.

### The MSA was never solver-bound

The old MSA used `cvc5.pythonic` — in-process bindings, same algorithm, same
`FORALL` formulation, same unsat-core options. Ours is if anything heavier (a
fresh solver per check where the Python used push/pop).

Same input to both — c1355's final `variables.txt` (587 variables) and
`SygusResult.sl` (1251 assertions), 300s timeout, same machine:

| | time | underspecified | timed out |
|---|---|---|---|
| C++ | **12.82s** | **108** | no |
| Python | **310.90s** | 143 | **yes** |

Splitting the Python check into construction and solving:

```
total 310.6s    build 249.3s (80%)    solve 21.7s (7%)
104 checks, 127,402 quantified constraints built
```

**The solver needs 21.7 seconds.** The other 289 go into Python building 127k
`Implies(t, ForAll(cand, a))` terms through the Z3 compatibility layer. The
C++ finishes everything in less time than the Python spends solving alone.

| hypothesis | verdict | evidence |
|---|---|---|
| cvc5 version | ruled out | Python ran cvc5 **1.3.4**, newer than the 1.2.0 we link |
| algorithm | ruled out | identical search, formulation, options |
| **term construction** | **confirmed** | 80% build, 7% solve |

It compounds: on timeout the search returns a *non-minimal* result, so the
next round starts from a larger pool. That is why MUS reached **70–88%** of
the artifact's runtime on larger designs while the set size barely moved
(c3540 842→847, s38584 11077→11065).

Reproducible: `build/bin/mus_bench <variables.txt> <SygusResult.sl> [timeout]`.

---

## 3. Negative states and reachability — two negative results

Deciding whether a random negative state is *reachable* is a different
question from checking a candidate, so it now has its own knob
(`reachability_bound`, default 10, `-1` selects k-induction). Both attempts to
exploit it failed, and both are worth reporting as negatives.

| experiment | design | result |
|---|---|---|
| reachability bound 10 (baseline) | plena_data_flow_control_flat | 0 assertions |
| **reachability bound 40**, candidate check left at 10 | plena | **0 assertions** — no change |
| candidate check (`block_bound`) 40 | plena | **2 assertions** |
| k-induction for reachability (`reachability_bound: -1`) | plena | 0 assertions |

**Reachability depth is not the bottleneck** — the candidate check is. My
hypothesis was the opposite; the attribution run refuted it, and the default
was reverted to 10.

k-induction is the rigorous answer to "is this state reachable", but what it
proves unreachable is a strict *subset* of what a bounded check rejects, and
an inconclusive answer loses the negative example entirely. Kept as an option,
not the default.

> I have no positive result to report here. If the change you have in mind is
> something other than the reachability knob, tell me and I will pull the
> numbers.

---

## 4. Defaults, chosen by measurement

Ranking over the 20 designs where all four configurations finished:

| config | MD mean | MD median | assertions (median) | total time |
|---|---|---|---|---|
| `block_msa` | 82.06% | 82.39% | 298 | 1956s |
| `block_msa_mini` | 82.02% | **82.39%** | **252** | 1986s |
| `block_msa_rand` | 82.14% | 81.24% | 1968 | 5336s |

All 22 designs: one-shot 67.3% / 312s, `block_msa` 81.9% / 4569s,
`block_msa_mini` 81.8% / 4610s.

- **`block_msa_rand` is dominated**: MD mean +0.08 pp (noise), median **−1.15
  pp**, **6.6× the assertions**, 2.7× the time, and it failed outright on
  c880 and c1355, where MSA scored 92.7% and 67.4%.
- **`mini` ties `msa` on median MD with 15% fewer assertions.**

```
blockified = true   msa = true
minimizer  = true   end_minimizer = true   block_minimizer = false
```

Iterating costs ~15× the wall clock and buys **14.6 pp** (67.3% → 81.9%).

Per-round minimiser **off**, from a c880 A/B:

| per-round minimiser | rounds | assertions | MD | its own cost |
|---|---|---|---|---|
| on | 48 | 680 | 92.69% | 40.7s |
| **off** | 52 | **733** | **93.73%** | 0 |

On large designs it removes only 0.3–0.5% (the set is already irredundant)
while costing 4.8% of the run. **Caveat: one design.** The 88-run used
`Block_minimizer: true`.

---

## 5. A configuration that never did anything — and the oracle it gave us

`block_msa_mini.json` differs from `block_msa.json` in one key,
`Workflow.Minimizer`, and **nothing read it**. Every "mini" result before this
release — ours and the artifact's — was a re-run of the plain configuration.

Now wired up: **4.3% to 50.0% fewer assertions**, median ≈19%.

Because the two configurations are the same run up to the final filter (fixed
seed, same rounds, same mined set), and the minimiser only drops an assertion
the kept set implies, **the detection rate must come out bit-identical**. Any
difference means an unsound minimiser or an inconsistent EBMC.

**20 of 22 designs had matching trajectories; all 20 gave ΔMD = +0.00** while
shedding up to half their assertions. The other two diverged before the
minimiser ran (the pipeline is not bit-reproducible under parallelism) — their
precondition fails, so they are inconclusive rather than counterexamples.

One more that changed results: the evaluator counted a **missing EBMC** as
detection. c880 reported 100%; the true figure is 61.6%.

---

## 6. Stimulus search — built, measured, not adopted

The premise: uniform random draws revisit a small corner of a control FSM, so
the states that reach the synthesiser are a poor sample. Reaching the
interesting states usually needs a RUN of cycles holding one input steady —
a load sequence, a counter walking to its bound — and independent per-cycle
draws produce such a run with probability falling off exponentially in its
length. On plena, eight consecutive cycles from a 3-in-16 subset is ~1.5e-6
per starting position.

Built: the generated harness takes an external stimulus file, can skip the
waveform (a search round runs thousands of times and throws it away), and
reports the states it visited. The state vector comes from
`verilator --xml-only`, which elaborates without compiling. A corpus-based
search mutates sequences — the operator that matters pins one signal across a
span of cycles — and greedy set cover picks the final traces. Same trace
count, same depth: only the content changes.

It works. On s838 it took state coverage from 5 to 11 and assertions from 576
to 889, **+54%**.

**Detection went down 0.41 points.**

Over 22 designs:

| | random vs fuzz |
|---|---|
| ΔMD mean | **-0.21pp** |
| ΔMD median | **+0.00pp** |
| better / worse / unchanged | **7 / 7 / 8** |
| correlation, extra states vs ΔMD | **+0.06** |

An invariant that must hold over more states is a weaker claim, and a weaker
assertion catches fewer mutants. The extra assertions were real and verified
and individually worth less. **Scored by assertion count this was a large win.**

Two things the table also shows:

- Most designs are already at the ceiling. 3 traces x 10 cycles caps at 30
  states and ten designs reach 30/30 at random, so a search has nothing to
  add and merely reshuffles which states are seen — c1355 lost 6.41 points
  that way with no change in state count.
- Under-sampling does not predict benefit: the three designs where the search
  added the most states gave +2.23, -1.11, -1.26.

`--trace-policy fuzz` stays in the tool, off by default. The measurement rules
on the current boolean grammar; a grammar that could express stronger
relations over a wider state set is the case it cannot rule on.

---

## 7. Two things worth having regardless

**Duplicate constraints.** A block sees k variables, so two sampled states
differing anywhere else produce a byte-identical constraint. On c880: 108,853
positive constraints across 3,238 blocks, 65,771 distinct — **40% pure
repetition**, worst block 81%, one constraint appearing 11 times in 36.
Dropping them loses nothing; on s838 the emitted set went 41,591 -> 19,471
with yield unchanged.

**Why a block found nothing.** The log said `no-assertion` for three outcomes
that want opposite responses. Decoded from exit codes and now labelled:

| | s838 | c880 |
|---|---|---|
| verified | 55.7% | 20.2% |
| **SyGuS infeasible** | **43.8%** | **46.8%** |
| candidate refuted | 0.5% | 28.7% |
| killed on time | 0% | 4.2% |

This contradicts the obvious reading. More positive states do make each SyGuS
call slower, but at this scale that costs 0-4% of blocks. What loses half of
them is that no boolean relation over the block's k variables satisfies the
constraints at all. **Speeding up the solver buys almost nothing; choosing the
variables better, or widening the grammar, is where the blocks are.**

---

## 8. Two claims that did not survive

Both were made from one design and corrected after 22:

- **"The MSA is negligible, there is nothing to gain."** True on s298 (0.25s
  per round). On c880 the MSA is 11.1% of the run and grows to 4.22s per
  round. The conclusion survived — the between-rounds minimiser still cannot
  help, because by then it removes 0.3-0.5% — but for the opposite reason at
  each scale.
- **"Turning off the per-round minimiser gains +1.04pp."** That was c880
  alone. Over 22 designs: +0.28pp mean, **+0.00pp median**, 7 better and 5
  worse, total time 4610s -> 4655s. Both inside the noise. The default did not
  change; the justification did — it does measurable work for no measurable
  gain, which is a simplicity argument, not a performance one.

The historical bounded-final configuration has a number of its own: **22
designs, MD mean 82.12%, total 4655s.** These measurements predate the current
k-induction final gate; reproduce them with `--no-final-unbounded`.

---

## 9. Open

1. **s953** — the one design clearly below the artifact (98.6% -> 86.2%).
2. **k-induction retention at scale.** The final gate now uses k-induction by
   default; `--unbounded` additionally enables it inside synthesis blocks. On
   s27/s298 the final gate cost nothing (100% survived, same runtime), but two
   small sequential circuits do not generalise. The controlled large-design
   run is still outstanding; use `--no-final-unbounded` only to reproduce the
   historical bounded-final experiments.
3. **Measure the implemented BitVec comparisons.**
   `--bv-predicates unsigned` now enables restricted same-width raw-signal
   equality, `bvult`, and `bvule`; it remains `off` by default. A complete A/B
   has not run because there is not yet a suitable fixed multi-bit-vector
   benchmark with a frozen mutant set. Selecting and pinning that benchmark is
   a prerequisite; an ad-hoc fresh mutant set would not be comparable.
4. c1355 / c880 / c499: our non-MSA work is slower than the artifact's
   (residual 0.5-0.9x). We emit far more assertions there — the obvious but
   unverified suspect.

---

## 10. How to read results here

Three habits this round earned:

- **Assertion count misleads.** §6 would have been a +54% success by that
  measure and was a small loss by detection. Yield rises when richer inputs
  rescue a block, falls when they make it infeasible, and stays flat while
  each invariant gets weaker. Only the mutation rate separates them.
- **One design is not a measurement.** Both claims in §8 came from a single
  A/B and neither survived 22 designs.
- **A config key nobody reads is worse than no key.** `Workflow.Minimizer`
  sat in the shipped configs for a release doing nothing, so every `mini`
  result — ours and the artifact's — was a re-run of the plain configuration.
  The `trace_policy` key was held back until the code behind it existed.

Provenance is built in: `smart --version`, the version in every generated
file's header and every run log's first record, `"run"` (writer pid) on each
log line, and `"mode"` on the check record saying whether the assertions were
proved bounded or by k-induction. `docs/FINDINGS.md` carries the full detail
including the dead ends.

---

## Appendix — per-design, `block_msa_mini` vs artifact

| design | mutants | artifact time | our time | speedup | artifact MD | our MD | ΔMD |
|---|---|---|---|---|---|---|---|
| c1355 | 546 | 2154s | 1871s | 1.2x | 56.6% | 67.4% | ** +10.8** |
| c17 | 6 | 31s | 4s | 7.6x | 100.0% | 100.0% | +0.0 |
| c432 | 160 | 901s | 199s | 4.5x | 100.0% | 100.0% | +0.0 |
| c499 | 202 | 310s | 294s | 1.1x | 53.5% | 58.4% | +5.0 |
| c880 | 383 | 662s | 753s | 0.9x | 78.1% | 92.7% | ** +14.6** |
| s1488 | 663 | 2647s | 220s | 12.0x | 80.1% | 80.1% | +0.0 |
| s27 | 17 | 31s | 6s | 5.1x | 88.2% | 88.2% | +0.0 |
| s298 | 137 | 143s | 7s | 20.4x | 79.6% | 75.9% | -3.6 |
| s344 | 179 | 149s | 12s | 12.4x | 79.3% | 82.7% | +3.4 |
| s349 | 180 | 198s | 16s | 12.3x | 70.6% | 83.3% | ** +12.8** |
| s382 | 183 | 186s | 15s | 12.4x | 72.7% | 73.2% | +0.5 |
| s386 | 169 | 145s | 18s | 8.0x | 79.3% | 80.5% | +1.2 |
| s400 | 188 | 455s | 10s | 45.5x | 77.7% | 74.5% | -3.2 |
| s420 | 238 | 728s | 22s | 33.1x | 71.8% | 83.6% | ** +11.8** |
| s444 | 206 | 124s | 13s | 9.5x | 71.4% | 76.2% | +4.9 |
| s510 | 221 | 130s | 22s | 5.9x | 74.7% | 74.7% | +0.0 |
| s641 | 402 | 2097s | 260s | 8.1x | 92.8% | 93.0% | +0.2 |
| s713 | 416 | 1956s | 416s | 4.7x | 90.1% | 90.6% | +0.5 |
| s820 | 298 | 725s | 98s | 7.4x | 68.8% | 70.8% | +2.0 |
| s832 | 296 | 698s | 138s | 5.1x | 81.8% | 82.1% | +0.3 |
| s838 | 482 | 1962s | 121s | 16.2x | 83.0% | 86.3% | +3.3 |
| s953 | 428 | 384s | 95s | 4.0x | 97.0% | 86.2% | ** -10.7** |
| **median** | | | | **7.8x** | | | **+0.5** |

Cells with |ΔMD| ≥ 10 points are bold.
