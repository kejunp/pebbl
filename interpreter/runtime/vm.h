#pragma once

#include "chunk.h"

#define STACK_MAX 2048

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