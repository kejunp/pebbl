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

#pragma once

#include <memory>
#include <string>

#include "../lexer/lexer.hpp"
#include "ast.hpp"

/// @brief Recursive descent parser for generating AST from tokens
class ASTGenerator {
public:
  explicit ASTGenerator(Lexer& lexer);

  /**
   * @brief Parses the entire program and returns the root AST node
   * @return Program AST node containing all statements
   */
  std::unique_ptr<ProgramNode> parse_program();

private:
  Lexer& lexer_;         ///< Reference to the lexer
  Token current_token_;  ///< Current token being processed
  Token peek_token_;     ///< Next token for lookahead

  /**
   * @brief Advances to the next token
   */
  void advance_token();

  /**
   * @brief Checks if current token matches expected type
   * @param type Expected token type
   * @return True if current token matches expected type
   */
  bool check_token(TokenType type) const;

  /**
   * @brief Consumes token if it matches expected type, otherwise reports error
   * @param type Expected token type
   * @param error_message Error message if token doesn't match
   * @return True if token was successfully consumed
   */
  bool consume_token(TokenType type, const std::string& error_message);

  /**
   * @brief Reports a parse error
   * @param message Error message
   */
  void report_error(const std::string& message) const;

  /**
   * @brief Checks if the program parsing is complete
   * @return True if no more meaningful tokens remain
   */
  bool is_program_complete() const;

  /**
   * @brief Checks if parsing should terminate due to unrecoverable tokens
   * @return True if parsing should stop cleanly
   */
  bool should_terminate_parsing() const;

  /**
   * @brief Checks if current token is a leftover separator from array/dict parsing
   * @return True if token is a separator that can't start a statement
   */
  bool is_leftover_separator_token() const;

  // Statement parsing methods
  /**
   * @brief Parses a statement
   * @return Statement AST node
   */
  std::unique_ptr<StatementNode> parse_statement();

  /**
   * @brief Parses a variable declaration statement (let/var)
   * @return Variable statement AST node
   */
  std::unique_ptr<VariableStatementNode> parse_variable_statement();

  /**
   * @brief Parses a return statement
   * @return Return statement AST node
   */
  std::unique_ptr<ReturnStatementNode> parse_return_statement();

  /**
   * @brief Parses a block statement ({ statements... })
   * @return Block statement AST node
   */
  std::unique_ptr<BlockStatementNode> parse_block_statement();

  /**
   * @brief Parses a while loop statement
   * @return While loop statement AST node
   */
  std::unique_ptr<WhileLoopStatementNode> parse_while_statement();

  /**
   * @brief Parses a for loop statement
   * @return For loop statement AST node
   */
  std::unique_ptr<ForLoopStatementNode> parse_for_statement();

  /**
   * @brief Parses an expression statement
   * @return Expression statement AST node
   */
  std::unique_ptr<ExpressionStatementNode> parse_expression_statement();

  // Expression parsing methods (with precedence)
  /**
   * @brief Parses an expression
   * @return Expression AST node
   */
  std::unique_ptr<ExpressionNode> parse_expression();

  /**
   * @brief Parses an assignment expression
   * @return Expression AST node
   */
  std::unique_ptr<ExpressionNode> parse_assignment();

  /**
   * @brief Parses an if-else expression
   * @return Expression AST node
   */
  std::unique_ptr<ExpressionNode> parse_if_else();

  /**
   * @brief Parses a logical OR expression
   * @return Expression AST node
   */
  std::unique_ptr<ExpressionNode> parse_logical_or();

  /**
   * @brief Parses a logical AND expression
   * @return Expression AST node
   */
  std::unique_ptr<ExpressionNode> parse_logical_and();

  /**
   * @brief Parses an equality expression (== !=)
   * @return Expression AST node
   */
  std::unique_ptr<ExpressionNode> parse_equality();

  /**
   * @brief Parses a comparison expression (< <= > >=)
   * @return Expression AST node
   */
  std::unique_ptr<ExpressionNode> parse_comparison();

  /**
   * @brief Parses a term expression (+ -)
   * @return Expression AST node
   */
  std::unique_ptr<ExpressionNode> parse_term();

  /**
   * @brief Parses a factor expression (* /)
   * @return Expression AST node
   */
  std::unique_ptr<ExpressionNode> parse_factor();

  /**
   * @brief Parses a unary expression (! -)
   * @return Expression AST node
   */
  std::unique_ptr<ExpressionNode> parse_unary();

  /**
   * @brief Parses a primary expression (literals, identifiers, parentheses)
   * @return Expression AST node
   */
  std::unique_ptr<ExpressionNode> parse_primary();

  // Literal parsing methods
  /**
   * @brief Parses an integer literal
   * @return Integer literal AST node
   */
  std::unique_ptr<IntegerLiteralNode> parse_integer_literal();

  /**
   * @brief Parses a float literal
   * @return Float literal AST node
   */
  std::unique_ptr<FloatLiteralNode> parse_float_literal();

  /**
   * @brief Parses a string literal
   * @return String literal AST node
   */
  std::unique_ptr<StringLiteralNode> parse_string_literal();

  /**
   * @brief Parses a boolean literal
   * @return Boolean literal AST node
   */
  std::unique_ptr<BooleanLiteralNode> parse_boolean_literal();

  /**
   * @brief Parses an identifier
   * @return Identifier AST node
   */
  std::unique_ptr<IdentifierNode> parse_identifier();

  /**
   * @brief Parses an array literal
   * @return Array literal AST node
   */
  std::unique_ptr<ArrayLiteralNode> parse_array_literal();

  /**
   * @brief Parses a dictionary literal
   * @return Dictionary literal AST node
   */
  std::unique_ptr<DictLiteralNode> parse_dict_literal();
};