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
  int label_counter_ = 0;
  std::vector<TackyInstruction> instructions_;

  std::string make_temporary() {
    return "tmp." + std::to_string(counter_++);
  }

  std::string generate_label(const std::string& prefix) {
    return prefix + std::to_string(label_counter_++);
  }

  static TackyUnaryOp gen_op(const UnaryOp& op) {
    return std::visit(
        Overloaded{
            [](const Negate&) -> TackyUnaryOp { return TackyNegate{}; },
            [](const Complement&) -> TackyUnaryOp { return TackyComplement{}; },
            [](const Not&) -> TackyUnaryOp { return TackyNot{}; },
        },
        op);
  }

  static TackyBinaryOp gen_binop(const BinaryOp& op) {
    return std::visit(
        Overloaded{
            [](const Add&) -> TackyBinaryOp { return TackyAdd{}; },
            [](const Subtract&) -> TackyBinaryOp { return TackySubtract{}; },
            [](const Multiply&) -> TackyBinaryOp { return TackyMultiply{}; },
            [](const Divide&) -> TackyBinaryOp { return TackyDivide{}; },
            [](const Remainder&) -> TackyBinaryOp { return TackyRemainder{}; },
            [](const BitAnd&) -> TackyBinaryOp { return TackyBitAnd{}; },
            [](const BitOr&) -> TackyBinaryOp { return TackyBitOr{}; },
            [](const BitXor&) -> TackyBinaryOp { return TackyBitXor{}; },
            [](const LeftShift&) -> TackyBinaryOp { return TackyLeftShift{}; },
            [](const RightShift&) -> TackyBinaryOp { return TackyRightShift{}; },
            [](const LessThan&) -> TackyBinaryOp { return TackyLessThan{}; },
            [](const LessEqual&) -> TackyBinaryOp { return TackyLessEqual{}; },
            [](const GreaterThan&) -> TackyBinaryOp { return TackyGreaterThan{}; },
            [](const GreaterEqual&) -> TackyBinaryOp {
              return TackyGreaterEqual{};
            },
            [](const Equal&) -> TackyBinaryOp { return TackyEqual{}; },
            [](const NotEqual&) -> TackyBinaryOp { return TackyNotEqual{}; },
            [](const LogicalAnd&) -> TackyBinaryOp {
              throw CompileError(
                  "internal error: logical and handled in gen_exp");
            },
            [](const LogicalOr&) -> TackyBinaryOp {
              throw CompileError(
                  "internal error: logical or handled in gen_exp");
            },
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
            },
            [&](const std::unique_ptr<Binary>& b) -> TackyVal {
              if (std::holds_alternative<LogicalAnd>(b->op)) {
                return gen_logical_and(b);
              }
              if (std::holds_alternative<LogicalOr>(b->op)) {
                return gen_logical_or(b);
              }
              TackyVal v1 = gen_exp(b->lhs);
              TackyVal v2 = gen_exp(b->rhs);
              TackyVar dst{make_temporary()};
              instructions_.push_back(
                  TackyBinary{gen_binop(b->op), v1, v2, dst});
              return dst;
            }},
        exp);
  }

  TackyVal gen_logical_and(const std::unique_ptr<Binary>& b) {
    std::string false_label = generate_label("and_false");
    std::string end_label = generate_label("and_end");
    TackyVal v1 = gen_exp(b->lhs);
    instructions_.push_back(TackyJumpIfZero{v1, false_label});
    TackyVal v2 = gen_exp(b->rhs);
    instructions_.push_back(TackyJumpIfZero{v2, false_label});
    TackyVar dst{make_temporary()};
    instructions_.push_back(TackyCopy{TackyConstant{1}, dst});
    instructions_.push_back(TackyJump{end_label});
    instructions_.push_back(TackyLabel{false_label});
    instructions_.push_back(TackyCopy{TackyConstant{0}, dst});
    instructions_.push_back(TackyLabel{end_label});
    return dst;
  }

  TackyVal gen_logical_or(const std::unique_ptr<Binary>& b) {
    std::string true_label = generate_label("or_true");
    std::string end_label = generate_label("or_end");
    TackyVal v1 = gen_exp(b->lhs);
    instructions_.push_back(TackyJumpIfNotZero{v1, true_label});
    TackyVal v2 = gen_exp(b->rhs);
    instructions_.push_back(TackyJumpIfNotZero{v2, true_label});
    TackyVar dst{make_temporary()};
    instructions_.push_back(TackyCopy{TackyConstant{0}, dst});
    instructions_.push_back(TackyJump{end_label});
    instructions_.push_back(TackyLabel{true_label});
    instructions_.push_back(TackyCopy{TackyConstant{1}, dst});
    instructions_.push_back(TackyLabel{end_label});
    return dst;
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
