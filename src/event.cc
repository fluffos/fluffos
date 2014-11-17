#include "base/std.h"

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/dns.h>
#include <event2/listener.h>
#include <event2/util.h>

#include "event.h"

#include "comm.h"          // for user socket
#include "console.h"       // for console
#include "socket_efuns.h"  // for lpc sockets
#include "eval.h"          // for set_eval

// FIXME: rewrite other part so this could become static.
struct event_base *g_event_base = NULL;
struct event *g_ev_tick = NULL;

static void libevent_log(int severity, const char *msg) { debug(event, "%d:%s\n", severity, msg); }

static void libevent_dns_log(int severity, const char *msg) {
  debug(dns, "%d:%s\n", severity, msg);
}

// Init a new event loop.
event_base *init_event_base() {
  event_set_log_callback(libevent_log);
  evdns_set_log_fn(libevent_dns_log);
#ifdef DEBUG
  event_enable_debug_mode();
#endif

  g_event_base = event_base_new();
  debug_message("Event backend in use: %s\n", event_base_get_method(g_event_base));
  return g_event_base;
}

static void on_main_loop_event(int fd, short what, void *arg) {
  auto event = (realtime_event *)arg;
  event->callback();
  delete event;
}

// Schedule a immediate event on main loop.
void add_realtime_event(realtime_event::callback_type callback) {
  auto event = new realtime_event(callback);
  event_base_once(g_event_base, -1, EV_TIMEOUT, on_main_loop_event, event, NULL);
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
  auto user = reinterpret_cast<interactive_t *>(arg);

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
  } catch (const char *) {
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
  auto user = reinterpret_cast<interactive_t *>(arg);

  if (user == NULL) {
    fatal("on_user_read: user == NULL, Driver BUG.");
    return;
  }

  // Read user input
  get_user_data(user);

  // TODO: currently get_user_data() will schedule command execution.
  // should probably move it here.
}

static void on_user_write(bufferevent *bev, void *arg) {
  auto user = reinterpret_cast<interactive_t *>(arg);
  if (user == NULL) {
    fatal("on_user_write: user == NULL, Driver BUG.");
    return;
  }
  // nothing to do.
}

static void on_user_events(bufferevent *bev, short events, void *arg) {
  auto user = reinterpret_cast<interactive_t *>(arg);

  if (user == NULL) {
    fatal("on_user_events: user == NULL, Driver BUG.");
    return;
  }

  if (events & (BEV_EVENT_ERROR | BEV_EVENT_EOF)) {
    user->iflags |= NET_DEAD;
    remove_interactive(user->ob, 0);
  } else {
    debug(event, "on_user_events: ignored unknown events: %d\n", events);
  }
}

void new_user_event_listener(interactive_t *user) {
  auto bev = bufferevent_socket_new(g_event_base, user->fd,
                                    BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
  bufferevent_setcb(bev, on_user_read, on_user_write, on_user_events, user);
  bufferevent_enable(bev, EV_READ | EV_WRITE);

  bufferevent_set_timeouts(bev, NULL, NULL);

  user->ev_buffer = bev;
  user->ev_command = event_new(g_event_base, -1, EV_TIMEOUT | EV_PERSIST, on_user_command, user);
}

static void on_external_port_event(evconnlistener *listener, evutil_socket_t fd, sockaddr *sa,
                                   int socklen, void *arg) {
  debug(event, "on_external_port_event: fd %d, addr: %s\n", fd, sockaddr_to_string(sa, socklen));
  auto *port = reinterpret_cast<port_def_t *>(arg);
  new_user_handler(fd, sa, socklen, port);
}

void new_external_port_event_listener(port_def_t *port, sockaddr *sa, socklen_t socklen) {
  port->ev_conn = evconnlistener_new_bind(
      g_event_base, on_external_port_event, port,
      LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE | LEV_OPT_CLOSE_ON_EXEC, 1024, sa, socklen);
  DEBUG_CHECK1(port->ev_conn == NULL, "listening failed: %s !",
               evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
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
void new_lpc_socket_event_listener(int idx, lpc_socket_t *sock, evutil_socket_t real_fd) {
  auto data = new lpc_socket_event_data;
  data->idx = idx;
  sock->ev_read = event_new(g_event_base, real_fd, EV_READ | EV_PERSIST, on_lpc_sock_read, data);
  sock->ev_write = event_new(g_event_base, real_fd, EV_WRITE, on_lpc_sock_write, data);
  sock->ev_data = data;
}
