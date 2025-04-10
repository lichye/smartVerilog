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

#     input GND,VDD,CK,john,cnt13,cnt21,cnt284,pcnt6,cnt261,cnt44,pcnt12,pcnt17,
#   cnt591,cnt45,cnt567,pcnt27,cnt283,cnt272,cnt10,cnt511,pcnt241,cnt509;

    random.seed(42)
    clock = Clock(dut.CK, 2, units="ns")  # Create a 10ns period clock on port clk
    cocotb.start_soon(clock.start())  # Start the clock
    await RisingEdge(dut.CK)


    for cycle in range(10000):
        await RisingEdge(dut.CK)
        # Assign random values to the inputs
        dut.GND.value = random.randint(0, 1)
        dut.VDD.value = random.randint(0, 1)
        dut.john.value = random.randint(0, 1)
        dut.cnt13.value = random.randint(0, 1)
        dut.cnt21.value = random.randint(0, 1)
        dut.cnt284.value = random.randint(0, 1)
        dut.pcnt6.value = random.randint(0, 1)
        dut.cnt261.value = random.randint(0, 1)
        dut.cnt44.value = random.randint(0, 1)
        dut.pcnt12.value = random.randint(0, 1)
        dut.pcnt17.value = random.randint(0, 1)
        dut.cnt591.value = random.randint(0, 1)
        dut.cnt45.value = random.randint(0, 1)
        dut.cnt567.value = random.randint(0, 1)
        dut.pcnt27.value = random.randint(0, 1)
        dut.cnt283.value = random.randint(0, 1)
        dut.cnt272.value = random.randint(0, 1)
        dut.cnt10.value = random.randint(0, 1)
        dut.cnt511.value = random.randint(0, 1)
        dut.pcnt241.value = random.randint(0, 1)
        dut.cnt509.value = random.randint(0, 1)



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
        hdl_toplevel="s510",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="s510", test_module="sim")


if __name__ == "__main__":
    runner()