#pragma once

#include <string>
#include <vector>

namespace compiler {

struct Token {
  enum class Kind {
    Eof,
    Keyword_Int,
    Keyword_Void,
    Keyword_Return,
    Keyword_If,
    Keyword_Else,
    Keyword_Goto,
    Identifier,
    Constant,
    OpenParen,
    CloseParen,
    OpenBrace,
    CloseBrace,
    Semicolon,
    Minus,
    DoubleMinus,
    Tilde,
    Plus,
    Star,
    Slash,
    Percent,
    Ampersand,
    Pipe,
    Caret,
    LeftShift,
    RightShift,
    Not,
    DoubleAmpersand,
    DoublePipe,
    DoubleEquals,
    NotEquals,
    Equals,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    DoublePlus,
    PlusEquals,
    MinusEquals,
    StarEquals,
    SlashEquals,
    PercentEquals,
    AmpersandEquals,
    PipeEquals,
    CaretEquals,
    LeftShiftEquals,
    RightShiftEquals,
    Question,
    Colon,
  };

  Kind kind = Kind::Eof;
  std::string identifier{};
  int constant = 0;
};

std::vector<Token> lex(const std::string& source);

}  // namespace compiler
