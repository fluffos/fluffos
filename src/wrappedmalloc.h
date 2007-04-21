#ifndef WRAPPED_MALLOC_H
#define WRAPPED_MALLOC_H
#ifdef WRAPPEDMALLOC
void *wrappedmalloc (int);
void *wrappedrealloc (void *, int);
void *wrappedcalloc (int, int);
void wrappedfree (void *);

void wrappedmalloc_init (void);
void dump_malloc_data (outbuffer_t *);
#endif
#endif
