# Progress: SMART C++ single-binary rewrite

Companion to [PLAN-cpp-single-binary.md](PLAN-cpp-single-binary.md).
Branch: `new-interface`.

## Update protocol (for every agent working a WP)

1. Before starting: set your WP's row to `IN PROGRESS`, add your agent/run
   identifier and date.
2. Check items off (`[x]`) only when the acceptance evidence exists; paste
   the commit hash next to the item.
3. When a WP is done: set the row to `DONE (<commit>)` and fill in the
   "Evidence" line with the exact command(s) run and a one-line result.
4. If you discover something that changes the plan (new gotcha, wrong
   assumption), append it to the **Deviations & discoveries log** at the
   bottom — do not silently edit the plan; the plan file is only edited by
   the coordinator.
5. One WP per commit series; do not mix WPs in a commit.

## Status board

| WP | Scope | Status | Agent | Depends on |
|----|-------|--------|-------|------------|
| WP2A | hw-cbmc submodule + build integration | NOT STARTED (critical path) | — | — |
| WP1 | CMake build (incl. hw-cbmc libs) | NOT STARTED | — | WP2A |
| WP2 | frontend adapter: ModuleInfo from hw-cbmc | NOT STARTED | — | WP2A |
| WP2-regex | regex frontend (SVModule) — DEMOTED to oracle/fallback | DONE (198a0aa) | coordinator (Claude/Fable) | — |
| WP3 | simgen (Verilator harness) | NOT STARTED | — | — (uses ModuleInfo; oracle supplies it meanwhile) |
| WP4 | pipeline (orchestration, Options/Config §1.1) | NOT STARTED | — | WP2, WP3 |
| WP5 | mus (MSA/MUS + minimizer, libcvc5) | NOT STARTED | — | WP1 |
| WP6 | sygus via libcvc5 API | NOT STARTED | — | WP1 |
| WP7 | emit + checker integration | NOT STARTED | — | WP4 |
| WP8 | cleanup, packaging, docs | NOT STARTED | — | all |

**Architecture change 2026-07-25:** the Verilog frontend is now hw-cbmc's
(git submodule + build), NOT our regex. Rationale: no hand-written SV parser,
same toolchain as the EBMC backend, upstream-maintained. The regex frontend
(WP2-regex, done at 198a0aa) is kept only as the `ModuleInfo` contract and the
parity oracle. See plan §0 decisions, WP2A, rewritten WP2.

## Pre-work already landed (context for all agents)

- [x] P0 bug fixes + usability pass — `6495c97` (master, merged into branch)
- [x] `gen_bench.py` behavioral spec + tests, 54/54 benchmarks — `d6a1ef7`
- [x] Plan — `e2fbc07`, config model §1.1 — `95f34aa`
- Environment note: host has NO verilator/ebmc/cocotb/nlohmann-json —
  build and test in the Docker image only (see plan §4 WP1 and gotcha 10).

## WP checklists

### WP2A — hw-cbmc submodule + build (critical path)
- [ ] `third_party/hw-cbmc` submodule pinned (record commit/tag; match EBMC 5.6)
- [ ] `.gitmodules`; nested cbmc submodule fetched (`--init --recursive`)
- [ ] Docker/Dockerfile + artifact/Dockerfile build hw-cbmc (cached layer),
      replacing the apt/.deb ebmc install
- [ ] link-vs-subprocess spike done; mechanism + required libs recorded
- [ ] Acceptance: clean checkout builds ebmc; frontend consumable (link demo
      or documented --show-parse/--show-varmap invocation)
- Evidence:

### WP1 — CMake
- [ ] `smart/CMakeLists.txt`: flex (`-P VCDParser` prefix!) + bison codegen
- [ ] hw-cbmc submodule libs + ebmc exposed to the smart build (with WP2A)
- [ ] nlohmann-json resolved (find_package or vendored header)
- [ ] libcvc5 available in Docker image (Dockerfile change, coordinate WP6)
- [ ] `smart.out` builds via CMake in Docker
- [ ] Acceptance: CMake-built `smart.out` passes `python run.py c17` in Docker
- Evidence:

### WP2 — frontend adapter (fill ModuleInfo from hw-cbmc)
- [ ] `HwcbmcFrontend.{h,cpp}` fills the existing `ModuleInfo` via WP2A mechanism
- [ ] `--dump-frontend` reuses `dumpJson`
- [ ] anyseq/anyconst + widths from hw-cbmc parse tree/symbol table
- [ ] Acceptance: `tools/parity_frontend.py` vs the frozen oracle — match or
      document each diff as a regex limitation hw-cbmc gets right
- Evidence:

### WP2-regex — DONE (demoted to oracle/fallback)
- [x] `SVModule.{h,cpp}`: parse / stripAssumes / injectAssumes / guesses — 198a0aa
- [x] standalone `frontend_dump` (JSON + --strip) — 198a0aa
- [x] `tools/parity_frontend.py` — 198a0aa
- [x] parity vs `gen_bench.py`, 56 designs, + 3 assume-strip checks — 198a0aa
- [x] 6 `test_gen_bench.py` cases ported to `test_svmodule.cpp` — 198a0aa
- KEEP: `ModuleInfo` struct + `dumpJson` (contract), `stripAssumes`/
  `injectAssumes` (textual surgery WP2 reuses), `guessClock/guessReset`.
  FROZEN: do not extend the regex parsing itself.
- Evidence: `python3 tools/parity_frontend.py` -> "parity OK: 56 designs
  (JSON), 3 assume-strip checks"; both suites pass. Build:
  `g++ -std=c++17 -O1 smart/src/frontend/SVModule.cpp
  smart/src/frontend/{frontend_dump,test_svmodule}.cpp`

### WP3 — simgen
- [ ] `Harness.{h,cpp}`: sim_main.cpp generation (stimulus semantics per plan)
- [ ] verilator subprocess wrapper (+ flags incl. conditional --public-flat-rw)
- [ ] assume-stripped sim copies vs original formal copies
- [ ] VCD scope compatibility verified against a cocotb-produced VCD (gotcha 11)
- [ ] `--cycles` genuinely controls trace depth (gotcha 12)
- [ ] Acceptance: tiny_and / s27 / axis_fifo / nru_a VCDs load with same
      signal set as cocotb flow
- Evidence:

### WP4 — pipeline
- [ ] `Options/Config` from single option table (§1.1), legacy adapter,
      `--dump-config`, `effective-config.json`
- [ ] workdir layout byte-compatible with `smart/runtime/` tree
- [ ] `preAnalyzer.py` ported
- [ ] `smart.cpp` main() refactored to `runSmartBlock()` library call
- [ ] thread pool + core deadline + global timeout + MSA/Random block loop
- [ ] progress output + per-block logs + fail-fast workdir retention
- [ ] Acceptance: tiny_and e2e; c17 assertion-set parity vs legacy pipeline
- Evidence:

### WP5 — mus
- [ ] fixtures captured from Docker runs into `smart/test/fixtures/`
- [ ] `getMus()` (port of get_mus) via libcvc5
- [ ] `minimiseAssertions()` (PowerLattice port) via libcvc5
- [ ] Acceptance: golden parity vs Python on 3 fixtures
- Evidence:

### WP6 — sygus API mode
- [ ] cvc5 API construction of the SyGuS problem in `SyGuSGenerater`
- [ ] per-call time limit from config (no hardcoded 5s)
- [ ] `--sygus-subprocess` A/B fallback flag
- [ ] Acceptance: c17 + tiny_and assertion sets identical in both modes
- Evidence:

### WP7 — emit + checker
- [ ] `AssertionWriter`: `<top>_assertion.sv` + assertions.txt/invariants.txt
- [ ] checker stage on `VerilogChecker` (parallel, timeout, bound/k-induction)
- [ ] EBMC command-builder unit test (gotcha 7)
- [ ] Acceptance: emitted file passes ebmc; bogus assertion is filtered
- Evidence:

### WP8 — cleanup & packaging
- [ ] remove Makefile build path, `--sygus-subprocess`, Python tool-flow files
      (evaluater.py/mutation.py STAY and still work off invariants.txt)
- [ ] `--check-env`, static linking, release tarball
- [ ] ReadMe/ARTIFACT/Dockerfiles updated
- [ ] final smoke matrix: {tiny_and, c17, s27, arb2, axis_fifo, nru_a} × {plain, --msa}
- Evidence:

## Deviations & discoveries log

(append-only; newest last)

- 2026-07-25: plan + progress tracker created. Nothing in flight yet.
- 2026-07-25 (WP2): libstdc++ std::regex SIGSEGVs (stack overflow from
  backtracking) on large ISCAS netlists (c2670: hundreds of 1-bit ports).
  Frontend scanning is hand-rolled; rule for all later WPs: **no std::regex
  over whole files/modules**, only over short strings.
- 2026-07-25 (WP2): found a spec bug via the parity harness — gen_bench.py
  DIR_RE/FREE_REG_RE type keywords lacked `\b`, so `output logic
  regfile_we_o` parsed as name `file_we_o`. Fixed in gen_bench.py first
  (lockstep rule), C++ matches. Any WP3 agent regenerating sim.py files
  from an older gen_bench.py should rebase past 198a0aa.
- 2026-07-25 (DIRECTION CHANGE): user decided the Verilog frontend must be
  hw-cbmc's, added as a git submodule and built with us — no self-written SV
  parser. "Since EBMC is our backend, use them as the frontend too; upstream
  maintains it." Plan reworked: new WP2A (submodule+build), WP2 becomes a thin
  adapter filling ModuleInfo from hw-cbmc, regex frontend demoted to oracle/
  fallback. Investigation facts: hw-cbmc is BSD-3, nests cbmc as a submodule,
  builds via `git submodule update --init --recursive && cd src && make`
  (produces ebmc + verilog/util static libs), `verilog_languaget` exposes
  parse()/typecheck()/get_parse_tree() but is coupled to CBMC's irept/
  symbol_table (so linking is a spike; ebmc --show-parse/--show-varmap is the
  subprocess fallback). Verilator has --json-only (no --xml-only) but drops
  yosys attributes, so hw-cbmc is the better anyseq/anyconst source.
