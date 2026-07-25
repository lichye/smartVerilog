#!/usr/bin/env python3
"""WP2 acceptance harness: byte-parity between gen_bench.py (the frozen
behavioral spec) and the C++ frontend (frontend_dump).

Usage:
    python3 tools/parity_frontend.py [path/to/frontend_dump]

Builds frontend_dump if no path is given (requires g++). Compares, for every
design in Benchmark/ and artifact/CaseStudy/Input/:
  1. the canonical frontend JSON (ports/params/free regs/assume flag/guesses)
  2. the assume-stripped source, for files that contain assume statements
Exits 1 on the first mismatch, printing a unified diff.
"""

import difflib
import glob
import json
import os
import subprocess
import sys
import tempfile

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.insert(0, os.path.join(ROOT, "smart", "src", "python"))
import gen_bench  # noqa: E402


def python_json(path, top):
    with open(path) as f:
        text = f.read()
    info = gen_bench.parse_module(text, top)
    clock = gen_bench.guess_clock(info)
    reset = gen_bench.guess_reset(info, clock) if clock else None
    return {
        "top": top,
        "params": {k: v for k, v in sorted(info["params"].items())},
        "ports": [{"dir": p["dir"], "width": p["width"], "name": p["name"]}
                  for p in info["ports"]],
        "free_regs": [{"kind": r["kind"], "width": r["width"], "name": r["name"]}
                      for r in info["free_regs"]],
        "has_assume": info["has_assume"],
        "clock": clock,
        "reset": reset,
    }


def build_dump():
    exe = os.path.join(tempfile.mkdtemp(prefix="parity_frontend_"), "frontend_dump")
    src = os.path.join(ROOT, "smart", "src", "frontend")
    subprocess.run(["g++", "-std=c++17", "-O1", "-o", exe,
                    os.path.join(src, "SVModule.cpp"),
                    os.path.join(src, "frontend_dump.cpp")], check=True)
    return exe


def collect_designs():
    designs = []
    for pattern in ["Benchmark/**/sim.py", "artifact/CaseStudy/Input/*/sim.py"]:
        for sim in glob.glob(os.path.join(ROOT, pattern), recursive=True):
            d = os.path.dirname(sim)
            top = os.path.basename(d)
            rtl = os.path.join(d, top + ".sv")
            if os.path.exists(rtl):
                designs.append((rtl, top))
    return sorted(set(designs))


def fail(name, expected, actual):
    print(f"MISMATCH: {name}")
    diff = difflib.unified_diff(expected.splitlines(True), actual.splitlines(True),
                                "python", "cpp")
    sys.stdout.writelines(list(diff)[:80])
    sys.exit(1)


def main():
    exe = sys.argv[1] if len(sys.argv) > 1 else build_dump()
    designs = collect_designs()
    if len(designs) < 50:
        print(f"error: only found {len(designs)} designs — wrong checkout?")
        sys.exit(1)

    json_ok = strip_ok = 0
    for rtl, top in designs:
        expected = json.dumps(python_json(rtl, top), sort_keys=True)
        out = subprocess.run([exe, rtl, top], capture_output=True, text=True)
        if out.returncode != 0:
            fail(f"{top} (cpp exited {out.returncode})", expected, out.stderr)
        actual = json.dumps(json.loads(out.stdout), sort_keys=True)
        if expected != actual:
            fail(top, json.dumps(json.loads(expected), indent=2, sort_keys=True),
                 json.dumps(json.loads(actual), indent=2, sort_keys=True))
        json_ok += 1

        with open(rtl) as f:
            text = f.read()
        if "assume" in text:
            py_stripped = gen_bench.strip_assumes(text)
            out = subprocess.run([exe, "--strip", rtl], capture_output=True,
                                 text=True)
            if out.stdout != py_stripped:
                fail(f"{top} (strip_assumes)", py_stripped, out.stdout)
            strip_ok += 1

    print(f"parity OK: {json_ok} designs (JSON), {strip_ok} assume-strip checks")


if __name__ == "__main__":
    main()
