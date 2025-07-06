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

#include "chunk.h"
#include "memutils.h"

void init_chunk(Chunk* chunk) {
  chunk->capacity = 0;
  chunk->count = 0;
  chunk->code = NULL;
  chunk->line_run_starts = NULL;
  chunk->line_run_lines = NULL;
  chunk->line_run_count = 0;
  chunk->line_run_capacity = 0;
  init_value_array(&chunk->constants);
}

void free_chunk(Chunk* chunk) {
  FREE_ARRAY(chunk->code);
  FREE_ARRAY(chunk->line_run_starts);
  FREE_ARRAY(chunk->line_run_lines);
  init_chunk(chunk);
}

void emit(Chunk* chunk, uint8_t byte, size_t line) {
  if (chunk->capacity < chunk->count + 1) {
    size_t old_capacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(old_capacity);
    chunk->code = GROW_ARRAY(uint8_t, chunk->code, chunk->capacity);
  }
  chunk->code[chunk->count] = byte;
  if (chunk->line_run_count == 0 || chunk->line_run_lines[chunk->line_run_count - 1] != line) {
    if (chunk->line_run_capacity < chunk->line_run_count + 1) {
      int old_line_cap = chunk->line_run_capacity;
      chunk->line_run_capacity = GROW_CAPACITY(old_line_cap);
      chunk->line_run_starts = GROW_ARRAY(size_t, chunk->line_run_starts, chunk->line_run_capacity);
      chunk->line_run_lines = GROW_ARRAY(size_t, chunk->line_run_lines, chunk->line_run_capacity);
    }
    chunk->line_run_starts[chunk->line_run_count] = chunk->count;
    chunk->line_run_lines[chunk->line_run_count] = line;
    chunk->line_run_count++;
  }
  chunk->count++;
}

size_t add_constant(Chunk* chunk, Value constant) {
  write_value_array(&chunk->constants, constant);
  return chunk->constants.count - 1;
}

void write_constant(Chunk* chunk, Value constant, size_t line) {
  size_t constant_idx = add_constant(chunk, constant);
  if (constant_idx <= 0xFF) {
    emit(chunk, (uint8_t)constant_idx, line);
  } else {
    emit(chunk, OP_CONSTANT_LONG, line);
    emit(chunk, (constant_idx >> 16) & 0xFF, line);
    emit(chunk, (constant_idx >> 8) & 0xFF, line);
    emit(chunk, constant_idx & 0xFF, line);
  }
}

size_t get_line(Chunk* chunk, size_t instruction) {
  size_t run = 0;
  for (size_t i = 0; i < chunk->line_run_count; ++i) {
    if (chunk->line_run_starts[i] > instruction) break;
    run = i;
  }
  return chunk->line_run_lines[run];
}