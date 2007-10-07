#ifndef _STRALLOC_H_
#define _STRALLOC_H_

/* ref-count debugging code */
#undef NOISY_DEBUG
#define NOISY_STRING "workroom"

/* implementation */
#ifdef NOISY_DEBUG
   void bp (void);
#  ifdef NOISY_STRING
#    define NDBG(x) if (strcmp(STRING(x), NOISY_STRING)==0) \
                    debug_message("%s - %d\n", STRING(x), REFS(x)), bp()
#  else
#    define NDBG(x) debug_message("%s - %d\n", STRING(x), REFS(x)), bp()
#  endif
#else
#  define NDBG(x)
#endif

#if defined(DEBUGMALLOC_EXTENSIONS) && defined(STRING_STATS)
/* Uncomment for very complete string ref checking, but be warned it runs
   _very_ slowly.  A conditional definition like:
   
   (current_prog && strcmp(current_prog->name, "foo") == 0 ? check_string_stats(0) : 0)
   
   is usually best.
 */
#define CHECK_STRING_STATS /* check_string_stats(0) */
#else
#define CHECK_STRING_STATS
#endif

#ifdef STRING_STATS
#define ADD_NEW_STRING(len, overhead) num_distinct_strings++; bytes_distinct_strings += len + 1; overhead_bytes += overhead
#define SUB_NEW_STRING(len, overhead) num_distinct_strings--; bytes_distinct_strings -= len + 1; overhead_bytes -= overhead

#define ADD_STRING(len) allocd_strings++; allocd_bytes += len + 1; CHECK_STRING_STATS
#define ADD_STRING_SIZE(len) allocd_bytes += len; bytes_distinct_strings += len
#define SUB_STRING(len) allocd_strings--; allocd_bytes -= len + 1; CHECK_STRING_STATS
#else
/* Blazing fast macros :) */
#define ADD_NEW_STRING(x, y)
#define SUB_NEW_STRING(x, y)
#define ADD_STRING(x)
#define ADD_STRING_SIZE(x)
#define SUB_STRING(x)
#endif

typedef struct malloc_block_s {
#ifdef DEBUGMALLOC_EXTENSIONS
    int extra_ref;
#endif
    unsigned short size;
    unsigned short ref;
} malloc_block_t;

#define MSTR_BLOCK(x) (((malloc_block_t *)(x)) - 1) 
#define MSTR_EXTRA_REF(x) (MSTR_BLOCK(x)->extra_ref)
#define MSTR_REF(x) (MSTR_BLOCK(x)->ref)
#define MSTR_SIZE(x) (MSTR_BLOCK(x)->size)
#define MSTR_UPDATE_SIZE(x, y) SAFE(\
                                    ADD_STRING_SIZE(y - MSTR_SIZE(x));\
                                    MSTR_BLOCK(x)->size = \
                                    (y > USHRT_MAX ? USHRT_MAX : y);\
                                )

#define FREE_MSTR(x) SAFE(\
                          DEBUG_CHECK(MSTR_REF(x) != 1, "FREE_MSTR used on a multiply referenced string\n");\
                          svalue_strlen_size = MSTR_SIZE(x);\
                          SUB_NEW_STRING(svalue_strlen_size, \
                                         sizeof(malloc_block_t));\
                          FREE(MSTR_BLOCK(x));\
                          SUB_STRING(svalue_strlen_size);\
                      )

/* This counts on some rather crucial alignment between malloc_block_t and
 * block_t.  COUNTED_STRLEN(x) is the same as strlen(sv->u.string) when
 * sv->subtype is STRING_MALLOC or STRING_SHARED, and runs significantly
 * faster.
 */
#define COUNTED_STRLEN(x) ((svalue_strlen_size = MSTR_SIZE(x)), svalue_strlen_size != USHRT_MAX ? svalue_strlen_size : strlen((x)+USHRT_MAX)+USHRT_MAX)
/* return the number of references to a STRING_MALLOC or STRING_SHARED 
   string */
#define COUNTED_REF(x)    MSTR_REF(x)

/* ref == 0 means the string has been referenced USHRT_MAX times and is
   immortal */
#define INC_COUNTED_REF(x) if (MSTR_REF(x)) MSTR_REF(x)++;
/* This is a conditional expression that evaluates to zero if the block
   should be deallocated */
#define DEC_COUNTED_REF(x) (!(MSTR_REF(x) == 0 || --MSTR_REF(x) > 0))

typedef struct block_s {
    struct block_s *next;       /* next block in the hash chain */
    unsigned long hash; //although it's actually an int, we need a long for alignment in COUNTED_STRING!
#if defined(DEBUGMALLOC_EXTENSIONS) || (SIZEOF_PTR == 8)
    int extra_ref;
#endif
    /* these two must be last */
    unsigned short size;        /* length of the string */
    unsigned short refs;        /* reference count    */
} block_t;

#define NEXT(x) (x)->next
#define REFS(x) (x)->refs
#define EXTRA_REF(x) (x)->extra_ref
#define SIZE(x) (x)->size
#define HASH(x) (x)->hash
#define BLOCK(x) (((block_t *)(x)) - 1) /* pointer arithmetic */
#define STRING(x) ((char *)(x + 1))

#define SHARED_STRLEN(x) COUNTED_STRLEN(x)

#define SVALUE_STRLEN(x) (((x)->subtype & STRING_COUNTED) ? \
                          COUNTED_STRLEN((x)->u.string) : \
                          strlen((x)->u.string))

/* For quick checks.  Avoid strlen(), etc.  This is  */
#define SVALUE_STRLEN_DIFFERS(x, y) ((((x)->subtype & STRING_COUNTED) && \
                                     ((y)->subtype & STRING_COUNTED)) ? \
                                     MSTR_SIZE((x)->u.string) != \
                                     MSTR_SIZE((y)->u.string) : 0)
/*
 * stralloc.c
 */
void init_strings (void);
char *findstring (const char *);
char *make_shared_string (const char *);
const char *ref_string (const char *);
void free_string (const char *);
void deallocate_string (char *);
int add_string_status (outbuffer_t *, int);

char *extend_string (const char *, int);

extern int svalue_strlen_size;

#ifdef STRING_STATS
extern int num_distinct_strings;
extern int bytes_distinct_strings;
extern int allocd_strings;
extern int allocd_bytes;
extern int overhead_bytes;
#endif

#endif
