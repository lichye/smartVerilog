"""Sanity tests for gen_bench.py against real benchmarks in the repo.
Run with:  python smart/src/python/test_gen_bench.py  (from the repo root)
"""

import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_bench

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))


def read(path):
    with open(os.path.join(ROOT, path)) as f:
        return f.read()


def test_ansi_no_clock():
    info = gen_bench.parse_module(read("Benchmark/user/tiny_and/tiny_and.sv"), "tiny_and")
    names = {p["name"]: p for p in info["inputs"]}
    assert set(names) == {"a", "b"}, names
    assert gen_bench.guess_clock(info) is None
    sim = gen_bench.render_sim_py(info)
    assert "Timer" in sim and "RisingEdge(dut." not in sim
    assert '"tiny_and"' in sim
    compile(sim, "sim.py", "exec")  # generated file must be valid python
    print("ok  ansi_no_clock (tiny_and)")


def test_non_ansi():
    info = gen_bench.parse_module(read("Benchmark/fmcad2025/s27/s27.sv"), "s27")
    names = [p["name"] for p in info["inputs"]]
    assert "CK" in names and "G0" in names, names
    sim = gen_bench.render_sim_py(info)
    compile(sim, "sim.py", "exec")
    print(f"ok  non_ansi (s27): inputs={names}")


def test_params_clock_reset():
    info = gen_bench.parse_module(read("Benchmark/HWSpec/axis_fifo/axis_fifo.sv"), "axis_fifo")
    ports = {p["name"]: p for p in info["inputs"]}
    assert "clk" in ports and "rst" in ports, list(ports)
    clock = gen_bench.guess_clock(info)
    reset = gen_bench.guess_reset(info, clock)
    assert clock == "clk" and reset["signal"] == "rst" and reset["active"] == 1
    tdata = ports.get("s_axis_tdata")
    assert tdata and tdata["width"] == 8, tdata  # parameter DATA_WIDTH = 8
    sim = gen_bench.render_sim_py(info)
    assert "Clock(dut.clk" in sim and "dut.rst.value = 1" in sim
    assert '("s_axis_tdata", {"max": 255})' in sim, "width-aware max"
    assert re.search(r'\("clk"', sim) is None, "clock must not be randomly driven"
    compile(sim, "sim.py", "exec")
    print("ok  params_clock_reset (axis_fifo)")


def test_free_regs_and_assumes():
    text = read("artifact/CaseStudy/Input/nru_a/nru_a.sv")
    info = gen_bench.parse_module(text, "nru_a")
    kinds = {(r["kind"], r["name"]) for r in info["free_regs"]}
    assert ("anyconst", "attacker_hitmap") in kinds, kinds
    assert ("anyseq", "hitmap1") in kinds, kinds
    assert info["has_assume"]

    stripped = gen_bench.strip_assumes(text)
    assert not re.search(r"\bassume\s*\(", stripped)
    assert stripped.count("endmodule") == text.count("endmodule")
    # balanced structure must survive: same number of begin/end pairs
    assert len(re.findall(r"\bbegin\b", stripped)) == len(re.findall(r"\bbegin\b", text))

    sim = gen_bench.render_sim_py(info)
    assert "--public-flat-rw" in sim
    assert '"attacker_hitmap"' in sim and "ANYCONST" in sim
    compile(sim, "sim.py", "exec")
    print("ok  free_regs_and_assumes (nru_a)")


def test_inject_assumes():
    text = read("Benchmark/user/tiny_and/tiny_and.sv")
    out = gen_bench.inject_assumes(text, "tiny_and", ["!(a && b)"])
    assert "assume(!(a && b));" in out
    assert out.index("assume") < out.index("endmodule")
    # stripping the injected assume must round-trip
    assert "assume" not in gen_bench.strip_assumes(out).replace("assume", "assume")\
        or not re.search(r"\bassume\s*\(", gen_bench.strip_assumes(out))
    print("ok  inject_assumes (tiny_and)")


def test_if_assume_stays_legal():
    src = "module m(input a); always @(*) begin if (a) assume(a); else assume(!a); end endmodule"
    stripped = gen_bench.strip_assumes(src)
    assert re.search(r"if \(a\) ;\s*else ;", stripped), stripped
    print("ok  if_assume_stays_legal")


if __name__ == "__main__":
    test_ansi_no_clock()
    test_non_ansi()
    test_params_clock_reset()
    test_free_regs_and_assumes()
    test_inject_assumes()
    test_if_assume_stays_legal()
    print("all gen_bench tests passed")
