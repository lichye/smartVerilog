# SMART

SMART is an automated specification mining tool for Verilog designs.
It synthesizes **SystemVerilog Assertions (SVA)** — including **LTL (Linear
Temporal Logic) properties** — from simulation traces using oracle-guided
synthesis.

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

SMART is one compiled binary. Everything it needs is built into the checkout;
nothing is installed system-wide and no container is required.

```bash
git clone --recursive https://github.com/lichye/smartVerilog.git
cd smartVerilog
./install.sh
```

`install.sh` builds hw-cbmc (EBMC and the Verilog frontend SMART links
against), fetches Icarus Verilog via oss-cad-suite, builds libcvc5, and then
builds `build/smart`. Expect the better part of an hour the first time,
almost all of it in CBMC and cvc5; afterwards, rebuilding SMART takes seconds.

Host requirements: a C++17 compiler, `make`, `flex`, `bison`, `git`, `curl`
and CMake ≥ 3.16 (`python3 -m pip install --user cmake` is enough).

```bash
./install.sh --check       # report what is missing without building
./build/smart --check-env
```

### 2.1 Benchmarks

The mutation benchmarks are a separate repository:

```bash
git clone https://github.com/lichye/MutationBenchmark.git
```

---

## 3. Usage

### 3.1 On your own design

```bash
smart my_design.sv                      # everything automatic
smart my_design.sv --top my_top         # if the module is not the file stem
smart my_design.sv --cycles 40 -j 8     # deeper traces, 8 workers
smart my_design.sv --msa                # iterate: mine, re-block, mine again
```

The design may contain immediate `assume(...)` statements and
`(* anyseq *)` / `(* anyconst *)` free registers; SMART understands both.
Dependency modules in sibling `.sv` files are picked up automatically.

The output, `<top>_assertion.sv`, is the original file with the proved
assertions inserted before `endmodule`, and a header recording the exact
settings that produced them. Every assertion in it has been proved by EBMC
against the unmodified design, so it can be re-checked independently:

```bash
ebmc my_design_assertion.sv --bound 10 --top my_design
```

Exit codes: `0` success (possibly with zero assertions), `1` bad input,
`2` missing tool, `3` pipeline failure.

### 3.2 Configuration

Every option has both a command-line flag and a config-file key of the same
name, and they are generated from one table — so anything you can set one way
you can set the other. Precedence is flag > `--config` file > default.

```bash
smart c17.sv --config Config/block_msa.json      # a paper experiment
smart c17.sv --config Config/block_msa.json -j 4 # ... with one knob changed
smart c17.sv --dump-config                       # what a run would actually use
smart --help                                     # every flag, with its key
```

The legacy nested `Config/*.json` schema still works, misspelled keys and all;
`--dump-config` prints the resolved settings in the current flat schema.

### 3.3 Running the paper benchmarks

`run.py` is the experiment layer: it locates a benchmark, runs `smart`, and
files the results under `Results/<config>_<benchmark>/`.

```bash
python3 run.py c17
python3 run.py c17 --config Config/block_msa.json
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
2. **Simulation** — a Verilog testbench is generated and run under Icarus
   Verilog, producing VCD traces. Assumes are stripped for simulation only.
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
cmake --build build --target smart -j$(nproc)
ctest --test-dir build            # unit tests
tools/smoke.sh                    # end-to-end matrix, each output re-proved
python3 tools/parity_frontend.py  # frozen frontend oracle
python3 tools/compare_frontends.py
```

## Citation
If you use our work, please cite this paper.
```
Leiqi Ye, Yixuan Li, Guy Frankel, Jianyi Cheng, Elizabeth Polgreen. Unlocking Hardware Verification with Oracle Guided Synthesis. The 25th Conference on Formal Methods in Computer-Aided Design (FMCAD)
```
