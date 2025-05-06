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

    clock = Clock(dut.CK, 2, units="ns")  # Create a 10ns period clock on port CK
    cocotb.start_soon(clock.start())  # Start the clock

    await RisingEdge(dut.CK)
    dut.rst_ni.value = 1


    # Simulate 10 cycles (or steps) without clock
    for cycle in range(100000):
        await RisingEdge(dut.CK)
        # Set inputs to random values
        dut.rst_ni = 0

        dut.mult_en_i = random.randint(0, 1)
        dut.div_en_i = random.randint(0, 1)

        dut.operator_i = random.randint(0, 3)
        dut.signed_mode_i = random.randint(0, 3)
        dut.op_a_i = random.randint(0, 2**32-1)
        dut.op_b_i = random.randint(0, 2**32-1)
        dut.alu_adder_ext_i = random.randint(0, 2**34-1)
        dut.alu_adder_i = random.randint(0, 2**32-1)
        dut.equal_to_zero = random.randint(0, 1)

    
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
    extra_args.append("--Wno-UNOPTFLAT")

    #debug of Args
    print("Args is ")
    print(extra_args)
    print("")

    runner = get_runner(sim)
    runner.build(
        verilog_sources=sources,
        hdl_toplevel="ibex_multdiv_slow",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="ibex_multdiv_slow", test_module="sim")


if __name__ == "__main__":
    runner()