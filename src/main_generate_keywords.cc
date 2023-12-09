#include "base/std.h"

#include "compiler/internal/compiler.h"
#include "compiler/internal/keyword.h"
#include "compiler/internal/lex.h"

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "efuns.autogen.h"

std::string get_compiler_type_name_trimmed(int type) {
  char buf[256] = {};
  get_type_name(&buf[0], &buf[sizeof(buf) - 1], type);
  std::string arg_type = buf;
  arg_type = trim(arg_type);
  return arg_type;
}

int main() {
  nlohmann::json j;

  for (int i=0; i< size_of_predefs; i++) {
    auto &efun = predefs[i];
    auto return_type = get_compiler_type_name_trimmed(efun.ret_type);
    std::vector<nlohmann::json> args;
    auto *argp = &efun_arg_types[efun.arg_index];

    int max_args = std::max(efun.min_args, efun.max_args == -1 ? (short)0 : efun.max_args);
    for(int i = 0; i < max_args; i++) {
      std::vector<std::string> arg_types;
      while (*argp != 0) {
        arg_types.push_back(get_compiler_type_name_trimmed(*argp));
        argp++;
      }
      argp++;
      args.emplace_back(arg_types);
    }


    j.push_back({
        {"name", efun.word},
        {"type", "efun"},
        {"returns", return_type},
        {"min_args", efun.min_args},
        {"max_args", efun.max_args},
        {"args_types", nlohmann::json(args)},
    });
  }

  std::ofstream file("keywords.json");
  file << j.dump(4);  // 4 spaces for indentation

  return 0;
}