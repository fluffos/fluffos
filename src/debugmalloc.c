/*
   wrapper functions for system malloc -- keep malloc stats.
   Truilkan@TMI - 92/04/17
*/

#define IN_MALLOC_WRAPPER
#define NO_OPCODES
#include "std.h"
#include "debugmalloc.h"
#include "malloc.h"
#include "md.h"

#undef NOISY_MALLOC

#ifdef NOISY_MALLOC
#define NOISY(x) printf(x)
#define NOISY1(x,y) printf(x,y)
#define NOISY2(x,y,z) printf(x,y,z)
#define NOISY3(w,x,y,z) printf(w,x,y,z)
#else
#define NOISY(x) 
#define NOISY1(x,y) 
#define NOISY2(x,y,z) 
#define NOISY3(w,x,y,z) 
#endif

typedef struct stats_s {
    unsigned int free_calls, alloc_calls, realloc_calls;
}       stats_t;

static stats_t stats;

void debugmalloc_init()
{
    stats.free_calls = 0;
    stats.alloc_calls = 0;
    stats.realloc_calls = 0;
    MDinit();
}

INLINE void *debugrealloc P4(void *, ptr, int, size, int, tag, char *, desc)
{
    void *tmp;

    NOISY3("realloc: %i (%x), %s\n", size, ptr, desc);
    stats.realloc_calls++;
    tmp = (md_node_t *) ptr - 1;
    if (MDfree(tmp)) {
	tmp = (void *) REALLOC(tmp, size + MD_OVERHEAD);
	MDmalloc(tmp, size, tag, desc);
	return (md_node_t *) tmp + 1;
    }
    return (void *) 0;
}

INLINE void *debugmalloc P3(int, size, int, tag, char *, desc)
{
    void *tmp;

    stats.alloc_calls++;
    tmp = (void *) MALLOC(size + MD_OVERHEAD);
    MDmalloc(tmp, size, tag, desc);
    NOISY3("malloc: %i (%x), %s\n", size, (md_node_t *)tmp + 1, desc);
    return (md_node_t *) tmp + 1;
}

INLINE void *debugcalloc P4(int, nitems, int, size, int, tag, char *, desc)
{
    void *tmp;

    stats.alloc_calls++;
    tmp = (void *) CALLOC(nitems * size + MD_OVERHEAD, 1);
    MDmalloc(tmp, nitems * size, tag, desc);
    NOISY3("calloc: %i (%x), %s\n", nitems*size, (md_node_t *)tmp + 1, desc);
    return (md_node_t *) tmp + 1;
}

INLINE void debugfree P1(void *, ptr)
{
    void *tmp;

    NOISY1("free (%x)\n", ptr);
    stats.free_calls++;
    tmp = (md_node_t *) ptr - 1;
    if (MDfree(tmp)) {
	FREE(tmp);		/* only free if safe to do so */
    }
}

void dump_malloc_data P1(outbuffer_t *, ob)
{
    int net;

    net = stats.alloc_calls - stats.free_calls;
    outbuf_add(ob, "using debug malloc:\n\n");
    outbuf_addv(ob, "total malloc'd:   %10lu\n", total_malloced);
    outbuf_addv(ob, "high water mark:  %10lu\n", hiwater);
    outbuf_addv(ob, "overhead:         %10lu\n",
		(TABLESIZE * sizeof(md_node_t *)) + (net * MD_OVERHEAD));
    outbuf_addv(ob, "#alloc calls:     %10lu\n", stats.alloc_calls);
    outbuf_addv(ob, "#free calls:      %10lu\n", stats.free_calls);
    outbuf_addv(ob, "#alloc - #free:   %10lu\n", net);
    outbuf_addv(ob, "#realloc calls:   %10lu\n", stats.realloc_calls);
}
