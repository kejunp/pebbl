/**
 * @file bytecode.cpp
 * @brief Implementation of bytecode utilities and debugging functions
 */

#include "bytecode.hpp"

#include <iomanip>
#include <sstream>

std::string opcode_to_string(OpCode opcode) {
  switch (opcode) {
    case OpCode::LOAD_CONST:
      return "LOAD_CONST";
    case OpCode::LOAD_NULL:
      return "LOAD_NULL";
    case OpCode::LOAD_TRUE:
      return "LOAD_TRUE";
    case OpCode::LOAD_FALSE:
      return "LOAD_FALSE";
    case OpCode::LOAD_VAR:
      return "LOAD_VAR";
    case OpCode::STORE_VAR:
      return "STORE_VAR";
    case OpCode::DEFINE_VAR:
      return "DEFINE_VAR";
    case OpCode::ADD:
      return "ADD";
    case OpCode::SUBTRACT:
      return "SUBTRACT";
    case OpCode::MULTIPLY:
      return "MULTIPLY";
    case OpCode::DIVIDE:
      return "DIVIDE";
    case OpCode::NEGATE:
      return "NEGATE";
    case OpCode::EQUAL:
      return "EQUAL";
    case OpCode::NOT_EQUAL:
      return "NOT_EQUAL";
    case OpCode::LESS:
      return "LESS";
    case OpCode::GREATER:
      return "GREATER";
    case OpCode::LESS_EQUAL:
      return "LESS_EQUAL";
    case OpCode::GREATER_EQUAL:
      return "GREATER_EQUAL";
    case OpCode::NOT:
      return "NOT";
    case OpCode::AND:
      return "AND";
    case OpCode::OR:
      return "OR";
    case OpCode::JUMP:
      return "JUMP";
    case OpCode::JUMP_IF_FALSE:
      return "JUMP_IF_FALSE";
    case OpCode::JUMP_IF_TRUE:
      return "JUMP_IF_TRUE";
    case OpCode::CALL:
      return "CALL";
    case OpCode::RETURN:
      return "RETURN";
    case OpCode::BUILD_ARRAY:
      return "BUILD_ARRAY";
    case OpCode::BUILD_DICT:
      return "BUILD_DICT";
    case OpCode::POP:
      return "POP";
    case OpCode::DUP:
      return "DUP";
    case OpCode::PUSH_ENV:
      return "PUSH_ENV";
    case OpCode::POP_ENV:
      return "POP_ENV";
    case OpCode::SETUP_LOOP:
      return "SETUP_LOOP";
    case OpCode::BREAK_LOOP:
      return "BREAK_LOOP";
    case OpCode::HALT:
      return "HALT";
    default:
      return "UNKNOWN";
  }
}

std::string disassemble_instruction(const Chunk& chunk, uint32_t offset) {
  std::stringstream ss;

  if (offset >= chunk.instructions.size()) {
    return "INVALID_OFFSET";
  }

  const Instruction& instr = chunk.instructions[offset];

  ss << std::setfill('0') << std::setw(4) << offset << " ";
  ss << std::left << std::setw(16) << opcode_to_string(instr.opcode);

  // Add operand information based on instruction type
  switch (instr.opcode) {
    case OpCode::LOAD_CONST:
      ss << " " << instr.operand;
      if (instr.operand < chunk.constants.size()) {
        // We'd need access to interpreter to stringify properly
        ss << " ; constant[" << instr.operand << "]";
      }
      break;

    case OpCode::LOAD_VAR:
    case OpCode::STORE_VAR:
    case OpCode::DEFINE_VAR:
      ss << " " << instr.operand;
      if (instr.operand < chunk.variable_names.size()) {
        ss << " ; '" << chunk.variable_names[instr.operand] << "'";
      }
      break;

    case OpCode::JUMP:
    case OpCode::JUMP_IF_FALSE:
    case OpCode::JUMP_IF_TRUE:
      ss << " " << instr.operand << " ; -> " << instr.operand;
      break;

    case OpCode::CALL:
    case OpCode::BUILD_ARRAY:
    case OpCode::BUILD_DICT:
      ss << " " << instr.operand << " ; argc=" << instr.operand;
      break;

    case OpCode::LOAD_NULL:
    case OpCode::LOAD_TRUE:
    case OpCode::LOAD_FALSE:
    case OpCode::ADD:
    case OpCode::SUBTRACT:
    case OpCode::MULTIPLY:
    case OpCode::DIVIDE:
    case OpCode::NEGATE:
    case OpCode::EQUAL:
    case OpCode::NOT_EQUAL:
    case OpCode::LESS:
    case OpCode::GREATER:
    case OpCode::LESS_EQUAL:
    case OpCode::GREATER_EQUAL:
    case OpCode::NOT:
    case OpCode::AND:
    case OpCode::OR:
    case OpCode::RETURN:
    case OpCode::POP:
    case OpCode::DUP:
    case OpCode::PUSH_ENV:
    case OpCode::POP_ENV:
    case OpCode::SETUP_LOOP:
    case OpCode::BREAK_LOOP:
    case OpCode::HALT:
      // No operand needed for these instructions
      break;

    default:
      if (instr.operand != 0) {
        ss << " " << instr.operand;
      }
      break;
  }

  return ss.str();
}

std::string disassemble_chunk(const Chunk& chunk) {
  std::stringstream ss;

  ss << "=== Bytecode Chunk ===\n";
  ss << "Instructions: " << chunk.instructions.size() << "\n";
  ss << "Constants: " << chunk.constants.size() << "\n";
  ss << "Variables: " << chunk.variable_names.size() << "\n";
  ss << "\n";

  // Disassemble constants
  if (!chunk.constants.size() == 0) {
    ss << "Constants:\n";
    for (size_t i = 0; i < chunk.constants.size(); ++i) {
      ss << "  [" << i << "] ; constant\n";  // Would need interpreter to stringify
    }
    ss << "\n";
  }

  // Disassemble variable names
  if (!chunk.variable_names.empty()) {
    ss << "Variables:\n";
    for (size_t i = 0; i < chunk.variable_names.size(); ++i) {
      ss << "  [" << i << "] '" << chunk.variable_names[i] << "'\n";
    }
    ss << "\n";
  }

  // Disassemble instructions
  ss << "Instructions:\n";
  for (uint32_t i = 0; i < chunk.instructions.size(); ++i) {
    ss << "  " << disassemble_instruction(chunk, i) << "\n";
  }

  return ss.str();
}
