%{
/*
 * lex.l — Flex scanner for the LPC lexer.
 *
 * Whitespace, comments, operators/punctuation, numeric literals,
 * identifiers, '(' compound tokens (array/mapping/function-pointer open),
 * function-pointer $N params, char literals, string literals, and
 * '#pragma'/'#line' directives are all matched natively by Flex's DFA
 * (several via exclusive start conditions -- SC_FUNC_OPEN,
 * SC_CHAR_BODY/SC_CHAR_CLOSE, SC_BLOCK_COMMENT, SC_STRING_BODY,
 * SC_DIRECTIVE_PRAGMA/SC_DIRECTIVE_LINE/SC_DIRECTIVE_SKIP). Only heredocs
 * and EOF/include-stack popping still trigger explicit Flex rules that call
 * into standalone parse*() helpers in lexer_utils.cc, which keep their original
 * hand-written, raw-`outp`-reading algorithms unchanged -- a heredoc's
 * closing terminator is supplied at compile time by the LPC source itself
 * (e.g. "@END ... END"), not known until then, so it can't be expressed as
 * a static Flex pattern; the ring-buffer include-stack pop is pure state
 * bookkeeping, not really "scanning" at all. See
 * plans/flex-lexer-migration.md for the risk/value writeup and the
 * per-conversion deviation notes. Only genuinely invalid input reaches the
 * catch-all `.` rule at the bottom.
 *
 * Caveat on comments specifically: the standalone preprocessor
 * (preprocessor.cc) already strips both "/* ... * /"-style and "//"-style
 * comments from its output before the lexer ever runs (replacing a block
 * comment with a single space, eliding a line comment outright -- see
 * LpcPreprocessor::run()). So in the actual compile pipeline, a literal
 * "/*" essentially never reaches SC_BLOCK_COMMENT below; it exists for
 * self-consistency of the grammar (and any direct/non-preprocessed use of
 * the lexer) rather than as a load-bearing part of normal compilation.
 * String literals and '#pragma'/'#line' get no such treatment -- the
 * preprocessor passes quoted text through byte-for-byte (it only avoids
 * macro-expanding inside strings), and while it recognizes '#pragma'/
 * '#line' by name (unlike '#include'/'#define'/etc., which it fully
 * consumes), it re-emits them into its output rather than acting on them
 * -- it also synthesizes its own "#line N "file"" markers around every
 * #include. So both of those genuinely reach the rules below on any file
 * with a string literal or an #include.
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

#include <algorithm>
#include <cstdlib>
#include <string>

#include <unicode/ustring.h>

#include "compiler/internal/compiler.h"
#include "compiler/internal/lex.h"
#include "compiler/internal/grammar_rules.h"
#include "compiler/internal/grammar.autogen.h"
#include "compiler/internal/scratchpad.h"
#include "efuns.autogen.h"

// Strip '_' digit-group separators from a matched numeric token before
// handing it to strtoll/strtod (the Flex patterns below only allow '_'
// directly between two digits, so this is always safe).
static std::string lpc_strip_underscores(const char *text, int len) {
  std::string s(text, len);
  s.erase(std::remove(s.begin(), s.end(), '_'), s.end());
  return s;
}

// Accumulator for the string literal currently being scanned by
// SC_STRING_BODY below. Unlike the old parseStringLiteral()'s manual
// small-scratch/yytext-spillover dance, this is just a plain growable
// buffer -- the scratchpad allocation (scratch_copy()) happens once,
// at the closing quote, from whatever's been collected here.
static std::string str_accum;

// Roughly matches the old MAXLINE-based "String too long" cap (that cap
// existed because the original accumulated directly into a fixed-size
// buffer; str_accum has no such fixed size, but an unbounded string
// literal is still almost certainly malformed input, not something to
// scan forever). A macro, not a function, because it needs to `return`
// out of whichever Flex rule invokes it. Called after every append to
// str_accum below, not just the bulk-growth ones (ordinary-run/raw-
// newline): the original's cap was enforced by its `while (l--)` loop
// counting every outer iteration -- one escape sequence or one raw
// character each -- so a string built entirely out of short escapes (e.g.
// thousands of "\n") must hit the same cap a long run of ordinary text
// would, not sail past it because none of the small per-escape rules
// happened to check.
#define STR_CHECK_OVERFLOW()                                    \
  do {                                                          \
    if (static_cast<int>(str_accum.size()) > MAXLINE) {         \
      int lpc_str_tok_;                                         \
      lexerror("String too long");                              \
      if (!u8_validate(str_accum.c_str())) {                    \
        lexerror("Invalid UTF8 string");                        \
        lpc_str_tok_ = YYerror;                                 \
      } else {                                                  \
        yylval.string = scratch_copy(str_accum.c_str());        \
        lpc_str_tok_ = L_STRING;                                \
      }                                                         \
      BEGIN(INITIAL);                                           \
      return lpc_str_tok_;                                      \
    }                                                            \
  } while (0)

// Handles the tail of a "#line N ["file"]" or GCC-style "# N ["file"]"
// directive: `tail` is everything Flex matched starting right after the
// "line" keyword (whitespace and all -- the SC_DIRECTIVE_KEYWORD rule below
// deliberately does NOT consume the whitespace between "line" and the
// number itself, unlike the number for the GCC-style keyword-less form,
// which is pushed back in full including its own leading position), not
// including the newline. Mirrors the original parseDirective()'s behavior:
// if there's no leading digit run (after skipping leading whitespace, same
// as the original's own whitespace-skip-then-isdigit(p[0]) check), silently
// do nothing (not an error); the filename is read as raw bytes up to the
// next '"' with no escape processing, exactly like the original.
static void lpc_lex_handle_line_directive(const char *tail) {
  while (*tail == ' ' || *tail == '\t') tail++;
  if (!isdigit(static_cast<unsigned char>(*tail))) return;
  char *end = nullptr;
  long line_num = strtol(tail, &end, 10);
  if (end == tail) return;  // no leading digits at all
  current_line = static_cast<int>(line_num) - 1;

  const char *p = end;
  while (*p == ' ' || *p == '\t') p++;
  if (*p != '"') return;
  p++;
  const char *fname_start = p;
  while (*p != '"' && *p != '\0') p++;
  std::string fname(fname_start, p - fname_start);

  if (current_file) {
    free_string(const_cast<char *>(current_file));
  }
  current_file = make_shared_string(fname.c_str());
  current_file_id = add_program_file(fname.c_str(), 0);
}

// Accumulator for the terminator identifier of the heredoc currently being
// opened by SC_HEREDOC_TERM below (cleared by the "@"/"@@" trigger rule, so
// an immediately-empty terminator -- e.g. "@\n" -- is correctly detected
// even though the identifier rule itself never fires for it).
static std::string heredoc_terminator;
static bool heredoc_is_array = false;

// YY_INPUT: pull bytes from lexer_utils.cc's linked_buf_t ring via this bridge.
#define YY_INPUT(buf, result, max_size) \
    result = lpc_lex_yy_input(buf, max_size)

// Declared here; defined in lexer_utils.cc. Bridges Flex's buffer-fill requests to
// the existing ring-buffer, advancing `outp` for every byte handed to Flex.
extern "C" int lpc_lex_yy_input(char *buf, int max_size);

// Bytes Flex has already fetched into its buffer beyond the position it has
// actually matched/consumed so far (bulk pre-fetch and/or one-char DFA
// lookahead). See the file-level comment above for why this matters.
#define YY_PENDING_LOOKAHEAD() \
    ((yy_n_chars) - (int)((yy_c_buf_p) - YY_CURRENT_BUFFER_LVALUE->yy_ch_buf))

%}

%option noyywrap nounput noinput nodefault
%option never-interactive
%option 8bit
%option array

/* SC_FUNC_OPEN: after "(:" (function-pointer open), skip whitespace and
 * decide whether this is a named function reference "(: name" or an
 * anonymous function literal "(: expr :)". See the "(" rules below.
 * SC_CHAR_BODY / SC_CHAR_CLOSE: character literal 'c' -- exactly one body
 * char/escape, then a closing quote. See the "'" rule below.
 * SC_BLOCK_COMMENT: a C-style block comment. Unlike heredocs, the closing
 * delimiter is fixed (not a runtime-supplied terminator), so this is a good
 * native-Flex fit despite scanning an unbounded body. See the block-comment
 * open rule below.
 * SC_STRING_BODY: a double-quoted string literal. Like block comments, the
 * closing delimiter is fixed; unlike comments, the preprocessor does NOT
 * strip these before the lexer sees them (it only skips over quoted text
 * to avoid macro-expanding inside it), so this one is a genuinely reached
 * part of the grammar. See the '"' open rule below.
 * SC_DIRECTIVE_KEYWORD: the whitespace-delimited word right after '#'
 * (before it's known to be "pragma", "line", a GCC-style bare line number,
 * or unrecognized). SC_DIRECTIVE_PRAGMA / SC_DIRECTIVE_LINE: the tail of a
 * "#pragma ..." or "#line N [\"file\"]" directive (also reached for the
 * preprocessor's own GCC-style "# N [\"file\"]" line markers, emitted
 * around every #include). Unlike '#include'/'#define'/etc., the
 * preprocessor recognizes these two by name but re-emits them into its
 * output rather than consuming them, so they're genuinely reached here.
 * See the directive rules below.
 * SC_HEREDOC_TERM: the "@"/"@@" prefix and terminator identifier of a
 * heredoc (e.g. "@END" in "@END\nbody\nEND\n"). Only this *opening* piece
 * is native -- the heredoc *body* is matched against the terminator at
 * runtime (the terminator is supplied by the LPC source itself, not known
 * until compile time), which structurally can't be a static Flex pattern,
 * so that part stays a hand-written helper (parseHeredoc(), still in
 * lexer_utils.cc). See the heredoc rules below. */
%x SC_FUNC_OPEN
%x SC_CHAR_BODY
%x SC_CHAR_CLOSE
%x SC_BLOCK_COMMENT
%x SC_STRING_BODY
%x SC_DIRECTIVE_KEYWORD
%x SC_DIRECTIVE_PRAGMA
%x SC_DIRECTIVE_LINE
%x SC_DIRECTIVE_SKIP
%x SC_HEREDOC_TERM

%%

[ \t\f\v\r] { /* ignore */ }
\n          { lpc_lex_newline(); }

"/*"        { BEGIN(SC_BLOCK_COMMENT); }
"//"[^\n]*  { /* line comment; trailing newline handled by the \n rule */ }

    /* Block comment body: matched natively, no outp access. A lone slash
     * is kept out of the ordinary-run pattern below (it gets its own rule
     * instead) so that a nested comment-open sequence starting right after
     * some other character is still recognized as its own two-character
     * unit and triggers the warning, rather than the slash being swept up
     * together with the preceding character by the catch-all run. */
<SC_BLOCK_COMMENT>"/*"      { yywarn("/* found in comment."); }
<SC_BLOCK_COMMENT>"*"+"/"   { BEGIN(INITIAL); }
<SC_BLOCK_COMMENT>"*"+      { /* run of '*' not (yet) followed by the closing '/' */ }
<SC_BLOCK_COMMENT>\n        { current_line++; }
<SC_BLOCK_COMMENT>[^*/\n]+  { /* ordinary comment text */ }
<SC_BLOCK_COMMENT>"/"       { /* lone '/', not the start of a nested "/*" */ }
<SC_BLOCK_COMMENT><<EOF>>   {
              lexerror("End of file in a comment");
              BEGIN(INITIAL);
            }

"0"[xX][0-9a-fA-F](_?[0-9a-fA-F])* {
              auto s = lpc_strip_underscores(yytext + 2, yyleng - 2);
              yylval.number = strtoll(s.c_str(), nullptr, 16);
              return L_NUMBER;
            }
"0"[bB][01](_?[01])* {
              auto s = lpc_strip_underscores(yytext + 2, yyleng - 2);
              yylval.number = strtoll(s.c_str(), nullptr, 2);
              return L_NUMBER;
            }
[0-9](_?[0-9])*"."(_?[0-9])+ {
              auto s = lpc_strip_underscores(yytext, yyleng);
              yylval.real = strtod(s.c_str(), nullptr);
              return L_REAL;
            }
%{
    /* A trailing '.' with no fraction digits (e.g. "1.") is still a valid
     * float -- but only when NOT immediately followed by another '.', which
     * must instead be left for the ".." range operator (e.g. "1..5" is
     * NUMBER(1) RANGE NUMBER(5), not REAL(1.0) DOT NUMBER(5)). */
%}
[0-9](_?[0-9])*"."/[^.] {
              auto s = lpc_strip_underscores(yytext, yyleng);
              yylval.real = strtod(s.c_str(), nullptr);
              return L_REAL;
            }
[0-9](_?[0-9])* {
              auto s = lpc_strip_underscores(yytext, yyleng);
              yylval.number = strtoll(s.c_str(), nullptr, 10);
              return L_NUMBER;
            }

[a-zA-Z_][a-zA-Z0-9_]* { return lpc_lex_resolve_identifier(); }

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

<SC_STRING_BODY>[^\\\"\n]+ {
          str_accum.append(yytext, yyleng);
          STR_CHECK_OVERFLOW();
        }
<SC_STRING_BODY>\n {
          current_line++;
          total_lines++;
          str_accum += '\n';
          STR_CHECK_OVERFLOW();
        }
<SC_STRING_BODY>"\""   {
          BEGIN(INITIAL);
          if (!u8_validate(str_accum.c_str())) {
            lexerror("Invalid UTF8 codepoint in string literal");
            return YYerror;
          }
          yylval.string = scratch_copy(str_accum.c_str());
          return L_STRING;
        }
<SC_STRING_BODY><<EOF>> {
          lexerror("End of file in string");
          BEGIN(INITIAL);
          return YYEOF;
        }

<SC_STRING_BODY>"\\\n"  { current_line++; total_lines++; }
<SC_STRING_BODY>"\\n"   { str_accum += '\n'; STR_CHECK_OVERFLOW(); }
<SC_STRING_BODY>"\\t"   { str_accum += '\t'; STR_CHECK_OVERFLOW(); }
<SC_STRING_BODY>"\\r"   { str_accum += '\r'; STR_CHECK_OVERFLOW(); }
<SC_STRING_BODY>"\\b"   { str_accum += '\b'; STR_CHECK_OVERFLOW(); }
<SC_STRING_BODY>"\\a"   { str_accum += '\x07'; STR_CHECK_OVERFLOW(); }
<SC_STRING_BODY>"\\e"   { str_accum += '\x1b'; STR_CHECK_OVERFLOW(); }
<SC_STRING_BODY>"\\\""  { str_accum += '"'; STR_CHECK_OVERFLOW(); }
<SC_STRING_BODY>"\\\\"  { str_accum += '\\'; STR_CHECK_OVERFLOW(); }
<SC_STRING_BODY>"\\"[0-7]+ {
          long long tmp = strtoll(yytext + 1, nullptr, 8);
          if (tmp > 255) {
            yywarn("Illegal character constant in string.");
            tmp = 'x';
          }
          str_accum += static_cast<char>(tmp);
          STR_CHECK_OVERFLOW();
        }
    /* Deviation from the original (documented in the migration plan): '\8'
     * and '\9' aren't valid octal digits. The original's strtoll() there
     * consumed zero characters and left them unconsumed, which -- via a
     * chain of implicit fallthrough -- appended a NUL byte to the string
     * followed by a literal '8'/'9', an obscure and almost certainly
     * unintended embedded-NUL quirk. Reported directly as an error here
     * instead of replicating that. */
<SC_STRING_BODY>"\\"[89]  { lexerror("Illegal octal escape in string."); }
<SC_STRING_BODY>"\\x"[0-9a-fA-F]+ {
          long long tmp = strtoll(yytext + 2, nullptr, 16);
          if (tmp > 255) {
            yywarn("Illegal character constant.");
            tmp = 'x';
          }
          str_accum += static_cast<char>(tmp);
          STR_CHECK_OVERFLOW();
        }
<SC_STRING_BODY>"\\x"   {
          str_accum += 'x';
          yywarn("\\x must be followed by a valid hex value; interpreting as 'x' instead.");
          STR_CHECK_OVERFLOW();
        }
    /* \uXXXX\uYYYY: a UTF-16 surrogate pair, matched and decoded as one
     * unit (the lead's second hex digit is 8-B, the trail's is C-F --
     * exactly the D800-DBFF / DC00-DFFF ranges). Ordered before the lone
     * "\uXXXX" rule below only for readability; Flex's longest-match rule
     * already prefers this 12-character form over the 6-character one
     * whenever both could apply. */
<SC_STRING_BODY>"\\u"[dD][89abAB][0-9a-fA-F][0-9a-fA-F]"\\u"[dD][c-fC-F][0-9a-fA-F][0-9a-fA-F] {
          UChar pair[2];
          char lead_buf[5] = {yytext[2], yytext[3], yytext[4], yytext[5], 0};
          char trail_buf[5] = {yytext[8], yytext[9], yytext[10], yytext[11], 0};
          pair[0] = static_cast<UChar>(strtol(lead_buf, nullptr, 16));
          pair[1] = static_cast<UChar>(strtol(trail_buf, nullptr, 16));
          char utf8[4];
          UErrorCode err = U_ZERO_ERROR;
          int32_t written = 0;
          u_strToUTF8(utf8, sizeof(utf8), &written, pair, 2, &err);
          if (U_FAILURE(err)) {
            lexerror("Illegal unicode sequence.");
          } else {
            str_accum.append(utf8, written);
            STR_CHECK_OVERFLOW();
          }
        }
    /* A lone \uXXXX not part of a valid pair above: either a genuine
     * single (non-surrogate) code point, or a malformed/unpaired
     * surrogate. Deviation from the original (documented in the migration
     * plan): several of the original's distinct error messages for the
     * different ways a surrogate can fail to pair up are consolidated
     * into these two -- nobody writes malformed surrogate pairs on
     * purpose, and both still report a clear error and recover. */
<SC_STRING_BODY>"\\u"[0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F] {
          UChar code = static_cast<UChar>(strtol(yytext + 2, nullptr, 16));
          if (U16_IS_SINGLE(code)) {
            char utf8[4];
            UErrorCode err = U_ZERO_ERROR;
            int32_t written = 0;
            u_strToUTF8(utf8, sizeof(utf8), &written, &code, 1, &err);
            if (U_FAILURE(err)) {
              lexerror("Illegal unicode sequence.");
            } else {
              str_accum.append(utf8, written);
              STR_CHECK_OVERFLOW();
            }
          } else if (U16_IS_SURROGATE_LEAD(code)) {
            lexerror("Illegal unicode sequence. Missing surrogate trail.");
          } else {
            lexerror("Illegal unicode sequence, expecting surrogate lead, got trail.");
          }
        }
<SC_STRING_BODY>"\\u"   { lexerror("Illegal unicode sequence."); }
<SC_STRING_BODY>"\\U"[0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F] {
          UChar res[2];
          auto size = u_unescape(yytext, res, 2);
          if (size == 0) {
            lexerror("Illegal unicode sequence.");
          } else {
            char utf8[4];
            UErrorCode err = U_ZERO_ERROR;
            int32_t written = 0;
            u_strToUTF8(utf8, sizeof(utf8), &written, res, size, &err);
            if (U_FAILURE(err)) {
              lexerror("Illegal unicode sequence.");
            } else {
              str_accum.append(utf8, written);
              STR_CHECK_OVERFLOW();
            }
          }
        }
<SC_STRING_BODY>"\\U"   { lexerror("Illegal unicode sequence."); }
<SC_STRING_BODY>"\\".   {
          str_accum += yytext[1];
          yywarn("Unknown \\ escape.");
          STR_CHECK_OVERFLOW();
        }

    /* Character literals 'c': fully native via two exclusive start
     * conditions -- SC_CHAR_BODY matches exactly one escape sequence or raw
     * byte, SC_CHAR_CLOSE then requires the closing quote. No outp access
     * at all: unlike strings/heredocs, a char literal's body is exactly one
     * grammar unit, so there's no scratchpad allocator or multi-byte
     * accumulation for Flex to sidestep. */
"'"                     { BEGIN(SC_CHAR_BODY); }
<SC_CHAR_BODY>"\\n"     { yylval.number = '\n'; BEGIN(SC_CHAR_CLOSE); }
<SC_CHAR_BODY>"\\t"     { yylval.number = '\t'; BEGIN(SC_CHAR_CLOSE); }
<SC_CHAR_BODY>"\\r"     { yylval.number = '\r'; BEGIN(SC_CHAR_CLOSE); }
<SC_CHAR_BODY>"\\b"     { yylval.number = '\b'; BEGIN(SC_CHAR_CLOSE); }
<SC_CHAR_BODY>"\\a"     { yylval.number = '\x07'; BEGIN(SC_CHAR_CLOSE); }
<SC_CHAR_BODY>"\\e"     { yylval.number = '\x1b'; BEGIN(SC_CHAR_CLOSE); }
<SC_CHAR_BODY>"\\'"     { yylval.number = '\''; BEGIN(SC_CHAR_CLOSE); }
<SC_CHAR_BODY>"\\\""    { yylval.number = '"'; BEGIN(SC_CHAR_CLOSE); }
<SC_CHAR_BODY>"\\\\"    { yylval.number = '\\'; BEGIN(SC_CHAR_CLOSE); }
<SC_CHAR_BODY>"\\"[0-7]+ {
          yylval.number = strtoll(yytext + 1, nullptr, 8);
          if (yylval.number > 255) {
            yywarn("Illegal character constant.");
            yylval.number = 'x';
          }
          BEGIN(SC_CHAR_CLOSE);
        }
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
<SC_CHAR_BODY>"\\x"[0-9a-fA-F]+ {
          yylval.number = strtoll(yytext + 2, nullptr, 16);
          if (yylval.number > 255) {
            yywarn("Illegal character constant.");
            yylval.number = 'x';
          }
          BEGIN(SC_CHAR_CLOSE);
        }
<SC_CHAR_BODY>"\\x"     {
          yylval.number = 'x';
          yywarn("\\x must be followed by a valid hex value; interpreting as 'x' instead.");
          BEGIN(SC_CHAR_CLOSE);
        }
<SC_CHAR_BODY>"\\\n"    {
          // No refill_buffer() call needed here (unlike the legacy
          // raw-outp helpers): YY_INPUT already refills the ring buffer for
          // every byte Flex consumes, including these two.
          yylval.number = '\n';
          current_line++;
          total_lines++;
          BEGIN(SC_CHAR_CLOSE);
        }
<SC_CHAR_BODY>"\\".     {
          yywarn("Unknown \\ escape.");
          yylval.number = static_cast<unsigned char>(yytext[1]);
          BEGIN(SC_CHAR_CLOSE);
        }
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
<SC_CHAR_BODY><<EOF>>  {
          lexerror("Illegal character constant");
          yylval.number = 0;
          BEGIN(INITIAL);
          return L_NUMBER;
        }
<SC_CHAR_CLOSE>"'"     { BEGIN(INITIAL); return L_NUMBER; }
<SC_CHAR_CLOSE><<EOF>> {
          // The original's closing-quote check (`if (*outp++ != '\'')`)
          // resets yylval.number to 0 unconditionally on ANY failure to
          // find the closing quote, EOF included (its raw-pointer style
          // doesn't distinguish EOF from any other unexpected byte there).
          // Bug found during a self-review pass: this rule used to leave
          // yylval.number at whatever the body char/escape had already
          // set it to instead of resetting it, observable via e.g. a
          // source consisting of just "'" -- refill_buffer() synthesizes
          // a trailing newline, which SC_CHAR_BODY's [^\\] rule consumes
          // as the (raw) body character before genuine EOF is reached
          // here, so yylval.number was left at '\n' (10) instead of 0.
          lexerror("Illegal character constant");
          yylval.number = 0;
          BEGIN(INITIAL);
          return L_NUMBER;
        }
<SC_CHAR_CLOSE>.|\n    {
          yyless(0);
          lexerror("Illegal character constant");
          yylval.number = 0;
          BEGIN(INITIAL);
          return L_NUMBER;
        }

    /* $N function-pointer parameters: fully native, no outp access at all
     * -- unlike the string/char/paren rules above, this token's grammar
     * (an optional run of digits) has no line-counting or ring-buffer
     * interaction, so there's nothing left for a legacy helper to do that
     * Flex's own DFA can't do directly. */
"$"[0-9]+ {
          if (!current_function_context) {
            lexerror("$var illegal outside of function pointer.");
            return YYerror;
          }
          if (current_function_context->num_parameters < 0) {
            lexerror("$var illegal inside anonymous function pointer.");
            return YYerror;
          }
          if (yyleng - 1 > MAXLINE - 5) {
            lexerror("Line too long");
            return yylex();
          }
          yylval.number = atoll(yytext + 1) - 1;
          if (yylval.number < 0) {
            lexerror("In function parameter $num, num must be >= 1.");
          } else if (yylval.number > 254) {
            lexerror("only 255 parameters allowed.");
          } else if (yylval.number >= current_function_context->num_parameters) {
            current_function_context->num_parameters = yylval.number + 1;
          }
          return L_PARAMETER;
        }
"$"     {
          if (!current_function_context) {
            lexerror("$var illegal outside of function pointer.");
            return YYerror;
          }
          if (current_function_context->num_parameters < 0) {
            lexerror("$var illegal inside anonymous function pointer.");
            return YYerror;
          }
          return '$';
        }

    /* '(' compound tokens: "({" (array), "([" (mapping), "(:"/"(::"
     * (function-pointer open vs. plain '(' followed by the "::" scope
     * operator). Whitespace (but not comments) may freely separate '(' from
     * the following '{'/'['/ ':'. */
"("[ \t\r\n\v\f]*"{" {
          for (int i = 1; i < yyleng - 1; i++) {
            if (yytext[i] == '\n') current_line++;
          }
          return L_ARRAY_OPEN;
        }
"("[ \t\r\n\v\f]*"[" {
          for (int i = 1; i < yyleng - 1; i++) {
            if (yytext[i] == '\n') current_line++;
          }
          return L_MAPPING_OPEN;
        }
"("[ \t\r\n\v\f]*"::" {
          // '(' followed by the "::" scope operator, not a function-pointer
          // open. Put everything but '(' back -- whitespace included -- and
          // let "::" scan as its own L_COLON_COLON token next; Flex's own
          // whitespace/newline rules re-tokenize any pushed-back whitespace.
          yyless(1);
          return '(';
        }
"("[ \t\r\n\v\f]*":" {
          for (int i = 1; i < yyleng - 1; i++) {
            if (yytext[i] == '\n') current_line++;
          }
          BEGIN(SC_FUNC_OPEN);
        }
"("     { return '('; }

    /* Tail of "(:" -- skip further whitespace, then either a named
     * function-pointer reference "(: name" or an anonymous function literal
     * "(: expr :)" (anything else, pushed back for the next yylex() call). */
<SC_FUNC_OPEN>[ \t\r\n\v\f]+ {
          for (int i = 0; i < yyleng; i++) {
            if (yytext[i] == '\n') current_line++;
          }
        }
<SC_FUNC_OPEN>[a-zA-Z_][a-zA-Z0-9_]* {
          // Same shape as the '"'/"'"/"(" rules above: with function_flag
          // set, lpc_lex_resolve_identifier() reads one more byte straight
          // from `outp` (to disambiguate "(: foo ," from "(: foo(...)"), so
          // it needs the same pending-lookahead correction and buffer flush.
          // (The plain identifier rule earlier in this file never needs
          // this: function_flag is always 0 there -- see the Phase 4
          // deviation notes in plans/flex-lexer-migration.md.)
          BEGIN(INITIAL);
          function_flag = 1;
          outp -= YY_PENDING_LOOKAHEAD();
          int result = lpc_lex_resolve_identifier();
          YY_FLUSH_BUFFER;
          return result;
        }
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

    /* '#pragma'/'#line' directives: fully native, no outp access. Anchored
     * to the start of a line -- replaces the hand-written scanner's
     * `*(outp - 2) == '\n'` check. A '#' NOT at the start of a line simply
     * won't match this rule and falls through to the generic `.` catch-all
     * at the bottom, which reports it as illegal (matching the original's
     * `else { goto badlex; }`).
     *
     * '#include'/'#define'/etc. never reach here: the preprocessor fully
     * consumes those. It recognizes '#pragma'/'#line' by name too, but
     * re-emits them verbatim rather than consuming them (and synthesizes
     * its own "#line N "file"" markers around every #include), so these
     * rules are genuinely exercised on any file with an #include.
     *
     * The keyword itself is recognized by first capturing the whole
     * whitespace-delimited word after '#' (SC_DIRECTIVE_KEYWORD below),
     * exactly like the original's identifier-scan-loop-then-strcmp, rather
     * than matching a fixed "pragma"/"line" string directly -- a fixed
     * string match has no way to require a word boundary afterward, so it
     * would wrongly treat e.g. "#pragmatic" or "#line123" as "#pragma"/
     * "#line" with a garbled argument instead of an unrecognized directive
     * (bug caught during a self-review pass; also unreachable in the real
     * pipeline today, since the preprocessor's own directive-name
     * extraction is exact-word-matched and would reject those as unknown
     * directives before the lexer ever saw them -- fixed anyway, for the
     * same "grammar self-consistency" reasoning as SC_BLOCK_COMMENT). */
^"#"[ \t\v\f\r]* { BEGIN(SC_DIRECTIVE_KEYWORD); }

<SC_DIRECTIVE_KEYWORD>[^ \t\v\f\r\n]+ {
          if (strcmp(yytext, "pragma") == 0) {
            BEGIN(SC_DIRECTIVE_PRAGMA);
          } else if (strcmp(yytext, "line") == 0) {
            BEGIN(SC_DIRECTIVE_LINE);
          } else if (isdigit(static_cast<unsigned char>(yytext[0]))) {
            // GCC-style "# N ..." line marker (no "line" keyword): push
            // the whole captured word back so SC_DIRECTIVE_LINE's own tail
            // capture includes the number itself, exactly as if it had
            // matched there directly.
            yyless(0);
            BEGIN(SC_DIRECTIVE_LINE);
          } else {
            // Unrecognized directive keyword: silently skipped, matching
            // the original's fallthrough when neither "pragma"/"line" nor
            // a leading digit matched.
            BEGIN(SC_DIRECTIVE_SKIP);
          }
        }
<SC_DIRECTIVE_KEYWORD>\n {
          // Empty keyword (e.g. a bare "#" or "#   " with nothing else on
          // the line): push the newline back so the top-level "\n" rule
          // handles it normally, matching the original's own "put the
          // newline back" dance at the end of parseDirective().
          yyless(0);
          BEGIN(INITIAL);
        }
<SC_DIRECTIVE_KEYWORD><<EOF>> { BEGIN(INITIAL); }

<SC_DIRECTIVE_PRAGMA>[^\n]* {
          // Skip leading whitespace robustly (a minor, documented
          // improvement over the original, which only skipped a single
          // whitespace character between "pragma" and its argument).
          char *p = yytext;
          while (*p == ' ' || *p == '\t') p++;
          handle_pragma(p);
          BEGIN(INITIAL);
        }
<SC_DIRECTIVE_PRAGMA><<EOF>> {
          handle_pragma(const_cast<char *>(""));
          BEGIN(INITIAL);
        }

<SC_DIRECTIVE_LINE>[^\n]* {
          lpc_lex_handle_line_directive(yytext);
          BEGIN(INITIAL);
        }
<SC_DIRECTIVE_LINE><<EOF>> { BEGIN(INITIAL); }

<SC_DIRECTIVE_SKIP>[^\n]* { BEGIN(INITIAL); }
<SC_DIRECTIVE_SKIP><<EOF>> { BEGIN(INITIAL); }

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
<SC_HEREDOC_TERM>[ \t\v\f\r]*\n {
          current_line++;
          BEGIN(INITIAL);
          if (heredoc_terminator.empty()) {
            lexerror("Illegal terminator");
            return yylex();
          }
          outp -= YY_PENDING_LOOKAHEAD();
          YY_FLUSH_BUFFER;
          return parseHeredoc(heredoc_terminator.c_str(), heredoc_is_array);
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
            return yylex();
          }
          outp -= YY_PENDING_LOOKAHEAD();
          YY_FLUSH_BUFFER;
          return parseHeredoc(heredoc_terminator.c_str(), heredoc_is_array);
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
"?"     { return '?'; }

"!="    { return L_NE; }
"!"     { return L_NOT; }

"::"    { return L_COLON_COLON; }
":"     { return ':'; }

"..."   { return L_DOT_DOT_DOT; }
".."    { return L_RANGE; }
"."     { return L_DOT; }

")"     { return ')'; }
"{"     { return '{'; }
"}"     { return '}'; }
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
          return parseEofOrIncludePop();
        }

.       {
          // Nothing above matched: genuinely invalid input. No outp
          // rewind/flush needed -- lpc_lex_badlex() only inspects the
          // character value, it doesn't touch outp or Flex's buffer.
          return lpc_lex_badlex(static_cast<unsigned char>(yytext[0]));
        }

%%
