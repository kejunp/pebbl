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

#include "tokens.hpp"

enum class ASTType {

};

/// @brief Base class of all AST nodes
struct ASTNode {
  virtual ~ASTNode() = default;

  /**
   * @brief Gives the type of the AST node
   * @return Returns a scoped enum called ASTType (e.g., LetStatement -> ASTTYPE_LET_STATEMENT)
   */
  virtual ASTType type() const = 0;

  /**
   * @brief Gives a pointer to the token, if no meaningful token, return nullptr
   * @return Returns a pointer to the token of the AST node (BinaryExpressionNode will return
   * whatever the operator is, etc.)
   */
  virtual const Token* get_token() const {
    return nullptr;
  }
};

/// @brief Base class for all statements
struct StatementNode : ASTNode {};

/// @brief Base class for all expressions
struct ExpressionNode : ASTNode {};
