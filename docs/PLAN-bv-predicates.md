# Plan: controlled BitVec predicates

**Status (2026-08-28).** Phase 1 is implemented: `--bv-predicates unsigned`
enables the restricted predicates below, while `off` remains the default and
preserves the historical grammar. Focused option, grammar, and parser regression
tests are present. The full mutation-detection experiment has not run because
there is not yet a suitable fixed multi-bit-vector benchmark with a frozen
mutant set. Select a licence-clear source, pin its revision, and freeze its
mutants before Stage A; do not substitute a fresh ad-hoc mutant set. The final
output gate now uses k-induction by default. Historical bounded-final results
must be reproduced with `--no-final-unbounded`.

## 1. Goal

Measure whether SMART's deliberately small, now-implemented family of
bit-vector predicates improves mutation detection. With the default `off`
grammar, the Boolean root cannot consume any `MixBv*` nonterminal, so a block
containing wide signals cannot express even `a == b`, `count < limit`, or
`read_ptr <= write_ptr`. This remains a plausible language-level explanation
for part of the 43.8–46.8% `infeasible` block rate recorded in
`docs/CHANGES-1.0.md`, not a measured conclusion.

This work does **not** change trace generation, block selection, bounds, MSA,
or minimisation. Those stay fixed so any measured difference is attributable
to the grammar.

Primary success metric: mutation-detection rate against one pinned RTL suite
and frozen mutant set. Assertion count is diagnostic only.

## 2. Non-goals

- Do not enable the new grammar by default before the controlled suite
  experiment.
- Do not add arbitrary comparisons over the existing recursive `MixBv*`
  arithmetic grammar in the first implementation. That search space includes
  expressions such as `bvule (bvadd a b) (bvmul c d)` and would confound the
  value of comparisons with a large solver-cost change.
- Do not add signed comparisons in the first experiment.
- Do not infer constants or parameters from source text yet.
- Do not change the random seed, trace policy, trace count/depth, block layout,
  candidate-check bound, or solver timeout between A and B.
- Do not claim improvement from yield alone. A richer trace policy already
  raised assertions by 54% on s838 while mutation detection fell.

## 3. User-facing interface

The implemented string option in the single option table in
`pipeline/Options.cpp` is:

```text
--bv-predicates MODE
bv_predicates: "off" | "unsigned"
```

- `off` is the default and must preserve the old generated grammar byte for
  byte.
- `unsigned` enables the phase-1 predicates below.
- Any other value is a user error that names the accepted values.
- Validate the resolved value in `Options` immediately after CLI/config
  parsing, before frontend work, workdir creation, or simulation. The block
  worker must call the same validation after loading its effective config.
- The resolved value must appear in `--dump-config`,
  `effective-config.json`, the generated assertion header, and the run-log
  start record. The first three happen through the existing option machinery;
  the run-log field must be added explicitly.

The string interface is intentional: later experiments may add `signed` or
`arithmetic` without replacing a Boolean flag or changing old configs.

## 4. Phase-1 grammar

For each bit width `W` present in the block, generate a restricted term
nonterminal containing only raw signals of that width:

```lisp
(CmpBv8 (_ BitVec 8)
  (count limit))
```

In `Atom`, add:

```lisp
(=     CmpBv8 CmpBv8)
(bvult CmpBv8 CmpBv8)
(bvule CmpBv8 CmpBv8)
```

Repeat the three productions for every width represented in the block. Never
compare different widths.

The generator's SyGuS format has two sections: a nonterminal declaration list
followed by grouped rule definitions. Add `CmpBvW`/`CmpBvXW` to both sections;
the example above depicts the rule definition, not the complete insertion.

For an LTL grammar, generate the current-state `CmpBvW` terms for `Atom` and a
separate next-state `CmpBvXW` containing the `_latency` signal names for
`AtomX`. Phase 1 does not introduce cross-time bit-vector terms inside one
atom; temporal relationships remain the existing `Atom => AtomX` shape.

Do not put constants or recursive arithmetic inside `CmpBv*` in phase 1.
Signal-to-constant predicates are phase 2, after phase 1 has a measured solver
cost and detection effect.

Only emit a comparison nonterminal when at least two signals share the width.
With one signal, `x=x`, `x<=x`, and `x<x` carry no state information and only
add search noise.

`bvult` and `bvule` have unsigned SMT semantics. The parsed SystemVerilog must
preserve that explicitly as `$unsigned(lhs) < $unsigned(rhs)` and
`$unsigned(lhs) <= $unsigned(rhs)`. A final EBMC gate prevents an unsound
assertion from escaping, but semantic drift between synthesis and checking
would still invalidate the experiment and waste refinement rounds.

### Naming and determinism

- Width-specific nonterminal names are `CmpBv<W>` and `CmpBvX<W>`.
- Iterate the existing ordered `sameTypeSignals` map and its stable signal
  vectors; generated grammar order must be deterministic.
- Emit no empty or single-signal comparison nonterminal.

## 5. Implemented shape

The phase-1 implementation follows this design; these items are complete.

1. `SyGuSGenerater` has an internal `BvPredicateMode { Off, Unsigned }` enum
   and a setter that validates the option string.
2. One shared `Options` validation path runs after both command-line parsing
   and block-worker config loading, rather than relying on a late generator
   exception for user input validation.
3. The block entry point (`smart.cpp`) passes the resolved `bv_predicates`
   option to the generator before the first `.sl` file is written.
4. Small helpers keep the feature out of another large
   `createKeyGrammar()` branch:
   - render one raw-term nonterminal for a width and latency;
   - render the Boolean productions that reference it.
5. `MixBv*` remains unchanged and is unused by the new phase-1 predicates.
6. The SMT-expression renderer preserves unsigned comparisons in emitted
   SystemVerilog.
7. Parser latency resets at the start of every top-level parse so a prior `_N`
   suffix cannot leak into a later expression when the process-global parser is
   reused across refinement rounds.
8. The two stale BitVec-comparison TODO comments were removed when the
   controlled insertion point replaced them.
9. The start run-log record and verbose block diagnostics record the selected
   mode.

## 6. Test coverage and gates before experiments

### Option tests

- Default resolves to `off`.
- CLI and flat JSON accept `unsigned`.
- CLI overrides config.
- An unknown mode fails before simulation or block creation.
- `--dump-config` round-trips the value.

### Grammar tests

The focused C++ test target constructs signals directly and inspects a
generated temporary `.sl` file.

- `off` contains none of `CmpBv`, `bvult`, or `bvule`. Freeze a golden from the
  pre-feature generator (available through Git) and require constructor
  default and explicit `off` to match it byte for byte.
- `unsigned` with two 8-bit signals emits all three predicates and lists only
  those signals in `CmpBv8`.
- Mixed 4-bit/8-bit signals produce separate nonterminals and no cross-width
  production.
- Boolean-only blocks do not emit a BitVec predicate nonterminal.
- LTL mode uses current names in `CmpBvW`, numeric suffix names such as `_1`
  in `CmpBvXW`, and does not mix them.
- Parse a cvc5 LTL `define-fun` containing `bvult`/`bvule` back into
  SystemVerilog and verify both the numeric latency and `$unsigned` casts.
- Reuse one parser for a next-state expression and a current-only expression;
  the first expression's latency must not leak into the second.
- cvc5 parses and solves at least one generated phase-1 grammar through the
  in-process API.

### Regression tests

- All existing CTest suites pass.
- A default/off `tiny_and` run still completes and its effective grammar has
  no BitVec predicate production.
- A small wide-signal fixture completes under `unsigned`; every emitted
  assertion is independently rechecked by EBMC.
- High-warning build remains clean for project sources.

## 7. Experiment protocol

### Stage A: feasibility gate

Stage A is blocked pending benchmark selection. The existing fixed suite does
not provide a suitable behavioral, multi-bit-vector workload for measuring this
feature. `s27`, `s953`, and `plena_data_flow_control_flat` remain useful smoke
or regression cases, but they are not a complete BitVec value benchmark.

Before running Stage A, select a licence-clear behavioral RTL source with
multi-bit counters, pointers, FSM state, lengths, or addresses; pin its source
revision and construct and freeze one mutant set.

For each design, run `off` and `unsigned` with identical config, seed, machine
load, and fixed mutants. Collect:

- mutation detections / fixed mutant count;
- infeasible, verified, refuted, killed, and setup-failed blocks;
- blocks returning a candidate;
- total run time and per-block wall time;
- verified assertion count;
- number of final assertions that contain `<`/`<=`; BV equality needs AST or
  signal-sort metadata and must be reported as unavailable rather than guessed
  from every Boolean `==`.

Current block records do not isolate SyGuS time from EBMC/refinement time. Do
not label block wall time as solver time. Adding structured
`sygus_seconds`/`sygus_calls` is a separate instrumentation task; until then,
the feasibility gate uses block wall time and total run time.

Proceed to Stage B only if:

- no soundness or independent-EBMC failure occurs;
- median block wall time does not increase by more than 2x;
- no design loses more than 2 percentage points MD without a diagnosed
  mechanism; and
- at least one design produces a verified comparison assertion or materially
  reduces infeasible blocks.

### Stage B: controlled suite A/B

- Use the newly frozen vector-benchmark mutant set; never regenerate mutants
  between modes or runs.
- Run `off` and `unsigned` from the same commit with the same seed and config.
- Do not run both variants concurrently on a loaded machine.
- Persist the full command, config, Git revision, tool versions, and mutant-set
  identity beside the results.
- Group `run-log.jsonl` records by `run` before aggregating timings.
- Report mean/median MD delta, better/worse/unchanged counts, runtime delta,
  infeasible-rate delta, and per-design comparison-assertion counts.

### Decision rule

Enable `unsigned` by default only if the controlled suite shows a positive
median MD change, no serious regression cluster, and acceptable runtime.
Otherwise keep it opt-in and record the negative result in `docs/FINDINGS.md`.

## 8. Follow-up phases, separately measured

Only after phase 1 is decided:

1. **Constants:** add zero, one, and all-ones to each restricted width; later
   consider elaborated parameters. Measure separately.
2. **Signed predicates:** `bvslt` / `bvsle`, enabled by a distinct mode and
   applied only where signedness is known rather than guessed.
3. **Arithmetic comparisons:** compare bounded-depth BV expressions. Introduce
   an explicit grammar-depth limit before connecting predicates to `MixBv*`.
4. **Structure-aware blocks:** build register/next-state and local-cone
   portfolios. This is the next major project, not part of the grammar A/B.

## 9. Deliverables and status

- **Done:** option and implementation with default-off compatibility.
- **Done:** focused grammar/option/parser regression tests registered in CTest.
- **Pending:** select and pin a suitable vector benchmark and freeze its mutant
  set.
- **Done:** a small wide-signal end-to-end fixture and independent EBMC check.
- **Pending:** Stage-A commands and result table, even if the result is negative.
- Update the measured-result section of `docs/FINDINGS.md` only after those
  measurements exist.
