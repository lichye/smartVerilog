FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y \
        cmake gcc g++ bison flex unzip wget make \
        python3 python3-pip python3-venv && \
    ln -sf python3 /usr/bin/python && \
    pip3 install pytest

RUN apt-get install cargo -y

WORKDIR /workspace
RUN apt-get install -y git
RUN date && git clone --branch develop https://github.com/lichye/smartVerilog.git

WORKDIR /workspace/smartVerilog
RUN git reset --hard
RUN git pull
RUN mkdir -p /workspace/smartVerilog/smart/user/

RUN pip install cocotb==1.8.0

WORKDIR /workspace/smartVerilog/otherTools
RUN wget -nc https://github.com/YosysHQ/oss-cad-suite-build/releases/download/2023-05-20/oss-cad-suite-linux-x64-20230520.tgz && \
    tar -xvzf oss-cad-suite-linux-x64-20230520.tgz

RUN wget https://github.com/diffblue/hw-cbmc/releases/download/ebmc-5.5/ebmc_5.5_amd64.deb && \
    apt-get install -y ./ebmc_5.5_amd64.deb

RUN wget https://github.com/cvc5/cvc5/releases/download/cvc5-1.2.0/cvc5-Linux-x86_64-static.zip && \
    unzip cvc5-Linux-x86_64-static.zip && \
    chmod +x cvc5-Linux-x86_64-static/bin/cvc5 && \
    mv cvc5-Linux-x86_64-static/bin/cvc5 /usr/local/bin/

RUN date && git clone https://github.com/guyguyguyguyguyguyguy/verilog_egraphs.git

RUN cd verilog_egraphs && \
    cd assert_egraph && \
    cargo build --release

ENV PATH="/workspace/smartVerilog/otherTools/verilog_egraphs/assert_egraph/target/release:$PATH"

WORKDIR /workspace/smartVerilog

CMD ["/bin/bash"]
