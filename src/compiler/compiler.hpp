#pragma once

#include <stdexcept>
#include <string>

namespace compiler {

class CompileError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

template <typename... Ts>
struct Overloaded : Ts... {
  using Ts::operator()...;
};

template <typename... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

}  // namespace compiler
