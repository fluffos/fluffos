#ifndef MY_MALLOC_H
#define MY_MALLOC_H

/*
 * to use sysmalloc or malloc replacements
 */
#if defined(SYSMALLOC)
#define MALLOC(x)       malloc(x)
#define FREE(x)         free(x)
#define REALLOC(x,y)    realloc(x,y)
#define CALLOC(x,y)     calloc(x,y)

#endif

#ifdef MALLOC64
#define MALLOC(x)       malloc64(x)
#define FREE(x)         free64(x)
#define REALLOC(x,y)    realloc64(x,y)
#define CALLOC(x,y)     calloc64(x,y)
#ifndef _FUNC_SPEC_
void free64(void *block);
void *realloc64(void *block, int size);
void *calloc64(int num, int size);
void *malloc64(int size);
#endif
#endif

#ifdef MALLOC32
#define MALLOC(x)       malloc32(x)
#define FREE(x)         free32(x)
#define REALLOC(x,y)    realloc32(x,y)
#define CALLOC(x,y)     calloc32(x,y)
#ifndef _FUNC_SPEC_
void free32(void *block);
void *realloc32(void *block, int size);
void *calloc32(int num, int size);
void *malloc32(int size);
#endif
#endif

#define DXALLOC(x,tag,desc)     xalloc(x)
#define DMALLOC(x,tag,desc)     MALLOC(x)
#define DREALLOC(x,y,tag,desc)  REALLOC(x,y)
#define DCALLOC(x,y,tag,desc)   CALLOC(x,y)
#endif /* MY_MALLOC_H */
