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

using TackyUnaryOp = std::variant<TackyNegate, TackyComplement>;

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

using TackyBinaryOp = std::variant<TackyAdd, TackySubtract, TackyMultiply,
                                   TackyDivide, TackyRemainder, TackyBitAnd,
                                   TackyBitOr, TackyBitXor, TackyLeftShift,
                                   TackyRightShift>;

struct TackyBinary {
  TackyBinaryOp op;
  TackyVal src1;
  TackyVal src2;
  TackyVal dst;
};

using TackyInstruction = std::variant<TackyReturn, TackyUnary, TackyBinary>;

struct TackyFunction {
  std::string name;
  std::vector<TackyInstruction> body;
};

struct TackyProgram {
  TackyFunction func;
};

}  // namespace compiler
