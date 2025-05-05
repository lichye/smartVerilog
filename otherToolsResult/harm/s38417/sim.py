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

    clock = Clock(dut.CK, 2, units="ns")  # Create a 10ns period clock on port clk
    cocotb.start_soon(clock.start())  # Start the clock
    await RisingEdge(dut.CK)

    # Simulate 10 cycles (or steps) without clock
    for cycle in range(10000):
        # Wait for a rising edge of the clock
        await RisingEdge(dut.CK)
        dut.GND.value = random.randint(0, 1)
        dut.VDD.value = random.randint(0, 1)
        dut.CK.value = random.randint(0, 1)
        dut.g51.value = random.randint(0, 1)
        dut.g563.value = random.randint(0, 1)
        dut.g1249.value = random.randint(0, 1)
        dut.g1943.value = random.randint(0, 1)
        dut.g2637.value = random.randint(0, 1)
        dut.g3212.value = random.randint(0, 1)
        dut.g3213.value = random.randint(0, 1)
        dut.g3214.value = random.randint(0, 1)
        dut.g3215.value = random.randint(0, 1)
        dut.g3216.value = random.randint(0, 1)
        dut.g3217.value = random.randint(0, 1)
        dut.g3218.value = random.randint(0, 1)
        dut.g3219.value = random.randint(0, 1)
        dut.g3220.value = random.randint(0, 1)
        dut.g3221.value = random.randint(0, 1)
        dut.g3222.value = random.randint(0, 1)
        dut.g3223.value = random.randint(0, 1)
        dut.g3224.value = random.randint(0, 1)
        dut.g3225.value = random.randint(0, 1)
        dut.g3226.value = random.randint(0, 1)
        dut.g3227.value = random.randint(0, 1)
        dut.g3228.value = random.randint(0, 1)
        dut.g3229.value = random.randint(0, 1)
        dut.g3230.value = random.randint(0, 1)
        dut.g3231.value = random.randint(0, 1)
        dut.g3232.value = random.randint(0, 1)
        dut.g3233.value = random.randint(0, 1)
        dut.g3234.value = random.randint(0, 1)



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
        hdl_toplevel="s38417",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="s38417", test_module="sim")


if __name__ == "__main__":
    runner()