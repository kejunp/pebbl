#pragma  once

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
        case Opcode::OP_CONSTANT:       return "OP_CONSTANT";
        case Opcode::OP_CONSTANT_LONG:  return "OP_CONSTANT_LONG";
        case Opcode::OP_NEGATE:         return "OP_NEGATE";
        case Opcode::OP_ADD:            return "OP_ADD";
        case Opcode::OP_SUBTRACT:       return "OP_SUBTRACT";
        case Opcode::OP_MULTIPLY:       return "OP_MULTIPLY";
        case Opcode::OP_DIVIDE:         return "OP_DIVIDE";
        case Opcode::OP_RETURN:         return "OP_RETURN";
        default:                        return "UNKNOWN";
    }
}