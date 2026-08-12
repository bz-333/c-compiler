#include "compiler/codegen.hpp"

#include "compiler/asmgen.hpp"
#include "compiler/emit.hpp"

namespace compiler {

std::string codegen(const TackyProgram& program) {
  return emit_assembly(generate_assembly(program));
}

}  // namespace compiler
