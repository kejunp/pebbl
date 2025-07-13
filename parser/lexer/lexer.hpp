#pragma once

#include "common.hpp"
#include "tokens.hpp"

class Lexer {
public:
  explicit Lexer(std::string&& input) noexcept;

  Token next_token();
private:
  std::string input_;
  std::size_t position_;
  std::size_t read_position_;
  std::size_t line_;
  char current_;

  void consume_char();

  [[nodiscard]]
  char peek_char() const;

  std::string read_identifier();
  std::string read_number();

  void consume_whitespace();
};