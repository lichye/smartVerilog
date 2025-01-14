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

#     input GND,VDD,CK,n3065gat,n3066gat,n3067gat,n3068gat,n3069gat,n3070gat,
#   n3071gat,n3072gat,
#   n3073gat,n3074gat,n3075gat,n3076gat,n3077gat,n3078gat,n3079gat,n3080gat,
#   n3081gat,n3082gat,n3083gat,n3084gat,n3085gat,n3086gat,n3087gat,n3088gat,
#   n3089gat,n3090gat,n3091gat,n3092gat,n3093gat,n3094gat,n3095gat,n3097gat,
#   n3098gat,n3099gat,n3100gat;

    for cycle in range(10):
        # Assign random values to the inputs
        dut.GND.value = random.randint(0, 1)
        dut.VDD.value = random.randint(0, 1)    
        dut.CK.value = random.randint(0, 1)
        dut.n3065gat.value = random.randint(0, 1)
        dut.n3066gat.value = random.randint(0, 1)
        dut.n3067gat.value = random.randint(0, 1)
        dut.n3068gat.value = random.randint(0, 1)
        dut.n3069gat.value = random.randint(0, 1)
        dut.n3070gat.value = random.randint(0, 1)
        dut.n3071gat.value = random.randint(0, 1)
        dut.n3072gat.value = random.randint(0, 1)
        dut.n3073gat.value = random.randint(0, 1)
        dut.n3074gat.value = random.randint(0, 1)
        dut.n3075gat.value = random.randint(0, 1)
        dut.n3076gat.value = random.randint(0, 1)
        dut.n3077gat.value = random.randint(0, 1)
        dut.n3078gat.value = random.randint(0, 1)
        dut.n3079gat.value = random.randint(0, 1)
        dut.n3080gat.value = random.randint(0, 1)
        dut.n3081gat.value = random.randint(0, 1)
        dut.n3082gat.value = random.randint(0, 1)
        dut.n3083gat.value = random.randint(0, 1)
        dut.n3084gat.value = random.randint(0, 1)
        dut.n3085gat.value = random.randint(0, 1)
        dut.n3086gat.value = random.randint(0, 1)
        dut.n3087gat.value = random.randint(0, 1)
        dut.n3088gat.value = random.randint(0, 1)
        dut.n3089gat.value = random.randint(0, 1)
        dut.n3090gat.value = random.randint(0, 1)
        dut.n3091gat.value = random.randint(0, 1)
        dut.n3092gat.value = random.randint(0, 1)
        dut.n3093gat.value = random.randint(0, 1)
        dut.n3094gat.value = random.randint(0, 1)
        dut.n3095gat.value = random.randint(0, 1)
        dut.n3097gat.value = random.randint(0, 1)
        dut.n3098gat.value = random.randint(0, 1)
        dut.n3099gat.value = random.randint(0, 1)
        dut.n3100gat.value = random.randint(0, 1)

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
        hdl_toplevel="s5378",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="s5378", test_module="sim")


if __name__ == "__main__":
    runner()