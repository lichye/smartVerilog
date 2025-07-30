#!/bin/bash
source otherTools/venv/bin/activate
source otherTools/oss-cad-suite/environment

bash clean.sh
# Check $1 is the name of the file
if [ $# -lt 1 ]; then
    echo "Lack of arguments."
    echo "Usage: $0 <benchmark_name>"
    exit 1
fi

src_benchmarks="Benchmark"
target_dir="$1"

found_dir=$(find "$src_benchmarks" -type d -name "$target_dir" -print -quit)

if [ -n "$found_dir" ]; then
    echo "Directory found: $found_dir"
else
    echo "Directory $target_dir not found under $src_benchmarks."
    exit 1
fi

# Clean up previous runs
rm -rf $1

cd smart
make all_clean
cd ..

DEST_DIR="smart/user"
cp -r "$found_dir/"* "$DEST_DIR"



cd smart

# Run the smart core
mkdir user/
python setup.py $1
rm -rf *task

python smart.py $1
rm -rf *.sby
rm -rf *task


# # Run the evaluator 
# python checker.py $1
# python evaluater.py $1

cd ..
mkdir $1
mv smart/*.txt $1
mv smart/user/* $1
mv smart/*.sl $1
echo "Done with $1"