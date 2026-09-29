#ifndef COMPILER_INTERNAL_LEXER_SCAN_H
#define COMPILER_INTERNAL_LEXER_SCAN_H

/*
 * ONE definition of "am I inside a literal or a comment right now?", for
 * the code that has to answer it without the DFA.
 *
 * Two kinds of caller are left, and neither can just use lexer.l:
 *
 *   - the function-like-macro argument collector (lexer_utils.cc) reads
 *     raw characters mid-rule through lpc_lex_getc(), because it must
 *     collect argument TEXT rather than tokens -- '#' hands a parameter
 *     its raw source spelling (AGENTS.md 7);
 *   - collect_args(), substitute() and lpc_lex_expand_string()
 *     (lexer_rules_pp.cc) walk text that was captured earlier, during
 *     EXPANSION, when no scanner is running over it at all. They need the
 *     same answer -- a ',' inside a literal is not an argument separator,
 *     and a parameter or macro name inside one is not substituted or
 *     expanded -- and they cannot get it from the DFA.
 *
 * Every one of them used to carry its own hand-rolled version, and they
 * disagreed with each other and with lexer.l. The disagreements were
 * bugs: an apostrophe in a comment inside an argument list opened a
 * character literal that ran to end of file (#1362), and an apostrophe
 * anywhere on a directive line hid a block comment that spanned lines, so
 * the comment's continuation was compiled as code. The same mistake, made
 * independently -- treating a lone "'" as a quote that scans forward for
 * a partner.
 *
 * The DFA does not do that. In lexer.l a character literal is SC_CHAR_BODY
 * (exactly one escape or one raw byte) followed by SC_CHAR_CLOSE (the
 * closing quote, or an error that pushes the offending byte back). It can
 * never consume a line, let alone a file. A '"' string and a '`' template
 * DO run to their closing quote and may span lines. That asymmetry is the
 * whole of the rule, and scan_one_unit() below is its single statement of
 * it.
 *
 * This is a SECOND statement of rules whose first statement is lexer.l,
 * so it is checked rather than trusted:
 * Preprocessor.ClassifierAgreesWithTheScannerOnRandomInput compares the
 * two on random input the DFA accepts. The directive reader that used to
 * be the third caller is gone -- SC_DIRECTIVE scans directives with real
 * Flex rules -- and the collector could follow it one day, but the
 * expansion-time walkers above never can.
 */

#include <cstddef>

namespace lpc_lex {

enum class Unit {
  kEof,
  kOrdinary,      // a single character that begins no literal or comment
  kString,        // "..."  -- may span lines
  kTemplate,      // `...`  -- may span lines
  kChar,          // '...'  -- one escape or one byte, then the close quote
  kBlockComment,  // slash-star .. star-slash
  kLineComment,   // // .. up to (NOT including) the newline that ends it
};

struct UnitInfo {
  Unit unit = Unit::kEof;
  // Set when end of input arrived before the literal or comment closed.
  // A caller that must diagnose it (an argument list, a directive line)
  // reads this; one that only classifies text can ignore it.
  bool unterminated = false;
  // Raw newlines consumed as part of this unit. Callers that maintain
  // total_lines add it; current_line is native (yylineno) and needs none.
  int newlines = 0;
  // The character a kOrdinary unit consumed; callers dispatch on it (an
  // argument separator, a brace, the newline that ends a directive).
  char ch = 0;
};


// The character a kOrdinary unit consumed, for callers that dispatch on it.
// (Set only for kOrdinary; other units expose their raw text through keep().)

/*
 * Consume exactly one lexical unit from `src` and describe it.
 *
 * `src` supplies lookahead and consumption, and never the reverse:
 *     int  peek(int k) const;  // k-th upcoming byte (0 = next), <= 0 at end
 *     void advance();          // consume peek(0), handing it to keep()
 *     void keep(int c);        // every consumed byte, in order; may be a no-op
 *
 * A peek NEVER removes anything from the input. That is the whole point:
 * deciding whether a "'" opens a character literal takes several bytes of
 * lookahead, and a design that reads them and hands back what it did not
 * want can leave bytes stranded when the caller stops mid-lookahead -- which
 * is a silent input-corruption bug, not a lexing one. Here a byte is
 * consumed only once its unit is known, so a caller can stop anywhere.
 *
 * Every consumed byte reaches keep() in order, including quotes and comment
 * delimiters, so a caller rebuilding text gets the raw spelling and one that
 * only classifies makes keep() a no-op.
 *
 * Comment recognition happens only where the DFA does it: never inside a
 * string, a template, or a character literal.
 */

namespace detail {

inline bool is_octal(int c) { return c >= '0' && c <= '7'; }
inline bool is_hex(int c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

// Length of the character-literal BODY starting at offset `at`, or 0 if what
// follows cannot be one. Mirrors lexer.l's SC_CHAR_BODY rules exactly: one
// escape ("\\x" + hex digits, "\\" + octal digits, "\\\r\n", "\\" + any) or
// one raw byte. No cap: the DFA accepts an over-long escape (with a warning),
// so treating it as "not a literal" here would be a disagreement.
template <class Src>
int char_body_length(const Src& src, int at) {
  int b = src.peek(at);
  if (b <= 0) return 0;
  if (b != '\\') return 1;
  int e = src.peek(at + 1);
  if (e <= 0) return 0;
  if (e == 'x') {
    int n = at + 2;
    while (is_hex(src.peek(n))) n++;
    return n - at;  // a bare "\x" is still one escape (the DFA reports it)
  }
  if (is_octal(e)) {
    int n = at + 2;
    while (is_octal(src.peek(n))) n++;
    return n - at;
  }
  if (e == '\r' && src.peek(at + 2) == '\n') return 3;  // SC_CHAR_BODY "\\\r\n"
  return 2;
}

}  // namespace detail

// Nested "${...}" inside a template may contain further templates. The DFA
// tracks that with a brace stack; this recurses, so it needs a bound. Input
// nested deeper than this is reported unterminated, which makes the caller
// diagnose it rather than silently mis-split an argument list.
constexpr int kMaxTemplateDepth = 64;

template <class Src>
UnitInfo scan_one_unit(Src& src, int depth = 0);

// The body of a block comment whose opening delimiter is already consumed.
// Internal to scan_one_unit(); it was once an entry point of its own, for a
// directive reader that was handed control mid-comment. SC_DIRECTIVE does
// that with yy_push_state(SC_BLOCK_COMMENT) now.
namespace detail {
template <class Src>
UnitInfo finish_block_comment(Src& src) {
  UnitInfo info;
  info.unit = Unit::kBlockComment;
  int prev = 0;
  for (;;) {
    int cc = src.peek(0);
    if (cc <= 0) {
      info.unterminated = true;
      return info;
    }
    src.advance();
    if (cc == '\n') {
      info.newlines++;
      prev = 0;
      continue;
    }
    // "/*/" cannot close on the '/' that opened it, so prev is cleared after
    // every match rather than carried into the next comparison.
    if (prev == '*' && cc == '/') return info;
    if (prev == '/' && cc == '*') {
      prev = 0;  // lexer.l's SC_BLOCK_COMMENT warns here; nothing reads it
      continue;
    }
    prev = cc;
  }
}

// The body of a line comment whose opening delimiter is already consumed.
// The newline that ends it is left unconsumed: the DFA's line-comment pattern
// leaves it to the newline rule, and callers need to see it (it terminates a
// directive line, and it is the whitespace that separates macro arguments).
// End of input ends a line comment normally -- the DFA's pattern matches an
// empty tail -- so this is never "unterminated".
template <class Src>
UnitInfo finish_line_comment(Src& src) {
  UnitInfo info;
  info.unit = Unit::kLineComment;
  for (;;) {
    int cc = src.peek(0);
    if (cc <= 0 || cc == '\n') return info;
    src.advance();
  }
}

}  // namespace detail

template <class Src>
UnitInfo scan_one_unit(Src& src, int depth) {
  UnitInfo info;

  int c = src.peek(0);
  if (c <= 0) {
    info.unit = Unit::kEof;
    return info;
  }

  // --- comments ----------------------------------------------------------
  if (c == '/') {
    int c2 = src.peek(1);
    if (c2 == '*') {
      src.advance();
      src.advance();
      return detail::finish_block_comment(src);
    }
    if (c2 == '/') {
      src.advance();
      src.advance();
      return detail::finish_line_comment(src);
    }
    src.advance();  // a lone '/': division, or "/="
    info.unit = Unit::kOrdinary;
    info.ch = static_cast<char>(c);
    return info;
  }

  // --- strings and templates: run to their closing quote, may span lines --
  if (c == '"' || c == '`') {
    info.unit = (c == '"') ? Unit::kString : Unit::kTemplate;
    src.advance();
    for (;;) {
      int cc = src.peek(0);
      if (cc <= 0) {
        info.unterminated = true;
        return info;
      }
      if (cc == '\\') {
        src.advance();
        int esc = src.peek(0);
        if (esc <= 0) {
          info.unterminated = true;
          return info;
        }
        src.advance();
        if (esc == '\n') info.newlines++;
        continue;
      }
      if (cc == c) {
        src.advance();
        return info;
      }
      // A template's "${" opens an expression that is ordinary code -- and
      // may hold another template. The DFA pairs the braces with its own
      // stack (lpc_lex_brace_open/close); this recurses through the same
      // classifier so a nested `...` inside ${...} cannot end the outer one.
      if (info.unit == Unit::kTemplate && cc == '$' && src.peek(1) == '{') {
        if (depth >= kMaxTemplateDepth) {
          info.unterminated = true;
          return info;
        }
        src.advance();
        src.advance();
        int braces = 1;
        while (braces > 0) {
          UnitInfo inner = scan_one_unit(src, depth + 1);
          if (inner.unit == Unit::kEof || inner.unterminated) {
            info.unterminated = true;
            info.newlines += inner.newlines;
            return info;
          }
          info.newlines += inner.newlines;
          if (inner.unit == Unit::kOrdinary) {
            if (inner.ch == '{') braces++;
            if (inner.ch == '}') braces--;
          }
        }
        continue;
      }
      src.advance();
      if (cc == '\n') info.newlines++;
    }
  }

  // --- character literals ------------------------------------------------
  // One escape or one raw byte, then the closing quote -- and if that quote
  // is not there, this was never a character literal. The apostrophe is then
  // just a character and everything after it is scanned normally, which is
  // what the DFA's SC_CHAR_CLOSE error path amounts to. Nothing beyond the
  // apostrophe is consumed unless the close is there, so a "don't" costs one
  // character, not a line and not a file.
  if (c == '\'') {
    int len = detail::char_body_length(src, 1);
    if (len > 0 && src.peek(1 + len) == '\'') {
      info.unit = Unit::kChar;
      for (int i = 0; i < len + 2; i++) {
        if (src.peek(0) == '\n') info.newlines++;
        src.advance();
      }
      return info;
    }
    src.advance();
    info.unit = Unit::kOrdinary;
    info.ch = '\'';
    return info;
  }

  // --- anything else -----------------------------------------------------
  src.advance();
  info.unit = Unit::kOrdinary;
  info.ch = static_cast<char>(c);
  if (c == '\n') info.newlines++;
  return info;
}

}  // namespace lpc_lex

#endif  // COMPILER_INTERNAL_LEXER_SCAN_H
