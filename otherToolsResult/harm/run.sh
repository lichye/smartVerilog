#!/bin/bash

# Check $1 is the name of the file
if [ $# -lt 1 ]; then
    echo "Lack of arguments."
    echo "Usage: $0 <benchmark_name>"
    exit 1
fi
src_benchmarks="$1"

if [ -d "$src_benchmarks" ]; then
    echo "Directory $src_benchmarks exists."
else
    echo "Directory $src_benchmarks does not exist."
    exit 1
fi

cd $1

harm --vcd $1.vcd --clk top::CK --conf $1.xml --generate-config
harm --vcd $1.vcd --clk top::CK --conf $1.xml --sva --dump-to result.txt

cd ..

cp $1/result.txt result.txt

echo "Done with $1"