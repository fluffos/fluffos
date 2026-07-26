#include "base/std.h"

#include <event2/buffer.h>

#include <libwebsockets.h>

#include "net/ws_ascii.h"
#include "comm.h"
#include "interactive.h"

namespace {

// Hold back a trailing incomplete UTF-8 sequence so a codepoint is never split
// across ws messages.
size_t ascii_truncate(ws_session* /*pss*/, unsigned char* data, size_t len, bool /*window_full*/) {
  auto numbytes = u8_truncate(data, len);
#ifdef DEBUG
  if (numbytes > 0 && !u8_validate(data, numbytes)) {
    char buf1[2048 + 1] = {};
    strncpy(buf1, reinterpret_cast<const char*>(data), numbytes);
    debug_message("Illegal UTF8 Websocket output string: %s.", buf1);
  }
#endif
  return numbytes;
}

}  // namespace

int ws_ascii_callback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in,
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
      return ws_handle_established(wsi, pss, nullptr);
    case LWS_CALLBACK_CLOSED:
      lwsl_info("LWS_CALLBACK_CLOSED: wsi %p\n", wsi);
      ws_handle_closed(pss);
      break;
    case LWS_CALLBACK_SERVER_WRITEABLE:
      return ws_handle_writeable(wsi, pss, ascii_truncate);
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
      comm_text_received(ip, (const char*)in, len);
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
void ws_ascii_send(struct lws* wsi, const char* data, size_t len) {
  ws_queue_send(wsi, PROTOCOL_WS_ASCII, data, len);
}
