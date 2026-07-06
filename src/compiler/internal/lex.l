%{
/*
 * lex.l — Flex scanner for the LPC lexer.
 *
 * This is the driver's ONE scan over LPC source: preprocessing is part of
 * lexing, not a separate pass (see plans/unified-push-lexer.md). Raw bytes
 * flow LexStream -> ring buffer (lexer_utils.cc) -> YY_INPUT -> the rules
 * here; every byte is read exactly once, one token's worth of work at a
 * time, driven by each LexTokenStream::next() pull.
 *
 * Whitespace, comments, operators/punctuation, numeric literals,
 * identifiers, '(' compound tokens (array/mapping/function-pointer open),
 * function-pointer $N params, char literals, and string literals are all
 * matched natively by Flex's DFA (several via exclusive start conditions
 * -- SC_FUNC_OPEN, SC_CHAR_BODY/SC_CHAR_CLOSE, SC_BLOCK_COMMENT,
 * SC_STRING_BODY, SC_TEMPLATE_BODY).
 *
 * Preprocessor directives are one anchored rule capturing the whole '#'
 * line, dispatched to lexer_rules_pp.cc (macro table, #if conditional
 * stack + the SC_COND_SKIP dead-branch mode, #include via the
 * include-stack push in lexer_utils.cc, #pragma/#line applied in place).
 * Macro EXPANSION happens at identifier-resolution time
 * (lpc_lex_resolve_identifier in lexer_utils.cc): the expanded body is
 * spliced into the ring buffer via add_input() and rescanned. Guarded
 * self-references left literal in the expansion are counted per name into
 * compiler_context_t::pending_plain at expansion time ("blue paint" per
 * occurrence, C-preprocessor style); the rescan consumes one count per
 * occurrence and resolves it as a plain identifier -- no in-band marker
 * bytes and no dedicated rule (this replaced an earlier end-of-splice
 * "\x1e<name>" sentinel token). Deliberately NOT a genuine Flex buffer
 * push (yy_scan_string()/yypush_buffer_state()) either -- tried, reverted;
 * see the long comment at the object-like macro expansion site in
 * lexer_utils.cc for the real incompatibility this ran into (the
 * function-like-macro argument collector's raw-outp reads silently break
 * once Flex is scanning from a buffer other than the ring buffer). Because
 * string/template/char bodies are scanned by their own start conditions
 * and never reach identifier resolution, "no macro expansion inside
 * strings" falls out for free.
 *
 * Only heredocs and EOF/include-stack popping still trigger explicit Flex
 * rules that call into standalone parse*() helpers in lexer_utils.cc,
 * which keep their original hand-written, raw-`outp`-reading algorithms
 * unchanged -- a heredoc's closing terminator is supplied at compile time
 * by the LPC source itself (e.g. "@END ... END"), not known until then,
 * so it can't be expressed as a static Flex pattern; the ring-buffer
 * include-stack pop is pure state bookkeeping, not really "scanning" at
 * all. See plans/flex-lexer-migration.md for the risk/value writeup and
 * per-conversion deviation notes. Only genuinely invalid input reaches
 * the catch-all `.` rule at the bottom.
 *
 * Include order: grammar_rules.h MUST come before grammar.autogen.h
 * because grammar.autogen.h references decl_t / func_block_t from it.
 *
 * Buffer synchronization: YY_INPUT below reads directly from lexer_utils.cc's
 * ring buffer (via outp/lpc_lex_yy_input), so `outp` always reflects the
 * total number of bytes ever handed to Flex -- which can run ahead of
 * the position of the token Flex just matched, since Flex may fetch
 * bytes in bulk and/or peek one extra lookahead character before
 * settling on the longest match. Whenever a rule hands control to one of
 * the remaining raw-`outp`-reading parse*() helpers, it must first rewind
 * `outp` by exactly that unread remainder -- computed via
 * YY_PENDING_LOOKAHEAD() from Flex's own buffer bookkeeping -- and flush
 * Flex's buffer so the next top-level yylex() call re-reads fresh from the
 * corrected `outp` position. Without this, characters already
 * buffered-but-unconsumed by Flex would be silently skipped by the helper.
 *
 * parseHeredoc() (on its error paths) and parseEofOrIncludePop() (on
 * include-stack pop) call `yylex()` recursively themselves once they're
 * done, to resume top-level scanning. For those, the flush must happen
 * BEFORE calling the helper, not after -- the recursive yylex() call needs
 * Flex's buffer already reset.
 */
#include "base/std.h"

#include <cstdlib>
#include <string>

#include "compiler/internal/compiler.h"
#include "compiler/internal/lex.h"
#include "compiler/internal/grammar_rules.h"
#include "compiler/internal/grammar.autogen.h"
#include "compiler/internal/scratchpad.h"
#include "compiler/internal/lexer_rules.h"
#include "compiler/internal/lexer_rules_pp.h"
#include "efuns.autogen.h"

#define YY_DECL int yylex(union YYSTYPE *yylval_param, void *yyscanner)
#define yylval (*yylval_param)
#define YY_PENDING_LOOKAHEAD() \
    (yyg->yy_n_chars - (int)(yyg->yy_c_buf_p - YY_CURRENT_BUFFER_LVALUE->yy_ch_buf))

// All template/string states moved to struct compiler_context_t in lex.h

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

// YY_INPUT: pull bytes from lexer_utils.cc's linked_buf_t ring via this bridge.
#define YY_INPUT(buf, result, max_size) \
    result = lpc_lex_yy_input(buf, max_size, yyscanner)

// Declared here; defined in lexer_utils.cc. Bridges Flex's buffer-fill requests to
// the existing ring-buffer, advancing `outp` for every byte handed to Flex.
extern "C" int lpc_lex_yy_input(char *buf, int max_size, void *yyscanner);

// Bytes Flex has already fetched into its buffer beyond the position it has
// actually matched/consumed so far (bulk pre-fetch and/or one-char DFA
// lookahead). See the file-level comment above for why this matters.
// Define short-hand references to the context variables!
#define str_accum                (yyextra->str_accum)
#define template_nesting         (yyextra->template_nesting)
#define template_brace_depth     (yyextra->template_brace_depth)
#define template_is_continuation (yyextra->template_is_continuation)
#define heredoc_terminator       (yyextra->heredoc_terminator)
#define heredoc_is_array         (yyextra->heredoc_is_array)
#define function_flag            (yyextra->function_flag)

%}

%option noyywrap nounput noinput nodefault
%option never-interactive
%option 8bit
%option array
%option reentrant
%option stack
%option extra-type="struct compiler_context_t *"

WS          [ \t\r\n\v\f]
WS_NO_NL    [ \t\r\v\f]

/* SC_FUNC_OPEN: after "(:" (function-pointer open), skip whitespace and
 * decide whether this is a named function reference "(: name" or an
 * anonymous function literal "(: expr :)". See the "(" rules below. Also
 * pushes into SC_BLOCK_COMMENT on "/*"/"//" (see below) so a comment
 * between "(:" and the name/expr is skipped rather than misread as the
 * start of an anonymous function-pointer body.
 * SC_CHAR_BODY / SC_CHAR_CLOSE: character literal 'c' -- exactly one body
 * char/escape, then a closing quote. See the "'" rule below.
 * SC_BLOCK_COMMENT: a C-style block comment. Unlike heredocs, the closing
 * delimiter is fixed (not a runtime-supplied terminator), so this is a good
 * native-Flex fit despite scanning an unbounded body. Entered via
 * yy_push_state()/left via yy_pop_state() (%option stack) rather than a
 * plain BEGIN(), so any state that should tolerate an embedded comment can
 * reuse it and automatically resume wherever it was, instead of needing
 * its own copy of the comment rules or hand-rolled "return to" tracking
 * (currently reused from both INITIAL and SC_FUNC_OPEN). See the
 * block-comment open rule below.
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
 * so that part stays a hand-written helper (parseHeredoc(), still in
 * lexer_utils.cc). See the heredoc rules below.
 *
 * SC_COND_SKIP: inside a false #if/#ifdef branch -- lines are consumed
 * without tokenizing, watching only for the nested/closing conditional
 * directives. See the directive rules below. */
%x SC_FUNC_OPEN
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
     * reusable sub-scan: whichever state had a "/*" appear inside it gets
     * control back exactly where it left off once the comment closes,
     * instead of every such state needing its own copy of these six
     * rules, or hand-rolling "which state do I return to" bookkeeping. See
     * <SC_FUNC_OPEN>"/*" below for the other current entry point. */
<INITIAL,SC_FUNC_OPEN>"/*"        { yy_push_state(SC_BLOCK_COMMENT, yyscanner); }
<INITIAL,SC_FUNC_OPEN>"//"[^\n]*  { /* line comment; trailing newline handled by the \n rule */ }

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
<SC_BLOCK_COMMENT>"/"       { /* lone '/', not the start of a nested "/*" */ }
<SC_BLOCK_COMMENT><<EOF>>   {
              lexerror("End of file in a comment");
              yy_pop_state(yyscanner);
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
     * Shared with SC_FUNC_OPEN (the state after "(:") via a multi-state
     * rule instead of a second copy of this exact pattern: the two states
     * need different handling once matched (SC_FUNC_OPEN must peek one
     * more raw byte to disambiguate "(: foo ," from "(: foo(...)"), so the
     * action branches on YY_START, but the identifier PATTERN itself --
     * and the actual identifier/macro resolution logic in
     * lpc_lex_resolve_identifier() -- exists exactly once. */
<INITIAL,SC_FUNC_OPEN>[a-zA-Z_][a-zA-Z0-9_]* {
          if (YY_START == SC_FUNC_OPEN) {
            BEGIN(INITIAL);
            function_flag = 1;
            outp -= YY_PENDING_LOOKAHEAD();
            int result = lpc_lex_resolve_identifier(yylval_param, yyscanner);
            YY_FLUSH_BUFFER;
            return result;
          }
          return lpc_lex_resolve_identifier(yylval_param, yyscanner);
        }

    /* String literals: fully native via SC_STRING_BODY, no outp access.
     * Escapes are decoded rule-by-rule into str_accum (declared above) as
     * they're matched; the scratchpad allocation happens once, at the
     * closing quote. See plans/flex-lexer-migration.md for the two
     * intentional deviations from the original hand-written version (an
     * embedded-NUL quirk on octal \8/\9, and simplified error messages on
     * a handful of malformed \u/\U corners) and one fixed-as-a-side-effect
     * inconsistency (\u/\U now decode the same way regardless of string
     * length, where the original silently stopped decoding them past the
     * first 255 bytes of a string). */
\"      { str_accum.clear(); BEGIN(SC_STRING_BODY); }

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
`      { str_accum.clear(); template_is_continuation = false; BEGIN(SC_TEMPLATE_BODY); }

<SC_STRING_BODY>\r\n {
          lpc_lex_newline(yyscanner);
          str_accum += '\n';
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
          str_accum.append(yytext, yyleng);
          STR_CHECK_OVERFLOW();
        }
<SC_STRING_BODY>\n {
          lpc_lex_newline(yyscanner);
          str_accum += '\n';
          STR_CHECK_OVERFLOW();
        }
<SC_TEMPLATE_BODY>\n { lpc_lex_newline(yyscanner); /* newlines are removed, not preserved */ }
<SC_STRING_BODY,SC_TEMPLATE_BODY>\r { str_accum += '\r'; STR_CHECK_OVERFLOW(); }
<SC_TEMPLATE_BODY>"$"  { str_accum += '$'; STR_CHECK_OVERFLOW(); }
<SC_STRING_BODY>"\""   {
          BEGIN(INITIAL);
          if (!u8_validate(str_accum.c_str())) {
            lexerror("Invalid UTF8 codepoint in string literal");
            return YYerror;
          }
          yylval.string = scratch_copy(str_accum.c_str());
          return L_STRING;
        }
    /* Template fragment tokens (L_TEMPLATE_HEAD/MIDDLE/TAIL) use
     * scratch_large_alloc() rather than scratch_copy(): unlike a plain
     * string literal (consumed almost immediately by string_literal:
     * L_STRING), a template fragment's value sits on the parser's value
     * stack for as long as it takes to scan the entire ${...} expression
     * that follows it -- which can itself contain more string/template
     * literals. scratch_copy()'s small ring buffer would get overwritten
     * by those before the outer template_literal/template_parts rule ever
     * gets to consume the fragment (this was a real, reproduced bug: the
     * head fragment came back containing the *tail* fragment's text). */
<SC_TEMPLATE_BODY>"${" {
          BEGIN(INITIAL);
          return lpc_lex_template_head_or_middle(yyscanner, yylval_param);
        }
<SC_TEMPLATE_BODY>`      {
          BEGIN(INITIAL);
          return lpc_lex_template_tail_or_string(yyscanner, yylval_param);
        }
<SC_STRING_BODY,SC_TEMPLATE_BODY><<EOF>> {
          lexerror(YY_START == SC_TEMPLATE_BODY ? "End of file in template literal"
                                                 : "End of file in string");
          BEGIN(INITIAL);
          return YYEOF;
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
          str_accum += static_cast<char>(lpc_lex_simple_escape(yytext[1]));
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
          yylval.number = lpc_lex_simple_escape(yytext[1]);
          BEGIN(SC_CHAR_CLOSE);
        }
<SC_CHAR_BODY>"\\"[0-7]+ { yylval.number = lpc_lex_char_octal_escape(yytext); BEGIN(SC_CHAR_CLOSE); }
<SC_CHAR_BODY>"\\"[89]  {
          // Deviation from the original hand-written scanner (documented in
          // plans/flex-lexer-migration.md): '\8'/'\9' aren't valid octal, so
          // the original's strtoll() there consumed zero digits and fell
          // through to its "missing closing quote" check, reporting the
          // same "Illegal character constant" error by a different path.
          // Reported directly here instead of replicating that fallthrough.
          lexerror("Illegal character constant");
          yylval.number = 0;
          BEGIN(SC_CHAR_CLOSE);
        }
<SC_CHAR_BODY>"\\x"[0-9a-fA-F]+ { yylval.number = lpc_lex_char_hex_escape(yytext); BEGIN(SC_CHAR_CLOSE); }
<SC_CHAR_BODY>"\\x"     { yylval.number = lpc_lex_char_bad_hex_escape(); BEGIN(SC_CHAR_CLOSE); }
<SC_CHAR_BODY>"\\\r\n" |
<SC_CHAR_BODY>"\\\n"    {
          yylval.number = '\n';
          lpc_lex_newline(yyscanner);
          BEGIN(SC_CHAR_CLOSE);
        }
<SC_CHAR_BODY>"\\".     { yylval.number = lpc_lex_char_unknown_escape(yytext); BEGIN(SC_CHAR_CLOSE); }
    /* Matches the original's unconditional raw-byte fallback: any
     * non-backslash byte is the body char, including a literal quote (an
     * empty-looking char literal treats the immediate closing quote as the
     * body, then fails the following closing-quote check) or a literal
     * embedded newline (current_line is NOT incremented here -- only an
     * escaped newline counts as a line break, matching upstream). */
<SC_CHAR_BODY>[^\\]     {
          yylval.number = static_cast<unsigned char>(yytext[0]);
          BEGIN(SC_CHAR_CLOSE);
        }
<SC_CHAR_CLOSE>"'"     {
          BEGIN(INITIAL);
          reinterpret_cast<compiler_context_t *>(yyget_extra(yyscanner))->is_char_literal = true;
          return L_NUMBER;
        }
    /* The three ways a char literal fails to close share one recovery
     * (lpc_lex_char_error: report, zero yylval.number, return L_NUMBER).
     * The zeroing matters even at EOF -- the original's closing-quote
     * check reset the value on ANY failure, and refill_buffer()'s
     * synthesized trailing newline means a source of just "'" reaches
     * <SC_CHAR_CLOSE><<EOF>> with yylval.number already set to '\n' by
     * the body rule (real bug found in self-review: it leaked through as
     * 10 instead of 0). The non-EOF failure pushes the offending byte
     * back for the next scan, matching the original. */
<SC_CHAR_BODY><<EOF>>  { BEGIN(INITIAL); return lpc_lex_char_error(yylval_param); }
<SC_CHAR_CLOSE><<EOF>> { BEGIN(INITIAL); return lpc_lex_char_error(yylval_param); }
<SC_CHAR_CLOSE>.|\n    {
          yyless(0);
          BEGIN(INITIAL);
          return lpc_lex_char_error(yylval_param);
        }

    /* $N function-pointer parameters: fully native, no outp access at all
     * -- unlike the string/char/paren rules above, this token's grammar
     * (an optional run of digits) has no line-counting or ring-buffer
     * interaction, so there's nothing left for a legacy helper to do that
     * Flex's own DFA can't do directly. */
"$"[0-9]+ {
          int result = lpc_lex_function_param(yylval_param, yytext, yyleng);
          if (result == kLpcLexFunctionParamRetry) return yylex(yylval_param, yyscanner);
          return result;
        }
"$"     { return lpc_lex_function_param(yylval_param, yytext, yyleng); }

    /* '(' compound tokens: "({" (array), "([" (mapping), "(:"/"(::"
     * (function-pointer open vs. plain '(' followed by the "::" scope
     * operator). Whitespace (but not comments) may freely separate '(' from
     * the following '{'/'['/ ':'. */
"("{WS}*"{" {
          lpc_lex_count_newlines(yytext, yyleng);
          /* The '{' consumed here as part of the "({"  array-open token
           * still closes with a *separate* bare '}' token (followed by
           * ')'), so inside a template literal's ${...} interpolation it
           * must count toward brace depth just like a standalone '{' --
           * otherwise the array's closing '}' is misread as the end of
           * the interpolation (real bug, found in self-review: any
           * `${ ({ ... })... }` garbled the rest of the template). */
          if (template_nesting > 0) {
            template_brace_depth[template_nesting]++;
          }
          return L_ARRAY_OPEN;
        }
"("{WS}*"[" {
          lpc_lex_count_newlines(yytext, yyleng);
          return L_MAPPING_OPEN;
        }
"("{WS}*"::" {
          // '(' followed by the "::" scope operator, not a function-pointer
          // open. Put everything but '(' back -- whitespace included -- and
          // let "::" scan as its own L_COLON_COLON token next; Flex's own
          // whitespace/newline rules re-tokenize any pushed-back whitespace.
          yyless(1);
          return '(';
        }
"("{WS}*":" {
          lpc_lex_count_newlines(yytext, yyleng);
          BEGIN(SC_FUNC_OPEN);
        }
"("     { return '('; }

    /* Tail of "(:" -- skip further whitespace, then either a named
     * function-pointer reference "(: name" or an anonymous function literal
     * "(: expr :)" (anything else, pushed back for the next yylex() call).
     * The identifier case itself is the shared <INITIAL,SC_FUNC_OPEN> rule
     * above. */
<SC_FUNC_OPEN>{WS}+ { lpc_lex_count_newlines(yytext, yyleng); }
<SC_FUNC_OPEN><<EOF>> {
          BEGIN(INITIAL);
          push_function_context();
          return L_FUNCTION_OPEN;
        }
<SC_FUNC_OPEN>. {
          yyless(0);
          BEGIN(INITIAL);
          push_function_context();
          return L_FUNCTION_OPEN;
        }

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
     *
     * Buffer choreography (load-bearing, in this order): a directive owns
     * its whole logical line, terminating newline included (the legacy
     * scanner's parseDirective consumed it too). Rewind outp to the end
     * of the matched text and consume + count the newline ourselves
     * BEFORE dispatching: if the directive turns out to be an #include,
     * the include-stack push must record exactly this position as the
     * parent's resume point -- with outp[-1] being the newline that
     * refill_buffer()'s include branch reuses as its leading sentinel --
     * and it ends by refilling the ring buffer from the included file.
     * Flush Flex's stale prefetch afterward in all cases so scanning
     * resumes from wherever outp now points (the flush also sets Flex's
     * beginning-of-line flag, keeping '^' anchors working for a directive
     * on the very next line). yytext stays valid through the call -- only
     * the flush afterward invalidates it. */
<INITIAL,SC_COND_SKIP>^[ \t]*"#"[^\n]*(\\\r?\n[^\n]*)* {
    outp -= YY_PENDING_LOOKAHEAD();
    if (*outp == '\n') {
        outp++;
        lpc_lex_newline(yyscanner);
    }
    LpcDirectiveAction act =
        lpc_lex_on_directive(yytext, yyleng, yyscanner, YY_START == SC_COND_SKIP);
    YY_FLUSH_BUFFER;
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
          heredoc_terminator.clear();
          heredoc_is_array = (yyleng == 2);
          BEGIN(SC_HEREDOC_TERM);
        }
<SC_HEREDOC_TERM>[a-zA-Z0-9_]+ { heredoc_terminator.assign(yytext, yyleng); }
<SC_HEREDOC_TERM>{WS_NO_NL}*\n {
          /* Deliberately NOT lpc_lex_newline(): its refill check may
           * memmove the ring buffer, and the `outp -=
           * YY_PENDING_LOOKAHEAD()` rewind below reaches BEHIND the
           * refill's relocation point -- refill before the rewind would
           * make the rewind land on stale bytes. The only newline-count
           * site with this constraint (every other one either has no
           * pending rewind or rewinds first). total_lines is skipped to
           * match; the heredoc body's own lines are counted by
           * parseHeredoc(). */
          current_line++;
          BEGIN(INITIAL);
          if (heredoc_terminator.empty()) {
            lexerror("Illegal terminator");
            return yylex(yylval_param, yyscanner);
          }
          outp -= YY_PENDING_LOOKAHEAD();
          YY_FLUSH_BUFFER;
          return parseHeredoc(heredoc_terminator.c_str(), heredoc_is_array, yylval_param, yyscanner);
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
          // Matches the original: EOF anywhere before the trailing
          // newline is "Illegal terminator", even if a well-formed
          // terminator identifier was already read.
          lexerror("Illegal terminator");
          BEGIN(INITIAL);
          return YYEOF;
        }
<SC_HEREDOC_TERM>. {
          // Non-whitespace, non-newline byte right after the terminator
          // name (or immediately after "@"/"@@"): the original leaves it
          // unconsumed for the body scan to treat as its first byte.
          yyless(0);
          BEGIN(INITIAL);
          if (heredoc_terminator.empty()) {
            lexerror("Illegal terminator");
            return yylex(yylval_param, yyscanner);
          }
          outp -= YY_PENDING_LOOKAHEAD();
          YY_FLUSH_BUFFER;
          return parseHeredoc(heredoc_terminator.c_str(), heredoc_is_array, yylval_param, yyscanner);
        }

"++"    { return L_INC; }
"+="    { yylval.number = F_ADD_EQ; return L_ASSIGN; }
"+"     { return '+'; }
"->"    { return L_ARROW; }
"--"    { return L_DEC; }
"-="    { yylval.number = F_SUB_EQ; return L_ASSIGN; }
"-"     { return '-'; }

"&&="   { yylval.number = F_LAND_EQ; return L_ASSIGN; }
"&&"    { return L_LAND; }
"&="    { yylval.number = F_AND_EQ; return L_ASSIGN; }
"&"     { return '&'; }

"||="   { yylval.number = F_LOR_EQ; return L_ASSIGN; }
"||"    { return L_LOR; }
"|="    { yylval.number = F_OR_EQ; return L_ASSIGN; }
"|"     { return '|'; }

"^="    { yylval.number = F_XOR_EQ; return L_ASSIGN; }
"^"     { return '^'; }

"<<="   { yylval.number = F_LSH_EQ; return L_ASSIGN; }
"<<"    { return L_LSH; }
"<="    { yylval.number = F_LE; return L_ORDER; }
"<"     { return '<'; }

">>="   { yylval.number = F_RSH_EQ; return L_ASSIGN; }
">>"    { return L_RSH; }
">="    { yylval.number = F_GE; return L_ORDER; }
">"     { yylval.number = F_GT; return L_ORDER; }

"*="    { yylval.number = F_MULT_EQ; return L_ASSIGN; }
"*"     { return '*'; }

"%="    { yylval.number = F_MOD_EQ; return L_ASSIGN; }
"%"     { return '%'; }

"/="    { yylval.number = F_DIV_EQ; return L_ASSIGN; }
"/"     { return '/'; }

"=="    { return L_EQ; }
"="     { yylval.number = F_ASSIGN; return L_ASSIGN; }

"??="   { yylval.number = F_NULLISH_EQ; return L_ASSIGN; }
"??"    { return L_QUESTION_QUESTION; }
    /* Optional chaining "?." (mapping member/bracket access, e.g. m?.key,
     * m?.[idx]): only recognized when followed by an identifier-start char
     * or '[', so a ternary's float branch ("cond ? .5 : x") still lexes as
     * '?' then the ".5" float literal, not "?." followed by "5". */
"?."/[A-Za-z_\[]  { return L_OPTIONAL_DOT; }
"?"     { return '?'; }

"!="    { return L_NE; }
"!"     { return L_NOT; }

"::"    { return L_COLON_COLON; }
":"     { return ':'; }

"..."   { return L_DOT_DOT_DOT; }
".."    { return L_RANGE; }
    /* Optional chaining bracket form "m.?[idx]" -- the only grammar
     * production for L_DOT_OPTIONAL requires a following '[', so no
     * lookahead disambiguation is needed here the way "?." above needs one:
     * a literal '?' right after '.' was already a syntax error otherwise. */
".?"    { return L_DOT_OPTIONAL; }
"."     { return L_DOT; }

")"     { return ')'; }
    /* '{'/'}' inside a template literal's ${...} interpolation need brace-
     * depth tracking so the '}' that ends the interpolation (depth back to
     * 0) can be told apart from one that's merely part of the interpolated
     * expression itself. See SC_TEMPLATE_BODY above. */
"{"     {
          if (template_nesting > 0) {
            template_brace_depth[template_nesting]++;
          }
          return '{';
        }
"}"     {
          if (template_nesting > 0 && template_brace_depth[template_nesting] == 0) {
            str_accum.clear();
            template_is_continuation = true;
            BEGIN(SC_TEMPLATE_BODY);
            return yylex(yylval_param, yyscanner);
          }
          if (template_nesting > 0) {
            template_brace_depth[template_nesting]--;
          }
          return '}';
        }
"["     { return '['; }
"]"     { return ']'; }
";"     { return ';'; }
","     { return ','; }
"~"     { return '~'; }

<<EOF>> {
          // No text was matched (yyleng == 0) and Flex's own buffer
          // bookkeeping doesn't carry its usual "pending lookahead"
          // meaning at EOF, so don't apply the general rewind formula --
          // `outp` is already correctly positioned at the LEX_EOF sentinel.
          // parseEofOrIncludePop() may call yylex() recursively (on the
          // include-stack-pop path), so flush before calling, not after.
          YY_FLUSH_BUFFER;
          return parseEofOrIncludePop(yylval_param, yyscanner);
        }

.       {
          // Nothing above matched: genuinely invalid input. No outp
          // rewind/flush needed -- lpc_lex_badlex() only inspects the
          // character value, it doesn't touch outp or Flex's buffer.
          return lpc_lex_badlex(static_cast<unsigned char>(yytext[0]), yyscanner);
        }

%%

#undef str_accum
#undef template_nesting
#undef template_brace_depth
#undef template_is_continuation
#undef heredoc_terminator
#undef heredoc_is_array
#undef function_flag

void lpc_lex_reset(void *yyscanner) {
  struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
  if (YY_CURRENT_BUFFER) {
    YY_FLUSH_BUFFER;
  }
  BEGIN(INITIAL);
  compiler_context_t *ctx = reinterpret_cast<compiler_context_t *>(yyget_extra(yyscanner));
  ctx->template_is_continuation = false;
  ctx->template_nesting = 0;
  // A compile that aborted mid-splice can leave unconsumed blue-paint
  // counts; they must not suppress expansion in the next compile that
  // reuses this scanner.
  ctx->pending_plain.clear();
}

void lpc_lex_flush_lookahead(void *yyscanner) {
  struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
  outp -= YY_PENDING_LOOKAHEAD();
  YY_FLUSH_BUFFER;
}
