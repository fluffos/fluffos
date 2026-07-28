/* used by md.c (the malloc debug module) */

#ifndef BASE_INTERNAL_MD_H
#define BASE_INTERNAL_MD_H

#include <inttypes.h>
#include <string>

#include "base/internal/options_incl.h"
#include "outbuf.h"

#if defined(CHECK_MEMORY) && !defined(DEBUGMALLOC_EXTENSIONS)
#undef CHECK_MEMORY
#endif

typedef struct md_node_s {
  int size;
  struct md_node_s* next;
#ifdef DEBUGMALLOC_EXTENSIONS
  int id;
  int tag;
  const char* desc;
  uint64_t gametick;
#endif
} md_node_t;

/* The tracker's own references to allocations -- the table[] bucket heads
 * and every node's ->next link -- are stored XOR-mangled. LeakSanitizer's
 * pointer scan must not be able to follow them: with raw pointers stored,
 * every allocation is permanently "reachable" through this table, and a
 * DEBUGMALLOC build can never report a real leak (the dealloc_object()
 * variable-contents leak was invisible to the Debug+ASan CI job exactly
 * this way; only the non-DEBUGMALLOC sanitizer job caught it). Live blocks
 * remain reachable through their real user pointers (LSan honors interior
 * pointers), so only genuinely-unreferenced blocks become leaks. The mask
 * keeps mangled values non-canonical so they can't alias a live chunk.
 * Encode(nullptr) stays nullptr: zero-initialized buckets terminate walks.
 * Note MDfree()'s unlink copies stored (encoded) values verbatim -- only
 * walk READS decode. */
static const uintptr_t kMdChainMask = (uintptr_t)0xA5A5A5A5A5A5A5A5ull;
static inline md_node_t* md_chain_encode(md_node_t* n) {
  return n ? (md_node_t*)((uintptr_t)n ^ kMdChainMask) : nullptr;
}
static inline md_node_t* md_chain_decode(md_node_t* n) {
  return n ? (md_node_t*)((uintptr_t)n ^ kMdChainMask) : nullptr;
}

#ifdef CHECK_MEMORY
#define MD_OVERHEAD (sizeof(md_node_t) + sizeof(int))
#define MD_MAGIC 0x4bee4bee
#else
#define MD_OVERHEAD (sizeof(md_node_t))
#endif
void check_all_blocks(int);
#ifdef DEBUGMALLOC_EXTENSIONS
// Orphaned reference-loop scan (trial deletion); see checkmemory.cc.
// Returns the number of unreachable array/class/mapping/funptr blocks;
// reclaims them when collect is nonzero. ob (nullable) receives detail.
int md_scan_orphaned_cycles(int collect, outbuffer_t* ob);
#endif

#define MD_TABLE_BITS 14u
#define MD_TABLE_SIZE (1u << MD_TABLE_BITS)
#define MD_HASH(x) (((uintptr_t)x >> 3) & (MD_TABLE_SIZE - 1))

#define PTR(x) ((void*)(x + 1))
#define NODET_TO_PTR(x, y) ((y)(x + 1))
#define PTR_TO_NODET(x) ((md_node_t*)(x) - 1)

#define DO_MARK(ptr, kind)                                                                    \
  if (PTR_TO_NODET(ptr)->tag != kind)                                                         \
    debug_message("Expected node of type %04x: got %s %04x\n", kind, PTR_TO_NODET(ptr)->desc, \
                  PTR_TO_NODET(ptr)->tag);                                                    \
  else                                                                                        \
    PTR_TO_NODET(ptr)->tag |= TAG_MARKED

#ifdef DEBUGMALLOC_EXTENSIONS
#define SET_TAG(x, y) set_tag(x, y)
#else
#define SET_TAG(x, y)
#endif

#define MAX_CATEGORY 10
#define MAX_TAGS 255
extern uint64_t blocks[MAX_TAGS];
extern uint64_t totals[MAX_TAGS];

extern md_node_t* table[];

extern int malloc_mask;
extern unsigned int total_malloced;
extern unsigned int hiwater;
void MDmalloc(md_node_t*, int, int, const char*);
int MDfree(md_node_t*);

#ifdef DEBUGMALLOC_EXTENSIONS
void set_tag(const void*, int);
#endif

void md_record_ref_journal(md_node_t* node, bool is_ref, int current_ref, std::string desc);
void md_print_ref_journal(md_node_t* node, outbuffer_t* outbuf);

#endif
