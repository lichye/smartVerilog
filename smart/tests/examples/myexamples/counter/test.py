import cocotb

from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import os
import random
from pathlib import Path

import cocotb
from cocotb.clock import Clock
from cocotb.runner import get_runner
from cocotb.triggers import FallingEdge



@cocotb.test()
async def my_first_test(dut):
    """Try accessing the design."""
    
    
    dut.clk.value = 1
    dut.reset.value = 1
    await Timer(1, units="ns")

    dut.clk.value = 0
    dut.reset.value = 0
    await Timer(1, units="ns")

    for cycle in range(10):
        dut.clk.value = 1
        await Timer(1, units="ns")


        dut.clk.value = 0
        await Timer(1, units="ns")


def runner():
    sim = os.getenv("SIM", "verilator")
    verilog_sources = ["counter.v"]

    # set parameters
    extra_args = []
    
    extra_args.append(f"--trace")
    
    print("Args is ")
    print(extra_args)
    print("")
    runner = get_runner(sim)
    runner.build(
        verilog_sources=verilog_sources,
        hdl_toplevel="counter",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="counter", test_module="test")


if __name__ == "__main__":
    runner()
