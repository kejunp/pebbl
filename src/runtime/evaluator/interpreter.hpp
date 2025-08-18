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

/**
 * @brief Runtime exception for interpreter errors
 */
class RuntimeError : public std::runtime_error {
public:
  RuntimeError(const std::string& message, const Token* token = nullptr)
    : std::runtime_error(message), token_(token) {}

  const Token* get_token() const { return token_; }

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
   */
  explicit Interpreter(GCHeap& heap);

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

private:
  GCHeap& heap_;
  std::shared_ptr<Environment> global_env_;
  std::shared_ptr<Environment> current_env_;
  
  // Control flow flags
  bool has_return_ = false;
  PEBBLObject return_value_;
  
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
  
  // Error reporting
  void runtime_error(const std::string& message, const Token* token = nullptr);
  
  // Builtin function management
  void register_builtin_functions();
};