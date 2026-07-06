/*
 * File: lexer_utils.cc
 *
 * Combines the lexer's global state and non-scanning helpers (identifier
 * hash table, ring-buffer management, heredoc body matching, pragma/error-
 * context handling) with the standalone-preprocessor support helpers
 * (predefines registry, include-path resolution). Originally two separate
 * files -- lex.c/lex.cc (renamed to lex_util.cc once the Flex migration in
 * lex.l left only this residual state+helpers behind) and lexer_utils.cc --
 * merged here since both serve the same "lexer support" role. See
 * plans/flex-lexer-migration.md.
 *
 * Revision:
 * 93-06-27 (Robocoder):
 *   Adjusted the meaning of the EXPECT_* flags;
 *     EXPECT_ELSE  ... means the last condition was false, so we want to find
 *                      an alternative or the end of the conditional block
 *     EXPECT_ENDIF ... means the last condition was true, so we want to find
 *                      the end of the conditional block
 *   Added #elif preprocessor command
 *   Fixed get_text_block bug so no text returned ""
 *   Added get_array_block()...using @@ENDMARKER to return array of strings
 */

#include "base/std.h"

#include "compiler/internal/lexer_utils.h"
#include "compiler/internal/lex.h"

#include <cstdio>    // for EOF
#include <fcntl.h>   // for O_RDONLY etc
#include <cstdlib>   // for exit(), FIXME
#include <cctype>    // for isspace
#include <unistd.h>  // for read(), FIXME
#include <vector>
#include <algorithm>  // for std::sort
#include <sstream>
#include <unicode/ustring.h>
#include <fmt/format.h>

#include "vm/vm.h"
#include "include/function.h"
#include "efuns.autogen.h"
#include "vm/internal/base/program.h"
#include "vm/internal/base/svalue.h"
#include "compiler.h"
#include "keyword.h"
#include "options.autogen.h"
#include "base/internal/file.h"

#include "compiler/internal/grammar_rules.h"
#include "grammar.autogen.h"

#include "scratchpad.h"

#include "symbol.h"
#include "compiler/internal/LexStream.h"
#include "compiler/internal/preprocessor.h"

#ifdef _WIN32
#include <io.h>
#endif

// FIXME: in master.h
extern struct object_t *master_ob;
// FIXME: in file.h
extern const char *check_valid_path(const char *, object_t *, const char *const, int);

// FIXME: lexer() is using global stack machine?!
void push_malloced_string(const char *p);
void pop_stack();

// FIXME: lexer needs a list of predefines
extern int NUM_OPTION_DEFS;

#define NELEM(a) (sizeof(a) / sizeof((a)[0]))

// FIXME: This means current source code can not contain "NUL" byte,
//  for now it seems suffice, but this should be fixed to check pointer address
//  for EOF, not for value.
#define LEX_EOF ((unsigned char)0)

char lex_ctype[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define is_wspace(c) lex_ctype[(unsigned char)(c)]

#define SKIPWHITE \
  while (is_wspace((unsigned char)*p) && (*p != '\n')) p++

int current_line;       /* line number in this file */
int current_line_base;  /* number of lines from other files */
int current_line_saved; /* last line in this file where line num
                           info was saved */
int total_lines;        /* Used to compute average compiled lines/s */
const char *current_file;
int current_file_id;
static const char *main_filename = nullptr;

/* Bit flags for pragmas in effect */
int pragmas;

int num_parse_error; /* Number of errors in the parser. */

lpc_predef_t *lpc_predefs = nullptr;

namespace {
std::unique_ptr<LexStream> current_stream = nullptr;
}  // namespace

int lex_fatal;
static char *last_nl;

// yytext is now defined by the Flex-generated lex.autogen.cc.
// YYLMAX (8192) >= MAXLINE (4096) so existing writes are safe.
char *outp;

typedef struct incstate_s {
  struct incstate_s *next;
  LexStream *stream;  // raw pointer because incstate_t is alloated using malloc
  int line;
  const char *file;
  int file_id;
  char *last_nl;
  char *outp;
} incstate_t;

static incstate_t *inctop = nullptr;

/* prevent unbridled recursion */
#define MAX_INCLUDE_DEPTH 32
static int incnum;

/* If more than this is needed, the code needs help :-) */
#define MAX_FUNCTION_DEPTH 10

static function_context_t function_context_stack[MAX_FUNCTION_DEPTH];
static int last_function_context;
function_context_t *current_function_context = nullptr;

int arrow_efun, evaluate_efun, this_efun, to_float_efun, to_int_efun, new_efun;

/*
 * The number of arguments stated below, are used by the compiler.
 * If min == max, then no information has to be coded about the
 * actual number of arguments. Otherwise, the actual number of arguments
 * will be stored in the byte after the instruction.
 * A maximum value of -1 means unlimited maximum value.
 *
 * If an argument has type 0 (T_INVALID) specified, then no checks will
 * be done at run time.
 *
 * The argument types are currently not checked by the compiler,
 * only by the runtime.
 */

static keyword_t reswords[] = {
#ifdef DEBUG
    {"__TREE__", L_TREE, 0},
#endif
#ifdef ARRAY_RESERVED_WORD
    {"array", L_ARRAY, 0},
#endif
    {"asm", 0, 0},
    {"break", L_BREAK, 0},
    {"buffer", L_BASIC_TYPE, TYPE_BUFFER},
    {"case", L_CASE, 0},
    {"catch", L_CATCH, 0},
#ifdef STRUCT_CLASS
    {"class", L_CLASS, 0},
#endif
    {"continue", L_CONTINUE, 0},
    {"default", L_DEFAULT, 0},
    {"do", L_DO, 0},
    {"efun", L_EFUN, 0},
    {"else", L_ELSE, 0},
    {"float", L_BASIC_TYPE, TYPE_REAL},
    {"for", L_FOR, 0},
    {"foreach", L_FOREACH, 0},
    {"function", L_BASIC_TYPE, TYPE_FUNCTION},
    {"if", L_IF, 0},
    {"in", L_IN, 0},
    {"inherit", L_INHERIT, 0},
    {"int", L_BASIC_TYPE, TYPE_NUMBER},
    {"mapping", L_BASIC_TYPE, TYPE_MAPPING},
    {"mixed", L_BASIC_TYPE, TYPE_ANY},
    {"new", L_NEW, 0},
    {"nomask", L_TYPE_MODIFIER, DECL_NOMASK},
#ifdef SENSIBLE_MODIFIERS
    {"nosave", L_TYPE_MODIFIER, DECL_NOSAVE},
#endif
    {"object", L_BASIC_TYPE, TYPE_OBJECT},
    {"parse_command", L_PARSE_COMMAND, 0},
    {"private", L_TYPE_MODIFIER, DECL_PRIVATE},
    {"protected", L_TYPE_MODIFIER, DECL_PROTECTED},
#ifdef SENSIBLE_MODIFIERS
    {"public", L_TYPE_MODIFIER, DECL_PUBLIC},
#else
    {"public", L_TYPE_MODIFIER, DECL_VISIBLE},
#endif
#ifdef REF_RESERVED_WORD
    {"ref", L_REF, 0},
#endif
    {"return", L_RETURN, 0},
    {"sscanf", L_SSCANF, 0},
#ifndef SENSIBLE_MODIFIERS
    {"static", L_TYPE_MODIFIER, DECL_NOSAVE | DECL_PROTECTED},
#endif
    {"string", L_BASIC_TYPE, TYPE_STRING},
#ifdef STRUCT_STRUCT
    {"struct", L_CLASS, 0},
#endif
    {"switch", L_SWITCH, 0},
    {"time_expression", L_TIME_EXPRESSION, 0},
    {"varargs", L_TYPE_MODIFIER, FUNC_VARARGS},
    {"void", L_BASIC_TYPE, TYPE_VOID},
    {"while", L_WHILE, 0},
};

// Used to determine valid ID chars (variable name function name etc)
#define isalunum(c) (uisalnum(c) || (c) == '_')

static ident_hash_elem_t **ident_hash_table;
static ident_hash_elem_t **ident_hash_head;
static ident_hash_elem_t **ident_hash_tail;

static ident_hash_elem_t *ident_dirty_list = nullptr;

instr_t instrs[MAX_INSTRS];

#define TERM_ADD_INPUT 1
#define TERM_INCLUDE 2
#define TERM_START 4

typedef struct linked_buf_s {
  struct linked_buf_s *prev;
  char term_type;
  char buf[DEFMAX];
  char *buf_end;
  char *outp;
  char *last_nl;
} linked_buf_t;

static linked_buf_t head_lbuf = {nullptr, TERM_START};
static linked_buf_t *cur_lbuf;

static void add_input(const char * /*p*/);
static int get_array_block(char * /*term*/);
static int get_text_block(char * /*term*/);
static void refill_buffer(void);
static int old_func(void);
static ident_hash_elem_t *quick_alloc_ident_entry(void);

int lookup_predef(const char *name) {
  unsigned int x;

  for (x = 0; x < size_of_predefs; x++) {
    if (strcmp(name, predefs[x].word) == 0) {
      return x;
    }
  }

  return -1;
}



// Non-static: called directly from lex.l's native $N and open-paren rules
// (Phase 6), in addition to the legacy helpers still in this file.
void lexerror(const char *s) {
  yyerror(s);
  lex_fatal++;
}



#define MAXCHUNK (MAXLINE * 4)
#define NUMCHUNKS (DEFMAX / MAXCHUNK)

#define NEWCHUNK(line)                                                              \
  if (len == MAXCHUNK - 1) {                                                        \
    line[curchunk][MAXCHUNK - 1] = '\0';                                            \
    if (curchunk == NUMCHUNKS - 1) {                                                \
      res = -2;                                                                     \
      break;                                                                        \
    }                                                                               \
    line[++curchunk] = (char *)DMALLOC(MAXCHUNK, TAG_COMPILER, "array/text chunk"); \
    len = 0;                                                                        \
  }

static int get_array_block(char *term) {
  unsigned int termlen;        /* length of terminator */
  char *array_line[NUMCHUNKS]; /* allocate memory in chunks */
  int header, len;             /* header length; position in chunk */
  int startpos, startchunk;    /* start of line */
  int curchunk, res;           /* current chunk; this function's result */
  int i;                       /* an index counter */
  unsigned char c;             /* a char */
  char *yyp = outp;

  /*
   * initialize
   */
  termlen = strlen(term);
  array_line[0] = reinterpret_cast<char *>(DMALLOC(MAXCHUNK, TAG_COMPILER, "array_block"));
  array_line[0][0] = '(';
  array_line[0][1] = '{';
  array_line[0][2] = '"';
  array_line[0][3] = '\0';
  header = 1;
  len = 3;
  startpos = 3;
  startchunk = 0;
  curchunk = 0;
  res = 0;

  while (true) {
    while (((c = *yyp++) != '\n') && (c != LEX_EOF)) {
      NEWCHUNK(array_line);
      if (c == '"' || c == '\\') {
        array_line[curchunk][len++] = '\\';
        NEWCHUNK(array_line);
      }
      array_line[curchunk][len++] = c;
    }

    if (c == '\n' && (yyp == last_nl + 1)) {
      outp = yyp;
      refill_buffer();
      yyp = outp;
    }

    /*
     * null terminate current chunk
     */
    array_line[curchunk][len] = '\0';

    if (res) {
      outp = yyp;
      break;
    }

    /*
     * check for terminator
     */
    if ((!strncmp(array_line[startchunk] + startpos, term, termlen)) &&
        (!uisalnum(*(array_line[startchunk] + startpos + termlen))) &&
        (*(array_line[startchunk] + startpos + termlen) != '_')) {
      /*
       * handle lone terminator on line
       */
      if (strlen(array_line[startchunk] + startpos) == termlen) {
        current_line++;
        outp = yyp;
      } else {
        /*
         * put back trailing portion after terminator
         */
        outp = --yyp; /* some operating systems give EOF only once */

        for (i = curchunk; i > startchunk; i--) {
          add_input(array_line[i]);
        }
        add_input(array_line[startchunk] + startpos + termlen);
      }

      /*
       * remove terminator from last chunk
       */
      array_line[startchunk][startpos - header] = '\0';

      /*
       * insert array block into input stream
       */
      *--outp = ')';
      *--outp = '}';
      for (i = startchunk; i >= 0; i--) {
        add_input(array_line[i]);
      }

      res = 1;
      break;
    } else {
      /*
       * only report end of file in array block, if not an include file
       */
      if (c == LEX_EOF && inctop == nullptr) {
        res = -1;
        outp = yyp;
        break;
      }
      if (c == '\n') {
        current_line++;
      }
      /*
       * make sure there's room in the current chunk for terminator (ie
       * it's simpler if we don't have to deal with a terminator that
       * spans across chunks) fudge for "\",\"TERMINAL?\0", where '?'
       * is unknown
       */
      if (len + termlen + 6 > MAXCHUNK) {
        if (curchunk == NUMCHUNKS - 1) {
          res = -2;
          outp = yyp;
          break;
        }
        array_line[++curchunk] =
            reinterpret_cast<char *>(DMALLOC(MAXCHUNK, TAG_COMPILER, "array_block"));
        len = 0;
      }
      // Remove trailing CR
      if (array_line[curchunk][len - 1] == '\r') {
        array_line[curchunk][len - 1] = '\0';
        len = len - 1;
      }
      /*
       * header
       */
      array_line[curchunk][len++] = '"';
      array_line[curchunk][len++] = ',';
      array_line[curchunk][len++] = '\n';
      array_line[curchunk][len++] = '"';
      array_line[curchunk][len] = '\0';

      startchunk = curchunk;
      startpos = len;
      header = 2;
    }
  }

  /*
   * free chunks
   */
  for (i = curchunk; i >= 0; i--) {
    FREE(array_line[i]);
  }

  return res;
}

static int get_text_block(char *term) {
  unsigned int termlen;       /* length of terminator */
  char *text_line[NUMCHUNKS]; /* allocate memory in chunks */
  int len;                    /* position in chunk */
  int startpos, startchunk;   /* start of line */
  int curchunk, res;          /* current chunk; this function's result */
  int i;                      /* an index counter */
  unsigned char c;            /* a char */
  char *yyp = outp;

  /*
   * initialize
   */
  termlen = strlen(term);
  text_line[0] = reinterpret_cast<char *>(DMALLOC(MAXCHUNK, TAG_COMPILER, "text_block"));
  text_line[0][0] = '"';
  text_line[0][1] = '\0';
  len = 1;
  startpos = 1;
  startchunk = 0;
  curchunk = 0;
  res = 0;

  while (true) {
    while (((c = *yyp++) != '\n') && (c != LEX_EOF)) {
      NEWCHUNK(text_line);
      if (c == '"' || c == '\\') {
        text_line[curchunk][len++] = '\\';
        NEWCHUNK(text_line);
      }
      text_line[curchunk][len++] = c;
    }

    if (c == '\n' && yyp == last_nl + 1) {
      outp = yyp;
      refill_buffer();
      yyp = outp;
    }

    /*
     * null terminate current chunk
     */
    text_line[curchunk][len] = '\0';

    if (res) {
      outp = yyp;
      break;
    }

    /*
     * check for terminator
     */
    if ((!strncmp(text_line[startchunk] + startpos, term, termlen)) &&
        (!uisalnum(*(text_line[startchunk] + startpos + termlen))) &&
        (*(text_line[startchunk] + startpos + termlen) != '_')) {
      if (strlen(text_line[startchunk] + startpos) == termlen) {
        current_line++;
        outp = yyp;
      } else {
        char *p, *q;
        /*
         * put back trailing portion after terminator
         */
        outp = --yyp; /* some operating systems give EOF only once */

        for (i = curchunk; i > startchunk; i--) {
          /* Ick.  go back and unprotect " and \ */
          p = text_line[i];
          while (*p && *p != '\\') {
            p++;
          }
          if (*p) {
            q = p++;
            do {
              *q++ = *p++;
              if (*p == '\\') {
                p++;
              }
            } while (*p);
            *q = 0;
          }

          add_input(text_line[i]);
        }
        p = text_line[startchunk] + startpos + termlen;
        while (*p && *p != '\\') {
          p++;
        }
        if (*p) {
          q = p++;
          do {
            *q++ = *p++;
            if (*p == '\\') {
              p++;
            }
          } while (*p);
          *q = 0;
        }
        add_input(text_line[startchunk] + startpos + termlen);
      }

      /*
       * remove terminator from last chunk
       */
      text_line[startchunk][startpos] = '\0';

      /*
       * insert text block into input stream
       */
      *--outp = '\0';
      *--outp = '"';

      for (i = startchunk; i >= 0; i--) {
        add_input(text_line[i]);
      }

      res = 1;
      break;
    } else {
      /*
       * only report end of file in text block, if not an include file
       */
      if (c == LEX_EOF && inctop == nullptr) {
        res = -1;
        outp = yyp;
        break;
      }
      if (c == '\n') {
        current_line++;
      }
      if (len > 0) {
        // Remove trailing CR
        if (text_line[curchunk][len - 1] == '\r') {
          text_line[curchunk][len - 1] = '\0';
          len = len - 1;
        }
      }
      /*
       * make sure there's room in the current chunk for terminator (ie
       * it's simpler if we don't have to deal with a terminator that
       * spans across chunks) fudge for "\\nTERMINAL?\0", where '?' is
       * unknown
       */
      if (len + termlen + 4 > MAXCHUNK) {
        if (curchunk == NUMCHUNKS - 1) {
          res = -2;
          outp = yyp;
          break;
        }
        text_line[++curchunk] =
            reinterpret_cast<char *>(DMALLOC(MAXCHUNK, TAG_COMPILER, "text_block"));
        len = 0;
      }
      /*
       * header
       */
      text_line[curchunk][len++] = '\\';
      text_line[curchunk][len++] = 'n';
      text_line[curchunk][len] = '\0';

      startchunk = curchunk;
      startpos = len;
    }
  }

  /*
   * free chunks
   */
  for (i = curchunk; i >= 0; i--) {
    FREE(text_line[i]);
  }

  return res;
}

// Called from lex.l's "\n" rule (Phase 3+): mirrors the newline bookkeeping
// that used to run inline in yylex_inner()'s switch.
//
// Uses >= rather than == : Flex's DFA always fetches one lookahead byte
// past whatever it just matched (even for an unambiguous single-char rule,
// to confirm the match can't be extended), so by the time this runs, `outp`
// may already be one byte past `last_nl + 1` -- an exact-equality check
// would be skipped entirely, causing refill_buffer() to never fire.
void lpc_lex_newline() {
  current_line++;
  total_lines++;
  if (outp >= last_nl + 1) {
    refill_buffer();
  }
}

#define SAVEC                     \
  if (yyp < yytext + MAXLINE - 5) \
    *yyp++ = c;                   \
  else {                          \
    lexerror("Line too long");    \
    break;                        \
  }

typedef struct {
  const char *name;
  int value;
} pragma_t;

static pragma_t our_pragmas[] = {{"strict_types", PRAGMA_STRICT_TYPES},
                                 {"save_types", PRAGMA_SAVE_TYPES},
                                 {"warnings", PRAGMA_WARNINGS},
                                 {"optimize", PRAGMA_OPTIMIZE},
                                 {"show_error_context", PRAGMA_ERROR_CONTEXT},
                                 {nullptr, 0}};

void handle_pragma(char *str) {
  int i;
  int no_flag;

  if (!*str) {
    yywarn("Unknown #pragma, ignored.");
    return;
  }

  // Ignore trailing whitespaces
  char *p = &str[strlen(str) - 1];
  while (p >= str && iswspace(*p)) {
    *p-- = '\0';
  }
  if (!*str) {
    yywarn("Unknown #pragma, ignored.");
    return;
  }

  if (strncmp(str, "no_", 3) == 0) {
    str += 3;
    no_flag = 1;
  } else {
    no_flag = 0;
  }

  for (i = 0; our_pragmas[i].name; i++) {
    if (strcmp(our_pragmas[i].name, str) == 0) {
      if (no_flag) {
        pragmas &= ~our_pragmas[i].value;
      } else {
        pragmas |= our_pragmas[i].value;
      }
      return;
    }
  }
  yywarn("Unknown #pragma, ignored.");
}

char *show_error_context() {
  static char buf[60];
  extern int yychar;
  char sub_context[25];
  char *yyp, *yyp2;
  int len;

  if (static_cast<unsigned char>(outp[-1]) == LEX_EOF) {
    strcpy(buf, " at the end of the file\n");
    return buf;
  }

  if (yychar == -1) {
    strcpy(buf, " around ");
  } else {
    strcpy(buf, " before ");
  }
  yyp = outp;
  yyp2 = sub_context;
  len = 20;
  while (len--) {
    if (*yyp == '\n') {
      if (len == 19) {
        strcat(buf, "the end of line");
      }
      break;
    } else if (static_cast<unsigned char>(*yyp) == LEX_EOF) {
      if (len == 19) {
        strcat(buf, "the end of file");
      }
      break;
    }
    *yyp2++ = *yyp++;
  }
  *yyp2 = 0;
  if (yyp2 != sub_context) {
    strcat(buf, sub_context);
  }
  strcat(buf, "\n");
  return buf;
}

std::vector<std::string> prepare_logs(const char *error_file, int line, const char *what, int flag,
                                      bool include_error_context) {
  std::vector<std::string> logs;
  logs.emplace_back(fmt::format(FMT_STRING("/{} line {}: {}{}\n"), error_file, line,
                                flag ? "Warning: " : "", what));

  if (include_error_context) {
    if (static_cast<unsigned char>(outp[-1]) != LEX_EOF) {
      const char *start = outp;
      while (start != &cur_lbuf->buf[0]) {
        if (start[-1] == '\n' || start[-1] == LEX_EOF) {
          break;
        }
        start--;
      }

      const char *end = outp;
      while (end != cur_lbuf->buf_end && *end != LEX_EOF) {
        if (end[1] == '\n' || end[1] == LEX_EOF) {
          break;
        }
        end++;
      }

      auto size = end - start;
      if (size > 0) {
        bool truncated = false;
        if (size > 120) {
          size = 117;
          truncated = true;
        }
        std::string content{start, static_cast<std::string::size_type>(size)};
        if (truncated) content += "...";
        content = trim(content);
        logs.emplace_back(fmt::format(FMT_STRING("  {}\n"), content));
        logs.emplace_back(fmt::format(
            FMT_STRING("  {}^\n"), std::string(truncated ? content.size() : (outp - start), ' ')));
      }
    }
  }

  return logs;
}

static void refill_buffer() {
  if (cur_lbuf != &head_lbuf) {
    if (outp >= cur_lbuf->buf_end && cur_lbuf->term_type == TERM_ADD_INPUT) {
      /* In this case it cur_lbuf cannot have been
         allocated due to #include */
      linked_buf_t *prev_lbuf = cur_lbuf->prev;

      FREE(cur_lbuf);
      cur_lbuf = prev_lbuf;
      outp = cur_lbuf->outp;
      last_nl = cur_lbuf->last_nl;
      if (cur_lbuf->term_type == TERM_ADD_INPUT || (outp != last_nl + 1)) {
        return;
      }
    }
  }

  /* Here we are sure that we need more from the file */
  /* Assume outp is one beyond a newline at last_nl */
  /* or after an #include .... */

  {
    char *end;
    char *p;
    int size;

    if (!inctop) {
      /* First check if there's enough space at the end */
      end = cur_lbuf->buf + DEFMAX;
      if (end - cur_lbuf->buf_end > MAXLINE + 5) {
        p = cur_lbuf->buf_end;
      } else {
        /* No more space at the end */
        size = cur_lbuf->buf_end - outp + 1; /* Include newline */
        memcpy(outp - MAXLINE - 1, outp - 1, size);
        outp -= MAXLINE;
        p = outp + size - 1;
      }

      size = current_stream->read(p, MAXLINE);
      cur_lbuf->buf_end = p += size;
      if (size < MAXLINE) {
        *(last_nl = p) = LEX_EOF;
        if (*(last_nl - 1) != '\n') {
          if (size + 1 > MAXLINE) {
            lexerror("No newline at end of file.");
          }
          *p++ = '\n';
          *(last_nl = p) = LEX_EOF;
        }
        return;
      }
      while (*--p != '\n') {
        ;
      }
      if (p == outp - 1) {
        lexerror("Line too long.");
        *(last_nl = cur_lbuf->buf_end - 1) = '\n';
        return;
      }
      last_nl = p;
      return;
    } else {
      int flag = 0;

      /* We are reading from an include file */
      /* Is there enough space? */
      end = inctop->outp;

      /* See if we are the last include in a different linked buffer */
      if (cur_lbuf->term_type == TERM_INCLUDE &&
          !(end >= cur_lbuf->buf && end < cur_lbuf->buf + DEFMAX)) {
        end = cur_lbuf->buf_end;
        flag = 1;
      }

      size = end - outp + 1; /* Include newline */
      if (outp - cur_lbuf->buf > 2 * MAXLINE) {
        memcpy(outp - MAXLINE - 1, outp - 1, size);
        outp -= MAXLINE;
        p = outp + size - 1;
      } else { /* No space, need to allocate new buffer */
        linked_buf_t *new_lbuf;
        char *new_outp;

        if (!(new_lbuf = reinterpret_cast<linked_buf_t *>(
                  DMALLOC(sizeof(linked_buf_t), TAG_COMPILER, "refill_bufer")))) {
          lexerror("Out of memory when allocating new buffer.\n");
          return;
        }
        cur_lbuf->last_nl = last_nl;
        cur_lbuf->outp = outp;
        new_lbuf->prev = cur_lbuf;
        new_lbuf->term_type = TERM_INCLUDE;
        new_outp = new_lbuf->buf + DEFMAX - MAXLINE - size - 5;
        memcpy(new_outp - 1, outp - 1, size);
        cur_lbuf = new_lbuf;
        outp = new_outp;
        p = outp + size - 1;
        flag = 1;
      }

      size = current_stream->read(p, MAXLINE);
      end = p += size;
      if (flag) {
        cur_lbuf->buf_end = p;
      }
      if (size < MAXLINE) {
        *(last_nl = p) = LEX_EOF;
        if (*(last_nl - 1) != '\n') {
          if (size + 1 > MAXLINE) {
            lexerror("No newline at end of file.");
          }
          *p++ = '\n';
          *(last_nl = p) = LEX_EOF;
        }
        return;
      }
      while (*--p != '\n') {
        ;
      }
      if (p == outp - 1) {
        lexerror("Line too long.");
        *(last_nl = end - 1) = '\n';
        return;
      }
      last_nl = p;
      return;
    }
  }
}

// Non-static: set directly by lex.l's SC_FUNC_OPEN identifier rule (Phase 6)
// before calling lpc_lex_resolve_identifier(), same as the legacy "(:name"
// accumulation loop used to do.
int function_flag = 0;

void push_function_context() {
  function_context_t *fc;
  parse_node_t *node;

  if (last_function_context == MAX_FUNCTION_DEPTH - 1) {
    lexerror("Function pointers nested too deep.");
    return;
  }
  fc = &function_context_stack[++last_function_context];
  fc->num_parameters = 0;
  fc->num_locals = 0;
  node = new_node_no_line();
  node->l.expr = node;
  node->r.expr = nullptr;
  node->kind = 0;
  fc->values_list = node;
  fc->bindable = 0;
  fc->parent = current_function_context;

  current_function_context = fc;
}

void pop_function_context() {
  current_function_context = current_function_context->parent;
  last_function_context--;
}

static int old_func() {
  add_input(" ");
  add_input(yytext);
  push_function_context();
  return L_FUNCTION_OPEN;
}

// Resolves the identifier currently sitting in the global `yytext` (already
// NUL-terminated) to a token: reserved word, function-pointer reference
// (only when `function_flag` is set -- exclusively true when called from
// the '(' case's "(: name" lookahead, never when Flex matches a plain
// identifier), a known defined name, or a fresh L_IDENTIFIER. Shared by
// lex.l's identifier rule and the '(' case in yylex_inner() so both paths
// -- Flex-driven and the legacy "(: name" lookahead -- agree on lookup
// semantics.
int lpc_lex_resolve_identifier() {
  ident_hash_elem_t *ihe;
  if ((ihe = lookup_ident(yytext))) {
    if (ihe->token & IHE_RESWORD) {
      if (function_flag) {
        function_flag = 0;
        add_input(yytext);
        push_function_context();
        return L_FUNCTION_OPEN;
      }
      yylval.number = ihe->sem_value;
      return ihe->token & TOKEN_MASK;
    }
    if (function_flag) {
      int val;
      unsigned char c;

      function_flag = 0;
      while ((c = *outp++) == ' ') {
        ;
      }
      outp--;
      /* Note that this gets code like:
       * #define x :)
       * #define y ,
       * function f = (: foo x;
       * function g = (: bar y 1 :);
       *
       * wrong.  But that is almost pathological.
       */
      if (c != ':' && c != ',') {
        return old_func();
      }
      if ((val = ihe->dn.local_num) >= 0) {
        if (c == ',') {
          return old_func();
        }
        yylval.number = (val << 8) | FP_L_VAR;
      } else if ((val = ihe->dn.global_num) >= 0) {
        if (c == ',') {
          return old_func();
        }
        yylval.number = (val << 8) | FP_G_VAR;
      } else if ((val = ihe->dn.function_num) >= 0) {
        yylval.number = (val << 8) | FP_LOCAL;
      } else if ((val = ihe->dn.simul_num) >= 0) {
        yylval.number = (val << 8) | FP_SIMUL;
      } else if ((val = ihe->dn.efun_num) >= 0) {
        yylval.number = (val << 8) | FP_EFUN;
      } else {
        return old_func();
      }
      return L_NEW_FUNCTION_OPEN;
    }
    yylval.ihe = ihe;
    return L_DEFINED_NAME;
  }
  if (function_flag) {
    function_flag = 0;
    add_input("(:");
  }
  yylval.string = scratch_copy(yytext);
  return L_IDENTIFIER;
}

// Called from lex.l's SC_HEREDOC_TERM rules once the "@"/"@@" prefix and the
// terminator identifier have already been recognized natively (the
// terminator is supplied by the LPC source at compile time, so matching the
// *body* against it can't be a static Flex pattern -- this is the one piece
// of lexing that structurally can't move to lex.l; see lex.l's file header).
// Handles both heredoc forms: "@TERM ... TERM" (a plain string, via
// get_text_block()) and "@@TERM ... TERM" (an array of lines, via
// get_array_block() -- Robocoder's "@@" block, which lands on a literal
// "({" for historical reasons). On a recoverable error (bad UTF-8, oversized
// block), lexerror() just logs and returns (it doesn't longjmp the way the
// runtime error() does), so those paths must resume the top-level scanner
// via `return yylex()` rather than `break`ing a switch that no longer
// exists here.
int parseHeredoc(const char *terminator, int is_array) {
  int rc;

  if (is_array) {
    rc = get_array_block(const_cast<char *>(terminator));

    if (rc > 0) {
      /* outp is pointing at "({" for histortical reasons */
      outp += 2;
      return L_ARRAY_OPEN;
    } else if (rc == -1) {
      lexerror("End of file in array block");
      return YYEOF;
    } else { /* if rc == -2 */
      lexerror("Array block exceeded maximum length");
      return YYerror;
    }
  } else {
    rc = get_text_block(const_cast<char *>(terminator));

    if (rc > 0) {
      int n;

      auto *p = outp;
      if (!u8_validate(&p)) {
        lexerror("Bad UTF-8 string in string block");
        outp = p;
        return yylex();
      }
      /*
       * make string token and clean up
       */
      yylval.string = scratch_copy_string(outp);

      n = strlen(outp) + 1;
      outp += n;

      return L_STRING;
    } else if (rc == -1) {
      lexerror("End of file in text block");
      return YYEOF;
    } else { /* if (rc == -2) */
      lexerror("Text block exceeded maximum length");
      return yylex();
    }
  }
}

// Shared "illegal character" fallback: reports (in DEBUG builds) and
// returns the same placeholder token the original inline `badlex:` label
// did. Called both from yylex_inner()'s remaining catch-all default case
// and from lex.l-triggered helpers whose own lookahead determined the
// input isn't well-formed (e.g. a '#' not at the start of a line).
int lpc_lex_badlex(unsigned char c) {
#ifdef DEBUG
  char buff[100];

  sprintf(buff, "Illegal character (hex %02x) '%c'", static_cast<unsigned>(c),
          static_cast<char>(c));
  yyerror(buff);
#endif
  return ' ';
}

// Called from lex.l's <<EOF>> rule. If we're inside an #include, pops the
// include stack and resumes scanning the including file (whitespace is
// Flex-only now, so this re-enters Flex's top-level scanner rather than
// looping internally the way the original switch-based version did).
// Otherwise this is genuine end of the top-level file: rewinds `outp` back
// onto the sentinel (matching the original's `outp--`) and returns -1.
int parseEofOrIncludePop() {
  if (inctop) {
    incstate_t *p;

    p = inctop;
    current_stream->close();
    save_file_info(current_file_id, current_line - current_line_saved);
    current_line_saved = p->line - 1;
    /* add the lines from this file, and readjust to be relative
       to the file we're returning to */
    current_line_base += current_line - current_line_saved;
    free_string(current_file);
    if (outp >= cur_lbuf->buf_end) {
      linked_buf_t *prev_lbuf;
      if ((prev_lbuf = cur_lbuf->prev)) {
        FREE(cur_lbuf);
        cur_lbuf = prev_lbuf;
      }
    }

    current_file = p->file;
    current_file_id = p->file_id;
    current_line = p->line;

    current_stream.reset(p->stream);
    p->stream = nullptr;
    last_nl = p->last_nl;
    outp = p->outp;
    inctop = p->next;
    incnum--;
    FREE((char *)p);
    outp[-1] = '\n';
    if (outp == last_nl + 1) {
      refill_buffer();
    }
    return yylex();
  }

  outp--;
  return -1;
}

extern YYSTYPE yylval;

void end_new_file() {
  while (inctop) {
    incstate_t *p;

    p = inctop;
    current_stream->close();
    current_stream.reset();
    free_string(current_file);
    current_file = p->file;
    current_stream.reset(p->stream);
    p->stream = nullptr;
    inctop = p->next;
    FREE((char *)p);
  }
  inctop = nullptr;
  if (main_filename) {
    free_string(const_cast<char *>(main_filename));
    main_filename = nullptr;
  }
  if (cur_lbuf != &head_lbuf) {
    linked_buf_t *prev_lbuf;

    while (cur_lbuf != &head_lbuf) {
      prev_lbuf = cur_lbuf->prev;
      FREE((char *)cur_lbuf);
      cur_lbuf = prev_lbuf;
    }
  }
}

void start_new_file(std::unique_ptr<LexStream> stream) {
  if (!main_filename && current_file) {
    main_filename = make_shared_string(current_file);
  }
  // 1. Read the incoming stream fully into a std::string
  std::string source_code;
  char buf[8192];
  size_t n;
  while ((n = stream->read(buf, sizeof(buf))) > 0) {
    source_code.append(buf, n);
  }
  stream->close();

  // 2. Prepend global include file if configured
  std::string merged_code;
  auto glf = CONFIG_STR(__GLOBAL_INCLUDE_FILE__);
  if (glf != nullptr && strlen(glf) > 0) {
    merged_code = "#include ";
    merged_code += glf;
    merged_code += "\n#line 1 \"";
    if (current_file) {
      merged_code += current_file;
    } else {
      merged_code += "unknown";
    }
    merged_code += "\"\n";
  }
  merged_code += source_code;

  // 3. Run the preprocessor
  std::string preprocessed;
  std::vector<std::string> prep_errors;
  {
    LpcPreprocessor preprocessor(std::move(merged_code), current_file);
    preprocessed = preprocessor.preprocess();
    prep_errors = preprocessor.errors();
  }

  // 4. Report preprocessor errors/warnings to the compiler
  for (const auto& err : prep_errors) {
    size_t colon = err.find(':');
    if (colon != std::string::npos) {
      int err_line = atoi(err.substr(0, colon).c_str());
      std::string msg = err.substr(colon + 2);
      int saved_line = current_line;
      current_line = err_line;
      yyerror(msg.c_str());
      current_line = saved_line;
    } else {
      yyerror(err.c_str());
    }
  }

  // 5. Feed the preprocessed output to the lexer
  current_stream = std::make_unique<StringLexStream>(std::move(preprocessed));

  lex_fatal = 0;
  last_function_context = -1;
  current_function_context = nullptr;
  cur_lbuf = &head_lbuf;
  cur_lbuf->outp = cur_lbuf->buf_end = outp = cur_lbuf->buf + (DEFMAX >> 1);
  *(last_nl = outp - 1) = '\n';
  pragmas = DEFAULT_PRAGMAS;
  incnum = 0;
  current_line = 1;
  current_line_base = 0;
  current_line_saved = 0;
  function_flag = 0;

  refill_buffer();
}

const char *query_instr_name(int instr) {
  const char *name;
  static char num_buf[20];

  // The param is clearly wrong, however to be safe, we just return something here.
  if (instr < 0 || instr >= (sizeof(instrs) / sizeof(instrs[0]))) {
    sprintf(num_buf, "op_invalid");
    return num_buf;
  }

  name = instrs[instr].name;

  if (name) {
    if (name[0] == '_') {
      name++;
    }
    return name;
  } else {
    sprintf(num_buf, "op%d", instr);
    return num_buf;
  }
}



/* IDEA: linked buffers, to allow "unlimited" buffer expansion */
static void add_input(const char *p) {
  int l = strlen(p);

  if (l >= DEFMAX - 10) {
    lexerror("Macro expansion buffer overflow");
    return;
  }

  if (outp < l + 5 + cur_lbuf->buf) {
    /* Not enough space, so let's move it up another linked_buf */
    linked_buf_t *new_lbuf;
    char *q, *new_outp, *buf;
    int size;

    q = outp;

    while (*q != '\n' && static_cast<unsigned char>(*q) != LEX_EOF) {
      q++;
    }
    /* Incorporate EOF later */
    if (*q != '\n' || ((q - outp) + l) >= DEFMAX - 11) {
      lexerror("Macro expansion buffer overflow");
      return;
    }
    size = (q - outp) + l + 1;
    cur_lbuf->outp = q + 1;
    cur_lbuf->last_nl = last_nl;

    new_lbuf =
        reinterpret_cast<linked_buf_t *>(DMALLOC(sizeof(linked_buf_t), TAG_COMPILER, "add_input"));
    new_lbuf->term_type = TERM_ADD_INPUT;
    new_lbuf->prev = cur_lbuf;
    buf = new_lbuf->buf;
    cur_lbuf = new_lbuf;
    last_nl = (new_lbuf->buf_end = buf + DEFMAX - 2) - 1;
    new_outp = new_lbuf->outp = buf + DEFMAX - 2 - size;
    memcpy(new_outp, p, l);
    memcpy(new_outp + l, outp, (q - outp) + 1);
    outp = new_outp;
    *(last_nl + 1) = 0;
    return;
  }

  outp -= l;
  strncpy(outp, p, l);
}

const char *main_file_name() {
  return main_filename ? main_filename : current_file;
}

/* identifier hash table stuff, size must be an even power of two */
#define IDENT_HASH_SIZE 1024
#define IdentHash(s) (whashstr((s)) & (IDENT_HASH_SIZE - 1))

/* The identifier table is hashed for speed.  The hash chains are circular
 * linked lists, so that we can rotate them, since identifier lookup is
 * rather irregular (i.e. we're likely to be asked about the same one
 * quite a number of times in a row).  This isn't as fast as moving entries
 * to the front but is done this way for two reasons:
 *
 * 1. this allows us to keep permanent identifiers consecutive and clean
 *    up faster
 * 2. it would only be faster in cases where two identifiers with the same
 *    hash value are used often within close proximity in the source.
 *    This should be rare, esp since the hash table is fairly sparse.
 *
 * ident_hash_table[hash] points to our current position (last lookup)
 * ident_hash_head[hash] points to the first permanent identifier
 * ident_hash_tail[hash] points to the last one
 * ident_dirty_list is a linked list of identifiers that need to be cleaned
 * when we're done; this happens if you define a global or function with
 * the same name as an efun or sefun.
 */

#define CHECK_ELEM(x, y, z)                               \
  if (!strcmp((x)->name, (y))) {                          \
    if (((x)->token & IHE_RESWORD) || ((x)->sem_value)) { \
      z                                                   \
    } else                                                \
      return 0;                                           \
  }

ident_hash_elem_t *lookup_ident(const char *name) {
  int h = IdentHash(name);
  ident_hash_elem_t *hptr, *hptr2;

  if ((hptr = ident_hash_table[h])) {
    CHECK_ELEM(hptr, name, return hptr;);
    hptr2 = hptr->next;
    while (hptr2 != hptr) {
      CHECK_ELEM(hptr2, name, ident_hash_table[h] = hptr2; return hptr2;);
      hptr2 = hptr2->next;
    }
  }
  return nullptr;
}

ident_hash_elem_t *find_or_add_perm_ident(const char *name) {
  int h = IdentHash(name);
  ident_hash_elem_t *hptr, *hptr2;

  if ((hptr = ident_hash_table[h])) {
    if (!strcmp(hptr->name, name)) {
      return hptr;
    }
    hptr2 = hptr->next;
    while (hptr2 != hptr) {
      if (!strcmp(hptr2->name, name)) {
        return hptr2;
      }
      hptr2 = hptr2->next;
    }
    hptr = reinterpret_cast<ident_hash_elem_t *>(
        DMALLOC(sizeof(ident_hash_elem_t), TAG_PERM_IDENT, "find_or_add_perm_ident:1"));
    hptr->next = ident_hash_head[h]->next;
    ident_hash_head[h]->next = hptr;
    if (ident_hash_head[h] == ident_hash_tail[h]) {
      ident_hash_tail[h] = hptr;
    }
  } else {
    hptr = (ident_hash_table[h] = reinterpret_cast<ident_hash_elem_t *>(
                DMALLOC(sizeof(ident_hash_elem_t), TAG_PERM_IDENT, "find_or_add_perm_ident:2")));
    ident_hash_head[h] = hptr;
    ident_hash_tail[h] = hptr;
    hptr->next = hptr;
  }
  hptr->name = name;
  hptr->token = 0;
  hptr->sem_value = 0;
  hptr->dn.simul_num = -1;
  hptr->dn.local_num = -1;
  hptr->dn.global_num = -1;
  hptr->dn.efun_num = -1;
  hptr->dn.function_num = -1;
  hptr->dn.class_num = -1;
  return hptr;
}

typedef struct lname_linked_buf_s {
  struct lname_linked_buf_s *next;
  char block[4096];
} lname_linked_buf_t;

lname_linked_buf_t *lnamebuf = nullptr;

int lb_index = 4096;

static char *alloc_local_name(const char *name) {
  int len = strlen(name) + 1;
  char *res;

  if (lb_index + len > 4096) {
    lname_linked_buf_t *new_buf;
    new_buf = reinterpret_cast<lname_linked_buf_t *>(
        DMALLOC(sizeof(lname_linked_buf_t), TAG_COMPILER, "alloc_local_name"));
    new_buf->next = lnamebuf;
    lnamebuf = new_buf;
    lb_index = 0;
  }
  res = &(lnamebuf->block[lb_index]);
  strcpy(res, name);
  lb_index += len;
  return res;
}

int num_free = 0;

typedef struct ident_hash_elem_list_s {
  struct ident_hash_elem_list_s *next;
  ident_hash_elem_t items[128];
} ident_hash_elem_list_t;

ident_hash_elem_list_t *ihe_list = nullptr;

#if 0
void dump_ihe(ident_hash_elem_t *ihe, int noisy)
{
  int sv = 0;
  if (ihe->token & IHE_RESWORD) {
    if (noisy) { printf("%s ", ihe->name); }
  } else {
    if (noisy) { printf("%s[", ihe->name); }
    if (ihe->dn.function_num != -1) {
      if (noisy) { printf("f"); }
      sv++;
    }
    if (ihe->dn.simul_num != -1) {
      if (noisy) { printf("s"); }
      sv++;
    }
    if (ihe->dn.efun_num != -1) {
      if (noisy) { printf("e"); }
      sv++;
    }
    if (ihe->dn.local_num != -1) {
      if (noisy) { printf("l"); }
      sv++;
    }
    if (ihe->dn.global_num != -1) {
      if (noisy) { printf("g"); }
      sv++;
    }
    if (ihe->sem_value != sv) {
      if (noisy) {
        printf("(*%i*)", ihe->sem_value - sv);
      } else { dump_ihe(ihe, 1); }
    }
    if (noisy) { printf("] "); }
  }
}

void debug_dump_ident_hash_table(int noisy)
{
  int zeros = 0;
  int i;
  ident_hash_elem_t *ihe, *ihe2;

  if (noisy) { printf("\n\nIdentifier Hash Table:\n"); }
  for (i = 0; i < IDENT_HASH_SIZE; i++) {
    ihe = ident_hash_table[i];
    if (!ihe) {
      zeros++;
    } else {
      if (zeros && noisy) { printf("<%i zeros>\n", zeros); }
      zeros = 0;
      dump_ihe(ihe, noisy);
      ihe2 = ihe->next;
      while (ihe2 != ihe) {
        dump_ihe(ihe2, noisy);
        ihe2 = ihe2->next;
      }
      if (noisy) { printf("\n"); }
    }
  }
  if (zeros && noisy) { printf("<%i zeros>\n", zeros); }
}
#endif

void free_unused_identifiers() {
  ident_hash_elem_list_t *ihel, *next;
  lname_linked_buf_t *lnb, *lnbn;
  int i;

  /* clean up dirty idents */
  while (ident_dirty_list) {
    if (ident_dirty_list->dn.function_num != -1) {
      ident_dirty_list->dn.function_num = -1;
      ident_dirty_list->sem_value--;
    }
    if (ident_dirty_list->dn.global_num != -1) {
      ident_dirty_list->dn.global_num = -1;
      ident_dirty_list->sem_value--;
    }
    if (ident_dirty_list->dn.class_num != -1) {
      ident_dirty_list->dn.class_num = -1;
      ident_dirty_list->sem_value--;
    }
    ident_dirty_list = ident_dirty_list->next_dirty;
  }

  for (i = 0; i < IDENT_HASH_SIZE; i++) {
    if ((ident_hash_table[i] = ident_hash_head[i])) {
      ident_hash_tail[i]->next = ident_hash_head[i];
    }
  }

  ihel = ihe_list;
  while (ihel) {
    next = ihel->next;
    FREE(ihel);
    ihel = next;
  }
  ihe_list = nullptr;
  num_free = 0;

  lnb = lnamebuf;
  while (lnb) {
    lnbn = lnb->next;
    FREE(lnb);
    lnb = lnbn;
  }
  lnamebuf = nullptr;
  lb_index = 4096;
#if 0
  debug_dump_ident_hash_table(0);
#endif
}

static ident_hash_elem_t *quick_alloc_ident_entry() {
  if (num_free) {
    num_free--;
    return &(ihe_list->items[num_free]);
  } else {
    ident_hash_elem_list_t *ihel;
    ihel = reinterpret_cast<ident_hash_elem_list_t *>(
        DMALLOC(sizeof(ident_hash_elem_list_t), TAG_COMPILER, "quick_alloc_ident_entry"));
    ihel->next = ihe_list;
    ihe_list = ihel;
    num_free = 127;
    return &(ihe_list->items[127]);
  }
}

ident_hash_elem_t *find_or_add_ident(const char *name, int flags) {
  int h = IdentHash(name);
  ident_hash_elem_t *hptr, *hptr2;

  if ((hptr = ident_hash_table[h])) {
    if (!strcmp(hptr->name, name)) {
      if ((hptr->token & IHE_PERMANENT) && (flags & FOA_GLOBAL_SCOPE) &&
          (hptr->dn.function_num == -1) && (hptr->dn.global_num == -1) &&
          (hptr->dn.class_num == -1)) {
        hptr->next_dirty = ident_dirty_list;
        ident_dirty_list = hptr;
      }
      return hptr;
    }
    hptr2 = hptr->next;
    while (hptr2 != hptr) {
      if (!strcmp(hptr2->name, name)) {
        if ((hptr2->token & IHE_PERMANENT) && (flags & FOA_GLOBAL_SCOPE) &&
            (hptr2->dn.function_num == -1) && (hptr2->dn.global_num == -1) &&
            (hptr2->dn.class_num == -1)) {
          hptr2->next_dirty = ident_dirty_list;
          ident_dirty_list = hptr2;
        }
        ident_hash_table[h] = hptr2; /* rotate */
        return hptr2;
      }
      hptr2 = hptr2->next;
    }
  }

  hptr = quick_alloc_ident_entry();
  if (!(hptr2 = ident_hash_tail[h]) && !(hptr2 = ident_hash_table[h])) {
    ident_hash_table[h] = hptr->next = hptr;
  } else {
    hptr->next = hptr2->next;
    hptr2->next = hptr;
  }

  if (flags & FOA_NEEDS_MALLOC) {
    hptr->name = alloc_local_name(name);
  } else {
    hptr->name = name;
  }
  hptr->token = 0;
  hptr->sem_value = 0;
  hptr->dn.simul_num = -1;
  hptr->dn.local_num = -1;
  hptr->dn.global_num = -1;
  hptr->dn.efun_num = -1;
  hptr->dn.function_num = -1;
  hptr->dn.class_num = -1;
  return hptr;
}

static void add_keyword_t(const char *name, keyword_t *entry) {
  int h = IdentHash(name);

  if (ident_hash_table[h]) {
    entry->next = ident_hash_head[h]->next;
    ident_hash_head[h]->next = reinterpret_cast<ident_hash_elem_t *>(entry);
    if (ident_hash_head[h] == ident_hash_tail[h]) {
      ident_hash_tail[h] = reinterpret_cast<ident_hash_elem_t *>(entry);
    }
  } else {
    ident_hash_head[h] = reinterpret_cast<ident_hash_elem_t *>(entry);
    ident_hash_tail[h] = reinterpret_cast<ident_hash_elem_t *>(entry);
    ident_hash_table[h] = reinterpret_cast<ident_hash_elem_t *>(entry);
    entry->next = reinterpret_cast<ident_hash_elem_t *>(entry);
  }
  entry->token |= IHE_RESWORD;
}

void init_identifiers() {
  unsigned int i;
  ident_hash_elem_t *ihe;



  /* allocate all three tables together */
  ident_hash_table = reinterpret_cast<ident_hash_elem_t **>(DCALLOC(
      IDENT_HASH_SIZE * 3, sizeof(ident_hash_elem_t *), TAG_IDENT_TABLE, "init_identifiers"));
  ident_hash_head = &ident_hash_table[IDENT_HASH_SIZE];
  ident_hash_tail = &ident_hash_table[2 * IDENT_HASH_SIZE];

  /* clean all three tables */
  for (i = 0; i < IDENT_HASH_SIZE * 3; i++) {
    ident_hash_table[i] = nullptr;
  }
  /* add the reserved words */
  for (i = 0; i < NELEM(reswords); i++) {
    add_keyword_t(reswords[i].word, &reswords[i]);
  }
  /* add the efuns */
  for (i = 0; i < size_of_predefs; i++) {
    if (predefs[i].word[0] == '_') {
      predefs[i].word++;
      if (strcmp(predefs[i].word, "call_other") == 0) {
        arrow_efun = i;
      }
      if (strcmp(predefs[i].word, "evaluate") == 0) {
        evaluate_efun = i;
      }
      if (strcmp(predefs[i].word, "this_object") == 0) {
        this_efun = i;
      }
      if (strcmp(predefs[i].word, "to_int") == 0) {
        to_int_efun = i;
      }
      if (strcmp(predefs[i].word, "to_float") == 0) {
        to_float_efun = i;
      }
      if (strcmp(predefs[i].word, "new") == 0) {
        new_efun = i;
      }
      continue;
    }

    ihe = find_or_add_perm_ident(predefs[i].word);
    ihe->token |= IHE_EFUN;
    ihe->sem_value++;
    ihe->dn.efun_num = i;
  }
}

// Bridge called by Flex's YY_INPUT macro (Phase 3+). Deliberately supplies
// at most one byte per call (rather than bulk-filling Flex's read buffer)
// so that `outp` never runs more than a single DFA lookahead character
// ahead of what Flex has actually matched. lex.l's DEFER_TO_LEGACY_SCANNER
// relies on that bound to correctly rewind `outp` when handing control
// back to the raw-outp-reading legacy scanner (yylex_inner()); a bulk read
// would let refill_buffer() relocate the ring mid-fetch, which the simple
// rewind arithmetic cannot account for.
extern "C" int lpc_lex_yy_input(char *buf, int max_size) {
  if (max_size <= 0) return 0;
  unsigned char c = static_cast<unsigned char>(*outp);
  if (c == LEX_EOF) return 0;
  buf[0] = static_cast<char>(c);
  ++outp;
  // >= (not ==): see lpc_lex_newline() for why an exact-equality check can
  // be skipped when Flex's one-byte DFA lookahead has already moved `outp`
  // past the trigger position.
  if (outp >= last_nl + 1) refill_buffer();
  return 1;
}

// ---------------------------------------------------------------------------
// Predefines registry & include-path resolution
// (originally lexer_utils.cc, standalone-preprocessor support helpers)
// ---------------------------------------------------------------------------

namespace {

// Predefines registry
std::unordered_map<std::string, PredefMacro> predefines;

std::vector<std::string> inc_list;
std::vector<std::string> inc_path;

std::string merge(std::string_view name) {
    if (name.empty()) return "";

    std::string dest;
    if (name[0] == '/') {
        // Absolute path from mudlib root
        dest = "";
    } else {
        // Start with current_file's directory
        dest = current_file ? current_file : "";
        size_t last_slash = dest.rfind('/');
        if (last_slash != std::string::npos) {
            dest = dest.substr(0, last_slash);
        } else {
            dest = "";
        }
    }

    // Now process components of name
    size_t pos = 0;
    while (pos < name.size()) {
        // Skip any leading slashes in the remaining part of name
        while (pos < name.size() && name[pos] == '/') {
            pos++;
            dest.clear(); // An absolute component clears the destination
        }
        if (pos >= name.size()) break;

        size_t next_slash = name.find('/', pos);
        std::string_view component = (next_slash == std::string_view::npos)
            ? name.substr(pos)
            : name.substr(pos, next_slash - pos);

        if (component == "..") {
            if (!dest.empty()) {
                size_t last_slash = dest.rfind('/');
                if (last_slash != std::string::npos) {
                    dest = dest.substr(0, last_slash);
                } else {
                    dest.clear();
                }
            }
        } else if (component == "." || component.empty()) {
            // Do nothing
        } else {
            if (!dest.empty()) {
                dest += '/';
            }
            dest += component;
        }

        if (next_slash == std::string_view::npos) break;
        pos = next_slash + 1;
    }
    return dest;
}

} // namespace

// ---------------------------------------------------------------------------
// Predefines Implementation
// ---------------------------------------------------------------------------

void add_predefine(std::string_view name, int nargs, std::string_view exps) {
    PredefMacro m;
    m.is_function_like = (nargs >= 0);
    m.nargs = nargs;
    m.body = exps;
    predefines[std::string(name)] = std::move(m);
}

void add_quoted_predefine(std::string_view def, std::string_view val) {
    std::string quoted;
    quoted.reserve(val.size() + 2);
    quoted += '"';
    quoted += val;
    quoted += '"';
    add_predefine(def, -1, quoted);
}

const std::unordered_map<std::string, PredefMacro>& get_predefines() {
    return predefines;
}

void add_predefines() {
    int i;
    lpc_predef_t *tmpf;

    add_predefine("MUDOS", -1, "");
    add_predefine("FLUFFOS", -1, "");

#define _STR_HELPER(x) #x
#define _STR(x) _STR_HELPER(x)

#ifdef PACKAGE_DB
    add_predefine("__PACKAGE_DB__", -1, "");
#endif
#ifdef USE_MYSQL
    add_predefine("__USE_MYSQL__", -1, _STR(USE_MYSQL));
#endif
#ifdef USE_POSTGRES
    add_predefine("__USE_POSTGRES__", -1, _STR(USE_POSTGRES));
#endif
#ifdef USE_SQLITE3
    add_predefine("__USE_SQLITE3__", -1, _STR(USE_SQLITE3));
#endif
#ifdef DEFAULT_DB
    add_predefine("__DEFAULT_DB__", -1, _STR(DEFAULT_DB));
#endif

    add_predefine("__GET_CHAR_IS_BUFFERED__", -1, "");
    add_predefine("__DSLIB__", -1, "");
#ifdef PACKAGE_DWLIB
    add_predefine("__DWLIB__", -1, "");
#endif

    // Adding version
    add_quoted_predefine("__VERSION__", PROJECT_VERSION);

    add_predefine("__PORT__", -1, std::to_string(external_port[0].port).c_str());
    for (i = 0; i < static_cast<int>(sizeof(option_defs) / sizeof(const char *)); i += 2) {
        add_predefine(option_defs[i], -1, option_defs[i + 1]);
    }
    add_quoted_predefine("__ARCH__", ARCH);
    add_quoted_predefine("__COMPILER__", COMPILER);
    add_quoted_predefine("__CXXFLAGS__", CXXFLAGS);

    /* Backwards Compat */
    add_quoted_predefine("MUD_NAME", CONFIG_STR(__MUD_NAME__));
#ifdef F_ED
    add_predefine("HAS_ED", -1, "");
#endif
#ifdef F_PRINTF
    add_predefine("HAS_PRINTF", -1, "");
#endif
#if (defined(RUSAGE) || defined(GET_PROCESS_STATS) || defined(TIMES))
    add_predefine("HAS_RUSAGE", -1, "");
#endif
    add_predefine("HAS_DEBUG_LEVEL", -1, "");
#ifdef DEBUG
    add_predefine("__DEBUG__", -1, "");
#endif
    for (tmpf = lpc_predefs; tmpf; tmpf = tmpf->next) {
        std::string_view flag = tmpf->flag;
        size_t eq = flag.find('=');
        if (eq == std::string_view::npos) {
            add_predefine(flag, -1, "");
        } else {
            add_predefine(flag.substr(0, eq), -1, flag.substr(eq + 1));
        }
    }
    add_predefine("SIZEOFINT", -1, std::to_string(sizeof(LPC_INT)));
    add_predefine("MAX_INT", -1, std::to_string(LPC_INT_MAX));
    add_predefine("MIN_INT", -1, std::to_string(LPC_INT_MIN));

    {
        // %f of DBL_MAX produces 309+ characters; use a dynamically sized buffer.
        int needed = std::snprintf(nullptr, 0, "%" LPC_FLOAT_FMTSTR_P, LPC_FLOAT_MAX);
        std::string max_float(needed + 1, '\0');
        std::snprintf(max_float.data(), max_float.size(), "%" LPC_FLOAT_FMTSTR_P, LPC_FLOAT_MAX);
        max_float.resize(needed);
        add_predefine("MAX_FLOAT", -1, max_float);

        needed = std::snprintf(nullptr, 0, "%" LPC_FLOAT_FMTSTR_P, LPC_FLOAT_MIN);
        std::string min_float(needed + 1, '\0');
        std::snprintf(min_float.data(), min_float.size(), "%" LPC_FLOAT_FMTSTR_P, LPC_FLOAT_MIN);
        min_float.resize(needed);
        add_predefine("MIN_FLOAT", -1, min_float);
    }

    // Following compile time configs are now always true
    add_predefine("__CACHE_STATS__", -1, "");
    add_predefine("__STRING_STATS__", -1, "");
    add_predefine("__CLASS_STATS__", -1, "");
    add_predefine("__ARRAY_STATS__", -1, "");
    add_predefine("__CALLOUT_HANDLES__", -1, "");
    add_predefine("__ARGUMENTS_IN_TRACEBACK__", -1, "");
    add_predefine("__LOCALS_IN_TRACEBACK__", -1, "");
    add_predefine("__DEBUG_MACRO__", -1, "");

    // Following compile time configs has been changed into runtime configs.
    if (CONFIG_INT(__RC_SANE_EXPLODE_STRING__)) {
        add_predefine("__SANE_EXPLODE_STRING__", -1, "");
    }
    if (CONFIG_INT(__RC_REVERSIBLE_EXPLODE_STRING__)) {
        add_predefine("__REVERSIBLE_EXPLODE_STRING__", -1, "");
    }
    if (CONFIG_INT(__RC_SANE_SORTING__)) {
        add_predefine("__SANE_SORTING__", -1, "");
    }
    if (CONFIG_INT(__RC_CALL_OTHER_TYPE_CHECK__)) {
        add_predefine("__CALL_OTHER_TYPE_CHECK__", -1, "");
    }
    if (CONFIG_INT(__RC_CALL_OTHER_WARN__)) {
        add_predefine("__CALL_OTHER_WARN__", -1, "");
    }
    if (CONFIG_INT(__RC_MUDLIB_ERROR_HANDLER__)) {
        add_predefine("__MUDLIB_ERROR_HANDLER__", -1, "");
    }
    if (CONFIG_INT(__RC_NO_RESETS__)) {
        add_predefine("__NO_RESETS__", -1, "");
    }
    if (CONFIG_INT(__RC_LAZY_RESETS__)) {
        add_predefine("__LAZY_RESETS__", -1, "");
    }
    if (CONFIG_INT(__RC_RANDOMIZED_RESETS__)) {
        add_predefine("__RANDOMIZED_RESETS__", -1, "");
    }
    if (CONFIG_INT(__RC_THIS_PLAYER_IN_CALL_OUT__)) {
        add_predefine("__THIS_PLAYER_IN_CALL_OUT__", -1, "");
    }
    if (CONFIG_INT(__RC_TRACE__)) {
        add_predefine("__TRACE__", -1, "");
    }
    if (CONFIG_INT(__RC_TRACE_CODE__)) {
        add_predefine("__TRACE_CODE__", -1, "");
    }
    if (CONFIG_INT(__RC_INTERACTIVE_CATCH_TELL__)) {
        add_predefine("__INTERACTIVE_CATCH_TELL__", -1, "");
    }
    if (CONFIG_INT(__RC_RECEIVE_SNOOP__)) {
        add_predefine("__RECEIVE_SNOOP__", -1, "");
    }
    if (CONFIG_INT(__RC_SNOOP_SHADOWED__)) {
        add_predefine("__SNOOP_SHADOWED__", -1, "");
    }
    if (CONFIG_INT(__RC_REVERSE_DEFER__)) {
        add_predefine("__REVERSE_DEFER__", -1, "");
    }
    if (CONFIG_INT(__RC_HAS_CONSOLE__)) {
        add_predefine("__HAS_CONSOLE__", -1, "");
    }
    if (CONFIG_INT(__RC_NONINTERACTIVE_STDERR_WRITE__)) {
        add_predefine("__NONINTERACTIVE_STDERR_WRITE__", -1, "");
    }
    if (CONFIG_INT(__RC_TRAP_CRASHES__)) {
        add_predefine("__TRAP_CRASHES__", -1, "");
    }
    if (CONFIG_INT(__RC_OLD_TYPE_BEHAVIOR__)) {
        add_predefine("__OLD_TYPE_BEHAVIOR__", -1, "");
    }
    if (CONFIG_INT(__RC_OLD_RANGE_BEHAVIOR__)) {
        add_predefine("__OLD_RANGE_BEHAVIOR__", -1, "");
    }
    if (CONFIG_INT(__RC_WARN_OLD_RANGE_BEHAVIOR__)) {
        add_predefine("__WARN_OLD_RANGE_BEHAVIOR__", -1, "");
    }
    if (CONFIG_INT(__RC_SUPPRESS_ARGUMENT_WARNINGS__)) {
        add_predefine("__SUPPRESS_ARGUMENT_WARNINGS__", -1, "");
    }
}



void print_all_predefines() {
    std::vector<std::string> results;
    for (const auto& pair : predefines) {
        results.push_back(pair.first);
    }
    std::sort(results.begin(), results.end());
    for (const auto& name : results) {
        debug_message("#define %s %s\n", name.c_str(), predefines[name].body.c_str());
    }
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_all_defines() {
    // No-op: include paths are stored in std::vector<std::string> and managed automatically
}
#endif

void set_inc_list(const char *list) {
    inc_list.clear();
    std::istringstream ss(list);
    for (std::string path; std::getline(ss, path, ':'); ) {
        if (path.empty()) continue;
        if (path[0] == '/') path.erase(path.begin());
        if (!legal_path(path.c_str())) {
            debug_message("'include dirs' must give paths without any '..'\n");
            exit(-1);
        }
        inc_list.push_back(std::move(path));
    }
    inc_path = inc_list;
}

void init_include_path() {
    push_malloced_string(add_slash(current_file));
    svalue_t *ret = safe_apply_master_ob(APPLY_GET_INCLUDE_PATH, 1);

    if (!ret || ret == reinterpret_cast<svalue_t *>(-1)) {
        return;
    }
    if (ret->type != T_ARRAY) {
        debug_message("'master::get_include_path' must return 'string *'\n");
        return;
    }
    array_t *arr = ret->u.arr;
    if (!arr->size) {
        debug_message("got empty include path for 'master::get_include_path(%s)'\n", current_file);
        return;
    }

    std::vector<std::string> path;
    path.reserve(arr->size);
    for (int i = 0; i < arr->size; i++) {
        if (arr->item[i].type != T_STRING) {
            debug_message("'master::get_include_path(%s)' must return 'string *'\n", current_file);
            return;
        }
        const char *elem = arr->item[i].u.string;
        if (!strcmp(elem, ":DEFAULT:")) {
            path.insert(path.end(), inc_list.begin(), inc_list.end());
            continue;
        }
        const char *check = (elem[0] == '/') ? &elem[1] : elem;
        if (!legal_path(check)) {
            debug_message(
                "'master::get_include_path(%s)' returns invalid value '%s', must give paths without "
                "any '..'\n",
                current_file, elem);
            return;
        }
        path.emplace_back(elem);
    }
    inc_path = std::move(path);
}

void deinit_include_path() {
    inc_path = inc_list;
}

std::pair<int, std::string> inc_open(std::string_view name, bool check_local) {
    std::string buf;

    if (check_local) {
        buf = merge(name);
        const char *tmp = check_valid_path(buf.c_str(), master_ob, "include", 0);
        if (tmp) {
            int fd = open(tmp, O_RDONLY);
            if (fd != -1) {
#ifdef _WIN32
                _setmode(fd, _O_BINARY);
#endif
                return {fd, buf};
            }
        }
    }

    for (size_t p = name.find('.'); p != std::string_view::npos; p = name.find('.', p + 1)) {
        if (p + 1 < name.size() && name[p + 1] == '.') {
            return {-1, ""};
        }
    }
    for (const auto &path : inc_path) {
        buf.clear();
        buf.append(path);
        buf += '/';
        buf.append(name);
        const char *tmp = check_valid_path(buf.c_str(), master_ob, "include", 0);
        if (tmp) {
            int fd = open(tmp, O_RDONLY);
            if (fd != -1) {
#ifdef _WIN32
                _setmode(fd, _O_BINARY);
#endif
                return {fd, buf};
            }
        }
    }
    return {-1, ""};
}
