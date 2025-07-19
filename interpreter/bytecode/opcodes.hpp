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

#include "common.hpp"

enum class Opcode : uint8_t {
  OP_CONSTANT = 1,
  OP_CONSTANT_LONG,
  OP_NEGATE,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_RETURN
};

template <typename T>
concept IsOpcode = std::same_as<T, uint8_t>;

inline std::string_view opcode_to_string(uint8_t opcode) {
  switch (static_cast<Opcode>(opcode)) {
    case Opcode::OP_CONSTANT:
      return "OP_CONSTANT";
    case Opcode::OP_CONSTANT_LONG:
      return "OP_CONSTANT_LONG";
    case Opcode::OP_NEGATE:
      return "OP_NEGATE";
    case Opcode::OP_ADD:
      return "OP_ADD";
    case Opcode::OP_SUBTRACT:
      return "OP_SUBTRACT";
    case Opcode::OP_MULTIPLY:
      return "OP_MULTIPLY";
    case Opcode::OP_DIVIDE:
      return "OP_DIVIDE";
    case Opcode::OP_RETURN:
      return "OP_RETURN";
    default:
      return "UNKNOWN";
  }
}