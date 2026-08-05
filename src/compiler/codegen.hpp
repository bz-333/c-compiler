#pragma once

#include <string>
#include <variant>
#include <vector>

#include "compiler/parser.hpp"

namespace compiler {

enum class Reg {
  Eax,
};

struct Imm {
  int value;
};

using Operand = std::variant<Imm, Reg>;

struct Mov {
  Operand src;
  Operand dst;
};

struct Ret {};

using Instruction = std::variant<Mov, Ret>;

struct AssemblyFunction {
  std::string name;
  std::vector<Instruction> instructions;
};

std::string codegen(const Program& program);

}  // namespace compiler
