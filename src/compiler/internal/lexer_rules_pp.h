#ifndef LEXER_RULES_PP_H
#define LEXER_RULES_PP_H

// lexer_rules_pp.h / lexer_rules_pp.cc — the preprocessing half of the
// lexer's rule logic (companion to lexer_rules.{h,cc}, same relationship
// to lex.l). This is where the old standalone preprocessor's genuinely
// unique logic landed when preprocessing was merged into the lexer's own
// single scan (plans/unified-push-lexer.md): the macro table and
// #define/#undef parsing, the #if/#elif expression evaluator
// (IfExprParser), textual macro expansion with recursion guards, and the
// directive dispatcher lex.l's one anchored '#'-line rule calls into.
// What did NOT move here: the #include input-stack push and macro
// expansion's ring-buffer splicing live in lexer_utils.cc, next to the
// buffer machinery they manipulate.

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

// PpMacro — one macro entry
struct PpMacro {
    bool is_function_like = false;
    bool is_predefined = false;
    std::vector<std::string> params;  // parameter names (function-like only)
    std::string body;                 // replacement body
};

// The #define table: name -> macro. A plain map -- every operation used on
// it is stock unordered_map surface.
using LpcMacroTable = std::unordered_map<std::string, PpMacro>;

// Helpers
std::string normalize_filename(const char* filename);
std::string_view trim(std::string_view s);
std::string strip_directive_comments(std::string_view s);
std::string stringize(std::string_view s);
std::vector<std::string> collect_args(std::string_view text, size_t& i);
std::string substitute(std::string_view body,
                       const std::vector<std::string>& params,
                       const std::vector<std::string>& args);

// #if/#elif integer expression evaluator (C-style operators, ternary,
// char-literal atoms, defined()/efun_defined() already resolved to 1/0 by
// lpc_lex_expand_string() before this ever runs). On any parse problem
// (unpaired bracket, division/modulo by zero, trailing '?' without ':',
// or leftover unparsed content after what looked like a complete
// expression) calls lexerror() directly -- matching every other
// lexer_rules*.cc helper's convention -- and returns 0.
long lpc_lex_eval_if_expr(std::string_view expr);

// One level of the #if/#ifdef conditional stack: whether this level's
// current branch emits, and whether ANY branch of it has been true yet
// (so a later #elif/#else knows not to fire).
struct CondState {
    bool emitting;
    bool had_true;
};

struct LexerSession {
    LpcMacroTable macros;
    std::vector<CondState> conds;

    void add_builtin_macros();

    static std::shared_ptr<LexerSession> make_session() {
        auto session = std::make_shared<LexerSession>();
        session->add_builtin_macros();
        return session;
    }
};

extern std::shared_ptr<LexerSession> current_session;

// True when every level of the #if/#ifdef conditional stack is emitting
// (i.e. the scanner is not inside a false branch). Vacuously true with no
// session (unit tests driving yylex() without one).
bool lpc_lex_emitting();

// The single entry point for lex.l's one captured-'#'-line rule, covering
// BOTH scan modes (in_skip_mode = YY_START == SC_COND_SKIP). Takes the
// whole matched logical line (leading whitespace, '#', backslash
// continuations and all; terminating newline NOT included -- the lex.l
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
LpcDirectiveAction lpc_lex_on_directive(const char* text, int len, void* yyscanner,
                                        bool in_skip_mode);

// #include implementation: resolves `rest` (macro-expanding it first when
// unquoted), pushes the current input source onto the include stack, and
// switches scanning to the opened file -- ending with the refill_buffer()
// that loads its first chunk. Defined in lexer_utils.cc, next to the
// include stack and ring buffer it manipulates. Preconditions documented
// at the definition. Also used by start_new_file() for the configured
// __GLOBAL_INCLUDE_FILE__.
bool lpc_lex_handle_include(std::string_view rest);

// Textual macro expansion (object-like and function-like, with `guard`
// carrying the names currently being expanded for self-reference
// termination). in_if_expr additionally enables defined()/efun_defined()
// evaluation for #if/#elif expressions.
//
// guard_hits: when non-null, each guarded self-reference left literal in
// text that will reach the RESULT directly gets counted (name -> count).
// The splice-and-rescan path (lpc_lex_resolve_identifier) passes the
// scanner's pending_plain map here so the rescan knows exactly which
// literal occurrences must stay plain identifiers -- see lex.h's
// pending_plain comment. #if/#include callers leave it null: their result
// text is consumed directly (expression evaluator / filename), never
// rescanned by the lexer, so no occurrence needs marking. Argument
// pre-expansion inside the walk also passes null internally -- its output
// goes through substitute() and is then RE-walked, and only the re-walk's
// emissions reach the result (counting both would double-count).
std::string lpc_lex_expand_string(std::string_view text, std::vector<std::string> guard = {},
                                  bool in_if_expr = false,
                                  std::unordered_map<std::string, int>* guard_hits = nullptr);

// __LINE__/__FILE__/__DIR__ expand from the compiler's LIVE position
// (current_line/current_file) -- one line counter, one file name is the
// point of the merged single-scan design. Returns false when `name` isn't
// one of them. Filenames are normalized to the mudlib-absolute "/path"
// form (the old preprocessor normalized at construction; the compiler's
// current_file lacks the leading '/').
bool lpc_lex_builtin_macro(std::string_view name, std::string* out);

#endif // LEXER_RULES_PP_H
