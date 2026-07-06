#include "base/std.h"

#include "compiler/internal/lexer_rules.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <string>

#include <unicode/ustring.h>

#include "compiler/internal/compiler.h"
#include "compiler/internal/lex.h"
#include "compiler/internal/grammar_rules.h"
#include "compiler/internal/grammar.autogen.h"
#include "compiler/internal/scratchpad.h"

// See lex.l's STR_CHECK_OVERFLOW comment: this cap roughly matches the old
// MAXLINE-based "String too long" limit.
namespace {

compiler_context_t *ctx_of(void *yyscanner) {
  return reinterpret_cast<compiler_context_t *>(yyget_extra(yyscanner));
}

// Shared by every string/template escape rule: appends one decoded char to
// whichever accumulator the current scan target uses.
void append_char(void *yyscanner, char c) { ctx_of(yyscanner)->str_accum += c; }
void append_str(void *yyscanner, const char *s, size_t n) {
  ctx_of(yyscanner)->str_accum.append(s, n);
}

const char *literal_kind(bool is_template) { return is_template ? "template literal" : "string"; }

}  // namespace

std::string lpc_strip_underscores(const char *text, int len) {
  std::string s(text, len);
  s.erase(std::remove(s.begin(), s.end(), '_'), s.end());
  return s;
}

void lpc_lex_brace_open(void *yyscanner) {
  compiler_context_t *ctx = yyget_extra(yyscanner);
  if (ctx->template_nesting > 0) {
    ctx->template_brace_depth[ctx->template_nesting]++;
  }
}

void lpc_lex_reset_context(struct compiler_context_t *ctx) {
  ctx->template_is_continuation = false;
  ctx->template_nesting = 0;
  // An aborted compile can strand the #if evaluator's suppression flag;
  // it must not disable macro expansion for the next compile.
  ctx->suppress_expansion = false;
}

bool lpc_lex_brace_close(void *yyscanner) {
  compiler_context_t *ctx = yyget_extra(yyscanner);
  if (ctx->template_nesting > 0 && ctx->template_brace_depth[ctx->template_nesting] == 0) {
    // This '}' ends the ${...} interpolation: resume the template body
    // with a fresh fragment accumulator.
    ctx->str_accum.clear();
    ctx->template_is_continuation = true;
    return true;
  }
  if (ctx->template_nesting > 0) {
    ctx->template_brace_depth[ctx->template_nesting]--;
  }
  return false;
}

void lpc_lex_count_newlines(const char *text, int len) {
  for (int i = 0; i < len; i++) {
    if (text[i] == '\n') {
      total_lines++;  // the line counter itself advances natively
                      // (%option yylineno scans the matched text)
    }
  }
}

// ---------------------------------------------------------------------------
// Numeric literals
// ---------------------------------------------------------------------------

int lpc_lex_number_hex(union YYSTYPE *yylval_param, const char *text, int len) {
  auto s = lpc_strip_underscores(text + 2, len - 2);
  yylval_param->number = strtoll(s.c_str(), nullptr, 16);
  return L_NUMBER;
}

int lpc_lex_number_bin(union YYSTYPE *yylval_param, const char *text, int len) {
  auto s = lpc_strip_underscores(text + 2, len - 2);
  yylval_param->number = strtoll(s.c_str(), nullptr, 2);
  return L_NUMBER;
}

int lpc_lex_number_real(union YYSTYPE *yylval_param, const char *text, int len) {
  auto s = lpc_strip_underscores(text, len);
  yylval_param->real = strtod(s.c_str(), nullptr);
  return L_REAL;
}

int lpc_lex_number_dec(union YYSTYPE *yylval_param, const char *text, int len) {
  auto s = lpc_strip_underscores(text, len);
  yylval_param->number = strtoll(s.c_str(), nullptr, 10);
  return L_NUMBER;
}

// ---------------------------------------------------------------------------
// String / template literal escape decoding
// ---------------------------------------------------------------------------

int lpc_lex_simple_escape(char c) {
  switch (c) {
    case 'n':  return '\n';
    case 't':  return '\t';
    case 'r':  return '\r';
    case 'b':  return '\b';
    case 'a':  return '\x07';
    case 'e':  return '\x1b';
    case '"':  return '"';
    case '\'': return '\'';
    case '`':  return '`';
    case '$':  return '$';
    case '\\': return '\\';
    default:   return -1;
  }
}

int lpc_lex_accum_overflow(void *yyscanner, union YYSTYPE *yylval_param, bool in_template) {
  compiler_context_t *ctx = ctx_of(yyscanner);
  if (static_cast<int>(ctx->str_accum.size()) <= MAXLINE) {
    return 0;
  }
  int tok;
  lexerror("String too long");
  if (!u8_validate(ctx->str_accum.c_str())) {
    lexerror("Invalid UTF8 string");
    tok = YYerror;
  } else {
    yylval_param->string = scratch_copy(ctx->str_accum.c_str());
    tok = L_STRING;
  }
  // See the header comment: an abandoned MIDDLE/TAIL fragment must undo
  // its template_nesting increment or it leaks for the rest of this
  // (already-failing) compile; a HEAD fragment never incremented yet.
  if (in_template && ctx->template_is_continuation) {
    ctx->template_nesting--;
  }
  return tok;
}

void lpc_lex_append_octal_escape(void *yyscanner, const char *text, bool is_template) {
  long long tmp = strtoll(text + 1, nullptr, 8);
  if (tmp > 255) {
    yywarn("Illegal character constant in %s.", literal_kind(is_template));
    tmp = 'x';
  }
  append_char(yyscanner, static_cast<char>(tmp));
}

void lpc_lex_append_bad_octal_escape(void *yyscanner, bool is_template) {
  // '\8'/'\9' aren't valid octal digits -- see lex.l's comment on the
  // original hand-written scanner's embedded-NUL quirk here. Reported
  // directly as an error instead of replicating that.
  (void)yyscanner;
  std::string msg = std::string("Illegal octal escape in ") + literal_kind(is_template) + ".";
  lexerror(msg.c_str());
}

void lpc_lex_append_hex_escape(void *yyscanner, const char *text, bool is_template) {
  long long tmp = strtoll(text + 2, nullptr, 16);
  if (tmp > 255) {
    yywarn("Illegal character constant.");
    tmp = 'x';
  }
  append_char(yyscanner, static_cast<char>(tmp));
  (void)is_template;
}

void lpc_lex_append_bad_hex_escape(void *yyscanner, bool is_template) {
  append_char(yyscanner, 'x');
  yywarn("\\x must be followed by a valid hex value; interpreting as 'x' instead.");
  (void)is_template;
}

void lpc_lex_append_unicode_pair_escape(void *yyscanner, const char *text) {
  // \uXXXX\uYYYY UTF-16 surrogate pair, matched and decoded as one unit --
  // see lex.l's rule comment for why the lead/trail hex digit ranges
  // guarantee this is exactly a D800-DBFF / DC00-DFFF pair.
  UChar pair[2];
  char lead_buf[5] = {text[2], text[3], text[4], text[5], 0};
  char trail_buf[5] = {text[8], text[9], text[10], text[11], 0};
  pair[0] = static_cast<UChar>(strtol(lead_buf, nullptr, 16));
  pair[1] = static_cast<UChar>(strtol(trail_buf, nullptr, 16));
  char utf8[4];
  UErrorCode err = U_ZERO_ERROR;
  int32_t written = 0;
  u_strToUTF8(utf8, sizeof(utf8), &written, pair, 2, &err);
  if (U_FAILURE(err)) {
    lexerror("Illegal unicode sequence.");
  } else {
    append_str(yyscanner, utf8, written);
  }
}

void lpc_lex_append_unicode_escape(void *yyscanner, const char *text) {
  // A lone \uXXXX not part of a valid pair: either a genuine single
  // (non-surrogate) code point, or a malformed/unpaired surrogate.
  UChar code = static_cast<UChar>(strtol(text + 2, nullptr, 16));
  if (U16_IS_SINGLE(code)) {
    char utf8[4];
    UErrorCode err = U_ZERO_ERROR;
    int32_t written = 0;
    u_strToUTF8(utf8, sizeof(utf8), &written, &code, 1, &err);
    if (U_FAILURE(err)) {
      lexerror("Illegal unicode sequence.");
    } else {
      append_str(yyscanner, utf8, written);
    }
  } else if (U16_IS_SURROGATE_LEAD(code)) {
    lexerror("Illegal unicode sequence. Missing surrogate trail.");
  } else {
    lexerror("Illegal unicode sequence, expecting surrogate lead, got trail.");
  }
}

void lpc_lex_append_long_unicode_escape(void *yyscanner, const char *text, int len) {
  UChar res[2];
  auto size = u_unescape(text, res, 2);
  if (size == 0) {
    lexerror("Illegal unicode sequence.");
    return;
  }
  char utf8[4];
  UErrorCode err = U_ZERO_ERROR;
  int32_t written = 0;
  u_strToUTF8(utf8, sizeof(utf8), &written, res, size, &err);
  if (U_FAILURE(err)) {
    lexerror("Illegal unicode sequence.");
  } else {
    append_str(yyscanner, utf8, written);
  }
  (void)len;
}

void lpc_lex_append_unknown_escape(void *yyscanner, const char *text, bool is_template) {
  append_char(yyscanner, text[1]);
  {
    // Fix-it: the escape means nothing, the character stands for itself
    // -- suggest dropping the backslash (rendered under the caret).
    compiler_context_t *ctx = ctx_of(yyscanner);
    int col = ctx->token_start_column + 1;
    compiler_pending_fixits.push_back(Diagnostic::FixIt{col, col + 2, std::string(1, text[1])});
    char msg[64];
    snprintf(msg, sizeof(msg), "Unknown escape sequence '\\%c'.", text[1]);
    yywarn("%s", msg);
  }
  (void)is_template;
}

int lpc_lex_string_close(void *yyscanner, union YYSTYPE *yylval_param) {
  compiler_context_t *ctx = ctx_of(yyscanner);
  if (!u8_validate(ctx->str_accum.c_str())) {
    lexerror("Invalid UTF8 codepoint in string literal");
    return YYerror;
  }
  yylval_param->string = scratch_copy(ctx->str_accum.c_str());
  return L_STRING;
}

int lpc_lex_template_head_or_middle(void *yyscanner, union YYSTYPE *yylval_param) {
  compiler_context_t *ctx = ctx_of(yyscanner);
  ctx->is_template = true;
  if (!u8_validate(ctx->str_accum.c_str())) {
    lexerror("Invalid UTF8 codepoint in template literal");
    return YYerror;
  }
  {
    // scratch_large_alloc(), not scratch_copy(): a template fragment's
    // value sits on the parser's value stack for as long as it takes to
    // scan the whole ${...} expression that follows it (which can itself
    // contain more string/template literals) -- scratch_copy()'s small
    // ring buffer would get overwritten by those first. See lex.l's rule
    // comment for the real bug this fixed.
    size_t slen = ctx->str_accum.size() + 1;
    char *res = scratch_large_alloc(static_cast<int>(slen));
    memcpy(res, ctx->str_accum.c_str(), slen);
    yylval_param->string = res;
  }
  if (!ctx->template_is_continuation) {
    ctx->template_nesting++;
    if (ctx->template_nesting >= MAX_TEMPLATE_NESTING) {
      // Undo the increment before bailing -- see lex.l's rule comment
      // (confirmed via UBSan: leaving it at MAX_TEMPLATE_NESTING here was a
      // real out-of-bounds write on the next '{'/'}').
      ctx->template_nesting--;
      lexerror("Template literal nesting too deep");
      return YYerror;
    }
  }
  ctx->template_brace_depth[ctx->template_nesting] = 0;
  return ctx->template_is_continuation ? L_TEMPLATE_MIDDLE : L_TEMPLATE_HEAD;
}

int lpc_lex_template_tail_or_string(void *yyscanner, union YYSTYPE *yylval_param) {
  compiler_context_t *ctx = ctx_of(yyscanner);
  ctx->is_template = true;
  if (!u8_validate(ctx->str_accum.c_str())) {
    lexerror("Invalid UTF8 codepoint in template literal");
    return YYerror;
  }
  if (ctx->template_is_continuation) {
    size_t slen = ctx->str_accum.size() + 1;
    char *res = scratch_large_alloc(static_cast<int>(slen));
    memcpy(res, ctx->str_accum.c_str(), slen);
    yylval_param->string = res;
    ctx->template_nesting--;
    return L_TEMPLATE_TAIL;
  }
  yylval_param->string = scratch_copy(ctx->str_accum.c_str());
  return L_STRING;
}

// ---------------------------------------------------------------------------
// Character literal escape decoding (writes directly to yylval.number, no
// accumulator -- a char literal's body is exactly one escape/byte).
// ---------------------------------------------------------------------------

LPC_INT lpc_lex_char_octal_escape(const char *text) {
  LPC_INT val = strtoll(text + 1, nullptr, 8);
  if (val > 255) {
    yywarn("Illegal character constant.");
    val = 'x';
  }
  return val;
}

LPC_INT lpc_lex_char_hex_escape(const char *text) {
  LPC_INT val = strtoll(text + 2, nullptr, 16);
  if (val > 255) {
    yywarn("Illegal character constant.");
    val = 'x';
  }
  return val;
}

LPC_INT lpc_lex_char_bad_hex_escape() {
  yywarn("\\x must be followed by a valid hex value; interpreting as 'x' instead.");
  return 'x';
}

LPC_INT lpc_lex_char_unknown_escape(const char *text) {
  yywarn("Unknown \\ escape.");
  return static_cast<unsigned char>(text[1]);
}

int lpc_lex_char_error(union YYSTYPE *yylval_param) {
  lexerror("Illegal character constant");
  yylval_param->number = 0;
  return L_NUMBER;
}

// ---------------------------------------------------------------------------
// $N / $ function-pointer parameter tokens
// ---------------------------------------------------------------------------

int lpc_lex_function_param(union YYSTYPE *yylval_param, const char *text, int len) {
  if (!current_function_context) {
    lexerror("$var illegal outside of function pointer.");
    return YYerror;
  }
  if (current_function_context->num_parameters < 0) {
    lexerror("$var illegal inside anonymous function pointer.");
    return YYerror;
  }
  if (len == 1) {
    // Bare "$".
    return '$';
  }
  if (len - 1 > MAXLINE - 5) {
    lexerror("Line too long");
    return kLpcLexFunctionParamRetry;
  }
  yylval_param->number = atoll(text + 1) - 1;
  if (yylval_param->number < 0) {
    lexerror("In function parameter $num, num must be >= 1.");
  } else if (yylval_param->number > 254) {
    lexerror("only 255 parameters allowed.");
  } else if (yylval_param->number >= current_function_context->num_parameters) {
    current_function_context->num_parameters = yylval_param->number + 1;
  }
  return L_PARAMETER;
}
