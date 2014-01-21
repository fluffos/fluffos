#include "std.h"

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/dns.h>
#include <event2/listener.h>
#include <event2/thread.h>
#include <event2/util.h>

#include "event.h"

#include "comm.h"          // for user socket
#include "console.h"       // for console
#include "socket_efuns.h"  // for lpc sockets
#include "eval.h"          // for set_eval
#include "util/threadpool-incl.h"

// FIXME: rewrite other part so this could become static.
struct event_base *g_event_base = NULL;
struct event *g_ev_tick = NULL;

static util::ThreadPool *g_threadpool_network_ = NULL;

static void libevent_log(int severity, const char *msg) {
  debug(event, "%d:%s\n", severity, msg);
}

static void libevent_dns_log(int severity, const char *msg) {
  debug(dns, "%d:%s\n", severity, msg);
}

// Init a new event loop.
event_base *init_event_base() {
  event_set_log_callback(libevent_log);
  evdns_set_log_fn(libevent_dns_log);
  evthread_use_pthreads();

#ifdef DEBUG
  event_enable_debug_mode();
  evthread_enable_lock_debuging();
#endif

  g_event_base = event_base_new();
  debug_message("Event backend in use: %s\n",
                event_base_get_method(g_event_base));
  return g_event_base;
}

// Init net threadpool
void init_network_threadpool() {
  g_threadpool_network_ = new util::ThreadPool(4);
}

void shutdown_network_threadpool() { delete g_threadpool_network_; }

static void on_main_loop_event(int fd, short what, void *arg) {
  auto event = (realtime_event *)arg;
  event->callback();
  delete event;
}

// Schedule a realtime event on main loop, safe to call from any thread.
void add_realtime_event(realtime_event::callback_type callback) {
  auto event = new realtime_event(callback);
  event_base_once(g_event_base, -1, EV_TIMEOUT, on_main_loop_event, event,
                  NULL);
}

extern void virtual_time_tick();
void on_virtual_time_tick(int fd, short what, void *arg) {
  struct timeval one_second = {1, 0};
  virtual_time_tick();
  event_add(g_ev_tick, &one_second);
}

int run_event_loop(struct event_base *base) {
  struct timeval one_second = {1, 0};

  // Schedule a repeating tick for advancing virtual time.
  g_ev_tick = evtimer_new(base, on_virtual_time_tick, NULL);

  event_add(g_ev_tick, &one_second);
  return event_base_loop(base, 0);
}

static void on_user_command(evutil_socket_t, short, void *);

static void maybe_schedule_user_command(interactive_t *user) {
  // If user has a complete command, schedule a command execution.
  if (user->iflags & CMD_IN_BUF) {
    event_active(user->ev_command, EV_TIMEOUT, 0);
  }
}

static void on_user_command(evutil_socket_t fd, short what, void *arg) {
  debug(event, "User has an full command ready: %d:%s%s%s%s \n", (int)fd,
        (what & EV_TIMEOUT) ? " timeout" : "", (what & EV_READ) ? " read" : "",
        (what & EV_WRITE) ? " write" : "", (what & EV_SIGNAL) ? " signal" : "");

  auto data = (user_event_data *)arg;
  auto user = all_users[data->idx];

  if (user == NULL) {
    fatal("on_user_command: user == NULL, Driver BUG.");
    return;
  }

  // FIXME: this function currently calls into mudlib and will throw errors
  // This catch block should be moved one level down.
  error_context_t econ;
  if (!save_context(&econ)) {
    fatal("BUG: on_user_comamnd can not save context!");
  }
  set_eval(max_cost);
  try {
    process_user_command(user);
  }
  catch (const char *) {
    restore_context(&econ);
  }
  pop_context(&econ);

  /* Has to be cleared if we jumped out of process_user_command() */
  current_interactive = 0;

  // if user still have pending command, continue to schedule it.
  //
  // NOTE: It is important to only execute one command here, then schedule next
  // command at the tail, This ensure users have a fair chance that no one can
  // keep running commands.
  //
  // currently command scehduling is done inside process_user_command().
  //
  // maybe_schedule_user_command(user);
}

static void on_user_read(bufferevent *bev, void *arg) {
  auto data = (user_event_data *)arg;
  auto user = all_users[data->idx];

  if (user == NULL) {
    fatal("on_user_read: user == NULL, Driver BUG.");
    return;
  }

  debug(event, "on_user_read, idx: %d.\n", data->idx);

  // Read user input
  get_user_data(user);

  // TODO: currently get_user_data() will schedule command execution.
  // should probably move it here.
}

static void on_user_write(bufferevent *bev, void *arg) {
  auto data = (user_event_data *)arg;
  auto user = all_users[data->idx];

  if (user == NULL) {
    fatal("on_user_write: user == NULL, Driver BUG.");
    return;
  }

  debug(event, "on_user_write, idx: %d, leftover: %lu.\n", data->idx,
        evbuffer_get_length(bufferevent_get_output(user->ev_buffer)));

  // nothing to do.
}

static void on_user_events(bufferevent *bev, short events, void *arg) {
  debug(event, "on_user_events: %d\n", events);

  auto data = (user_event_data *)arg;
  auto user = all_users[data->idx];

  if (user == NULL) {
    fatal("on_user_events: user == NULL, Driver BUG.");
    return;
  }

  if ((events & BEV_EVENT_EOF) || (events & BEV_EVENT_TIMEOUT)) {
    user->iflags |= NET_DEAD;
    remove_interactive(user->ob, 0);
  }
}

void new_user_event_listener(interactive_t *user, int idx) {
  user->ev_data = new user_event_data;
  user->ev_data->idx = idx;

  auto bev =
      bufferevent_socket_new(g_event_base, user->fd, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(bev, on_user_read, on_user_write, on_user_events,
                    user->ev_data);
  bufferevent_enable(bev, EV_READ | EV_WRITE);

  const timeval timeout_write = {10, 0};
  bufferevent_set_timeouts(bev, NULL, &timeout_write);

  user->ev_buffer = bev;
  user->ev_command = event_new(g_event_base, -1, EV_TIMEOUT | EV_PERSIST,
                               on_user_command, user->ev_data);
}

static void on_external_port_event(evconnlistener *listener, evutil_socket_t fd,
                                   sockaddr *sa, int socklen, void *arg) {
  debug(event, "on_external_port_event: fd %d, addr: %s\n", fd,
        sockaddr_to_string(sa, socklen));

  port_def_t *port = reinterpret_cast<port_def_t *>(arg);

  g_threadpool_network_->enqueue([=]() { async_on_accept(fd, port); });
}

void new_external_port_event_listener(port_def_t *port, sockaddr *sa,
                                      socklen_t socklen) {
  port->ev_conn = evconnlistener_new_bind(
      g_event_base, on_external_port_event, port,
      LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE | LEV_OPT_CLOSE_ON_EXEC, 1024,
      sa, socklen);
  DEBUG_CHECK(port->ev_conn == NULL, "listening failed!");
}

void on_lpc_sock_read(evutil_socket_t fd, short what, void *arg) {
  debug(event, "Got an event on socket %d:%s%s%s%s \n", (int)fd,
        (what & EV_TIMEOUT) ? " timeout" : "", (what & EV_READ) ? " read" : "",
        (what & EV_WRITE) ? " write" : "", (what & EV_SIGNAL) ? " signal" : "");

  auto data = (lpc_socket_event_data *)arg;
  socket_read_select_handler(data->idx);
}
void on_lpc_sock_write(evutil_socket_t fd, short what, void *arg) {
  debug(event, "Got an event on socket %d:%s%s%s%s \n", (int)fd,
        (what & EV_TIMEOUT) ? " timeout" : "", (what & EV_READ) ? " read" : "",
        (what & EV_WRITE) ? " write" : "", (what & EV_SIGNAL) ? " signal" : "");

  auto data = (lpc_socket_event_data *)arg;
  socket_write_select_handler(data->idx);
}

// Initialize LPC socket data structure and register events
void new_lpc_socket_event_listener(int idx, evutil_socket_t real_fd) {
  auto data = new lpc_socket_event_data;
  data->idx = idx;
  lpc_socks[idx].ev_read = event_new(
      g_event_base, real_fd, EV_READ | EV_PERSIST, on_lpc_sock_read, data);
  lpc_socks[idx].ev_write =
      event_new(g_event_base, real_fd, EV_WRITE, on_lpc_sock_write, data);
  lpc_socks[idx].ev_data = data;
}

#ifdef HAS_CONSOLE
static void on_console_event(evutil_socket_t fd, short what, void *arg) {
  debug(event, "Got an event on stdin socket %d:%s%s%s%s \n", (int)fd,
        (what & EV_TIMEOUT) ? " timeout" : "", (what & EV_READ) ? " read" : "",
        (what & EV_WRITE) ? " write" : "", (what & EV_SIGNAL) ? " signal" : "");

  if (has_console <= 0) {
    event_del((struct event *)arg);
    return;
  }
  on_console_input();
}

void init_console(struct event_base *base) {
  if (has_console > 0) {
    debug_message("Opening console... \n");
    struct event *ev_console = NULL;
    ev_console = event_new(base, STDIN_FILENO, EV_READ | EV_PERSIST,
                           on_console_event, ev_console);
    event_add(ev_console, NULL);
  }
}
#endif
