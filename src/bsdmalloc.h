#ifndef BSDMALLOC_H
#define BSDMALLOC_H

#ifdef BSDMALLOC
void *bsdmalloc_malloc (size_t);
void *bsdmalloc_calloc (size_t, size_t);
void *bsdmalloc_realloc (void *, size_t);
void bsdmalloc_free (void *);
#ifdef DO_MSTATS
void show_mstats (outbuffer_t *, char *);
#endif
#endif

#endif
