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

# input GND,VDD,CK,g43,g49,g633,g634,g635,g645,g647,g648,g690,g694,g698,g702,
#   g722,g723,g751,
#   g752,g753,g754,g755,g756,g757,g781,g941,g962,g1000,g1008,g1016,g1080,g1234,
#   g1553,g1554,g786,g1206,g929,g955,g795,g1194,g1198,g1202,g24,g1203,g1196,g29,
#   g22,g28,g10,g23,g37,g26,g1,g27,g42,g11,g32,g41,g31,g45,g9,g44,g21,g30,g25;

    for cycle in range(10):
        # Assign random values to the inputs
        dut.GND.value = random.randint(0, 1)
        dut.VDD.value = random.randint(0, 1)
        dut.CK.value = random.randint(0, 1)
        dut.g43.value = random.randint(0, 1)
        dut.g49.value = random.randint(0, 1)
        dut.g633.value = random.randint(0, 1)
        dut.g634.value = random.randint(0, 1)
        dut.g635.value = random.randint(0, 1)
        dut.g645.value = random.randint(0, 1)
        dut.g647.value = random.randint(0, 1)
        dut.g648.value = random.randint(0, 1)
        dut.g690.value = random.randint(0, 1)
        dut.g694.value = random.randint(0, 1)
        dut.g698.value = random.randint(0, 1)
        dut.g702.value = random.randint(0, 1)
        dut.g722.value = random.randint(0, 1)
        dut.g723.value = random.randint(0, 1)
        dut.g751.value = random.randint(0, 1)
        dut.g752.value = random.randint(0, 1)
        dut.g753.value = random.randint(0, 1)
        dut.g754.value = random.randint(0, 1)
        dut.g755.value = random.randint(0, 1)
        dut.g756.value = random.randint(0, 1)
        dut.g757.value = random.randint(0, 1)
        dut.g781.value = random.randint(0, 1)
        dut.g941.value = random.randint(0, 1)
        dut.g962.value = random.randint(0, 1)
        dut.g1000.value = random.randint(0, 1)
        dut.g1008.value = random.randint(0, 1)
        dut.g1016.value = random.randint(0, 1)
        dut.g1080.value = random.randint(0, 1)
        dut.g1234.value = random.randint(0, 1)
        dut.g1553.value = random.randint(0, 1)
        dut.g1554.value = random.randint(0, 1)
        dut.g786.value = random.randint(0, 1)
        dut.g1206.value = random.randint(0, 1)
        dut.g929.value = random.randint(0, 1)
        dut.g955.value = random.randint(0, 1)
        dut.g795.value = random.randint(0, 1)
        dut.g1194.value = random.randint(0, 1)
        dut.g1198.value = random.randint(0, 1)
        dut.g1202.value = random.randint(0, 1)
        dut.g24.value = random.randint(0, 1)
        dut.g1203.value = random.randint(0, 1)
        dut.g1196.value = random.randint(0, 1)
        dut.g29.value = random.randint(0, 1)
        dut.g22.value = random.randint(0, 1)
        dut.g28.value = random.randint(0, 1)
        dut.g10.value = random.randint(0, 1)
        dut.g23.value = random.randint(0, 1)
        dut.g37.value = random.randint(0, 1)
        dut.g26.value = random.randint(0, 1)
        dut.g1.value = random.randint(0, 1)
        dut.g27.value = random.randint(0, 1)
        dut.g42.value = random.randint(0, 1)
        dut.g11.value = random.randint(0, 1)
        dut.g32.value = random.randint(0, 1)
        dut.g41.value = random.randint(0, 1)
        dut.g31.value = random.randint(0, 1)
        dut.g45.value = random.randint(0, 1)
        dut.g9.value = random.randint(0, 1)
        dut.g44.value = random.randint(0, 1)
        dut.g21.value = random.randint(0, 1)
        dut.g30.value = random.randint(0, 1)
        dut.g25.value = random.randint(0, 1)


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
        hdl_toplevel="s13207",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="s13207", test_module="sim")


if __name__ == "__main__":
    runner()