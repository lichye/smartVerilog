#!/usr/bin/env python3
"""WP2 acceptance: hw-cbmc frontend vs the frozen regex oracle.

Unlike tools/parity_frontend.py — which demands byte parity between the C++
regex frontend and gen_bench.py, and stops at the first mismatch — this walks
every design and CLASSIFIES the divergences, because hw-cbmc is expected to
disagree with the oracle wherever the regex was guessing.

Usage:
    python3 tools/compare_frontends.py [build/bin/hwcbmc_dump] [build/bin/frontend_dump]

Divergence classes (see docs/PLAN-cpp-single-binary.md WP2):
  width-resolved      oracle said null, hw-cbmc resolved a width  -> hw-cbmc right
  param-extra         hw-cbmc reports a constant the oracle missed-> hw-cbmc right
  order               port order differs                          -> hw-cbmc right
  width-differs       both resolved, values differ                -> NEEDS REVIEW
  param-missing       oracle reports a constant hw-cbmc dropped   -> NEEDS REVIEW
  param-value         same name, different value                  -> NEEDS REVIEW
  other               anything else                               -> NEEDS REVIEW

Why `order` is benign: hw-cbmc reports ports in module-header order, which is
the declaration order. The oracle scans the `input`/`output` declarations of a
non-ANSI header instead, so it emits all inputs before all outputs — e.g. s27
is declared `module s27(GND,VDD,CK,G0,G1,G17,G2,G3)` and the oracle moves G17
to the end. Order matters for stimulus reproducibility (it fixes the sequence
of RNG draws), not for correctness, and hw-cbmc has the right answer.

Designs in KNOWN_LIMITATIONS below diverge for a reason already understood and
recorded; they are reported but do not fail the run.

Exit 0 when every divergence is benign or a known limitation, 1 otherwise.
"""

import json
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from parity_frontend import ROOT, collect_designs  # noqa: E402

BENIGN = {"width-resolved", "param-extra", "order"}

# design (path suffix) -> why it diverges. Reported, not fatal.
KNOWN_LIMITATIONS = {
    "i2c_slave_wbm/i2c_slave_wbm.sv":
        "hw-cbmc 5.6 aborts elaborating this design (invariant violation in "
        "verilog_synthesis) — upstream `ebmc --show-symbol-table` on the same "
        "two files dumps core too. We degrade to parse-tree ports, losing "
        "parameters and parameterized widths.",
    "i2c_slave_axil_master/i2c_slave_axil_master.sv":
        "same hw-cbmc 5.6 elaboration abort as i2c_slave_wbm.",
    "other/s1196/s1196.sv":
        "hw-cbmc 5.6 aborts on this design's 2-argument instantiations of the "
        "3-port `dff` module; degrades to parse-tree ports.",
    "ibex_controller/ibex_controller.sv":
        "ORACLE bug: its regex truncates a port name to 'inp'. hw-cbmc is "
        "right.",
    "ibex_decoder/ibex_decoder.sv":
        "ORACLE bug: three port names truncated to 'inp'. hw-cbmc is right.",
}


def known_limitation(name):
    for suffix, reason in KNOWN_LIMITATIONS.items():
        if name.endswith(suffix):
            return reason
    return None


def dump(exe, rtl, top):
    if not os.path.exists(exe):
        print(f"error: {exe} is missing. Build it with:\n"
              f"    cmake --build build --target {os.path.basename(exe)}",
              file=sys.stderr)
        sys.exit(2)
    out = subprocess.run([exe, rtl, top], capture_output=True, text=True)
    if out.returncode != 0:
        return None, (out.stderr.strip() or f"exit {out.returncode}")
    try:
        return json.loads(out.stdout), None
    except json.JSONDecodeError as e:
        return None, f"bad JSON: {e}"


def classify(oracle, hwcbmc):
    """Yield (class, detail) for every difference between the two views."""
    if oracle["top"] != hwcbmc["top"]:
        yield "other", f"top: {oracle['top']} vs {hwcbmc['top']}"

    for key in ("has_assume", "clock", "reset"):
        if oracle[key] != hwcbmc[key]:
            yield "other", f"{key}: {oracle[key]!r} vs {hwcbmc[key]!r}"

    o_params, h_params = oracle["params"], hwcbmc["params"]
    for name in sorted(set(h_params) - set(o_params)):
        yield "param-extra", f"{name} = {h_params[name]}"
    for name in sorted(set(o_params) - set(h_params)):
        yield "param-missing", f"{name} = {o_params[name]}"
    for name in sorted(set(o_params) & set(h_params)):
        if o_params[name] != h_params[name]:
            yield "param-value", f"{name}: {o_params[name]} vs {h_params[name]}"

    for key, label in (("ports", "port"), ("free_regs", "free reg")):
        o_items = {i["name"]: i for i in oracle[key]}
        h_items = {i["name"]: i for i in hwcbmc[key]}
        o_names = [i["name"] for i in oracle[key]]
        h_names = [i["name"] for i in hwcbmc[key]]
        if o_names != h_names:
            only_oracle = [n for n in o_names if n not in h_names]
            only_hwcbmc = [n for n in h_names if n not in o_names]
            if only_oracle or only_hwcbmc:
                yield "other", (f"{label} names differ: "
                                f"oracle-only={only_oracle} "
                                f"hw-cbmc-only={only_hwcbmc}")
            else:
                yield "order", f"{label} order differs"
            continue
        for name, o in o_items.items():
            h = h_items[name]
            for field in ("dir", "kind"):
                if field in o and o[field] != h.get(field):
                    yield "other", f"{label} {name} {field}: {o[field]} vs {h.get(field)}"
            if o["width"] != h["width"]:
                if o["width"] is None and h["width"] is not None:
                    yield "width-resolved", f"{label} {name}: null -> {h['width']}"
                else:
                    yield "width-differs", \
                        f"{label} {name}: {o['width']} vs {h['width']}"


def main():
    hw_exe = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, "build/bin/hwcbmc_dump")
    oracle_exe = sys.argv[2] if len(sys.argv) > 2 else os.path.join(ROOT, "build/bin/frontend_dump")

    designs = collect_designs()
    if len(designs) < 50:
        print(f"error: only found {len(designs)} designs — wrong checkout?")
        return 1

    identical = 0
    counts = {}
    problems = []
    failures = []
    known = {}
    unresolved = {}

    for rtl, top in designs:
        name = os.path.relpath(rtl, ROOT)
        oracle, oracle_err = dump(oracle_exe, rtl, top)
        hwcbmc, hwcbmc_err = dump(hw_exe, rtl, top)

        if oracle is None:
            failures.append((name, f"oracle failed: {oracle_err}"))
            continue
        if hwcbmc is None:
            failures.append((name, f"hw-cbmc failed: {hwcbmc_err}"))
            continue

        diffs = list(classify(oracle, hwcbmc))
        if not diffs:
            identical += 1
            continue
        reason = known_limitation(name)
        for kind, detail in diffs:
            counts[kind] = counts.get(kind, 0) + 1
            if kind in BENIGN:
                continue
            unresolved.setdefault(kind, 0)
            if reason is not None:
                known.setdefault(name, (reason, []))[1].append(f"[{kind}] {detail}")
            else:
                unresolved[kind] += 1
                problems.append((name, kind, detail))

    print(f"designs compared: {len(designs)}")
    print(f"byte-identical:   {identical}")
    for kind in sorted(counts):
        mark = "ok    " if kind in BENIGN or not unresolved.get(kind) else "REVIEW"
        note = "" if kind in BENIGN else "  (all accounted for)" \
            if not unresolved.get(kind) else f"  ({unresolved[kind]} unexplained)"
        print(f"  {mark} {kind:<16} {counts[kind]}{note}")

    if known:
        print(f"\nknown limitations ({len(known)} designs, not fatal):")
        for name, (reason, details) in sorted(known.items()):
            print(f"  {name}: {reason}")
            for detail in details[:4]:
                print(f"      {detail}")
            if len(details) > 4:
                print(f"      ... and {len(details) - 4} more")

    if failures:
        print(f"\nfrontend failures ({len(failures)}):")
        for name, why in failures:
            print(f"  {name}: {why}")

    if problems:
        print(f"\ndivergences needing review ({len(problems)}):")
        for name, kind, detail in problems:
            print(f"  {name}  [{kind}]  {detail}")

    return 1 if problems or failures else 0


if __name__ == "__main__":
    sys.exit(main())
