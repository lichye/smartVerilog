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

@cocotb.test()
async def my_first_test(dut):
    """Test a design without clock signals."""
    random.seed(42)

    clock = Clock(dut.clk, 2, units="ns")  # Create a 10ns period clock on port clk
    cocotb.start_soon(clock.start())  # Start the clock

    await RisingEdge(dut.clk)

    dut.reset = 1
    await RisingEdge(dut.clk)

    for cycle in range(10):
        await RisingEdge(dut.clk)
        # dut.reset = 0
        # dut.os_req = random.randint(0, 1)
        # dut.hitmap = random.randint(0, 255)
        # dut.user_req = random.randint(0, 1)
        # dut.addr = random.randint(0, 2**32 - 1)


    
def runner():
    sim = os.getenv("SIM", "verilator")

    dir_path = os.path.dirname(os.path.abspath(__file__))
    sources = glob.glob(os.path.join(dir_path, "*.sv"))
    working_dir = os.getcwd()
    
    # move ibex_pkg.sv to sim_build
    # if not os.path.exists(working_dir+"/sim_build"):
    #     subprocess.run(["mkdir", "sim_build"])
    if not os.path.exists(working_dir+"/sim_build"):
        os.makedirs(working_dir+"/sim_build")
    if not os.path.exists(working_dir+"/sim_build/cacheline_consts.vh"):
        subprocess.run(["cp",dir_path+"/cacheline_consts.vh",working_dir+"/sim_build/cacheline_consts.vh"])
    # set parameters
    extra_args = []
    extra_args.append(f"--trace")
    # extra_args.append("--lint-only")
    extra_args.append("-Wno-WIDTHEXPAND")
    extra_args.append("-Wno-WIDTHTRUNC")
    extra_args.append("--Wno-UNOPTFLAT")
    extra_args.append("--Wno-CASEOVERLAP")

    #debug of Args
    print("Args is ")
    print(extra_args)
    print("")

    runner = get_runner(sim)
    runner.build(
        verilog_sources=sources,
        hdl_toplevel="formal",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="formal", test_module="sim")


if __name__ == "__main__":
    runner()