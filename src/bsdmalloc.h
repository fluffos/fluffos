#ifndef BSDMALLOC_H
#define BSDMALLOC_H

#ifdef BSDMALLOC
void *bsdmalloc_malloc PROT((size_t));
void *bsdmalloc_calloc PROT((size_t, size_t));
void *bsdmalloc_realloc PROT((void *, size_t));
void bsdmalloc_free PROT((void *));
#ifdef DO_MSTATS
void show_mstats PROT((outbuffer_t *, char *));
#endif
#endif

#endif
