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

./harm --vcd trace.vcd --clk clock --conf config.xml

harm --vcd bl_master1h.vcd --conf bl_masterConfig.xml

harm --csv ex3.csv --conf ex3Config.xml