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

### 2.3 Benchmark 
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
/smartVerilog/Result/c17/
```
### 3.1 User own case
The use test cases should be in one directory like this:
```bash
smartVerilog/User/test/
--> test.sv
--> sim.py (This should be the simulation cocotb file)
```
The user can run with command
```bash
python run.py test
```
## 4. Configuration

SMART can be configured via:

- run.py

- default.json

You can control synthesis limits, verification timeouts, trace handling, and assertion generation options (including LTL-related settings).

## 5. Output

SMART produces verified SystemVerilog Assertions (SVA), including:

- State invariants

- Bit-vector relational properties

- LTL temporal properties

Only assertions that pass formal verification are kept. The runing result will be in the directory:
```bash
Result/c17/c17_result.txt
Result/c17/assertions.txt
```

## Citation
If you use our work, please cite this paper.
```
Leiqi Ye, Yixuan Li, Guy Frankel, Jianyi Cheng, Elizabeth Polgreen. Unlocking Hardware Verification with Oracle Guided Synthesis. The 25rd Conference on Formal Methods in Computer-Aided Design (FMCAD)
```
