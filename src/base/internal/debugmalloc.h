#ifndef MALLOC_INCL_H
#define MALLOC_INCL_H

#include "base/internal/options_incl.h"

/* malloc wrappers */
#if defined(DEBUGMALLOC)
#define FREE(x) debugfree(x)
#define DMALLOC(x, t, d) debugmalloc(x, t, d)
#define DREALLOC(x, y, tag, desc) debugrealloc(x, y, tag, desc)
#define DCALLOC(x, y, tag, desc) debugcalloc(x, y, tag, desc)
#else
#define FREE(x) free(x)
#define DMALLOC(x, tag, desc) malloc(x)
#define DREALLOC(x, y, tag, desc) realloc(x, y)
#define DCALLOC(x, y, tag, desc) calloc(x, y)
#endif

// FIXME: remove this
#define RESIZE(ptr, num, type, tag, desc) \
  ((type *)DREALLOC((void *)ptr, sizeof(type) * (num), tag, desc))

void *debugmalloc(int, int, const char *);
void *debugrealloc(void *, int, int, const char *);
void *debugcalloc(int, int, int, const char *);
void debugfree(void *);

void debugmalloc_init(void);
void dump_malloc_data(struct outbuffer_t *);

void set_malloc_mask(int);
char *dump_debugmalloc(const char *, int);

/* tags */
// NOTE: the digit after + must be unique, range is 0-255.
static const int TAG_TEMPORARY = (1 << 8);
static const int TAG_PERMANENT = (2 << 8);
static const int TAG_COMPILER = (3 << 8);
static const int TAG_DATA = (4 << 8);
static const int TAG_MISC = (5 << 8);
static const int TAG_MARKED = (1 << 12);

static const int TAG_PROGRAM = (TAG_PERMANENT + 11);
static const int TAG_CALL_OUT = (TAG_PERMANENT + 12);
static const int TAG_INTERACTIVE = (TAG_PERMANENT + 13);
static const int TAG_ED = (TAG_PERMANENT + 14);

static const int TAG_INC_LIST = (TAG_PERMANENT + 16);
static const int TAG_PERM_IDENT = (TAG_PERMANENT + 17);
static const int TAG_IDENT_TABLE = (TAG_PERMANENT + 18);
static const int TAG_RESERVED = (TAG_PERMANENT + 19);
static const int TAG_MUDLIB_STATS = (TAG_PERMANENT + 20);
static const int TAG_OBJECT = (TAG_PERMANENT + 21);
static const int TAG_OBJ_TBL = (TAG_PERMANENT + 22);
static const int TAG_CONFIG = (TAG_PERMANENT + 23);
static const int TAG_SIMULS = (TAG_PERMANENT + 24);
static const int TAG_SENTENCE = (TAG_PERMANENT + 25);
static const int TAG_STR_TBL = (TAG_PERMANENT + 26);
static const int TAG_SWAP = (TAG_PERMANENT + 27);
static const int TAG_UID = (TAG_PERMANENT + 28);
static const int TAG_OBJ_NAME = (TAG_PERMANENT + 29);
static const int TAG_PREDEFINES = (TAG_PERMANENT + 30);
static const int TAG_LINENUMBERS = (TAG_PERMANENT + 31);
static const int TAG_LOCALS = (TAG_PERMANENT + 32);
static const int TAG_LPC_OBJECT = (TAG_PERMANENT + 33);
static const int _OBSOLETE_TAG_USER = (TAG_PERMANENT + 34);
static const int TAG_DEBUGMALLOC = (TAG_PERMANENT + 35);
static const int _OBSOLETE_TAG_HEART_BEAT = (TAG_PERMANENT + 36);
#ifdef PACKAGE_PARSER
static const int TAG_PARSER = (TAG_PERMANENT + 37);
#endif
static const int TAG_INPUT_TO = (TAG_PERMANENT + 38);
static const int TAG_SOCKETS = (TAG_PERMANENT + 39);
static const int TAG_STRING = (TAG_DATA + 40);
static const int TAG_MALLOC_STRING = (TAG_DATA + 41);
static const int TAG_SHARED_STRING = (TAG_DATA + 42);
static const int TAG_FUNP = (TAG_DATA + 43);
static const int TAG_ARRAY = (TAG_DATA + 44);
static const int TAG_MAPPING = (TAG_DATA + 45);
static const int TAG_MAP_NODE_BLOCK = (TAG_DATA + 46);
static const int TAG_MAP_TBL = (TAG_DATA + 47);
static const int TAG_BUFFER = (TAG_DATA + 48);
static const int TAG_CLASS = (TAG_DATA + 49);
#ifdef PACKAGE_DB
static const int TAG_DB = (TAG_PERMANENT + 50);
#endif
static const int TAG_INTERPRETER = (TAG_PERMANENT + 51);
#ifdef PACKAGE_PCRE
static const int TAG_PCRE_CACHE = (TAG_PERMANENT + 52);
#endif

#endif
