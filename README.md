# THE PEBBLE PROGRAMMING LANGUAGE
[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](LICENSE.txt)
[![Tests](https://github.com/kejunp/pebbl/actions/workflows/tests.yml/badge.svg)](https://github.com/kejunp/pebbl/actions/workflows/tests.yml)
[![Format Check](https://github.com/kejunp/pebbl/actions/workflows/clang-format.yml/badge.svg)](https://github.com/kejunp/pebbl/actions/workflows/clang-format.yml)
[![Generate Docs](https://github.com/kejunp/pebbl/actions/workflows/doxygen.yml/badge.svg)](https://github.com/kejunp/pebbl/actions/workflows/doxygen.yml)

PEBBL is a minimal hobby programming language

## HOW TO COMPILE

To compile, use the cmake CLI tool (you need to have **CMake** and **Ninja** installed on your machine, see instructions below):
```sh
mkdir -p build
cmake -S . -B build -G "Ninja"
cmake --build build
```
The executable will be pebbli in the build directory

## PREREQUISITES
> [!NOTE]
> You need to install Ninja and CMake on your system if you haven't already:  
> **For Windows**:  
> 1: go to [cmake.org](https://cmake.org) and download the installer.  
> 2: go to the [official Ninja releases page](https://github.com/ninja-build/ninja/releases) and download ```ninja-win.zip```  
> 3: Extract it somewhere (e.g., ```C:\Tools\ninja```)
> 4: Add that directory to your **PATH**:
> - Press ```Win + R```, type ```sysdm.cpl```, go to **Advanced → Environment Variables**
> - Edit ```Path```, add: ```C:\Tools\ninja``` (or wherever you extracted the .zip file)
> 5: Test it:
> ```ps1
> ninja --version
> ```
>  
> **For macOS**:
> ```zsh
> brew install ninja
> ninja --version
> ```
>  
> **For Ubuntu/Debian based Linux**:
> ```bash
> sudo apt install ninja-build
> ninja --version
> ```
