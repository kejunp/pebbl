# THE PEBBLE PROGRAMMING LANGUAGE
[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](LICENSE.txt)
[![Tests](https://github.com/kejunp/pebbl/actions/workflows/tests.yml/badge.svg)](https://github.com/kejunp/pebbl/actions/workflows/tests.yml)
[![Format Check](https://github.com/kejunp/pebbl/actions/workflows/clang-format.yml/badge.svg)](https://github.com/kejunp/pebbl/actions/workflows/clang-format.yml)
[![Generate Docs](https://github.com/kejunp/pebbl/actions/workflows/doxygen.yml/badge.svg)](https://github.com/kejunp/pebbl/actions/workflows/doxygen.yml)

PEBBL is a minimal hobby programming language

## HOW TO COMPILE

To compile, first create a build directory, like so:
```sh
mkdir -p build && cd build
```
Next, use the cmake command 

```sh
cmake ..
```

(If you haven't installed cmake,  
use  
```bash
sudo apt update
sudo apt install cmake
```
for Linux  
```zsh
brew install cmake
```
for macOS  
and download the installer from [cmake.org](https://cmake.org/) for Windows
)  

Then run the generated Makefile

```sh
make
```

> [!NOTE]
> If cmake outputs the ninja build system,  
> you need to use
>```sh
> ninja
>```
> Instead of
>```sh
> make
>```
