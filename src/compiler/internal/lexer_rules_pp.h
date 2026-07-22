#ifndef LEXER_RULES_PP_H
#define LEXER_RULES_PP_H

// lexer_rules_pp.h / lexer_rules_pp.cc — the preprocessing half of the
// lexer's rule logic (companion to lexer_rules.{h,cc}, same relationship
// to lexer.l). This is where the old standalone preprocessor's genuinely
// unique logic landed when preprocessing was merged into the lexer's own
// single scan: the macro table and
// #define/#undef parsing, the #if/#elif expression evaluator
// (lpc_lex_eval_if_expr), textual macro expansion with recursion guards,
// and the directive entry point lexer.l's one anchored '#'-line rule calls
// (lpc_lex_on_directive). What did NOT move here: the #include
// input-stack push and macro expansion's ring-buffer splicing live in
// lexer_utils.cc, next to the buffer machinery they manipulate.

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "compiler/internal/compiler.h"
#include "compiler/internal/scratchpad.h"

// PpMacro / CondState / LpcMacroTable live in compiler.h (they are
// CompileState members).

// Helpers
ScratchString normalize_filename(const char* filename);
std::string_view trim(std::string_view s);
ScratchString strip_directive_comments(std::string_view s);
ScratchString stringize(std::string_view s);
ScratchVector<ScratchString> collect_args(std::string_view text, size_t& i);
// Parameter substitution (with # stringize marking and ## paste in its
// second pass). `args` are the RAW argument spellings -- what # and ##
// operands receive (C semantics). `expanded_args`, when non-null,
// supplies the macro-expanded form used for every OTHER parameter
// reference (C's argument pre-expansion); null = raw everywhere (the
// textual directive-side path, which pre-expands before calling).
ScratchString substitute(std::string_view body, const std::vector<std::string>& params,
                         const ScratchVector<ScratchString>& args,
                         const ScratchVector<ScratchString>* expanded_args = nullptr);

// #if/#elif integer expression evaluator over TOKENS: pushes `expr` as a
// LPC_BUF_IF_EXPR buffer, pulls tokens through the scanner itself
// (numbers via the real literal decoders; macro references expand
// through the ordinary rescan machinery into a flat sequence, preserving
// C precedence; defined()/efun_defined() operands pulled with expansion
// suppressed), then evaluates with C-style precedence and ternary. On
// any parse problem (unpaired bracket, division/modulo by zero, trailing
// '?' without ':', leftover content) calls lexerror() directly and
// returns 0.
int64_t lpc_lex_eval_if_expr(std::string_view expr, void* yyscanner);

// There is NO session object. The preprocessor state lives directly in
// g_compile: `macros` holds USER #defines only (predefines are immutable
// -- redefining or #undef'ing one is an error -- so they never enter the
// per-compile table; lookups fall back to a shared table derived once
// from the boot predefine registry, rebuilt only when the registry's
// version changes). start_new_file() clears conds and, unless the caller
// asks to keep macros (REPL chunk persistence), clears the user table --
// capacity is retained, so a steady-state compile allocates NOTHING for
// preprocessor setup (the old per-compile session heap-copied the whole
// predefine registry).

// Unified macro lookup: user table first, then the predefine table.
// Returned pointer is valid until the next #define/#undef.
const PpMacro* pp_find_macro(std::string_view name);
// True when `name` names a predefine (or __FILE__/__LINE__/__DIR__).
bool pp_is_predefined(std::string_view name);

// True when every level of the #if/#ifdef conditional stack is emitting
// (i.e. the scanner is not inside a false branch). Vacuously true with no
// session (unit tests driving yylex() without one).
bool lpc_lex_emitting();

// The single entry point for lexer.l's one captured-'#'-line rule, covering
// BOTH scan modes (in_skip_mode = YY_START == SC_COND_SKIP). Takes the
// whole matched logical line (leading whitespace, '#', backslash
// continuations and all; terminating newline NOT included -- the lexer.l
// rule consumes and counts that itself, see its comment), folds and
// parses it exactly once (name + rest + embedded continuation newlines,
// which are counted into current_line/total_lines here), then either
// dispatches it (emit mode, or a dead-branch-ending #elif/#else/#endif at
// skip depth 0) or merely tracks conditional nesting (skip mode). The
// return value tells the rule which start-condition transition to make;
// all other directive effects (macro table, conditional stack, #include
// push, #pragma/#line/#error/...) have already been applied on return.
enum class LpcDirectiveAction {
  kNone,       // stay in the current scan mode
  kEnterSkip,  // a condition turned false: BEGIN(SC_COND_SKIP)
  kExitSkip,   // the dead branch ended: BEGIN(INITIAL)
};
// pulled_lines: physical newlines lpc_lex_complete_directive() consumed
// beyond the terminating one (0 when the rule's capture was already the
// whole logical line) -- backed out of the first-line attribution below.
LpcDirectiveAction lpc_lex_on_directive(const char* text, int len, void* yyscanner,
                                        bool in_skip_mode, int pulled_lines = 0);

// The directive rule's capture stops at the first physical newline (plus
// backslash continuations), which cannot span a /* comment that closes on
// a LATER line -- the comment's remaining lines used to be tokenized as
// code (#1236). Called by the rule action BEFORE
// lpc_lex_consume_directive_newline(): scans the captured text
// (quote-aware, same rules as strip_directive_comments()) and, when it
// ends inside an open /* comment, keeps pulling raw bytes through
// lpc_lex_getc() until the comment closes and the logical line really
// ends. Each comment is folded to a single space (a comment is
// whitespace, so text after the close still belongs to the directive, and
// another /* there may open again). Returns false -- out/pulled_lines
// untouched, nothing consumed -- when the capture has no open comment
// (the common case costs one scan of the captured text) or the top buffer
// is a splice/if-expr buffer (no newlines to pull; same guard as
// lpc_lex_consume_directive_newline()). Returns true when it pulled the
// tail: *out is the completed logical line, *pulled_lines the newline
// count for lpc_lex_on_directive(), and the terminating newline has been
// consumed AND counted -- the rule action must NOT consume it again.
bool lpc_lex_complete_directive(const char* text, int len, void* yyscanner, ScratchString* out,
                                int* pulled_lines);

// #include implementation: resolves `rest` (macro-expanding it first when
// unquoted), records the including file's identity on the include
// metadata stack, and pushes the opened file's whole content as a Flex
// buffer (LPC_BUF_INCLUDE) -- the pop side restores the metadata when
// that buffer runs dry. Defined in lexer_utils.cc, next to the include
// stack it manipulates. Preconditions documented at the definition. Also
// used by start_new_file() for the configured __GLOBAL_INCLUDE_FILE__.
bool lpc_lex_handle_include(std::string_view rest, void* yyscanner);

// Textual macro expansion (object-like and function-like, with `guard`
// carrying the names currently being expanded for self-reference
// termination). ONLY two textual consumers remain -- function-like
// ARGUMENT pre-expansion (C's "arguments are fully expanded first" step)
// and #include's unquoted-filename form; both consume the result as
// text, never rescanned. Ordinary macro expansion is rescan-driven
// (lpc_lex_resolve_identifier pushes the RAW substituted body as a Flex
// buffer) and #if/#elif expressions are evaluated over TOKENS
// (lpc_lex_eval_if_expr below).
ScratchString lpc_lex_expand_string(std::string_view text, ScratchVector<ScratchString> guard = {});

// __LINE__/__FILE__/__DIR__ expand from the compiler's LIVE position
// (current_line/current_file) -- one line counter, one file name is the
// point of the merged single-scan design. Returns false when `name` isn't
// one of them. Filenames are normalized to the mudlib-absolute "/path"
// form (the old preprocessor normalized at construction; the compiler's
// current_file lacks the leading '/').
bool lpc_lex_builtin_macro(std::string_view name, ScratchString* out);

#endif  // LEXER_RULES_PP_H
