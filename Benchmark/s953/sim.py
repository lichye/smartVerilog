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
    
    
#     input GND,VDD,CK,Rdy1RtHS1,Rdy2RtHS1,Rdy1BmHS1,Rdy2BmHS1,IInDoneHS1,RtTSHS1,
#   TpArrayHS1,
#   OutputHS1,WantBmHS1,WantRtHS1,OutAvHS1,FullOHS1,FullIIHS1,Prog_2,Prog_1,
#   Prog_0;


    for cycle in range(10):
        # Assign random values to the inputs
        dut.GND.value = random.randint(0, 1)
        dut.VDD.value = random.randint(0, 1)
        dut.CK.value = random.randint(0, 1)
        dut.Rdy1RtHS1.value = random.randint(0, 1)
        dut.Rdy2RtHS1.value = random.randint(0, 1)
        dut.Rdy1BmHS1.value = random.randint(0, 1)
        dut.Rdy2BmHS1.value = random.randint(0, 1)
        dut.IInDoneHS1.value = random.randint(0, 1)
        dut.RtTSHS1.value = random.randint(0, 1)
        dut.TpArrayHS1.value = random.randint(0, 1)
        dut.OutputHS1.value = random.randint(0, 1)
        dut.WantBmHS1.value = random.randint(0, 1)
        dut.WantRtHS1.value = random.randint(0, 1)
        dut.OutAvHS1.value = random.randint(0, 1)
        dut.FullOHS1.value = random.randint(0, 1)
        dut.FullIIHS1.value = random.randint(0, 1)
        dut.Prog_2.value = random.randint(0, 1)
        dut.Prog_1.value = random.randint(0, 1)
        dut.Prog_0.value = random.randint(0, 1)

        await Timer(10, units="ns")



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
    extra_args.append("-Wno-UNOPTFLAT")

    #debug of Args
    print("Args is ")
    print(extra_args)
    print("")

    runner = get_runner(sim)
    runner.build(
        verilog_sources=sources,
        hdl_toplevel="s953",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="s953", test_module="sim")


if __name__ == "__main__":
    runner()