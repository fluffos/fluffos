#include "base/std.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <map>

#include "vm/vm.h"
#include "mainlib.h"
#include "compiler/internal/disassembler.h"
#include "base/internal/rc.h"

#include "lws_wrapper/LwsHttpServer.h"
#include "lws_wrapper/LWSHandler.h"
#include "lws_wrapper/IWebSocketHandler.h"

// Forward declaration from disassembler.cc
void dump_prog(program_t *prog, FILE *f, int options);

// --- Configuration ---
const int IDE_API_SERVER_PORT = 3001;

// --- MainIdeLogic class implementing IWebSocketHandler ---
class MainIdeLogic : public IWebSocketHandler {
public:
    MainIdeLogic(const std::filesystem::path& project_root) : project_root_path_(project_root) {}

    // --- IWebSocketHandler implementations ---
    nlohmann::json handle_websocket_compile(const nlohmann::json& request) override {
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

    nlohmann::json handle_websocket_files(const nlohmann::json& request) override {
        nlohmann::json response_data = nlohmann::json::object();
        nlohmann::json file_list = nlohmann::json::array();
        std::string relative_dir_path = ".";
        if (request.contains("path")) {
            relative_dir_path = request["path"].get<std::string>();
        }
        std::filesystem::path dir_path = resolve_project_path(project_root_path_, relative_dir_path);

        if (dir_path.empty() || !std::filesystem::exists(dir_path) || !std::filesystem::is_directory(dir_path)) {
            response_data["error"] = "Directory not found or forbidden";
            return response_data;
        }

        for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
            file_list.push_back({
                {"name", entry.path().filename().string()},
                {"isDirectory", entry.is_directory()}
            });
        }
        response_data["files"] = file_list;
        return response_data;
    }

    nlohmann::json handle_websocket_file_get(const nlohmann::json& request) override {
        nlohmann::json result_json;
        if (!request.contains("path")) {
            result_json["error"] = "Missing 'path' in WebSocket file_get request";
            return result_json;
        }
        const std::string relative_file_path = request["path"].get<std::string>();
        std::filesystem::path file_path = resolve_project_path(project_root_path_, relative_file_path);
        if (file_path.empty() || !std::filesystem::exists(file_path) || !std::filesystem::is_regular_file(file_path)) {
            result_json["error"] = "File not found or forbidden";
            return result_json;
        }
        std::ifstream ifs(file_path);
        std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        result_json["content"] = content;
        return result_json;
    }

    nlohmann::json handle_websocket_file_post(const nlohmann::json& request) override {
        nlohmann::json result_json;
        if (!request.contains("path") || !request.contains("content")) {
            result_json["error"] = "Missing 'path' or 'content' in WebSocket file_post request";
            return result_json;
        }
        const std::string relative_file_path = request["path"].get<std::string>();
        const std::string content = request["content"].get<std::string>();

        std::filesystem::path file_path = resolve_project_path(project_root_path_, relative_file_path);
        if (file_path.empty() || !std::filesystem::exists(file_path.parent_path()) || !std::filesystem::is_directory(file_path.parent_path())) {
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

private:
    std::filesystem::path project_root_path_;

    // Safely resolve a path within the project root
    std::filesystem::path resolve_project_path(const std::filesystem::path& project_root, const std::string& relative_path) {
        std::filesystem::path resolved_path = project_root / relative_path;
        resolved_path = resolved_path.lexically_normal();
        // Ensure the resolved path is still within the project_root_path
        if (!resolved_path.string().rfind(project_root.string(), 0) == 0) {
            return {}; // Return empty path if outside root
        }
        return resolved_path;
    }
};

// --- Main Server Entrypoint ---
int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./ide_api_server <config_file>" << std::endl;
        return 1;
    }
    std::filesystem::path config_file_path = argv[1];

    // Initialize FluffOS VM
    if (!std::filesystem::exists(config_file_path)) {
        std::cerr << "Error: config file not found at " << config_file_path << std::endl;
        return 1;
    }
    init_main( (char*)config_file_path.string().c_str() );
    vm_start();

    // Get project_root_path from runtime config and resolve to absolute path
    std::filesystem::path project_root_path = std::filesystem::absolute(CONFIG_STR(__MUD_LIB_DIR__));
    if (project_root_path.empty()) {
        std::cerr << "Error: __MUD_LIB_DIR__ not set in config file." << std::endl;
        return 1;
    }

    MainIdeLogic main_logic(project_root_path);
    LWSHandler lws_handler(&main_logic, project_root_path);
    LwsHttpServer server(IDE_API_SERVER_PORT, &lws_handler);
    if (!server.init()) {
        return 1;
    }

    server.run();

    clear_state();
    return 0;
}
