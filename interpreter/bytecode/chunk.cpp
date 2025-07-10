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

#include "chunk.hpp"

#include <stdexcept>

size_t Chunk::add_constant(Value constant) {
  constants.push_back(constant);
  return constants.size() - 1;
}

void Chunk::write_constant(Value constant, size_t line) {
  size_t idx = add_constant(constant);

  // Write OP_CONSTANT or OP_CONSTANT_LONG depending on the index size
  if (idx <= 0xFF) {
    emit(static_cast<uint8_t>(Opcode::OP_CONSTANT), line);
    emit(static_cast<uint8_t>(idx), line);
  } else {
    emit(static_cast<uint8_t>(Opcode::OP_CONSTANT_LONG), line);
    emit(static_cast<uint8_t>((idx >> 16) & 0xFF), line);
    emit(static_cast<uint8_t>((idx >> 8) & 0xFF), line);
    emit(static_cast<uint8_t>(idx & 0xFF), line);
  }
}

size_t Chunk::get_line(size_t instruction) const {
  // Find the run corresponding to the instruction offset
  size_t run = 0;
  for (size_t i = 0; i < line_run_starts.size(); ++i) {
    if (line_run_starts[i] > instruction) break;
    run = i;
  }
  return line_run_lines.at(run);
}

void Chunk::emit(uint8_t byte, size_t line) {
  code.push_back(byte);

  // Start a new line run if this is the first line, or if the line changed
  if (line_run_lines.empty() || line_run_lines.back() != line) {
    line_run_starts.push_back(code.size() - 1);
    line_run_lines.push_back(line);
  }
}
