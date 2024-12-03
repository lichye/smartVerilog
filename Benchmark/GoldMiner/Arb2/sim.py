import random
import os
import sys

import cocotb
from cocotb.clock import Clock
from cocotb.runner import get_runner
from cocotb.triggers import FallingEdge
from cocotb.triggers import RisingEdge, Timer
from cocotb.binary import BinaryValue
from pathlib import Path



@cocotb.test()
async def my_first_test(dut):
    random.seed(42)
    """Try accessing the design."""
    clock = Clock(dut.clk, 2, units="ns")  # Create a 10ns period clock on port clk
    cocotb.start_soon(clock.start())  # Start the clock

    
    await RisingEdge(dut.clk)
    dut.rst.value = 1

    for cycle in range(10):
        #clk rising edge
        await RisingEdge(dut.clk)
        dut.req1.value = random.randint(0,1)
        dut.req2.value = random.randint(0,1)
        dut.rst.value = 0



def runner(verilog_path):
    sim = os.getenv("SIM", "verilator")
    verilog_sources = verilog_path

    # set parameters
    extra_args = []
    
    extra_args.append(f"--trace")
    
    print("Args is ")
    print(extra_args)
    print("")
    runner = get_runner(sim)
    runner.build(
        verilog_sources=verilog_sources,
        hdl_toplevel="arb2",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="arb2", test_module="sim")


if __name__ == "__main__":
    if(len(sys.argv) !=2):
        print("Should give verilog design path")
        exit(1)
    else:
        verilog_sources = [sys.argv[1]]
        print(verilog_sources)
        runner(verilog_sources)