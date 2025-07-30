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

#   input  wire [3:0]  s_axil_awaddr,
#     input  wire [2:0]  s_axil_awprot,
#     input  wire        s_axil_awvalid,
#     input  wire [31:0] s_axil_wdata,
#     input  wire [3:0]  s_axil_wstrb,
#     input  wire        s_axil_wvalid,
#     input  wire        s_axil_bready,
#     input  wire [3:0]  s_axil_araddr,
#     input  wire [2:0]  s_axil_arprot,
#     input  wire        s_axil_arvalid,
#      input  wire        s_axil_rready,
#      input  wire        i2c_scl_i,
#     input  wire        i2c_sda_i,


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
        dut.s_axil_araddr = random.randint(0, 15)
        dut.s_axil_arprot = random.randint(0, 7)
        dut.s_axil_arvalid = 1
        dut.s_axil_wdata = random.randint(0, 4294967295)
        dut.s_axil_wstrb = random.randint(0, 15)
        dut.s_axil_wvalid = 1
        dut.s_axil_bready = 1
        dut.s_axil_awaddr = random.randint(0, 15)
        dut.s_axil_awprot = random.randint(0, 7)
        dut.s_axil_awvalid = 1
        dut.s_axil_rready = 1
        dut.i2c_scl_i = random.randint(0, 1)
        dut.i2c_sda_i = random.randint(0, 1)



    
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
        hdl_toplevel="i2c_master_axil",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="i2c_master_axil", test_module="sim")


if __name__ == "__main__":
    runner()