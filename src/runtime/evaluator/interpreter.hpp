/**
 * @file interpreter.hpp
 * @brief Main interpreter/evaluator for executing PEBBL AST
 */

#pragma once

#include <memory>
#include <vector>

#include "ast.hpp"
#include "environment.hpp"
#include "gc.hpp"
#include "object.hpp"
#include "compiler.hpp"
#include "vm.hpp"

/**
 * @brief Runtime exception for interpreter errors
 */
class RuntimeError : public std::runtime_error {
public:
  RuntimeError(const std::string& message, const Token* token = nullptr) :
      std::runtime_error(message), token_(token) {
  }

  const Token* get_token() const {
    return token_;
  }

private:
  const Token* token_;
};

/**
 * @brief Main interpreter for executing PEBBL programs
 *
 * The Interpreter class evaluates AST nodes and executes PEBBL programs.
 * It manages environments for variable scoping and integrates with the
 * garbage collection system for memory management.
 */
class Interpreter {
public:
  /**
   * @brief Constructor
   * @param heap GC heap for object allocation
   * @param use_bytecode Whether to use bytecode interpreter (default: false for tree-walker)
   */
  explicit Interpreter(GCHeap& heap, bool use_bytecode = false);

  /**
   * @brief Execute a program
   * @param program The program AST node
   * @return The result of the last statement/expression
   */
  PEBBLObject execute(const ProgramNode& program);

  /**
   * @brief Evaluate an expression
   * @param expr The expression AST node
   * @return The value of the expression
   */
  PEBBLObject evaluate(const ExpressionNode& expr);

  /**
   * @brief Execute a statement
   * @param stmt The statement AST node
   * @return The result value (may be null for most statements)
   */
  PEBBLObject execute(const StatementNode& stmt);

  /**
   * @brief Convert a PEBBLObject to its string representation
   * @param value The value to stringify
   * @return String representation of the value
   */
  std::string stringify(PEBBLObject value);

  // GC root tracing
  void trace_roots(class Tracer& tracer);

  // Heap access for builtin functions
  GCHeap& get_heap() const {
    return heap_;
  }

  // Public error reporting for builtin functions
  void report_error(const std::string& message) {
    runtime_error(message);
  }
  
  /**
   * @brief Enable or disable bytecode execution mode
   * @param enable True to enable bytecode mode, false for tree-walking mode
   */
  void set_bytecode_mode(bool enable);
  
  /**
   * @brief Check if bytecode mode is enabled
   * @return True if using bytecode interpreter
   */
  bool is_bytecode_mode() const { return use_bytecode_; }

private:
  GCHeap& heap_;
  std::shared_ptr<Environment> global_env_;
  std::shared_ptr<Environment> current_env_;

  // Control flow flags
  bool has_return_ = false;
  PEBBLObject return_value_;
  
  // Bytecode execution components
  bool use_bytecode_;
  std::unique_ptr<Compiler> compiler_;
  std::unique_ptr<VM> vm_;

  // Expression evaluation methods
  PEBBLObject evaluate_binary(const BinaryExpressionNode& expr);
  PEBBLObject evaluate_unary(const UnaryExpressionNode& expr);
  PEBBLObject evaluate_literal(const LiteralNode& expr);
  PEBBLObject evaluate_identifier(const IdentifierNode& expr);
  PEBBLObject evaluate_assignment(const AssignmentExpressionNode& expr);
  PEBBLObject evaluate_if_else(const IfElseExpressionNode& expr);
  PEBBLObject evaluate_array_literal(const ArrayLiteralNode& expr);
  PEBBLObject evaluate_dict_literal(const DictLiteralNode& expr);
  PEBBLObject evaluate_call(const CallExpressionNode& expr);

  // Statement execution methods
  PEBBLObject execute_expression_statement(const ExpressionStatementNode& stmt);
  PEBBLObject execute_variable_statement(const VariableStatementNode& stmt);
  PEBBLObject execute_return_statement(const ReturnStatementNode& stmt);
  PEBBLObject execute_block_statement(const BlockStatementNode& stmt);
  PEBBLObject execute_while_statement(const WhileLoopStatementNode& stmt);
  PEBBLObject execute_for_statement(const ForLoopStatementNode& stmt);
  PEBBLObject execute_function_statement(const FunctionStatementNode& stmt);

  // Utility methods
  bool is_truthy(PEBBLObject value);
  bool are_equal(PEBBLObject left, PEBBLObject right);

  // Scope management
  void push_environment(std::shared_ptr<Environment> env);
  void pop_environment();

  // Helper method for tracing environment objects
  void trace_environment_objects(std::shared_ptr<Environment> env, class Tracer& tracer);

  // Error reporting
  void runtime_error(const std::string& message, const Token* token = nullptr);

  // Builtin function management
  void register_builtin_functions();
  
  // Bytecode integration helpers
  void sync_globals_to_vm();
  void sync_globals_from_vm();
};