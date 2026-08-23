#include "compiler/unique_names.hpp"

#include <string>

namespace compiler {

namespace {

int counter = 0;

}  // namespace

std::string make_unique_name(const std::string& prefix) {
  return prefix + "." + std::to_string(counter++);
}

}  // namespace compiler