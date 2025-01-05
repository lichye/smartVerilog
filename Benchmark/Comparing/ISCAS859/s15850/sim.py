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

#     input GND,VDD,CK,g18,g27,g109,g741,g742,g743,g744,g872,g873,g877,g881,g1712,
#   g1960,g1961,
#   g1696,g750,g85,g42,g1700,g102,g104,g101,g29,g28,g103,g83,g23,g87,g922,g892,
#   g84,g919,g1182,g925,g48,g895,g889,g1185,g41,g43,g99,g1173,g1203,g1188,g1197,
#   g46,g31,g45,g92,g89,g898,g91,g93,g913,g82,g88,g1194,g47,g96,g910,g95,g904,
#   g1176,g901,g44,g916,g100,g886,g30,g86,g1170,g1200,g1191,g907,g90,g94,g1179;

    # Example: Initialize signals if necessary
    dut.GND.value = 0
    dut.VDD.value = 0
    dut.CK.value = 0
    dut.g18.value = 0
    dut.g27.value = 0
    dut.g109.value = 0
    dut.g741.value = 0
    dut.g742.value = 0
    dut.g743.value = 0
    dut.g744.value = 0
    dut.g872.value = 0
    dut.g873.value = 0
    dut.g877.value = 0
    dut.g881.value = 0
    dut.g1712.value = 0
    dut.g1960.value = 0
    dut.g1961.value = 0
    dut.g1696.value = 0
    dut.g750.value = 0
    dut.g85.value = 0
    dut.g42.value = 0
    dut.g1700.value = 0
    dut.g102.value = 0
    dut.g104.value = 0
    dut.g101.value = 0
    dut.g29.value = 0
    dut.g28.value = 0
    dut.g103.value = 0
    dut.g83.value = 0
    dut.g23.value = 0
    dut.g87.value = 0
    dut.g922.value = 0
    dut.g892.value = 0
    dut.g84.value = 0
    dut.g919.value = 0
    dut.g1182.value = 0
    dut.g925.value = 0
    dut.g48.value = 0
    dut.g895.value = 0
    dut.g889.value = 0
    dut.g1185.value = 0
    dut.g41.value = 0
    dut.g43.value = 0
    dut.g99.value = 0
    dut.g1173.value = 0
    dut.g1203.value = 0
    dut.g1188.value = 0
    dut.g1197.value = 0
    dut.g46.value = 0
    dut.g31.value = 0
    dut.g45.value = 0
    dut.g92.value = 0
    dut.g89.value = 0
    dut.g898.value = 0
    dut.g91.value = 0
    dut.g93.value = 0
    dut.g913.value = 0
    dut.g82.value = 0
    dut.g88.value = 0
    dut.g1194.value = 0
    dut.g47.value = 0
    dut.g96.value = 0
    dut.g910.value = 0
    dut.g95.value = 0
    dut.g904.value = 0
    dut.g1176.value = 0
    dut.g901.value = 0
    dut.g44.value = 0
    dut.g916.value = 0
    dut.g100.value = 0
    dut.g886.value = 0
    dut.g30.value = 0
    dut.g86.value = 0
    dut.g1170.value = 0
    dut.g1200.value = 0
    dut.g1191.value = 0
    dut.g907.value = 0
    dut.g90.value = 0
    dut.g94.value = 0
    dut.g1179.value = 0

    

    # Simulate 10 cycles (or steps) without clock
    for cycle in range(10):
        # Set random values for inputs
        dut.GND.value = random.randint(0, 1)
        dut.VDD.value = random.randint(0, 1)
        dut.CK.value = random.randint(0, 1)
        dut.g18.value = random.randint(0, 1)
        dut.g27.value = random.randint(0, 1)
        dut.g109.value = random.randint(0, 1)
        dut.g741.value = random.randint(0, 1)
        dut.g742.value = random.randint(0, 1)
        dut.g743.value = random.randint(0, 1)
        dut.g744.value = random.randint(0, 1)
        dut.g872.value = random.randint(0, 1)
        dut.g873.value = random.randint(0, 1)
        dut.g877.value = random.randint(0, 1)
        dut.g881.value = random.randint(0, 1)
        dut.g1712.value = random.randint(0, 1)
        dut.g1960.value = random.randint(0, 1)
        dut.g1961.value = random.randint(0, 1)
        dut.g1696.value = random.randint(0, 1)
        dut.g750.value = random.randint(0, 1)
        dut.g85.value = random.randint(0, 1)
        dut.g42.value = random.randint(0, 1)
        dut.g1700.value = random.randint(0, 1)
        dut.g102.value = random.randint(0, 1)
        dut.g104.value = random.randint(0, 1)
        dut.g101.value = random.randint(0, 1)
        dut.g29.value = random.randint(0, 1)
        dut.g28.value = random.randint(0, 1)
        dut.g103.value = random.randint(0, 1)
        dut.g83.value = random.randint(0, 1)
        dut.g23.value = random.randint(0, 1)
        dut.g87.value = random.randint(0, 1)
        dut.g922.value = random.randint(0, 1)
        dut.g892.value = random.randint(0, 1)
        dut.g84.value = random.randint(0, 1)
        dut.g919.value = random.randint(0, 1)
        dut.g1182.value = random.randint(0, 1)
        dut.g925.value = random.randint(0, 1)
        dut.g48.value = random.randint(0, 1)
        dut.g895.value = random.randint(0, 1)
        dut.g889.value = random.randint(0, 1)
        dut.g1185.value = random.randint(0, 1)
        dut.g41.value = random.randint(0, 1)
        dut.g43.value = random.randint(0, 1)
        dut.g99.value = random.randint(0, 1)
        dut.g1173.value = random.randint(0, 1)
        dut.g1203.value = random.randint(0, 1)
        dut.g1188.value = random.randint(0, 1)
        dut.g1197.value = random.randint(0, 1)
        dut.g46.value = random.randint(0, 1)
        dut.g31.value = random.randint(0, 1)
        dut.g45.value = random.randint(0, 1)
        dut.g92.value = random.randint(0, 1)
        dut.g89.value = random.randint(0, 1)
        dut.g898.value = random.randint(0, 1)
        dut.g91.value = random.randint(0, 1)
        dut.g93.value = random.randint(0, 1)
        dut.g913.value = random.randint(0, 1)
        dut.g82.value = random.randint(0, 1)
        dut.g88.value = random.randint(0, 1)
        dut.g1194.value = random.randint(0, 1)
        dut.g47.value = random.randint(0, 1)
        dut.g96.value = random.randint(0, 1)
        dut.g910.value = random.randint(0, 1)
        dut.g95.value = random.randint(0, 1)
        dut.g904.value = random.randint(0, 1)
        dut.g1176.value = random.randint(0, 1)
        dut.g901.value = random.randint(0, 1)
        dut.g44.value = random.randint(0, 1)
        dut.g916.value = random.randint(0, 1)
        dut.g100.value = random.randint(0, 1)
        dut.g886.value = random.randint(0, 1)
        dut.g30.value = random.randint(0, 1)
        dut.g86.value = random.randint(0, 1)
        dut.g1170.value = random.randint(0, 1)
        dut.g1200.value = random.randint(0, 1)
        dut.g1191.value = random.randint(0, 1)
        dut.g907.value = random.randint(0, 1)
        dut.g90.value = random.randint(0, 1)
        dut.g94.value = random.randint(0, 1)
        dut.g1179.value = random.randint(0, 1)


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
        hdl_toplevel="s15850",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="s15850", test_module="sim")


if __name__ == "__main__":
    runner()