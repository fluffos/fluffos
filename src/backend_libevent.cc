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
#include <set>

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

/* Backing store for add_loop_yield_event(). libevent's active_later queue
 * needs a caller-owned struct event (event_base_once() allocates its own and
 * offers no way to reach that queue), so we pair one with the TickEvent and
 * free both when it fires. Tracked in a list so clear_walltime_events() can
 * reclaim any that never did. */
struct LoopYieldEvent {
  struct event* ev;
  TickEvent* tick;
};
std::set<LoopYieldEvent*> g_loop_yield_events;

void on_loop_yield_event(evutil_socket_t /*fd*/, short /*what*/, void* arg) {
  auto* self = reinterpret_cast<LoopYieldEvent*>(arg);
  auto* tick = self->tick;
  g_loop_yield_events.erase(self);
  /* libevent has already taken it off the active queue before invoking us,
   * and it is neither persistent nor added, so freeing it here is safe. */
  event_free(self->ev);
  delete self;
  backend_dispose_tick_event(tick);
}
}  // namespace

/* Declared in libevent's event-internal.h, which is not installed and drags
 * in the whole private base layout, so it is spelled out here instead.
 *
 * This is the one internal libevent symbol the driver uses. It is the only
 * way to reach the active_later queue -- the public API has no equivalent of
 * libuv's check phase / setImmediate -- and libevent's own bufferevent code
 * uses the same queue for the same reason (event_deferred_cb_schedule_ runs
 * MAX_DEFERREDS_QUEUED=32 callbacks eagerly, then defers the rest). The
 * symbol is non-static, and nothing inside libevent references it, so it is
 * only retained in the link because we do. The queue is a 2.1 feature (it
 * does not appear in ChangeLog-2.0), which is why libevent is vendored
 * rather than probed for: a missing symbol here is a link error, not a
 * silent behaviour change.
 * RE-CHECK THIS ON EVERY LIBEVENT UPGRADE (AGENTS.md section 14). */
extern "C" void event_active_later_(struct event* ev, int res);

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

TickEvent* add_loop_yield_event(TickEvent::callback_type callback) {
  auto* tick = new TickEvent(callback);
  auto* self = new LoopYieldEvent{};
  self->tick = tick;
  /* Created but deliberately never event_add()ed: it carries no timeout and
   * no fd, it exists only as a handle the active_later queue can hold. */
  self->ev = event_new(g_event_base, -1, 0, on_loop_yield_event, self);
  if (self->ev == nullptr) {
    fatal("add_loop_yield_event: out of memory");
  }
  g_loop_yield_events.insert(self);
  /* Promotion to the active queue happens at the TOP of the next loop
   * iteration, before dispatch(), and the pending entry keeps
   * N_ACTIVE_CALLBACKS non-zero so that iteration polls with a zero timeout
   * instead of blocking. Net effect: poll, then run -- and because promotion
   * only ever happens in the loop body, a callback that re-posts itself this
   * way provably cannot run twice in one event_process_active() pass. */
  event_active_later_(self->ev, EV_TIMEOUT);
  return tick;
}

int clear_walltime_events() {
  // Wall-time events are one-shot event_base_once() entries owned by
  // libevent; there is no queue of ours to drain. Loop-yield events are
  // ours, though.
  int n = 0;
  for (auto* self : g_loop_yield_events) {
    event_free(self->ev);
    delete self->tick;
    delete self;
    n++;
  }
  g_loop_yield_events.clear();
  return n;
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
