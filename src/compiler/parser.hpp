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

struct Add {};

struct Subtract {};

struct Multiply {};

struct Divide {};

struct Remainder {};

struct BitAnd {};

struct BitOr {};

struct BitXor {};

struct LeftShift {};

struct RightShift {};

using BinaryOp = std::variant<Add, Subtract, Multiply, Divide, Remainder,
                              BitAnd, BitOr, BitXor, LeftShift, RightShift>;

struct Unary;

struct Binary;

using Exp = std::variant<Constant, std::unique_ptr<Unary>, std::unique_ptr<Binary>>;

struct Unary {
  UnaryOp op;
  Exp operand;
};

struct Binary {
  BinaryOp op;
  Exp lhs;
  Exp rhs;
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
