FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        clang \
        clangd \
        clang-format \
        clang-tidy \
        cmake \
        gdb \
        git \
        lldb \
        ninja-build \
        pkg-config \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

ENV CC=clang \
    CXX=clang++

