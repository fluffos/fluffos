/*
 * File: lexer_utils.cc
 *
 * Combines the lexer's global state and non-scanning helpers (identifier
 * hash table, the main-stream YY_INPUT bridge, pushed-buffer/include
 * bookkeeping, heredoc body matching, pragma/error-context handling) with
 * the standalone-preprocessor support helpers (predefines registry,
 * include-path resolution). Originally two separate
 * files -- lex.c/lex.cc (renamed to lex_util.cc once the Flex migration in
 * lex.l left only this residual state+helpers behind) and lexer_utils.cc --
 * merged here since both serve the same "lexer support" role. See
 * plans/flex-lexer-migration.md.
 *
 * Revision:
 * 93-06-27 (Robocoder):
 *   Adjusted the meaning of the EXPECT_* flags;
 *     EXPECT_ELSE  ... means the last condition was false, so we want to find
 *                      an alternative or the end of the conditional block
 *     EXPECT_ENDIF ... means the last condition was true, so we want to find
 *                      the end of the conditional block
 *   Added #elif preprocessor command
 *   Fixed get_text_block bug so no text returned ""
 *   Added get_array_block()...using @@ENDMARKER to return array of strings
 */

#include "base/std.h"

#include "compiler/internal/lexer_utils.h"
#include "compiler/internal/lex.h"
#include "compiler/internal/lexer_rules_pp.h"

#include <cstdio>    // for EOF
#include <fcntl.h>   // for O_RDONLY etc
#include <cstdlib>   // for exit(), FIXME
#include <cctype>    // for isspace
#include <unistd.h>  // for read(), FIXME
#include <vector>
#include <algorithm>  // for std::sort
#include <sstream>
#include <unicode/ustring.h>
#include <fmt/format.h>

#include "vm/vm.h"
#include "efuns.autogen.h"
#include "vm/internal/base/program.h"
#include "vm/internal/base/svalue.h"
#include "compiler.h"
#include "keyword.h"
#include "options.autogen.h"
#include "base/internal/file.h"

#include "compiler/internal/grammar_rules.h"
#include "grammar.autogen.h"

#include "scratchpad.h"

#include "symbol.h"
#include "compiler/internal/LexStream.h"

#ifdef _WIN32
#include <io.h>
#endif

// FIXME: in master.h
extern struct object_t *master_ob;
// FIXME: in file.h
extern const char *check_valid_path(const char *, object_t *, const char *const, int);

// FIXME: lexer() is using global stack machine?!
void push_malloced_string(const char *p);
void pop_stack();

// FIXME: lexer needs a list of predefines
extern int NUM_OPTION_DEFS;

#define NELEM(a) (sizeof(a) / sizeof((a)[0]))

// FIXME: This means current source code can not contain "NUL" byte,
//  for now it seems suffice, but this should be fixed to check pointer address
//  for EOF, not for value.

char lex_ctype[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define is_wspace(c) lex_ctype[(unsigned char)(c)]

#define SKIPWHITE \
  while (is_wspace((unsigned char)*p) && (*p != '\n')) p++

// The `current_line` macro's fallback storage (see lex.h): holds the
// value when no scanner/buffer is live -- loaded by end_new_file() with
// the final native value so post-compile readers see what the legacy
// global held.
int lpc_lex_line_fallback = 0;
int current_line_base;  /* number of lines from other files */
int current_line_saved; /* last line in this file where line num
                           info was saved */
int total_lines;        /* Used to compute average compiled lines/s */
const char *current_file;
int current_file_id;
static const char *main_filename = nullptr;

/* Bit flags for pragmas in effect */
int pragmas;

int num_parse_error; /* Number of errors in the parser. */

lpc_predef_t *lpc_predefs = nullptr;

namespace {
std::unique_ptr<LexStream> current_stream = nullptr;
// Last byte handed to Flex from the main stream, for the trailing-newline
// invariant in lpc_lex_yy_input. Reset per compile in start_new_file.
char main_last_char = '\n';
}  // namespace
std::shared_ptr<LexerSession> current_session = nullptr;

int lex_fatal;

// One level of the #include stack -- pure metadata now. The included
// file's CONTENT is a pushed Flex buffer (lpc_lex_push_string_buffer with
// LPC_BUF_INCLUDE), so the parent's resume position is kept by Flex's own
// buffer stack; what remains here is what the pop must restore for
// diagnostics and the per-file line accounting (save_file_info /
// current_line_base arithmetic). `file` is a shared string owned by the
// entry until its pop.
struct IncState {
  int line;
  const char *file;
  int file_id;
};

static std::vector<IncState> inc_stack;

/* prevent unbridled recursion */
#define MAX_INCLUDE_DEPTH 32

/* If more than this is needed, the code needs help :-) */
#define MAX_FUNCTION_DEPTH 10

static function_context_t function_context_stack[MAX_FUNCTION_DEPTH];
static int last_function_context;
function_context_t *current_function_context = nullptr;

int arrow_efun, evaluate_efun, this_efun, to_float_efun, to_int_efun, new_efun;

/*
 * The number of arguments stated below, are used by the compiler.
 * If min == max, then no information has to be coded about the
 * actual number of arguments. Otherwise, the actual number of arguments
 * will be stored in the byte after the instruction.
 * A maximum value of -1 means unlimited maximum value.
 *
 * If an argument has type 0 (T_INVALID) specified, then no checks will
 * be done at run time.
 *
 * The argument types are currently not checked by the compiler,
 * only by the runtime.
 */

static keyword_t reswords[] = {
#ifdef DEBUG
    {"__TREE__", L_TREE, 0},
#endif
#ifdef ARRAY_RESERVED_WORD
    {"array", L_ARRAY, 0},
#endif
    {"asm", 0, 0},
    {"break", L_BREAK, 0},
    {"buffer", L_BASIC_TYPE, TYPE_BUFFER},
    {"case", L_CASE, 0},
    {"catch", L_CATCH, 0},
#ifdef STRUCT_CLASS
    {"class", L_CLASS, 0},
#endif
    {"continue", L_CONTINUE, 0},
    {"default", L_DEFAULT, 0},
    {"do", L_DO, 0},
    {"efun", L_EFUN, 0},
    {"else", L_ELSE, 0},
    {"float", L_BASIC_TYPE, TYPE_REAL},
    {"for", L_FOR, 0},
    {"foreach", L_FOREACH, 0},
    {"function", L_BASIC_TYPE, TYPE_FUNCTION},
    {"if", L_IF, 0},
    {"in", L_IN, 0},
    {"inherit", L_INHERIT, 0},
    {"int", L_BASIC_TYPE, TYPE_NUMBER},
    {"mapping", L_BASIC_TYPE, TYPE_MAPPING},
    {"mixed", L_BASIC_TYPE, TYPE_ANY},
    {"new", L_NEW, 0},
    {"nomask", L_TYPE_MODIFIER, DECL_NOMASK},
#ifdef SENSIBLE_MODIFIERS
    {"nosave", L_TYPE_MODIFIER, DECL_NOSAVE},
#endif
    {"object", L_BASIC_TYPE, TYPE_OBJECT},
    {"parse_command", L_PARSE_COMMAND, 0},
    {"private", L_TYPE_MODIFIER, DECL_PRIVATE},
    {"protected", L_TYPE_MODIFIER, DECL_PROTECTED},
#ifdef SENSIBLE_MODIFIERS
    {"public", L_TYPE_MODIFIER, DECL_PUBLIC},
#else
    {"public", L_TYPE_MODIFIER, DECL_VISIBLE},
#endif
#ifdef REF_RESERVED_WORD
    {"ref", L_REF, 0},
#endif
    {"return", L_RETURN, 0},
    {"sscanf", L_SSCANF, 0},
#ifndef SENSIBLE_MODIFIERS
    {"static", L_TYPE_MODIFIER, DECL_NOSAVE | DECL_PROTECTED},
#endif
    {"string", L_BASIC_TYPE, TYPE_STRING},
#ifdef STRUCT_STRUCT
    {"struct", L_CLASS, 0},
#endif
    {"switch", L_SWITCH, 0},
    {"time_expression", L_TIME_EXPRESSION, 0},
    {"varargs", L_TYPE_MODIFIER, FUNC_VARARGS},
    {"void", L_BASIC_TYPE, TYPE_VOID},
    {"while", L_WHILE, 0},
};

// Used to determine valid ID chars (variable name function name etc)
#define isalunum(c) (uisalnum(c) || (c) == '_')

static ident_hash_elem_t **ident_hash_table;
static ident_hash_elem_t **ident_hash_head;
static ident_hash_elem_t **ident_hash_tail;

static ident_hash_elem_t *ident_dirty_list = nullptr;

instr_t instrs[MAX_INSTRS];

static ident_hash_elem_t *quick_alloc_ident_entry(void);

int lookup_predef(const char *name) {
  unsigned int x;

  for (x = 0; x < size_of_predefs; x++) {
    if (strcmp(name, predefs[x].word) == 0) {
      return x;
    }
  }

  return -1;
}



// Non-static: called directly from lex.l's native $N and open-paren rules
// (Phase 6), in addition to the legacy helpers still in this file.
void lexerror(const char *s) {
  yyerror(s);
  lex_fatal++;
}



// Called from lex.l's "\n" rule and the other newline-consuming actions.
// LINE counting is native now (%option yylineno: Flex counts newlines in
// matched text and through yyinput() into the current buffer's own
// counter -- see lex.h's current_line macro); what remains here is the
// total-lines statistic and the line-boundary purge of dead expansion
// frames (see the linger policy at purge_exhausted_expansions).
static void purge_exhausted_expansions();
void lpc_lex_newline(void * /*yyscanner*/) {
  total_lines++;
  purge_exhausted_expansions();
}

typedef struct {
  const char *name;
  int value;
} pragma_t;

static pragma_t our_pragmas[] = {{"strict_types", PRAGMA_STRICT_TYPES},
                                 {"save_types", PRAGMA_SAVE_TYPES},
                                 {"warnings", PRAGMA_WARNINGS},
                                 {"optimize", PRAGMA_OPTIMIZE},
                                 {"show_error_context", PRAGMA_ERROR_CONTEXT},
                                 {nullptr, 0}};

void handle_pragma(char *str) {
  int i;
  int no_flag;

  if (!*str) {
    yywarn("Unknown #pragma, ignored.");
    return;
  }

  // Ignore trailing whitespaces
  char *p = &str[strlen(str) - 1];
  while (p >= str && iswspace(*p)) {
    *p-- = '\0';
  }
  if (!*str) {
    yywarn("Unknown #pragma, ignored.");
    return;
  }

  if (strncmp(str, "no_", 3) == 0) {
    str += 3;
    no_flag = 1;
  } else {
    no_flag = 0;
  }

  for (i = 0; our_pragmas[i].name; i++) {
    if (strcmp(our_pragmas[i].name, str) == 0) {
      if (no_flag) {
        pragmas &= ~our_pragmas[i].value;
      } else {
        pragmas |= our_pragmas[i].value;
      }
      return;
    }
  }
  // Near-miss suggestion (8.5/8.6): an unknown pragma within edit
  // distance 2 of a real one gets a "did you mean" note.
  {
    auto edit_distance = [](const char *a, const char *b) {
      size_t la = strlen(a), lb = strlen(b);
      std::vector<size_t> prev(lb + 1), cur(lb + 1);
      for (size_t j = 0; j <= lb; j++) prev[j] = j;
      for (size_t ii = 1; ii <= la; ii++) {
        cur[0] = ii;
        for (size_t j = 1; j <= lb; j++) {
          size_t sub = prev[j - 1] + (a[ii - 1] == b[j - 1] ? 0 : 1);
          cur[j] = std::min(std::min(prev[j] + 1, cur[j - 1] + 1), sub);
        }
        std::swap(prev, cur);
      }
      return prev[lb];
    };
    const char *best = nullptr;
    size_t best_d = 3;
    for (i = 0; our_pragmas[i].name; i++) {
      size_t d = edit_distance(str, our_pragmas[i].name);
      if (d < best_d) {
        best_d = d;
        best = our_pragmas[i].name;
      }
    }
    if (best != nullptr) {
      compiler_pending_notes.push_back(std::string("did you mean '#pragma ") +
                                       (no_flag ? "no_" : "") + best + "'?");
    }
  }
  yywarn("Unknown #pragma, ignored.");
}

// The scanner whose buffers feed the CURRENT compile -- the compiler is
// deliberately non-reentrant (see compiler.h), so a single slot suffices.
// Set by start_new_file(), cleared by end_new_file(); consulted by the
// error-context snippet below, whose callers (lexerror/yyerror paths)
// don't carry a scanner.
static void *active_scanner = nullptr;

void *lpc_lex_active_scanner(void) { return active_scanner; }

// Native-position logic (current_line, diagnostic snippets, the legacy
// error-context block) lives further down, built on lex.l's raw
// buffer-introspection primitives.
// ---------------------------------------------------------------------------
// Native-position logic, built on lex.l's raw buffer-introspection
// primitives (lpc_lex_buffer_count/lineno/extents -- the generated
// scanner's buffer types are private to that translation unit, so lex.l
// exposes accessors and ALL policy lives here).
// ---------------------------------------------------------------------------

// Index of the innermost REAL frame on the buffer stack: the top-most
// INCLUDE buffer, else the base buffer (0). Splice buffers' positions are
// synthetic; an out-of-range kind (-1, the transient window while a
// push/pop is half-done) is skipped the same way. -1 = no buffers.
static int innermost_real_buffer_index(void *yyscanner) {
  int count = lpc_lex_buffer_count(yyscanner);
  if (count <= 0) {
    return -1;
  }
  for (int i = count - 1; i > 0; --i) {
    if (lpc_lex_buffer_kind_at(i - 1) == LPC_BUF_INCLUDE &&
        lpc_lex_buffer_lineno(yyscanner, i) != nullptr) {
      return i;
    }
  }
  return 0;
}

// The storage behind the `current_line` macro (lex.h): a reference to the
// innermost real frame's native line counter, falling back to
// lpc_lex_line_fallback when no scanner or buffer is live.
int &lpc_lex_current_line_ref(void) {
  void *yyscanner = active_scanner;
  if (yyscanner == nullptr) {
    return lpc_lex_line_fallback;
  }
  int idx = innermost_real_buffer_index(yyscanner);
  if (idx < 0) {
    return lpc_lex_line_fallback;
  }
  int *lineno = lpc_lex_buffer_lineno(yyscanner, idx);
  return lineno != nullptr ? *lineno : lpc_lex_line_fallback;
}

// The current physical line's text in the innermost real frame -- the
// same frame `current_line` reads, so Diagnostic line/column/snippet stay
// mutually consistent: while a splice is scanning, this is the INVOCATION
// line in the real file. Bounded by what's still resident in that buffer
// (bulk chunks for the base buffer, the whole file for an include).
std::string lpc_lex_current_source_line(void) {
  if (active_scanner == nullptr) {
    return "";
  }
  int idx = innermost_real_buffer_index(active_scanner);
  if (idx < 0) {
    return "";
  }
  const char *base;
  const char *limit;
  const char *pos;
  char held;
  if (!lpc_lex_buffer_extents(active_scanner, idx, &base, &limit, &pos, &held)) {
    return "";
  }
  // Treat a NUL as a line boundary alongside '\n': flex's yyinput() NULs
  // every byte it consumes ("preserve yytext" semantics), so the newline
  // that terminated a preceding directive line -- consumed via yyinput()
  // -- survives only as a NUL residue. (Raw reads leave the same residue
  // mid-line in rarer cases -- a macro's collected arguments -- where
  // this yields a left-truncated snippet; acceptable.)
  const char *start = pos;
  while (start > base && start[-1] != '\n' && start[-1] != '\0') {
    start--;
  }
  std::string content(start, static_cast<size_t>(pos - start));
  if (pos < limit) {
    // The byte AT the live scan position may be the held-out NUL; resolve
    // it BEFORE deciding whether the line continues -- if the held char
    // is the line's newline, the line ends exactly here (walking on and
    // substituting later would splice the next line into the snippet).
    char c0 = *pos;
    if (held != 0 && c0 == '\0') {
      c0 = held;
    }
    if (c0 != '\n' && c0 != '\0') {
      content += c0;
      for (const char *q = pos + 1; q < limit && *q != '\n' && *q != '\0'; ++q) {
        content += *q;
      }
    }
  }
  // Strip a trailing CR (CRLF sources) but preserve leading whitespace:
  // the caret column must align with the text as printed.
  if (!content.empty() && content.back() == '\r') {
    content.pop_back();
  }
  return content;
}

// Legacy " snippet + caret at scan position" block for prepare_logs()'s
// runtime path (apply.cc trace warnings). Reads the CURRENT buffer, like
// the old ring version read around outp.
std::string lpc_lex_error_context_block(void) {
  if (active_scanner == nullptr) {
    return "";
  }
  int count = lpc_lex_buffer_count(active_scanner);
  if (count <= 0) {
    return "";
  }
  const char *base;
  const char *limit;
  const char *pos;
  char held;
  if (!lpc_lex_buffer_extents(active_scanner, count - 1, &base, &limit, &pos, &held)) {
    return "";
  }
  const char *start = pos;
  while (start > base && start[-1] != '\n' && start[-1] != '\0') {
    start--;
  }
  const char *end = pos;
  while (end < limit && *end != '\n' && (*end != '\0' || end == pos)) {
    end++;
  }
  auto size = end - start;
  if (size <= 0) {
    return "";
  }
  bool truncated = false;
  if (size > 120) {
    size = 117;
    truncated = true;
  }
  std::string content(start, static_cast<size_t>(size));
  if (held != 0 && pos >= start && pos < start + size && content[pos - start] == '\0') {
    content[pos - start] = held;
  }
  if (truncated) content += "...";
  // Trim like the legacy ring version did (leading indentation dropped).
  size_t first = content.find_first_not_of(" \t\r");
  size_t last = content.find_last_not_of(" \t\r");
  content = (first == std::string::npos) ? "" : content.substr(first, last - first + 1);
  if (content.empty()) {
    return "";
  }
  std::string block = "  " + content + "\n";
  block += "  " + std::string(truncated ? content.size() : static_cast<size_t>(pos - start), ' ') +
           "^\n";
  return block;
}

std::vector<std::string> prepare_logs(const char *error_file, int line, const char *what, int flag,
                                      bool include_error_context) {
  std::vector<std::string> logs;
  logs.emplace_back(fmt::format(FMT_STRING("/{}:{}: {}: {}\n"), error_file, line,
                                flag ? "warning" : "error", what));

  if (include_error_context) {
    std::string block = lpc_lex_error_context_block();
    if (!block.empty()) {
      logs.emplace_back(std::move(block));
    }
  }

  return logs;
}




void push_function_context() {
  function_context_t *fc;
  parse_node_t *node;

  if (last_function_context == MAX_FUNCTION_DEPTH - 1) {
    lexerror("Function pointers nested too deep.");
    return;
  }
  fc = &function_context_stack[++last_function_context];
  fc->num_parameters = 0;
  fc->num_locals = 0;
  node = new_node_no_line();
  node->l.expr = node;
  node->r.expr = nullptr;
  node->kind = 0;
  fc->values_list = node;
  fc->bindable = 0;
  fc->parent = current_function_context;

  current_function_context = fc;
}

void pop_function_context() {
  current_function_context = current_function_context->parent;
  last_function_context--;
}

// Resolves the identifier lex.l's identifier rule just matched to a
// token: macro expansion (rescan-driven, below), reserved word, known
// defined name, or a fresh L_IDENTIFIER. (The old "(: name" lookahead
// duties moved into the grammar with 9.2.)
// ---------------------------------------------------------------------------
// Pushed-buffer bookkeeping. The kind stack below runs parallel to Flex's
// pushed-buffer stack (see lex.h's LpcPushedBufferKind): EXPANSION pops
// own the innermost live provenance frame ("during expansion of macro
// 'F' ..." diagnostics); INCLUDE pops restore current_file/current_line
// and close the per-file line accounting; PLAIN pops (probed-but-unused
// bytes, heredoc array synthesis, "(:" re-splices) need nothing.
// ---------------------------------------------------------------------------

namespace {
struct ExpansionFrame {
  std::string name;
  std::string def_file;
  int def_line;
  int invocation_line;    // real-file line of the macro USE
  int invocation_column;  // 1-based start column of the name token
  bool live;  // false once its buffer popped; lingers until a line boundary
};
std::vector<ExpansionFrame> expansion_frames;

// Parallel to the stack of pushed buffers: LpcPushedBufferKind values.
std::vector<char> pushed_kind_stack;
}  // namespace

static void pop_include_state(int ending_line);

int lpc_lex_pushed_depth(void) { return static_cast<int>(pushed_kind_stack.size()); }

int lpc_lex_buffer_kind_at(int i) {
  if (i < 0 || i >= static_cast<int>(pushed_kind_stack.size())) return -1;
  return pushed_kind_stack[static_cast<size_t>(i)];
}

int lpc_lex_top_buffer_kind(void) {
  return pushed_kind_stack.empty() ? -1 : pushed_kind_stack.back();
}

void lpc_lex_note_buffer_push(int kind) {
  pushed_kind_stack.push_back(static_cast<char>(kind));
}

void lpc_lex_note_buffer_pop(int ending_lineno) {
  if (pushed_kind_stack.empty()) return;
  char kind = pushed_kind_stack.back();
  pushed_kind_stack.pop_back();
  if (kind == LPC_BUF_EXPANSION) {
    // Mark the innermost live frame dead -- frames and expansion buffers
    // nest LIFO, so it is this buffer's frame.
    for (auto it = expansion_frames.rbegin(); it != expansion_frames.rend(); ++it) {
      if (it->live) {
        it->live = false;
        break;
      }
    }
  } else if (kind == LPC_BUF_INCLUDE) {
    pop_include_state(ending_lineno);
  }
}

// True when `name` is the guard of a LIVE expansion buffer: rescanning a
// macro's own name anywhere within its expansion (including nested
// expansions pushed on top) resolves it as a plain identifier -- the
// buffer-lifetime self-reference guard that supersedes the old
// per-occurrence blue paint. Dead (lingering, diagnostics-only) frames
// deliberately do NOT guard.
static bool lpc_lex_name_guarded(const std::string &name) {
  for (const auto &f : expansion_frames) {
    if (f.live && f.name == name) {
      return true;
    }
  }
  return false;
}

// Nested rescans stack one Flex buffer + one yylex() frame per level; a
// pathological chain (#define A0 A1 A1 / ...) must fail cleanly, not
// blow the C stack.
#define MAX_EXPANSION_NESTING 128

// Pushes a frame for an expansion buffer about to be pushed. Zero-length
// expansions push nothing (no buffer, no frame). The invocation position
// (the macro USE, clang's "expansion location") is the live real-frame
// line plus the name token's start column, snapshotted by YY_USER_ACTION
// when the identifier matched (argument collection in between doesn't
// disturb it -- raw reads run no user action).
static void push_expansion_frame(const std::string &name, const PpMacro &m,
                                 int invocation_column) {
  expansion_frames.push_back(
      ExpansionFrame{name, m.def_file, m.def_line, current_line, invocation_column, true});
}

// Dead frames deliberately LINGER instead of vanishing at their buffer's
// pop: a token's bytes are all consumed before its rule action (or the
// parser's action on its lookahead) runs, so an error on a splice's final
// token would otherwise fire with its frame already gone (found by
// Diagnostics.ExpansionChainNote: a one-token expansion body lost its
// note every time). Splices never contain newlines (folded at #define,
// argument newlines collapsed), so the newline ending the line is the
// natural hard boundary where dead frames are purged (see
// lpc_lex_yy_input) -- provenance granularity within the line, exactly
// like current_line itself.
static void purge_exhausted_expansions() {
  while (!expansion_frames.empty() && !expansion_frames.back().live) {
    expansion_frames.pop_back();
  }
}

std::vector<LpcExpansionSite> lpc_lex_expansion_chain(void) {
  std::vector<LpcExpansionSite> out;
  for (auto it = expansion_frames.rbegin(); it != expansion_frames.rend(); ++it) {
    out.push_back(LpcExpansionSite{it->name, it->def_file, it->def_line, it->invocation_line,
                                   it->invocation_column});
  }
  return out;
}

std::vector<std::pair<std::string, int>> lpc_lex_include_stack(void) {
  std::vector<std::pair<std::string, int>> out;
  for (auto it = inc_stack.rbegin(); it != inc_stack.rend(); ++it) {
    // The entry's line was saved AFTER the #include line's terminating
    // newline was consumed (see lpc_lex_handle_include's preconditions),
    // so the directive itself sits one line earlier.
    out.emplace_back(it->file != nullptr ? it->file : "?", it->line - 1);
  }
  return out;
}

// The bookkeeping half of an #include buffer's pop (the buffer itself is
// popped by lpc_lex_pop_pushed_buffer's yypop_buffer_state): restore the
// including file's identity and close this file's line accounting --
// verbatim the arithmetic of the legacy ring-based include pop, minus the
// ring/stream restoration that Flex's buffer stack now does for free.
static void pop_include_state(int ending_line) {
  if (inc_stack.empty()) return;
  IncState p = inc_stack.back();
  inc_stack.pop_back();

  save_file_info(current_file_id, ending_line - current_line_saved);
  current_line_saved = p.line - 1;
  /* add the lines from this file, and readjust to be relative
     to the file we're returning to */
  current_line_base += ending_line - current_line_saved;
  free_string(current_file);

  current_file = p.file;
  current_file_id = p.file_id;
  // No current_line restore: the parent buffer's native counter froze at
  // p.line when the include was pushed and resumes by itself.
}

int lpc_lex_resolve_identifier(union YYSTYPE *yylval_param, struct YYLTYPE *yylloc_param,
                               void *yyscanner) {
  compiler_context_t *yyextra = reinterpret_cast<compiler_context_t *>(yyget_extra(yyscanner));
  // Copy yytext up front, before ANY lpc_lex_getc(): a getc can pop the
  // splice buffer that yytext points into (freeing it), so every later
  // use of the identifier's spelling must go through this copy.
  const std::string text = yyget_text(yyscanner);
  if (current_session && !yyextra->suppress_expansion && !lpc_lex_name_guarded(text)) {
    // (A name guarded by a live expansion buffer resolves as a plain
    // identifier -- self-reference termination, C-preprocessor style.)
    auto it = current_session->macros.find(text);
    if (it != current_session->macros.end()) {
      const PpMacro& m = it->second;
      if (static_cast<int>(expansion_frames.size()) >= MAX_EXPANSION_NESTING) {
        lexerror("Macro expansion nested too deep");
      } else if (!m.is_function_like) {
        // __LINE__/__FILE__/__DIR__ expand from the live scan position;
        // any other object-like macro pushes its RAW body as a fresh Flex
        // buffer -- nested macro references are resolved when the rescan
        // reaches them, right here, one buffer per level (no textual
        // pre-expansion pass). The frame pushed alongside doubles as the
        // self-reference guard for the buffer's lifetime.
        //
        // A buffer push is safe here (an earlier attempt was reverted,
        // see plans/MASTER-PLAN.md 5.5): a bare reference to a
        // function-like macro can end a body with its "(args)" following
        // in the PARENT input (`#define APPLY F` + `APPLY(3)`), and the
        // argument collector reads through lpc_lex_getc(), which drains
        // the expansion buffer and pops through to the parent.
        std::string expanded;
        bool is_builtin = lpc_lex_builtin_macro(text, &expanded);
        if (!is_builtin) {
          expanded = m.body;
        }

        if (!expanded.empty()) {
          if (!is_builtin) {
            push_expansion_frame(text, m, yyextra->token_start_column + 1);
          }
          lpc_lex_push_string_buffer(expanded.c_str(), expanded.size(),
                                     /*is_expansion=*/!is_builtin, yyscanner);
        }
        return yylex(yylval_param, yylloc_param, yyscanner);
      } else {
        int saved_line = current_line;
        int saved_total = total_lines;
        // Characters are read THROUGH Flex (lpc_lex_getc: its buffer,
        // then YY_INPUT) rather than from raw `outp` -- no rewind/flush
        // choreography, no desync with the DFA's prefetch, and a splice
        // buffer ending mid-collection is popped through transparently.
        // Every probed byte is recorded for the no-parenthesis restore
        // below, which pushes the text back as a fresh splice buffer
        // rather than rewinding a pointer (a saved outp would dangle
        // across a refill_buffer() memmove -- a latent corruption in the
        // old raw-read version).
        std::string consumed_text;

        auto get_next_char = [&]() -> char {
          int gc = lpc_lex_getc(yyscanner);
          if (gc <= 0) {
            return '\0';
          }
          consumed_text += static_cast<char>(gc);
          return static_cast<char>(gc);
        };

        char c;
        while (true) {
          c = get_next_char();
          if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            if (c == '\n') {
              total_lines++;  // line counting itself is native (yyinput)
            }
            continue;
          }
          break;
        }

        if (c == '(') {
          std::vector<std::string> args;
          std::string arg;
          int depth = 0;
          char inq = 0;
          while (true) {
            char ch = get_next_char();
            if (ch == '\0') {
              lexerror("End of file in macro arguments");
              break;
            }
            if (ch == '\n') {
              total_lines++;  // line counting itself is native (yyinput)
            }
            if (inq) {
              if (ch == '\\') {
                arg += ch;
                char ch2 = get_next_char();
                if (ch2 == '\0') {
                  lexerror("End of file in macro arguments");
                  break;
                }
                if (ch2 == '\n') {
                  total_lines++;  // line counting itself is native (yyinput)
                }
                arg += ch2;
                continue;
              }
              arg += ch;
              if (ch == inq) inq = 0;
            } else if (ch == '"' || ch == '\'' || ch == '`') {
              inq = ch;
              arg += ch;
            } else if (ch == '(') {
              depth++;
              arg += ch;
            } else if (ch == ')') {
              if (depth == 0) {
                args.push_back(std::string(trim(arg)));
                break;
              }
              depth--;
              arg += ch;
            } else if (ch == ',' && depth == 0) {
              args.push_back(std::string(trim(arg)));
              arg.clear();
            } else {
              // A raw newline in an argument was already counted above --
              // but a copy of this text may be spliced back and rescanned,
              // where lex.l's \n rule would count it AGAIN (found as a
              // real __LINE__ drift by MultiLineMacroArgKeepsLineCount).
              // Collapse it to a space here: the count stays with the
              // consumption (right, because an UNUSED parameter's text
              // never reappears at all), and splices carry no newlines.
              // Quoted text (the inq path above) is kept verbatim -- a
              // string literal's bytes can't be altered; a raw newline
              // inside a quoted macro argument still double-counts, an
              // accepted pathological corner.
              arg += (ch == '\n') ? ' ' : ch;
            }
          }

          // C argument semantics: # and ## operands receive the RAW
          // argument spelling; every other parameter reference receives
          // the argument's complete macro expansion (pre-expanded here
          // textually -- the one sanctioned textual-expansion use left,
          // exactly C's "arguments are fully expanded first" step; it is
          // what lets SECOND(1, SECOND(2, 3))'s inner SECOND expand even
          // while the outer SECOND's buffer guards the name).
          std::vector<std::string> expanded_args;
          expanded_args.reserve(args.size());
          for (const auto &a : args) {
            expanded_args.push_back(lpc_lex_expand_string(a));
          }
          std::string expanded = substitute(m.body, m.params, args, &expanded_args);

          if (!expanded.empty()) {
            push_expansion_frame(text, m, yyextra->token_start_column + 1);
            lpc_lex_push_string_buffer(expanded.c_str(), expanded.size(),
                                       /*is_expansion=*/1, yyscanner);
          }
          return yylex(yylval_param, yylloc_param, yyscanner);
        } else {
          // No '(' follows: not an invocation. Put every probed byte back
          // (see consumed_text's comment) and let the identifier resolve
          // as a plain symbol; the pushed-back bytes are scanned next.
          current_line = saved_line;
          total_lines = saved_total;
          if (!consumed_text.empty()) {
            lpc_lex_push_string_buffer(consumed_text.c_str(), consumed_text.size(),
                                       /*is_expansion=*/0, yyscanner);
          }
        }
      }
    }
  }

  ident_hash_elem_t *ihe;
  if ((ihe = lookup_ident(text.c_str()))) {
    if (ihe->token & IHE_RESWORD) {
      yylval_param->number = ihe->sem_value;
      return ihe->token & TOKEN_MASK;
    }
    yylval_param->ihe = ihe;
    return L_DEFINED_NAME;
  }
  yylval_param->string = scratch_copy(text.c_str());
  return L_IDENTIFIER;
}

// Called from lex.l's SC_HEREDOC_TERM rules once the "@"/"@@" prefix and
// the terminator identifier have already been recognized natively (the
// terminator is supplied by the LPC source at compile time, so matching
// the *body* against it can't be a static Flex pattern). The body is read
// line-by-line THROUGH Flex via lpc_lex_getc() -- no raw `outp` access,
// no rewind/flush choreography at the call sites. Handles both forms:
// "@TERM ... TERM" (one string token, built directly) and
// "@@TERM ... TERM" (an array of line strings: splices `({ ... })` and
// splices `"l1", "l2", })` for normal rescanning -- Robocoder's "@@"
// block). On a recoverable error (bad UTF-8, oversized block), lexerror()
// just logs and returns, so those paths resume the top-level scanner via
// `return yylex()`.
// The shared tail of lex.l's two heredoc-start rules (newline-terminated
// and content-follows forms): validate the accumulated terminator and
// hand off to the body reader. On an empty terminator, reports and
// resumes the top-level scan.
int lpc_lex_start_heredoc(union YYSTYPE *yylval_param, struct YYLTYPE *yylloc_param,
                          void *yyscanner) {
  compiler_context_t *ctx = yyget_extra(yyscanner);
  if (ctx->heredoc_terminator.empty()) {
    lexerror("Illegal terminator");
    return yylex(yylval_param, yylloc_param, yyscanner);
  }
  return parseHeredoc(ctx->heredoc_terminator.c_str(), ctx->heredoc_is_array, yylval_param,
                      yylloc_param, yyscanner);
}

int parseHeredoc(const char *terminator, int is_array, union YYSTYPE *yylval_param,
                 struct YYLTYPE *yylloc_param, void *yyscanner) {
  const size_t termlen = strlen(terminator);
  // Legacy capacity: NUMCHUNKS chunks of MAXCHUNK bytes == DEFMAX total.
  const size_t max_block = DEFMAX;

  std::string text;                // text form: raw body, newlines included
  std::vector<std::string> lines;  // array form: raw body lines
  size_t total = 0;

  for (;;) {
    // One physical line; trailing CR stripped (CRLF input).
    std::string line;
    int c;
    while ((c = lpc_lex_getc(yyscanner)) > 0 && c != '\n') {
      line += static_cast<char>(c);
    }
    bool saw_newline = (c == '\n');
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }

    // Terminator line? Matches the legacy check: the line starts with the
    // terminator and the next character is not part of a longer
    // identifier (so "ENDING" does not close "END").
    if (line.size() >= termlen && line.compare(0, termlen, terminator) == 0 &&
        (line.size() == termlen ||
         (!uisalnum(static_cast<unsigned char>(line[termlen])) && line[termlen] != '_'))) {
      if (line.size() == termlen) {
        // Lone terminator: its line is fully consumed here (and its
        // newline already counted natively at the lpc_lex_getc read).
      } else {
        // Trailing content after the terminator: push it (and the line's
        // newline, which is then counted at rescan) back for normal
        // scanning.
        std::string rest = line.substr(termlen);
        if (saw_newline) {
          rest += '\n';
        }
        lpc_lex_push_string_buffer(rest.c_str(), rest.size(), 0, yyscanner);
      }
      break;
    }

    if (c <= 0) {
      lexerror(is_array ? "End of file in array block" : "End of file in text block");
      return YYEOF;
    }

    // Body-line newlines are counted natively at the lpc_lex_getc reads.
    total += line.size() + 1;
    if (total > max_block) {
      if (is_array) {
        lexerror("Array block exceeded maximum length");
        return YYerror;
      }
      lexerror("Text block exceeded maximum length");
      return yylex(yylval_param, yylloc_param, yyscanner);
    }

    if (is_array) {
      lines.push_back(std::move(line));
    } else {
      text += line;
      text += '\n';
    }
  }

  if (is_array) {
    // Splice the whole array literal -- `({ "l1", "l2", })` with '"' and
    // '\\' escaped -- for normal rescanning ('(' and '{' are ordinary
    // tokens the grammar pairs, since 9.1). Element separators
    // deliberately carry no newline: each body line was already counted
    // above, so the splice must not make the rescan count those lines
    // again.
    std::string splice = "({ ";
    splice.reserve(total + lines.size() * 4 + 6);
    for (const auto &l : lines) {
      splice += '"';
      for (char ch : l) {
        if (ch == '"' || ch == '\\') {
          splice += '\\';
        }
        splice += ch;
      }
      splice += "\", ";
    }
    splice += "})";
    if (splice.size() >= DEFMAX - 10) {
      lexerror("Array block exceeded maximum length");
      return YYerror;
    }
    lpc_lex_push_string_buffer(splice.c_str(), splice.size(), 0, yyscanner);
    return yylex(yylval_param, yylloc_param, yyscanner);
  }

  if (!u8_validate(text.c_str())) {
    lexerror("Bad UTF-8 string in string block");
    return yylex(yylval_param, yylloc_param, yyscanner);
  }
  char *res = scratch_large_alloc(static_cast<int>(text.size() + 1));
  memcpy(res, text.c_str(), text.size() + 1);
  yylval_param->string = res;
  return L_STRING;
}

// Shared "illegal character" fallback: reports (in DEBUG builds) and
// returns the same placeholder token the original inline `badlex:` label
// did. Called both from yylex_inner()'s remaining catch-all default case
// and from lex.l-triggered helpers whose own lookahead determined the
// input isn't well-formed (e.g. a '#' not at the start of a line).
int lpc_lex_badlex(unsigned char c, void *yyscanner) {
  // Reported UNCONDITIONALLY: the legacy version gated this behind
  // #ifdef DEBUG and silently substituted ' ' in release builds, hiding
  // real input corruption from users -- and making every diagnostic test
  // that provokes an illegal character pass only in Debug builds (found
  // when a local RelWithDebInfo run failed seven of them).
  char buff[100];

  if (isprint(c)) {
    sprintf(buff, "Illegal character '%c' (0x%02x)", static_cast<char>(c),
            static_cast<unsigned>(c));
  } else {
    sprintf(buff, "Illegal character 0x%02x", static_cast<unsigned>(c));
  }
  yyerror(yyscanner, buff);
  return ' ';
}

// Called from lex.l's <<EOF>> rule, only at genuine end of the top-level
// file (include buffers pop in the <<EOF>> rule itself before this is
// ever reached). Returns -1, the compile loop's end-of-tokens signal.
int parseMainEof(union YYSTYPE *yylval_param, void *yyscanner) {
  if (current_session && !current_session->conds.empty()) {
    yyerror("Missing #endif");
    // Recover the session: leaving the conditional stack non-empty would
    // make lpc_lex_emitting() false forever, silently skipping ALL input
    // of any later chunk fed through the same (e.g. REPL) session.
    current_session->conds.clear();
  }
  return -1;
}

void end_new_file() {
  // Freeze the final line value into the macro's fallback storage before
  // the active-scanner slot clears (post-compile readers -- fatal()'s
  // "during compilation of" note, test harnesses -- expect the legacy
  // global's last-value semantics).
  lpc_lex_line_fallback = current_line;
  // An aborted compile can leave include metadata stacked (the matching
  // Flex buffers are torn down by lpc_lex_reset / yylex_destroy); unwind
  // it so current_file ends up back at the main file's identity.
  while (!inc_stack.empty()) {
    free_string(current_file);
    current_file = inc_stack.back().file;
    current_file_id = inc_stack.back().file_id;
    inc_stack.pop_back();
  }
  if (main_filename) {
    free_string(const_cast<char *>(main_filename));
    main_filename = nullptr;
  }
  active_scanner = nullptr;
}

void start_new_file(std::unique_ptr<LexStream> stream, void *yyscanner,
                     std::shared_ptr<LexerSession> session) {
  if (!main_filename && current_file) {
    main_filename = make_shared_string(current_file);
  }

  if (!session) {
    session = LexerSession::make_session();
  }
  current_session = std::move(session);
  current_stream = std::move(stream);

  // Fresh diagnostics per compile / per REPL chunk -- same boundary on
  // which num_parse_error is effectively reset by the callers. Stale
  // expansion frames from an aborted compile must not haunt the next.
  compiler_diags.clear();
  expansion_frames.clear();
  compiler_current_load_reason = std::move(compiler_next_load_reason);
  compiler_next_load_reason.clear();
  // One-shot context a previous ABORTED compile may have left queued
  // (its consuming report never happened) -- must not haunt this one.
  compiler_pending_notes.clear();
  compiler_pending_fixits.clear();
  rule_clear_operand_ranges();
  compiler_directive_start_line = 0;

  // lpc_lex_reset pops any pushed buffers an aborted compile left stacked
  // (it walks lpc_lex_pushed_depth(), so the kind stack must still be
  // intact here); afterwards clear the bookkeeping defensively in case
  // the stacks ever disagreed. Any include metadata still stacked at this
  // point (end_new_file normally unwound it) only needs its shared
  // strings released -- current_file already holds the NEW compile's
  // identity and must not be touched.
  lpc_lex_reset(yyscanner);
  pushed_kind_stack.clear();
  for (auto &is : inc_stack) {
    free_string(const_cast<char *>(is.file));
  }
  inc_stack.clear();
  last_function_context = -1;
  current_function_context = nullptr;
  main_last_char = '\n';
  active_scanner = yyscanner;
  pragmas = DEFAULT_PRAGMAS;
  current_line = 1;
  current_line_base = 0;
  current_line_saved = 0;

  // Push the configured global include file exactly like a real
  // `#include "..."` / `#include <...>` appearing before the file's first
  // line (the config value already carries its quoting/angle delimiters):
  // its content becomes a pushed buffer scanned before the main file's
  // first byte (which Flex pulls through YY_INPUT on demand).
  const char *glf = CONFIG_STR(__GLOBAL_INCLUDE_FILE__);
  if (glf != nullptr && strlen(glf) != 0) {
    lpc_lex_handle_include(glf, yyscanner);
  }
}

// ---------------------------------------------------------------------------
// LexTokenStream
// ---------------------------------------------------------------------------

LexTokenStream::LexTokenStream() : ctx_(std::make_unique<compiler_context_t>()) {
  yylex_init_extra(ctx_.get(), &scanner_);
}

LexTokenStream::~LexTokenStream() {
  if (scanner_) {
    yylex_destroy(scanner_);
  }
}

void LexTokenStream::load(std::unique_ptr<LexStream> stream, std::shared_ptr<LexerSession> session) {
  start_new_file(std::move(stream), scanner_, std::move(session));
}

int LexTokenStream::next(union YYSTYPE *yylval_param, struct YYLTYPE *yylloc_param) {
  YYLTYPE local;
  return yylex(yylval_param, yylloc_param != nullptr ? yylloc_param : &local, scanner_);
}

const char *query_instr_name(int instr) {
  const char *name;
  static char num_buf[20];

  // The param is clearly wrong, however to be safe, we just return something here.
  if (instr < 0 || instr >= (sizeof(instrs) / sizeof(instrs[0]))) {
    sprintf(num_buf, "op_invalid");
    return num_buf;
  }

  name = instrs[instr].name;

  if (name) {
    if (name[0] == '_') {
      name++;
    }
    return name;
  } else {
    sprintf(num_buf, "op%d", instr);
    return num_buf;
  }
}



const char *main_file_name() {
  return main_filename ? main_filename : current_file;
}

/* identifier hash table stuff, size must be an even power of two */
#define IDENT_HASH_SIZE 1024
#define IdentHash(s) (whashstr((s)) & (IDENT_HASH_SIZE - 1))

/* The identifier table is hashed for speed.  The hash chains are circular
 * linked lists, so that we can rotate them, since identifier lookup is
 * rather irregular (i.e. we're likely to be asked about the same one
 * quite a number of times in a row).  This isn't as fast as moving entries
 * to the front but is done this way for two reasons:
 *
 * 1. this allows us to keep permanent identifiers consecutive and clean
 *    up faster
 * 2. it would only be faster in cases where two identifiers with the same
 *    hash value are used often within close proximity in the source.
 *    This should be rare, esp since the hash table is fairly sparse.
 *
 * ident_hash_table[hash] points to our current position (last lookup)
 * ident_hash_head[hash] points to the first permanent identifier
 * ident_hash_tail[hash] points to the last one
 * ident_dirty_list is a linked list of identifiers that need to be cleaned
 * when we're done; this happens if you define a global or function with
 * the same name as an efun or sefun.
 */

#define CHECK_ELEM(x, y, z)                               \
  if (!strcmp((x)->name, (y))) {                          \
    if (((x)->token & IHE_RESWORD) || ((x)->sem_value)) { \
      z                                                   \
    } else                                                \
      return 0;                                           \
  }

ident_hash_elem_t *lookup_ident(const char *name) {
  int h = IdentHash(name);
  ident_hash_elem_t *hptr, *hptr2;

  if ((hptr = ident_hash_table[h])) {
    CHECK_ELEM(hptr, name, return hptr;);
    hptr2 = hptr->next;
    while (hptr2 != hptr) {
      CHECK_ELEM(hptr2, name, ident_hash_table[h] = hptr2; return hptr2;);
      hptr2 = hptr2->next;
    }
  }
  return nullptr;
}

ident_hash_elem_t *find_or_add_perm_ident(const char *name) {
  int h = IdentHash(name);
  ident_hash_elem_t *hptr, *hptr2;

  if ((hptr = ident_hash_table[h])) {
    if (!strcmp(hptr->name, name)) {
      return hptr;
    }
    hptr2 = hptr->next;
    while (hptr2 != hptr) {
      if (!strcmp(hptr2->name, name)) {
        return hptr2;
      }
      hptr2 = hptr2->next;
    }
    hptr = reinterpret_cast<ident_hash_elem_t *>(
        DMALLOC(sizeof(ident_hash_elem_t), TAG_PERM_IDENT, "find_or_add_perm_ident:1"));
    hptr->next = ident_hash_head[h]->next;
    ident_hash_head[h]->next = hptr;
    if (ident_hash_head[h] == ident_hash_tail[h]) {
      ident_hash_tail[h] = hptr;
    }
  } else {
    hptr = (ident_hash_table[h] = reinterpret_cast<ident_hash_elem_t *>(
                DMALLOC(sizeof(ident_hash_elem_t), TAG_PERM_IDENT, "find_or_add_perm_ident:2")));
    ident_hash_head[h] = hptr;
    ident_hash_tail[h] = hptr;
    hptr->next = hptr;
  }
  hptr->name = name;
  hptr->token = 0;
  hptr->sem_value = 0;
  hptr->dn.simul_num = -1;
  hptr->dn.local_num = -1;
  hptr->dn.global_num = -1;
  hptr->dn.efun_num = -1;
  hptr->dn.function_num = -1;
  hptr->dn.class_num = -1;
  return hptr;
}

typedef struct lname_linked_buf_s {
  struct lname_linked_buf_s *next;
  char block[4096];
} lname_linked_buf_t;

lname_linked_buf_t *lnamebuf = nullptr;

int lb_index = 4096;

static char *alloc_local_name(const char *name) {
  int len = strlen(name) + 1;
  char *res;

  if (lb_index + len > 4096) {
    lname_linked_buf_t *new_buf;
    new_buf = reinterpret_cast<lname_linked_buf_t *>(
        DMALLOC(sizeof(lname_linked_buf_t), TAG_COMPILER, "alloc_local_name"));
    new_buf->next = lnamebuf;
    lnamebuf = new_buf;
    lb_index = 0;
  }
  res = &(lnamebuf->block[lb_index]);
  strcpy(res, name);
  lb_index += len;
  return res;
}

int num_free = 0;

typedef struct ident_hash_elem_list_s {
  struct ident_hash_elem_list_s *next;
  ident_hash_elem_t items[128];
} ident_hash_elem_list_t;

ident_hash_elem_list_t *ihe_list = nullptr;

#if 0
void dump_ihe(ident_hash_elem_t *ihe, int noisy)
{
  int sv = 0;
  if (ihe->token & IHE_RESWORD) {
    if (noisy) { printf("%s ", ihe->name); }
  } else {
    if (noisy) { printf("%s[", ihe->name); }
    if (ihe->dn.function_num != -1) {
      if (noisy) { printf("f"); }
      sv++;
    }
    if (ihe->dn.simul_num != -1) {
      if (noisy) { printf("s"); }
      sv++;
    }
    if (ihe->dn.efun_num != -1) {
      if (noisy) { printf("e"); }
      sv++;
    }
    if (ihe->dn.local_num != -1) {
      if (noisy) { printf("l"); }
      sv++;
    }
    if (ihe->dn.global_num != -1) {
      if (noisy) { printf("g"); }
      sv++;
    }
    if (ihe->sem_value != sv) {
      if (noisy) {
        printf("(*%i*)", ihe->sem_value - sv);
      } else { dump_ihe(ihe, 1); }
    }
    if (noisy) { printf("] "); }
  }
}

void debug_dump_ident_hash_table(int noisy)
{
  int zeros = 0;
  int i;
  ident_hash_elem_t *ihe, *ihe2;

  if (noisy) { printf("\n\nIdentifier Hash Table:\n"); }
  for (i = 0; i < IDENT_HASH_SIZE; i++) {
    ihe = ident_hash_table[i];
    if (!ihe) {
      zeros++;
    } else {
      if (zeros && noisy) { printf("<%i zeros>\n", zeros); }
      zeros = 0;
      dump_ihe(ihe, noisy);
      ihe2 = ihe->next;
      while (ihe2 != ihe) {
        dump_ihe(ihe2, noisy);
        ihe2 = ihe2->next;
      }
      if (noisy) { printf("\n"); }
    }
  }
  if (zeros && noisy) { printf("<%i zeros>\n", zeros); }
}
#endif

void free_unused_identifiers() {
  ident_hash_elem_list_t *ihel, *next;
  lname_linked_buf_t *lnb, *lnbn;
  int i;

  /* clean up dirty idents */
  while (ident_dirty_list) {
    if (ident_dirty_list->dn.function_num != -1) {
      ident_dirty_list->dn.function_num = -1;
      ident_dirty_list->sem_value--;
    }
    if (ident_dirty_list->dn.global_num != -1) {
      ident_dirty_list->dn.global_num = -1;
      ident_dirty_list->sem_value--;
    }
    if (ident_dirty_list->dn.class_num != -1) {
      ident_dirty_list->dn.class_num = -1;
      ident_dirty_list->sem_value--;
    }
    ident_dirty_list = ident_dirty_list->next_dirty;
  }

  for (i = 0; i < IDENT_HASH_SIZE; i++) {
    if ((ident_hash_table[i] = ident_hash_head[i])) {
      ident_hash_tail[i]->next = ident_hash_head[i];
    }
  }

  ihel = ihe_list;
  while (ihel) {
    next = ihel->next;
    FREE(ihel);
    ihel = next;
  }
  ihe_list = nullptr;
  num_free = 0;

  lnb = lnamebuf;
  while (lnb) {
    lnbn = lnb->next;
    FREE(lnb);
    lnb = lnbn;
  }
  lnamebuf = nullptr;
  lb_index = 4096;
#if 0
  debug_dump_ident_hash_table(0);
#endif
}

static ident_hash_elem_t *quick_alloc_ident_entry() {
  if (num_free) {
    num_free--;
    return &(ihe_list->items[num_free]);
  } else {
    ident_hash_elem_list_t *ihel;
    ihel = reinterpret_cast<ident_hash_elem_list_t *>(
        DMALLOC(sizeof(ident_hash_elem_list_t), TAG_COMPILER, "quick_alloc_ident_entry"));
    ihel->next = ihe_list;
    ihe_list = ihel;
    num_free = 127;
    return &(ihe_list->items[127]);
  }
}

ident_hash_elem_t *find_or_add_ident(const char *name, int flags) {
  int h = IdentHash(name);
  ident_hash_elem_t *hptr, *hptr2;

  if ((hptr = ident_hash_table[h])) {
    if (!strcmp(hptr->name, name)) {
      if ((hptr->token & IHE_PERMANENT) && (flags & FOA_GLOBAL_SCOPE) &&
          (hptr->dn.function_num == -1) && (hptr->dn.global_num == -1) &&
          (hptr->dn.class_num == -1)) {
        hptr->next_dirty = ident_dirty_list;
        ident_dirty_list = hptr;
      }
      return hptr;
    }
    hptr2 = hptr->next;
    while (hptr2 != hptr) {
      if (!strcmp(hptr2->name, name)) {
        if ((hptr2->token & IHE_PERMANENT) && (flags & FOA_GLOBAL_SCOPE) &&
            (hptr2->dn.function_num == -1) && (hptr2->dn.global_num == -1) &&
            (hptr2->dn.class_num == -1)) {
          hptr2->next_dirty = ident_dirty_list;
          ident_dirty_list = hptr2;
        }
        ident_hash_table[h] = hptr2; /* rotate */
        return hptr2;
      }
      hptr2 = hptr2->next;
    }
  }

  hptr = quick_alloc_ident_entry();
  if (!(hptr2 = ident_hash_tail[h]) && !(hptr2 = ident_hash_table[h])) {
    ident_hash_table[h] = hptr->next = hptr;
  } else {
    hptr->next = hptr2->next;
    hptr2->next = hptr;
  }

  if (flags & FOA_NEEDS_MALLOC) {
    hptr->name = alloc_local_name(name);
  } else {
    hptr->name = name;
  }
  hptr->token = 0;
  hptr->sem_value = 0;
  hptr->dn.simul_num = -1;
  hptr->dn.local_num = -1;
  hptr->dn.global_num = -1;
  hptr->dn.efun_num = -1;
  hptr->dn.function_num = -1;
  hptr->dn.class_num = -1;
  return hptr;
}

static void add_keyword_t(const char *name, keyword_t *entry) {
  int h = IdentHash(name);

  if (ident_hash_table[h]) {
    entry->next = ident_hash_head[h]->next;
    ident_hash_head[h]->next = reinterpret_cast<ident_hash_elem_t *>(entry);
    if (ident_hash_head[h] == ident_hash_tail[h]) {
      ident_hash_tail[h] = reinterpret_cast<ident_hash_elem_t *>(entry);
    }
  } else {
    ident_hash_head[h] = reinterpret_cast<ident_hash_elem_t *>(entry);
    ident_hash_tail[h] = reinterpret_cast<ident_hash_elem_t *>(entry);
    ident_hash_table[h] = reinterpret_cast<ident_hash_elem_t *>(entry);
    entry->next = reinterpret_cast<ident_hash_elem_t *>(entry);
  }
  entry->token |= IHE_RESWORD;
}

void init_identifiers() {
  unsigned int i;
  ident_hash_elem_t *ihe;



  /* allocate all three tables together */
  ident_hash_table = reinterpret_cast<ident_hash_elem_t **>(DCALLOC(
      IDENT_HASH_SIZE * 3, sizeof(ident_hash_elem_t *), TAG_IDENT_TABLE, "init_identifiers"));
  ident_hash_head = &ident_hash_table[IDENT_HASH_SIZE];
  ident_hash_tail = &ident_hash_table[2 * IDENT_HASH_SIZE];

  /* clean all three tables */
  for (i = 0; i < IDENT_HASH_SIZE * 3; i++) {
    ident_hash_table[i] = nullptr;
  }
  /* add the reserved words */
  for (i = 0; i < NELEM(reswords); i++) {
    add_keyword_t(reswords[i].word, &reswords[i]);
  }
  /* add the efuns */
  for (i = 0; i < size_of_predefs; i++) {
    if (predefs[i].word[0] == '_') {
      predefs[i].word++;
      if (strcmp(predefs[i].word, "call_other") == 0) {
        arrow_efun = i;
      }
      if (strcmp(predefs[i].word, "evaluate") == 0) {
        evaluate_efun = i;
      }
      if (strcmp(predefs[i].word, "this_object") == 0) {
        this_efun = i;
      }
      if (strcmp(predefs[i].word, "to_int") == 0) {
        to_int_efun = i;
      }
      if (strcmp(predefs[i].word, "to_float") == 0) {
        to_float_efun = i;
      }
      if (strcmp(predefs[i].word, "new") == 0) {
        new_efun = i;
      }
      continue;
    }

    ihe = find_or_add_perm_ident(predefs[i].word);
    ihe->token |= IHE_EFUN;
    ihe->sem_value++;
    ihe->dn.efun_num = i;
  }
}

// Bridge called by Flex's YY_INPUT macro: bulk-reads the MAIN file's
// stream straight into Flex's base buffer. (The historic one-byte trickle
// existed only to bound Flex's prefetch for the raw-outp readers' rewind
// arithmetic -- all gone: splices/includes are pushed in-memory buffers
// that never touch this path, and every raw reader goes through
// lpc_lex_getc().) Preserves the ring's guaranteed-trailing-newline
// invariant: a file whose last byte isn't '\n' gets one appended before
// EOF is reported.
extern "C" int lpc_lex_yy_input(char *buf, int max_size, void * /*yyscanner*/) {
  if (max_size <= 0 || !current_stream) return 0;
  int n = current_stream->read(buf, max_size);
  if (n > 0) {
    main_last_char = buf[n - 1];
    return n;
  }
  if (main_last_char != '\n') {
    main_last_char = '\n';
    buf[0] = '\n';
    return 1;
  }
  return 0;
}

// ---------------------------------------------------------------------------
// Predefines registry & include-path resolution
// (originally lexer_utils.cc, standalone-preprocessor support helpers)
// ---------------------------------------------------------------------------

namespace {

// Predefines registry
std::unordered_map<std::string, PredefMacro> predefines;

std::vector<std::string> inc_list;
std::vector<std::string> inc_path;

std::string merge(std::string_view name) {
    if (name.empty()) return "";

    std::string dest;
    if (name[0] == '/') {
        // Absolute path from mudlib root
        dest = "";
    } else {
        // Start with current_file's directory
        dest = current_file ? current_file : "";
        size_t last_slash = dest.rfind('/');
        if (last_slash != std::string::npos) {
            dest = dest.substr(0, last_slash);
        } else {
            dest = "";
        }
    }

    // Now process components of name
    size_t pos = 0;
    while (pos < name.size()) {
        // Skip any leading slashes in the remaining part of name
        while (pos < name.size() && name[pos] == '/') {
            pos++;
            dest.clear(); // An absolute component clears the destination
        }
        if (pos >= name.size()) break;

        size_t next_slash = name.find('/', pos);
        std::string_view component = (next_slash == std::string_view::npos)
            ? name.substr(pos)
            : name.substr(pos, next_slash - pos);

        if (component == "..") {
            if (!dest.empty()) {
                size_t last_slash = dest.rfind('/');
                if (last_slash != std::string::npos) {
                    dest = dest.substr(0, last_slash);
                } else {
                    dest.clear();
                }
            }
        } else if (component == "." || component.empty()) {
            // Do nothing
        } else {
            if (!dest.empty()) {
                dest += '/';
            }
            dest += component;
        }

        if (next_slash == std::string_view::npos) break;
        pos = next_slash + 1;
    }
    return dest;
}

} // namespace

// ---------------------------------------------------------------------------
// Predefines Implementation
// ---------------------------------------------------------------------------

void add_predefine(std::string_view name, int nargs, std::string_view exps) {
    PredefMacro m;
    m.is_function_like = (nargs >= 0);
    m.nargs = nargs;
    m.body = exps;
    predefines[std::string(name)] = std::move(m);
}

void add_quoted_predefine(std::string_view def, std::string_view val) {
    std::string quoted;
    quoted.reserve(val.size() + 2);
    quoted += '"';
    quoted += val;
    quoted += '"';
    add_predefine(def, -1, quoted);
}

const std::unordered_map<std::string, PredefMacro>& get_predefines() {
    return predefines;
}

void add_predefines() {
    int i;
    lpc_predef_t *tmpf;

    add_predefine("MUDOS", -1, "");
    add_predefine("FLUFFOS", -1, "");

#define _STR_HELPER(x) #x
#define _STR(x) _STR_HELPER(x)

#ifdef PACKAGE_DB
    add_predefine("__PACKAGE_DB__", -1, "");
#endif
#ifdef USE_MYSQL
    add_predefine("__USE_MYSQL__", -1, _STR(USE_MYSQL));
#endif
#ifdef USE_POSTGRES
    add_predefine("__USE_POSTGRES__", -1, _STR(USE_POSTGRES));
#endif
#ifdef USE_SQLITE3
    add_predefine("__USE_SQLITE3__", -1, _STR(USE_SQLITE3));
#endif
#ifdef DEFAULT_DB
    add_predefine("__DEFAULT_DB__", -1, _STR(DEFAULT_DB));
#endif

    add_predefine("__GET_CHAR_IS_BUFFERED__", -1, "");
    add_predefine("__DSLIB__", -1, "");
#ifdef PACKAGE_DWLIB
    add_predefine("__DWLIB__", -1, "");
#endif

    // Adding version
    add_quoted_predefine("__VERSION__", PROJECT_VERSION);

    add_predefine("__PORT__", -1, std::to_string(external_port[0].port).c_str());
    for (i = 0; i < static_cast<int>(sizeof(option_defs) / sizeof(const char *)); i += 2) {
        add_predefine(option_defs[i], -1, option_defs[i + 1]);
    }
    add_quoted_predefine("__ARCH__", ARCH);
    add_quoted_predefine("__COMPILER__", COMPILER);
    add_quoted_predefine("__CXXFLAGS__", CXXFLAGS);

    /* Backwards Compat */
    add_quoted_predefine("MUD_NAME", CONFIG_STR(__MUD_NAME__));
#ifdef F_ED
    add_predefine("HAS_ED", -1, "");
#endif
#ifdef F_PRINTF
    add_predefine("HAS_PRINTF", -1, "");
#endif
#if (defined(RUSAGE) || defined(GET_PROCESS_STATS) || defined(TIMES))
    add_predefine("HAS_RUSAGE", -1, "");
#endif
    add_predefine("HAS_DEBUG_LEVEL", -1, "");
#ifdef DEBUG
    add_predefine("__DEBUG__", -1, "");
#endif
    for (tmpf = lpc_predefs; tmpf; tmpf = tmpf->next) {
        std::string_view flag = tmpf->flag;
        size_t eq = flag.find('=');
        if (eq == std::string_view::npos) {
            add_predefine(flag, -1, "");
        } else {
            add_predefine(flag.substr(0, eq), -1, flag.substr(eq + 1));
        }
    }
    add_predefine("SIZEOFINT", -1, std::to_string(sizeof(LPC_INT)));
    add_predefine("MAX_INT", -1, std::to_string(LPC_INT_MAX));
    add_predefine("MIN_INT", -1, std::to_string(LPC_INT_MIN));

    {
        // %f of DBL_MAX produces 309+ characters; use a dynamically sized buffer.
        int needed = std::snprintf(nullptr, 0, "%" LPC_FLOAT_FMTSTR_P, LPC_FLOAT_MAX);
        std::string max_float(needed + 1, '\0');
        std::snprintf(max_float.data(), max_float.size(), "%" LPC_FLOAT_FMTSTR_P, LPC_FLOAT_MAX);
        max_float.resize(needed);
        add_predefine("MAX_FLOAT", -1, max_float);

        needed = std::snprintf(nullptr, 0, "%" LPC_FLOAT_FMTSTR_P, LPC_FLOAT_MIN);
        std::string min_float(needed + 1, '\0');
        std::snprintf(min_float.data(), min_float.size(), "%" LPC_FLOAT_FMTSTR_P, LPC_FLOAT_MIN);
        min_float.resize(needed);
        add_predefine("MIN_FLOAT", -1, min_float);
    }

    // Following compile time configs are now always true
    add_predefine("__CACHE_STATS__", -1, "");
    add_predefine("__STRING_STATS__", -1, "");
    add_predefine("__CLASS_STATS__", -1, "");
    add_predefine("__ARRAY_STATS__", -1, "");
    add_predefine("__CALLOUT_HANDLES__", -1, "");
    add_predefine("__ARGUMENTS_IN_TRACEBACK__", -1, "");
    add_predefine("__LOCALS_IN_TRACEBACK__", -1, "");
    add_predefine("__DEBUG_MACRO__", -1, "");

    // Following compile time configs has been changed into runtime configs.
    if (CONFIG_INT(__RC_SANE_EXPLODE_STRING__)) {
        add_predefine("__SANE_EXPLODE_STRING__", -1, "");
    }
    if (CONFIG_INT(__RC_REVERSIBLE_EXPLODE_STRING__)) {
        add_predefine("__REVERSIBLE_EXPLODE_STRING__", -1, "");
    }
    if (CONFIG_INT(__RC_SANE_SORTING__)) {
        add_predefine("__SANE_SORTING__", -1, "");
    }
    if (CONFIG_INT(__RC_CALL_OTHER_TYPE_CHECK__)) {
        add_predefine("__CALL_OTHER_TYPE_CHECK__", -1, "");
    }
    if (CONFIG_INT(__RC_CALL_OTHER_WARN__)) {
        add_predefine("__CALL_OTHER_WARN__", -1, "");
    }
    if (CONFIG_INT(__RC_MUDLIB_ERROR_HANDLER__)) {
        add_predefine("__MUDLIB_ERROR_HANDLER__", -1, "");
    }
    if (CONFIG_INT(__RC_NO_RESETS__)) {
        add_predefine("__NO_RESETS__", -1, "");
    }
    if (CONFIG_INT(__RC_LAZY_RESETS__)) {
        add_predefine("__LAZY_RESETS__", -1, "");
    }
    if (CONFIG_INT(__RC_RANDOMIZED_RESETS__)) {
        add_predefine("__RANDOMIZED_RESETS__", -1, "");
    }
    if (CONFIG_INT(__RC_THIS_PLAYER_IN_CALL_OUT__)) {
        add_predefine("__THIS_PLAYER_IN_CALL_OUT__", -1, "");
    }
    if (CONFIG_INT(__RC_TRACE__)) {
        add_predefine("__TRACE__", -1, "");
    }
    if (CONFIG_INT(__RC_TRACE_CODE__)) {
        add_predefine("__TRACE_CODE__", -1, "");
    }
    if (CONFIG_INT(__RC_INTERACTIVE_CATCH_TELL__)) {
        add_predefine("__INTERACTIVE_CATCH_TELL__", -1, "");
    }
    if (CONFIG_INT(__RC_RECEIVE_SNOOP__)) {
        add_predefine("__RECEIVE_SNOOP__", -1, "");
    }
    if (CONFIG_INT(__RC_SNOOP_SHADOWED__)) {
        add_predefine("__SNOOP_SHADOWED__", -1, "");
    }
    if (CONFIG_INT(__RC_REVERSE_DEFER__)) {
        add_predefine("__REVERSE_DEFER__", -1, "");
    }
    if (CONFIG_INT(__RC_HAS_CONSOLE__)) {
        add_predefine("__HAS_CONSOLE__", -1, "");
    }
    if (CONFIG_INT(__RC_NONINTERACTIVE_STDERR_WRITE__)) {
        add_predefine("__NONINTERACTIVE_STDERR_WRITE__", -1, "");
    }
    if (CONFIG_INT(__RC_TRAP_CRASHES__)) {
        add_predefine("__TRAP_CRASHES__", -1, "");
    }
    if (CONFIG_INT(__RC_OLD_TYPE_BEHAVIOR__)) {
        add_predefine("__OLD_TYPE_BEHAVIOR__", -1, "");
    }
    if (CONFIG_INT(__RC_OLD_RANGE_BEHAVIOR__)) {
        add_predefine("__OLD_RANGE_BEHAVIOR__", -1, "");
    }
    if (CONFIG_INT(__RC_WARN_OLD_RANGE_BEHAVIOR__)) {
        add_predefine("__WARN_OLD_RANGE_BEHAVIOR__", -1, "");
    }
    if (CONFIG_INT(__RC_SUPPRESS_ARGUMENT_WARNINGS__)) {
        add_predefine("__SUPPRESS_ARGUMENT_WARNINGS__", -1, "");
    }
}



void print_all_predefines() {
    std::vector<std::string> results;
    for (const auto& pair : predefines) {
        results.push_back(pair.first);
    }
    std::sort(results.begin(), results.end());
    for (const auto& name : results) {
        debug_message("#define %s %s\n", name.c_str(), predefines[name].body.c_str());
    }
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_all_defines() {
    // No-op: include paths are stored in std::vector<std::string> and managed automatically
}
#endif

void set_inc_list(const char *list) {
    inc_list.clear();
    std::istringstream ss(list);
    for (std::string path; std::getline(ss, path, ':'); ) {
        if (path.empty()) continue;
        if (path[0] == '/') path.erase(path.begin());
        if (!legal_path(path.c_str())) {
            debug_message("'include dirs' must give paths without any '..'\n");
            exit(-1);
        }
        inc_list.push_back(std::move(path));
    }
    inc_path = inc_list;
}

void init_include_path() {
    // No VM context: keep the config-file include path (inc_list) as-is --
    // there is no master object to ask, and no eval stack to push onto.
    if (!compiler_vm_context) {
        inc_path = inc_list;
        return;
    }

    push_malloced_string(add_slash(current_file));
    svalue_t *ret = safe_apply_master_ob(APPLY_GET_INCLUDE_PATH, 1);

    if (!ret || ret == reinterpret_cast<svalue_t *>(-1)) {
        return;
    }
    if (ret->type != T_ARRAY) {
        debug_message("'master::get_include_path' must return 'string *'\n");
        return;
    }
    array_t *arr = ret->u.arr;
    if (!arr->size) {
        debug_message("got empty include path for 'master::get_include_path(%s)'\n", current_file);
        return;
    }

    std::vector<std::string> path;
    path.reserve(arr->size);
    for (int i = 0; i < arr->size; i++) {
        if (arr->item[i].type != T_STRING) {
            debug_message("'master::get_include_path(%s)' must return 'string *'\n", current_file);
            return;
        }
        const char *elem = arr->item[i].u.string;
        if (!strcmp(elem, ":DEFAULT:")) {
            path.insert(path.end(), inc_list.begin(), inc_list.end());
            continue;
        }
        const char *check = (elem[0] == '/') ? &elem[1] : elem;
        if (!legal_path(check)) {
            debug_message(
                "'master::get_include_path(%s)' returns invalid value '%s', must give paths without "
                "any '..'\n",
                current_file, elem);
            return;
        }
        path.emplace_back(elem);
    }
    inc_path = std::move(path);
}

void deinit_include_path() {
    inc_path = inc_list;
}

std::pair<int, std::string> inc_open(std::string_view name, bool check_local) {
    std::string buf;

    if (check_local) {
        buf = merge(name);
        const char *tmp = check_valid_path(buf.c_str(), master_ob, "include", 0);
        if (tmp) {
            int fd = open(tmp, O_RDONLY);
            if (fd != -1) {
#ifdef _WIN32
                _setmode(fd, _O_BINARY);
#endif
                return {fd, buf};
            }
        }
    }

    for (size_t p = name.find('.'); p != std::string_view::npos; p = name.find('.', p + 1)) {
        if (p + 1 < name.size() && name[p + 1] == '.') {
            return {-1, ""};
        }
    }
    for (const auto &path : inc_path) {
        buf.clear();
        buf.append(path);
        buf += '/';
        buf.append(name);
        const char *tmp = check_valid_path(buf.c_str(), master_ob, "include", 0);
        if (tmp) {
            int fd = open(tmp, O_RDONLY);
            if (fd != -1) {
#ifdef _WIN32
                _setmode(fd, _O_BINARY);
#endif
                return {fd, buf};
            }
        }
    }
    return {-1, ""};
}

bool lpc_lex_handle_include(std::string_view rest, void *yyscanner) {
  std::string name_expr(trim(rest));
  if (name_expr.empty()) {
    lexerror("Bad #include directive");
    return false;
  }
  if (name_expr[0] != '"' && name_expr[0] != '<') {
    name_expr = lpc_lex_expand_string(name_expr);
    name_expr = std::string(trim(name_expr));
  }
  if (name_expr.size() >= 2 && (name_expr[0] == '"' || name_expr[0] == '<')) {
    char delim = name_expr[0] == '"' ? '"' : '>';
    std::string filename(name_expr.substr(1, name_expr.size() - 2));

    if (inc_stack.size() >= MAX_INCLUDE_DEPTH) {
      lexerror("#include nested too deeply");
      return false;
    }

    auto [fd, resolved] = inc_open(filename, delim == '"');
    if (fd != -1) {
      // Slurp the whole file and push it as a Flex buffer: the parent's
      // resume position (base ring or an outer include's buffer) is kept
      // by Flex's own buffer stack, so only the file-identity/line
      // bookkeeping remains here -- and it must mirror the legacy push
      // exactly for the per-file line-number arithmetic to close against
      // pop_include_state().
      //
      // Precondition established by the caller (lex.l's directive rule):
      // the directive's terminating newline has been consumed and
      // counted, so current_line is already the line AFTER the #include.
      std::string content;
      char rdbuf[8192];
      ssize_t n;
      while ((n = read(fd, rdbuf, sizeof(rdbuf))) > 0) {
        content.append(rdbuf, static_cast<size_t>(n));
      }
      close(fd);
      // The ring guaranteed every file ended in a newline (appending one
      // when missing); keep that invariant for the buffer form so a
      // directive or line comment on the include's last line still
      // terminates.
      if (content.empty() || content.back() != '\n') {
        content += '\n';
      }

      // The parent buffer's native line counter freezes at resume_line
      // (the line AFTER the directive) for the whole include and resumes
      // by itself at the pop -- only the per-file accounting is manual.
      int resume_line = current_line;
      inc_stack.push_back(IncState{resume_line, current_file, current_file_id});

      int directive_line = resume_line - 1;
      save_file_info(current_file_id, directive_line - current_line_saved);
      current_line_base += directive_line;
      current_line_saved = 0;
      current_file = make_shared_string(resolved.c_str());
      current_file_id = add_program_file(resolved.c_str(), 0);
      lpc_lex_push_string_buffer(content.c_str(), content.size(), LPC_BUF_INCLUDE, yyscanner);
      return true;
    } else {
      lexerror(("Cannot #include " + filename).c_str());
      return false;
    }
  } else {
    lexerror("Bad #include directive");
    return false;
  }
}
