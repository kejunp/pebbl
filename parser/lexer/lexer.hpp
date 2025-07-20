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

#include <string>
#include <utility>

#include "common.hpp"
#include "tokens.hpp"

/// @brief Token stream implementation
class Lexer {
public:
  explicit Lexer(std::string&& input) noexcept;

  /**
   * @brief Returns next token in the source code
   */
  Token next_token();

  inline Lexer& operator>>(Token& token) {
    token = next_token();
    return *this;
  }

private:
  std::string input_;          ///< Source code
  std::size_t position_;       ///< Index that the lexer is at currently in the source code
  std::size_t read_position_;  ///< The next position the lexer is going to read
  std::size_t line_;           ///< The current line of the source code
  char current_char_;          ///< The current character (input_[position_])

  /**
   * @brief Consumes a character and goes to the next one
   */
  void consume_char();

  /**
   * @brief Returns the next character from the source code
   */
  [[nodiscard]]
  char peek_char() const;

  /**
   * @brief Consumes the characters until the section isn't a valid identifier,
   * then returns the identifier
   */
  std::string read_identifier();

  /**
   * @brief Consumes the characters until the section isn't a valid number, then
   * returns the number
   */
  std::pair<TokenType, std::string> read_number();

  /**
   * @brief Skips all whitespace
   */
  void consume_whitespace();

  /**
   * @brief Creates a token
   */
  inline Token make_token(TokenType type, std::string&& lexeme) {
    for (size_t i = 0; i < lexeme.length(); ++i) {
      consume_char();
    }
    return Token{.type = type, .lexeme = std::move(lexeme), .line = line_};
  }

  /**
   * @overload
   */
  Token make_token(TokenType type, const std::string& lexeme) {
    for (size_t i = 0; i < lexeme.length(); ++i) {
      consume_char();
    }
    return Token{.type = type, .lexeme = lexeme, .line = line_};
  }
};