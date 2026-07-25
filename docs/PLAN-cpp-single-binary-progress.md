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
| WP2A | hw-cbmc submodule + build integration | DONE | coordinator (Claude/Opus) | — |
| WP1 | CMake build (incl. hw-cbmc libs) | DONE | coordinator (Claude/Opus) | WP2A |
| WP2 | frontend adapter: ModuleInfo from hw-cbmc | DONE | coordinator (Claude/Opus) | WP2A |
| WP2-regex | regex frontend (SVModule) — DEMOTED to oracle/fallback | DONE (198a0aa) | coordinator (Claude/Fable) | — |
| WP3 | simgen (iverilog testbench) | NOT STARTED | — | — (uses ModuleInfo; oracle supplies it meanwhile) |
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
- Environment note (REVISED 2026-07-25, user decision): **build and test on
  the host; Docker is out of scope.** External third-party packages installed
  locally are fine — the goal is a directly testable local environment.
  Already resolved: ebmc (built from the submodule), nlohmann-json (vendored
  at `smart/third_party/nlohmann/json.hpp`, `smart/Makefile -I ./third_party`;
  `make compile` builds `smart.out` on the host again). Still to install when
  their WP starts: verilator (WP3), libcvc5 C++ dev (WP5/WP6).

## WP checklists

### WP2A — hw-cbmc submodule + build (critical path) — DONE
- [x] `third_party/hw-cbmc` pinned to `ebmc-5.6` (`9b402aa7`), matching the
      EBMC the current results were produced with
- [x] `.gitmodules`; nested cbmc fetched (`--init --recursive`) at `3c915ebe`
- [x] host build script `third_party/build-hw-cbmc.sh` (patches +
      `minisat2-download` + `make -C src`); Dockerfiles deliberately NOT
      touched — Docker is out of scope now
- [x] link-vs-subprocess spike done: **LINK**. Libs/includes/define recorded
      in plan §4 WP2A "Recorded decisions"
- [x] upstream grammar bug found + patched (`third_party/patches/`,
      `apply-patches.sh`): single `(* attr *)` instances were dropped
- [x] Acceptance: `ebmc --version` -> 5.6; `hwcbmc_spike` links libverilog and
      prints ports/params/freeRegs/hasAssume
- Evidence:
  - `third_party/build-hw-cbmc.sh` -> `5.6`
  - `g++ -std=c++17 -D'LOCAL_IREP_IDS=<hw_cbmc_irep_ids.h>' -I third_party/hw-cbmc/src
    -I third_party/hw-cbmc/lib/cbmc/src smart/src/frontend/hwcbmc_spike.cpp
    -Wl,--start-group <7 .a files> -Wl,--end-group`
  - `hwcbmc_spike CaseStudy/sim/nru/formal.sv` -> `module formal`, 1 port,
    9 `anyseq` + 1 `anyconst` free regs, `hasAssume 0` (correct: the only
    assume in that file is commented out)
  - `ebmc --show-symbol-table` on a `#(parameter W=8)` module -> ports as
    `unsignedbv` with resolved `width` (8, 4) — the WP2 width source

### WP1 — CMake — DONE
- [x] `smart/CMakeLists.txt`: flex (`-P VCDParser`) + bison codegen into
      `generated/`; `smart_core` static lib; `smart.out`, `frontend_dump`,
      `test_svmodule` targets; `ctest` runs the SVModule spec tests
- [x] `third_party/CMakeLists.txt`: `hw-cbmc` custom target (drives
      `build-hw-cbmc.sh`) + `hwcbmc_frontend` INTERFACE lib carrying the libs,
      include dirs and `LOCAL_IREP_IDS`; `hwcbmc_spike` target links it
      (EXCLUDE_FROM_ALL — building it triggers the slow hw-cbmc build)
- [x] nlohmann-json vendored (`smart/third_party/nlohmann/json.hpp`)
- [ ] libcvc5 — deferred to WP5/WP6, which are the first consumers
- [x] `smart.out` builds via CMake on the host
- [x] Acceptance: CMake-built `smart.out` passes `python run.py c17` locally
- Evidence:
  - `cmake -B build -S . && cmake --build build -j32` -> `build/bin/{smart.out,
    frontend_dump,test_svmodule}`; `test_svmodule` -> "all SVModule tests
    passed"; `parity_frontend.py build/bin/frontend_dump` -> "parity OK: 56
    designs"
  - `SMART_BIN=$PWD/build/bin/smart.out python3 run.py c17` -> exit 0,
    "found 33 new assertions", "The number of verified assertions is: 33"
    (verified by the submodule-built EBMC 5.6), `Results/smart_c17/
    invariants.txt` = 33 lines
  - local env now complete: `python3 run.py --check-env` -> all [ok]

### WP2 — frontend adapter (fill ModuleInfo from hw-cbmc) — DONE
- [x] `HwcbmcFrontend.{h,cpp}` fills the existing `ModuleInfo` by linking
      hw-cbmc (WP2A mechanism)
- [x] `hwcbmc_dump` CLI reuses `dumpJson` (WP4 folds it in as
      `--dump-frontend`); same CLI as `frontend_dump` so either can be driven
- [x] anyseq/anyconst from the parse tree, widths from the elaborated symbol
      table, ports from the elaborated module type (normalises ANSI and
      non-ANSI headers), params = every elaborated compile-time constant
- [x] multi-file designs: sibling `.sv`/`.v` files are elaborated together via
      langapi's `language_filest`, which orders modules by dependency
- [x] Acceptance: `tools/compare_frontends.py` — 0 unexplained divergences
- Evidence: `python3 tools/compare_frontends.py` -> exit 0
  - 56 designs, 15 byte-identical
  - benign: `param-extra` 1279 (hw-cbmc reports localparams and constants the
    regex could not evaluate), `order` 25 (hw-cbmc keeps module-header port
    order; the oracle reorders non-ANSI ports inputs-first),
    `width-resolved` 2 (oracle null -> real width)
  - 4 designs with recorded limitations: 2 i2c cores + s1196 where hw-cbmc 5.6
    aborts during elaboration (so does upstream `ebmc` on the same files — we
    degrade to parse-tree ports instead of dying), and ibex_controller /
    ibex_decoder where the ORACLE truncates port names to `inp`
  - the oracle path is unchanged: `test_svmodule` green,
    `parity_frontend.py` -> "parity OK: 56 designs"
- FOLLOW-UP: check whether hw-cbmc >= 6.0 elaborates the i2c cores; if it
  does, that is a concrete reason to bump the submodule pin.

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

### WP3 — simgen (iverilog, decided 2026-07-25)
- [ ] `Harness.{h,cpp}`: `tb.sv` generation (stimulus semantics per plan)
- [ ] iverilog/vvp subprocess wrapper (`-g2012`, `-I <design dir>`)
- [ ] assume-stripped sim copies vs original formal copies
- [ ] VCD scope contract honoured: DUT instance name == top module name
      (the trace loader matches `scope->name`); verified against a
      cocotb-produced VCD
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
- 2026-07-25 (WP2A): pinned `ebmc-5.6` over newer tags — it is the EBMC the
  existing results were produced with, and it demonstrably parses the whole
  benchmark set (the current pipeline verifies against it).
- 2026-07-25 (WP2A): **the plan's claim that hw-cbmc natively understands
  `(* anyseq *)` / `(* anyconst *)` was WRONG.** hw-cbmc's grammar has
  `attr_spec_list: attr_spec { init($$); }` — the base case allocates an empty
  list and throws `$1` away, so a *single*-attribute instance parses to an
  empty attribute list and `add_attributes()` then drops it entirely; with
  `(* a, b *)` only the trailing attributes survive. Present in ebmc-5.6,
  ebmc-6.0 and main (1a4ffdb). Fixed with a one-line patch under
  `third_party/patches/` applied at build time. **TODO: send upstream** and
  drop the patch once merged.
- 2026-07-25 (WP2A): widths are NOT usable from the parse tree (ranges stay
  unevaluated, `[W-1:0]`); they come from the elaborated symbol table after
  typecheck, where ports are `unsignedbv` with a resolved `width`. WP2 must
  run parse (attributes, port order, assumes) AND typecheck (widths).
- 2026-07-25 (WP2A): CBMC downloads its SAT backend — `make -C lib/cbmc/src
  minisat2-download` is required before `make -C src`, else the build fails in
  `sat/satcheck_minisat2.cpp`. `build-hw-cbmc.sh` handles it.
- 2026-07-25 (DIRECTION CHANGE, user): **Docker is dropped.** Everything is
  built and tested locally; installing third-party packages on the host is
  fine. Docker/ and artifact/ are left untouched for the frozen artifact. The
  plan's §0, §4 preamble, WP1/WP3/WP4/WP5/WP8 acceptance texts and gotchas
  10/15/17 were updated accordingly.
- 2026-07-25: nlohmann-json vendored at `smart/third_party/nlohmann/json.hpp`
  (v3.11.3) + `-I ./third_party` in `smart/Makefile`; `make compile` now works
  on the host without the apt package.
- 2026-07-25 (WP1): CMake 4.4.0 installed locally with `pip install --user
  cmake` (the host has no system cmake); build with
  `PATH=$HOME/.local/bin:$PATH cmake -B build -S .`.
- 2026-07-25 (WP1): local run needs python >= 3.10 —
  `minimal_satisfiable_assignment.py` uses a `match` statement, and the host
  default is 3.9. The venv is built with `python3.11 -m venv --clear
  otherTools/venv`. (WP5 removes this dependency entirely.)
- 2026-07-25 (WP1): `run.py clean_smart()` globbed `smart/*.txt` and deleted
  `smart/CMakeLists.txt`. Fixed with a keep-list. Anything else added to
  `smart/` with a `.txt`/`.log`/`.sl` name is at risk from the same glob.
- 2026-07-25 (WP1): `oss-cad-suite/bin/cvc5` is 1.0.1-dev and shadows the
  1.2.0 at `/usr/local/bin/cvc5` once the suite is on PATH — a silent solver
  downgrade. `run.py` now calls `prefer_system_cvc5()` and prints which one it
  uses. WP4's `--check-env` must report the cvc5 it will actually invoke.
- 2026-07-25 (WP1): `run.py --check-env` checked python modules with the
  interpreter running run.py, not the venv's, so it always reported cocotb
  missing. Fixed; also dropped the `z3` check (nothing imports z3) and added
  `iverilog`.
- 2026-07-25 (WP1): new escape hatches for A/B-ing binaries:
  `SMART_BIN=<path> python3 run.py <bench>` copies a prebuilt `smart.out` into
  place after the clean step, and `SMART_NO_COMPILE=1` makes `setup.py` skip
  its `make compile`.
- 2026-07-25 (WP3 DECISION, measured): simulation is **iverilog**, not
  Verilator — both were acceptable to the user, so the tie was broken by
  test: hierarchical assignment (`dut.free_a = $random`) drives `(* anyseq *)`
  regs with no `--public-flat-rw` and no `top__DOT__` mangling; we control the
  VCD scope tree, which retires gotcha 11; `iverilog -g2012` elaborates
  tiny_and/s27/axis_fifo and ibex_decoder (with `-I <design dir>` for its
  `` `include "ibex_pkg.sv" ``). Verilator stays a documented fallback. NOTE
  the new load-bearing contract: the generated testbench must name the DUT
  instance exactly the top module name, because `Trace::createSignal` sets
  `Signal.moduleName = scope->name` and `Module::getAllSignals` filters on it.
- 2026-07-25 (WP2): calling `verilog_languaget::typecheck` directly only works
  for designs with no submodules. Anything that instantiates another module
  (s27's `dff`, the i2c cores) trips a precondition inside verilog_synthesis.
  Elaboration must go through langapi's `language_filest`, the path ebmc
  itself uses, which typechecks modules in dependency order.
- 2026-07-25 (WP2): CBMC signals internal invariant violations by ABORTING the
  process — unacceptable in a linked library, and three benchmark designs hit
  one. `cbmc_invariants_should_throwt` (util/invariant.h) flips them to
  `invariant_failedt` exceptions; the adapter catches those and falls back to
  parse-tree ports. Any future code linking CBMC libs should do the same.
- 2026-07-25 (WP2): port ORDER differs from the oracle on all 25 non-ANSI
  designs, and hw-cbmc is right — it preserves module-header order, e.g.
  `module s27(GND,VDD,CK,G0,G1,G17,G2,G3)`, while the oracle scans the
  `input`/`output` declarations and so emits inputs first. This changes the
  order of per-port RNG draws, so same-seed traces will NOT match the legacy
  pipeline byte-for-byte once WP3/WP4 use the new frontend. Expect it during
  end-to-end parity work.
- 2026-07-25 (WP2): `params` is now every elaborated compile-time constant of
  the module (parameter and localparam), not the subset the regex could
  evaluate — hence 1279 `param-extra` entries. Deliberate: the values are
  exact, and nothing downstream needs the oracle's narrower set.
