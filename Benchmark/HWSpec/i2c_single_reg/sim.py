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

# input  wire       clk,
#     input  wire       rst,

#     /*
#      * I2C interface
#      */
#     input  wire       scl_i,
#     output wire       scl_o,
#     output wire       scl_t,
#     input  wire       sda_i,
#     output wire       sda_o,
#     output wire       sda_t,

#     /*
#      * Data register
#      */
#     input  wire [7:0] data_in,
#     input  wire       data_latch,
#     output wire [7:0] data_out

@cocotb.test()
async def my_first_test(dut):
    """Test a design without clock signals."""
    random.seed(42)

    clock = Clock(dut.clk, 2, units="ns")  # Create a 10ns period clock on port clk
    cocotb.start_soon(clock.start())  # Start the clock

    await RisingEdge(dut.clk)

    # Simulate 10 cycles (or steps) without clock
    dut.rst = 1
    await RisingEdge(dut.clk)

    for cycle in range(10):
        await RisingEdge(dut.clk)
        dut.rst = 0
        dut.scl_i = random.randint(0, 1)
        dut.sda_i = random.randint(0, 1)
        dut.data_latch = random.randint(0, 1)
        dut.data_in = random.randint(0, 255)


    
def runner():
    sim = os.getenv("SIM", "icarus")

    dir_path = os.path.dirname(os.path.abspath(__file__))
    sources = glob.glob(os.path.join(dir_path, "*.sv"))

    runner = get_runner(sim)

    if sim == "icarus":
        build_args = ["-g2012", "-DDUMP_VCD"]
    elif sim == "verilator":
        build_args = [
            "--trace",
        ]
    else:
        build_args = []

    runner.build(
        verilog_sources=sources,
        hdl_toplevel="i2c_single_reg",
        build_args=build_args,   # 统一用 build_args
    )

    runner.test(hdl_toplevel="i2c_single_reg", test_module="sim")




if __name__ == "__main__":
    runner()