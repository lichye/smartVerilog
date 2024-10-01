#!/bin/bash

iverilog -g2001 -osimv -s ibex_compressed_decoder_bench /opt/goldmine/RunTime/goldmine.out/ibex_compressed_decoder/ibex_compressed_decoder_bench.v /opt/goldmine/verilog/IBex/ibex_compressed_decoder.v  -I/opt/goldmine/verilog/IBex