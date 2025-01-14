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

    # input GND,VDD,CK,START,B0,B1,B2,B3,A0,A1,A2,A3;
    # Simulate 10 cycles (or steps) without clock
    dut.GND.value = 0
    dut.VDD.value = 0
    dut.CK.value = 0
    dut.START.value = 0
    dut.B0.value = 0
    dut.B1.value = 0
    dut.B2.value = 0
    dut.B3.value = 0
    dut.A0.value = 0
    dut.A1.value = 0
    dut.A2.value = 0
    dut.A3.value = 0

    for cycle in range(10):
        dut.GND.value = random.randint(0, 1)
        dut.VDD.value = random.randint(0, 1)
        dut.CK.value = random.randint(0, 1)
        dut.START.value = random.randint(0, 1)
        dut.B0.value = random.randint(0, 1)
        dut.B1.value = random.randint(0, 1)
        dut.B2.value = random.randint(0, 1)
        dut.B3.value = random.randint(0, 1)
        dut.A0.value = random.randint(0, 1)
        dut.A1.value = random.randint(0, 1)
        dut.A2.value = random.randint(0, 1)
        dut.A3.value = random.randint(0, 1)
        
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
        hdl_toplevel="s349",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="s349", test_module="sim")


if __name__ == "__main__":
    runner()