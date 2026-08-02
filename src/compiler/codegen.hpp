#pragma once

#include <string>

#include "compiler/parser.hpp"

namespace compiler {

std::string codegen(const Program& program);

}  // namespace compiler
