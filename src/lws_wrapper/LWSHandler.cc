#include "LWSHandler.h"
#include "../../base/std.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>

// Static protocols array for libwebsockets
struct lws_protocols LWSHandler::protocols_[] = {
    { "api", LWSHandler::lws_callback_websocket, 0, 0, 0, NULL, 0 }, // Only WebSocket protocol
    { NULL, NULL, 0, 0, 0, NULL, 0 } /* terminator */
};

LWSHandler::LWSHandler(IWebSocketHandler* ws_handler, const std::filesystem::path& project_root)
    : ws_handler_(ws_handler), project_root_path_(project_root) {}

// Static callback function required by libwebsockets for WebSocket
int LWSHandler::lws_callback_websocket(struct lws *wsi, enum lws_callback_reasons reason,
                                      void *user, void *in, size_t len) {
    LWSHandler* self = static_cast<LWSHandler*>(lws_context_user(lws_get_context(wsi)));
    if (!self) {
        lwsl_err("LWSHandler instance not found in context user data\n");
        return -1;
    }

    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            std::cout << "WebSocket established!" << std::endl;
            break;
        case LWS_CALLBACK_RECEIVE:
            return self->handle_websocket_message(wsi, in, len);
        case LWS_CALLBACK_CLOSED:
            std::cout << "WebSocket closed." << std::endl;
            break;
        default:
            break;
    }
    return 0;
}

// Member function to handle WebSocket messages
int LWSHandler::handle_websocket_message(struct lws *wsi, void *in, size_t len) {
    nlohmann::json response_json;
    try {
        std::string request_str((char*)in, len);
        nlohmann::json request_json = nlohmann::json::parse(request_str);

        if (!request_json.contains("command")) {
            response_json["error"] = "Missing 'command' in WebSocket request";
        } else {
            std::string command = request_json["command"].get<std::string>();
            // Include requestId in the response if present in the request
            if (request_json.contains("requestId")) {
                response_json["requestId"] = request_json["requestId"];
            }

            if (command == "compile") {
                response_json.update(ws_handler_->handle_websocket_compile(request_json));
            } else if (command == "files") {
                response_json.update(ws_handler_->handle_websocket_files(request_json));
            } else if (command == "file_get") {
                response_json.update(ws_handler_->handle_websocket_file_get(request_json));
            } else if (command == "file_post") {
                response_json.update(ws_handler_->handle_websocket_file_post(request_json));
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
    return 0;
}

// Helper to send WebSocket JSON response
void LWSHandler::send_websocket_json_response(struct lws *wsi, const nlohmann::json& response_json) {
    std::string response_str = response_json.dump();
    // libwebsockets requires a certain amount of padding for WebSocket frames
    // LWS_PRE is the recommended padding size
    size_t len = response_str.length();
    size_t buffered_len = LWS_PRE + len;
    unsigned char *buf = new unsigned char[buffered_len];
    memcpy(buf + LWS_PRE, response_str.c_str(), len);

    lws_write(wsi, buf + LWS_PRE, len, LWS_WRITE_TEXT);
    delete[] buf;
}
