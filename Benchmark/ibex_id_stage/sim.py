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

    clock = Clock(dut.clk_i, 2, units="ns")  # Create a 10ns period clock on port clk
    cocotb.start_soon(clock.start())  # Start the clock

    await RisingEdge(dut.clk_i)

    # input wire clk_i;
	# input wire rst_ni;
	# input wire test_en_i;

    # input wire instr_valid_i;
	# input wire instr_new_i;
	# input wire [31:0] instr_rdata_i;
	# input wire [15:0] instr_rdata_c_i;
	# input wire instr_is_compressed_i;
    # input wire branch_decision_i;

    # input wire illegal_c_insn_i;
	# input wire instr_fetch_err_i;
	# input wire [31:0] pc_id_i;
	# input wire ex_valid_i;
	# input wire lsu_valid_i;

    # input [1:0] priv_mode_i;
	# input wire csr_mstatus_tw_i;
	# input wire illegal_csr_insn_i;

    # input wire lsu_addr_incr_req_i;
	# input wire [31:0] lsu_addr_last_i;
	# input wire csr_mstatus_mie_i;
	# input wire csr_msip_i;
	# input wire csr_mtip_i;
	# input wire csr_meip_i;
	# input wire [14:0] csr_mfip_i;
	# input wire irq_pending_i;
	# input wire irq_nm_i;
	# input wire lsu_load_err_i;
	# input wire lsu_store_err_i;

    # input wire debug_req_i;
	# input wire debug_single_step_i;
	# input wire debug_ebreakm_i;
	# input wire debug_ebreaku_i;
	# input wire [31:0] regfile_wdata_lsu_i;
	# input wire [31:0] regfile_wdata_ex_i;
	# input wire [31:0] csr_rdata_i;

    # Simulate 10 cycles (or steps) without clock
    dut.rst_ni = 1
    await RisingEdge(dut.clk_i)

    for cycle in range(10):
        await RisingEdge(dut.clk_i)
        dut.rst_ni = 0
        dut.test_en_i = random.randint(0, 1)
        dut.instr_valid_i = random.randint(0, 1)
        dut.instr_new_i = random.randint(0, 1)
        dut.instr_rdata_i = random.randint(0, 2**32-1)
        dut.instr_rdata_c_i = random.randint(0, 2**16-1)
        dut.instr_is_compressed_i = random.randint(0, 1)
        dut.branch_decision_i = random.randint(0, 1)
        dut.illegal_c_insn_i = random.randint(0, 1)
        dut.instr_fetch_err_i = random.randint(0, 1)
        dut.pc_id_i = random.randint(0, 2**32-1)
        dut.ex_valid_i = random.randint(0, 1)
        dut.lsu_valid_i = random.randint(0, 1)
        dut.priv_mode_i = random.randint(0, 3)
        dut.csr_mstatus_tw_i = random.randint(0, 1)
        dut.illegal_csr_insn_i = random.randint(0, 1)
        dut.lsu_addr_incr_req_i = random.randint(0, 1)
        dut.lsu_addr_last_i = random.randint(0, 2**32-1)
        dut.csr_mstatus_mie_i = random.randint(0, 1)
        dut.csr_msip_i = random.randint(0, 1)
        dut.csr_mtip_i = random.randint(0, 1)
        dut.csr_meip_i = random.randint(0, 1)
        dut.csr_mfip_i = random.randint(0, 2**15-1)
        dut.irq_pending_i = random.randint(0, 1)
        dut.irq_nm_i = random.randint(0, 1)
        dut.lsu_load_err_i = random.randint(0, 1)
        dut.lsu_store_err_i = random.randint(0, 1)
        dut.debug_req_i = random.randint(0, 1)
        dut.debug_single_step_i = random.randint(0, 1)
        dut.debug_ebreakm_i = random.randint(0, 1)
        dut.debug_ebreaku_i = random.randint(0, 1)
        dut.regfile_wdata_lsu_i = random.randint(0, 2**32-1)
        dut.regfile_wdata_ex_i = random.randint(0, 2**32-1)
        dut.csr_rdata_i = random.randint(0, 2**32-1)


    
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
    extra_args.append("--Wno-UNOPTFLAT")
    extra_args.append("--Wno-CASEOVERLAP")

    #debug of Args
    print("Args is ")
    print(extra_args)
    print("")

    runner = get_runner(sim)
    runner.build(
        verilog_sources=sources,
        hdl_toplevel="ibex_id_stage",
        build_args=extra_args,
    )

    runner.test(hdl_toplevel="ibex_id_stage", test_module="sim")


if __name__ == "__main__":
    runner()