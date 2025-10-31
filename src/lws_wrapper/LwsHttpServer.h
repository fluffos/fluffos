#ifndef LWS_HTTP_SERVER_H
#define LWS_HTTP_SERVER_H

#include <libwebsockets.h>
#include <filesystem>

#include "LWSHandler.h"

class LwsHttpServer {
public:
    LwsHttpServer(int port, LWSHandler* lwsHandler);
    ~LwsHttpServer();

    bool init();
    void run();

private:
    int port_;
    LWSHandler* lwsHandler_;
    struct lws_context *context_;
};

#endif // LWS_HTTP_SERVER_H
