#ifndef LWS_HANDLER_H
#define LWS_HANDLER_H

#include <libwebsockets.h>
#include <string>
#include <map>
#include <filesystem>

#include "IWebSocketHandler.h"

class LWSHandler {
public:
    LWSHandler(IWebSocketHandler* ws_handler, const std::filesystem::path& project_root);
    ~LWSHandler() = default;

    const struct lws_protocols* get_protocols() const { return protocols_; }
    const std::filesystem::path& get_project_root_path() const { return project_root_path_; }

    // Static callback function required by libwebsockets for WebSocket
    static int lws_callback_websocket(struct lws *wsi, enum lws_callback_reasons reason,
                                      void *user, void *in, size_t len);

private:
    IWebSocketHandler* ws_handler_;
    std::filesystem::path project_root_path_;

    // Member function to handle WebSocket messages, called by the static callback
    int handle_websocket_message(struct lws *wsi, void *in, size_t len);

    // Helper to send WebSocket JSON response
    void send_websocket_json_response(struct lws *wsi, const nlohmann::json& response_json);

    // Static protocols array for libwebsockets
    static struct lws_protocols protocols_[];
};

#endif // LWS_HANDLER_H
