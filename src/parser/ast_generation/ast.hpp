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

#include "tokens.hpp"

enum class ASTType { IDENTIFIER, VARIABLE_STATEMENT };

/// @brief Base class of all AST nodes
struct ASTNode {
  virtual ~ASTNode() = default;

  /**
   * @brief Gives the type of the AST node
   * @return Returns a scoped enum called ASTType (e.g., VariableStatement ->
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

struct VariableStatement final : StatementNode {
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

  bool is_mutible() const noexcept {
    return token.type == TokenType::VAR;
  }
};