/*
   wrapper functions for system malloc -- keep malloc stats.
   Truilkan@TMI - 92/04/17
*/

/* config.h gets INLINE */
#include "config.h"
#include "lint.h"
#include "interpret.h"
#ifdef NeXT
#include <stdlib.h>
#endif

#include "md.h"

extern unsigned int total_malloced;
extern unsigned int hiwater;

typedef struct stats_s {
	unsigned int free_calls, alloc_calls, realloc_calls;
} stats_t;

static stats_t stats;

void debugmalloc_init()
{
	stats.free_calls = 0;
	stats.alloc_calls = 0;
	stats.realloc_calls = 0;
	MDinit();
}

INLINE void *debugrealloc(ptr,size,tag,desc)
void *ptr;
int size;
int tag;
char *desc;
{
	void *tmp;

	stats.realloc_calls++;
	tmp = (node_t *)ptr - 1;
	if (MDfree(tmp)) {
		tmp = (void *)realloc(tmp,size + sizeof(node_t));
		MDmalloc(tmp, size, tag, desc);
		return (node_t *)tmp + 1;
	}
	return (void *)0;
}

INLINE void *debugmalloc(size, tag, desc)
int size;
int tag;
char *desc;
{
	void *tmp;

	stats.alloc_calls++;
	tmp = (void *)malloc(size + sizeof(node_t));
	MDmalloc(tmp, size, tag, desc);
	return (node_t *)tmp + 1;
}

INLINE void *debugcalloc(nitems,size,tag,desc)
int nitems;
int size;
int tag;
char *desc;
{
	void *tmp;

	stats.alloc_calls++;
	tmp = (void *)calloc(nitems * size + sizeof(node_t), 1);
	MDmalloc(tmp, nitems * size, tag, desc);
	return (node_t *)tmp + 1;
}

INLINE void debugfree(ptr)
void *ptr;
{
	void *tmp;

	stats.free_calls++;
	tmp = (node_t *)ptr - 1;
	if (MDfree(tmp)) {
		free(tmp);  /* only free if safe to do so */
	}
}

void dump_malloc_data()
{
	int net;

	net = stats.alloc_calls - stats.free_calls;
	add_message("using debug malloc:\n\n");
	add_message("total malloc'd:   %10lu\n", total_malloced);
	add_message("high water mark:  %10lu\n", hiwater);
	add_message("overhead:         %10lu\n",
		(TABLESIZE * sizeof(node_t *)) + (net * sizeof(node_t)));
	add_message("#alloc calls:     %10lu\n",stats.alloc_calls);
	add_message("#free calls:      %10lu\n",stats.free_calls);
	add_message("#alloc - #free:   %10lu\n", net);
	add_message("#realloc calls:   %10lu\n",stats.realloc_calls);
}
