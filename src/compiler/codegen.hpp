#pragma once

#include <string>
#include <variant>
#include <vector>

#include "compiler/tacky.hpp"

namespace compiler {

enum class Reg {
  Ax,
  R10,
};

struct Imm {
  int value;
};

struct Pseudo {
  std::string name;
};

struct Stack {
  int offset;
};

using Operand = std::variant<Imm, Pseudo, Reg, Stack>;

struct Mov {
  Operand src;
  Operand dst;
};

enum class AsmUnaryOp {
  Negate,
  Complement,
};

struct AsmUnary {
  AsmUnaryOp op;
  Operand operand;
};

struct Ret {};

struct AllocateStack {
  int num_bytes;
};

using Instruction = std::variant<Mov, AsmUnary, Ret, AllocateStack>;

struct AssemblyFunction {
  std::string name;
  std::vector<Instruction> instructions;
};

std::string codegen(const TackyProgram& program);

}  // namespace compiler
