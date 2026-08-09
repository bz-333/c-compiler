#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "compiler/lexer.hpp"

namespace compiler {

struct Constant {
  int value;
};

struct Negate {};

struct Complement {};

using UnaryOp = std::variant<Negate, Complement>;

struct Unary;

using Exp = std::variant<Constant, std::unique_ptr<Unary>>;

struct Unary {
  UnaryOp op;
  Exp operand;
};

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
