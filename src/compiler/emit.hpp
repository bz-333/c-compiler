#pragma once

#include <string>

#include "compiler/codegen.hpp"

namespace compiler {

std::string emit_assembly(const AssemblyFunction& func);

}  // namespace compiler
