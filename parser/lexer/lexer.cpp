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
  Token token;
  token.type = type;
  token.lexeme = std::move(lexeme);
  token.line = line;
  return token;
}

Token make_token(TokenType type, const std::string& lexeme, std::size_t line) {
  Token token;
  token.type = type;
  token.lexeme = lexeme;
  token.line = line;
  return token;
}
}  // namespace

Lexer::Lexer(std::string&& input) noexcept :
    input_(std::move(input)), position_{}, read_position_{0}, current_char_{'\0'} {
  consume_char();
}

Token Lexer::next_token() {
  Token token;
  consume_whitespace();
  switch (current_char_) {
    case '(':
      token = make_token(TokenType::LPAREN, "(", line_);
      break;
    case ')':
      token = make_token(TokenType::RPAREN, ")", line_);
      break;
    case '{':
      token = make_token(TokenType::LBRACE, "{", line_);
      break;
    case '}':
      token = make_token(TokenType::RBRACE, "}", line_);
      break;
    case '[':
      token = make_token(TokenType::LBRACKET, "[", line_);
      break;
    case ']':
      token = make_token(TokenType::RBRACKET, "]", line_);
      break;
    case ',':
      token = make_token(TokenType::COMMA, ",", line_);
      break;
    case '.':
      token = make_token(TokenType::DOT, ".", line_);
      break;
    case ';':
      token = make_token(TokenType::SEMICOLON, ";", line_);
      break;
    case '+':
      token = make_token(TokenType::PLUS, "+", line_);
      break;
    case '-':
      token = make_token(TokenType::MINUS, "-", line_);
      break;
    case '*':
      token = make_token(TokenType::ASTERISK, "*", line_);
      break;
    case '/':
      token = make_token(TokenType::SLASH, "/", line_);
      break;
    case '!':
      if (peek_char() == '=') {
        consume_char();
        token = make_token(TokenType::NOT_EQUAL, "!=", line_);
      } else {
        token = make_token(TokenType::BANG, "!", line_);
      }
      break;
    case '=':
      if (peek_char() == '=') {
        consume_char();
        token = make_token(TokenType::EQUAL, "==", line_);
      } else {
        token = make_token(TokenType::ASSIGN, "=", line_);
      }
      break;
    case '<':
      if (peek_char() == '=') {
        consume_char();
        token = make_token(TokenType::LESS_EQUAL, "<=", line_);
      } else {
        token = make_token(TokenType::LESS, "<", line_);
      }
      break;
    case '>':
      if (peek_char() == '=') {
        consume_char();
        token = make_token(TokenType::GREATER_EQUAL, ">=", line_);
      } else {
        token = make_token(TokenType::GREATER, ">", line_);
      }
      break;
    default:
      if (std::isalpha(current_char_) || current_char_ == '_') {
        const std::string lexeme = read_identifier();
        const TokenType type = lookup_identifier(lexeme);
        token = make_token(type, lexeme, line_);
      } else if (std::isdigit(current_char_)) {
        const std::string lexeme = read_number();
        token = make_token(TokenType::NUMBER, lexeme, line_);
      } else {
        consume_char();
        token = make_token(TokenType::ERROR, std::string(1, current_char_), line_);
      }
      return token;
  }
  consume_char();
  return token;
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
  const std::size_t start_position = read_position_;
  while (std::isalnum(current_char_) || current_char_ == '_') {
    consume_char();
  }
  return input_.substr(start_position, read_position_ - start_position);
}

std::string Lexer::read_number() {
  const std::size_t start_position = read_position_;
  while (std::isdigit(current_char_)) {
    consume_char();
  }
  return input_.substr(start_position, read_position_ - start_position);
}

void Lexer::consume_whitespace() {
  while (true) {
    switch (peek_char()) {
      case '\n':
        ++line_;
      [[fallthrough]]
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