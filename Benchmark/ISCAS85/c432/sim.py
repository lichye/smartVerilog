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
    
    # input N1,N4,N8,N11,N14,N17,N21,N24,N27,N30,
    #   N34,N37,N40,N43,N47,N50,N53,N56,N60,N63,
    #   N66,N69,N73,N76,N79,N82,N86,N89,N92,N95,
    #   N99,N102,N105,N108,N112,N115;

    dut.N1.value = random.randint(0, 1)
    dut.N4.value = random.randint(0, 1)
    dut.N8.value = random.randint(0, 1)
    dut.N11.value = random.randint(0, 1)
    dut.N14.value = random.randint(0, 1)
    dut.N17.value = random.randint(0, 1)
    dut.N21.value = random.randint(0, 1)
    dut.N24.value = random.randint(0, 1)
    dut.N27.value = random.randint(0, 1)
    dut.N30.value = random.randint(0, 1)
    dut.N34.value = random.randint(0, 1)
    dut.N37.value = random.randint(0, 1)
    dut.N40.value = random.randint(0, 1)
    dut.N43.value = random.randint(0, 1)
    dut.N47.value = random.randint(0, 1)
    dut.N50.value = random.randint(0, 1)
    dut.N53.value = random.randint(0, 1)
    dut.N56.value = random.randint(0, 1)
    dut.N60.value = random.randint(0, 1)
    dut.N63.value = random.randint(0, 1)
    dut.N66.value = random.randint(0, 1)
    dut.N69.value = random.randint(0, 1)
    dut.N73.value = random.randint(0, 1)
    dut.N76.value = random.randint(0, 1)
    dut.N79.value = random.randint(0, 1)
    dut.N82.value = random.randint(0, 1)
    dut.N86.value = random.randint(0, 1)
    dut.N89.value = random.randint(0, 1)
    dut.N92.value = random.randint(0, 1)
    dut.N95.value = random.randint(0, 1)
    dut.N99.value = random.randint(0, 1)
    dut.N102.value = random.randint(0, 1)
    dut.N105.value = random.randint(0, 1)
    dut.N108.value = random.randint(0, 1)
    dut.N112.value = random.randint(0, 1)
    dut.N115.value = random.randint(0, 1)

    # Simulate 10 cycles (or steps) without clock
    for cycle in range(10):
        # Set random values for inputs
        dut.N1.value = random.randint(0, 1)
        dut.N4.value = random.randint(0, 1)
        dut.N8.value = random.randint(0, 1)
        dut.N11.value = random.randint(0, 1)
        dut.N14.value = random.randint(0, 1)
        dut.N17.value = random.randint(0, 1)
        dut.N21.value = random.randint(0, 1)
        dut.N24.value = random.randint(0, 1)
        dut.N27.value = random.randint(0, 1)
        dut.N30.value = random.randint(0, 1)
        dut.N34.value = random.randint(0, 1)
        dut.N37.value = random.randint(0, 1)
        dut.N40.value = random.randint(0, 1)
        dut.N43.value = random.randint(0, 1)
        dut.N47.value = random.randint(0, 1)
        dut.N50.value = random.randint(0, 1)
        dut.N53.value = random.randint(0, 1)
        dut.N56.value = random.randint(0, 1)
        dut.N60.value = random.randint(0, 1)
        dut.N63.value = random.randint(0, 1)
        dut.N66.value = random.randint(0, 1)
        dut.N69.value = random.randint(0, 1)
        dut.N73.value = random.randint(0, 1)
        dut.N76.value = random.randint(0, 1)
        dut.N79.value = random.randint(0, 1)
        dut.N82.value = random.randint(0, 1)
        dut.N86.value = random.randint(0, 1)
        dut.N89.value = random.randint(0, 1)
        dut.N92.value = random.randint(0, 1)
        dut.N95.value = random.randint(0, 1)
        dut.N99.value = random.randint(0, 1)
        dut.N102.value = random.randint(0, 1)
        dut.N105.value = random.randint(0, 1)
        dut.N108.value = random.randint(0, 1)
        dut.N112.value = random.randint(0, 1)
        dut.N115.value = random.randint(0, 1)

        # Wait for 10ns to simulate processing time
        await Timer(10, units="ns")


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

    #debug of Args
    print("Args is ")
    print(extra_args)
    print("")

    runner = get_runner(sim)
    runner.build(
        verilog_sources=sources,
        hdl_toplevel="c432",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="c432", test_module="sim")


if __name__ == "__main__":
    runner()