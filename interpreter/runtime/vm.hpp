/*
   Copyright 2025 Kejun Pan

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*/

#pragma once

#include <array>
#include <cstddef>
#include <vector>

#include "chunk.hpp"

/**
 * @brief Maximum size of the VM value stack.
 */
constexpr std::size_t STACK_MAX = 1024;

/**
 * @brief Possible results from interpreting code.
 */
enum class InterpretResult { OK, COMPILE_ERROR, RUNTIME_ERROR };

/**
 * @brief Virtual Machine for interpreting bytecode in a Chunk.
 */
class VM {
  public:
  /**
   * @brief Constructs a VM operating on the given Chunk.
   * @param chunk_ref Reference to the bytecode chunk to execute.
   */
  explicit VM(Chunk& chunk_ref);

  // Rule of five, as copy is not allowed (references can't be rebound).
  VM(const VM&) = delete;
  VM& operator=(const VM&) = delete;
  VM(VM&&) = default;
  VM& operator=(VM&&) = default;
  ~VM() = default;

  /**
   * @brief Pushes a value onto the VM stack.
   * @param value The value to push.
   */
  void push(const Value& value);

  /**
   * @brief Pops a value from the VM stack.
   * @return The popped value.
   */
  Value pop();

  /**
   * @brief Gets the current stack size.
   */
  std::size_t stack_size() const;

  /**
   * @brief Gets a const reference to the chunk.
   */
  const Chunk& get_chunk() const;

  /**
   * @brief Gets the instruction pointer (iterator).
   */
  std::vector<uint8_t>::const_iterator get_instr_ptr() const;

private:
  Chunk& chunk;
  std::vector<uint8_t>::const_iterator instr_ptr;
  std::array<Value, STACK_MAX> stack;
  std::array<Value, STACK_MAX>::iterator stack_ptr;
};