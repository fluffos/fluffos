#include "base/std.h"

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/util.h>
#include <event2/listener.h>

#include <libwebsockets.h>

#include "net/ws_ascii.h"
#include "interactive.h"

// from comm.cc
interactive_t *new_user(port_def_t *port, evutil_socket_t fd, sockaddr *addr, socklen_t addrlen);
extern void on_user_logon(interactive_t *);
extern void remove_interactive(object_t *ob, int dested);
int cmd_in_buf(interactive_t *ip);

void on_user_websocket_received(interactive_t *ip, const char *data, size_t len);

namespace {

/* one of these is created for each vhost our protocol is used with */
struct per_vhost_data {
  struct lws_context *context;
  struct lws_vhost *vhost;
  const struct lws_protocols *protocol;

  ws_ascii_session *pss_list; /* linked-list of live pss*/
};

}  // namespace

int ws_ascii_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in,
                      size_t len) {
  auto *pss = (ws_ascii_session *)user;
  auto *vhd =
      (struct per_vhost_data *)lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));

  switch (reason) {
    case LWS_CALLBACK_PROTOCOL_INIT: {
      lwsl_info("LWS_CALLBACK_PROTOCOL_INIT\n");

      vhd = reinterpret_cast<per_vhost_data *>(lws_protocol_vh_priv_zalloc(
          lws_get_vhost(wsi), lws_get_protocol(wsi), sizeof(struct per_vhost_data)));
      vhd->context = lws_get_context(wsi);
      vhd->protocol = lws_get_protocol(wsi);
      vhd->vhost = lws_get_vhost(wsi);
    } break;
    case LWS_CALLBACK_PROTOCOL_DESTROY:
      lwsl_info("LWS_CALLBACK_PROTOCOL_DESTROY\n");
      break;
    case LWS_CALLBACK_ESTABLISHED: {
      /* generate a block of output before travis times us out */
      lwsl_info("LWS_CALLBACK_ESTABLISHED\n");

      auto port = (port_def_t *)lws_context_user(lws_get_context(wsi));
      auto fd = lws_get_socket_fd(wsi);

      sockaddr_storage addr = {0};
      socklen_t addrlen = sizeof(addr);
      auto result = getpeername(fd, reinterpret_cast<sockaddr *>(&addr), &addrlen);
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

          struct evutil_addrinfo *res = nullptr;
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

      auto ip = new_user(port, fd, reinterpret_cast<sockaddr *>(&addr), addrlen);

      pss->user = ip;
      pss->buffer = evbuffer_new();

      ip->iflags |= HANDSHAKE_COMPLETE;
      ip->lws = wsi;

      auto base = evconnlistener_get_base(port->ev_conn);
      event_base_once(
          base, -1, EV_TIMEOUT,
          [](evutil_socket_t fd, short what, void *arg) {
            auto user = reinterpret_cast<interactive_t *>(arg);
            on_user_logon(user);
          },
          (void *)ip, nullptr);
      break;
    }
    case LWS_CALLBACK_CLOSED: {
      lwsl_info("LWS_CALLBACK_CLOSED: wsi %p\n", wsi);

      auto ip = pss->user;
      if (!ip) {
        return -1;
      }

      remove_interactive(ip->ob, 0);
      pss->user = nullptr;

      evbuffer_free(pss->buffer);
      pss->buffer = nullptr;

      break;
    }
    case LWS_CALLBACK_SERVER_WRITEABLE: {
      lwsl_info("LWS_CALLBACK_SERVER_WRITEABLE\n");

      auto total = evbuffer_get_length(pss->buffer);

      static unsigned char buf[LWS_PRE + MAX_TEXT];
      auto numbytes = evbuffer_copyout(pss->buffer, &buf[LWS_PRE], sizeof(buf) - LWS_PRE);
      if (numbytes > 0) {
        numbytes = u8_truncate(&buf[LWS_PRE], numbytes);
#ifdef DEBUG
        if (!u8_validate(&buf[LWS_PRE], numbytes)) {
          char buf1[MAX_TEXT + 1];
          strncpy(buf1, reinterpret_cast<const char *>(&buf[LWS_PRE]), numbytes);
          debug_message("Illegal UTF8 Websocket output string: %s.", buf1);
        }
#endif
        // TODO: we could use LWS_WRITE_TEXT , however it is much safer to use binary mode, its
        // better to let client deal with incorrect encoding.
        auto m = lws_write(wsi, buf + LWS_PRE, numbytes, LWS_WRITE_BINARY);
        if (m < 0) {
          lwsl_warn("ERROR %d writing to ws socket.\n", m);
          return -1;
        }
        evbuffer_drain(pss->buffer, m);
        total -= m;
        // May have more text to write.
        if (total > 0) {
          lws_callback_on_writable(wsi);
        }
      }
      break;
    }
    case LWS_CALLBACK_RECEIVE: {
      lwsl_info(
          "LWS_CALLBACK_RECEIVE: %4d (rpp %5d, first %d, "
          "last %d, bin %d, len %zd)\n",
          (int)len, (int)lws_remaining_packet_payload(wsi), lws_is_first_fragment(wsi),
          lws_is_final_fragment(wsi), lws_frame_is_binary(wsi), len);

      if (len <= 0) {
        break;
      }
      // don't accept binary frame, we want client to always send valid utf8.
      // lws handles the utf8 check for us.
      if (lws_frame_is_binary(wsi)) {
        return -1;
      }
      auto ip = pss->user;
      if (!ip) {  // we are already disconnected
        return -1;
      }
      on_user_websocket_received(ip, (const char *)in, len);
      break;
    }
    default:
      lwsl_info("Unknown callback: %d, \n", reason);
      break;
  }

  return 0;
}

void ws_ascii_send(struct lws *wsi, const char *data, size_t len) {
  DEBUG_CHECK(lws_get_protocol(wsi)->id != PROTOCOL_WS_ASCII, "wrong protocol!");
  auto pss = reinterpret_cast<ws_ascii_session *>(lws_wsi_user(wsi));
  DEBUG_CHECK(pss == nullptr, "no session data!");

  evbuffer_add(pss->buffer, data, len);
  lws_callback_on_writable(wsi);
}
