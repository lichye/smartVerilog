#!/bin/bash
# Basic setup
# ubuntu-20.04.6-desktop-amd64.iso with git
# sudo apt-get install git
# git clone https://github.com/lichye/smartVerilog.git
# cd smartVerilog

#install basic libary
yes | apt-get update
yes | apt-get install cmake gcc g++ bison flex
yes | apt-get install python3 python-is-python3 pytest
yes | apt-get install pip
yes | apt-get install python3-venv
yes | pip3 install pytest
# Install cocotb
yes | pip install "cocotb==1.8.0"

mkdir otherTools
cd otherTools

#install yosys
wget -nc https://github.com/YosysHQ/oss-cad-suite-build/releases/download/2023-05-20/oss-cad-suite-linux-x64-20230520.tgz
tar -xvzf oss-cad-suite-linux-x64-20230520.tgz
source oss-cad-suite/environment

#install ebmc
wget https://github.com/diffblue/hw-cbmc/releases/download/ebmc-5.4/ebmc_5.4_amd64.deb
sudo dpkg -i ebmc_5.4_amd64.deb

# Install cvc5
git clone https://github.com/cvc5/cvc5.git
cd cvc5
git checkout cvc5-1.2.0

./configure.sh --auto-download
cd build         # default is ./build
make             # use -jN for parallel build with N threads
make check       # to run default set of tests
yes | sudo make install     # to install into the prefix specified above
cd ..
cd ..
cd ..
