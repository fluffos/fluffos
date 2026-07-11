#include "base/std.h"

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/util.h>
#include <event2/listener.h>

#include <libwebsockets.h>
#include <cstdlib>

#include "net/ws_telnet.h"
#include "comm.h"
#include "net/transport_native.h"
#include "interactive.h"
#include "net/telnet.h"
#include "net/sys_telnet.h"  // IAC / SB / SE / WILL / DONT

namespace {

/* one of these is created for each vhost our protocol is used with */
struct per_vhost_data {
  struct lws_context* context;
  struct lws_vhost* vhost;
  const struct lws_protocols* protocol;

  ws_telnet_session* pss_list; /* linked-list of live pss*/
};

// Find how many bytes of an outgoing telnet window can be sent without
// splitting an IAC sequence (command, option negotiation, or subnegotiation)
// across websocket messages. A subnegotiation larger than a full window has
// to be split to make progress; in that case (window_full) cut inside it, on
// an IAC-pair boundary.
size_t telnet_truncate(const unsigned char* data, size_t len, bool window_full) {
  size_t safe = 0;
  size_t i = 0;
  while (i < len) {
    if (data[i] != IAC) {
      safe = ++i;
      continue;
    }
    if (i + 1 >= len) break;  // trailing lone IAC
    auto cmd = data[i + 1];
    if (cmd == SB) {
      // subnegotiation runs until IAC SE; IAC IAC inside is escaped data
      size_t j = i + 2;
      size_t end = 0;
      while (j < len) {
        if (data[j] != IAC) {
          j++;
          continue;
        }
        if (j + 1 >= len) break;  // trailing lone IAC inside the subnegotiation
        if (data[j + 1] == SE) {
          end = j + 2;
          break;
        }
        j += 2;  // IAC IAC escape (or any other IAC pair)
      }
      if (!end) {
        if (safe == 0 && window_full) {
          safe = j;
        }
        break;
      }
      safe = i = end;
    } else if (cmd >= WILL && cmd <= DONT) {
      if (i + 2 >= len) break;  // option byte not in the window yet
      safe = i += 3;
    } else {
      safe = i += 2;  // two-byte command, including the IAC IAC escape
    }
  }
  return safe;
}

}  // namespace

int ws_telnet_callback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in,
                       size_t len) {
  auto* pss = (ws_telnet_session*)user;
  auto* vhd =
      (struct per_vhost_data*)lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));

  switch (reason) {
    case LWS_CALLBACK_PROTOCOL_INIT:
      lwsl_info("LWS_CALLBACK_PROTOCOL_INIT\n");
      // freed automatically when context is destroyed.
      vhd = reinterpret_cast<per_vhost_data*>(lws_protocol_vh_priv_zalloc(
          lws_get_vhost(wsi), lws_get_protocol(wsi), sizeof(struct per_vhost_data)));
      vhd->context = lws_get_context(wsi);
      vhd->protocol = lws_get_protocol(wsi);
      vhd->vhost = lws_get_vhost(wsi);
      break;
    case LWS_CALLBACK_PROTOCOL_DESTROY:
      lwsl_info("LWS_CALLBACK_PROTOCOL_DESTROY\n");
      break;
    case LWS_CALLBACK_ESTABLISHED: {
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

      // handshake complete so lets setup telnet layer
      ip->telnet = net_telnet_init(ip);
      send_initial_telnet_negotiations(ip);

      auto base = evconnlistener_get_base(port->ev_conn);
      event_base_once(
          base, -1, EV_TIMEOUT,
          [](evutil_socket_t fd, short what, void* arg) {
            auto user = reinterpret_cast<interactive_t*>(arg);
            on_user_logon(user);
          },
          (void*)ip, nullptr);
      break;
    }
    case LWS_CALLBACK_CLOSED: {
      lwsl_info("LWS_CALLBACK_CLOSED: wsi %p\n", wsi);

      auto* ip = pss->user;
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

      static unsigned char buf[LWS_PRE + 2048];
      auto numbytes = evbuffer_copyout(pss->buffer, &buf[LWS_PRE], sizeof(buf) - LWS_PRE);
      if (numbytes > 0) {
        // Hold back a trailing incomplete telnet IAC sequence so a command,
        // negotiation, or subnegotiation is never split across ws messages.
        // evbuffer_copyout() does not drain, so the held-back bytes stay at
        // the front of pss->buffer and go out with the next write. Once MCCP
        // (COMPRESS2) is active the stream is deflated binary, not telnet
        // framing: it must go out verbatim, without IAC scanning or hold-back.
        if (pss->user && !(pss->user->iflags & USING_COMPRESS)) {
          auto new_numbytes = static_cast<ev_ssize_t>(telnet_truncate(
              &buf[LWS_PRE], numbytes, numbytes == static_cast<ev_ssize_t>(sizeof(buf) - LWS_PRE)));
          if (new_numbytes == 0) {
            // Only an incomplete sequence is buffered; wait for the rest of it
            // instead of re-arming the writeable callback in a busy loop.
            break;
          }
          numbytes = new_numbytes;
        }
        auto m = lws_write(wsi, buf + LWS_PRE, numbytes, LWS_WRITE_BINARY);
        // Per lws docs, a return less than the requested length means the
        // connection has failed. On success lws consumed the entire payload
        // (truncated sends are buffered and flushed internally), and the
        // return can exceed numbytes under encapsulation (TLS), so it must
        // NOT be used as a drain count: doing so silently drops the start
        // of the next chunk on wss connections.
        if (m < static_cast<int>(numbytes)) {
          lwsl_warn("ERROR %d writing to ws socket.\n", m);
          return -1;
        }
        evbuffer_drain(pss->buffer, numbytes);
        total -= numbytes;
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
      auto ip = pss->user;
      if (!ip) {  // we are already disconnected
        return -1;
      }
      comm_telnet_received(ip, (const char*)in, len);
      break;
    }
    default:
      lwsl_info("Unknown callback: %d, \n", reason);
      break;
  }

  return 0;
}

void ws_telnet_send(struct lws* wsi, const char* data, size_t len) {
  DEBUG_CHECK(lws_get_protocol(wsi)->id != PROTOCOL_WS_TELNET, "wrong protocol!");
  auto pss = reinterpret_cast<ws_telnet_session*>(lws_wsi_user(wsi));
  DEBUG_CHECK(pss == nullptr, "no session data!");

  evbuffer_add(pss->buffer, data, len);
  lws_callback_on_writable(wsi);
}
