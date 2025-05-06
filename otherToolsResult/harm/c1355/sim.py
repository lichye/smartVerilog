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
        # Set random values for inputs
        await RisingEdge(dut.CK)
        dut.N1.value = random.randint(0, 1)
        dut.N8.value = random.randint(0, 1)
        dut.N15.value = random.randint(0, 1)
        dut.N22.value = random.randint(0, 1)
        dut.N29.value = random.randint(0, 1)
        dut.N36.value = random.randint(0, 1)
        dut.N43.value = random.randint(0, 1)
        dut.N50.value = random.randint(0, 1)
        dut.N57.value = random.randint(0, 1)
        dut.N64.value = random.randint(0, 1)
        dut.N71.value = random.randint(0, 1)
        dut.N78.value = random.randint(0, 1)
        dut.N85.value = random.randint(0, 1)
        dut.N92.value = random.randint(0, 1)
        dut.N99.value = random.randint(0, 1)
        dut.N106.value = random.randint(0, 1)
        dut.N113.value = random.randint(0, 1)
        dut.N120.value = random.randint(0, 1)
        dut.N127.value = random.randint(0, 1)
        dut.N134.value = random.randint(0, 1)
        dut.N141.value = random.randint(0, 1)
        dut.N148.value = random.randint(0, 1)
        dut.N155.value = random.randint(0, 1)
        dut.N162.value = random.randint(0, 1)
        dut.N169.value = random.randint(0, 1)
        dut.N176.value = random.randint(0, 1)
        dut.N183.value = random.randint(0, 1)
        dut.N190.value = random.randint(0, 1)
        dut.N197.value = random.randint(0, 1)
        dut.N204.value = random.randint(0, 1)
        dut.N211.value = random.randint(0, 1)
        dut.N218.value = random.randint(0, 1)
        dut.N225.value = random.randint(0, 1)
        dut.N226.value = random.randint(0, 1)
        dut.N227.value = random.randint(0, 1)
        dut.N228.value = random.randint(0, 1)
        dut.N229.value = random.randint(0, 1)
        dut.N230.value = random.randint(0, 1)
        dut.N231.value = random.randint(0, 1)
        dut.N232.value = random.randint(0, 1)
        dut.N233.value = random.randint(0, 1)

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

    #debug of Args
    print("Args is ")
    print(extra_args)
    print("")

    runner = get_runner(sim)
    runner.build(
        verilog_sources=sources,
        hdl_toplevel="c1355",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="c1355", test_module="sim")


if __name__ == "__main__":
    runner()