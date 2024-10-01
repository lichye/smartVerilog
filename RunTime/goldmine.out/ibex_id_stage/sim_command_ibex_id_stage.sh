#!/bin/bash

iverilog -g2001 -osimv -s ibex_id_stage_bench /opt/goldmine/RunTime/goldmine.out/ibex_id_stage/ibex_id_stage_bench.v /opt/goldmine/verilog/IBex/ibex_id_stage.v /opt/goldmine/verilog/IBex/ibex_decoder.v /opt/goldmine/verilog/IBex/ibex_controller.v /opt/goldmine/verilog/IBex/ibex_register_file_ff.v  -I/opt/goldmine/verilog/IBex