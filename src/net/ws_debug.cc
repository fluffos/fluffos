#include "base/internal/log.h"
#include "base/std.h"

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/util.h>
#include <event2/listener.h>

#include <libwebsockets.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "net/ws_debug.h"
#include "interactive.h"
#include "vm/vm.h"
#include "mainlib.h"
#include "compiler/internal/disassembler.h"
#include "base/internal/rc.h"

// Forward declaration from disassembler.cc
void dump_prog(program_t* prog, FILE* f, int options);

// from comm.cc
interactive_t* new_user(port_def_t* port, evutil_socket_t fd, sockaddr* addr, socklen_t addrlen);
extern void on_user_logon(interactive_t*);
extern void remove_interactive(object_t* ob, int dested);

namespace {

struct per_vhost_data {
  struct lws_context* context;
  struct lws_vhost* vhost;
  const struct lws_protocols* protocol;
};

// Safely resolve a path within the project root
std::filesystem::path resolve_project_path(const std::filesystem::path& project_root,
                                           const std::string& relative_path) {
  std::filesystem::path normalized_root = project_root.lexically_normal();
  std::filesystem::path resolved_path = normalized_root / relative_path;
  resolved_path = resolved_path.lexically_normal();

  debug_message("ws_debug: normalized_root '%s'\n", normalized_root.c_str());
  debug_message("ws_debug: resolved_path before check '%s'\n", resolved_path.c_str());

  // Ensure the resolved path is still within the project_root_path
  auto pos = resolved_path.string().rfind(normalized_root.string(), 0);
  debug_message("ws_debug: rfind result: %zu (should be 0)\n", pos);

  if (pos != 0) {
    debug_message("ws_debug: FAILED security check, returning empty\n");
    return {};  // Return empty path if outside root
  }
  return resolved_path;
}

nlohmann::json handle_websocket_compile(const nlohmann::json& request) {
  nlohmann::json result_json;
  if (!request.contains("path")) {
    result_json["error"] = "Missing 'path' in WebSocket compile request";
    return result_json;
  }
  const std::string file_path = request["path"].get<std::string>();

  if (!master_ob) {
    result_json["error"] = "VM not initialized. Call init() first.";
    return result_json;
  }
  current_object = master_ob;

  object_t* obj = nullptr;
  error_context_t econ{};
  save_context(&econ);
  try {
    obj = find_object(file_path.c_str());
  } catch (...) {
    restore_context(&econ);
    result_json["error"] = "Error during C++ compilation/linking.";
    return result_json;
  }
  pop_context(&econ);

  if (obj == nullptr || obj->prog == nullptr) {
    result_json["error"] = "Failed to load LPC object. It may have a syntax error.";
    return result_json;
  }

  result_json["path"] = obj->prog->filename;
  result_json["inherits"] = obj->prog->num_inherited;
  result_json["program_size"] = obj->prog->program_size;

  FILE* temp_fp = tmpfile();
  if (!temp_fp) {
    result_json["error"] = "Could not create temporary file to capture output.";
    return result_json;
  }
  dump_prog(obj->prog, temp_fp, 1 | 2);
  rewind(temp_fp);
  fseek(temp_fp, 0, SEEK_END);
  long size = ftell(temp_fp);
  rewind(temp_fp);
  std::string disassembly_output(size, '\0');
  fread(&disassembly_output[0], 1, size, temp_fp);
  fclose(temp_fp);

  result_json["disassembly"] = disassembly_output;
  return result_json;
}

nlohmann::json handle_websocket_files(const nlohmann::json& request) {
  nlohmann::json response_data = nlohmann::json::object();
  nlohmann::json file_list = nlohmann::json::array();
  std::string relative_dir_path = ".";
  if (request.contains("path")) {
    relative_dir_path = request["path"].get<std::string>();
  }

  std::filesystem::path project_root_path = std::filesystem::absolute(CONFIG_STR(__MUD_LIB_DIR__));
  std::filesystem::path dir_path = resolve_project_path(project_root_path, relative_dir_path);

  debug_message("ws_debug: files request for '%s'\n", relative_dir_path.c_str());
  debug_message("ws_debug: project_root '%s'\n", project_root_path.c_str());
  debug_message("ws_debug: resolved_path '%s'\n", dir_path.c_str());

  if (dir_path.empty() || !std::filesystem::exists(dir_path) ||
      !std::filesystem::is_directory(dir_path)) {
    debug_message("ws_debug: directory not found or invalid\n");
    response_data["error"] = "Directory not found or forbidden";
    return response_data;
  }

  int count = 0;
  for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
    file_list.push_back(
        {{"name", entry.path().filename().string()}, {"isDirectory", entry.is_directory()}});
    count++;
  }
  debug_message("ws_debug: found %d files\n", count);
  response_data["files"] = file_list;
  return response_data;
}

nlohmann::json handle_websocket_file_get(const nlohmann::json& request) {
  nlohmann::json result_json;
  if (!request.contains("path")) {
    result_json["error"] = "Missing 'path' in WebSocket file_get request";
    return result_json;
  }
  const std::string relative_file_path = request["path"].get<std::string>();
  std::filesystem::path project_root_path = std::filesystem::absolute(CONFIG_STR(__MUD_LIB_DIR__));
  std::filesystem::path file_path = resolve_project_path(project_root_path, relative_file_path);
  if (file_path.empty() || !std::filesystem::exists(file_path) ||
      !std::filesystem::is_regular_file(file_path)) {
    result_json["error"] = "File not found or forbidden";
    return result_json;
  }
  std::ifstream ifs(file_path);
  std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
  result_json["content"] = content;
  return result_json;
}

nlohmann::json handle_websocket_file_post(const nlohmann::json& request) {
  nlohmann::json result_json;
  if (!request.contains("path") || !request.contains("content")) {
    result_json["error"] = "Missing 'path' or 'content' in WebSocket file_post request";
    return result_json;
  }
  const std::string relative_file_path = request["path"].get<std::string>();
  const std::string content = request["content"].get<std::string>();

  std::filesystem::path project_root_path = std::filesystem::absolute(CONFIG_STR(__MUD_LIB_DIR__));
  std::filesystem::path file_path = resolve_project_path(project_root_path, relative_file_path);
  if (file_path.empty() || !std::filesystem::exists(file_path.parent_path()) ||
      !std::filesystem::is_directory(file_path.parent_path())) {
    result_json["error"] = "Parent directory not found or forbidden";
    return result_json;
  }
  std::ofstream ofs(file_path);
  if (!ofs.is_open()) {
    result_json["error"] = "Could not open file for writing";
    return result_json;
  }
  ofs << content;
  result_json["message"] = "File saved successfully";
  return result_json;
}

nlohmann::json handle_websocket_objects() {
  nlohmann::json result_json;
  nlohmann::json objects_array = nlohmann::json::array();

  if (!master_ob) {
    result_json["error"] = "VM not initialized";
    return result_json;
  }

  // Get all objects using FluffOS get_objects function
  object_t** list = nullptr;
  int count = 0;
  get_objects(&list, &count, nullptr, nullptr);

  // Iterate through all objects and collect info
  for (int i = 0; i < count; i++) {
    object_t* ob = list[i];
    if (!ob || (ob->flags & O_DESTRUCTED)) {
      continue;
    }

    nlohmann::json obj_info;
    obj_info["obname"] = ob->obname;
    obj_info["load_time"] = ob->load_time;
    obj_info["flags"] = ob->flags;
    obj_info["ref_count"] = ob->ref;

    objects_array.push_back(obj_info);
  }

  // Clean up the list
  pop_stack();  // get_objects pushes the list onto the stack

  result_json["objects"] = objects_array;
  debug_message("ws_debug: returned %d objects\n", (int)objects_array.size());
  return result_json;
}

nlohmann::json handle_websocket_object_inspect(const nlohmann::json& request) {
  nlohmann::json result_json;

  if (!request.contains("obname")) {
    result_json["error"] = "Missing 'obname' in WebSocket object_inspect request";
    return result_json;
  }

  const std::string obname = request["obname"].get<std::string>();

  if (!master_ob) {
    result_json["error"] = "VM not initialized";
    return result_json;
  }

  // Find the object
  object_t* ob = find_object(obname.c_str());
  if (!ob || (ob->flags & O_DESTRUCTED)) {
    result_json["error"] = "Object not found or destructed";
    return result_json;
  }

  result_json["obname"] = ob->obname;
  result_json["load_time"] = ob->load_time;
  result_json["flags"] = ob->flags;
  result_json["ref_count"] = ob->ref;

  // Get variables
  nlohmann::json variables_array = nlohmann::json::array();
  if (ob->prog) {
    for (int i = 0; i < ob->prog->num_variables_defined; i++) {
      nlohmann::json var_info;
      var_info["name"] = ob->prog->variable_table[i];

      // Get variable value as string
      svalue_t* val = &ob->variables[i];
      std::string value_str;
      switch (val->type) {
        case T_NUMBER:
          value_str = std::to_string(val->u.number);
          break;
        case T_STRING:
          value_str = "\"" + std::string(val->u.string) + "\"";
          break;
        case T_OBJECT:
          value_str = val->u.ob ? val->u.ob->obname : "0";
          break;
        case T_ARRAY:
          value_str = "array[" + std::to_string(val->u.arr->size) + "]";
          break;
        case T_MAPPING:
          value_str = "mapping";
          break;
        default:
          value_str = "unknown";
      }

      var_info["value"] = value_str;
      var_info["type"] = val->type;
      variables_array.push_back(var_info);
    }
  }
  result_json["variables"] = variables_array;

  // Get functions
  nlohmann::json functions_array = nlohmann::json::array();
  if (ob->prog) {
    for (int i = 0; i < ob->prog->num_functions_defined; i++) {
      functions_array.push_back(ob->prog->function_table[i].funcname);
    }
  }
  result_json["functions"] = functions_array;

  // Get inheritance info
  nlohmann::json inherits_array = nlohmann::json::array();
  if (ob->prog) {
    for (int i = 0; i < ob->prog->num_inherited; i++) {
      inherits_array.push_back(ob->prog->inherit[i].prog->filename);
    }
  }
  result_json["inherits"] = inherits_array;

  debug_message("ws_debug: inspected object %s (%d vars, %d functions, %d inherits)\n",
                obname.c_str(), (int)variables_array.size(), (int)functions_array.size(),
                (int)inherits_array.size());

  return result_json;
}

void send_websocket_json_response(struct lws* wsi, const nlohmann::json& response_json) {
  std::string response_str = response_json.dump();
  // libwebsockets requires a certain amount of padding for WebSocket frames
  // LWS_PRE is the recommended padding size
  size_t len = response_str.length();
  size_t buffered_len = LWS_PRE + len;
  unsigned char* buf = new unsigned char[buffered_len];
  memcpy(buf + LWS_PRE, response_str.c_str(), len);

  lws_write(wsi, buf + LWS_PRE, len, LWS_WRITE_TEXT);
  delete[] buf;
}

}  // namespace

int ws_debug_callback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in,
                      size_t len) {
  auto* pss = (ws_debug_session*)user;
  auto* vhd =
      (struct per_vhost_data*)lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));

  switch (reason) {
    case LWS_CALLBACK_PROTOCOL_INIT:
      vhd = reinterpret_cast<per_vhost_data*>(lws_protocol_vh_priv_zalloc(
          lws_get_vhost(wsi), lws_get_protocol(wsi), sizeof(struct per_vhost_data)));
      vhd->context = lws_get_context(wsi);
      vhd->protocol = lws_get_protocol(wsi);
      vhd->vhost = lws_get_vhost(wsi);
      break;

    case LWS_CALLBACK_ESTABLISHED: {
      char name[256] = {};
      size_t namelen = sizeof(name);
      debug_message("ws_debug: LWS_CALLBACK_ESTABLISHED, remote=%s\n",
                    lws_get_peer_simple(wsi, name, namelen));
      break;
    }

    case LWS_CALLBACK_CLOSED: {
      debug_message("ws_debug: LWS_CALLBACK_CLOSED\n");
      break;
    }

    case LWS_CALLBACK_RECEIVE: {
      debug_message("ws_debug: LWS_CALLBACK_RECEIVE\n");
      nlohmann::json response_json;
      try {
        std::string request_str((char*)in, len);
        nlohmann::json request_json = nlohmann::json::parse(request_str);
        debug_message("ws_debug: received request: %s\n", request_str.c_str());

        if (!request_json.contains("command")) {
          response_json["error"] = "Missing 'command' in WebSocket request";
        } else {
          std::string command = request_json["command"].get<std::string>();
          if (request_json.contains("requestId")) {
            response_json["requestId"] = request_json["requestId"];
          }

          if (command == "compile") {
            response_json.update(handle_websocket_compile(request_json));
          } else if (command == "files") {
            response_json.update(handle_websocket_files(request_json));
          } else if (command == "file_get") {
            response_json.update(handle_websocket_file_get(request_json));
          } else if (command == "file_post") {
            response_json.update(handle_websocket_file_post(request_json));
          } else if (command == "objects") {
            response_json.update(handle_websocket_objects());
          } else if (command == "object_inspect") {
            response_json.update(handle_websocket_object_inspect(request_json));
          } else {
            response_json["error"] = "Unknown WebSocket command: " + command;
          }
        }
      } catch (const nlohmann::json::parse_error& e) {
        response_json["error"] = "JSON parse error: " + std::string(e.what());
      } catch (const std::exception& e) {
        response_json["error"] = "Error handling WebSocket message: " + std::string(e.what());
      }

      send_websocket_json_response(wsi, response_json);
      break;
    }

    default:
      break;
  }
  return 0;
}
