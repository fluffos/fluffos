#ifndef DNS_H
#define DNS_H

#include "net/net_compat.h"  // for ev_socklen_t

void init_dns_event_base(struct event_base*);

void query_name_by_addr(struct object_t*);
#ifdef DEBUGMALLOC_EXTENSIONS
void mark_dns_requests(void);
#endif

int query_addr_by_name(const char*, struct svalue_t*);
const char* query_ip_name(struct object_t*);
const char* query_ip_number(struct object_t*);
char* query_host_name(void);

// Record a resolved name in the shared address cache (dns.cc); called by
// the resolver implementation (dns_libevent.cc).
void dns_add_ip_entry(struct sockaddr*, ev_socklen_t, char*);

#endif
