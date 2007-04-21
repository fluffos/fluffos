#ifndef SMALLOC_H
#define SMALLOC_H

#ifdef SMALLOC
void * CDECL smalloc_malloc (size_t);
void * CDECL smalloc_realloc (void *, size_t);
void * CDECL smalloc_calloc (size_t, size_t);
void CDECL smalloc_free (void *);
#ifdef DO_MSTATS
void show_mstats (outbuffer_t *, char *);
#endif
#endif

#endif
