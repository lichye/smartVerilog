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

    # Example: Initialize signals if necessary
    dut.instr_new_i.value = 0
    dut.instr_rdata_i.value = 0
    dut.illegal_c_insn_i.value = 0

    # Simulate 10 cycles (or steps) without clock
    for cycle in range(10):
        # Set random values for inputs
        dut.instr_new_i.value = random.randint(0, 1)
        dut.instr_rdata_i.value = random.randint(0, 2 ** 32 - 1)
        dut.illegal_c_insn_i.value = random.randint(0, 1)

        # Wait for 10ns to simulate processing time
        await Timer(10, units="ns")

        # Log the current state for debugging
        dut._log.info(
            f"Cycle {cycle}: instr_new_i={dut.instr_new_i.value}, "
            f"instr_rdata_i={dut.instr_rdata_i.value}, "
            f"illegal_c_insn_i={dut.illegal_c_insn_i.value}"
        )


def runner():
    sim = os.getenv("SIM", "verilator")

    dir_path = os.path.dirname(os.path.abspath(__file__))

    print("File path is "+str(dir_path))

    subprocess.run(["mkdir", "sim_build"])

    subprocess.run(["cp","ibex_pkg.sv","sim_build/ibex_pkg.sv"])

    # set parameters
    extra_args = []
    
    extra_args.append(f"--trace")

    # extra_args.append("--lint-only")
    extra_args.append("-Wno-WIDTHEXPAND")
    extra_args.append("-Wno-WIDTHTRUNC")


    sources = glob.glob(os.path.join(dir_path, "*.sv"))

    print("Sources is "+str(sources))
    
    print("Args is ")
    print(extra_args)
    print("")
    runner = get_runner(sim)
    runner.build(
        verilog_sources=sources,
        hdl_toplevel="ibex_decoder",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="ibex_decoder", test_module="sim")


if __name__ == "__main__":
    runner()