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

#include "ast_generator.hpp"

#include <iostream>
#include <sstream>

#include "../../common.hpp"

ASTGenerator::ASTGenerator(Lexer& lexer) : lexer_(lexer) {
  advance_token();
  advance_token();
}

std::unique_ptr<ProgramNode> ASTGenerator::parse_program() {
  auto program = std::make_unique<ProgramNode>();

  while (current_token_.type != TokenType::EOF_TYPE) {
    if (auto stmt = parse_statement()) {
      program->statements.push_back(std::move(stmt));
    } else {
      advance_token();
    }
  }

  return program;
}

void ASTGenerator::advance_token() {
  current_token_ = peek_token_;
  peek_token_ = lexer_.next_token();
}

bool ASTGenerator::check_token(TokenType type) const {
  return current_token_.type == type;
}

bool ASTGenerator::consume_token(TokenType type, const std::string& error_message) {
  if (check_token(type)) {
    advance_token();
    return true;
  }
  report_error(error_message);
  return false;
}

void ASTGenerator::report_error(const std::string& message) const {
  std::cerr << "Parse error at line " << current_token_.line << ": " << message << " (got '"
            << current_token_.lexeme << "')" << std::endl;
}

std::unique_ptr<StatementNode> ASTGenerator::parse_statement() {
  switch (current_token_.type) {
    case TokenType::LET:
    case TokenType::VAR:
      return parse_variable_statement();
    case TokenType::RETURN:
      return parse_return_statement();
    case TokenType::LBRACE:
      return parse_block_statement();
    case TokenType::WHILE:
      return parse_while_statement();
    case TokenType::FOR:
      return parse_for_statement();
    default:
      return parse_expression_statement();
  }
}

std::unique_ptr<VariableStatementNode> ASTGenerator::parse_variable_statement() {
  auto stmt = std::make_unique<VariableStatementNode>();
  stmt->token = current_token_;

  advance_token();

  if (!check_token(TokenType::IDENTIFIER)) {
    report_error("Expected identifier after variable declaration");
    return nullptr;
  }

  stmt->name = parse_identifier();

  if (!consume_token(TokenType::ASSIGN, "Expected '=' after variable name")) {
    return nullptr;
  }

  stmt->value = parse_expression();

  consume_token(TokenType::SEMICOLON, "Expected ';' after variable declaration");

  return stmt;
}

std::unique_ptr<ReturnStatementNode> ASTGenerator::parse_return_statement() {
  auto stmt = std::make_unique<ReturnStatementNode>();
  stmt->token = current_token_;

  advance_token();

  if (!check_token(TokenType::SEMICOLON)) {
    stmt->return_value = parse_expression();
  }

  consume_token(TokenType::SEMICOLON, "Expected ';' after return statement");

  return stmt;
}

std::unique_ptr<BlockStatementNode> ASTGenerator::parse_block_statement() {
  auto stmt = std::make_unique<BlockStatementNode>();

  if (!consume_token(TokenType::LBRACE, "Expected '{'")) {
    return nullptr;
  }

  while (!check_token(TokenType::RBRACE) && !check_token(TokenType::EOF_TYPE)) {
    if (auto inner_stmt = parse_statement()) {
      stmt->statements.push_back(std::move(inner_stmt));
    } else {
      advance_token();
    }
  }

  consume_token(TokenType::RBRACE, "Expected '}'");

  return stmt;
}

std::unique_ptr<WhileLoopStatementNode> ASTGenerator::parse_while_statement() {
  auto stmt = std::make_unique<WhileLoopStatementNode>();
  stmt->token = current_token_;

  advance_token();

  stmt->condition = parse_expression();

  stmt->block = parse_block_statement();

  return stmt;
}

std::unique_ptr<ForLoopStatementNode> ASTGenerator::parse_for_statement() {
  auto stmt = std::make_unique<ForLoopStatementNode>();
  stmt->token = current_token_;

  advance_token();

  if (!check_token(TokenType::IDENTIFIER)) {
    report_error("Expected identifier in for loop");
    return nullptr;
  }

  stmt->identifier = parse_identifier();

  if (!consume_token(TokenType::IDENTIFIER, "Expected 'in' keyword")) {
    return nullptr;
  }

  stmt->iterable = parse_expression();

  return stmt;
}

std::unique_ptr<ExpressionStatementNode> ASTGenerator::parse_expression_statement() {
  auto stmt = std::make_unique<ExpressionStatementNode>();
  stmt->expression = parse_expression();

  consume_token(TokenType::SEMICOLON, "Expected ';' after expression");

  return stmt;
}

std::unique_ptr<ExpressionNode> ASTGenerator::parse_expression() {
  return parse_assignment();
}

std::unique_ptr<ExpressionNode> ASTGenerator::parse_assignment() {
  auto expr = parse_if_else();

  if (check_token(TokenType::ASSIGN)) {
    auto assignment = std::make_unique<AssignmentExpressionNode>();
    assignment->token = current_token_;
    assignment->target = std::move(expr);

    advance_token();

    assignment->value = parse_assignment();
    return assignment;
  }

  return expr;
}

std::unique_ptr<ExpressionNode> ASTGenerator::parse_if_else() {
  auto expr = parse_logical_or();

  if (check_token(TokenType::IF)) {
    auto if_expr = std::make_unique<IfElseExpressionNode>();
    if_expr->token = current_token_;
    if_expr->then_expression = std::move(expr);

    advance_token();

    if_expr->condition = parse_expression();

    if (check_token(TokenType::ELSE)) {
      advance_token();
      if_expr->else_expression = parse_if_else();
    }

    return if_expr;
  }

  return expr;
}

std::unique_ptr<ExpressionNode> ASTGenerator::parse_logical_or() {
  auto expr = parse_logical_and();

  while (check_token(TokenType::OR)) {
    auto binary = std::make_unique<BinaryExpressionNode>();
    binary->operator_token = current_token_;
    binary->left = std::move(expr);

    advance_token();

    binary->right = parse_logical_and();
    expr = std::move(binary);
  }

  return expr;
}

std::unique_ptr<ExpressionNode> ASTGenerator::parse_logical_and() {
  auto expr = parse_equality();

  while (check_token(TokenType::AND)) {
    auto binary = std::make_unique<BinaryExpressionNode>();
    binary->operator_token = current_token_;
    binary->left = std::move(expr);

    advance_token();

    binary->right = parse_equality();
    expr = std::move(binary);
  }

  return expr;
}

std::unique_ptr<ExpressionNode> ASTGenerator::parse_equality() {
  auto expr = parse_comparison();

  while (check_token(TokenType::NOT_EQUAL) || check_token(TokenType::EQUAL)) {
    auto binary = std::make_unique<BinaryExpressionNode>();
    binary->operator_token = current_token_;
    binary->left = std::move(expr);

    advance_token();

    binary->right = parse_comparison();
    expr = std::move(binary);
  }

  return expr;
}

std::unique_ptr<ExpressionNode> ASTGenerator::parse_comparison() {
  auto expr = parse_term();

  while (check_token(TokenType::GREATER) || check_token(TokenType::GREATER_EQUAL) ||
         check_token(TokenType::LESS) || check_token(TokenType::LESS_EQUAL)) {
    auto binary = std::make_unique<BinaryExpressionNode>();
    binary->operator_token = current_token_;
    binary->left = std::move(expr);

    advance_token();

    binary->right = parse_term();
    expr = std::move(binary);
  }

  return expr;
}

std::unique_ptr<ExpressionNode> ASTGenerator::parse_term() {
  auto expr = parse_factor();

  while (check_token(TokenType::MINUS) || check_token(TokenType::PLUS)) {
    auto binary = std::make_unique<BinaryExpressionNode>();
    binary->operator_token = current_token_;
    binary->left = std::move(expr);

    advance_token();

    binary->right = parse_factor();
    expr = std::move(binary);
  }

  return expr;
}

std::unique_ptr<ExpressionNode> ASTGenerator::parse_factor() {
  auto expr = parse_unary();

  while (check_token(TokenType::SLASH) || check_token(TokenType::ASTERISK)) {
    auto binary = std::make_unique<BinaryExpressionNode>();
    binary->operator_token = current_token_;
    binary->left = std::move(expr);

    advance_token();

    binary->right = parse_unary();
    expr = std::move(binary);
  }

  return expr;
}

std::unique_ptr<ExpressionNode> ASTGenerator::parse_unary() {
  if (check_token(TokenType::BANG) || check_token(TokenType::MINUS)) {
    auto unary = std::make_unique<UnaryExpressionNode>();
    unary->operator_token = current_token_;

    advance_token();

    unary->operand = parse_unary();
    return unary;
  }

  return parse_primary();
}

std::unique_ptr<ExpressionNode> ASTGenerator::parse_primary() {
  switch (current_token_.type) {
    case TokenType::TRUE:
    case TokenType::FALSE:
      return parse_boolean_literal();
    case TokenType::INTEGER:
      return parse_integer_literal();
    case TokenType::FLOAT:
      return parse_float_literal();
    case TokenType::STRING:
      return parse_string_literal();
    case TokenType::IDENTIFIER:
      return parse_identifier();
    case TokenType::LPAREN: {
      advance_token();
      auto expr = parse_expression();
      consume_token(TokenType::RPAREN, "Expected ')' after expression");
      return expr;
    }
    default:
      report_error("Unexpected token in expression");
      return nullptr;
  }
}

std::unique_ptr<IntegerLiteralNode> ASTGenerator::parse_integer_literal() {
  auto literal = std::make_unique<IntegerLiteralNode>();
  literal->token = current_token_;

  std::stringstream ss(current_token_.lexeme);
  ss >> literal->value;

  advance_token();
  return literal;
}

std::unique_ptr<FloatLiteralNode> ASTGenerator::parse_float_literal() {
  auto literal = std::make_unique<FloatLiteralNode>();
  literal->token = current_token_;

  literal->value = std::stod(current_token_.lexeme);

  advance_token();
  return literal;
}

std::unique_ptr<StringLiteralNode> ASTGenerator::parse_string_literal() {
  auto literal = std::make_unique<StringLiteralNode>();
  literal->token = current_token_;

  literal->value = current_token_.lexeme.substr(1, current_token_.lexeme.length() - 2);

  advance_token();
  return literal;
}

std::unique_ptr<BooleanLiteralNode> ASTGenerator::parse_boolean_literal() {
  auto literal = std::make_unique<BooleanLiteralNode>();
  literal->token = current_token_;

  literal->value = (current_token_.type == TokenType::TRUE);

  advance_token();
  return literal;
}

std::unique_ptr<IdentifierNode> ASTGenerator::parse_identifier() {
  auto identifier = std::make_unique<IdentifierNode>();
  identifier->token = current_token_;
  identifier->name = current_token_.lexeme;

  advance_token();
  return identifier;
}