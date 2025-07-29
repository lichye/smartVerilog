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

    # input N1,N5,N9,N12,N15,N18,N23,N26,N29,N32,
    #   N35,N38,N41,N44,N47,N50,N53,N54,N55,N56,
    #   N57,N58,N59,N60,N61,N62,N63,N64,N65,N66,
    #   N69,N70,N73,N74,N75,N76,N77,N78,N79,N80,
    #   N81,N82,N83,N84,N85,N86,N87,N88,N89,N94,
    #   N97,N100,N103,N106,N109,N110,N111,N112,N113,N114,
    #   N115,N118,N121,N124,N127,N130,N133,N134,N135,N138,
    #   N141,N144,N147,N150,N151,N152,N153,N154,N155,N156,
    #   N157,N158,N159,N160,N161,N162,N163,N164,N165,N166,
    #   N167,N168,N169,N170,N171,N172,N173,N174,N175,N176,
    #   N177,N178,N179,N180,N181,N182,N183,N184,N185,N186,
    #   N187,N188,N189,N190,N191,N192,N193,N194,N195,N196,
    #   N197,N198,N199,N200,N201,N202,N203,N204,N205,N206,
    #   N207,N208,N209,N210,N211,N212,N213,N214,N215,N216,
    #   N217,N218,N219,N220,N221,N222,N223,N224,N225,N226,
    #   N227,N228,N229,N230,N231,N232,N233,N234,N235,N236,
    #   N237,N238,N239,N240,N242,N245,N248,N251,N254,N257,
    #   N260,N263,N267,N271,N274,N277,N280,N283,N286,N289,
    #   N293,N296,N299,N303,N307,N310,N313,N316,N319,N322,
    #   N325,N328,N331,N334,N337,N340,N343,N346,N349,N352,
    #   N355,N358,N361,N364,N367,N382,N241_I;

    dut.N1.value = 0
    dut.N5.value = 0
    dut.N9.value = 0
    dut.N12.value = 0
    dut.N15.value = 0
    dut.N18.value = 0
    dut.N23.value = 0
    dut.N26.value = 0
    dut.N29.value = 0
    dut.N32.value = 0
    dut.N35.value = 0
    dut.N38.value = 0
    dut.N41.value = 0
    dut.N44.value = 0
    dut.N47.value = 0
    dut.N50.value = 0
    dut.N53.value = 0
    dut.N54.value = 0
    dut.N55.value = 0
    dut.N56.value = 0
    dut.N57.value = 0
    dut.N58.value = 0
    dut.N59.value = 0
    dut.N60.value = 0
    dut.N61.value = 0
    dut.N62.value = 0
    dut.N63.value = 0
    dut.N64.value = 0
    dut.N65.value = 0
    dut.N66.value = 0
    dut.N69.value = 0
    dut.N70.value = 0
    dut.N73.value = 0
    dut.N74.value = 0
    dut.N75.value = 0
    dut.N76.value = 0
    dut.N77.value = 0
    dut.N78.value = 0
    dut.N79.value = 0
    dut.N80.value = 0
    dut.N81.value = 0
    dut.N82.value = 0
    dut.N83.value = 0
    dut.N84.value = 0
    dut.N85.value = 0
    dut.N86.value = 0
    dut.N87.value = 0
    dut.N88.value = 0
    dut.N89.value = 0
    dut.N94.value = 0
    dut.N97.value = 0
    dut.N100.value = 0
    dut.N103.value = 0
    dut.N106.value = 0
    dut.N109.value = 0
    dut.N110.value = 0
    dut.N111.value = 0
    dut.N112.value = 0
    dut.N113.value = 0
    dut.N114.value = 0
    dut.N115.value = 0
    dut.N118.value = 0
    dut.N121.value = 0
    dut.N124.value = 0
    dut.N127.value = 0
    dut.N130.value = 0
    dut.N133.value = 0
    dut.N134.value = 0
    dut.N135.value = 0
    dut.N138.value = 0
    dut.N141.value = 0
    dut.N144.value = 0
    dut.N147.value = 0
    dut.N150.value = 0
    dut.N151.value = 0
    dut.N152.value = 0
    dut.N153.value = 0
    dut.N154.value = 0
    dut.N155.value = 0
    dut.N156.value = 0
    dut.N157.value = 0
    dut.N158.value = 0
    dut.N159.value = 0
    dut.N160.value = 0
    dut.N161.value = 0
    dut.N162.value = 0
    dut.N163.value = 0
    dut.N164.value = 0
    dut.N165.value = 0
    dut.N166.value = 0
    dut.N167.value = 0
    dut.N168.value = 0
    dut.N169.value = 0
    dut.N170.value = 0
    dut.N171.value = 0
    dut.N172.value = 0
    dut.N173.value = 0
    dut.N174.value = 0
    dut.N175.value = 0
    dut.N176.value = 0
    dut.N177.value = 0
    dut.N178.value = 0
    dut.N179.value = 0
    dut.N180.value = 0
    dut.N181.value = 0
    dut.N182.value = 0
    dut.N183.value = 0
    dut.N184.value = 0
    dut.N185.value = 0
    dut.N186.value = 0
    dut.N187.value = 0
    dut.N188.value = 0
    dut.N189.value = 0
    dut.N190.value = 0
    dut.N191.value = 0
    dut.N192.value = 0
    dut.N193.value = 0
    dut.N194.value = 0
    dut.N195.value = 0
    dut.N196.value = 0
    dut.N197.value = 0
    dut.N198.value = 0
    dut.N199.value = 0
    dut.N200.value = 0
    dut.N201.value = 0
    dut.N202.value = 0
    dut.N203.value = 0
    dut.N204.value = 0
    dut.N205.value = 0
    dut.N206.value = 0
    dut.N207.value = 0
    dut.N208.value = 0
    dut.N209.value = 0
    dut.N210.value = 0
    dut.N211.value = 0
    dut.N212.value = 0
    dut.N213.value = 0
    dut.N214.value = 0
    dut.N215.value = 0
    dut.N216.value = 0
    dut.N217.value = 0
    dut.N218.value = 0
    dut.N219.value = 0
    dut.N220.value = 0
    dut.N221.value = 0
    dut.N222.value = 0
    dut.N223.value = 0
    dut.N224.value = 0
    dut.N225.value = 0
    dut.N226.value = 0
    dut.N227.value = 0
    dut.N228.value = 0
    dut.N229.value = 0
    dut.N230.value = 0
    dut.N231.value = 0
    dut.N232.value = 0
    dut.N233.value = 0
    dut.N234.value = 0
    dut.N235.value = 0
    dut.N236.value = 0
    dut.N237.value = 0
    dut.N238.value = 0
    dut.N239.value = 0
    dut.N240.value = 0
    dut.N242.value = 0
    dut.N245.value = 0
    dut.N248.value = 0
    dut.N251.value = 0
    dut.N254.value = 0
    dut.N257.value = 0
    dut.N260.value = 0
    dut.N263.value = 0
    dut.N267.value = 0
    dut.N271.value = 0
    dut.N274.value = 0
    dut.N277.value = 0
    dut.N280.value = 0
    dut.N283.value = 0
    dut.N286.value = 0
    dut.N289.value = 0
    dut.N293.value = 0
    dut.N296.value = 0
    dut.N299.value = 0
    dut.N303.value = 0
    dut.N307.value = 0
    dut.N310.value = 0
    dut.N313.value = 0
    dut.N316.value = 0
    dut.N319.value = 0
    dut.N322.value = 0
    dut.N325.value = 0
    dut.N328.value = 0
    dut.N331.value = 0
    dut.N334.value = 0
    dut.N337.value = 0
    dut.N340.value = 0
    dut.N343.value = 0
    dut.N346.value = 0
    dut.N349.value = 0
    dut.N352.value = 0
    dut.N355.value = 0
    dut.N358.value = 0
    dut.N361.value = 0
    dut.N364.value = 0
    dut.N367.value = 0
    dut.N382.value = 0
    dut.N241_I.value = 0

    # Simulate 10 cycles (or steps) without clock
    for cycle in range(10):
        dut.N1.value = random.randint(0, 1)
        dut.N5.value = random.randint(0, 1)
        dut.N9.value = random.randint(0, 1)
        dut.N12.value = random.randint(0, 1)
        dut.N15.value = random.randint(0, 1)
        dut.N18.value = random.randint(0, 1)
        dut.N23.value = random.randint(0, 1)
        dut.N26.value = random.randint(0, 1)
        dut.N29.value = random.randint(0, 1)
        dut.N32.value = random.randint(0, 1)
        dut.N35.value = random.randint(0, 1)
        dut.N38.value = random.randint(0, 1)
        dut.N41.value = random.randint(0, 1)
        dut.N44.value = random.randint(0, 1)
        dut.N47.value = random.randint(0, 1)
        dut.N50.value = random.randint(0, 1)
        dut.N53.value = random.randint(0, 1)
        dut.N54.value = random.randint(0, 1)
        dut.N55.value = random.randint(0, 1)
        dut.N56.value = random.randint(0, 1)
        dut.N57.value = random.randint(0, 1)
        dut.N58.value = random.randint(0, 1)
        dut.N59.value = random.randint(0, 1)
        dut.N60.value = random.randint(0, 1)
        dut.N61.value = random.randint(0, 1)
        dut.N62.value = random.randint(0, 1)
        dut.N63.value = random.randint(0, 1)
        dut.N64.value = random.randint(0, 1)
        dut.N65.value = random.randint(0, 1)
        dut.N66.value = random.randint(0, 1)
        dut.N69.value = random.randint(0, 1)
        dut.N70.value = random.randint(0, 1)
        dut.N73.value = random.randint(0, 1)
        dut.N74.value = random.randint(0, 1)
        dut.N75.value = random.randint(0, 1)
        dut.N76.value = random.randint(0, 1)
        dut.N77.value = random.randint(0, 1)
        dut.N78.value = random.randint(0, 1)
        dut.N79.value = random.randint(0, 1)
        dut.N80.value = random.randint(0, 1)
        dut.N81.value = random.randint(0, 1)
        dut.N82.value = random.randint(0, 1)
        dut.N83.value = random.randint(0, 1)
        dut.N84.value = random.randint(0, 1)
        dut.N85.value = random.randint(0, 1)
        dut.N86.value = random.randint(0, 1)
        dut.N87.value = random.randint(0, 1)
        dut.N88.value = random.randint(0, 1)
        dut.N89.value = random.randint(0, 1)
        dut.N94.value = random.randint(0, 1)
        dut.N97.value = random.randint(0, 1)
        dut.N100.value = random.randint(0, 1)
        dut.N103.value = random.randint(0, 1)
        dut.N106.value = random.randint(0, 1)
        dut.N109.value = random.randint(0, 1)
        dut.N110.value = random.randint(0, 1)
        dut.N111.value = random.randint(0, 1)
        dut.N112.value = random.randint(0, 1)
        dut.N113.value = random.randint(0, 1)
        dut.N114.value = random.randint(0, 1)
        dut.N115.value = random.randint(0, 1)
        dut.N118.value = random.randint(0, 1)
        dut.N121.value = random.randint(0, 1)
        dut.N124.value = random.randint(0, 1)
        dut.N127.value = random.randint(0, 1)
        dut.N130.value = random.randint(0, 1)
        dut.N133.value = random.randint(0, 1)
        dut.N134.value = random.randint(0, 1)
        dut.N135.value = random.randint(0, 1)
        dut.N138.value = random.randint(0, 1)
        dut.N141.value = random.randint(0, 1)
        dut.N144.value = random.randint(0, 1)
        dut.N147.value = random.randint(0, 1)
        dut.N150.value = random.randint(0, 1)
        dut.N151.value = random.randint(0, 1)
        dut.N152.value = random.randint(0, 1)
        dut.N153.value = random.randint(0, 1)
        dut.N154.value = random.randint(0, 1)
        dut.N155.value = random.randint(0, 1)
        dut.N156.value = random.randint(0, 1)
        dut.N157.value = random.randint(0, 1)
        dut.N158.value = random.randint(0, 1)
        dut.N159.value = random.randint(0, 1)
        dut.N160.value = random.randint(0, 1)
        dut.N161.value = random.randint(0, 1)
        dut.N162.value = random.randint(0, 1)
        dut.N163.value = random.randint(0, 1)
        dut.N164.value = random.randint(0, 1)
        dut.N165.value = random.randint(0, 1)
        dut.N166.value = random.randint(0, 1)
        dut.N167.value = random.randint(0, 1)
        dut.N168.value = random.randint(0, 1)
        dut.N169.value = random.randint(0, 1)
        dut.N170.value = random.randint(0, 1)
        dut.N171.value = random.randint(0, 1)
        dut.N172.value = random.randint(0, 1)
        dut.N173.value = random.randint(0, 1)
        dut.N174.value = random.randint(0, 1)
        dut.N175.value = random.randint(0, 1)
        dut.N176.value = random.randint(0, 1)
        dut.N177.value = random.randint(0, 1)
        dut.N178.value = random.randint(0, 1)
        dut.N179.value = random.randint(0, 1)
        dut.N180.value = random.randint(0, 1)
        dut.N181.value = random.randint(0, 1)
        dut.N182.value = random.randint(0, 1)
        dut.N183.value = random.randint(0, 1)
        dut.N184.value = random.randint(0, 1)
        dut.N185.value = random.randint(0, 1)
        dut.N186.value = random.randint(0, 1)
        dut.N187.value = random.randint(0, 1)
        dut.N188.value = random.randint(0, 1)
        dut.N189.value = random.randint(0, 1)
        dut.N190.value = random.randint(0, 1)
        dut.N191.value = random.randint(0, 1)
        dut.N192.value = random.randint(0, 1)
        dut.N193.value = random.randint(0, 1)
        dut.N194.value = random.randint(0, 1)
        dut.N195.value = random.randint(0, 1)
        dut.N196.value = random.randint(0, 1)
        dut.N197.value = random.randint(0, 1)
        dut.N198.value = random.randint(0, 1)
        dut.N199.value = random.randint(0, 1)
        dut.N200.value = random.randint(0, 1)
        dut.N201.value = random.randint(0, 1)
        dut.N202.value = random.randint(0, 1)
        dut.N203.value = random.randint(0, 1)
        dut.N204.value = random.randint(0, 1)
        dut.N205.value = random.randint(0, 1)
        dut.N206.value = random.randint(0, 1)
        dut.N207.value = random.randint(0, 1)
        dut.N208.value = random.randint(0, 1)
        dut.N209.value = random.randint(0, 1)
        dut.N210.value = random.randint(0, 1)
        dut.N211.value = random.randint(0, 1)
        dut.N212.value = random.randint(0, 1)
        dut.N213.value = random.randint(0, 1)
        dut.N214.value = random.randint(0, 1)
        dut.N215.value = random.randint(0, 1)
        dut.N216.value = random.randint(0, 1)
        dut.N217.value = random.randint(0, 1)
        dut.N218.value = random.randint(0, 1)
        dut.N219.value = random.randint(0, 1)
        dut.N220.value = random.randint(0, 1)
        dut.N221.value = random.randint(0, 1)
        dut.N222.value = random.randint(0, 1)
        dut.N223.value = random.randint(0, 1)
        dut.N224.value = random.randint(0, 1)
        dut.N225.value = random.randint(0, 1)
        dut.N226.value = random.randint(0, 1)
        dut.N227.value = random.randint(0, 1)
        dut.N228.value = random.randint(0, 1)
        dut.N229.value = random.randint(0, 1)
        dut.N230.value = random.randint(0, 1)
        dut.N231.value = random.randint(0, 1)
        dut.N232.value = random.randint(0, 1)
        dut.N233.value = random.randint(0, 1)
        dut.N234.value = random.randint(0, 1)
        dut.N235.value = random.randint(0, 1)
        dut.N236.value = random.randint(0, 1)
        dut.N237.value = random.randint(0, 1)
        dut.N238.value = random.randint(0, 1)
        dut.N239.value = random.randint(0, 1)
        dut.N240.value = random.randint(0, 1)
        dut.N242.value = random.randint(0, 1)
        dut.N245.value = random.randint(0, 1)
        dut.N248.value = random.randint(0, 1)
        dut.N251.value = random.randint(0, 1)
        dut.N254.value = random.randint(0, 1)
        dut.N257.value = random.randint(0, 1)
        dut.N260.value = random.randint(0, 1)
        dut.N263.value = random.randint(0, 1)
        dut.N267.value = random.randint(0, 1)
        dut.N271.value = random.randint(0, 1)
        dut.N274.value = random.randint(0, 1)
        dut.N277.value = random.randint(0, 1)
        dut.N280.value = random.randint(0, 1)
        dut.N283.value = random.randint(0, 1)
        dut.N286.value = random.randint(0, 1)
        dut.N289.value = random.randint(0, 1)
        dut.N293.value = random.randint(0, 1)
        dut.N296.value = random.randint(0, 1)
        dut.N299.value = random.randint(0, 1)
        dut.N303.value = random.randint(0, 1)
        dut.N307.value = random.randint(0, 1)
        dut.N310.value = random.randint(0, 1)
        dut.N313.value = random.randint(0, 1)
        dut.N316.value = random.randint(0, 1)
        dut.N319.value = random.randint(0, 1)
        dut.N322.value = random.randint(0, 1)
        dut.N325.value = random.randint(0, 1)
        dut.N328.value = random.randint(0, 1)
        dut.N331.value = random.randint(0, 1)
        dut.N334.value = random.randint(0, 1)
        dut.N337.value = random.randint(0, 1)
        dut.N340.value = random.randint(0, 1)
        dut.N343.value = random.randint(0, 1)
        dut.N346.value = random.randint(0, 1)
        dut.N349.value = random.randint(0, 1)
        dut.N352.value = random.randint(0, 1)
        dut.N355.value = random.randint(0, 1)
        dut.N358.value = random.randint(0, 1)
        dut.N361.value = random.randint(0, 1)
        dut.N364.value = random.randint(0, 1)
        dut.N367.value = random.randint(0, 1)
        dut.N382.value = random.randint(0, 1)
        dut.N241_I.value = random.randint(0, 1)
        
        # Wait for 10ns to simulate processing time
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

    #debug of Args
    print("Args is ")
    print(extra_args)
    print("")

    runner = get_runner(sim)
    runner.build(
        verilog_sources=sources,
        hdl_toplevel="c7552",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="c7552", test_module="sim")


if __name__ == "__main__":
    runner()