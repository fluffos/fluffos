#include "base/std.h"

#include "vm/vm.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <argparse/argparse.hpp>

#include "ofile.h"

int main(int argc, char** argv) {
  // FIXME: shouldn't be needed
  config_init();
  init_strings();

  argparse::ArgumentParser program("json2o");

  program.add_argument("json_file").help("json file to read, if - then stdin is used");
  program.add_argument("o_file").default_value("-").help("o file to read, if omitted, then stdout is used");

  try {
    program.parse_args(argc, argv);
  }
  catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  std::stringstream s;

  auto json_file = program.get("json_file");
  if (json_file == "-") {
    s << std::cin.rdbuf();
  } else {
    std::ifstream ifs(json_file, std::ios::binary);
    if (!ifs) {
      std::cerr << "Error: cannot open file " << json_file << std::endl;
      return 1;
    }
    s << ifs.rdbuf();
  }

  std::string content = s.str();
  if (content.empty()) {
    std::cerr << "Error: empty json file " << json_file << std::endl;
    return 1;
  }

  try {
    nlohmann::json j = nlohmann::json::parse(content);
    OFile obj(j);
    std::string result = obj.to_ofile();
    auto o_file = program.get("o_file");
    if (o_file == "-") {
      std::cout << result;
    } else {
      std::ofstream ofs(o_file, std::ios::binary);
      ofs << result;
      ofs.close();
    }
  } catch (const std::exception& err) {
    std::cerr << "Error: " << err.what() << std::endl;
    return 1;
  }

  return 0;
}