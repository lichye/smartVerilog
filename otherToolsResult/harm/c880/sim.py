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
        dut.N8.value = random.randint(0, 1)
        dut.N13.value = random.randint(0, 1)
        dut.N17.value = random.randint(0, 1)
        dut.N26.value = random.randint(0, 1)
        dut.N29.value = random.randint(0, 1)
        dut.N36.value = random.randint(0, 1)
        dut.N42.value = random.randint(0, 1)
        dut.N51.value = random.randint(0, 1)
        dut.N55.value = random.randint(0, 1)
        dut.N59.value = random.randint(0, 1)
        dut.N68.value = random.randint(0, 1)
        dut.N72.value = random.randint(0, 1)
        dut.N73.value = random.randint(0, 1)
        dut.N74.value = random.randint(0, 1)
        dut.N75.value = random.randint(0, 1)
        dut.N80.value = random.randint(0, 1)
        dut.N85.value = random.randint(0, 1)
        dut.N86.value = random.randint(0, 1)
        dut.N87.value = random.randint(0, 1)
        dut.N88.value = random.randint(0, 1)
        dut.N89.value = random.randint(0, 1)
        dut.N90.value = random.randint(0, 1)
        dut.N91.value = random.randint(0, 1)
        dut.N96.value = random.randint(0, 1)
        dut.N101.value = random.randint(0, 1)
        dut.N106.value = random.randint(0, 1)
        dut.N111.value = random.randint(0, 1)
        dut.N116.value = random.randint(0, 1)
        dut.N121.value = random.randint(0, 1)
        dut.N126.value = random.randint(0, 1)
        dut.N130.value = random.randint(0, 1)
        dut.N135.value = random.randint(0, 1)
        dut.N138.value = random.randint(0, 1)
        dut.N143.value = random.randint(0, 1)
        dut.N146.value = random.randint(0, 1)
        dut.N149.value = random.randint(0, 1)
        dut.N152.value = random.randint(0, 1)
        dut.N153.value = random.randint(0, 1)
        dut.N156.value = random.randint(0, 1)
        dut.N159.value = random.randint(0, 1)
        dut.N165.value = random.randint(0, 1)
        dut.N171.value = random.randint(0, 1)
        dut.N177.value = random.randint(0, 1)
        dut.N183.value = random.randint(0, 1)
        dut.N189.value = random.randint(0, 1)
        dut.N195.value = random.randint(0, 1)
        dut.N201.value = random.randint(0, 1)
        dut.N207.value = random.randint(0, 1)
        dut.N210.value = random.randint(0, 1)
        dut.N219.value = random.randint(0, 1)
        dut.N228.value = random.randint(0, 1)
        dut.N237.value = random.randint(0, 1)
        dut.N246.value = random.randint(0, 1)
        dut.N255.value = random.randint(0, 1)
        dut.N259.value = random.randint(0, 1)
        dut.N260.value = random.randint(0, 1)
        dut.N261.value = random.randint(0, 1)
        dut.N267.value = random.randint(0, 1)
        dut.N268.value = random.randint(0, 1)


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
        hdl_toplevel="c880",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="c880", test_module="sim")


if __name__ == "__main__":
    runner()