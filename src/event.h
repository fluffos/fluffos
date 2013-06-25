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

extern struct event_base *g_event_base;

event_base* init_event_base();

void init_console();

struct event* new_external_port_event(port_def_t *);

void run_for_at_most_one_second(struct event_base *);

void add_user_write_event();

#if defined(PACKAGE_SOCKETS) || defined(PACKAGE_EXTERNAL)
void add_lpc_sock_event();
#endif

#endif /* EVENT_H_ */
