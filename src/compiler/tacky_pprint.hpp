#pragma once

#include <iosfwd>

#include "compiler/tacky.hpp"

namespace compiler {

void pretty_print(const TackyProgram& program, std::ostream& os);

}  // namespace compiler
