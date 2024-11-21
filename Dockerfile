# This Dockerfile configures a Docker environment that
# contains all the required packages for the tool
FROM ubuntu:22.04

USER root

# Install apt packages 
ADD install-pkgs.sh install-pkgs.sh
RUN bash install-pkgs.sh

CMD ["bash"]

# ADD Verilator 5.020
# ADD cocotb-.1.8.0
# ADD Python