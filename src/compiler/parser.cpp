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
    case Token::Kind::Plus:
      return "'+'";
    case Token::Kind::Star:
      return "'*'";
    case Token::Kind::Slash:
      return "'/'";
    case Token::Kind::Percent:
      return "'%'";
    case Token::Kind::Ampersand:
      return "'&'";
    case Token::Kind::Pipe:
      return "'|'";
    case Token::Kind::Caret:
      return "'^'";
    case Token::Kind::LeftShift:
      return "'<<'";
    case Token::Kind::RightShift:
      return "'>>'";
    case Token::Kind::Not:
      return "'!'";
    case Token::Kind::DoubleAmpersand:
      return "'&&'";
    case Token::Kind::DoublePipe:
      return "'||'";
    case Token::Kind::DoubleEquals:
      return "'=='";
    case Token::Kind::NotEquals:
      return "'!='";
    case Token::Kind::Equals:
      return "'='";
    case Token::Kind::Less:
      return "'<'";
    case Token::Kind::Greater:
      return "'>'";
    case Token::Kind::LessEqual:
      return "'<='";
    case Token::Kind::GreaterEqual:
      return "'>='";
    case Token::Kind::DoublePlus:
      return "'++'";
    case Token::Kind::PlusEquals:
      return "'+='";
    case Token::Kind::MinusEquals:
      return "'-='";
    case Token::Kind::StarEquals:
      return "'*='";
    case Token::Kind::SlashEquals:
      return "'/='";
    case Token::Kind::PercentEquals:
      return "'%='";
    case Token::Kind::AmpersandEquals:
      return "'&='";
    case Token::Kind::PipeEquals:
      return "'|='";
    case Token::Kind::CaretEquals:
      return "'^='";
    case Token::Kind::LeftShiftEquals:
      return "'<<='";
    case Token::Kind::RightShiftEquals:
      return "'>>='";
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
    std::vector<BlockItem> body;
    while (peek().kind != Token::Kind::CloseBrace) {
      body.push_back(parse_block_item());
    }
    expect(Token::Kind::CloseBrace);
    return Function{name, std::move(body)};
  }

  BlockItem parse_block_item() {
    if (peek().kind == Token::Kind::Keyword_Int) {
      return parse_declaration();
    }
    return parse_statement();
  }

  Declaration parse_declaration() {
    expect(Token::Kind::Keyword_Int);
    std::string name = expect(Token::Kind::Identifier).identifier;
    std::optional<Exp> init;
    if (peek().kind == Token::Kind::Equals) {
      advance();
      init = parse_exp();
    }
    expect(Token::Kind::Semicolon);
    return Declaration{name, std::move(init)};
  }

  Stmt parse_statement() {
    if (peek().kind == Token::Kind::Keyword_Return) {
      advance();
      Exp exp = parse_exp();
      expect(Token::Kind::Semicolon);
      return Return{std::move(exp)};
    }
    if (peek().kind == Token::Kind::Semicolon) {
      advance();
      return Null{};
    }
    Exp exp = parse_exp();
    expect(Token::Kind::Semicolon);
    return Expression{std::move(exp)};
  }

  Exp parse_exp(int min_prec = 0) {
    Exp left = parse_factor();
    while (true) {
      Token::Kind kind = peek().kind;
      if (!is_binary_op(kind) || precedence(kind) < min_prec) {
        break;
      }
      if (is_assignment_op(kind)) {
        advance();
        Exp right = parse_exp(precedence(kind));
        if (kind == Token::Kind::Equals) {
          left = Exp{std::make_unique<Assignment>(
              Assignment{std::move(left), std::move(right)})};
        } else {
          left = Exp{std::make_unique<CompoundAssignment>(
              CompoundAssignment{compound_binop(kind), std::move(left),
                                 std::move(right)})};
        }
      } else {
        BinaryOp op = parse_binop();
        Exp right = parse_exp(precedence(kind) + 1);
        left = Exp{std::make_unique<Binary>(Binary{op, std::move(left), std::move(right)})};
      }
    }
    return left;
  }

  Exp parse_factor() {
    const Token& token = peek();
    Exp result;
    if (token.kind == Token::Kind::Constant) {
      result = parse_constant();
    } else if (token.kind == Token::Kind::Identifier) {
      std::string name = advance().identifier;
      result = Var{name};
    } else if (token.kind == Token::Kind::OpenParen) {
      advance();
      result = parse_exp(0);
      expect(Token::Kind::CloseParen);
    } else if (token.kind == Token::Kind::Minus || token.kind == Token::Kind::Tilde ||
             token.kind == Token::Kind::Not) {
      UnaryOp op = parse_unary_op();
      Exp operand = parse_factor();
      return Exp{std::make_unique<Unary>(Unary{op, std::move(operand)})};
    } else if (token.kind == Token::Kind::DoublePlus ||
               token.kind == Token::Kind::DoubleMinus) {
      PrefixOp op = parse_prefix_op();
      Exp operand = parse_factor();
      return Exp{std::make_unique<Prefix>(Prefix{op, std::move(operand)})};
    } else {
      throw CompileError("expected expression, got " + describe(token));
    }
    while (peek().kind == Token::Kind::DoublePlus ||
           peek().kind == Token::Kind::DoubleMinus) {
      PostfixOp op = parse_postfix_op();
      result = Exp{std::make_unique<Postfix>(Postfix{op, std::move(result)})};
    }
    return result;
  }

  static bool is_prefix_op(Token::Kind kind) {
    return kind == Token::Kind::Minus || kind == Token::Kind::Tilde ||
           kind == Token::Kind::Not || kind == Token::Kind::DoublePlus ||
           kind == Token::Kind::DoubleMinus;
  }

  UnaryOp parse_unary_op() {
    const Token& token = advance();
    if (token.kind == Token::Kind::Minus) {
      return Negate{};
    }
    if (token.kind == Token::Kind::Tilde) {
      return Complement{};
    }
    if (token.kind == Token::Kind::Not) {
      return Not{};
    }
    throw CompileError("expected unary operator, got " + describe(token));
  }

  PrefixOp parse_prefix_op() {
    const Token& token = advance();
    if (token.kind == Token::Kind::DoublePlus) {
      return Increment{};
    }
    if (token.kind == Token::Kind::DoubleMinus) {
      return Decrement{};
    }
    throw CompileError("expected prefix operator, got " + describe(token));
  }

  PostfixOp parse_postfix_op() {
    const Token& token = advance();
    if (token.kind == Token::Kind::DoublePlus) {
      return Increment{};
    }
    if (token.kind == Token::Kind::DoubleMinus) {
      return Decrement{};
    }
    throw CompileError("expected postfix operator, got " + describe(token));
  }

  BinaryOp parse_binop() {
    const Token& token = advance();
    switch (token.kind) {
      case Token::Kind::Plus:
        return Add{};
      case Token::Kind::Minus:
        return Subtract{};
      case Token::Kind::Star:
        return Multiply{};
      case Token::Kind::Slash:
        return Divide{};
      case Token::Kind::Percent:
        return Remainder{};
      case Token::Kind::Ampersand:
        return BitAnd{};
      case Token::Kind::Pipe:
        return BitOr{};
      case Token::Kind::Caret:
        return BitXor{};
      case Token::Kind::LeftShift:
        return LeftShift{};
      case Token::Kind::RightShift:
        return RightShift{};
      case Token::Kind::Less:
        return LessThan{};
      case Token::Kind::LessEqual:
        return LessEqual{};
      case Token::Kind::Greater:
        return GreaterThan{};
      case Token::Kind::GreaterEqual:
        return GreaterEqual{};
      case Token::Kind::DoubleEquals:
        return Equal{};
      case Token::Kind::NotEquals:
        return NotEqual{};
      case Token::Kind::DoubleAmpersand:
        return LogicalAnd{};
      case Token::Kind::DoublePipe:
        return LogicalOr{};
      default:
        throw CompileError("expected binary operator, got " + describe(token));
    }
  }

  static bool is_binary_op(Token::Kind kind) {
    return kind == Token::Kind::Plus || kind == Token::Kind::Minus ||
           kind == Token::Kind::Star || kind == Token::Kind::Slash ||
           kind == Token::Kind::Percent || kind == Token::Kind::Ampersand ||
           kind == Token::Kind::Pipe || kind == Token::Kind::Caret ||
           kind == Token::Kind::LeftShift || kind == Token::Kind::RightShift ||
           kind == Token::Kind::Less || kind == Token::Kind::Greater ||
           kind == Token::Kind::LessEqual || kind == Token::Kind::GreaterEqual ||
           kind == Token::Kind::DoubleEquals ||
           kind == Token::Kind::NotEquals ||
           kind == Token::Kind::DoubleAmpersand ||
           kind == Token::Kind::DoublePipe || is_assignment_op(kind);
  }

  static bool is_assignment_op(Token::Kind kind) {
    return kind == Token::Kind::Equals || kind == Token::Kind::PlusEquals ||
           kind == Token::Kind::MinusEquals ||
           kind == Token::Kind::StarEquals ||
           kind == Token::Kind::SlashEquals ||
           kind == Token::Kind::PercentEquals ||
           kind == Token::Kind::AmpersandEquals ||
           kind == Token::Kind::PipeEquals ||
           kind == Token::Kind::CaretEquals ||
           kind == Token::Kind::LeftShiftEquals ||
           kind == Token::Kind::RightShiftEquals;
  }

  static BinaryOp compound_binop(Token::Kind kind) {
    switch (kind) {
      case Token::Kind::PlusEquals:
        return Add{};
      case Token::Kind::MinusEquals:
        return Subtract{};
      case Token::Kind::StarEquals:
        return Multiply{};
      case Token::Kind::SlashEquals:
        return Divide{};
      case Token::Kind::PercentEquals:
        return Remainder{};
      case Token::Kind::AmpersandEquals:
        return BitAnd{};
      case Token::Kind::PipeEquals:
        return BitOr{};
      case Token::Kind::CaretEquals:
        return BitXor{};
      case Token::Kind::LeftShiftEquals:
        return LeftShift{};
      case Token::Kind::RightShiftEquals:
        return RightShift{};
      default:
        throw CompileError("internal error: not a compound assignment token");
    }
  }

  static int precedence(Token::Kind kind) {
    switch (kind) {
      case Token::Kind::Equals:
      case Token::Kind::PlusEquals:
      case Token::Kind::MinusEquals:
      case Token::Kind::StarEquals:
      case Token::Kind::SlashEquals:
      case Token::Kind::PercentEquals:
      case Token::Kind::AmpersandEquals:
      case Token::Kind::PipeEquals:
      case Token::Kind::CaretEquals:
      case Token::Kind::LeftShiftEquals:
      case Token::Kind::RightShiftEquals:
        return 1;
      case Token::Kind::Star:
      case Token::Kind::Slash:
      case Token::Kind::Percent:
        return 50;
      case Token::Kind::Plus:
      case Token::Kind::Minus:
        return 45;
      case Token::Kind::LeftShift:
      case Token::Kind::RightShift:
        return 40;
      case Token::Kind::Less:
      case Token::Kind::LessEqual:
      case Token::Kind::Greater:
      case Token::Kind::GreaterEqual:
        return 35;
      case Token::Kind::DoubleEquals:
      case Token::Kind::NotEquals:
        return 30;
      case Token::Kind::Ampersand:
        return 25;
      case Token::Kind::Caret:
        return 20;
      case Token::Kind::Pipe:
        return 15;
      case Token::Kind::DoubleAmpersand:
        return 10;
      case Token::Kind::DoublePipe:
        return 5;
      default:
        return 0;
    }
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
