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
rm -rf Results/$1

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


# #Run basic evaluator
# python checker.py $1
# python evaluater.py $1

# # Run the reducer
python src/python/minimise_assertions.py runtime/SygusResult.sl runtime/reducedResult.txt
python src/python/clean_assertion.py
# # # Run the evaluator 
# python checker.py $1
# python evaluater.py $1

# # Run the LTL
# python smart.py $1
# python smart.py $1 1
# rm -rf *.sby
# rm -rf *task

# Run the evaluator
python checker.py $1 10
python evaluater.py $1 10

cd ..
mkdir Results
mkdir Results/$1
mv smart/*.txt Results/$1
mv smart/user/* Results/$1
echo "Done with $1"