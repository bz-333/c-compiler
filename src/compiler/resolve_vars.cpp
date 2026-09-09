#include "compiler/resolve_vars.hpp"

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "compiler/compiler.hpp"
#include "compiler/unique_names.hpp"

namespace compiler {

namespace {

class VarResolver {
 public:
  Program resolve(const Program& program) {
    std::vector<BlockItem> body;
    for (const BlockItem& item : program.func.body) {
      body.push_back(resolve_block_item(item));
    }
    return Program{Function{program.func.name, std::move(body)}};
  }

 private:
  std::map<std::string, std::string> variable_map_;

  BlockItem resolve_block_item(const BlockItem& item) {
    return std::visit(
        Overloaded{
            [&](const Declaration& d) -> BlockItem {
              return resolve_declaration(d);
            },
            [&](const Stmt& s) -> BlockItem { return resolve_statement(s); },
        },
        item);
  }

  Declaration resolve_declaration(const Declaration& decl) {
    if (variable_map_.count(decl.name) != 0) {
      throw CompileError("duplicate declaration of variable: " + decl.name);
    }
    std::string unique_name = make_unique_name(decl.name);
    variable_map_.emplace(decl.name, unique_name);
    std::optional<Exp> init;
    if (decl.init) {
      init = resolve_exp(*decl.init);
    }
    return Declaration{unique_name, std::move(init)};
  }

  Stmt resolve_statement(const Stmt& stmt) {
    return std::visit(
        Overloaded{
            [&](const Return& r) -> Stmt {
              return Return{resolve_exp(r.exp)};
            },
            [&](const Expression& e) -> Stmt {
              return Expression{resolve_exp(e.exp)};
            },
            [&](const Null&) -> Stmt { return Null{}; },
            [&](const std::unique_ptr<If>& i) -> Stmt {
              Exp condition = resolve_exp(i->condition);
              Stmt then = resolve_statement(i->then);
              std::optional<Stmt> else_stmt;
              if (i->else_) {
                else_stmt = resolve_statement(*i->else_);
              }
              return Stmt{std::make_unique<If>(If{std::move(condition),
                                                  std::move(then),
                                                  std::move(else_stmt)})};
            },
        },
        stmt);
  }

  Exp resolve_exp(const Exp& exp) {
    return std::visit(
        Overloaded{
            [&](const Constant& c) -> Exp { return c; },
            [&](const Var& v) -> Exp {
              auto it = variable_map_.find(v.name);
              if (it == variable_map_.end()) {
                throw CompileError("undeclared variable: " + v.name);
              }
              return Var{it->second};
            },
            [&](const std::unique_ptr<Unary>& u) -> Exp {
              return Exp{std::make_unique<Unary>(
                  Unary{u->op, resolve_exp(u->operand)})};
            },
            [&](const std::unique_ptr<Binary>& b) -> Exp {
              return Exp{std::make_unique<Binary>(Binary{
                  b->op, resolve_exp(b->lhs), resolve_exp(b->rhs)})};
            },
            [&](const std::unique_ptr<Assignment>& a) -> Exp {
              if (!std::holds_alternative<Var>(a->lhs)) {
                throw CompileError("invalid lvalue in assignment");
              }
              return Exp{std::make_unique<Assignment>(Assignment{
                  resolve_exp(a->lhs), resolve_exp(a->rhs)})};
            },
            [&](const std::unique_ptr<CompoundAssignment>& c) -> Exp {
              if (!std::holds_alternative<Var>(c->lhs)) {
                throw CompileError("invalid lvalue in assignment");
              }
              return Exp{std::make_unique<CompoundAssignment>(
                  CompoundAssignment{c->op, resolve_exp(c->lhs),
                                     resolve_exp(c->rhs)})};
            },
            [&](const std::unique_ptr<Prefix>& p) -> Exp {
              Exp operand = resolve_exp(p->operand);
              if (!std::holds_alternative<Var>(operand)) {
                throw CompileError("invalid lvalue in increment/decrement");
              }
              return Exp{std::make_unique<Prefix>(
                  Prefix{p->op, std::move(operand)})};
            },
            [&](const std::unique_ptr<Postfix>& p) -> Exp {
              Exp operand = resolve_exp(p->operand);
              if (!std::holds_alternative<Var>(operand)) {
                throw CompileError("invalid lvalue in increment/decrement");
              }
              return Exp{std::make_unique<Postfix>(
                  Postfix{p->op, std::move(operand)})};
            },
            [&](const std::unique_ptr<Conditional>& c) -> Exp {
              return Exp{std::make_unique<Conditional>(Conditional{
                  resolve_exp(c->condition), resolve_exp(c->then_exp),
                  resolve_exp(c->else_exp)})};
            },
        },
        exp);
  }
};

}  // namespace

Program resolve_vars(Program program) {
  return VarResolver().resolve(program);
}

}  // namespace compiler