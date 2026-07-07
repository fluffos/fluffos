%{
/*
 * lexer.l — Flex scanner for the LPC lexer.
 *
 * This is the driver's ONE scan over LPC source: preprocessing is part of
 * lexing, not a separate pass. The main
 * file's content IS the base buffer (lpc_lex_set_source); macro
 * splices and #include contents are in-memory buffers pushed on Flex's
 * buffer stack. Every byte is read exactly once, one token's worth of
 * work at a time, one yylex() pull per parser token.
 *
 * Whitespace, comments, operators/punctuation, numeric literals,
 * identifiers, '(' compound tokens (array/mapping/function-pointer open),
 * function-pointer $N params, char literals, and string literals are all
 * matched natively by Flex's DFA (several via exclusive start conditions
 * -- SC_CHAR_BODY/SC_CHAR_CLOSE, SC_BLOCK_COMMENT, SC_STRING_BODY,
 * SC_TEMPLATE_BODY).
 *
 * Preprocessor directives are one anchored rule capturing the whole '#'
 * line, dispatched to lexer_rules_pp.cc (macro table, #if conditional
 * stack + the SC_COND_SKIP dead-branch mode, #include as a pushed buffer
 * holding the file's whole content -- see lexer_utils.cc's
 * lpc_lex_handle_include -- #pragma/#line applied in place).
 * Macro EXPANSION happens at identifier-resolution time
 * (lpc_lex_resolve_identifier in lexer_utils.cc): the expanded body is
 * pushed as a fresh Flex buffer (lpc_lex_push_string_buffer, defined in
 * this file's trailer) and rescanned; the buffer pops back to the parent
 * at its <<EOF>>. Guarded self-references left literal in the expansion
 * are counted per name into compiler_context_t::pending_plain at
 * expansion time ("blue paint" per occurrence, C-preprocessor style); the
 * rescan consumes one count per occurrence and resolves it as a plain
 * identifier. Expansion provenance for diagnostics is one frame per live
 * expansion buffer (see lexer_utils.cc). Because string/template/char
 * bodies are scanned by their own start conditions and never reach
 * identifier resolution, "no macro expansion inside strings" falls out
 * for free.
 *
 * Only heredocs and main-file EOF still trigger explicit Flex rules that
 * call into standalone parse*() helpers in lexer_utils.cc -- a heredoc's
 * closing terminator is supplied at compile time by the LPC source itself
 * (e.g. "@END ... END"), not known until then, so it can't be expressed
 * as a static Flex pattern. Those helpers (and the macro-argument
 * collector) read through Flex itself via lpc_lex_getc() -- a thin
 * wrapper over the generated yyinput() -- so they drain Flex's own buffer
 * before touching fresh input, and no rewind/flush choreography is needed
 * around them. Only genuinely invalid
 * input reaches the catch-all `.` rule at the bottom.
 *
 * Include order: grammar_rules.h MUST come before grammar.autogen.h
 * because grammar.autogen.h references decl_t / func_block_t from it.
 *
 * Buffer management is entirely Flex-native: the base buffer bulk-reads
 * the main file as the base in-memory buffer; splices (macro expansions, pushbacks
 * of probed bytes) and #include contents are pushed as in-memory buffers
 * on the buffer STACK, each popped at its own <<EOF>> (or drained through
 * lpc_lex_getc()); the parent buffer's position is preserved by Flex's
 * own bookkeeping, so no rewind/flush choreography exists anywhere.
 */
#include "base/std.h"

#include <cstdlib>
#include <string>

#include "compiler/internal/compiler.h"
#include "compiler/internal/lexer.h"
#include "compiler/internal/grammar_rules.h"
#include "compiler/internal/grammar.autogen.h"
#include "compiler/internal/scratchpad.h"
#include "compiler/internal/lexer_rules.h"
#include "compiler/internal/lexer_rules_pp.h"
#include "efuns.autogen.h"

// %option array sizes yytext as char[YYLMAX]. The generated default (8K)
// was unreachable while the old ring buffer capped physical lines at
// MAXLINE (4K); with that cap gone, give every token the same 64K
// headroom the ring gave a whole compile unit, and route the overflow
// into a graceful compile abort instead of flex's stock exit(2) -- see
// lpc_lex_fatal (compiler.cc). (Bounding the run patterns with {1,4096}
// repeats was tried and reverted: flex expands bounded repeats by
// duplicating DFA states, ballooning the generated scanner to over a
// million lines.) The #undef silences flex's own default definition
// (8192, emitted earlier in the generated file) -- a redefine warning
// now that CI regenerates the scanner on every platform.
#ifdef YYLMAX
#undef YYLMAX
#endif
#define YYLMAX 65536
#define YY_FATAL_ERROR(msg) lpc_lex_fatal(msg)

// Column tracking (Phase 8.1): %option yylineno does NOT maintain
// yycolumn -- convention is to do it in YY_USER_ACTION. The matched
// token's START column is snapshotted into the scanner context for
// diagnostics (clang's caret points at the construct's start), then the
// running column advances past the match -- recomputed from the last
// newline for matches that contain one. Like yylineno, yycolumn is
// per-buffer storage (yy_bs_column), so expansion/include pushes isolate
// and restore columns automatically. Tabs count as one column; rendering
// decides display width. EOF pseudo-actions bypass YY_RULE_SETUP, so this
// never runs with stale yyleng.
#define YY_USER_ACTION                                        \
  yyextra->token_start_column = yycolumn;                     \
  /* Write through THIS frame's own parameter, never the yylloc  \
   * macro (yyg->yylloc_r): a nested yylex() -- the #if evaluator \
   * pulls -- repoints yylloc_r at its own shorter-lived location \
   * and the suspended outer frame would write through the stale  \
   * pointer (ASan stack-use-after-return). */                    \
  yylloc_param->first_line = yylloc_param->last_line = current_line; \
  yylloc_param->first_column = yycolumn + 1;                  \
  {                                                           \
    int uac_i_;                                               \
    for (uac_i_ = yyleng - 1;                                 \
         uac_i_ >= 0 && yytext[uac_i_] != '\n'; --uac_i_) {   \
    }                                                         \
    if (uac_i_ >= 0) {                                        \
      yycolumn = yyleng - uac_i_ - 1;                         \
    } else {                                                  \
      yycolumn += yyleng;                                     \
    }                                                         \
  }                                                           \
  yylloc_param->last_column = yycolumn;

// yyless() adjusts yylineno for returned newlines but NOT yycolumn; every
// yyless() in the rules goes through this wrapper, which rewinds the
// column to just past the kept prefix (all call sites keep a newline-free
// prefix -- "(" or nothing). The returned text is then re-matched and
// re-counted by YY_USER_ACTION as usual.
#define LPC_YYLESS(n)                                   \
  do {                                                  \
    yycolumn = yyextra->token_start_column + (n);       \
    yyless(n);                                          \
  } while (0)

// All template/string states moved to struct compiler_context_t in lexer.h

// Roughly matches the old MAXLINE-based "String too long" cap. Called
// after every append to str_accum below, not just the bulk-growth ones
// (ordinary-run/raw-newline): the original's cap counted every outer
// iteration -- one escape sequence or one raw character each -- so a
// string built entirely out of short escapes (e.g. thousands of "\n")
// must hit the same cap a long run of ordinary text would. The check +
// recovery logic lives in lpc_lex_accum_overflow() (lexer_rules.cc, see
// its header comment); only this skeleton stays a macro, because it must
// `return` out of whichever Flex rule invokes it and BEGIN() is private
// to the generated scanner.
#define STR_CHECK_OVERFLOW()                                              \
  do {                                                                    \
    int lpc_str_tok_ = lpc_lex_accum_overflow(yyscanner, yylval_param,    \
                                              YY_START == SC_TEMPLATE_BODY); \
    if (lpc_str_tok_) {                                                   \
      BEGIN(INITIAL);                                                     \
      return lpc_str_tok_;                                                \
    }                                                                     \
  } while (0)

// YY_INPUT: constant EOF. EVERY input -- the main file included -- is an
// in-memory buffer installed via yy_scan_bytes (lpc_lex_set_source for
// the base, lpc_lex_push_string_buffer for splices/includes), and those
// buffers never request a refill (yy_fill_buffer == 0). The only buffer
// that could ever call here is the defensive placeholder base in
// lpc_lex_push_string_buffer; report end-of-input for it.
#define YY_INPUT(buf, result, max_size) \
    (void)(buf); (void)(max_size); result = 0;

// Defined in the trailer below (uses the buffer-stack helpers).
static int lpc_lex_pop_splice_if_any(void *yyscanner);
static void lpc_lex_consume_directive_newline(void *yyscanner);


%}

%option noyywrap nounput nodefault
%option noyyalloc noyyfree noyyrealloc
%option never-interactive
%option 8bit
%option array
%option reentrant
%option bison-bridge
%option bison-locations
%option stack
%option yylineno
%option extra-type="struct compiler_context_t *"

WS          [ \t\r\n\v\f]
WS_NO_NL    [ \t\r\v\f]

/* SC_CHAR_BODY / SC_CHAR_CLOSE: character literal 'c' -- exactly one body
 * char/escape, then a closing quote. See the "'" rule below.
 * SC_BLOCK_COMMENT: a C-style block comment. Unlike heredocs, the closing
 * delimiter is fixed (not a runtime-supplied terminator), so this is a good
 * native-Flex fit despite scanning an unbounded body. Entered via
 * yy_push_state()/left via yy_pop_state() (%option stack) rather than a
 * plain BEGIN(), so any state that should tolerate an embedded comment can
 * reuse it and automatically resume wherever it was, instead of needing
 * its own copy of the comment rules or hand-rolled "return to" tracking.
 * See the block-comment open rule below.
 * SC_STRING_BODY: a double-quoted string literal. Like block comments, the
 * closing delimiter is fixed; unlike comments, the preprocessor does NOT
 * strip these before the lexer sees them (it only skips over quoted text
 * to avoid macro-expanding inside it), so this one is a genuinely reached
 * part of the grammar. See the '"' open rule below.
 * SC_TEMPLATE_BODY: a backtick template literal `...${expr}...`. Scans the
 * literal-text fragment between backtick/interpolation boundaries the same
 * way SC_STRING_BODY does; the interpolated expr itself is scanned by the
 * ordinary top-level rules (Flex returns to INITIAL for it), with '{'/'}'
 * brace-depth tracking (on the plain '{'/'}' rules below) to find the
 * matching '}' that ends the interpolation and resumes SC_TEMPLATE_BODY.
 * A backtick literal with no interpolation at all is returned as a plain
 * L_STRING instead, so it folds into ordinary string concatenation. See
 * the "`" open rule below and template_nesting/template_brace_depth above.
 * SC_HEREDOC_TERM: the "@"/"@@" prefix and terminator identifier of a
 * heredoc (e.g. "@END" in "@END\nbody\nEND\n"). Only this *opening* piece
 * is native -- the heredoc *body* is matched against the terminator at
 * runtime (the terminator is supplied by the LPC source itself, not known
 * until compile time), which structurally can't be a static Flex pattern,
 * so that part stays a hand-written helper (parseHeredoc() in
 * lexer_utils.cc, reading the body line-by-line through lpc_lex_getc()).
 * See the heredoc rules below.
 *
 * SC_COND_SKIP: inside a false #if/#ifdef branch -- lines are consumed
 * without tokenizing, watching only for the nested/closing conditional
 * directives. See the directive rules below. */
%x SC_CHAR_BODY
%x SC_CHAR_CLOSE
%x SC_BLOCK_COMMENT
%x SC_STRING_BODY
%x SC_TEMPLATE_BODY
%x SC_HEREDOC_TERM
%x SC_COND_SKIP

%%



{WS_NO_NL} { /* ignore */ }
\n          { lpc_lex_newline(yyscanner); }
"\\"\r?\n   { lpc_lex_newline(yyscanner); }

    /* SC_BLOCK_COMMENT is entered via yy_push_state()/left via
     * yy_pop_state() (%option stack), not a plain BEGIN(), so it's a
     * reusable sub-scan: whichever state had a comment-open appear in it gets
     * control back exactly where it left off once the comment closes,
     * instead of every such state needing its own copy of these six
     * rules, or hand-rolling "which state do I return to" bookkeeping. */
"/*"    { yy_push_state(SC_BLOCK_COMMENT, yyscanner); }
"//"[^\n]*  { /* line comment; trailing newline handled by the \n rule */ }

    /* Block comment body: matched natively, no outp access. A lone slash
     * is kept out of the ordinary-run pattern below (it gets its own rule
     * instead) so that a nested comment-open sequence starting right after
     * some other character is still recognized as its own two-character
     * unit and triggers the warning, rather than the slash being swept up
     * together with the preceding character by the catch-all run. */
<SC_BLOCK_COMMENT>"/*"      { yywarn("/* found in comment."); }
<SC_BLOCK_COMMENT>"*"+"/"   { yy_pop_state(yyscanner); }
<SC_BLOCK_COMMENT>"*"+      { /* run of '*' not (yet) followed by the closing '/' */ }
<SC_BLOCK_COMMENT>\n        { lpc_lex_newline(yyscanner); }
<SC_BLOCK_COMMENT>[^*/\n]+  { /* ordinary comment text */ }
<SC_BLOCK_COMMENT>"/"       { /* lone slash, not the start of a nested comment-open */ }
<SC_BLOCK_COMMENT><<EOF>>   {
              /* Every state-specific <<EOF>> rule pops a drained pushed
               * buffer first (lpc_lex_pop_splice_if_any) and keeps
               * scanning in the same start condition -- the construct
               * simply continues into the parent buffer's bytes. Only
               * genuine end-of-input is the error/finalize case. */
              if (!lpc_lex_pop_splice_if_any(yyscanner)) {
                lexerror("End of file in a comment");
                yy_pop_state(yyscanner);
              }
            }

"0"[xX][0-9a-fA-F](_?[0-9a-fA-F])* { return lpc_lex_number_hex(yylval_param, yytext, yyleng); }
"0"[bB][01](_?[01])* { return lpc_lex_number_bin(yylval_param, yytext, yyleng); }
[0-9](_?[0-9])*"."(_?[0-9])+ { return lpc_lex_number_real(yylval_param, yytext, yyleng); }
%{
    /* A trailing '.' with no fraction digits (e.g. "1.") is still a valid
     * float -- but only when NOT immediately followed by another '.', which
     * must instead be left for the ".." range operator (e.g. "1..5" is
     * NUMBER(1) RANGE NUMBER(5), not REAL(1.0) DOT NUMBER(5)). */
%}
[0-9](_?[0-9])*"."/[^.] { return lpc_lex_number_real(yylval_param, yytext, yyleng); }
[0-9](_?[0-9])* { return lpc_lex_number_dec(yylval_param, yytext, yyleng); }

    /* Note: '##' token pasting exists only INSIDE macro bodies (resolved
     * by substitute() before a body is ever spliced into the input); a
     * bare '##' in ordinary code is invalid and falls through to the '.'
     * catch-all like any other illegal character.
     *
     */
[a-zA-Z_][a-zA-Z0-9_]* {
          return lpc_lex_resolve_identifier(yylval_param, yylloc_param, yyscanner);
        }

    /* String literals: fully native via SC_STRING_BODY, no outp access.
     * Escapes are decoded rule-by-rule into str_accum (declared above) as
     * they're matched; the scratchpad allocation happens once, at the
     * closing quote. Two intentional deviations from the original
     * hand-written version (an
     * embedded-NUL quirk on octal \8/\9, and simplified error messages on
     * a handful of malformed \u/\U corners) and one fixed-as-a-side-effect
     * inconsistency (\u/\U now decode the same way regardless of string
     * length, where the original silently stopped decoding them past the
     * first 255 bytes of a string). */
\"      { yyextra->str_accum.clear(); BEGIN(SC_STRING_BODY); }

    /* Template literals `...${expr}...`: the literal-text fragments are
     * scanned natively via SC_TEMPLATE_BODY, sharing SC_STRING_BODY's
     * escape-decoding rules below wherever the two genuinely agree (every
     * rule tagged <SC_STRING_BODY,SC_TEMPLATE_BODY> or joined with the "|"
     * same-action continuation is reused, not duplicated); only the
     * pieces that differ get their own rule -- the delimiter itself, the
     * plain-text-run/bare-newline handling (a template strips bare
     * newlines instead of preserving them), '$'/backtick-vs-quote escapes,
     * and \u/\U (template literals don't support them -- see
     * docs/lpc/types/strings.md). "${" ends the fragment and hands control
     * back to INITIAL to scan the interpolated expression (the matching
     * '}' is found via the brace-depth tracking on the plain '{'/'}' rules
     * further below). */
`      { yyextra->str_accum.clear(); yyextra->template_is_continuation = false; BEGIN(SC_TEMPLATE_BODY); }

<SC_STRING_BODY>\r\n {
          lpc_lex_newline(yyscanner);
          yyextra->str_accum += '\n';
          STR_CHECK_OVERFLOW();
        }
    /* Template CRLF is dropped entirely (no append) -- NOT shared with the
     * string rule above, which preserves it as a lone '\n'; this is the
     * two-character-match counterpart to the bare-\n rule below, both
     * following the "bare newline removed, not preserved" template rule.
     * Genuinely distinct action, not just a distinct pattern, so a plain
     * <SC_STRING_BODY,SC_TEMPLATE_BODY> tag would be wrong here. */
<SC_TEMPLATE_BODY>\r\n { lpc_lex_newline(yyscanner); }
<SC_STRING_BODY>[^\\\"\r\n]+ |
<SC_TEMPLATE_BODY>[^\\`$\r\n]+ {
          yyextra->str_accum.append(yytext, yyleng);
          STR_CHECK_OVERFLOW();
        }
<SC_STRING_BODY>\n {
          lpc_lex_newline(yyscanner);
          yyextra->str_accum += '\n';
          STR_CHECK_OVERFLOW();
        }
<SC_TEMPLATE_BODY>\n { lpc_lex_newline(yyscanner); /* newlines are removed, not preserved */ }
<SC_STRING_BODY,SC_TEMPLATE_BODY>\r { yyextra->str_accum += '\r'; STR_CHECK_OVERFLOW(); }
<SC_TEMPLATE_BODY>"$"  { yyextra->str_accum += '$'; STR_CHECK_OVERFLOW(); }
<SC_STRING_BODY>"\""   {
          BEGIN(INITIAL);
          return lpc_lex_string_close(yyscanner, yylval_param);
        }
<SC_TEMPLATE_BODY>"${" {
          BEGIN(INITIAL);
          return lpc_lex_template_head_or_middle(yyscanner, yylval_param);
        }
<SC_TEMPLATE_BODY>`      {
          BEGIN(INITIAL);
          return lpc_lex_template_tail_or_string(yyscanner, yylval_param);
        }
<SC_STRING_BODY,SC_TEMPLATE_BODY><<EOF>> {
          if (!lpc_lex_pop_splice_if_any(yyscanner)) {
            lexerror(YY_START == SC_TEMPLATE_BODY ? "End of file in template literal"
                                                   : "End of file in string");
            BEGIN(INITIAL);
            return YYEOF;
          }
        }

<SC_STRING_BODY,SC_TEMPLATE_BODY>"\\\r\n" { lpc_lex_newline(yyscanner); }
<SC_STRING_BODY,SC_TEMPLATE_BODY>"\\\n"  { lpc_lex_newline(yyscanner); }
    /* Simple one-character escapes, all decoded through the one shared
     * table (lpc_lex_simple_escape, lexer_rules.cc). Which escapes each
     * context RECOGNIZES is expressed by these per-context character
     * classes -- e.g. \` in a plain string still falls to the
     * unknown-escape rule below with its warning -- but the decode mapping
     * itself exists exactly once for string, template, AND char-literal
     * bodies. */
<SC_STRING_BODY,SC_TEMPLATE_BODY>"\\"[ntrbae\\] |
<SC_STRING_BODY>"\\\"" |
<SC_TEMPLATE_BODY>"\\"[`$] {
          yyextra->str_accum += static_cast<char>(lpc_lex_simple_escape(yytext[1]));
          STR_CHECK_OVERFLOW();
        }
<SC_STRING_BODY,SC_TEMPLATE_BODY>"\\"[0-7]+ {
          lpc_lex_append_octal_escape(yyscanner, yytext, YY_START == SC_TEMPLATE_BODY);
          STR_CHECK_OVERFLOW();
        }
    /* Deviation from the original (documented in the migration plan): '\8'
     * and '\9' aren't valid octal digits. The original's strtoll() there
     * consumed zero characters and left them unconsumed, which -- via a
     * chain of implicit fallthrough -- appended a NUL byte to the string
     * followed by a literal '8'/'9', an obscure and almost certainly
     * unintended embedded-NUL quirk. Reported directly as an error here
     * instead of replicating that. */
<SC_STRING_BODY,SC_TEMPLATE_BODY>"\\"[89]  {
          lpc_lex_append_bad_octal_escape(yyscanner, YY_START == SC_TEMPLATE_BODY);
        }
<SC_STRING_BODY,SC_TEMPLATE_BODY>"\\x"[0-9a-fA-F]+ {
          lpc_lex_append_hex_escape(yyscanner, yytext, YY_START == SC_TEMPLATE_BODY);
          STR_CHECK_OVERFLOW();
        }
<SC_STRING_BODY,SC_TEMPLATE_BODY>"\\x"   {
          lpc_lex_append_bad_hex_escape(yyscanner, YY_START == SC_TEMPLATE_BODY);
          STR_CHECK_OVERFLOW();
        }
    /* \uXXXX\uYYYY: a UTF-16 surrogate pair, matched and decoded as one
     * unit (the lead's second hex digit is 8-B, the trail's is C-F --
     * exactly the D800-DBFF / DC00-DFFF ranges). Ordered before the lone
     * "\uXXXX" rule below only for readability; Flex's longest-match rule
     * already prefers this 12-character form over the 6-character one
     * whenever both could apply. String-only: template literals don't
     * support \u/\U. */
<SC_STRING_BODY>"\\u"[dD][89abAB][0-9a-fA-F][0-9a-fA-F]"\\u"[dD][c-fC-F][0-9a-fA-F][0-9a-fA-F] {
          lpc_lex_append_unicode_pair_escape(yyscanner, yytext);
          STR_CHECK_OVERFLOW();
        }
    /* A lone \uXXXX not part of a valid pair above: either a genuine
     * single (non-surrogate) code point, or a malformed/unpaired
     * surrogate. Deviation from the original (documented in the migration
     * plan): several of the original's distinct error messages for the
     * different ways a surrogate can fail to pair up are consolidated
     * into these two -- nobody writes malformed surrogate pairs on
     * purpose, and both still report a clear error and recover. */
<SC_STRING_BODY>"\\u"[0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F] {
          lpc_lex_append_unicode_escape(yyscanner, yytext);
          STR_CHECK_OVERFLOW();
        }
<SC_STRING_BODY>"\\u"   { lexerror("Illegal unicode sequence."); }
<SC_STRING_BODY>"\\U"[0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F] {
          lpc_lex_append_long_unicode_escape(yyscanner, yytext, yyleng);
          STR_CHECK_OVERFLOW();
        }
<SC_STRING_BODY>"\\U"   { lexerror("Illegal unicode sequence."); }
<SC_STRING_BODY,SC_TEMPLATE_BODY>"\\".   {
          lpc_lex_append_unknown_escape(yyscanner, yytext, YY_START == SC_TEMPLATE_BODY);
          STR_CHECK_OVERFLOW();
        }

    /* Character literals 'c': fully native via two exclusive start
     * conditions -- SC_CHAR_BODY matches exactly one escape sequence or raw
     * byte, SC_CHAR_CLOSE then requires the closing quote. No outp access
     * at all: unlike strings/heredocs, a char literal's body is exactly one
     * grammar unit, so there's no scratchpad allocator or multi-byte
     * accumulation for Flex to sidestep. */
"'"                     { BEGIN(SC_CHAR_BODY); }
    /* Simple escapes decode through the same shared table as
     * string/template bodies (lpc_lex_simple_escape); char literals
     * recognize the ['"] quotes that strings/templates warn about, per
     * this rule's character class. */
<SC_CHAR_BODY>"\\"[ntrbae'"\\] {
          yylval_param->number = lpc_lex_simple_escape(yytext[1]);
          BEGIN(SC_CHAR_CLOSE);
        }
<SC_CHAR_BODY>"\\"[0-7]+ { yylval_param->number = lpc_lex_char_octal_escape(yytext); BEGIN(SC_CHAR_CLOSE); }
<SC_CHAR_BODY>"\\"[89]  {
          // Deviation from the original hand-written scanner (documented in
          // historical note): '\8'/'\9' aren't valid octal, so
          // the original's strtoll() there consumed zero digits and fell
          // through to its "missing closing quote" check, reporting the
          // same "Illegal character constant" error by a different path.
          // Reported directly here instead of replicating that fallthrough.
          lexerror("Illegal character constant");
          yylval_param->number = 0;
          BEGIN(SC_CHAR_CLOSE);
        }
<SC_CHAR_BODY>"\\x"[0-9a-fA-F]+ { yylval_param->number = lpc_lex_char_hex_escape(yytext); BEGIN(SC_CHAR_CLOSE); }
<SC_CHAR_BODY>"\\x"     { yylval_param->number = lpc_lex_char_bad_hex_escape(); BEGIN(SC_CHAR_CLOSE); }
<SC_CHAR_BODY>"\\\r\n" |
<SC_CHAR_BODY>"\\\n"    {
          yylval_param->number = '\n';
          lpc_lex_newline(yyscanner);
          BEGIN(SC_CHAR_CLOSE);
        }
<SC_CHAR_BODY>"\\".     { yylval_param->number = lpc_lex_char_unknown_escape(yytext); BEGIN(SC_CHAR_CLOSE); }
    /* Matches the original's unconditional raw-byte fallback: any
     * non-backslash byte is the body char, including a literal quote (an
     * empty-looking char literal treats the immediate closing quote as the
     * body, then fails the following closing-quote check) or a literal
     * embedded newline (current_line is NOT incremented here -- only an
     * escaped newline counts as a line break, matching upstream). */
<SC_CHAR_BODY>[^\\]     {
          yylval_param->number = static_cast<unsigned char>(yytext[0]);
          BEGIN(SC_CHAR_CLOSE);
        }
<SC_CHAR_CLOSE>"'"     {
          BEGIN(INITIAL);
          reinterpret_cast<compiler_context_t *>(yyget_extra(yyscanner))->is_char_literal = true;
          return L_NUMBER;
        }
    /* The three ways a char literal fails to close share one recovery
     * (lpc_lex_char_error: report, zero yylval_param->number, return L_NUMBER).
     * The zeroing matters even at EOF -- the original's closing-quote
     * check reset the value on ANY failure, and refill_buffer()'s
     * synthesized trailing newline means a source of just "'" reaches
     * <SC_CHAR_CLOSE><<EOF>> with yylval_param->number already set to '\n' by
     * the body rule (real bug found in self-review: it leaked through as
     * 10 instead of 0). The non-EOF failure pushes the offending byte
     * back for the next scan, matching the original. */
<SC_CHAR_BODY><<EOF>>  {
          if (!lpc_lex_pop_splice_if_any(yyscanner)) { BEGIN(INITIAL); return lpc_lex_char_error(yylval_param); }
        }
<SC_CHAR_CLOSE><<EOF>> {
          if (!lpc_lex_pop_splice_if_any(yyscanner)) { BEGIN(INITIAL); return lpc_lex_char_error(yylval_param); }
        }
<SC_CHAR_CLOSE>.|\n    {
          LPC_YYLESS(0);
          BEGIN(INITIAL);
          return lpc_lex_char_error(yylval_param);
        }

    /* $N function-pointer parameters: fully native -- this token's
     * grammar (an optional run of digits) needs no helper beyond decoding
     * the number. */
"$"[0-9]+ {
          int result = lpc_lex_function_param(yylval_param, yytext, yyleng);
          if (result == kLpcLexFunctionParamRetry) return yylex(yylval_param, yylloc_param, yyscanner);
          return result;
        }
"$"     { return lpc_lex_function_param(yylval_param, yytext, yyleng); }

    /* '(' compound tokens, reduced by 9.1/9.2 to just "(:" (one
     * function-pointer-open token; the named-vs-anonymous split lives in
     * the grammar) plus its "(::" longest-match guard. Array "({" and mapping "([" literals are
     * ORDINARY '(' '{' / '(' '[' token pairs now -- the grammar pairs
     * them (their closers always were separate '}' ')' tokens),
     * whitespace between flows through the normal rules, and the plain
     * '{' rule's template-brace tracking covers the array-literal case
     * with no special-casing. */
"("{WS}*"::" {
          // '(' followed by the "::" scope operator, not a function-pointer
          // open. Put everything but '(' back -- whitespace included -- and
          // let "::" scan as its own L_COLON_COLON token next; Flex's own
          // whitespace/newline rules re-tokenize any pushed-back whitespace.
          LPC_YYLESS(1);
          return '(';
        }
"("{WS}*":" {
          /* 9.2: "(:" is one token; the named-vs-anonymous split lives in
           * the GRAMMAR now (shift-preference on the ':'/','' lookahead
           * after a defined name). The function context is pushed here
           * for every form and popped by whichever functional
           * constructor reduces -- uniform, where the legacy lexer-side
           * split pushed only for anon/override forms (and leaked the
           * override's context). */
          lpc_lex_count_newlines(yytext, yyleng);
          push_function_context();
          return L_FUNCTION_OPEN;
        }
"("     { return '('; }


    /* Preprocessor directives -- ONE anchored rule, serving BOTH scan
     * modes (emit and SC_COND_SKIP dead-branch), captures the whole
     * logical line ('#' first on its line, backslash-continuations folded
     * in) and hands it to lpc_lex_on_directive() in lexer_rules_pp.cc,
     * which parses the line once and does everything: #define/#undef
     * mutate the session macro table, #if/#ifdef/#elif/#else/#endif drive
     * the conditional stack, #include pushes onto the input-source stack,
     * #pragma/#line/#error/#warn/#echo apply immediately; in skip mode it
     * only tracks conditional nesting and dispatches the directive that
     * ends the dead branch (dead code is consumed by the SC_COND_SKIP
     * rules below without being tokenized -- it may be syntactically
     * invalid on purpose). The returned action is the only thing left for
     * this rule to do: the start-condition transition. Because this is
     * the one and only scan -- there is no separate preprocessor pass
     * anymore -- applying each directive's effect right here IS
     * position-correct by construction: a mid-file `#pragma no_warnings`
     * affects exactly the code after it, and a #line reset lines up with
     * the token stream, with a single current_line counter that nothing
     * else fights.
     */
<INITIAL,SC_COND_SKIP>^[ \t]*"#"[^\n]*(\\\r?\n[^\n]*)* {
    lpc_lex_consume_directive_newline(yyscanner);
    LpcDirectiveAction act =
        lpc_lex_on_directive(yytext, yyleng, yyscanner, YY_START == SC_COND_SKIP);
    if (act == LpcDirectiveAction::kEnterSkip) {
        BEGIN(SC_COND_SKIP);
    } else if (act == LpcDirectiveAction::kExitSkip) {
        BEGIN(INITIAL);
    }
}

<SC_COND_SKIP>[^\n]+ { /* dead-branch text: never tokenized */ }
<SC_COND_SKIP>\n { lpc_lex_newline(yyscanner); }

    /* Heredoc "@"/"@@" prefix and terminator identifier: fully native, no
     * outp access. "@@" (Robocoder's array-of-lines form) vs plain "@" is
     * distinguished by yyleng; the terminator name is accumulated by the
     * identifier rule below, cleared up front so an empty terminator
     * (nothing alnum/underscore before the trailing whitespace/newline) is
     * still detected even though that rule then never fires. */
"@""@"?  {
          yyextra->heredoc_terminator.clear();
          yyextra->heredoc_is_array = (yyleng == 2);
          BEGIN(SC_HEREDOC_TERM);
        }
<SC_HEREDOC_TERM>[a-zA-Z0-9_]+ { yyextra->heredoc_terminator.assign(yytext, yyleng); }
<SC_HEREDOC_TERM>{WS_NO_NL}*\n {
          /* parseHeredoc() reads the body through lpc_lex_getc() (Flex's
           * own stream), so no rewind/flush choreography is needed here
           * anymore -- which also lets this newline count uniformly via
           * lpc_lex_newline() (it was the one documented exception while
           * the rewind still followed it). */
          lpc_lex_newline(yyscanner);
          BEGIN(INITIAL);
          return lpc_lex_start_heredoc(yylval_param, yylloc_param, yyscanner);
        }
    /* Trailing whitespace NOT immediately followed by a newline (i.e. the
     * rule above couldn't complete because EOF or non-whitespace content
     * comes next instead) -- matches the original's is_wspace skip loop,
     * which keeps consuming whitespace one character at a time regardless
     * of what eventually follows it. Bug fixed here (found during a
     * self-review pass, not present in the original): without this rule,
     * the "." catch-all below -- being the only other option Flex can
     * match when "[ \t\v\f\r]*\n" fails to complete -- would match just
     * the FIRST whitespace character and incorrectly treat it as the
     * first byte of the heredoc body, instead of skipping it like the
     * original does. */
<SC_HEREDOC_TERM>[ \t\v\f\r]+ { /* consumed; next scan decides via <<EOF>> or "." below */ }
<SC_HEREDOC_TERM><<EOF>> {
          if (!lpc_lex_pop_splice_if_any(yyscanner)) {
            // Matches the original: EOF anywhere before the trailing
            // newline is "Illegal terminator", even if a well-formed
            // terminator identifier was already read.
            lexerror("Illegal terminator");
            BEGIN(INITIAL);
            return YYEOF;
          }
        }
<SC_HEREDOC_TERM>. {
          // Non-whitespace, non-newline byte right after the terminator
          // name (or immediately after "@"/"@@"): the original leaves it
          // unconsumed for the body scan to treat as its first byte --
          // yyless(0) puts it back in Flex's buffer, where parseHeredoc's
          // lpc_lex_getc() reads it first.
          LPC_YYLESS(0);
          BEGIN(INITIAL);
          return lpc_lex_start_heredoc(yylval_param, yylloc_param, yyscanner);
        }

"++"    { yylval_param->number = '+'; return L_INC_DEC; }
"+="    { yylval_param->number = F_ADD_EQ; return L_ASSIGN; }
"+"     { return '+'; }
"->"    { return L_ARROW; }
"--"    { yylval_param->number = '-'; return L_INC_DEC; }
"-="    { yylval_param->number = F_SUB_EQ; return L_ASSIGN; }
"-"     { return '-'; }

"&&="   { yylval_param->number = F_LAND_EQ; return L_ASSIGN; }
"&&"    { return L_LAND; }
"&="    { yylval_param->number = F_AND_EQ; return L_ASSIGN; }
"&"     { return '&'; }

"||="   { yylval_param->number = F_LOR_EQ; return L_ASSIGN; }
"||"    { return L_LOR; }
"|="    { yylval_param->number = F_OR_EQ; return L_ASSIGN; }
"|"     { return '|'; }

"^="    { yylval_param->number = F_XOR_EQ; return L_ASSIGN; }
"^"     { return '^'; }

"<<="   { yylval_param->number = F_LSH_EQ; return L_ASSIGN; }
"<<"    { yylval_param->number = F_LSH; return L_SHIFT; }
"<="    { yylval_param->number = F_LE; return L_ORDER; }
"<"     { return '<'; }

">>="   { yylval_param->number = F_RSH_EQ; return L_ASSIGN; }
">>"    { yylval_param->number = F_RSH; return L_SHIFT; }
">="    { yylval_param->number = F_GE; return L_ORDER; }
">"     { yylval_param->number = F_GT; return L_ORDER; }

"*="    { yylval_param->number = F_MULT_EQ; return L_ASSIGN; }
"*"     { return '*'; }

"%="    { yylval_param->number = F_MOD_EQ; return L_ASSIGN; }
"%"     { return '%'; }

"/="    { yylval_param->number = F_DIV_EQ; return L_ASSIGN; }
"/"     { return '/'; }

"=="    { yylval_param->number = F_EQ; return L_EQ_NE; }
"="     { yylval_param->number = F_ASSIGN; return L_ASSIGN; }

"??="   { yylval_param->number = F_NULLISH_EQ; return L_ASSIGN; }
"??"    { return L_QUESTION_QUESTION; }
    /* Optional chaining "?." (mapping member/bracket access, e.g. m?.key,
     * m?.[idx]): only recognized when followed by an identifier-start char
     * or '[', so a ternary's float branch ("cond ? .5 : x") still lexes as
     * '?' then the ".5" float literal, not "?." followed by "5". */
"?."/[A-Za-z_\[]  { return L_OPTIONAL_DOT; }
"?"     { return '?'; }

"!="    { yylval_param->number = F_NE; return L_EQ_NE; }
"!"     { return '!'; }

"::"    { return L_COLON_COLON; }
":"     { return ':'; }

"..."   { return L_DOT_DOT_DOT; }
".."    { return L_RANGE; }
    /* Optional chaining bracket form "m.?[idx]" -- the only grammar
     * production for L_DOT_OPTIONAL requires a following '[', so no
     * lookahead disambiguation is needed here the way "?." above needs one:
     * a literal '?' right after '.' was already a syntax error otherwise. */
".?"    { return L_DOT_OPTIONAL; }
"."     { return '.'; }

")"     { return ')'; }
    /* '{'/'}' inside a template literal's ${...} interpolation need brace-
     * depth tracking so the '}' that ends the interpolation (depth back to
     * 0) can be told apart from one that's merely part of the interpolated
     * expression itself. See SC_TEMPLATE_BODY above. */
"{"     {
          lpc_lex_brace_open(yyscanner);
          return '{';
        }
"}"     {
          if (lpc_lex_brace_close(yyscanner)) {
            BEGIN(SC_TEMPLATE_BODY);
            return yylex(yylval_param, yylloc_param, yyscanner);
          }
          return '}';
        }
"["     { return '['; }
"]"     { return ']'; }
";"     { return ';'; }
","     { return ','; }
"~"     { return '~'; }

<<EOF>> {
          if (lpc_lex_pushed_depth() > 0) {
            if (lpc_lex_top_buffer_kind() == LPC_BUF_IF_EXPR) {
              // A #if expression buffer ran dry: end the evaluator's
              // token pull exactly here -- never continue into the
              // parent input.
              lpc_lex_pop_pushed_buffer(yyscanner);
              return LPC_IFEXPR_END;
            }
            // A pushed buffer (splice or #include content) ran dry: pop
            // back to the parent buffer (which resumes at exactly the
            // position it left off; an include pop also restores
            // current_file/current_line) and keep scanning -- no return,
            // Flex continues in whatever start condition is active.
            lpc_lex_pop_pushed_buffer(yyscanner);
          } else {
            // Genuine end of the main file. Flex's EOF state is sticky:
            // any further next() calls land here again and return the
            // same end-of-tokens signal.
            return parseMainEof(yylval_param, yyscanner);
          }
        }

.       {
          // Nothing above matched: genuinely invalid input. No outp
          // rewind/flush needed -- lpc_lex_badlex() only inspects the
          // character value, it doesn't touch outp or Flex's buffer.
          return lpc_lex_badlex(static_cast<unsigned char>(yytext[0]), yyscanner);
        }

%%

// Read ONE character from the scanner's input, through Flex itself (its
// buffer, then YY_INPUT) -- the official mechanism for helpers that must
// consume raw characters mid-rule (heredoc bodies, macro-argument
// collection) without desynchronizing from the DFA's own prefetch.
// Returns 0 at end of input (and for a genuine NUL byte, which LPC source
// treats as end-of-input anyway -- matching the legacy raw readers).
// A pushed splice buffer running dry mid-read is popped and the read
// continues from the parent buffer -- the cross-boundary case a raw
// reader must survive (a function-like macro's `(args)` following a
// splice that ends with the macro's name). Wrapped here because Flex
// generates yyinput() as static, private to lexer.autogen.cc.
int lpc_lex_getc(void *yyscanner) {
  struct yyguts_t *yyg = (struct yyguts_t *)yyscanner;
  for (;;) {
    int c = yyinput(yyscanner);
    if (c > 0) {
      // yyinput() maintains yylineno but not yycolumn -- keep the raw
      // readers' column upkeep here (8.1's enumerated bypass site).
      if (c == '\n') {
        yycolumn = 0;
      } else {
        yycolumn++;
      }
      return c;
    }
    if (lpc_lex_pushed_depth() == 0) return c;
    lpc_lex_pop_pushed_buffer(yyscanner);
  }
}

// THE splice mechanism (see lexer.h): push `text` as a fresh Flex buffer on
// top of the current one. Composing yy_scan_bytes() with the buffer STACK
// needs care: yy_scan_bytes()'s internal yy_switch_to_buffer() REPLACES
// the current top-of-stack slot rather than pushing, so the "flush the
// old buffer's position, advance the stack top" half of
// yypush_buffer_state() must be done manually FIRST -- the internal
// switch then lands in a fresh, already-advanced slot instead of
// clobbering (and later double-freeing) the live one. ASan caught the
// naive yypush_buffer_state(yy_scan_string(...)) composition as a
// heap-use-after-free on the very first macro expansion (see
// the compiler README / git history).
// ---------------------------------------------------------------------------
// Flex allocator hookup (%option noyyalloc/noyyfree/noyyrealloc).
//
// One allocator serves two lifetimes, so every block carries a 16-byte
// tag header saying which side owns it:
//   kind 1 (malloc): scanner guts (yyguts_t) and the buffer STACK array
//     -- they live across compiles and may be yyrealloc'd, so they must
//     never touch the arena.
//   kind 2 (arena):  per-compile yy_buffer_state structs -- allocated
//     inside the window the buffer-install helpers open around
//     yy_scan_buffer, reclaimed wholesale at scratch_destroy (yyfree on
//     them is a no-op). lpc_lex_teardown() deletes every live buffer
//     BEFORE the arena reset so no code ever touches a stale struct.
// ---------------------------------------------------------------------------
namespace {
struct LpcYyHdr {
  uint64_t kind;  // 1 = malloc, 2 = arena
  uint64_t size;  // payload bytes (for defensive realloc copies)
};
bool s_flex_alloc_arena = false;
}  // namespace

void *yyalloc(yy_size_t size, yyscan_t /*yyscanner*/) {
  LpcYyHdr *h;
  if (s_flex_alloc_arena) {
    h = static_cast<LpcYyHdr *>(scratch_raw_allocate(size + sizeof(LpcYyHdr), 16));
    h->kind = 2;
  } else {
    h = static_cast<LpcYyHdr *>(malloc(size + sizeof(LpcYyHdr)));
    h->kind = 1;
  }
  h->size = size;
  return h + 1;
}

void yyfree(void *ptr, yyscan_t /*yyscanner*/) {
  if (ptr == nullptr) return;
  LpcYyHdr *h = static_cast<LpcYyHdr *>(ptr) - 1;
  if (h->kind == 1) free(h);
  /* kind 2: arena memory, reclaimed at scratch_destroy. */
}

void *yyrealloc(void *ptr, yy_size_t size, yyscan_t yyscanner) {
  if (ptr == nullptr) return yyalloc(size, yyscanner);
  LpcYyHdr *h = static_cast<LpcYyHdr *>(ptr) - 1;
  if (h->kind == 1) {
    h = static_cast<LpcYyHdr *>(realloc(h, size + sizeof(LpcYyHdr)));
    h->size = size;
    return h + 1;
  }
  /* Arena block (shouldn't happen: only the malloc-side buffer stack is
   * ever realloc'd) -- defensive copy-out to malloc. */
  void *np = nullptr;
  {
    LpcYyHdr *nh = static_cast<LpcYyHdr *>(malloc(size + sizeof(LpcYyHdr)));
    nh->kind = 1;
    nh->size = size;
    memcpy(nh + 1, ptr, h->size < size ? h->size : size);
    np = nh + 1;
  }
  return np;
}

// Delete every live flex buffer (pushed splices AND the base) while their
// arena-backed structs are still valid -- MUST run before scratch_destroy
// (compiler.cc's epilog and error cleanup both call the lexer_utils
// wrapper lpc_lex_teardown_active). After this the scanner holds no
// buffers; the next lpc_lex_set_source materializes a fresh base.
void lpc_lex_teardown(void *yyscanner) {
  struct yyguts_t *yyg = (struct yyguts_t *)yyscanner;
  while (lpc_lex_pushed_depth() > 0) {
    lpc_lex_pop_pushed_buffer(yyscanner);
  }
  if (YY_CURRENT_BUFFER) {
    yy_delete_buffer(YY_CURRENT_BUFFER, yyscanner);
    YY_CURRENT_BUFFER_LVALUE = NULL;
  }
}

// In-place variant: `base`/`size` follow the lpc_lex_set_source contract
// (writable arena memory, two '\0' sentinels included in size). The text
// is scanned where it lies -- zero copy, zero flex text allocation.
void lpc_lex_push_prepared_buffer(char *base, size_t size, int kind, void *yyscanner) {
  struct yyguts_t *yyg = (struct yyguts_t *)yyscanner;
  yyensure_buffer_stack(yyscanner);
  if (!YY_CURRENT_BUFFER) {
    /* Pushing before the first yylex() call (start_new_file's global
     * include file): Flex normally creates the base buffer lazily at
     * yylex() entry, so materialize it now -- otherwise this push would
     * land in slot 0 AS the base, and its eventual yypop_buffer_state()
     * would find no parent to reload, leaving yy_c_buf_p dangling into
     * the freed buffer (ASan-verified heap-use-after-free). */
    YY_CURRENT_BUFFER_LVALUE = yy_create_buffer(yyin, YY_BUF_SIZE, yyscanner);
    yy_load_buffer_state(yyscanner);
  }
  /* Same bookkeeping yypush_buffer_state() does before switching: flush
   * the scan position into the old buffer's state so scanning resumes
   * exactly here when the pushed buffer pops. */
  *yyg->yy_c_buf_p = yyg->yy_hold_char;
  YY_CURRENT_BUFFER_LVALUE->yy_buf_pos = yyg->yy_c_buf_p;
  YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars;
  yyg->yy_buffer_stack_top++;
  s_flex_alloc_arena = true;  // the yy_buffer_state struct is per-compile
  YY_BUFFER_STATE nb = yy_scan_buffer(base, size, yyscanner);
  s_flex_alloc_arena = false;
  if (nb == NULL) {
    lpc_lex_fatal("lpc_lex_push_prepared_buffer: buffer missing sentinels");
    return;
  }
  /* yy_scan_buffer fills the buffer struct by hand and -- unlike
   * yy_init_buffer() -- leaves yy_bs_lineno/column UNINITIALIZED;
   * %option yylineno counting would start from garbage. */
  yylineno = 1;
  yycolumn = 0;
  yyg->yy_did_buffer_switch_on_eof = 1;
  lpc_lex_note_buffer_push(kind);
}

// Copying variant for callers whose text is NOT scannable in place
// (persistent macro bodies, string_views over foreign memory): one arena
// copy plus sentinels, then the in-place push -- still no flex malloc.
void lpc_lex_push_string_buffer(const char *text, size_t len, int kind,
                                void *yyscanner) {
  char *base = static_cast<char *>(scratch_raw_allocate(len + 2, 1));
  memcpy(base, text, len);
  base[len] = 0;
  base[len + 1] = 0;
  lpc_lex_push_prepared_buffer(base, len + 2, kind, yyscanner);
}

void lpc_lex_pop_pushed_buffer(void *yyscanner) {
  struct yyguts_t *yyg = (struct yyguts_t *)yyscanner;
  int ending_lineno = YY_CURRENT_BUFFER ? yylineno : 0;
  lpc_lex_note_buffer_pop(ending_lineno);
  yypop_buffer_state(yyscanner);
}

// A directive owns its whole logical line, terminating newline included
// -- consumed and counted BEFORE dispatching, so an #include's stack
// entry records the line AFTER the directive as the parent's resume
// line. The newline is simply the next byte of whatever real source is
// feeding Flex (base stream or an include's pushed buffer; both
// auto-append a trailing '\n' at EOF), read through yyinput(), which
// also sets the BOL flag so '^' anchors keep working on the very next
// line. Splice/if-expr buffers contain no newlines -- nothing to
// consume (a '#' at a fresh splice buffer's start can match the
// anchored rule; pathological, but must not eat a byte).
static void lpc_lex_consume_directive_newline(void *yyscanner) {
  struct yyguts_t *yyg = (struct yyguts_t *)yyscanner;
  int kind = lpc_lex_top_buffer_kind();
  if (kind == LPC_BUF_PLAIN || kind == LPC_BUF_EXPANSION || kind == LPC_BUF_IF_EXPR) {
    return;
  }
  int nl = yyinput(yyscanner);
  if (nl == '\n') {
    lpc_lex_newline(yyscanner);
    yycolumn = 0;  // yyinput() doesn't maintain yycolumn
  }
}

// The shared <<EOF>> decision: a drained pushed buffer (splice or
// #include content -- NOT a #if expression, whose edge is hard) is
// popped and scanning simply continues; returns 0 when this is a
// genuine end-of-input the rule must handle itself.
static int lpc_lex_pop_splice_if_any(void *yyscanner) {
  if (lpc_lex_pushed_depth() > 0 && lpc_lex_top_buffer_kind() != LPC_BUF_IF_EXPR) {
    lpc_lex_pop_pushed_buffer(yyscanner);
    return 1;
  }
  return 0;
}

int lpc_lex_ifexpr_next(union YYSTYPE *yylval_param, void *yyscanner) {
  struct yyguts_t *yyg = (struct yyguts_t *)yyscanner;
  int saved = YY_START;
  // The nested yylex() repoints yyg's yylval_r/yylloc_r at OUR
  // parameters; restore the suspended outer frame's pointers afterward
  // (rule actions write through their own parameters, but flex-internal
  // code paths consult the yyg copies).
  YYSTYPE *saved_lval = yyget_lval(yyscanner);
  YYLTYPE *saved_lloc = yyget_lloc(yyscanner);
  YYLTYPE lloc;
  BEGIN(INITIAL);
  int tok = yylex(yylval_param, &lloc, yyscanner);
  BEGIN(saved);
  yyset_lval(saved_lval, yyscanner);
  yyset_lloc(saved_lloc, yyscanner);
  return tok;
}

// ---------------------------------------------------------------------------
// Raw buffer-introspection primitives: pure flex-state accessors, zero
// policy. The generated scanner's buffer types (yyguts_t, yy_buffer_state)
// are private to this translation unit, so these are the ONLY window other
// files get; all actual logic built on them (the current_line real-frame
// walk, the diagnostic snippet/caret assembly) lives in lexer_utils.cc.
// ---------------------------------------------------------------------------

// Number of buffers on the stack, including the base buffer; 0 when the
// scanner has no buffer yet.
int lpc_lex_buffer_count(void *yyscanner) {
  struct yyguts_t *yyg = (struct yyguts_t *)yyscanner;
  if (!YY_CURRENT_BUFFER) {
    return 0;
  }
  return (int)yyg->yy_buffer_stack_top + 1;
}

// Pointer to buffer i's native line counter (i = 0 is the base buffer),
// or NULL when absent.
int *lpc_lex_buffer_lineno(void *yyscanner, int i) {
  struct yyguts_t *yyg = (struct yyguts_t *)yyscanner;
  if (!YY_CURRENT_BUFFER || i < 0 || i > (int)yyg->yy_buffer_stack_top ||
      !yyg->yy_buffer_stack[i]) {
    return NULL;
  }
  return &yyg->yy_buffer_stack[i]->yy_bs_lineno;
}

// Content extents of buffer i: [*base, *limit) are the resident bytes,
// *pos the scan position (the TOP buffer's live cursor; a lower buffer's
// frozen resume point), *held the held-out character replacing the NUL at
// the live cursor during a rule action (top buffer only, else 0).
// Returns 0 when the buffer is absent or its state is inconsistent.
int lpc_lex_buffer_extents(void *yyscanner, int i, const char **base, const char **limit,
                           const char **pos, char *held) {
  struct yyguts_t *yyg = (struct yyguts_t *)yyscanner;
  if (!YY_CURRENT_BUFFER || i < 0 || i > (int)yyg->yy_buffer_stack_top) {
    return 0;
  }
  YY_BUFFER_STATE b = yyg->yy_buffer_stack[i];
  if (!b || !b->yy_ch_buf) {
    return 0;
  }
  int is_top = (i == (int)yyg->yy_buffer_stack_top);
  *base = b->yy_ch_buf;
  *limit = b->yy_ch_buf + (is_top ? yyg->yy_n_chars : b->yy_n_chars);
  *pos = is_top ? yyg->yy_c_buf_p : b->yy_buf_pos;
  *held = is_top ? yyg->yy_hold_char : 0;
  if (*pos == NULL || *pos < *base) {
    return 0;
  }
  if (*pos > *limit) {
    *pos = *limit;
  }
  return 1;
}



// Install a PREPARED buffer as THE base (stack slot 0): the main file's
// whole content scanned IN PLACE via yy_scan_buffer -- zero copy, zero
// flex text allocation. `base` must be writable arena memory (flex does
// its hold-char/NUL dance in it) whose final two bytes are '\0' '\0'
// sentinels; `size` INCLUDES those two bytes. Any previous base is
// deleted (a reused scanner replaces its base per compile; flex never
// frees the text -- yy_is_our_buffer is 0 for scan buffers).
void lpc_lex_set_source(char *base, size_t size, void *yyscanner) {
  struct yyguts_t *yyg = (struct yyguts_t *)yyscanner;
  yyensure_buffer_stack(yyscanner);
  if (YY_CURRENT_BUFFER) {
    yy_delete_buffer(YY_CURRENT_BUFFER, yyscanner);
    YY_CURRENT_BUFFER_LVALUE = NULL;
  }
  s_flex_alloc_arena = true;  // the yy_buffer_state struct is per-compile
  YY_BUFFER_STATE b = yy_scan_buffer(base, size, yyscanner);
  s_flex_alloc_arena = false;
  if (b == NULL) {
    lpc_lex_fatal("lpc_lex_set_source: buffer missing sentinels");
    return;
  }
  /* yy_scan_buffer leaves yy_bs_lineno/column uninitialized -- same
   * fixup as lpc_lex_push_string_buffer. */
  yylineno = 1;
  yycolumn = 0;
  yyg->yy_did_buffer_switch_on_eof = 1;
  BEGIN(INITIAL);
}

void lpc_lex_reset(void *yyscanner) {
  struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
  // A compile that aborted mid-splice can leave pushed splice buffers
  // stacked; pop them so the next compile starts on the base buffer.
  while (lpc_lex_pushed_depth() > 0) {
    lpc_lex_pop_pushed_buffer(yyscanner);
  }
  if (YY_CURRENT_BUFFER) {
    YY_FLUSH_BUFFER;
    // Flushing deliberately preserves a buffer's line counter (see
    // yy_init_buffer's comment in the generated code); a fresh compile on
    // a reused scanner must start its base buffer back at line 1.
    yylineno = 1;
    yycolumn = 0;
  }
  BEGIN(INITIAL);
  lpc_lex_reset_context(reinterpret_cast<compiler_context_t *>(yyget_extra(yyscanner)));
}

