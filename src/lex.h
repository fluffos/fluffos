#ifndef _LEX_H_
#define _LEX_H_

#include "std.h"
#include "instrs.h"

#define DEFMAX 10000
#define MAXLINE 1024
#define MLEN 4096
#define NSIZE 256
#define MAX_INSTRS 512
#define EXPANDMAX 25000
#define NARGS 25
#define MARKS '@'

#define SKIPWHITE while (isspace(*p) && (*p != '\n')) p++

#define PRAGMA_STRICT_TYPES    1
#define PRAGMA_WARNINGS        2
#define PRAGMA_SAVE_TYPES      4
#define PRAGMA_SAVE_BINARY     8
#define PRAGMA_OPTIMIZE       16
#define PRAGMA_ERROR_CONTEXT  32

/* With this on, compiler is allowed to
 * assume types are correct
 */
#define OPTIMIZE_HIGH          1				    
#define OPTIMIZE_ALL           OPTIMIZE_HIGH

/* for find_or_add_ident */
#define FOA_GLOBAL_SCOPE       0x1
#define FOA_NEEDS_MALLOC       0x2

typedef struct {
  SIGNED short local_num, global_num, efun_num;
  SIGNED short function_num, simul_num;
} defined_name;

/* to speed up cleaning the hash table, and identify the union */
#define IHE_RESWORD    0x8000
#define IHE_EFUN       0x4000
#define IHE_SIMUL      0x2000
#define IHE_PERMANENT  (IHE_RESWORD | IHE_EFUN | IHE_SIMUL)
#define TOKEN_MASK     0x0fff

#define INDENT_HASH_SIZE 1024 /* must be a power of 2 */

struct ident_hash_elem {
    char *name;
    short token; /* only flags */
    short sem_value; /* for these, a count of the ambiguity */
    struct ident_hash_elem *next;
/* the fields above must correspond to struct keyword */
    struct ident_hash_elem *next_dirty;
    defined_name dn;
};

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

/*
 * lex.c
 */
extern int current_line;
extern int current_line_base;
extern int current_line_saved;
extern int total_lines;
extern char *current_file;
extern int current_file_id;
extern int pragmas;
extern int optimization;
extern int num_parse_error;
extern struct lpc_predef_s *lpc_predefs;
extern int efun_arg_types[];
extern char yytext[1024];
extern struct instr instrs[];
extern keyword predefs[];

int yylex PROT((void));
void init_num_args PROT((void));
char *query_instr_name PROT((int));
char *get_f_name PROT((int));
void set_inc_list PROT((char *));
void start_new_file PROT((int));
void end_new_file PROT((void));
int lookup_predef PROT((char *));
void add_predefines PROT((void));
char *main_file_name PROT((void));
char *get_defined_name PROT((defined_name *));
struct ident_hash_elem *find_or_add_ident PROT((char *, int));
struct ident_hash_elem *find_or_add_perm_ident PROT((char *));
struct ident_hash_elem *lookup_ident PROT((char *));
void free_unused_identifiers PROT((void));
void init_identifiers PROT((void));
char *show_error_context PROT((void));
#ifdef DEBUGMALLOC_EXTENSIONS
void mark_all_defines();
#endif

#endif
