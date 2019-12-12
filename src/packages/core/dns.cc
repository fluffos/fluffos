#include "base/package_api.h"

#include "packages/core/dns.h"

#include <event2/event.h>
#include <event2/dns.h>
#include <event2/util.h>
#ifdef _WIN32
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#endif
static struct evdns_base *g_dns_base = nullptr;

void init_dns_event_base(struct event_base *base) {
  // Configure a DNS resolver with default nameserver
  g_dns_base = evdns_base_new(base, 1);
}

static void add_ip_entry(struct sockaddr * /*addr*/, ev_socklen_t size, char * /*name*/);

typedef struct addr_name_query_s {
  sockaddr_storage addr;
  ev_socklen_t addrlen;
  struct evdns_request *req;
} addr_name_query_t;

// Reverse DNS lookup.
void on_addr_name_result(int err, char type, int count, int ttl, void *addresses, void *arg) {
  auto query = reinterpret_cast<addr_name_query_t *>(arg);

  if (err) {
    debug(dns, "DNS reverse lookup fail: {}.\n", evdns_err_to_string(err));
  } else if (count == 0) {
    debug(dns, "DNS reverse lookup returns no result.\n");
  } else {
    auto result = *(reinterpret_cast<char **>(addresses));
    debug(dns, "DNS reverse lookup result: {}: {}\n", type, result);
    add_ip_entry(reinterpret_cast<sockaddr *>(&query->addr), query->addrlen, result);
  }
  delete query;
}

// Start a reverse lookup.
void query_name_by_addr(object_t *ob) {
  auto query = new addr_name_query_t;

  debug(dns, "query_name_by_addr: starting lookup for {}.\n", query_ip_number(ob));

  // By the time resolve finish, ob may be already gone, we have to
  // copy the address.
  memcpy(&query->addr, &ob->interactive->addr, ob->interactive->addrlen);
  query->addrlen = ob->interactive->addrlen;

  // Check for mapped v4 address, if we are querying for v6 address.
  if (query->addr.ss_family == AF_INET6) {
    in6_addr *addr6 = &((reinterpret_cast<sockaddr_in6 *>(&query->addr))->sin6_addr);
    if (IN6_IS_ADDR_V4MAPPED(addr6) || IN6_IS_ADDR_V4COMPAT(addr6)) {
      in_addr *addr4 = &(reinterpret_cast<in_addr *>(addr6))[3];
      debug(dns, "Found mapped v4 address, using extracted v4 address to resolve.\n") query->req =
          evdns_base_resolve_reverse(g_dns_base, addr4, 0, on_addr_name_result, query);
    } else {
      query->req =
          evdns_base_resolve_reverse_ipv6(g_dns_base, addr6, 0, on_addr_name_result, query);
    }
  } else {
    in_addr *addr4 = &(reinterpret_cast<sockaddr_in *>(&query->addr))->sin_addr;
    query->req = evdns_base_resolve_reverse(g_dns_base, addr4, 0, on_addr_name_result, query);
  }
}

struct addr_number_query {
  LPC_INT key;
  shared_string_ptr_t name;
  svalue_t call_back;
  object_t *ob_to_call;
  evdns_getaddrinfo_request *req;
  int err;
  evutil_addrinfo *res;

  addr_number_query(void) :
    key {0},
    name {nullptr},
    call_back {},
    ob_to_call {nullptr},
    req {nullptr},
    err {0},
    res {nullptr}
  {}
};

// query finished, call the LPC callback.
void on_query_addr_by_name_finish(addr_number_query *query) {
  if (query->err) {
    debug(dns, "DNS lookup fail: {},request: {}, err: {}.\n", query->key,
          *(query->name), evutil_gai_strerror(query->err));
    push_undefined();
    push_undefined();
  } else {
    // push the name
    push_string(*(query->name));

    // push IP address
    char host[NI_MAXHOST];
    int ret = getnameinfo(query->res->ai_addr, query->res->ai_addrlen, host, sizeof(host), nullptr,
                          0, NI_NUMERICHOST);
    if (!ret) {
      push_string(host);
    } else {
      debug(dns, "on_query_addr_by_name_finish: getnameinfo: {} \n", evutil_gai_strerror(ret));
      push_undefined();
    }
    debug(dns, "DNS lookup success: id {}: {} -> {} \n", query->key, *(query->name),
          host);
  }

  // push the key
  push_number(query->key);

  if (query->call_back.type == T_STRING) {
    safe_apply(query->call_back.u.string, query->ob_to_call, 3, ORIGIN_INTERNAL);
  } else {
    safe_call_function_pointer(query->call_back.u.fp, 3);
  }

  if (query->res != nullptr) evutil_freeaddrinfo(query->res);

  free_svalue(&query->call_back, "on_addr_result");
  free_object(&query->ob_to_call, "on_addr_result: ");
  delete query;
}

// intermediate result from evdns_getaddrinfo
void on_getaddr_result(int err, evutil_addrinfo *res, void *arg) {
  auto query = reinterpret_cast<addr_number_query *>(arg);
  query->err = err;
  query->res = res;

  // Schedule an immediate event to call LPC callback.
  add_gametick_event(std::chrono::milliseconds(0),
                     [=] { return on_query_addr_by_name_finish(query); });
}

/*
 * Try to resolve "name" and call the callback when finish.
 */
int query_addr_by_name(const char *name, svalue_t *call_back) {
  static int key = 0;

  struct evutil_addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = 0;  // EVUTIL_AI_V4MAPPED | EVUTIL_AI_ADDRCONFIG;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;

  auto query = new addr_number_query {};

  query->key = key++;
  query->name.reset(new shared_string {name});
  query->ob_to_call = current_object;
  assign_svalue_no_free(&query->call_back, call_back);

  add_ref(current_object, "query_addr_number: ");

  query->req = evdns_getaddrinfo(g_dns_base, name, nullptr, &hints, on_getaddr_result, query);

  debug(dns, "DNS lookup scheduled: {}, {}\n", query->key, name);

  return query->key;
} /* query_addr_number() */

#define IPSIZE 200
struct ipentry_t {
  struct sockaddr_storage addr;
  socklen_t addrlen;
  shared_string_ptr_t name;

  ipentry_t(void) : name {nullptr} {}
};

static ipentry_t iptable[IPSIZE];
static int ipcur;

static void add_ip_entry(struct sockaddr *addr, socklen_t size, char *name) {
  size_t i;
  const std::string entry {name};

  for (i = 0; i < IPSIZE; i++) {
    if (iptable[i].addrlen == size && !memcmp(&iptable[i].addr, addr, size)) {
      return;
    }
  }
  memcpy(&iptable[ipcur].addr, addr, size);
  iptable[ipcur].addrlen = size;
  iptable[ipcur].name.reset(new shared_string {entry});

  ipcur = (ipcur + 1) % IPSIZE;
}

const std::string query_ip_name(object_t *ob) {
  int i;

  if (ob == nullptr) {
    ob = command_giver;
  }
  if (!ob || ob->interactive == nullptr) {
    return std::string {};
  }
  for (i = 0; i < IPSIZE; i++) {
    if (iptable[i].addrlen == ob->interactive->addrlen &&
        !memcmp(&iptable[i].addr, &ob->interactive->addr, ob->interactive->addrlen) &&
        iptable[i].name) {
      return *(iptable[i].name);
    }
  }
  return query_ip_number(ob);
}

const std::string query_ip_number(object_t *ob) {
  if (ob == nullptr) {
    ob = command_giver;
  }
  if (!ob || ob->interactive == nullptr) {
    return nullptr;
  }
  char host[NI_MAXHOST];
  getnameinfo(reinterpret_cast<sockaddr *>(&ob->interactive->addr), sizeof(ob->interactive->addr),
              host, sizeof(host), nullptr, 0, NI_NUMERICHOST);
  return std::string(host);
}
