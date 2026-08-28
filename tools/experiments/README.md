# The scripts behind the numbers

Every measured claim in `docs/FINDINGS.md` §4 came from one of these. They ran
in a container so the toolchain was fixed and the host stayed clean, and the
paths below are that container's — `/work/smartVerilog` is the checkout,
`/work/MB` is `MutationBenchmark/`. Adjust or bind-mount to suit; nothing here
depends on being containerised.

| script | what it produced |
|---|---|
| `eval_one.sh <config> <design>` | one cell: mine with a `Config/*.json`, then score against the fixed mutants |
| `runall.sh` | §4.3, the 4 x 22 config ranking |
| `eval_pol.sh <policy> <design>` | one cell at the shipped defaults under one `--trace-policy` |
| `runpol.sh` | §4.6, the 2 x 22 stimulus-policy study |
| `check_oracle.py [log]` | §4.5, the end-minimiser soundness oracle |
| `policy_table.py [log]` | §4.6, the policy comparison table |
| `bv_ab.py prepare/run/summarise` | controlled `off` vs `unsigned` BitVec-predicate Stage A/B |

## Controlled BitVec-predicate A/B

**Status:** the implementation and bookkeeping are complete, but the named
Stage A/B selections are not a valid value experiment yet. The current fixed
suite is overwhelmingly scalar, and no suitable fixed multi-bit-vector
benchmark plus frozen mutant set has been selected. Use the commands below only
for plumbing checks until that prerequisite in `docs/PLAN-bv-predicates.md` is
closed; do not report their output as a BitVec effectiveness result.

`bv_ab.py` implements the preparation and bookkeeping protocol from
`docs/PLAN-bv-predicates.md`. Preparation validates that every design already
has a non-empty fixed mutant set, hashes those files, the main/sibling RTL, the
frozen config, and the SMART binary, records the Git state, tool versions, host
and cell load average, and writes the exact mining commands. It does **not** run
SMART:

```bash
python3 tools/experiments/bv_ab.py prepare --stage A \
  --config Config/block_msa.json --seed 42 --output /work/bv-stage-a
```

Review `/work/bv-stage-a/manifest.json` and `mine-commands.sh`, arrange an
otherwise idle machine, then explicitly execute the manifest:

```bash
python3 tools/experiments/bv_ab.py run /work/bv-stage-a/manifest.json
python3 tools/experiments/bv_ab.py summarise /work/bv-stage-a/manifest.json
```

Cells run serially in `off`, `unsigned` order with the same frozen config and
seed. `run` rechecks all hashes before starting. Evaluation copies the fixed
`MutationBenchmark/<design>/benchmarks` tree into an isolated cell, verifies
the copy's hash, and deliberately does not copy `mutation.py`; an absent or
changed mutant set is therefore a hard error, never a fresh generation.

The current Stage B selector names the same 22 scalar-heavy designs as the
earlier studies (`--stage B`); it is retained for plumbing regression, not as
the eventual vector benchmark suite. Use repeatable `--design NAME` during
plumbing checks without changing the named Stage A/B manifests. Results include
`summary.csv` and `summary.json` with MD,
block reasons, candidate blocks, final-check failures, timer-reported CVC5-call
time, block wall time, and final assertions whose associated SMT definition
actually contains a raw BitVec comparison. CVC5-call time is diagnostic and is
not presented as full SyGuS-stage time. Run-log records are grouped by `run`
before any timing is read; required fields or effective mode mismatches fail
loudly.

The primary `md_fixed_percent` is always `detected / fixed_mutants`, including
timed-out mutants in the denominator. The evaluator's historical
timeout-excluding percentage is retained only as
`evaluator_non_timeout_md_percent`. A non-zero evaluator exit, evaluator
timeout, tool error, or denominator mismatch marks the cell invalid; invalid
cells stay visible with reasons but cannot contribute a pair, delta, or A/B
aggregate. For Stage A, a complete valid run also enforces the planned 2x
timing gate using the median of the per-design `unsigned/off` median block-wall
ratios; a failed gate makes `run`/`summarise` exit non-zero.

The bookkeeping plumbing has a self-contained test that creates synthetic
logs in a temporary directory and invokes no external tools:

```bash
python3 -B tools/experiments/test_bv_ab.py
```

## Two things they get right that are easy to get wrong

**ebmc must be on PATH.** Every one of these exports it explicitly and
`eval_one.sh` refuses to start without it. When it is missing, `evaluater.py`
sees exit 127 from every mutant check and counts each one as *detected*: c880
came out at 100% mutation detection against a true 61.6%. A silent perfect
score is the failure mode.

**Mutants come from `MutationBenchmark/`, never from `mutation.py`.** The
generator picks its operators at random — two runs on c880 both produced 383
mutants with 294 of them different. Detection rates only compare against one
fixed set, so these scripts fail loudly (`NO-FIXED-MUTANTS`) rather than
generate their own.

## Reading the oracle

`check_oracle.py` exists because `block_msa` and `block_msa_mini` are the same
run up to the final filter — same seed, same rounds, same mined set. The
minimiser only drops an assertion the kept set implies, so any state violating
it violates something kept, so **the detection rate has to come out
bit-identical**. A difference means an unsound minimiser or an inconsistent
EBMC.

It checks the precondition too. Two of 22 designs diverged before the
minimiser ran — the pipeline is not bit-reproducible under parallelism — and
those cells are inconclusive rather than counterexamples. The script reports
the trajectory alongside the delta so the two cases are not confused.
