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

    # input N1,N5,N9,N13,N17,N21,N25,N29,N33,N37,
    #   N41,N45,N49,N53,N57,N61,N65,N69,N73,N77,
    #   N81,N85,N89,N93,N97,N101,N105,N109,N113,N117,
    #   N121,N125,N129,N130,N131,N132,N133,N134,N135,N136,
    #   N137;
    # 
    # Example: Initialize signals if necessary
    dut.N1.value = 0
    dut.N5.value = 0
    dut.N9.value = 0
    dut.N13.value = 0
    dut.N17.value = 0
    dut.N21.value = 0
    dut.N25.value = 0
    dut.N29.value = 0
    dut.N33.value = 0
    dut.N37.value = 0
    dut.N41.value = 0
    dut.N45.value = 0
    dut.N49.value = 0
    dut.N53.value = 0
    dut.N57.value = 0
    dut.N61.value = 0
    dut.N65.value = 0
    dut.N69.value = 0
    dut.N73.value = 0
    dut.N77.value = 0
    dut.N81.value = 0
    dut.N85.value = 0
    dut.N89.value = 0
    dut.N93.value = 0
    dut.N97.value = 0
    dut.N101.value = 0
    dut.N105.value = 0
    dut.N109.value = 0
    dut.N113.value = 0
    dut.N117.value = 0
    dut.N121.value = 0
    dut.N125.value = 0
    dut.N129.value = 0
    dut.N130.value = 0
    dut.N131.value = 0
    dut.N132.value = 0
    dut.N133.value = 0
    dut.N134.value = 0
    dut.N135.value = 0
    dut.N136.value = 0
    dut.N137.value = 0

    # Simulate 10 cycles (or steps) without clock
    for cycle in range(10):
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

        # Wait for 10ns to simulate processing time
        await Timer(10, units="ns")


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