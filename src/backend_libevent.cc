/*
 * backend_libevent.cc -- the native event loop.
 *
 * Owns the libevent base: the repeating gametick timer, wall-time events
 * (event_base_once) and the blocking event_base_loop() the native driver
 * lives in. The event-queue logic itself is in backend.cc; the WASM build
 * compiles wasm/backend_wasm.cc instead of this file.
 */

#include "base/std.h"

#include "backend.h"

#include <event2/dns.h>     // for evdns_set_log_fn
#include <event2/event.h>   // for event_add, etc
#include <event2/thread.h>  // for thread support
#include <chrono>

#include "vm/vm.h"

// FIXME: rewrite other part so this could become static.
struct event_base* g_event_base = nullptr;

namespace {
void libevent_log(int severity, const char* msg) {
  if (severity == EVENT_LOG_ERR) {
    debug(all, "libevent:%d:%s\n", severity, msg);
  } else {
    debug(event, "libevent:%d:%s\n", severity, msg);
  }
}
void libevent_dns_log(int severity, const char* msg) {
  if (severity == EVENT_LOG_ERR) {
    debug(all, "libevent dns:%d:%s\n", severity, msg);
  } else {
    debug(dns, "libevent dns:%d:%s\n", severity, msg);
  }
}
}  // namespace

// Initialize backend
event_base* init_backend() {
  event_set_log_callback(libevent_log);
  evdns_set_log_fn(libevent_dns_log);
#ifdef DEBUG
  event_enable_debug_logging(EVENT_DBG_ALL);
  event_enable_debug_mode();
#endif
#ifdef _WIN32
  evthread_use_windows_threads();
#else
  evthread_use_pthreads();
#endif
  g_event_base = event_base_new();
  debug_message("Event backend in use: %s\n", event_base_get_method(g_event_base));
  return g_event_base;
}

namespace {
// TODO: remove the need for this
// Global variable for game ticket event handle.
struct event* g_ev_tick = nullptr;

inline struct timeval gametick_timeval() {
  static struct timeval const val{
      CONFIG_INT(__RC_GAMETICK_MSEC__) / 1000,         // secs
      CONFIG_INT(__RC_GAMETICK_MSEC__) % 1000 * 1000,  // usecs
  };
  return val;
}

void on_game_tick(evutil_socket_t /*fd*/, short /*what*/, void* arg) {
  backend_run_one_gametick();

  auto* ev = *(reinterpret_cast<struct event**>(arg));
  auto t = gametick_timeval();
  event_add(ev, &t);
}

void on_walltime_event(evutil_socket_t /*fd*/, short /*what*/, void* arg) {
  backend_dispose_tick_event(reinterpret_cast<TickEvent*>(arg));
}
}  // namespace

// Schedule a immediate event on main loop.
TickEvent* add_walltime_event(std::chrono::milliseconds delay_msecs,
                              TickEvent::callback_type callback) {
  auto* event = new TickEvent(callback);
  struct timeval val{
      (int)(delay_msecs.count() / 1000),
      (int)(delay_msecs.count() % 1000 * 1000),
  };
  struct timeval* delay_ptr = nullptr;
  if (delay_msecs.count() != 0) {
    delay_ptr = &val;
  }
  event_base_once(g_event_base, -1, EV_TIMEOUT, on_walltime_event, event, delay_ptr);
  return event;
}

int clear_walltime_events() {
  // Wall-time events are one-shot event_base_once() entries owned by
  // libevent; there is no queue of ours to drain.
  return 0;
}

/*
 * This is the backend. We will stay here for ever (almost).
 */
void backend(struct event_base* base) {
  clear_state();
  g_current_gametick = 0;

  backend_register_tick_events();

  // NOTE: we don't use EV_PERSITENT here because that use fix-rate scheduling.
  //
  // Schedule a repeating tick for advancing virtual time.
  // Gametick provides a fixed-delay scheduling with a guaranteed minimum delay for
  // heartbeats, callouts, and various cleaning function.
  g_ev_tick = evtimer_new(base, on_game_tick, &g_ev_tick);

  auto t = gametick_timeval();
  event_add(g_ev_tick, &t);

  try {
    event_base_loop(base, 0);
  } catch (...) {  // catch everything
    fatal("BUG: jumped out of event loop!");
  }
  // We've reached here meaning we are in shutdown sequence.
  shutdownMudOS(-1);
} /* backend() */
