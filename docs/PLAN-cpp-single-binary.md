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
- Verilator and EBMC remain external PATH dependencies (subprocess).
- cvc5 is used via **libcvc5 C++ API** (the Python side already uses
  `cvc5.pythonic`, so this unifies the solver story; no z3 in the binary).
- Build system: **CMake**.
- cocotb / venv / all Python leaves the runtime path of the tool.

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

Multi-file designs: extra `.sv` dependency files may sit next to the input;
they are compiled into the simulation and formal runs but only the top module
is mined. `smart a.sv b.sv --top a` should work (first file or --top decides).

## 2. Architecture

```
smart (single binary)
 ├─ frontend/   SV module model: ports, params, free regs, assume strip/inject   [WP2]
 ├─ simgen/     C++ Verilator harness generation + verilator subprocess + VCD    [WP3]
 ├─ pipeline/   orchestration: workdir, stages, thread pool, config              [WP4]
 ├─ mus/        MSA/MUS via libcvc5 (port of minimal_satisfiable_assignment.py)  [WP5]
 ├─ existing (reuse, refactor to library):
 │    parser/  VCD flex/bison parser, Smt/Sygus/Verilog expr parsers
 │    tracer/  Module/Trace/State/Value
 │    sygus/   SyGuSGenerater (WP6 converts subprocess cvc5 -> libcvc5), StateMaker
 │    helper/  VerilogChecker (EBMC subprocess), Timer, Utils
 └─ emit/      <top>_assertion.sv writer                                          [WP7]
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
| Port/param/free-reg parsing, assume strip/inject, clock/reset guessing | `smart/src/python/gen_bench.py` + `test_gen_bench.py` | Validated on all 54 repo benchmarks. C++ parser must pass a parity diff against it (WP2 acceptance). |
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
one agent each. **Parallelizable groups:** {WP1, WP2, WP3} can run
concurrently; WP5 and WP6 concurrently after WP1; WP4 after WP2+WP3; WP7+WP8
last. Run everything inside the Docker image (`Docker/Dockerfile` /
`artifact/Dockerfile`) — the host may lack verilator/ebmc/nlohmann-json.

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
- **Acceptance:** in Docker, `cmake -B build && cmake --build build` produces
  a `smart.out` that passes an existing benchmark run (`python run.py c17`)
  when copied in place of the make-built one.

### WP2 — frontend: SV module model in C++
- **Deliverable:** `smart/src/frontend/SVModule.{h,cpp}` with:
  `parse(text, top)` -> {ports(dir,width,name), params, freeRegs(kind,width,name),
  hasAssume}; `stripAssumes(text)`; `injectAssumes(text, top, exprs, clock)`;
  `guessClock/guessReset`. Pure functions, no I/O.
- **Reference/spec:** `gen_bench.py` — mirror its regex semantics exactly:
  comment stripping preserving newlines; ANSI header vs non-ANSI body port
  decls; comma-separated name lists stopped at direction keywords; width from
  `[msb:lsb]` with parameter substitution and integer arithmetic (unresolvable
  -> width unknown -> treat as 1 with a warning); Verilog sized literals
  (8'hFF); `(* anyseq|anyconst *)` regs; assume stripping replaces the whole
  statement with `;` so `if (c) assume(x); else ...` stays legal.
- Use std::regex or hand-rolled scanning (preferred for the balanced-paren
  pieces, as in the Python).
- **Acceptance (parity harness, part of this WP):**
  `tools/parity_frontend.py`: for every `Benchmark/**/<top>.sv` and
  `artifact/CaseStudy/Input/nru_a/nru_a.sv`, dump JSON from gen_bench.py and
  from a `smart --dump-frontend <file>` debug flag; diff must be empty for
  all 54+ designs. Also port the 6 cases of `test_gen_bench.py` to a C++
  test (`ctest`).

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
  keys), `WorkDir.{h,cpp}`.
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
    width 1 + warning in gen_bench; keep that behavior, don't crash.
14. `run.py --list-benchmarks / --check-env` UX from this branch should have
    equivalents in the binary (`--check-env` is in WP4/WP8 scope).

## 6. Validation strategy (cross-cutting)

- **Frontend parity:** WP2's 54-design JSON diff vs `gen_bench.py` (the
  committed Python is the spec; if the C++ finds a Python bug, fix Python
  first, keep them in lockstep until WP8).
- **End-to-end parity:** same-seed assertion-set comparison vs the legacy
  Python pipeline on {tiny_and, c17, s27, arb2} before removing anything.
- **Fixtures:** capture `.sl`/VCD fixtures from Docker runs into
  `smart/test/fixtures/` early (WP5/WP3 depend on them).
- Every WP lands as a separate PR/commit on `new-interface` with its
  acceptance evidence (command + output) in the commit message.

## 7. Suggested agent assignment

| Agent | WPs | Can start |
|---|---|---|
| A | WP1 CMake | immediately |
| B | WP2 frontend | immediately |
| C | WP3 simgen | immediately (uses gen_bench.py spec while WP2 lands) |
| D | WP5 mus + WP6 sygus | after WP1 (needs libcvc5 in build) |
| E | WP4 pipeline | after WP2+WP3 |
| F | WP7 emit + WP8 cleanup | last |
