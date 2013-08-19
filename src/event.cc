#include "std.h"

#include <event2/event.h>
#include <event2/dns.h>
#include <event2/util.h>

#include "event.h"

#include "comm.h" // for user socket
#include "console.h" // for console
#include "socket_efuns.h"  // for lpc sockets

//FIXME: rewrite other part so this could become static.
struct event_base *g_event_base = NULL;

static void libevent_log(int severity, const char *msg)
{
  debug(event, "%d:%s\n", severity, msg);
}

static void libevent_dns_log(int severity, const char *msg)
{
  debug(dns, "%d:%s\n", severity, msg);
}

// Init a new event loop.
event_base *init_event_base()
{
#ifdef DEBUG
  event_enable_debug_mode();
#endif
  event_set_log_callback(libevent_log);
  evdns_set_log_fn(libevent_dns_log);

  g_event_base = event_base_new();
  debug_message("Event backend in use: %s\n", event_base_get_method(g_event_base));
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
  static struct event *ev = NULL;
  static int in_loop = 0;

  if (in_loop) {
    fatal("Reentrant into event loop, this means some event handler not "
          "using safe_apply, jumped out of event loop, this is driver bug, "
          "please file bug report.\n");
    return 0;
  }
  if (ev == NULL) {
    ev = evtimer_new(base, exit_after_one_second, base);
  }
  event_add(ev, &one_second);

  debug(event, "Entering event loop for at most 1 sec! \n");
  in_loop = 1;
  r = event_base_loop(base, EVLOOP_ONCE);
  in_loop = 0;

  return r;
}

static void on_user_command(evutil_socket_t, short, void *);

static void maybe_schedule_user_command(interactive_t *user)
{
  // If user has a complete command, schedule a command execution.
  if (user->iflags & CMD_IN_BUF) {
    event_active(user->ev_command, EV_TIMEOUT, 0);
  }
}

static void on_user_command(evutil_socket_t fd, short what, void *arg)
{
  debug(event, "User has an full command ready: %d:%s%s%s%s \n",
        (int) fd,
        (what & EV_TIMEOUT) ? " timeout" : "",
        (what & EV_READ)    ? " read" : "",
        (what & EV_WRITE)   ? " write" : "",
        (what & EV_SIGNAL)  ? " signal" : "");

  auto data = (user_event_data *)arg;
  auto user = all_users[data->idx];

  if (user == NULL) {
    fatal("on_user_command: user == NULL, Driver BUG.");
    return;
  }

  // FIXME: this function currently calls into mudlib and will throw errors
  // This catch block should be moved one level down.
  error_context_t econ;
  try {
    if (!save_context(&econ)) {
      fatal("BUG: on_user_comamnd can not save context!");
    }
    process_user_command(user);
  } catch (const char *)  {
    restore_context(&econ);
  }
  // if user still have pending command, continue to schedule it.
  //
  // NOTE: It is important to only execute one command here, then schedule next
  // command at the tail, This ensure users have a fair chance that no one can
  // keep running commands.
  // currently inside process_user_command().
  //
  // maybe_schedule_user_command(user);
}

static void on_user_read(evutil_socket_t fd, short what, void *arg)
{
  debug(event, "Got an event on user socket %d:%s%s%s%s \n",
        (int) fd,
        (what & EV_TIMEOUT) ? " timeout" : "",
        (what & EV_READ)    ? " read" : "",
        (what & EV_WRITE)   ? " write" : "",
        (what & EV_SIGNAL)  ? " signal" : "");

  auto data = (user_event_data *)arg;
  auto user = all_users[data->idx];

  if (user == NULL) {
    fatal("on_user_read: user == NULL, Driver BUG.");
    return;
  }

  // Read user input
  auto idx = data->idx;

  get_user_data(user);

  // TODO: currently get_user_data will schedule command execution.
  // should probabaly move here.
}

static void on_user_write(evutil_socket_t fd, short what, void *arg)
{
  debug(event, "Got an event on user socket %d:%s%s%s%s \n",
        (int) fd,
        (what & EV_TIMEOUT) ? " timeout" : "",
        (what & EV_READ)    ? " read" : "",
        (what & EV_WRITE)   ? " write" : "",
        (what & EV_SIGNAL)  ? " signal" : "");

  auto data = (user_event_data *)arg;
  auto user = all_users[data->idx];

  if (user == NULL) {
    fatal("on_user_read: user == NULL, Driver BUG.");
    return;
  }

  flush_message(user);
}

void new_user_event_listener(int idx)
{
  auto data = new user_event_data;
  data->idx = idx;

  auto user = all_users[idx];

  user->ev_read = event_new(g_event_base, user->fd, EV_READ | EV_PERSIST, on_user_read, data);
  user->ev_write = event_new(g_event_base, user->fd, EV_WRITE, on_user_write, data);
  user->ev_command = event_new(g_event_base, -1, EV_TIMEOUT | EV_PERSIST, on_user_command, data);
  user->ev_data = data;

  event_add(user->ev_read, NULL);
  event_add(user->ev_write, NULL);
  event_add(user->ev_command, NULL);
}

static void on_external_port_event(evutil_socket_t fd, short what, void *arg)
{
  debug(event, "Got an event on listen socket %d:%s%s%s%s \n",
        (int) fd,
        (what & EV_TIMEOUT) ? " timeout" : "",
        (what & EV_READ)    ? " read" : "",
        (what & EV_WRITE)   ? " write" : "",
        (what & EV_SIGNAL)  ? " signal" : "");

  // FIXME: remove the need to pass the argument.
  new_user_handler((port_def_t *)arg);
}

void new_external_port_event_listener(port_def_t *port)
{
  port->ev_read = event_new(g_event_base, port->fd,
                            EV_READ | EV_PERSIST, on_external_port_event, port);
  event_add(port->ev_read, NULL);
}

void on_lpc_sock_read(evutil_socket_t fd, short what, void *arg)
{
  debug(event, "Got an event on socket %d:%s%s%s%s \n",
        (int) fd,
        (what & EV_TIMEOUT) ? " timeout" : "",
        (what & EV_READ)    ? " read" : "",
        (what & EV_WRITE)   ? " write" : "",
        (what & EV_SIGNAL)  ? " signal" : "");

  auto data = (lpc_socket_event_data *)arg;
  socket_read_select_handler(data->idx);
}
void on_lpc_sock_write(evutil_socket_t fd, short what, void *arg)
{
  debug(event, "Got an event on socket %d:%s%s%s%s \n",
        (int) fd,
        (what & EV_TIMEOUT) ? " timeout" : "",
        (what & EV_READ)    ? " read" : "",
        (what & EV_WRITE)   ? " write" : "",
        (what & EV_SIGNAL)  ? " signal" : "");

  auto data = (lpc_socket_event_data *)arg;
  socket_write_select_handler(data->idx);
}

// Initialize LPC socket data structure and register events
void new_lpc_socket_event_listener(int idx, evutil_socket_t real_fd)
{
  auto data = new lpc_socket_event_data;
  data->idx = idx;
  lpc_socks[idx].ev_read = event_new(g_event_base, real_fd, EV_READ | EV_PERSIST, on_lpc_sock_read, data);
  lpc_socks[idx].ev_write = event_new(g_event_base, real_fd, EV_WRITE, on_lpc_sock_write, data);
  lpc_socks[idx].ev_data = data;
}

#ifdef HAS_CONSOLE
static void on_console_event(evutil_socket_t fd, short what, void *arg)
{
  debug(event, "Got an event on stdin socket %d:%s%s%s%s \n", (int) fd,
        (what & EV_TIMEOUT) ? " timeout" : "", (what & EV_READ) ? " read" : "",
        (what & EV_WRITE) ? " write" : "", (what & EV_SIGNAL) ? " signal" : "");

  if (has_console <= 0) {
    event_del((struct event *)arg);
    return;
  }
  on_console_input();
}

void init_console(struct event_base *base)
{
  if (has_console > 0) {
    debug_message("Opening console... \n");
    struct event *ev_console = NULL;
    ev_console = event_new(base, STDIN_FILENO,
                           EV_READ | EV_PERSIST, on_console_event, ev_console);
    event_add(ev_console, NULL);
  }
}
#endif
