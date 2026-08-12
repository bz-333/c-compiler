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

using TackyInstruction = std::variant<TackyReturn, TackyUnary>;

struct TackyFunction {
  std::string name;
  std::vector<TackyInstruction> body;
};

struct TackyProgram {
  TackyFunction func;
};

}  // namespace compiler
