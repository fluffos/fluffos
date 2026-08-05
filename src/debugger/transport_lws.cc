// Debugger transport: a standalone libwebsockets context on a PRIVATE
// libevent base.
//
// Unlike the player websocket ports (src/net/websocket.cc), this context is
// deliberately NOT bound to the driver's main event base.  It is serviced by
// explicit passes over its own base from three places (see debug_server.cc):
//   1. a recurring timer on the main base while the VM is idle,
//   2. the instruction hook every kPollEvery instructions while LPC runs
//      (this is what makes `pause` work inside a runaway loop),
//   3. the blocking stop loop while the VM is stopped at a breakpoint.
// Because the main base is parked while stopped, no heart_beats, call_outs or
// user commands can dispatch during a pause -- stop-the-world for free.

#include "base/std.h"

#include <event2/event.h>
#include <libwebsockets.h>

#include <cstring>

#include "debugger/debugger.h"

namespace dbg {

namespace {

constexpr size_t kMaxInboundMsg = 4u << 20;  // 4 MiB: a hostile client cap

int dap_callback(struct lws* wsi, enum lws_callback_reasons reason, void* /*user*/, void* in,
                 size_t len) {
  auto& s = g_session;
  switch (reason) {
    case LWS_CALLBACK_ESTABLISHED:
      if (s.client) {
        // Single-client policy: reject the newcomer, keep the session.
        lwsl_warn("debugger: rejecting second concurrent client\n");
        return -1;
      }
      on_client_established(wsi);
      break;
    case LWS_CALLBACK_CLOSED:
      on_client_closed(wsi);
      break;
    case LWS_CALLBACK_SERVER_WRITEABLE: {
      if (wsi != s.client) {
        break;
      }
      // One complete DAP message per ws TEXT frame.  lws consumes the whole
      // payload on success (partials are buffered and flushed internally);
      // when the pipe chokes, lws's own POLLOUT path fires this callback
      // again, and we re-request below whenever data remains (same pattern
      // as ws_ascii.cc / ws_telnet.cc).
      while (!s.outq.empty() && !lws_send_pipe_choked(wsi)) {
        const std::string& msg = s.outq.front();
        std::vector<unsigned char> buf(LWS_PRE + msg.size());
        memcpy(buf.data() + LWS_PRE, msg.data(), msg.size());
        int m = lws_write(wsi, buf.data() + LWS_PRE, msg.size(), LWS_WRITE_TEXT);
        if (m < static_cast<int>(msg.size())) {
          lwsl_warn("debugger: ws write failed (%d)\n", m);
          return -1;
        }
        s.outq.pop_front();
      }
      if (!s.outq.empty()) {
        lws_callback_on_writable(wsi);
      } else if (s.pending_close) {
        // Our own queue -- NOT lws's internal truncated-send buffer -- has
        // now fully drained: safe to sever the connection. Do not rely on
        // lws's PENDING_FLUSH_STORED_SEND_BEFORE_CLOSE for this: that only
        // flushes a partial lws_write() lws already attempted, and knows
        // nothing about messages still sitting in outq waiting for their
        // first lws_write() call -- gating the close on that raced ahead of
        // this very drain loop and dropped the response entirely.
        return -1;
      }
      break;
    }
    case LWS_CALLBACK_RECEIVE: {
      if (wsi != s.client || len == 0) {
        break;
      }
      if (lws_frame_is_binary(wsi)) {
        return -1;  // DAP is JSON text
      }
      s.inbuf.append(static_cast<const char*>(in), len);
      if (s.inbuf.size() > kMaxInboundMsg) {
        lwsl_warn("debugger: inbound message too large, closing\n");
        s.inbuf.clear();
        return -1;
      }
      if (lws_is_final_fragment(wsi)) {
        std::string msg;
        msg.swap(s.inbuf);
        dispatch_message(msg);
      }
      break;
    }
    default:
      break;
  }
  return 0;
}

// Own protocols table -- never share the player ports' table.  Clients must
// request the "dap" subprotocol on upgrade.
struct lws_protocols dbg_protocols[] = {
    {"http", lws_callback_http_dummy, 0, 0, 0},
    {"dap", dap_callback, 0, 64 * 1024, 1},
    {nullptr, nullptr, 0, 0, 0} /* terminator */
};

void on_tick(evutil_socket_t /*fd*/, short /*what*/, void* /*arg*/) {
  // Purpose: guarantee transport_wait()'s EVLOOP_ONCE returns on a bounded
  // interval so the stop loop can notice state changes (client death, driver
  // shutdown) even on an otherwise silent connection.
}

}  // namespace

bool transport_init(int port, const char* address) {
  auto& s = g_session;

  s.base = event_base_new();
  if (!s.base) {
    return false;
  }

  struct lws_context_creation_info info = {};
  void* foreign_loops[1] = {s.base};

  info.port = port;
  info.iface = address;
  info.protocols = dbg_protocols;
  info.foreign_loops = foreign_loops;
  info.options = LWS_SERVER_OPTION_LIBEVENT | LWS_SERVER_OPTION_VALIDATE_UTF8;
  info.vhost_name = "debugger";
  info.pt_serv_buf_size = 64 * 1024;

  s.ctx = lws_create_context(&info);
  if (!s.ctx) {
    event_base_free(s.base);
    s.base = nullptr;
    return false;
  }

  s.tick_ev = event_new(s.base, -1, EV_PERSIST, on_tick, nullptr);
  struct timeval tv = {0, 100 * 1000};
  event_add(s.tick_ev, &tv);
  return true;
}

void transport_shutdown() {
  auto& s = g_session;
  if (s.ctx) {
    lws_context_destroy(s.ctx);
    s.ctx = nullptr;
    s.client = nullptr;
  }
  if (s.tick_ev) {
    event_free(s.tick_ev);
    s.tick_ev = nullptr;
  }
  if (s.base) {
    event_base_free(s.base);
    s.base = nullptr;
  }
}

void transport_pump() {
  auto& s = g_session;
  if (s.base) {
    event_base_loop(s.base, EVLOOP_NONBLOCK);
  }
}

void transport_wait() {
  auto& s = g_session;
  if (s.base) {
    // Blocks until the next event on the PRIVATE base; tick_ev bounds the
    // wait to ~100ms.  libevent retries EINTR (e.g. a stray SIGVTALRM)
    // internally.
    event_base_loop(s.base, EVLOOP_ONCE);
  }
}

void transport_send(const djson& msg) {
  auto& s = g_session;
  // `replace` keeps invalid UTF-8 from mudlib strings from throwing.
  s.outq.push_back(msg.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
  if (s.client) {
    lws_callback_on_writable(s.client);
  }
}

void transport_kill_client(bool flush) {
  auto& s = g_session;
  if (!s.client) {
    return;
  }
  if (flush && !s.outq.empty()) {
    // Defer the actual close to LWS_CALLBACK_SERVER_WRITEABLE, once our own
    // outq (not lws's internal buffer) has been handed off in full.
    s.pending_close = true;
    lws_callback_on_writable(s.client);
    return;
  }
  lws_set_timeout(s.client, pending_timeout::PENDING_TIMEOUT_KILLED_BY_PARENT, LWS_TO_KILL_ASYNC);
}

}  // namespace dbg
