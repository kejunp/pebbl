FROM mcr.microsoft.com/devcontainers/base:ubuntu

# Install system dependencies
RUN apt-get update \
    && apt-get install -y clang-format doxygen graphviz cmake gcc g++ gdb \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*