#include "debug.hpp"

#include <iomanip>
#include <iostream>
#include <string_view>

void disassemble_chunk(const Chunk& chunk, std::string_view name) {
  std::cout << "--- " << name << " ---\n";
  for (std::size_t offset = 0; offset < chunk.code.size();) {
    offset = disassemble_instruction(chunk, offset);
  }
}

std::size_t disassemble_instruction(const Chunk& chunk, std::size_t offset) {
  std::cout << std::setw(4) << offset << " ";

  // Print source line info
  if (offset > 0 && chunk.get_line(offset) == chunk.get_line(offset - 1))
    std::cout << "   | ";
  else
    std::cout << std::setw(4) << chunk.get_line(offset) << " ";

  auto opcode_byte = chunk.code[offset];
  auto mnemonic = opcode_to_string(opcode_byte);

  std::cout << mnemonic;

  switch (static_cast<Opcode>(opcode_byte)) {
    case Opcode::OP_CONSTANT: {
      uint8_t constant_idx = chunk.code[offset + 1];
      std::cout << " " << std::setw(4) << static_cast<int>(constant_idx) << " ";
      std::cout << "[";
      if (constant_idx < chunk.constants.size())
        std::cout << chunk.constants[constant_idx];
      else
        std::cout << "<out-of-range>";
      std::cout << "]";
      return offset + 2;
    }
    case Opcode::OP_CONSTANT_LONG: {
      // Read 3 bytes big-endian
      auto idx =
          (static_cast<std::size_t>(chunk.code[offset + 1]) << 16) |
          (static_cast<std::size_t>(chunk.code[offset + 2]) << 8) | static_cast<std::size_t>(chunk.code[offset + 3]);
      std::cout << " " << std::setw(4) << idx << " ";
      std::cout << "[";
      if (idx < chunk.constants.size())
        std::cout << chunk.constants[idx];
      else
        std::cout << "<out-of-range>";
      std::cout << "]";
      return offset + 4;
    }
    case Opcode::OP_NEGATE:
    case Opcode::OP_ADD:
    case Opcode::OP_SUBTRACT:
    case Opcode::OP_MULTIPLY:
    case Opcode::OP_DIVIDE:
    case Opcode::OP_RETURN:
      // Simple, 1-byte opcodes
      return offset + 1;
    default:
      std::cout << " (unknown opcode)";
      return offset + 1;
  }
  // Defensive fallback
  return offset + 1;
}