#!/usr/bin/env python3
"""Compare the trace policies on the isolation-study log.

MD is the metric. Assertion count cannot settle it: richer positive states
raise the count when they rescue a block from wasted refinement, lower it when
they make the block infeasible, and leave it alone while WEAKENING each
invariant -- which shows up only in MD.
"""
import collections, statistics, sys

rows = collections.defaultdict(dict)
for line in open(sys.argv[1] if len(sys.argv) > 1 else "/work/pol.log").read().splitlines()[1:]:
    p = line.split()
    if len(p) >= 6 and p[2].isdigit():
        rows[p[1]][p[0]] = dict(assertions=int(p[2]), secs=int(p[3].rstrip('s')),
                                md=float(p[4]), mutants=int(p[5]),
                                states=int(p[6]) if len(p) > 6 else 0,
                                rand_states=int(p[7]) if len(p) > 7 else 0)
    elif len(p) >= 3:
        rows[p[1]][p[0]] = None

both = [d for d in rows if rows[d].get('random') and rows[d].get('fuzz')]
if not both:
    print("no design has both policies yet"); sys.exit(0)

print(f"{'design':<9}{'rand MD':>9}{'fuzz MD':>9}{'ΔMD':>8}   "
      f"{'rand A':>7}{'fuzz A':>7}   {'rand s':>7}{'fuzz s':>7}   {'状态 r/f':>10}")
dmd, dt = [], []
for d in sorted(both):
    r, f = rows[d]['random'], rows[d]['fuzz']
    dmd.append(f['md'] - r['md']); dt.append(f['secs'] - r['secs'])
    mark = " <<" if abs(f['md'] - r['md']) >= 1 else ""
    print(f"{d:<9}{r['md']:>8.1f}%{f['md']:>8.1f}%{f['md']-r['md']:>+8.2f}   "
          f"{r['assertions']:>7}{f['assertions']:>7}   {r['secs']:>6}s{f['secs']:>6}s   "
          f"{f['rand_states']:>4}/{f['states']:<5}{mark}")

print(f"\nn={len(both)}")
print(f"  MD  均值 {statistics.mean(dmd):+.2f}pp  中位 {statistics.median(dmd):+.2f}pp"
      f"  更好 {sum(1 for x in dmd if x > 0.05)}  更差 {sum(1 for x in dmd if x < -0.05)}"
      f"  持平 {sum(1 for x in dmd if abs(x) <= 0.05)}")
print(f"  时间 合计 {sum(rows[d]['random']['secs'] for d in both)}s -> "
      f"{sum(rows[d]['fuzz']['secs'] for d in both)}s")

# Does the benefit follow how far random was from the ceiling?
gap = [(rows[d]['fuzz']['states'] - rows[d]['fuzz']['rand_states'], rows[d]['fuzz']['md'] - rows[d]['random']['md'])
       for d in both if rows[d]['fuzz']['states']]
if len(gap) > 2:
    xs = [g for g, _ in gap]; ys = [m for _, m in gap]
    mx, my = statistics.mean(xs), statistics.mean(ys)
    num = sum((x-mx)*(y-my) for x, y in gap)
    den = (sum((x-mx)**2 for x in xs) * sum((y-my)**2 for y in ys)) ** 0.5
    print(f"  额外状态数 vs ΔMD 的相关系数: {num/den:+.2f}" if den else "")
