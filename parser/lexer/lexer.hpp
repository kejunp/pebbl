#pragma once

#include "common.hpp"
#include "tokens.hpp"

class Lexer {
public:
  explicit Lexer(std::string&& input) noexcept;

  /**
   * @brief Returns next token in the source code
   */
  Token next_token();

private:
  std::string input_;          ///< Source code
  std::size_t position_;       ///< Index that the lexer is at currently in the source code
  std::size_t read_position_;  ///< The next position the lexer is going to read
  std::size_t line_;           ///< The current line of the source code
  char current_;               ///< The current character (input_[position_])

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
   * @brief Consumes the characters until the section isn't a valid identifier, then returns the identifier
   */
  std::string read_identifier();

  /**
   * @brief Consumes the characters until the section isn't a valid number, then returns the number
   */
  std::string read_number();

  /**
   * @brief Skips all whitespace
   */
  void consume_whitespace();
};