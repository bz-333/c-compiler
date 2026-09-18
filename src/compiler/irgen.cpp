#include "compiler/irgen.hpp"

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "compiler/compiler.hpp"
#include "compiler/unique_names.hpp"

namespace compiler {

namespace {

class IrGenerator {
 public:
  TackyProgram generate(const Program& program) {
    for (const BlockItem& item : program.func.body.body) {
      std::visit(
          Overloaded{
              [&](const Declaration& decl) { gen_declaration(decl); },
              [&](const Stmt& stmt) { gen_statement(stmt); },
          },
          item);
    }
    instructions_.push_back(TackyReturn{TackyConstant{0}});
    return TackyProgram{
        TackyFunction{program.func.name, std::move(instructions_)}};
  }

 private:
  int label_counter_ = 0;
  std::vector<TackyInstruction> instructions_;

  std::string make_temporary() { return make_unique_name("tmp"); }

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

  static TackyBinaryOp increment_binop(const PrefixOp& op) {
    return std::holds_alternative<Increment>(op)
               ? TackyBinaryOp{TackyAdd{}}
               : TackyBinaryOp{TackySubtract{}};
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
            [&](const Var& v) -> TackyVal { return TackyVar{v.name}; },
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
            },
            [&](const std::unique_ptr<Assignment>& a) -> TackyVal {
              const Var* lhs = std::get_if<Var>(&a->lhs);
              if (lhs == nullptr) {
                throw CompileError(
                    "internal error: invalid lvalue reached irgen");
              }
              TackyVal result = gen_exp(a->rhs);
              instructions_.push_back(
                  TackyCopy{result, TackyVar{lhs->name}});
              return TackyVar{lhs->name};
            },
            [&](const std::unique_ptr<CompoundAssignment>& c) -> TackyVal {
              const Var* lhs = std::get_if<Var>(&c->lhs);
              if (lhs == nullptr) {
                throw CompileError(
                    "internal error: invalid lvalue reached irgen");
              }
              TackyVal lv = gen_exp(c->lhs);
              TackyVal rv = gen_exp(c->rhs);
              TackyVar tmp{make_temporary()};
              instructions_.push_back(
                  TackyBinary{gen_binop(c->op), lv, rv, tmp});
              instructions_.push_back(TackyCopy{tmp, TackyVar{lhs->name}});
              return TackyVar{lhs->name};
            },
            [&](const std::unique_ptr<Prefix>& p) -> TackyVal {
              const Var* operand = std::get_if<Var>(&p->operand);
              if (operand == nullptr) {
                throw CompileError(
                    "internal error: invalid lvalue reached irgen");
              }
              TackyVar v{operand->name};
              TackyVar tmp{make_temporary()};
              instructions_.push_back(TackyBinary{
                  increment_binop(p->op), v, TackyConstant{1}, tmp});
              instructions_.push_back(TackyCopy{tmp, v});
              return v;
            },
            [&](const std::unique_ptr<Postfix>& p) -> TackyVal {
              const Var* operand = std::get_if<Var>(&p->operand);
              if (operand == nullptr) {
                throw CompileError(
                    "internal error: invalid lvalue reached irgen");
              }
              TackyVar v{operand->name};
              TackyVar old{make_temporary()};
              TackyVar tmp{make_temporary()};
              instructions_.push_back(TackyCopy{v, old});
              instructions_.push_back(TackyBinary{
                  increment_binop(p->op), v, TackyConstant{1}, tmp});
              instructions_.push_back(TackyCopy{tmp, v});
              return old;
            },
            [&](const std::unique_ptr<Conditional>& c) -> TackyVal {
              std::string e2_label = generate_label("e2");
              std::string end_label = generate_label("end");
              TackyVal cond = gen_exp(c->condition);
              instructions_.push_back(TackyJumpIfZero{cond, e2_label});
              TackyVal v1 = gen_exp(c->then_exp);
              TackyVar result{make_temporary()};
              instructions_.push_back(TackyCopy{v1, result});
              instructions_.push_back(TackyJump{end_label});
              instructions_.push_back(TackyLabel{e2_label});
              TackyVal v2 = gen_exp(c->else_exp);
              instructions_.push_back(TackyCopy{v2, result});
              instructions_.push_back(TackyLabel{end_label});
              return result;
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

  void gen_declaration(const Declaration& decl) {
    if (decl.init) {
      TackyVal val = gen_exp(*decl.init);
      instructions_.push_back(TackyCopy{val, TackyVar{decl.name}});
    }
  }

  void gen_statement(const Stmt& stmt) {
    std::visit(
        Overloaded{
            [&](const Return& r) {
              TackyVal val = gen_exp(r.exp);
              instructions_.push_back(TackyReturn{val});
            },
            [&](const Expression& e) { gen_exp(e.exp); },
            [&](const Null&) {},
            [&](const Goto& g) {
              instructions_.push_back(TackyJump{g.name});
            },
            [&](const std::unique_ptr<Labeled>& l) {
              instructions_.push_back(TackyLabel{l->name});
              gen_statement(l->statement);
            },
            [&](const std::unique_ptr<Compound>&) {
              throw CompileError(
                  "compound statements not yet supported by irgen");
            },
            [&](const std::unique_ptr<If>& i) {
              TackyVal cond = gen_exp(i->condition);
              if (i->else_) {
                std::string else_label = generate_label("else");
                std::string end_label = generate_label("end");
                instructions_.push_back(TackyJumpIfZero{cond, else_label});
                gen_statement(i->then);
                instructions_.push_back(TackyJump{end_label});
                instructions_.push_back(TackyLabel{else_label});
                gen_statement(*i->else_);
                instructions_.push_back(TackyLabel{end_label});
              } else {
                std::string end_label = generate_label("end");
                instructions_.push_back(TackyJumpIfZero{cond, end_label});
                gen_statement(i->then);
                instructions_.push_back(TackyLabel{end_label});
              }
            },
        },
        stmt);
  }
};

}  // namespace

TackyProgram irgen(const Program& program) {
  return IrGenerator().generate(program);
}

}  // namespace compiler
