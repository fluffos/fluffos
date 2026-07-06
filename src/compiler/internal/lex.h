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
// no_warnings` correct by construction. See
// src/compiler/internal/README.md for the architecture.

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
#include <utility>
#include <vector>

#define MAX_TEMPLATE_NESTING 16

struct compiler_context_t {
  int template_nesting = 0;
  int template_brace_depth[MAX_TEMPLATE_NESTING] = {0};
  bool template_is_continuation = false;
  std::string str_accum;
  std::string heredoc_terminator;
  bool heredoc_is_array = false;
  // Nesting depth of #if/#ifdef directives seen INSIDE a dead branch while
  // in SC_COND_SKIP (the real conditional stack lives in LexerSession).
  int skip_depth = 0;
  // Column (0-based) at which the most recently matched token STARTED --
  // snapshotted by lex.l's YY_USER_ACTION before the running yycolumn
  // advances past the match. Diagnostics read this for the caret
  // position (clang points at the construct, not the scan cursor).
  int token_start_column = 0;
  // Token metadata for harnesses that reconstruct source from tokens
  // (test_compiler.cc): set when the just-returned L_NUMBER came from a
  // char literal / the just-returned L_STRING came from a template
  // literal. Not consumed by the compiler itself.
  bool is_char_literal = false;
  bool is_template = false;
  // While true, lpc_lex_resolve_identifier() performs NO macro expansion
  // -- the #if evaluator sets it around pulling a defined()/
  // efun_defined() operand, which C requires to be the unexpanded name.
  bool suppress_expansion = false;
  // (Macro self-reference guarding lives on the expansion-buffer frames
  // now -- a name is plain while a live expansion buffer carries it as
  // its guard; see lpc_lex_name_guarded in lexer_utils.cc. The old
  // per-occurrence "blue paint" map this context used to hold retired
  // with 7.4's raw-body rescan design.)
};
struct compiler_context_t *yyget_extra(void *yyscanner);
char *yyget_text(void *yyscanner);
int yylex_init_extra(struct compiler_context_t *extra, void **scanner);
int yylex_destroy(void *scanner);

union YYSTYPE;
// LexTokenStream lives in LexStream.h (included above) alongside the rest
// of the byte/token stream hierarchy it consumes, not here.

extern instr_t instrs[MAX_INSTRS];

// current_line -- the compiler-facing "line in the current file" -- is
// NATIVE Flex state now (%option yylineno): each buffer carries its own
// per-buffer line counter (yy_bs_lineno), which Flex advances for every
// newline it matches or hands out through yyinput(). The accessor returns
// a reference to the innermost REAL frame's counter -- the top-most
// INCLUDE buffer's, or the base (main file) buffer's; splice buffers'
// counters are synthetic and skipped -- so every existing read, write,
// ++ and -- (#line, the collector's restore, the include arithmetic)
// operates directly on the native storage. Isolation across expansions
// and includes is automatic: pushing a buffer freezes the parent's
// counter, popping resumes it. When no scanner/buffer is live (before the
// first compile, after end_new_file) the reference falls back to
// lpc_lex_line_fallback, which end_new_file loads with the final value so
// post-compile readers (fatal(), tests) see what the legacy global held.
// Defined in lexer_utils.cc, walking the buffer stack through the raw
// introspection primitives below.
int &lpc_lex_current_line_ref(void);
#define current_line (lpc_lex_current_line_ref())
extern int lpc_lex_line_fallback;

// The scanner driving the current compile (compile-scoped singleton; the
// compiler is deliberately non-reentrant). Set by start_new_file, cleared
// by end_new_file. Defined in lexer_utils.cc.
void *lpc_lex_active_scanner(void);

// Kind of the i-th PUSHED buffer (0 = bottom-most pushed, i.e. just above
// the base buffer), or -1 when out of range -- the transient
// one-larger/one-smaller windows around a push/pop treat -1 as "skip".
// Defined in lexer_utils.cc next to the kind stack.
int lpc_lex_buffer_kind_at(int i);

extern int current_line_base;
extern int current_line_saved;
extern int total_lines;
extern const char *current_file;
extern int current_file_id;
extern int pragmas;
extern int num_parse_error;
extern lpc_predef_t *lpc_predefs;
extern int efun_arg_types[];
extern int context;
extern int num_refs;
extern int lex_fatal;
extern int arrow_efun, evaluate_efun, this_efun, to_float_efun, to_int_efun, new_efun;

union YYSTYPE;
struct YYLTYPE;

void lpc_lex_reset(void *yyscanner);
void push_function_context(void);
void pop_function_context(void);
int yylex(union YYSTYPE *yylval_param, struct YYLTYPE *yylloc_param, void *yyscanner);  // generated by Flex (lex.autogen.cc)
void lpc_lex_newline(void *yyscanner);  // called from lex.l's "\n" rule
void lexerror(const char *s);  // called from lexer_utils.cc helpers and lex.l's native rules
// Graceful replacement for flex's stock exit(2) fatal handler; wired in
// via lex.l's YY_FATAL_ERROR override. Defined in compiler.cc (needs
// error()).
[[noreturn]] void lpc_lex_fatal(const char *msg);
int lpc_lex_resolve_identifier(union YYSTYPE *yylval_param, struct YYLTYPE *yylloc_param, void *yyscanner);  // called from lex.l's identifier rule
void handle_pragma(char *str);  // called from the directive dispatcher (lexer_rules_pp.cc)
int parseHeredoc(const char *terminator, int is_array, union YYSTYPE *yylval_param, struct YYLTYPE *yylloc_param, void *yyscanner);  // heredoc body reader (lexer_utils.cc)
// The shared tail of the two SC_HEREDOC_TERM start rules: terminator
// validation + parseHeredoc hand-off (lexer_utils.cc).
int lpc_lex_start_heredoc(union YYSTYPE *yylval_param, struct YYLTYPE *yylloc_param, void *yyscanner);
int parseMainEof(union YYSTYPE *yylval_param, void *yyscanner);  // called from lex.l's <<EOF>> rule
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
#ifdef DEBUGMALLOC_EXTENSIONS

#endif
// Read one character through Flex itself (its buffer, then YY_INPUT) --
// the official mechanism for mid-rule raw reads (heredoc bodies,
// macro-argument collection), replacing direct `outp` access. Returns 0
// at end of input (or a genuine NUL, which LPC source treats the same).
// When the current buffer is a pushed splice (see below) that runs dry,
// it is popped and the read continues transparently from the parent --
// this is what lets a function-like macro's `(args)` be collected across
// a splice boundary (`#define APPLY F` + `APPLY(3)`: the literal `F`
// comes from the splice, its arguments from the file). Defined in lex.l's
// trailer: Flex generates yyinput() as static.
int lpc_lex_getc(void *yyscanner);

// What a pushed Flex buffer holds -- drives the bookkeeping at its pop.
// PLAIN: pushback of probed-but-unused bytes, heredoc array synthesis,
// "(:" re-splices (no bookkeeping). EXPANSION: a macro expansion's text
// (owns the innermost live provenance frame, see
// lpc_lex_expansion_chain). INCLUDE: an #include file's whole content
// (its pop restores current_file/current_line and closes the
// save_file_info line accounting -- the include STACK metadata lives in
// lexer_utils.cc).
enum LpcPushedBufferKind {
  LPC_BUF_PLAIN = 0,
  LPC_BUF_EXPANSION = 1,
  LPC_BUF_INCLUDE = 2,
  // A #if/#elif expression being tokenized for the evaluator: unlike the
  // other kinds, its <<EOF>> does NOT continue into the parent -- it
  // returns LPC_IFEXPR_END so the token pull stops exactly at the
  // expression's edge.
  LPC_BUF_IF_EXPR = 3,
};

// Pseudo-token yylex() returns when a LPC_BUF_IF_EXPR buffer runs dry
// (never reaches the parser: only the #if evaluator's pulls see it).
#define LPC_IFEXPR_END (-2)

// Push `text` as a fresh Flex buffer on top of the current one; scanning
// consumes it fully, then pops back to the parent buffer at exactly the
// position it left off (Flex's own bookkeeping -- no rewind/flush
// choreography). This carries every splice (macro expansions, heredoc
// array-block synthesis, pushbacks of probed bytes) AND every #include's
// content. The pop happens either in lex.l's <<EOF>> rules (Flex scanned
// the buffer dry) or inside lpc_lex_getc() (a raw reader drained it).
// Defined in lex.l's trailer: composing yy_scan_bytes() with the buffer
// STACK requires the "advance the stack top first" fix for
// yy_scan_bytes's internal yy_switch_to_buffer(), which otherwise
// clobbers the live top slot (ASan-verified UAF, see
// this file's git history / the compiler README).
void lpc_lex_push_string_buffer(const char *text, size_t len, int kind, void *yyscanner);

// Pop one pushed buffer (kind-dispatched bookkeeping + yypop_buffer_state).
// Callers must check lpc_lex_pushed_depth() > 0 first.
void lpc_lex_pop_pushed_buffer(void *yyscanner);

// How many pushed buffers are currently stacked on top of the base
// buffer. 0 = scanning the main file's own bytes. Defined in
// lexer_utils.cc.
int lpc_lex_pushed_depth(void);

// Kind of the buffer currently feeding Flex (the stack top), or -1 when
// scanning the base buffer. lex.l's directive rule uses it to decide how
// the directive's terminating newline is consumed. Defined in
// lexer_utils.cc.
int lpc_lex_top_buffer_kind(void);

// Pushed-buffer bookkeeping used by the lex.l-trailer helpers above
// (defined in lexer_utils.cc, next to the expansion frames and include
// metadata they update). ending_lineno is the popped buffer's final
// native line counter (yyget_lineno just before the pop) -- an INCLUDE
// pop feeds it to save_file_info as the include's last line.
void lpc_lex_note_buffer_push(int kind);
void lpc_lex_note_buffer_pop(int ending_lineno);

// Snapshot of the live #include stack for diagnostics, innermost first:
// each entry is (including file, line of the #include directive in it).
// Empty when scanning the top-level file. Defined in lexer_utils.cc, next
// to the include stack it walks.
std::vector<std::pair<std::string, int>> lpc_lex_include_stack(void);

// One live macro-expansion frame, for diagnostics ("during expansion of
// macro 'F' (defined at file:line)"). def_file is empty for predefines/
// builtins (no source definition site).
struct LpcExpansionSite {
  std::string name;
  std::string def_file;
  int def_line;
  // Where the macro was USED (stamped when the expansion buffer was
  // pushed): line in the real file and 1-based start column of the name
  // token. Diagnostics inside expansions attribute to the OUTERMOST
  // site's invocation position (clang's "expansion location").
  int invocation_line;
  int invocation_column;
};
// Snapshot of the macro expansions whose spliced text is still being
// scanned, innermost first. One frame per live EXPANSION splice buffer
// (see lpc_lex_push_string_buffer): pushed with the buffer, marked dead
// when that buffer pops, and purged at the next line boundary rather than
// immediately -- a token's bytes are all consumed before its rule action
// (or the parser's action on it) runs, so an error on a splice's final
// token needs the frame to outlive the buffer briefly (see
// lexer_utils.cc's linger-policy comment). Empty whenever scanning
// ordinary source.
std::vector<LpcExpansionSite> lpc_lex_expansion_chain(void);

// Source-snippet + caret block (current physical line as resident in the
// scanner's current Flex buffer, caret at the scan position) for
// PRAGMA_ERROR_CONTEXT output; empty when unavailable. Shared by
// prepare_logs() and the compile diagnostic reporter; scanner-less
// callers go through the compile-scoped active-scanner slot in
// lexer_utils.cc.
std::string lpc_lex_error_context_block(void);
// The current physical line's text in the innermost REAL frame (top-most
// include buffer or the base main-file buffer) -- the invocation line
// when a splice is scanning. Captured into Diagnostic::snippet at report
// time for the caret block. Empty when unavailable. Defined in
// lexer_utils.cc on the primitives below.
std::string lpc_lex_current_source_line(void);

// Raw buffer-introspection primitives -- pure flex-state accessors with
// zero policy, defined in lex.l's trailer because the generated scanner's
// buffer types are private to that translation unit. Everything built on
// them (the current_line real-frame walk, snippet/caret assembly) lives
// in lexer_utils.cc.
int lpc_lex_buffer_count(void *yyscanner);
int *lpc_lex_buffer_lineno(void *yyscanner, int i);
int lpc_lex_buffer_extents(void *yyscanner, int i, const char **base, const char **limit,
                           const char **pos, char *held);

// Pull one token for the #if/#elif expression evaluator: yylex() under
// the INITIAL start condition (the surrounding scan may be in
// SC_COND_SKIP for a dead-branch #elif), restoring the caller's
// condition after each token. Returns LPC_IFEXPR_END when the expression
// buffer is exhausted. Defined in lex.l's trailer (BEGIN/YY_START are
// private to the generated scanner).
int lpc_lex_ifexpr_next(union YYSTYPE *yylval_param, void *yyscanner);
// Print all predefines using debug_message().
void print_all_predefines();
// Get error/warning message from lexer
std::vector<std::string> prepare_logs(const char *, int, const char *, int, bool);
#endif
