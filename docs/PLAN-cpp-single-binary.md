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
The binary parses it, generates random simulation traces with Verilator,
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
  semantics (it natively understands `assume` / `(* anyseq *)` /
  `(* anyconst *)`), and — critically — **upstream maintains the frontend
  for us**. See WP2A/WP2.
- Verilator and EBMC remain external tools; Verilator via subprocess for
  simulation. EBMC is now also built from the submodule (single source of
  truth for the CBMC toolchain) but still invoked as a subprocess for
  verification (no EBMC C++ API to link).
- cvc5 is used via **libcvc5 C++ API** (the Python side already uses
  `cvc5.pythonic`, so this unifies the solver story; no z3 in the binary).
- Build system: **CMake**.
- cocotb / venv / all Python leaves the runtime path of the tool.
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
  --check-env             report verilator/ebmc/cvc5-lib versions and exit
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
 ├─ simgen/     C++ Verilator harness generation + verilator subprocess + VCD    [WP3]
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
WP2A} first; then {WP2, WP3, WP5, WP6}; WP4 after WP2+WP3; WP7+WP8 last. Run
everything inside the Docker image (`Docker/Dockerfile` / `artifact/Dockerfile`)
— the host lacks verilator/ebmc/nlohmann-json and cannot build hw-cbmc.

### WP1 — CMake build (no behavior change)
- **Deliverable:** `smart/CMakeLists.txt` building today's `smart.out` and
  the new `smart` binary target skeleton; top-level `CMakeLists.txt`.
- **Reference:** `smart/Makefile` (flex `-P VCDParser`, bison `--defines`,
  include dirs, c++17; see lines 1-70).
- Steps: FindFLEX/FindBISON with the same prefix/output names into
  `${CMAKE_BINARY_DIR}/generated`; `find_package(cvc5)` (Docker image has
  cvc5 1.2.0 — the static zip does NOT ship the C++ lib, so the Dockerfile
  must gain a `cvc5 -dev` install or build-from-source stage; coordinate with
  WP6); nlohmann-json via find_package or vendored single header under
  `smart/third_party/`.
- Keep the old Makefile working until WP8 removes it (setup.py calls
  `make compile`).
- Also add/build the `hw-cbmc` submodule targets (coordinate with WP2A):
  an ExternalProject or custom target that runs the submodule's `cd src;
  make` (or its CMake, if the pinned revision has one) and exposes the
  produced static libs + the `ebmc` binary path to the rest of the build.
- **Acceptance:** in Docker, `cmake -B build && cmake --build build` produces
  a `smart.out` that passes an existing benchmark run (`python run.py c17`)
  when copied in place of the make-built one; and the hw-cbmc submodule
  builds and its `ebmc` runs `--version`.

### WP2A — hw-cbmc submodule + build integration (critical path)
- **Deliverable:** `third_party/hw-cbmc` git submodule pinned to a specific
  tag/commit (pick the release matching EBMC 5.6, our current backend, unless
  a newer tag is deliberately chosen — record the choice); `.gitmodules`;
  Docker/CMake steps that fetch nested submodules (`git submodule update
  --init --recursive` — hw-cbmc itself vendors cbmc as a submodule) and build
  it.
- **Build facts (verified from upstream):** BSD-3-Clause; build is
  `git submodule update --init --recursive` then `cd src && make` (compiles
  the bundled cbmc automatically); needs flex + bison + a C++ toolchain;
  produces the `ebmc` binary under `src/ebmc/` and static libraries for the
  Verilog frontend + CBMC util. Building is slow (compiles much of CBMC) —
  cache it as a Docker layer.
- Steps: add the submodule; teach `Docker/Dockerfile` + `artifact/Dockerfile`
  to init+build it (replacing the current `apt`/`.deb` EBMC install so the
  binary and the linkable libs come from ONE source); export to the smart
  build: the include dirs (`third_party/hw-cbmc/src`,
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
- **Acceptance:** in Docker, from a clean checkout,
  `git submodule update --init --recursive` + the build produces a working
  `ebmc` AND either (link path) a trivial C++ program that links libverilog
  and prints a parsed module's port count, or (subprocess path) a documented
  `ebmc --show-parse`/`--show-varmap` invocation whose output contains the
  ports/params/attributes WP2 needs. Document which path was chosen and why.

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
- **Acceptance:** `tools/parity_frontend.py` (already written) run with the
  hw-cbmc-backed `--dump-frontend` must match the gen_bench.py oracle on the
  56 repo designs, OR every divergence is listed with a note explaining why
  hw-cbmc is right and the regex was wrong (e.g. a width the regex couldn't
  resolve). The 6 `test_svmodule.cpp` spec cases still pass. Net goal: the
  pipeline gets a MORE correct ModuleInfo than the regex, never a worse one.

### WP3 — simgen: Verilator harness instead of cocotb
- **Deliverable:** `smart/src/simgen/Harness.{h,cpp}`: given the WP2 model +
  run options, (a) write `sim_main.cpp` (a Verilator C++ testbench), (b) run
  `verilator --cc --exe --build --trace [-Wno-WIDTHEXPAND -Wno-WIDTHTRUNC
  -Wno-UNOPTFLAT -Wno-CASEOVERLAP] [--public-flat-rw when freeRegs]`,
  (c) execute it `traces` times with seeds seed+i, moving each `dump.vcd` to
  `runtime/sim_results/sim<i>.vcd`.
- **Stimulus semantics (from the generated sim.py in gen_bench.render_sim_py):**
  seed the RNG; set each anyconst reg once to draw(spec); if clocked: start
  clock (toggle every step), apply reset for reset.cycles cycles then
  deassert; per cycle assign every non-clock non-reset input and every anyseq
  reg a fresh draw(spec) (uniform 0..2^width-1, honoring --config per-input
  max/values/const overrides), then advance one clock; unclocked designs
  advance time steps instead. Trace depth = cycles (default 10 — this
  finally makes the config field `Depth_Trace` real; wire it up).
- Driving internal (* anyseq *) regs from the harness: with
  `--public-flat-rw` Verilator exposes them on the model as public members;
  name mangling for scoped signals is `rootp-><top>__DOT__<name>`. Simulation
  copies of the RTL must be assume-stripped (WP2 stripAssumes) — write the
  stripped copies into `workdir/sim_src/`, originals into `runtime/verilog/`
  and `runtime/formal/`.
- **VCD compatibility caution:** the existing flex/bison VCD parser consumes
  cocotb/verilator VCDs today; Verilator-direct VCDs have the same format but
  scope layout may add a `TOP` wrapper scope. Check
  `smart/src/parser/VCDFileParser.*` + `tracer/Module::addTracesfromDir` for
  how scopes/signal names are matched, and either configure the trace call
  (`tfp->dump` under the right scope) or teach the trace loader to skip a
  `TOP` scope. Validate against a cocotb-produced VCD from the current
  pipeline (run `python run.py tiny_and` in Docker to get one).
- **Acceptance:** in Docker, for `tiny_and`, `s27` (non-ANSI, unclocked),
  `axis_fifo` (clocked+reset, parameterized widths), and `nru_a`
  (anyseq/anyconst/assume): harness builds, produces N VCDs, and
  `Module::addTracesfromDir` loads them with the same signal set as the
  cocotb flow (write a small dump-signals debug flag to compare).

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
- **Acceptance:** `smart Benchmark/user/tiny_and/tiny_and.sv` end-to-end in
  Docker produces >=1 verified assertion in `tiny_and_assertion.sv`;
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
  on 3 captured `runtime/SygusResult.sl` fixtures from real runs (generate in
  Docker, commit under `smart/test/fixtures/`), assert the C++ returns the
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
- Update `ReadMe.md`, `ARTIFACT.md`, `Docker/Dockerfile` +
  `artifact/Dockerfile` (install cvc5 dev lib, drop venv/cocotb from the
  tool path — keep Python only for the artifact/experiment layer).
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
10. Host machines may lack everything except cvc5 — ALL build/test happens
    in the Docker image. `nlohmann/json.hpp` is not on the host.
11. VCD from Verilator-direct may wrap signals in a `TOP` scope (vs cocotb) —
    see WP3 caution; this is the most likely silent-breakage point of the
    whole migration.
12. Trace depth 10 was historically hardcoded ("Depth_Trace: Default is 10
    and hard to change") — after WP3 it must genuinely follow `--cycles`.
13. Widths that don't resolve to integers (complex param exprs) fall back to
    width 1 + warning in the regex oracle; hw-cbmc should resolve most of
    these correctly — where it does, prefer its width and note the oracle diff.
14. `run.py --list-benchmarks / --check-env` UX from this branch should have
    equivalents in the binary (`--check-env` is in WP4/WP8 scope);
    `--check-env` must also report the hw-cbmc-built EBMC version.
15. hw-cbmc vendors cbmc as a NESTED submodule — always
    `git submodule update --init --recursive`, and the Docker build must too.
    A plain `--init` silently yields an unbuildable hw-cbmc.
16. hw-cbmc's static libs are NOT a curated public API. If the WP2A link
    spike drags, fall back to `ebmc --show-parse`/`--show-varmap` subprocess
    rather than fighting CBMC's link graph — the ModuleInfo contract is the
    same either way.
17. Building hw-cbmc compiles much of CBMC and is slow; it MUST be a cached
    Docker layer, not rebuilt per run. Pin the submodule commit so the cache
    is stable.
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
- **Fixtures:** capture `.sl`/VCD fixtures from Docker runs into
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
