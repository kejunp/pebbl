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

/**
 * @file ast_generator.cpp
 * @brief Implementation of the recursive descent parser for generating ASTs
 */

#include "ast_generator.hpp"

#include <iostream>
#include <sstream>
#include <cctype>

#include "../../common.hpp"

ASTGenerator::ASTGenerator(Lexer& lexer) : lexer_(lexer) {
  // Initialize both current and peek tokens
  // For proper two-token lookahead, we need to get the first two tokens
  current_token_ = lexer_.next_token();
  peek_token_ = lexer_.next_token();
}

std::unique_ptr<ProgramNode> ASTGenerator::parse_program() {
  auto program = std::make_unique<ProgramNode>();

  while (current_token_.type != TokenType::EOF_TYPE) {
    auto stmt = parse_statement();
    if (stmt) {
      program->statements.push_back(std::move(stmt));
      
      // Check if program is complete after successful parsing
      if (is_program_complete()) {
        break;
      }
    } else {
      // Failed to parse statement - check if we should terminate cleanly
      if (should_terminate_parsing()) {
        break;
      }
      
      // Try to recover by advancing token
      advance_token();
    }
  }

  return program;
}

bool ASTGenerator::is_program_complete() const {
  // If we're at EOF, program is complete
  if (current_token_.type == TokenType::EOF_TYPE) {
    return true;
  }
  
  // If current token is a leftover separator that cannot start a new statement,
  // consider the program complete (these are remnants from array/dict parsing)
  return is_leftover_separator_token();
}

bool ASTGenerator::should_terminate_parsing() const {
  // Terminate cleanly if we encounter tokens that are clearly leftover separators
  return is_leftover_separator_token();
}

bool ASTGenerator::is_leftover_separator_token() const {
  // These tokens cannot start a statement and are likely leftovers from parsing
  // Also check for any token that clearly indicates parsing artifacts
  return current_token_.type == TokenType::COMMA || 
         current_token_.type == TokenType::COLON ||
         current_token_.type == TokenType::RBRACE ||
         current_token_.type == TokenType::RBRACKET ||
         current_token_.lexeme == ":" ||  // Explicit check for colon lexeme
         current_token_.lexeme == "," ||  // Explicit check for comma lexeme  
         current_token_.lexeme == "}" ||  // Explicit check for right brace lexeme
         current_token_.lexeme == "]";    // Explicit check for right bracket lexeme
}

void ASTGenerator::advance_token() {
  current_token_ = peek_token_;
  peek_token_ = lexer_.next_token();
  
  // Skip empty or whitespace-only tokens
  while (current_token_.type != TokenType::EOF_TYPE && 
         (current_token_.lexeme.empty() || 
          (current_token_.lexeme.size() == 1 && std::isspace(current_token_.lexeme[0])))) {
    current_token_ = peek_token_;
    peek_token_ = lexer_.next_token();
  }
}

bool ASTGenerator::check_token(TokenType type) const {
  return current_token_.type == type;
}

bool ASTGenerator::consume_token(TokenType type, const std::string& error_message) {
  if (check_token(type)) {
    advance_token();
    return true;
  }
  
  // Report error but ALWAYS advance to prevent infinite loops
  report_error(error_message);
  advance_token();  // Critical fix: advance even on failure
  return false;
}

void ASTGenerator::report_error(const std::string& message) const {
  // Don't report errors for empty tokens, whitespace, or leftover separator tokens
  if (current_token_.lexeme.empty() || 
      (current_token_.lexeme.size() == 1 && std::isspace(current_token_.lexeme[0])) ||
      (message == "Unexpected token in expression" && is_leftover_separator_token())) {
    return;
  }
  
  std::cerr << "\033[31mpebbli: Error: Parse error at line " << current_token_.line << ": " << message << " (got '"
            << current_token_.lexeme << "')\033[0m" << std::endl;
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
    case TokenType::FUNC:
      return parse_function_statement();
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

  if (!consume_token(TokenType::IN, "Expected 'in' keyword")) {
    return nullptr;
  }

  stmt->iterable = parse_expression();
  
  if (!stmt->iterable) {
    report_error("Expected expression after 'in'");
    return nullptr;
  }

  stmt->body = parse_block_statement();
  
  if (!stmt->body) {
    report_error("Expected block statement for for loop body");
    return nullptr;
  }

  return stmt;
}

std::unique_ptr<FunctionStatementNode> ASTGenerator::parse_function_statement() {
  auto stmt = std::make_unique<FunctionStatementNode>();
  stmt->token = current_token_;

  advance_token();

  if (!check_token(TokenType::IDENTIFIER)) {
    report_error("Expected function name");
    return nullptr;
  }

  stmt->name = parse_identifier();

  if (!consume_token(TokenType::LPAREN, "Expected '(' after function name")) {
    return nullptr;
  }

  // Parse parameter list
  while (!check_token(TokenType::RPAREN) && current_token_.type != TokenType::EOF_TYPE) {
    if (!check_token(TokenType::IDENTIFIER)) {
      report_error("Expected parameter name");
      return nullptr;
    }

    stmt->parameters.push_back(parse_identifier());

    if (check_token(TokenType::COMMA)) {
      advance_token();
    } else if (!check_token(TokenType::RPAREN)) {
      report_error("Expected ',' or ')' in parameter list");
      return nullptr;
    }
  }

  if (!consume_token(TokenType::RPAREN, "Expected ')' after parameter list")) {
    return nullptr;
  }

  stmt->body = parse_block_statement();
  
  if (!stmt->body) {
    report_error("Expected function body");
    return nullptr;
  }

  return stmt;
}

std::unique_ptr<ExpressionStatementNode> ASTGenerator::parse_expression_statement() {
  auto stmt = std::make_unique<ExpressionStatementNode>();
  stmt->expression = parse_expression();
  
  if (!stmt->expression) {
    return nullptr;
  }

  // Consume semicolon if present
  if (check_token(TokenType::SEMICOLON)) {
    advance_token();
  }

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
  // Check for if-else expression: if condition { then } else { else }
  if (check_token(TokenType::IF)) {
    auto if_expr = std::make_unique<IfElseExpressionNode>();
    if_expr->token = current_token_;
    
    advance_token();
    
    // Parse condition
    if_expr->condition = parse_logical_or();
    
    // Expect opening brace for then expression
    if (!consume_token(TokenType::LBRACE, "Expected '{' after if condition")) {
      return nullptr;
    }
    
    // Parse then expression
    if_expr->then_expression = parse_expression();
    
    // Expect closing brace
    if (!consume_token(TokenType::RBRACE, "Expected '}' after then expression")) {
      return nullptr;
    }
    
    // Optional else clause
    if (check_token(TokenType::ELSE)) {
      advance_token();
      
      // Expect opening brace for else expression
      if (!consume_token(TokenType::LBRACE, "Expected '{' after else")) {
        return nullptr;
      }
      
      // Parse else expression
      if_expr->else_expression = parse_expression();
      
      // Expect closing brace
      if (!consume_token(TokenType::RBRACE, "Expected '}' after else expression")) {
        return nullptr;
      }
    }
    
    return if_expr;
  }
  
  return parse_logical_or();
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

  return parse_call();
}

std::unique_ptr<ExpressionNode> ASTGenerator::parse_call() {
  auto expr = parse_primary();
  
  if (!expr) {
    return nullptr;
  }
  
  // Handle function calls
  while (check_token(TokenType::LPAREN)) {
    expr = parse_call_expression(std::move(expr));
    if (!expr) {
      return nullptr;
    }
  }
  
  return expr;
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
    case TokenType::LBRACKET:
      return parse_array_literal();
    case TokenType::LBRACE:
      return parse_dict_literal();
    case TokenType::LPAREN: {
      advance_token();
      auto expr = parse_expression();
      consume_token(TokenType::RPAREN, "Expected ')' after expression");
      return expr;
    }
    // Semicolon and EOF indicate end of expression, not an error
    case TokenType::SEMICOLON:
    case TokenType::EOF_TYPE:
      return nullptr;
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

std::unique_ptr<ArrayLiteralNode> ASTGenerator::parse_array_literal() {
  auto array = std::make_unique<ArrayLiteralNode>();
  array->token = current_token_;

  // Validate we're starting with '['
  if (!check_token(TokenType::LBRACKET)) {
    report_error("Expected '[' at start of array");
    return nullptr;
  }
  
  consume_token(TokenType::LBRACKET, "Expected '['");

  // Handle empty array case
  if (check_token(TokenType::RBRACKET)) {
    consume_token(TokenType::RBRACKET, "Expected ']'");
    return array;
  }

  // Parse array elements
  int element_count = 0;
  const int max_elements = 10000;  // Prevent runaway parsing
  
  do {
    // Safety check to prevent infinite loops
    if (element_count++ > max_elements) {
      report_error("Array too large, stopping parse");
      break;
    }
    
    // Skip any unexpected tokens gracefully
    if (current_token_.type == TokenType::EOF_TYPE) {
      report_error("Unexpected EOF in array");
      break;
    }
    
    auto element = parse_expression();
    if (element) {
      array->elements.push_back(std::move(element));
    } else {
      // If we can't parse an element, skip to next comma or end
      while (!check_token(TokenType::COMMA) && 
             !check_token(TokenType::RBRACKET) && 
             current_token_.type != TokenType::EOF_TYPE) {
        advance_token();
      }
    }

    // Handle comma separator
    if (check_token(TokenType::COMMA)) {
      advance_token();
      // Allow trailing comma by checking for immediate ']'
      if (check_token(TokenType::RBRACKET)) {
        break;
      }
    } else {
      break;
    }
  } while (!check_token(TokenType::RBRACKET) && current_token_.type != TokenType::EOF_TYPE);

  consume_token(TokenType::RBRACKET, "Expected ']' after array elements");
  return array;
}

std::unique_ptr<DictLiteralNode> ASTGenerator::parse_dict_literal() {
  auto dict = std::make_unique<DictLiteralNode>();
  dict->token = current_token_;

  // Validate we're starting with '{'
  if (!check_token(TokenType::LBRACE)) {
    report_error("Expected '{' at start of dictionary");
    return nullptr;
  }
  
  consume_token(TokenType::LBRACE, "Expected '{'");

  // Handle empty dictionary case
  if (check_token(TokenType::RBRACE)) {
    consume_token(TokenType::RBRACE, "Expected '}'");
    return dict;
  }

  // Parse dictionary entries
  int entry_count = 0;
  const int max_entries = 10000;  // Prevent runaway parsing
  
  do {
    // Safety check to prevent infinite loops
    if (entry_count++ > max_entries) {
      report_error("Dictionary too large, stopping parse");
      break;
    }
    
    // Skip any unexpected tokens gracefully
    if (current_token_.type == TokenType::EOF_TYPE) {
      report_error("Unexpected EOF in dictionary");
      break;
    }
    
    // Parse key
    auto key = parse_expression();
    if (!key) {
      report_error("Expected dictionary key");
      // Skip to next comma, colon, or end
      while (!check_token(TokenType::COMMA) && 
             !check_token(TokenType::COLON) &&
             !check_token(TokenType::RBRACE) && 
             current_token_.type != TokenType::EOF_TYPE) {
        advance_token();
      }
      if (check_token(TokenType::COMMA)) {
        advance_token();
        continue;
      }
      if (check_token(TokenType::RBRACE)) {
        break;
      }
    }

    // Parse colon separator
    if (!consume_token(TokenType::COLON, "Expected ':' after dictionary key")) {
      // Skip to next comma or end on error
      while (!check_token(TokenType::COMMA) && 
             !check_token(TokenType::RBRACE) && 
             current_token_.type != TokenType::EOF_TYPE) {
        advance_token();
      }
      if (check_token(TokenType::COMMA)) {
        advance_token();
        continue;
      }
      break;
    }
    
    // Parse value
    auto value = parse_expression();
    if (!value) {
      report_error("Expected dictionary value");
      // Skip to next comma or end
      while (!check_token(TokenType::COMMA) && 
             !check_token(TokenType::RBRACE) && 
             current_token_.type != TokenType::EOF_TYPE) {
        advance_token();
      }
      if (check_token(TokenType::COMMA)) {
        advance_token();
        continue;
      }
      break;
    }

    // Successfully parsed key-value pair
    ExpressionNode* key_ptr = key.get();
    dict->keys.push_back(std::move(key));
    dict->entries[key_ptr] = std::move(value);

    // Handle comma separator
    if (check_token(TokenType::COMMA)) {
      advance_token();
      // Allow trailing comma by checking for immediate '}'
      if (check_token(TokenType::RBRACE)) {
        break;
      }
    } else {
      break;
    }
  } while (!check_token(TokenType::RBRACE) && current_token_.type != TokenType::EOF_TYPE);

  consume_token(TokenType::RBRACE, "Expected '}' after dictionary entries");
  return dict;
}

std::unique_ptr<CallExpressionNode> ASTGenerator::parse_call_expression(std::unique_ptr<ExpressionNode> function) {
  auto call = std::make_unique<CallExpressionNode>();
  call->token = current_token_;
  call->function = std::move(function);

  advance_token(); // consume '('

  // Parse argument list
  while (!check_token(TokenType::RPAREN) && current_token_.type != TokenType::EOF_TYPE) {
    auto arg = parse_expression();
    if (arg) {
      call->arguments.push_back(std::move(arg));
    } else {
      // Skip to next comma or end on error
      while (!check_token(TokenType::COMMA) && 
             !check_token(TokenType::RPAREN) && 
             current_token_.type != TokenType::EOF_TYPE) {
        advance_token();
      }
    }

    if (check_token(TokenType::COMMA)) {
      advance_token();
    } else if (!check_token(TokenType::RPAREN)) {
      report_error("Expected ',' or ')' in argument list");
      return nullptr;
    }
  }

  if (!consume_token(TokenType::RPAREN, "Expected ')' after arguments")) {
    return nullptr;
  }

  return call;
}