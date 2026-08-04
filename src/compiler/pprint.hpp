#pragma once

#include <iosfwd>

#include "compiler/parser.hpp"

namespace compiler {

void pretty_print(const Program& program, std::ostream& os);

}  // namespace compiler
