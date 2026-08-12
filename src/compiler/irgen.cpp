#include "compiler/irgen.hpp"

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "compiler/compiler.hpp"

namespace compiler {

namespace {

class IrGenerator {
 public:
  TackyProgram generate(const Program& program) {
    gen_statement(program.func.body);
    return TackyProgram{
        TackyFunction{program.func.name, std::move(instructions_)}};
  }

 private:
  int counter_ = 0;
  std::vector<TackyInstruction> instructions_;

  std::string make_temporary() {
    return "tmp." + std::to_string(counter_++);
  }

  static TackyUnaryOp gen_op(const UnaryOp& op) {
    return std::visit(
        Overloaded{
            [](const Negate&) -> TackyUnaryOp { return TackyNegate{}; },
            [](const Complement&) -> TackyUnaryOp { return TackyComplement{}; },
        },
        op);
  }

  TackyVal gen_exp(const Exp& exp) {
    return std::visit(
        Overloaded{
            [&](const Constant& c) -> TackyVal {
              return TackyConstant{c.value};
            },
            [&](const std::unique_ptr<Unary>& u) -> TackyVal {
              TackyVal src = gen_exp(u->operand);
              TackyVar dst{make_temporary()};
              instructions_.push_back(TackyUnary{gen_op(u->op), src, dst});
              return dst;
            }},
        exp);
  }

  void gen_statement(const Stmt& stmt) {
    std::visit(
        Overloaded{[&](const Return& r) {
          TackyVal val = gen_exp(r.exp);
          instructions_.push_back(TackyReturn{val});
        }},
        stmt);
  }
};

}  // namespace

TackyProgram irgen(const Program& program) {
  return IrGenerator().generate(program);
}

}  // namespace compiler
