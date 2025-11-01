#include "LwsHttpServer.h"

#include <iostream>
#include <string.h>

LwsHttpServer::LwsHttpServer(int port, LWSHandler* lwsHandler)
    : port_(port), lwsHandler_(lwsHandler), context_(nullptr) {}

LwsHttpServer::~LwsHttpServer() {
    if (context_) {
        lws_context_destroy(context_);
    }
}

bool LwsHttpServer::init() {
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    info.port = port_;
    info.protocols = lwsHandler_->get_protocols();
    info.gid = -1;
    info.uid = -1;
    info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
    // Set the user data for the context to be a pointer to the LWSHandler instance
    // This allows the static callback in LWSHandler to retrieve its instance.
    info.user = lwsHandler_;

    context_ = lws_create_context(&info);
    if (!context_) {
        std::cerr << "lws_create_context failed" << std::endl;
        return false;
    }

    std::cout << "IDE API Server listening on http://localhost:" << port_ << std::endl;
    std::cout << "Project root for file operations: " << lwsHandler_->get_project_root_path() << std::endl;
    return true;
}

void LwsHttpServer::run() {
    while (1) {
        lws_service(context_, 0);
    }
}
