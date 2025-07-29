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

    clock = Clock(dut.clk, 2, units="ns")  # Create a 10ns period clock on port clk
    cocotb.start_soon(clock.start())  # Start the clock

    await RisingEdge(dut.clk)

    # Simulate 10 cycles (or steps) without clock
    dut.rst = 1
    await RisingEdge(dut.clk)
    dut.rst = 0

    # input  wire [6:0]  s_axis_cmd_address,
    # input  wire        s_axis_cmd_start,
    # input  wire        s_axis_cmd_read,
    # input  wire        s_axis_cmd_write,
    # input  wire        s_axis_cmd_write_multiple,
    # input  wire        s_axis_cmd_stop,
    # input  wire        s_axis_cmd_valid,

    # input  wire [7:0]  s_axis_data_tdata,
    # input  wire        s_axis_data_tvalid,
    # input  wire        s_axis_data_tlast,
    # input  wire        m_axis_data_tready,

    # input  wire        scl_i,
    # input  wire        sda_i,
    
    # input  wire [15:0] prescale,
    # input  wire        stop_on_idle

    for cycle in range(10):
        await RisingEdge(dut.clk)
        dut.rst = 0
        dut.s_axis_cmd_address.value = random.randint(0, 127)
        dut.s_axis_cmd_start.value = random.choice([0, 1])
        dut.s_axis_cmd_read.value = random.choice([0, 1])
        dut.s_axis_cmd_write.value = random.choice([0, 1])
        dut.s_axis_cmd_write_multiple.value = random.choice([0, 1])
        dut.s_axis_cmd_stop.value = random.choice([0, 1])
        dut.s_axis_cmd_valid.value = random.choice([0, 1])
        dut.s_axis_data_tdata.value = random.randint(0, 255)
        dut.s_axis_data_tvalid.value = random.choice([0, 1])
        dut.s_axis_data_tlast.value = random.choice([0, 1])
        dut.m_axis_data_tready.value = random.choice([0, 1])
        dut.scl_i.value = random.choice([0, 1])
        dut.sda_i.value = random.choice([0, 1])
        dut.prescale.value = random.randint(0, 65535)
        dut.stop_on_idle.value = random.choice([0, 1])



    
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

    #debug of Args
    print("Args is ")
    print(extra_args)
    print("")

    runner = get_runner(sim)
    runner.build(
        verilog_sources=sources,
        hdl_toplevel="i2c_master",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="i2c_master", test_module="sim")


if __name__ == "__main__":
    runner()