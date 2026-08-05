import glob
import os

import cocotb
from cocotb.runner import get_runner
from cocotb.triggers import Timer


@cocotb.test()
async def truth_table(dut):
    for a in range(2):
        for b in range(2):
            dut.a.value = a
            dut.b.value = b
            await Timer(1, units="ns")
            assert int(dut.y.value) == (a & b)


def runner():
    sim = os.getenv("SIM", "verilator")
    dir_path = os.path.dirname(os.path.abspath(__file__))
    sources = glob.glob(os.path.join(dir_path, "*.sv"))

    runner = get_runner(sim)
    runner.build(
        verilog_sources=sources,
        hdl_toplevel="tiny_and",
        build_args=["--trace"],
    )
    runner.test(hdl_toplevel="tiny_and", test_module="sim")


if __name__ == "__main__":
    runner()
