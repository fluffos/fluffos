#include "base/std.h"

#include <event2/buffer.h>

#include <libwebsockets.h>

#include "net/ws_telnet.h"
#include "comm.h"
#include "interactive.h"
#include "net/telnet.h"
#include "net/sys_telnet.h"  // IAC / SB / SE / WILL / DONT

namespace {

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

// Hold back a trailing incomplete telnet IAC sequence so a command,
// negotiation, or subnegotiation is never split across ws messages. Once MCCP
// (COMPRESS2) is active the stream is deflated binary, not telnet framing: it
// must go out verbatim, without IAC scanning or hold-back.
size_t telnet_ws_truncate(ws_session* pss, unsigned char* data, size_t len, bool window_full) {
  if (pss->user && (pss->user->iflags & USING_COMPRESS)) {
    return len;
  }
  return telnet_truncate(data, len, window_full);
}

// Bring up the telnet layer on a freshly established websocket user.
void telnet_setup(struct interactive_t* ip) {
  ip->telnet = net_telnet_init(ip);
  send_initial_telnet_negotiations(ip);
}

}  // namespace

int ws_telnet_callback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in,
                       size_t len) {
  auto* pss = (ws_session*)user;

  switch (reason) {
    case LWS_CALLBACK_PROTOCOL_INIT:
      ws_handle_protocol_init(wsi);
      break;
    case LWS_CALLBACK_PROTOCOL_DESTROY:
      lwsl_info("LWS_CALLBACK_PROTOCOL_DESTROY\n");
      break;
    case LWS_CALLBACK_ESTABLISHED:
      return ws_handle_established(wsi, pss, telnet_setup);
    case LWS_CALLBACK_CLOSED:
      lwsl_info("LWS_CALLBACK_CLOSED: wsi %p\n", wsi);
      ws_handle_closed(pss);
      break;
    case LWS_CALLBACK_SERVER_WRITEABLE:
      return ws_handle_writeable(wsi, pss, telnet_ws_truncate);
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

// Queue the bytes and request a writeable callback; actual sending only
// happens in LWS_CALLBACK_SERVER_WRITEABLE.
void ws_telnet_send(struct lws* wsi, const char* data, size_t len) {
  ws_queue_send(wsi, PROTOCOL_WS_TELNET, data, len);
}
