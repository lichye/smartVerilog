# PLAN: SMART as a single C++ command-line binary

**Branch:** `new-interface` · **Plan written:** 2026-07-25 · **Owner:** magna

## 0. Goal

Replace the Python orchestration pipeline with one compiled binary:

```
./smart cat.sv                 # everything automatic
# -> writes cat_assertion.sv   (original design + formally verified
#                               `assert property (...)` lines before endmodule)
```

The user provides **one** SystemVerilog file. It may contain immediate
`assume(...)` statements and `(* anyseq *)` / `(* anyconst *)` free registers.
The binary parses it, generates random simulation traces with Icarus Verilog,
mines candidate SVA via SyGuS (cvc5), verifies them with EBMC, and emits
`<top>_assertion.sv`.

**Decisions already made (do not relitigate):**
- Mutation evaluation (`evaluater.py`, `mutation.py`) does NOT go into the
  binary. It stays as Python experiment tooling in the repo, unchanged.
- **The Verilog frontend is hw-cbmc's, not ours.** We do NOT write a
  SystemVerilog parser (regex or otherwise — SV is not a regular language;
  hand-rolling is a maintenance sink). hw-cbmc (the project EBMC ships from,
  BSD-3-Clause) is added as a **git submodule under `third_party/hw-cbmc`**,
  built as part of our build, and its Verilog frontend produces the module
  model we consume. Rationale: EBMC is already our verification backend, so
  using the same toolchain for parsing gives one dependency, consistent
  semantics, and — critically — **upstream maintains the frontend for us**.
  See WP2A/WP2. (Correction from the WP2A spike: hw-cbmc understands `assume`
  natively, but `(* anyseq *)` / `(* anyconst *)` needed a one-line upstream
  grammar fix, carried as a patch — see WP2A "Recorded decisions".)
- The simulator and EBMC remain external tools, both driven as subprocesses.
  Simulation is **Icarus Verilog** (`iverilog` + `vvp`) — see WP3 for the
  measured rationale; cocotb is gone from the tool path. EBMC is now also built from the submodule (single source of
  truth for the CBMC toolchain) but still invoked as a subprocess for
  verification (no EBMC C++ API to link).
- cvc5 is used via **libcvc5 C++ API** (the Python side already uses
  `cvc5.pythonic`, so this unifies the solver story; no z3 in the binary).
  It has to be BUILT — the released zip ships only the binary and the pip
  wheel ships a library without headers. `tools/build-cvc5.sh`, one-off.
- Build system: **CMake**.
- cocotb / venv / all Python leaves the runtime path of the tool.
- **Build and test on the host, not in Docker (2026-07-25).** Every WP's
  acceptance must be reproducible with a plain local build. Third-party
  dependencies installed on the host (verilator, libcvc5, ...) are entirely
  fine — the goal is a directly testable local environment, not a
  zero-dependency one. Two of the old container-only deps are already gone:
  EBMC now comes from the submodule and nlohmann-json is vendored under
  `smart/third_party/`. Docker/ and artifact/ stay in the repo for the frozen
  artifact, but no WP targets them and no acceptance criterion may require
  them.
- The regex frontend committed in WP2 (`smart/src/frontend/SVModule.*`,
  `gen_bench.py`) is DEMOTED to (a) the interface contract — the `ModuleInfo`
  fields the hw-cbmc adapter must fill — and (b) a cross-check oracle for the
  parity harness. It is NOT the shipping parser. Do not extend it.

## 1. CLI contract (freeze this first)

```
smart <design.sv> [options]
  -o, --output FILE       default: <top>_assertion.sv next to input
  --top NAME              default: file stem
  --assume EXPR           repeatable; injected as immediate assumes (formal only)
  --clock NAME            default: auto-guess clk/clock/clk_i/i_clk, print choice
  --reset NAME[:0|1]      default: auto-guess rst/reset/rst_n/... (:active level)
  --cycles N              trace depth, default 10
  --traces N              number of traces, default 3
  --seed N                base RNG seed, default 42 (trace i uses seed+i)
  --bound N               EBMC bound, default 10; --unbounded uses --k-induction
  --refine N              CEGIS refinement depth, default 5
  -j, --jobs N            parallel synthesis/check workers, default hw_concurrency
  --timeout SECS          global wall clock, default 43200
  --core-timeout SECS     per-SyGuS-block timeout, default 100
  --blockified / --msa / --random   MAPminer block strategy (defaults off = plain SMART)
  --config FILE           optional JSON overriding any of the above (new flat schema;
                          MUST also accept the legacy Config/*.json schema, including
                          the misspelled keys `Threadhold` and `Nagative_state_number`)
  --workdir DIR           default: ./smart-work-<top> (kept on failure, printed)
  --keep-work             keep workdir on success too
  --check-env             report iverilog/ebmc/cvc5-lib versions and exit
  -v / -q                 verbosity (replaces compile-time smartVerbose in setups.h)
```

Exit codes: 0 = success (assertions emitted, possibly zero — print count),
1 = user error (bad input file/module), 2 = environment error (missing tool),
3 = pipeline failure (simulation/synthesis/check crashed). Never exit 0 on
failure (old run.py bug class).

### 1.1 Configuration model: config file AND command line, fully equivalent

Experiments must be drivable both ways — this is a hard requirement:

- **Every experiment knob has both a config key and a CLI flag.** The flag
  table above IS the schema: each option maps 1:1 to a key in the new flat
  JSON config (`cycles`, `traces`, `seed`, `bound`, `jobs`, `core_timeout`,
  `blockified`, `msa`, `random`, `k_size`, `block_size`, `msa_stable_depth`,
  `refinement_depth`, `sygus_timeout_ms`, `check_timeout`, ... — WP4 keeps
  the definitive list in one table in `Options.h` so config and CLI can't
  drift apart; generate both parsers from that single table).
- **Precedence:** CLI flag > `--config` file > built-in default. Repeatable
  runs of a paper experiment use only a config file
  (`smart c17.sv --config Config/block_msa.json`); quick interactive tweaks
  use flags on top of it (`--jobs 4` overriding the config's 16).
- **Legacy configs keep working:** the adapter in WP4 maps the existing
  nested `Config/*.json` schema (Workflow/Blockified_settings/SMART_settings/
  Checker_settings, incl. misspelled keys) onto the flat schema, so all 11
  shipped configs and the artifact scripts remain valid experiment
  definitions. Keys that only concern the retired Python layer
  (`Evaluation`, `Evaluation_settings`) are accepted and ignored with a
  notice (mutation evaluation is external Python now).
- **Reproducibility aid:** `--dump-config` prints the fully resolved
  effective configuration as new-schema JSON (after file+flags merge) and
  exits; the same JSON is always written to `workdir/effective-config.json`
  and echoed into the result header of `<top>_assertion.sv` as a comment
  block, so any published result names its exact experiment settings.

Multi-file designs: extra `.sv` dependency files may sit next to the input;
they are compiled into the simulation and formal runs but only the top module
is mined. `smart a.sv b.sv --top a` should work (first file or --top decides).

## 2. Architecture

```
smart (single binary)
 ├─ frontend/   ModuleInfo{ports,params,freeRegs,assumes} — FILLED by the
 │              hw-cbmc adapter (WP2); assume strip/inject (textual, WP2).
 │              SVModule.* regex kept only as oracle/fallback.
 ├─ simgen/     testbench generation + iverilog/vvp subprocess + VCD             [WP3]
 ├─ pipeline/   orchestration: workdir, stages, thread pool, config              [WP4]
 ├─ mus/        MSA/MUS via libcvc5 (port of minimal_satisfiable_assignment.py)  [WP5]
 ├─ existing (reuse, refactor to library):
 │    parser/  VCD flex/bison parser, Smt/Sygus/Verilog expr parsers
 │    tracer/  Module/Trace/State/Value
 │    sygus/   SyGuSGenerater (WP6 converts subprocess cvc5 -> libcvc5), StateMaker
 │    helper/  VerilogChecker (EBMC subprocess), Timer, Utils
 └─ emit/      <top>_assertion.sv writer                                          [WP7]

third_party/
 └─ hw-cbmc/   git submodule (BSD-3); nested submodule: cbmc. Built by CMake.
               Provides: EBMC binary (verification) + the Verilog frontend
               (libverilog + CBMC util libs) that WP2 links against.          [WP2A]
```

Workdir layout must stay byte-compatible with today's `smart/runtime/` tree
until WP4 is done, because the reused C++ core hardcodes relative paths
(`runtime/verilog/<top>.sv`, `runtime/sim_results/simN.vcd`,
`runtime/variables/`, `runtime/formal/`, `runtime/SygusResult.sl`).
Grep `runtime/` in `smart/src/**` before changing any path.

## 3. Behavioral reference implementations (the spec)

The Python files below are the executable spec. Port behavior, not code.

| Behavior | Reference | Notes |
|---|---|---|
| ModuleInfo field contract (what ports/params/freeRegs/assume flags the pipeline needs) + assume strip/inject + clock/reset guessing | `smart/src/frontend/SVModule.{h,cpp}` + `smart/src/python/gen_bench.py` + `test_gen_bench.py` | The regex impl is the CONTRACT and the parity ORACLE, not the shipping parser. WP2 fills the same `ModuleInfo` from hw-cbmc's frontend and must match this oracle on the 56 repo designs (differences that are genuinely the regex's fault are documented, not forced). |
| Trace collection flow | `smart/setup.py` | dir setup lines 85-133; sim loop 200-211; the `_a`/`_assume` VCD rename (52-62) is legacy — do NOT port; single-module flow makes it obsolete. |
| Synthesis loop, block scheduling, timeout semantics | `smart/smart.py` | `runBlockSmart()` 42-126, `GenerateNewBlocks()` 128-203 (MSA/Random/mixed block generation with k = round(2.7+k_size*log10(V)), n = max(Block_size*V^0.9, cores)). |
| MUS/MSA | `smart/src/python/minimal_satisfiable_assignment.py` (`get_mus(v_file, a_file, timeout)`) + `utils.py` | Uses cvc5.pythonic with produce-unsat-cores + minimal-unsat-cores; port to libcvc5. |
| Assertion set minimization | `smart/src/python/minimise_assertions.py` (`run_minimisation(in.sl, out.sl, timeout)`) | PowerLattice over define-funs; cvc5.pythonic. Port in WP5 (same solver setup). |
| Formal check of mined assertions | `smart/checker.py` | Inject one assertion before `endmodule` of top module (write_assertion_file 12-45), run `ebmc file.sv --bound N --top T` or `--k-induction`, returncode 0 = verified, 124 = timeout. 180s per-assertion timeout (make it a flag, default 180). |
| Candidate variable pre-analysis | `smart/src/python/preAnalyzer.py` | Extracts variables per module, writes initial `runtime/variables/` subsets. |
| SV preprocessing | `smart/src/python/Prep.py` | NOTE: its clock-copy-variable machinery is DEAD CODE (`check_clock_signal` always returns False, lines 4-9). Effectively Prep only re-splits modules into files. Do not port the copy machinery; just place each module's source into the workdir. |
| smart core per-block run | `smart.cpp` | argc==8 contract; WP4 turns `main()` into a library entry `runSmartBlock(BlockJob const&)` called from a thread pool instead of `timeout 100 ./smart.out ...` subprocesses. Keep `smart.out` thin main() wrapper for debugging. |

## 4. Work packages

Each WP states: deliverable, references, steps, acceptance. WPs are sized for
one agent each. **Dependency order:** WP2A (submodule + build) is now on the
critical path — WP1 and WP2 both need it. **Parallelizable groups:** {WP1,
WP2A} first; then {WP2, WP3, WP5, WP6}; WP4 after WP2+WP3; WP7+WP8 last. Build
and run everything **on the host**; install whatever third-party packages a WP
needs locally and record the install command in that WP's evidence.

### WP1 — CMake build (no behavior change)
- **Deliverable:** `smart/CMakeLists.txt` building today's `smart.out` and
  the new `smart` binary target skeleton; top-level `CMakeLists.txt`.
- **Reference:** `smart/Makefile` (flex `-P VCDParser`, bison `--defines`,
  include dirs, c++17; see lines 1-70).
- Steps: FindFLEX/FindBISON with the same prefix/output names into
  `${CMAKE_BINARY_DIR}/generated`; `find_package(cvc5)` (the host has the
  cvc5 1.2.0 *binary* only — the static zip does NOT ship the C++ lib, so a
  local libcvc5 install or build-from-source is needed; coordinate with WP6);
  nlohmann-json is DONE — vendored at `smart/third_party/nlohmann/json.hpp`.
- Keep the old Makefile working until WP8 removes it (setup.py calls
  `make compile`).
- Also add/build the `hw-cbmc` submodule targets: a custom target that runs
  `third_party/build-hw-cbmc.sh` (WP2A) and exposes the static libs, the
  include dirs, the `LOCAL_IREP_IDS` define and the `ebmc` path listed in
  WP2A's recorded decisions.
- **Acceptance:** on the host, `cmake -B build && cmake --build build`
  produces a `smart.out` that passes an existing benchmark run
  (`python run.py c17`) when copied in place of the make-built one; and the
  hw-cbmc submodule builds and its `ebmc` runs `--version`.

### WP2A — hw-cbmc submodule + build integration (critical path)
- **Deliverable:** `third_party/hw-cbmc` git submodule pinned to a specific
  tag/commit (pick the release matching EBMC 5.6, our current backend, unless
  a newer tag is deliberately chosen — record the choice); `.gitmodules`;
  build steps that fetch nested submodules (`git submodule update
  --init --recursive` — hw-cbmc itself vendors cbmc as a submodule) and build
  it.
- **Build facts (verified from upstream):** BSD-3-Clause; build is
  `git submodule update --init --recursive` then `cd src && make` (compiles
  the bundled cbmc automatically); needs flex + bison + a C++ toolchain;
  produces the `ebmc` binary under `src/ebmc/` and static libraries for the
  Verilog frontend + CBMC util. Building is slow (compiles much of CBMC) — it
  is a one-off, not a per-run step.
- Steps: add the submodule; provide a build script that inits and builds it
  (so the `ebmc` binary and the linkable libs come from ONE source instead of
  an apt/.deb install); export to the smart build: the include dirs (`third_party/hw-cbmc/src`,
  `third_party/hw-cbmc/lib/cbmc/src`), the static libs needed to link the
  Verilog frontend (at minimum `libverilog.a` + `libbigint.a`/`libutil.a` +
  `libsolvers.a`/`liblangapi.a` — determine the exact set by trial-link), and
  the built `ebmc` path for the checker.
- **Decision to record for WP2 (link vs subprocess):** two ways to consume
  the frontend, pick per what actually links cleanly:
  - **Link (preferred):** link `libverilog` + CBMC util into `smart`, call
    `verilog_languaget::parse()/typecheck()`, walk `get_parse_tree()` /
    symbol table. Structured, no text parsing, exact widths. Risk: CBMC's
    `.a`s are not a curated public API; link order / include coupling is
    fiddly (irept, message handlers, cmdline). Spike this first.
  - **Subprocess fallback:** run `ebmc --show-parse` / `--show-varmap` on the
    design and parse the dump. Lighter to wire, but the text format is not a
    stability contract. Use only if the link spike is too costly.
- **Acceptance:** on the host, from a clean checkout,
  `git submodule update --init --recursive` + the build produces a working
  `ebmc` AND either (link path) a trivial C++ program that links libverilog
  and prints a parsed module's port count, or (subprocess path) a documented
  `ebmc --show-parse`/`--show-varmap` invocation whose output contains the
  ports/params/attributes WP2 needs. Document which path was chosen and why.

#### Recorded decisions (WP2A outcome, 2026-07-25)

- **Pin: `ebmc-5.6`** (`9b402aa7`), matching the EBMC the current results were
  produced with, per the default in this plan. `third_party/hw-cbmc`, nested
  `lib/cbmc` at `3c915ebe`.
- **Mechanism: LINK, not subprocess.** The spike
  (`smart/src/frontend/hwcbmc_spike.cpp`) links cleanly and reads the parse
  tree directly. Recorded link set (order matters, inside
  `-Wl,--start-group ... --end-group`):
  `src/verilog/verilog.a`, `src/temporal-logic/temporal-logic.a`,
  `src/trans-word-level/trans-word-level.a`,
  `lib/cbmc/src/{langapi,solvers,util,big-int,json}/*.a`;
  includes `-I third_party/hw-cbmc/src -I third_party/hw-cbmc/lib/cbmc/src`;
  and — mandatory — `-D'LOCAL_IREP_IDS=<hw_cbmc_irep_ids.h>'`, without which
  our irep ids disagree with the libraries'.
- **Where each ModuleInfo field comes from:**
  - ports (order, direction, name), params, `hasAssume`, and the
    anyseq/anyconst attributes: the **parse tree**
    (`verilog_languaget::parse()` + `get_parse_tree()`), which also runs the
    preprocessor, so `` `define ``/`` `include `` designs work.
  - **widths: the elaborated symbol table**, not the parse tree. The parse
    tree carries unevaluated ranges (`[W-1:0]` stays an expression); after
    `typecheck` the port symbols are `unsignedbv` with the resolved `width`,
    plus `input`/`output` flags and `#name`. This is strictly better than the
    regex oracle and resolves plan gotcha 13.
- **Upstream bug found and patched.** hw-cbmc's grammar discards
  single-attribute instances, so `(* anyseq *) reg x;` parsed to an *empty*
  attribute list — the one thing WP2 most needs from hw-cbmc was invisible.
  Present in `ebmc-5.6`, `ebmc-6.0` and `main`. Fixed by a one-line patch in
  `third_party/patches/`, applied by `third_party/apply-patches.sh` (called
  from `third_party/build-hw-cbmc.sh`). **Send this upstream**; drop the patch
  when it merges. Details: `third_party/patches/README.md`.
- **Build:** `third_party/build-hw-cbmc.sh [jobs]` — applies patches, does
  CBMC's `minisat2-download`, then `make -C src`. ~10 min on 32 cores.

### WP2 — frontend adapter: fill ModuleInfo from hw-cbmc
- **Deliverable:** `smart/src/frontend/HwcbmcFrontend.{h,cpp}` that produces
  the SAME `ModuleInfo` struct already defined in `SVModule.h`
  (ports{dir,width,name}, params, freeRegs{kind,width,name}, hasAssume) from a
  `.sv` file + top name, via the WP2A-chosen mechanism (link or subprocess).
  `smart --dump-frontend <file>` prints the canonical JSON (reuse
  `dumpJson`).
- **Keep from the existing WP2 code (do NOT rewrite):** the `ModuleInfo`
  struct and `dumpJson` (interface + JSON contract); `stripAssumes` and
  `injectAssumes` — these are TEXTUAL source surgery (remove/insert assume
  statements while preserving the user's formatting for re-emission); a real
  AST would force lossy pretty-printing, so text surgery stays. With hw-cbmc
  you additionally get exact assume source locations from the parse tree —
  use them to make stripping precise instead of the current scanner if it
  proves more robust, but the textual re-emit contract is unchanged.
  `guessClock`/`guessReset` stay as-is (pure functions over ModuleInfo).
- **anyseq/anyconst:** hw-cbmc understands these natively — extract them from
  the parse tree/symbol table (they are the `freeRegs` with kind anyseq/
  anyconst). This is the main thing the regex was approximating.
- **Acceptance (met):** `tools/compare_frontends.py` walks the 56 designs and
  classifies every divergence; it exits 0 only when each one is either benign
  (hw-cbmc strictly better) or a recorded limitation. `parity_frontend.py`
  keeps the frozen oracle honest and still reports "parity OK: 56 designs".
  Net goal held: the pipeline gets a MORE correct ModuleInfo than the regex.
- **Elaboration is mandatory, and it must go through `language_filest`.**
  Calling `verilog_languaget::typecheck` on the top module alone trips an
  invariant inside verilog_synthesis for any design with submodules. Also set
  `cbmc_invariants_should_throwt` around it: CBMC aborts the process on
  invariant violations, and three benchmark designs hit one — with the guard
  they degrade to parse-tree ports instead.

### WP3 — simgen: iverilog testbench instead of cocotb
- **Simulator decision (2026-07-25, measured):** **Icarus Verilog**, not
  Verilator. Both were on the table; iverilog wins on the three things this
  pipeline actually needs:
  1. free registers are driven by plain hierarchical assignment
     (`dut_inst.free_a = $random;`) — no `--public-flat-rw`, no
     `rootp->top__DOT__name` mangling;
  2. we control the VCD scope layout from our own testbench, which retires
     gotcha 11 (Verilator's extra `TOP` scope) entirely;
  3. no C++ harness to compile — the generated artifact is readable Verilog.
  It also handles the hardest SV in the benchmark set (`ibex_decoder`,
  `ibex_id_stage`) given `-I <design dir>` for their `` `include "ibex_pkg.sv" ``.
  Verified: `iverilog -g2012` elaborates tiny_and / s27 / axis_fifo /
  ibex_decoder, and hierarchical assignment to `(* anyseq *)` regs shows up in
  the VCD. Verilator stays a documented fallback for designs iverilog cannot
  elaborate.
- **Deliverable:** `smart/src/simgen/Harness.{h,cpp}`: given the WP2 model +
  run options, (a) write `tb.sv` (a generated Verilog testbench),
  (b) run `iverilog -g2012 -I <design dir> -o sim <design.sv> tb.sv`,
  (c) run `vvp sim` `traces` times with seeds seed+i, moving each `dump.vcd`
  to `runtime/sim_results/sim<i>.vcd`.
- **VCD scope contract (load-bearing):** `Trace::createSignal` sets
  `Signal.moduleName = scope->name`, and `Module::getAllSignals` filters on
  `moduleName == <top>`. The VCD scope name is the *instance* name, so the
  testbench MUST instantiate the DUT with the instance name equal to the top
  module name (`tiny_and tiny_and (...)`) and dump it
  (`$dumpfile("dump.vcd"); $dumpvars(0, tb);`). Get this wrong and the loader
  silently finds zero signals.
- **Stimulus semantics (from the generated sim.py in gen_bench.render_sim_py;
  the semantics carry over unchanged, only the simulator differs):**
  seed the RNG; set each anyconst reg once to draw(spec); if clocked: start
  clock (toggle every step), apply reset for reset.cycles cycles then
  deassert; per cycle assign every non-clock non-reset input and every anyseq
  reg a fresh draw(spec) (uniform 0..2^width-1, honoring --config per-input
  max/values/const overrides), then advance one clock; unclocked designs
  advance time steps instead. Trace depth = cycles (default 10 — this
  finally makes the config field `Depth_Trace` real; wire it up).
- Driving internal `(* anyseq *)` / `(* anyconst *)` regs from the testbench:
  hierarchical assignment, `<top>.<name> = <draw>;` — anyconst once in the
  initial block, anyseq every cycle alongside the inputs. Simulation copies of
  the RTL must be assume-stripped (WP2 stripAssumes) — write the stripped
  copies into `workdir/sim_src/`, originals into `runtime/verilog/` and
  `runtime/formal/`.
- **VCD compatibility:** today's VCDs come from cocotb driving **verilator**
  (`sim.py`: `os.getenv("SIM", "verilator")`), so the flex/bison parser is
  currently fed verilator's layout. iverilog's `$dumpvars` emits the same VCD
  format; what changes is the scope tree, which the contract above pins down.
  Still validate against a cocotb-produced VCD from the current pipeline (run
  `python run.py tiny_and` to get one) and compare signal sets.
- **Acceptance:** for `tiny_and`, `s27` (non-ANSI, unclocked),
  `axis_fifo` (clocked+reset, parameterized widths), and `nru_a`
  (anyseq/anyconst/assume): the generated testbench elaborates under
  `iverilog -g2012`, `vvp` produces N VCDs, and `Module::addTracesfromDir`
  loads them with the same signal set as the cocotb flow (write a small
  dump-signals debug flag to compare).

### WP4 — pipeline: orchestration in C++
- **Deliverable:** `smart/src/pipeline/Pipeline.{h,cpp}`, `Options.{h,cpp}`
  (CLI parsing — use a small vendored arg library or hand-rolled; no boost),
  `Config.{h,cpp}` (new flat JSON + legacy schema adapter incl. misspelled
  keys), `WorkDir.{h,cpp}`. Options/Config implement §1.1: one option table
  drives both parsers, precedence CLI > config > defaults, `--dump-config`,
  `workdir/effective-config.json`, config echo in the output header.
- Stages: frontend -> workdir setup (mirror setup.py's tree) -> simgen ->
  preAnalysis (port `preAnalyzer.py`: initial variable subsets into
  `runtime/variables/`) -> synthesis loop -> checker -> emit.
- Synthesis loop = port of `smart.py`: thread pool (std::async or a simple
  worker pool) running `runSmartBlock()` (refactored from `smart.cpp` main —
  keep its internal logic untouched: trace load, modifySignals, sygus,
  checkExprSafety, refinement loop); per-block wall timeout `--core-timeout`
  enforced by the pool (the old `timeout 100` subprocess trick disappears —
  blocks must check a deadline/cancellation flag between refinement
  iterations; cvc5 API calls get per-call timeouts, see WP6); global
  `--timeout` checked between rounds; block strategies MSA/Random/mixed per
  `GenerateNewBlocks()` including the k/n formulas and `MSA_stable_depth`
  early-stop.
- Progress output: one line per round (`[hh:mm:ss] blocks 12/40 done,
  assertions 7`), plus per-block logs under `workdir/logs/` (same idea as the
  current runtime/logs). `-v` streams block logs to stderr.
- On failure: leave workdir, print its path and the failing stage. On
  success: delete workdir unless `--keep-work`.
- **Acceptance:** `smart Benchmark/user/tiny_and/tiny_and.sv` end-to-end
  produces >=1 verified assertion in `tiny_and_assertion.sv`;
  `smart Benchmark/fmcad2025/c17/c17.sv` produces an assertion set equal (as
  a set of normalized strings) to the legacy `python run.py c17` run's
  `invariants.txt` with the same seed/cycles/bound settings.

### WP5 — mus: MSA/MUS + minimizer via libcvc5
- **Deliverable:** `smart/src/mus/Mus.{h,cpp}` exposing
  `getMus(variablesFile, sygusResultFile, timeoutSecs)` and
  `minimiseAssertions(inSl, outSl, timeoutSecs)`.
- **Reference:** `minimal_satisfiable_assignment.py` (+`utils.py` parsing of
  the `.sl`/variables formats) and `minimise_assertions.py` (PowerLattice —
  sort by complexity, walk sub-lattices, SAT-check implication with cvc5;
  solver options produce-models/produce-unsat-cores/minimal-unsat-cores).
- The `.sl` define-fun parsing can reuse the existing C++
  `SmtFunctionParser`/`SygusExpr` in `src/parser/` — check before writing a
  new parser.
- **Acceptance:** golden tests — run the Python `get_mus`/`run_minimisation`
  on 3 captured `runtime/SygusResult.sl` fixtures from real runs (commit
  under `smart/test/fixtures/`), assert the C++ returns the
  same MUS variable sets / minimized cores.

### WP6 — sygus: libcvc5 instead of subprocess
- **Deliverable:** `SyGuSGenerater` gains a mode that builds the SyGuS
  problem through the cvc5 C++ API (Solver::synthFun etc.) instead of writing
  `.sl` + `system("timeout 5 cvc5 --lang=sygus2 ...")`
  (`SyGuSGenerater.cpp:255`). Per-call timeout via cvc5 resource/time limits
  (`tlimit-per`), default 5000ms, from `--config`/flag, no more hardcoded 5s.
- Keep `.sl` emission behind `--keep-work` for debuggability (the MUS/
  minimizer/WP5 also read `SygusResult.sl` — they consume the *result* file,
  which must still be written).
- **Acceptance:** c17 + tiny_and runs produce the same assertion sets as the
  subprocess mode (add a temporary `--sygus-subprocess` fallback flag for
  A/B; remove in WP8).

### WP7 — emit + checker integration
- **Deliverable:** `smart/src/emit/AssertionWriter.{h,cpp}`: take verified
  assertion strings, inject `    assert property (<a>);` lines before the
  top module's `endmodule` in a copy of the ORIGINAL user file (assumes
  intact), write `<top>_assertion.sv`. Also write plain-text
  `assertions.txt`/`invariants.txt` into the workdir for tooling continuity.
- Checker stage: port `checker.py` semantics onto the existing
  `VerilogChecker` (EBMC subprocess): per-assertion injected copy, parallel
  jobs, 180s default per-assertion timeout, bounded (`--bound N`) vs
  `--k-induction`; verified subset only goes to the emitter. Note
  `VerilogChecker::runEBMC` had a missing-space bug (fixed on this branch,
  `VerilogChecker.cpp:219`) — write a unit test for command construction.
- **Acceptance:** `cat_assertion.sv` from a tiny_and run passes
  `ebmc tiny_and_assertion.sv --bound 10 --top tiny_and` with exit 0; a file
  with an injected bogus assertion fails it (sanity that the checker gate
  actually filters).

### WP8 — cleanup, packaging, docs
- Remove: Makefile build path (after WP1 parity), `--sygus-subprocess`,
  setup.py/smart.py/checker.py from the tool flow (they stay in git history;
  `evaluater.py`/`mutation.py` STAY — mutation evaluation remains Python and
  must keep working against the new binary's output: it reads
  `invariants.txt`, which WP7 still writes).
- `smart --check-env`; static-link what's possible (cvc5, stdc++); strip
  binary; `make release` equivalent producing a tarball.
- Update `ReadMe.md` and `install.sh` (local build + the third-party
  packages the tool needs; drop venv/cocotb from the tool path — keep Python
  only for the experiment layer). `Docker/` and `artifact/` are left alone:
  they belong to the frozen artifact.
- Final regression: `artifact/run_smoke.sh` equivalent driving the new
  binary on {tiny_and, c17, s27, arb2, axis_fifo, nru_a} × {plain, --msa}.

## 5. Gotchas (learned from the current code — read before coding)

1. `Prep.py`'s clocked-copy-variable feature is dead code; don't port it.
2. The `_a`/`_assume` VCD-rename hack (`setup.py:52-62`) is replaced by
   assume-stripping; don't port it.
3. `checker.py` returns Python *sets* like `{prop, "verified"}` — an
   accident that happens to work; use proper structs.
4. Legacy config keys are misspelled (`Threadhold`, `Nagative_state_number`)
   and consumed as-is by `smart.cpp:82-84`; the new Config must accept both
   spellings (warn on the old ones).
5. `StateMaker::setSeed(42)` is hardcoded (`smart.cpp:91`) — route `--seed`.
6. EBMC exit code 124 = timeout(1) kill in the old flow; with in-process
   deadlines use your own timeout status enum.
7. EBMC needs ` -D FORMAL ` (with spaces) and `--top`; see the fixed bug in
   `VerilogChecker.cpp:219` — add a command-builder unit test.
8. The VCD parser is flex/bison with prefix `VCDParser` — CMake must
   reproduce the exact flex `-P` prefix or the symbols won't link.
9. `smart.cpp:32` has a stale default `config_path = "User/config.ini"` —
   delete when refactoring main() into `runSmartBlock`.
10. SUPERSEDED (2026-07-25): build/test happens on the HOST. `nlohmann/json.hpp`
    is now vendored at `smart/third_party/nlohmann/json.hpp` (v3.11.3) and
    `smart/Makefile` has `-I ./third_party`; EBMC comes from the submodule.
    Installed locally since: iverilog 13.0 + verilator 5.011 + yosys, via
    `otherTools/oss-cad-suite` (the tarball install.sh already used). Still to
    install: libcvc5 C++ dev (WP5/WP6 — only the cvc5 *binary* is present, and
    note `oss-cad-suite/bin/cvc5` is 1.0.1-dev and SHADOWS the 1.2.0 at
    `/usr/local/bin/cvc5` once the suite is on PATH).
11. RETIRED by the WP3 iverilog decision (Verilator's `TOP` wrapper scope is
    no longer in the picture). Its replacement is the WP3 "VCD scope contract":
    the DUT instance name must equal the top module name, because the trace
    loader matches signals on `scope->name == <top>`.
12. Trace depth 10 was historically hardcoded ("Depth_Trace: Default is 10
    and hard to change") — after WP3 it must genuinely follow `--cycles`.
13. Widths that don't resolve to integers (complex param exprs) fall back to
    width 1 + warning in the regex oracle; hw-cbmc should resolve most of
    these correctly — where it does, prefer its width and note the oracle diff.
14. `run.py --list-benchmarks / --check-env` UX from this branch should have
    equivalents in the binary (`--check-env` is in WP4/WP8 scope);
    `--check-env` must also report the hw-cbmc-built EBMC version.
15. hw-cbmc vendors cbmc as a NESTED submodule — always
    `git submodule update --init --recursive`. A plain `--init` silently
    yields an unbuildable hw-cbmc. CBMC additionally *downloads* its SAT
    backend: `make -C lib/cbmc/src minisat2-download` (done by
    `third_party/build-hw-cbmc.sh`); without it the build dies in
    `sat/satcheck_minisat2.cpp`.
16. hw-cbmc's static libs are NOT a curated public API. If the WP2A link
    spike drags, fall back to `ebmc --show-parse`/`--show-varmap` subprocess
    rather than fighting CBMC's link graph — the ModuleInfo contract is the
    same either way.
17. Building hw-cbmc compiles much of CBMC and is slow (~10 min on 32 cores);
    it is a one-off setup step, never part of a run. The submodule commit is
    pinned so incremental rebuilds stay cheap.
18. std::regex on libstdc++ stack-overflows on large ISCAS netlists (WP2
    discovery); the regex oracle avoids it by hand-scanning. Any new C++ that
    must scan whole files: do NOT use std::regex over the full text.
19. The regex-oracle spec bug fixed at 198a0aa (missing `\b` after type
    keywords → `regfile_we_o` truncated to `file_we_o`) shows why the regex
    is being retired; hw-cbmc won't have this class of bug.

## 6. Validation strategy (cross-cutting)

- **Frontend parity:** `tools/parity_frontend.py` compares the hw-cbmc-backed
  `--dump-frontend` against the gen_bench.py/SVModule regex ORACLE on the 56
  repo designs. Goal is not byte-identity but "hw-cbmc is at least as correct":
  each diff is either fixed or documented as a regex limitation hw-cbmc gets
  right. The regex oracle is frozen (do not extend it).
- **End-to-end parity:** same-seed assertion-set comparison vs the legacy
  Python pipeline on {tiny_and, c17, s27, arb2} before removing anything.
- **Fixtures:** capture `.sl`/VCD fixtures from real runs into
  `smart/test/fixtures/` early (WP5/WP3 depend on them).
- Every WP lands as a separate PR/commit on `new-interface` with its
  acceptance evidence (command + output) in the commit message.

## 7. Suggested agent assignment

| Agent | WPs | Can start |
|---|---|---|
| A | WP2A hw-cbmc submodule+build, then WP1 CMake | immediately (critical path) |
| B | WP2 frontend adapter | after WP2A picks link-vs-subprocess |
| C | WP3 simgen | immediately (consumes ModuleInfo; regex oracle fills it meanwhile) |
| D | WP5 mus + WP6 sygus | after WP1 (needs libcvc5 in build) |
| E | WP4 pipeline | after WP2+WP3 |
| F | WP7 emit + WP8 cleanup | last |

Note: WP2A is the new critical-path item. WP3 does not block on it (it takes
a `ModuleInfo`, which the regex oracle can supply during development); WP2's
adapter and WP1's link step both need WP2A's chosen mechanism + libs.


## 8. Outcome (2026-07-25)

Every work package is landed. `smart design.sv` runs the whole flow in one
binary: hw-cbmc frontend, iverilog simulation, cvc5 synthesis, EBMC checking,
emission. The Python orchestration layer is deleted; `evaluater.py` /
`mutation.py` (mutation evaluation) and `gen_bench.py` (the frozen frontend
oracle) are what remain, exactly as the plan intended.

The four things that turned out differently from the plan, all recorded in the
progress doc with their reasoning:

1. **Blocks are processes, not threads.** The VCD scanner is not reentrant and
   the block code is built on globals; more to the point, a block that hangs
   is hanging inside cvc5 or EBMC, where a thread cannot be cancelled. The
   `timeout 100 ./smart.out` trick is still gone — the deadline is ours now,
   and it kills the process group rather than just the leader.
2. **Assertion-set parity with the legacy pipeline is not achievable**, and
   asking for it was a mistake in the plan: traces, port order, subset draws
   and solver choices all differ. What is checkable, and checked, is that
   every emitted assertion is proved against the unmodified design by an
   EBMC run the tool did not perform.
3. **SyGuS goes through the API's parser, not `synthFun()` calls**, keeping
   the existing grammar generator instead of re-expressing it; the goals of
   that WP (no subprocess, no hardcoded 5s) hold either way.
4. **`--sygus-subprocess` is kept**, not removed: the two modes legitimately
   pick different valid solutions, so it is the only way to reproduce the old
   solver's behaviour.

Three real defects surfaced on the way, none of them in the new code:
hw-cbmc drops single `(* attr *)` instances (patched, to send upstream); the
VCD parser segfaulted on Icarus Verilog's empty `$dumpall` and mis-sized every
value whose leading bits were omitted (IEEE 1364 §18.2.1 — Verilator writes
full width, which is why it had never shown); and cvc5's own libpoly does not
configure under CMake 4.