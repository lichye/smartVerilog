
FROM ubuntu:24.04

# Update Ubuntu Software repository
RUN apt update
RUN apt-get update && \
    apt-get install -y \
        cmake gcc g++ bison flex unzip wget make \
        python3 python3-pip python3-venv && \
    ln -sf python3 /usr/bin/python
RUN apt-get install nano -y
RUN apt-get install vim -y

RUN pip3 install --break-system-packages cocotb==1.8.0
RUN pip3 install --break-system-packages pytest

ARG DEBIAN_FRONTEND=noninteractive
RUN apt install -y git
RUN git clone https://github.com/SamueleGerminiani/harm.git
RUN apt-get install -y uuid-dev pkg-config
RUN apt install -y build-essential

RUN apt install -y wget unzip python3 python3-dev
ENV PYTHON=python3

#no cache from here
ARG CACHE_DATE=not_a_date

RUN wget -q https://github.com/Kitware/CMake/releases/download/v3.31.1/cmake-3.31.1-linux-x86_64.sh -O cmake.sh \
    && chmod +x cmake.sh \
    && ./cmake.sh --prefix=/usr/local --exclude-subdir --skip-license \
    && rm cmake.sh

WORKDIR "/harm/third_party"
RUN sh ./install_all.sh


WORKDIR "/harm"
RUN git reset --hard
RUN git pull
RUN mkdir build
WORKDIR "/harm/build"
RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN make -j16
ENV PATH="$PATH:/harm/build"




WORKDIR /workspace
RUN date && git clone --branch fmcad2025 https://github.com/lichye/smartVerilog.git

WORKDIR /workspace/smartVerilog/otherTools
RUN wget -nc https://github.com/YosysHQ/oss-cad-suite-build/releases/download/2023-05-20/oss-cad-suite-linux-x64-20230520.tgz && \
    tar -xvzf oss-cad-suite-linux-x64-20230520.tgz

RUN wget https://github.com/diffblue/hw-cbmc/releases/download/ebmc-5.5/ebmc_5.5_amd64.deb && \
    apt-get install -y ./ebmc_5.5_amd64.deb

RUN wget https://github.com/cvc5/cvc5/releases/download/cvc5-1.2.0/cvc5-Linux-x86_64-static.zip && \
unzip cvc5-Linux-x86_64-static.zip && \
chmod +x cvc5-Linux-x86_64-static/bin/cvc5 && \
mv cvc5-Linux-x86_64-static/bin/cvc5 /usr/local/bin/



WORKDIR /workspace/smartVerilog
RUN mkdir -p /workspace/smartVerilog/smart/user/

# run the command
CMD ["bash"]