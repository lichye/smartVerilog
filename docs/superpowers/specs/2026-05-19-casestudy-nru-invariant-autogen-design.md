# Design: Automatic NRU Invariant / Proof-Harness Generation (Case Study)

- **Date:** 2026-05-19
- **Scope:** `artifact/CaseStudy` — NRU only (PLRU deferred)
- **Status:** Approved (design); pending spec review

## 1. Problem & Context

The Section 6 / Table 2 case study ships two author-curated, hand-flattened
proof harnesses, `Precomputed/nru_proof.sv` and `Precomputed/plru_proof.sv`.
Each contains a flattened two-instance cache design (`flat` / `flat_a`), a
scenario driver, and a `//Mapminer Properties` assertion block (the security
property `(!check||eq_hit)` conjoined with SMART-mined invariants) that EBMC
k-induction proves (`flat.assert.8: PROVED`).

Today `make invariants` only **text-extracts** the pre-baked assertion block
from those files (`extract_invariants.py`). There is no automated path that
actually **re-runs the synthesizer** to regenerate the invariants and rebuild
the proof harness. The MAPminer config originally used for this case study was
the plain **SMART baseline** (`Config/smart.json`: `Blockified=false`,
`Parallel=false`, `Minimizer=false`) — the heavy block/MSA/parallel machinery
is not needed for this small single-property target.

Goal: an automatic, container-based pipeline that mines NRU invariants with
SMART, assembles a full standalone proof harness like
`Precomputed/nru_proof.sv`, and verifies it with EBMC k-induction.

## 2. Goals / Non-Goals

**Goals**
- Automatically (re)generate a standalone `Generated/nru/nru_proof.sv`.
- Mine via SMART using `Config/smart.json`, on the flattened design.
- Verify the generated harness with EBMC k-induction as a hard gate.
- Run entirely inside the existing Docker image (single host entrypoint).
- Keep a refinement loop so a heuristic SMART dump can be driven toward a
  k-inductive conjunction (Approach C).

**Non-Goals**
- PLRU (added later once NRU works end-to-end).
- Mutating `Precomputed/*` (stays canonical, untouched).
- Byte-identical reproduction of the author-curated clause set.
- Host-native execution (host lacks `oss-cad-suite`; Docker-only).
- Replacing `make invariants` — it stays as the deterministic, no-Docker
  fallback / reviewer path.

## 3. Success Criteria

Binary and observable: `Generated/nru/nru_proof.sv`, fed to the same command
as `make proof-nru`
(`ebmc --systemverilog --top flat --k-induction --bound 10`), reports
`flat.assert.8: PROVED`. The pipeline's exit status reflects that result;
success is never claimed without the EBMC output as evidence.

## 4. Architecture

### 4.1 Container model
A new host target `make gen-nru` performs a single `docker run` that mounts
`CaseStudy/` into the `mapminer` image and invokes an in-container
orchestrator. All heavy work — SMART mining, EBMC k-induction, the refinement
loop — runs **inside** the container, where `/workspace/smartVerilog`
(`run.py`, `Config/smart.json`, `smart/`), `yosys`, `cvc5`, `cocotb`,
`verilator`, and `ebmc` live. Only the final artifacts cross back to the host.
This unifies the existing `run_mapminer.sh` (mining) and `proof-nru` (EBMC)
patterns into one entrypoint.

### 4.2 Mining input = the flattened design
SMART mined on the flattened design, so the mining benchmark is derived from
the ground truth rather than the modular `Input/nru`:
- From `Precomputed/nru_proof.sv`, strip the `//Mapminer Properties` assert
  (reuse the `extract_invariants.py` marker/paren-balance logic) → `nru_flat.sv`
  (top `flat`; base property and FMCAD block retained as comments).
- Generate a small cocotb `sim.py` with `hdl_toplevel="flat"` that clocks the
  module and lets the `(* anyseq *)` free regs be driven randomly so SMART's
  pre-analysis can collect traces.
This guarantees mined clauses are in the exact
`c1_*/c2_*/counter/stable/check` namespace the harness expects — no
name-mapping guesswork.

### 4.3 Components
| Component | Location | Responsibility |
|---|---|---|
| `make gen-nru` | `CaseStudy/Makefile` | Host entrypoint: one `docker run`, mount `CaseStudy/`, invoke orchestrator, surface exit status |
| Orchestrator | `CaseStudy/gen_invariants.py` (runs in container) | Drives derive → mine → assemble → gate → refine; writes outputs + log |
| Flat-benchmark deriver | part of orchestrator (shared marker logic with `extract_invariants.py`) | Produce `nru_flat.sv` + `sim.py` for mining |
| Assembler | part of orchestrator | Conjoin `(!check||eq_hit)` with mined clauses; splice into template at `//Mapminer Properties` anchor |
| EBMC gate | in-container call | `ebmc --systemverilog --top flat --k-induction --bound 10` |
| Outputs | `Generated/nru/` | `nru_proof.sv`, `assertions.txt`, `gen_nru.log` |

### 4.4 Refinement loop (Approach C)
1. **Mine:** copy derived flat benchmark into container `Benchmark/<bench>/`;
   run `python run.py <bench> Config/smart.json`; collect
   `Results/smart_<bench>/assertions.txt` (mined clause list).
2. **Assemble:** conjoin base property with all mined clauses; splice into the
   `nru_flat.sv` template → candidate `nru_proof.sv`.
3. **Gate:** run EBMC k-induction. If `flat.assert.8: PROVED` → done.
4. **Refine** (only on gate failure): use the EBMC k-induction
   counterexample to greedily drop clause(s) implicated in the failed
   induction step; re-run the gate on the reduced conjunction. If the reduced
   set still fails, re-mine once with `Check_unbounded: true`. Bounded by a
   max-iteration count and a wallclock budget.
5. **Emit:** write final `Generated/nru/nru_proof.sv`, `assertions.txt`, and
   `gen_nru.log` (iterations, final clause set, PROVED/FAILED, EBMC output).

### 4.5 Make integration & fallback
- `make gen-nru` is new; NRU only.
- `make invariants` / `extract_invariants.py` unchanged — remains the
  deterministic, no-Docker fallback and documented reviewer path.
- `Precomputed/nru_proof.sv` never mutated; remains canonical.
- On budget exhaustion without `PROVED`: exit non-zero, keep the best
  candidate + full log, and point the reviewer to `make invariants` /
  Precomputed. No silent success.

## 5. Risks & Mitigations

| Risk | Mitigation |
|---|---|
| SMART verifies each clause is a *true* invariant but their conjunction need not be **k-inductive**; a raw dump may fail the gate | The refinement loop (4.4) + honest non-zero exit + Precomputed fallback. We do not assume first-dump success. |
| EBMC k-induction CEX shows the induction step failed but not minimally *which* clause to drop; refinement is greedy and may not converge | Bound by max-iterations + wallclock; fail honestly rather than loop forever. Convergence is explicitly not guaranteed. |
| Derived `flat`-top `sim.py` must yield traces SMART can mine (free `anyseq` regs driven randomly) | Validate the mining step produces a non-empty `assertions.txt` inside the image before trusting the loop; treat empty mining as a hard failure with a clear message. |
| Container resource/time (SMART + repeated EBMC) | Reuse `TIMEOUT_SEC`/budget knobs analogous to existing Makefile vars; surface progress in `gen_nru.log`. |

## 6. Assumptions

- The `mapminer` Docker image is loaded and contains `/workspace/smartVerilog`
  (`run.py`, `Config/smart.json`, `smart/`), `ebmc`, `yosys`, `cvc5`,
  `cocotb`, `verilator`.
- `run.py <bench> Config/smart.json` mines state invariants (non-LTL) over the
  flattened benchmark and writes `Results/smart_<bench>/assertions.txt`, one
  clause per line, in the `flat` namespace.
- The `//Mapminer Properties` marker convention in the proof harness is stable
  (already relied on by `extract_invariants.py`).

## 7. Validation of the Pipeline Itself

End-to-end validation runs inside the image: invoke `make gen-nru`, confirm a
well-formed `Generated/nru/nru_proof.sv` is produced, and confirm the EBMC gate
prints `flat.assert.8: PROVED`. No success is asserted from the host without
that EBMC output. (Per `superpowers:verification-before-completion`.)
