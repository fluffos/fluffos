#include "base/std.h"

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/util.h>
#include <event2/listener.h>

#include <libwebsockets.h>

#include "net/ws_common.h"
#include "comm.h"
#include "net/transport_native.h"
#include "interactive.h"

namespace {

/* one of these is created for each vhost our protocol is used with */
struct per_vhost_data {
  struct lws_context* context;
  struct lws_vhost* vhost;
  const struct lws_protocols* protocol;

  ws_session* pss_list; /* linked-list of live pss */
};

}  // namespace

void ws_handle_protocol_init(struct lws* wsi) {
  lwsl_info("LWS_CALLBACK_PROTOCOL_INIT\n");
  // freed automatically when context is destroyed.
  auto* vhd = reinterpret_cast<per_vhost_data*>(lws_protocol_vh_priv_zalloc(
      lws_get_vhost(wsi), lws_get_protocol(wsi), sizeof(struct per_vhost_data)));
  vhd->context = lws_get_context(wsi);
  vhd->protocol = lws_get_protocol(wsi);
  vhd->vhost = lws_get_vhost(wsi);
}

int ws_handle_established(struct lws* wsi, ws_session* pss,
                          void (*transport_setup)(struct interactive_t*)) {
  /* generate a block of output before travis times us out */
  lwsl_info("LWS_CALLBACK_ESTABLISHED\n");

  auto port = (port_def_t*)lws_context_user(lws_get_context(wsi));
  auto fd = lws_get_socket_fd(lws_get_network_wsi(wsi));

  sockaddr_storage addr = {0};
  socklen_t addrlen = sizeof(addr);
  auto result = getpeername(fd, reinterpret_cast<sockaddr*>(&addr), &addrlen);
  if (result) {
    lwsl_warn("LWS_CALLBACK_ESTABLISHED: getpeername error, %d\n", evutil_socket_geterror(fd));
    return -1;  // TODO: maybe do something else?
  }

  // Process X-REAL-IP
  {
    char buf[64];  // maximum characters of ip address is 45.
    auto buflen = lws_hdr_copy(wsi, buf, sizeof(buf), WSI_TOKEN_HTTP_X_REAL_IP);
    if (buflen > 0) {
      struct evutil_addrinfo hints = {0};
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = AI_NUMERICHOST;
      hints.ai_protocol = 0; /* Any protocol */

      struct evutil_addrinfo* res = nullptr;
      auto ret = evutil_getaddrinfo(buf, nullptr, &hints, &res);
      if (ret) {
        lwsl_warn("LWS_CALLBACK_ESTABLISHED: invalid X-REAL-IP : %s , error: %s.\n", buf,
                  evutil_gai_strerror(ret));
        return false;
      }
      if (res && res->ai_addrlen > 0) {
        memcpy(&addr, res->ai_addr, res->ai_addrlen);
        addrlen = res->ai_addrlen;

        evutil_freeaddrinfo(res);
      }
    }
  }

  auto ip = new_user(port, fd, reinterpret_cast<sockaddr*>(&addr), addrlen);

  pss->user = ip;
  pss->buffer = evbuffer_new();

  ip->iflags |= HANDSHAKE_COMPLETE;
  ip->lws = wsi;

  // handshake complete
  if (transport_setup) {
    transport_setup(ip);
  }

  auto base = evconnlistener_get_base(port->ev_conn);
  event_base_once(
      base, -1, EV_TIMEOUT,
      [](evutil_socket_t fd, short what, void* arg) {
        auto user = reinterpret_cast<interactive_t*>(arg);
        on_user_logon(user);
      },
      (void*)ip, nullptr);
  return 0;
}

void ws_handle_closed(ws_session* pss) {
  auto* ip = pss->user;
  if (ip) {
    // lws is already tearing this wsi down; for ws-over-h2 the stream's h2
    // parent is unlinked before this callback fires, so nothing may call back
    // into lws on it. Drop the handle first: remove_interactive flushes output
    // and runs the net_dead apply before transport close, and
    // close_user_websocket() would request a writeable callback that derefs the
    // missing h2 parent (SIGSEGV on release builds).
    ip->lws = nullptr;
    remove_interactive(ip->ob, 0);
    pss->user = nullptr;
  }
  // Always free the session resources: on driver-initiated closes
  // close_user_websocket() has already nulled pss->user, and an early return
  // here used to leak the evbuffer on that path.
  if (pss->buffer) {
    evbuffer_free(pss->buffer);
    pss->buffer = nullptr;
  }
}

int ws_handle_writeable(struct lws* wsi, ws_session* pss, ws_truncate_fn truncate) {
  lwsl_info("LWS_CALLBACK_SERVER_WRITEABLE\n");

  // All sending happens here (lws rule: only write from the WRITEABLE
  // callback); a callback with nothing to do is a harmless no-op. Drain as many
  // windows as the socket accepts, gated on lws_send_pipe_choked(). Choked can
  // simply mean the socket is full right now (zero-timeout poll) with nothing
  // pending inside lws -- lws will NOT re-arm the callback on its own in that
  // case, so the exit below must re-request it whenever data remains, or output
  // wedges for good on any burst that fills the kernel send buffer while the
  // peer reads slowly.
  static unsigned char buf[LWS_PRE + 2048];
  while (evbuffer_get_length(pss->buffer) > 0 && !lws_send_pipe_choked(wsi)) {
    auto numbytes = evbuffer_copyout(pss->buffer, &buf[LWS_PRE], sizeof(buf) - LWS_PRE);
    if (numbytes <= 0) {
      break;
    }
    // Hold back a trailing partial protocol unit so it is never split across ws
    // messages. evbuffer_copyout() does not drain, so the held-back bytes stay
    // at the front of pss->buffer and go out with the next write.
    if (truncate) {
      auto new_numbytes = static_cast<ev_ssize_t>(
          truncate(pss, &buf[LWS_PRE], numbytes,
                   numbytes == static_cast<ev_ssize_t>(sizeof(buf) - LWS_PRE)));
      if (new_numbytes == 0) {
        // Only an incomplete unit is buffered; the exit below keeps a writeable
        // callback requested until the rest of it arrives.
        break;
      }
      numbytes = new_numbytes;
    }
    // TODO: we could use LWS_WRITE_TEXT, however it is much safer to use binary
    // mode, its better to let the client deal with incorrect encoding.
    auto m = lws_write(wsi, buf + LWS_PRE, numbytes, LWS_WRITE_BINARY);
    // A short return means the connection failed. On success lws consumed the
    // whole payload (partials are buffered and flushed internally -- for
    // ws-over-h2 that includes whole frames parked inside lws while the peer's
    // flow-control window is exhausted), and the return can EXCEED numbytes on
    // TLS -- never use it as a drain count.
    if (m < static_cast<int>(numbytes)) {
      lwsl_warn("ERROR %d writing to ws socket.\n", m);
      return -1;
    }
    evbuffer_drain(pss->buffer, numbytes);
  }
  // Data still queued: request the next writeable callback. Never rely on lws
  // to re-arm it (see above) -- this keeps the invariant that queued data always
  // has a callback requested.
  if (evbuffer_get_length(pss->buffer) > 0) {
    lws_callback_on_writable(wsi);
  }
  return 0;
}

void ws_queue_send(struct lws* wsi, int expected_protocol, const char* data, size_t len) {
  DEBUG_CHECK(lws_get_protocol(wsi)->id != expected_protocol, "wrong protocol!");
  auto pss = reinterpret_cast<ws_session*>(lws_wsi_user(wsi));
  DEBUG_CHECK(pss == nullptr, "no session data!");

  evbuffer_add(pss->buffer, data, len);
  lws_callback_on_writable(wsi);
}
