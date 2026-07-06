#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include <vector>

class LexStream;

#include "vm/internal/base/function.h"  // for function_t
#include "vm/internal/base/program.h"   // for DECL_MODS etc
#include "trees.h"
#include "compiler/internal/compiler_utils.h"
#include "compiler/internal/scratchpad.h"

/* The end of a static buffer */
#define EndOf(x) (x + sizeof(x) / sizeof(x[0]))

/* structure for holding information about arguments for function prototype */
struct argument_t {
  short num_arg;
  char flags;
};

/*
 * Information for allocating a block that can grow dynamically
 * using realloc. That means that no pointers should be kept into such
 * an area, as it might be moved.
 */

struct mem_block_t {
  char *block;
  int current_size;
  int max_size;
};

#define START_BLOCK_SIZE 4096

/* NUMPAREAS ares are saved with the program code after compilation,
 * the rest are only temporary.
 */
#define A_PROGRAM 0 /* executable code */
#define A_FUNCTIONS 1
#define A_STRINGS 2 /* table of strings */
#define A_VAR_NAME 3
#define A_VAR_TYPE 4
#define A_LINENUMBERS 5 /* linenumber information */
#define A_FILE_INFO 6   /* start of file line nos */
#define A_INHERITS 7    /* table of inherited progs */
#define A_CLASS_DEF 8
#define A_CLASS_MEMBER 9
#define A_ARGUMENT_TYPES 10 /* */
#define A_ARGUMENT_INDEX 11 /* */
#define NUMPAREAS 12
#define A_CASES 13       /* keep track of cases */
#define A_STRING_NEXT 14 /* next prog string in hash chain */
#define A_STRING_REFS 15 /* reference count of prog string */
#define A_INCLUDES 16    /* list of included files */
#define A_FUNCTIONALS 17
#define A_FUNCTION_DEFS 18
#define A_VAR_TEMP 19 /* table of variables */
#define NUMAREAS 20

#define TREE_MAIN 0
#define TREE_INIT 1
#define NUMTREES 2

#define CURRENT_PROGRAM_SIZE (prog_code - mem_block[A_PROGRAM].block)
#define UPDATE_PROGRAM_SIZE mem_block[A_PROGRAM].current_size = CURRENT_PROGRAM_SIZE

/*
 * Types available. The number '0' is valid as any type. These types
 * are only used by the compiler, when type checks are enabled. Compare with
 * the run-time types, named T_ interpret.h.
 */

#define TYPE_UNKNOWN 0 /* This type must be casted */
#define TYPE_ANY 1     /* Will match any type */
#define TYPE_NOVALUE 2
#define TYPE_VOID 3
#define TYPE_NUMBER 4
#define TYPE_STRING 5
#define TYPE_OBJECT 6
#define TYPE_MAPPING 7
#define TYPE_FUNCTION 8
#define TYPE_REAL 9
#define TYPE_BUFFER 10
#define TYPE_MASK 0xf

struct local_info_t {
  int runtime_index;
  parse_node_t *funcptr_default;
  struct ident_hash_elem_t *ihe;
};

extern mem_block_t mem_block[NUMAREAS];
extern const char *compiler_type_names[];

#define LOOP_CONTEXT 0x1
#define SWITCH_CONTEXT 0x2
#define SWITCH_STRINGS 0x4
#define SWITCH_NUMBERS 0x8
#define SWITCH_DEFAULT 0x10
#define SWITCH_RANGES 0x20
#define SWITCH_NOT_EMPTY 0x40
#define LOOP_FOREACH 0x80
#define SPECIAL_CONTEXT 0x100
#define ARG_LIST 0x200

struct function_context_t {
  parse_node_t *values_list;
  short bindable;
  short num_parameters;
  short num_locals;
  struct function_context_t *parent;
};

extern function_context_t *current_function_context;
extern int var_defined;
extern parse_node_t *comp_trees[NUMTREES];
extern unsigned short *comp_def_index_map;
extern unsigned short *func_index_map;

typedef struct compiler_temp_t {
  unsigned short flags;
  unsigned short offset;
  unsigned short function_index_offset;
  struct program_t *prog; /* inherited if nonzero */
  union {
    struct function_t *func;
    long index;
  } u;
  struct compiler_temp_t *next;
} compiler_temp_t;

/*
 * Some good macros to have.
 */

#define IS_CLASS(t) ((t & (TYPE_MOD_ARRAY | TYPE_MOD_CLASS)) == TYPE_MOD_CLASS)
#define CLASS_IDX(t) (t & ~(DECL_MODS | TYPE_MOD_CLASS))

#define COMP_TYPE(e, t) \
  (!(e & (TYPE_MOD_ARRAY | TYPE_MOD_CLASS)) && (compatible[(e & ~DECL_MODS)] & (1 << (t))))
#define IS_TYPE(e, t) \
  (!(e & (TYPE_MOD_ARRAY | TYPE_MOD_CLASS)) && (is_type[(e & ~DECL_MODS)] & (1 << (t))))

#define FUNCTION_TEMP(n) ((compiler_temp_t *)mem_block[A_FUNCTION_DEFS].block + (n))
#define FUNCTION_NEXT(n) (FUNCTION_TEMP(n)->next)
/* function_t from A_FUNCTIONS index */
#define FUNC(n) ((function_t *)mem_block[A_FUNCTIONS].block + (n))
/* program for inherited entry from full function index */
#define FUNCTION_PROG(n) (FUNCTION_TEMP(n)->prog)
#define FUNCTION_ALIAS(n) (FUNCTION_TEMP(n)->alias_for)
/* function_t from full function index */
#define FUNCTION_DEF(n) \
  (FUNCTION_PROG(n) ? FUNCTION_TEMP(n)->u.func : FUNC(FUNCTION_TEMP(n)->u.index))
/* flags from full function index */
#define FUNCTION_FLAGS(n) (FUNCTION_TEMP(n)->flags)

#define NUM_INHERITS (mem_block[A_INHERITS].current_size / sizeof(inherit_t))

#define INHERIT(n) ((inherit_t *)mem_block[A_INHERITS].block + (n))
#define VAR_TEMP(n) ((variable_t *)mem_block[A_VAR_TEMP].block + (n))
#define SIMUL(n) (simuls[n].func)
#define PROG_STRING(n) (((const char **)mem_block[A_STRINGS].block)[n])
#define CLASS(n) ((class_def_t *)mem_block[A_CLASS_DEF].block + (n))

#define SOME_NUMERIC_CASE_LABELS 0x40000
#define NO_STRING_CASE_LABELS 0x80000

#define ARG_IS_PROTO 1
#define ARG_IS_VARARGS 2

#define NOVALUE_USED_FLAG 1024

int validate_function_call(int, parse_node_t *);
parse_node_t *validate_efun_call(int, parse_node_t *);
extern mem_block_t mem_block[];
extern int exact_types, global_modifiers;
extern int current_type;
extern char *prog_code;
extern char *prog_code_max;
extern unsigned char string_tags[0x20];
extern short freed_string;
extern local_info_t *locals, *locals_ptr;
extern unsigned short *type_of_locals, *type_of_locals_ptr;
extern int current_number_of_locals;
extern int max_num_locals;
extern int current_tree;

extern int type_of_locals_size;
extern int locals_size;
extern int current_number_of_locals;
extern int max_num_locals;
extern short compatible[11];
extern short is_type[11];
extern int comp_last_inherited;

char *get_type_modifiers(char *, char *, int);
char *get_two_types(char *, char *, int, int);
char *get_type_name(char *, char *, int);
void init_locals(void);

void save_file_info(int, int);
int add_program_file(const char *, int);
void yyerror(const char *fmt, ...);
void yyerror(void *yyscanner, const char *msg);
// The %locations form Bison calls (location first, then parse-params).
struct YYLTYPE;
void yyerror(struct YYLTYPE *llocp, void *yyscanner, const char *msg);
void yywarn(const char *fmt, ...);
char *the_file_name(const char *);
void free_all_local_names(int);
void pop_n_locals(int);
void reactivate_current_locals(void);
void clean_up_locals(void);
void deactivate_current_locals(void);
int add_local_name(const char *, int, parse_node_t* = nullptr);
inline int add_local_name(const ScratchString *s, int type, parse_node_t *def = nullptr) {
  return add_local_name(s->c_str(), type, def);
}
void reallocate_locals(void);
void initialize_locals(void);
int get_id_number(void);

// VM context: non-null while the compiler is allowed to interact with the
// running VM — push values onto the eval stack, call master applies
// (APPLY_LOG_ERROR, APPLY_GET_INCLUDE_PATH), record mudlib stats. The
// driver's normal compile paths (load_object() etc.) run with the default
// &g_driver_vm_context. Unit tests that drive the lexer/compiler pieces
// directly, without booting a VM, leave the global null and every VM
// interaction is skipped — the eval stack may not even exist there
// (sp == nullptr), so pushing would be UB, not just wrong.
//
// Opaque-empty by design for now: the only signal is null vs non-null. If
// the compiler ever needs real per-VM state (e.g. multiple isolates), it
// grows fields here instead of new globals.
struct vm_context_t {};
extern vm_context_t g_driver_vm_context;
extern vm_context_t *compiler_vm_context;

// ---------------------------------------------------------------------------
// CompileSession — ONE object per compile_file() invocation: the compile's
// identity (filename, vm_context) and the anchor structured diagnostics
// hang provenance off (see Diagnostic below). `current_compile` points at
// the running compile's session, or null outside any compile.
//
// The parent/depth/chain() machinery exists but is inert by design:
// compile_file() is NEVER re-entered — the compiler is deliberately
// non-reentrant by design. The
// `inherit`-of-an-unloaded-parent case is handled entirely OUTSIDE the
// compiler by load_object() (simulate.cc): the child compile cleanly
// abandons (rule_inheritence() sets the inherit_file global, epilog()
// discards the partial program), then load_object() recursively loads the
// parent — recursion bounded by __INHERIT_CHAIN_SIZE__ — and reloads the
// child from scratch. That abort-and-restart is SAFER than nested
// compiles would be: loading the parent runs arbitrary LPC (its
// create()), which must never execute mid-compile. The guard in
// compile_file() rejecting re-entry is therefore the design, not a
// limitation. If cross-load provenance ("loading X because Y inherits
// it") is ever wanted for diagnostics, it belongs in load_object()'s
// recursion as a name chain feeding capture_diagnostic() — not in
// compiler nesting; parent/chain() here is the ready-made hook for it.
struct CompileSession {
  const char *filename;         // as given to compile_file()
  vm_context_t *vm_context;     // null => no VM interactions allowed
  CompileSession *parent;       // always null today (no nesting; see above)
  int depth;                    // always 0 today

  // Provenance line for diagnostics: "while compiling A, triggered by B,
  // triggered by C". Empty string for a top-level compile with no chain.
  std::string chain() const;
};
extern CompileSession *current_compile;  // the running compile's session

// ---------------------------------------------------------------------------
// Structured diagnostics — every yyerror()/yywarn() records one of these,
// capturing at the moment of the report the provenance a flat text line
// can't carry: the live #include stack, the live macro-expansion chain,
// and site-supplied notes. render_diagnostic()'s clang-style multi-line
// form IS the driver's default output (via report_compile_diagnostic());
// structured consumers (lpcshell) read the fields directly instead.
// ---------------------------------------------------------------------------
struct Diagnostic {
  bool is_warning;
  std::string file;  // innermost file (current_file at capture time)
  int line;          // line within `file`
  // 1-based column of the diagnosed token's START (0 = unknown, so every
  // producer that never learned about columns stays valid). For an error
  // inside a macro expansion this is the OUTERMOST invocation's column --
  // clang's "expansion location"; the inside of the expansion is the
  // expansions[] notes' job.
  int column = 0;
  // The diagnosed physical line's text as still resident in the scanner's
  // innermost real buffer at capture time (empty = unavailable). Rendered
  // as an indented snippet with a caret at `column`.
  std::string snippet;
  std::string message;
  // The live #include stack at capture, innermost includer first:
  // (includer file, line of its #include directive). Rendered clang-style
  // as "In file included from F:N:" prefix lines, outermost first.
  std::vector<std::pair<std::string, int>> included_from;
  // The live macro-expansion chain at capture, innermost first --
  // display-ready lines: "during expansion of macro 'F' (defined at
  // /file:line)". Rendered as indented notes after the main line.
  std::vector<std::string> expansions;
  // Everything else: site-supplied context (compiler_pending_notes, e.g.
  // "previous definition of 'FOO' was at ..."), the compile-session
  // chain. Rendered as indented notes after the expansion chain.
  std::vector<std::string> notes;
  // Fix-it hints (8.5): a replacement for the [col_start, col_end)
  // 1-based column span on the diagnosed line. Rendered as a clang-style
  // replacement line under the caret. Producers attach these only when
  // the fix is unambiguous (e.g. a near-miss #pragma name).
  struct FixIt {
    int col_start;
    int col_end;
    std::string replacement;
  };
  std::vector<FixIt> fixits;
  // Operand/sub-expression ranges (8.3): 1-based [col_start, col_end]
  // spans, rendered as '~' runs on the caret line when `line` matches the
  // diagnosed line. Attached by grammar actions via
  // rule_set_operand_ranges() around type-checking helpers.
  struct Range {
    int line;
    int col_start;
    int col_end;
  };
  std::vector<Range> ranges;
};

// This compile's captured diagnostics, cleared by start_new_file() (i.e.
// per compile / per REPL chunk, on the same boundary num_parse_error
// effectively resets). A plain global rather than a CompileSession member
// so the unit harnesses that drive the lexer without compile_file() (no
// session on the stack) still capture.
extern std::vector<Diagnostic> compiler_diags;

// The clang-style multi-line rendering:
//   In file included from /std/room.c:3:
//   /include/setup.h:9: error: message
//     note: during expansion of macro 'SETUP' (defined at /include/setup.h:2)
//     note: previous definition of 'FOO' was at /include/old.h:4
// `color` adds ANSI severity/caret coloring -- for tty consumers
// (lpcshell checks isatty); the driver's log output stays plain.
std::string render_diagnostic(const Diagnostic &d, bool color = false);

// THE compile-diagnostic output path (compiler_utils.cc): renders `d`
// clang-style, appends the PRAGMA_ERROR_CONTEXT snippet block when that
// pragma is active, prints via debug_message(), and -- when a VM is
// attached (compiler_vm_context) -- reports the same text to the master
// via APPLY_LOG_ERROR. yyerror()/yywarn() call this with the Diagnostic
// they just captured; smart_log() remains only for the RUNTIME callers
// (apply.cc's trace-driven warnings) that have no compile context.
void report_compile_diagnostic(const Diagnostic &d);

// When set, report_compile_diagnostic() captures but does NOT print or
// master-report -- for structured consumers (lpcshell) that render
// compiler_diags themselves, and specifically must not spray a doomed
// trial-parse's errors onto the console before retrying another form.
extern bool compiler_diags_quiet;

// Extra note lines for the NEXT captured diagnostic, consumed (appended +
// cleared) by the capture in yyerror()/yywarn(). Lets an error site attach
// context it alone knows -- e.g. the #define redefinition warning adds
// "previous definition of 'FOO' was at file:line" before reporting. Only
// meaningful immediately before a report; anything stale is cleared at the
// next capture.
extern std::vector<std::string> compiler_pending_notes;
// Fix-it hints queued by the NEXT report's site, same one-shot contract
// as compiler_pending_notes.
extern std::vector<Diagnostic::FixIt> compiler_pending_fixits;

// Load-chain provenance (the optional 6.x note): load_object() sets
// compiler_next_load_reason just before recursively loading an inherited
// file; the NEXT compile's start_new_file() consumes it into
// compiler_current_load_reason (compile-scoped), and every diagnostic of
// that compile carries it as a note ("while loading '/x' inherited by
// '/y'"). One-shot hand-off, so an error() unwind can never leave a
// stale reason attached to an unrelated later compile.
extern std::string compiler_next_load_reason;
extern std::string compiler_current_load_reason;

// Operand ranges queued by a grammar action around a type-checking helper
// (one-shot, consumed by the next captured diagnostic; the action clears
// them after the helper returns so they can never leak to an unrelated
// report). Defined in compiler.cc; the grammar calls the rule_* pair.
extern std::vector<Diagnostic::Range> compiler_pending_ranges;
// The operator's own position: when set (line != 0) and it matches the
// diagnosed line, the caret moves onto the operator -- clang's
// `~~~~~ ^ ~~~~~` shape for binary-op type errors.
extern int compiler_pending_caret_line;
extern int compiler_pending_caret_col;
void rule_set_operand_ranges(int l1, int c1, int e1, int lop, int cop, int l2, int c2, int e2);
void rule_clear_operand_ranges(void);

// Line-attribution override for directive-dispatch reports. The lex.l
// directive rule consumes and counts the directive's terminating newline
// BEFORE dispatching (load-bearing for the #include push and #line
// arithmetic -- see the rule's comment), so during dispatch current_line
// is already the line AFTER the directive; an error reported from inside
// dispatch (#error, a bad #if expression, an unresolvable #include, ...)
// would attribute one line too low. lpc_lex_on_directive() sets this to
// the directive's own first line around the dispatch call and zeroes it
// after; yyerror()/yywarn() use it, when nonzero, instead of
// current_line. Zero everywhere else (parse errors, lexer errors,
// EOF-time errors all attribute via current_line as usual).
extern int compiler_directive_start_line;

// Belt-and-suspenders cap on session-stack depth. Unreachable in practice
// (the reentrancy guard rejects nesting outright, so depth is always 0);
// kept so that if the guard were ever bypassed by a future bug, the
// failure is a clear chained error report instead of runaway recursion.
#define MAX_COMPILE_DEPTH 32

program_t *compile_file(std::unique_ptr<LexStream>, const char *,
                        vm_context_t *vm_context = &g_driver_vm_context);

void reset_function_blocks(void);
void copy_variables(program_t *, int);
void copy_structures(const program_t *);
int copy_functions(program_t *, int);
void type_error(const char *, int);
int compatible_types(int, int);
int compatible_types2(int, int);
int arrange_call_inherited(const char *, parse_node_t *);
void add_arg_type(unsigned short);
int define_new_function(const char *, int, int, int, int);
int define_variable(const char *, int);
int define_new_variable(const char *, int);
inline int define_new_variable(const ScratchString *s, int type) {
  return define_new_variable(s->c_str(), type);
}
short store_prog_string(const char *);
inline short store_prog_string(const ScratchString *s) { return store_prog_string(s->c_str()); }
void free_prog_string(short);
#ifdef DEBUG
int dump_function_table(void);
#endif
void prepare_cases(parse_node_t *, int);
void push_func_block(void);
void pop_func_block(void);
int decl_fix(int);
parse_node_t *check_refs(int, parse_node_t *, parse_node_t *);

int lookup_any_class_member(char *, unsigned short *);
// Like lookup_any_class_member() but silent when no member is found.
int lookup_any_class_member_soft(const char *, unsigned short *);
inline int lookup_any_class_member_soft(const ScratchString *s, unsigned short *t) {
  return lookup_any_class_member_soft(s->c_str(), t);
}
int lookup_class_member(int, const char *, unsigned short *);
inline int lookup_class_member(int which, const ScratchString *s, unsigned short *t) {
  return lookup_class_member(which, s->c_str(), t);
}
parse_node_t *reorder_class_values(int, parse_node_t *);

parse_node_t *promote_to_float(parse_node_t *);
parse_node_t *promote_to_int(parse_node_t *);
parse_node_t *add_type_check(parse_node_t *, int);
parse_node_t *do_promotions(parse_node_t *, int);
parse_node_t *throw_away_call(parse_node_t *);
parse_node_t *throw_away_mapping(parse_node_t *);

#define realloc_mem_block(m)                                                               \
  do {                                                                                     \
    mem_block_t *M = m;                                                                    \
    M->max_size <<= 1;                                                                     \
    M->block = (char *)DREALLOC(M->block, M->max_size, TAG_COMPILER, "realloc_mem_block"); \
  } while (0)

#define add_to_mem_block(n, data, size)                                                     \
  do {                                                                                      \
    mem_block_t *mbp = &mem_block[n];                                                       \
    int Size = size;                                                                        \
                                                                                            \
    if (mbp->current_size + Size > mbp->max_size) {                                         \
      do {                                                                                  \
        mbp->max_size <<= 1;                                                                \
      } while (mbp->current_size + Size > mbp->max_size);                                   \
                                                                                            \
      mbp->block =                                                                          \
          (char *)DREALLOC(mbp->block, mbp->max_size, TAG_COMPILER, "insert_in_mem_block"); \
    }                                                                                       \
    memcpy(mbp->block + mbp->current_size, data, Size);                                     \
    mbp->current_size += Size;                                                              \
  } while (0)

char *allocate_in_mem_block(int, int);



// FIXME: 'inherit_file' is used as a flag.
extern char *inherit_file;

#endif
