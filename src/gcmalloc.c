#define IN_MALLOC_WRAPPER
#include <gc/gc.h>

void malloc_init() {
  GC_INIT();
}
#ifdef DO_MSTATS
void show_mstats (outbuffer_t * ob, char * s) {
    outbuf_add(ob, "No malloc statistics available with SYSMALLOC\n");
}
#endif
