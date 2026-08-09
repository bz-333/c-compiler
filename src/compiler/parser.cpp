#include "compiler/parser.hpp"

#include <cstddef>
#include <string>
#include <vector>

#include "compiler/compiler.hpp"

namespace compiler {

namespace {

const char* kind_name(Token::Kind kind) {
  switch (kind) {
    case Token::Kind::Eof:
      return "end of input";
    case Token::Kind::Keyword_Int:
      return "'int'";
    case Token::Kind::Keyword_Void:
      return "'void'";
    case Token::Kind::Keyword_Return:
      return "'return'";
    case Token::Kind::Identifier:
      return "identifier";
    case Token::Kind::Constant:
      return "constant";
    case Token::Kind::OpenParen:
      return "'('";
    case Token::Kind::CloseParen:
      return "')'";
    case Token::Kind::OpenBrace:
      return "'{'";
    case Token::Kind::CloseBrace:
      return "'}'";
    case Token::Kind::Semicolon:
      return "';'";
    case Token::Kind::Minus:
      return "'-'";
    case Token::Kind::DoubleMinus:
      return "'--'";
    case Token::Kind::Tilde:
      return "'~'";
  }
  return "?";
}

class Parser {
 public:
  explicit Parser(const std::vector<Token>& tokens) : tokens_(tokens) {}

  Program parse_program() {
    Function func = parse_function();
    expect(Token::Kind::Eof);
    return Program{std::move(func)};
  }

 private:
  const std::vector<Token>& tokens_;
  std::size_t pos_ = 0;

  const Token& peek() const { return tokens_[pos_]; }

  const Token& advance() {
    const Token& token = tokens_[pos_];
    if (token.kind != Token::Kind::Eof) {
      ++pos_;
    }
    return token;
  }

  const Token& expect(Token::Kind kind) {
    const Token& token = peek();
    if (token.kind != kind) {
      throw CompileError("expected " + std::string(kind_name(kind)) +
                         ", got " + describe(token));
    }
    return advance();
  }

  static std::string describe(const Token& token) {
    return kind_name(token.kind);
  }

  Function parse_function() {
    expect(Token::Kind::Keyword_Int);
    std::string name = expect(Token::Kind::Identifier).identifier;
    expect(Token::Kind::OpenParen);
    expect(Token::Kind::Keyword_Void);
    expect(Token::Kind::CloseParen);
    expect(Token::Kind::OpenBrace);
    Stmt body = parse_statement();
    expect(Token::Kind::CloseBrace);
    return Function{name, std::move(body)};
  }

  Stmt parse_statement() {
    expect(Token::Kind::Keyword_Return);
    Exp exp = parse_exp();
    expect(Token::Kind::Semicolon);
    return Return{std::move(exp)};
  }

  Exp parse_exp() {
    const Token& token = peek();
    if (token.kind == Token::Kind::Constant) {
      return parse_constant();
    }
    if (token.kind == Token::Kind::Minus || token.kind == Token::Kind::Tilde) {
      UnaryOp op = parse_unary_op();
      Exp operand = parse_exp();
      return Exp{std::make_unique<Unary>(Unary{op, std::move(operand)})};
    }
    if (token.kind == Token::Kind::OpenParen) {
      advance();
      Exp inner = parse_exp();
      expect(Token::Kind::CloseParen);
      return inner;
    }
    throw CompileError("expected expression, got " + describe(token));
  }

  UnaryOp parse_unary_op() {
    const Token& token = advance();
    if (token.kind == Token::Kind::Minus) {
      return Negate{};
    }
    if (token.kind == Token::Kind::Tilde) {
      return Complement{};
    }
    throw CompileError("expected unary operator, got " + describe(token));
  }

  Exp parse_constant() {
    Token token = expect(Token::Kind::Constant);
    return Constant{token.constant};
  }
};

}  // namespace

Program parse(const std::vector<Token>& tokens) {
  return Parser(tokens).parse_program();
}

}  // namespace compiler
