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

# input GND,VDD,CK,g35,g36,g6744,g6745,g6746,g6747,g6748,g6749,g6750,g6751,g6752,
#   g6753,g84,
#   g120,g5,g113,g126,g99,g53,g116,g92,g56,g91,g44,g57,g100,g54,g124,g125,g114,
#   g134,g72,g115,g135,g90,g127,g64,g73;
    # Simulate 10 cycles (or steps) without clock
    for cycle in range(5000):
        await RisingEdge(dut.CK)
        dut.GND.value = random.randint(0, 1)
        dut.VDD.value = random.randint(0, 1)
        dut.CK.value = random.randint(0, 1)
        dut.g35.value = random.randint(0, 1)
        dut.g36.value = random.randint(0, 1)
        dut.g6744.value = random.randint(0, 1)
        dut.g6745.value = random.randint(0, 1)
        dut.g6746.value = random.randint(0, 1)
        dut.g6747.value = random.randint(0, 1)
        dut.g6748.value = random.randint(0, 1)
        dut.g6749.value = random.randint(0, 1)
        dut.g6750.value = random.randint(0, 1)
        dut.g6751.value = random.randint(0, 1)
        dut.g6752.value = random.randint(0, 1)
        dut.g6753.value = random.randint(0, 1)
        dut.g84.value = random.randint(0, 1)
        dut.g120.value = random.randint(0, 1)
        dut.g5.value = random.randint(0, 1)
        dut.g113.value = random.randint(0, 1)
        dut.g126.value = random.randint(0, 1)
        dut.g99.value = random.randint(0, 1)
        dut.g53.value = random.randint(0, 1)
        dut.g116.value = random.randint(0, 1)
        dut.g92.value = random.randint(0, 1)
        dut.g56.value = random.randint(0, 1)
        dut.g91.value = random.randint(0, 1)
        dut.g44.value = random.randint(0, 1)
        dut.g57.value = random.randint(0, 1)
        dut.g100.value = random.randint(0, 1)
        dut.g54.value = random.randint(0, 1)
        dut.g124.value = random.randint(0, 1)
        dut.g125.value = random.randint(0, 1)
        dut.g114.value = random.randint(0, 1)
        dut.g134.value = random.randint(0, 1)
        dut.g72.value = random.randint(0, 1)
        dut.g115.value = random.randint(0, 1)
        dut.g135.value = random.randint(0, 1)
        dut.g90.value = random.randint(0, 1)
        dut.g127.value = random.randint(0, 1)
        dut.g64.value = random.randint(0, 1)
        dut.g73.value = random.randint(0, 1)



def runner():
    sim = os.getenv("SIM", "verilator")

    dir_path = os.path.dirname(os.path.abspath(__file__))
    sources = glob.glob(os.path.join(dir_path, "*.sv"))
    working_dir = os.getcwd()
    
    # move ibex_pkg.sv to sim_build
    if not os.path.exists(working_dir+"/sim_build"):
        subprocess.run(["mkdir", "sim_build"])
    if not os.path.exists(working_dir+"/sim_build/ibex_pkg.sv"):
        subprocess.run(["cp",dir_path+"/ibex_pkg.sv",working_dir+"/sim_build/ibex_pkg.sv"])

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
        hdl_toplevel="s38584",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="s38584", test_module="sim")


if __name__ == "__main__":
    runner()