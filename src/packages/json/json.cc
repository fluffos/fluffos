#include "base/package_api.h"

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

namespace {

/**
 * Convert a JSON value to an LPC svalue recursively.
 */
svalue_t json_to_svalue_recurse(const json& j) {
  svalue_t sv = {};

  if (j.is_null()) {
    sv.type = T_NUMBER;
    sv.u.number = 0;
  } else if (j.is_boolean()) {
    sv.type = T_NUMBER;
    sv.u.number = j.get<bool>() ? 1 : 0;
  } else if (j.is_number_integer()) {
    sv.type = T_NUMBER;
    sv.u.number = j.get<LPC_INT>();
  } else if (j.is_number_float()) {
    sv.type = T_REAL;
    sv.u.real = j.get<LPC_FLOAT>();
  } else if (j.is_string()) {
    std::string str = j.get<std::string>();
    sv.type = T_STRING;
    sv.u.string = string_copy(str.data(), "json_to_svalue_recurse: string");
    sv.subtype = STRING_MALLOC;
  } else if (j.is_array()) {
    sv.type = T_ARRAY;
    sv.u.arr = allocate_array(j.size());
    for (size_t i = 0; i < j.size(); i++) {
      auto item = json_to_svalue_recurse(j[i]);
      assign_svalue_no_free(&sv.u.arr->item[i], &item);
      free_svalue(&item, "json_to_svalue_recurse: array item");
    }
  } else if (j.is_object()) {
    sv.type = T_MAPPING;
    // Convert JSON object to LPC mapping (key-value pairs)
    array_t* map_keys = allocate_array(j.size());
    array_t* map_values = allocate_array(j.size());

    size_t i = 0;
    for (auto& [key, value] : j.items()) {
      // Key is always a string in JSON
      svalue_t key_sv = {};
      key_sv.type = T_STRING;
      key_sv.u.string = string_copy(key.c_str(), "json_to_svalue_recurse: mapping key");
      key_sv.subtype = STRING_MALLOC;
      assign_svalue_no_free(&map_keys->item[i], &key_sv);
      free_svalue(&key_sv, "json_to_svalue_recurse: mapping key cleanup");

      // Value can be anything
      auto val_sv = json_to_svalue_recurse(value);
      assign_svalue_no_free(&map_values->item[i], &val_sv);
      free_svalue(&val_sv, "json_to_svalue_recurse: mapping value cleanup");

      i++;
    }

    sv.u.map = mkmapping(map_keys, map_values);
    free_array(map_keys);
    free_array(map_values);
  }

  return sv;
}

/**
 * Convert an LPC svalue to a JSON value recursively.
 */
json svalue_to_json_recurse(const svalue_t* sv) {
  switch (sv->type) {
    case T_NUMBER:
      return json(sv->u.number);

    case T_REAL:
      return json(sv->u.real);

    case T_STRING:
      return json(std::string(sv->u.string));

    case T_ARRAY: {
      json arr = json::array();
      for (int i = 0; i < sv->u.arr->size; i++) {
        arr.push_back(svalue_to_json_recurse(&sv->u.arr->item[i]));
      }
      return arr;
    }

    case T_MAPPING: {
      json obj = json::object();
      for (int i = 0; i < sv->u.map->table_size; i++) {
        for (auto* node = sv->u.map->table[i]; node; node = node->next) {
          const svalue_t* key = &node->values[0];
          const svalue_t* value = &node->values[1];

          // Only string keys are supported in JSON
          if (key->type == T_STRING) {
            obj[key->u.string] = svalue_to_json_recurse(value);
          }
        }
      }
      return obj;
    }

    default:
      // Unsupported types (objects, functions, etc.) become null
      return json(nullptr);
  }
}

}  // namespace

#ifdef F_JSON_PARSE
void f_json_parse() {
  if (st_num_arg != 1) {
    error("json_parse() requires exactly 1 argument");
  }

  if (sp->type != T_STRING) {
    error("json_parse() requires a string argument");
  }

  std::string json_str(sp->u.string);
  pop_stack();

  try {
    json j = json::parse(json_str);
    svalue_t result = json_to_svalue_recurse(j);
    push_svalue(&result);
    free_svalue(&result, "f_json_parse: result cleanup");
  } catch (const json::parse_error& e) {
    error("json_parse(): JSON parse error: %s", e.what());
  } catch (const std::exception& e) {
    error("json_parse(): %s", e.what());
  }
}
#endif

#ifdef F_JSON_STRINGIFY
void f_json_stringify() {
  int indent = -1;  // -1 means compact (no pretty printing)

  if (st_num_arg < 1 || st_num_arg > 2) {
    error("json_stringify() requires 1 or 2 arguments");
  }

  // Second argument is optional indent level for pretty printing
  if (st_num_arg == 2) {
    if (sp->type != T_NUMBER) {
      error("json_stringify(): indent must be a number");
    }
    indent = sp->u.number;
    pop_stack();
  }

  try {
    json j = svalue_to_json_recurse(sp);
    pop_stack();

    // indent=-1 produces compact JSON (default behavior)
    // indent>=0 produces pretty JSON with specified indent
    std::string result = j.dump(indent, ' ', false);
    copy_and_push_string(result.c_str());
  } catch (const std::exception& e) {
    error("json_stringify(): %s", e.what());
  }
}
#endif
