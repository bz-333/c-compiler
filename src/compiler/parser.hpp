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

struct CompoundAssignment;

struct Prefix;

struct Postfix;

struct Conditional;

struct Var {
  std::string name;
};

struct Increment {};

struct Decrement {};

using PrefixOp = std::variant<Increment, Decrement>;

using PostfixOp = std::variant<Increment, Decrement>;

using Exp = std::variant<Constant, Var, std::unique_ptr<Unary>,
                         std::unique_ptr<Binary>, std::unique_ptr<Assignment>,
                         std::unique_ptr<CompoundAssignment>,
                         std::unique_ptr<Prefix>, std::unique_ptr<Postfix>,
                         std::unique_ptr<Conditional>>;

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

struct CompoundAssignment {
  BinaryOp op;
  Exp lhs;
  Exp rhs;
};

struct Prefix {
  PrefixOp op;
  Exp operand;
};

struct Postfix {
  PostfixOp op;
  Exp operand;
};

struct Conditional {
  Exp condition;
  Exp then_exp;
  Exp else_exp;
};

struct Return {
  Exp exp;
};

struct Expression {
  Exp exp;
};

struct Null {};

struct If;

using Stmt = std::variant<Return, Expression, Null, std::unique_ptr<If>>;

struct If {
  Exp condition;
  Stmt then;
  std::optional<Stmt> else_;
};

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
