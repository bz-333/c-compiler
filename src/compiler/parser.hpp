#pragma once

#include <vector>

#include "compiler/lexer.hpp"

namespace compiler {

struct Program {};

Program parse(const std::vector<Token>& tokens);

}  // namespace compiler
