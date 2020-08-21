// Author: Yucong Sun <sunyucogn@gmail.com>
//
// This file implements an C-style preprocessor, supports basic C preprocessor
// command.
//
// This tool is used in the build process to pre-process several files. It is
// self-contained and runs cross-platform, so we don't have to depends on other
// tooling in the build process.
//
// This file is largely based on the original edit_source.c and preprocess.c from MudOS.

#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "ghc/filesystem.hpp"
namespace fs = ghc::filesystem;

unsigned int whashstr(const char *s) {
  int i = 0;
  unsigned long __h = 0;
  for (; *s && i++ < 100; ++s) {
    __h = 37 * __h + *s;
  }
  return __h;
}

/* The ANSI versions must take an unsigned char, and must work on EOF.  These
 * versions take a (possibly signed) char, and do not work correctly on EOF.
 *
 * Note that calling isdigit(x) where x is a signed char with x < 0 (i.e.
 * high bit set) invokes undefined behavior.
 */
#include <ctype.h>
#define uisdigit(x) isdigit((unsigned char)x)
#define uislower(x) islower((unsigned char)x)
#define uisspace(x) isspace((unsigned char)x)
#define uisalnum(x) isalnum((unsigned char)x)
#define uisupper(x) isupper((unsigned char)x)
#define uisalpha(x) isalpha((unsigned char)x)
#define uisxdigit(x) isxdigit((unsigned char)x)
#define uisascii(x) isascii((unsigned char)x)
#define uisprint(x) isprint((unsigned char)x)

/* Compare two number */
#define COMPARE_NUMS(x, y) (((x) > (y) ? 1 : ((x) < (y) ? -1 : 0)))

typedef struct {
  short local_num, global_num, efun_num;
  short function_num, simul_num, class_num;
} defined_name_t;

typedef struct ifstate_s {
  struct ifstate_s *next;
  int state;
} ifstate_t;

typedef struct defn_s {
  struct defn_s *next;
  char *name;
  char *exps;
  int flags;
  int nargs;
} defn_t;

#define DEFMAX 20000  // at least 4 times MAXLINE
#define MAXLINE 4096
#define MLEN 4096
#define NSIZE 256
#define MAX_INSTRS 512
#define EXPANDMAX 25000
#define NARGS 25
#define MARKS '@'

#define EXPECT_ELSE 1
#define EXPECT_ENDIF 2

char *outp;
static int buffered = 0;
static int nexpands = 0;

FILE *yyin = nullptr, *yyout = nullptr;

// Used to determine valid ID chars (variable name function name etc)
#define isalunum(c) (uisalnum(c) || (c) == '_')

#define PRAGMA_NOTE_CASE_START 1

char ppchar = '#';

// Global directory to search for includes
std::vector<fs::path> base_dirs;
char *current_file = nullptr;
int current_line;

int grammar_mode = 0; /* which section we're in for .y files */
int in_c_case, cquote, pragmas, block_nest;

char yytext[MAXLINE];
static char defbuf[DEFMAX];

typedef struct incstate_t {
  struct incstate_t *next;
  FILE *yyin;
  int line;
  char *file;
} incstate;

static incstate *inctop = nullptr;

#define CHAR_QUOTE 1
#define STRING_QUOTE 2

static void add_define(const char *, int, const char *);

defn_t *lookup_define(const char *);

void yyerror(const char *str) {
  fprintf(stderr, "%s:%d: %s\n", current_file, current_line, str);
  exit(1);
}

void yywarn(char *str) {
  /* ignore errors :)  local_options generates redefinition warnings,
     which we don't want to see */
}

void yyerrorp(const char *str) {
  char buff[200];
  sprintf(buff, str, ppchar);
  fprintf(stderr, "%s:%d: %s\n", current_file, current_line, buff);
  exit(1);
}

static void add_input(const char *p) {
  size_t l = strlen(p);

  if (outp - l < defbuf) yyerror("Macro expansion buffer overflow.\n");
  strncpy(outp - l, p, l);
  outp -= l;
}

#define SKIPW(foo) \
  while (isspace(*foo)) foo++;

static char *skip_comment(char *tmp, int flag) {
  int c;

  for (;;) {
    while ((c = *++tmp) != '*') {
      if (c == EOF) yyerror("End of file in a comment");
      if (c == '\n') {
        nexpands = 0;
        current_line++;
        if (!fgets(yytext, MAXLINE - 1, yyin)) yyerror("End of file in a comment");
        if (flag && yyout) fputs(yytext, yyout);
        tmp = yytext;
      }
    }
    do {
      if ((c = *tmp++) == '/') return tmp + 1;
      if (c == '\n') {
        nexpands = 0;
        current_line++;
        if (!fgets(yytext, MAXLINE - 1, yyin)) yyerror("End of file in a comment");
        if (flag && yyout) fputs(yytext, yyout);
        tmp = yytext;
      }
    } while (c == '*');
  }
}

static void refill() {
  char *p, *yyp;
  char c;

  if (fgets(p = yyp = defbuf + (DEFMAX >> 1), MAXLINE - 1, yyin)) {
    while (((c = *yyp++) != '\n') && (c != EOF)) {
      if (c == '/') {
        if ((c = *yyp) == '*') {
          yyp = skip_comment(yyp, 0);
          continue;
        } else if (c == '/')
          break;
      }
      *p++ = c;
    }
  } else
    yyerror("End of macro definition in \\");
  nexpands = 0;
  current_line++;
  *p = 0;
}

static void handle_define() {
  char namebuf[NSIZE];
  char args[NARGS][NSIZE];
  char mtext[MLEN];
  char *end;
  char *tmp = outp, *q;

  q = namebuf;
  end = q + NSIZE - 1;
  while (isalunum(*tmp)) {
    if (q < end)
      *q++ = *tmp++;
    else
      yyerror("Name too long.\n");
  }
  if (q == namebuf) yyerror("Macro name missing.\n");
  if (*namebuf != '_' && !isalpha(*namebuf)) yyerror("Invalid macro name.\n");
  *q = 0;
  if (*tmp == '(') { /* if "function macro" */
    int arg;
    int inid;
    char *ids = (char *)nullptr;

    tmp++; /* skip '(' */
    SKIPW(tmp);
    if (*tmp == ')') {
      arg = 0;
    } else {
      for (arg = 0; arg < NARGS;) {
        end = (q = args[arg]) + NSIZE - 1;
        while (isalunum(*tmp) || (*tmp == '#')) {
          if (q < end)
            *q++ = *tmp++;
          else
            yyerror("Name too long.\n");
        }
        if (q == args[arg]) {
          char buff[200];
          sprintf(buff, "Missing argument %d in #define parameter list", arg + 1);
          yyerror(buff);
        }
        arg++;
        SKIPW(tmp);
        if (*tmp == ')') break;
        if (*tmp++ != ',') {
          yyerror("Missing ',' in #define parameter list");
        }
        SKIPW(tmp);
      }
      if (arg == NARGS) yyerror("Too many macro arguments");
    }
    tmp++; /* skip ')' */
    end = mtext + MLEN - 2;
    for (inid = 0, q = mtext; *tmp;) {
      if (isalunum(*tmp)) {
        if (!inid) {
          inid++;
          ids = tmp;
        }
      } else {
        if (inid) {
          int idlen = tmp - ids;
          int n, l;

          for (n = 0; n < arg; n++) {
            l = strlen(args[n]);
            if (l == idlen && strncmp(args[n], ids, l) == 0) {
              q -= idlen;
              *q++ = MARKS;
              *q++ = n + MARKS + 1;
              break;
            }
          }
          inid = 0;
        }
      }
      if ((*q = *tmp++) == MARKS) *++q = MARKS;
      if (q < end)
        q++;
      else
        yyerror("Macro text too long");
      if (!*tmp && tmp[-2] == '\\') {
        q -= 2;
        refill();
        tmp = defbuf + (DEFMAX >> 1);
      }
    }
    *--q = 0;
    add_define(namebuf, arg, mtext);
  } else if (isspace(*tmp) || (!*tmp && (*(tmp + 1) = '\0', *tmp = ' '))) {
    end = mtext + MLEN - 2;
    for (q = mtext; *tmp;) {
      *q = *tmp++;
      if (q < end)
        q++;
      else
        yyerror("Macro text too long");
      if (!*tmp && tmp[-2] == '\\') {
        q -= 2;
        refill();
        tmp = defbuf + (DEFMAX >> 1);
      }
    }
    *q = 0;
    add_define(namebuf, -1, mtext);
  } else {
    yyerror("Illegal macro symbol");
  }
  return;
}

#define SKPW \
  while (isspace(*outp)) outp++

static char cmygetc() {
  char c;

  for (;;) {
    if ((c = *outp++) == '/') {
      if ((c = *outp) == '*')
        outp = skip_comment(outp, 0);
      else if (c == '/')
        return -1;
      else
        return c;
    } else
      return c;
  }
}

/* Check if yytext is a macro and expand if it is. */
static int expand_define() {
  defn_t *p;
  char expbuf[DEFMAX];
  char *args[NARGS];
  char buf[DEFMAX];
  char *q, *e, *b;

  if (nexpands++ > EXPANDMAX) yyerror("Too many macro expansions");
  if (!(p = lookup_define(yytext))) return 0;
  if (p->nargs == -1) {
    add_input(p->exps);
  } else {
    char c;
    int parcnt = 0, dquote = 0, squote = 0;
    int n;

    SKPW;
    if (*outp++ != '(') yyerror("Missing '(' in macro call");
    SKPW;
    if ((c = *outp++) == ')')
      n = 0;
    else {
      q = expbuf;
      args[0] = q;
      for (n = 0; n < NARGS;) {
        switch (c) {
          case '"':
            if (!squote) dquote ^= 1;
            break;
          case '\'':
            if (!dquote) squote ^= 1;
            break;
          case '(':
            if (!squote && !dquote) parcnt++;
            break;
          case ')':
            if (!squote && !dquote) parcnt--;
            break;
          case '#':
            if (!squote && !dquote) {
              *q++ = c;
              if (*outp++ != '#') yyerror("'#' expected");
            }
            break;
          case '\\':
            if (squote || dquote) {
              *q++ = c;
              c = *outp++;
            }
            break;
          case '\n':
            if (squote || dquote) yyerror("Newline in string");
            break;
        }
        if (c == ',' && !parcnt && !dquote && !squote) {
          *q++ = 0;
          args[++n] = q;
        } else if (parcnt < 0) {
          *q++ = 0;
          n++;
          break;
        } else {
          if (c == EOF) yyerror("Unexpected end of file");
          if (q >= expbuf + DEFMAX - 5) {
            yyerror("Macro argument overflow");
          } else {
            *q++ = c;
          }
        }
        if (!squote && !dquote) {
          if ((c = cmygetc()) < 0) yyerror("End of macro in // comment");
        } else
          c = *outp++;
      }
      if (n == NARGS) {
        yyerror("Maximum macro argument count exceeded");
        return 0;
      }
    }
    if (n != p->nargs) {
      yyerror("Wrong number of macro arguments");
      return 0;
    }
    /* Do expansion */
    b = buf;
    e = p->exps;
    while (*e) {
      if (*e == '#' && *(e + 1) == '#') e += 2;
      if (*e == MARKS) {
        if (*++e == MARKS)
          *b++ = *e++;
        else {
          for (q = args[*e++ - MARKS - 1]; *q;) {
            *b++ = *q++;
            if (b >= buf + DEFMAX) yyerror("Macro expansion overflow");
          }
        }
      } else {
        *b++ = *e++;
        if (b >= buf + DEFMAX) yyerror("Macro expansion overflow");
      }
    }
    *b++ = 0;
    add_input(buf);
  }
  return 1;
}

static char exgetc() {
  char c, *yyp;

  SKPW;
  while (isalpha(c = *outp) || c == '_') {
    yyp = yytext;
    do {
      *yyp++ = c;
    } while (isalnum(c = *++outp) || (c == '_'));
    *yyp = '\0';
    if (!strcmp(yytext, "defined")) {
      /* handle the defined "function" in #/%if */
      SKPW;
      if (*outp++ != '(') yyerror("Missing ( after 'defined'");
      SKPW;
      yyp = yytext;
      if (isalpha(c = *outp) || c == '_') {
        do {
          *yyp++ = c;
        } while (isalnum(c = *++outp) || (c == '_'));
        *yyp = '\0';
      } else
        yyerror("Incorrect definition macro after defined(\n");
      SKPW;
      if (*outp != ')') yyerror("Missing ) in defined");
      if (lookup_define(yytext))
        add_input("1 ");
      else
        add_input("0 ");
    } else {
      if (!expand_define())
        add_input("0 ");
      else
        SKPW;
    }
  }
  return c;
}

static int skip_to(const char *token, const char *atoken) {
  char b[20], *p, *end;
  char c;
  int nest;

  for (nest = 0;;) {
    if (!fgets(outp = defbuf + (DEFMAX >> 1), MAXLINE - 1, yyin)) {
      yyerror("Unexpected end of file while skipping");
    }
    current_line++;
    if ((c = *outp++) == ppchar) {
      while (isspace(*outp)) outp++;
      end = b + sizeof b - 1;
      for (p = b; (c = *outp++) != '\n' && !isspace(c) && c != EOF;) {
        if (p < end) *p++ = c;
      }
      *p = 0;
      if (!strcmp(b, "if") || !strcmp(b, "ifdef") || !strcmp(b, "ifndef")) {
        nest++;
      } else if (nest > 0) {
        if (!strcmp(b, "endif")) nest--;
      } else {
        if (!strcmp(b, token)) {
          *--outp = c;
          add_input(b);
          *--outp = ppchar;
          buffered = 1;
          return 1;
        } else if (atoken && !strcmp(b, atoken)) {
          *--outp = c;
          add_input(b);
          *--outp = ppchar;
          buffered = 1;
          return 0;
        } else if (!strcmp(b, "elif")) {
          *--outp = c;
          add_input(b);
          *--outp = ppchar;
          buffered = 1;
          return !atoken;
        }
      }
    }
  }
}

static int cond_get_exp(int);

static void handle_cond(int);

#define DXALLOC(x, y, z) malloc(x)
#define FREE(x) free(x)
#define ALLOCATE(x, y, z) (x *)malloc(sizeof(x))
#define DREALLOC(w, x, y, z) realloc(w, x)

/* must be a power of 4 */
#define DEFHASH 128
#define defhash(s) (whashstr((s)) & (DEFHASH - 1))

#define DEF_IS_UNDEFINED 1
#define DEF_IS_PREDEF 2
/* used only in edit_source */
#define DEF_IS_NOT_LOCAL 4

static defn_t *defns[DEFHASH];
static ifstate_t *iftop = nullptr;

static defn_t *lookup_definition(const char *s) {
  defn_t *p;
  int h;

  h = defhash(s);
  for (p = defns[h]; p; p = p->next)
    if (strcmp(s, p->name) == 0) return p;
  return nullptr;
}

defn_t *lookup_define(const char *s) {
  defn_t *p = lookup_definition(s);

  if (p && (p->flags & DEF_IS_UNDEFINED))
    return nullptr;
  else
    return p;
}

static void add_define(const char *name, int nargs, const char *exps) {
  defn_t *p = lookup_definition(name);
  int h, len;

  /* trim off leading and trailing whitespace */
  while (uisspace(*exps)) exps++;
  for (len = strlen(exps); len && uisspace(exps[len - 1]); len--)
    ;
  if (*exps == '#' && *(exps + 1) == '#') {
    yyerror("'##' at start of macro definition");
    return;
  }
  if (len > 2 && *(exps + len - 2) == '#' && *(exps + len - 1) == '#') {
    yyerror("'##' at end of macro definition");
    return;
  }

  if (p) {
    if (p->flags & DEF_IS_UNDEFINED) {
      p->exps = (char *)DREALLOC(p->exps, len + 1, TAG_COMPILER, "add_define: redef");
      memcpy(p->exps, exps, len);
      p->exps[len] = 0;
      p->flags = 0;
      p->nargs = nargs;
    } else {
      if (p->flags & DEF_IS_PREDEF) {
        yyerror("Illegal to redefine predefined value.");
        return;
      }
      if (nargs != p->nargs || strcmp(exps, p->exps)) {
        char buf[200 + NSIZE];

        sprintf(buf, "redefinition of #define %s\n", name);
        yywarn(buf);

        p->exps = (char *)DREALLOC(p->exps, len + 1, TAG_COMPILER, "add_define: redef");
        memcpy(p->exps, exps, len);
        p->exps[len] = 0;
        p->nargs = nargs;
      }
      p->flags &= ~DEF_IS_NOT_LOCAL;
    }
  } else {
    p = ALLOCATE(defn_t, TAG_COMPILER, "add_define: def");
    p->name = (char *)DXALLOC(strlen(name) + 1, TAG_COMPILER, "add_define: def name");
    strcpy(p->name, name);
    p->exps = (char *)DXALLOC(len + 1, TAG_COMPILER, "add_define: def exps");
    memcpy(p->exps, exps, len);
    p->exps[len] = 0;
    p->flags = 0;
    p->nargs = nargs;
    h = defhash(name);
    p->next = defns[h];
    defns[h] = p;
  }
}

static void handle_elif()
{
  if (iftop) {
    if (iftop->state == EXPECT_ELSE) {
      /* last cond was false... */
      int cond;
      ifstate_t *p = iftop;

      /* pop previous condition */
      iftop = p->next;
      FREE((char *)p);

      cond = cond_get_exp(0);
      if (*outp != '\n') {
        yyerror("Condition too complex in #elif");
      } else
        handle_cond(cond);
    } else { /* EXPECT_ENDIF */
      /*
       * last cond was true...skip to end of
       * conditional
       */
      skip_to("endif", (char *)nullptr);
    }
  } else {
    yyerrorp("Unexpected %celif");
  }
}

static void handle_else(void) {
  if (iftop) {
    if (iftop->state == EXPECT_ELSE) {
      iftop->state = EXPECT_ENDIF;
    } else {
      skip_to("endif", (char *)nullptr);
    }
  } else {
    yyerrorp("Unexpected %cendif");
  }
}

static void handle_endif(void) {
  if (iftop && (iftop->state == EXPECT_ENDIF || iftop->state == EXPECT_ELSE)) {
    ifstate_t *p = iftop;

    iftop = p->next;
    FREE((char *)p);
  } else {
    yyerrorp("Unexpected %cendif");
  }
}

#define BNOT 1
#define LNOT 2
#define UMINUS 3
#define UPLUS 4

#define MULT 1
#define DIV 2
#define MOD 3
#define BPLUS 4
#define BMINUS 5
#define LSHIFT 6
#define RSHIFT 7
#define LESS 8
#define LEQ 9
#define GREAT 10
#define GEQ 11
#define EQ 12
#define NEQ 13
#define BAND 14
#define XOR 15
#define BOR 16
#define LAND 17
#define LOR 18
#define QMARK 19

static char optab[] = {0, 4, 0, 0, 0, 26, 56, 0, 0, 0,  18, 14, 0,  10, 0, 22, 0,  0, 0,
                        0, 0, 0, 0, 0, 0,  0,  0, 0, 30, 50, 40, 74, 0,  0, 0,  0,  0, 0,
                        0, 0, 0, 0, 0, 0,  0,  0, 0, 0,  0,  0,  0,  0,  0, 0,  0,  0, 0,
                        0, 0, 0, 0, 0, 70, 0,  0, 0, 0,  0,  0,  0,  0,  0, 0,  0,  0, 0,
                        0, 0, 0, 0, 0, 0,  0,  0, 0, 0,  0,  0,  0,  0,  0, 0,  63, 0, 1};
static char optab2[] = {
    BNOT, 0,   0,   LNOT, '=', NEQ,  7, 0,   0,   UMINUS, 0, BMINUS, 10,   UPLUS, 0,   BPLUS,
    10,   0,   0,   MULT, 11,  0,    0, DIV, 11,  0,      0, MOD,    11,   0,     '<', LSHIFT,
    9,    '=', LEQ, 8,    0,   LESS, 8, 0,   '>', RSHIFT, 9, '=',    GEQ,  8,     0,   GREAT,
    8,    0,   '=', EQ,   7,   0,    0, 0,   '&', LAND,   3, 0,      BAND, 6,     0,   '|',
    LOR,  2,   0,   BOR,  4,   0,    0, XOR, 5,   0,      0, QMARK,  1};

static int cond_get_exp(int priority) {
  char c;
  int value, value2, x;

  if ((c = exgetc()) == '(') {
    value = cond_get_exp(0);
    if ((c = exgetc()) != ')') yyerrorp("bracket not paired in %cif");
  } else if (ispunct(c)) {
    if (!(x = optab[c - ' '])) {
      yyerrorp("illegal character in %cif");
      return 0;
    }
    value = cond_get_exp(12);
    switch (optab2[x - 1]) {
      case BNOT:
        value = ~value;
        break;
      case LNOT:
        value = !value;
        break;
      case UMINUS:
        value = -value;
        break;
      case UPLUS:
        value = value;
        break;
      default:
        yyerrorp("illegal unary operator in %cif");
    }
  } else {
    int base;

    if (!isdigit(c)) {
      if (c == '\n') {
        yyerrorp("missing expression in %cif");
      } else
        yyerrorp("illegal character in %cif");
      return 0;
    }
    value = 0;
    if (c != '0') {
      base = 10;
    } else {
      c = *outp++;
      if (c == 'x' || c == 'X') {
        base = 16;
        c = *outp++;
      } else
        base = 8;
    }
    outp++;
    for (;;) {
      if (isdigit(c))
        x = -'0';
      else if (isupper(c))
        x = -'A' + 10;
      else if (islower(c))
        x = -'a' + 10;
      else
        break;
      x += c;
      if (x > base) break;
      value = value * base + x;
      c = *outp++;
    }
    outp--;
  }
  for (;;) {
    if (!ispunct(c = exgetc())) break;
    if (!(x = optab[c - ' '])) break;
    outp++;
    value2 = exgetc();
    for (;; x += 3) {
      if (!optab2[x]) {
        outp--;
        if (!optab2[x + 1]) {
          yyerrorp("illegal operator use in %cif");
          return 0;
        }
        break;
      }
      if (value2 == optab2[x]) break;
    }
    if (priority >= optab2[x + 2]) {
      if (optab2[x]) *--outp = value2;
      break;
    }
    value2 = cond_get_exp(optab2[x + 2]);
    switch (optab2[x + 1]) {
      case MULT:
        value *= value2;
        break;
      case DIV:
        if (value2)
          value /= value2;
        else
          yyerrorp("division by 0 in %cif");
        break;
      case MOD:
        if (value2)
          value %= value2;
        else
          yyerrorp("modulo by 0 in %cif");
        break;
      case BPLUS:
        value += value2;
        break;
      case BMINUS:
        value -= value2;
        break;
      case LSHIFT:
        value <<= value2;
        break;
      case RSHIFT:
        value >>= value2;
        break;
      case LESS:
        value = value < value2;
        break;
      case LEQ:
        value = value <= value2;
        break;
      case GREAT:
        value = value > value2;
        break;
      case GEQ:
        value = value >= value2;
        break;
      case EQ:
        value = value == value2;
        break;
      case NEQ:
        value = value != value2;
        break;
      case BAND:
        value &= value2;
        break;
      case XOR:
        value ^= value2;
        break;
      case BOR:
        value |= value2;
        break;
      case LAND:
        value = value && value2;
        break;
      case LOR:
        value = value || value2;
        break;
      case QMARK:
        if ((c = exgetc()) != ':') yyerrorp("'?' without ':' in %cif");
        if (value) {
          cond_get_exp(1);
          value = value2;
        } else
          value = cond_get_exp(1);
        break;
    }
  }
  //outp--;
  return value;
}

static void handle_cond(int c) {
  ifstate_t *p;

  if (!c) skip_to("else", "endif");
  p = ALLOCATE(ifstate_t, TAG_COMPILER, "handle_cond");
  p->next = iftop;
  iftop = p;
  p->state = c ? EXPECT_ENDIF : EXPECT_ELSE;
}

static int maybe_open_input_file(char *fn) {
  if ((yyin = fopen(fn, "r")) == nullptr) {
    return 0;
  }
  if (current_file) free((char *)current_file);
  current_file = (char *)malloc(strlen(fn) + 1);
  current_line = 0;
  strcpy(current_file, fn);
  return 1;
}

static void open_input_file(char *fn) {
  if (!maybe_open_input_file(fn)) {
    perror(fn);
    exit(-1);
  }
}

static void open_output_file(char *fn) {
  if ((yyout = fopen(fn, "w")) == nullptr) {
    perror(fn);
    exit(-1);
  }
}

static void close_output_file() {
  fclose(yyout);
  yyout = nullptr;
}

static char *protect(char *p) {
  static char buf[1024];
  char *bufp = buf;

  while (*p) {
    if (*p == '"' || *p == '\\') *bufp++ = '\\';
    *bufp++ = *p++;
  }
  *bufp = 0;
  return buf;
}

static void deltrail() {
  char *p;

  p = outp;
  while (*p && !isspace(*p) && *p != '\n') {
    p++;
  }
  *p = 0;
}

static void handle_include(char *name) {
  char *p;
  static char buf[1024];
  FILE *f;
  incstate *is;

  if (*name != '"') {
    defn_t *d;

    if ((d = lookup_define(name)) && d->nargs == -1) {
      char *q;

      q = d->exps;
      while (isspace(*q)) q++;
      handle_include(q);
    } else {
      yyerrorp("Missing leading \" in %cinclude");
    }
    return;
  }
  for (p = ++name; *p && *p != '"'; p++)
    ;
  if (!*p) yyerrorp("Missing trailing \" in %cinclude");

  *p = 0;

  bool found = false;
  fs::path include_file;
  for(const auto& base_dir: base_dirs) {
    include_file = base_dir / name;
    if (fs::exists(include_file)) {
      found = true;
      break;
    }
  }
  if (!found) {
    sprintf(buf, "Cannot %cinclude %s", ppchar, name);
    yyerror(buf);
    return;
  }

  fprintf(stderr, "Opening file: %s\n", include_file.c_str());
  if ((f = fopen(include_file.c_str(), "r")) != nullptr) {
    is = (incstate *)malloc(sizeof(incstate) /*, 61, "handle_include: 1" */);
    is->yyin = yyin;
    is->line = current_line;
    is->file = current_file;
    is->next = inctop;
    inctop = is;
    current_line = 0;
    current_file = (char *)malloc(strlen(name) + 1 /*, 62, "handle_include: 2" */);
    strcpy(current_file, name);
    yyin = f;
  } else {
    sprintf(buf, "Cannot %cinclude %s", ppchar, name);
    yyerror(buf);
  }
}

static void handle_pragma(char *name) {
  if (!strcmp(name, "auto_note_compiler_case_start"))
    pragmas |= PRAGMA_NOTE_CASE_START;
  else if (!strcmp(name, "no_auto_note_compiler_case_start"))
    pragmas &= ~PRAGMA_NOTE_CASE_START;
  else if (!strncmp(name, "ppchar:", 7) && *(name + 8))
    ppchar = *(name + 8);
  else
    yyerrorp("Unidentified %cpragma");
}

static void preprocess() {
  char *yyp, *yyp2;
  int c;
  int cond;

  while (buffered ? (yyp = yyp2 = outp)
                  : fgets(yyp = yyp2 = defbuf + (DEFMAX >> 1), MAXLINE - 1, yyin)) {
    if (!buffered)
      current_line++;
    else
      buffered = 0;
    while (isspace(*yyp2)) yyp2++;
    if ((c = *yyp2) == ppchar) {
      int quote = 0;
      char sp_buf = 0, *oldoutp;

      if (c == '%' && yyp2[1] == '%') grammar_mode++;
      outp = nullptr;
      if (yyp != yyp2) yyerrorp("Misplaced '%c'.\n");
      while (isspace(*++yyp2))
        ;
      yyp++;
      for (;;) {
        if ((c = *yyp2++) == '"')
          quote ^= 1;
        else {
          if (!quote && c == '/') {
            if (*yyp2 == '*') {
              yyp2 = skip_comment(yyp2, 0);
              continue;
            } else if (*yyp2 == '/')
              break;
          }
          if (!outp && isspace(c)) outp = yyp;
          if (c == '\n' || c == EOF) break;
        }
        *yyp++ = c;
      }

      if (outp) {
        if (yyout) sp_buf = *(oldoutp = outp);
        *outp++ = 0;
        while (isspace(*outp)) outp++;
      } else
        outp = yyp;
      *yyp = 0;
      yyp = defbuf + (DEFMAX >> 1) + 1;

      if (!strcmp("define", yyp)) {
        handle_define();
      } else if (!strcmp("if", yyp)) {
        cond = cond_get_exp(0);
        if (*outp)
          yyerrorp("Condition too complex in %cif");
        else
          handle_cond(cond);
      } else if (!strcmp("ifdef", yyp)) {
        deltrail();
        handle_cond(lookup_define(outp) != nullptr);
      } else if (!strcmp("ifndef", yyp)) {
        deltrail();
        handle_cond(!lookup_define(outp));
      } else if (!strcmp("elif", yyp)) {
        handle_elif();
      } else if (!strcmp("else", yyp)) {
        handle_else();
      } else if (!strcmp("endif", yyp)) {
        handle_endif();
      } else if (!strcmp("undef", yyp)) {
        defn_t *d;

        deltrail();
        if ((d = lookup_definition(outp))) {
          d->flags |= DEF_IS_UNDEFINED;
          d->flags &= ~DEF_IS_NOT_LOCAL;
        } else {
          add_define(outp, -1, " ");
          d = lookup_definition(outp);
          d->flags |= DEF_IS_UNDEFINED;
          d->flags &= ~DEF_IS_NOT_LOCAL;
        }
      } else if (!strcmp("echo", yyp)) {
        fprintf(stderr, "echo at line %d of %s: %s\n", current_line, current_file, outp);
      } else if (!strcmp("include", yyp)) {
        handle_include(outp);
      } else if (!strcmp("pragma", yyp)) {
        handle_pragma(outp);
      } else if (yyout) {
        if (!strcmp("line", yyp)) {
          fprintf(yyout, "#line %d \"%s\"\n", current_line, current_file);
        } else {
          if (sp_buf) *oldoutp = sp_buf;
          if (pragmas & PRAGMA_NOTE_CASE_START) {
            if (*yyp == '%') pragmas &= ~PRAGMA_NOTE_CASE_START;
          }
          fprintf(yyout, "%s\n", yyp - 1);
        }
      } else {
        char buff[200];
        sprintf(buff, "Unrecognised %c directive : %s\n", ppchar, yyp);
        yyerror(buff);
      }
    } else if (c == '/') {
      if ((c = *++yyp2) == '*') {
        if (yyout) fputs(yyp, yyout);
        yyp2 = skip_comment(yyp2, 1);
      } else if (c == '/' && !yyout)
        continue;
      else if (yyout) {
        fprintf(yyout, "%s", yyp);
      }
    } else if (yyout) {
      fprintf(yyout, "%s", yyp);
      if (pragmas & PRAGMA_NOTE_CASE_START) {
        static int line_to_print;

        line_to_print = 0;

        if (!in_c_case) {
          while (isalunum(*yyp2)) yyp2++;
          while (isspace(*yyp2)) yyp2++;
          if (*yyp2 == ':') {
            in_c_case = 1;
            yyp2++;
          }
        }

        if (in_c_case) {
          while ((c = *yyp2++)) {
            switch (c) {
              case '{': {
                if (!cquote && (++block_nest == 1)) line_to_print = 1;
                break;
              }

              case '}': {
                if (!cquote) {
                  if (--block_nest < 0) yyerror("Too many }'s");
                }
                break;
              }

              case '"':
                if (!(cquote & CHAR_QUOTE)) cquote ^= STRING_QUOTE;
                break;

              case '\'':
                if (!(cquote & STRING_QUOTE)) cquote ^= CHAR_QUOTE;
                break;

              case '\\':
                if (cquote && *yyp2) yyp2++;
                break;

              case '/':
                if (!cquote) {
                  if ((c = *yyp2) == '*') {
                    yyp2 = skip_comment(yyp2, 1);
                  } else if (c == '/') {
                    *(yyp2 - 1) = '\n';
                    *yyp2 = '\0';
                  }
                }
                break;

              case ':':
                if (!cquote && !block_nest)
                  yyerror("Case started before ending previous case with ;");
                break;

              case ';':
                if (!cquote && !block_nest) in_c_case = 0;
            }
          }
        }

        if (line_to_print) fprintf(yyout, "#line %d \"%s\"\n", current_line + 1, current_file);
      }
    }
  }
  if (iftop) {
    ifstate_t *p = iftop;

    while (iftop) {
      p = iftop;
      iftop = p->next;
      free((char *)p);
    }
    yyerrorp("Missing %cendif");
  }
  fclose(yyin);
  free(current_file);
  current_file = nullptr;
  nexpands = 0;
  if (inctop) {
    incstate *p = inctop;

    current_file = p->file;
    current_line = p->line;
    yyin = p->yyin;
    inctop = p->next;
    free((char *)p);
    preprocess();
  } else
    yyin = nullptr;
}

void do_preprocess(int argc, char **argv) {
  if (argc < 3) {
     fprintf(stderr, "Usage: preprocessor -I<base_dir> -I<base_dir> <global.h>\n");
     exit(-1);
  }

  auto i = 1;
  for(; i < argc; i++) {
    char* arg = argv[i];
    if (arg[0] == '-') {
      switch(arg[1]) {
        case 'i':
        case 'I':
        {
          auto base_dir = fs::path(&arg[2]);
          base_dirs.push_back(base_dir);
          fprintf(stderr, "Adding search directory: %s\n", base_dir.c_str());
        }
          break;
        default:
          fprintf(stderr, "Unknown flag: %s.\n", arg);
          exit(-1);
          break;
      }
    } else {
      break;
    }
  }

  if (argc <= i) {
    fprintf(stderr, "Not enough arguments!\n");
    exit(-1);
  }

  char *global = argv[i];
  open_input_file(global);
  preprocess();
}
