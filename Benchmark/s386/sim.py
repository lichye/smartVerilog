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

    # input GND,VDD,CK,v6,v5,v4,v3,v2,v1,v0;
    dut.GND.value = 0
    dut.VDD.value = 0
    dut.CK.value = 0
    dut.v6.value = 0
    dut.v5.value = 0
    dut.v4.value = 0
    dut.v3.value = 0
    dut.v2.value = 0
    dut.v1.value = 0
    dut.v0.value = 0


    for cycle in range(10):
        # Assign random values to the inputs
        dut.GND.value = random.randint(0, 1)
        dut.VDD.value = random.randint(0, 1)
        dut.CK.value = random.randint(0, 1)
        dut.v6.value = random.randint(0, 1)
        dut.v5.value = random.randint(0, 1)
        dut.v4.value = random.randint(0, 1)
        dut.v3.value = random.randint(0, 1)
        dut.v2.value = random.randint(0, 1)
        dut.v1.value = random.randint(0, 1)
        dut.v0.value = random.randint(0, 1)
        

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
        hdl_toplevel="s386",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="s386", test_module="sim")


if __name__ == "__main__":
    runner()