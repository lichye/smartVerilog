# Define the verilog code directory and the verilog source file
VERILOG_DIR = verilog
VERILOG_SRC = $(VERILOG_DIR)/addsub.sv

SIM_BUILD = sim_build
SIM_RESULTS = sim_results
SMT_RESULTS = smt_results


clean:
	rm -rf $(SIM_BUILD)
	rm -rf $(SMT_RESULTS)/*
	rm -rf $(SIM_RESULTS)/*
	rm -rf __pycache__
	
sim:
	python sim.py $(VERILOG_SRC)
	mv $(SIM_BUILD)/dump.vcd $(SIM_RESULTS)/sim.vcd

smt:
	ebmc $(VERILOG_SRC) --module addsub --bound 10 --vcd $(SMT_RESULTS)/smt.vcd