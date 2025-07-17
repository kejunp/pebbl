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