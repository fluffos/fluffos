#include "base/std.h"

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/util.h>
#include <event2/listener.h>
#include <libwebsockets.h>

#include "net/ws_ascii.h"

enum PROTOCOL_ID {
  WS_HTTP = 0,
  WS_ASCII = PROTOCOL_WS_ASCII,
};

static struct lws_protocols protocols[] = {
    {"http", lws_callback_http_dummy, 0, 0, WS_HTTP},
    {"ascii", ws_ascii_callback, sizeof(struct ws_ascii_session), 4096, WS_ASCII},
    {NULL, NULL, 0, 0} /* terminator */
};

static const struct lws_extension extensions[] = {
    {"permessage-deflate", lws_extension_callback_pm_deflate,
     "permessage-deflate"
     "; client_no_context_takeover"
     "; client_max_window_bits"},
    {NULL, NULL, NULL /* terminator */}};

// modified on create.
static struct lws_http_mount mount = {
    /* .mount_next */ nullptr, /* linked-list "next" */
    /* .mountpoint */ "/",     /* mountpoint URL */
    /* .origin */ nullptr,     /* serve from dir */
    /* .def */ "index.html",   /* default filename */
    /* .protocol */ nullptr,
    /* .cgienv */ nullptr,
    /* .extra_mimetypes */ nullptr,
    /* .interpret */ nullptr,
    /* .cgi_timeout */ 0,
    /* .cache_max_age */ 0,
    /* .auth_mask */ 0,
    /* .cache_reusable */ 0,
    /* .cache_revalidate */ 0,
    /* .cache_intermediaries */ 0,
    /* .origin_protocol */ LWSMPRO_FILE, /* files in a dir */
    /* .mountpoint_len */ 1,             /* char count */
    /* .basic_auth_login_file */ nullptr,
};

void lws_log(int severity, const char *msg) { debug(websocket, "lws:%d:%s", severity, msg); }

struct lws_context *init_websocket_context(event_base *base, port_def_t *port) {
  int logs = LLL_USER | LLL_WARN | LLL_ERR;

#ifdef DEBUG
  logs |= LLL_WARN | LLL_NOTICE | LLL_INFO;
  // More debug levels
  /* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
  /* | LLL_EXT */ /* | LLL_CLIENT */  /* | LLL_LATENCY */
  /* | LLL_DEBUG */;
#endif
  lws_set_log_level(logs, lws_log);

  struct lws_context_creation_info info = {0};
  void *foreign_loops[1] = {base};

  info.foreign_loops = foreign_loops;

  DEBUG_CHECK(CONFIG_STR(__RC_WEBSOCKET_HTTP_DIR__) == nullptr, "Bug, no websocket http lib dir!");
  mount.origin = CONFIG_STR(__RC_WEBSOCKET_HTTP_DIR__);

  info.mounts = &mount;
  info.port = CONTEXT_PORT_NO_LISTEN_SERVER;
  info.protocols = protocols;
  info.extensions = extensions;
  info.pt_serv_buf_size = 32 * 1024;
  info.options = LWS_SERVER_OPTION_LIBEVENT | LWS_SERVER_OPTION_VALIDATE_UTF8;
  // info.options |= LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
  info.user = (void *)port;

  auto context = lws_create_context(&info);

  if (!context) {
    lwsl_err("lws init failed\n");
    return nullptr;
  }

  std::string res;
  for (auto &p : protocols) {
    if (p.name) {
      res += p.name;
      res += " ";
    }
  }
  lwsl_user("WS protocols supported: %s\n", res.c_str());

  return context;
}

struct lws *init_user_websocket(struct lws_context *context, evutil_socket_t fd) {
  return lws_adopt_socket(context, fd);
}

void websocket_send_text(struct lws *wsi, const char *data, size_t len) {
  switch (lws_get_protocol(wsi)->id) {
    case WS_ASCII:
      ws_ascii_send(wsi, data, len);
      break;
    default:
      // No way to send message
      return;
  }
}

void close_websocket_context(struct lws_context *context) { lws_context_destroy(context); }

void close_user_websocket(struct lws *wsi) {
  lws_set_timeout(wsi, pending_timeout::PENDING_FLUSH_STORED_SEND_BEFORE_CLOSE, LWS_TO_KILL_ASYNC);
  switch (lws_get_protocol(wsi)->id) {
    case WS_ASCII: {
      auto pss = reinterpret_cast<ws_ascii_session *>(lws_wsi_user(wsi));
      if (pss) {
        if (evbuffer_get_length(pss->buffer) > 0) {
          // try flush before closing.
          lws_callback_on_writable(wsi);
        }
        pss->user = nullptr;
      }
      break;
    }
    default:
      break;
  }
}
