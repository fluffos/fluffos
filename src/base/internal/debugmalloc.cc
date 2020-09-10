/*
   wrapper functions for system malloc -- keep malloc stats.
*/

// FIXME: get rid of malloc stats and merge with md.
#include <cstdlib>

#include "debugmalloc.h"

#include "base/internal/md.h"      // for md_node_t, MDmalloc, etc
#include "base/internal/outbuf.h"  // for outbuf_addv, outbuf_add, etc

// Would have been provided somewhere.
void fatal(const char *, ...);

#undef NOISY_MALLOC

#ifdef NOISY_MALLOC
#define NOISY(x) printf(x)
#define NOISY1(x, y) printf(x, y)
#define NOISY2(x, y, z) printf(x, y, z)
#define NOISY3(w, x, y, z) printf(w, x, y, z)
#else
#define NOISY(x)
#define NOISY1(x, y)
#define NOISY2(x, y, z)
#define NOISY3(w, x, y, z)
#endif

typedef struct stats_s {
  unsigned int free_calls, alloc_calls, realloc_calls;
} stats_t;

static stats_t stats = {0, 0, 0};

void *debugrealloc(void *ptr, int size, int tag, const char *desc) {
  void *tmp;

  if (size <= 0) {
    fatal("illegal size in debugrealloc()");
  }

  NOISY3("realloc: %i (%x), %s\n", size, ptr, desc);
  stats.realloc_calls++;
  tmp = reinterpret_cast<md_node_t *>(ptr) - 1;
  if (MDfree(tmp)) {
    tmp = realloc(tmp, size + MD_OVERHEAD);
    MDmalloc(reinterpret_cast<md_node_t *>(tmp), size, tag, desc);
    return reinterpret_cast<md_node_t *>(tmp) + 1;
  }
  return (void *)nullptr;
}

void *debugmalloc(int size, int tag, const char *desc) {
  void *tmp;

  if (size <= 0) {
    fatal("illegal size in debugmalloc()");
  }
  stats.alloc_calls++;
  tmp = malloc(size + MD_OVERHEAD);
  MDmalloc(reinterpret_cast<md_node_t *>(tmp), size, tag, desc);
  NOISY3("malloc: %i (%x), %s\n", size, (md_node_t *)tmp + 1, desc);
  return reinterpret_cast<md_node_t *>(tmp) + 1;
}

void *debugcalloc(int nitems, int size, int tag, const char *desc) {
  void *tmp;

  if (size <= 0) {
    fatal("illegal size in debugcalloc()");
  }

  stats.alloc_calls++;
  tmp = calloc((uint64_t)nitems * size + MD_OVERHEAD, 1);
  MDmalloc(reinterpret_cast<md_node_t *>(tmp), nitems * size, tag, desc);
  NOISY3("calloc: %i (%x), %s\n", nitems * size, (md_node_t *)tmp + 1, desc);
  return reinterpret_cast<md_node_t *>(tmp) + 1;
}

void debugfree(void *ptr) {
  md_node_t *tmp;

  NOISY1("free (%x)\n", ptr);
  stats.free_calls++;
  tmp = reinterpret_cast<md_node_t *>(ptr) - 1;
  if (MDfree(tmp)) {
    free(tmp); /* only free if safe to do so */
  }
}

void dump_malloc_data(outbuffer_t *ob) {
  unsigned int net;

  net = stats.alloc_calls - stats.free_calls;
  outbuf_add(ob, "using debug malloc:\n\n");
  outbuf_addv(ob, "total malloc'd:   %10u\n", total_malloced);
  outbuf_addv(ob, "high water mark:  %10u\n", hiwater);
  outbuf_addv(ob, "overhead:         %10" PRIu64 "\n",
              (MD_TABLE_SIZE * sizeof(md_node_t *)) + (net * MD_OVERHEAD));
  outbuf_addv(ob, "#alloc calls:     %10u\n", stats.alloc_calls);
  outbuf_addv(ob, "#free calls:      %10u\n", stats.free_calls);
  outbuf_addv(ob, "#alloc - #free:   %10u\n", net);
  outbuf_addv(ob, "#realloc calls:   %10u\n", stats.realloc_calls);
}
