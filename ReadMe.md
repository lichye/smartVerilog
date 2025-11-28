# Install 
## Install-1  in clean ubuntu(related work is NOT supported):
Download the zip file and unzip the whole file, then run the command line below smartVerilog/.

    bash install.sh

## Install-2 in Docker(Preferred):

    docker pull magna2024/smart 

## stability of result

    git clone https://github.com/lichye/MutationBenchmark.git

# Usage

    python run.py [benchmarkName]

## Example

    python run.py c17

Result will be in SmartVerilog/Result/c17/

There is some setup Config for usage, change it in run.py or default.json.

## Expriment time cost:
From 1min to 1.5day for different cases.

To run all the benchmark needs totally 10days for smart and 5days for HARM.