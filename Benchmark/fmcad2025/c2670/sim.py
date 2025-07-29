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

#    input N1,N2,N3,N4,N5,N6,N7,N8,N11,N14,
#       N15,N16,N19,N20,N21,N22,N23,N24,N25,N26,
#       N27,N28,N29,N32,N33,N34,N35,N36,N37,N40,
#       N43,N44,N47,N48,N49,N50,N51,N52,N53,N54,
#       N55,N56,N57,N60,N61,N62,N63,N64,N65,N66,
#       N67,N68,N69,N72,N73,N74,N75,N76,N77,N78,
#       N79,N80,N81,N82,N85,N86,N87,N88,N89,N90,
#       N91,N92,N93,N94,N95,N96,N99,N100,N101,N102,
#       N103,N104,N105,N106,N107,N108,N111,N112,N113,N114,
#       N115,N116,N117,N118,N119,N120,N123,N124,N125,N126,
#       N127,N128,N129,N130,N131,N132,N135,N136,N137,N138,
#       N139,N140,N141,N142,N219,N224,N227,N230,N231,N234,
#       N237,N241,N246,N253,N256,N259,N262,N263,N266,N269,
#       N272,N275,N278,N281,N284,N287,N290,N294,N297,N301,
#       N305,N309,N313,N316,N319,N322,N325,N328,N331,N334,
#       N337,N340,N343,N346,N349,N352,N355,N143_I,N144_I,N145_I,
#       N146_I,N147_I,N148_I,N149_I,N150_I,N151_I,N152_I,N153_I,N154_I,N155_I,
#       N156_I,N157_I,N158_I,N159_I,N160_I,N161_I,N162_I,N163_I,N164_I,N165_I,
#       N166_I,N167_I,N168_I,N169_I,N170_I,N171_I,N172_I,N173_I,N174_I,N175_I,
#       N176_I,N177_I,N178_I,N179_I,N180_I,N181_I,N182_I,N183_I,N184_I,N185_I,
#       N186_I,N187_I,N188_I,N189_I,N190_I,N191_I,N192_I,N193_I,N194_I,N195_I,
#       N196_I,N197_I,N198_I,N199_I,N200_I,N201_I,N202_I,N203_I,N204_I,N205_I,
#       N206_I,N207_I,N208_I,N209_I,N210_I,N211_I,N212_I,N213_I,N214_I,N215_I,
#       N216_I,N217_I,N218_I;
    dut.N1.value = 0
    dut.N2.value = 0
    dut.N3.value = 0
    dut.N4.value = 0
    dut.N5.value = 0
    dut.N6.value = 0
    dut.N7.value = 0
    dut.N8.value = 0
    dut.N11.value = 0
    dut.N14.value = 0
    dut.N15.value = 0
    dut.N16.value = 0
    dut.N19.value = 0
    dut.N20.value = 0
    dut.N21.value = 0
    dut.N22.value = 0
    dut.N23.value = 0
    dut.N24.value = 0
    dut.N25.value = 0
    dut.N26.value = 0
    dut.N27.value = 0
    dut.N28.value = 0
    dut.N29.value = 0
    dut.N32.value = 0
    dut.N33.value = 0
    dut.N34.value = 0
    dut.N35.value = 0
    dut.N36.value = 0
    dut.N37.value = 0
    dut.N40.value = 0
    dut.N43.value = 0
    dut.N44.value = 0
    dut.N47.value = 0
    dut.N48.value = 0
    dut.N49.value = 0
    dut.N50.value = 0
    dut.N51.value = 0
    dut.N52.value = 0
    dut.N53.value = 0
    dut.N54.value = 0
    dut.N55.value = 0
    dut.N56.value = 0
    dut.N57.value = 0
    dut.N60.value = 0
    dut.N61.value = 0
    dut.N62.value = 0
    dut.N63.value = 0
    dut.N64.value = 0
    dut.N65.value = 0
    dut.N66.value = 0
    dut.N67.value = 0
    dut.N68.value = 0
    dut.N69.value = 0
    dut.N72.value = 0
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
    dut.N85.value = 0
    dut.N86.value = 0
    dut.N87.value = 0
    dut.N88.value = 0
    dut.N89.value = 0
    dut.N90.value = 0
    dut.N91.value = 0
    dut.N92.value = 0
    dut.N93.value = 0
    dut.N94.value = 0
    dut.N95.value = 0
    dut.N96.value = 0
    dut.N99.value = 0
    dut.N100.value = 0
    dut.N101.value = 0
    dut.N102.value = 0
    dut.N103.value = 0
    dut.N104.value = 0
    dut.N105.value = 0
    dut.N106.value = 0
    dut.N107.value = 0
    dut.N108.value = 0
    dut.N111.value = 0
    dut.N112.value = 0
    dut.N113.value = 0
    dut.N114.value = 0
    dut.N115.value = 0
    dut.N116.value = 0
    dut.N117.value = 0
    dut.N118.value = 0
    dut.N119.value = 0
    dut.N120.value = 0
    dut.N123.value = 0
    dut.N124.value = 0
    dut.N125.value = 0
    dut.N126.value = 0
    dut.N127.value = 0
    dut.N128.value = 0
    dut.N129.value = 0
    dut.N130.value = 0
    dut.N131.value = 0
    dut.N132.value = 0
    dut.N135.value = 0
    dut.N136.value = 0
    dut.N137.value = 0
    dut.N138.value = 0
    dut.N139.value = 0
    dut.N140.value = 0
    dut.N141.value = 0
    dut.N142.value = 0
    dut.N143_I.value = 0
    dut.N144_I.value = 0
    dut.N145_I.value = 0
    dut.N146_I.value = 0
    dut.N147_I.value = 0
    dut.N148_I.value = 0
    dut.N149_I.value = 0
    dut.N150_I.value = 0
    dut.N151_I.value = 0
    dut.N152_I.value = 0
    dut.N153_I.value = 0
    dut.N154_I.value = 0
    dut.N155_I.value = 0
    dut.N156_I.value = 0
    dut.N157_I.value = 0
    dut.N158_I.value = 0
    dut.N159_I.value = 0
    dut.N160_I.value = 0
    dut.N161_I.value = 0
    dut.N162_I.value = 0
    dut.N163_I.value = 0
    dut.N164_I.value = 0
    dut.N165_I.value = 0
    dut.N166_I.value = 0
    dut.N167_I.value = 0
    dut.N168_I.value = 0
    dut.N169_I.value = 0
    dut.N170_I.value = 0
    dut.N171_I.value = 0
    dut.N172_I.value = 0
    dut.N173_I.value = 0
    dut.N174_I.value = 0
    dut.N175_I.value = 0
    dut.N176_I.value = 0
    dut.N177_I.value = 0
    dut.N178_I.value = 0
    dut.N179_I.value = 0
    dut.N180_I.value = 0
    dut.N181_I.value = 0
    dut.N182_I.value = 0
    dut.N183_I.value = 0
    dut.N184_I.value = 0
    dut.N185_I.value = 0
    dut.N186_I.value = 0
    dut.N187_I.value = 0
    dut.N188_I.value = 0
    dut.N189_I.value = 0
    dut.N190_I.value = 0
    dut.N191_I.value = 0
    dut.N192_I.value = 0
    dut.N193_I.value = 0
    dut.N194_I.value = 0
    dut.N195_I.value = 0
    dut.N196_I.value = 0
    dut.N197_I.value = 0
    dut.N198_I.value = 0
    dut.N199_I.value = 0
    dut.N200_I.value = 0
    dut.N201_I.value = 0
    dut.N202_I.value = 0
    dut.N203_I.value = 0
    dut.N204_I.value = 0
    dut.N205_I.value = 0
    dut.N206_I.value = 0
    dut.N207_I.value = 0
    dut.N208_I.value = 0
    dut.N209_I.value = 0
    dut.N210_I.value = 0
    dut.N211_I.value = 0
    dut.N212_I.value = 0
    dut.N213_I.value = 0
    dut.N214_I.value = 0
    dut.N215_I.value = 0
    dut.N216_I.value = 0
    dut.N217_I.value = 0
    dut.N218_I.value = 0
    dut.N219.value = 0
    dut.N224.value = 0
    dut.N227.value = 0
    dut.N230.value = 0
    dut.N231.value = 0
    dut.N234.value = 0
    dut.N237.value = 0
    dut.N241.value = 0
    dut.N246.value = 0
    dut.N253.value = 0
    dut.N256.value = 0
    dut.N259.value = 0
    dut.N262.value = 0
    dut.N263.value = 0
    dut.N266.value = 0
    dut.N269.value = 0
    dut.N272.value = 0
    dut.N275.value = 0
    dut.N278.value = 0
    dut.N281.value = 0
    dut.N284.value = 0
    dut.N287.value = 0
    dut.N290.value = 0
    dut.N294.value = 0
    dut.N297.value = 0
    dut.N301.value = 0
    dut.N305.value = 0
    dut.N309.value = 0
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





    # Simulate 10 cycles (or steps) without clock
    for cycle in range(10):
        dut.N1.value = random.randint(0, 1)
        dut.N2.value = random.randint(0, 1)
        dut.N3.value = random.randint(0, 1)
        dut.N4.value = random.randint(0, 1)
        dut.N5.value = random.randint(0, 1)
        dut.N6.value = random.randint(0, 1)
        dut.N7.value = random.randint(0, 1)
        dut.N8.value = random.randint(0, 1)
        dut.N11.value = random.randint(0, 1)
        dut.N14.value = random.randint(0, 1)
        dut.N15.value = random.randint(0, 1)
        dut.N16.value = random.randint(0, 1)
        dut.N19.value = random.randint(0, 1)
        dut.N20.value = random.randint(0, 1)
        dut.N21.value = random.randint(0, 1)
        dut.N22.value = random.randint(0, 1)
        dut.N23.value = random.randint(0, 1)
        dut.N24.value = random.randint(0, 1)
        dut.N25.value = random.randint(0, 1)
        dut.N26.value = random.randint(0, 1)
        dut.N27.value = random.randint(0, 1)
        dut.N28.value = random.randint(0, 1)
        dut.N29.value = random.randint(0, 1)
        dut.N32.value = random.randint(0, 1)
        dut.N33.value = random.randint(0, 1)
        dut.N34.value = random.randint(0, 1)
        dut.N35.value = random.randint(0, 1)
        dut.N36.value = random.randint(0, 1)
        dut.N37.value = random.randint(0, 1)
        dut.N40.value = random.randint(0, 1)
        dut.N43.value = random.randint(0, 1)
        dut.N44.value = random.randint(0, 1)
        dut.N47.value = random.randint(0, 1)
        dut.N48.value = random.randint(0, 1)
        dut.N49.value = random.randint(0, 1)
        dut.N50.value = random.randint(0, 1)
        dut.N51.value = random.randint(0, 1)
        dut.N52.value = random.randint(0, 1)
        dut.N53.value = random.randint(0, 1)
        dut.N54.value = random.randint(0, 1)
        dut.N55.value = random.randint(0, 1)
        dut.N56.value = random.randint(0, 1)
        dut.N57.value = random.randint(0, 1)
        dut.N60.value = random.randint(0, 1)
        dut.N61.value = random.randint(0, 1)
        dut.N62.value = random.randint(0, 1)
        dut.N63.value = random.randint(0, 1)
        dut.N64.value = random.randint(0, 1)
        dut.N65.value = random.randint(0, 1)
        dut.N66.value = random.randint(0, 1)
        dut.N67.value = random.randint(0, 1)
        dut.N68.value = random.randint(0, 1)
        dut.N69.value = random.randint(0, 1)
        dut.N72.value = random.randint(0, 1)
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
        dut.N85.value = random.randint(0, 1)
        dut.N86.value = random.randint(0, 1)
        dut.N87.value = random.randint(0, 1)
        dut.N88.value = random.randint(0, 1)
        dut.N89.value = random.randint(0, 1)
        dut.N90.value = random.randint(0, 1)
        dut.N91.value = random.randint(0, 1)
        dut.N92.value = random.randint(0, 1)
        dut.N93.value = random.randint(0, 1)
        dut.N94.value = random.randint(0, 1)
        dut.N95.value = random.randint(0, 1)
        dut.N96.value = random.randint(0, 1)
        dut.N99.value = random.randint(0, 1)
        dut.N100.value = random.randint(0, 1)
        dut.N101.value = random.randint(0, 1)
        dut.N102.value = random.randint(0, 1)
        dut.N103.value = random.randint(0, 1)
        dut.N104.value = random.randint(0, 1)
        dut.N105.value = random.randint(0, 1)
        dut.N106.value = random.randint(0, 1)
        dut.N107.value = random.randint(0, 1)

        dut.N108.value = random.randint(0, 1)
        dut.N111.value = random.randint(0, 1)
        dut.N112.value = random.randint(0, 1)
        dut.N113.value = random.randint(0, 1)
        dut.N114.value = random.randint(0, 1)
        dut.N115.value = random.randint(0, 1)
        dut.N116.value = random.randint(0, 1)
        dut.N117.value = random.randint(0, 1)
        dut.N118.value = random.randint(0, 1)
        dut.N119.value = random.randint(0, 1)
        dut.N120.value = random.randint(0, 1)
        dut.N123.value = random.randint(0, 1)
        dut.N124.value = random.randint(0, 1)
        dut.N125.value = random.randint(0, 1)
        dut.N126.value = random.randint(0, 1)
        dut.N127.value = random.randint(0, 1)
        dut.N128.value = random.randint(0, 1)
        dut.N129.value = random.randint(0, 1)
        dut.N130.value = random.randint(0, 1)
        dut.N131.value = random.randint(0, 1)
        dut.N132.value = random.randint(0, 1)
        dut.N135.value = random.randint(0, 1)
        dut.N136.value = random.randint(0, 1)
        dut.N137.value = random.randint(0, 1)
        dut.N138.value = random.randint(0, 1)
        dut.N139.value = random.randint(0, 1)
        dut.N140.value = random.randint(0, 1)
        dut.N141.value = random.randint(0, 1)
        dut.N142.value = random.randint(0, 1)
        dut.N143_I.value = random.randint(0, 1)
        dut.N144_I.value = random.randint(0, 1)
        dut.N145_I.value = random.randint(0, 1)
        dut.N146_I.value = random.randint(0, 1)
        dut.N147_I.value = random.randint(0, 1)
        dut.N148_I.value = random.randint(0, 1)
        dut.N149_I.value = random.randint(0, 1)
        dut.N150_I.value = random.randint(0, 1)
        dut.N151_I.value = random.randint(0, 1)
        dut.N152_I.value = random.randint(0, 1)
        dut.N153_I.value = random.randint(0, 1)
        dut.N154_I.value = random.randint(0, 1)
        dut.N155_I.value = random.randint(0, 1)
        dut.N156_I.value = random.randint(0, 1)
        dut.N157_I.value = random.randint(0, 1)
        dut.N158_I.value = random.randint(0, 1)
        dut.N159_I.value = random.randint(0, 1)
        dut.N160_I.value = random.randint(0, 1)
        dut.N161_I.value = random.randint(0, 1)
        dut.N162_I.value = random.randint(0, 1)
        dut.N163_I.value = random.randint(0, 1)
        dut.N164_I.value = random.randint(0, 1)
        dut.N165_I.value = random.randint(0, 1)
        dut.N166_I.value = random.randint(0, 1)
        dut.N167_I.value = random.randint(0, 1)
        dut.N168_I.value = random.randint(0, 1)
        dut.N169_I.value = random.randint(0, 1)
        dut.N170_I.value = random.randint(0, 1)
        dut.N171_I.value = random.randint(0, 1)
        dut.N172_I.value = random.randint(0, 1)
        dut.N173_I.value = random.randint(0, 1)
        dut.N174_I.value = random.randint(0, 1)
        dut.N175_I.value = random.randint(0, 1)
        dut.N176_I.value = random.randint(0, 1)
        dut.N177_I.value = random.randint(0, 1)
        dut.N178_I.value = random.randint(0, 1)
        dut.N179_I.value = random.randint(0, 1)
        dut.N180_I.value = random.randint(0, 1)
        dut.N181_I.value = random.randint(0, 1)
        dut.N182_I.value = random.randint(0, 1)
        dut.N183_I.value = random.randint(0, 1)
        dut.N184_I.value = random.randint(0, 1)
        dut.N185_I.value = random.randint(0, 1)
        dut.N186_I.value = random.randint(0, 1)
        dut.N187_I.value = random.randint(0, 1)
        dut.N188_I.value = random.randint(0, 1)
        dut.N189_I.value = random.randint(0, 1)
        dut.N190_I.value = random.randint(0, 1)
        dut.N191_I.value = random.randint(0, 1)
        dut.N192_I.value = random.randint(0, 1)
        dut.N193_I.value = random.randint(0, 1)
        dut.N194_I.value = random.randint(0, 1)
        dut.N195_I.value = random.randint(0, 1)
        dut.N196_I.value = random.randint(0, 1)
        dut.N197_I.value = random.randint(0, 1)
        dut.N198_I.value = random.randint(0, 1)
        dut.N199_I.value = random.randint(0, 1)
        dut.N200_I.value = random.randint(0, 1)
        dut.N201_I.value = random.randint(0, 1)
        dut.N202_I.value = random.randint(0, 1)
        dut.N203_I.value = random.randint(0, 1)
        dut.N204_I.value = random.randint(0, 1)
        dut.N205_I.value = random.randint(0, 1)
        dut.N206_I.value = random.randint(0, 1)
        dut.N207_I.value = random.randint(0, 1)
        dut.N208_I.value = random.randint(0, 1)
        dut.N209_I.value = random.randint(0, 1)
        dut.N210_I.value = random.randint(0, 1)
        dut.N211_I.value = random.randint(0, 1)
        dut.N212_I.value = random.randint(0, 1)
        dut.N213_I.value = random.randint(0, 1)
        dut.N214_I.value = random.randint(0, 1)
        dut.N215_I.value = random.randint(0, 1)
        dut.N216_I.value = random.randint(0, 1)
        dut.N217_I.value = random.randint(0, 1)
        dut.N218_I.value = random.randint(0, 1)
        dut.N219.value = random.randint(0, 1)
        dut.N224.value = random.randint(0, 1)
        dut.N227.value = random.randint(0, 1)
        dut.N230.value = random.randint(0, 1)
        dut.N231.value = random.randint(0, 1)
        dut.N234.value = random.randint(0, 1)
        dut.N237.value = random.randint(0, 1)
        dut.N241.value = random.randint(0, 1)
        dut.N246.value = random.randint(0, 1)
        dut.N253.value = random.randint(0, 1)
        dut.N256.value = random.randint(0, 1)
        dut.N259.value = random.randint(0, 1)
        dut.N262.value = random.randint(0, 1)
        dut.N263.value = random.randint(0, 1)
        dut.N266.value = random.randint(0, 1)
        dut.N269.value = random.randint(0, 1)
        dut.N272.value = random.randint(0, 1)
        dut.N275.value = random.randint(0, 1)
        dut.N278.value = random.randint(0, 1)
        dut.N281.value = random.randint(0, 1)
        dut.N284.value = random.randint(0, 1)
        dut.N287.value = random.randint(0, 1)
        dut.N290.value = random.randint(0, 1)
        dut.N294.value = random.randint(0, 1)
        dut.N297.value = random.randint(0, 1)
        dut.N301.value = random.randint(0, 1)
        dut.N305.value = random.randint(0, 1)
        dut.N309.value = random.randint(0, 1)
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
        hdl_toplevel="c2670",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="c2670", test_module="sim")


if __name__ == "__main__":
    runner()