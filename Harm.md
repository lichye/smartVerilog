1. clone the code

git clone https://github.com/SamueleGerminiani/harm

2. install third party code

sudo apt-get install -y uuid-dev pkg-config

cd third_party

bash install_all.sh

3. build the code

mkdir build && cd build

cmake -DCMAKE_BUILD_TYPE=Release ..

make


4. export the cmdline

export PATH=$PATH:path_to_/build

harm --vcd bl_master1h.vcd --clk clock --conf bl_masterConfig.xml
