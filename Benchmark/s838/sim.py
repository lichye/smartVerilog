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
    
    
#     input GND,VDD,CK,P_0,C_32,C_31,C_30,C_29,C_28,C_27,C_26,C_25,C_24,C_23,C_22,
#   C_21,C_20,
#   C_19,C_18,C_17,C_16,C_15,C_14,C_13,C_12,C_11,C_10,C_9,C_8,C_7,C_6,C_5,C_4,
#   C_3,C_2,C_1,C_0;

    for cycle in range(10):
        # Assign random values to the inputs
        dut.GND.value = random.randint(0, 1)
        dut.VDD.value = random.randint(0, 1)
        dut.CK.value = random.randint(0, 1)
        dut.P_0.value = random.randint(0, 1)
        dut.C_32.value = random.randint(0, 1)
        dut.C_31.value = random.randint(0, 1)
        dut.C_30.value = random.randint(0, 1)
        dut.C_29.value = random.randint(0, 1)
        dut.C_28.value = random.randint(0, 1)
        dut.C_27.value = random.randint(0, 1)
        dut.C_26.value = random.randint(0, 1)
        dut.C_25.value = random.randint(0, 1)
        dut.C_24.value = random.randint(0, 1)
        dut.C_23.value = random.randint(0, 1)
        dut.C_22.value = random.randint(0, 1)
        dut.C_21.value = random.randint(0, 1)
        dut.C_20.value = random.randint(0, 1)
        dut.C_19.value = random.randint(0, 1)
        dut.C_18.value = random.randint(0, 1)
        dut.C_17.value = random.randint(0, 1)
        dut.C_16.value = random.randint(0, 1)
        dut.C_15.value = random.randint(0, 1)
        dut.C_14.value = random.randint(0, 1)
        dut.C_13.value = random.randint(0, 1)
        dut.C_12.value = random.randint(0, 1)
        dut.C_11.value = random.randint(0, 1)
        dut.C_10.value = random.randint(0, 1)
        dut.C_9.value = random.randint(0, 1)
        dut.C_8.value = random.randint(0, 1)
        dut.C_7.value = random.randint(0, 1)
        dut.C_6.value = random.randint(0, 1)
        dut.C_5.value = random.randint(0, 1)
        dut.C_4.value = random.randint(0, 1)
        dut.C_3.value = random.randint(0, 1)
        dut.C_2.value = random.randint(0, 1)
        dut.C_1.value = random.randint(0, 1)
        dut.C_0.value = random.randint(0, 1)
        

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
        hdl_toplevel="s838",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="s838", test_module="sim")


if __name__ == "__main__":
    runner()