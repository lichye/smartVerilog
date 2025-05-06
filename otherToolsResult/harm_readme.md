# pull the docker image
docker pull samger/harm

# run the docker image
docker run -it --rm \
  -v /home/magna/smartVerilog/otherToolsResult/harm:/smart \
  samger/harm

# make harm into the path
# under the file dir: harm/build
export PATH=$PATH:$(pwd)


# run the shell script
bash run.sh c17

# Could also run the command by hand

harm --vcd $1.vcd --clk top::clk --conf $1.xml --generate-config
harm --vcd c17.vcd --clk top::clk --conf c17.xml --sva --dump-to assertions.txt


