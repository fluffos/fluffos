#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#ifdef SYSMALLOC
#define MALLOC(x)       malloc(x)
#define FREE(x)         free(x)
#define REALLOC(x,y)    realloc(x,y)
#define CALLOC(x,y)     calloc(x,y)
#endif

#ifdef SMALLOC
#define MALLOC(x)       smalloc_malloc(x)
#define FREE(x)         smalloc_free(x)
#define REALLOC(x,y)    smalloc_realloc(x,y)
#define CALLOC(x,y)     smalloc_calloc(x,y)
#endif

#ifdef BSDMALLOC
#define MALLOC(x)       bsdmalloc_malloc(x)
#define FREE(x)         bsdmalloc_free(x)
#define REALLOC(x,y)    bsdmalloc_realloc(x,y)
#define CALLOC(x,y)     bsdmalloc_calloc(x,y)
#endif

#define XALLOC(x)               xalloc(x)
#define DXALLOC(x,tag,desc)     xalloc(x)
#define DMALLOC(x,tag,desc)     MALLOC(x)
#define DREALLOC(x,y,tag,desc)  REALLOC(x,y)
#define DCALLOC(x,y,tag,desc)   CALLOC(x,y)

#endif /* MY_MALLOC_H */
