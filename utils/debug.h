#pragma once

#include "chunk.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Disassembles opcodes and prints out their mnemonics
 * @param chunk Chunk of opcodes to disassemble
 * @param name Name of the chunk to disassemble (will print ---name---)
 */
void disassemble_chunk(Chunk* chunk, const char* name);

/**
 * @brief Disassemble a instruction and prints out their mnemonics
 * @param chunk Chunk where the instruction resides
 * @param offset Offset of the instruction in the chunk (instruction at chunk->code[offset])
 */
size_t disassemble_instruction(Chunk* chunk, size_t offset);

#ifdef __cplusplus
}
#endif