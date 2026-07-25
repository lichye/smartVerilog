#!/bin/bash
# Basic setup
# ubuntu-20.04.6-desktop-amd64.iso with git
# sudo apt-get install git
# git clone https://github.com/lichye/smartVerilog.git
# cd smartVerilog

#install basic libary
yes | sudo apt-get update
yes | sudo apt-get install cmake gcc g++ bison flex unzip wget make
yes | sudo apt-get install python3 python-is-python3
yes | sudo apt-get install pip
yes | sudo apt-get install python3-venv
yes | sudo apt install nlohmann-json3-dev
yes | sudo pip3 install pytest

python3 -m venv otherTools/venv
source otherTools/venv/bin/activate
yes | pip install "cocotb==1.8.0"
yes | pip install more_itertools
yes | pip install matplotlib
yes | pip install igraph
yes | pip install z3-solver
yes | pip install pysmt
yes | pip install utils
yes | pip install cvc5

mkdir -p smart/user

# Install other tools
mkdir -p otherTools
cd otherTools

#install yosys/verilator via oss-cad-suite
wget -nc https://github.com/YosysHQ/oss-cad-suite-build/releases/download/2023-05-20/oss-cad-suite-linux-x64-20230520.tgz
tar -xvzf oss-cad-suite-linux-x64-20230520.tgz
source oss-cad-suite/environment

#install ebmc
wget https://github.com/diffblue/hw-cbmc/releases/download/ebmc-5.6/ebmc_5.6_amd64.deb
sudo dpkg -i ebmc_5.6_amd64.deb

# Install cvc5 (static release binary)
wget https://github.com/cvc5/cvc5/releases/download/cvc5-1.2.0/cvc5-Linux-x86_64-static.zip
unzip cvc5-Linux-x86_64-static.zip
chmod +x cvc5-Linux-x86_64-static/bin/cvc5
sudo mv cvc5-Linux-x86_64-static/bin/cvc5 /usr/local/bin/
cd ..

# Verify the environment
python run.py --check-env
