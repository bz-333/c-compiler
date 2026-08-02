#pragma once

#include <stdexcept>
#include <string>

namespace compiler {

class CompileError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

}  // namespace compiler
