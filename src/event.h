/*
 * event.h
 *
 *  Created on: Jun 29, 2013
 *      Author: sunyc
 */

#ifndef EVENT_H_
#define EVENT_H_

#include <functional>

#include <event2/event.h>
#include <event2/dns.h>

extern struct event_base *g_event_base;
event_base *init_event_base();

void init_network_threadpool();
void shutdown_network_threadpool();

struct realtime_event {
  typedef std::function<void()> callback_type;

  callback_type callback;
  realtime_event(callback_type &callback) : callback(callback) {}
};
void add_realtime_event(realtime_event::callback_type);

int run_event_loop(struct event_base *);

// User socket event
struct user_event_data {
  int idx;
};
void new_user_event_listener(struct interactive_t *);
// LPC socket event
#if defined(PACKAGE_SOCKETS) || defined(PACKAGE_EXTERNAL)
struct lpc_socket_event_data {
  int idx;
};
void new_lpc_socket_event_listener(int, struct lpc_socket_t *sock, evutil_socket_t);
#endif

#endif /* EVENT_H_ */
