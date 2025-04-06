# Use an official base image with Ubuntu
FROM mcr.microsoft.com/vscode/devcontainers/base:ubuntu

# Install essential tools for C/C++ development
RUN apt-get update && \
    apt-get install -y build-essential gdb cmake flex bison && \
    apt-get clean

RUN  chmod u+x run.sh