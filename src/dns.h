#ifndef DNS_H
#define DNS_H

#include "lpc_incl.h"

void init_dns_event_base(struct event_base *);

void query_name_by_addr(object_t *);
LPC_INT query_addr_by_name(const char *, svalue_t *);
const char *query_ip_name(object_t *);
const char *query_ip_number(object_t *);
char *query_host_name(void);
const char *query_ip_number(object_t *);

#endif
