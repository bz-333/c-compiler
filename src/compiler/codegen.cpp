#include "compiler/codegen.hpp"

#include <ostream>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "matchit.h"
#include "compiler/compiler.hpp"

namespace compiler {

namespace {

using namespace matchit;

std::string emit_operand(const Operand& operand) {
  return std::visit(
      Overloaded{
          [](const Imm& imm) {
            return "$" + std::to_string(imm.value);
          },
          [](Reg) { return std::string{"%eax"}; },
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
          [](const Ret&) { return std::string{"    ret"}; },
      },
      instruction);
}

std::string emit_program(const AssemblyFunction& func) {
  std::ostringstream out;
  out << "    .globl " << func.name << '\n'
      << func.name << ":\n";
  for (const Instruction& instruction : func.instructions) {
    out << emit_instruction(instruction) << '\n';
  }
  out << "    .section .note.GNU-stack,\"\",@progbits\n";
  return out.str();
}

AssemblyFunction generate_assembly(const Program& program) {
  std::vector<Instruction> instructions;

  Id<int> value;
  match(program.func.body)(
      pattern | as<Return>(
                    app(&Return::exp,
                        as<Constant>(app(&Constant::value, value)))) = [&] {
        instructions.push_back(Mov{Imm{*value}, Reg::Eax});
        instructions.push_back(Ret{});
      },
      pattern | _ = [] {
        throw CompileError("internal error: unsupported statement");
      });

  return AssemblyFunction{program.func.name, std::move(instructions)};
}

}  // namespace

std::string codegen(const Program& program) {
  return emit_program(generate_assembly(program));
}

}  // namespace compiler
