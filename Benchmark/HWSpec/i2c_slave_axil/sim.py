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

# input wire                    clk,
#     input wire                    rst,

#     /*
#      * I2C interface
#      */
#     input  wire                   i2c_scl_i,
#     output wire                   i2c_scl_o,
#     output wire                   i2c_scl_t,
#     input  wire                   i2c_sda_i,
#     output wire                   i2c_sda_o,
#     output wire                   i2c_sda_t,

#     /*
#      * AXI lite master interface
#      */
#     output wire [ADDR_WIDTH-1:0]  m_axil_awaddr,
#     output wire [2:0]             m_axil_awprot,
#     output wire                   m_axil_awvalid,
#     input  wire                   m_axil_awready,
#     output wire [DATA_WIDTH-1:0]  m_axil_wdata,
#     output wire [STRB_WIDTH-1:0]  m_axil_wstrb,
#     output wire                   m_axil_wvalid,
#     input  wire                   m_axil_wready,
#     input  wire [1:0]             m_axil_bresp,
#     input  wire                   m_axil_bvalid,
#     output wire                   m_axil_bready,
#     output wire [ADDR_WIDTH-1:0]  m_axil_araddr,
#     output wire [2:0]             m_axil_arprot,
#     output wire                   m_axil_arvalid,
#     input  wire                   m_axil_arready,
#     input  wire [DATA_WIDTH-1:0]  m_axil_rdata,
#     input  wire [1:0]             m_axil_rresp,
#     input  wire                   m_axil_rvalid,
#     output wire                   m_axil_rready,

#     /*
#      * Status
#      */
#     output wire                   busy,
#     output wire                   bus_addressed,
#     output wire                   bus_active,

#     /*
#      * Configuration
#      */
#     input  wire                   enable,
#     input  wire [6:0]             device_address

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
        dut.enable = random.randint(0, 1)
        dut.device_address = random.randint(0, 127)
        dut.i2c_scl_i = random.randint(0, 1)
        dut.i2c_sda_i = random.randint(0, 1)
        dut.m_axil_awaddr = random.randint(0, 255)
        dut.m_axil_awprot = random.randint(0, 7)
        dut.m_axil_awvalid = random.randint(0, 1)
        dut.m_axil_wdata = random.randint(0, 255)
        dut.m_axil_wstrb = random.randint(0, 15)
        dut.m_axil_wvalid = random.randint(0, 1)
        dut.m_axil_bready = random.randint(0, 1)
        dut.m_axil_araddr = random.randint(0, 255)
        dut.m_axil_arprot = random.randint(0, 7)
        dut.m_axil_arvalid = random.randint(0, 1)
        dut.m_axil_rready = random.randint(0, 1)
        dut.busy = random.randint(0, 1)
        dut.bus_addressed = random.randint(0, 1)
        dut.bus_active = random.randint(0, 1)


    
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
        hdl_toplevel="i2c_slave_axil_master",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="i2c_slave_axil_master", test_module="sim")


if __name__ == "__main__":
    runner()