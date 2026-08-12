#pragma once

#include "compiler/parser.hpp"
#include "compiler/tacky.hpp"

namespace compiler {

TackyProgram irgen(const Program& program);

}  // namespace compiler
