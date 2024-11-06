import random

import os
import random
from pathlib import Path

import cocotb
from cocotb.clock import Clock
from cocotb.runner import get_runner
from cocotb.triggers import FallingEdge
from cocotb.triggers import RisingEdge, Timer
from cocotb.binary import BinaryValue


@cocotb.test()
async def my_first_test(dut):
    """Try accessing the design."""
    clock = Clock(dut.clk, 2, units="ns")  # Create a 10ns period clock on port clk
    cocotb.start_soon(clock.start())  # Start the clock

    for cycle in range(10):
        #clk rising edge
        await RisingEdge(dut.clk)

        dut.clk.value = 1
        dut.dataa.value = random.randint(0,100)
        dut.datab.value = random.randint(0,100)
        dut.add_sub.value = 1



def runner():
    sim = os.getenv("SIM", "verilator")
    verilog_sources = ["addsub.sv"]

    # set parameters
    extra_args = []
    
    extra_args.append(f"--trace")
    
    print("Args is ")
    print(extra_args)
    print("")
    runner = get_runner(sim)
    runner.build(
        verilog_sources=verilog_sources,
        hdl_toplevel="addsub",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="addsub", test_module="test")


if __name__ == "__main__":
    runner()
