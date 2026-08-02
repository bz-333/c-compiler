#include "compiler/lexer.hpp"

#include <cctype>
#include <cstdlib>
#include <string>

#include "compiler/compiler.hpp"

namespace compiler {

namespace {

bool is_ident_start(char c) {
  return std::isalpha(static_cast<unsigned char>(c)) != 0 || c == '_';
}

bool is_ident_char(char c) {
  return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
}

Token::Kind keyword_kind(const std::string& word) {
  if (word == "int") {
    return Token::Kind::Keyword_Int;
  }
  if (word == "void") {
    return Token::Kind::Keyword_Void;
  }
  if (word == "return") {
    return Token::Kind::Keyword_Return;
  }
  return Token::Kind::Identifier;
}

}  // namespace

std::vector<Token> lex(const std::string& source) {
  std::vector<Token> tokens;
  std::size_t i = 0;

  while (i < source.size()) {
    const char c = source[i];

    if (std::isspace(static_cast<unsigned char>(c)) != 0) {
      ++i;
      continue;
    }

    if (is_ident_start(c)) {
      std::size_t start = i;
      while (i < source.size() && is_ident_char(source[i])) {
        ++i;
      }
      std::string word = source.substr(start, i - start);
      Token::Kind kind = keyword_kind(word);
      if (kind == Token::Kind::Identifier) {
        tokens.push_back({Token::Kind::Identifier, word});
      } else {
        tokens.push_back({kind});
      }
      continue;
    }

    if (std::isdigit(static_cast<unsigned char>(c)) != 0) {
      std::size_t start = i;
      while (i < source.size() &&
             std::isdigit(static_cast<unsigned char>(source[i])) != 0) {
        ++i;
      }
      if (i < source.size() && is_ident_char(source[i])) {
        throw CompileError("invalid constant: " + source.substr(start, i - start + 1));
      }
      std::string digits = source.substr(start, i - start);
      tokens.push_back({Token::Kind::Constant, "", std::stoi(digits)});
      continue;
    }

    switch (c) {
      case '(':
        tokens.push_back({Token::Kind::OpenParen});
        break;
      case ')':
        tokens.push_back({Token::Kind::CloseParen});
        break;
      case '{':
        tokens.push_back({Token::Kind::OpenBrace});
        break;
      case '}':
        tokens.push_back({Token::Kind::CloseBrace});
        break;
      case ';':
        tokens.push_back({Token::Kind::Semicolon});
        break;
      default:
        throw CompileError(std::string("unexpected character: '") + c + "'");
    }
    ++i;
  }

  tokens.push_back({Token::Kind::Eof});
  return tokens;
}

}  // namespace compiler
