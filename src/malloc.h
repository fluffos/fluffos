#ifndef MY_MALLOC_H
#define MY_MALLOC_H

/*
 * to use sysmalloc or malloc replacements
 */
#if defined(SYSMALLOC) || \
    (defined(SMALLOC) && defined(SBRK_OK)) || \
    defined(BSDMALLOC)
#define MALLOC(x)       malloc(x)
#define FREE(x)         free(x)
#define REALLOC(x,y)    realloc(x,y)
#define CALLOC(x,y)     calloc(x,y)

#endif

/* smalloc - choice between replacement or wrapper */
#if defined(SMALLOC) && !defined(SYSMALLOC)
#  ifdef SBRK_OK
#    define smalloc_malloc        malloc
#    define smalloc_free          free
#    define smalloc_realloc       realloc
#    define smalloc_calloc        calloc
#  else
#    define MALLOC(x)       smalloc_malloc(x)
#    define FREE(x)         smalloc_free(x)
#    define REALLOC(x,y)    smalloc_realloc(x,y)
#    define CALLOC(x,y)     smalloc_calloc(x,y)
#  endif
#endif

/* bsdmalloc - always a replacement */
#if defined(BSDMALLOC) && !defined(SYSMALLOC)
#  define bsdmalloc_malloc      malloc
#  define bsdmalloc_free        free
#  define bsdmalloc_realloc     realloc
#  define bsdmalloc_calloc      calloc
#endif

#define DXALLOC(x,tag,desc)     xalloc(x)
#define DMALLOC(x,tag,desc)     MALLOC(x)
#define DREALLOC(x,y,tag,desc)  REALLOC(x,y)
#define DCALLOC(x,y,tag,desc)   CALLOC(x,y)

#endif /* MY_MALLOC_H */
