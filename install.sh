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
yes | sudo pip3 install pytest

python3 -m venv ~/venv
source ~/venv/bin/activate
yes | pip install "cocotb==1.8.0"

cd smart
mkdir user/
cd ..

# Install other tools
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
# git clone https://github.com/cvc5/cvc5.git
# cd cvc5
# git checkout cvc5-1.2.0

# sudo ./configure.sh --auto-download
# cd build         
# yes | sudo make            
# yes | sudo make check      
# yes | sudo make install
# cd ..
# cd ..
# cd ..


wget https://github.com/cvc5/cvc5/releases/download/cvc5-1.2.0/cvc5-Linux-x86_64-static.zip
unzip cvc5-Linux-x86_64-static.zip
cd cvc5-Linux-x86_64-static
cd bin
chmod +x cvc5
sudo mv cvc5 /usr/local/bin/
cd ..
cd ..

# wget https://github.com/cvc5/cvc5/releases/download/cvc5-1.2.0/cvc5-Linux-x86_64-static-gpl.zip