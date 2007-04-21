#ifndef MALLOC_INCL_H
#define MALLOC_INCL_H

typedef struct {
    int real_size;
    char *buffer;
} outbuffer_t;

void outbuf_zero (outbuffer_t *);
void outbuf_add (outbuffer_t *, const char *);
void outbuf_addchar (outbuffer_t *, char);
void CDECL outbuf_addv (outbuffer_t *, const char *, ...);
void outbuf_fix (outbuffer_t *);
void outbuf_push (outbuffer_t *);
int outbuf_extend (outbuffer_t *, int);

#include "bsdmalloc.h"
#include "smalloc.h"
#include "wrappedmalloc.h"
#include "debugmalloc.h"

#ifdef DEBUGMALLOC
/* tags */
#define TAG_TEMPORARY       (1 << 8)
#define TAG_PERMANENT       (2 << 8)
#define TAG_COMPILER        (3 << 8)
#define TAG_DATA            (4 << 8)
#define TAG_MISC            (5 << 8)
#define TAG_MARKED          (1 << 12)

#define TAG_PROGRAM         (TAG_PERMANENT + 11)
#define TAG_CALL_OUT        (TAG_PERMANENT + 12)
#define TAG_INTERACTIVE     (TAG_PERMANENT + 13)
#define TAG_ED              (TAG_PERMANENT + 14)

#define TAG_INC_LIST        (TAG_PERMANENT + 16)
#define TAG_PERM_IDENT      (TAG_PERMANENT + 17)
#define TAG_IDENT_TABLE     (TAG_PERMANENT + 18)
#define TAG_RESERVED        (TAG_PERMANENT + 19)
#define TAG_MUDLIB_STATS    (TAG_PERMANENT + 20)
#define TAG_OBJECT          (TAG_PERMANENT + 21)
#define TAG_OBJ_TBL         (TAG_PERMANENT + 22)
#define TAG_CONFIG          (TAG_PERMANENT + 23)
#define TAG_SIMULS          (TAG_PERMANENT + 24)
#define TAG_SENTENCE        (TAG_PERMANENT + 25)
#define TAG_STR_TBL         (TAG_PERMANENT + 26)
#define TAG_SWAP            (TAG_PERMANENT + 27)
#define TAG_UID             (TAG_PERMANENT + 28)
#define TAG_OBJ_NAME        (TAG_PERMANENT + 29)
#define TAG_PREDEFINES      (TAG_PERMANENT + 30)
#define TAG_LINENUMBERS     (TAG_PERMANENT + 31)
#define TAG_LOCALS          (TAG_PERMANENT + 32)
#define TAG_LPC_OBJECT      (TAG_PERMANENT + 33)
#define TAG_USERS           (TAG_PERMANENT + 34)
#define TAG_DEBUGMALLOC     (TAG_PERMANENT + 35)
#define TAG_HEART_BEAT      (TAG_PERMANENT + 36)
#ifdef PACKAGE_PARSER
#define TAG_PARSER          (TAG_PERMANENT + 37)
#endif
#define TAG_INPUT_TO        (TAG_PERMANENT + 38)
#define TAG_SOCKETS         (TAG_PERMANENT + 39)
#ifdef PACKAGE_DB
#define TAG_DB              (TAG_PERMANENT + 40)
#endif
#define TAG_INTERPRETER     (TAG_PERMANENT + 41)

#define TAG_STRING          (TAG_DATA + 40)
#define TAG_MALLOC_STRING   (TAG_DATA + 41)
#define TAG_SHARED_STRING   (TAG_DATA + 42)
#define TAG_FUNP            (TAG_DATA + 43)
#define TAG_ARRAY           (TAG_DATA + 44)
#define TAG_MAPPING         (TAG_DATA + 45)
#define TAG_MAP_NODE_BLOCK  (TAG_DATA + 46)
#define TAG_MAP_TBL         (TAG_DATA + 47)
#define TAG_BUFFER          (TAG_DATA + 48)
#define TAG_CLASS           (TAG_DATA + 49)
#endif

#endif

