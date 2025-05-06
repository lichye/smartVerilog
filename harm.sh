# This works in harm
if [ $# -lt 1 ]; then
    echo "Lack of arguments."
    echo "Usage: $0 <benchmark_name>"
    exit 1
fi
src_benchmarks="otherToolsResult/harm/$1"

if [ -d "$src_benchmarks" ]; then
    echo "Directory $src_benchmarks exists."
else
    echo "Directory $src_benchmarks does not exist."
    exit 1
fi

source otherTools/venv/bin/activate
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

cd ..

mv smart/runtime/sim_results/sim1.vcd otherToolsResult/harm/$1/$1.vcd

cd otherToolsResult/harm
bash run.sh $1
python getResult.py
cd ..
cd ..
cp otherToolsResult/harm/assertions.txt smart/assertions.txt
cd otherToolsResult/harm
make clean
cd ..
cd ..

cd smart
python checker.py $1
python evaluater.py $1

cd ..

mkdir $1
mv smart/*.txt $1

bash clean.sh
echo "Done with $1"
