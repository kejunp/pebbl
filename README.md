# THE PEBBL PROGRAMMING LANGUAGE
[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](LICENSE.txt)
[![Tests](https://github.com/kejunp/pebbl/actions/workflows/tests.yml/badge.svg)](https://github.com/kejunp/pebbl/actions/workflows/tests.yml)
[![Format Check](https://github.com/kejunp/pebbl/actions/workflows/clang-format.yml/badge.svg)](https://github.com/kejunp/pebbl/actions/workflows/clang-format.yml)
[![Generate Docs](https://github.com/kejunp/pebbl/actions/workflows/doxygen.yml/badge.svg)](https://github.com/kejunp/pebbl/actions/workflows/doxygen.yml)

PEBBL is a minimal hobby programming language

## HOW TO COMPILE

To compile, use the cmake CLI tool (you need to have **CMake** installed on your machine, see instructions below):
```sh
mkdir -p build
cmake -S . -B build
cmake --build build
```
The executable will be pebbli in the build directory

## PREREQUISITES
> [!NOTE]
> You need to install Ninja and CMake on your system if you haven't already:  
> **For Windows**:  
> 1: go to [cmake.org](https://cmake.org) and download the installer.
> 2: test with ```bash
> cmake --version
>```
>  
> **For macOS**:
> ```zsh
> brew install cmake
> cmake --version
> ```
>  
> **For Ubuntu/Debian based Linux**:
> ```bash
> sudo apt install cmake
> cmake --version
> ```

> [!TIP]
> This dev container auto installs claude as your coding assistant, if you do not like this, you can always install GitHub Copilot, Supermaven, etc.