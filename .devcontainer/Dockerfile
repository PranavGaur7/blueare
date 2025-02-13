# Use an official base image (you can choose one depending on your needs)
FROM mcr.microsoft.com/vscode/devcontainers/base:ubuntu

# Install flex and bison
RUN apt-get update && \
    apt-get install -y flex bison && \
    apt-get clean
