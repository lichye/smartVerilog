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
        dut.N13.value = random.randint(0, 1)
        dut.N20.value = random.randint(0, 1)
        dut.N33.value = random.randint(0, 1)
        dut.N41.value = random.randint(0, 1)
        dut.N45.value = random.randint(0, 1)
        dut.N50.value = random.randint(0, 1)
        dut.N58.value = random.randint(0, 1)
        dut.N68.value = random.randint(0, 1)
        dut.N77.value = random.randint(0, 1)
        dut.N87.value = random.randint(0, 1)
        dut.N97.value = random.randint(0, 1)
        dut.N107.value = random.randint(0, 1)
        dut.N116.value = random.randint(0, 1)
        dut.N124.value = random.randint(0, 1)
        dut.N125.value = random.randint(0, 1)
        dut.N128.value = random.randint(0, 1)
        dut.N132.value = random.randint(0, 1)
        dut.N137.value = random.randint(0, 1)
        dut.N143.value = random.randint(0, 1)
        dut.N150.value = random.randint(0, 1)
        dut.N159.value = random.randint(0, 1)
        dut.N169.value = random.randint(0, 1)
        dut.N179.value = random.randint(0, 1)
        dut.N190.value = random.randint(0, 1)
        dut.N200.value = random.randint(0, 1)
        dut.N213.value = random.randint(0, 1)
        dut.N222.value = random.randint(0, 1)
        dut.N223.value = random.randint(0, 1)
        dut.N226.value = random.randint(0, 1)
        dut.N232.value = random.randint(0, 1)
        dut.N238.value = random.randint(0, 1)
        dut.N244.value = random.randint(0, 1)
        dut.N250.value = random.randint(0, 1)
        dut.N257.value = random.randint(0, 1)
        dut.N264.value = random.randint(0, 1)
        dut.N270.value = random.randint(0, 1)
        dut.N274.value = random.randint(0, 1)
        dut.N283.value = random.randint(0, 1)
        dut.N294.value = random.randint(0, 1)
        dut.N303.value = random.randint(0, 1)
        dut.N311.value = random.randint(0, 1)
        dut.N317.value = random.randint(0, 1)
        dut.N322.value = random.randint(0, 1)
        dut.N326.value = random.randint(0, 1)
        dut.N329.value = random.randint(0, 1)
        dut.N330.value = random.randint(0, 1)
        dut.N343.value = random.randint(0, 1)
        dut.N349.value = random.randint(0, 1)
        dut.N350.value = random.randint(0, 1)



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
        hdl_toplevel="c3540",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="c3540", test_module="sim")


if __name__ == "__main__":
    runner()