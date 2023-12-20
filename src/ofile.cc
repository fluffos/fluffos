#include "base/std.h"

#include "ofile.h"

#include "vm/vm.h"
#include "vm/internal/base/svalue.h"

#include <nlohmann/json.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

typedef unsigned char BYTE;

static const std::string BASE64_CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static inline bool is_base64(BYTE c) { return (isalnum(c) || (c == '+') || (c == '/')); }

std::string base64_encode(BYTE const* buf, unsigned int bufLen) {
  std::string ret;
  int i = 0;
  int j = 0;
  BYTE char_array_3[3];
  BYTE char_array_4[4];

  while (bufLen--) {
    char_array_3[i++] = *(buf++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; (i < 4); i++) ret += BASE64_CHARS[char_array_4[i]];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 3; j++) char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++) ret += BASE64_CHARS[char_array_4[j]];

    while ((i++ < 3)) ret += '=';
  }

  return ret;
}

std::vector<BYTE> base64_decode(std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in = 0;
  BYTE char_array_4[4], char_array_3[3];
  std::vector<BYTE> ret;

  while (in_len-- && (encoded_string[in] != '=') && is_base64(encoded_string[in])) {
    char_array_4[i++] = encoded_string[in];
    in++;
    if (i == 4) {
      for (i = 0; i < 4; i++) char_array_4[i] = BASE64_CHARS.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++) ret.push_back(char_array_3[i]);
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 4; j++) char_array_4[j] = 0;

    for (j = 0; j < 4; j++) char_array_4[j] = BASE64_CHARS.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
  }

  return ret;
}

std::string type_2_name(int type) {
  switch (type) {
    case T_STRING:
      return "string";
    case T_NUMBER:
      return "number";
    case T_REAL:
      return "float";
    case T_ARRAY:
      return "array";
    case T_CLASS:
      return "class";
    case T_MAPPING:
      return "mapping";
    case T_OBJECT:
      return "object";
    case T_BUFFER:
      return "buffer";
    default:
      return "unknown";
  }
}

int name_2_type(const std::string name) {
        if (name == "string") {
        return T_STRING;
        } else if (name == "number") {
        return T_NUMBER;
        } else if (name == "float") {
        return T_REAL;
        } else if (name == "array") {
        return T_ARRAY;
        } else if (name == "class") {
        return T_CLASS;
        } else if (name == "mapping") {
        return T_MAPPING;
        } else if (name == "object") {
        return T_OBJECT;
        } else if (name == "buffer") {
        return T_BUFFER;
        } else {
        return T_INVALID;
        }
}

nlohmann::json svalue_to_json_recurse(const svalue_t* sv) {
  nlohmann::json j;
  j["type"] = type_2_name(sv->type);
  switch (sv->type) {
    case T_NUMBER:
      j["value"] = sv->u.number;
      break;
    case T_STRING:
      j["value"] = sv->u.string;
      break;
    case T_REAL:
      j["value"] = sv->u.real;
      break;
    case T_CLASS:
      // fall through
    case T_ARRAY: {
      nlohmann::json arr = nlohmann::json::array();
      for (int i = 0; i < sv->u.arr->size; i++) {
        arr.push_back(svalue_to_json_recurse(&sv->u.arr->item[i]));
      }
      j["value"] = arr;
      break;
    }
    case T_OBJECT:
      // ignored: save_svalue() doesn't save objects
      break;
    case T_MAPPING: {
      nlohmann::json items = nlohmann::json::array();
      for (int i = 0; i < sv->u.map->table_size; i++) {
        for (auto* node = sv->u.map->table[i]; node; node = node->next) {
          nlohmann::json item;
          item["key"] = svalue_to_json_recurse(&node->values[0]);
          item["value"] = svalue_to_json_recurse(&node->values[1]);
          items.push_back(item);
        }
      }
      j["value"] = items;
      break;
    }
    case T_BUFFER:
      j["value"] = base64_encode(sv->u.buf->item, sv->u.buf->size);
      break;
    default:
      std::cerr << "Error: unknown svalue type: " << sv->type
                << " ignored, output will be incomplete." << std::endl;
      break;
  }
  return j;
}

svalue_t svalue_from_json_recurse(nlohmann::json j) {
  svalue_t sv = {};
  sv.type = name_2_type(j["type"]);
  auto value = j["value"];
  switch (sv.type) {
    case T_NUMBER:
      if (!value.is_number_integer()) {
        throw std::runtime_error("Invalid number value: " + value.dump());
      }
      sv.u.number = value.get<LPC_INT>();
      break;
    case T_REAL:
      if (!value.is_number_float()) {
        throw std::runtime_error("Invalid float value: " + value.dump());
      }
      sv.u.real = value.get<LPC_FLOAT>();
      break;
    case T_STRING:
      if (!value.is_string()) {
        throw std::runtime_error("Invalid string value: " + value.dump());
      }
      sv.u.string = string_copy(value.get<std::string>().data(), "svalue_from_json_recurse: string");
      sv.subtype = STRING_MALLOC;
      break;
    case T_CLASS:
    // fall through
    case T_ARRAY: {
      if (!value.is_array()) {
        throw std::runtime_error("Invalid array value: " + value.dump());
      }
      sv.u.arr = allocate_array(value.size());
      for (int i = 0; i < value.size(); i++) {
        auto sv_item = svalue_from_json_recurse(value[i]);
        assign_svalue_no_free(&sv.u.arr->item[i], &sv_item);
        free_svalue(&sv_item, "svalue_from_json_recurse");
      }
      break;
    }
    case T_OBJECT:
      // ignored: save_svalue() doesn't save objects
      break;
    case T_MAPPING: {
      if (!value.is_array()) {
        throw std::runtime_error("Invalid mapping value: " + value.dump());
      }

      array_t* map_keys = allocate_array(value.size());
      array_t* map_values = allocate_array(value.size());

      int i = 0;
      for (auto it : value.get<std::vector<nlohmann::json>>()) {
        auto sv_key = svalue_from_json_recurse(it["key"]);
        assign_svalue_no_free(&map_keys->item[i], &sv_key);
        free_svalue(&sv_key, "svalue_from_json_recurse");

        auto sv_val = svalue_from_json_recurse(it["value"]);
        assign_svalue_no_free(&map_values->item[i], &sv_val);
        free_svalue(&sv_val, "svalue_from_json_recurse");
        i++;
      }
      sv.u.map = mkmapping(map_keys, map_values);
      free_array(map_keys);
      free_array(map_values);
      break;
    }
    case T_BUFFER: {
      if (!value.is_string()) {
        throw std::runtime_error("Invalid buffer value: " + value.dump());
      }
      sv.u.buf = allocate_buffer(value.size());
      auto decoded = base64_decode(value);
      memcpy(sv.u.buf->item, decoded.data(), decoded.size());
      break;
    }
    default:
      std::cerr << "Error: unknown svalue type: " << sv.type
                << " ignored, output will be incomplete." << std::endl;
      break;
  }
  return sv;
}

nlohmann::json OFile::to_json() {
  nlohmann::json j;
  j["program_name"] = program_name;
  j["variables"] = nlohmann::json::array();
  for (auto& v : variables) {
    nlohmann::json item;
    item["name"] = v.first;
    item["value"] = svalue_to_json_recurse(&v.second);
    j["variables"].push_back(item);
  }
  return j;
}

OFile::OFile(const std::string& o_str) {
  std::stringstream ifs(o_str);

  std::string line;
  std::getline(ifs, line, '\n');
  if (line.empty()) {
    throw std::runtime_error("Invalid o file: empty program name");
  }

  // parse program name
  this->program_name = line;

  // Parse variables
  while (std::getline(ifs, line, '\n')) {
    auto pos = line.find(' ');
    if (pos == std::string::npos) {
      throw std::runtime_error("Invalid o file: malformed variable line");
    }
    std::string name = line.substr(0, pos);
    std::string value = line.substr(pos + 1);
    svalue_t sv = {};
    int res = restore_svalue(value.data(), &sv);
    if (res != 0) {
      throw std::runtime_error("Invalid o file: malformed variable value");
    }
    this->variables.emplace_back(name, sv);
  }
}

OFile::OFile(const nlohmann::json& j) {
    program_name = j["program_name"];
    for (const auto& v : j["variables"]) {
      variables.emplace_back(v["name"], svalue_from_json_recurse(v["value"]));
    }
}

std::string OFile::to_ofile() {
    std::stringstream ss;
    ss << program_name << "\n";
    for (auto& v : variables) {
      ss << v.first << " ";
      auto size = svalue_save_size(&v.second);
      std::string buf(size - 1, '\0');
      auto *p = buf.data();
      save_svalue(&v.second, &p);
      ss << buf << "\n";
    }
    return ss.str();
}
OFile::~OFile() {
  for (auto& v : variables) {
    free_svalue(&v.second, "OFile::~OFile");
  }
}
