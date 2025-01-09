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
    # input GND,VDD,CK,G0,G1,G2,G3;

    # Example: Initialize signals if necessary
    # dut.instr_new_i.value = 0
    # dut.instr_rdata_i.value = 0
    # dut.illegal_c_insn_i.value = 0

    dut.GND = 0
    dut.VDD = 0
    dut.CK = 0
    dut.G0 = 0
    dut.G1 = 0
    dut.G2 = 0
    dut.G3 = 0

    # Simulate 10 cycles (or steps) without clock
    for cycle in range(10):
        # Set random values for inputs
        dut.GND = random.randint(0, 1)
        dut.VDD = random.randint(0, 1)
        dut.CK = random.randint(0, 1)
        dut.G0 = random.randint(0, 1)
        dut.G1 = random.randint(0, 1)
        dut.G2 = random.randint(0, 1)
        dut.G3 = random.randint(0, 1)

        # Wait for 10ns to simulate processing time
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
        hdl_toplevel="s27",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="s27", test_module="sim")


if __name__ == "__main__":
    runner()