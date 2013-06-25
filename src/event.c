#include "std.h"

#include <event2/event.h>
#include <event2/dns.h>

#include "comm.h" // for user socket
#include "console.h" // for console
#include "socket_efuns.h"  // for lpc sockets

struct event_base *g_event_base = NULL;

// Init a new event loop.
event_base* init_event_base() {
#ifdef DEBUG
  event_enable_debug_mode();
#endif
  g_event_base = event_base_new();
  return g_event_base;
}

static void exit_after_one_second(evutil_socket_t fd, short events, void *arg)
{
  event_base_loopbreak((struct event_base *)arg);
}

int run_for_at_most_one_second(struct event_base *base)
{
  int r;
  struct timeval one_second = {1, 0};
  static struct event *ev = evtimer_new(base, exit_after_one_second, base);

  event_add(ev, &one_second);
  r = event_base_loop(base, EVLOOP_ONCE);

  return r;
}

static void on_external_port_event(evutil_socket_t fd, short what, void *arg)
{
  debug_message("Got an event on listen socket %d:%s%s%s%s",
                (int) fd,
                (what & EV_TIMEOUT) ? " timeout" : "",
                (what & EV_READ)    ? " read" : "",
                (what & EV_WRITE)   ? " write" : "",
                (what & EV_SIGNAL)  ? " signal" : "");

  // FIXME: remove the need to pass the argument.
  new_user_handler((port_def_t *)arg);
}

struct event* new_external_port_event(port_def_t *port) {
  struct event *ev;
  ev = event_new(g_event_base, port->fd,
      EV_READ| EV_PERSIST, on_external_port_event, port);
  event_add(ev, NULL);
  return ev;
}

// FIXME: rethink if this is necessary.
// Watch write event for users, this will make sure user message
// will get flushed at least once each round, if connection allows.
void add_user_write_event()
{
  for (int i = 0; i < max_users; i++) {
    if (all_users[i] && all_users[i]->message_length != 0)
      event_add(all_users[i]->ev_write, NULL);
  }
}

#if defined(PACKAGE_SOCKETS) || defined(PACKAGE_EXTERNAL)
void add_lpc_sock_event()
{
  for (int i = 0; i < max_lpc_socks; i++) {
    if (lpc_socks[i].state != STATE_CLOSED) {
      if (lpc_socks[i].state != STATE_FLUSHING &&
          (lpc_socks[i].flags & S_WACCEPT) == 0) {
        event_add(lpc_socks[i].ev_read, NULL);
      }
      if (lpc_socks[i].flags & S_BLOCKED) {
        event_add(lpc_socks[i].ev_write, NULL);
      }
    }
  }
}
#endif

static void on_console_event(evutil_socket_t fd, short what, void *arg)
{
  debug_message("Got an event on stdin socket %d:%s%s%s%s \n",
                (int) fd,
                (what & EV_TIMEOUT) ? " timeout" : "",
                (what & EV_READ)    ? " read" : "",
                (what & EV_WRITE)   ? " write" : "",
                (what & EV_SIGNAL)  ? " signal" : "");

  if (has_console <= 0) {
    event_del((struct event *)arg);
    return;
  }
  on_console_input();
}

void init_console() {
  if (has_console > 0) {
    debug_message("Opening console... \n");
    struct event* ev_console = NULL;
    ev_console = event_new(
        g_event_base, STDIN_FILENO,
        EV_READ | EV_PERSIST, on_console_event, ev_console);
    event_add(ev_console, NULL);
  }
}
