/*
   wrapper functions for system malloc -- keep malloc stats.
   Truilkan@TMI - 92/04/17
*/

#define IN_MALLOC_WRAPPER
#define NO_OPCODES
#include "std.h"
#include "my_malloc.h"
#include "lpc_incl.h"
#include "comm.h"

typedef struct stats_s {
    unsigned int free_calls, alloc_calls, realloc_calls;
}       stats_t;

static stats_t stats;

void wrappedmalloc_init()
{
    stats.free_calls = 0;
    stats.alloc_calls = 0;
    stats.realloc_calls = 0;
}

INLINE void *wrappedrealloc (void * ptr, int size)
{
    stats.realloc_calls++;
    return (void *) REALLOC(ptr, size);
}

INLINE void *wrappedmalloc (int size)
{
    stats.alloc_calls++;
    return (void *) MALLOC(size);
}

INLINE void *wrappedcalloc (int nitems, int size)
{
    stats.alloc_calls++;
    return (void *) CALLOC(nitems, size);
}

INLINE void wrappedfree (void * ptr)
{
    stats.free_calls++;
    FREE(ptr);
}

void dump_malloc_data (outbuffer_t * ob)
{
    outbuf_add(ob, "using wrapped malloc:\n\n");
    outbuf_addv(ob, "#alloc calls:     %10lu\n", stats.alloc_calls);
    outbuf_addv(ob, "#free calls:      %10lu\n", stats.free_calls);
    outbuf_addv(ob, "#alloc - #free:   %10lu\n",
		stats.alloc_calls - stats.free_calls);
    outbuf_addv(ob, "#realloc calls:   %10lu\n", stats.realloc_calls);
}
