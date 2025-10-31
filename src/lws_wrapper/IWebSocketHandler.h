#ifndef I_WEBSOCKET_HANDLER_H
#define I_WEBSOCKET_HANDLER_H

#include <string>
#include <nlohmann/json.hpp>
#include <filesystem>

class IWebSocketHandler {
public:
    virtual ~IWebSocketHandler() = default;

    // WebSocket API Handlers
    virtual nlohmann::json handle_websocket_compile(const nlohmann::json& request) = 0;
    virtual nlohmann::json handle_websocket_files(const nlohmann::json& request) = 0;
    virtual nlohmann::json handle_websocket_file_get(const nlohmann::json& request) = 0;
    virtual nlohmann::json handle_websocket_file_post(const nlohmann::json& request) = 0;
};

#endif // I_WEBSOCKET_HANDLER_H
