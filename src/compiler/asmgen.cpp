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
            [&](const TackyBinary& b) {
              Operand src1 = convert_val(b.src1);
              Operand src2 = convert_val(b.src2);
              Operand dst = convert_val(b.dst);
              std::visit(
                  Overloaded{
                      [&](const TackyAdd&) {
                        instructions.push_back(Mov{src1, dst});
                        instructions.push_back(
                            AsmBinary{AsmBinaryOp::Add, src2, dst});
                      },
                      [&](const TackySubtract&) {
                        instructions.push_back(Mov{src1, dst});
                        instructions.push_back(
                            AsmBinary{AsmBinaryOp::Sub, src2, dst});
                      },
                      [&](const TackyMultiply&) {
                        instructions.push_back(Mov{src1, dst});
                        instructions.push_back(
                            AsmBinary{AsmBinaryOp::Mult, src2, dst});
                      },
                      [&](const TackyDivide&) {
                        instructions.push_back(Mov{src1, Reg::Ax});
                        instructions.push_back(Cdq{});
                        instructions.push_back(Idiv{src2});
                        instructions.push_back(Mov{Reg::Ax, dst});
                      },
                      [&](const TackyRemainder&) {
                        instructions.push_back(Mov{src1, Reg::Ax});
                        instructions.push_back(Cdq{});
                        instructions.push_back(Idiv{src2});
                        instructions.push_back(Mov{Reg::Dx, dst});
                      },
                  },
                  b.op);
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
            [&](AsmBinary& binary) {
              binary.src = replace_pseudo(binary.src, mapping, offset);
              binary.dst = replace_pseudo(binary.dst, mapping, offset);
            },
            [&](Idiv& idiv) {
              idiv.operand = replace_pseudo(idiv.operand, mapping, offset);
            },
            [&](Cdq&) {},
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
    bool rewritten = false;
    std::visit(
        Overloaded{
            [&](Mov& mov) {
              if (is_memory(mov.src) && is_memory(mov.dst)) {
                fixed.push_back(Mov{mov.src, Reg::R10});
                fixed.push_back(Mov{Reg::R10, mov.dst});
                rewritten = true;
              }
            },
            [&](AsmBinary& binary) {
              if (binary.op == AsmBinaryOp::Mult && is_memory(binary.dst)) {
                fixed.push_back(Mov{binary.dst, Reg::R11});
                fixed.push_back(
                    AsmBinary{binary.op, binary.src, Reg::R11});
                fixed.push_back(Mov{Reg::R11, binary.dst});
                rewritten = true;
              } else if (is_memory(binary.src) && is_memory(binary.dst)) {
                fixed.push_back(Mov{binary.src, Reg::R10});
                fixed.push_back(
                    AsmBinary{binary.op, Reg::R10, binary.dst});
                rewritten = true;
              }
            },
            [&](Idiv& idiv) {
              if (std::holds_alternative<Imm>(idiv.operand)) {
                fixed.push_back(Mov{idiv.operand, Reg::R10});
                fixed.push_back(Idiv{Reg::R10});
                rewritten = true;
              }
            },
            [&](auto&) {},
        },
        instr);
    if (!rewritten) {
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
