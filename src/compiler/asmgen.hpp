#pragma once

#include "compiler/codegen.hpp"

namespace compiler {

AssemblyFunction generate_assembly(const TackyProgram& program);

}  // namespace compiler
