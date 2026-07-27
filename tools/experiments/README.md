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
