# This file is public domain, it can be freely copied without restrictions.
# SPDX-License-Identifier: CC0-1.0

TOPLEVEL_LANG = iverilog
VERILOG_SOURCES = $(shell pwd)/digital.sv
COCOTB_TOPLEVEL = digital

COCOTB_TEST_MODULES = test_analog_model

VERBOSE=1

ifneq ($(filter $(SIM),ius xcelium),)
    SIM_ARGS += -unbuffered
endif

include $(shell cocotb-config --makefiles)/Makefile.sim
