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

#     input GND,VDD,CK,g89,g94,g98,g102,g107,g301,g306,g310,g314,g319,g557,g558,g559,
#   g560,g561,
#   g562,g563,g564,g705,g639,g567,g45,g42,g39,g702,g32,g38,g46,g36,g47,g40,g37,
#   g41,g22,g44,g23;

    for cycle in range(10):
        # Assign random values to the inputs
        dut.GND.value = random.randint(0, 1)
        dut.VDD.value = random.randint(0, 1)    
        dut.CK.value = random.randint(0, 1)
        dut.g89.value = random.randint(0, 1)
        dut.g94.value = random.randint(0, 1)
        dut.g98.value = random.randint(0, 1)
        dut.g102.value = random.randint(0, 1)
        dut.g107.value = random.randint(0, 1)
        dut.g301.value = random.randint(0, 1)
        dut.g306.value = random.randint(0, 1)
        dut.g310.value = random.randint(0, 1)
        dut.g314.value = random.randint(0, 1)
        dut.g319.value = random.randint(0, 1)
        dut.g557.value = random.randint(0, 1)
        dut.g558.value = random.randint(0, 1)
        dut.g559.value = random.randint(0, 1)
        dut.g560.value = random.randint(0, 1)
        dut.g561.value = random.randint(0, 1)
        dut.g562.value = random.randint(0, 1)
        dut.g563.value = random.randint(0, 1)
        dut.g564.value = random.randint(0, 1)
        dut.g705.value = random.randint(0, 1)
        dut.g639.value = random.randint(0, 1)
        dut.g567.value = random.randint(0, 1)
        dut.g45.value = random.randint(0, 1)
        dut.g42.value = random.randint(0, 1)
        dut.g39.value = random.randint(0, 1)
        dut.g702.value = random.randint(0, 1)
        dut.g32.value = random.randint(0, 1)
        dut.g38.value = random.randint(0, 1)
        dut.g46.value = random.randint(0, 1)
        dut.g36.value = random.randint(0, 1)
        dut.g47.value = random.randint(0, 1)
        dut.g40.value = random.randint(0, 1)
        dut.g37.value = random.randint(0, 1)
        dut.g41.value = random.randint(0, 1)
        dut.g22.value = random.randint(0, 1)
        dut.g44.value = random.randint(0, 1)
        dut.g23.value = random.randint(0, 1)

        await Timer(10, units="ns")



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
        hdl_toplevel="s9234",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="s9234", test_module="sim")


if __name__ == "__main__":
    runner()