#ifndef _LEX_H_
#define _LEX_H_

#define DEFMAX 20000  // at least 4 times MAXLINE
#define MAXLINE 4096
#define MLEN 4096
#define NSIZE 256
#define MAX_INSTRS 512
#define EXPANDMAX 25000
#define NARGS 25
#define MARKS '@'

#define SKIPWHITE \
  while (isspace((unsigned char)*p) && (*p != '\n')) p++

#define PRAGMA_STRICT_TYPES 1
#define PRAGMA_WARNINGS 2
#define PRAGMA_SAVE_TYPES 4
#define PRAGMA_SAVE_BINARY 8
#define PRAGMA_OPTIMIZE 16
#define PRAGMA_ERROR_CONTEXT 32
#define PRAGMA_OPTIMIZE_HIGH 64
/* for find_or_add_ident */
#define FOA_GLOBAL_SCOPE 0x1
#define FOA_NEEDS_MALLOC 0x2

typedef struct {
  short local_num, global_num, efun_num;
  short function_num, simul_num, class_num;
} defined_name_t;

typedef struct ifstate_s {
  struct ifstate_s *next;
  int state;
} ifstate_t;

typedef struct defn_s {
  struct defn_s *next;
  char *name;
  char *exps;
  int flags;
  int nargs;
} defn_t;

/* must be a power of 4 */
#define DEFHASH 128
#define defhash(s) (whashstr((s)) & (DEFHASH - 1))

#define DEF_IS_UNDEFINED 1
#define DEF_IS_PREDEF 2
/* used only in edit_source */
#define DEF_IS_NOT_LOCAL 4

/* to speed up cleaning the hash table, and identify the union */
#define IHE_RESWORD 0x8000 /* reserved word */
#define IHE_EFUN 0x4000    /* efun name */
#define IHE_SIMUL 0x2000   /* active simul efun name */
#define IHE_ORPHAN 0x1000  /* old and unused simul efun name */
#define IHE_PERMANENT (IHE_RESWORD | IHE_EFUN | IHE_SIMUL | IHE_ORPHAN)
#define TOKEN_MASK 0x0fff

#define INDENT_HASH_SIZE 1024 /* must be a power of 2 */

struct ident_hash_elem_t {
  const char *name;
  short token;              /* only flags */
  unsigned short sem_value; /* for these, a count of the ambiguity */
  struct ident_hash_elem_t *next;
  /* the fields above must correspond to struct keyword_t */
  struct ident_hash_elem_t *next_dirty;
  defined_name_t dn;
};

struct lpc_predef_t {
  char *flag;
  struct lpc_predef_t *next;
};

#define EXPECT_ELSE 1
#define EXPECT_ENDIF 2

extern lpc_predef_t *lpc_predefs;

/*
 * Information about all instructions. This is not really needed as the
 * automatically generated efun_arg_types[] should be used.
 */

typedef struct {
  short max_arg, min_arg; /* Can't use char to represent -1 */
  short type[4];          /* need a short to hold the biggest type flag */
  short Default;
  unsigned short ret_type;
  const char *name;
  int arg_index;
} instr_t;

/*
 * lex.c
 */
extern instr_t instrs[512];
extern int current_line;
extern int current_line_base;
extern int current_line_saved;
extern int total_lines;
extern char *current_file;
extern int current_file_id;
extern int pragmas;
extern int num_parse_error;
extern lpc_predef_t *lpc_predefs;
extern int efun_arg_types[];
extern char yytext[MAXLINE];
extern int lex_fatal;
extern int arrow_efun, evaluate_efun, this_efun, to_float_efun, to_int_efun, new_efun;

void push_function_context(void);
void pop_function_context(void);
int yylex(void);
void init_num_args(void);
const char *query_instr_name(int);
char *get_f_name(int);
void init_include_path(void);
void deinit_include_path(void);
void set_inc_list(char *);
void start_new_file(int);
void end_new_file(void);
int lookup_predef(const char *);
void add_predefines(void);
char *main_file_name(void);
char *get_defined_name(defined_name_t *);
ident_hash_elem_t *find_or_add_ident(const char *, int);
ident_hash_elem_t *find_or_add_perm_ident(const char *);
ident_hash_elem_t *lookup_ident(const char *);
void free_unused_identifiers(void);
void init_identifiers(void);
char *show_error_context(void);
#ifdef DEBUGMALLOC_EXTENSIONS
void mark_all_defines(void);
#endif

#endif
