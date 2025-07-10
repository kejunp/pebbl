/*
   Copyright 2025 Kejun Pan

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#include "value.hpp"
#include <vector>
#include <concepts>
#include "opcodes.hpp"

class Chunk {
private:

  std::vector<size_t> line_run_starts;
  std::vector<size_t> line_run_lines;

public:
  std::vector<uint8_t> code;

  ValueArray constants;

  Chunk() = default;

  ~Chunk() = default;

  /**
   * @brief Adds a constant to the constant pool of this (Chunk class)
   * @param constant The constant to write
   * @return Index of the constant written
   */
  size_t add_constant(Value constant);

  /**
   * @brief Writes the opcodes for a constant (does not just write to pool)
   * @param constant Constant to write
   * @param line The line in which the opcodes will be stored
   */
  void write_constant(Value constant, size_t line);

  /**
   * @brief Finds the line of a given instruction offset
   * @param instruction The offset of the instruction
   * @return The line number
   */
  size_t get_line(size_t instruction) const;

  /**
   * @brief Writes a opcode this (Chunk class)
   * @param byte Opcode to write
   * @param line The line to write the opcode to (for debugging)
   */
  void emit(uint8_t byte, size_t line);

  /**
   * @brief Writes many opcodes to this (Chunk class)
   * @param bytes Bytes to write to the code
   * @param line Line to emit to
   */
  template <IsOpcode... Bytes>
  inline void emit_many(Bytes... bytes, size_t line) {
    (emit(bytes, line), ...);
  }
};