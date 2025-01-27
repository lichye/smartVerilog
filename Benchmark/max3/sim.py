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

    for cycle in range(10):
        dut.a = random.randint(0, 255)
        dut.b = random.randint(0, 255)
        dut.c = random.randint(0, 255)
        
        # Wait for 10ns to simulate processing time
        await Timer(10, units="ns")



def runner():
    sim = os.getenv("SIM", "verilator")

    dir_path = os.path.dirname(os.path.abspath(__file__))
    sources = glob.glob(os.path.join(dir_path, "*.sv"))
    working_dir = os.getcwd()
    
    # move ibex_pkg.sv to sim_build
    if not os.path.exists(working_dir+"/sim_build"):
        subprocess.run(["mkdir", "sim_build"])
    if not os.path.exists(working_dir+"/sim_build/ibex_pkg.sv"):
        subprocess.run(["cp",dir_path+"/ibex_pkg.sv",working_dir+"/sim_build/ibex_pkg.sv"])

    # set parameters
    extra_args = []
    extra_args.append(f"--trace")
    # extra_args.append("--lint-only")
    extra_args.append("-Wno-WIDTHEXPAND")
    extra_args.append("-Wno-WIDTHTRUNC")
    extra_args.append("-Wno-UNOPTFLAT")

    #debug of Args
    print("Args is ")
    print(extra_args)
    print("")

    runner = get_runner(sim)
    runner.build(
        verilog_sources=sources,
        hdl_toplevel="max3",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="max3", test_module="sim")


if __name__ == "__main__":
    runner()