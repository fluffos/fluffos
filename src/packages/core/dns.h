#ifndef DNS_H
#define DNS_H

void init_dns_event_base(struct event_base *);

void query_name_by_addr(struct object_t *);
int query_addr_by_name(const char *, struct svalue_t *);
const char *query_ip_name(struct object_t *);
const char *query_ip_number(struct object_t *);
char *query_host_name(void);
const char *query_ip_number(struct object_t *);

#endif
