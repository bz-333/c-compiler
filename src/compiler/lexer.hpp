#pragma once

#include <string>
#include <vector>

namespace compiler {

struct Token {
  enum class Kind {
    Eof,
  };

  Kind kind = Kind::Eof;
};

std::vector<Token> lex(const std::string& source);

}  // namespace compiler
