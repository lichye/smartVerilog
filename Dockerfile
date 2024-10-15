# This Dockerfile configures a Docker environment that
# contains all the required packages for the tool
FROM ubuntu:22.04

USER root

# Install apt packages 
ADD install-pkgs.sh install-pkgs.sh
RUN bash install-pkgs.sh

CMD ["bash"]

# Install PyTorch and Torch-MLIR
RUN pip3 install --upgrade pip
RUN pip3 install --pre torch-mlir torchvision \
                 -f https://github.com/llvm/torch-mlir-release/releases/expanded_assets/dev-wheels \
                 --extra-index-url https://download.pytorch.org/whl/nightly/cpu

# Install pip packages 
ADD install-pips.sh install-pips.sh
RUN bash install-pips.sh

# Add environment variables
ARG VHLS_PATH
ARG VHLS_VERSION
ADD install-env.sh install-env.sh
RUN bash install-env.sh $VHLS_PATH $VHLS_VERSION