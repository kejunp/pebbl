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
  int* line_run_lines;       ///< Source line numbers for each run
  size_t line_run_count;     ///< Number of line runs
  size_t line_run_capacity;  ///< Allocated capacity for line runs

  ValueArray constants;
} Chunk;


#define OPCODE(name) OP_##name,
typedef enum {
  #include "opcodes.inc"
} OpCode;
#undef OPCODE

#ifdef __cplusplus
}
#endif
