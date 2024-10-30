import random

import os
import random
from pathlib import Path

import cocotb
from cocotb.clock import Clock
from cocotb.runner import get_runner
from cocotb.triggers import FallingEdge
from cocotb.triggers import RisingEdge, Timer
from cocotb.binary import BinaryValue


@cocotb.test()
async def my_first_test(dut):
    """Try accessing the design."""
    

    for cycle in range(10):
        #clk rising edge
        dut.clk.value = 1
        await Timer(1, units="ns")

        dut.dataa.value = random.randint(0,100)
        dut.datab.value = random.randint(0,100)
        dut.add_sub.value = 1

        print(f"a={int(dut.dataa.value)}, add_sub={dut.add_sub.value}, b={int(dut.datab.value)}, result={int(dut.result.value)}")
        assert(dut.result.value == dut.dataa.value + dut.datab.value)
        # print(f"Cycle:{cycle}")

        #clk falling edge
        dut.clk.value = 0
        await Timer(1, units="ns")


def runner():
    sim = os.getenv("SIM", "verilator")
    verilog_sources = ["addsub.v"]

    # set parameters
    extra_args = []
    
    extra_args.append(f"--trace")
    
    print("Args is ")
    print(extra_args)
    print("")
    runner = get_runner(sim)
    runner.build(
        verilog_sources=verilog_sources,
        hdl_toplevel="addsub",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="addsub", test_module="test")


if __name__ == "__main__":
    runner()
