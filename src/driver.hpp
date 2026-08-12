#pragma once

#include <string>
#include <vector>

class Driver {
 public:
  enum class Stage {
    Compile,
    Lex,
    Parse,
    Codegen,
    PrettyPrint,
    Tacky,
    TackyPrettyPrint,
  };

  struct Options {
    Stage stage = Stage::Compile;
    bool stop_at_assembly = false;
    bool stop_at_object = false;
    bool verbose = false;
    std::string output;
    std::vector<std::string> inputs;
  };

  Driver() = default;
  ~Driver();

  int run(int argc, char** argv);

 private:
  int parse_args(int argc, char** argv, Options& opts);
  int compile(const Options& opts);
  int compile_one(const Options& opts, const std::string& input,
                  std::vector<std::string>& objects);
  int preprocess(const std::string& input, const std::string& output) const;
  int assemble(const std::string& input, const std::string& output) const;
  int link(const std::vector<std::string>& objects, const std::string& output) const;
  int run_command(const std::string& command) const;
  std::string temp_path(const std::string& base, const std::string& ext);
  static void print_usage(const char* program);

  bool verbose_ = false;
  std::vector<std::string> temp_files_;
};
