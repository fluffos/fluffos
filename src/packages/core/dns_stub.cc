/*
 * dns_stub.cc -- the "no network resolver" DNS implementation (WASM).
 *
 * There is no nameserver to consult on this target (the page is the only
 * peer), but mudlibs still call resolve() -- some during boot/login,
 * before they can survive an LPC error. Instead of raising, resolve()
 * here succeeds synthetically with the exact call/return shapes of the
 * native resolver (dns_libevent.cc): it returns a key immediately and
 * schedules the LPC callback on the next gametick as
 *
 *     callback(name, ip, key)
 *
 * where 'ip' echoes the input when it is already a numeric address
 * (matching what a native getaddrinfo() of a dotted quad yields) and is
 * "127.0.0.1" otherwise -- the same loopback address every WASM
 * connection reports from query_ip_number().
 *
 * The address cache in dns.cc is shared and compiled unchanged; the
 * native driver compiles dns_libevent.cc instead of this file.
 */

#include "base/package_api.h"

#include "packages/core/dns.h"

#include <arpa/inet.h>   // for inet_pton
#include <netinet/in.h>  // for in_addr/in6_addr

#include <set>

#include "backend.h"

void init_dns_event_base(struct event_base* /*base*/) {
  // No resolver on this target.
}

void query_name_by_addr(object_t* /*ob*/) {
  // No reverse DNS; query_ip_name() falls back to the numeric address
  // through the shared cache in dns.cc.
}

namespace {

// One synthetic resolve() in flight: mirrors AddrNumberQuery in
// dns_libevent.cc (minus the evdns request handle) so the refs it holds
// until the callback runs are identical.
struct StubAddrQuery {
  LPC_INT key;
  const char* name;
  svalue_t call_back;
  object_t* ob_to_call;
  /* User context captured at registration so this_player() survives into
     the callback, like call_out() (issue #1104). Gated on the
     'this_player in call_out' setting. */
  object_t* command_giver;
};

std::set<StubAddrQuery*> pending_addr_queries;

void on_stub_resolve_finish(StubAddrQuery* query) {
  // Same argument shape as on_query_addr_by_name_finish():
  // (name, dotted-quad, key).
  copy_and_push_string(query->name);

  in_addr a4;
  in6_addr a6;
  if (inet_pton(AF_INET, query->name, &a4) == 1 || inet_pton(AF_INET6, query->name, &a6) == 1) {
    // Already numeric: echo it, as a native getaddrinfo() would.
    copy_and_push_string(query->name);
  } else {
    copy_and_push_string("127.0.0.1");
  }

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

  free_string(query->name);
  free_svalue(&query->call_back, "on_stub_resolve_finish");
  free_object(&query->ob_to_call, "on_stub_resolve_finish: ");
  if (query->command_giver) {
    free_object(&query->command_giver, "on_stub_resolve_finish: command_giver");
  }
  pending_addr_queries.erase(query);
  delete query;
}

}  // namespace

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

int query_addr_by_name(const char* name, svalue_t* call_back) {
  static unsigned int key = 0;

  auto* query = new StubAddrQuery;
  memset(query, 0, sizeof(StubAddrQuery));

  query->key = key++;
  query->name = make_shared_string(name);
  query->ob_to_call = current_object;
  assign_svalue_no_free(&query->call_back, call_back);
  if (CONFIG_INT(__RC_THIS_PLAYER_IN_CALL_OUT__) && command_giver) {
    query->command_giver = command_giver;
    add_ref(command_giver, "query_addr_by_name: command_giver");
  }

  add_ref(current_object, "query_addr_by_name: ");
  pending_addr_queries.insert(query);

  // Native resolution completes on a later tick; keep that visible
  // ordering (resolve() returns before the callback ever runs).
  add_gametick_event(0, [=] { return on_stub_resolve_finish(query); });

  debug(dns, "DNS lookup (synthetic): %" LPC_INT_FMTSTR_P ", %s\n", query->key, name);

  return query->key;
}
