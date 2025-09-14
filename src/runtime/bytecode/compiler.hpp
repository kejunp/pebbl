/**
 * @file compiler.hpp
 * @brief AST to bytecode compiler for the PEBBL language
 */

#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <stack>
#include "bytecode.hpp"
#include "ast.hpp"
#include "gc.hpp"


/**
 * @brief Scope type for compilation
 */
enum class ScopeType {
  GLOBAL,
  FUNCTION,
  BLOCK,
  LOOP
};

/**
 * @brief Compilation scope information
 */
struct CompilationScope {
  ScopeType type;
  std::unordered_map<std::string, VariableInfo> variables;
  uint32_t variable_count;
  uint32_t loop_start;     // For loop scopes
  uint32_t loop_exit;      // For loop scopes
  
  CompilationScope(ScopeType t) : type(t), variable_count(0), loop_start(0), loop_exit(0) {}
};

/**
 * @brief Compiler for converting AST to bytecode
 */
class Compiler {
public:
  /**
   * @brief Constructor
   * @param heap GC heap for allocating string constants
   */
  explicit Compiler(GCHeap& heap);
  
  /**
   * @brief Compile a program AST to bytecode
   * @param program The program AST node
   * @return Compiled bytecode chunk
   */
  std::unique_ptr<Chunk> compile(const ProgramNode& program);
  
  /**
   * @brief Compile a single expression (for REPL or testing)
   * @param expr The expression AST node
   * @return Compiled bytecode chunk
   */
  std::unique_ptr<Chunk> compile_expression(const ExpressionNode& expr);

private:
  GCHeap& heap_;
  std::unique_ptr<Chunk> current_chunk_;
  std::stack<CompilationScope> scope_stack_;
  bool has_error_;
  std::string error_message_;
  
  // Compilation methods for statements
  void compile_statement(const StatementNode& stmt);
  void compile_expression_statement(const ExpressionStatementNode& stmt);
  void compile_variable_statement(const VariableStatementNode& stmt);
  void compile_return_statement(const ReturnStatementNode& stmt);
  void compile_block_statement(const BlockStatementNode& stmt);
  void compile_while_statement(const WhileLoopStatementNode& stmt);
  void compile_for_statement(const ForLoopStatementNode& stmt);
  void compile_function_statement(const FunctionStatementNode& stmt);
  
  // Compilation methods for expressions
  void compile_expression(const ExpressionNode& expr);
  void compile_literal(const LiteralNode& expr);
  void compile_identifier(const IdentifierNode& expr);
  void compile_binary_expression(const BinaryExpressionNode& expr);
  void compile_unary_expression(const UnaryExpressionNode& expr);
  void compile_assignment_expression(const AssignmentExpressionNode& expr);
  void compile_if_else_expression(const IfElseExpressionNode& expr);
  void compile_array_literal(const ArrayLiteralNode& expr);
  void compile_dict_literal(const DictLiteralNode& expr);
  void compile_call_expression(const CallExpressionNode& expr);
  
  // Utility methods
  void emit_instruction(OpCode opcode);
  void emit_instruction(OpCode opcode, uint32_t operand);
  uint32_t emit_jump(OpCode opcode);
  void patch_jump(uint32_t instruction_index);
  uint32_t add_constant(PEBBLObject constant);
  
  // Scope management
  void push_scope(ScopeType type);
  void pop_scope();
  CompilationScope& current_scope();
  
  // Variable management
  uint32_t resolve_variable(const std::string& name);
  uint32_t define_variable(const std::string& name, bool is_mutable);
  bool is_global_scope() const;
  
  // Error handling
  void error(const std::string& message);
  void error(const std::string& message, const Token* token);
  
  // Jump management for loops
  void emit_loop_start();
  void emit_loop_end();
  uint32_t get_loop_start() const;
  uint32_t get_loop_exit() const;
  
  // Helper for binary operators
  OpCode binary_op_to_opcode(TokenType token_type);
  OpCode unary_op_to_opcode(TokenType token_type);
  
  // Constants management
  uint32_t add_string_constant(const std::string& value);
  uint32_t add_number_constant(int32_t value);
  uint32_t add_number_constant(double value);
};

