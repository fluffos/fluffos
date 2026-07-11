#include "base/package_api.h"

#include "packages/core/dns.h"

#ifndef _WIN32
#include <netdb.h>       // for getnameinfo
#include <netinet/in.h>  // for sockaddr_storage
#include <sys/socket.h>
#else
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#endif

/*
 * The transport-agnostic half of DNS support: the address->name cache
 * consulted by query_ip_name()/query_ip_number(). The resolver that fills
 * it lives in the per-target implementation:
 *
 *   - dns_libevent.cc: evdns-based async resolver (native driver).
 *   - dns_stub.cc: no resolver (WASM -- there is no network to resolve
 *     against); lookups fail the same way as a native host without a
 *     configured nameserver.
 */

enum { IPSIZE = 200 };
using ipentry_t = struct {
  struct sockaddr_storage addr;
  socklen_t addrlen;
  const char* name;
};

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

const char* query_ip_name(object_t* ob) {
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

// Record a resolved name for an address (called by the resolver
// implementation when a reverse lookup finishes).
void dns_add_ip_entry(struct sockaddr* addr, ev_socklen_t size, char* name) {
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

const char* query_ip_number(object_t* ob) {
  if (ob == nullptr) {
    ob = command_giver;
  }
  if (!ob || ob->interactive == nullptr) {
    return nullptr;
  }
  char host[NI_MAXHOST];
  getnameinfo(reinterpret_cast<sockaddr*>(&ob->interactive->addr), sizeof(ob->interactive->addr),
              host, sizeof(host), nullptr, 0, NI_NUMERICHOST);
  return make_shared_string(host);
}
