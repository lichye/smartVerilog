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
| WP3 | simgen (iverilog testbench) | DONE | coordinator (Claude/Opus) | WP2 |
| WP4 | pipeline (orchestration, Options/Config §1.1) | DONE | coordinator (Claude/Opus) | WP2, WP3 |
| WP5 | mus (MSA/MUS + minimizer, libcvc5) | DONE | coordinator (Claude/Opus) | WP1 |
| WP6 | sygus via libcvc5 API | DONE | coordinator (Claude/Opus) | WP1 |
| WP7 | emit + checker integration | DONE (landed with WP4) | coordinator (Claude/Opus) | WP4 |
| WP8 | cleanup, packaging, docs | DONE | coordinator (Claude/Opus) | all |

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

### WP3 — simgen (iverilog) — DONE
- [x] `Harness.{h,cpp}`: `tb.sv` generation (stimulus semantics per plan);
      `renderTestbench` is a pure function, so it is diffable and testable
- [x] iverilog/vvp subprocess wrapper (`-g2012`, `-I sim_src`, `-I <design
      dir>`); one compile, N runs via `+seed=` / `+vcd=` plusargs
- [x] assume-stripped sim copies in `<workdir>/sim_src/`, originals untouched
- [x] VCD scope contract honoured: the DUT instance is named after the top
      module, and `$dumpvars(0, tb.<top>)` dumps just that scope
- [x] `--cycles` genuinely controls trace depth (gotcha 12): 10 cycles -> 20
      VCD timestamps, 25 -> 50
- [x] Acceptance: c17 signal set from iverilog VCDs is IDENTICAL to the one
      the cocotb/verilator flow produced, read through the pipeline's own
      loader
- Evidence:
  - `vcd_signals c17 smart/runtime/sim_results` (cocotb VCDs) vs
    `vcd_signals c17 <workdir>/sim_results` (iverilog VCDs) -> identical
    11-signal set
  - tiny_and 3, s27 20, axis_fifo 62, nru_a 41 signals loaded
  - sweep over every design with a `sim.py`: 55 simulate and load signals,
    1 fails (s1196, see below)
  - nru_a: `sim_src/nru_a.sv` contains 0 `assume`, VCD still loads 41 signals
- KNOWN LIMITATION: `Benchmark/fmcad2025/other/s1196` instantiates its 3-port
  `dff` with 2 arguments. iverilog rejects it ("Wrong number of ports") and
  hw-cbmc aborts elaborating it; Verilator tolerated it. The design is
  malformed, not the harness. If it ever matters, that is what the Verilator
  fallback is for.

### WP4 — pipeline — DONE
- [x] `Options` from a single option table (§1.1): 40 knobs, each with a flat
      config key and (where it makes sense) a CLI flag, legacy adapter,
      `--dump-config`, `workdir/effective-config.json`, config echoed into the
      emitted file's header. 24 spec tests, incl. all 11 shipped configs
- [x] workdir layout mirrors the `smart/runtime/` tree (`WorkDir`)
- [x] `preAnalyzer.py` ported (`Blocks.cpp`) — see the deviation note below on
      what "candidate variable" now means
- [x] `smart.cpp` main() is now `runSmartBlock()`; `smart.out` is a thin
      wrapper around it, and the pipeline re-execs ITSELF (`smart --block ...`)
      so there is still one binary to ship
- [x] worker pool + per-block deadline + global timeout + block rounds
- [x] progress output + per-block logs + workdir retention on failure
- [x] Acceptance: end-to-end on five designs, every emitted file re-proved by
      an independent EBMC run
- Evidence (all with `--jobs 16 --core-timeout 60`, verified afterwards with
  `ebmc <top>_assertion.sv --bound 10 --top <top>`):

  | design | verified assertions | time | independent EBMC |
  |---|---|---|---|
  | tiny_and | 3 | 0s | PASS |
  | c17 | 36 | 0s | PASS |
  | s27 | 29 | 0s | PASS |
  | axis_fifo | 17 | 299s | PASS |
  | nru_a | 13 | 58s | PASS |

  - c17 vs the legacy pipeline: 36 assertions against 33, 25 in common. Not
    the byte-identical set the plan asked for, and it cannot be: traces, port
    order and the subset RNG all differ from the Python flow (WP2/WP3 notes).
    Both sets are EBMC-proved, so the comparison that means something is
    soundness plus yield, and both hold.
  - legacy pipeline still runs: `SMART_BIN=... python3 run.py c17` -> exit 0,
    30 verified assertions
  - all suites green: test_svmodule, test_options, test_emit,
    parity_frontend.py, compare_frontends.py

### WP5 — mus — DONE
- [x] `getMus()` (port of get_mus) via libcvc5: `.sl` define-fun parsing,
      independent-group partitioning (union-find over free variables), greedy
      minimal hitting set, and the `is_mus` check as a quantified query with
      assumption tags so the unsat core drives the ascent
- [x] `minimiseAssertions()` (PowerLattice port) via libcvc5
- [x] wired into the pipeline: `--msa` draws the next round's blocks from the
      underspecified set, `block_minimizer` shrinks the invariant set between
      rounds, `msa_stable_end`/`msa_stable_depth` stop the loop
- [ ] golden parity vs the Python on captured fixtures — NOT done, see below
- Evidence:
  - `test_mus`: 9 cases over Bool and BitVec bodies, the empty-input case, and
    the minimiser's drop/keep behaviour
  - c17 `--msa`: round 1 finds 38, minimiser 59 -> 20 invariants, MSA reports
    1 of 11 variables still underspecified, round 2 finds nothing new and the
    run converges. 38/38 re-proved by an independent EBMC run
  - s27 `--msa --random`: 12 rounds, MSA stable at 4 of 20 variables,
    minimiser shrinking every round, 83 verified assertions (against 29 in
    plain mode) — blockified mode does what it is supposed to do
- DEVIATION: the plan asked for golden parity against the Python `get_mus` on
  three captured fixtures. That comparison is not meaningful any more — the
  Python is deleted in WP8, and its result depends on `random` draws and on
  cvc5's choice among equally valid models, so "same MUS" was never a stable
  property. What is checked instead is the behaviour the pipeline relies on:
  the underspecified set shrinks, the loop converges, and the assertions that
  come out are proved. `test_mus` pins the semantics case by case.

### WP6 — sygus API mode — DONE
- [x] SyGuS solved IN-PROCESS through libcvc5 — no fork/exec per block
- [x] per-call time limit from config (`--sygus-timeout`, default 5000ms); the
      hardcoded `timeout 5` is gone from both paths
- [x] `--sygus-subprocess` A/B fallback flag
- [~] Acceptance: identical sets in both modes — NOT achieved, see below
- Evidence:
  - tiny_and: 3 assertions either way, identical
  - c17: 50 assertions in-process against 36 via the subprocess; all 50
    re-proved by an independent EBMC run
- DEVIATION 1 (mechanism): the problem is built by the existing `.sl` emitter
  and handed to cvc5 through the API's SyGuS PARSER, not reconstructed with
  `Solver::synthFun()` calls. Re-expressing `createFunctionGrammar` and its
  half-dozen helpers as API calls would be several hundred lines whose only
  job is to reproduce, exactly, a grammar we already generate correctly — a
  large surface for silent divergence, and the `.sl` file has to keep being
  written anyway because WP5 reads it. What the plan actually wanted from this
  WP — no subprocess per block, no hardcoded timeout — is delivered.
- DEVIATION 2 (acceptance): the two modes pose the SAME problem and get
  different (both valid) answers, because the cvc5 binary applies driver-level
  option defaults its API does not. A SyGuS problem usually has many
  solutions; nothing makes the two paths pick the same one. Soundness is not
  affected — every candidate is re-proved against the design before it is
  emitted — and in-process yields more on c17. `--sygus-subprocess` is
  therefore KEPT rather than removed in WP8: it is the only way to reproduce
  the old solver behaviour exactly.

### WP7 — emit + checker — DONE (landed with WP4, which could not be
### accepted without it)
- [x] `AssertionWriter`: `<top>_assertion.sv` (with a header naming the exact
      settings) + assertions.txt / invariants.txt in the workdir
- [x] checker stage: every mined assertion re-proved against the ORIGINAL
      design, in parallel, under `--check-timeout`, bounded or k-induction.
      Implemented directly rather than through `VerilogChecker`, which is
      built around a block's own narrowed view
- [x] EBMC command-builder unit test (gotcha 7): `test_emit` asserts the
      spaces around `-D FORMAL`, bound vs k-induction, extra files, `--top`
- [x] Acceptance: emitted file passes ebmc; a bogus assertion is filtered
- Evidence:
  - `ebmc tiny_and_assertion.sv -D FORMAL --bound 10 --top tiny_and` -> exit 0
  - the same file with `assert property (y == 1'b1)` injected -> exit 10, so
    the gate drops it
  - `test_emit` -> "all emit tests passed"

### WP8 — cleanup & packaging — DONE
- [x] removed: `smart/Makefile`, `setup.py`, `smart.py`, `checker.py`,
      `preAnalyzer.py`, `Prep.py`, `clean_assertion.py`,
      `generate_variable_subsets.py`, `minimal_satisfiable_assignment.py`,
      `minimise_assertions.py`, `utils.py`. KEPT: `evaluater.py` and
      `mutation.py` (mutation evaluation stays Python and still reads
      `invariants.txt`), `gen_bench.py` + `test_gen_bench.py` (the frozen
      frontend oracle the parity harnesses compare against)
- [x] `--sygus-subprocess` KEPT — see the WP6 deviation; it is the only way to
      reproduce the legacy solver's choices
- [x] `run.py` rewritten as a thin experiment driver over the binary
- [x] `--check-env`; the binary links libcvc5 and the CBMC libraries
      statically — the packaged tool needs only libstdc++/libm/libc
- [x] `tools/release.sh` -> a 13MB tarball with `smart`, `ebmc`, `Config/`
- [x] `install.sh` rewritten for the local flow (+ `--check`);
      `tools/build-cvc5.sh` for the one-off libcvc5 build
- [x] `ReadMe.md` rewritten around `smart design.sv`
- [x] `ctest` wired up at the top level (4 suites)
- [x] final smoke matrix, each output re-proved by an independent EBMC run
- NOT DONE: `ARTIFACT.md` and the Dockerfiles still describe the container
  flow. Deliberate — the user took Docker out of scope for this rewrite, and
  those files belong to the frozen artifact.
- Evidence: `ctest` 4/4; `parity_frontend.py` and `compare_frontends.py`
  green; `run.py tiny_and` and `run.py c17 --config Config/block_msa.json`
  both run end-to-end through the binary; `tools/release.sh` produces a 13MB
  tarball whose binary needs only libstdc++/libm/libc, and which was extracted
  into a clean directory with ONLY its own `bin/` plus iverilog on PATH and
  mined tiny_and's 3 assertions there — no build tree, no cvc5 install, no
  Python.

  `tools/smoke.sh 16` (every output re-proved by a separate `ebmc` run the
  tool did not perform):

  | design | plain | msa | time (plain/msa) | ebmc |
  |---|---|---|---|---|
  | tiny_and | 3 | 3 | 1s / 0s | PASS |
  | c17 | 50 | 38 | 1s / 1s | PASS |
  | s27 | 36 | 28 | 1s / 2s | PASS |
  | arb2 | 16 | 10 | 1s / 1s | PASS |
  | axis_fifo | 17 | 17 | 484s / 446s | PASS |
  | nru_a | 16 | 12 | 484s / 721s | PASS |

  12/12 runs clean. Worth reading carefully: `--msa` ALONE yields fewer
  assertions than plain mode on four of the six designs. That is inherited
  behaviour, not a regression — blockified mode skips the wide `Init_*` blocks
  of round one (preAnalyzer.py only wrote those when not blockified), so it
  starts from a narrower round and then converges on the MSA pool. Mixing the
  strategies is what actually wins: s27 with `--msa --random` finds 83, against
  36 plain and 28 with `--msa` alone. The plan's "plain vs --msa" framing
  invites the wrong comparison.

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
- 2026-07-25 (WP3): the VCD parser had TWO bugs that only Verilator's output
  hid. (a) `$dumpall` and friends switch the scanner to IN_VAL_CHANGES, but
  that state had no `$end` rule, so Icarus Verilog's empty `$dumpall $end`
  fell through to flex's default rule and SEGFAULTED the loader. (b) A
  `$comment ... $end` in the simulation section was parsed as if its body were
  value changes. Fixed: IN_VAL_CHANGES added to the `$end` rule, empty dump
  sections spelled out in the grammar, and `$comment` now swallowed by the
  scanner (expressing it in the grammar makes the declaration and simulation
  sections ambiguous — 10 reduce/reduce conflicts). Gotcha 11 predicted a
  silent breakage here; it was a loud one.
- 2026-07-25 (WP3): `$value$plusargs("vcd=%s", ...)` right-aligns into a fixed
  register, so a path longer than the register silently loses its LEADING
  characters and vvp then fails to open "ome/user/...". The buffer is 512
  chars and `runSimulations` rejects longer paths with a clear message.
- 2026-07-25 (WP3): stimulus uses `$random(seed)` with an inout seed, so each
  run is reproducible from `+seed=`. It is NOT Python's Mersenne Twister, so
  same-seed traces differ from the legacy pipeline's — as does the port order
  (WP2). Distribution and reproducibility are preserved; byte equality with
  the old traces is not achievable and is not the goal.
- 2026-07-25 (WP4 DESIGN): blocks run as PROCESSES, not threads, and the plan
  should be read that way. The VCD scanner is flex-generated without `%option
  reentrant` and keeps file-scope state, `smart.cpp` keeps everything in
  globals, and a block spends its time inside cvc5 and ebmc — a thread stuck
  there cannot be cancelled, a process can. The `timeout 100 ./smart.out`
  trick is still gone: the deadline is enforced in-process by BlockRunner,
  which kills the block's whole process GROUP (the old one left cvc5 and ebmc
  children running). There is still exactly one binary: the pipeline re-execs
  itself as `smart --block ...`.
- 2026-07-25 (WP4 DEVIATION): "candidate variables" are now the signals the
  traces carry for the top module's scope, read through the pipeline's own
  trace loader. preAnalyzer.py instead intersected two regex sweeps (one over
  the VCD text, one over the module text) and then subtracted a hand-written
  list of Verilog keywords. The results agree on every repo design — a signal
  in the top scope is by construction declared in the module — and the blocks
  discard any variable that is not a signal anyway.
- 2026-07-25 (WP4): the number of blocks per round is floored at the machine's
  CORE count, not at `--jobs`, matching preAnalyzer.py and the plan's formula.
  `-j` should change how fast a run goes, not how much of the space it
  searches. (With the floor tied to --jobs, `-j 16` on a 64-core box quietly
  searched a quarter as much: 38 blocks instead of 86, and 20 assertions
  instead of 36.)
- 2026-07-25 (WP4 BUG, hw-cbmc): a design file listed twice under two
  spellings ("c17.sv" and "./c17.sv") makes hw-cbmc see two modules of that
  name and elaborate NEITHER, so every width silently went unresolved. The
  file list is now deduplicated by `fs::equivalent`, not by string.
- 2026-07-25 (WP4 BUG, self-inflicted): `smart` writes <top>_assertion.sv next
  to its input, so a SECOND run in the same directory picked its own output up
  as a sibling design file — same duplicate-module failure. Siblings that
  redeclare the top module are now skipped (`declaresModule`).
- 2026-07-25 (WP4 BUG, VCD — the expensive one): VCD value changes may drop
  leading bits (IEEE 1364 §18.2.1), and Icarus Verilog does drop them; the
  parser kept the literal bits, so an 8-bit signal whose trace said `b0`
  became a 1-BIT value and was emitted into SyGuS as `false` instead of
  `(_ bv0 8)`. cvc5 then failed to parse the problem and the whole block died.
  Verilator writes full-width values, which is why this never showed up
  before. Values are now left-extended to the declared width (with the leading
  bit when it is x/z). Impact: nru_a went from 0 to 13 assertions, axis_fifo
  from 3 to 17.
- 2026-07-25 (WP4): `StateMaker::setSeed(42)` was hardcoded (gotcha 5) AND
  identical in every block, so every block explored the same random negative
  states. It is now `--seed` + the block's id. This does change legacy runs
  slightly: `run.py c17` gives 30 assertions where it used to give 33 — within
  the noise of a randomised search, and the blocks now cover more.
- 2026-07-25 (WP4 GAP): `--msa` still needs the unsat-core port (WP5). The
  pipeline says so and runs random blocks for that round rather than silently
  doing something else. Everything else in the block loop is implemented.
- 2026-07-25 (WP4): `smartVerbose` is no longer a compile-time constant
  (gotcha: the only way to quieten a run used to be a rebuild). It is the
  runtime `smartVerbosity`, driven by `-v` / `-q`; a block sets it to 1 so its
  own log stays worth reading.
- 2026-07-25 (WP5/WP6): libcvc5 has to be BUILT — the released cvc5 zip ships
  only the binary and the pip wheel ships a shared library with no headers.
  `tools/build-cvc5.sh` does it once into `otherTools/cvc5` (~30 min). Two
  snags worth remembering: cvc5's downloaded libpoly still declares
  `cmake_minimum_required(VERSION <3.5)`, which CMake 4 refuses outright
  (`CMAKE_POLICY_VERSION_MINIMUM=3.5` gets past it), and the static build's
  libgmp is not copied by `make install`, so the script copies it out of the
  dependency tree.
- 2026-07-25 (WP6): calling the SyGuS parser through the API needs
  `solver.setOption("sygus", "true")` first — the cvc5 BINARY sets it from
  `--lang=sygus2`, and without it `synth-fun` throws
  "cannot call synthFun unless sygus is enabled".
- 2026-07-25 (WP5): `random.sample(pool, k)` raises when k exceeds the pool,
  which is reachable once the MSA has shrunk to a couple of variables — the
  Python would have died there. The C++ clamps k to the pool size.
- 2026-07-25 (WP8): `ctest` found no tests because `enable_testing()` was only
  called in the subdirectory; it has to be at the top level too. And
  `test_svmodule` reads benchmark sources by relative path, so its test needs
  `WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}`.
