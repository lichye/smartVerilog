# SMART

SMART is an automated specification-mining tool for Verilog designs. It mines
candidate **SystemVerilog Assertions (SVA)** from simulation traces, then uses
EBMC to prove every assertion that it emits. The default final gate is
k-induction, so output properties are invariants rather than merely bounded
observations.

The goal of SMART is to reduce the manual effort required to write correct and
meaningful formal specifications for hardware verification.

```bash
smart cat.sv          # -> cat_assertion.sv, the design plus proved assertions
```

---

## 1. Features

- Automatic generation of SystemVerilog Assertions (SVA)
- **Support for LTL (Linear Temporal Logic) properties**
- Works with both **structural** and **behavioral** Verilog
- No hand-written assertion templates required
- Every emitted assertion is formally proved against the original design
- Effective at detecting hardware mutations

---

## 2. Installation

SMART is one compiled binary. `install.sh` builds or downloads its dependencies
under the checkout; nothing is installed system-wide and no container is
required.

```bash
git clone https://github.com/lichye/smartVerilog.git
cd smartVerilog
./install.sh
```

`install.sh` downloads the pinned hw-cbmc source under `.deps/`, builds EBMC
and the Verilog frontend SMART links against, fetches Verilator (the default
simulator) and Icarus Verilog via oss-cad-suite, builds libcvc5, and then builds
`build/smart`. Expect the better part of an hour the first time, almost all of
it in CBMC and cvc5; afterwards, rebuilding SMART takes seconds.

Host requirements: a C++17 compiler, `make`, `flex`, `bison`, `git`, `curl`
and CMake ≥ 3.16 (`python3 -m pip install --user cmake` is enough).

```bash
./install.sh --check       # report what is missing without building
./build/smart --check-env
```

### 2.1 Benchmarks and repository layout

`Benchmark/` is the small versioned corpus used by tests and examples. It
contains FMCAD/ISCAS-style circuits, selected HWSpec designs, and a tiny
self-contained smoke fixture. The legacy `sim.py` files there are frozen
frontend-oracle fixtures; normal SMART runs generate their own testbench.

The mutation benchmarks are a separate repository:

```bash
git clone https://github.com/lichye/MutationBenchmark.git
```

Large research results, local toolchains, and the published artifact are not
part of this source repository. They are intentionally ignored so a clone stays
focused on the tool.

---

## 3. Usage

### 3.1 On your own design

```bash
smart my_design.sv                      # everything automatic
smart my_design.sv --top my_top         # if the module is not the file stem
smart my_design.sv --cycles 40 -j 8     # deeper traces, 8 workers
smart my_design.sv --no-blockified      # faster single-round mode
smart my_design.sv --unbounded          # also use k-induction inside blocks
```

The design may contain immediate `assume(...)` statements and
`(* anyseq *)` / `(* anyconst *)` free registers; SMART understands both.
Dependency modules in sibling `.sv` files are picked up automatically.

The output, `<top>_assertion.sv`, is the original file with the proved
invariants inserted before `endmodule`, and a header recording the exact
settings that produced them. The final output gate uses EBMC k-induction by
default, independently of the bounded checks used while mining, so every
emitted invariant can be re-checked with:

```bash
ebmc my_design_assertion.sv dependency_a.sv dependency_b.sv \
  -D FORMAL --k-induction --top my_top
```

For a single-file design, omit the dependency files. Keep `-D FORMAL` when
rechecking so source guarded by `` `ifdef FORMAL`` matches SMART's final gate.

`--no-final-unbounded` is the explicit compatibility escape hatch for a
bounded final check; such results are bounded properties rather than proved
invariants.

Exit codes: `0` success (possibly with zero assertions), `1` bad input,
`2` missing tool, `3` pipeline failure.

### 3.2 Configuration

Every option has a stable flat config key, and user-facing controls also have a
command-line flag generated from the same table. A few legacy experiment-only
controls remain config-only. Precedence is flag > `--config` file > default.

```bash
smart c17.sv --config Config/block_msa.json --no-final-unbounded      # reproduce the paper experiment
smart c17.sv --config Config/block_msa.json --no-final-unbounded -j 4 # ... with one knob changed
smart c17.sv --dump-config                       # what a run would actually use
smart --help                                     # every flag, with its key
```

The legacy nested `Config/*.json` schema still works, misspelled keys and all;
`--dump-config` prints the resolved settings in the current flat schema.

`--assumption-mining off|suggest` reserves the interface for a future research
track. It defaults to `off`. `suggest` is intentionally not runnable yet: the
tool reports that no backend has been selected before parsing or simulating a
design, rather than silently inventing assumptions from DUT-only traces.

### 3.3 Running the paper benchmarks

`run.py` is the experiment layer: it locates a benchmark, runs `smart`, and
files the results under `Results/<config>_<benchmark>/` (or
`Results/default_<benchmark>/` when no config is supplied).

```bash
python3 run.py c17                                      # current built-in defaults
python3 run.py c17 --config Config/smart.json --no-final-unbounded
python3 run.py c17 --config Config/block_msa.json --no-final-unbounded
python3 run.py --list-benchmarks
python3 run.py --list-configs
python3 run.py --check-env
```

A benchmark directory needs only the RTL, named after the directory:

```text
Benchmark/user/my_design/
  my_design.sv      # top-level design; the directory name must match
  *.sv              # optional dependency modules/packages
```

(Older benchmark directories also contain a cocotb `sim.py`. It is no longer
used — SMART generates its own Verilog testbench — and can be ignored.)

---

## 4. How a run works

1. **Frontend** — hw-cbmc parses and elaborates the design; SMART reads the
   ports, widths, parameters, free registers and assumes from it.
2. **Simulation** — a Verilog testbench is generated and run under Verilator
   by default, producing VCD traces. Assumes are stripped for simulation only;
   `--simulator iverilog` selects the alternative backend.
3. **Pre-analysis** — the signals in the traces become the candidate variable
   pool, which is split into per-block subsets.
4. **Synthesis** — each block is a SyGuS problem solved with cvc5 and refined
   against EBMC counterexamples. Blocks run in parallel, each with its own
   deadline.
5. **Blocking** (`--msa` / `--random`) — after a round, the invariants found
   so far are minimised and the variables they still leave underspecified
   become the next round's pool.
6. **Check** — every mined assertion is re-proved against the *original*
   design. Only survivors are emitted.
7. **Emit** — `<top>_assertion.sv`.

---

## 5. Output

- `<top>_assertion.sv` — the design with the proved assertions
- `Results/<config>_<design>/invariants.txt` — the proved assertions, one per line
- `Results/<config>_<design>/assertions.txt` — everything mined, before the final check
- `Results/<config>_<design>/effective-config.json` — the exact settings used

Keep the working directory with `--keep-work` to inspect traces, per-block
logs and the SyGuS problems; it is also kept automatically when a run fails.

---

## 6. Development

```bash
cmake -B build -S .
cmake --build build -j$(nproc)  # build the product and all test executables
ctest --test-dir build            # unit tests
tools/smoke.sh                    # end-to-end matrix, each output re-proved
python3 -B tools/parity_frontend.py  # frozen frontend oracle
python3 -B tools/compare_frontends.py
```

## Citation
If you use SMART, please cite:
```
Leiqi Ye, Yixuan Li, Guy Frankel, Jianyi Cheng, Elizabeth Polgreen. Unlocking Hardware Verification with Oracle Guided Synthesis. The 25th Conference on Formal Methods in Computer-Aided Design (FMCAD)
```
