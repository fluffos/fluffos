#ifndef SMALLOC_H
#define SMALLOC_H

#ifdef SMALLOC
#ifdef OS2
void *smalloc_malloc PROT((unsigned int));
#else
void *smalloc_malloc PROT((size_t));
#endif
#ifdef OS2
void *smalloc_realloc PROT((void *, unsigned int));
#else
void *smalloc_realloc PROT((void *, size_t));
#endif
#ifdef OS2
void *smalloc_calloc PROT((unsigned int, unsigned int));
#else
void *smalloc_calloc PROT((size_t, size_t));
#endif
void smalloc_free PROT((void *));
#ifdef DO_MSTATS
void show_mstats PROT((outbuffer_t *, char *));
#endif
#endif

#endif
