# SMART

SMART is an automated specification mining tool for Verilog designs.  
It synthesizes **SystemVerilog Assertions (SVA)** — including **LTL (Linear Temporal Logic) properties** — from simulation traces using oracle-guided synthesis.

The goal of SMART is to reduce the manual effort required to write correct and meaningful formal specifications for hardware verification.

---

## 1. Features

- Automatic generation of SystemVerilog Assertions (SVA)
- **Support for LTL (Linear Temporal Logic) properties**
- Works with both **structural** and **behavioral** Verilog
- No hand-written assertion templates required
- All generated assertions are formally verified
- Effective at detecting hardware mutations

---

## 2. Installation

### 2.1 Native Ubuntu (limited support)

```bash
git clone https://github.com/lichye/smartVerilog.git
cd smartVerilog
bash install.sh
```

### 2.2 Docker (Recommended)

```bash
docker pull magna2024/smart
```

### 2.3 Artifact Release

For a reproducible artifact package, use the self-contained image flow documented in [ARTIFACT.md](ARTIFACT.md).  
It includes a release-oriented Dockerfile, quick-start commands, and image export guidance.

### 2.4 Benchmark 
To make sure the evalute benchmark is stable, we can use the pre-generated benchmark.
```bash
cd smartVerilog/
git clone https://github.com/lichye/MutationBenchmark.git
```

## 3. Usage

Run SMART on a benchmark:
```bash
python run.py c17
```
Results will be generated in:
```
/smartVerilog/Results/smart_c17/
```

### 3.1 Running a new hardware design

To run SMART/MAPminer on a design that is not one of the included paper
benchmarks, add a benchmark directory under `Benchmark/`:

```text
Benchmark/user/my_design/
  my_design.sv      # SystemVerilog design under test
  sim.py            # cocotb simulation that drives the design
  Makefile          # optional cleanup/build helper
  *.sv              # optional dependency modules/packages
```

The directory must contain the top-level RTL file and a cocotb `sim.py` that
drives representative traces. In `sim.py`, set `hdl_toplevel` to the top module
name and include any local RTL dependency files.

For a source checkout with the Python/toolchain dependencies installed:

```bash
python run.py my_design Config/block_msa.json
```

For a quick smoke test of the input format, this repository includes:

```text
Benchmark/user/tiny_and/
  tiny_and.sv
  sim.py
```

Run it with:

```bash
python run.py tiny_and Config/smart.json
```

The recommended artifact path is to use the Docker image or rebuild it from the
artifact Dockerfile. The Docker environment includes cocotb, Verilator, EBMC,
CVC5, and the other dependencies needed by the pipeline. Native source-checkout
runs require those dependencies to be installed locally.

For a benchmark named `my_design`, the pipeline is:

1. `run.py` finds `Benchmark/**/my_design/`.
2. The benchmark files are copied into `smart/user/`.
3. `sim.py` is run through cocotb/Verilator to collect traces.
4. SMART/MAPminer synthesizes candidate SVA from those traces.
5. The checker formally verifies generated assertions on the original design.
6. If mutation data is available, the evaluator computes mutation-detection
   metrics.
7. Results are written to `Results/<config>_my_design/`.

Mutation benchmarks are optional for bring-up. If no matching directory exists
under `MutationBenchmark/`, SMART/MAPminer still generates and verifies
assertions, but mutation-detection metrics are unavailable for that design.

## 4. Configuration

SMART can be configured via:

- run.py

- smart.json

You can control synthesis limits, verification timeouts, trace handling, and assertion generation options (including LTL-related settings).

## 5. Output

SMART produces verified SystemVerilog Assertions (SVA), including:

- State invariants

- Bit-vector relational properties

- LTL temporal properties

Only assertions that pass formal verification are kept. The running result will be in the directory:
```bash
Results/smart_c17/result_c17.txt
Results/smart_c17/assertions.txt
```

## Citation
If you use our work, please cite this paper.
```
Leiqi Ye, Yixuan Li, Guy Frankel, Jianyi Cheng, Elizabeth Polgreen. Unlocking Hardware Verification with Oracle Guided Synthesis. The 25rd Conference on Formal Methods in Computer-Aided Design (FMCAD)
```
