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
#include <unordered_map>

enum class TokenType {
  // Operators
  LPAREN,
  RPAREN,
  LBRACE,
  RBRACE,
  COMMA,
  DOT,
  SEMICOLON,
  PLUS,
  MINUS,
  ASTERISK,
  SLASH,
  BANG,
  NOT_EQUAL,
  ASSIGN,
  EQUAL,
  LESS,
  GREATER,
  LESS_EQUAL,
  GREATER_EQUAL,

  // Literals
  IDENTIFIER,
  STRING,
  NUMBER,

  // Keywords
  AND,
  OR,
  IF,
  ELSE,
  TRUE,
  FALSE,
  FOR,
  WHILE,
  FUNC,
  RETURN,
  LET,
  INHERITS,
  CLASS,
  SUPER,
  THIS,
  NIL,
  ERROR,
  EOF_TYPE
};

struct Token {
  TokenType type;      ///< The TokenType
  std::string lexeme;  ///< The literal (The actual source code that translates to the token)
  std::size_t line;    ///< The line that the token was on
};

inline TokenType lookup_identifier(const std::string& name) {
  static const std::unordered_map<std::string, TokenType> keywords = {
      {"and", TokenType::AND},
      {"or", TokenType::OR},
      {"if", TokenType::IF},
      {"else", TokenType::ELSE},
      {"true", TokenType::TRUE},
      {"false", TokenType::FALSE},
      {"for", TokenType::FOR},
      {"while", TokenType::WHILE},
      {"func", TokenType::FUNC},
      {"return", TokenType::RETURN},
      {"let", TokenType::LET},
      {"inherits", TokenType::INHERITS},
      {"class", TokenType::CLASS},
      {"super", TokenType::SUPER},
      {"this", TokenType::THIS},
      {"nil", TokenType::NIL}};
  if (auto it = keywords.find(name); it != keywords.end()) {
    return it->second;
  } else {
    return TokenType::IDENTIFIER;
  }
}