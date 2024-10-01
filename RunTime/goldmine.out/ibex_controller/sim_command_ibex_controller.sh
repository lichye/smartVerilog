#!/bin/bash

iverilog -g2001 -osimv -s ibex_controller_bench /opt/goldmine/RunTime/goldmine.out/ibex_controller/ibex_controller_bench.v /opt/goldmine/verilog/IBex/ibex_controller.v  -I/opt/goldmine/verilog/IBex