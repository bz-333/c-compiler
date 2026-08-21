#pragma once

#include <string>
#include <variant>
#include <vector>

#include "compiler/tacky.hpp"

namespace compiler {

enum class Reg {
  Ax,
  Dx,
  Ecx,
  R10,
  R11,
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

enum class AsmBinaryOp {
  Add,
  Sub,
  Mult,
  BitAnd,
  BitOr,
  BitXor,
  LeftShift,
  RightShift,
};

struct AsmBinary {
  AsmBinaryOp op;
  Operand src;
  Operand dst;
};

struct Idiv {
  Operand operand;
};

struct Cdq {};

enum class CondCode {
  E,
  NE,
  G,
  GE,
  L,
  LE,
};

struct Cmp {
  Operand src;
  Operand dst;
};

struct Jmp {
  std::string target;
};

struct JmpCC {
  CondCode code;
  std::string target;
};

struct SetCC {
  CondCode code;
  Operand operand;
};

struct Label {
  std::string name;
};

struct Ret {};

struct AllocateStack {
  int num_bytes;
};

using Instruction = std::variant<Mov, AsmUnary, AsmBinary, Cmp, Idiv, Cdq,
                                 Jmp, JmpCC, SetCC, Label, Ret, AllocateStack>;

struct AssemblyFunction {
  std::string name;
  std::vector<Instruction> instructions;
};

std::string codegen(const TackyProgram& program);

}  // namespace compiler
