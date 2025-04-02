# This file contains the good command that run harm will use

# Run the docker
docker run -it --rm \
  -v /home/magna/smartVerilog/OtherToolsResult/harm:/smart \
  samger/harm

export PATH=$PATH:$(pwd)

# Move result file from harm to smart
cp OtherToolsReult/harm/assertions.txt smart/assertions.txt
