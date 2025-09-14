/**
 * @file compiler.cpp
 * @brief Implementation of the AST to bytecode compiler
 */

#include "compiler.hpp"

#include <iostream>
#include <stdexcept>

#include "object.hpp"

Compiler::Compiler(GCHeap& heap) : heap_(heap), has_error_(false) {
}

std::unique_ptr<Chunk> Compiler::compile(const ProgramNode& program) {
  current_chunk_ = std::make_unique<Chunk>();
  has_error_ = false;

  // Clear scope stack and push global scope
  while (!scope_stack_.empty()) {
    scope_stack_.pop();
  }
  push_scope(ScopeType::GLOBAL);

  // Compile all statements
  for (const auto& statement : program.statements) {
    compile_statement(*statement);
    if (has_error_) {
      return nullptr;
    }
  }

  // Add halt instruction at the end
  emit_instruction(OpCode::HALT);

  pop_scope();
  return std::move(current_chunk_);
}

std::unique_ptr<Chunk> Compiler::compile_expression(const ExpressionNode& expr) {
  current_chunk_ = std::make_unique<Chunk>();
  has_error_ = false;

  while (!scope_stack_.empty()) {
    scope_stack_.pop();
  }
  push_scope(ScopeType::GLOBAL);

  compile_expression(expr);

  if (!has_error_) {
    emit_instruction(OpCode::HALT);
  }

  pop_scope();

  if (has_error_) {
    return nullptr;
  }

  return std::move(current_chunk_);
}

void Compiler::compile_statement(const StatementNode& stmt) {
  switch (stmt.type()) {
    case ASTType::EXPRESSION_STATEMENT:
      compile_expression_statement(static_cast<const ExpressionStatementNode&>(stmt));
      break;
    case ASTType::VARIABLE_STATEMENT:
      compile_variable_statement(static_cast<const VariableStatementNode&>(stmt));
      break;
    case ASTType::RETURN_STATEMENT:
      compile_return_statement(static_cast<const ReturnStatementNode&>(stmt));
      break;
    case ASTType::BLOCK_STATEMENT:
      compile_block_statement(static_cast<const BlockStatementNode&>(stmt));
      break;
    case ASTType::WHILE_LOOP_STATEMENT:
      compile_while_statement(static_cast<const WhileLoopStatementNode&>(stmt));
      break;
    case ASTType::FOR_LOOP_STATEMENT:
      compile_for_statement(static_cast<const ForLoopStatementNode&>(stmt));
      break;
    case ASTType::FUNCTION_STATEMENT:
      compile_function_statement(static_cast<const FunctionStatementNode&>(stmt));
      break;
    default:
      error("Unknown statement type", stmt.get_token());
  }
}

void Compiler::compile_expression_statement(const ExpressionStatementNode& stmt) {
  compile_expression(*stmt.expression);
  // Expression statements need to pop their result if not at global scope
  if (!is_global_scope()) {
    emit_instruction(OpCode::POP);
  }
}

void Compiler::compile_variable_statement(const VariableStatementNode& stmt) {
  // Compile the initializer expression
  compile_expression(*stmt.value);

  // Define the variable
  uint32_t var_index = define_variable(stmt.name->name, stmt.is_mutable());
  emit_instruction(OpCode::DEFINE_VAR, var_index);
}

void Compiler::compile_return_statement(const ReturnStatementNode& stmt) {
  if (stmt.return_value) {
    compile_expression(*stmt.return_value);
  } else {
    emit_instruction(OpCode::LOAD_NULL);
  }
  emit_instruction(OpCode::RETURN);
}

void Compiler::compile_block_statement(const BlockStatementNode& stmt) {
  push_scope(ScopeType::BLOCK);

  for (const auto& statement : stmt.statements) {
    compile_statement(*statement);
    if (has_error_) break;
  }

  pop_scope();
}

void Compiler::compile_while_statement(const WhileLoopStatementNode& stmt) {
  push_scope(ScopeType::LOOP);

  uint32_t loop_start = current_chunk_->get_instruction_count();
  current_scope().loop_start = loop_start;

  // Compile condition
  compile_expression(*stmt.condition);

  // Jump if false (to end of loop)
  uint32_t exit_jump = emit_jump(OpCode::JUMP_IF_FALSE);
  current_scope().loop_exit = exit_jump;

  // Compile loop body
  compile_statement(*stmt.block);

  // Jump back to condition
  emit_instruction(OpCode::JUMP, loop_start);

  // Patch the exit jump
  patch_jump(exit_jump);

  pop_scope();
}

void Compiler::compile_for_statement(const ForLoopStatementNode& stmt) {
  push_scope(ScopeType::LOOP);

  // Compile the iterable expression
  compile_expression(*stmt.iterable);

  // For now, we'll implement a simplified version that assumes array iteration
  // This would need to be extended for different iterable types

  // TODO: Implement proper for-loop bytecode generation
  // This is a complex operation that would require:
  // 1. Runtime type checking of the iterable
  // 2. Iterator setup
  // 3. Loop condition checking
  // 4. Variable binding for each iteration

  error("For loops not yet implemented in bytecode compiler", stmt.get_token());

  pop_scope();
}

void Compiler::compile_function_statement(const FunctionStatementNode& stmt) {
  // For now, we'll store function definitions as constants
  // This is simplified - a full implementation would need proper closure handling

  // Create a new chunk for the function body
  auto function_chunk = std::make_unique<Chunk>();
  auto saved_chunk = std::move(current_chunk_);
  current_chunk_ = std::move(function_chunk);

  push_scope(ScopeType::FUNCTION);

  // Define parameters as local variables
  for (const auto& param : stmt.parameters) {
    define_variable(param->name, true);
  }

  // Compile function body
  compile_statement(*stmt.body);

  // Ensure function returns null if no explicit return
  emit_instruction(OpCode::LOAD_NULL);
  emit_instruction(OpCode::RETURN);

  pop_scope();

  // TODO: Store the compiled function chunk as a constant
  // This would require extending the object system to handle compiled functions

  current_chunk_ = std::move(saved_chunk);

  error("Function definitions not yet fully implemented in bytecode compiler", stmt.get_token());
}

void Compiler::compile_expression(const ExpressionNode& expr) {
  switch (expr.type()) {
    case ASTType::INTEGER_LITERAL:
    case ASTType::FLOAT_LITERAL:
    case ASTType::STRING_LITERAL:
    case ASTType::BOOLEAN_LITERAL:
      compile_literal(static_cast<const LiteralNode&>(expr));
      break;
    case ASTType::IDENTIFIER:
      compile_identifier(static_cast<const IdentifierNode&>(expr));
      break;
    case ASTType::BINARY_EXPRESSION:
      compile_binary_expression(static_cast<const BinaryExpressionNode&>(expr));
      break;
    case ASTType::UNARY_EXPRESSION:
      compile_unary_expression(static_cast<const UnaryExpressionNode&>(expr));
      break;
    case ASTType::ASSIGNMENT_EXPRESSION:
      compile_assignment_expression(static_cast<const AssignmentExpressionNode&>(expr));
      break;
    case ASTType::IF_ELSE_EXPRESSION:
      compile_if_else_expression(static_cast<const IfElseExpressionNode&>(expr));
      break;
    case ASTType::ARRAY_LITERAL:
      compile_array_literal(static_cast<const ArrayLiteralNode&>(expr));
      break;
    case ASTType::DICT_LITERAL:
      compile_dict_literal(static_cast<const DictLiteralNode&>(expr));
      break;
    case ASTType::CALL_EXPRESSION:
      compile_call_expression(static_cast<const CallExpressionNode&>(expr));
      break;
    default:
      error("Unknown expression type", expr.get_token());
  }
}

void Compiler::compile_literal(const LiteralNode& expr) {
  switch (expr.type()) {
    case ASTType::INTEGER_LITERAL: {
      const auto& int_literal = static_cast<const IntegerLiteralNode&>(expr);
      int32_t value = static_cast<int32_t>(int_literal.value);
      uint32_t const_index = add_number_constant(value);
      emit_instruction(OpCode::LOAD_CONST, const_index);
      break;
    }
    case ASTType::FLOAT_LITERAL: {
      const auto& float_literal = static_cast<const FloatLiteralNode&>(expr);
      uint32_t const_index = add_number_constant(float_literal.value);
      emit_instruction(OpCode::LOAD_CONST, const_index);
      break;
    }
    case ASTType::STRING_LITERAL: {
      const auto& string_literal = static_cast<const StringLiteralNode&>(expr);
      uint32_t const_index = add_string_constant(string_literal.value);
      emit_instruction(OpCode::LOAD_CONST, const_index);
      break;
    }
    case ASTType::BOOLEAN_LITERAL: {
      const auto& bool_literal = static_cast<const BooleanLiteralNode&>(expr);
      if (bool_literal.value) {
        emit_instruction(OpCode::LOAD_TRUE);
      } else {
        emit_instruction(OpCode::LOAD_FALSE);
      }
      break;
    }
    default:
      error("Invalid literal type", expr.get_token());
  }
}

void Compiler::compile_identifier(const IdentifierNode& expr) {
  uint32_t var_index = resolve_variable(expr.name);
  emit_instruction(OpCode::LOAD_VAR, var_index);
}

void Compiler::compile_binary_expression(const BinaryExpressionNode& expr) {
  // Compile operands (left first, then right for stack order)
  compile_expression(*expr.left);
  compile_expression(*expr.right);

  // Emit the appropriate operation
  OpCode opcode = binary_op_to_opcode(expr.operator_token.type);
  if (opcode != OpCode::HALT) {  // HALT is used as "invalid" opcode
    emit_instruction(opcode);
  } else {
    error("Unsupported binary operator", &expr.operator_token);
  }
}

void Compiler::compile_unary_expression(const UnaryExpressionNode& expr) {
  // Compile operand
  compile_expression(*expr.operand);

  // Emit the appropriate operation
  OpCode opcode = unary_op_to_opcode(expr.operator_token.type);
  if (opcode != OpCode::HALT) {
    emit_instruction(opcode);
  } else {
    error("Unsupported unary operator", &expr.operator_token);
  }
}

void Compiler::compile_assignment_expression(const AssignmentExpressionNode& expr) {
  // Compile the value
  compile_expression(*expr.value);

  // Handle assignment target
  if (expr.target->type() == ASTType::IDENTIFIER) {
    const auto& identifier = static_cast<const IdentifierNode&>(*expr.target);
    uint32_t var_index = resolve_variable(identifier.name);
    emit_instruction(OpCode::STORE_VAR, var_index);
    // Assignment expressions also leave the value on the stack
    emit_instruction(OpCode::DUP);
  } else {
    error("Invalid assignment target", expr.get_token());
  }
}

void Compiler::compile_if_else_expression(const IfElseExpressionNode& expr) {
  // Compile condition
  compile_expression(*expr.condition);

  // Jump if false to else branch
  uint32_t else_jump = emit_jump(OpCode::JUMP_IF_FALSE);

  // Compile then branch
  compile_expression(*expr.then_expression);

  if (expr.else_expression) {
    // Jump over else branch
    uint32_t end_jump = emit_jump(OpCode::JUMP);

    // Patch else jump
    patch_jump(else_jump);

    // Compile else branch
    compile_expression(*expr.else_expression);

    // Patch end jump
    patch_jump(end_jump);
  } else {
    // No else branch, just patch the jump and push null
    patch_jump(else_jump);
    emit_instruction(OpCode::LOAD_NULL);
  }
}

void Compiler::compile_array_literal(const ArrayLiteralNode& expr) {
  // Compile all elements
  for (const auto& element : expr.elements) {
    compile_expression(*element);
  }

  // Build array with the number of elements
  emit_instruction(OpCode::BUILD_ARRAY, static_cast<uint32_t>(expr.elements.size()));
}

void Compiler::compile_dict_literal(const DictLiteralNode& expr) {
  // Compile all key-value pairs
  for (const auto& [key_ptr, value_ptr] : expr.entries) {
    compile_expression(*key_ptr);    // Key
    compile_expression(*value_ptr);  // Value
  }

  // Build dictionary with the number of pairs
  emit_instruction(OpCode::BUILD_DICT, static_cast<uint32_t>(expr.entries.size()));
}

void Compiler::compile_call_expression(const CallExpressionNode& expr) {
  // Compile function expression
  compile_expression(*expr.function);

  // Compile arguments
  for (const auto& arg : expr.arguments) {
    compile_expression(*arg);
  }

  // Emit call instruction with argument count
  emit_instruction(OpCode::CALL, static_cast<uint32_t>(expr.arguments.size()));
}

void Compiler::emit_instruction(OpCode opcode) {
  current_chunk_->add_instruction(opcode);
}

void Compiler::emit_instruction(OpCode opcode, uint32_t operand) {
  current_chunk_->add_instruction(opcode, operand);
}

uint32_t Compiler::emit_jump(OpCode opcode) {
  uint32_t instruction_index = current_chunk_->get_instruction_count();
  current_chunk_->add_instruction(opcode, 0);  // Placeholder operand
  return instruction_index;
}

void Compiler::patch_jump(uint32_t instruction_index) {
  uint32_t jump_target = current_chunk_->get_instruction_count();
  current_chunk_->patch_jump(instruction_index, jump_target);
}

uint32_t Compiler::add_constant(PEBBLObject constant) {
  return current_chunk_->add_constant(constant);
}

void Compiler::push_scope(ScopeType type) {
  scope_stack_.emplace(type);
}

void Compiler::pop_scope() {
  if (!scope_stack_.empty()) {
    scope_stack_.pop();
  }
}

CompilationScope& Compiler::current_scope() {
  if (scope_stack_.empty()) {
    throw std::runtime_error("No active compilation scope");
  }
  return scope_stack_.top();
}

uint32_t Compiler::resolve_variable(const std::string& name) {
  // Look up variable in current scope chain
  auto& scope = current_scope();
  auto it = scope.variables.find(name);
  if (it != scope.variables.end()) {
    return it->second.index;
  }

  // For simplicity, return a name index for global lookup
  return current_chunk_->add_variable_name(name);
}

uint32_t Compiler::define_variable(const std::string& name, bool is_mutable) {
  auto& scope = current_scope();
  uint32_t index = scope.variable_count++;
  scope.variables[name] = VariableInfo(name, is_mutable, index);
  return current_chunk_->add_variable_name(name);
}

bool Compiler::is_global_scope() const {
  return !scope_stack_.empty() && scope_stack_.top().type == ScopeType::GLOBAL;
}

void Compiler::error(const std::string& message) {
  has_error_ = true;
  error_message_ = message;
  std::cerr << "Compilation Error: " << message << std::endl;
}

void Compiler::error(const std::string& message, const Token* token) {
  has_error_ = true;
  error_message_ = message;
  std::cerr << "Compilation Error";
  if (token) {
    std::cerr << " at line " << token->line;
  }
  std::cerr << ": " << message << std::endl;
}

OpCode Compiler::binary_op_to_opcode(TokenType token_type) {
  switch (token_type) {
    case TokenType::PLUS:
      return OpCode::ADD;
    case TokenType::MINUS:
      return OpCode::SUBTRACT;
    case TokenType::ASTERISK:
      return OpCode::MULTIPLY;
    case TokenType::SLASH:
      return OpCode::DIVIDE;
    case TokenType::EQUAL:
      return OpCode::EQUAL;
    case TokenType::NOT_EQUAL:
      return OpCode::NOT_EQUAL;
    case TokenType::LESS:
      return OpCode::LESS;
    case TokenType::GREATER:
      return OpCode::GREATER;
    case TokenType::LESS_EQUAL:
      return OpCode::LESS_EQUAL;
    case TokenType::GREATER_EQUAL:
      return OpCode::GREATER_EQUAL;
    case TokenType::AND:
      return OpCode::AND;
    case TokenType::OR:
      return OpCode::OR;
    default:
      return OpCode::HALT;  // Invalid
  }
}

OpCode Compiler::unary_op_to_opcode(TokenType token_type) {
  switch (token_type) {
    case TokenType::MINUS:
      return OpCode::NEGATE;
    case TokenType::BANG:
      return OpCode::NOT;
    default:
      return OpCode::HALT;  // Invalid
  }
}

uint32_t Compiler::add_string_constant(const std::string& value) {
  auto* str_obj = heap_.allocate<PEBBLString>(value);
  return add_constant(PEBBLObject::make_gc_ptr(str_obj));
}

uint32_t Compiler::add_number_constant(int32_t value) {
  return add_constant(PEBBLObject::make_int32(value));
}

uint32_t Compiler::add_number_constant(double value) {
  return add_constant(PEBBLObject::make_double(value));
}
