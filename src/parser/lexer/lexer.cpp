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
 * @file lexer.cpp
 * @brief Implementation of the lexical analyzer for PEBBL source code
 */

#include "lexer.hpp"

#include <cctype>

Lexer::Lexer(std::string&& input) noexcept :
    input_(std::move(input)), position_{}, read_position_{0}, line_{1}, current_char_{'\0'} {
  consume_char();
}

Token Lexer::next_token() {
  consume_whitespace();

  if (current_char_ == '\0') {
    return make_token(TokenType::EOF_TYPE, "");
  }

  switch (current_char_) {
    case '(':
      return make_token(TokenType::LPAREN, "(");
    case ')':
      return make_token(TokenType::RPAREN, ")");
    case '{':
      return make_token(TokenType::LBRACE, "{");
    case '}':
      return make_token(TokenType::RBRACE, "}");
    case '[':
      return make_token(TokenType::LBRACKET, "[");
    case ']':
      return make_token(TokenType::RBRACKET, "]");
    case ',':
      return make_token(TokenType::COMMA, ",");
    case '.':
      return make_token(TokenType::DOT, ".");
    case ';':
      return make_token(TokenType::SEMICOLON, ";");
    case ':':
      return make_token(TokenType::COLON, ":");
    case '+':
      return make_token(TokenType::PLUS, "+");
    case '-':
      return make_token(TokenType::MINUS, "-");
    case '*':
      return make_token(TokenType::ASTERISK, "*");
    case '/':
      return make_token(TokenType::SLASH, "/");
    case '!':
      if (peek_char() == '=') {
        return make_token(TokenType::NOT_EQUAL, "!=");
      }
      return make_token(TokenType::BANG, "!");
    case '=':
      if (peek_char() == '=') {
        return make_token(TokenType::EQUAL, "==");
      }
      return make_token(TokenType::ASSIGN, "=");
    case '<':
      if (peek_char() == '=') {
        return make_token(TokenType::LESS_EQUAL, "<=");
      }
      return make_token(TokenType::LESS, "<");
    case '>':
      if (peek_char() == '=') {
        return make_token(TokenType::GREATER_EQUAL, ">=");
      }
      return make_token(TokenType::GREATER, ">");
    default:
      if (std::isalpha(current_char_) || current_char_ == '_') {
        const std::string lexeme = read_identifier();
        const TokenType type = lookup_identifier(lexeme);
        return Token{.type = type, .lexeme = lexeme, .line = line_};
      } else if (
          std::isdigit(current_char_) || (current_char_ == '.' && std::isdigit(peek_char()))) {
        const auto [type, lexeme] = read_number();
        return Token{.type = type, .lexeme = lexeme, .line = line_};
      } else if (current_char_ == '"') {
        const auto lexeme = read_string();
        return Token{.type = TokenType::STRING, .lexeme = lexeme, .line = line_};
      } else {
        return make_token(TokenType::ERROR, std::string(1, current_char_));
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
  const auto start_position = position_;
  while (std::isalnum(current_char_) || current_char_ == '_') {
    consume_char();
  }
  return input_.substr(start_position, read_position_ - start_position);
}

std::string Lexer::read_string() {
  const auto start_position = position_;
  do {
    consume_char();
  } while (current_char_ != '"');
  consume_char();
  return input_.substr(start_position, read_position_ - start_position);
}

void Lexer::consume_whitespace() {
  while (true) {
    switch (current_char_) {
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
