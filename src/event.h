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
void init_console(struct event_base *);

void new_user_event_listener(interactive_t *);
void new_external_port_event_listener(port_def_t *);

void add_user_write_event();
#if defined(PACKAGE_SOCKETS) || defined(PACKAGE_EXTERNAL)
void add_lpc_sock_event();
#endif

#endif /* EVENT_H_ */
