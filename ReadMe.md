## Install:
bash install.sh

## OR install in Docker
cd Docker
docker build -t smart .
docker run -it --rm smart

## Run Benchmark:
bash run.sh [Benchmark_name]

example:
bash run.sh max3
bash run.sh c17

The results will go to the [Benchmark_name] directory

## clean repository:
bash clean.sh