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

    # input wire                        clk,
    # input wire                        rst,

    # /*
    #  * I2C interface
    #  */
    # input  wire                       i2c_scl_i,
    # input  wire                       i2c_sda_i,

    # /*
    #  * Wishbone interface
    #  */
    # input  wire [WB_DATA_WIDTH-1:0]   wb_dat_i,   // DAT_I() data in
    # input  wire                       wb_ack_i,   // ACK_I acknowledge input
    # input  wire                       wb_err_i,   // ERR_I error input

    # /*
    #  * Configuration
    #  */
    # input  wire                       enable,
    # input  wire [6:0]                 device_address

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
        dut.i2c_scl_i = random.randint(0, 1)
        dut.i2c_sda_i = random.randint(0, 1)
        dut.i2c_sda_i = random.randint(0, 1)
        dut.wb_dat_i = random.randint(0, 255)
        dut.wb_ack_i = random.randint(0, 1)
        dut.wb_err_i = random.randint(0, 1)
        dut.enable = random.randint(0, 1)
        dut.device_address = random.randint(0, 127)

    
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
    extra_args.append("--Wno-UNOPTFLAT")
    extra_args.append("--Wno-CASEOVERLAP")
    extra_args.append("--Wno-CASEINCOMPLETE")
    extra_args.append("--Wno-PINMISSING")
    extra_args.append("--Wno-SELRANGE")
    extra_args.append("--Wno-IMPLICIT")

    #debug of Args
    print("Args is ")
    print(extra_args)
    print("")

    runner = get_runner(sim)
    runner.build(
        verilog_sources=sources,
        hdl_toplevel="i2c_slave_wbm",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="i2c_slave_wbm", test_module="sim")


if __name__ == "__main__":
    runner()