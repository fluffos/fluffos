#ifndef BSDMALLOC_H
#define BSDMALLOC_H

#ifdef BSDMALLOC
void *bsdmalloc_malloc PROT((unsigned int));
void *bsdmalloc_calloc PROT((unsigned int, unsigned int));
void *bsdmalloc_realloc PROT((void *, unsigned int));
void bsdmalloc_free PROT((void *));
#ifdef DO_MSTATS
void show_mstats PROT((char *));
#endif
#endif

#endif
