# Automatic NRU Invariant / Proof-Harness Generation — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a container-based pipeline that mines NRU invariants with SMART (`smart.json`), assembles a standalone proof harness from the flattened design, and gates it on EBMC k-induction with a bounded refinement loop.

**Architecture:** Pure text/parse logic (derive flat benchmark, assemble harness, parse EBMC output, screen clauses) lives in a single host-unit-testable Python module. A thin in-container entry script and a `make gen-nru` target run SMART + EBMC + the refinement loop inside the existing `mapminer` Docker image. `Precomputed/*` and `make invariants` are never modified.

**Tech Stack:** Python 3 (stdlib only, no pytest dependency), Bash, Make, Docker, SMART (`run.py`), EBMC.

Spec: `docs/superpowers/specs/2026-05-19-casestudy-nru-invariant-autogen-design.md`

---

## File Structure

| File | Responsibility |
|---|---|
| `artifact/CaseStudy/gen_invariants.py` | Pure functions (split/assemble/parse/screen) + `main()` orchestrator with `--dry-run` and in-container modes |
| `artifact/CaseStudy/test_gen_invariants.py` | Stdlib-only tests for the pure functions (host-runnable: `python3 test_gen_invariants.py`) |
| `artifact/CaseStudy/_gen_nru_entry.sh` | In-container entrypoint: sources SMART env, runs `gen_invariants.py` |
| `artifact/CaseStudy/Makefile` | New `gen-nru` target (host `docker run`, image resolution) |
| `artifact/CaseStudy/testdata/ebmc_kind_sample.txt` | Captured real EBMC k-induction output, used as parser fixture |
| `artifact/CaseStudy/Generated/gen_nru/` | Outputs: `nru_proof.sv`, `assertions.txt`, `gen_nru.log` |

**Deviation from spec §4.3 (intentional):** outputs go to `Generated/gen_nru/`, not `Generated/nru/`, so they never clobber the `make invariants` extract path (`Generated/nru/assertions.txt`). This upholds the spec requirement that the fallback path stays untouched.

**DRY note:** `gen_invariants.py` re-implements ~15 lines of marker/paren-balance logic also present in `extract_invariants.py`. This duplication is deliberate: the spec mandates `extract_invariants.py` stay byte-stable as the reviewer fallback, so we do not couple them.

---

## Task 1: Split a proof harness around the Mapminer assert

**Files:**
- Create: `artifact/CaseStudy/gen_invariants.py`
- Test: `artifact/CaseStudy/test_gen_invariants.py`

- [ ] **Step 1: Write the failing test**

In `artifact/CaseStudy/test_gen_invariants.py`:

```python
import os
import gen_invariants as g

HERE = os.path.dirname(os.path.abspath(__file__))
NRU_PROOF = os.path.join(HERE, "Precomputed", "nru_proof.sv")


def test_split_around_marker_assert_roundtrips():
    text = open(NRU_PROOF).read()
    prefix, assert_block, suffix = g.split_around_marker_assert(text, "Mapminer Properties")
    # The three parts must reconstruct the original file exactly.
    assert prefix + assert_block + suffix == text
    # assert_block is exactly one assert statement.
    assert assert_block.startswith("assert")
    assert assert_block.rstrip().endswith(";")
    # prefix keeps the marker comment; suffix keeps the FMCAD comment block.
    assert "//Mapminer Properties" in prefix
    assert "FMCAD Paper" in suffix
    # The base property stays commented in the prefix, not duplicated in suffix.
    assert "// assert(!check || eq_hit);" in prefix


def test_split_raises_when_marker_missing():
    try:
        g.split_around_marker_assert("module flat(); endmodule", "Mapminer Properties")
        raise AssertionError("expected ValueError")
    except ValueError:
        pass


if __name__ == "__main__":
    fns = sorted((k, v) for k, v in globals().items()
                 if k.startswith("test_") and callable(v))
    for name, fn in fns:
        fn()
        print("ok", name)
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd artifact/CaseStudy && python3 test_gen_invariants.py`
Expected: FAIL — `ModuleNotFoundError: No module named 'gen_invariants'`

- [ ] **Step 3: Write minimal implementation**

In `artifact/CaseStudy/gen_invariants.py`:

```python
#!/usr/bin/env python3
"""Automatic NRU invariant mining + proof-harness assembly (container pipeline)."""

from __future__ import annotations


def split_around_marker_assert(text: str, marker: str = "Mapminer Properties"):
    """Return (prefix, assert_block, suffix).

    assert_block is the single `assert( ... );` statement that follows `marker`.
    prefix + assert_block + suffix == text.
    """
    marker_pos = text.find(marker)
    if marker_pos < 0:
        raise ValueError(f"marker not found: {marker}")

    assert_pos = text.find("assert", marker_pos)
    if assert_pos < 0:
        raise ValueError(f"assertion not found after marker: {marker}")

    paren_pos = text.find("(", assert_pos)
    if paren_pos < 0:
        raise ValueError("assertion has no opening parenthesis")

    depth = 0
    end_pos = None
    for idx in range(paren_pos, len(text)):
        ch = text[idx]
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
            if depth == 0:
                semi_pos = text.find(";", idx)
                if semi_pos < 0:
                    raise ValueError("assertion has no terminating semicolon")
                end_pos = semi_pos + 1
                break
    if end_pos is None:
        raise ValueError("assertion parentheses did not balance")

    return text[:assert_pos], text[assert_pos:end_pos], text[end_pos:]
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cd artifact/CaseStudy && python3 test_gen_invariants.py`
Expected: PASS — `ok test_split_around_marker_assert_roundtrips` and `ok test_split_raises_when_marker_missing`

- [ ] **Step 5: Commit**

```bash
cd /home/magna/Projects/published/smartVerilog
git add artifact/CaseStudy/gen_invariants.py artifact/CaseStudy/test_gen_invariants.py
git commit -m "feat(casestudy): split proof harness around Mapminer assert"
```

---

## Task 2: Build the Mapminer assert from a clause list

**Files:**
- Modify: `artifact/CaseStudy/gen_invariants.py`
- Test: `artifact/CaseStudy/test_gen_invariants.py`

- [ ] **Step 1: Write the failing test**

Append to `test_gen_invariants.py` (before the `__main__` block):

```python
def test_build_mapminer_assert_shapes_conjunction():
    clauses = ["(!c1_hit || c2_rst)", "(!rst || c1_rst)"]
    block = g.build_mapminer_assert(clauses)
    assert block.startswith("assert(")
    assert block.rstrip().endswith(");")
    assert "(!check || eq_hit) &&" in block          # base property always first
    assert "(!c1_hit || c2_rst)" in block
    assert "(!rst || c1_rst)" in block
    # clauses are &&-joined inside the inner paren group
    assert block.count("&&") == 2                     # base&&group, clause&&clause


def test_build_mapminer_assert_wraps_bare_clauses():
    block = g.build_mapminer_assert(["a < b"])         # not parenthesised
    assert "(a < b)" in block


def test_build_mapminer_assert_rejects_empty():
    try:
        g.build_mapminer_assert([])
        raise AssertionError("expected ValueError")
    except ValueError:
        pass
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd artifact/CaseStudy && python3 test_gen_invariants.py`
Expected: FAIL — `AttributeError: module 'gen_invariants' has no attribute 'build_mapminer_assert'`

- [ ] **Step 3: Write minimal implementation**

Append to `gen_invariants.py`:

```python
BASE_PROPERTY = "(!check || eq_hit)"


def _wrap(clause: str) -> str:
    c = clause.strip()
    if c.startswith("(") and c.endswith(")"):
        return c
    return f"({c})"


def build_mapminer_assert(clauses: list[str]) -> str:
    """Build `assert( (!check||eq_hit) && ( c0 && c1 && ... ) );`."""
    cleaned = [_wrap(c) for c in clauses if c and c.strip()]
    if not cleaned:
        raise ValueError("no clauses to assemble")
    indent = " " * 16
    joined = (" &&\n" + indent + "    ").join(cleaned)
    return (
        "assert(\n"
        f"{indent}{BASE_PROPERTY} &&\n"
        f"{indent}(\n"
        f"{indent}    {joined}\n"
        f"{indent})\n"
        f"{' ' * 12});"
    )
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cd artifact/CaseStudy && python3 test_gen_invariants.py`
Expected: PASS — all `test_build_mapminer_assert_*` lines print `ok`

- [ ] **Step 5: Commit**

```bash
cd /home/magna/Projects/published/smartVerilog
git add artifact/CaseStudy/gen_invariants.py artifact/CaseStudy/test_gen_invariants.py
git commit -m "feat(casestudy): assemble Mapminer assert from clause list"
```

---

## Task 3: Assemble a full standalone proof harness

**Files:**
- Modify: `artifact/CaseStudy/gen_invariants.py`
- Test: `artifact/CaseStudy/test_gen_invariants.py`

- [ ] **Step 1: Write the failing test**

Append to `test_gen_invariants.py`:

```python
def test_assemble_proof_harness_replaces_only_the_assert():
    template = open(NRU_PROOF).read()
    out = g.assemble_proof_harness(template, ["(!rst || c1_rst)"])
    # Module + driver preserved verbatim.
    assert out.startswith("module flat (")
    assert out.rstrip().endswith("endmodule")
    # New clause present; the old hand-curated clause is gone.
    assert "(!rst || c1_rst)" in out
    assert "(!(hitmap2 <= c2_metadata) || c1_rst)" not in out
    # Exactly one active (uncommented) assert statement remains.
    active = [ln for ln in out.splitlines()
              if ln.lstrip().startswith("assert(")]
    assert len(active) == 1
    # Markers/comments preserved.
    assert "//Mapminer Properties" in out
    assert "FMCAD Paper" in out
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd artifact/CaseStudy && python3 test_gen_invariants.py`
Expected: FAIL — `AttributeError: ... 'assemble_proof_harness'`

- [ ] **Step 3: Write minimal implementation**

Append to `gen_invariants.py`:

```python
def assemble_proof_harness(template_text: str, clauses: list[str],
                           marker: str = "Mapminer Properties") -> str:
    """Replace the marker's assert statement with a fresh one built from clauses."""
    prefix, _old_assert, suffix = split_around_marker_assert(template_text, marker)
    return prefix + build_mapminer_assert(clauses) + suffix
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cd artifact/CaseStudy && python3 test_gen_invariants.py`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
cd /home/magna/Projects/published/smartVerilog
git add artifact/CaseStudy/gen_invariants.py artifact/CaseStudy/test_gen_invariants.py
git commit -m "feat(casestudy): assemble full standalone proof harness"
```

---

## Task 4: Derive the flat mining benchmark (design + sim.py)

**Files:**
- Modify: `artifact/CaseStudy/gen_invariants.py`
- Test: `artifact/CaseStudy/test_gen_invariants.py`

The mining benchmark is the flattened design with the Mapminer assert removed (so SMART mines fresh invariants over the exact `flat` namespace), plus a cocotb `sim.py` whose top is `flat`.

- [ ] **Step 1: Write the failing test**

Append to `test_gen_invariants.py`:

```python
import ast


def test_derive_flat_design_strips_active_assert():
    template = open(NRU_PROOF).read()
    design = g.derive_flat_design(template)
    assert design.startswith("module flat (")
    assert design.rstrip().endswith("endmodule")
    # No active assert remains (only commented ones allowed).
    for ln in design.splitlines():
        assert not ln.lstrip().startswith("assert(")
    # Marker comment retained so re-assembly anchor still exists.
    assert "//Mapminer Properties" in design


def test_make_flat_sim_py_is_valid_python_targeting_flat():
    src = g.make_flat_sim_py("nru_flat.sv")
    ast.parse(src)                                  # must be syntactically valid
    assert 'hdl_toplevel="flat"' in src
    assert "nru_flat.sv" in src
    assert "@cocotb.test()" in src
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd artifact/CaseStudy && python3 test_gen_invariants.py`
Expected: FAIL — `AttributeError: ... 'derive_flat_design'`

- [ ] **Step 3: Write minimal implementation**

Append to `gen_invariants.py`:

```python
def derive_flat_design(template_text: str,
                        marker: str = "Mapminer Properties") -> str:
    """Flattened design with the active Mapminer assert removed (commented)."""
    prefix, _old, suffix = split_around_marker_assert(template_text, marker)
    return prefix + "/* assert removed for mining */ ;" + suffix


def make_flat_sim_py(design_filename: str) -> str:
    """cocotb runner that simulates the `flat` top so SMART can collect traces."""
    return f'''import os
import random

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge
from cocotb.runner import get_runner


@cocotb.test()
async def drive_flat(dut):
    random.seed(42)
    cocotb.start_soon(Clock(dut.clk, 2, units="ns").start())
    for _ in range(64):
        await RisingEdge(dut.clk)


def runner():
    sim = os.getenv("SIM", "verilator")
    here = os.path.dirname(os.path.abspath(__file__))
    args = ["--trace", "-Wno-WIDTHEXPAND", "-Wno-WIDTHTRUNC",
            "--Wno-UNOPTFLAT", "--Wno-CASEOVERLAP"]
    run = get_runner(sim)
    run.build(verilog_sources=[os.path.join(here, "{design_filename}")],
              hdl_toplevel="flat", build_args=args)
    run.test(hdl_toplevel="flat", test_module="sim")


if __name__ == "__main__":
    runner()
'''
```

Note: `derive_flat_design` replaces the assert with a harmless empty statement so the
`if (!rst && stable) begin ... end` block still elaborates. The free `(* anyseq *)`
regs are left undriven in sim; SMART's pre-analysis randomizes free signals (same as
the existing modular benchmark, whose cocotb test also only clocks the design).

- [ ] **Step 4: Run test to verify it passes**

Run: `cd artifact/CaseStudy && python3 test_gen_invariants.py`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
cd /home/magna/Projects/published/smartVerilog
git add artifact/CaseStudy/gen_invariants.py artifact/CaseStudy/test_gen_invariants.py
git commit -m "feat(casestudy): derive flat mining benchmark + sim.py"
```

---

## Task 5: Probe and parse EBMC k-induction output

EBMC's exact property-result wording must be observed, not guessed. This task has an empirical probe step that captures real output as a test fixture, then implements the parser against it.

**Files:**
- Create: `artifact/CaseStudy/testdata/ebmc_kind_sample.txt`
- Modify: `artifact/CaseStudy/gen_invariants.py`
- Test: `artifact/CaseStudy/test_gen_invariants.py`

- [ ] **Step 1: Capture real EBMC output as a fixture (in container)**

Run (from `artifact/CaseStudy`, image `mapminer` must be loaded):

```bash
docker run --rm -v "$(pwd):/case:Z" mapminer \
  ebmc --systemverilog --top flat --k-induction --bound 10 \
  /case/Precomputed/nru_proof.sv | tee testdata/ebmc_kind_sample.txt
```

Expected: output contains a PROVED line for the Mapminer assertion (per `PRECOMPUTED.md`: `flat.assert.8: PROVED`). Confirm the fixture file is non-empty and records the exact result wording/format.

- [ ] **Step 2: Write the failing test (against the captured fixture)**

Append to `test_gen_invariants.py`:

```python
def test_parse_ebmc_results_reads_fixture():
    sample = open(os.path.join(HERE, "testdata", "ebmc_kind_sample.txt")).read()
    res = g.parse_ebmc_results(sample)
    # res maps property-name -> bool(proved). The Mapminer property must be proved.
    assert any(name.endswith("assert.8") for name in res), res
    assert all(v is True for v in res.values()), res
    assert g.all_proved(res) is True


def test_all_proved_false_on_refuted():
    res = {"flat.assert.0": True, "flat.c_3": False}
    assert g.all_proved(res) is False
    assert g.failed_properties(res) == ["flat.c_3"]
```

- [ ] **Step 3: Run test to verify it fails**

Run: `cd artifact/CaseStudy && python3 test_gen_invariants.py`
Expected: FAIL — `AttributeError: ... 'parse_ebmc_results'`

- [ ] **Step 4: Write minimal implementation (match the fixture's wording)**

Append to `gen_invariants.py`. Adjust the `PROVED`/`REFUTED` token set ONLY if the captured fixture uses different words:

```python
import re

_RESULT_RE = re.compile(
    r"^(?P<name>[\w.\[\]:$]+):\s*(?P<verdict>PROVED|REFUTED|FAILURE|FAILED|"
    r"SUCCESS|UNKNOWN|INCONCLUSIVE)\b",
    re.MULTILINE,
)
_PROVED = {"PROVED", "SUCCESS"}


def parse_ebmc_results(stdout: str) -> dict:
    """Map EBMC property name -> True if proved, else False."""
    out = {}
    for m in _RESULT_RE.finditer(stdout):
        out[m.group("name")] = m.group("verdict").upper() in _PROVED
    return out


def all_proved(results: dict) -> bool:
    return bool(results) and all(results.values())


def failed_properties(results: dict) -> list:
    return [n for n, ok in results.items() if not ok]
```

- [ ] **Step 5: Run test to verify it passes**

Run: `cd artifact/CaseStudy && python3 test_gen_invariants.py`
Expected: PASS (if FAIL because EBMC uses other wording, extend `_RESULT_RE`/`_PROVED` to match the fixture, then re-run)

- [ ] **Step 6: Commit**

```bash
cd /home/magna/Projects/published/smartVerilog
git add artifact/CaseStudy/gen_invariants.py artifact/CaseStudy/test_gen_invariants.py artifact/CaseStudy/testdata/ebmc_kind_sample.txt
git commit -m "feat(casestudy): parse EBMC k-induction results (fixture-backed)"
```

---

## Task 6: Build the per-clause screening harness

The refinement loop needs to know *which* clauses break k-induction. Strategy: emit each mined clause as its own labeled assert (`c_0: assert(clause0);` …) alongside the base property; EBMC then reports a per-label verdict, so failing clauses can be dropped. This uses EBMC itself as the evaluator (no custom SV expression interpreter — out of scope, YAGNI). It is heuristic: a clause that fails alone may have been inductive within the full set, so convergence is not guaranteed (spec §5).

**Files:**
- Modify: `artifact/CaseStudy/gen_invariants.py`
- Test: `artifact/CaseStudy/test_gen_invariants.py`

- [ ] **Step 1: Write the failing test**

Append to `test_gen_invariants.py`:

```python
def test_build_screening_harness_labels_each_clause():
    template = open(NRU_PROOF).read()
    clauses = ["(!c1_hit || c2_rst)", "(!rst || c1_rst)"]
    out, labels = g.build_screening_harness(template, clauses)
    assert labels == ["c_0", "c_1"]
    assert "c_0: assert((!c1_hit || c2_rst));" in out
    assert "c_1: assert((!rst || c1_rst));" in out
    # base property kept as its own labeled assert too
    assert "base_p: assert((!check || eq_hit));" in out
    assert out.startswith("module flat (")
    assert out.rstrip().endswith("endmodule")
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd artifact/CaseStudy && python3 test_gen_invariants.py`
Expected: FAIL — `AttributeError: ... 'build_screening_harness'`

- [ ] **Step 3: Write minimal implementation**

Append to `gen_invariants.py`:

```python
def build_screening_harness(template_text: str, clauses: list[str],
                            marker: str = "Mapminer Properties"):
    """Replace the Mapminer assert with one labeled assert per clause.

    Returns (harness_text, labels) where labels[i] is the EBMC property suffix
    for clauses[i].
    """
    prefix, _old, suffix = split_around_marker_assert(template_text, marker)
    lines = [f"base_p: assert({BASE_PROPERTY});"]
    labels = []
    for i, c in enumerate(clauses):
        label = f"c_{i}"
        labels.append(label)
        lines.append(f"{label}: assert({_wrap(c)});")
    indent = " " * 12
    block = ("\n" + indent).join(lines)
    return prefix + block + suffix, labels
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cd artifact/CaseStudy && python3 test_gen_invariants.py`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
cd /home/magna/Projects/published/smartVerilog
git add artifact/CaseStudy/gen_invariants.py artifact/CaseStudy/test_gen_invariants.py
git commit -m "feat(casestudy): per-clause screening harness builder"
```

---

## Task 7: Refinement selection (pure, fixture-driven)

**Files:**
- Modify: `artifact/CaseStudy/gen_invariants.py`
- Test: `artifact/CaseStudy/test_gen_invariants.py`

- [ ] **Step 1: Write the failing test**

Append to `test_gen_invariants.py`:

```python
def test_select_surviving_clauses_drops_failing_labels():
    clauses = ["(a)", "(b)", "(c)"]
    labels = ["c_0", "c_1", "c_2"]
    # EBMC says c_1 failed k-induction (label may be prefixed by top name).
    results = {"flat.base_p": True, "flat.c_0": True,
               "flat.c_1": False, "flat.c_2": True}
    survivors = g.select_surviving_clauses(clauses, labels, results)
    assert survivors == ["(a)", "(c)"]


def test_select_surviving_clauses_all_pass():
    clauses = ["(a)"]
    labels = ["c_0"]
    assert g.select_surviving_clauses(clauses, labels,
                                      {"flat.c_0": True}) == ["(a)"]
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd artifact/CaseStudy && python3 test_gen_invariants.py`
Expected: FAIL — `AttributeError: ... 'select_surviving_clauses'`

- [ ] **Step 3: Write minimal implementation**

Append to `gen_invariants.py`:

```python
def select_surviving_clauses(clauses: list[str], labels: list[str],
                             results: dict) -> list[str]:
    """Keep clauses whose screening label proved (label may carry a top prefix)."""
    survivors = []
    for clause, label in zip(clauses, labels):
        ok = next((v for n, v in results.items()
                   if n == label or n.endswith("." + label)), None)
        if ok:
            survivors.append(clause)
    return survivors
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cd artifact/CaseStudy && python3 test_gen_invariants.py`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
cd /home/magna/Projects/published/smartVerilog
git add artifact/CaseStudy/gen_invariants.py artifact/CaseStudy/test_gen_invariants.py
git commit -m "feat(casestudy): pure refinement clause selection"
```

---

## Task 8: Orchestrator `main()` + `--dry-run`

Wires the pure pieces with SMART/EBMC subprocess calls. `--dry-run` exercises the full flow on host using `Precomputed/nru_proof.sv`'s own clauses as a stand-in for SMART output and skipping EBMC, so the wiring is testable without Docker.

**Files:**
- Modify: `artifact/CaseStudy/gen_invariants.py`
- Test: `artifact/CaseStudy/test_gen_invariants.py`

- [ ] **Step 1: Write the failing test**

Append to `test_gen_invariants.py`:

```python
import subprocess
import sys
import tempfile


def test_dry_run_emits_valid_harness(tmp_path=None):
    out_dir = tempfile.mkdtemp()
    rc = subprocess.call([sys.executable, os.path.join(HERE, "gen_invariants.py"),
                          "--dry-run", "--out-dir", out_dir])
    assert rc == 0
    proof = os.path.join(out_dir, "nru_proof.sv")
    assert os.path.exists(proof)
    text = open(proof).read()
    assert text.startswith("module flat (")
    assert "(!check || eq_hit)" in text
    assert os.path.exists(os.path.join(out_dir, "assertions.txt"))
    assert os.path.exists(os.path.join(out_dir, "gen_nru.log"))
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd artifact/CaseStudy && python3 test_gen_invariants.py`
Expected: FAIL — `gen_invariants.py` has no CLI / unknown `--dry-run`

- [ ] **Step 3: Write minimal implementation**

Append to `gen_invariants.py`:

```python
import argparse
import json
import os
import shutil
import subprocess
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
PRECOMPUTED_NRU = HERE / "Precomputed" / "nru_proof.sv"
SMART_ROOT = Path("/workspace/smartVerilog")        # inside container
BENCH = "nru_flat"
EBMC_CMD = ["ebmc", "--systemverilog", "--top", "flat",
            "--k-induction", "--bound", "10"]


def read_mined_clauses(assertions_path: Path) -> list[str]:
    lines = []
    for ln in assertions_path.read_text().splitlines():
        ln = ln.strip()
        if ln:
            lines.append(ln)
    return lines


def run_ebmc(proof_path: Path) -> dict:
    proc = subprocess.run(EBMC_CMD + [str(proof_path)],
                           capture_output=True, text=True)
    return parse_ebmc_results(proc.stdout + proc.stderr)


def _prepare_bench(template: str, work: Path) -> Path:
    bench_dir = work / BENCH
    bench_dir.mkdir(parents=True, exist_ok=True)
    (bench_dir / f"{BENCH}.sv").write_text(derive_flat_design(template))
    (bench_dir / "sim.py").write_text(make_flat_sim_py(f"{BENCH}.sv"))
    return bench_dir


def _mine_with_smart(bench_dir: Path, unbounded: bool) -> list[str]:
    dest = SMART_ROOT / "Benchmark" / BENCH
    shutil.rmtree(dest, ignore_errors=True)
    shutil.copytree(bench_dir, dest)
    config = "Config/smart.json"
    if unbounded:
        cfg = json.loads((SMART_ROOT / "Config" / "smart.json").read_text())
        cfg["Checker_settings"]["Check_unbounded"] = True
        cfg["Evaluation_settings"]["Check_unbounded"] = True
        cpath = SMART_ROOT / "Config" / "smart_unbounded.json"
        cpath.write_text(json.dumps(cfg))
        config = "Config/smart_unbounded.json"
    subprocess.run(["python", "run.py", BENCH, config],
                   cwd=SMART_ROOT, check=True)
    return read_mined_clauses(
        SMART_ROOT / "Results" / f"smart_{BENCH}" / "assertions.txt")


def generate(out_dir: Path, dry_run: bool, max_iter: int, budget_s: int) -> int:
    template = PRECOMPUTED_NRU.read_text()
    out_dir.mkdir(parents=True, exist_ok=True)
    log = []
    started = time.time()

    def emit(clauses, proved, note):
        (out_dir / "nru_proof.sv").write_text(
            assemble_proof_harness(template, clauses))
        (out_dir / "assertions.txt").write_text("\n".join(clauses) + "\n")
        log.append(note)
        (out_dir / "gen_nru.log").write_text("\n".join(log) + "\n")
        return 0 if proved else 1

    if dry_run:
        _, old_assert, _ = split_around_marker_assert(template)
        clauses = [ln.strip().rstrip("&").strip()
                   for ln in old_assert.splitlines()
                   if ln.strip().startswith("(!") or ln.strip().startswith("(c")]
        return emit(clauses or [BASE_PROPERTY], True,
                    "dry-run: reused template clauses, EBMC skipped")

    work = Path(subprocess.run(["mktemp", "-d"], capture_output=True,
                               text=True, check=True).stdout.strip())
    bench_dir = _prepare_bench(template, work)
    clauses = _mine_with_smart(bench_dir, unbounded=False)
    log.append(f"mined {len(clauses)} clauses from SMART")
    if not clauses:
        return emit([BASE_PROPERTY], False, "FAIL: SMART produced no clauses")

    re_mined = False
    for it in range(max_iter):
        if time.time() - started > budget_s:
            log.append(f"budget {budget_s}s exhausted at iter {it}")
            break
        proof = out_dir / "nru_proof.sv"
        proof.write_text(assemble_proof_harness(template, clauses))
        results = run_ebmc(proof)
        log.append(f"iter {it}: gate {results}")
        if all_proved(results):
            return emit(clauses, True, f"PROVED at iter {it}")
        scr, labels = build_screening_harness(template, clauses)
        scr_path = out_dir / "screening.sv"
        scr_path.write_text(scr)
        scr_results = run_ebmc(scr_path)
        survivors = select_surviving_clauses(clauses, labels, scr_results)
        log.append(f"iter {it}: {len(survivors)}/{len(clauses)} clauses survive")
        if survivors and survivors != clauses:
            clauses = survivors
            continue
        if not re_mined:
            re_mined = True
            clauses = _mine_with_smart(bench_dir, unbounded=True) or clauses
            log.append(f"re-mined (unbounded): {len(clauses)} clauses")
            continue
        break
    return emit(clauses, False, "FAIL: no k-inductive set within budget")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--out-dir",
                    default=str(HERE / "Generated" / "gen_nru"))
    ap.add_argument("--max-iter", type=int, default=6)
    ap.add_argument("--budget-s", type=int, default=1800)
    a = ap.parse_args()
    return generate(Path(a.out_dir), a.dry_run, a.max_iter, a.budget_s)


if __name__ == "__main__":
    raise SystemExit(main())
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cd artifact/CaseStudy && python3 test_gen_invariants.py && python3 -c "import test_gen_invariants as t; t.test_dry_run_emits_valid_harness()"`
Expected: PASS — all `ok` lines, and dry-run produces `nru_proof.sv`, `assertions.txt`, `gen_nru.log`

- [ ] **Step 5: Run full pure suite**

Run: `cd artifact/CaseStudy && python3 test_gen_invariants.py`
Expected: every `test_*` prints `ok`

- [ ] **Step 6: Commit**

```bash
cd /home/magna/Projects/published/smartVerilog
git add artifact/CaseStudy/gen_invariants.py artifact/CaseStudy/test_gen_invariants.py
git commit -m "feat(casestudy): orchestrator with dry-run + refinement loop"
```

---

## Task 9: In-container entry script + `make gen-nru`

**Files:**
- Create: `artifact/CaseStudy/_gen_nru_entry.sh`
- Modify: `artifact/CaseStudy/Makefile`

- [ ] **Step 1: Create the entry script**

`artifact/CaseStudy/_gen_nru_entry.sh`:

```bash
#!/usr/bin/env bash
set -euo pipefail
# Runs INSIDE the mapminer container. Mirrors run.py's env setup.
cd /workspace/smartVerilog
source otherTools/venv/bin/activate 2>/dev/null || true
source otherTools/oss-cad-suite/environment 2>/dev/null || true
exec python3 /case/gen_invariants.py --out-dir /case/Generated/gen_nru "$@"
```

Make it executable:

```bash
chmod +x artifact/CaseStudy/_gen_nru_entry.sh
```

- [ ] **Step 2: Add the Makefile target**

In `artifact/CaseStudy/Makefile`, add `gen-nru` to `.PHONY` (line 1) and append this target (reuses the existing `ENGINE`/`IMAGE` vars and the `proof-nru` mount pattern):

```make
gen-nru:
	$(ENGINE) run --rm -v "$$(pwd):/case:Z" $(IMAGE) \
		bash -lc 'chmod +x /case/_gen_nru_entry.sh && /case/_gen_nru_entry.sh'
```

- [ ] **Step 3: Verify Make wiring without Docker (dry-run path)**

Run: `cd artifact/CaseStudy && python3 gen_invariants.py --dry-run --out-dir /tmp/gen_nru_check && ls /tmp/gen_nru_check`
Expected: lists `assertions.txt  gen_nru.log  nru_proof.sv`

- [ ] **Step 4: Verify the Makefile target parses**

Run: `cd artifact/CaseStudy && make -n gen-nru`
Expected: prints the `docker run ... _gen_nru_entry.sh` command, no Make errors

- [ ] **Step 5: Commit**

```bash
cd /home/magna/Projects/published/smartVerilog
git add artifact/CaseStudy/_gen_nru_entry.sh artifact/CaseStudy/Makefile
git commit -m "feat(casestudy): make gen-nru container entrypoint"
```

---

## Task 10: End-to-end validation in the container + docs

**Files:**
- Modify: `artifact/CaseStudy/README.md`

- [ ] **Step 1: Confirm the image is available**

Run: `docker image inspect mapminer >/dev/null 2>&1 && echo IMAGE_OK || echo "load image.tar.gz first (docker load -i ../image.tar.gz)"`
Expected: `IMAGE_OK`. If not, load the image before continuing.

- [ ] **Step 2: Run the real pipeline end-to-end**

Run: `cd artifact/CaseStudy && make gen-nru; echo "exit=$?"`
Expected: completes; `Generated/gen_nru/nru_proof.sv`, `assertions.txt`, `gen_nru.log` exist. Inspect `gen_nru.log` for the per-iteration trace and final verdict.

- [ ] **Step 3: Independently verify the generated harness proves**

Run:
```bash
cd artifact/CaseStudy && docker run --rm -v "$(pwd):/case:Z" mapminer \
  ebmc --systemverilog --top flat --k-induction --bound 10 \
  /case/Generated/gen_nru/nru_proof.sv | tee Generated/gen_nru/ebmc_verify.txt | tail -5
```
Expected: a PROVED verdict for the Mapminer property (same wording as the Task 5 fixture). This is the spec §3 success criterion. If REFUTED/budget-exhausted, record the `gen_nru.log` outcome — the script's non-zero exit and log are the honest result; do not claim success.

- [ ] **Step 4: Document the new path in README**

In `artifact/CaseStudy/README.md`, after the "Optional live MAPminer runs" section, add:

```markdown
## Automatic NRU Invariant Regeneration (optional)

`make gen-nru` runs the full mine → assemble → EBMC k-induction loop inside
the container and emits a standalone proof harness:

```bash
make gen-nru
```

Outputs (kept separate from the `make invariants` extract path):

```text
Generated/gen_nru/nru_proof.sv     # regenerated standalone harness
Generated/gen_nru/assertions.txt   # mined clause set used
Generated/gen_nru/gen_nru.log      # per-iteration refinement trace + verdict
```

SMART mining is heuristic and the refinement loop is bounded, so this path is
not guaranteed to converge to a k-inductive set. `make invariants` and
`Precomputed/nru_proof.sv` remain the deterministic reviewer fallback.
```

- [ ] **Step 5: Final commit**

```bash
cd /home/magna/Projects/published/smartVerilog
git add artifact/CaseStudy/README.md artifact/CaseStudy/Generated/gen_nru/ebmc_verify.txt
git commit -m "docs(casestudy): document make gen-nru + record EBMC verification"
```

---

## Self-Review

**1. Spec coverage:**
- §4.1 container model → Task 9 (`make gen-nru`, entry script, single `docker run`).
- §4.2 mining input = flattened design derived from Precomputed → Task 4 (`derive_flat_design`, `make_flat_sim_py`).
- §4.3 components → Tasks 1–9 (split, assemble, deriver, parser, screening, orchestrator, Makefile).
- §4.4 refinement loop (mine → assemble → gate → refine → emit) → Task 8 `generate()`.
- §4.5 make integration & fallback → Task 9 + Task 10 Step 4 (README); `make invariants`/`extract_invariants.py` never edited; `Generated/gen_nru/` separation noted up front.
- §3 success criterion (EBMC `PROVED`) → Task 8 gate + Task 10 Step 3 independent verification.
- §5 risks (non-convergence, honest failure) → Task 8 budget + non-zero exit + log; documented in README.
- §6 assumptions (image, run.py output path, marker stability) → Task 5 Step 1, Task 8 `_mine_with_smart`, Task 10 Step 1.
- §7 pipeline validation in image → Task 10. No gaps found.

**2. Placeholder scan:** No TBD/TODO/"handle edge cases"/"similar to Task N". Every code step shows complete code. The only conditional is Task 5 Step 4 (extend regex if fixture wording differs) — this is an explicit, bounded adjustment against a captured fixture, not a placeholder.

**3. Type consistency:** Names are consistent across tasks: `split_around_marker_assert` → (prefix, assert_block, suffix); `build_mapminer_assert(clauses)`; `assemble_proof_harness(template, clauses)`; `derive_flat_design`/`make_flat_sim_py`; `parse_ebmc_results`→dict, `all_proved`/`failed_properties`; `build_screening_harness`→(text, labels); `select_surviving_clauses(clauses, labels, results)`; orchestrator `generate()`/`main()`. `BASE_PROPERTY` and `_wrap` defined once (Task 2) and reused (Tasks 6, 8). Output filenames (`nru_proof.sv`, `assertions.txt`, `gen_nru.log`) consistent across Tasks 8–10.
