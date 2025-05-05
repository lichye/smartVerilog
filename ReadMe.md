# Structual introduction:
smartVerilog/:
- smart: this contains the source code
- Benchmark: this contains benchmark test suits
- Docker: this contains the Dockerfile to install smart/harm
- install.sh: this contains the cmdline to install smart
- run.sh: the shell to run smart
- harm.sh: the shell to run harm

# Expriment:
All this experiment can be carried under smartVerilog/ directory clone from the repository.

## Install in clean ubuntu:
bash install.sh

## Install in Docker
cd Docker
docker build -t smart .
docker run -it --rm smart

## Run Benchmark:
bash run.sh [Benchmark_name]

example:
bash run.sh c17

The results will go to the [Benchmark_name] directory

## clean repository:
bash clean.sh

# Related Work:
## Install in Docker
cd Docker/egraph
docker build -t harm .
docker run -it --rm harm
## Run Benchmark
bash harm.sh [Benchmark_name]

example:
bash harm.sh c17