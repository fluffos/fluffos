#ifndef DNS_H
#define DNS_H

void init_dns_event_base(struct event_base *);

void query_name_by_addr(struct object_t *);
int query_addr_by_name(const std::string, struct svalue_t *);
const std::string query_ip_name(struct object_t *);
const std::string query_ip_number(struct object_t *);
std::string query_host_name(void);

#endif
