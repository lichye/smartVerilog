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
    

#     input GND,VDD,CK,DATA_0_31,DATA_0_30,DATA_0_29,DATA_0_28,DATA_0_27,DATA_0_26,
#   DATA_0_25,
#   DATA_0_24,DATA_0_23,DATA_0_22,DATA_0_21,DATA_0_20,DATA_0_19,DATA_0_18,
#   DATA_0_17,DATA_0_16,DATA_0_15,DATA_0_14,DATA_0_13,DATA_0_12,DATA_0_11,
#   DATA_0_10,DATA_0_9,DATA_0_8,DATA_0_7,DATA_0_6,DATA_0_5,DATA_0_4,DATA_0_3,
#   DATA_0_2,DATA_0_1,DATA_0_0,RESET,TM1,TM0;

    # Simulate 10 cycles (or steps) without clock
    for cycle in range(10):
        # Set random values for inputs
        dut.GND.value = random.randint(0, 1)
        dut.VDD.value = random.randint(0, 1)
        dut.CK.value = random.randint(0, 1)
        dut.DATA_0_31.value = random.randint(0, 1)
        dut.DATA_0_30.value = random.randint(0, 1)
        dut.DATA_0_29.value = random.randint(0, 1)
        dut.DATA_0_28.value = random.randint(0, 1)
        dut.DATA_0_27.value = random.randint(0, 1)
        dut.DATA_0_26.value = random.randint(0, 1)
        dut.DATA_0_25.value = random.randint(0, 1)
        dut.DATA_0_24.value = random.randint(0, 1)
        dut.DATA_0_23.value = random.randint(0, 1)
        dut.DATA_0_22.value = random.randint(0, 1)
        dut.DATA_0_21.value = random.randint(0, 1)
        dut.DATA_0_20.value = random.randint(0, 1)
        dut.DATA_0_19.value = random.randint(0, 1)
        dut.DATA_0_18.value = random.randint(0, 1)
        dut.DATA_0_17.value = random.randint(0, 1)
        dut.DATA_0_16.value = random.randint(0, 1)
        dut.DATA_0_15.value = random.randint(0, 1)
        dut.DATA_0_14.value = random.randint(0, 1)
        dut.DATA_0_13.value = random.randint(0, 1)
        dut.DATA_0_12.value = random.randint(0, 1)
        dut.DATA_0_11.value = random.randint(0, 1)
        dut.DATA_0_10.value = random.randint(0, 1)
        dut.DATA_0_9.value = random.randint(0, 1)
        dut.DATA_0_8.value = random.randint(0, 1)
        dut.DATA_0_7.value = random.randint(0, 1)
        dut.DATA_0_6.value = random.randint(0, 1)
        dut.DATA_0_5.value = random.randint(0, 1)
        dut.DATA_0_4.value = random.randint(0, 1)
        dut.DATA_0_3.value = random.randint(0, 1)
        dut.DATA_0_2.value = random.randint(0, 1)
        dut.DATA_0_1.value = random.randint(0, 1)
        dut.DATA_0_0.value = random.randint(0, 1)
        dut.RESET.value = random.randint(0, 1)
        dut.TM1.value = random.randint(0, 1)
        dut.TM0.value = random.randint(0, 1)
        
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
    extra_args.append("-Wno-UNOPTFLAT")
    
    #debug of Args
    print("Args is ")
    print(extra_args)
    print("")

    runner = get_runner(sim)
    runner.build(
        verilog_sources=sources,
        hdl_toplevel="s35932",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="s35932", test_module="sim")


if __name__ == "__main__":
    runner()