#include "base/std.h"

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/util.h>
#include <event2/listener.h>
#include <libwebsockets.h>

#include "net/ws_ascii.h"
#include "net/ws_telnet.h"

enum PROTOCOL_ID {
  WS_HTTP = 0,
  WS_ASCII = PROTOCOL_WS_ASCII,
  WS_TELNET = PROTOCOL_WS_TELNET,
};

static struct lws_protocols protocols[] = {
    {"http", lws_callback_http_dummy, 0, 0, WS_HTTP},
    {"ascii", ws_ascii_callback, sizeof(struct ws_session), 4096, WS_ASCII},
    {"telnet", ws_telnet_callback, sizeof(struct ws_session), 4096, WS_TELNET},
    // for backward compatiblity with fluffos 2.x
    {"binary", ws_telnet_callback, sizeof(struct ws_session), 4096, WS_TELNET},
    {NULL, NULL, 0, 0} /* terminator */
};

static const struct lws_extension extensions[] = {
    {"permessage-deflate", lws_extension_callback_pm_deflate,
     "permessage-deflate"
     "; client_no_context_takeover"
     "; client_max_window_bits"},
    {NULL, NULL, NULL /* terminator */}};

// Field-by-field init (not positional aggregate init): lws has inserted new
// members mid-struct across releases, which silently shifts positional
// initializers. Remaining fields are zero/nullptr; origin is set on create.
static struct lws_http_mount init_mount() {
  struct lws_http_mount m = {};
  m.mountpoint = "/";               /* mountpoint URL */
  m.def = "index.html";             /* default filename */
  m.origin_protocol = LWSMPRO_FILE; /* files in a dir */
  m.mountpoint_len = 1;             /* char count */
  return m;
}
static struct lws_http_mount mount = init_mount();

void lws_log(int severity, const char* msg) {
  if (severity == LLL_ERR) {
    debug(all, "lws ERROR: %s", msg);
  } else {
    debug(websocket, "lws %d: %s", severity, msg);
  }
}

struct lws_context* init_websocket_context(event_base* base, port_def_t* port) {
  int logs = LLL_USER | LLL_WARN | LLL_ERR;

#ifdef DEBUG
  logs |= LLL_WARN | LLL_NOTICE | LLL_INFO | LLL_DEBUG;
  // More debug levels
  /* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
  /* | LLL_EXT */ /* | LLL_CLIENT */  /* | LLL_LATENCY */
  /* | LLL_DEBUG */;
#endif
  lws_set_log_level(logs, lws_log);

  struct lws_context_creation_info info = {0};
  void* foreign_loops[1] = {base};

  info.foreign_loops = foreign_loops;

  DEBUG_CHECK(CONFIG_STR(__RC_WEBSOCKET_HTTP_DIR__) == nullptr, "Bug, no websocket http lib dir!");
  mount.origin = CONFIG_STR(__RC_WEBSOCKET_HTTP_DIR__);

  info.mounts = &mount;
  info.port = CONTEXT_PORT_NO_LISTEN_SERVER;
  info.protocols = protocols;
  info.extensions = extensions;
  info.pt_serv_buf_size = 128 * 1024;
  info.options = LWS_SERVER_OPTION_LIBEVENT | LWS_SERVER_OPTION_VALIDATE_UTF8;

  if (!port->tls_cert.empty() && !port->tls_key.empty()) {
    info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info.options |= LWS_SERVER_OPTION_ALLOW_NON_SSL_ON_SSL_PORT;
    info.options |= LWS_SERVER_OPTION_REDIRECT_HTTP_TO_HTTPS;
    info.ssl_cipher_list =
        "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:"
        "ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:DHE-"
        "RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384";
    info.ssl_cert_filepath = port->tls_cert.c_str();
    info.ssl_private_key_filepath = port->tls_key.c_str();
    info.ssl_options_clear = SSL_OP_CIPHER_SERVER_PREFERENCE;
    info.ssl_options_set = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3;
  }
  // info.options |= LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
  info.user = (void*)port;

  auto context = lws_create_context(&info);

  if (!context) {
    lwsl_err("lws init failed\n");
    return nullptr;
  }

  std::string res;
  for (auto& p : protocols) {
    if (p.name) {
      res += p.name;
      res += " ";
    }
  }
  lwsl_user("WS protocols supported: %s\n", res.c_str());

  return context;
}

struct lws* init_user_websocket(struct lws_context* context, evutil_socket_t fd) {
  // Since lws v4.3 the context's vhost list is headed by an internal "system"
  // vhost, which carries none of our ws protocols; lws_adopt_socket() adopts
  // onto the list head, so adopt explicitly onto our own vhost instead.
  auto* vhost = lws_get_vhost_by_name(context, "default");
  if (!vhost) {
    return nullptr;
  }
  return lws_adopt_socket_vhost(vhost, fd);
}

void websocket_send_text(struct lws* wsi, const char* data, size_t len) {
  inet_volume += len;
  switch (lws_get_protocol(wsi)->id) {
    case WS_TELNET:
      ws_telnet_send(wsi, data, len);
      break;
    case WS_ASCII:
      ws_ascii_send(wsi, data, len);
      break;
    default:
      // No way to send message
      return;
  }
}

void close_websocket_context(struct lws_context* context) { lws_context_destroy(context); }

void close_user_websocket(struct lws* wsi) {
  // NOTE: LWS_TO_KILL_ASYNC does not mean "give the peer a grace period to
  // drain" -- per lws-timeout-timer.h it marks the wsi to be killed "at the
  // next timeout check", i.e. essentially immediately. Using it here for the
  // matched-protocol case raced the requested writable callback below and
  // regularly won, silently dropping whatever was still queued in
  // pss->buffer (that evbuffer lives outside lws, so an async kill doesn't
  // wait for it). The real bounded drain window is the explicit `5` (seconds)
  // in the close_from_writable branch below; LWS_TO_KILL_ASYNC is only
  // correct for the fallback case, where there is no application buffer to
  // protect. See src/www/AGENTS.md's "Driver-initiated close is
  // close-AFTER-FLUSH" note for the four invariants this depends on.
  bool close_from_writable = false;
  switch (lws_get_protocol(wsi)->id) {
    case WS_TELNET:
    case WS_ASCII: {
      auto pss = reinterpret_cast<ws_session*>(lws_wsi_user(wsi));
      if (pss) {
        pss->close_after_flush = true;
        close_from_writable = true;
        pss->user = nullptr;
      }
      break;
    }
    default:
      break;
  }

  if (close_from_writable) {
    // The application-side evbuffer is outside lws, so an async kill can win
    // before the requested writable callback moves these final bytes into lws.
    // Give the application buffer a bounded drain; the protocol callback closes
    // as soon as it is empty, while this timeout covers a permanently choked peer.
    //
    // The session is also done READING: input that arrives during the drain
    // window is discarded by the protocol callbacks' nulled-pss->user guards
    // (LWS_CALLBACK_RECEIVE breaks instead of hard-closing, which would
    // truncate the very output this drain exists to flush). Note that
    // lws_rx_flow_control() is NOT the way to express that here: with the
    // libevent event lib, flipping POLLIN on a live wsi desyncs the fd
    // watcher and starves POLLOUT, stalling the drain until the deadline
    // kills it.
    lws_set_timeout(wsi, pending_timeout::PENDING_FLUSH_STORED_SEND_BEFORE_CLOSE, 5);
    lws_callback_on_writable(wsi);
    return;
  }

  lws_set_timeout(wsi, pending_timeout::PENDING_FLUSH_STORED_SEND_BEFORE_CLOSE, LWS_TO_KILL_ASYNC);
}
