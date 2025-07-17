FROM mcr.microsoft.com/devcontainers/base:ubuntu

# Install system dependencies
RUN apt-get update && apt-get install -y \
    clang-format \
    doxygen \
    graphviz \
    cmake \
    gcc \
    g++ \
    gdb \
    unzip \
    curl \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/* \
 && curl -LO https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-linux.zip \
 && unzip ninja-linux.zip \
 && mv ninja /usr/local/bin/ \
 && chmod +x /usr/local/bin/ninja