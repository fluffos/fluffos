#include "base/std.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <argparse/argparse.hpp>

#include "vm/internal/base/svalue.h"

#include "ofile.h"

int main(int argc, char** argv) {
  // FIXME: config_init shouldn't be needed
  config_init();

  argparse::ArgumentParser program("o2json");

  program.add_argument("-pretty")
      .help("pretty print json")
      .flag();

  program.add_argument("-ascii")
      .help("ascii mode")
      .flag();

  program.add_argument("o_file").help("o file to read");
  program.add_argument("json_file").help("json file to write");

  try {
    program.parse_args(argc, argv);
  }
  catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  auto o_file = program.get("o_file");

  std::ifstream ifs(o_file, std::ios::binary);
  if (!ifs) {
    std::cerr << "Error: cannot open file " << o_file << std::endl;
    return 1;
  }
  std::stringstream s;
  s << ifs.rdbuf();
  std::string content = s.str();

  OFile obj(content);
  std::string result = obj.to_json().dump(program["-pretty"] == true ? 2 : -1, ' ', program["-ascii"] == true);

  std::ofstream ofs(program.get("json_file"), std::ios::binary);
  ofs << result;
  ofs.close();

  return 0;
}