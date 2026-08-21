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
          [&](const TackyNot&) { os << "Not"; },
      },
      op);
}

void print_binop(const TackyBinaryOp& op, std::ostream& os) {
  std::visit(
      Overloaded{
          [&](const TackyAdd&) { os << "Add"; },
          [&](const TackySubtract&) { os << "Subtract"; },
          [&](const TackyMultiply&) { os << "Multiply"; },
          [&](const TackyDivide&) { os << "Divide"; },
          [&](const TackyRemainder&) { os << "Remainder"; },
          [&](const TackyBitAnd&) { os << "BitAnd"; },
          [&](const TackyBitOr&) { os << "BitOr"; },
          [&](const TackyBitXor&) { os << "BitXor"; },
          [&](const TackyLeftShift&) { os << "LeftShift"; },
          [&](const TackyRightShift&) { os << "RightShift"; },
          [&](const TackyEqual&) { os << "Equal"; },
          [&](const TackyNotEqual&) { os << "NotEqual"; },
          [&](const TackyLessThan&) { os << "LessThan"; },
          [&](const TackyLessEqual&) { os << "LessEqual"; },
          [&](const TackyGreaterThan&) { os << "GreaterThan"; },
          [&](const TackyGreaterEqual&) { os << "GreaterEqual"; },
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
          },
          [&](const TackyBinary& b) {
            indent(os, depth);
            os << "TackyBinary(\n";
            indent(os, depth + 1);
            print_binop(b.op, os);
            os << ",\n";
            indent(os, depth + 1);
            print_val(b.src1, os);
            os << ",\n";
            indent(os, depth + 1);
            print_val(b.src2, os);
            os << ",\n";
            indent(os, depth + 1);
            print_val(b.dst, os);
            os << '\n';
            indent(os, depth);
            os << ')';
          },
          [&](const TackyCopy& c) {
            indent(os, depth);
            os << "TackyCopy(\n";
            indent(os, depth + 1);
            print_val(c.src, os);
            os << ",\n";
            indent(os, depth + 1);
            print_val(c.dst, os);
            os << '\n';
            indent(os, depth);
            os << ')';
          },
          [&](const TackyJump& j) {
            indent(os, depth);
            os << "TackyJump(\"" << j.target << "\")";
          },
          [&](const TackyJumpIfZero& j) {
            indent(os, depth);
            os << "TackyJumpIfZero(\n";
            indent(os, depth + 1);
            print_val(j.condition, os);
            os << ",\n";
            indent(os, depth + 1);
            os << '"' << j.target << '"';
            os << '\n';
            indent(os, depth);
            os << ')';
          },
          [&](const TackyJumpIfNotZero& j) {
            indent(os, depth);
            os << "TackyJumpIfNotZero(\n";
            indent(os, depth + 1);
            print_val(j.condition, os);
            os << ",\n";
            indent(os, depth + 1);
            os << '"' << j.target << '"';
            os << '\n';
            indent(os, depth);
            os << ')';
          },
          [&](const TackyLabel& l) {
            indent(os, depth);
            os << "TackyLabel(\"" << l.name << "\")";
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
