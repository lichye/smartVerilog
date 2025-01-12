# firstly system verision
# ubuntu-20.04.6-desktop-amd64.iso

mkdir otherTools
cd otherTools

# Install cvc5
git clone https://github.com/cvc5/cvc5.git
cd cvc5
git checkout cvc5-1.2.0

./configure.sh
cd build         # default is ./build
make             # use -jN for parallel build with N threads
make check       # to run default set of tests
make install     # to install into the prefix specified above


# Install cocotb
pip install "cocotb==1.8.0"

# Install Pip-test
pip install pytest

#install yosys
wget -nc https://github.com/YosysHQ/oss-cad-suite-build/releases/download/2023-05-20/oss-cad-suite-linux-x64-20230520.tgz
tar -xvzf oss-cad-suite-linux-x64-20230520.tgz