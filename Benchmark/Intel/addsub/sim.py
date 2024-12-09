import random
import os
import sys
import glob

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

    for cycle in range(10):
        #clk rising edge
        await RisingEdge(dut.clk)

        dut.clk.value = 1
        dut.dataa.value = random.randint(0,100)
        dut.datab.value = random.randint(0,100)
        dut.add_sub.value = 1



def runner():
    sim = os.getenv("SIM", "verilator")

    dir_path = os.path.dirname(os.path.abspath(__file__))
    sources = glob.glob(os.path.join(dir_path, "*.sv"))

    # set parameters
    extra_args = []        
    extra_args.append(f"--trace")
    
   
    print("Args is ")
    print(extra_args)
    print("")
    runner = get_runner(sim)
    runner.build(
        verilog_sources=sources,
        hdl_toplevel="addsub",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="addsub", test_module="sim")


if __name__ == "__main__":
    runner()