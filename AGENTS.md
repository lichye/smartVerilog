# Working on SMART

SMART mines SystemVerilog assertions from random simulation traces and emits
the ones EBMC proves: `smart cat.sv` -> `cat_assertion.sv`. One compiled C++
binary; no Python in the pipeline.

## Getting a working shell

```bash
source load_environment.sh        # required for anything but ./build/smart
```

Nothing this tool needs is installed system-wide. The checkout carries all of
it — `smart`, `ebmc` (hw-cbmc submodule), `cvc5` (built from source), and
`verilator`/`iverilog` (oss-cad-suite). The binary appends those directories to
its own PATH at startup, so it runs without help; **everything else does not** —
calling `ebmc` by hand, running `smart/evaluater.py`, poking at cvc5.

Order matters in one place: oss-cad-suite ships cvc5 1.0.1-dev and ours is
1.2.0. Sourcing the script puts ours first.

```bash
cmake -B build -S . && cmake --build build -j$(nproc)   # builds smart
cd build && ctest                                       # 4 suites
smart --check-env                                       # what the tool sees
```

`tools/build-cvc5.sh` builds libcvc5 (~30 min, one-off). `./install.sh` sets up
the rest.

## Conventions that are not obvious

**One option table.** `smart/src/pipeline/Options.cpp` drives the CLI *and* the
config file. Adding a row makes an option reachable from both. Legacy nested
config keys are still accepted, including two misspelled upstream
(`Threadhold`, `Nagative_state_number`) — spelled here as they appear on disk,
because shipped configs use them.

**Never add a config key nothing reads.** `Workflow.Minimizer` sat in the
shipped configs through a whole release with no code behind it, so every
`block_msa_mini` result — ours *and* the published artifact's — was a
byte-for-byte re-run of the plain configuration. If the behaviour is not
implemented yet, the key waits.

**Comments carry the measurement, not the intention.** Defaults in
`Options.cpp` cite the numbers that chose them, including which claims were
later overturned. When you change a default, change the number with it.

**`runtime/` names are a contract.** The reused core (`smart.cpp`,
`VerilogChecker`) resolves several paths relative to its working directory.
Grep `runtime/` under `smart/src/` before renaming anything in `WorkDir`.

## Measurement discipline

This is where the time goes, so it is worth being blunt about it.

**The metric is mutation detection, not assertion count.** They come apart in a
specific and repeatable way: richer inputs raise the count when they rescue a
block from wasted refinement, lower it when they make the block infeasible, and
leave it alone while making every invariant *weaker*. The stimulus-search study
was +54% assertions on s838 and -0.41pp detection. Yield would have called it a
win.

**Mutants must be the fixed set.** `MutationBenchmark/` — never a fresh
`mutation.py` run. The generator picks operators at random: two runs on c880
both produced 383 mutants, 294 of them different. Rates are comparable only
against one fixed set.

**One design is not a measurement.** Two claims in `docs/FINDINGS.md` were made
from a single A/B and neither survived 22 designs. Both corrections are recorded
next to the original claim on purpose.

**A number without the command that produced it is not evidence.** Runs are not
bit-reproducible — blocks finish in wall-clock order and the MSA is a greedy
algorithm, so two identical invocations can take different trajectories. That
is understood and accepted; it means a single-cell difference is not a finding.

**Timings taken under load are not comparable.** The machine runs 16-way
parallel experiments; a run started beside one is not a control for a run
started alone.

## Reading a run

`--keep-work` keeps `smart-work-<top>/`. The interesting file is
`run-log.jsonl`, one JSON object per stage and per block, flushed as it goes so
it survives a kill.

```bash
jq -c 'select(.stage)'  smart-work-<top>/run-log.jsonl    # stages
jq -c 'select(.block)'  smart-work-<top>/run-log.jsonl    # blocks
```

Fields that exist because of specific mistakes:

- **`run`** — the writer's pid. `RunLog::record` reopens in append mode, so a
  run whose workdir is deleted underneath it recreates the file and keeps
  writing. Two runs then interleave in one log with two independent `t`
  timelines. **Group by `run` before reading any timing.**
- **`reason`** on a block — `verified` / `refuted` / `infeasible` / `killed` /
  `setup-failed`. All three failure modes used to read `no-assertion`. The
  split matters: infeasible is 44-47% of blocks and killed-on-time is 0-4%, so
  a faster solver buys almost nothing.
- **`mode`** on the check record — `bounded` or `k-induction`. A bounded run
  emits properties that hold to `bound` only: sound to report, **not**
  invariants, and unsound to assume inside another proof. `invariants.txt` is
  named the same either way and its line format is depended on by
  `evaluater.py`, so the log is the only place that distinguishes them. Use
  `--unbounded` when the output is meant to be assumed.

## Where the documentation is

| file | what it is |
|---|---|
| `docs/FINDINGS.md` | defects, dead ends *with their numbers*, environment facts. Read §3 before repeating an experiment. |
| `docs/CHANGES-1.0.md` | work record: what was built, measured, and overturned |
| `docs/PLAN-cpp-single-binary*.md` | the rewrite plan and its tracker (complete) |
| `ARTIFACT.md`, `artifact/` | frozen for the published artifact — deliberately still describes the container flow |

## Mining inside a submodule

`--module <instance>` points the miner at a scope other than the top:

```bash
smart s27.sv --module DFF_0        # mines dff's internals, checks all instances
```

Three names that used to be one, and are not interchangeable:

- **`--top`** — what EBMC elaborates. Always the design top.
- **`--module`** — a **VCD scope**, which is an *instance* (`DFF_0`). Its
  signals become the candidate variables.
- **the injection target** — a module *definition* (`dff`), resolved from the
  instance by `verilator --xml-only`'s cell list. Writing into the definition
  means every instance is constrained, so an invariant that only held in the
  instance you mined gets refuted rather than silently kept.

Passing a module name where an instance is expected fails loudly: the trace
has no scope by that name and the run stops with "no candidate variables".

## Live gotchas

- **hw-cbmc drops a single `(* anyseq *)`** — `attr_spec_list` discards `$1`.
  Patched at build time from `third_party/patches/`. Upstream is out of scope by
  decision, so the submodule working tree is *expected* to show as modified.
  Do not commit it.
- **`--trace-policy fuzz`** exists and is off by default; it reached more states
  and did not improve detection. Kept because the measurement rules on the
  current boolean grammar, not on a richer one.
- **`SyGuSGenerater.cpp` has a marked TODO** (`//wait for add bv compare
  grammar`). Nearly half of all blocks fail as infeasible and a counter bound
  like `count <= limit` is not expressible today. That is where the blocks are.
