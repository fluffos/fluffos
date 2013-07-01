/*
 * event.h
 *
 *  Created on: Jun 29, 2013
 *      Author: sunyc
 */

#ifndef EVENT_H_
#define EVENT_H_

#include <event2/event.h>
#include <event2/dns.h>

#include "comm.h"

extern struct event_base *g_event_base;

event_base *init_event_base();
int run_for_at_most_one_second(struct event_base *);

// Listening socket event
void new_external_port_event_listener(port_def_t *);

// User socket event
struct user_event_data {
  int idx;
};
void new_user_event_listener(int);
void add_user_write_event();

// LPC socket event
#if defined(PACKAGE_SOCKETS) || defined(PACKAGE_EXTERNAL)
struct lpc_socket_event_data {
       int idx;
};
void add_lpc_sock_event();
void new_lpc_socket_event_listener(int, evutil_socket_t);
#endif

// Console STDIN socket event
#ifdef HAS_CONSOLE
void init_console(struct event_base *);
#endif

#endif /* EVENT_H_ */
