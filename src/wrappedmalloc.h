#ifndef WRAPPED_MALLOC_H
#define WRAPPED_MALLOC_H
#ifdef WRAPPED_MALLOC
void *wrappedmalloc PROT((int));
void *wrappedrealloc PROT((void *, int));
void *wrappedcalloc PROT((int, int));
void wrappedfree PROT((void *));

void wrappedmalloc_init PROT((void));
void dump_malloc_data PROT((void));
#endif
#endif
