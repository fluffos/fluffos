/* used by md.c (the malloc debug module) */

typedef struct node_s {
	int size;
	struct node_s *next;
#ifdef DEBUGMALLOC_EXTENSIONS
	int id;
	int tag;
	char *desc;
#endif
} node_t;

#define TABLESIZE 20357  /* 20357 is prime */

#define PTR(x) ((void *)(x + 1))

extern int malloc_mask;

void MDinit();
void MDmalloc PROT((node_t *, int, int, char *));
int MDfree PROT((void *));

#define MAX_CATEGORY 130
