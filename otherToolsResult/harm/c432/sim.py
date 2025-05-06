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
    
    # Set up the clock
    clock = Clock(dut.CK, 10, units="ns")
    cocotb.start_soon(clock.start())
    await RisingEdge(dut.CK)

    # Simulate 10 cycles (or steps) without clock
    for cycle in range(10000):
        await RisingEdge(dut.CK)

        # Set random values for inputs
        dut.N1.value = random.randint(0, 1)
        dut.N4.value = random.randint(0, 1)
        dut.N8.value = random.randint(0, 1)
        dut.N11.value = random.randint(0, 1)
        dut.N14.value = random.randint(0, 1)
        dut.N17.value = random.randint(0, 1)
        dut.N21.value = random.randint(0, 1)
        dut.N24.value = random.randint(0, 1)
        dut.N27.value = random.randint(0, 1)
        dut.N30.value = random.randint(0, 1)
        dut.N34.value = random.randint(0, 1)
        dut.N37.value = random.randint(0, 1)
        dut.N40.value = random.randint(0, 1)
        dut.N43.value = random.randint(0, 1)
        dut.N47.value = random.randint(0, 1)
        dut.N50.value = random.randint(0, 1)
        dut.N53.value = random.randint(0, 1)
        dut.N56.value = random.randint(0, 1)
        dut.N60.value = random.randint(0, 1)
        dut.N63.value = random.randint(0, 1)
        dut.N66.value = random.randint(0, 1)
        dut.N69.value = random.randint(0, 1)
        dut.N73.value = random.randint(0, 1)
        dut.N76.value = random.randint(0, 1)
        dut.N79.value = random.randint(0, 1)
        dut.N82.value = random.randint(0, 1)
        dut.N86.value = random.randint(0, 1)
        dut.N89.value = random.randint(0, 1)
        dut.N92.value = random.randint(0, 1)
        dut.N95.value = random.randint(0, 1)
        dut.N99.value = random.randint(0, 1)
        dut.N102.value = random.randint(0, 1)
        dut.N105.value = random.randint(0, 1)
        dut.N108.value = random.randint(0, 1)
        dut.N112.value = random.randint(0, 1)
        dut.N115.value = random.randint(0, 1)


def runner():
    sim = os.getenv("SIM", "verilator")

    dir_path = os.path.dirname(os.path.abspath(__file__))
    sources = glob.glob(os.path.join(dir_path, "*.sv"))
    working_dir = os.getcwd()
    
    if not os.path.exists(working_dir+"/sim_build"):
        subprocess.run(["mkdir", "sim_build"])
  
    # set parameters
    extra_args = []
    extra_args.append(f"--trace")
    # extra_args.append("--lint-only")
    extra_args.append("-Wno-WIDTHEXPAND")
    extra_args.append("-Wno-WIDTHTRUNC")

    #debug of Args
    print("Args is ")
    print(extra_args)
    print("")

    runner = get_runner(sim)
    runner.build(
        verilog_sources=sources,
        hdl_toplevel="c432",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="c432", test_module="sim")


if __name__ == "__main__":
    runner()