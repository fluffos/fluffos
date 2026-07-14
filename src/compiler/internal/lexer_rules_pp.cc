#include "base/std.h"

#include "compiler/internal/lexer_rules_pp.h"

#include <cctype>
#include <cstdlib>

#include "compiler/internal/compiler.h"
#include "compiler/internal/lexer.h"
#include "compiler/internal/lexer_utils.h"
// grammar_rules.h must precede grammar.autogen.h (decl_t/func_block_t);
// needed for the token ids and YYSTYPE the #if token evaluator consumes.
#include "compiler/internal/grammar_rules.h"
#include "compiler/internal/grammar.autogen.h"

ScratchString normalize_filename(const char* filename) {
  if (!filename) return ScratchString("/unknown");
  if (filename[0] == '/') return ScratchString(filename);
  ScratchString r("/");
  r += filename;
  return r;
}

std::string_view trim(std::string_view s) {
  size_t a = s.find_first_not_of(" \t\r");
  if (a == std::string_view::npos) return "";
  size_t b = s.find_last_not_of(" \t\r");
  return s.substr(a, b - a + 1);
}

ScratchString strip_directive_comments(std::string_view s) {
  ScratchString r;
  r.reserve(s.size());
  size_t i = 0;
  while (i < s.size()) {
    if (s[i] == '"' || s[i] == '\'' || s[i] == '`') {
      char q = s[i++];
      r += q;
      while (i < s.size() && s[i] != q) {
        if (s[i] == '\\') {
          r += s[i++];
        }
        if (i < s.size()) r += s[i++];
      }
      if (i < s.size()) r += s[i++];
    } else if (i + 1 < s.size() && s[i] == '/' && s[i + 1] == '/') {
      break;  // rest is a comment
    } else if (i + 1 < s.size() && s[i] == '/' && s[i + 1] == '*') {
      i += 2;
      while (i + 1 < s.size() && (s[i] != '*' || s[i + 1] != '/')) i++;
      if (i + 1 < s.size()) i += 2;
      // A comment is ONE space (C translation phase 3), not nothing:
      // eliding it entirely would paste the surrounding tokens together
      // ("1/*x*/2" must stay two tokens, not become "12").
      r += ' ';
    } else {
      r += s[i++];
    }
  }
  return r;
}

// Index of the first '/*' that never closes within `s`, or npos when
// every block comment in `s` is terminated. Quote/'//' handling mirrors
// strip_directive_comments() above -- a '/*' inside a string literal or
// after '//' does not open a comment.
static size_t open_comment_start(std::string_view s) {
  size_t i = 0;
  while (i < s.size()) {
    if (s[i] == '"' || s[i] == '\'' || s[i] == '`') {
      char q = s[i++];
      while (i < s.size() && s[i] != q) {
        if (s[i] == '\\') i++;
        if (i < s.size()) i++;
      }
      if (i < s.size()) i++;
    } else if (i + 1 < s.size() && s[i] == '/' && s[i + 1] == '/') {
      return std::string_view::npos;  // '//' runs to the newline that ends the capture
    } else if (i + 1 < s.size() && s[i] == '/' && s[i + 1] == '*') {
      size_t start = i;
      i += 2;
      while (i + 1 < s.size() && (s[i] != '*' || s[i + 1] != '/')) i++;
      if (i + 1 >= s.size()) return start;  // ran off the capture: still open
      i += 2;
    } else {
      i++;
    }
  }
  return std::string_view::npos;
}

bool lpc_lex_complete_directive(const char* text, int len, void* yyscanner, ScratchString* out,
                                int* pulled_lines) {
  std::string_view sv(text, len);
  size_t open = open_comment_start(sv);
  if (open == std::string_view::npos) return false;

  int kind = lpc_lex_top_buffer_kind();
  if (kind == LPC_BUF_PLAIN || kind == LPC_BUF_EXPANSION || kind == LPC_BUF_IF_EXPR) return false;

  out->assign(sv.substr(0, open));
  *out += ' ';

  // Newlines consumed here that do NOT end up in *out (comment-internal
  // ones and the terminator) get the same bookkeeping the rule's own
  // terminator consumption does (native yylineno advance inside
  // lpc_lex_getc() + lpc_lex_newline() for total_lines); kept
  // continuation newlines are counted from the text later, exactly like
  // capture-embedded ones.
  int consumed = 0;
  bool in_comment = true;
  bool in_line_comment = false;
  char in_quote = 0;
  bool quote_escape = false;
  int prev = 0;
  for (;;) {
    int c = lpc_lex_getc(yyscanner);
    if (c <= 0) {
      // Same terminal case as SC_BLOCK_COMMENT's <<EOF>> rule (and
      // lpc_lex_getc() already continued into parent buffers, matching
      // its lpc_lex_pop_splice_if_any step).
      lexerror("End of file in a comment (opened on a preprocessor directive)");
      break;
    }
    if (in_comment) {
      if (c == '\n') {
        lpc_lex_newline(yyscanner);
        consumed++;
        prev = 0;
      } else if (prev == '*' && c == '/') {
        in_comment = false;
        prev = 0;
      } else if (prev == '/' && c == '*') {
        yywarn("/* found in comment.");  // same warning as SC_BLOCK_COMMENT
        prev = 0;
      } else {
        prev = c;
      }
      continue;
    }
    if (c == '\n') {
      // A backslash before the newline splices the next physical line
      // in, matching the capture pattern's (\\\r?\n[^\n]*)* tail (and C,
      // where splicing precedes comment recognition -- so it applies
      // inside '//' too).
      size_t n = out->size();
      bool spliced = (n >= 1 && (*out)[n - 1] == '\\') ||
                     (n >= 2 && (*out)[n - 1] == '\r' && (*out)[n - 2] == '\\');
      if (spliced) {
        *out += '\n';
        continue;
      }
      lpc_lex_newline(yyscanner);
      consumed++;
      break;  // the logical line's terminator
    }
    if (in_line_comment) {
      *out += static_cast<char>(c);
      continue;
    }
    if (in_quote) {
      *out += static_cast<char>(c);
      if (quote_escape) {
        quote_escape = false;
      } else if (c == '\\') {
        quote_escape = true;
      } else if (c == in_quote) {
        in_quote = 0;
      }
      continue;
    }
    *out += static_cast<char>(c);
    if (c == '"' || c == '\'' || c == '`') {
      in_quote = static_cast<char>(c);
      quote_escape = false;
      continue;
    }
    size_t n = out->size();
    if (n >= 2 && (*out)[n - 2] == '/' && (*out)[n - 1] == '*') {
      out->resize(n - 2);
      *out += ' ';
      in_comment = true;
      prev = 0;
    } else if (n >= 2 && (*out)[n - 2] == '/' && (*out)[n - 1] == '/') {
      in_line_comment = true;
    }
  }
  // The formula in lpc_lex_on_directive() already subtracts the
  // terminator; report only the newlines beyond it.
  *pulled_lines = consumed > 0 ? consumed - 1 : 0;
  return true;
}

ScratchString stringize(std::string_view s) {
  ScratchString r("\"");
  for (char c : s) {
    if (c == '"' || c == '\\') {
      r += '\\';
    }
    r += c;
  }
  r += '"';
  return r;
}

ScratchVector<ScratchString> collect_args(std::string_view text, size_t& i) {
  ScratchVector<ScratchString> args;
  ScratchString arg;
  int depth = 0;
  char inq = 0;
  while (i < text.size()) {
    char c = text[i++];
    if (inq) {
      if (c == '\\' && i < text.size()) {
        arg += c;
        arg += text[i++];
        continue;
      }
      arg += c;
      if (c == inq) inq = 0;
    } else if (c == '"' || c == '\'' || c == '`') {
      inq = c;
      arg += c;
    } else if (c == '(') {
      depth++;
      arg += c;
    } else if (c == ')') {
      if (depth == 0) {
        args.emplace_back(trim(arg));
        break;
      }
      depth--;
      arg += c;
    } else if (c == ',' && depth == 0) {
      args.emplace_back(trim(arg));
      arg.clear();
    } else {
      arg += c;
    }
  }
  return args;
}

ScratchString substitute(std::string_view body, const std::vector<std::string>& params,
                         const ScratchVector<ScratchString>& args,
                         const ScratchVector<ScratchString>* expanded_args) {
  // C parameter-substitution selection: a parameter that is an operand
  // of # (handled in the stringize branch) or ## gets the RAW argument
  // spelling; any other use gets the macro-expanded form when the
  // caller supplied one (expanded_args non-null; the rescan-driven
  // expansion path does), else raw.
  auto pasted_operand = [&body](size_t tok_start, size_t tok_end) {
    size_t k = tok_start;
    while (k > 0 && (body[k - 1] == ' ' || body[k - 1] == '\t')) {
      k--;
    }
    bool before = (k >= 2 && body[k - 1] == '#' && body[k - 2] == '#');
    size_t f = tok_end;
    while (f < body.size() && (body[f] == ' ' || body[f] == '\t')) {
      f++;
    }
    bool after = (f + 1 < body.size() && body[f] == '#' && body[f + 1] == '#');
    return before || after;
  };

  ScratchString temp;
  size_t i = 0;
  while (i < body.size()) {
    if (body[i] == '#') {
      if (i + 1 < body.size() && body[i + 1] == '#') {
        temp += "##";
        i += 2;
        continue;
      }

      size_t j = i + 1;
      while (j < body.size() && (body[j] == ' ' || body[j] == '\t')) {
        j++;
      }

      size_t start = j;
      while (j < body.size() &&
             (std::isalnum(static_cast<unsigned char>(body[j])) || body[j] == '_')) {
        j++;
      }
      std::string_view ident = body.substr(start, j - start);

      bool replaced = false;
      for (size_t p = 0; p < params.size() && p < args.size(); p++) {
        if (ident == params[p]) {
          temp += stringize(args[p]);
          replaced = true;
          i = j;
          break;
        }
      }
      if (replaced) {
        continue;
      }
    }

    if (std::isalpha(static_cast<unsigned char>(body[i])) || body[i] == '_') {
      size_t start = i;
      while (i < body.size() &&
             (std::isalnum(static_cast<unsigned char>(body[i])) || body[i] == '_')) {
        i++;
      }
      std::string_view tok = body.substr(start, i - start);
      bool replaced = false;
      for (size_t p = 0; p < params.size() && p < args.size(); p++) {
        if (tok == params[p]) {
          if (expanded_args != nullptr && p < expanded_args->size() && !pasted_operand(start, i)) {
            temp += (*expanded_args)[p];
          } else {
            temp += args[p];
          }
          replaced = true;
          break;
        }
      }
      if (!replaced) {
        temp += tok;
      }
      continue;
    }

    if (body[i] == '"' || body[i] == '\'') {
      char q = body[i++];
      temp += q;
      while (i < body.size() && body[i] != q) {
        if (body[i] == '\\') temp += body[i++];
        if (i < body.size()) temp += body[i++];
      }
      if (i < body.size()) temp += body[i++];
      continue;
    }

    temp += body[i++];
  }

  ScratchString result;
  i = 0;
  while (i < temp.size()) {
    if (temp[i] == '"' || temp[i] == '\'') {
      char q = temp[i++];
      result += q;
      while (i < temp.size() && temp[i] != q) {
        if (temp[i] == '\\') result += temp[i++];
        if (i < temp.size()) result += temp[i++];
      }
      if (i < temp.size()) result += temp[i++];
      continue;
    }

    size_t j = i;
    while (j < temp.size() && (temp[j] == ' ' || temp[j] == '\t')) {
      j++;
    }
    if (j + 1 < temp.size() && temp[j] == '#' && temp[j + 1] == '#') {
      while (!result.empty() && (result.back() == ' ' || result.back() == '\t')) result.pop_back();
      j += 2;
      while (j < temp.size() && (temp[j] == ' ' || temp[j] == '\t')) {
        j++;
      }
      i = j;
      continue;
    }
    result += temp[i++];
  }

  return result;
}

// ---------------------------------------------------------------------------
// #if/#elif expression evaluator
// ---------------------------------------------------------------------------
//
// Plain struct + free functions taking an explicit state pointer, matching
// the rest of lexer_rules*.cc's style (no classes/methods elsewhere in
// this file or in lexer_rules.cc) -- a recursive-descent evaluator built
// the same way lexer.l's own escape-decoding helpers are: small functions
// each handling one grammar piece, threading state explicitly instead of
// through `this`.

namespace {

// ---------------------------------------------------------------------------
// #if/#elif expression evaluation over TOKENS (7.4): the expression text
// is pushed as a LPC_BUF_IF_EXPR buffer and pulled through the scanner
// itself (lpc_lex_ifexpr_next) -- numbers arrive via the real literal
// decoders (hex/binary/char escapes included), macro references expand
// through the ordinary rescan machinery into a FLAT token sequence
// (preserving C precedence: `#define X 1+1` + `#if X*2` is 1+1*2 = 3),
// and defined()/efun_defined() operands are pulled with expansion
// suppressed (C requires the unexpanded name). The buffer's own <<EOF>>
// returns LPC_IFEXPR_END, so the pull stops exactly at the expression's
// edge. Only then does this evaluator run, over the collected vector --
// same precedence/ternary/error structure as the old char-level walker,
// with tokens for atoms.
// ---------------------------------------------------------------------------

struct IfTok {
  int op;    // 0 = number; otherwise an operator code (see ifexpr_binop)
  long val;  // the number when op == 0
};

struct IfTokState {
  const std::vector<IfTok>* toks;
  size_t pos = 0;
  ScratchString error;  // first error wins; empty means no error yet
};

bool ifexpr_at_end(const IfTokState* st) { return st->pos >= st->toks->size(); }

int ifexpr_peek_op(const IfTokState* st) { return ifexpr_at_end(st) ? 0 : (*st->toks)[st->pos].op; }

void ifexpr_set_error(IfTokState* st, const char* msg) {
  if (st->error.empty()) st->error = msg;
}

long ifexpr_top(IfTokState* st);

long ifexpr_atom(IfTokState* st) {
  if (ifexpr_at_end(st)) return 0;
  const IfTok& t = (*st->toks)[st->pos];
  if (t.op == 0) {
    st->pos++;
    return t.val;
  }
  switch (t.op) {
    case '(': {
      st->pos++;
      long v = ifexpr_top(st);
      if (ifexpr_peek_op(st) == ')') {
        st->pos++;
      } else {
        ifexpr_set_error(st, "bracket not paired in #if");
      }
      return v;
    }
    case '!':
      st->pos++;
      return !ifexpr_atom(st);
    case '~':
      st->pos++;
      return ~ifexpr_atom(st);
    case '-':
      st->pos++;
      return -ifexpr_atom(st);
    case '+':
      st->pos++;
      return ifexpr_atom(st);
    default:
      // Mirrors the old walker's unknown-atom behavior: consume, 0.
      st->pos++;
      return 0;
  }
}

long ifexpr_binop(IfTokState* st, int min_prec) {
  long lhs = ifexpr_atom(st);
  for (;;) {
    if (ifexpr_at_end(st)) break;

    int c0 = ifexpr_peek_op(st);
    int prec = -1, op = 0;

    if (c0 == 'O') {
      op = 'O';
      prec = 1;
    } else if (c0 == 'A') {
      op = 'A';
      prec = 2;
    } else if (c0 == 'E') {
      op = 'E';
      prec = 6;
    } else if (c0 == 'N') {
      op = 'N';
      prec = 6;
    } else if (c0 == 'L') {
      op = 'L';
      prec = 7;
    } else if (c0 == 'G') {
      op = 'G';
      prec = 7;
    } else if (c0 == 's') {
      op = 's';
      prec = 8;
    } else if (c0 == 'S') {
      op = 'S';
      prec = 8;
    } else if (c0 == '|') {
      op = '|';
      prec = 3;
    } else if (c0 == '^') {
      op = '^';
      prec = 4;
    } else if (c0 == '&') {
      op = '&';
      prec = 5;
    } else if (c0 == '<') {
      op = '<';
      prec = 7;
    } else if (c0 == '>') {
      op = '>';
      prec = 7;
    } else if (c0 == '+') {
      op = '+';
      prec = 9;
    } else if (c0 == '-') {
      op = '-';
      prec = 9;
    } else if (c0 == '*') {
      op = '*';
      prec = 10;
    } else if (c0 == '/') {
      op = '/';
      prec = 10;
    } else if (c0 == '%') {
      op = '%';
      prec = 10;
    }

    if (prec < min_prec) break;

    st->pos++;
    long rhs = ifexpr_binop(st, prec + 1);

    switch (op) {
      case 'O':
        lhs = lhs || rhs;
        break;
      case 'A':
        lhs = lhs && rhs;
        break;
      case '|':
        lhs = lhs | rhs;
        break;
      case '^':
        lhs = lhs ^ rhs;
        break;
      case '&':
        lhs = lhs & rhs;
        break;
      case 'E':
        lhs = lhs == rhs;
        break;
      case 'N':
        lhs = lhs != rhs;
        break;
      case 'L':
        lhs = lhs <= rhs;
        break;
      case 'G':
        lhs = lhs >= rhs;
        break;
      case '<':
        lhs = lhs < rhs;
        break;
      case '>':
        lhs = lhs > rhs;
        break;
      case 's':
        // A negative or >=64-bit (lhs/rhs are `long`) shift count is
        // undefined behavior.
        if (rhs < 0 || rhs >= 64) {
          ifexpr_set_error(st, "shift amount out of range in #if");
          lhs = 0;
        } else {
          lhs = lhs << rhs;
        }
        break;
      case 'S':
        if (rhs < 0 || rhs >= 64) {
          ifexpr_set_error(st, "shift amount out of range in #if");
          lhs = 0;
        } else {
          lhs = lhs >> rhs;
        }
        break;
      case '+':
        lhs = lhs + rhs;
        break;
      case '-':
        lhs = lhs - rhs;
        break;
      case '*':
        lhs = lhs * rhs;
        break;
      case '/':
        if (rhs == 0) {
          ifexpr_set_error(st, "division by 0 in #if");
          lhs = 0;
        } else if (rhs == -1) {
          // x / -1 == -x; direct division traps (SIGFPE) for LONG_MIN.
          lhs = (long)(0ULL - (unsigned long)lhs);
        } else {
          lhs = lhs / rhs;
        }
        break;
      case '%':
        if (rhs == 0) {
          ifexpr_set_error(st, "modulo by 0 in #if");
          lhs = 0;
        } else if (rhs == -1) {
          lhs = 0;  // x % -1 == 0; direct computation traps for LONG_MIN.
        } else {
          lhs = lhs % rhs;
        }
        break;
      default:
        break;
    }
  }
  return lhs;
}

long ifexpr_top(IfTokState* st) {
  long cond = ifexpr_binop(st, 0);
  if (ifexpr_peek_op(st) == '?') {
    st->pos++;
    long true_val = ifexpr_top(st);
    if (ifexpr_peek_op(st) == ':') {
      st->pos++;
    } else {
      ifexpr_set_error(st, "'?' without ':' in #if");
    }
    long false_val = ifexpr_top(st);
    return cond ? true_val : false_val;
  }
  return cond;
}

// The name of an identifier-flavored token, for defined()'s operand.
// Empty when the token isn't name-shaped.
ScratchString ifexpr_token_name(int tok, const union YYSTYPE* lv) {
  if (tok == L_IDENTIFIER && lv->string != nullptr) return *lv->string;
  if (tok == L_DEFINED_NAME && lv->ihe != nullptr && lv->ihe->name != nullptr)
    return ScratchString(lv->ihe->name);
  return ScratchString();
}

// Pull the defined(X)/efun_defined(X) operand with macro expansion
// suppressed and evaluate it. Consumes through the closing ')' (or the
// bare name). Returns the 0/1 result; sets *ended when the expression
// ran out mid-operand.
long ifexpr_pull_defined(bool efun_form, void* yyscanner, bool* ended) {
  compiler_context_t* ctx = yyget_extra(yyscanner);
  ctx->suppress_expansion = true;
  union YYSTYPE lv;
  int tok = lpc_lex_ifexpr_next(&lv, yyscanner);
  bool paren = false;
  if (tok == '(') {
    paren = true;
    tok = lpc_lex_ifexpr_next(&lv, yyscanner);
  }
  long result = 0;
  if (tok == LPC_IFEXPR_END || tok <= 0) {
    *ended = true;
    ctx->suppress_expansion = false;
    return 0;
  }
  ScratchString name = ifexpr_token_name(tok, &lv);
  if (!name.empty()) {
    if (efun_form) {
      result = (lookup_predef(name.c_str()) >= 0) ? 1 : 0;
    } else {
      result = pp_find_macro(std::string_view(name.data(), name.size())) != nullptr ? 1 : 0;
    }
  }
  if (paren) {
    tok = lpc_lex_ifexpr_next(&lv, yyscanner);
    if (tok == LPC_IFEXPR_END || tok <= 0) {
      *ended = true;
    } else if (tok != ')') {
      lexerror("bracket not paired in #if");
    }
  }
  ctx->suppress_expansion = false;
  return result;
}

}  // namespace

long lpc_lex_eval_if_expr(std::string_view expr, void* yyscanner) {
  std::vector<IfTok> toks;
  ScratchString text(trim(expr));
  if (!text.empty()) {
    lpc_lex_push_string_buffer(text.c_str(), text.size(), LPC_BUF_IF_EXPR, yyscanner);
    bool ended = false;
    while (!ended) {
      union YYSTYPE lv;
      int tok = lpc_lex_ifexpr_next(&lv, yyscanner);
      switch (tok) {
        case LPC_IFEXPR_END:
          ended = true;
          break;
        case L_NUMBER:
          toks.push_back(IfTok{0, static_cast<long>(lv.number)});
          break;
        case L_REAL:
          lexerror("floating point constants are not allowed in #if");
          toks.push_back(IfTok{0, 0});
          break;
        case L_LOR:
          toks.push_back(IfTok{'O', 0});
          break;
        case L_LAND:
          toks.push_back(IfTok{'A', 0});
          break;
        case L_EQ_NE:
          toks.push_back(IfTok{lv.number == F_EQ ? 'E' : 'N', 0});
          break;
        case L_SHIFT:
          toks.push_back(IfTok{lv.number == F_LSH ? 's' : 'S', 0});
          break;
        case L_ORDER:
          // "<=", ">=", ">" arrive as one token whose yylval
          // carries the VM opcode ("<" is a bare '<' char).
          toks.push_back(IfTok{lv.number == F_LE ? 'L' : lv.number == F_GE ? 'G' : '>', 0});
          break;
        case '(':
        case ')':
        case '!':
        case '~':
        case '&':
        case '|':
        case '^':
        case '<':
        case '>':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '?':
        case ':':
          toks.push_back(IfTok{tok, 0});
          break;
        default: {
          if (tok <= 0) {
            // A scan-level error path ended the expression
            // early (e.g. unterminated string); the buffer, if
            // still stacked, is drained below.
            ended = true;
            break;
          }
          ScratchString name = ifexpr_token_name(tok, &lv);
          if (name == "defined" || name == "efun_defined") {
            toks.push_back(
                IfTok{0, ifexpr_pull_defined(name == "efun_defined", yyscanner, &ended)});
          } else {
            // Any other token -- an undefined identifier, a
            // keyword, a stray construct -- evaluates as 0,
            // like the old walker's unknown atoms.
            toks.push_back(IfTok{0, 0});
          }
          break;
        }
      }
    }
    // Defensive drain: an error path can leave the expression buffer
    // stacked; it must never leak LPC_IFEXPR_END into the parser.
    while (lpc_lex_top_buffer_kind() == LPC_BUF_IF_EXPR) {
      lpc_lex_pop_pushed_buffer(yyscanner);
    }
  }

  IfTokState st;
  st.toks = &toks;
  long result = ifexpr_top(&st);
  if (!st.error.empty()) {
    lexerror(st.error.c_str());
    return 0;
  }
  if (!ifexpr_at_end(&st)) {
    lexerror("Condition too complex");
    return 0;
  }
  return result;
}

// The shared predefine table: a PpMacro-shaped view of the boot predefine
// registry plus the three builtins. Built lazily and rebuilt ONLY when
// the registry's version changes (tests register predefines after boot);
// per-compile setup copies NOTHING -- the old session heap-copied this
// entire table on every compile.
static const LpcMacroTable& predefine_macros() {
  static LpcMacroTable table;
  static unsigned built_version = 0;
  unsigned v = get_predefines_version();
  if (built_version != v) {
    table.clear();
    for (const auto& pair : get_predefines()) {
      PpMacro m;
      m.is_function_like = pair.second.is_function_like;
      m.is_predefined = true;
      m.body = pair.second.body;
      table[pair.first] = std::move(m);
    }
    table["__FILE__"] = PpMacro{false, true, {}, ""};
    table["__LINE__"] = PpMacro{false, true, {}, ""};
    table["__DIR__"] = PpMacro{false, true, {}, ""};
    built_version = v;
  }
  return table;
}

const PpMacro* pp_find_macro(std::string_view name) {
  // Reused key: the tables are std::string-keyed (persistent state);
  // one static key avoids a heap allocation per lookup. The compiler
  // is single-threaded and non-reentrant.
  static std::string key;
  key.assign(name.data(), name.size());
  auto it = g_compile.macros.find(key);
  if (it != g_compile.macros.end()) return &it->second;
  const auto& pre = predefine_macros();
  auto pit = pre.find(key);
  if (pit != pre.end()) return &pit->second;
  return nullptr;
}

bool pp_is_predefined(std::string_view name) {
  const PpMacro* m = pp_find_macro(name);
  return m != nullptr && m->is_predefined;
}

bool lpc_lex_emitting() {
  if (!g_compile.pp_active) return true;
  for (const auto& cs : g_compile.conds) {
    if (!cs.emitting) return false;
  }
  return true;
}

// total_lines += for each '\n' embedded in the matched text
// (backslash-continuation line breaks inside a directive) -- the LINE
// counter itself advances natively (%option yylineno scans the matched
// text). Called exactly once per captured line, by lpc_lex_on_directive().
static void count_directive_newlines(const char* text, int len) {
  for (int i = 0; i < len; i++) {
    if (text[i] == '\n') {
      total_lines++;  // the newline itself is counted natively
                      // (%option yylineno scans the matched text)
    }
  }
}

bool lpc_lex_builtin_macro(std::string_view name, ScratchString* out) {
  if (name == "__LINE__") {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", current_line);
    *out = buf;
    return true;
  }
  if (name == "__FILE__") {
    *out = stringize(normalize_filename(current_file));
    return true;
  }
  if (name == "__DIR__") {
    ScratchString filename = normalize_filename(current_file);
    size_t last_slash = filename.find_last_of('/');
    // normalize_filename() guarantees a leading '/', so there is always
    // at least one slash to keep.
    *out = stringize(filename.substr(0, last_slash + 1));
    return true;
  }
  return false;
}

ScratchString lpc_lex_expand_string(std::string_view text, ScratchVector<ScratchString> guard) {
  if (!g_compile.pp_active) return ScratchString(text);
  ScratchString result;
  size_t i = 0;
  while (i < text.size()) {
    if (text[i] == '"' || text[i] == '\'') {
      char q = text[i++];
      result += q;
      while (i < text.size() && text[i] != q) {
        if (text[i] == '\\') result += text[i++];
        if (i < text.size()) result += text[i++];
      }
      if (i < text.size()) result += text[i++];
      continue;
    }
    if (std::isalpha(static_cast<unsigned char>(text[i])) || text[i] == '_') {
      size_t start = i;
      while (i < text.size() &&
             (std::isalnum(static_cast<unsigned char>(text[i])) || text[i] == '_'))
        i++;
      std::string_view id = text.substr(start, i - start);

      {
        ScratchString builtin;
        if (lpc_lex_builtin_macro(id, &builtin)) {
          result += builtin;
          continue;
        }
      }

      bool guarded = false;
      for (const auto& g : guard) {
        if (g == id) {
          guarded = true;
          break;
        }
      }

      const PpMacro* found = pp_find_macro(id);
      if (!guarded && found != nullptr) {
        const PpMacro& m = *found;
        if (!m.is_function_like) {
          auto g2 = guard;
          g2.emplace_back(id);
          result += lpc_lex_expand_string(m.body, std::move(g2));
        } else {
          size_t j = i;
          while (j < text.size() && (text[j] == ' ' || text[j] == '\t')) j++;
          if (j < text.size() && text[j] == '(') {
            j++;
            auto args = collect_args(text, j);
            i = j;
            ScratchVector<ScratchString> expanded_args;
            auto g2 = guard;
            g2.emplace_back(id);
            // Argument pre-expansion deliberately passes no
            for (const auto& a : args) expanded_args.push_back(lpc_lex_expand_string(a, guard));
            ScratchString subst = substitute(m.body, m.params, expanded_args);
            result += lpc_lex_expand_string(subst, std::move(g2));
          } else {
            // Function-like macro name with no argument list in
            // this text: left literal and NOT counted -- if its
            // '(' turns out to follow in the input stream after
            // this text is spliced, the rescan may legitimately
            // expand it there (C behavior).
            result += id;
          }
        }
      } else {
        result += id;
      }
      continue;
    }
    result += text[i++];
  }
  return result;
}

static ScratchString fold_backslash_newlines(std::string_view text) {
  ScratchString result;
  result.reserve(text.size());
  for (size_t i = 0; i < text.size();) {
    if (text[i] == '\\') {
      size_t j = i + 1;
      if (j < text.size() && text[j] == '\r') j++;
      if (j < text.size() && text[j] == '\n') {
        i = j + 1;
        continue;
      }
    }
    result.push_back(text[i]);
    i++;
  }
  return result;
}

// Applies one already-parsed directive: `dir` is the directive keyword,
// `rest` its payload (both views into the caller's folded line). This is
// the single implementation behind lpc_lex_on_directive() -- both scan
// modes funnel through it, so a skip-mode-ending #elif is evaluated by
// exactly the same code as an emit-mode one.
static void dispatch_directive(std::string_view dir, std::string_view rest, void* yyscanner) {
  if (dir == "define") {
    if (lpc_lex_emitting()) {
      // Comments are whitespace (C translation phase 3): strip them from
      // the whole definition BEFORE parsing the name/params/body. A '//'
      // tail otherwise lands in the stored body and -- expansion buffers
      // carry no newline to end it -- comments out the rest of whatever
      // spliced text the macro later expands into (#1240).
      ScratchString rest_stripped = strip_directive_comments(rest);
      rest = std::string_view(rest_stripped);
      size_t idx = 0;
      while (idx < rest.size() && (rest[idx] == ' ' || rest[idx] == '\t')) idx++;
      size_t ns = idx;
      while (idx < rest.size() &&
             (std::isalnum(static_cast<unsigned char>(rest[idx])) || rest[idx] == '_'))
        idx++;
      std::string_view name = rest.substr(ns, idx - ns);
      if (name.empty()) {
        lexerror("#define: missing name");
        return;
      }

      if (pp_is_predefined(name)) {
        lexerror("Illegal to redefine a predefined value.");
        return;
      }
      auto existing = g_compile.macros.find(std::string(name));

      PpMacro m;
      if (idx < rest.size() && rest[idx] == '(') {
        m.is_function_like = true;
        idx++;  // skip '('
        while (idx < rest.size() && rest[idx] != ')') {
          size_t const before = idx;
          while (idx < rest.size() && (rest[idx] == ' ' || rest[idx] == '\t')) idx++;
          size_t ps = idx;
          while (idx < rest.size() &&
                 (std::isalnum(static_cast<unsigned char>(rest[idx])) || rest[idx] == '_'))
            idx++;
          if (idx > ps) m.params.emplace_back(rest.substr(ps, idx - ps));
          while (idx < rest.size() && (rest[idx] == ' ' || rest[idx] == '\t')) idx++;
          if (idx < rest.size() && rest[idx] == ',') idx++;
          // A stray character (not identifier/comma/space/')' -- e.g. '@' or a
          // UTF-8 lead byte) consumes nothing; bail instead of looping forever.
          if (idx == before) {
            lexerror("#define: malformed macro parameter list");
            return;
          }
        }
        if (idx < rest.size()) idx++;  // skip ')'
      }
      while (idx < rest.size() && (rest[idx] == ' ' || rest[idx] == '\t')) idx++;
      // trim(): a stripped trailing comment leaves trailing whitespace,
      // which must not make "1" vs "1 " look like a redefinition.
      ScratchString body =
          (idx < rest.size()) ? ScratchString(trim(rest.substr(idx))) : ScratchString();

      std::string_view bv(body);
      auto bv_trim = trim(bv);
      if (bv_trim.size() >= 2 && bv_trim.substr(0, 2) == "##") {
        lexerror("'##' cannot appear at start of macro body");
        return;
      }
      if (bv_trim.size() >= 2 && bv_trim.substr(bv_trim.size() - 2) == "##") {
        lexerror("'##' cannot appear at end of macro body");
        return;
      }

      if (existing != g_compile.macros.end()) {
        if (std::string_view(existing->second.body) != std::string_view(body)) {
          // Redefining a macro with a different body is ALLOWED
          // (the new definition takes effect below) -- it is a
          // non-fatal warning, not an error. It went through
          // lexerror() before, which increments num_parse_error
          // and failed the whole compile despite the "Warning:"
          // wording; yywarn() is the real warning path
          // (respects #pragma no_warnings, does not fail the
          // compile). Identical-body redefinition stays silent.
          if (!existing->second.def_file.empty()) {
            compiler_pending_notes.push_back("previous definition of '" + std::string(name) +
                                             "' was at /" + existing->second.def_file + ":" +
                                             std::to_string(existing->second.def_line));
          }
          yywarn("Macro '%s' redefined", std::string(name).c_str());
        }
      }

      // Definition site for diagnostics ("defined at ...", "previous
      // definition was at ..."). compiler_directive_start_line holds
      // this #define's own first line while we're dispatching it.
      m.def_file = current_file != nullptr ? current_file : "";
      m.def_line = compiler_directive_start_line;
      m.body.assign(body.data(), body.size());
      g_compile.macros[std::string(name)] = std::move(m);
    }
  } else if (dir == "undef") {
    if (lpc_lex_emitting()) {
      // Same phase-3 rule as #define: "#undef X // why" names X, not
      // "X // why" (which silently erased nothing).
      std::string name(trim(std::string_view(strip_directive_comments(rest))));
      if (pp_is_predefined(name)) {
        lexerror("Illegal to #undef a predefined value.");
      } else {
        g_compile.macros.erase(name);
      }
    }
  } else if (dir == "ifdef") {
    // Strip comments or "#ifdef X // why" looks up the wrong name and
    // silently takes the false branch.
    bool def = pp_find_macro(trim(std::string_view(strip_directive_comments(rest)))) != nullptr;
    bool emit = lpc_lex_emitting() && def;
    g_compile.conds.push_back({emit, emit});
  } else if (dir == "ifndef") {
    bool def = pp_find_macro(trim(std::string_view(strip_directive_comments(rest)))) != nullptr;
    bool emit = lpc_lex_emitting() && !def;
    g_compile.conds.push_back({emit, emit});
  } else if (dir == "if") {
    bool cond = false;
    if (lpc_lex_emitting()) {
      ScratchString stripped = strip_directive_comments(rest);
      ScratchString trimmed(trim(std::string_view(stripped)));
      if (trimmed.empty()) {
        lexerror("missing expression in #if");
      } else {
        cond = (lpc_lex_eval_if_expr(trimmed, yyscanner) != 0);
      }
    }
    g_compile.conds.push_back({cond, cond});
  } else if (dir == "elif") {
    if (g_compile.conds.empty()) {
      lexerror("unexpected #elif");
    } else {
      bool had = g_compile.conds.back().had_true;
      g_compile.conds.pop_back();
      bool outer = lpc_lex_emitting();
      bool cond = false;
      if (outer && !had) {
        ScratchString stripped = strip_directive_comments(rest);
        ScratchString trimmed(trim(std::string_view(stripped)));
        if (trimmed.empty()) {
          lexerror("missing expression in #elif");
        } else {
          cond = (lpc_lex_eval_if_expr(trimmed, yyscanner) != 0);
        }
      }
      bool now = outer && !had && cond;
      g_compile.conds.push_back({now, had || now});
    }
  } else if (dir == "else") {
    if (g_compile.conds.empty()) {
      lexerror("unexpected #else");
    } else {
      bool had = g_compile.conds.back().had_true;
      g_compile.conds.pop_back();
      bool now = lpc_lex_emitting() && !had;
      g_compile.conds.push_back({now, true});
    }
  } else if (dir == "endif") {
    if (g_compile.conds.empty()) {
      lexerror("unexpected #endif");
    } else {
      g_compile.conds.pop_back();
    }
  } else if (dir == "include") {
    if (lpc_lex_emitting()) {
      lpc_lex_handle_include(rest, yyscanner);
    }
  } else if (dir == "error") {
    if (lpc_lex_emitting()) {
      ScratchString msg("#error ");
      msg += trim(rest);
      lexerror(msg.c_str());
    }
  } else if (dir == "warn") {
    if (lpc_lex_emitting()) {
      // A real warning (respects #pragma no_warnings, does not fail
      // the compile) -- it went through lexerror() before, which
      // failed the whole compile despite the directive's name.
      ScratchString msg("#warn ");
      msg += trim(rest);
      yywarn("%s", msg.c_str());
    }
  } else if (dir == "echo") {
    if (lpc_lex_emitting()) {
      fprintf(stderr, "%s\n", ScratchString(trim(rest)).c_str());
    }
  } else if (dir == "pragma") {
    if (lpc_lex_emitting()) {
      // Pragma payloads are word lists, so a trailing comment would read
      // as an unknown pragma word -- strip like the other parsed forms.
      ScratchString rest_str(trim(std::string_view(strip_directive_comments(rest))));
      handle_pragma(const_cast<char*>(rest_str.c_str()));
    }
  } else if (dir == "line" ||
             (dir.empty() && !rest.empty() && std::isdigit(static_cast<unsigned char>(rest[0])))) {
    std::string_view p = rest;
    size_t idx = 0;
    while (idx < p.size() && std::isspace(static_cast<unsigned char>(p[idx]))) idx++;
    if (idx < p.size() && std::isdigit(static_cast<unsigned char>(p[idx]))) {
      size_t start = idx;
      while (idx < p.size() && std::isdigit(static_cast<unsigned char>(p[idx]))) idx++;
      long line_num = strtol(std::string(p.substr(start, idx - start)).c_str(), nullptr, 10);
      // No -1 here: the lexer.l directive rule consumed and counted the
      // directive's terminating newline BEFORE dispatching, so the
      // next line scanned is exactly the one `#line N` names.
      current_line = static_cast<int>(line_num);

      while (idx < p.size() && std::isspace(static_cast<unsigned char>(p[idx]))) idx++;
      if (idx < p.size() && p[idx] == '"') {
        idx++;
        size_t file_start = idx;
        while (idx < p.size() && p[idx] != '"') idx++;
        if (idx < p.size() && p[idx] == '"') {
          ScratchString new_file =
              normalize_filename(ScratchString(p.substr(file_start, idx - file_start)).c_str());
          free_string(current_file);
          current_file = make_shared_string(new_file.c_str());
        }
      }
    }
  } else if (dir == "breakpoint") {
    // Ignored
  } else if (!dir.empty()) {
    if (lpc_lex_emitting()) {
      ScratchString msg("Unknown preprocessor directive: #");
      msg += dir;
      lexerror(msg.c_str());
    }
  }
}

LpcDirectiveAction lpc_lex_on_directive(const char* text, int len, void* yyscanner,
                                        bool in_skip_mode, int pulled_lines) {
  // The directive's own first line, for error attribution: the lexer.l
  // rule consumed + counted the terminating newline before calling us,
  // so current_line is already one past the directive's LAST physical
  // line; its embedded continuations haven't been counted yet, so its
  // FIRST line is exactly current_line - 1 -- minus any extra physical
  // lines lpc_lex_complete_directive() pulled for a spanning /* comment.
  // Published around the dispatch calls below via
  // compiler_directive_start_line (see its comment in compiler.h) so
  // yyerror()/yywarn() attribute to the directive rather than the line
  // after it.
  int directive_line = current_line - 1 - pulled_lines;

  // Embedded continuation newlines are physical lines regardless of scan
  // mode or whether the directive dispatches -- counted exactly once,
  // here (the terminating newline is the lexer.l rule's job, not ours).
  count_directive_newlines(text, len);

  if (!g_compile.pp_active) return LpcDirectiveAction::kNone;

  // Fold + parse the captured line exactly once: both the skip-mode
  // classification and the full dispatch below read the same name/rest.
  ScratchString folded = fold_backslash_newlines(std::string_view(text, len));
  std::string_view sv(folded);
  size_t i = 0;
  while (i < sv.size() && (sv[i] == ' ' || sv[i] == '\t')) i++;
  if (i < sv.size() && sv[i] == '#') i++;
  while (i < sv.size() && (sv[i] == ' ' || sv[i] == '\t')) i++;

  size_t name_start = i;
  while (i < sv.size() && (std::isalnum(static_cast<unsigned char>(sv[i])) || sv[i] == '_')) i++;
  std::string_view dir = sv.substr(name_start, i - name_start);

  while (i < sv.size() && (sv[i] == ' ' || sv[i] == '\t')) i++;
  std::string_view rest = sv.substr(i);
  while (!rest.empty() && (rest.back() == '\r' || rest.back() == '\n')) {
    rest.remove_suffix(1);
  }

  auto* ctx = reinterpret_cast<compiler_context_t*>(yyget_extra(yyscanner));

  if (!in_skip_mode) {
    compiler_directive_start_line = directive_line;
    dispatch_directive(dir, rest, yyscanner);
    compiler_directive_start_line = 0;
    if (!lpc_lex_emitting()) {
      ctx->skip_depth = 0;
      return LpcDirectiveAction::kEnterSkip;
    }
    return LpcDirectiveAction::kNone;
  }

  // Dead branch: track nesting by name only; dispatch nothing except a
  // directive that can end this branch at depth 0. Everything else in a
  // false branch -- #include, #define, unknown directives -- is skipped
  // without validation, matching the old preprocessor's dead-branch
  // behavior.
  if (dir == "if" || dir == "ifdef" || dir == "ifndef") {
    ctx->skip_depth++;
    return LpcDirectiveAction::kNone;
  }
  if (ctx->skip_depth > 0) {
    if (dir == "endif") {
      ctx->skip_depth--;
    }
    return LpcDirectiveAction::kNone;
  }
  if (dir == "endif" || dir == "elif" || dir == "else") {
    compiler_directive_start_line = directive_line;
    dispatch_directive(dir, rest, yyscanner);
    compiler_directive_start_line = 0;
    return lpc_lex_emitting() ? LpcDirectiveAction::kExitSkip : LpcDirectiveAction::kNone;
  }
  return LpcDirectiveAction::kNone;
}
