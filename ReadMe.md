# Structual introduction:
smartVerilog:
- smart: this contains the source code
- Benchmark: this contains benchmark test suits
- Docker: this contains the Dockerfile to install smart/harm
- install.sh: this contains the cmdline to install smart
- run.sh: the shell to run smart
- harm.sh: the shell to run harm

# Expriment:
All this experiment can be carried under smartVerilog/ directory clone from the repository.

## Install 
Firstly, git clone this repository.

way-1 in clean ubuntu:

    bash install.sh

way-2 in Docker(Preferred):
    
    cd Docker

    docker build -t smart --no-cache .

    docker run -it --rm smart

## Run Benchmark under smartVerilog directory:

    bash run.sh [Benchmark_name]


example:
    
    bash run.sh c17

When you see "Done with c17" means the expriment is done for c17.

The results will go to the [Benchmark_name] directory: with name result_[Benchmark_name].txt

example: c17/result_c17.txt

Inside the file the content would be:

    Smart Time: 2.412510633468628
    Overall Time: 2.4580302238464355
    Pre analysis Time: 0.0446622371673584
    We found 14 assertions:
    ......
    *Important information in the paper:*
    The Verified correctness(VC) rate is : 100.0
    The mutation detection(MD) rate is : 100.0

## clean repository:
    
    bash clean.sh

# Related Work(HARM):

## Install in Docker

    cd Docker/egraph
    docker build -t --no-cache harm .
    docker run -it --rm harm

## Run Benchmark

    bash harm.sh [Benchmark_name]

    example:
    
    bash harm.sh c17

# The resource requirements:
## SMART
RAM: 16GB Core: 4+

## HARM
RAM: 32GB Core: 8+

## Expriment time cost:
From 1min to 1.5day for different cases.

To run all the benchmark needs totally 10days for smart and 5days for HARM.