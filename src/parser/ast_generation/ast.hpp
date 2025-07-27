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
#include <vector>
#include <boost/multiprecision/cpp_int.hpp>

#include "tokens.hpp"

enum class ASTType {
  IDENTIFIER,
  VARIABLE_STATEMENT,
  RETURN_STATEMENT,
  EXPRESSION_STATEMENT,
  PROGRAM_ROOT,
  BLOCK_STATEMENT,
  WHILE_LOOP_STATEMENT,
  FOR_LOOP_STATEMENT,
  INTEGER_LITERAL
};

/// @brief Base class of all AST nodes
struct ASTNode {
  virtual ~ASTNode() = default;

  /**
   * @brief Gives the type of the AST node
   * @return Returns a scoped enum called ASTType (e.g., VariableStatementNode ->
   * AstType::VARIABLE_STATEMENT)
   */
  virtual ASTType type() const noexcept = 0;

  /**
   * @brief Gives a pointer to the token, if no meaningful token, return nullptr
   * @return Returns a pointer to the token of the AST node (BinaryExpressionNode will return
   * whatever the operator is, etc.)
   */
  virtual const Token* get_token() const noexcept {
    return nullptr;
  }
};

/// @brief Base class for all statements
struct StatementNode : ASTNode {};

/// @brief Base class for all expressions
struct ExpressionNode : ASTNode {};

/// @brief Base class for all literals
struct LiteralNode : ExpressionNode {};

/// @brief The root of all ASTs (sorta like a block statement, but the block is global)
struct ProgramNode final : ASTNode {
  std::vector<std::unique_ptr<StatementNode>> statements;

  /**
   * @return Returns ASTType::PROGRAM_ROOT
   */
  ASTType type() const noexcept override {
    return ASTType::PROGRAM_ROOT;
  }
};

/// @brief A identifier
struct IdentifierNode final : ExpressionNode {
  Token token;       ///< A Identifier token
  std::string name;  ///< The name of the identifier

  /**
   * @return Returns ASTType::IDENTIFIER
   */
  ASTType type() const noexcept override {
    return ASTType::IDENTIFIER;
  }

  /**
   * @return Returns pointer to a identifier token
   */
  const Token* get_token() const noexcept override {
    return &token;
  }
};

/// @brief A variable declaration/definition: let immut = 5; var mut = 5; etc.
struct VariableStatementNode final : StatementNode {
  Token token;  ///< Either let or var

  std::unique_ptr<IdentifierNode> name;  ///< The identifier ( let [name] = 25;)
  std::unique_ptr<ExpressionNode>
      value;  ///< The expression that you are assigning to ( let x = [value])

  /**
   * @return Returns ASTType::VARIABLE_STATEMENT
   */
  ASTType type() const noexcept override {
    return ASTType::VARIABLE_STATEMENT;
  }

  const Token* get_token() const noexcept override {
    return &token;
  }

  /**
   * @brief Is the variable mutable?
   * @return True if the variable is mutable, and vise versa
   */
  bool is_mutible() const noexcept {
    return token.type == TokenType::VAR;
  }
};

/**
 * @brief A return statement (different from a implicit return, e.g. 5; will become a expression
 * statement, return 5; will become this)
 */
struct ReturnStatementNode final : StatementNode {
  Token token;  ///< Always a token with type RETURN, lexeme of return, kept here for line number
  std::unique_ptr<ExpressionNode> return_value;  ///< Expression to return

  /**
   * @return Returns ASTType::RETURN_STATEMENT
   */
  ASTType type() const noexcept override {
    return ASTType::RETURN_STATEMENT;
  }

  const Token* get_token() const noexcept override {
    return &token;
  }
};

struct ForLoopStatementNode final : StatementNode {
  Token token;  ///< Always a token with TokenType::FOR and lexeme "for"
  std::unique_ptr<IdentifierNode> identifier;  ///< The iterator (e.g. for [identifier] in range..)
  std::unique_ptr<ExpressionNode> iterable;    ///< The thing to iterate over (e.g., a list)

  /**
   * @return Returns ASTType::FOR_LOOP_STATEMENT
   */
  ASTType type() const noexcept override {
    return ASTType::FOR_LOOP_STATEMENT;
  }

  const Token* get_token() const noexcept override {
    return &token;
  }
};

/// @brief A while loop (e.g., while x < y { let x = 5 let y = 4;})
struct WhileLoopStatementNode final : StatementNode {
  Token token;  ///< Always a token with TokenType::WHILE and lexeme "while"
  std::unique_ptr<ExpressionNode> condition;  ///< The condition
  std::unique_ptr<BlockStatementNode> block;  ///< If the condition is true, this happens

  /**
   * @return Returns ASTType::WHILE_LOOP_STATEMENT
   */
  ASTType type() const noexcept override {
    return ASTType::WHILE_LOOP_STATEMENT;
  }

  const Token* get_token() const noexcept override {
    return &token;
  }
};

/// @brief A block statement { [statements... ] }
struct BlockStatementNode final : StatementNode {
  std::vector<std::unique_ptr<StatementNode>> statements;

  /**
   * @return Returns ASTType::BLOCK_STATEMENT
   */
  ASTType type() const noexcept override {
    return ASTType::BLOCK_STATEMENT;
  }
};

/// @brief A wrapper around a expression (but it is a statement)
struct ExpressionStatementNode final : StatementNode {
  std::unique_ptr<ExpressionNode> expression;

  /**
   * @return Returns ASTType::EXPRESSION_STATEMENT
   */
  ASTType type() const noexcept override {
    return ASTType::EXPRESSION_STATEMENT;
  }

  const Token* get_token() const noexcept override {
    return expression->get_token();
  }
};

/// @brief A integer literal
struct IntegerLiteralNode : LiteralNode {
  Token token;
  boost::multiprecision::cpp_int value;

  /**
   * @return Returns ASTType::INTEGER_LITERAL
   */
  ASTType type() const noexcept override {
    return ASTType::INTEGER_LITERAL;
  }

  const Token* get_token() const noexcept override {
    return &token;
  }
};