#define IN_MALLOC_WRAPPER
#define NO_OPCODES
#include "std.h"
#include "lpc_incl.h"
#include "simulate.h"
#include "comm.h"

#ifdef DO_MSTATS
void show_mstats (outbuffer_t * ob, char * s) {
    outbuf_add(ob, "No malloc statistics available with SYSMALLOC\n");
}
#endif

