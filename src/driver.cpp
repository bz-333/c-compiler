#include "driver.hpp"

#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "compiler/codegen.hpp"
#include "compiler/compiler.hpp"
#include "compiler/lexer.hpp"
#include "compiler/parser.hpp"
#include "compiler/pprint.hpp"

namespace {

std::string quote(const std::string& arg) {
  std::string out = "'";
  for (char c : arg) {
    if (c == '\'') {
      out += "'\\''";
    } else {
      out += c;
    }
  }
  out += '\'';
  return out;
}

std::string base_name(const std::string& path) {
  std::size_t slash = path.find_last_of('/');
  std::string name = slash == std::string::npos ? path : path.substr(slash + 1);
  std::size_t dot = name.find_last_of('.');
  if (dot != std::string::npos) {
    name = name.substr(0, dot);
  }
  return name;
}

std::string replace_extension(const std::string& path, const std::string& ext) {
  std::size_t slash = path.find_last_of('/');
  std::size_t start = slash == std::string::npos ? 0 : slash + 1;
  std::size_t dot = path.find_last_of('.');
  if (dot != std::string::npos && dot >= start) {
    return path.substr(0, dot) + ext;
  }
  return path + ext;
}

std::string read_file(const std::string& path) {
  std::ifstream in(path);
  std::ostringstream buffer;
  buffer << in.rdbuf();
  return buffer.str();
}

void write_file(const std::string& path, const std::string& contents) {
  std::ofstream out(path);
  out << contents;
}

}  // namespace

Driver::~Driver() {
  for (const std::string& path : temp_files_) {
    std::remove(path.c_str());
  }
}

int Driver::run(int argc, char** argv) {
  Options opts;
  int rc = parse_args(argc, argv, opts);
  if (rc != 0) {
    return rc;
  }
  verbose_ = opts.verbose;
  return compile(opts);
}

int Driver::parse_args(int argc, char** argv, Options& opts) {
  const char* program = argc > 0 ? argv[0] : "ccompiler";
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--lex") {
      opts.stage = Stage::Lex;
    } else if (arg == "--parse") {
      opts.stage = Stage::Parse;
    } else if (arg == "--codegen") {
      opts.stage = Stage::Codegen;
    } else if (arg == "--pprint") {
      opts.stage = Stage::PrettyPrint;
    } else if (arg == "-S") {
      opts.stop_at_assembly = true;
    } else if (arg == "-c") {
      opts.stop_at_object = true;
    } else if (arg == "-v") {
      opts.verbose = true;
    } else if (arg == "-o") {
      if (i + 1 >= argc) {
        std::cerr << "error: -o requires an argument\n";
        return 1;
      }
      opts.output = argv[++i];
    } else if (arg == "-h" || arg == "--help") {
      print_usage(program);
      std::exit(0);
    } else if (!arg.empty() && arg[0] == '-') {
      std::cerr << "error: unknown option: " << arg << '\n';
      print_usage(program);
      return 1;
    } else {
      opts.inputs.push_back(arg);
    }
  }
  if (opts.inputs.empty()) {
    std::cerr << "error: no input files\n";
    print_usage(program);
    return 1;
  }
  return 0;
}

int Driver::compile(const Options& opts) {
  std::vector<std::string> objects;
  for (const std::string& input : opts.inputs) {
    int rc = compile_one(opts, input, objects);
    if (rc != 0) {
      return rc;
    }
  }
  if (opts.stage == Stage::Compile && !opts.stop_at_assembly &&
      !opts.stop_at_object) {
    std::string output = opts.output.empty() ? "a.out" : opts.output;
    return link(objects, output);
  }
  return 0;
}

int Driver::compile_one(const Options& opts, const std::string& input,
                        std::vector<std::string>& objects) {
  std::string preprocessed = temp_path(base_name(input), ".i");
  int rc = preprocess(input, preprocessed);
  if (rc != 0) {
    return rc;
  }

  std::string source = read_file(preprocessed);

  try {
    std::vector<compiler::Token> tokens = compiler::lex(source);
    if (opts.stage == Stage::Lex) {
      return 0;
    }

    compiler::Program program = compiler::parse(tokens);
    if (opts.stage == Stage::Parse) {
      return 0;
    }

    if (opts.stage == Stage::PrettyPrint) {
      compiler::pretty_print(program, std::cout);
      return 0;
    }

    std::string assembly = compiler::codegen(program);
    if (opts.stage == Stage::Codegen) {
      return 0;
    }

    if (opts.stop_at_assembly) {
      std::string output =
          opts.output.empty() ? replace_extension(input, ".s") : opts.output;
      write_file(output, assembly);
      return 0;
    }

    std::string asm_file = temp_path(base_name(input), ".s");
    write_file(asm_file, assembly);

    std::string obj_file;
    if (opts.stop_at_object) {
      obj_file = opts.output.empty() ? replace_extension(input, ".o") : opts.output;
    } else {
      obj_file = temp_path(base_name(input), ".o");
    }

    rc = assemble(asm_file, obj_file);
    if (rc != 0) {
      return rc;
    }

    if (opts.stop_at_object) {
      return 0;
    }

    objects.push_back(obj_file);
    return 0;
  } catch (const compiler::CompileError& e) {
    std::cerr << input << ": " << e.what() << '\n';
    return 1;
  }
}

int Driver::preprocess(const std::string& input, const std::string& output) const {
  return run_command("gcc -E -P " + quote(input) + " -o " + quote(output));
}

int Driver::assemble(const std::string& input, const std::string& output) const {
  return run_command("as " + quote(input) + " -o " + quote(output));
}

int Driver::link(const std::vector<std::string>& objects,
                 const std::string& output) const {
  std::string command = "gcc";
  for (const std::string& object : objects) {
    command += " " + quote(object);
  }
  command += " -o " + quote(output);
  return run_command(command);
}

int Driver::run_command(const std::string& command) const {
  if (verbose_) {
    std::cerr << command << '\n';
  }
  int status = std::system(command.c_str());
  if (status == -1) {
    std::cerr << "error: failed to execute: " << command << '\n';
    return 1;
  }
  if (!WIFEXITED(status)) {
    std::cerr << "error: terminated abnormally: " << command << '\n';
    return 1;
  }
  return WEXITSTATUS(status);
}

std::string Driver::temp_path(const std::string& base, const std::string& ext) {
  static int counter = 0;
  std::string path = "/tmp/ccompiler-" + base + "-" +
                     std::to_string(static_cast<long>(::getpid())) + "-" +
                     std::to_string(counter++) + ext;
  temp_files_.push_back(path);
  return path;
}

void Driver::print_usage(const char* program) {
  std::cerr << "Usage: " << program << " [options] <input files>\n\n"
            << "Options:\n"
            << "  --lex       Run the lexer only; produce no output\n"
            << "  --parse     Run the lexer and parser; produce no output\n"
            << "  --codegen   Run all stages up to code generation; produce no output\n"
            << "  --pprint    Parse and print the AST\n"
            << "  -S          Compile to assembly and stop\n"
            << "  -c          Compile and assemble but do not link\n"
            << "  -o <file>   Write output to <file>\n"
            << "  -v          Print the external commands being run\n"
            << "  -h, --help  Show this help message\n";
}
