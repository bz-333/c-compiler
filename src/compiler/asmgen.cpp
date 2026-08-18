#include "compiler/asmgen.hpp"

#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "compiler/compiler.hpp"

namespace compiler {

namespace {

Operand convert_val(const TackyVal& val) {
  return std::visit(
      Overloaded{
          [](const TackyConstant& c) -> Operand { return Imm{c.value}; },
          [](const TackyVar& v) -> Operand { return Pseudo{v.name}; },
      },
      val);
}

AsmUnaryOp convert_op(const TackyUnaryOp& op) {
  return std::visit(
      Overloaded{
          [](const TackyNegate&) { return AsmUnaryOp::Negate; },
          [](const TackyComplement&) { return AsmUnaryOp::Complement; },
      },
      op);
}

AssemblyFunction convert_tacky(const TackyProgram& program) {
  std::vector<Instruction> instructions;
  for (const TackyInstruction& instr : program.func.body) {
    std::visit(
        Overloaded{
            [&](const TackyReturn& r) {
              instructions.push_back(Mov{convert_val(r.src), Reg::Ax});
              instructions.push_back(Ret{});
            },
            [&](const TackyUnary& u) {
              Operand dst = convert_val(u.dst);
              instructions.push_back(Mov{convert_val(u.src), dst});
              instructions.push_back(AsmUnary{convert_op(u.op), dst});
            },
            [&](const TackyBinary&) {
              throw CompileError(
                  "binary instructions not yet supported by codegen");
            }},
        instr);
  }
  return AssemblyFunction{program.func.name, std::move(instructions)};
}

bool is_memory(const Operand& operand) {
  return std::holds_alternative<Stack>(operand);
}

Operand replace_pseudo(Operand operand, std::map<std::string, Stack>& mapping,
                       int& offset) {
  if (const auto* pseudo = std::get_if<Pseudo>(&operand)) {
    auto it = mapping.find(pseudo->name);
    if (it == mapping.end()) {
      offset += 4;
      it = mapping.emplace(pseudo->name, Stack{-offset}).first;
    }
    return it->second;
  }
  return operand;
}

int replace_pseudos(std::vector<Instruction>& instructions) {
  std::map<std::string, Stack> mapping;
  int offset = 0;
  for (Instruction& instr : instructions) {
    std::visit(
        Overloaded{
            [&](Mov& mov) {
              mov.src = replace_pseudo(mov.src, mapping, offset);
              mov.dst = replace_pseudo(mov.dst, mapping, offset);
            },
            [&](AsmUnary& unary) {
              unary.operand = replace_pseudo(unary.operand, mapping, offset);
            },
            [&](Ret&) {},
            [&](AllocateStack&) {},
        },
        instr);
  }
  return offset;
}

void fixup(std::vector<Instruction>& instructions, int stack_size) {
  std::vector<Instruction> fixed;
  fixed.reserve(instructions.size() + 1);
  fixed.push_back(AllocateStack{stack_size});
  for (Instruction& instr : instructions) {
    if (const auto* mov = std::get_if<Mov>(&instr);
        mov != nullptr && is_memory(mov->src) && is_memory(mov->dst)) {
      fixed.push_back(Mov{mov->src, Reg::R10});
      fixed.push_back(Mov{Reg::R10, mov->dst});
    } else {
      fixed.push_back(std::move(instr));
    }
  }
  instructions = std::move(fixed);
}

}  // namespace

AssemblyFunction generate_assembly(const TackyProgram& program) {
  AssemblyFunction func = convert_tacky(program);
  int stack_size = replace_pseudos(func.instructions);
  fixup(func.instructions, stack_size);
  return func;
}

}  // namespace compiler
