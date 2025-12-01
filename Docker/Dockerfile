FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y \
        cmake gcc g++ bison flex unzip wget make \
        nlohmann-json3-dev vim nano \
        python3 python3-pip python3-venv && \
    ln -sf python3 /usr/bin/python && \
    pip3 install pytest

WORKDIR /workspace
RUN apt-get install -y git
RUN git clone https://github.com/lichye/smartVerilog.git

WORKDIR /workspace/smartVerilog
RUN mkdir -p /workspace/smartVerilog/smart/user/

RUN pip install cocotb==1.8.0
RUN pip install more_itertools
RUN pip install matplotlib
RUN pip install igraph
RUN pip install z3-solver
RUN pip install pysmt
RUN pip install utils
RUN pip install cvc5

WORKDIR /workspace/smartVerilog/otherTools
RUN wget -nc https://github.com/YosysHQ/oss-cad-suite-build/releases/download/2023-05-20/oss-cad-suite-linux-x64-20230520.tgz && \
    tar -xvzf oss-cad-suite-linux-x64-20230520.tgz

RUN wget https://github.com/diffblue/hw-cbmc/releases/download/ebmc-5.6/ebmc_5.6_amd64.deb && \
    apt-get install -y ./ebmc_5.6_amd64.deb

RUN wget https://github.com/cvc5/cvc5/releases/download/cvc5-1.2.0/cvc5-Linux-x86_64-static.zip && \
    unzip cvc5-Linux-x86_64-static.zip && \
    chmod +x cvc5-Linux-x86_64-static/bin/cvc5 && \
    mv cvc5-Linux-x86_64-static/bin/cvc5 /usr/local/bin/

WORKDIR /workspace/smartVerilog
CMD ["/bin/bash"]
