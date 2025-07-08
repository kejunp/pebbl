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

#include "chunk.h"

#define STACK_MAX 1024

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  Chunk* chunk;
  uint8_t* ip;
  Value stack[STACK_MAX];
  Value* stack_top;
} VM;

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

/**
 * @brief Initializes the global VM (not a local one, so no pointer)
 */
void init_vm();

/**
 * @brief Frees the global VM (not a local one, so no pointer, see vm.c)
 */
void free_vm();

/**
 * @brief Pushes a value onto the VM stack.
 * @param value Value to push.
 */
void push(Value value);

/**
 * @brief Pops a value from the VM stack.
 * @return The value popped from the stack.
 */
Value pop(void);

#ifdef __cplusplus
}
#endif