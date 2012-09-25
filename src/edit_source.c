#define CONFIGURE_VERSION       5

#define EDIT_SOURCE
#define NO_MALLOC
#define NO_SOCKETS
#define NO_OPCODES
#include "std.h"
#include "lex.h"
#include "preprocess.h"
#include "make_func.h"
#include "cc.h"
#include "hash.h"
#include <stdlib.h>
#include <unistd.h>

#ifdef WIN32
#include <process.h>
#include <malloc.h>
#endif

#if defined(DEBUG) || defined(WIN32)
#define TO_DEV_NULL ""
#else
#define TO_DEV_NULL ">/dev/null 2>&1"
#endif

/* Using an include file at this point would be bad */
#ifdef PEDANTIC
char *malloc(int);
char *realloc(char *, int);
void free(char *);
#endif

char *outp;
static int buffered = 0;
static int nexpands = 0;

FILE *yyin = 0, *yyout = 0;

#define SYNTAX "edit_source [-process file] [-options] [-malloc] [-build_func_spec 'command'] [-build_efuns] [-configure]\n"

/* The files we fool with.  (Actually, there are more.  See -process).
 *
 * TODO: teach this file how to fix bugs in the source code :)
 */
#define OPTIONS_INCL      "options_incl.h"
#define PACKAGES          "packages/packages"
#define OPTIONS_H         "options.h"
#define LOCAL_OPTIONS     "local_options"
#define OPTION_DEFINES    "option_defs.c"
#define FUNC_SPEC         "func_spec.c"
#define FUNC_SPEC_CPP     "func_spec.cpp"
#define EFUN_TABLE        "efunctions.h"
#define OPC_PROF          "opc.h"
#define OPCODES           "opcodes.h"
#define EFUN_PROTO        "efun_protos.h"
#define EFUN_DEFS         "efun_defs.c"

#define PRAGMA_NOTE_CASE_START 1

int num_packages = 0;
char *packages[100];
char ppchar;

char *current_file = 0;
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

static incstate *inctop = 0;

#define CHAR_QUOTE 1
#define STRING_QUOTE 2

static void add_define (const char *, int, const char *);

#ifdef WIN32
#include <io.h>

int compile(char *command) {
   FILE *tf = fopen("trash_me.bat","wt+");

   fprintf(tf,"%s%s\n%s",
        "@echo off\n",
      command,
      "if errorlevel == 1 goto error\n"
      "del trash_me.err >nul\n"
      "goto ok\n"
      ":error\n"
      "echo ERROR > trash_me.err\n"
      ":ok\n");
   fclose(tf);

   if (!system("trash_me.bat > nul")) return 1;
   if (_access("trash_me.err",0) ) return 1;
   return 0;
}
#else
int compile (char * str) {
    return system(str);
}
#endif

#if defined(WIN32)
int dos_style_link (char * x, char * y) {
    char link_cmd[100];
    sprintf(link_cmd, "copy %s %s", x, y);
    return system(link_cmd);
}
#endif

void yyerror (const char * str)
{
    fprintf(stderr, "%s:%d: %s\n", current_file, current_line, str);
    exit(1);
}

void mf_fatal (const char * str)
{
    yyerror(str);
}

void yywarn (char * str)
{
    /* ignore errors :)  local_options generates redefinition warnings,
       which we don't want to see */
}

void yyerrorp (const char * str)
{
    char buff[200];
    sprintf(buff, str, ppchar);
    fprintf(stderr, "%s:%d: %s\n", current_file, current_line, buff);
    exit(1);
}

static void add_input (const char * p)
{
    int l = strlen(p);

    if (outp - l < defbuf) yyerror("Macro expansion buffer overflow.\n");
    strncpy(outp - l, p, l);
    outp -= l;
}

#define SKIPW(foo) while (isspace(*foo)) foo++;

static char *skip_comment(char *tmp, int flag)
{
    int c;

    for (;;) {
        while ((c = *++tmp) !=  '*') {
            if (c == EOF) yyerror("End of file in a comment");
            if (c == '\n') {
                nexpands = 0;
                current_line++;
                if (!fgets(yytext, MAXLINE - 1, yyin)) yyerror("End of file in a comment");
                if (flag && yyout) fputs(yytext, yyout);
                tmp = yytext - 1;
            }
        }
        do {
            if ((c = *++tmp) == '/')
                return tmp + 1;
            if (c == '\n') {
                nexpands = 0;
                current_line++;
                if (!fgets(yytext, MAXLINE - 1, yyin)) yyerror("End of file in a comment");
                if (flag && yyout) fputs(yytext, yyout);
                tmp = yytext - 1;
            }
        } while (c == '*');
    }
}

static void refill()
{
    register char *p, *yyp;
    int c;

    if (fgets(p = yyp = defbuf + (DEFMAX >> 1), MAXLINE - 1, yyin)) {
      while (((c = *yyp++) != '\n') && (c != EOF)) {
          if (c == '/') {
              if ((c = *yyp) == '*') {
                  yyp = skip_comment(yyp, 0);
                  continue;
              }
              else if (c == '/') break;
          }
          *p++ = c;
      }
    }
    else yyerror("End of macro definition in \\");
    nexpands = 0;
    current_line++;
    *p = 0;
    return;
}

static void handle_define()
{
    char namebuf[NSIZE];
    char args[NARGS][NSIZE];
    char mtext[MLEN];
    char *end;
    register char *tmp = outp, *q;

    q = namebuf;
    end = q + NSIZE - 1;
    while (isalunum(*tmp)) {
        if (q < end) *q++ = *tmp++;
        else yyerror("Name too long.\n");
    }
    if (q == namebuf) yyerror("Macro name missing.\n");
    if (*namebuf != '_' && !isalpha(*namebuf)) yyerror("Invalid macro name.\n");
    *q = 0;
    if (*tmp == '(') {            /* if "function macro" */
        int arg;
        int inid;
        char *ids = (char *) NULL;

        tmp++;                    /* skip '(' */
        SKIPW(tmp);
        if (*tmp == ')') {
            arg = 0;
        } else {
            for (arg = 0; arg < NARGS;) {
                end = (q = args[arg]) + NSIZE - 1;
                while (isalunum(*tmp) || (*tmp == '#')) {
                    if (q < end) *q++ = *tmp++;
                    else yyerror("Name too long.\n");
                }
                if (q == args[arg]) {
                    char buff[200];
                    sprintf(buff, "Missing argument %d in #define parameter list", arg + 1);
                    yyerror(buff);
                }
                arg++;
                SKIPW(tmp);
                if (*tmp == ')')
                    break;
                if (*tmp++ != ',') {
                    yyerror("Missing ',' in #define parameter list");
                }
                SKIPW(tmp);
            }
            if (arg == NARGS) yyerror("Too many macro arguments");
        }
        tmp++;                    /* skip ')' */
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
            if (q < end) q++;
            else yyerror("Macro text too long");
            if (!*tmp && tmp[-2] == '\\') {
                q -= 2;
                refill();
                tmp = defbuf + (DEFMAX >> 1);
            }
        }
        *--q = 0;
        add_define(namebuf, arg, mtext);
    } else if (isspace(*tmp) || (!*tmp && (*(tmp+1) = '\0', *tmp = ' '))) {
        end = mtext + MLEN - 2;
        for (q = mtext; *tmp;) {
            *q = *tmp++;
            if (q < end) q++;
            else yyerror("Macro text too long");
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

#define SKPW while (isspace(*outp)) outp++

static int cmygetc() {
    int c;

    for (;;) {
      if ((c = *outp++) == '/') {
          if ((c = *outp) == '*') outp = skip_comment(outp, 0);
          else if (c == '/') return -1;
          else return c;
      } else return c;
    }
}

/* Check if yytext is a macro and expand if it is. */
static int expand_define()
{
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
        int c, parcnt = 0, dquote = 0, squote = 0;
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
                    if (!squote)
                        dquote ^= 1;
                    break;
                case '\'':
                    if (!dquote)
                        squote ^= 1;
                    break;
                case '(':
                    if (!squote && !dquote)
                        parcnt++;
                    break;
                case ')':
                    if (!squote && !dquote)
                        parcnt--;
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
                  } break;
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
              }
                else c = *outp++;
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
            if (*e == '#' && *(e + 1) == '#')
                e += 2;
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

static int exgetc()
{
    register char c, *yyp;

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
          }
          else yyerror("Incorrect definition macro after defined(\n");
          SKPW;
          if (*outp != ')') yyerror("Missing ) in defined");
          if (lookup_define(yytext))
              add_input("1 ");
          else
              add_input("0 ");
      } else {
          if (!expand_define())
              add_input("0 ");
          else SKPW;
      }
    }
    return c;
}

static int skip_to(const char *token, const char *atoken)
{
    char b[20], *p, *end;
    int c;
    int nest;

    for (nest = 0;;) {
        if (!fgets(outp = defbuf + (DEFMAX >> 1), MAXLINE-1,yyin)) {
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
                if (!strcmp(b, "endif"))
                    nest--;
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

#include "preprocess.c"

static int maybe_open_input_file (const char * fn) {
    if ((yyin = fopen(fn, "r")) == NULL) {
        return 0;
    }
    if (current_file) free((char *)current_file);
    current_file = (char *)malloc(strlen(fn) + 1);
    current_line = 0;
    strcpy(current_file, fn);
    return 1;
}

static void open_input_file (const char * fn) {
    if (!maybe_open_input_file(fn)) {
        perror(fn);
        exit(-1);
    }
}

static void open_output_file (const char * fn) {
    if ((yyout = fopen(fn, "w")) == NULL) {
        perror(fn);
        exit(-1);
    }
}

static void close_output_file() {
    fclose(yyout);
    yyout = 0;
}

static char *protect (const char * p) {
    static char buf[1024];
    char *bufp = buf;

    while (*p) {
        if (*p=='"' || *p == '\\') *bufp++ = '\\';
        *bufp++ = *p++;
    }
    *bufp = 0;
    return buf;
}

static void
create_option_defines() {
    defn_t *p;
    int count = 0;
    int i;

    fprintf(stderr, "Writing build options to %s ...\n", OPTION_DEFINES);
    open_output_file(OPTION_DEFINES);
    fprintf(yyout, "{\n");
    for (i = 0; i < DEFHASH; i++) {
        for (p = defns[i]; p; p = p->next)
            if (!(p->flags & DEF_IS_UNDEFINED)) {
                count++;
                fprintf(yyout, "  \"__%s__\", \"%s\",\n",
                        p->name, protect(p->exps));
                if (strncmp(p->name, "PACKAGE_", 8)==0) {
                    int len;
                    char *tmp, *t;

                    len = strlen(p->name + 8);
                    t = tmp = (char *)malloc(len + 1);
                    strcpy(tmp, p->name + 8);
                    while (*t) {
                        if (isupper(*t))
                            *t = tolower(*t);
                        t++;
                    }
                    if (num_packages == 100) {
                        fprintf(stderr, "Too many packages.\n");
                        exit(-1);
                    }
                    packages[num_packages++] = tmp;
                }
            }
    }
    fprintf(yyout,"};\n\n#define NUM_OPTION_DEFS %d\n\n", count);
    close_output_file();
}

static void deltrail() {
    register char *p;

    p = outp;
    while (*p && !isspace(*p) && *p != '\n') {
      p++;
    }
    *p = 0;
}

static void
handle_include (char * name)
{
    char *p;
    static char buf[1024];
    FILE *f;
    incstate *is;

    if (*name != '"') {
        defn_t *d;

        if ((d = lookup_define(name)) && d->nargs == -1) {
            char *q;

            q = d->exps;
            while (isspace(*q))
                q++;
            handle_include(q);
      } else {
            yyerrorp("Missing leading \" in %cinclude");
      }
        return;
    }
    for (p = ++name; *p && *p != '"'; p++);
    if (!*p) yyerrorp("Missing trailing \" in %cinclude");

    *p = 0;
    if ((f = fopen(name, "r")) != NULL) {
        is = (incstate *)
            malloc(sizeof(incstate) /*, 61, "handle_include: 1" */);
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

static void
handle_pragma (char * name)
{
    if (!strcmp(name, "auto_note_compiler_case_start"))
        pragmas |= PRAGMA_NOTE_CASE_START;
    else if (!strcmp(name, "no_auto_note_compiler_case_start"))
        pragmas &= ~PRAGMA_NOTE_CASE_START;
    else if (!strncmp(name, "ppchar:", 7) && *(name + 8))
        ppchar = *(name + 8);
    else yyerrorp("Unidentified %cpragma");
}

static void
preprocess() {
    register char *yyp, *yyp2;
    int c;
    int cond;

    while (buffered ? (yyp = yyp2 = outp) : fgets(yyp = yyp2 = defbuf + (DEFMAX >> 1), MAXLINE-1, yyin)) {
        if (!buffered) current_line++;
        else buffered = 0;
        while (isspace(*yyp2)) yyp2++;
        if ((c = *yyp2) == ppchar) {
            int quote = 0;
            char sp_buf = 0, *oldoutp;

            if (c == '%' && yyp2[1] == '%')
                grammar_mode++;
            outp = 0;
            if (yyp != yyp2) yyerrorp("Misplaced '%c'.\n");
            while (isspace(*++yyp2));
            yyp++;
            for (;;) {
                if ((c = *yyp2++) == '"') quote ^= 1;
                else{
                    if (!quote && c == '/') {
                        if (*yyp2 == '*') {
                            yyp2 = skip_comment(yyp2, 0);
                            continue;
                        }
                        else if (*yyp2 == '/') break;
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
            }
            else outp = yyp;
            *yyp = 0;
            yyp = defbuf + (DEFMAX >> 1) + 1;

            if (!strcmp("define", yyp)) {
                handle_define();
            } else if (!strcmp("if", yyp)) {
                cond = cond_get_exp(0);
                if (*outp != '\n') yyerrorp("Condition too complex in %cif");
                else handle_cond(cond);
            } else if (!strcmp("ifdef", yyp)) {
                deltrail();
                handle_cond(lookup_define(outp) != 0);
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
                    fprintf(yyout, "#line %d \"%s\"\n", current_line,
                            current_file);
                } else {
                    if (sp_buf) *oldoutp = sp_buf;
                    if (pragmas & PRAGMA_NOTE_CASE_START) {
                        if (*yyp == '%') pragmas &= ~PRAGMA_NOTE_CASE_START;
                    }
                    fprintf(yyout, "%s\n", yyp-1);
                }
            } else {
                char buff[200];
                sprintf(buff, "Unrecognised %c directive : %s\n", ppchar, yyp);
                yyerror(buff);
            }
        }
        else if (c == '/') {
            if ((c = *++yyp2) == '*') {
                if (yyout) fputs(yyp, yyout);
                yyp2 = skip_comment(yyp2, 1);
            } else if (c == '/' && !yyout) continue;
            else if (yyout) {
                fprintf(yyout, "%s", yyp);
            }
        }
        else if (yyout) {
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
                        switch(c) {
                          case '{':
                            {
                                if (!cquote && (++block_nest == 1))
                                    line_to_print = 1;
                                break;
                            }

                          case '}':
                            {
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
                                    *(yyp2-1) = '\n';
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

                if (line_to_print)
                    fprintf(yyout, "#line %d \"%s\"\n", current_line + 1,current_file);

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
    current_file = 0;
    nexpands = 0;
    if (inctop) {
      incstate *p = inctop;

      current_file = p->file;
      current_line = p->line;
      yyin = p->yyin;
      inctop = p->next;
      free((char *) p);
      preprocess();
    } else yyin = 0;
}

void make_efun_tables()
{
#define NUM_FILES     5
    static const char* outfiles[NUM_FILES] = {
        EFUN_TABLE, OPC_PROF, OPCODES, EFUN_PROTO, EFUN_DEFS
    };
    FILE *files[NUM_FILES];
    int i;

    fprintf(stderr, "Building efun tables ...\n");
    for (i = 0; i < NUM_FILES; i++) {
        files[i] = fopen(outfiles[i], "w");
        if (!files[i]) {
            fprintf(stderr, "make_func: unable to open %s\n", outfiles[i]);
            exit(-1);
        }
        fprintf(files[i],
                "/*\n\tThis file is automatically generated by make_func.\n");
        fprintf(files[i],
                "\tdo not make any manual changes to this file.\n*/\n\n");
    }

    fprintf(files[0],"\n#include \"efun_protos.h\"\n\n");
    fprintf(files[0],"\ntypedef void (*func_t) (void);\n\n");
    fprintf(files[0],"func_t efun_table[] = {\n");

    fprintf(files[1],"\ntypedef struct opc_s { char *name; int count; } opc_t;\n\n");
    fprintf(files[1],"opc_t opc_efun[] = {\n");

    fprintf(files[2], "\n/* operators */\n\n");
    for (i = 0; i < op_code; i++) {
        fprintf(files[2],"#define %-30s %d\n", oper_codes[i], i+1);
    }

    fprintf(files[2],"\n/* 1 arg efuns */\n#define BASE %d\n\n", op_code+1);
    for (i = 0; i < efun1_code; i++) {
        fprintf(files[0],"\tf_%s,\n", efun1_names[i]);
        fprintf(files[1],"{\"%s\", 0},\n", efun1_names[i]);
        fprintf(files[2],"#define %-30s %d\n", efun1_codes[i], i+op_code+1);
        fprintf(files[3],"void f_%s (void);\n", efun1_names[i]);
    }

    fprintf(files[2],"\n/* efuns */\n#define ONEARG_MAX %d\n\n", efun1_code + op_code+1);
    for (i = 0; i < efun_code; i++) {
        fprintf(files[0],"\tf_%s,\n", efun_names[i]);
        fprintf(files[1],"{\"%s\", 0},\n", efun_names[i]);
        fprintf(files[2],"#define %-30s %d\n", efun_codes[i], i+op_code+efun1_code+1);
        fprintf(files[3],"void f_%s (void);\n", efun_names[i]);
    }
    fprintf(files[0], "};\n");
    fprintf(files[1], "};\n");

    if (efun1_code + op_code >= 256) {
        fprintf(stderr, "You have way too many efuns.  Contact the MudOS developers if you really need this many.\n");
    }
    if (efun_code >= 256) {
        fprintf(stderr, "You have way too many efuns.  Contact the MudOS developers if you really need this many.\n");
    }
    fprintf(files[2],"\n/* efuns */\n#define NUM_OPCODES %d\n\n", efun_code + efun1_code + op_code);

    /* Now sort the main_list */
    for (i = 0; i < num_buff; i++) {
       int j;
       for (j = 0; j < i; j++)
           if (strcmp(key[i], key[j]) < 0) {
              const char *tmp;
              tmp = key[i]; key[i] = key[j]; key[j] = tmp;
              tmp = buf[i]; buf[i] = buf[j]; buf[j] = tmp;
           }
    }

    /* Now display it... */
    fprintf(files[4], "{\n");
    for (i = 0; i < num_buff; i++)
        fprintf(files[4], "%s", buf[i]);
    fprintf(files[4], "\n};\nint efun_arg_types[] = {\n");
    for (i=0; i < last_current_type; i++) {
        if (arg_types[i] == 0)
            fprintf(files[4], "0,\n");
        else
            fprintf(files[4], "%s,", ctype(arg_types[i]));
    }
    fprintf(files[4],"};\n");

    for (i=0; i < NUM_FILES; i++)
        fclose(files[i]);
}

static void handle_local_defines(int check) {
    defn_t *p;
    int i;
    int problem = 0;

    for (i = 0; i < DEFHASH; i++)
        for (p = defns[i]; p; p = p->next)
            p->flags |= DEF_IS_NOT_LOCAL;

    /* undefine _OPTIONS_H_ so it doesn't get propagated to the mudlib
       or interfere with copies of options.h */
    if ((p = lookup_define("_OPTIONS_H_"))) {
        p->flags |= DEF_IS_UNDEFINED;
        p->flags &= ~DEF_IS_NOT_LOCAL;
    }
    if ((p = lookup_define("DEBUG")))
        p->flags &= ~DEF_IS_NOT_LOCAL;

    ppchar = '#';
    preprocess();

    if ((p = lookup_define("_OPTIONS_H_")))
        p->flags |= DEF_IS_UNDEFINED;

    if (!check)
        return;

    for (i = 0; i < DEFHASH; i++)
        for (p = defns[i]; p; p = p->next)
            if (p->flags & DEF_IS_NOT_LOCAL) {
                fprintf(stderr, "No setting for %s in '%s'.\n",
                        p->name, LOCAL_OPTIONS);
                problem = 1;
            }

    if (problem) {
        fprintf(stderr, "\
***This local_options file appears to have been written for an\n\
***earlier version of the MudOS driver.  Please lookup the new options\n\
***(mentioned above) in the options.h file, decide how you would like them\n\
***set, and add those settings to the local_options file.\n");
        exit(-1);
    }
}

static void write_options_incl (int local) {
    open_output_file(OPTIONS_INCL);
    if (local) {
        fprintf(yyout, "#include \"%s\"\n", LOCAL_OPTIONS);
    } else {
        fprintf(yyout, "#include \"%s\"\n", OPTIONS_H);
    }
    close_output_file();
}

static void handle_options(int full) {
    open_input_file(OPTIONS_H);
    ppchar = '#';
    preprocess();

    if (!full) {
        /* don't do any checking, just find out what is defined */
        if (maybe_open_input_file(LOCAL_OPTIONS))
            handle_local_defines(0);
        return;
    }

    if (maybe_open_input_file(LOCAL_OPTIONS)) {
        fprintf(stdout, "Using '%s' file ...\n", LOCAL_OPTIONS);
        handle_local_defines(1);
        write_options_incl(1);
    } else {
        fprintf(stderr, "No \"%s\" file present.  If you create one from \"%s\",\nyou can use it when you get a new driver, and you will be warned if there are\nchanges to the real %s which you should include in your local file.\n",
                LOCAL_OPTIONS, OPTIONS_H, OPTIONS_H);
        write_options_incl(0);
    }

    create_option_defines();
}

static void handle_build_func_spec (char * command) {
    char buf[1024];
    int i;

    fprintf(stderr, "Building compiler files ...\n");
    sprintf(buf, "%s %s >%s", command, FUNC_SPEC, FUNC_SPEC_CPP);
    system(buf);
    for (i = 0; i < num_packages; i++) {
        sprintf(buf, "%s -I. packages/%s_spec.c >>%s",
                command, packages[i], FUNC_SPEC_CPP);
        system(buf);
    }

    open_output_file(PACKAGES);
    fprintf(yyout, "SRC=");
    for (i=0; i < num_packages; i++)
        fprintf(yyout, "%s.c ", packages[i]);
    fprintf(yyout, "\nOBJ=");
    for (i=0; i < num_packages; i++)
        fprintf(yyout, "%s.$(O) ", packages[i]);
    fprintf(yyout, "\n");
    close_output_file();
}

static void handle_process (char * file) {
    char buf[1024];
    int l;

    strcpy(buf, file);
    l = strlen(buf);
    if (strcmp(buf + l - 4, ".pre")) {
        fprintf(stderr, "Filename for -process must end in .pre\n");
        exit(-1);
    }
    *(buf + l - 4) = 0;

    fprintf(stderr, "Creating '%s' from '%s' ...\n", buf, file);

#ifdef DEBUG
    /* pass down the DEBUG define from CFLAGS */
    add_define("DEBUG", -1, " ");
#endif

    open_input_file(file);
    open_output_file(buf);
    ppchar = '%';
    preprocess();
    close_output_file();
}

static void handle_build_efuns() {
    void yyparse();

    num_buff = op_code = efun_code = efun1_code = 0;

    open_input_file(FUNC_SPEC_CPP);
    yyparse();
    make_efun_tables();
}

static void handle_applies() {
    FILE *f = fopen("applies", "r");
    FILE *out = fopen("applies.h", "w");
    FILE *table = fopen("applies_table.c", "w");
    char buf[8192];
    char *colon;
    char *p;
    int apply_number = 0;

    fprintf(out, "/* autogenerated from 'applies' */\n#ifndef APPLIES_H\n#define APPLIES_H\n\nextern const char *applies_table[];\n\n/* the folowing must be the first character of __INIT */\n#define APPLY___INIT_SPECIAL_CHAR\t\t'#'\n");
    fprintf(table, "/* autogenerated from 'applies' */\n\nconst char *applies_table[] = {\n");

    while (fgets(buf, 8192, f)) {
        buf[strlen(buf)-1] = 0;
        if (buf[0] == '#') break;
        if ((colon = strchr(buf, ':'))) {
            *colon++ = 0;
            fprintf(out, "#define APPLY_%-30s\t\"%s\"\n", buf, colon);
        } else {
            fprintf(out, "#define APPLY_%-30s\t", buf);
            p = buf;
            while (*p) {
                *p = tolower(*p);
                p++;
            }
            fprintf(out, "\"%s\"\n", buf);
        }
    }
    while (fgets(buf, 8192, f)) {
        buf[strlen(buf)-1] = 0;
        if ((colon = strchr(buf, ':'))) {
            *colon++ = 0;
            fprintf(table, "\t\"%s\",\n", colon);
            fprintf(out, "#define APPLY_%-30s\t%i\n", buf, apply_number++);
        } else {
            fprintf(out, "#define APPLY_%-30s\t%i\n", buf, apply_number++);
            p = buf;
            while (*p) {
                *p = tolower(*p);
                p++;
            }
            fprintf(table, "\t\"%s\",\n", buf);
        }
    }

    fprintf(table, "};\n");
    fprintf(out, "\n#define NUM_MASTER_APPLIES\t%i\n\n#endif\n", apply_number);

    fclose(out);
    fclose(table);
    fclose(f);
}

static void handle_malloc() {
#ifdef PEDANTIC
    int unlink(char *);
    int link(char *, char *);
#endif

    const char *the_malloc = 0, *the_wrapper = 0;

    if (lookup_define("SYSMALLOC"))
        the_malloc = "sysmalloc.c";
    if (lookup_define("SMALLOC"))
        the_malloc = "smalloc.c";
    if (lookup_define("BSDMALLOC"))
        the_malloc = "bsdmalloc.c";
    if (lookup_define("MMALLOC"))
        the_malloc = "mmalloc.c";
    if(lookup_define("MALLOC64"))
        the_malloc = "64bitmalloc.c";
    if(lookup_define("MALLOC32"))
        the_malloc = "32bitmalloc.c";
    if (lookup_define("GNUMALLOC"))
        the_malloc = "gnumalloc.c";

    if (lookup_define("WRAPPEDMALLOC"))
        the_wrapper = "wrappedmalloc.c";
    if (lookup_define("DEBUGMALLOC"))
        the_wrapper = "debugmalloc.c";

    if (!the_malloc && !the_wrapper) {
        fprintf(stderr, "Memory package and/or malloc wrapper incorrectly specified in options.h\n");
        exit(-1);
    }

    if (unlink("malloc.c") == -1 && errno != ENOENT)
        perror("unlink malloc.c");
    if (unlink("mallocwrapper.c") == -1 && errno != ENOENT)
        perror("unlink mallocwrapper.c");

    if (the_wrapper) {
        printf("Using memory allocation package: %s\n\t\tWrapped with: %s\n",
               the_malloc, the_wrapper);
        if (link(the_wrapper, "mallocwrapper.c") == -1)
            perror("link mallocwrapper.c");
    } else {
        printf("Using memory allocation package: %s\n", the_malloc);
    }
    if (link(the_malloc, "malloc.c") == -1)
        perror("link malloc.c");
}

static int check_include2 (const char * tag, const char * file,
                             const char * before, const char * after) {
    char buf[1024];
    FILE *ct;

    printf("Checking for include file <%s> ... ", file);
    ct = fopen("comptest.c", "w");
    fprintf(ct, "#include \"configure.h\"\n#include \"std_incl.h\"\n%s\n#include <%s>\n%s\n",
            before, file, after);
     fclose(ct);

    sprintf(buf, "%s %s -c comptest.c " TO_DEV_NULL, COMPILER, CFLAGS);
    if (!compile(buf)) {
        fprintf(yyout, "#define %s\n", tag);
        /* Make sure the define exists for later checks */
        fflush(yyout);
        printf("exists\n");
        return 1;
    }
    printf("does not exist or is unusable\n");
    return 0;
}

static int check_include (const char * tag, const char * file) {
    char buf[1024];
    FILE *ct;

    printf("Checking for include file <%s> ... ", file);
    ct = fopen("comptest.c", "w");
    fprintf(ct, "#include \"configure.h\"\n#include \"std_incl.h\"\n#include \"file_incl.h\"\n#include <%s>\n", file);
    fclose(ct);

    sprintf(buf, "%s %s -c comptest.c " TO_DEV_NULL, COMPILER, CFLAGS);
    if (!compile(buf)) {
        fprintf(yyout, "#define %s\n", tag);
        /* Make sure the define exists for later checks */
        fflush(yyout);
        printf("exists\n");
        return 1;
    }
    printf("does not exist\n");
    return 0;
}

static int check_library (const char * lib) {
    char buf[1024];
    FILE *ct;

    printf("Checking for library %s ... ", lib);
    ct = fopen("comptest.c", "w");
    fprintf(ct, "int main() { return 0; }\n");
    fclose(ct);

    sprintf(buf, "%s %s comptest.c %s" TO_DEV_NULL, COMPILER, CFLAGS, lib);
    if (!compile(buf)) {
        fprintf(yyout, " %s", lib);
        printf("exists\n");
        return 1;
    }
    printf("does not exist\n");
    return 0;
}

#if 0 /* not used any more */
static int check_ret_type (char * tag, char * pre,
                             char * type, char * func) {
    char buf[1024];
    FILE *ct;

    printf("Checking return type of %s() ...", func);
    ct = fopen("comptest.c", "w");
    fprintf(ct, "%s\n\n%s%s();\n", pre, type, func);
    fclose(ct);

    sprintf(buf, "%s %s -c comptest.c >/dev/null 2>&1", COMPILER, CFLAGS);
    if (!system(buf)) {
        fprintf(yyout, "#define %s\n", tag);
        printf("returns %s\n", type);
        return 1;
    }
    printf("does not return %s\n", type);
    return 0;
}
#endif

/* This should check a.out existence, not exit value */
static int check_prog (const char * tag, const char * pre, const char * code, int andrun) {
    char buf[1024];
    FILE *ct;

    ct = fopen("comptest.c", "w");
    fprintf(ct, "#include \"configure.h\"\n#include \"std_incl.h\"\n%s\n\nint main() {%s}\n", (pre ? pre : ""), code);
    fclose(ct);

    sprintf(buf, "%s %s comptest.c -o comptest" TO_DEV_NULL, COMPILER, CFLAGS);
    if (!compile(buf) && (!andrun || !system("./comptest"))) {
        if (tag) {
            fprintf(yyout, "#define %s\n", tag);
            fflush(yyout);
        }
        return 1;
    }

    return 0;
}

static int check_code (const char * pre, const char * code) {
    char buf[1024];
    FILE *ct;
    int rc;

    ct = fopen("comptest.c", "w");
    fprintf(ct, "#include \"configure.h\"\n#include \"std_incl.h\"\n%s\n\nint main() {%s}\n", (pre ? pre : ""), code);
    fclose(ct);

    sprintf(buf, "%s %s comptest.c -o comptest" TO_DEV_NULL, COMPILER, CFLAGS);
    if (compile(buf) || (rc = system("./comptest")) == 127 || rc == -1) {
        return -1;
    }
    return rc;
}

static void check_linux_libc() {
    char buf[1024];
    FILE *ct;

    ct = fopen("comptest.c", "w");
    fprintf(ct, "int main() { }\n");
    fclose(ct);

    sprintf(buf, "%s -g comptest.c -o comptest >/dev/null 2>&1", COMPILER);
    if (system(buf)) {
        fprintf(stderr, "   libg.a/so installed wrong, trying workaround ...\n");
        sprintf(buf, "%s -g comptest.c -lc -o comptest >/dev/null 2>&1", COMPILER);
        if (system(buf)) {
            fprintf(stderr, "*** FAILED.\n");
            exit(-1);
        }
        fprintf(yyout, " -lc");
    }
}

static const char *memmove_prog = "\
char buf[80];\n\
strcpy(buf,\"0123456789ABCDEF\");\n\
memmove(&buf[1],&buf[4],13);\n\
if(strcmp(buf,\"0456789ABCDEF\")) exit(-1);\n\
memmove(&buf[8],&buf[6],9);\n\
if(strcmp(buf,\"0456789A9ABCDEF\")) exit(-1);\n\
return 0;\n";

static int check_memmove (const char * tag, const char * str) {
    return check_prog(tag, str, memmove_prog, 1);
}

static void find_memmove() {
    printf("Checking for memmove() ...");
    if (check_memmove(0, "")) {
        printf(" exists\n");
        return;
    }
    if (check_memmove("USE_BCOPY", "#define memmove(a,b,c) bcopy(b,a,c)")) {
        printf(" simulating via bcopy()\n");
        return;
    }
    printf(" missing; using MudOS's version\n");
    fprintf(yyout, "#define MEMMOVE_MISSING\n");
}

static void verbose_check_prog (const char * msg, const char * def, const char * pre,
                                  const char * prog, int andrun) {
    printf("%s ...", msg);
    if (check_prog(def, pre, prog, andrun))
        printf(" exists\n");
    else printf(" does not exist\n");
}

static int check_configure_version() {
    char buf[1024];
    FILE *ct;

    ct = fopen("comptest.c", "w");
    fprintf(ct, "#include \"configure.h\"\n\n#if CONFIGURE_VERSION < %i\nthrash and die\n#endif\n\nint main() { }\n", CONFIGURE_VERSION);
    fclose(ct);

    sprintf(buf, "%s %s comptest.c -o comptest " TO_DEV_NULL, COMPILER, CFLAGS);
    return !compile(buf);
}

static void handle_configure() {
    if (check_configure_version()) return;

    open_output_file("configure.h");

#ifndef WIN32
    check_include("INCL_STDLIB_H", "stdlib.h");
    check_include("INCL_UNISTD_H", "unistd.h");
    if (check_include("INCL_TIME_H", "time.h")) {
        if (!check_prog(0, "#include <time.h>", "tzset();", 0)) {
            if (check_prog(0, 0, "void tzset(); tzset();", 0))
                fprintf(yyout, "#define PROTO_TZSET\n#define USE_TZSET\n");
        }
        else
            fprintf(yyout, "#define USE_TZSET\n");
    } else {
        if (check_prog(0, 0, "void tzset(); tzset();", 0))
            fprintf(yyout, "#define PROTO_TZSET\n#define USE_TZSET\n");
    }
    check_include("INCL_SYS_TIMES_H", "sys/times.h");
    check_include("INCL_FCNTL_H", "fcntl.h");
    check_include("INCL_SYS_TIME_H", "sys/time.h");
    check_include("INCL_DOS_H", "dos.h");
    check_include("INCL_USCLKC_H", "usclkc.h");
    check_include("INCL_LIMITS_H", "limits.h");
    check_include("INCL_LOCALE_H", "locale.h");
    if (!check_prog(0, 0, "int x = USHRT_MAX;", 0)) {
        if (!check_prog(0, 0, "int x = MAXSHORT;", 0))
            check_include("INCL_VALUES_H", "values.h");
        fprintf(yyout, "#define USHRT_MAX  (MAXSHORT)\n");
    }

    check_include("INCL_NETINET_IN_H", "netinet/in.h");
    check_include("INCL_ARPA_INET_H", "arpa/inet.h");

    check_include("INCL_SYS_TYPES_H", "sys/types.h");
    check_include("INCL_SYS_IOCTL_H", "sys/ioctl.h");
    check_include("INCL_SYS_SOCKET_H", "sys/socket.h");
    check_include("INCL_NETDB_H", "netdb.h");
    /* TELOPT_NAWS is missing from <arpa/telnet.h> on some systems */
    check_include2("INCL_ARPA_TELNET_H", "arpa/telnet.h", "", "int i=TELOPT_NAWS;");
    check_include("INCL_SYS_SEMA_H", "sys/sema.h");
    check_include("INCL_SYS_SOCKETVAR_H", "sys/socketvar.h");
    check_include("INCL_SOCKET_H", "socket.h");
    check_include("INCL_RESOLVE_H", "resolve.h");

    check_include("INCL_SYS_STAT_H", "sys/stat.h");

    /* sys/dir.h is BSD, dirent is sys V.  Try to do it the BSD way first. */
    /* If that fails, fall back to sys V */
    if (check_prog("BSD_READDIR", "#include <sys/dir.h>", "struct direct *d; d->d_namlen;", 0)) {
        check_include("INCL_SYS_DIR_H", "sys/dir.h");
    } else {
        /* could be either of these */
        check_include("INCL_DIRENT_H", "dirent.h");
        check_include("INCL_SYS_DIRENT_H", "sys/dirent.h");
        fprintf(yyout, "#define USE_STRUCT_DIRENT\n");
    }

    check_include("INCL_SYS_FILIO_H", "sys/filio.h");
    check_include("INCL_SYS_SOCKIO_H", "sys/sockio.h");
    check_include("INCL_SYS_MKDEV_H", "sys/mkdev.h");
    check_include("INCL_SYS_RESOURCE_H", "sys/resource.h");
    check_include("INCL_SYS_RUSAGE_H", "sys/rusage.h");
    check_include("INCL_SYS_WAIT_H", "sys/wait.h");
    check_include("INCL_SYS_CRYPT_H", "sys/crypt.h");
    check_include("INCL_CRYPT_H", "crypt.h");
    check_include("INCL_MALLOC_H", "my_malloc.h");

    /* for NeXT */
    if (!check_include("INCL_MACH_MACH_H", "mach/mach.h"))
        check_include("INCL_MACH_H", "mach.h");

    /* figure out what we need to do to get major()/minor() */
    check_include("INCL_SYS_SYSMACROS_H", "sys/sysmacros.h");

#ifdef DEBUG
    /* includes just to shut up gcc's warnings on some systems */
    check_include("INCL_BSTRING_H", "bstring.h");
#endif

    /* Runtime loading support */
    if (check_include("INCL_DLFCN_H", "dlfcn.h")) {
        if (!check_prog(0, "#include <dlfcn.h>", "int x = RTLD_LAZY;", 0))
            fprintf(yyout, "#define RTLD_LAZY      1\n");
    } else {
        if (!check_prog(0, 0, "int x = RTLD_LAZY;", 0))
            fprintf(yyout, "#define RTLD_LAZY     1\n");
    }

    /* SunOS is missing definition for INADDR_NONE */
    printf("Checking for missing INADDR_NONE ... ");
    if (!check_prog(0, "#include <netinet/in.h>", "int x = INADDR_NONE;", 0)) {
        printf("missing\n");
        fprintf(yyout, "#define INADDR_NONE (unsigned int)0xffffffff\n");
    } else printf("ok\n");

    printf("Checking for random number generator ...");
    if (check_prog("DRAND48", 0, "srand48(0);", 0)) {
        printf(" using drand48()\n");
    } else
    if (check_prog("RAND", 0, "srand(0);", 0)) {
        printf(" using rand()\n");
    } else
    if (check_prog("RANDOM", 0, "srandom(0);", 0)) {
        printf("using random()\n");
    } else {
        printf("WARNING: did not find a random number generator\n");
        exit(-1);
    }

    if (check_prog("USE_BSD_SIGNALS", 0, "SIGCHLD; wait3(0, 0, 0);", 0)) {
        printf("Using BSD signals.\n");
    } else {
        printf("Using System V signals.\n");
    }

    printf("Checking if signal() returns SIG_ERR on error ...");
    if (check_prog("SIGNAL_ERROR SIG_ERR", 0, "if (signal(0, 0) == SIG_ERR) ;", 0)) {
        printf(" yes\n");
    } else {
        fprintf(yyout, "#define SIGNAL_ERROR BADSIG\n");
        printf(" no\n");
    }

    printf("Not Checking for inline ...(usage in driver code all broken anyway)");
    //if (!check_prog("INLINE inline", "inline void foo() { }", "foo();", 0)) {
        //printf(" __inline ...");
        //if (!check_prog("INLINE __inline", "__inline void foo() {}", "foo();", 0)) {
            fprintf(yyout, "#define INLINE\n");
        //}
    //}
    printf(" const ...\n");
    if (!check_prog("CONST const", "int foo(const int *, const int *);", "", 0))
        fprintf(yyout, "#define CONST\n");

    verbose_check_prog("Checking for strerror()", "HAS_STRERROR",
                       "", "strerror(12);", 0);
    verbose_check_prog("Checking for POSIX getcwd()", "HAS_GETCWD",
                       "", "getcwd(\"\", 1000);", 0);
    verbose_check_prog("Checking for getrusage()", "RUSAGE",
                       "", "getrusage(0, 0);", 0);
    verbose_check_prog("Checking for times()", "TIMES",
                       "", "times(0);", 0);
    verbose_check_prog("Checking for gettimeofday()", "HAS_GETTIMEOFDAY",
                       "", "gettimeofday(0, 0);", 0);
    verbose_check_prog("Checking for fchmod()", "HAS_FCHMOD",
                       "", "fchmod(0, 0);", 0);

    printf("Checking for big or little endian ... ");
    if (!check_code("char num[] = { 0x11, 0x22, 0x33, 0x44 }; int *foo = (int *)num;",
                    "return (*foo == 0x44332211);")) {
        printf("big\n");
        fprintf(yyout, "#define BIGENDIAN 1\n");
        fflush(yyout);
    } else printf("little\n");

    find_memmove();
#endif

    fprintf(yyout, "#define SIZEOF_INT %i\n", sizeof(int));
    fprintf(yyout, "#define SIZEOF_PTR %i\n", sizeof(char *));
    fprintf(yyout, "#define SIZEOF_SHORT %i\n", sizeof(short));
    fprintf(yyout, "#define SIZEOF_FLOAT %i\n", sizeof(double));
    fprintf(yyout, "#define SIZEOF_LONG %i\n", sizeof(long));

    if (sizeof(unsigned long) == 4)
        fprintf(yyout, "#define UINT32 unsigned long\n");
    else if (sizeof(unsigned int) == 4)
        fprintf(yyout, "#define UINT32 unsigned int\n");
    else {
        printf("WARNING: could not find a 32 bit integral type.\n");
        exit(-1);
    }

    /* PACKAGE_DB stuff */
    if (lookup_define("PACKAGE_DB")) {
        /* -I would be nicer for added include paths, but we don't have an easy way to
         * set -I paths right now
         */
        if (lookup_define("USE_MSQL")) {
            if (!(check_include("INCL_LOCAL_MSQL_H", "/usr/local/include/msql.h")
                || check_include("INCL_LOCAL_MSQL_MSQL_H", "/usr/local/msql/include/msql.h")
                || check_include("INCL_LOCAL_MINERVA_MSQL_H", "/usr/local/Minerva/include/msql.h")
                || check_include("INCL_LIB_HUGHES_MSQL_H", "/usr/lib/Hughes/include/msql.h"))) {
                fprintf(stderr, "Cannot find msql.h, compilation is going to fail miserably.\n");
            }
        }
        if (lookup_define("USE_MYSQL")) {
            if (!(check_include("INCL_LOCAL_MYSQL_H", "/usr/local/include/mysql.h")
                || check_include("INCL_LOCAL_INCLUDE_MYSQL_MYSQL_H", "/usr/local/include/mysql/mysql.h")
                || check_include("INCL_LOCAL_MYSQL_MYSQL_H", "/usr/local/mysql/include/mysql.h")
                || check_include("INCL_MYSQL_MYSQL_H", "/usr/include/mysql/mysql.h")
                || check_include("INCL_MYSQL_INCLUDE_MYSQL_H", "/usr/mysql/include/mysql/mysql.h"))) {
                fprintf(stderr, "Cannot find mysql.h, compilation is going to fail miserably.\n");
            }
        }
        if (lookup_define("USE_POSTGRES")) {
        	if (!(check_include("USE_POSTGRES", "/usr/include/postgresql/libpq-fe.h"))) {
        		fprintf(stderr,
        				"Cannot find libpq-fe.h, compilation is going to fail miserably.\n");
        	}
        }
    }

    fprintf(yyout, "#define CONFIGURE_VERSION   %i\n\n", CONFIGURE_VERSION);

    close_output_file();

#ifdef WIN32
    system("echo Windows detected. Applying libs.");
    if (lookup_define("HAVE_ZLIB")){
        system("echo  -lwsock32 -lws2_32 -lz> system_libs");
    }
        else system("echo  -lwsock32 -lws2_32 > system_libs");
    system("copy windows\\configure.h tmp.config.h");
    system("type configure.h >> tmp.config.h");
    system("del configure.h");
    system("rename tmp.config.h configure.h");
#else

    open_output_file("system_libs");
    check_library("-lresolv");
    check_library("-lbsd");
    check_library("-lBSD");
    check_library("-ly");

    /* don't add -lcrypt if crypt() is in libc.a */
    if (!check_prog(0, "#include \"lint.h\"",
                    "char *x = crypt(\"foo\", \"bar\");", 0))
        check_library("-lcrypt");
    /* don't add -lmalloc if malloc() works */
    if (!check_prog(0, "", "char *x = malloc(100);", 0))
        check_library("-lmalloc");

    /* we don't currently use it anywhere
    if (!check_prog(0, "", "void *x = dlopen(0, 0);", 0))
        check_library("-ldl");
    */
    check_library("-lsocket");
    check_library("-linet");
    check_library("-lnsl");
    check_library("-lnsl_s");
    check_library("-lseq");
    check_library("-lm");

    if (lookup_define("CYGWIN"))
            check_library("-liconv");


    if (lookup_define("MINGW")){
        check_library("-lwsock32");
        check_library("-lws2_32");
    }

    if (lookup_define("HAVE_ZLIB"))
        check_library("-lz");

    if (lookup_define("PACKAGE_ASYNC"))
        check_library("-lpthread");
    if (lookup_define("PACKAGE_HASH"))
        check_library("-lssl");
    if (lookup_define("PACKAGE_PCRE"))
        check_library("-lpcre");
    fprintf(stderr, "Checking for flaky Linux systems ...\n");
    check_linux_libc();

    /* PACKAGE_DB stuff */
    if (lookup_define("PACKAGE_DB") && lookup_define("USE_MSQL")) {
        if (!(check_library("-lmsql") ||
              check_library("-L/usr/local/lib -lmsql") ||
              check_library("-L/usr/local/msql/lib -lmsql") ||
              check_library("-L/usr/local/Minerva/lib -lmsql") ||
              check_library("-L/usr/lib/Hughes/lib -lmsql"))) {
            fprintf(stderr, "Cannot find libmsql.a, compilation is going to fail miserably\n");
        }
    }
    if (lookup_define("PACKAGE_DB") && lookup_define("USE_MYSQL")) {
        if (!(check_library("-lmysqlclient") ||
              check_library("-L/usr/local/lib -lmysqlclient") ||
              check_library("-L/usr/local/lib/mysql -lmysqlclient") ||
              check_library("-L/usr/local/mysql/lib -lmysqlclient") ||
              check_library("-L/usr/lib64/mysql -lmysqlclient") ||
              check_library("-L/usr/lib/mysql -lmysqlclient") ||
              check_library("-L/usr/mysql/lib/64/mysql -lmysqlclient"))) {
            fprintf(stderr, "Cannot find libmysqlclient.a, compilation is going to fail miserably\n");
        }
    }
    if (lookup_define("PACKAGE_DB") && lookup_define("USE_POSTGRES")) {
    	if (!(check_library("-lpq"))) {
    		fprintf(stderr, "Cannot find libpq.a, compilation is going to fail miserably\n");
    	}
    }
    fprintf(yyout, "\n\n");
    close_output_file();
#endif
}

int main (int argc, char ** argv) {
    int idx = 1;

    while (idx < argc) {
        if (argv[idx][0] != '-') {
            fprintf(stderr, SYNTAX);
            exit(-1);
        }
        if (strcmp(argv[idx], "-configure")==0) {
            handle_options(0);
            handle_configure();
        } else
        if (strcmp(argv[idx], "-process")==0) {
            handle_process(argv[++idx]);
        } else
        if (strcmp(argv[idx], "-options")==0) {
            handle_options(1);
        } else
        if (strcmp(argv[idx], "-malloc")==0) {
            handle_malloc();
        } else
        if (strcmp(argv[idx], "-build_applies")==0) {
            handle_applies();
        } else
        if (strcmp(argv[idx], "-build_func_spec")==0) {
            handle_build_func_spec(argv[++idx]);
        } else
        if (strcmp(argv[idx], "-build_efuns")==0) {
            handle_build_efuns();
        } else {
            fprintf(stderr, "Unrecognized flag %s\n", argv[idx]);
            exit(-1);
        }
        idx++;
    }
    printf("\n");
    return 0;
}
