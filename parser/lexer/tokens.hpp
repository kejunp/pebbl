#pragma once

#include <string>

enum class TokenType {
  // Operators
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_LBRACE,
  TOKEN_RBRACE,
  TOKEN_COMMA,
  TOKEN_DOT,
  TOKEN_SEMICOLON,
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_ASTERISK,
  TOKEN_SLASH,
  TOKEN_BANG,
  TOKEN_NOT_EQUAL,
  TOKEN_ASSIGN,
  TOKEN_EQUAL,
  TOKEN_LESS,
  TOKEN_GREATER,
  TOKEN_LESS_EQUAL,
  TOKEN_GREATER_EQUAL,

  // Literals
  TOKEN_IDENTIFIER,
  TOKEN_STRING,
  TOKEN_NUMBER,

  // Keywords
  TOKEN_AND,
  TOKEN_OR,
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_TRUE,
  TOKEN_FALSE,
  TOKEN_FOR,
  TOKEN_WHILE,
  TOKEN_FUNC,
  TOKEN_RETURN,
  TOKEN_LET,
  TOKEN_PRINT,
  TOKEN_CLASS,
  TOKEN_SUPER,
  TOKEN_THIS,
  TOKEN_NIL,
  TOKEN_ERROR,
  TOKEN_EOF
};

struct Token {
  TokenType type;      ///< The TokenType
  std::string lexeme;  ///< The literal (The actual source code that translates to the token)
  std::size_t line;    ///< The line that the token was on
};