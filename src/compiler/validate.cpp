#include "compiler/validate.hpp"

#include <utility>

#include "compiler/check_labels.hpp"
#include "compiler/resolve_vars.hpp"

namespace compiler {

Program validate(Program program) {
  program = resolve_vars(std::move(program));
  program = check_labels(std::move(program));
  return program;
}

}  // namespace compiler