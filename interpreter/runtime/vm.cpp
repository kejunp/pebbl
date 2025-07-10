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

#include "vm.hpp"

VM::VM(Chunk& chunk_ref) : chunk(chunk_ref), instr_ptr(chunk_ref.code.begin()), stack_ptr(stack.begin()) {
}

void VM::push(const Value& value) {
  *stack_ptr = value;
  ++stack_ptr;
}

Value VM::pop() {
  --stack_ptr;
  return *stack_ptr;
}

std::size_t VM::stack_size() const {
  return static_cast<std::size_t>(stack_ptr - stack.begin());
}

const Chunk& VM::get_chunk() const {
  return chunk;
}

std::vector<uint8_t>::const_iterator VM::get_instr_ptr() const {
  return instr_ptr;
}