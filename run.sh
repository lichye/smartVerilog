#!/bin/bash
# $1 is the name of the file

if [ $# -lt 1 ]; then
    echo "Lack of arguments."
    echo "Usage: $0 <benchmark_name>"
    exit 1
fi
src_benchmarks="Benchmark/$1"

if [ -d "$src_benchmarks" ]; then
    echo "Directory $src_benchmarks exists."
else
    echo "Directory $src_benchmarks does not exist."
    exit 1
fi

source otherTools/oss-cad-suite/environment

cd smart
make all_clean
cd ..


DEST_DIR="smart/user"
echo "Full path: $src_benchmarks"
cp "$src_benchmarks"/* "$DEST_DIR"

cd smart
mkdir user/
python setup.py $1
python smart.py $1
python evaluater.py $1
cd ..