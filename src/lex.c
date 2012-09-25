/*
 * File: lex.c
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

#define SUPPRESS_COMPILER_INLINES
#include "std.h"
#include "file_incl.h"
#include "lpc_incl.h"
#include "compiler.h"
#if defined(WIN32) && !defined(MINGW)
#  include "grammar_tab.h"
#else
#  include "grammar.tab.h"
#endif
#include "scratchpad.h"
#include "preprocess.h"
#include "md.h"
#include "hash.h"
#include "file.h"
#include "main.h"
#include "cc.h"
#include "master.h"

#define NELEM(a) (sizeof (a) / sizeof((a)[0]))
#define LEX_EOF ((unsigned char) EOF)

char lex_ctype[256] = {0,0,0,0,0,0,0,0,0,1,0,1,1,1,0,0,0,0,0,0,0,0,0,
                       0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
                       0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                       0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                       0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                       0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                       0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                       0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                       0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                       0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                       0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                       0,0,0};

#define is_wspace(c) lex_ctype[(unsigned char)(c)]

int current_line;               /* line number in this file */
int current_line_base;          /* number of lines from other files */
int current_line_saved;         /* last line in this file where line num
                                   info was saved */
int total_lines;                /* Used to compute average compiled lines/s */
char *current_file;
int current_file_id;

/* Bit flags for pragmas in effect */
int pragmas;

int num_parse_error;            /* Number of errors in the parser. */

lpc_predef_t *lpc_predefs = NULL;

static int yyin_desc;
int lex_fatal;
static char **inc_list;
static int inc_list_size;
static int defines_need_freed = 0;
static char *last_nl;
static int nexpands = 0;

#define EXPANDMAX 25000
static char *expands[EXPANDMAX];
static int expand_depth = 0;

char yytext[MAXLINE];
char *outp;

typedef struct incstate_s {
    struct incstate_s *next;
    int yyin_desc;
    int line;
    char *file;
    int file_id;
    char *last_nl;
    char *outp;
} incstate_t;

static incstate_t *inctop = 0;

/* prevent unbridled recursion */
#define MAX_INCLUDE_DEPTH 32
static int incnum;

/* If more than this is needed, the code needs help :-) */
#define MAX_FUNCTION_DEPTH 10

static function_context_t function_context_stack[MAX_FUNCTION_DEPTH];
static int last_function_context;
function_context_t *current_function_context = 0;

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
keyword_t predefs[] =
#include "efun_defs.c"

const char *option_defs[] =
#include "option_defs.c"

static keyword_t reswords[] =
{
#ifdef DEBUG
    {"__TREE__", L_TREE, 0 },
#endif
#ifdef ARRAY_RESERVED_WORD
    {"array", L_ARRAY, 0 },
#endif
    {"asm", 0, 0},
    {"break", L_BREAK, 0},
#ifndef NO_BUFFER_TYPE
    {"buffer", L_BASIC_TYPE, TYPE_BUFFER},
#endif
    {"case", L_CASE, 0},
    {"catch", L_CATCH, 0},
#ifdef STRUCT_CLASS
    {"class", L_CLASS, 0},
#endif
#ifdef COMPAT_32
    {"closure", L_BASIC_TYPE, TYPE_FUNCTION},
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
    {"ref", L_REF, 0 },
#endif
    {"return", L_RETURN, 0},
    {"sscanf", L_SSCANF, 0},
#ifndef SENSIBLE_MODIFIERS
    {"static", L_TYPE_MODIFIER, DECL_NOSAVE | DECL_PROTECTED },
#endif
#ifdef HAS_STATUS_TYPE
    {"status", L_BASIC_TYPE, TYPE_NUMBER},
#endif
    {"string", L_BASIC_TYPE, TYPE_STRING},
#ifdef STRUCT_STRUCT
    {"struct", L_CLASS, 0},
#endif
    {"switch", L_SWITCH, 0},
    {"time_expression", L_TIME_EXPRESSION, 0},
    {"varargs", L_TYPE_MODIFIER, FUNC_VARARGS },
#ifdef VIRTUAL_RESERVED_WORD
    {"virtual", L_TYPE_MODIFIER, 0 },
#endif
    {"void", L_BASIC_TYPE, TYPE_VOID},
    {"while", L_WHILE, 0},
};

static ident_hash_elem_t **ident_hash_table;
static ident_hash_elem_t **ident_hash_head;
static ident_hash_elem_t **ident_hash_tail;

static ident_hash_elem_t *ident_dirty_list = 0;

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

static linked_buf_t head_lbuf = { NULL, TERM_START };
static linked_buf_t *cur_lbuf;

static void handle_define (char *);
static void free_defines (void);
static void add_define (const char *, int, const char *);
static void add_predefine (const char *, int, const char *);
static int expand_define (void);
static void add_input (const char *);
static int cond_get_exp (int);
static void merge (char *name, char *dest);
static void add_quoted_predefine (const char *, const char *);
static void lexerror (const char *);
static int skip_to (const char *, const char *);
static void handle_cond (int);
static int inc_open (char *, char *, int);
static void include_error (const char *, int);
static void handle_include (char *, int);
static int get_terminator (char *);
static int get_array_block (char *);
static int get_text_block (char *);
static void skip_line (void);
static void skip_comment (void);
static void deltrail (char *);
static void handle_pragma (char *);
static int cmygetc (void);
static void refill (void);
static void refill_buffer (void);
static int exgetc (void);
static int old_func (void);
static ident_hash_elem_t *quick_alloc_ident_entry (void);
static void yyerrorp (const char *);

#define LEXER
#include "preprocess.c"

int lookup_predef(const char * name)
{
    int x;

    for(x = 0; x < (sizeof(predefs) / sizeof(keyword_t)); x++)
    {
        if(strcmp(name, predefs[x].word) == 0)
        {
            return x;
        }
    }

    return -1;
}

static void merge (char * name, char * dest)
{
    char *from;

    strcpy(dest, current_file);
    if ((from = strrchr(dest, '/')))    /* strip filename */
        *from = 0;
    else
        /* current_file was the file_name */
        /* include from the root directory */
        *dest = 0;

    from = name;
    while (*from == '/') {
        from++;
        *dest = 0;              /* absolute path */
    }

    while (*from) {
        if (!strncmp(from, "../", 3)) {
            char *tmp;

            if (*dest == 0)     /* including from above mudlib is NOT allowed */
                break;
            tmp = strrchr(dest, '/');
            if (tmp == NULL)    /* 1 component in dest */
                *dest = 0;
            else
                *tmp = 0;
            from += 3;          /* skip "../" */
        } else if (!strncmp(from, "./", 2)) {
            from += 2;
        } else {                /* append first component to dest */
            char *q;

            if (*dest)
                strcat(dest, "/");      /* only if dest is not empty !! */
            q = strchr(from, '/');

            if (q) {            /* from has 2 or more components */
                while (*from == '/')    /* find the start */
                    from++;
                strncat(dest, from, q - from);
                for (from = q + 1; *from == '/'; from++);
            } else {
                /* this was the last component */
                strcat(dest, from);
                break;
            }
        }
    }
}

static void
yyerrorp (const char * s) {
    char buf[200];
    sprintf(buf, s, '#');
    yyerror(buf);
    lex_fatal++;
}

static void
lexerror (const char * s)
{
    yyerror(s);
    lex_fatal++;
}

static int
skip_to (const char * token, const char * atoken)
{
    char b[20], *p;
    unsigned char c;
    register char *yyp = outp,  *startp;
    char *b_end = b + 19;
    int nest;

    for (nest = 0;;) {
        if ((c = *yyp++) == '#') {
            while (is_wspace(c = *yyp++));
            startp = yyp - 1;
            for (p = b; !isspace(c) && c != LEX_EOF; c = *yyp++) {
                if (p < b_end) *p++ = c;
                else break;
            }
            *p = 0;
            if (!strcmp(b, "if") || !strcmp(b, "ifdef") || !strcmp(b, "ifndef")) {
                nest++;
            } else if (nest > 0) {
                if (!strcmp(b, "endif"))
                    nest--;
            } else {
                if (!strcmp(b, token)) {
                    outp = startp;
                    *--outp = '#';
                    return 1;
                } else if (atoken && !strcmp(b, atoken)) {
                    outp = startp;
                    *--outp = '#';
                    return 0;
                } else if (!strcmp(b, "elif")) {
                    outp = startp;
                    *--outp = '#';
                    return (atoken == 0);
                }
            }
        }
        while (c != '\n' && c != LEX_EOF) c = *yyp++;
        if (c == LEX_EOF) {
            lexerror("Unexpected end of file while looking for #endif");
            outp = yyp - 1;
            return 1;
        }
        current_line++;
        total_lines++;
        if (yyp == last_nl + 1) {
            outp = yyp;
            refill_buffer();
            yyp = outp;
        }
    }
}

static int
inc_open (char * buf, char * name, int check_local)
{
    int i, f;
    char *p;
	const char *tmp;
    if (check_local) {
        merge(name, buf);
        tmp = check_valid_path(buf, master_ob, "include", 0);
        if (tmp && (f = open(tmp, O_RDONLY)) != -1)
            return f;
    }
    /*
     * Search all include dirs specified.
     */
    for (p = strchr(name, '.'); p; p = strchr(p + 1, '.')) {
        if (p[1] == '.')
            return -1;
    }
    for (i = 0; i < inc_list_size; i++) {
        sprintf(buf, "%s/%s", inc_list[i], name);
        tmp = check_valid_path(buf, master_ob, "include", 0);
        if (tmp && (f = open(tmp, O_RDONLY)) != -1) {
            return f;
        }
    }
    return -1;
}

static void
include_error (const char * msg, int global)
{
    current_line--;

    if (global) {
        int saved_pragmas = pragmas;

        pragmas &= ~PRAGMA_ERROR_CONTEXT;
        lexerror(msg);
        pragmas = saved_pragmas;
    } else {
        yyerror(msg);
    }

    current_line++;
}

static void
handle_include (char * name, int global)
{
    char *p, *nameptr;
    static char buf[MAXLINE];
    incstate_t *is;
    int delim, f;

    if (*name != '"' && *name != '<') {
        defn_t *d;

        if ((d = lookup_define(name)) && d->nargs == -1) {
            char *q;

            q = d->exps;
            while (uisspace(*q))
                q++;
            handle_include(q, global);
        } else {
            include_error("Missing leading \" or < in #include", global);
        }
        return;
    }
    name = string_copy(name, "handle_include");
    push_malloced_string(name);
    delim = *name++ == '"' ? '"' : '>';
    for (p = name; *p && *p != delim; p++);
    if (!*p) {
        pop_stack();
        include_error("Missing trailing \" or > in #include", global);
        return;
    }
    if (strlen(name) > sizeof(buf) - 100) {
        pop_stack();
        include_error("Include name too long.", global);
        return;
    }
    *p = 0;
    if (++incnum == MAX_INCLUDE_DEPTH) {
        include_error("Maximum include depth exceeded.", global);
    } else if ((f = inc_open(buf, name, delim == '"')) != -1) {
        is = ALLOCATE(incstate_t, TAG_COMPILER, "handle_include: 1");
        is->yyin_desc = yyin_desc;
        is->line = current_line;
        is->file = current_file;
        is->file_id = current_file_id;
        is->last_nl = last_nl;
        is->next = inctop;
        is->outp = outp;
        inctop = is;
        current_line--;
        save_file_info(current_file_id, current_line  - current_line_saved);
        current_line_base += current_line;
        current_line_saved = 0;
        current_line = 1;
        current_file = make_shared_string(buf);
        current_file_id = add_program_file(buf, 0);
        yyin_desc = f;
        refill_buffer();
    } else {
        sprintf(buf, "Cannot #include %s", name);
        include_error(buf, global);
    }
    pop_stack();
}

static int
get_terminator (char * terminator)
{
    unsigned char c;
    int j = 0;

    while (((c = *outp++) != LEX_EOF) && (isalnum(c) || c == '_'))
        terminator[j++] = c;

    terminator[j] = '\0';

    while (is_wspace(c) && c != LEX_EOF)
        c = *outp++;

    if (c == LEX_EOF)
        return 0;

    if (c == '\n') {
        current_line++;
        if (outp == last_nl + 1) refill_buffer();
    } else {
        outp--;
    }

    return j;
}

#define MAXCHUNK (MAXLINE*4)
#define NUMCHUNKS (DEFMAX/MAXCHUNK)

#define NEWCHUNK(line) \
    if (len == MAXCHUNK - 1) { \
        line[curchunk][MAXCHUNK - 1] = '\0'; \
        if (curchunk == NUMCHUNKS - 1) { \
            res = -2; \
            break; \
        } \
        line[++curchunk] = \
              (char *)DXALLOC(MAXCHUNK, TAG_COMPILER, "array/text chunk"); \
        len = 0; \
    }

static int
get_array_block (char * term)
{
    int termlen;                /* length of terminator */
    char *array_line[NUMCHUNKS];/* allocate memory in chunks */
    int header, len;            /* header length; position in chunk */
    int startpos, startchunk;   /* start of line */
    int curchunk, res;          /* current chunk; this function's result */
    int i;                      /* an index counter */
    unsigned char c;            /* a char */
    char *yyp = outp;

    /*
     * initialize
     */
    termlen = strlen(term);
    array_line[0] = (char *) DXALLOC(MAXCHUNK, TAG_COMPILER, "array_block");
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

    while (1) {
        while (((c = *yyp++) != '\n') && (c != LEX_EOF)) {
            NEWCHUNK(array_line);
            if (c == '"' || c == '\\') {
                array_line[curchunk][len++] = '\\';
                NEWCHUNK(array_line);
            }
            array_line[curchunk][len++] = c;
        }

        if (c == '\n' && (yyp == last_nl + 1)) {
           outp = yyp; refill_buffer(); yyp = outp;
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
                outp = --yyp;   /* some operating systems give EOF only once */

                for (i = curchunk; i > startchunk; i--)
                    add_input(array_line[i]);
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
            for (i = startchunk; i >= 0; i--)
                add_input(array_line[i]);

            res = 1;
            break;
        } else {
            /*
             * only report end of file in array block, if not an include file
             */
            if (c == LEX_EOF && inctop == 0) {
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
            if (len + termlen + 5 > MAXCHUNK) {
                if (curchunk == NUMCHUNKS - 1) {
                    res = -2;
                    outp = yyp;
                    break;
                }
                array_line[++curchunk] =
                    (char *) DXALLOC(MAXCHUNK, TAG_COMPILER, "array_block");
                len = 0;
            }
            /*
             * header
             */
            array_line[curchunk][len++] = '"';
            array_line[curchunk][len++] = ',';
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
    for (i = curchunk; i >= 0; i--)
        FREE(array_line[i]);

    return res;
}

static int
get_text_block (char * term)
{
    int termlen;                /* length of terminator */
    char *text_line[NUMCHUNKS]; /* allocate memory in chunks */
    int len;                    /* position in chunk */
    int startpos, startchunk;   /* start of line */
    int curchunk, res;          /* current chunk; this function's result */
    int i;                      /* an index counter */
    unsigned char c;            /* a char */
    register char *yyp = outp;

    /*
     * initialize
     */
    termlen = strlen(term);
    text_line[0] = (char *) DXALLOC(MAXCHUNK, TAG_COMPILER, "text_block");
    text_line[0][0] = '"';
    text_line[0][1] = '\0';
    len = 1;
    startpos = 1;
    startchunk = 0;
    curchunk = 0;
    res = 0;

    while (1) {
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
                outp = --yyp;   /* some operating systems give EOF only once */

                for (i = curchunk; i > startchunk; i--) {
                    /* Ick.  go back and unprotect " and \ */
                    p = text_line[i];
                    while (*p && *p != '\\')
                        p++;
                    if (*p) {
                        q = p++;
                        do {
                            *q++ = *p++;
                            if (*p == '\\') p++;
                        } while (*p);
                        *q = 0;
                    }

                    add_input(text_line[i]);
                }
                p = text_line[startchunk] + startpos + termlen;
                while (*p && *p != '\\')
                    p++;
                if (*p) {
                    q = p++;
                    do {
                        *q++ = *p++;
                        if (*p == '\\') p++;
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

            for (i = startchunk; i >= 0; i--)
                add_input(text_line[i]);

            res = 1;
            break;
        } else {
            /*
             * only report end of file in text block, if not an include file
             */
            if (c == LEX_EOF && inctop == 0) {
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
                    (char *) DXALLOC(MAXCHUNK, TAG_COMPILER, "text_block");
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
    for (i = curchunk; i >= 0; i--)
        FREE(text_line[i]);

    return res;
}

static void skip_line()
{
    unsigned char c;
    register char *yyp = outp;

    while (((c = *yyp++) != '\n') && (c != LEX_EOF));

    /* Next read of this '\n' will do refill_buffer() if neccesary */
    if (c == '\n') yyp--;
    outp = yyp;
}

static void skip_comment()
{
    unsigned char c = '*';
    register char *yyp = outp;

    for (;;) {
        while ((c = *yyp++) != '*') {
            if (c == LEX_EOF) {
                outp = --yyp;
                lexerror("End of file in a comment");
                return;
            }
            if (c == '\n') {
                nexpands = 0;
                current_line++;
                if (yyp == last_nl + 1) {
                    outp = yyp;
                    refill_buffer();
                    yyp = outp;
                }
            }
        }
        if (*(yyp - 2) == '/')
            yywarn("/* found in comment.");
        do {
            if ((c = *yyp++) == '/') {
                outp = yyp;
                return;
            }
            if (c == '\n') {
                nexpands = 0;
                current_line++;
                if (yyp == last_nl + 1) {
                    outp = yyp;
                    refill_buffer();
                    yyp = outp;
                }
            }
        } while (c == '*');
    }
}

static void
deltrail (char * sp)
{
    char *p;

    p = sp;
    if (!*p) {
        lexerror("Illegal # command");
    } else {
        while (*p && !uisspace(*p))
            p++;
        *p = 0;
    }
}

#define SAVEC \
    if (yyp < yytext+MAXLINE-5)\
       *yyp++ = c;\
    else {\
       lexerror("Line too long");\
       break;\
    }

typedef struct {
    const char *name;
    int value;
} pragma_t;

static pragma_t our_pragmas[] = {
    { "strict_types", PRAGMA_STRICT_TYPES },
    { "save_types", PRAGMA_SAVE_TYPES },
    { "warnings", PRAGMA_WARNINGS },
    { "optimize", PRAGMA_OPTIMIZE },
    { "show_error_context", PRAGMA_ERROR_CONTEXT },
    { 0, 0 }
};

static void handle_pragma (char * str)
{
    int i;
    int no_flag;

    if (strncmp(str, "no_", 3) == 0) {
        str += 3;
        no_flag = 1;
    } else
        no_flag = 0;

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
    register char *yyp, *yyp2;
    int len;

    if ((unsigned char)outp[-1] == LEX_EOF) {
        strcpy(buf, " at the end of the file\n");
        return buf;
    }

    if (yychar == -1) strcpy(buf, " around ");
    else strcpy(buf, " before ");
    yyp = outp;
    yyp2 = sub_context;
    len = 20;
    while (len--) {
        if (*yyp == '\n') {
            if (len == 19) strcat(buf, "the end of line");
            break;
        } else if ((unsigned char)*yyp == LEX_EOF) {
            if (len == 19) strcat(buf, "the end of file");
            break;
        }
        *yyp2++ = *yyp++;
    }
    *yyp2 = 0;
    if (yyp2 != sub_context) strcat(buf, sub_context);
    strcat(buf, "\n");
    return buf;
}

#ifdef WIN32
int correct_read(int handle, char *buf, unsigned int count)
{
    unsigned int tmp,size=0;

    do {
        tmp=read(handle,buf,count);
        if (tmp < 0) return tmp;
        if (tmp == 0) return size;
        size+=tmp;
        count-=tmp;
        buf+=tmp;
    } while (count>0);
    return size;
}
#else
#define correct_read read
#endif

static void refill_buffer() {
    if (cur_lbuf != &head_lbuf) {
        if (outp >= cur_lbuf->buf_end &&
            cur_lbuf->term_type == TERM_ADD_INPUT) {
            /* In this case it cur_lbuf cannot have been
               allocated due to #include */
            linked_buf_t *prev_lbuf = cur_lbuf->prev;

            FREE(cur_lbuf);
            cur_lbuf = prev_lbuf;
            outp = cur_lbuf->outp;
            last_nl = cur_lbuf->last_nl;
            if (cur_lbuf->term_type == TERM_ADD_INPUT
                || (outp != last_nl + 1))
                return;
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
            }
            else {
                /* No more space at the end */
                size = cur_lbuf->buf_end - outp + 1;  /* Include newline */
                memcpy(outp - MAXLINE - 1, outp - 1, size);
                outp -= MAXLINE;
                p = outp + size - 1;
            }

            size = correct_read(yyin_desc, p, MAXLINE);
            cur_lbuf->buf_end = p += size;
            if (size < MAXLINE) {
            	*(last_nl = p) = LEX_EOF;
            	if(*(last_nl-1) != '\n'){
            		if(size +1 > MAXLINE)
            			yyerror("No newline at end of file.");
            		*p++ = '\n';
            		*(last_nl = p) = LEX_EOF;
            	}
            	return;
			}
            while (*--p != '\n');
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
            } else {    /* No space, need to allocate new buffer */
                linked_buf_t *new_lbuf;
                char *new_outp;

                if (!(new_lbuf = ALLOCATE(linked_buf_t, TAG_COMPILER, "refill_bufer"))) {
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

            size = correct_read(yyin_desc, p, MAXLINE);
            end = p += size;
            if (flag) cur_lbuf->buf_end = p;
            if (size < MAXLINE) {
            	*(last_nl = p) = LEX_EOF;
            	if(*(last_nl-1) != '\n'){
            		if(size +1 > MAXLINE)
            			yyerror("No newline at end of file.");
            		*p++ = '\n';
            		*(last_nl = p) = LEX_EOF;
            	}
            	return;
            }
            while (*--p != '\n');
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

static int function_flag = 0;

INLINE void push_function_context() {
    function_context_t *fc;
    parse_node_t *node;

    if (last_function_context == MAX_FUNCTION_DEPTH - 1) {
        yyerror("Function pointers nested too deep.");
        return;
    }
    fc = &function_context_stack[++last_function_context];
    fc->num_parameters = 0;
    fc->num_locals = 0;
    node = new_node_no_line();
    node->l.expr = node;
    node->r.expr = 0;
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

#define return_assign(opcode) { yylval.number = opcode; return L_ASSIGN; }
#define return_order(opcode) { yylval.number = opcode; return L_ORDER; }

/* To halt execution when a #breakpoint line is encountered, set a breakpoint
   at this function */
static void lex_breakpoint() {
}

int yylex()
{
    static char partial[MAXLINE + 5];   /* extra 5 for safety buffer */
    static char terminator[MAXLINE + 5];
    int is_float;
    double myreal;
    char *partp;

    register char *yyp;
    unsigned char c;

    yytext[0] = 0;

    partp = partial;
    partial[0] = 0;

    for (;;) {
        if (lex_fatal) {
            return -1;
        }
        switch (c = *outp++) {
        case LEX_EOF:
            if (inctop) {
                incstate_t *p;

                p = inctop;
                close(yyin_desc);
                save_file_info(current_file_id, current_line - current_line_saved);
                current_line_saved = p->line - 1;
                /* add the lines from this file, and readjust to be relative
                   to the file we're returning to */
                current_line_base += current_line - current_line_saved;
                free_string(current_file);
                nexpands = 0;
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

                yyin_desc = p->yyin_desc;
                last_nl = p->last_nl;
                outp = p->outp;
                inctop = p->next;
                incnum--;
                FREE((char *) p);
                outp[-1] = '\n';
                if (outp == last_nl + 1) refill_buffer();
                break;
            }
            if (iftop) {
                ifstate_t *p = iftop;

                yyerror(p->state == EXPECT_ENDIF ? "Missing #endif" : "Missing #else/#elif");
                while (iftop) {
                    p = iftop;
                    iftop = p->next;
                    FREE((char *) p);
                }
            }
            outp--;
            return -1;
        case '\r':
	    yywarn("^M");
	    break;
	case '\t':
#ifdef WARN_TAB
	    yywarn("<TAB>");
#endif
	    break;
        case '\n':
	    nexpands = 0;
	    current_line++;
	    total_lines++;
	    if (outp == last_nl + 1)
	      refill_buffer();
        case ' ':
        case '\f':
        case '\v':
            break;
        case '+':
        {
            switch(*outp++) {
                case '+': return L_INC;
                case '=': return_assign(F_ADD_EQ);
                default: outp--; return '+';
            }
        }
        case '-':
        {
            switch(*outp++) {
                case '>': return L_ARROW;
                case '-': return L_DEC;
                case '=': return_assign(F_SUB_EQ);
                default: outp--; return '-';
            }
        }
        case '&':
        {
            switch(*outp++) {
                case '&': return L_LAND;
                case '=': return_assign(F_AND_EQ);
                default: outp--; return '&';
            }
        }
        case '|':
        {
            switch(*outp++) {
                case '|': return L_LOR;
                case '=': return_assign(F_OR_EQ);
                default: outp--; return '|';
            }
        }
        case '^':
        {
            if (*outp++ == '=') return_assign(F_XOR_EQ);
            outp--;
            return '^';
        }
        case '<':
        {
            switch(*outp++) {
                case '<':
                {
                    if (*outp++ == '=') return_assign(F_LSH_EQ);
                    outp--;
                    return L_LSH;
                }
                case '=': return_order(F_LE);
                default: outp--; return '<';
            }
        }
        case '>':
        {
            switch(*outp++) {
                case '>':
                {
                    if (*outp++ == '=') return_assign(F_RSH_EQ);
                    outp--;
                    return L_RSH;
                }
                case '=': return_order(F_GE);
                default: outp--; return_order(F_GT);
            }
        }
        case '*':
        {
            if (*outp++ == '=') return_assign(F_MULT_EQ);
            outp--;
            return '*';
        }
        case '%':
        {
            if (*outp++ == '=') return_assign(F_MOD_EQ);
            outp--;
            return '%';
        }
        case '/':
            switch(*outp++) {
                case '*': skip_comment(); break;
                case '/': skip_line(); break;
                case '=': return_assign(F_DIV_EQ);
                default: outp--; return '/';
            }
            break;
        case '=':
            if (*outp++ == '=') return L_EQ;
            outp--;
            yylval.number = F_ASSIGN;
            return L_ASSIGN;
        case '(':
            yyp = outp;
#ifdef WOMBLES
            c = *yyp++;
#else
            while (isspace(c = *yyp++)) {
                if (c == '\n') {
                    current_line++;
                    if (yyp == last_nl + 1) {
                        outp = yyp;
                        refill_buffer();
                        yyp = outp;
                    }
                }
            }
#endif
            switch(c) {
                case '{' : { outp = yyp; return L_ARRAY_OPEN; }
                case '[' : { outp = yyp; return L_MAPPING_OPEN; }
                case ':' :
                {
                    if ((c = *yyp++) == ':') {
                        outp = yyp -= 2;
                        return '(';
                    } else {
                        while (isspace(c)) {
                            if (c == '\n') {
                                if ((yyp == last_nl + 1)) {
                                    outp = yyp;
                                    refill_buffer();
                                    yyp = outp;
                                }
                                current_line++;
                            }
                            c = *yyp++;
                        }

                        outp = yyp;

                        if (isalpha(c) || c == '_') {
                            function_flag = 1;
                            goto parse_identifier;
                        }

                        outp--;
                        push_function_context();
                        return L_FUNCTION_OPEN;
                    }

                }
                default:
                {
                    outp = yyp - 1;
                    return '(';
                }
            }

        case '$':
            if (!current_function_context) {
                yyerror("$var illegal outside of function pointer.");
                return '$';
            }
            if (current_function_context->num_parameters == -2) {
                yyerror("$var illegal inside anonymous function pointer.");
                return '$';
            } else {
                if (!isdigit(c = *outp++)) {
                    outp--;
                    return '$';
                }
                yyp = yytext;
                SAVEC;
                for (;;) {
                    if (!isdigit(c = *outp++)) break;
                    SAVEC;
                }
                outp--;
                *yyp = 0;
                yylval.number = atol(yytext) - 1;
                if (yylval.number < 0)
                    yyerror("In function parameter $num, num must be >= 1.");
                else if (yylval.number > 254)
                    yyerror("only 255 parameters allowed.");
                else if (yylval.number >= current_function_context->num_parameters)
                    current_function_context->num_parameters = yylval.number + 1;
                return L_PARAMETER;
            }
        case ')':
        case '{':
        case '}':
        case '[':
        case ']':
        case ';':
        case ',':
        case '~':
#ifndef USE_TRIGRAPHS
        case '?':
            return c;
#else
            return c;
        /*
         * You're probably asking, what the heck are trigraphs?
         * The character set of C source is contained within seven-bit
         * ASCII, a superset of ISO 646-1983 Invariant Code Set;
         * to allow programs to be represented in the reduced set,
         * certain single characters are replaced by a corresponding
         * trigraph (3 character) sequence.  These are:
         *     ??=   #       ??(   [       ??<   {
         *     ??/   \       ??)   ]       ??>   }
         *     ??'   ^       ??!   |       ??-   ~
         */
        case '?':
            if (*outp++ != '?') { outp--; return '?'; }
            switch (*outp++) {
                case '=':  return '#';
                case '/':  return '\\';
                case '\'': return '^';
                case '(':  return '[';
                case ')':  return ']';
                case '!':  return '|';
                case '<':  return '{';
                case '>':  return '}';
                case '-':  return '~';
                default:
                    outp -= 2;
                    return '?';
            }
#endif
        case '!':
            if (*outp++ == '=') return L_NE;
            outp--;
            return L_NOT;
        case ':':
            if (*outp++ == ':') return L_COLON_COLON;
            outp--;
            return ':';
        case '.':
            if (*outp++ == '.') {
                if (*outp++ == '.')
                    return L_DOT_DOT_DOT;
                outp--;
                return L_RANGE;
            }
            outp--;
            goto badlex;
        case '#':
            if (*(outp - 2) == '\n') {
                char *sp = 0;
                int quote;

                while (is_wspace(c = *outp++));

                yyp = yytext;

                for (quote = 0;;) {

                    if (c == '"')
                        quote ^= 1;
                    else if (c == '/' && !quote) {
                        if (*outp == '*') {
                            outp++;
                            skip_comment();
                            c = *outp++;
                        } else
                        if (*outp == '/') {
                            outp++;
                            skip_line();
                            c = *outp++;
                        }
                    }
                    if (!sp && isspace(c))
                        sp = yyp;
                    if (c == '\n' || c == LEX_EOF) break;
                    SAVEC;
                    c = *outp++;
                }
                if (sp) {
                    *sp++ = 0;
                    while (uisspace(*sp))
                        sp++;
                } else {
                    sp = yyp;
                }
                *yyp = 0;
                if (!strcmp("include", yytext)) {
                    current_line++;
                    if (c == LEX_EOF) {
                        *(last_nl = --outp) = LEX_EOF;
                        outp[-1] = '\n';
                    }
                    handle_include(sp, 0);
                    break;
                } else {
                    if (outp == last_nl + 1) refill_buffer();

                    if (strcmp("define", yytext) == 0) {
                        handle_define(sp);
                    } else if (strcmp("if", yytext) == 0) {
                        int cond;

                        *--outp = '\0';
                        add_input(sp);
                        cond = cond_get_exp(0);
                        if (*outp++) {
                            yyerror("Condition too complex in #if");
                            while (*outp++);
                        } else
                            handle_cond(cond);
                    } else if (strcmp("ifdef", yytext) == 0) {
                        deltrail(sp);
                        handle_cond(lookup_define(sp) != 0);
                    } else if (strcmp("ifndef", yytext) == 0) {
                        deltrail(sp);
                        handle_cond(lookup_define(sp) == 0);
                    } else if (strcmp("elif", yytext) == 0) {
                        handle_elif(sp);
                    } else if (strcmp("else", yytext) == 0) {
                        handle_else();
                    } else if (strcmp("endif", yytext) == 0) {
                        handle_endif();
                    } else if (strcmp("undef", yytext) == 0) {
                        defn_t *d;

                        deltrail(sp);
                        if ((d = lookup_define(sp))) {
                            if (d->flags & DEF_IS_PREDEF)
                                yyerror("Illegal to #undef a predefined value.");
                            else
                                d->flags |= DEF_IS_UNDEFINED;
                        }
                    } else if (strcmp("echo", yytext) == 0) {
                        debug_message("%s\n", sp);
                    } else if (strcmp("error", yytext) == 0) {
                        char buf[MAXLINE+1];
                        strcpy(buf, yytext);
                        strcat(buf, "\n");
                        yyerror(buf);
                    } else if (strcmp("warn", yytext) == 0) {
                        char buf[MAXLINE+1];
                        strcpy(buf, yytext);
                        strcat(buf, "\n");
                        yywarn(buf);
                    } else if (strcmp("pragma", yytext) == 0) {
                        handle_pragma(sp);
                    } else if (strcmp("breakpoint", yytext) == 0) {
                        lex_breakpoint();
                    } else {
                        yyerror("Unrecognised # directive");
                    }
                    *--outp = '\n';
                    break;
                }
            } else {
#ifdef COMPAT_32
                if (*outp == '\'') {
                    outp++;
                    return L_LAMBDA;
                }
#endif
                goto badlex;
            }
        case '\'':

            if (*outp++ == '\\') {
                switch(*outp++) {
                case 'n': yylval.number = '\n'; break;
                case 't': yylval.number = '\t'; break;
                case 'r': yylval.number = '\r'; break;
                case 'b': yylval.number = '\b'; break;
                case 'a': yylval.number = '\x07'; break;
                case 'e': yylval.number = '\x1b'; break;
                case '\'': yylval.number = '\''; break;
                case '"': yylval.number = '"'; break;
                case '\\': yylval.number = '\\'; break;
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                    outp--;
                    yylval.number = strtol(outp, &outp, 8);
                    if (yylval.number > 255) {
                        yywarn("Illegal character constant.");
                        yylval.number = 'x';
                    }
                    break;
                case 'x':
                    if (!isxdigit((unsigned char)*outp)) {
                        yylval.number = 'x';
                        yywarn("\\x must be followed by a valid hex value; interpreting as 'x' instead.");
                    } else {
                        yylval.number = strtol(outp, &outp, 16);
                        if (yylval.number > 255) {
                            yywarn("Illegal character constant.");
                            yylval.number = 'x';
                        }
                    }
                    break;
                case '\n':
                    yylval.number = '\n';
                    current_line++;
                    total_lines++;
                    if ((outp = last_nl + 1))
                        refill_buffer();
                    break;
                default:
                    yywarn("Unknown \\ escape.");
                    yylval.number = *(outp - 1);
                    break;
                }
            } else {
                yylval.number = *(outp - 1);
            }

            if (*outp++ != '\'') {
                outp--;
                yyerror("Illegal character constant");
                yylval.number = 0;
            }
            return L_NUMBER;
        case '@':
            {
                int rc;
                int tmp;

                if ((tmp = *outp++) != '@') {
                    /* check for Robocoder's @@ block */
                    outp--;
                }
                if (!get_terminator(terminator)) {
                    lexerror("Illegal terminator");
                    break;
                }
                if (tmp == '@') {
                    rc = get_array_block(terminator);

                    if (rc > 0) {
                        /* outp is pointing at "({" for histortical reasons */
                        outp += 2;
                        return L_ARRAY_OPEN;
                    } else if (rc == -1) {
                        lexerror("End of file in array block");
                        return LEX_EOF;
                    } else {    /* if rc == -2 */
                        yyerror("Array block exceeded maximum length");
                    }
                } else {
                    rc = get_text_block(terminator);

                    if (rc > 0) {
                        int n;

                        /*
                         * make string token and clean up
                         */
                        yylval.string = scratch_copy_string(outp);

                        n = strlen(outp) + 1;
                        outp += n;

                        return L_STRING;
                    } else if (rc == -1) {
                        lexerror("End of file in text block");
                        return LEX_EOF;
                    } else {    /* if (rc == -2) */
                        yyerror("Text block exceeded maximum length");
                    }
                }
            }
            break;
        case '"':
            {
                int l;
                register unsigned char *to = scr_tail + 1;

                if ((l = scratch_end - to) > 255) l = 255;
                while (l--) {
                    switch(c = *outp++) {
                    case LEX_EOF:
                        lexerror("End of file in string");
                        return LEX_EOF;

                    case '"':
                        *to++ = 0;
                        if (!l && (to == scratch_end)) {
                            char *res = scratch_large_alloc(to - scr_tail - 1);
                            strcpy(res, (char *) (scr_tail + 1));
                            yylval.string = res;
                            return L_STRING;
                        }

                        scr_last = scr_tail + 1;
                        scr_tail = to;
                        *to = to - scr_last;
                        yylval.string = (char *) scr_last;
                        return L_STRING;

                    case '\n':
                        current_line++;
                        total_lines++;
                        if (outp == last_nl + 1) refill_buffer();
                        *to++ = '\n';
                        break;

                    case '\\':
                        /* Don't copy the \ in yet */
                        switch((unsigned char)*outp++) {
                        case '\n':
                            current_line++;
                            total_lines++;
                            if (outp == last_nl + 1) refill_buffer();
                            l++; /* Nothing is copied */
                            break;
                        case LEX_EOF:
                            lexerror("End of file in string");
                            return LEX_EOF;
                        case 'n': *to++ = '\n'; break;
                        case 't': *to++ = '\t'; break;
                        case 'r': *to++ = '\r'; break;
                        case 'b': *to++ = '\b'; break;
                        case 'a': *to++ = '\x07'; break;
                        case 'e': *to++ = '\x1b'; break;
                        case '"': *to++ = '"'; break;
                        case '\\': *to++ = '\\'; break;
                        case '0': case '1': case '2': case '3': case '4':
                        case '5': case '6': case '7': case '8': case '9':
                        {
                            int tmp;
                            outp--;
                            tmp = strtol(outp, &outp, 8);
                            if (tmp > 255) {
                                yywarn("Illegal character constant in string.");
                                tmp = 'x';
                            }
                            *to++ = tmp;
                            break;
                        }
                        case 'x':
                        {
                            int tmp;
                            if (!isxdigit((unsigned char)*outp)) {
                                *to++ = 'x';
                                yywarn("\\x must be followed by a valid hex value; interpreting as 'x' instead.");
                            } else {
                                tmp = strtol(outp, &outp, 16);
                                if (tmp > 255) {
                                    yywarn("Illegal character constant.");
                                    tmp = 'x';
                                }
                                *to++ = tmp;
                            }
                            break;
                        }
                        default:
                            *to++ = *(outp - 1);
                            yywarn("Unknown \\ escape.");
                        }
                        break;
                    default:
                        *to++ = c;
                    }
                }

                /* Not enough space, we now copy the rest into yytext */
                l = MAXLINE - (to - scr_tail);

                yyp = yytext;
                while (l--) {
                    switch(c = *outp++) {
                        case LEX_EOF:
                            lexerror("End of file in string");
                            return LEX_EOF;

                        case '"':
                        {
                            char *res;
                            *yyp++ = '\0';
                            res = scratch_large_alloc((yyp - yytext) + (to - scr_tail) - 1);
                            strncpy(res, (char *) (scr_tail + 1), (to - scr_tail) - 1);
                            strcpy(res + (to - scr_tail) - 1, yytext);
                            yylval.string = res;
                            return L_STRING;
                        }

                        case '\n':
                            current_line++;
                            total_lines++;
                            if (outp == last_nl + 1) refill_buffer();
                            *yyp++ = '\n';
                            break;

                       case '\\':
                           /* Don't copy the \ in yet */
                           switch((unsigned char)*outp++) {
                           case '\n':
                               current_line++;
                               total_lines++;
                               if (outp == last_nl + 1) refill_buffer();
                               l++; /* Nothing is copied */
                               break;
                           case LEX_EOF:
                               lexerror("End of file in string");
                               return LEX_EOF;
                           case 'n': *yyp++ = '\n'; break;
                           case 't': *yyp++ = '\t'; break;
                           case 'r': *yyp++ = '\r'; break;
                           case 'b': *yyp++ = '\b'; break;
                           case 'a': *yyp++ = '\x07'; break;
                           case 'e': *yyp++ = '\x1b'; break;
                           case '"': *yyp++ = '"'; break;
                           case '\\': *yyp++ = '\\'; break;
                           case '0': case '1': case '2': case '3': case '4':
                           case '5': case '6': case '7': case '8': case '9':
                           {
                               int tmp;
                               outp--;
                               tmp = strtol(outp, &outp, 8);
                               if (tmp > 255) {
                                   yywarn("Illegal character constant in string.");
                                   tmp = 'x';
                               }
                               *yyp++ = tmp;
                               break;
                           }
                           case 'x':
                           {
                               int tmp;
                               if (!isxdigit((unsigned char)*outp)) {
                                   *yyp++ = 'x';
                                   yywarn("\\x must be followed by a valid hex value; interpreting as 'x' instead.");
                               } else {
                                   tmp = strtol(outp, &outp, 16);
                                   if (tmp > 255) {
                                       yywarn("Illegal character constant.");
                                       tmp = 'x';
                                   }
                                   *yyp++ = tmp;
                               }
                               break;
                           }
                           default:
                               *yyp++ = *(outp - 1);
                               yywarn("Unknown \\ escape.");
                           }
                           break;

                    default: *yyp++ = c;
                    }
                }

                /* Not even enough length, declare too long string error */
                lexerror("String too long");
                *yyp++ = '\0';
                {
                    char *res;
                    res = scratch_large_alloc((yyp - yytext) + (to - scr_tail) - 1);
                    strncpy(res, (char *) (scr_tail + 1), (to - scr_tail) - 1);
                    strcpy(res + (to - scr_tail) - 1, yytext);
                    yylval.string = res;
                    return L_STRING;
                }
            }
        case '0':
            c = *outp++;
            if (c == 'X' || c == 'x') {
                yyp = yytext;
                for (;;) {
                    c = *outp++;
                    SAVEC;
                    if (!isxdigit((unsigned char)c))
                        break;
                }
                outp--;
                yylval.number = strtol(yytext, (char **) NULL, 0x10);
                return L_NUMBER;
            }
            outp--;
            c = '0';
            /* fall through */
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            is_float = 0;
            yyp = yytext;
            *yyp++ = c;
            for (;;) {
                c = *outp++;
                if (c == '.') {
                    if (!is_float) {
                        is_float = 1;
                    } else {
                        is_float = 0;
                        outp--;
                        break;
                    }
                } else if (!isdigit((unsigned char)c))
                    break;
                SAVEC;
            }
            outp--;
            *yyp = 0;
            if (is_float) {
                sscanf(yytext, "%lf", &myreal);
                yylval.real = myreal;
                return L_REAL;
            } else {
                yylval.number = atol(yytext);
                return L_NUMBER;
            }
        default:
            if (isalpha((unsigned char)c) || c == '_') {
                int r;

parse_identifier:
                yyp = yytext;
                *yyp++ = c;
                for (;;) {
                    c = *outp++;
                    if (!isalnum((unsigned char)c) && (c != '_'))
                        break;
                    SAVEC;
                }
                *yyp = 0;
                if (c == '#') {
                    if (*outp++ != '#')
                        lexerror("Single '#' in identifier -- use '##' for token pasting");
                    outp -= 2;
                    if (!expand_define()) {
                        if (partp + (r = strlen(yytext)) + (function_flag ? 3 : 0) - partial > MAXLINE)
                            lexerror("Pasted token is too long");
                        if (function_flag) {
                            strcpy(partp, "(: ");
                            partp += 3;
                        }
                        strcpy(partp, yytext);
                        partp += r;
                        outp += 2;
                    }
                } else if (partp != partial) {
                    outp--;
                    if (!expand_define())
                        add_input(yytext);
                    while ((c = *outp++) == ' ');
                    outp--;
                    add_input(partial);
                    partp = partial;
                    partial[0] = 0;
                } else {
                    outp--;
                    if (!expand_define()) {
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

                                function_flag = 0;
                                while ((c = *outp++) == ' ');
                                outp--;
                                /* Note that this gets code like:
                                 * #define x :)
                                 * #define y ,
                                 * function f = (: foo x;
                                 * function g = (: bar y 1 :);
                                 *
                                 * wrong.  But that is almost pathological.
                                 */
                                if (c != ':' && c != ',')
                                    return old_func();
                                if ((val=ihe->dn.local_num) >= 0) {
                                    if (c == ',') return old_func();
                                    yylval.number = (val << 8) | FP_L_VAR;
                                } else if ((val=ihe->dn.global_num) >= 0) {
                                    if (c == ',') return old_func();
                                    yylval.number = (val << 8) | FP_G_VAR;
                                } else if ((val=ihe->dn.function_num) >=0) {
                                    yylval.number = (val << 8)|FP_LOCAL;
                                } else if ((val=ihe->dn.simul_num) >=0) {
                                    yylval.number = (val << 8)|FP_SIMUL;
                                } else if ((val=ihe->dn.efun_num) >=0) {
                                    yylval.number = (val << 8)|FP_EFUN;
                                } else return old_func();
                                return L_NEW_FUNCTION_OPEN;
                            }
                            yylval.ihe = ihe;
                            return L_DEFINED_NAME;
                        }
                        yylval.string = scratch_copy(yytext);
                        return L_IDENTIFIER;
                    }
                    if (function_flag) {
                        function_flag = 0;
                        add_input("(:");
                    }
                }
                break;
            }
            goto badlex;
        }
    }
  badlex:
    {
#ifdef DEBUG
        char buff[100];

        sprintf(buff, "Illegal character (hex %02x) '%c'", (unsigned) c, (char) c);
        yyerror(buff);
#endif
        return ' ';
    }
}

extern YYSTYPE yylval;

void end_new_file()
{
    while (inctop) {
        incstate_t *p;

        p = inctop;
        close(yyin_desc);
        free_string(current_file);
        current_file = p->file;
        yyin_desc = p->yyin_desc;
        inctop = p->next;
        FREE((char *) p);
    }
    inctop = 0;
    while (iftop) {
        ifstate_t *p;

        p = iftop;
        iftop = p->next;
        FREE((char *) p);
    }
    if (defines_need_freed) {
        free_defines();
        defines_need_freed = 0;
    }
    if (cur_lbuf != &head_lbuf) {
        linked_buf_t *prev_lbuf;

        while (cur_lbuf != &head_lbuf) {
            prev_lbuf = cur_lbuf->prev;
            FREE ((char *) cur_lbuf);
            cur_lbuf = prev_lbuf;
        }
    }
}

static void add_quoted_predefine (const char * def, const char * val)
{
    char save_buf[MAXLINE];

    strcpy(save_buf, "\"");
    strcat(save_buf, val);
    strcat(save_buf, "\"");
    add_predefine(def, -1, save_buf);
}

void add_predefines()
{
    char save_buf[80];
    int i;
    lpc_predef_t *tmpf;

    add_predefine("MUDOS", -1, "");
    add_predefine("FLUFFOS", -1, "");
#ifdef WIN32
    add_predefine("__WIN32__", -1, "");
#endif
#ifdef RUSAGE
    add_predefine("__HAS_RUSAGE__", -1, "");
#endif
#ifdef M64
    add_predefine("__M64__", -1, "");
#endif
#ifdef PACKAGE_DB
    add_predefine("__PACKAGE_DB__", -1, "");
#endif
#ifdef GET_CHAR_IS_BUFFERED
    add_predefine("__GET_CHAR_IS_BUFFERED__", -1, "");
#endif
#ifdef PACKAGE_DSLIB
    add_predefine("__DSLIB__", -1, "");
#endif
#ifdef PACKAGE_DWLIB
    add_predefine("__DWLIB__", -1, "");
#endif
#ifdef FD_SETSIZE
    sprintf(save_buf, "%d", FD_SETSIZE);
#else
    sprintf(save_buf, "%d", 64);
#endif
    add_predefine("__FD_SETSIZE__", -1, save_buf);
    get_version(save_buf);
    add_quoted_predefine("__VERSION__", save_buf);
    sprintf(save_buf, "%d", external_port[0].port);
    add_predefine("__PORT__", -1, save_buf);
    for (i = 0; i < 2 * NUM_OPTION_DEFS; i += 2) {
        add_predefine(option_defs[i], -1, option_defs[i+1]);
    }
    add_quoted_predefine("__ARCH__", ARCH);
    add_quoted_predefine("__COMPILER__", COMPILER);
    add_quoted_predefine("__OPTIMIZATION__", OPTIMIZE);

    /* Backwards Compat */
#ifndef CDLIB
    add_quoted_predefine("MUD_NAME", MUD_NAME);
#endif
#ifdef F_ED
    add_predefine("HAS_ED", -1, "");
#endif
#ifdef F_PRINTF
    add_predefine("HAS_PRINTF", -1, "");
#endif
#if (defined(RUSAGE) || defined(GET_PROCESS_STATS) || defined(TIMES))
    add_predefine("HAS_RUSAGE", -1, "");
#endif
#ifdef DEBUG_MACRO
    add_predefine("HAS_DEBUG_LEVEL", -1, "");
#endif
    for (tmpf = lpc_predefs; tmpf; tmpf = tmpf->next) {
        char namebuf[NSIZE];
        char mtext[MLEN];

        *mtext = '\0';
        sscanf(tmpf->flag, "%[^=]=%[ -~=]", namebuf, mtext);
        if (strlen(namebuf) >= NSIZE)
            fatal("NSIZE exceeded");
        if (strlen(mtext) >= MLEN)
            fatal("MLEN exceeded");
        add_predefine(namebuf, -1, mtext);
    }
    sprintf(save_buf, "%ld", sizeof(long));
    add_predefine("SIZEOFINT", -1, save_buf);    
    sprintf(save_buf, "%ld", LONG_MAX);
    add_predefine("MAX_INT", -1, save_buf);
}

void start_new_file (int f)
{
    if (defines_need_freed) {
        free_defines();
    }
    defines_need_freed = 1;
    if (current_file) {
        char *dir;
        char *tmp;
        int ln;

        ln = strlen(current_file);
        dir = (char *) DMALLOC(ln + 4, TAG_COMPILER, "start_new_file");
        dir[0] = '"';
        dir[1] = '/';
        memcpy(dir + 2, current_file, ln);
        dir[ln + 2] = '"';
        dir[ln + 3] = 0;
        add_define("__FILE__", -1, dir);
        tmp = strrchr(dir, '/');
        tmp[1] = '"';
        tmp[2] = 0;
        add_define("__DIR__", -1, dir);
        FREE(dir);
    }
    yyin_desc = f;
    lex_fatal = 0;
    last_function_context = -1;
    current_function_context = 0;
    cur_lbuf = &head_lbuf;
    cur_lbuf->outp = cur_lbuf->buf_end = outp = cur_lbuf->buf + (DEFMAX >> 1);
    *(last_nl = outp - 1) = '\n';
    pragmas = DEFAULT_PRAGMAS;
    nexpands = 0;
    incnum = 0;
    current_line = 1;
    current_line_base = 0;
    current_line_saved = 0;
    function_flag = 0;
    if (*GLOBAL_INCLUDE_FILE) {
        char *gifile;

        /* need a writable copy */
        gifile = alloc_cstring(GLOBAL_INCLUDE_FILE, "global include");
        handle_include(gifile, 1);
        FREE(gifile);
    } else refill_buffer();
}

const char *query_instr_name (int instr)
{
    const char *name;
    static char num_buf[20];

    name = instrs[instr].name;

    if (name) {
        if (name[0] == '_') name++;
        return name;
    } else {
        sprintf(num_buf, "efun%d", instr);
        return num_buf;
    }
}

#define add_instr_name(w, x, y, z) int_add_instr_name(w, y, z)

static void int_add_instr_name (const char * name, int n, short t)
{
    instrs[n].name = name;
    instrs[n].ret_type = t;
}

static void init_instrs()
{
    int i, n;

    for (i = 0; i < BASE; i++) {
        instrs[i].ret_type = -1;
    }
    for (i = 0; i < NELEM(predefs); i++) {
        n = predefs[i].token;
        if (n & F_ALIAS_FLAG) {
            predefs[i].token ^= F_ALIAS_FLAG;
        } else {
            instrs[n].min_arg = predefs[i].min_args;
            instrs[n].max_arg = predefs[i].max_args;
            instrs[n].name = predefs[i].word;
            instrs[n].type[0] = predefs[i].arg_type1;
            instrs[n].type[1] = predefs[i].arg_type2;
            instrs[n].type[2] = predefs[i].arg_type3;
            instrs[n].type[3] = predefs[i].arg_type4;
            instrs[n].Default = predefs[i].Default;
            instrs[n].ret_type = predefs[i].ret_type;
            instrs[n].arg_index = predefs[i].arg_index;
        }
    }
    /*
     * eoperators have a return type now.  T_* is used instead of TYPE_*
     * since operators can return multiple types.
     */
#ifdef NO_BUFFER_TYPE
#define OR_BUFFER
#else
#define OR_BUFFER | T_BUFFER
#endif

    add_instr_name("<", "c_lt();\n", F_LT, T_NUMBER);
    add_instr_name(">", "c_gt();\n", F_GT, T_NUMBER);
    add_instr_name("<=", "c_le();\n", F_LE, T_NUMBER);
    add_instr_name(">=", "c_ge();\n", F_GE, T_NUMBER);
    add_instr_name("==", "f_eq();\n", F_EQ, T_NUMBER);
    add_instr_name("+=", "c_add_eq(0);\n", F_ADD_EQ, T_ANY);
    add_instr_name("(void)+=", "c_add_eq(1);\n", F_VOID_ADD_EQ, T_NUMBER);
    add_instr_name("!", "c_not();\n", F_NOT, T_NUMBER);
    add_instr_name("&", "f_and();\n", F_AND, T_ARRAY | T_NUMBER);
    add_instr_name("&=", "f_and_eq();\n", F_AND_EQ, T_NUMBER);
    add_instr_name("index", "c_index();\n", F_INDEX, T_ANY);
    add_instr_name("member", "c_member(%i);\n", F_MEMBER, T_ANY);
    add_instr_name("new_empty_class", "c_new_class(%i, 0);\n", F_NEW_EMPTY_CLASS, T_ANY);
    add_instr_name("new_class", "c_new_class(%i, 1);\n", F_NEW_CLASS, T_ANY);
    add_instr_name("rindex", "c_rindex();\n", F_RINDEX, T_ANY);
    add_instr_name("loop_cond_local", "C_LOOP_COND_LV(%i, %i); if (lpc_int)\n", F_LOOP_COND_LOCAL, -1);
    add_instr_name("loop_cond_number", "C_LOOP_COND_NUM(%i, %i); if (lpc_int)\n", F_LOOP_COND_NUMBER, -1);
    add_instr_name("loop_incr", "C_LOOP_INCR(%i);\n", F_LOOP_INCR, -1);
    add_instr_name("foreach", 0, F_FOREACH, -1);
    add_instr_name("exit_foreach", "c_exit_foreach();\n", F_EXIT_FOREACH, -1);
    add_instr_name("expand_varargs", 0, F_EXPAND_VARARGS, -1);
    add_instr_name("next_foreach", "c_next_foreach();\n", F_NEXT_FOREACH, -1);
    add_instr_name("member_lvalue", "c_member_lvalue(%i);\n", F_MEMBER_LVALUE, T_LVALUE);
    add_instr_name("index_lvalue", "push_indexed_lvalue(0);\n",
                   F_INDEX_LVALUE, T_LVALUE|T_LVALUE_BYTE);
    add_instr_name("rindex_lvalue", "push_indexed_lvalue(1);\n",
                   F_RINDEX_LVALUE, T_LVALUE|T_LVALUE_BYTE);
    add_instr_name("nn_range_lvalue", "push_lvalue_range(0x00);\n",
                   F_NN_RANGE_LVALUE, T_LVALUE_RANGE);
    add_instr_name("nr_range_lvalue", "push_lvalue_range(0x01);\n",
                   F_NR_RANGE_LVALUE, T_LVALUE_RANGE);
    add_instr_name("rr_range_lvalue", "push_lvalue_range(0x11);\n",
                   F_RR_RANGE_LVALUE, T_LVALUE_RANGE);
    add_instr_name("rn_range_lvalue", "push_lvalue_range(0x10);\n",
                   F_RN_RANGE_LVALUE, T_LVALUE_RANGE);
    add_instr_name("nn_range", "f_range(0x00);\n",
                   F_NN_RANGE, T_ARRAY|T_STRING OR_BUFFER);
    add_instr_name("rr_range", "f_range(0x11);\n",
                   F_RR_RANGE, T_ARRAY|T_STRING OR_BUFFER);
    add_instr_name("nr_range", "f_range(0x01);\n",
                   F_NR_RANGE, T_ARRAY|T_STRING OR_BUFFER);
    add_instr_name("rn_range", "f_range(0x10);\n",
                   F_RN_RANGE, T_ARRAY|T_STRING OR_BUFFER);
    add_instr_name("re_range", "f_extract_range(1);\n",
                   F_RE_RANGE, T_ARRAY|T_STRING OR_BUFFER);
    add_instr_name("ne_range", "f_extract_range(0);\n",
                   F_NE_RANGE, T_ARRAY|T_STRING OR_BUFFER);
    add_instr_name("global", "C_GLOBAL(%i);\n", F_GLOBAL, T_ANY);
    add_instr_name("local", "C_LOCAL(%i);\n", F_LOCAL, T_ANY);
    add_instr_name("make_ref", "c_make_ref(%i);\n", F_MAKE_REF, T_REF);
    add_instr_name("kill_refs", "c_kill_refs(%i);\n", F_KILL_REFS, T_ANY);
    add_instr_name("ref", "C_REF(%i);\n", F_REF, T_ANY);
    add_instr_name("ref_lvalue", "C_REF_LVALUE(%i);\n", F_REF_LVALUE, T_LVALUE);
    add_instr_name("transfer_local", "C_TRANSFER_LOCAL(%i);\n", F_TRANSFER_LOCAL, T_ANY);
    add_instr_name("number", 0, F_NUMBER, T_NUMBER);
    add_instr_name("real", 0, F_REAL, T_REAL);
    add_instr_name("local_lvalue", "C_LVALUE(fp + %i);\n", F_LOCAL_LVALUE, T_LVALUE);
    add_instr_name("while_dec", "C_WHILE_DEC(%i); if (lpc_int)\n", F_WHILE_DEC, -1);
    add_instr_name("const1", "push_number(1);\n", F_CONST1, T_NUMBER);
    add_instr_name("subtract", "c_subtract();\n", F_SUBTRACT, T_NUMBER | T_REAL | T_ARRAY);
    add_instr_name("(void)assign", "c_void_assign();\n", F_VOID_ASSIGN, T_NUMBER);
    add_instr_name("(void)assign_local", "c_void_assign_local(fp + %i);\n", F_VOID_ASSIGN_LOCAL, T_NUMBER);
    add_instr_name("assign", "c_assign();\n", F_ASSIGN, T_ANY);
    add_instr_name("branch", 0, F_BRANCH, -1);
    add_instr_name("bbranch", 0, F_BBRANCH, -1);
    add_instr_name("byte", 0, F_BYTE, T_NUMBER);
    add_instr_name("-byte", 0, F_NBYTE, T_NUMBER);
    add_instr_name("branch_ne", 0, F_BRANCH_NE, -1);
    add_instr_name("branch_ge", 0, F_BRANCH_GE, -1);
    add_instr_name("branch_le", 0, F_BRANCH_LE, -1);
    add_instr_name("branch_eq", 0, F_BRANCH_EQ, -1);
    add_instr_name("bbranch_lt", 0, F_BBRANCH_LT, -1);
    add_instr_name("bbranch_when_zero", 0, F_BBRANCH_WHEN_ZERO, -1);
    add_instr_name("bbranch_when_non_zero", 0, F_BBRANCH_WHEN_NON_ZERO, -1);
    add_instr_name("branch_when_zero", 0, F_BRANCH_WHEN_ZERO, -1);
    add_instr_name("branch_when_non_zero", 0, F_BRANCH_WHEN_NON_ZERO, -1);
    add_instr_name("pop", "pop_stack();\n", F_POP_VALUE, -1);
    add_instr_name("const0", "push_number(0);\n", F_CONST0, T_NUMBER);
#ifdef F_JUMP_WHEN_ZERO
    add_instr_name("jump_when_zero", F_JUMP_WHEN_ZERO, -1);
    add_instr_name("jump_when_non_zero", F_JUMP_WHEN_NON_ZERO, -1);
#endif
#ifdef F_LOR
    add_instr_name("||", 0, F_LOR, -1);
    add_instr_name("&&", 0, F_LAND, -1);
#endif
    add_instr_name("-=", "f_sub_eq();\n", F_SUB_EQ, T_ANY);
#ifdef F_JUMP
    add_instr_name("jump", F_JUMP, -1);
#endif
    add_instr_name("return_zero", "c_return_zero();\nreturn;\n", F_RETURN_ZERO, -1);
    add_instr_name("return", "c_return();\nreturn;\n", F_RETURN, -1);
    add_instr_name("sscanf", "c_sscanf(%i);\n", F_SSCANF, T_NUMBER);
    add_instr_name("parse_command", "c_parse_command(%i);\n", F_PARSE_COMMAND, T_NUMBER);
    add_instr_name("string", 0, F_STRING, T_STRING);
    add_instr_name("short_string", 0, F_SHORT_STRING, T_STRING);
    add_instr_name("call", "c_call(%i, %i);\n", F_CALL_FUNCTION_BY_ADDRESS, T_ANY);
    add_instr_name("call_inherited", "c_call_inherited(%i, %i, %i);\n", F_CALL_INHERITED, T_ANY);
    add_instr_name("aggregate_assoc", "C_AGGREGATE_ASSOC(%i);\n", F_AGGREGATE_ASSOC, T_MAPPING);
#ifdef DEBUG
    add_instr_name("break_point", "break_point();\n", F_BREAK_POINT, -1);
#endif
    add_instr_name("aggregate", "C_AGGREGATE(%i);\n", F_AGGREGATE, T_ARRAY);
    add_instr_name("(::)", 0, F_FUNCTION_CONSTRUCTOR, T_FUNCTION);
    /* sorry about this one */
    add_instr_name("simul_efun",
                   "call_simul_efun(%i, (lpc_int = %i + num_varargs, num_varargs = 0, lpc_int));\n",
                   F_SIMUL_EFUN, T_ANY);
    add_instr_name("global_lvalue", "C_LVALUE(&current_object->variables[variable_index_offset + %i]);\n", F_GLOBAL_LVALUE, T_LVALUE);
    add_instr_name("|", "f_or();\n", F_OR, T_ARRAY | T_NUMBER);
    add_instr_name("<<", "f_lsh();\n", F_LSH, T_NUMBER);
    add_instr_name(">>", "f_rsh();\n", F_RSH, T_NUMBER);
    add_instr_name(">>=", "f_rsh_eq();\n", F_RSH_EQ, T_NUMBER);
    add_instr_name("<<=", "f_lsh_eq();\n", F_LSH_EQ, T_NUMBER);
    add_instr_name("^", "f_xor();\n", F_XOR, T_NUMBER);
    add_instr_name("^=", "f_xor_eq();\n", F_XOR_EQ, T_NUMBER);
    add_instr_name("|=", "f_or_eq();\n", F_OR_EQ, T_NUMBER);
    add_instr_name("+", "c_add();\n", F_ADD, T_ANY);
    add_instr_name("!=", "f_ne();\n", F_NE, T_NUMBER);
    add_instr_name("catch", 0, F_CATCH, T_ANY);
    add_instr_name("end_catch", 0, F_END_CATCH, -1);
    add_instr_name("-", "c_negate();\n", F_NEGATE, T_NUMBER | T_REAL);
    add_instr_name("~", "c_compl();\n", F_COMPL, T_NUMBER);
    add_instr_name("++x", "c_pre_inc();\n", F_PRE_INC, T_NUMBER | T_REAL);
    add_instr_name("--x", "c_pre_dec();\n", F_PRE_DEC, T_NUMBER | T_REAL);
    add_instr_name("*", "c_multiply();\n", F_MULTIPLY, T_REAL | T_NUMBER | T_MAPPING);
    add_instr_name("*=", "f_mult_eq();\n", F_MULT_EQ, T_REAL | T_NUMBER | T_MAPPING);
    add_instr_name("/", "c_divide();\n", F_DIVIDE, T_REAL | T_NUMBER);
    add_instr_name("/=", "f_div_eq();\n", F_DIV_EQ, T_NUMBER | T_REAL);
    add_instr_name("%", "c_mod();\n", F_MOD, T_NUMBER);
    add_instr_name("%=", "f_mod_eq();\n", F_MOD_EQ, T_NUMBER);
    add_instr_name("inc(x)", "c_inc();\n", F_INC, -1);
    add_instr_name("dec(x)", "c_dec();\n", F_DEC, -1);
    add_instr_name("x++", "c_post_inc();\n", F_POST_INC, T_NUMBER | T_REAL);
    add_instr_name("x--", "c_post_dec();\n", F_POST_DEC, T_NUMBER | T_REAL);
    add_instr_name("switch", 0, F_SWITCH, -1);
    add_instr_name("time_expression", 0, F_TIME_EXPRESSION, -1);
    add_instr_name("end_time_expression", 0, F_END_TIME_EXPRESSION, T_NUMBER);
}

#define get_next_char(c) if ((c = *outp++) == '\n' && outp == last_nl + 1) refill_buffer()

#define GETALPHA(p, q, m, e) \
    if (*p == '_' || uisalpha(*p)) {\
        while(isalunum((unsigned char)*p)) {\
            *q = *p++;\
            if (q < (m))\
                q++;\
            else {\
                lexerror("Name too long");\
                return;\
            }\
        }\
    } else lexerror(e);\
    *q++ = 0

static int cmygetc()
{
    int c;

    for (;;) {
        get_next_char(c);
        if (c == '/') {
            switch(*outp++) {
                case '*': skip_comment(); break;
                case '/': skip_line(); break;
                default: outp--; return c;
            }
        } else {
            return c;
        }
    }
}

static void refill()
{
    char *p;
    unsigned char c;

    p = yytext;
    do {
        c = *outp++;
        if (p < yytext + MAXLINE - 5)
            *p++ = c;
        else {
            lexerror("Line too long");
            break;
        }
    } while (c != '\n' && c != LEX_EOF);
    if ((c == '\n') && (outp == last_nl + 1)) refill_buffer();
    p[-1] = ' ';
    *p = 0;
    nexpands = 0;
    current_line++;
}

static void handle_define (char * yyt)
{
    char namebuf[NSIZE];
    char args[NARGS][NSIZE];
    char mtext[MLEN];
    char *p, *q;

    p = yyt;
    strcat(p, " ");
    q = namebuf;
    GETALPHA(p, q, namebuf + NSIZE - 1, "Invalid macro name");
    if (*p == '(') {            /* if "function macro" */
        int squote, dquote;
        int arg;
        int inid;
        char *ids = (char *) NULL;

        p++;                    /* skip '(' */
        SKIPWHITE;
        if (*p == ')') {
            arg = 0;
        } else {
            for (arg = 0; arg < NARGS;) {
                q = args[arg];
                GETALPHA(p, q, args[arg] + NSIZE - 1, "Invalid macro argument");
                arg++;
                SKIPWHITE;
                if (*p == ')')
                    break;
                if (*p++ != ',') {
                    yyerror("Missing ',' in #define parameter list");
                    return;
                }
                SKIPWHITE;
            }
            if (arg == NARGS) {
                lexerror("Too many macro arguments");
                return;
            }
        }
        p++;                    /* skip ')' */
        q = mtext;
        *q++ = ' ';
        squote = dquote = 0;
        for (inid = 0; *p;) {
            if (isalunum((unsigned char)*p)) { /* FIXME */
                if (!inid) {
                    inid++;
                    ids = p;
                }
            } else {
                if (!squote && !dquote && inid) {
                    int idlen = p - ids;
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
            *q = *p;
            if (*q == '\\' && *++p) {
                if (q < mtext + MLEN - 2)
                    q++;
                else {
                    lexerror("Macro text too long");
                    return;
                }
                *q = *p;
                /* skip the quote checks if we just had a \ */
            }
            else if (*q == '"') {
                dquote ^= !squote;
            } else if (*q == '\'') {
                squote ^= !dquote;
            }

            if (*p++ == MARKS)
                *++q = MARKS;
            if (q < mtext + MLEN - 2)
                q++;
            else {
                lexerror("Macro text too long");
                return;
            }
            if (!*p && p[-2] == '\\') {
                q -= 2;
                refill();
                p = yytext;
            }
        }
        *--q = 0;
        add_define(namebuf, arg, mtext);
    } else if (is_wspace(*p) || (*p == '\\')) {
        for (q = mtext; *p;) {
            *q = *p++;
            if (q < mtext + MLEN - 2)
                q++;
            else {
                lexerror("Macro text too long");
                return;
            }
            if (!*p && p[-2] == '\\') {
                q -= 2;
                refill();
                p = yytext;
            }
        }
        *--q = 0;
        add_define(namebuf, -1, mtext);
    } else {
        lexerror("Illegal macro symbol");
    }
    return;
}

/* IDEA: linked buffers, to allow "unlimited" buffer expansion */
static void add_input (const char * p)
{
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

        while (*q != '\n' && (unsigned char)*q != LEX_EOF) q++;
        /* Incorporate EOF later */
        if (*q != '\n' || ((q  - outp) + l) >= DEFMAX - 11) {
            lexerror("Macro expansion buffer overflow");
            return;
        }
        size = (q - outp) + l + 1;
        cur_lbuf->outp = q + 1;
        cur_lbuf->last_nl = last_nl;

        new_lbuf = ALLOCATE(linked_buf_t, TAG_COMPILER, "add_input");
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

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_all_defines() {
    int i;
    defn_t *tmp;

    for (i = 0; i < inc_list_size; i++)
        EXTRA_REF(BLOCK(inc_list[i]))++;

    for (i = 0; i < DEFHASH; i++) {
        tmp = defns[i];
        while (tmp) {
            DO_MARK(tmp, TAG_PREDEFINES);
            DO_MARK(tmp->name, TAG_PREDEFINES);
            DO_MARK(tmp->exps, TAG_PREDEFINES);
            tmp = tmp->next;
        }
    }
}
#endif

static void add_predefine (const char * name, int nargs, const char * exps)
{
    defn_t *p;
    int h;

    if ((p = lookup_define(name))) {
        if (nargs != p->nargs || strcmp(exps, p->exps)) {
            char buf[200 + NSIZE];

            sprintf(buf, "redefinition of #define %s\n", name);
            yywarn(buf);
        }
        p->exps = (char *)DREALLOC(p->exps, strlen(exps) + 1, TAG_PREDEFINES, "add_define: redef");
        strcpy(p->exps, exps);
        p->nargs = nargs;
    } else {
        p = ALLOCATE(defn_t, TAG_PREDEFINES, "add_define: def");
        p->name = (char *) DXALLOC(strlen(name) + 1, TAG_PREDEFINES, "add_define: def name");
        strcpy(p->name, name);
        p->exps = (char *) DXALLOC(strlen(exps) + 1, TAG_PREDEFINES, "add_define: def exps");
        strcpy(p->exps, exps);
        p->flags = DEF_IS_PREDEF;
        p->nargs = nargs;
        h = defhash(name);
        p->next = defns[h];
        defns[h] = p;
    }
}

static void free_defines()
{
    defn_t *p, *q;
    int i;

    for (i = 0; i < DEFHASH; i++) {
        for (p = defns[i]; p; p = q) {
            /* predefines are at the end of the list */
            if (p->flags & DEF_IS_PREDEF) {
                break;
            }
            q = p->next;
            FREE(p->name);
            FREE(p->exps);
            FREE((char *) p);
        }
        defns[i] = p;
        /* in case they undefined a predef */
        while (p) {
            p->flags &= ~DEF_IS_UNDEFINED;
            p = p->next;
        }
    }
    nexpands = 0;
}

#define SKIPW \
        do {\
            c = cmygetc();\
        } while (is_wspace(c));

static int extract_args (char ** argv, char * argb)
{
    int argc = 0, dquote = 0, parcnt = 0, squote = 0;
    char *out;
    unsigned char c;

    SKIPW;
    if (c != '(') {
        yyerror("Missing '(' in macro call");
        if (c == '\n' && outp == last_nl + 1) refill_buffer();
        return -1;
    }

    SKIPW;
    if (c == ')')
        return 0;

    argv[0] = out = argb;
    while (argc < NARGS) {
        switch (c) {
            case '\"':
                if (!squote)
                    dquote ^= 1;
                break;
            case '\'':
                if (!dquote)
                    squote ^= 1;
                break;
            case '\\':
                if (squote || dquote) {
                    *out++ = c;
                    get_next_char(c);
                }
                break;
            case '(':
                if (!squote && !dquote)
                    parcnt++;
                break;
            case ')':
                if (!squote && !dquote)
                    parcnt--;
                break;
            case '\n':
                if (outp == last_nl + 1) refill_buffer();
                if (squote || dquote) {
                    lexerror("Newline in string");
                    return -1;
                }
                /* Change this to a space so we don't count it more than once */
                current_line++;
                total_lines++;
                c = ' ';
                break;
            case LEX_EOF:
                lexerror("Unexpected end of file");
                return -1;
        }

        /* negative parcnt means we're done collecting args */
        if (parcnt < 0 || (c == ',' && !parcnt && !dquote && !squote)) {

            /* strip off trailing whitespace char if there was one */
            if (uisspace(*(out - 1))) *(out - 1) = 0;
            else *out++ = 0;
            if (parcnt < 0)
                return argc + 1;
            argv[++argc] = out;
        } else {
            /* don't save leading whitespace and don't accumulate trailing whitespace */
            if (!uisspace(c) || dquote || squote || (out > argv[argc] && !uisspace(*(out - 1)))) {
                if (out >= argb + DEFMAX - NARGS) {
                    lexerror("Macro argument overflow");
                    return -1;
                }
                *out++ = c;
            }
        }

        if (!squote && !dquote) c = cmygetc();
        else get_next_char(c);
    }

    lexerror("Maximum macro argument count exceeded");
    return -1;
}

/* Check if yytext is a macro and expand if it is. */
static char *expand_define2 (char * text)
{
    int argc = 0, i, paste = 0, pasting = 0;
    defn_t *macro;
    char expbuf[DEFMAX], *argv[NARGS], *expand_buffer, *in, *out, *freeme = 0;

    /* special handling for __LINE__ macro */
    if (!strcmp(text, "__LINE__")) {
      expand_buffer = (char *)DXALLOC(20, TAG_COMPILER, "expand_define2");
      sprintf(expand_buffer, "%i", current_line);
      return expand_buffer;
    }

    /* have we already expanded this macro? */
    for (i = 0;  i < expand_depth;  i++) {
        if (!strcmp(expands[i], text))
            return 0;
    }
    expands[expand_depth++] = text;

    if (nexpands++ > EXPANDMAX) {
        expand_depth--;
        lexerror("Too many macro expansions");
        return 0;
    }

    macro = lookup_define(text);
    if (!macro) {
        expand_depth--;
        return 0;
    }

    if (macro->nargs >= 0) {
        if ((argc = extract_args(argv, expbuf)) == -1) {
            expand_depth--;
            return 0;
        }
        if (argc != macro->nargs) {
            expand_depth--;
            yyerror("Wrong number of macro arguments");
            return 0;
        }
    }

    if (!argc) {
        expand_buffer = (char *)DXALLOC(strlen(macro->exps) + 1, TAG_COMPILER, "expand_define2");
        strcpy(expand_buffer, macro->exps);
        expand_depth--;
        return expand_buffer;
    }

    /* Perform expansion with args */
    in  = macro->exps;
    out = expand_buffer = (char *)DXALLOC(DEFMAX, TAG_COMPILER, "expand_define2");

#define SAVECHAR(x) SAFE(\
                    if (out + 1 < expand_buffer + DEFMAX) {\
                        *out++ = (x);\
                    } else {\
                        if (freeme) FREE(freeme);\
                        FREE(expand_buffer);\
                        lexerror("Macro expansion overflow");\
                        expand_depth--;\
                        return 0;\
                    })

#define SAVESTR(x, y)   SAFE(\
                        if (out + (y) < expand_buffer + DEFMAX) {\
                            memcpy(out, (x), (y));\
                            out += (y);\
                        } else {\
                            if (freeme) FREE(freeme);\
                            FREE(expand_buffer);\
                            lexerror("Macro expansion overflow");\
                            expand_depth--;\
                            return 0;\
                        }\
                        if (freeme) {\
                            FREE(freeme);\
                            freeme = 0;\
                        })

    while (*in) {
        char *skip = in + 1;

        if (*in == MARKS && *++in != MARKS) {
            char *exp;

            exp = argv[*in++ - MARKS - 1];

            if (paste) {
                paste = 0;
                *(out - 1) = '\"';
                while (*exp) {
                    switch (*exp) {
                        case '\"':
                            SAVECHAR('\\');
                            break;
                        case '\\':
                            SAVECHAR(*exp);
                            exp++;
                            break;
                    }
                    SAVECHAR(*exp);
                    exp++;
                }
                SAVECHAR('\"');
            } else {
                int len;

                /* don't expand if token pasting with ## */
                if (!pasting && (freeme = expand_define2(exp)) != 0)
                    exp = freeme;
                len = strlen(exp);
                SAVESTR(exp, len);
            }
        } else {
            paste = (*in == '#');
            /* FIXME: Here we need to recursively expand any macros that we
             * come across.  This cannot possibly be done the way lex.c is
             * currently architected.  I've tried.  This all needs to be
             * redone anyway, but this just going to have to do for now :-(
             */
            SAVECHAR(*in);
            in++;
        }

        /* skip over whitespace and see if we've got ## for token pasting.  if
         * we do, skip over it and continue at the first non-whitespace char.
         * note that comments are not considered whitespace here like they
         * should be.
         */
        pasting = 0;
        while (*skip && uisspace(*skip)) skip++;
        if (*skip == '#' && *(skip + 1) == '#') {
            skip += 2;
            /* guaranteed by add_define to not have end of input before non-whitespace */
            while (uisspace(*skip)) skip++;
            in = skip;
            pasting = (*in == MARKS && *(in + 1) != MARKS);
        }
    }

    *out = 0;
    expand_depth--;
    return expand_buffer;
}

int expand_define (void)
{
    char *expand_buffer;

    if ((expand_buffer = expand_define2(yytext)) != 0) {
        add_input(expand_buffer);
        FREE(expand_buffer);
        return 1;
    }

    return 0;
}

/* Stuff to evaluate expression.  I havn't really checked it. /LA
** Written by "J\"orn Rennecke" <amylaar@cs.tu-berlin.de>
*/
#define SKPW    do c = *outp++; while(is_wspace(c)); outp--

static int exgetc()
{
    unsigned char c;
    char *yyp;

    c = *outp++;
    while (isalpha(c) || c == '_') {
        yyp = yytext;
        do {
            SAVEC;
            c = *outp++;
        } while (isalunum(c));
        outp--;
        *yyp = '\0';
        if (strcmp(yytext, "defined") == 0 ||
            strcmp(yytext, "efun_defined") == 0) {
            int efund = (yytext[0] == 'e');
            int flag;

            /* handle the defined "function" in #if */
            do
                c = *outp++;
            while (is_wspace(c));
            if (c != '(') {
                yyerror("Missing ( in defined");
                continue;
            }
            do
                c = *outp++;
            while (is_wspace(c));
            yyp = yytext;
            while (isalunum(c)) {
                SAVEC;
                c = *outp++;
            }
            *yyp = '\0';
            while (is_wspace(c))
                c = *outp++;
            if (c != ')') {
                yyerror("Missing ) in defined");
                continue;
            }
            SKPW;
            if (efund) {
                ident_hash_elem_t *ihe = lookup_ident(yytext);
                flag = (ihe && ihe->dn.efun_num != -1);
            } else {
                flag = (lookup_define(yytext) != 0);
            }
            if (flag)
                add_input(" 1 ");
            else
                add_input(" 0 ");
        } else {
            if (!expand_define())
                add_input(" 0 ");
        }
        c = *outp++;
    }
    return c;
}

void set_inc_list (char * list)
{
    int i, size;
    char *p;

    if (list == 0) {
        fprintf(stderr, "The config string 'include dirs' must bet set.\n");
        fprintf(stderr, "It should contain a list of all directories to be searched\n");
        fprintf(stderr, "for include files, separated by a ':'.\n");
        exit(-1);
    }
    size = 1;
    p = list;
    while (1) {
        p = strchr(p, ':');
        if (!p)
            break;
        size++;
        p++;
    }
    inc_list = CALLOCATE(size, char *, TAG_INC_LIST, "set_inc_list");
    inc_list_size = size;
    for (i = size - 1; i >= 0; i--) {
        p = strrchr(list, ':');
        if (p) {
            *p = '\0';
            p++;
        } else {
            if (i) {
                fprintf(stderr, "Fatal error in set_inc_list: bad state.\n");
                exit(1);
            }
            p = list;
        }
        if (*p == '/')
            p++;
        /*
         * Even make sure that the mud administrator has not made an error.
         */
        if (!legal_path(p)) {
            fprintf(stderr, "'include dirs' must give paths without any '..'\n");
            exit(-1);
        }
        inc_list[i] = make_shared_string(p);
    }
    for (i = 0; i < size - 1; i++)
        list[strlen(list)] = ':';
}

char *main_file_name()
{
    incstate_t *is;

    if (inctop == 0)
        return current_file;
    is = inctop;
    while (is->next)
        is = is->next;
    return is->file;
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

#define CHECK_ELEM(x, y, z) if (!strcmp((x)->name, (y))) { \
      if (((x)->token & IHE_RESWORD) || ((x)->sem_value)) { z } \
      else return 0; }

ident_hash_elem_t *lookup_ident (const char * name) {
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
    return 0;
}

ident_hash_elem_t *find_or_add_perm_ident (const char * name) {
    int h = IdentHash(name);
    ident_hash_elem_t *hptr, *hptr2;

    if ((hptr = ident_hash_table[h])) {
        if (!strcmp(hptr->name, name)) return hptr;
        hptr2 = hptr->next;
        while (hptr2 != hptr) {
            if (!strcmp(hptr2->name, name)) return hptr2;
            hptr2 = hptr2->next;
        }
        hptr = ALLOCATE(ident_hash_elem_t, TAG_PERM_IDENT, "find_or_add_perm_ident:1");
        hptr->next = ident_hash_head[h]->next;
        ident_hash_head[h]->next = hptr;
        if (ident_hash_head[h] == ident_hash_tail[h])
            ident_hash_tail[h] = hptr;
    } else {
        hptr = (ident_hash_table[h] = ALLOCATE(ident_hash_elem_t, TAG_PERM_IDENT,
                                               "find_or_add_perm_ident:2"));
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

lname_linked_buf_t *lnamebuf = 0;

int lb_index = 4096;

static char *alloc_local_name (const char * name) {
    int len = strlen(name)+1;
    char *res;

    if (lb_index + len > 4096) {
        lname_linked_buf_t *new_buf;
        new_buf = ALLOCATE(lname_linked_buf_t, TAG_COMPILER, "alloc_local_name");
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

ident_hash_elem_list_t *ihe_list = 0;

#if 0
void dump_ihe (ident_hash_elem_t * ihe, int noisy) {
    int sv = 0;
    if (ihe->token & IHE_RESWORD) {
        if (noisy) printf("%s ", ihe->name);
    } else {
        if (noisy) printf("%s[", ihe->name);
        if (ihe->dn.function_num != -1) {
            if (noisy) printf("f");
            sv++;
        }
        if (ihe->dn.simul_num != -1) {
            if (noisy) printf("s");
            sv++;
        }
        if (ihe->dn.efun_num != -1) {
            if (noisy) printf("e");
            sv++;
        }
        if (ihe->dn.local_num != -1) {
            if (noisy) printf("l");
            sv++;
        }
        if (ihe->dn.global_num != -1) {
            if (noisy) printf("g");
            sv++;
        }
        if (ihe->sem_value != sv) {
            if (noisy) {
                printf("(*%i*)", ihe->sem_value - sv);
            } else dump_ihe(ihe, 1);
        }
        if (noisy) printf("] ");
    }
}

void debug_dump_ident_hash_table (int noisy) {
    int zeros = 0;
    int i;
    ident_hash_elem_t *ihe, *ihe2;

    if (noisy) printf("\n\nIdentifier Hash Table:\n");
    for (i = 0; i < IDENT_HASH_SIZE; i++) {
        ihe = ident_hash_table[i];
        if (!ihe)
            zeros++;
        else {
            if (zeros && noisy) printf("<%i zeros>\n", zeros);
            zeros = 0;
            dump_ihe(ihe, noisy);
            ihe2 = ihe->next;
            while (ihe2 != ihe) {
                dump_ihe(ihe2, noisy);
                ihe2 = ihe2->next;
            }
            if (noisy) printf("\n");
        }
    }
    if (zeros && noisy) printf("<%i zeros>\n", zeros);
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

    for (i = 0; i < IDENT_HASH_SIZE; i++)
        if ((ident_hash_table[i] = ident_hash_head[i]))
            ident_hash_tail[i]->next = ident_hash_head[i];

    ihel = ihe_list;
    while (ihel) {
        next = ihel->next;
        FREE(ihel);
        ihel = next;
    }
    ihe_list = 0;
    num_free = 0;

    lnb = lnamebuf;
    while (lnb) {
        lnbn = lnb->next;
        FREE(lnb);
        lnb = lnbn;
    }
    lnamebuf = 0;
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
        ihel = ALLOCATE(ident_hash_elem_list_t, TAG_COMPILER,
                        "quick_alloc_ident_entry");
        ihel->next = ihe_list;
        ihe_list = ihel;
        num_free = 127;
        return &(ihe_list->items[127]);
    }
}

ident_hash_elem_t *
find_or_add_ident (const char * name, int flags) {
    int h = IdentHash(name);
    ident_hash_elem_t *hptr, *hptr2;

    if ((hptr = ident_hash_table[h])) {
        if (!strcmp(hptr->name, name)) {
            if ((hptr->token & IHE_PERMANENT) && (flags & FOA_GLOBAL_SCOPE)
                && (hptr->dn.function_num==-1)&&(hptr->dn.global_num==-1)
                && (hptr->dn.class_num==-1)) {
                hptr->next_dirty = ident_dirty_list;
                ident_dirty_list = hptr;
            }
            return hptr;
        }
        hptr2 = hptr->next;
        while (hptr2 != hptr) {
            if (!strcmp(hptr2->name, name)) {
                if ((hptr2->token & IHE_PERMANENT)&&(flags & FOA_GLOBAL_SCOPE)
                 && (hptr2->dn.function_num==-1)&&(hptr2->dn.global_num==-1)
                    && (hptr2->dn.class_num == -1)) {
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

static void add_keyword_t (const char * name, keyword_t * entry) {
    int h = IdentHash(name);

    if (ident_hash_table[h]) {
        entry->next = ident_hash_head[h]->next;
        ident_hash_head[h]->next = (ident_hash_elem_t *)entry;
        if (ident_hash_head[h] == ident_hash_tail[h])
            ident_hash_tail[h] = (ident_hash_elem_t *)entry;
    } else {
        ident_hash_head[h] = (ident_hash_elem_t *)entry;
        ident_hash_tail[h] = (ident_hash_elem_t *)entry;
        ident_hash_table[h] = (ident_hash_elem_t *)entry;
        entry->next = (ident_hash_elem_t *)entry;
    }
    entry->token |= IHE_RESWORD;
}

void init_identifiers() {
    int i;
    ident_hash_elem_t *ihe;

    init_instrs();

    /* allocate all three tables together */
    ident_hash_table = CALLOCATE(IDENT_HASH_SIZE * 3, ident_hash_elem_t *,
                                 TAG_IDENT_TABLE, "init_identifiers");
    ident_hash_head = (ident_hash_elem_t **)&ident_hash_table[IDENT_HASH_SIZE];
    ident_hash_tail = (ident_hash_elem_t **)&ident_hash_table[2*IDENT_HASH_SIZE];

    /* clean all three tables */
    for (i=0; i<IDENT_HASH_SIZE * 3; i++) {
        ident_hash_table[i]=0;
    }
    /* add the reserved words */
    for (i=0; i<NELEM(reswords); i++) {
        add_keyword_t(reswords[i].word, &reswords[i]);
    }
    /* add the efuns */
    for (i=0; i<NELEM(predefs); i++) {
        if (predefs[i].word[0] == '_') {
            predefs[i].word++;
            if (strcmp(predefs[i].word, "call_other") == 0)
                arrow_efun = i;
            if (strcmp(predefs[i].word, "evaluate") == 0)
                evaluate_efun = i;
            if (strcmp(predefs[i].word, "this_object") == 0)
                this_efun = i;
            if (strcmp(predefs[i].word, "to_int") == 0)
                to_int_efun = i;
            if (strcmp(predefs[i].word, "to_float") == 0)
                to_float_efun = i;
            if (strcmp(predefs[i].word, "new") == 0)
                new_efun = i;
            continue;
        }

        ihe = find_or_add_perm_ident(predefs[i].word);
        ihe->token |= IHE_EFUN;
        ihe->sem_value++;
        ihe->dn.efun_num = i;
    }
}

