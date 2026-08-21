#pragma once

#include <string>
#include <variant>
#include <vector>

namespace compiler {

struct TackyConstant {
  int value;
};

struct TackyVar {
  std::string name;
};

using TackyVal = std::variant<TackyConstant, TackyVar>;

struct TackyNegate {};

struct TackyComplement {};

struct TackyNot {};

using TackyUnaryOp = std::variant<TackyNegate, TackyComplement, TackyNot>;

struct TackyReturn {
  TackyVal src;
};

struct TackyUnary {
  TackyUnaryOp op;
  TackyVal src;
  TackyVal dst;
};

struct TackyAdd {};

struct TackySubtract {};

struct TackyMultiply {};

struct TackyDivide {};

struct TackyRemainder {};

struct TackyBitAnd {};

struct TackyBitOr {};

struct TackyBitXor {};

struct TackyLeftShift {};

struct TackyRightShift {};

struct TackyEqual {};

struct TackyNotEqual {};

struct TackyLessThan {};

struct TackyLessEqual {};

struct TackyGreaterThan {};

struct TackyGreaterEqual {};

using TackyBinaryOp = std::variant<TackyAdd, TackySubtract, TackyMultiply,
                                   TackyDivide, TackyRemainder, TackyBitAnd,
                                   TackyBitOr, TackyBitXor, TackyLeftShift,
                                   TackyRightShift, TackyEqual, TackyNotEqual,
                                   TackyLessThan, TackyLessEqual,
                                   TackyGreaterThan, TackyGreaterEqual>;

struct TackyBinary {
  TackyBinaryOp op;
  TackyVal src1;
  TackyVal src2;
  TackyVal dst;
};

struct TackyCopy {
  TackyVal src;
  TackyVal dst;
};

struct TackyJump {
  std::string target;
};

struct TackyJumpIfZero {
  TackyVal condition;
  std::string target;
};

struct TackyJumpIfNotZero {
  TackyVal condition;
  std::string target;
};

struct TackyLabel {
  std::string name;
};

using TackyInstruction =
    std::variant<TackyReturn, TackyUnary, TackyBinary, TackyCopy, TackyJump,
                 TackyJumpIfZero, TackyJumpIfNotZero, TackyLabel>;

struct TackyFunction {
  std::string name;
  std::vector<TackyInstruction> body;
};

struct TackyProgram {
  TackyFunction func;
};

}  // namespace compiler
