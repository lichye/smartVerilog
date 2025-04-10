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



    for cycle in range(10000):
        # Wait for a rising edge of the clock
        await RisingEdge(dut.CK)
        # Assign random values to the inputs
        dut.GND.value = random.randint(0, 1)
        dut.VDD.value = random.randint(0, 1)
        dut.CK.value = random.randint(0, 1)
        dut.P_0.value = random.randint(0, 1)
        dut.C_16.value = random.randint(0, 1)
        dut.C_15.value = random.randint(0, 1)
        dut.C_14.value = random.randint(0, 1)
        dut.C_13.value = random.randint(0, 1)
        dut.C_12.value = random.randint(0, 1)
        dut.C_11.value = random.randint(0, 1)
        dut.C_10.value = random.randint(0, 1)
        dut.C_9.value = random.randint(0, 1)
        dut.C_8.value = random.randint(0, 1)
        dut.C_7.value = random.randint(0, 1)
        dut.C_6.value = random.randint(0, 1)
        dut.C_5.value = random.randint(0, 1)
        dut.C_4.value = random.randint(0, 1)
        dut.C_3.value = random.randint(0, 1)
        dut.C_2.value = random.randint(0, 1)
        dut.C_1.value = random.randint(0, 1)
        dut.C_0.value = random.randint(0, 1)


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
    extra_args.append("-Wno-UNOPTFLAT")

    #debug of Args
    print("Args is ")
    print(extra_args)
    print("")

    runner = get_runner(sim)
    runner.build(
        verilog_sources=sources,
        hdl_toplevel="s420",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="s420", test_module="sim")


if __name__ == "__main__":
    runner()