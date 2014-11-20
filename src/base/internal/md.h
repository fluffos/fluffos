/* used by md.c (the malloc debug module) */

#ifndef BASE_INTERNAL_MD_H
#define BASE_INTERNAL_MD_H

#include "base/internal/options_incl.h"

#if defined(CHECK_MEMORY) && !defined(DEBUGMALLOC_EXTENSIONS)
#undef CHECK_MEMORY
#endif

typedef struct md_node_s {
  int size;
  struct md_node_s *next;
#ifdef DEBUGMALLOC_EXTENSIONS
  int id;
  int tag;
  const char *desc;
#endif
#ifdef CHECK_MEMORY
  int magic;
#endif
} md_node_t;

#ifdef CHECK_MEMORY
#define MD_OVERHEAD (sizeof(md_node_t) + sizeof(int))
#define MD_MAGIC 0x4bee4bee
#else
#define MD_OVERHEAD (sizeof(md_node_t))
#endif
void check_all_blocks(int);

#define MD_TABLE_BITS 14
#define MD_TABLE_SIZE (1 << MD_TABLE_BITS)
#define MD_HASH(x) (((unsigned long)x >> 3) & (MD_TABLE_SIZE - 1))

#define PTR(x) ((void *)(x + 1))
#define NODET_TO_PTR(x, y) ((y)(x + 1))
#define PTR_TO_NODET(x) ((md_node_t *)(x) - 1)

#define DO_MARK(ptr, kind)                                                                      \
  if (PTR_TO_NODET(ptr)->tag != kind)                                                           \
    fprintf(stderr, "Expected node of type %04x: got %s %04x\n", kind, PTR_TO_NODET(ptr)->desc, \
            PTR_TO_NODET(ptr)->tag);                                                            \
  else                                                                                          \
  PTR_TO_NODET(ptr)->tag |= TAG_MARKED

#ifdef DEBUGMALLOC_EXTENSIONS
#define SET_TAG(x, y) set_tag(x, y)
#else
#define SET_TAG(x, y)
#endif

#define MAX_CATEGORY 130
extern md_node_t **table;
extern int totals[MAX_CATEGORY];
extern int blocks[MAX_CATEGORY];

extern int malloc_mask;
extern unsigned int total_malloced;
extern unsigned int hiwater;
void MDinit(void);
void MDmalloc(md_node_t *, int, int, const char *);
int MDfree(void *);

#ifdef DEBUGMALLOC_EXTENSIONS
void set_tag(const void *, int);
#endif

#define LEFT_MAGIC(node) ((node)->magic)
#define RIGHT_MAGIC_ADDR(node) ((unsigned char *)(node) + sizeof(md_node_t) + (node)->size)
#define STORE_RIGHT_MAGIC(node)                                  \
  *(RIGHT_MAGIC_ADDR(node)) = (char)(MD_MAGIC >> 24) & 0xff;     \
  *(RIGHT_MAGIC_ADDR(node) + 1) = (char)(MD_MAGIC >> 16) & 0xff; \
  *(RIGHT_MAGIC_ADDR(node) + 2) = (char)(MD_MAGIC >> 8) & 0xff;  \
  *(RIGHT_MAGIC_ADDR(node) + 3) = (char)MD_MAGIC & 0xff
#define FETCH_RIGHT_MAGIC(l, node)                                                \
  l = (*(RIGHT_MAGIC_ADDR(node)) << 24) + (*(RIGHT_MAGIC_ADDR(node) + 1) << 16) + \
      (*(RIGHT_MAGIC_ADDR(node) + 2) << 8) + (*(RIGHT_MAGIC_ADDR(node) + 3))

#endif
