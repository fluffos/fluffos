#ifndef _LEX_H_
#define _LEX_H_

#define DEFMAX 10000
#define MLEN 4096
#define NSIZE 256
#define MAX_INSTRS 512
#define EXPANDMAX 25000
#define NARGS 25
#define MARKS '@'

#define SKIPWHITE while (isspace(*p)) p++

#define PRAGMA_STRICT_TYPES    1
#define PRAGMA_WARNINGS        2
#define PRAGMA_SAVE_TYPES      4
#define PRAGMA_SAVE_BINARY     8
#define PRAGMA_OPTIMIZE       16

/* With this on, compiler is allowed to
 * assume types are correct
 */
#define OPTIMIZE_HIGH          1				    

#define OPTIMIZE_ALL           OPTIMIZE_HIGH

typedef struct {
    char *word;
    unsigned short token;       /* flags here too */
    short sem_value;            /* semantic value for predefined tokens */
    struct ident_hash_elem *next;
/* the fields above must correspond to struct ident_hash_elem */
    short min_args;		/* Minimum number of arguments. */
    short max_args;		/* Maximum number of arguments. */
    short ret_type;		/* The return type used by the compiler. */
    unsigned short arg_type1;	/* Type of argument 1 */
    unsigned short arg_type2;	/* Type of argument 2 */
    short arg_index;		/* Index pointing to where to find arg type */
    short Default;		/* an efun to use as default for last
				 * argument */
} keyword;

struct lpc_predef_s {
    char *flag;
    struct lpc_predef_s *next;
};

#define EXPECT_ELSE 1
#define EXPECT_ENDIF 2

extern struct lpc_predef_s *lpc_predefs;

#define isalunum(c) (isalnum(c) || (c) == '_')
#endif
