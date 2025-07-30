import random
import os
import sys
import glob
import shutil
import subprocess

import cocotb
from cocotb.clock import Clock
from cocotb.runner import get_runner
from cocotb.triggers import FallingEdge
from cocotb.triggers import RisingEdge, Timer
from cocotb.binary import BinaryValue
from pathlib import Path

# parameter DATA_WIDTH = 8,
# input  wire                   clk,
# input  wire                   rst,
# input  wire        m_axis_cmd_ready,
# input  wire        m_axis_data_tready,
# input  wire        start

@cocotb.test()
async def my_first_test(dut):
    """Test a design without clock signals."""
    random.seed(42)

    clock = Clock(dut.clk, 2, units="ns")  # Create a 10ns period clock on port clk
    cocotb.start_soon(clock.start())  # Start the clock

    await RisingEdge(dut.clk)
    dut.rst = 1
    for cycle in range(10):
        await RisingEdge(dut.clk)
        dut.rst = 0
        dut.m_axis_cmd_ready = random.randint(0, 1)
        dut.m_axis_data_tready = random.randint(0, 1)
        dut.start = random.randint(0, 1)
        





    
def runner():
    sim = os.getenv("SIM", "verilator")

    dir_path = os.path.dirname(os.path.abspath(__file__))
    sources = glob.glob(os.path.join(dir_path, "*.sv"))
    working_dir = os.getcwd()
    
    # set parameters
    extra_args = []
    extra_args.append(f"--trace")
    # extra_args.append("--lint-only")
    extra_args.append("-Wno-WIDTHEXPAND")
    extra_args.append("-Wno-WIDTHTRUNC")
    extra_args.append("--Wno-UNOPTFLAT")
    extra_args.append("--Wno-CASEOVERLAP")
    extra_args.append("--Wno-CASEINCOMPLETE")
    extra_args.append("--Wno-SELRANGE")

    #debug of Args
    print("Args is ")
    print(extra_args)
    print("")

    runner = get_runner(sim)
    runner.build(
        verilog_sources=sources,
        hdl_toplevel="i2c_init",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="i2c_init", test_module="sim")


if __name__ == "__main__":
    runner()