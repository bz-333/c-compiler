#pragma once

#include <string>
#include <variant>
#include <vector>

#include "compiler/lexer.hpp"

namespace compiler {

struct Constant {
  int value;
};

using Exp = std::variant<Constant>;

struct Return {
  Exp exp;
};

using Stmt = std::variant<Return>;

struct Function {
  std::string name;
  Stmt body;
};

struct Program {
  Function func;
};

Program parse(const std::vector<Token>& tokens);

}  // namespace compiler
