#ifndef LEXER_RULES_PP_H
#define LEXER_RULES_PP_H

// lexer_rules_pp.h / lexer_rules_pp.cc — the preprocessing half of the
// lexer's rule logic (companion to lexer_rules.{h,cc}, same relationship
// to lex.l). This is where the old standalone preprocessor's genuinely
// unique logic landed when preprocessing was merged into the lexer's own
// single scan: the macro table and
// #define/#undef parsing, the #if/#elif expression evaluator
// (lpc_lex_eval_if_expr), textual macro expansion with recursion guards,
// and the directive entry point lex.l's one anchored '#'-line rule calls
// (lpc_lex_on_directive). What did NOT move here: the #include
// input-stack push and macro expansion's ring-buffer splicing live in
// lexer_utils.cc, next to the buffer machinery they manipulate.

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
    // Definition site, for diagnostics ("during expansion of macro 'F'
    // (defined at file:line)", "previous definition was at ..."). Empty
    // file for predefines/builtins (no source location).
    std::string def_file;
    int def_line = 0;
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
// Parameter substitution (with # stringize marking and ## paste in its
// second pass). `args` are the RAW argument spellings -- what # and ##
// operands receive (C semantics). `expanded_args`, when non-null,
// supplies the macro-expanded form used for every OTHER parameter
// reference (C's argument pre-expansion); null = raw everywhere (the
// textual directive-side path, which pre-expands before calling).
std::string substitute(std::string_view body,
                       const std::vector<std::string>& params,
                       const std::vector<std::string>& args,
                       const std::vector<std::string>* expanded_args = nullptr);

// #if/#elif integer expression evaluator over TOKENS: pushes `expr` as a
// LPC_BUF_IF_EXPR buffer, pulls tokens through the scanner itself
// (numbers via the real literal decoders; macro references expand
// through the ordinary rescan machinery into a flat sequence, preserving
// C precedence; defined()/efun_defined() operands pulled with expansion
// suppressed), then evaluates with C-style precedence and ternary. On
// any parse problem (unpaired bracket, division/modulo by zero, trailing
// '?' without ':', leftover content) calls lexerror() directly and
// returns 0.
long lpc_lex_eval_if_expr(std::string_view expr, void* yyscanner);

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
// unquoted), records the including file's identity on the include
// metadata stack, and pushes the opened file's whole content as a Flex
// buffer (LPC_BUF_INCLUDE) -- the pop side restores the metadata when
// that buffer runs dry. Defined in lexer_utils.cc, next to the include
// stack it manipulates. Preconditions documented at the definition. Also
// used by start_new_file() for the configured __GLOBAL_INCLUDE_FILE__.
bool lpc_lex_handle_include(std::string_view rest, void *yyscanner);

// Textual macro expansion (object-like and function-like, with `guard`
// carrying the names currently being expanded for self-reference
// termination). ONLY two textual consumers remain -- function-like
// ARGUMENT pre-expansion (C's "arguments are fully expanded first" step)
// and #include's unquoted-filename form; both consume the result as
// text, never rescanned. Ordinary macro expansion is rescan-driven
// (lpc_lex_resolve_identifier pushes the RAW substituted body as a Flex
// buffer) and #if/#elif expressions are evaluated over TOKENS
// (lpc_lex_eval_if_expr below).
std::string lpc_lex_expand_string(std::string_view text, std::vector<std::string> guard = {});

// __LINE__/__FILE__/__DIR__ expand from the compiler's LIVE position
// (current_line/current_file) -- one line counter, one file name is the
// point of the merged single-scan design. Returns false when `name` isn't
// one of them. Filenames are normalized to the mudlib-absolute "/path"
// form (the old preprocessor normalized at construction; the compiler's
// current_file lacks the leading '/').
bool lpc_lex_builtin_macro(std::string_view name, std::string* out);

#endif // LEXER_RULES_PP_H
