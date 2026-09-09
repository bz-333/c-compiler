#include "compiler/check_labels.hpp"

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "compiler/compiler.hpp"

namespace compiler {

namespace {

class LabelChecker {
 public:
  void check(const Program& program) {
    for (const BlockItem& item : program.func.body) {
      std::visit(
          Overloaded{
              [&](const Declaration&) {},
              [&](const Stmt& stmt) { collect_statement(stmt); },
          },
          item);
    }
    for (const std::string& target : gotos_) {
      if (labels_.count(target) == 0) {
        throw CompileError("undefined label: " + target);
      }
    }
  }

 private:
  std::set<std::string> labels_;
  std::vector<std::string> gotos_;

  void collect_statement(const Stmt& stmt) {
    std::visit(
        Overloaded{
            [&](const Return&) {},
            [&](const Expression&) {},
            [&](const Null&) {},
            [&](const std::unique_ptr<If>& i) {
              collect_statement(i->then);
              if (i->else_) {
                collect_statement(*i->else_);
              }
            },
            [&](const Goto& g) { gotos_.push_back(g.name); },
            [&](const std::unique_ptr<Labeled>& l) {
              if (labels_.count(l->name) != 0) {
                throw CompileError("duplicate label: " + l->name);
              }
              labels_.insert(l->name);
              collect_statement(l->statement);
            },
        },
        stmt);
  }
};

}  // namespace

Program check_labels(Program program) {
  LabelChecker().check(program);
  return program;
}

}  // namespace compiler