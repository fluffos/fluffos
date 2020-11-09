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
  g_dns_base = evdns_base_new(base, EVDNS_BASE_INITIALIZE_NAMESERVERS);

#ifdef _WIN32
  // Hack: force loading localhost entires
  evdns_base_load_hosts(g_dns_base, nullptr);
#endif
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
    debug(dns, "DNS reverse lookup fail: %s.\n", evdns_err_to_string(err));
  } else if (count == 0) {
    debug(dns, "DNS reverse lookup returns no result.\n");
  } else {
    auto result = *(reinterpret_cast<char **>(addresses));
    debug(dns, "DNS reverse lookup result: %d: %s\n", type, result);
    add_ip_entry(reinterpret_cast<sockaddr *>(&query->addr), query->addrlen, result);
  }
  delete query;
}

// Start a reverse lookup.
void query_name_by_addr(object_t *ob) {
  auto query = new addr_name_query_t;

  const char *addr = query_ip_number(ob);
  debug(dns, "query_name_by_addr: starting lookup for %s.\n", addr);
  free_string(addr);

  // By the time resolve finish, ob may be already gone, we have to
  // copy the address.
  memcpy(&query->addr, &ob->interactive->addr, ob->interactive->addrlen);
  query->addrlen = ob->interactive->addrlen;

  // Check for mapped v4 address, if we are querying for v6 address.
  if (query->addr.ss_family == AF_INET6) {
    in6_addr *addr6 = &((reinterpret_cast<sockaddr_in6 *>(&query->addr))->sin6_addr);
    if (IN6_IS_ADDR_V4MAPPED(addr6) || IN6_IS_ADDR_V4COMPAT(addr6)) {
      in_addr *addr4 = &(reinterpret_cast<in_addr *>(addr6))[3];
      debug(dns, "Found mapped v4 address, using extracted v4 address to resolve.\n");
      query->req = evdns_base_resolve_reverse(g_dns_base, addr4, 0, on_addr_name_result, query);
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
  char *name;
  svalue_t call_back;
  object_t *ob_to_call;
  evdns_getaddrinfo_request *req;
  int err;
  evutil_addrinfo *res;
};

// query finished, call the LPC callback.
void on_query_addr_by_name_finish(addr_number_query *query) {
  if (query->err) {
    debug(dns, "DNS lookup fail: %" LPC_INT_FMTSTR_P ",request: %s, err: %s.\n", query->key,
          query->name, evutil_gai_strerror(query->err));
    push_undefined();
    push_undefined();
  } else {
    auto result = query->res;
#ifndef IPV6
    // Skip to first IPv4 result.
    while (result != nullptr && result->ai_family != AF_INET) {
      debug(dns, "Skipping IPv6 results %s -> %s \n", query->name,
            sockaddr_to_string(result->ai_addr, result->ai_addrlen));
      result = result->ai_next;
    }
#endif
    if (result == nullptr) {
      debug(dns, "%" LPC_INT_FMTSTR_P ": DNS lookup success but no suitable result.\n", query->key);
      push_undefined();
      push_undefined();
    } else {
      // push the name
      copy_and_push_string(query->name);

      // push IP address
      char host[NI_MAXHOST];
      int ret = getnameinfo(result->ai_addr, result->ai_addrlen, host, sizeof(host), nullptr, 0,
                            NI_NUMERICHOST);
      if (!ret) {
        copy_and_push_string(host);
      } else {
        debug(dns, "on_query_addr_by_name_finish: getnameinfo: %s \n", evutil_gai_strerror(ret));
        push_undefined();
      }
      debug(dns, "DNS lookup success: id %" LPC_INT_FMTSTR_P ": %s -> %s \n", query->key,
            query->name, host);
    }
  }

  // push the key
  push_number(query->key);
  set_eval(max_eval_cost);
  if (query->call_back.type == T_STRING) {
    safe_apply(query->call_back.u.string, query->ob_to_call, 3, ORIGIN_INTERNAL);
  } else {
    safe_call_function_pointer(query->call_back.u.fp, 3);
  }

  if (query->res != nullptr) evutil_freeaddrinfo(query->res);

  free_string(query->name);
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
  static unsigned int key = 0;

  struct evutil_addrinfo hints = {0};
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = EVUTIL_AI_ADDRCONFIG;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;

  auto query = new addr_number_query;
  memset(query, 0, sizeof(addr_number_query));

  query->key = key++;
  query->name = make_shared_string(name);
  query->ob_to_call = current_object;
  assign_svalue_no_free(&query->call_back, call_back);

  add_ref(current_object, "query_addr_number: ");

  query->req = evdns_getaddrinfo(g_dns_base, name, nullptr, &hints, on_getaddr_result, query);

  debug(dns, "DNS lookup scheduled: %" LPC_INT_FMTSTR_P ", %s\n", query->key, name);

  return query->key;
} /* query_addr_number() */

#define IPSIZE 200
typedef struct {
  struct sockaddr_storage addr;
  socklen_t addrlen;
  char *name;
} ipentry_t;

static ipentry_t iptable[IPSIZE];
static int ipcur;

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_iptable() {
  int i;

  for (i = 0; i < IPSIZE; i++)
    if (iptable[i].name) {
      EXTRA_REF(BLOCK(iptable[i].name))++;
    }
}
#endif

const char *query_ip_name(object_t *ob) {
  int i;

  if (ob == nullptr) {
    ob = command_giver;
  }
  if (!ob || ob->interactive == nullptr) {
    return nullptr;
  }
  for (i = 0; i < IPSIZE; i++) {
    if (iptable[i].addrlen == ob->interactive->addrlen &&
        !memcmp(&iptable[i].addr, &ob->interactive->addr, ob->interactive->addrlen) &&
        iptable[i].name) {
      return (iptable[i].name);
    }
  }
  return query_ip_number(ob);
}

static void add_ip_entry(struct sockaddr *addr, socklen_t size, char *name) {
  int i;

  for (i = 0; i < IPSIZE; i++) {
    if (iptable[i].addrlen == size && !memcmp(&iptable[i].addr, addr, size)) {
      return;
    }
  }
  memcpy(&iptable[ipcur].addr, addr, size);
  iptable[ipcur].addrlen = size;

  if (iptable[ipcur].name) {
    free_string(iptable[ipcur].name);
  }
  iptable[ipcur].name = make_shared_string(name);
  ipcur = (ipcur + 1) % IPSIZE;
}

const char *query_ip_number(object_t *ob) {
  if (ob == nullptr) {
    ob = command_giver;
  }
  if (!ob || ob->interactive == nullptr) {
    return nullptr;
  }
  char host[NI_MAXHOST];
  getnameinfo(reinterpret_cast<sockaddr *>(&ob->interactive->addr), sizeof(ob->interactive->addr),
              host, sizeof(host), nullptr, 0, NI_NUMERICHOST);
  return make_shared_string(host);
}
