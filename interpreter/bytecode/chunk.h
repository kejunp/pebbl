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

#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  size_t count;
  size_t capacity;
  uint8_t* code;

  size_t* line_run_starts;   ///< Offsets in code[] where line runs start (array indices)
  size_t* line_run_lines;       ///< Source line numbers for each run
  size_t line_run_count;     ///< Number of line runs
  size_t line_run_capacity;  ///< Allocated capacity for line runs
  ValueArray constants;
} Chunk;

#define OPCODE(op) op,
typedef enum {
  DUMMY,                  // To shut up intelliSense about needing a identifier
  #include "opcodes.inc"
} OpCode;
#undef OPCODE

/**
 * @brief Initializes a Chunk struct
 * @param chunk Pointer to uninitialized chunk 
 */
void init_chunk(Chunk* chunk);

/**
 * @brief Deallocates any allocated memory and resets struct
 * @param chunk Chunk to be freed
 */
void free_chunk(Chunk* chunk);

/**
 * @brief Writes a opcode to a Chunk struct
 * @param chunk The chunk to write to
 * @param byte Opcode to write
 * @param line The line to write the opcode to (for debugging)
 */
void emit(Chunk* chunk, uint8_t byte, size_t line);

/**
 * @brief Adds a constant to the constant pool of a chunk
 * @param chunk The chunk to add the constant to
 * @param constant The constant to write
 * @return Index of the constant written
 */
size_t add_constant(Chunk* chunk, Value constant);

/**
 * @brief Writes the opcodes for a constant (does not just write to pool)
 * @param chunk Chunk to write the opcodes to
 * @param constant Constant to write
 * @param line The line in which the opcodes will be stored
 */
void write_constant(Chunk* chunk, Value constant, size_t line);

/**
 * @brief Finds the line of a given instruction offset
 * @param chunk The chunk in which to search
 * @param instruction The offset of the instruction
 * @return The line number
 */
size_t get_line(Chunk* chunk, size_t instruction);

#ifdef __cplusplus
}
#endif
