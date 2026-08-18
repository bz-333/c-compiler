#include "compiler/emit.hpp"

#include <sstream>
#include <string>
#include <variant>

#include "compiler/compiler.hpp"

namespace compiler {

namespace {

std::string emit_operand(const Operand& operand) {
  return std::visit(
      Overloaded{
          [](const Imm& imm) { return "$" + std::to_string(imm.value); },
          [](Reg reg) {
            switch (reg) {
              case Reg::Ax:
                return std::string{"%eax"};
              case Reg::Dx:
                return std::string{"%edx"};
              case Reg::R10:
                return std::string{"%r10d"};
              case Reg::R11:
                return std::string{"%r11d"};
            }
            return std::string{"%eax"};
          },
          [](const Stack& stack) {
            return std::to_string(stack.offset) + "(%rbp)";
          },
          [](const Pseudo&) -> std::string {
            throw CompileError("internal error: pseudo register survived to emission");
          },
      },
      operand);
}

std::string emit_instruction(const Instruction& instruction) {
  return std::visit(
      Overloaded{
          [](const Mov& mov) {
            return "    movl " + emit_operand(mov.src) + ", " +
                   emit_operand(mov.dst);
          },
          [](const AsmUnary& unary) {
            std::string op = unary.op == AsmUnaryOp::Negate ? "negl" : "notl";
            return "    " + op + " " + emit_operand(unary.operand);
          },
          [](const AsmBinary& binary) {
            std::string op;
            switch (binary.op) {
              case AsmBinaryOp::Add:
                op = "addl";
                break;
              case AsmBinaryOp::Sub:
                op = "subl";
                break;
              case AsmBinaryOp::Mult:
                op = "imull";
                break;
            }
            return "    " + op + " " + emit_operand(binary.src) + ", " +
                   emit_operand(binary.dst);
          },
          [](const Idiv& idiv) {
            return "    idivl " + emit_operand(idiv.operand);
          },
          [](const Cdq&) { return std::string{"    cdq"}; },
          [](const AllocateStack& alloc) {
            return "    subq $" + std::to_string(alloc.num_bytes) + ", %rsp";
          },
          [](const Ret&) {
            return std::string{"    movq %rbp, %rsp\n    popq %rbp\n    ret"};
          },
      },
      instruction);
}

}  // namespace

std::string emit_assembly(const AssemblyFunction& func) {
  std::ostringstream out;
  out << "    .globl " << func.name << '\n'
      << func.name << ":\n"
      << "    pushq %rbp\n"
      << "    movq %rsp, %rbp\n";
  for (const Instruction& instruction : func.instructions) {
    out << emit_instruction(instruction) << '\n';
  }
  out << "    .section .note.GNU-stack,\"\",@progbits\n";
  return out.str();
}

}  // namespace compiler
