/* used by md.c (the malloc debug module) */

#if defined(CHECK_MEMORY) && !defined(DEBUGMALLOC_EXTENSIONS)
#undef CHECK_MEMORY
#endif

typedef struct node_s {
    int size;
    struct node_s *next;
#ifdef DEBUGMALLOC_EXTENSIONS
    int id;
    int tag;
    char *desc;
#endif
#ifdef CHECK_MEMORY
    int magic;
#endif
}      node_t;

#ifdef CHECK_MEMORY
#define MD_OVERHEAD (sizeof(node_t) + sizeof(int))
#define MD_MAGIC 0x4bee4bee

void check_all_blocks PROT((int));
#else
#define MD_OVERHEAD (sizeof(node_t))
#endif

#define TABLESIZE 20357		/* 20357 is prime */

#define PTR(x) ((void *)(x + 1))
#define NODET_TO_PTR(x, y) ((y)(x + 1))
#define PTR_TO_NODET(x) ((node_t *)(x) - 1)

#define DO_MARK(ptr, kind) if (PTR_TO_NODET(ptr)->tag != kind) \
    fprintf(stderr, "Expected node of type %04x: got %s %04x\n", \
        kind, PTR_TO_NODET(ptr)->desc, PTR_TO_NODET(ptr)->tag); \
    else PTR_TO_NODET(ptr)->tag |= TAG_MARKED

#ifdef DEBUGMALLOC_EXTENSIONS
#define SET_TAG(x, y) set_tag(x, y)
#else
#define SET_TAG(x, y)
#endif

extern int malloc_mask;
extern unsigned int total_malloced;
extern unsigned int hiwater;
void MDinit PROT((void));
void MDmalloc PROT((node_t *, int, int, char *));
int MDfree PROT((void *));

#ifdef DEBUGMALLOC_EXTENSIONS
#include "lpc_incl.h"

void mark_svalue PROT((struct svalue *));
void set_tag PROT((void *, int));
#endif

#define MAX_CATEGORY 130



