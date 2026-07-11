#include "base/std.h"

#include "compiler/internal/stage_output.h"

#include "compiler/internal/compiler.h"
#include "compiler/internal/lexer.h"
#include "compiler/internal/grammar_rules.h"
#include "compiler/internal/grammar.autogen.h"
#include "compiler/internal/lexer_rules_pp.h"
#include "compiler/internal/scratchpad.h"

// ---------------------------------------------------------------------------
// lpcc's pre-parse stage dumps: pull tokens through the real
// reentrant scanner (lexing + preprocessing, no parser) and print them.
// The pp form is token-reconstructed text -- with the single-scan design
// there IS no preprocessed text artifact, so this renders what the
// PARSER would see, gcc -E shaped (line structure preserved from token
// positions; strings/chars re-quoted).
// ---------------------------------------------------------------------------

namespace {

// Re-quote a string literal's decoded payload for display.
void print_quoted(const char* s, size_t n, char quote, FILE* out) {
  fputc(quote, out);
  for (size_t i = 0; i < n; i++) {
    char c = s[i];
    switch (c) {
      case '\n':
        fputs("\\n", out);
        break;
      case '\t':
        fputs("\\t", out);
        break;
      case '\r':
        fputs("\\r", out);
        break;
      case '\\':
        fputs("\\\\", out);
        break;
      default:
        if (c == quote) {
          fputc('\\', out);
          fputc(c, out);
        } else {
          fputc(c, out);
        }
    }
  }
  fputc(quote, out);
}

void print_token_pp(int kind, const YYSTYPE& lval, compiler_context_t* ctx, const char* text,
                    FILE* out) {
  switch (kind) {
    case L_STRING:
      if (lval.string != nullptr) {
        print_quoted(lval.string->data(), lval.string->size(), ctx->is_template ? '`' : '"', out);
      }
      break;
    case L_TEMPLATE_HEAD:
    case L_TEMPLATE_MIDDLE:
    case L_TEMPLATE_TAIL: {
      const char* open = (kind == L_TEMPLATE_HEAD) ? "`" : "}";
      const char* close = (kind == L_TEMPLATE_TAIL) ? "`" : "${";
      fputs(open, out);
      if (lval.string != nullptr) {
        fwrite(lval.string->data(), 1, lval.string->size(), out);
      }
      fputs(close, out);
      break;
    }
    case L_NUMBER:
      if (ctx->is_char_literal) {
        fprintf(out, "'%c'", static_cast<char>(lval.number));
      } else {
        fprintf(out, "%" PRIu64, static_cast<uint64_t>(lval.number));
      }
      break;
    case L_REAL:
      fprintf(out, "%g", lval.real);
      break;
    case L_IDENTIFIER:
      if (lval.string != nullptr) fwrite(lval.string->data(), 1, lval.string->size(), out);
      break;
    case L_DEFINED_NAME:
      if (lval.ihe != nullptr && lval.ihe->name != nullptr) fputs(lval.ihe->name, out);
      break;
    default:
      fputs(text, out);
  }
}

}  // namespace

bool lpc_dump_stage_tokens(int fd, const char* name, bool pp_form, FILE* out) {
  // Minimal compile-shaped environment: the lexer's bookkeeping
  // (add_program_file / save_file_info for #include accounting) writes
  // into the compiler mem_block areas that prolog() normally allocates.
  for (int i = 0; i < NUMAREAS; i++) {
    mem_block[i].block =
        reinterpret_cast<char*>(DMALLOC(START_BLOCK_SIZE, TAG_COMPILER, "stage_output"));
    mem_block[i].current_size = 0;
    mem_block[i].max_size = START_BLOCK_SIZE;
  }
  // Shared-string table discipline (same as the test harness): zeroed
  // tags mark every hash chain empty, so store_prog_string() never walks
  // stale string_idx entries into a previous call's FREED A_STRINGS block
  // -- without this, the SECOND dump in a process spun forever in
  // release builds (caught by the StageOutput tests run back to back).
  memset(string_tags, 0, sizeof(string_tags));
  freed_string = -1;
  num_parse_error = 0;
  ScratchString norm = normalize_filename(name);
  current_file = make_shared_string(norm.c_str());
  current_file_id = add_program_file(norm.c_str(), /*top=*/1);

  compiler_context_t lex_ctx;
  void* scanner = nullptr;
  yylex_init_extra(&lex_ctx, &scanner);
  bool ok = start_new_file_fd(fd, scanner);
  if (!ok) {
    fprintf(stderr, "lpcc: cannot read %s\n", name);
  }

  compiler_context_t* ctx = &lex_ctx;
  YYSTYPE lval;
  int last_line = 1;
  bool line_start = true;
  while (ok) {
    YYLTYPE lloc;
    int kind = yylex(&lval, &lloc, scanner);
    if (kind <= 0) break;
    if (pp_form) {
      // Preserve source line structure from token positions.
      if (lloc.first_line > last_line) {
        int gap = lloc.first_line - last_line;
        for (int i = 0; i < (gap > 2 ? 2 : gap); i++) fputc('\n', out);
        last_line = lloc.first_line;
        line_start = true;
      }
      if (!line_start) fputc(' ', out);
      print_token_pp(kind, lval, ctx, yyget_text(scanner), out);
      line_start = false;
    } else {
      fprintf(out, "%5d:%-4d %5d  ", lloc.first_line, lloc.first_column, kind);
      print_token_pp(kind, lval, ctx, yyget_text(scanner), out);
      fputc('\n', out);
    }
    ctx->is_char_literal = false;
    ctx->is_template = false;
  }
  if (ok && pp_form) fputc('\n', out);

  // Unified cleanup for success AND load failure: buffers down first,
  // then the scanner, then the compile-shaped environment.
  lpc_lex_teardown_active();
  lpc_lex_scanner_destroyed(scanner);
  yylex_destroy(scanner);
  free_string(const_cast<char*>(current_file));
  current_file = nullptr;
  for (int i = 0; i < NUMAREAS; i++) {
    FREE(mem_block[i].block);
  }
  return ok;
}
