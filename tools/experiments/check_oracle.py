#!/usr/bin/env python3
"""Soundness oracle for the end-of-run minimiser.

The minimiser only removes an assertion A that the kept set S implies. Any
state violating A therefore violates some member of S, so every mutant A could
detect is still detected by S. The MD rate has to come out bit-identical.

A difference means one of:
  - the minimiser dropped something S does not imply  (our bug)
  - EBMC judged the same mutant two ways              (its bug, or a timeout
    moving a mutant in or out of the denominator)

`block_msa` and `block_msa_mini` are the same run up to the final filter
(seed 42, same round count, same mined set), so the comparison is exact.
Usage: check_oracle.py [all.log]
"""
import collections, json, os, sys

log = sys.argv[1] if len(sys.argv) > 1 else "/work/all.log"
rows = collections.defaultdict(dict)
for line in open(log).read().splitlines()[1:]:
    p = line.split()
    if len(p) >= 5:
        try:
            rows[p[1]][p[0]] = (int(p[2]), p[3], float(p[4]))
        except ValueError:
            pass

def trajectory(cfg, design):
    """(rounds, mined) from the run log, so we can prove the runs match."""
    path = f"/work/wk/{cfg}_{design}/run-log.jsonl"
    if not os.path.exists(path):
        return None
    recs, run = [], None
    for line in open(path):
        try:
            d = json.loads(line)
        except json.JSONDecodeError:
            continue
        # One log can hold two runs if a workdir was deleted under a live
        # writer; keep the last one only.
        if run is None or d.get("run") != run:
            run = d.get("run")
        recs.append(d)
    rounds = [d for d in recs if d.get("stage") == "round"]
    return (len(rounds), rounds[-1].get("total")) if rounds else None

bad, checked = [], 0
print(f"{'design':<10}{'msa':>7}{'mini':>7}{'削减':>8}{'msa MD':>9}{'mini MD':>9}{'ΔMD':>8}  轨迹")
for design, v in sorted(rows.items()):
    a, b = v.get("block_msa"), v.get("block_msa_mini")
    if not (a and b):
        continue
    checked += 1
    d = b[2] - a[2]
    ta, tb = trajectory("block_msa", design), trajectory("block_msa_mini", design)
    same = "同" if ta and tb and ta == tb else f"差异 {ta} vs {tb}"
    flag = "  <== 查" if abs(d) > 1e-9 else ""
    print(f"{design:<10}{a[0]:>7}{b[0]:>7}{100*(a[0]-b[0])/max(a[0],1):>7.1f}%"
          f"{a[2]:>8.1f}%{b[2]:>8.1f}%{d:>+8.2f}  {same}{flag}")
    if abs(d) > 1e-9 or same != "同":
        bad.append((design, d, same))

print(f"\n核对 {checked} 个设计;{len(bad)} 个异常")
for design, d, same in bad:
    print(f"  {design}: ΔMD={d:+.2f}  轨迹{same}")
sys.exit(1 if bad else 0)
