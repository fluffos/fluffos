#ifndef SMALLOC_H
#define SMALLOC_H

#ifdef SMALLOC
void *smalloc_malloc PROT((size_t));
void *smalloc_realloc PROT((void *, size_t));
void *smalloc_calloc PROT((size_t, size_t));
void smalloc_free PROT((void *));
#ifdef DO_MSTATS
void show_mstats PROT((outbuffer_t *, char *));
#endif
#endif

#endif
