/* used by md.c (the malloc debug module) */

#if defined(CHECK_MEMORY) && !defined(DEBUGMALLOC_EXTENSIONS)
#undef CHECK_MEMORY
#endif

typedef struct md_node_s {
    int size;
    struct md_node_s *next;
#ifdef DEBUGMALLOC_EXTENSIONS
    int id;
    int tag;
    char *desc;
#endif
#ifdef CHECK_MEMORY
    int magic;
#endif
} md_node_t;

#ifdef CHECK_MEMORY
#define MD_OVERHEAD (sizeof(md_node_t) + sizeof(int))
#define MD_MAGIC 0x4bee4bee

void check_all_blocks (int);
#else
#define MD_OVERHEAD (sizeof(md_node_t))
#endif

#define MD_TABLE_BITS 14
#define MD_TABLE_SIZE (1 << MD_TABLE_BITS)
#define MD_HASH(x) (((unsigned long)x >> 3) & (MD_TABLE_SIZE - 1))

#define PTR(x) ((void *)(x + 1))
#define NODET_TO_PTR(x, y) ((y)(x + 1))
#define PTR_TO_NODET(x) ((md_node_t *)(x) - 1)

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
void MDinit (void);
void MDmalloc (md_node_t *, int, int, char *);
int MDfree (void *);

#ifdef DEBUGMALLOC_EXTENSIONS
struct svalue_s;

void mark_svalue (struct svalue_s *);
void set_tag (const void *, int);
void check_string_stats (outbuffer_t *);
#endif

#define MAX_CATEGORY 130



