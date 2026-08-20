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

struct Not {};

using UnaryOp = std::variant<Negate, Complement, Not>;

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

struct LessThan {};

struct LessEqual {};

struct GreaterThan {};

struct GreaterEqual {};

struct Equal {};

struct NotEqual {};

struct LogicalAnd {};

struct LogicalOr {};

using BinaryOp = std::variant<Add, Subtract, Multiply, Divide, Remainder,
                              BitAnd, BitOr, BitXor, LeftShift, RightShift,
                              LessThan, LessEqual, GreaterThan, GreaterEqual,
                              Equal, NotEqual, LogicalAnd, LogicalOr>;

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
