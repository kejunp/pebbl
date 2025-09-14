/**
 * @file bytecode.hpp
 * @brief Bytecode instruction definitions and data structures for the PEBBL virtual machine
 */

#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include "object.hpp"


/**
 * @brief Bytecode operation codes
 */
enum class OpCode : uint8_t {
  // Constants and literals
  LOAD_CONST,      // Load constant from constant pool
  LOAD_NULL,       // Load null value
  LOAD_TRUE,       // Load true value
  LOAD_FALSE,      // Load false value
  
  // Variables
  LOAD_VAR,        // Load variable from environment
  STORE_VAR,       // Store variable to environment
  DEFINE_VAR,      // Define new variable
  
  // Arithmetic operations
  ADD,             // Binary addition
  SUBTRACT,        // Binary subtraction
  MULTIPLY,        // Binary multiplication
  DIVIDE,          // Binary division
  NEGATE,          // Unary negation
  
  // Comparison operations
  EQUAL,           // Equality comparison
  NOT_EQUAL,       // Inequality comparison
  LESS,            // Less than comparison
  GREATER,         // Greater than comparison
  LESS_EQUAL,      // Less than or equal comparison
  GREATER_EQUAL,   // Greater than or equal comparison
  
  // Logical operations
  NOT,             // Logical not
  AND,             // Logical and
  OR,              // Logical or
  
  // Control flow
  JUMP,            // Unconditional jump
  JUMP_IF_FALSE,   // Conditional jump if false
  JUMP_IF_TRUE,    // Conditional jump if true
  
  // Function calls
  CALL,            // Call function with n arguments
  RETURN,          // Return from function
  
  // Collections
  BUILD_ARRAY,     // Build array from n stack items
  BUILD_DICT,      // Build dictionary from n key-value pairs
  
  // Stack manipulation
  POP,             // Pop top value from stack
  DUP,             // Duplicate top value
  
  // Environment management
  PUSH_ENV,        // Push new environment scope
  POP_ENV,         // Pop environment scope
  
  // Loop support
  SETUP_LOOP,      // Setup loop context
  BREAK_LOOP,      // Break from loop
  
  // Special
  HALT,            // Stop execution
};

/**
 * @brief Single bytecode instruction
 */
struct Instruction {
  OpCode opcode;
  uint32_t operand;  // Can be used for indices, offsets, counts, etc.
  
  Instruction() : opcode(OpCode::HALT), operand(0) {}
  Instruction(OpCode op) : opcode(op), operand(0) {}
  Instruction(OpCode op, uint32_t arg) : opcode(op), operand(arg) {}
};

/**
 * @brief Variable information for compilation
 */
struct VariableInfo {
  std::string name;
  bool is_mutable;
  uint32_t index;  // Index in local variables or global environment
  
  VariableInfo(const std::string& n, bool mut, uint32_t idx) 
    : name(n), is_mutable(mut), index(idx) {}
};

/**
 * @brief Bytecode chunk containing instructions and constants
 */
class Chunk {
public:
  std::vector<Instruction> instructions;
  std::vector<PEBBLObject> constants;
  std::vector<std::string> variable_names;  // For debugging and variable lookup
  
  /**
   * @brief Add an instruction to the chunk
   */
  void add_instruction(OpCode opcode) {
    instructions.emplace_back(opcode);
  }
  
  /**
   * @brief Add an instruction with operand
   */
  void add_instruction(OpCode opcode, uint32_t operand) {
    instructions.emplace_back(opcode, operand);
  }
  
  /**
   * @brief Add a constant to the constant pool
   * @return Index of the constant in the pool
   */
  uint32_t add_constant(PEBBLObject constant) {
    constants.push_back(constant);
    return static_cast<uint32_t>(constants.size() - 1);
  }
  
  /**
   * @brief Add a variable name for debugging
   * @return Index of the variable name
   */
  uint32_t add_variable_name(const std::string& name) {
    variable_names.push_back(name);
    return static_cast<uint32_t>(variable_names.size() - 1);
  }
  
  /**
   * @brief Get current instruction count (for jump targets)
   */
  uint32_t get_instruction_count() const {
    return static_cast<uint32_t>(instructions.size());
  }
  
  /**
   * @brief Patch a jump instruction with the correct offset
   */
  void patch_jump(uint32_t instruction_index, uint32_t target) {
    if (instruction_index < instructions.size()) {
      instructions[instruction_index].operand = target;
    }
  }
  
  /**
   * @brief Clear all data
   */
  void clear() {
    instructions.clear();
    constants.clear();
    variable_names.clear();
  }
  
  /**
   * @brief Get instruction at index (for debugging)
   */
  const Instruction& get_instruction(uint32_t index) const {
    return instructions[index];
  }
  
  /**
   * @brief Get constant at index
   */
  const PEBBLObject& get_constant(uint32_t index) const {
    return constants[index];
  }
  
  /**
   * @brief Get variable name at index
   */
  const std::string& get_variable_name(uint32_t index) const {
    return variable_names[index];
  }
  
  /**
   * @brief Get total size in bytes (for memory usage estimation)
   */
  size_t size_bytes() const {
    return instructions.size() * sizeof(Instruction) +
           constants.size() * sizeof(PEBBLObject) +
           variable_names.size() * sizeof(std::string);
  }
};

/**
 * @brief Convert opcode to string for debugging
 */
std::string opcode_to_string(OpCode opcode);

/**
 * @brief Disassemble bytecode chunk for debugging
 */
std::string disassemble_chunk(const Chunk& chunk);

/**
 * @brief Disassemble single instruction for debugging
 */
std::string disassemble_instruction(const Chunk& chunk, uint32_t offset);

