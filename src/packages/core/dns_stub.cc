/*
 * dns_stub.cc -- the "no resolver" DNS implementation.
 *
 * Used by targets without a network resolver (the WASM build). The
 * address cache in dns.cc still works; anything that would actually
 * resolve fails exactly like a native host without a configured
 * nameserver. The native driver compiles dns_libevent.cc instead of this
 * file.
 */

#include "base/package_api.h"

#include "packages/core/dns.h"

void init_dns_event_base(struct event_base* /*base*/) {
  // No resolver on this target.
}

void query_name_by_addr(object_t* /*ob*/) {
  // No reverse DNS; query_ip_name() falls back to the numeric address
  // through the shared cache in dns.cc.
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_dns_requests() {}
#endif

int query_addr_by_name(const char* /*name*/, svalue_t* /*call_back*/) {
  // Same failure mode as a native host without a configured nameserver.
  error("resolve: DNS resolver is not available.\n");
  return 0;
}
