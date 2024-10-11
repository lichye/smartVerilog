website:
https://sites.google.com/view/goldmine-illinois/home

1. clone the code
git clone https://bitbucket.org/debjitp/goldminer/src/master/

2. modify goldminer.cfg

3. build the docker

docker build --network=host -t pgoldmine .

4. change the correct path

docker run -it -v Path_to/RunTime:/opt/goldmine/RunTime  pgoldmine /bin/bash



cd src/vcd_parser
python setup.py build_ext --inplace
ln -s ./vcd_parser/parse_timeframes.so 
cd ../../RunTime
python ../src/goldmine.py -h

sh run_goldmine.sh

/usr/bin/iverilog