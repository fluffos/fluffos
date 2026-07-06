#ifndef _LEX_H_
#define _LEX_H_

#include <memory>

#include "compiler/internal/LexStream.h"

#define DEFMAX 65536  // at least 4 times MAXLINE
#define MAXLINE 4096
#define MAX_INSTRS 512

// Preprocessing is part of the lexer's own single scan now (see
// lexer_rules_pp.h and lex.l's directive rule) -- there is no separate
// preprocessor pass, no preprocessed-text intermediate, and no sentinel
// markers. Every directive's effect (macro table, conditional stack,
// #include input-stack push, pragmas flags, current_line/current_file)
// applies at exactly its position in the one and only scan, which is what
// makes position-sensitive directives like a mid-file `#pragma
// no_warnings` correct by construction. See plans/unified-push-lexer.md
// for the design and for the history of why the split-pass alternatives
// were abandoned.

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
#include <string>

#include <unordered_map>
#include <vector>

#define MAX_TEMPLATE_NESTING 16

struct compiler_context_t {
  int template_nesting = 0;
  int template_brace_depth[MAX_TEMPLATE_NESTING] = {0};
  bool template_is_continuation = false;
  std::string str_accum;
  std::string heredoc_terminator;
  bool heredoc_is_array = false;
  int function_flag = 0;
  // Nesting depth of #if/#ifdef directives seen INSIDE a dead branch while
  // in SC_COND_SKIP (the real conditional stack lives in LexerSession).
  int skip_depth = 0;
  // Token metadata for harnesses that reconstruct source from tokens
  // (test_compiler.cc): set when the just-returned L_NUMBER came from a
  // char literal / the just-returned L_STRING came from a template
  // literal. Not consumed by the compiler itself.
  bool is_char_literal = false;
  bool is_template = false;
  // Macro self-reference "blue paint", per OCCURRENCE rather than per
  // region: name -> how many upcoming literal occurrences of that
  // identifier in already-spliced expansion text must resolve as a plain
  // identifier instead of expanding again. Incremented by
  // lpc_lex_expand_string() (via its guard_hits out-param) exactly once
  // for each guarded self-reference it leaves literal in text destined
  // for the ring buffer; decremented by lpc_lex_resolve_identifier() when
  // that occurrence is rescanned. This replaces the earlier
  // active-expansion name stack + "\x1e<name>" end-of-splice sentinel
  // token: the counts carry the same information with no in-band marker
  // byte, no dedicated lex.l rule, and no positional bookkeeping -- and
  // match C-preprocessor semantics more closely (paint sticks to the
  // occurrence itself, not to a scan region).
  std::unordered_map<std::string, int> pending_plain;
};
struct compiler_context_t *yyget_extra(void *yyscanner);
char *yyget_text(void *yyscanner);
int yylex_init_extra(struct compiler_context_t *extra, void **scanner);
int yylex_destroy(void *scanner);

union YYSTYPE;
// LexTokenStream and PreprocessingLexStream live in LexStream.h (included
// above) alongside the rest of the byte/token stream hierarchy they're
// part of, not here.

extern instr_t instrs[MAX_INSTRS];
extern int current_line;
extern int current_line_base;
extern int current_line_saved;
extern int total_lines;
extern const char *current_file;
extern int current_file_id;
extern int pragmas;
extern int num_parse_error;
extern lpc_predef_t *lpc_predefs;
extern int efun_arg_types[];
extern char *outp;     // ring-buffer read cursor; also touched by lex.l's rules
extern char *last_nl;
extern int context;
extern int num_refs;
extern int lex_fatal;
extern int arrow_efun, evaluate_efun, this_efun, to_float_efun, to_int_efun, new_efun;

union YYSTYPE;

void lpc_lex_reset(void *yyscanner);
void push_function_context(void);
void pop_function_context(void);
int yylex(union YYSTYPE *yylval_param, void *yyscanner);          // generated by Flex (lex.autogen.cc)
void lpc_lex_newline(void *yyscanner);  // called from lex.l's "\n" rule
void lexerror(const char *s);  // called from lexer_utils.cc helpers and lex.l's native rules
int lpc_lex_resolve_identifier(union YYSTYPE *yylval_param, void *yyscanner);  // called from lex.l's identifier rule
void handle_pragma(char *str);  // called from lex.l's LPC_PRAGMA_MARKER rule
int parseHeredoc(const char *terminator, int is_array, union YYSTYPE *yylval_param, void *yyscanner);  // called from lex.l's SC_HEREDOC_TERM rules
int parseEofOrIncludePop(union YYSTYPE *yylval_param, void *yyscanner);  // called from lex.l's <<EOF>> rule
int lpc_lex_badlex(unsigned char c, void *yyscanner);  // called from lex.l's "." catch-all rule
void init_num_args(void);

const char *query_instr_name(int);
char *get_f_name(int);
struct LexerSession;
void start_new_file(std::unique_ptr<LexStream> stream, void *yyscanner,
                     std::shared_ptr<LexerSession> session = nullptr);
void end_new_file(void);
int lookup_predef(const char *);
void add_predefines(void);
const char *main_file_name(void);
char *get_defined_name(defined_name_t *);
ident_hash_elem_t *find_or_add_ident(const char *, int);
ident_hash_elem_t *find_or_add_perm_ident(const char *);
ident_hash_elem_t *lookup_ident(const char *);
void free_unused_identifiers(void);
void init_identifiers(void);
char *show_error_context(void);
#ifdef DEBUGMALLOC_EXTENSIONS

#endif
void lpc_lex_flush_lookahead(void *yyscanner);
// Print all predefines using debug_message().
void print_all_predefines();
// Get error/warning message from lexer
std::vector<std::string> prepare_logs(const char *, int, const char *, int, bool);
#endif
