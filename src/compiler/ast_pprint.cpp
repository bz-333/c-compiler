#include "compiler/ast_pprint.hpp"

#include <ostream>
#include <string>
#include <variant>

#include "compiler/compiler.hpp"

namespace compiler {

namespace {

void indent(std::ostream& os, int depth) {
  for (int i = 0; i < depth; ++i) {
    os << "  ";
  }
}

void print_unary_op(const UnaryOp& op, std::ostream& os) {
  std::visit(Overloaded{
                 [&](const Negate&) { os << "Negate"; },
                 [&](const Complement&) { os << "Complement"; },
                 [&](const Not&) { os << "Not"; },
             },
             op);
}

void print_prefix_op(const PrefixOp& op, std::ostream& os) {
  std::visit(Overloaded{
                 [&](const Increment&) { os << "Increment"; },
                 [&](const Decrement&) { os << "Decrement"; },
             },
             op);
}

void print_postfix_op(const PostfixOp& op, std::ostream& os) {
  std::visit(Overloaded{
                 [&](const Increment&) { os << "Increment"; },
                 [&](const Decrement&) { os << "Decrement"; },
             },
             op);
}

void print_binary_op(const BinaryOp& op, std::ostream& os) {
  std::visit(Overloaded{
                 [&](const Add&) { os << "Add"; },
                 [&](const Subtract&) { os << "Subtract"; },
                 [&](const Multiply&) { os << "Multiply"; },
                 [&](const Divide&) { os << "Divide"; },
                 [&](const Remainder&) { os << "Remainder"; },
                 [&](const BitAnd&) { os << "BitAnd"; },
                 [&](const BitOr&) { os << "BitOr"; },
                 [&](const BitXor&) { os << "BitXor"; },
                 [&](const LeftShift&) { os << "LeftShift"; },
                 [&](const RightShift&) { os << "RightShift"; },
                 [&](const LessThan&) { os << "LessThan"; },
                 [&](const LessEqual&) { os << "LessEqual"; },
                 [&](const GreaterThan&) { os << "GreaterThan"; },
                 [&](const GreaterEqual&) { os << "GreaterEqual"; },
                 [&](const Equal&) { os << "Equal"; },
                 [&](const NotEqual&) { os << "NotEqual"; },
                 [&](const LogicalAnd&) { os << "LogicalAnd"; },
                 [&](const LogicalOr&) { os << "LogicalOr"; },
             },
             op);
}

void print_exp(const Exp& exp, std::ostream& os) {
  std::visit(
      Overloaded{
          [&](const Constant& c) { os << "Constant(" << c.value << ")"; },
          [&](const Var& v) { os << "Var(\"" << v.name << "\")"; },
          [&](const std::unique_ptr<Unary>& u) {
            os << "Unary(";
            print_unary_op(u->op, os);
            os << ", ";
            print_exp(u->operand, os);
            os << ')';
          },
          [&](const std::unique_ptr<Binary>& b) {
            os << "Binary(";
            print_binary_op(b->op, os);
            os << ", ";
            print_exp(b->lhs, os);
            os << ", ";
            print_exp(b->rhs, os);
            os << ')';
          },
          [&](const std::unique_ptr<Assignment>& a) {
            os << "Assignment(";
            print_exp(a->lhs, os);
            os << ", ";
            print_exp(a->rhs, os);
            os << ')';
          },
          [&](const std::unique_ptr<CompoundAssignment>& c) {
            os << "CompoundAssignment(";
            print_binary_op(c->op, os);
            os << ", ";
            print_exp(c->lhs, os);
            os << ", ";
            print_exp(c->rhs, os);
            os << ')';
          },
          [&](const std::unique_ptr<Prefix>& p) {
            os << "Prefix(";
            print_prefix_op(p->op, os);
            os << ", ";
            print_exp(p->operand, os);
            os << ')';
          },
          [&](const std::unique_ptr<Postfix>& p) {
            os << "Postfix(";
            print_postfix_op(p->op, os);
            os << ", ";
            print_exp(p->operand, os);
            os << ')';
          },
          [&](const std::unique_ptr<Conditional>& c) {
            os << "Conditional(";
            print_exp(c->condition, os);
            os << ", ";
            print_exp(c->then_exp, os);
            os << ", ";
            print_exp(c->else_exp, os);
            os << ')';
          }},
      exp);
}

void print_stmt(const Stmt& stmt, std::ostream& os, int depth) {
  std::visit(
      Overloaded{
          [&](const Return& r) {
            indent(os, depth);
            os << "Return(\n";
            indent(os, depth + 1);
            print_exp(r.exp, os);
            os << '\n';
            indent(os, depth);
            os << ')';
          },
          [&](const Expression& e) {
            indent(os, depth);
            os << "Expression(\n";
            indent(os, depth + 1);
            print_exp(e.exp, os);
            os << '\n';
            indent(os, depth);
            os << ')';
          },
          [&](const Null&) {
            indent(os, depth);
            os << "Null";
          },
          [&](const Goto& g) {
            indent(os, depth);
            os << "Goto(\"" << g.name << "\")";
          },
          [&](const std::unique_ptr<Labeled>& l) {
            indent(os, depth);
            os << "Labeled(\n";
            indent(os, depth + 1);
            os << "name=\"" << l->name << "\",\n";
            indent(os, depth + 1);
            os << "statement=\n";
            print_stmt(l->statement, os, depth + 2);
            os << '\n';
            indent(os, depth);
            os << ')';
          },
          [&](const std::unique_ptr<If>& i) {
            indent(os, depth);
            os << "If(\n";
            indent(os, depth + 1);
            os << "condition=";
            print_exp(i->condition, os);
            os << ",\n";
            indent(os, depth + 1);
            os << "then=\n";
            print_stmt(i->then, os, depth + 2);
            os << ",\n";
            indent(os, depth + 1);
            os << "else=";
            if (i->else_) {
              os << '\n';
              print_stmt(*i->else_, os, depth + 2);
            } else {
              os << "(none)";
            }
            os << '\n';
            indent(os, depth);
            os << ')';
          }},
      stmt);
}

void print_declaration(const Declaration& decl, std::ostream& os, int depth) {
  indent(os, depth);
  os << "Declaration(\n";
  indent(os, depth + 1);
  os << "name=\"" << decl.name << "\",\n";
  indent(os, depth + 1);
  os << "init=";
  if (decl.init) {
    print_exp(*decl.init, os);
  } else {
    os << "(none)";
  }
  os << '\n';
  indent(os, depth);
  os << ')';
}

void print_block_item(const BlockItem& item, std::ostream& os, int depth) {
  std::visit(Overloaded{
                 [&](const Stmt& stmt) { print_stmt(stmt, os, depth); },
                 [&](const Declaration& decl) {
                   print_declaration(decl, os, depth);
                 },
             },
             item);
}

void print_function(const Function& func, std::ostream& os, int depth) {
  os << "Function(\n";
  indent(os, depth + 1);
  os << "name=\"" << func.name << "\",\n";
  indent(os, depth + 1);
  os << "body=[\n";
  for (const BlockItem& item : func.body) {
    print_block_item(item, os, depth + 2);
    os << '\n';
  }
  indent(os, depth + 1);
  os << "]\n";
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
