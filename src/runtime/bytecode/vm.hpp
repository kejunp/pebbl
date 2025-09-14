/**
 * @file vm.hpp
 * @brief Virtual machine for executing PEBBL bytecode
 */

#pragma once

#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>

#include "bytecode.hpp"
#include "environment.hpp"
#include "gc.hpp"
#include "object.hpp"

// Forward declarations to avoid circular includes
class PEBBLFunction;
class PEBBLBuiltinFunction;

/**
 * @brief Call frame for function calls
 */
struct CallFrame {
  const Chunk* chunk;
  uint32_t instruction_pointer;
  uint32_t stack_base;  // Base of this frame's local variables on the stack

  CallFrame(const Chunk* c, uint32_t ip, uint32_t base) :
      chunk(c), instruction_pointer(ip), stack_base(base) {
  }
};

/**
 * @brief Virtual machine execution result
 */
enum class VMResult { OK, COMPILE_ERROR, RUNTIME_ERROR };

/**
 * @brief Virtual machine for executing bytecode
 */
class VM {
public:
  /**
   * @brief Constructor
   * @param heap GC heap for object allocation
   */
  explicit VM(GCHeap& heap);

  /**
   * @brief Execute a bytecode chunk
   * @param chunk The bytecode chunk to execute
   * @return Execution result
   */
  VMResult execute(const Chunk& chunk);

  /**
   * @brief Get the top value from the stack (result of execution)
   * @return Top stack value
   */
  PEBBLObject get_result() const;

  /**
   * @brief Reset the VM state
   */
  void reset();

  /**
   * @brief Check if the VM has a runtime error
   * @return True if there was a runtime error
   */
  bool has_error() const {
    return has_error_;
  }

  /**
   * @brief Get the last error message
   * @return Error message
   */
  const std::string& get_error() const {
    return error_message_;
  }

  /**
   * @brief Set a global variable (for builtin functions)
   * @param name Variable name
   * @param value Variable value
   */
  void set_global(const std::string& name, PEBBLObject value);

  /**
   * @brief Get a global variable (for builtin functions)
   * @param name Variable name
   * @return Variable value
   */
  PEBBLObject get_global(const std::string& name);

  /**
   * @brief Trace GC roots
   * @param tracer GC tracer
   */
  void trace_roots(class Tracer& tracer);

  /**
   * @brief Convert a value to its string representation
   * @param value The value to stringify
   * @return String representation
   */
  std::string stringify(PEBBLObject value);

private:
  GCHeap& heap_;
  std::vector<PEBBLObject> stack_;
  std::vector<CallFrame> frames_;
  std::shared_ptr<Environment> global_env_;
  std::shared_ptr<Environment> current_env_;

  // Error handling
  bool has_error_;
  std::string error_message_;

  // Constants for stack management
  static constexpr size_t STACK_MAX = 256;
  static constexpr size_t FRAMES_MAX = 64;

  // Execution methods
  VMResult run();

  // Stack manipulation
  void push(PEBBLObject value);
  PEBBLObject pop();
  PEBBLObject peek(uint32_t distance = 0);
  void reset_stack();

  // Instruction handlers
  void handle_load_const(uint32_t operand);
  void handle_load_null();
  void handle_load_true();
  void handle_load_false();
  void handle_load_var(uint32_t operand);
  void handle_store_var(uint32_t operand);
  void handle_define_var(uint32_t operand);
  void handle_add();
  void handle_subtract();
  void handle_multiply();
  void handle_divide();
  void handle_negate();
  void handle_equal();
  void handle_not_equal();
  void handle_less();
  void handle_greater();
  void handle_less_equal();
  void handle_greater_equal();
  void handle_not();
  void handle_and();
  void handle_or();
  void handle_jump(uint32_t operand);
  void handle_jump_if_false(uint32_t operand);
  void handle_jump_if_true(uint32_t operand);
  void handle_call(uint32_t argc);
  void handle_return();
  void handle_build_array(uint32_t count);
  void handle_build_dict(uint32_t count);
  void handle_pop();
  void handle_dup();

  // Utility methods
  bool is_truthy(PEBBLObject value);
  bool are_equal(PEBBLObject left, PEBBLObject right);
  CallFrame& current_frame();
  const Chunk& current_chunk();

  // Error reporting
  void runtime_error(const std::string& message);
  void runtime_error(const std::string& message, uint32_t instruction);

  // Environment management
  void push_environment(std::shared_ptr<Environment> env);
  void pop_environment();

  // Builtin function support
  PEBBLObject call_builtin_function(
      PEBBLBuiltinFunction* func, const std::vector<PEBBLObject>& args);

  // Function calling support
  bool call_function(PEBBLFunction* function, uint32_t argc);
  bool call_builtin(PEBBLBuiltinFunction* function, uint32_t argc);

  // Arithmetic operation helpers
  bool perform_numeric_operation(
      PEBBLObject left, PEBBLObject right, OpCode operation, PEBBLObject& result);
  bool perform_comparison_operation(
      PEBBLObject left, PEBBLObject right, OpCode operation, PEBBLObject& result);
};
