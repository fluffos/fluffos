#pragma once

#include <utility> // std::pair, std::make_pair
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct svalue_t; // forward declaration

class OFile {
 public:
  std::string program_name;
  std::vector<std::pair<std::string, svalue_t>> variables;

  OFile() = default;
  ~OFile();

  OFile(const std::string& o_str);
  OFile(const nlohmann::json& json_str);
  std::string to_ofile();
  nlohmann::json to_json();
};