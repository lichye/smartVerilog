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
    
    # input GND,VDD,CK,G0,G1,G2,G3,G4,G5,G6,G7,G8,G9,G10,G11,G12,G13,G14,G15,G16,G18;



    for cycle in range(10):
        # Assign random values to the inputs
        dut.GND.value = random.randint(0, 1)
        dut.VDD.value = random.randint(0, 1)
        dut.CK.value = random.randint(0, 1)
        dut.G0.value = random.randint(0, 1)
        dut.G1.value = random.randint(0, 1)
        dut.G2.value = random.randint(0, 1)
        dut.G3.value = random.randint(0, 1)
        dut.G4.value = random.randint(0, 1)
        dut.G5.value = random.randint(0, 1)
        dut.G6.value = random.randint(0, 1)
        dut.G7.value = random.randint(0, 1)
        dut.G8.value = random.randint(0, 1)
        dut.G9.value = random.randint(0, 1)
        dut.G10.value = random.randint(0, 1)
        dut.G11.value = random.randint(0, 1)
        dut.G12.value = random.randint(0, 1)
        dut.G13.value = random.randint(0, 1)
        dut.G14.value = random.randint(0, 1)
        dut.G15.value = random.randint(0, 1)
        dut.G16.value = random.randint(0, 1)
        dut.G18.value = random.randint(0, 1)


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
        hdl_toplevel="s832",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="s832", test_module="sim")


if __name__ == "__main__":
    runner()