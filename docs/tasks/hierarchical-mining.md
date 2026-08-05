# Task book: mine at top without hand-flattening

Goal (Leiqi, 2026-08-05): feed a hierarchical design as-is, mine assertions in
the **top** module, with submodule internals visible as candidate variables —
the tool provides what hand-flattening used to. `--module` (mining *inside* a
submodule) stays as-is; this is a different, complementary mode.

## What exists / what is missing

| piece | status |
|---|---|
| Multi-file input, EBMC gets all files | done |
| Free registers inside instances driven | done (`fb0deeb`) |
| Submodule signals as candidates when mining top | **missing** — `Trace::getAllSignals(moduleName)` (Trace.cpp:99) filters on scope leaf name, so top mining sees only top's own signals |
| Hierarchical names in emitted assertions (`U0.count`) | **unknown feasibility** — gated on the spike below |

## Stage 1 — spike (sonnet, test job)

Question: does the in-tree EBMC 5.6 prove/refute an SVA assertion written in
the top module that references a submodule signal by hierarchical name?

Method:
- `source load_environment.sh` (ebmc is NOT on PATH otherwise; a missing ebmc
  reads as success elsewhere in this repo — see AGENTS.md).
- Two-level design: `inner` with a saturating counter `reg [3:0] count`
  (never exceeds 10), instantiated as `U0` in top.
- In top: `assert property (@(posedge clk) U0.count <= 4'd10);` — must PASS
  under `--bound 20` and ideally k-induction.
- Same but `<= 4'd5` — must FAIL with a counterexample (guards against
  vacuous/ignored references).
- Also try a two-level path (`U0.V0.x`) — one level might work where deeper
  does not.

Acceptance: a verdict per case with the exact ebmc invocation and output
excerpt. "PASS/FAIL as expected" or the precise error text.

## Stage 2 — implementation (opus, only if stage 1 passes)

- When mining top on a hierarchical design, collect candidates from **all**
  VCD scopes; a signal's Verilog name is its instance path with the top
  prefix stripped (`top.U0.count` → `U0.count`). Full paths, so leaf-name
  collisions cannot happen in this mode.
- SyGuS symbol names cannot contain `.` — mangle for the solver, unmangle for
  emission (the `__DOT__` convention already exists in the Verilator side).
- Injection target: top (no cross-file logic needed; already the default).
- Watch candidate-count blow-up: blockified rounds group variables, but the
  pre-analysis threshold may need the hierarchy-aware count.

## Acceptance for the feature

- On a design that exists in both forms (plena hand-flat vs hierarchical
  original), the hierarchical run's assertion set is compared against the flat
  run's. Metric is mutation detection where fixed mutants exist; else assertion
  set equivalence up to naming.
- ctest stays green; `--module` behaviour unchanged.
- Findings recorded in `docs/FINDINGS.md` (new §5e), including negative ones.

## Rules that bind this task

- Fixed mutants only (`MutationBenchmark/`), no fresh `mutation.py` runs.
- No full-scale runs while testing; unit corpora only.
- Comments in English. Commit on `new-interface`, never touch `artifact/` or
  the hw-cbmc submodule.
