#include "compiler/tacky_pprint.hpp"

#include <ostream>
#include <string>
#include <variant>
#include <vector>

#include "compiler/compiler.hpp"

namespace compiler {

namespace {

void indent(std::ostream& os, int depth) {
  for (int i = 0; i < depth; ++i) {
    os << "  ";
  }
}

void print_val(const TackyVal& val, std::ostream& os) {
  std::visit(
      Overloaded{
          [&](const TackyConstant& c) { os << "Constant(" << c.value << ")"; },
          [&](const TackyVar& v) { os << "Var(\"" << v.name << "\")"; },
      },
      val);
}

void print_op(const TackyUnaryOp& op, std::ostream& os) {
  std::visit(
      Overloaded{
          [&](const TackyNegate&) { os << "Negate"; },
          [&](const TackyComplement&) { os << "Complement"; },
      },
      op);
}

void print_instruction(const TackyInstruction& instr, std::ostream& os,
                       int depth) {
  std::visit(
      Overloaded{
          [&](const TackyReturn& r) {
            indent(os, depth);
            os << "TackyReturn(\n";
            indent(os, depth + 1);
            print_val(r.src, os);
            os << '\n';
            indent(os, depth);
            os << ')';
          },
          [&](const TackyUnary& u) {
            indent(os, depth);
            os << "TackyUnary(\n";
            indent(os, depth + 1);
            print_op(u.op, os);
            os << ",\n";
            indent(os, depth + 1);
            print_val(u.src, os);
            os << ",\n";
            indent(os, depth + 1);
            print_val(u.dst, os);
            os << '\n';
            indent(os, depth);
            os << ')';
          }},
      instr);
}

void print_function(const TackyFunction& func, std::ostream& os, int depth) {
  indent(os, depth);
  os << "TackyFunction(\n";
  indent(os, depth + 1);
  os << "name=\"" << func.name << "\",\n";
  indent(os, depth + 1);
  os << "body=[\n";
  for (std::size_t i = 0; i < func.body.size(); ++i) {
    print_instruction(func.body[i], os, depth + 2);
    if (i + 1 < func.body.size()) {
      os << ',';
    }
    os << '\n';
  }
  indent(os, depth + 1);
  os << "]\n";
  indent(os, depth);
  os << ')';
}

void print_program(const TackyProgram& program, std::ostream& os, int depth) {
  indent(os, depth);
  os << "TackyProgram(\n";
  print_function(program.func, os, depth + 1);
  os << '\n';
  indent(os, depth);
  os << ')';
}

}  // namespace

void pretty_print(const TackyProgram& program, std::ostream& os) {
  print_program(program, os, 0);
  os << '\n';
}

}  // namespace compiler
