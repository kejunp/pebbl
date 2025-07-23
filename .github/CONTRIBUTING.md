# Contributing to PEBBL

Thank you for your interest in contributing to PEBBL!  
This document outlines how to build, test, and contribute code to the project.

---

## Project Structure

- `interpreter/` — VM, bytecode executor, and runtime
- `parser/` — Tokenizer and parser for the language
- `compiler/` — (Planned or stubbed) source-to-bytecode translator
- `utils/` — Useful utilities such as debug.hpp/.cpp, and common.hpp
- `.github/` — GitHub Actions, issue templates, and PR templates

(All source code will be in the src/ directory)
---

## How to Build (Linux / macOS / Windows)

PEBBL uses **CMake** as its build system.

### Requirements

- CMake ≥ 3.20
- A C++20-compatible compiler:
  - **Linux/macOS**: GCC or Clang
  - **Windows**: MSVC (Visual Studio 2019 or later)

### Linux/macOS

```bash
git clone https://github.com/kejunp/pebbl.git
cd pebbl
mkdir build && cd build
cmake ..
make
```

### Windows
*Run from "Developer Command Prompt for Visual Studio*
```cmd
git clone https://github.com/kejunp/pebbl.git
cd pebbl
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Running the Language
After building, run the interpreter
```sh
./pebbli ../examples/hello.peb
```
(or .\pebbli.exe on Windows)

You can also run tests manually (if added):
```sh
ctest
```

## Code Style
we use clang-format for consistent formatting
Before committing:
```bash
clang-format -i path/to/file.cpp
```
> CI tests will fail if files are not properly formatted

### Language Standards
> C++20 minimum (Anything over C++20 is fine as long as it is compatible with C++20)

> [!TIP]
>
> ### Contribution Tips
> Keep PRs focused: small and isolated changes are easier to review.
> If proposing a new language feature, please include:
> - Motivation
> - Example syntax
> - Impact on parser/compiler/runtime