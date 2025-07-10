#pragma once

#include "chunk.hpp"
#include <string_view>

/**
 * @brief Disassembles opcodes and prints out their mnemonics.
 * @param chunk Reference to the chunk of opcodes to disassemble.
 * @param name Name of the chunk to disassemble (prints as "---name---").
 */
void disassemble_chunk(const Chunk& chunk, std::string_view name);

/**
 * @brief Disassembles a single instruction and prints its mnemonic.
 * @param chunk Reference to the chunk containing the instruction.
 * @param offset Offset of the instruction in the chunk (instruction at chunk.code[offset]).
 * @return The offset of the next instruction after the disassembled one.
 */
std::size_t disassemble_instruction(const Chunk& chunk, std::size_t offset);