/*
 * dns_libevent.cc -- the native async resolver, on libevent's evdns.
 *
 * Fills the address cache in dns.cc and services the resolve() efun. The
 * WASM build compiles dns_stub.cc instead of this file.
 */

#include "base/package_api.h"

#include <set>

#include "packages/core/dns.h"

#include <event2/event.h>
#include <event2/dns.h>
#include <event2/util.h>
#ifdef _WIN32
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#endif

#include "backend.h"

static struct evdns_base* g_dns_base = nullptr;

void init_dns_event_base(struct event_base* base) {
  // Configure a DNS resolver with default nameserver
  g_dns_base = evdns_base_new(base, EVDNS_BASE_INITIALIZE_NAMESERVERS);

#ifdef _WIN32
  // Hack: force loading localhost entires
  evdns_base_load_hosts(g_dns_base, nullptr);
#endif
}

using addr_name_query_t = struct addr_name_query_s {
  sockaddr_storage addr;
  ev_socklen_t addrlen;
  struct evdns_request* req;
};

// Reverse DNS lookup.
void on_addr_name_result(int err, char type, int count, int /*ttl*/, void* addresses, void* arg) {
  auto* query = reinterpret_cast<addr_name_query_t*>(arg);

  if (err) {
    debug(dns, "DNS reverse lookup fail: %s.\n", evdns_err_to_string(err));
  } else if (count == 0) {
    debug(dns, "DNS reverse lookup returns no result.\n");
  } else {
    auto* result = *(reinterpret_cast<char**>(addresses));
    debug(dns, "DNS reverse lookup result: %d: %s\n", type, result);
    dns_add_ip_entry(reinterpret_cast<sockaddr*>(&query->addr), query->addrlen, result);
  }
  delete query;
}

// Start a reverse lookup.
void query_name_by_addr(object_t* ob) {
  // No resolver configured (see query_addr_by_name): skip rather than
  // dereference a NULL evdns base.
  if (g_dns_base == nullptr) {
    return;
  }

  auto* query = new addr_name_query_t;

  const char* addr = query_ip_number(ob);
  debug(dns, "query_name_by_addr: starting lookup for %s.\n", addr);
  free_string(addr);

  // By the time resolve finish, ob may be already gone, we have to
  // copy the address.
  memcpy(&query->addr, &ob->interactive->addr, ob->interactive->addrlen);
  query->addrlen = ob->interactive->addrlen;

  // Check for mapped v4 address, if we are querying for v6 address.
  if (query->addr.ss_family == AF_INET6) {
    in6_addr* addr6 = &((reinterpret_cast<sockaddr_in6*>(&query->addr))->sin6_addr);
    if (IN6_IS_ADDR_V4MAPPED(addr6) || IN6_IS_ADDR_V4COMPAT(addr6)) {
      in_addr* addr4 = &(reinterpret_cast<in_addr*>(addr6))[3];
      debug(dns, "Found mapped v4 address, using extracted v4 address to resolve.\n");
      query->req = evdns_base_resolve_reverse(g_dns_base, addr4, 0, on_addr_name_result, query);
    } else {
      query->req =
          evdns_base_resolve_reverse_ipv6(g_dns_base, addr6, 0, on_addr_name_result, query);
    }
  } else {
    in_addr* addr4 = &(reinterpret_cast<sockaddr_in*>(&query->addr))->sin_addr;
    query->req = evdns_base_resolve_reverse(g_dns_base, addr4, 0, on_addr_name_result, query);
  }
}

struct AddrNumberQuery {
  LPC_INT key;
  const char* name;
  svalue_t call_back;
  object_t* ob_to_call;
  /* User context captured at registration so this_player() survives into
     the callback, like call_out() (issue #1104). Gated on the
     'this_player in call_out' setting. */
  object_t* command_giver;
  evdns_getaddrinfo_request* req;
  int err;
  evutil_addrinfo* res;
};

// Every in-flight resolve() query: the query owns refs on the target
// object, the callback and the hostname until its callback runs, so the
// DEBUGMALLOC sweep must account for them (mark_dns_requests, mirroring
// mark_call_outs; found by the resolve() efun test + the testsuite's
// per-file leak gate).
static std::set<AddrNumberQuery*> pending_addr_queries;

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_dns_requests() {
  for (auto* query : pending_addr_queries) {
    EXTRA_REF(BLOCK(query->name))++;
    if (query->ob_to_call) {
      query->ob_to_call->extra_ref++;
    }
    if (query->call_back.type == T_STRING) {
      EXTRA_REF(BLOCK(query->call_back.u.string))++;
    } else if (query->call_back.type == T_FUNCTION) {
      query->call_back.u.fp->hdr.extra_ref++;
    }
    if (query->command_giver) {
      query->command_giver->extra_ref++;
    }
  }
}
#endif

// query finished, call the LPC callback.
void on_query_addr_by_name_finish(AddrNumberQuery* query) {
  if (query->err) {
    debug(dns, "DNS lookup fail: %" LPC_INT_FMTSTR_P ",request: %s, err: %s.\n", query->key,
          query->name, evutil_gai_strerror(query->err));
    push_undefined();
    push_undefined();
  } else {
    auto* result = query->res;
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
      int const ret = getnameinfo(result->ai_addr, result->ai_addrlen, host, sizeof(host), nullptr,
                                  0, NI_NUMERICHOST);
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
  /* Restore the user context captured at registration (issue #1104). */
  object_t* new_command_giver = nullptr;
  if (query->command_giver && !(query->command_giver->flags & O_DESTRUCTED)) {
    new_command_giver = query->command_giver;
  }
  save_command_giver(new_command_giver);
  if (query->call_back.type == T_STRING) {
    safe_apply(query->call_back.u.string, query->ob_to_call, 3, ORIGIN_INTERNAL);
  } else {
    safe_call_function_pointer(query->call_back.u.fp, 3);
  }
  restore_command_giver();

  if (query->res != nullptr) evutil_freeaddrinfo(query->res);

  free_string(query->name);
  free_svalue(&query->call_back, "on_addr_result");
  free_object(&query->ob_to_call, "on_addr_result: ");
  if (query->command_giver) {
    free_object(&query->command_giver, "on_addr_result: command_giver");
  }
  pending_addr_queries.erase(query);
  delete query;
}

// intermediate result from evdns_getaddrinfo
void on_getaddr_result(int err, evutil_addrinfo* res, void* arg) {
  auto* query = reinterpret_cast<AddrNumberQuery*>(arg);
  query->err = err;
  query->res = res;

  // Schedule an immediate event to call LPC callback.
  add_gametick_event(0, [=] { return on_query_addr_by_name_finish(query); });
}

/*
 * Try to resolve "name" and call the callback when finish.
 */
int query_addr_by_name(const char* name, svalue_t* call_back) {
  static unsigned int key = 0;

  // evdns_base_new(EVDNS_BASE_INITIALIZE_NAMESERVERS) returns NULL when
  // the host has no configured resolver (common in CI containers/
  // sandboxes); evdns_getaddrinfo(NULL, ...) would then segfault. Fail
  // the resolve cleanly instead of crashing the driver.
  if (g_dns_base == nullptr) {
    error("resolve: DNS resolver is not available.\n");
  }

  struct evutil_addrinfo hints = {0};
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = EVUTIL_AI_ADDRCONFIG;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;

  auto* query = new AddrNumberQuery;
  memset(query, 0, sizeof(AddrNumberQuery));

  query->key = key++;
  query->name = make_shared_string(name);
  query->ob_to_call = current_object;
  assign_svalue_no_free(&query->call_back, call_back);
  if (CONFIG_INT(__RC_THIS_PLAYER_IN_CALL_OUT__) && command_giver) {
    query->command_giver = command_giver;
    add_ref(command_giver, "query_addr_number: command_giver");
  }

  add_ref(current_object, "query_addr_number: ");
  pending_addr_queries.insert(query);

  query->req = evdns_getaddrinfo(g_dns_base, name, nullptr, &hints, on_getaddr_result, query);

  debug(dns, "DNS lookup scheduled: %" LPC_INT_FMTSTR_P ", %s\n", query->key, name);

  return query->key;
} /* query_addr_number() */
