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
        await RisingEdge(dut.CK)
        # Set random values for inputs
        dut.N1.value = random.randint(0, 1)
        dut.N5.value = random.randint(0, 1)
        dut.N9.value = random.randint(0, 1)
        dut.N13.value = random.randint(0, 1)
        dut.N17.value = random.randint(0, 1)
        dut.N21.value = random.randint(0, 1)
        dut.N25.value = random.randint(0, 1)
        dut.N29.value = random.randint(0, 1)
        dut.N33.value = random.randint(0, 1)
        dut.N37.value = random.randint(0, 1)
        dut.N41.value = random.randint(0, 1)
        dut.N45.value = random.randint(0, 1)
        dut.N49.value = random.randint(0, 1)
        dut.N53.value = random.randint(0, 1)
        dut.N57.value = random.randint(0, 1)
        dut.N61.value = random.randint(0, 1)
        dut.N65.value = random.randint(0, 1)
        dut.N69.value = random.randint(0, 1)    
        dut.N73.value = random.randint(0, 1)
        dut.N77.value = random.randint(0, 1)
        dut.N81.value = random.randint(0, 1)
        dut.N85.value = random.randint(0, 1)
        dut.N89.value = random.randint(0, 1)
        dut.N93.value = random.randint(0, 1)
        dut.N97.value = random.randint(0, 1)
        dut.N101.value = random.randint(0, 1)
        dut.N105.value = random.randint(0, 1)
        dut.N109.value = random.randint(0, 1)
        dut.N113.value = random.randint(0, 1)
        dut.N117.value = random.randint(0, 1)
        dut.N121.value = random.randint(0, 1)
        dut.N125.value = random.randint(0, 1)
        dut.N129.value = random.randint(0, 1)
        dut.N130.value = random.randint(0, 1)
        dut.N131.value = random.randint(0, 1)
        dut.N132.value = random.randint(0, 1)
        dut.N133.value = random.randint(0, 1)
        dut.N134.value = random.randint(0, 1)
        dut.N135.value = random.randint(0, 1)
        dut.N136.value = random.randint(0, 1)
        dut.N137.value = random.randint(0, 1)


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
        hdl_toplevel="c499",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="c499", test_module="sim")


if __name__ == "__main__":
    runner()