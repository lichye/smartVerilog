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
        # input N1,N2,N3,N6,N7;
        await RisingEdge(dut.CK)
        dut.N1.value = random.randint(0, 1)
        dut.N18.value = random.randint(0, 1)
        dut.N35.value = random.randint(0, 1)
        dut.N52.value = random.randint(0, 1)
        dut.N69.value = random.randint(0, 1)
        dut.N86.value = random.randint(0, 1)
        dut.N103.value = random.randint(0, 1)
        dut.N120.value = random.randint(0, 1)
        dut.N137.value = random.randint(0, 1)
        dut.N154.value = random.randint(0, 1)
        dut.N171.value = random.randint(0, 1)
        dut.N188.value = random.randint(0, 1)
        dut.N205.value = random.randint(0, 1)
        dut.N222.value = random.randint(0, 1)
        dut.N239.value = random.randint(0, 1)
        dut.N256.value = random.randint(0, 1)
        dut.N273.value = random.randint(0, 1)
        dut.N290.value = random.randint(0, 1) 
        dut.N307.value = random.randint(0, 1)
        dut.N324.value = random.randint(0, 1)
        dut.N341.value = random.randint(0, 1)
        dut.N358.value = random.randint(0, 1)
        dut.N375.value = random.randint(0, 1)
        dut.N392.value = random.randint(0, 1)
        dut.N409.value = random.randint(0, 1)
        dut.N426.value = random.randint(0, 1)
        dut.N443.value = random.randint(0, 1)
        dut.N460.value = random.randint(0, 1)
        dut.N477.value = random.randint(0, 1)
        dut.N494.value = random.randint(0, 1)
        dut.N511.value = random.randint(0, 1)
        dut.N528.value = random.randint(0, 1)



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
        hdl_toplevel="c6288",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="c6288", test_module="sim")


if __name__ == "__main__":
    runner()