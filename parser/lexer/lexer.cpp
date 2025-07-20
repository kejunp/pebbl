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

#include "lexer.hpp"

#include <cctype>

namespace {

Token make_token(TokenType type, std::string&& lexeme, std::size_t line) {
  return Token{type, std::move(lexeme), line};
}

Token make_token(TokenType type, const std::string& lexeme, std::size_t line) {
  return Token{type, lexeme, line};
}

}  // namespace

Lexer::Lexer(std::string&& input) noexcept :
    input_(std::move(input)), position_{}, read_position_{0}, current_char_{'\0'} {
  consume_char();
}

Token Lexer::next_token() {
  consume_whitespace();

  if (current_char_ == '\0') {
    return make_token(TokenType::EOF_TYPE, "", line_);
  }

  switch (current_char_) {
    case '(':
      return make_token(TokenType::LPAREN, "(", line_);
    case ')':
      return make_token(TokenType::RPAREN, ")", line_);
    case '{':
      return make_token(TokenType::LBRACE, "{", line_);
    case '}':
      return make_token(TokenType::RBRACE, "}", line_);
    case '[':
      return make_token(TokenType::LBRACKET, "[", line_);
    case ']':
      return make_token(TokenType::RBRACKET, "]", line_);
    case ',':
      return make_token(TokenType::COMMA, ",", line_);
    case '.':
      return make_token(TokenType::DOT, ".", line_);
    case ';':
      return make_token(TokenType::SEMICOLON, ";", line_);
    case '+':
      return make_token(TokenType::PLUS, "+", line_);
    case '-':
      return make_token(TokenType::MINUS, "-", line_);
    case '*':
      return make_token(TokenType::ASTERISK, "*", line_);
    case '/':
      return make_token(TokenType::SLASH, "/", line_);
    case '!':
      if (peek_char() == '=') {
        consume_char();
        consume_char();
        return make_token(TokenType::NOT_EQUAL, "!=", line_);
      }
      consume_char();
      return make_token(TokenType::BANG, "!", line_);
    case '=':
      if (peek_char() == '=') {
        consume_char();
        consume_char();
        return make_token(TokenType::EQUAL, "==", line_);
      }
      consume_char();
      return make_token(TokenType::ASSIGN, "=", line_);
    case '<':
      if (peek_char() == '=') {
        consume_char();
        consume_char();
        return make_token(TokenType::LESS_EQUAL, "<=", line_);
      }
      consume_char();
      return make_token(TokenType::LESS, "<", line_);
    case '>':
      if (peek_char() == '=') {
        consume_char();
        consume_char();
        return make_token(TokenType::GREATER_EQUAL, ">=", line_);
      }
      consume_char();
      return make_token(TokenType::GREATER, ">", line_);
    default:
      if (std::isalpha(current_char_) || current_char_ == '_') {
        const std::string lexeme = read_identifier();
        const TokenType type = lookup_identifier(lexeme);
        return make_token(type, lexeme, line_);
      } else if (
          std::isdigit(current_char_) || (current_char_ == '.' && std::isdigit(peek_char()))) {
        const auto [type, lexeme] = read_number();
        return make_token(type, lexeme, line_);
      } else {
        const char unknown = current_char_;
        consume_char();
        return make_token(TokenType::ERROR, std::string(1, unknown), line_);
      }
  }
}

std::pair<TokenType, std::string> Lexer::read_number() {
  const auto start_position = position_;
  auto type = TokenType::INTEGER;
  auto has_dot = false;

  if (current_char_ == '.') {
    has_dot = true;
    type = TokenType::FLOAT;
    consume_char();
  }

  while (std::isdigit(current_char_) || (!has_dot && current_char_ == '.')) {
    if (current_char_ == '.') {
      has_dot = true;
      type = TokenType::FLOAT;
    }
    consume_char();
  }

  auto lexeme = input_.substr(start_position, read_position_ - start_position);
  return {type, lexeme};
}

void Lexer::consume_char() {
  if (read_position_ >= input_.length()) {
    current_char_ = '\0';
  } else {
    current_char_ = input_[read_position_];
  }
  position_ = read_position_++;
}

char Lexer::peek_char() const {
  if (read_position_ >= input_.length()) {
    return '\0';
  } else {
    return input_[read_position_];
  }
}

std::string Lexer::read_identifier() {
  const std::size_t start_position = position_;
  while (std::isalnum(current_char_) || current_char_ == '_') {
    consume_char();
  }
  return input_.substr(start_position, read_position_ - start_position);
}

void Lexer::consume_whitespace() {
  while (true) {
    switch (peek_char()) {
      case '\n':
        ++line_;
        [[fallthrough]];
      case ' ':
      case '\t':
      case '\r':
        consume_char();
        break;
      default:
        return;
    }
  }
}
