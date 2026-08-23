#pragma once

#include <memory>
#include <optional>
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

struct Assignment;

struct Var {
  std::string name;
};

using Exp = std::variant<Constant, Var, std::unique_ptr<Unary>,
                         std::unique_ptr<Binary>, std::unique_ptr<Assignment>>;

struct Unary {
  UnaryOp op;
  Exp operand;
};

struct Binary {
  BinaryOp op;
  Exp lhs;
  Exp rhs;
};

struct Assignment {
  Exp lhs;
  Exp rhs;
};

struct Return {
  Exp exp;
};

struct Expression {
  Exp exp;
};

struct Null {};

using Stmt = std::variant<Return, Expression, Null>;

struct Declaration {
  std::string name;
  std::optional<Exp> init;
};

using BlockItem = std::variant<Stmt, Declaration>;

struct Function {
  std::string name;
  std::vector<BlockItem> body;
};

struct Program {
  Function func;
};

Program parse(const std::vector<Token>& tokens);

}  // namespace compiler
