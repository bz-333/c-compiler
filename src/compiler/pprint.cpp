#include "compiler/pprint.hpp"

#include <ostream>
#include <string>
#include <variant>

namespace compiler {

namespace {

template <typename... Ts>
struct Overloaded : Ts... {
  using Ts::operator()...;
};

template <typename... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

void indent(std::ostream& os, int depth) {
  for (int i = 0; i < depth; ++i) {
    os << "  ";
  }
}

void print_exp(const Exp& exp, std::ostream& os, int depth) {
  std::visit(
      Overloaded{[&](const Constant& c) { os << "Constant(" << c.value << ")"; }},
      exp);
}

void print_stmt(const Stmt& stmt, std::ostream& os, int depth) {
  std::visit(
      Overloaded{[&](const Return& r) {
        os << "Return(\n";
        indent(os, depth + 1);
        print_exp(r.exp, os, depth + 1);
        os << '\n';
        indent(os, depth);
        os << ')';
      }},
      stmt);
}

void print_function(const Function& func, std::ostream& os, int depth) {
  os << "Function(\n";
  indent(os, depth + 1);
  os << "name=\"" << func.name << "\",\n";
  indent(os, depth + 1);
  os << "body=";
  print_stmt(func.body, os, depth + 1);
  os << '\n';
  indent(os, depth);
  os << ')';
}

void print_program(const Program& program, std::ostream& os, int depth) {
  os << "Program(\n";
  indent(os, depth + 1);
  print_function(program.func, os, depth + 1);
  os << '\n';
  indent(os, depth);
  os << ')';
}

}  // namespace

void pretty_print(const Program& program, std::ostream& os) {
  print_program(program, os, 0);
  os << '\n';
}

}  // namespace compiler
