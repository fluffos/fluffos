#ifndef LEXER_RULES_H
#define LEXER_RULES_H

#include <string>

#include "vm/internal/base/number.h"  // for LPC_INT

// lexer_rules.h / lexer_rules.cc — the substantive logic behind lex.l's
// rule actions, pulled out into ordinary functions so lex.l itself stays a
// thin table of patterns + calls, mirroring how grammar.y's rules mostly
// just call a rule_*() function defined in grammar_rules*.cc.
//
// What can and can't live here: Flex's generated scanner hides its state
// (yyguts_t, BEGIN()/YY_START, yyless()) as macros/types private to the
// generated lex.autogen.cc translation unit -- they are not usable from a
// separately-compiled .cc file. So a rule that changes start condition
// (BEGIN(...)) or pushes back matched input (yyless()) must keep that part
// of its action inline in lex.l; only functions that need just the matched
// text (yytext/yyleng, passed explicitly), yylval, and/or the reentrant
// extra-data (compiler_context_t, reachable via the public yyget_extra())
// can move here. In practice that covers nearly everything actually
// *computed* by the lexer (escape decoding, number parsing, Unicode
// conversion); what's left inline in lex.l is almost entirely start-
// condition transitions and buffer-rewind bookkeeping.

union YYSTYPE;

// ---------------------------------------------------------------------------
// Numeric literals
// ---------------------------------------------------------------------------

// Each parses `text` (exactly what the corresponding lex.l pattern matched,
// underscores and all) into yylval and returns the token to return.
int lpc_lex_number_hex(union YYSTYPE *yylval_param, const char *text, int len);
int lpc_lex_number_bin(union YYSTYPE *yylval_param, const char *text, int len);
int lpc_lex_number_real(union YYSTYPE *yylval_param, const char *text, int len);
int lpc_lex_number_dec(union YYSTYPE *yylval_param, const char *text, int len);

// ---------------------------------------------------------------------------
// String / template literal body: escape decoding, appended to
// yyextra->str_accum (reached via yyget_extra(yyscanner)).
//
// String and template literals share every escape form the grammar
// supports (templates just don't get \u/\U, and use a different word --
// "template literal" vs "string" -- in their error messages), so one set of
// functions serves both SC_STRING_BODY and SC_TEMPLATE_BODY, parameterized
// by `is_template`.
// ---------------------------------------------------------------------------

// THE simple-escape table, shared by all three literal contexts (string,
// template, char): decodes the single character after the backslash.
// Which escapes are RECOGNIZED in each context (vs. falling to that
// context's unknown-escape rule with its warning) is decided by each
// rule's character class in lex.l -- this is only the one shared mapping
// those rules decode through, so '\n' means byte 10 everywhere without
// three copies of the table. Returns -1 for anything not in the table
// (callers' patterns make that unreachable).
int lpc_lex_simple_escape(char c);

void lpc_lex_append_octal_escape(void *yyscanner, const char *text, bool is_template);
void lpc_lex_append_hex_escape(void *yyscanner, const char *text, bool is_template);
void lpc_lex_append_bad_octal_escape(void *yyscanner, bool is_template);
void lpc_lex_append_bad_hex_escape(void *yyscanner, bool is_template);
void lpc_lex_append_unicode_pair_escape(void *yyscanner, const char *text);
void lpc_lex_append_unicode_escape(void *yyscanner, const char *text);
void lpc_lex_append_long_unicode_escape(void *yyscanner, const char *text, int len);
void lpc_lex_append_unknown_escape(void *yyscanner, const char *text, bool is_template);

// The body of lex.l's STR_CHECK_OVERFLOW() macro (which stays a macro only
// because it must `return` out of whichever rule invoked it): checks the
// accumulated literal against the MAXLINE cap and, on overflow, reports,
// finalizes what accumulated so far into yylval, and undoes a template
// MIDDLE/TAIL fragment's nesting increment (that level of ${...}
// interpolation is being abandoned instead of closing normally -- leaving
// the increment leaks it for the rest of this already-failing compile,
// corrupting brace-depth tracking for any '{'/'}' the recovery scan sees
// afterward; a HEAD fragment never incremented yet, which
// template_is_continuation distinguishes). Returns 0 to keep scanning,
// else the token (L_STRING or YYerror) to return after BEGIN(INITIAL).
int lpc_lex_accum_overflow(void *yyscanner, union YYSTYPE *yylval_param, bool in_template);

// SC_TEMPLATE_BODY's "${" / closing-backtick rules: validates the just-
// accumulated str_accum fragment, allocates it onto the scratchpad into
// yylval, and (for "${") tracks template_nesting/template_brace_depth or
// (for the closing backtick) decides between L_STRING (no interpolation)
// and L_TEMPLATE_TAIL. Returns the token to return; caller still does
// BEGIN(INITIAL) itself (start-condition changes can't move out of lex.l).
int lpc_lex_template_head_or_middle(void *yyscanner, union YYSTYPE *yylval_param);
int lpc_lex_template_tail_or_string(void *yyscanner, union YYSTYPE *yylval_param);

// ---------------------------------------------------------------------------
// Character literal body: decodes directly into yylval.number (no
// accumulator -- a char literal's body is exactly one escape/byte).
// ---------------------------------------------------------------------------

LPC_INT lpc_lex_char_octal_escape(const char *text);
LPC_INT lpc_lex_char_hex_escape(const char *text);
LPC_INT lpc_lex_char_bad_hex_escape();
LPC_INT lpc_lex_char_unknown_escape(const char *text);

// The shared "Illegal character constant" recovery tail (three lex.l rules
// end a broken char literal identically): reports the error, zeroes
// yylval.number, returns L_NUMBER. Caller still does BEGIN/yyless itself.
int lpc_lex_char_error(union YYSTYPE *yylval_param);

// ---------------------------------------------------------------------------
// Misc
// ---------------------------------------------------------------------------

// Strip '_' digit-group separators (lex.l's numeric patterns only allow '_'
// directly between two digits, so this is always safe).
std::string lpc_strip_underscores(const char *text, int len);

// current_line++/total_lines++ for each '\n' in the matched text -- used
// by the "(" WS* "{"/"["/":" compound-open rules and SC_FUNC_OPEN's
// whitespace run, where a matched span can contain newlines that must
// still count. Counts BOTH counters, like every other newline path (the
// legacy scanner did too; an earlier version of this helper skipped
// total_lines, undercounting the compiled-lines/s stat).
void lpc_lex_count_newlines(const char *text, int len);

// ---------------------------------------------------------------------------
// $N / $ function-pointer parameter tokens
// ---------------------------------------------------------------------------

// Sentinel meaning: the token was too long to be valid; the caller must
// retry via `return yylex(yylval_param, yyscanner)` itself. A recursive
// yylex() call is kept visible at each of its call sites in lex.l (matching
// every other such call site there) rather than hidden inside this
// function, so this can't just do the retry itself.
inline constexpr int kLpcLexFunctionParamRetry = -2;

// Handles both "$" and "$N" (text/len covers whichever matched). Returns
// the token to return, or kLpcLexFunctionParamRetry.
int lpc_lex_function_param(union YYSTYPE *yylval_param, const char *text, int len);

#endif
