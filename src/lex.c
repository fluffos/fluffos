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

#include "std.h"
#include "lex.h"
#include "config.h"
#include "compiler.h"
#include "grammar.tab.h"
#include "interpret.h"
#include "stralloc.h"
#include "scratchpad.h"
/* fatal */
#include "simulate.h"
/* whashstr */
#include "hash.h"
/* legal_path */
#include "file.h"

#define NELEM(a) (sizeof (a) / sizeof((a)[0]))

int current_line;               /* line number in this file */
int current_line_base;          /* number of lines from other files */
int current_line_saved;         /* last line in this file where line num
				   info was saved */

int total_lines;		/* Used to compute average compiled lines/s */
char *current_file;
int current_file_id;

/* Bit flags for pragmas in effect */
int pragmas;
int optimization;

int num_parse_error;		/* Number of errors in the parser. */


#define MAX_LINE 1024

struct lpc_predef_s *lpc_predefs = NULL;

static FILE *yyin;
static int lex_fatal;
static char **inc_list;
static int inc_list_size;
static int defines_need_freed = 0;

#define EXPANDMAX 25000
static int nexpands;

#define MAXLINE 1024
char yytext[MAXLINE];


static int slast, lastchar;

struct defn {
    struct defn *next;
    char *name;
    char *exps;
    int flags;
    int nargs;
};

#define DEF_IS_UNDEFINED 1
#define DEF_IS_PREDEF    2

static struct ifstate {
    struct ifstate *next;
    int state;
}      *iftop = 0;

#define EXPECT_ELSE 1
#define EXPECT_ENDIF 2

static struct incstate {
    struct incstate *next;
    FILE *yyin;
    int line;
    char *file;
    int file_id;
    int slast, lastchar;
}       *inctop = 0;

/* prevent unbridled recursion */
#define MAX_INCLUDE_DEPTH 32
static int incnum;

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
keyword predefs[] =
#include "efun_defs.c"

char *option_defs[] = 
#include "option_defs.c"

static keyword efun_keyword = { "efun", L_EFUN | IHE_RESWORD, 0};
static keyword asm_keyword = { "asm", L_ASM | IHE_RESWORD, 0};

static keyword reswords[] =
{
#ifdef ARRAY_RESERVED_WORD
    {"array", '*', 0},
#endif
    {"break", L_BREAK, 0},
#ifndef DISALLOW_BUFFER_TYPE
    {"buffer", L_BASIC_TYPE, TYPE_BUFFER},
#endif
    {"case", L_CASE, 0},
    {"catch", L_CATCH, 0},
    {"continue", L_CONTINUE, 0},
    {"default", L_DEFAULT, 0},
    {"do", L_DO, 0},
    {"else", L_ELSE, 0},
    {"float", L_BASIC_TYPE, TYPE_REAL},
    {"for", L_FOR, 0},
    {"function", L_FUNCTION, TYPE_FUNCTION},
    {"if", L_IF, 0},
    {"inherit", L_INHERIT, 0},
    {"int", L_BASIC_TYPE, TYPE_NUMBER},
    {"mapping", L_BASIC_TYPE, TYPE_MAPPING},
    {"mixed", L_BASIC_TYPE, TYPE_ANY},
    {"nomask", L_TYPE_MODIFIER, TYPE_MOD_NO_MASK},
    {"object", L_OBJECT, TYPE_OBJECT},
    {"parse_command", L_PARSE_COMMAND, 0},
    {"private", L_TYPE_MODIFIER, TYPE_MOD_PRIVATE},
    {"protected", L_TYPE_MODIFIER, TYPE_MOD_PROTECTED},
    {"public", L_TYPE_MODIFIER, TYPE_MOD_PUBLIC},
    {"return", L_RETURN, 0},
    {"sscanf", L_SSCANF, 0},
    {"static", L_TYPE_MODIFIER, TYPE_MOD_STATIC},
#ifdef HAS_STATUS_TYPE
    {"status", L_BASIC_TYPE, TYPE_NUMBER},
#endif
    {"string", L_BASIC_TYPE, TYPE_STRING},
    {"switch", L_SWITCH, 0},
    {"time_expression", L_TIME_EXPRESSION, 0},
    {"varargs", L_TYPE_MODIFIER, TYPE_MOD_VARARGS},
    {"void", L_BASIC_TYPE, TYPE_VOID},
    {"while", L_WHILE, 0},
};

static struct ident_hash_elem **ident_hash_table;
static struct ident_hash_elem **ident_hash_head;
static struct ident_hash_elem **ident_hash_tail;

static struct ident_hash_elem *ident_dirty_list = 0;

struct instr instrs[MAX_INSTRS];
static char num_buf[20];

static char defbuf[DEFMAX];
static int nbuf;
static char *outp;

static void handle_define PROT((char *));
static void free_defines PROT((void));
static void add_define PROT((char *, int, char *, int));
static int expand_define PROT((void));
static void add_input PROT((char *));
static void myungetc PROT((int));
static int cond_get_exp PROT((int));
static void merge PROT((char *name, char *dest));
static void add_quoted_define PROT((char *, char *, int));
static struct defn *lookup_define PROT((char *s));
static INLINE int mygetc PROT((void));
static INLINE int gobble PROT((int));
static void lexerror PROT((char *));
static int skip_to PROT((char *, char *));
static void handle_cond PROT((int));
static FILE *inc_open PROT((char *, char *));
static void handle_include PROT((char *));
static int get_terminator PROT((char *));
static int get_array_block PROT((char *));
static int get_text_block PROT((char *));
static void skip_line PROT((void));
static void skip_comment PROT((void));
static void deltrail PROT((char *));
static void handle_pragma PROT((char *));
static int yylex1 PROT((void));
static void add_instr_name PROT((char *, int, short));
static int cmygetc PROT((void));
static void refill PROT((void));
static int exgetc PROT((void));
static old_func PROT((void));
static struct ident_hash_elem *quick_alloc_ident_entry PROT((void));

/* macro functions */
#define add_predefine(x, y, z) add_define(x, y, z, DEF_IS_PREDEF)
#define add_quoted_predefine(x, y) add_quoted_define(x, y, DEF_IS_PREDEF)

static void merge P2(char *, name, char *, dest)
{
    char *from;

    strcpy(dest, current_file);
    if ((from = strrchr(dest, '/')))	/* strip filename */
	*from = 0;
    else
	/* current_file was the file_name */
	/* include from the root directory */
	*dest = 0;

    from = name;
    while (*from == '/') {
	from++;
	*dest = 0;		/* absolute path */
    }

    while (*from) {
	if (!strncmp(from, "../", 3)) {
	    char *tmp;

	    if (*dest == 0)	/* including from above mudlib is NOT allowed */
		break;
	    tmp = strrchr(dest, '/');
	    if (tmp == NULL)	/* 1 component in dest */
		*dest = 0;
	    else
		*tmp = 0;
	    from += 3;		/* skip "../" */
	} else if (!strncmp(from, "./", 2)) {
	    from += 2;
	} else {		/* append first component to dest */
	    char *q;

	    if (*dest)
		strcat(dest, "/");	/* only if dest is not empty !! */
	    q = strchr(from, '/');

	    if (q) {		/* from has 2 or more components */
		while (*from == '/')	/* find the start */
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

static INLINE int mygetc()
{
    int c;

    if (nbuf) {
	nbuf--;
	c = *outp++;
    } else {
	c = getc(yyin);
    }
    lastchar = slast;
    slast = c;
/* fprintf(stderr, "c='%c'", c); */
    return c;
}

static INLINE int
gobble P1(int, c)
{
    int d;

    d = mygetc();
    if (c == d)
	return 1;
    *--outp = d;
    nbuf++;
    return 0;
}

static void
lexerror P1(char *, s)
{
    yyerror(s);
    lex_fatal++;
}

static int
skip_to P2(char *, token, char *, atoken)
{
    char b[20], *p;
    int c;
    int nest;

    for (nest = 0;;) {
	c = mygetc();
	if (c == '#') {
	    do {
		c = mygetc();
	    } while (isspace(c));
	    for (p = b; c != '\n' && !isspace(c) && c != EOF;) {
		if (p < b + sizeof b - 1)
		    *p++ = c;
		c = mygetc();
	    }
	    *p = 0;
/*fprintf(stderr, "skip checks %s\n", b);*/
	    if (strcmp(b, "if") == 0 || strcmp(b, "ifdef") == 0 ||
		strcmp(b, "ifndef") == 0) {
		nest++;
	    } else if (nest > 0) {
		if (strcmp(b, "endif") == 0)
		    nest--;
	    } else {
		if (strcmp(b, token) == 0) {
		    myungetc(c);
		    add_input(b);
		    myungetc('#');
		    return 1;
		} else if (atoken && strcmp(b, atoken) == 0) {
		    myungetc(c);
		    add_input(b);
		    myungetc('#');
		    return 0;
		} else if (strcmp(b, "elif") == 0) {
		    myungetc(c);
		    add_input(b);
		    myungetc('#');
		    return (atoken == 0);
		}
	    }
	}
/*fprintf(stderr, "skipping (%d) %c", c, c);*/
	while (c != '\n' && c != EOF) {
	    c = mygetc();
/*fprintf(stderr, "%c", c);*/
	}
	if (c == EOF) {
	    lexerror("Unexpected end of file while skipping");
	    return 1;
	}
	current_line++;
	total_lines++;
    }
}

static void
handle_cond P1(int, c)
{
    struct ifstate *p;

/*fprintf(stderr, "cond %d\n", c);*/
    if (!c)
	skip_to("else", "endif");
    p = (struct ifstate *) DXALLOC(sizeof(struct ifstate), 60, "handle_cond");
    p->next = iftop;
    iftop = p;
    p->state = c ? EXPECT_ENDIF : EXPECT_ELSE;
}

static FILE *
     inc_open P2(char *, buf, char *, name)
{
    FILE *f;
    int i;
    char *p;

    merge(name, buf);
    if ((f = fopen(buf, "r")) != NULL) {
	return f;
    }
    /*
     * Search all include dirs specified.
     */
    for (p = strchr(name, '.'); p; p = strchr(p + 1, '.')) {
	if (p[1] == '.')
	    return NULL;
    }
    for (i = 0; i < inc_list_size; i++) {
	sprintf(buf, "%s/%s", inc_list[i], name);
	if ((f = fopen(buf, "r"))) {
	    return f;
	}
    }
    return NULL;
}

static void
handle_include P1(char *, name)
{
    char *p;
    static char buf[1024];
    FILE *f;
    struct incstate *is;
    int delim;

/*fprintf(stderr, "handle include '%s'\n", name);*/
    if (nbuf) {
	lexerror("Internal preprocessor error");
	return;
    }
    if (*name != '"' && *name != '<') {
	struct defn *d;

	if ((d = lookup_define(name)) && d->nargs == -1) {
	    char *q;

	    q = d->exps;
	    while (isspace(*q))
		q++;
	    handle_include(q);
	} else {
	    yyerror("Missing leading \" or < in #include");
	}
	return;
    }
    delim = *name++ == '"' ? '"' : '>';
    for (p = name; *p && *p != delim; p++);
    if (!*p) {
	yyerror("Missing trailing \" or > in #include");
	return;
    }
    if (strlen(name) > sizeof(buf) - 100) {
	yyerror("Include name too long.");
	return;
    }
    *p = 0;
    if (++incnum == MAX_INCLUDE_DEPTH) {
	yyerror("Maximum include depth exceeded.");
    } else if ((f = inc_open(buf, name)) != NULL) {
	is = (struct incstate *)
	    DXALLOC(sizeof(struct incstate), 61, "handle_include: 1");
	is->yyin = yyin;
	is->line = current_line;
	is->file = current_file;
	is->file_id = current_file_id;
	is->slast = slast;
	is->lastchar = lastchar;
	is->next = inctop;
	inctop = is;
	
	current_line--;
	save_file_info(current_file_id, current_line  - current_line_saved);
	current_line_base += current_line;
	current_line_saved = 0;
	current_line = 1;
	current_file = make_shared_string(buf);
	current_file_id = add_program_file(buf, 0);
	slast = lastchar = '\n';
	yyin = f;
/*fprintf(stderr, "pushed to %s\n", buf);*/
    } else {
	sprintf(buf, "Cannot #include %s", name);
	yyerror(buf);
    }
}

static int
get_terminator P1(char *, terminator)
{
    int c, j = 0;

    while (((c = mygetc()) != EOF) && (isalnum(c) || c == '_'))
	terminator[j++] = c;

    terminator[j] = '\0';

    while (isspace(c) && c != EOF && c != '\n')
	c = mygetc();

    if (c == EOF)
	return 0;

    if (c == '\n') {
	current_line++;
    } else {
	myungetc(c);
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
              (char *)DXALLOC(MAXCHUNK, 124, "array/text chunk"); \
        len = 0; \
    }

static int
get_array_block P1(char *, term)
{
    int termlen;		/* length of terminator */
    char *array_line[NUMCHUNKS];/* allocate memory in chunks */
    int header, len;		/* header length; position in chunk */
    int startpos, startchunk;	/* start of line */
    int curchunk, res;		/* current chunk; this function's result */
    int c, i;			/* a char; an index counter */

    if (!(termlen = strlen(term))) {
	return 0;
    }
    /*
     * initialize
     */
    array_line[0] = (char *) DXALLOC(MAXCHUNK, 125, "array_block");
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
	while (((c = mygetc()) != '\n') && (c != EOF)) {
	    NEWCHUNK(array_line);
	    if (c == '"' || c == '\\') {
		array_line[curchunk][len++] = '\\';
		NEWCHUNK(array_line);
	    }
	    array_line[curchunk][len++] = c;
	}

	/*
	 * null terminate current chunk
	 */
	array_line[curchunk][len] = '\0';

	if (res)
	    break;

	/*
	 * check for terminator
	 */
	if ((!strncmp(array_line[startchunk] + startpos, term, termlen)) &&
	    (!isalnum(*(array_line[startchunk] + startpos + termlen))) &&
	    (*(array_line[startchunk] + startpos + termlen) != '_')) {
	    /*
	     * handle lone terminator on line
	     */
	    if (strlen(array_line[startchunk] + startpos) == termlen) {
		current_line++;
	    } else {
		/*
		 * put back trailing portion after terminator
		 */
		myungetc(c);	/* some operating systems give EOF only once */

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
	    myungetc(')');
	    myungetc('}');
	    for (i = startchunk; i >= 0; i--)
		add_input(array_line[i]);

	    res = 1;
	    break;
	} else {
	    /*
	     * only report end of file in array block, if not an include file
	     */
	    if (c == EOF && inctop == 0) {
		res = -1;
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
		    break;
		}
		array_line[++curchunk] =
		    (char *) DXALLOC(MAXCHUNK, 126, "array_block");
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
	FREE(array_line[curchunk]);

    return res;
}

static int
get_text_block P1(char *, term)
{
    int termlen;		/* length of terminator */
    char *text_line[NUMCHUNKS];	/* allocate memory in chunks */
    int len;			/* position in chunk */
    int startpos, startchunk;	/* start of line */
    int curchunk, res;		/* current chunk; this function's result */
    int c, i;			/* a char; an index counter */

    if (!(termlen = strlen(term))) {
	return 0;
    }
    /*
     * initialize
     */
    text_line[0] = (char *) DXALLOC(MAXCHUNK, 127, "text_block");
    text_line[0][0] = '"';
    text_line[0][1] = '\0';
    len = 1;
    startpos = 1;
    startchunk = 0;
    curchunk = 0;
    res = 0;

    while (1) {
	while (((c = mygetc()) != '\n') && (c != EOF)) {
	    NEWCHUNK(text_line);
	    if (c == '"' || c == '\\') {
		text_line[curchunk][len++] = '\\';
		NEWCHUNK(text_line);
	    }
	    text_line[curchunk][len++] = c;
	}

	/*
	 * null terminate current chunk
	 */
	text_line[curchunk][len] = '\0';

	if (res)
	    break;

	/*
	 * check for terminator
	 */
	if ((!strncmp(text_line[startchunk] + startpos, term, termlen)) &&
	    (!isalnum(*(text_line[startchunk] + startpos + termlen))) &&
	    (*(text_line[startchunk] + startpos + termlen) != '_')) {
	    if (strlen(text_line[startchunk] + startpos) == termlen) {
		current_line++;
	    } else {
		/*
		 * put back trailing portion after terminator
		 */
		myungetc(c);	/* some operating systems give EOF only once */

		for (i = curchunk; i > startchunk; i--)
		    add_input(text_line[i]);
		add_input(text_line[startchunk] + startpos + termlen);
	    }

	    /*
	     * remove terminator from last chunk
	     */
	    text_line[startchunk][startpos] = '\0';

	    /*
	     * insert text block into input stream
	     */
	    myungetc('\0');
	    myungetc('"');
	    for (i = startchunk; i >= 0; i--)
		add_input(text_line[i]);

	    res = 1;
	    break;
	} else {
	    /*
	     * only report end of file in text block, if not an include file
	     */
	    if (c == EOF && inctop == 0) {
		res = -1;
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
		    break;
		}
		text_line[++curchunk] =
		    (char *) DXALLOC(MAXCHUNK, 128, "text_block");
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
	FREE(text_line[curchunk]);

    return res;
}

static void skip_line()
{
    int c;

    while (((c = mygetc()) != '\n') && (c != EOF)) {
	/* empty while */
    }
    if (c == '\n') {
	myungetc(c);
    }
}

static void skip_comment()
{
    int comment_start;
    int c = '*';

    for (;;) {
	while ((comment_start = c, c = mygetc()) != '*') {
	    if (c == EOF) {
		lexerror("End of file in a comment");
		return;
	    }
	    if (c == '\n') {
		nexpands = 0;
		current_line++;
	    }
	}
	if (comment_start == '/')
	    yywarn("/* found in comment.");
	do {
	    if ((c = mygetc()) == '/')
		return;
	    if (c == '\n') {
		nexpands = 0;
		current_line++;
	    }
	} while (c == '*');
    }
}

#define TRY(c, t) if (gobble(c)) return t
#define TRY_ASSIGN_EQ(c, v) if (gobble('=')) { yylval.number = v; return L_ASSIGN; } else return c

static void
deltrail P1(char *, sp)
{
    char *p;

    p = sp;
    if (!*p) {
	lexerror("Illegal # command");
    } else {
	while (*p && !isspace(*p))
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

static void handle_pragma P1(char *, str)
{
    extern int code_size();

    if (strcmp(str, "strict_types") == 0) {
#ifndef IGNORE_STRICT_PRAGMA
	pragmas |= PRAGMA_STRICT_TYPES;
#endif
    } else if (strcmp(str, "save_types") == 0) {
	pragmas |= PRAGMA_SAVE_TYPES;
#ifdef SAVE_BINARIES
    } else if (strcmp(str, "save_binary") == 0) {
	pragmas |= PRAGMA_SAVE_BINARY;
#endif
    } else if (strcmp(str, "warnings") ==0 ) {
	pragmas |= PRAGMA_WARNINGS;
    } else if (strncmp(str, "optimize", 8) == 0) {
        pragmas |= PRAGMA_OPTIMIZE;
	optimization = 0;
	if (str[8] == ' ') {
	  if (strcmp(str + 9, "all") == 0) {
	    optimization = OPTIMIZE_ALL;
	  } else if (strcmp(str + 9, "high") == 0) {
	    optimization = OPTIMIZE_HIGH;
	  }
	}
    } else yywarn("unknown pragma, ignored");
}

#ifdef NEW_FUNCTIONS
static int function_flag = 0;

static int old_func() {
    add_input(yytext);
    yylval.context = function_context;
    function_context.num_parameters = 0;
    function_context.num_locals = 0;
    return L_FUNCTION_OPEN;
}
#endif

static int yylex1()
{
    static char partial[MAXLINE + 5];	/* extra 5 for safety buffer */
    static char terminator[MAXLINE + 5];
    int is_float;
    float myreal;
    char *partp;

    register char *yyp;		/* Xeno */
    register int c;		/* Xeno */

    partp = partial;		/* Xeno */
    partial[0] = 0;		/* Xeno */

    for (;;) {
	if (lex_fatal) {
	    return -1;
	}
	switch (c = mygetc()) {
	case EOF:
	    if (inctop) {
		struct incstate *p;

		p = inctop;
		fclose(yyin);
/*fprintf(stderr, "popping to %s\n", p->file);*/
		save_file_info(current_file_id, current_line - current_line_saved);
		current_line_saved = p->line - 1;
		/* add the lines from this file, and readjust to be relative
		   to the file we're returning to */
		current_line_base += current_line - current_line_saved;
		free_string(current_file);
		nexpands = 0;
		
		current_file = p->file;
		current_file_id = p->file_id;
		current_line = p->line;
		
		yyin = p->yyin;
		slast = p->slast;
		lastchar = p->lastchar;
		inctop = p->next;
		incnum--;
		FREE((char *) p);
		break;
	    }
	    if (iftop) {
		struct ifstate *p = iftop;
				
		yyerror(p->state == EXPECT_ENDIF ? "Missing #endif" : "Missing #else/#elif");
		while (iftop) {
		    p = iftop;
		    iftop = p->next;
		    FREE((char *) p);
		}
	    }
	    return -1;
	case '\n':
	    {
		nexpands = 0;
		current_line++;
		total_lines++;
	    }
	case ' ':
	case '\t':
	case '\f':
	case '\v':
	    break;
	case '+':
	    TRY('+', L_INC);
	    TRY_ASSIGN_EQ('+', F_ADD_EQ);
	case '-':
	    TRY('>', L_ARROW);
	    TRY('-', L_DEC);
	    TRY_ASSIGN_EQ('-', F_SUB_EQ);
	case '&':
	    TRY('&', L_LAND);
	    TRY_ASSIGN_EQ('&', F_AND_EQ);
	case '|':
	    TRY('|', L_LOR);
	    TRY_ASSIGN_EQ('|', F_OR_EQ);
	case '^':
	    TRY_ASSIGN_EQ('^', F_XOR_EQ);
	case '<':
	    if (gobble('<')) {
		TRY_ASSIGN_EQ(L_LSH, F_LSH_EQ);
	    }
	    TRY('=', L_LE);
	    return c;
	case '>':
	    if (gobble('>')) {
		TRY_ASSIGN_EQ(L_RSH, F_RSH_EQ);
	    }
	    TRY('=', L_GE);
	    return c;
	case '*':
	    TRY_ASSIGN_EQ('*', F_MULT_EQ);
	case '%':
	    TRY_ASSIGN_EQ('%', F_MOD_EQ);
	case '/':
	    if (gobble('*')) {
		skip_comment();
		break;
	    } else if (gobble('/')) {
		skip_line();
		break;
	    }
	    TRY_ASSIGN_EQ('/', F_DIV_EQ);
	case '=':
	    TRY('=', L_EQ);
	    yylval.number = F_ASSIGN;
	    return L_ASSIGN;
	case '(':
            while (((c = mygetc()) != EOF) && isspace(c)) {
                if (c == '\n') {
                    current_line++;
                }
            }
            if (c == '{')  return L_ARRAY_OPEN;
            if (c == '[')  return L_MAPPING_OPEN;
            if (c != ':') {
		slast = lastchar;
                myungetc(c);
                return '(';
            }

	    /* this is tricky; we have to check for (:: */
	    if (!gobble(':')){
#ifdef NEW_FUNCTIONS
		while ((c = mygetc()) != EOF && isspace(c)){
		    if (c == '\n'){
			current_line++;
		    }
		}
		if (isalpha(c) || c == '_'){
		    function_flag = 1;
		    goto parse_identifier;
		}
		slast = lastchar;
		myungetc(c);
		yylval.context = function_context;
		function_context.num_parameters = 0;
		function_context.num_locals = 0;
#endif
		return L_FUNCTION_OPEN;
	    }
	    /* put the :: back and return a ( */
	    myungetc(':');
	    myungetc(':');
	    return '(';
#ifdef NEW_FUNCTIONS
	case '$':
	    if (function_context.num_parameters == -1) {
		yyerror("$var illegal outside of function.");
		return '$';
	    } else {
		if (!isdigit(c = mygetc())) {
		    myungetc(c);
		    return '$';
		}
		yyp = yytext;
		SAVEC;
		for (;;) {
		    if (!isdigit(c = mygetc())) break;
		    SAVEC;
		}
		myungetc(c);
		*yyp = 0;
		yylval.number = atoi(yytext) - 1;
		if (yylval.number > 255)
		    yyerror("only 255 parameters allowed.");
		if (yylval.number >= function_context.num_parameters)
		    function_context.num_parameters = yylval.number + 1;
		return L_PARAMETER;
	    }
#endif
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
            if (!gobble('?'))
                return c;
            c = mygetc();
            switch (c) {
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
                    myungetc(c);
                    myungetc('?');
                    return '?';
            }
#endif
	case '!':
	    TRY('=', L_NE);
	    return L_NOT;
	case ':':
	    TRY(':', L_COLON_COLON);
	    return ':';
	case '.':
	    TRY('.', L_RANGE);
	    goto badlex;
	case '#':
	    if (lastchar == '\n') {
		char *sp = 0;
		int quote;

		yyp = yytext;
		do {
		    c = mygetc();
		} while (isspace(c));
		for (quote = 0;;) {

		    if (c == '"')
			quote ^= 1;
		    if (!quote && c == '/') {	/* gc - handle comments
						 * cpp-like! 1.6.91 */
			if (gobble('*')) {
			    skip_comment();
			    c = mygetc();
			} else if (gobble('/')) {
			    skip_line();
			    c = mygetc();
			}
		    }
		    if (!sp && isspace(c))
			sp = yyp;
		    if (c == '\n' || c == EOF)
			break;
		    SAVEC;
		    c = mygetc();
		}
		if (sp) {
		    *sp++ = 0;
		    while (isspace(*sp))
			sp++;
		} else {
		    sp = yyp;
		}
		*yyp = 0;
		if (strcmp("define", yytext) == 0) {
		    handle_define(sp);
		} else if (strcmp("if", yytext) == 0) {
		    int cond;

		    myungetc(0);
		    add_input(sp);
		    cond = cond_get_exp(0);
		    if (mygetc()) {
			yyerror("Condition too complex in #if");
			while (mygetc());
		    } else
			handle_cond(cond);
		} else if (strcmp("ifdef", yytext) == 0) {
		    deltrail(sp);
		    handle_cond(lookup_define(sp) != 0);
		} else if (strcmp("ifndef", yytext) == 0) {
		    deltrail(sp);
		    handle_cond(lookup_define(sp) == 0);
		} else if (strcmp("elif", yytext) == 0) {
		    if (iftop) {
			if (iftop->state == EXPECT_ELSE) {
			    /* last cond was false... */
			    int cond;
			    struct ifstate *p = iftop;

			    /* pop previous condition */
			    iftop = p->next;
			    FREE((char *) p);

			    myungetc(0);
			    add_input(sp);
			    cond = cond_get_exp(0);
			    if (mygetc()) {
				yyerror("Condition too complex in #elif");
				while (mygetc());
			    } else {
				handle_cond(cond);
			    }
			} else {/* EXPECT_ENDIF */
			    /*
			     * last cond was true...skip to end of
			     * conditional
			     */
			    skip_to("endif", (char *) 0);
			}
		    } else {
			yyerror("Unexpected #elif");
		    }
		} else if (strcmp("else", yytext) == 0) {
		    if (iftop) {
			if (iftop->state == EXPECT_ELSE) {
			    iftop->state = EXPECT_ENDIF;
			} else {
			    skip_to("endif", (char *) 0);
			}
		    } else {
			yyerror("Unexpected #else");
		    }
		} else if (strcmp("endif", yytext) == 0) {
		    if (iftop && (iftop->state == EXPECT_ENDIF ||
				  iftop->state == EXPECT_ELSE)) {
			struct ifstate *p = iftop;

			iftop = p->next;
			FREE((char *) p);
		    } else {
			yyerror("Unexpected #endif");
		    }
		} else if (strcmp("undef", yytext) == 0) {
		    struct defn *d;

		    deltrail(sp);
		    if ((d = lookup_define(sp)))
			d->flags |= DEF_IS_UNDEFINED;
		} else if (strcmp("echo", yytext) == 0) {
		    fprintf(stderr, "%s\n", sp);
		} else if (strcmp("include", yytext) == 0) {
		    current_line++;
		    handle_include(sp);
		    break;
		} else if (strcmp("pragma", yytext) == 0) {
		    handle_pragma(sp);
		} else {
		    yyerror("Unrecognised # directive");
		}
		myungetc('\n');
		break;
	    } else
		goto badlex;
	case '\'':

	    yylval.number = mygetc();
	    if (yylval.number == '\\') {
		int tmp = mygetc();

		if (tmp == 'n') {	/* fix from Wing@TMI 92/08/21 */
		    yylval.number = '\n';
		} else if (tmp == 't') {
		    yylval.number = '\t';
		} else if (tmp == 'r') {
		    yylval.number = '\r';
		} else if (tmp == 'b') {
		    yylval.number = '\b';
		} else {
		    if (tmp != '\'' && tmp != '\\')
			yywarn("Unknown \\x character.");
		    yylval.number = tmp;
		}
	    }
	    if (!gobble('\''))
		yyerror("Illegal character constant");
	    return L_NUMBER;
	case '@':
	    {
		int rc;
		int tmp;

		tmp = mygetc();
		if (tmp != '@') {
		    /* check for Robocoder's @@ block */
		    myungetc(tmp);
		}
		if (!get_terminator(terminator)) {
		    yyerror("Illegal terminator");
		}
		if (tmp == '@') {
		    rc = get_array_block(terminator);

		    if (rc > 0) {
			return mygetc();
		    } else if (rc == -1) {
			yyerror("End of file in array block");
		    } else {	/* if rc == -2 */
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
			nbuf -= n;

			return L_STRING;
		    } else if (rc == -1) {
			yyerror("End of file in text block");
		    } else {	/* if (rc == -2) */
			yyerror("Text block exceeded maximum length");
		    }
		}
	    }
	    break;
	case '"':
	    yyp = yytext;
	    *yyp++ = c;
	    for (;;) {
		c = mygetc();
		if (c == EOF) {
		    lexerror("End of file in string");
		    return EOF;
		}
#if 0
		else if (c == '\n') {
		    lexerror("Newline in string");
		    return EOF;
		}
#endif
		SAVEC;
		if (c == '"')
		    break;
		if (c == '\\') {
		    c = mygetc();
		    if (c == '\n') {
			yyp--;
			current_line++;
			total_lines++;
		    } else if (c == EOF) {
			myungetc(c);	/* some operating systems give EOF
					 * only once */
		    } else
			*yyp++ = c;
		}
	    }
	    *yyp = 0;
	    yylval.string = scratch_copy_string(yytext);
	    return L_STRING;

	case '0':
	    c = mygetc();
	    if (c == 'X' || c == 'x') {
		yyp = yytext;
		for (;;) {
		    c = mygetc();
		    SAVEC;
		    if (!isxdigit(c))
			break;
		}
		myungetc(c);
		yylval.number = (int) strtol(yytext, (char **) NULL, 0x10);
		return L_NUMBER;
	    }
	    myungetc(c);
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
		c = mygetc();
		if (c == '.') {
		    if (!is_float) {
			is_float = 1;
		    } else {
			is_float = 0;
			myungetc(c);
			break;
		    }
		} else if (!isdigit(c))
		    break;
		SAVEC;
	    }
	    myungetc(c);
	    *yyp = 0;
	    if (is_float) {
		sscanf(yytext, "%f", &myreal);
		yylval.real = (float)myreal;
		return L_REAL;
	    } else {
		yylval.number = atoi(yytext);
		return L_NUMBER;
	    }
	default:
	    if (isalpha(c) || c == '_') {
		int r;

parse_identifier:
		yyp = yytext;
		*yyp++ = c;
		for (;;) {
		    c = mygetc();
		    if (!isalunum(c))
			break;
		    SAVEC;
		}
#ifdef WANT_MISSING_SPACE_BUG
		while (c == ' ')
		    c = mygetc();
#endif
		*yyp = 0;
		if (c == '#') {
		    if (mygetc() != '#')
			lexerror("Single '#' in identifier -- use '##' for token pasting");
		    myungetc(c);
		    myungetc(c);
		    if (!expand_define()) {
#ifdef NEW_FUNCTIONS
			if (partp + (r = strlen(yytext)) + (function_flag ? 3 : 0) - partial > MAXLINE)
			    lexerror("Pasted token is too long");
			if (function_flag){
			    strcpy(partp, "(: ");
			    partp += 3;
			}
#else
                        if (partp + (r = strlen(yytext)) - partial > MAXLINE)
                            lexerror("Pasted token is too long");
#endif
			strcpy(partp, yytext);
			partp += r;
			mygetc();
			mygetc();
		    }
		} else if (partp != partial) {
		    myungetc(c);
		    if (!expand_define())
			add_input(yytext);
		    while ((c = mygetc()) == ' ');
		    myungetc(c);
		    add_input(partial);
		    partp = partial;
		    partial[0] = 0;
		} else {
		    myungetc(c);
		    if (!expand_define()) {
			struct ident_hash_elem *ihe;
			if (ihe = lookup_ident(yytext)) {
			    if (ihe->token & IHE_RESWORD) {
#ifdef NEW_FUNCTIONS
				if (function_flag){
				    function_flag = 0;
				    add_input(yytext);
				    yylval.context = function_context;
				    function_context.num_parameters = 0;
				    function_context.num_locals = 0;
				    return L_FUNCTION_OPEN;
				}
#endif
				yylval.number = ihe->sem_value;
				return ihe->token & TOKEN_MASK;
			    }
#ifdef NEW_FUNCTIONS
			    if (function_flag){
				int val;

				function_flag = 0;
				while ((c = mygetc()) == ' ');
				myungetc(c);
				if (c != ':' && c != ',') return old_func();
				if ((val=ihe->dn.local_num) >= 0) {
				    if (c == ',') return old_func();
				    yylval.number = (val << 8) | FP_L_VAR;
				} else if ((val=ihe->dn.global_num) >= 0) {
				    if (c == ',') return old_func();
				    yylval.number = (val << 8) | FP_G_VAR;
				} else if ((val=ihe->dn.simul_num) >=0) {
				    yylval.number = (val << 8)|FP_SIMUL_EFUN;
				} else if ((val=ihe->dn.function_num) >=0) {
				    yylval.number = (val << 8)|FP_LFUN;
				} else if ((val=ihe->dn.efun_num) >=0) {
				    yylval.number = (val << 8)|FP_EFUN;
				} else return old_func();
				return L_NEW_FUNCTION_OPEN;
			    }
#endif
			    yylval.ihe = ihe;
			    return L_DEFINED_NAME;
			}
			yylval.string = scratch_copy(yytext);
			return L_IDENTIFIER;
		    }
#ifdef NEW_FUNCTIONS
		    if (function_flag) {
			function_flag = 0;
			add_input("(:");
		    }
#endif
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
	fprintf(stderr, "partial = %s\n", partial);
	yyerror(buff);
#endif
	return ' ';
    }
}

int yylex()
{
    int r;

    yytext[0] = 0;
    r = yylex1();
/*    fprintf(stderr, "lex=%d(%s) ", r, yytext);*/
    return r;
}

extern YYSTYPE yylval;

void end_new_file()
{
    while (inctop) {
	struct incstate *p;

	p = inctop;
	fclose(yyin);
	free_string(current_file);
	current_file = p->file;
	yyin = p->yyin;
	inctop = p->next;
	FREE((char *) p);
    }
    inctop = 0;
    while (iftop) {
	struct ifstate *p;

	p = iftop;
	iftop = p->next;
	FREE((char *) p);
    }
    if (defines_need_freed) {
	free_defines();
	defines_need_freed = 0;
    }
}

static void add_quoted_define P3(char *, def, char *, val, int, flags)
{
    char save_buf[1024];

    strcpy(save_buf, "\"");
    strcat(save_buf, val);
    strcat(save_buf, "\"");
    add_define(def, -1, save_buf, flags);
}

void add_predefines()
{
    char save_buf[80];
    int i;

    add_predefine("LPC3", -1, "");
    add_predefine("MUDOS", -1, "");
    get_version(save_buf);
    add_quoted_predefine("__VERSION__", save_buf);
    sprintf(save_buf, "%d", PORTNO);
    add_predefine("__PORT__", -1, save_buf);
    for (i = 0; i < 2 * NUM_OPTION_DEFS; i += 2) {
	add_predefine(option_defs[i], -1, option_defs[i+1]);
    }
    add_quoted_predefine("__ARCH__", ARCH);
#ifdef COMPILER
    add_quoted_predefine("__COMPILER__", COMPILER);
#endif
#ifdef OPTIMIZE
    add_quoted_predefine("__OPTIMIZATION__", OPTIMIZE);
#endif
    /* Backwards Compat */
    add_quoted_predefine("SAVE_EXTENSION", SAVE_EXTENSION);
#ifndef CDLIB
    add_quoted_predefine("MUD_NAME", MUD_NAME);
#endif
#ifndef NO_UIDS
    add_predefine("USE_EUID", -1, "");
#endif
#ifndef DISALLOW_BUFFER_TYPE
    add_predefine("HAS_BUFFER_TYPE", -1, "");
#endif
#ifdef SOCKET_EFUNS
    add_predefine("HAS_SOCKETS", -1, "");
#endif
#ifndef NO_SHADOWS
    add_predefine("HAS_SHADOWS", -1, "");
#endif
#if (defined(DEBUGMALLOC) && defined(DEBUGMALLOC_EXTENSIONS))
    add_predefine("HAS_DEBUGMALLOC", -1, "");
#endif
#ifdef MATH
    add_predefine("HAS_MATH", -1, "");
#endif
#ifdef PROFILE_FUNCTIONS
    add_predefine("HAS_PROFILE_FUNCTIONS", -1, "");
#endif
#ifdef MATRIX
    add_predefine("HAS_MATRIX", -1, "");
#endif
#ifdef F_ED
    add_predefine("HAS_ED", -1, "");
#endif
#ifdef F_PRINTF
    add_predefine("HAS_PRINTF", -1, "");
#endif
#ifdef PRIVS
    add_predefine("HAS_PRIVS", -1, "");
#endif
#ifdef EACH
    add_predefine("HAS_EACH", -1, "");
#endif
#ifdef CACHE_STATS
    add_predefine("HAS_CACHE_STATS", -1, "");
#endif
#if (defined(RUSAGE) || defined(GET_PROCESS_STATS) || defined(TIMES)) || defined(LATTICE)
    add_predefine("HAS_RUSAGE", -1, "");
#endif
#ifdef DEBUG_MACRO
    add_predefine("HAS_DEBUG_LEVEL", -1, "");
#endif
#ifdef OPCPROF
    add_predefine("HAS_OPCPROF", -1, "");
#endif
#ifdef MUDLIB_ERROR_HANDLER
    add_predefine("HAS_MUDLIB_ERROR_HANDLER", -1, "");
#endif
#ifndef NO_MUDLIB_STATS
    add_predefine("HAS_MUDLIB_STATS", -1, "");
#endif
#ifndef NO_LIGHT
    add_predefine("HAS_LIGHT", -1, "");
#endif
}

void start_new_file P1(FILE *, f)
{
    struct lpc_predef_s *tmpf;
    char *gifile;

    if (defines_need_freed) {
	free_defines();
    }
    defines_need_freed = 1;
    if (current_file) {
	char *dir;
	char *tmp;
	int ln;

	ln = strlen(current_file);
	dir = (char *) DMALLOC(ln + 4, 64, "start_new_file");
	dir[0] = '"';
	dir[1] = '/';
	memcpy(dir + 2, current_file, ln);
	dir[ln + 2] = '"';
	dir[ln + 3] = 0;
	add_define("__FILE__", -1, dir, 0);
	tmp = strrchr(dir, '/');
	tmp[1] = '"';
	tmp[2] = 0;
	add_define("__DIR__", -1, dir, 0);
	FREE(dir);
    }
    for (tmpf = lpc_predefs; tmpf; tmpf = tmpf->next) {
	char namebuf[NSIZE];
	char mtext[MLEN];

	*mtext = '\0';
	sscanf(tmpf->flag, "%[^=]=%[ -~=]", namebuf, mtext);
	if (strlen(namebuf) >= NSIZE)
	    fatal("NSIZE exceeded");
	if (strlen(mtext) >= MLEN)
	    fatal("MLEN exceeded");
	add_define(namebuf, -1, mtext, 0);
    }
    yyin = f;
    slast = '\n';
    lastchar = '\n';
    lex_fatal = 0;
    nbuf = 0;
    outp = defbuf + DEFMAX;
    pragmas = DEFAULT_PRAGMAS;
    optimization = 0;
    nexpands = 0;
    current_line = 1;
    current_line_base = 0;
    current_line_saved = 0;
    if (strlen(GLOBAL_INCLUDE_FILE)) {
	/* need a writable copy */
	gifile = string_copy(GLOBAL_INCLUDE_FILE);
	handle_include(gifile);
	FREE(gifile);
    }
}

char *query_instr_name P1(int, instr)
{
    char *name;

    name = instrs[instr].name;
    if (name) {
	return name;
    } else {
	sprintf(num_buf, "%d", instr);
	return num_buf;
    }
}

static void add_instr_name P3(char *, name, int, n, short, t)
{
    instrs[n].name = name;
    instrs[n].ret_type = t;
}

void init_num_args()
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
	    instrs[n].Default = predefs[i].Default;
	    instrs[n].ret_type = predefs[i].ret_type;
	    instrs[n].arg_index = predefs[i].arg_index;
	}
    }
    /*
     * eoperators have a return type now.  T_* is used instead of TYPE_*
     * since operators can return multiple types.
     */
    add_instr_name("<", F_LT, T_NUMBER);
    add_instr_name(">", F_GT, T_NUMBER);
    add_instr_name("<=", F_LE, T_NUMBER);
    add_instr_name(">=", F_GE, T_NUMBER);
    add_instr_name("==", F_EQ, T_NUMBER);
    add_instr_name("+=", F_ADD_EQ, T_ANY);
    add_instr_name("(void)+=", F_VOID_ADD_EQ, T_NUMBER);
    add_instr_name("!", F_NOT, T_NUMBER);
    add_instr_name("&", F_AND, T_POINTER | T_NUMBER);
    add_instr_name("&=", F_AND_EQ, T_NUMBER);
    add_instr_name("index", F_INDEX, T_ANY);
    add_instr_name("loop_cond", F_LOOP_COND, -1);
    add_instr_name("loop_incr", F_LOOP_INCR, -1);
    add_instr_name("while_dec", F_WHILE_DEC, -1);
    add_instr_name("indexed_lvalue", F_INDEXED_LVALUE, T_LVALUE);
    add_instr_name("global", F_GLOBAL, T_ANY);
    add_instr_name("local", F_LOCAL, T_ANY);
    add_instr_name("number", F_NUMBER, T_NUMBER);
    add_instr_name("real", F_REAL, T_REAL);
    add_instr_name("local_lvalue", F_LOCAL_LVALUE, T_LVALUE);
    add_instr_name("const1", F_CONST1, T_NUMBER);
    add_instr_name("subtract", F_SUBTRACT, T_NUMBER | T_REAL | T_POINTER);
    add_instr_name("(void)assign", F_VOID_ASSIGN, T_NUMBER);
    add_instr_name("assign", F_ASSIGN, T_ANY);
    add_instr_name("branch", F_BRANCH, -1);
    add_instr_name("bbranch", F_BBRANCH, -1);
    add_instr_name("byte", F_BYTE, T_NUMBER);
    add_instr_name("-byte", F_NBYTE, T_NUMBER);
    add_instr_name("bbranch_when_zero", F_BBRANCH_WHEN_ZERO, -1);
    add_instr_name("bbranch_when_non_zero", F_BBRANCH_WHEN_NON_ZERO, -1);
    add_instr_name("branch_when_zero", F_BRANCH_WHEN_ZERO, -1);
    add_instr_name("branch_when_non_zero", F_BRANCH_WHEN_NON_ZERO, -1);
    add_instr_name("pop", F_POP_VALUE, -1);
    add_instr_name("const0", F_CONST0, T_NUMBER);
#ifdef F_JUMP_WHEN_ZERO
    add_instr_name("jump_when_zero", F_JUMP_WHEN_ZERO, -1);
    add_instr_name("jump_when_non_zero", F_JUMP_WHEN_NON_ZERO, -1);
#endif
#ifdef F_LOR
    add_instr_name("||", F_LOR, -1);
    add_instr_name("&&", F_LAND, -1);
#endif
    add_instr_name("-=", F_SUB_EQ, T_ANY);
#ifdef F_JUMP
    add_instr_name("jump", F_JUMP, -1);
#endif
    add_instr_name("return", F_RETURN, -1);
    add_instr_name("sscanf", F_SSCANF, T_NUMBER);
    add_instr_name("parse_command", F_PARSE_COMMAND, T_NUMBER);
    add_instr_name("string", F_STRING, T_STRING);
    add_instr_name("call", F_CALL_FUNCTION_BY_ADDRESS, T_ANY);
    add_instr_name("aggregate_assoc", F_AGGREGATE_ASSOC, T_MAPPING);
    add_instr_name("break_point", F_BREAK_POINT, -1);
    add_instr_name("call_extra", F_CALL_EXTRA, -1);
    add_instr_name("aggregate", F_AGGREGATE, T_POINTER);
    add_instr_name("(::)", F_FUNCTION_CONSTRUCTOR, T_FUNCTION);
#ifndef NEW_FUNCTIONS
    add_instr_name("evaluate", F_EVALUATE, -1);
#endif
    add_instr_name("simul_efun", F_SIMUL_EFUN, T_ANY);
    add_instr_name("global_lvalue", F_GLOBAL_LVALUE, T_LVALUE);
    add_instr_name("|", F_OR, T_NUMBER);
    add_instr_name("<<", F_LSH, T_NUMBER);
    add_instr_name(">>", F_RSH, T_NUMBER);
    add_instr_name(">>=", F_RSH_EQ, T_NUMBER);
    add_instr_name("<<=", F_LSH_EQ, T_NUMBER);
    add_instr_name("^", F_XOR, T_NUMBER);
    add_instr_name("^=", F_XOR_EQ, T_NUMBER);
    add_instr_name("|=", F_OR_EQ, T_NUMBER);
    add_instr_name("+", F_ADD, T_ANY);
    add_instr_name("!=", F_NE, T_NUMBER);
    add_instr_name("catch", F_CATCH, T_ANY);
    add_instr_name("end_catch", F_END_CATCH, -1);
    add_instr_name("-", F_NEGATE, T_NUMBER | T_REAL);
    add_instr_name("~", F_COMPL, T_NUMBER);
    add_instr_name("++x", F_PRE_INC, T_NUMBER | T_REAL);
    add_instr_name("--x", F_PRE_DEC, T_NUMBER | T_REAL);
    add_instr_name("*", F_MULTIPLY, T_REAL | T_NUMBER | T_MAPPING);
    add_instr_name("*=", F_MULT_EQ, T_REAL | T_NUMBER | T_MAPPING);
    add_instr_name("/", F_DIVIDE, T_REAL | T_NUMBER);
    add_instr_name("/=", F_DIV_EQ, T_NUMBER | T_REAL);
    add_instr_name("%", F_MOD, T_NUMBER);
    add_instr_name("%=", F_MOD_EQ, T_NUMBER);
    add_instr_name("inc(x)", F_INC, -1);
    add_instr_name("dec(x)", F_DEC, -1);
    add_instr_name("x++", F_POST_INC, T_NUMBER | T_REAL);
    add_instr_name("x--", F_POST_DEC, T_NUMBER | T_REAL);
    add_instr_name("switch", F_SWITCH, -1);
    add_instr_name("break", F_BREAK, -1);
    add_instr_name("pop_break", F_POP_BREAK, -1);
    add_instr_name("range", F_RANGE, T_STRING | T_BUFFER | T_POINTER | T_NUMBER);
    add_instr_name("time_expression", F_TIME_EXPRESSION, -1);
    add_instr_name("end_time_expression", F_END_TIME_EXPRESSION, T_NUMBER);
    instrs[F_RANGE].type[0] = T_POINTER | T_STRING;
}

char *get_f_name P1(int, n)
{
    if (instrs[n].name)
	return instrs[n].name;
    else {
	static char buf[30];

	sprintf(buf, "<OTHER %d>", n);
	return buf;
    }
}

#define GETALPHA(p, q, m) \
    while(isalunum(*p)) {\
	*q = *p++;\
	if (q < (m))\
	    q++;\
	else {\
	    lexerror("Name too long");\
	    return;\
	}\
    }\
    *q++ = 0

/* kludge to allow token pasting */
#define GETDEFINE(p, q, m) \
    while (isalunum(*p) || (*p == '#')) {\
       *q = *p++; \
       if (q < (m)) \
           q++; \
       else { \
           lexerror("Name too long"); \
           return; \
       } \
    } \
    *q++ = 0

static int cmygetc()
{
    int c;

    for (;;) {
	c = mygetc();
	if (c == '/') {
	    if (gobble('*'))
		skip_comment();
	    else if (gobble('/'))
		skip_line();
	    else
		return c;
	} else
	    return c;
    }
}

static void refill()
{
    char *p;
    int c;

    p = yytext;
    do {
	c = cmygetc();
	if (p < yytext + MAXLINE - 5)
	    *p++ = c;
	else {
	    lexerror("Line too long");
	    break;
	}
    } while (c != '\n' && c != EOF);
    p[-1] = ' ';
    *p = 0;
    nexpands = 0;
    current_line++;
}

static void handle_define P1(char *, yyt)
{
    char namebuf[NSIZE];
    char args[NARGS][NSIZE];
    char mtext[MLEN];
    char *p, *q;

    p = yyt;
    strcat(p, " ");
    q = namebuf;
    GETALPHA(p, q, namebuf + NSIZE - 1);
    if (*p == '(') {		/* if "function macro" */
	int arg;
	int inid;
	char *ids = (char *) NULL;

	p++;			/* skip '(' */
	SKIPWHITE;
	if (*p == ')') {
	    arg = 0;
	} else {
	    for (arg = 0; arg < NARGS;) {
		q = args[arg];
		GETDEFINE(p, q, args[arg] + NSIZE - 1);
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
	p++;			/* skip ')' */
	for (inid = 0, q = mtext; *p;) {
	    if (isalunum(*p)) {
		if (!inid) {
		    inid++;
		    ids = p;
		}
	    } else {
		if (inid) {
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
	add_define(namebuf, arg, mtext, 0);
    } else if (isspace(*p)) {
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
	add_define(namebuf, -1, mtext, 0);
    } else {
	lexerror("Illegal macro symbol");
    }
    return;
}

static void myungetc P1(int, c)
{
    *--outp = c;
    nbuf++;
}

/* IDEA: linked buffers, to allow "unlimited" buffer expansion */
static void add_input P1(char *, p)
{
    int l = strlen(p);

/*if (l > 2)
fprintf(stderr, "add '%s'\n", p);*/
    if (nbuf + l >= DEFMAX - 10) {
	lexerror("Macro expansion buffer overflow");
	return;
    }
    outp -= l;
    nbuf += l;
    strncpy(outp, p, l);
}

/* must be a power of four */
#define DEFHASH 64
static struct defn *defns[DEFHASH];

#define defhash(s) (whashstr((s), 10) & (DEFHASH - 1))

static void add_define P4(char *, name, int, nargs, char *, exps, int, flags)
{
    struct defn *p;
    int h;

    if ((p = lookup_define(name))) {
	if (nargs != p->nargs || strcmp(exps, p->exps)) {
	    char buf[200 + NSIZE];

	    sprintf(buf, "Warning: redefinition of #define %s\n", name);
	    yywarn(buf);
	}
	p->exps = (char *)DREALLOC(p->exps, strlen(exps) + 1, 65, "add_define: redef");
	strcpy(p->exps, exps);
	p->nargs = nargs;
    } else {
	p = (struct defn *) DXALLOC(sizeof(struct defn), 65, "add_define: def");
	p->name = (char *) DXALLOC(strlen(name) + 1, 65, "add_define: def name");
	strcpy(p->name, name);
	p->exps = (char *) DXALLOC(strlen(exps) + 1, 65, "add_define: def exps");
	strcpy(p->exps, exps);
	p->flags = flags;
	p->nargs = nargs;
	h = defhash(name);
	p->next = defns[h];
	defns[h] = p;
    }
    /* fprintf(stderr, "define '%s' %d '%s'\n", name, nargs, exps); */
}

static void free_defines()
{
    struct defn *p, *q;
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

static struct defn *
     lookup_define P1(char *, s)
{
    struct defn *p;
    int h;

    h = defhash(s);
    for (p = defns[h]; p; p = p->next)
	if (!(p->flags & DEF_IS_UNDEFINED) && strcmp(s, p->name) == 0)
	    return p;
    return 0;
}

#define SKIPW \
        do {\
	    c = cmygetc();\
	} while(isspace(c));


/* Check if yytext is a macro and expand if it is. */
static int expand_define()
{
    struct defn *p;
    char expbuf[DEFMAX];
    char *args[NARGS];
    char buf[DEFMAX];
    char *q, *e, *b;

    if (nexpands++ > EXPANDMAX) {
	lexerror("Too many macro expansions");
	return 0;
    }
    p = lookup_define(yytext);
    if (!p) {
	return 0;
    }
    if (p->nargs == -1) {
	add_input(p->exps);
    } else {
	int c, parcnt = 0, dquote = 0, squote = 0;
	int n;

	SKIPW;
	if (c != '(') {
	    yyerror("Missing '(' in macro call");
	    return 0;
	}
	SKIPW;
	if (c == ')')
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
			if (mygetc() != '#') {
			    lexerror("'#' expected");
			    return 0;
			}
		    }
		    break;
		case '\\':
		    if (squote || dquote) {
			*q++ = c;
			c = mygetc();
		    } break;
		case '\n':
		    if (squote || dquote) {
			lexerror("Newline in string");
			return 0;
		    } break;
		}
		if (c == ',' && !parcnt && !dquote && !squote) {
		    *q++ = 0;
		    args[++n] = q;
		} else if (parcnt < 0) {
		    *q++ = 0;
		    n++;
		    break;
		} else {
		    if (c == EOF) {
			lexerror("Unexpected end of file");
			return 0;
		    }
		    if (q >= expbuf + DEFMAX - 5) {
			lexerror("Macro argument overflow");
			return 0;
		    } else {
			*q++ = c;
		    }
		}
		if (!squote && !dquote)
		    c = cmygetc();
		else
		    c = mygetc();
	    }
	    if (n == NARGS) {
		lexerror("Maximum macro argument count exceeded");
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
			if (b >= buf + DEFMAX) {
			    lexerror("Macro expansion overflow");
			    return 0;
			}
		    }
		}
	    } else {
		*b++ = *e++;
		if (b >= buf + DEFMAX) {
		    lexerror("Macro expansion overflow");
		    return 0;
		}
	    }
	}
	*b++ = 0;
	add_input(buf);
    }
    return 1;
}

/* Stuff to evaluate expression.  I havn't really checked it. /LA
** Written by "J\"orn Rennecke" <amylaar@cs.tu-berlin.de>
*/
#define SKPW 	do c = mygetc(); while(isspace(c)); myungetc(c)

static int exgetc()
{
    register char c, *yyp;

    c = mygetc();
    while (isalpha(c) || c == '_') {
	yyp = yytext;
	do {
	    SAVEC;
	    c = mygetc();
	} while (isalunum(c));
	myungetc(c);
	*yyp = '\0';
	if (strcmp(yytext, "defined") == 0) {
	    /* handle the defined "function" in #if */
	    do
		c = mygetc();
	    while (isspace(c));
	    if (c != '(') {
		yyerror("Missing ( in defined");
		continue;
	    }
	    do
		c = mygetc();
	    while (isspace(c));
	    yyp = yytext;
	    while (isalunum(c)) {
		SAVEC;
		c = mygetc();
	    }
	    *yyp = '\0';
	    while (isspace(c))
		c = mygetc();
	    if (c != ')') {
		yyerror("Missing ) in defined");
		continue;
	    }
	    SKPW;
	    if (lookup_define(yytext))
		add_input(" 1 ");
	    else
		add_input(" 0 ");
	} else {
	    if (!expand_define())
		add_input(" 0 ");
	}
	c = mygetc();
    }
    return c;
}

#define BNOT   1
#define LNOT   2
#define UMINUS 3
#define UPLUS  4

#define MULT   1
#define DIV    2
#define MOD    3
#define BPLUS  4
#define BMINUS 5
#define LSHIFT 6
#define RSHIFT 7
#define LESS   8
#define LEQ    9
#define GREAT 10
#define GEQ   11
#define EQ    12
#define NEQ   13
#define BAND  14
#define XOR   15
#define BOR   16
#define LAND  17
#define LOR   18
#define QMARK 19

static char _optab[] =
{0, 4, 0, 0, 0, 26, 56, 0, 0, 0, 18, 14, 0, 10, 0, 22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 30, 50, 40, 74,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 70, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 63, 0, 1};
static char optab2[] =
{BNOT, 0, 0, LNOT, '=', NEQ, 7, 0, 0, UMINUS, 0, BMINUS, 10, UPLUS, 0, BPLUS, 10,
 0, 0, MULT, 11, 0, 0, DIV, 11, 0, 0, MOD, 11,
 0, '<', LSHIFT, 9, '=', LEQ, 8, 0, LESS, 8, 0, '>', RSHIFT, 9, '=', GEQ, 8, 0, GREAT, 8,
 0, '=', EQ, 7, 0, 0, 0, '&', LAND, 3, 0, BAND, 6, 0, '|', LOR, 2, 0, BOR, 4,
 0, 0, XOR, 5, 0, 0, QMARK, 1};

#define optab1 (_optab-' ')

static int cond_get_exp P1(int, priority)
{
    int c;
    int value, value2, x;

    do
	c = exgetc();
    while (isspace(c));
    if (c == '(') {

	value = cond_get_exp(0);
	do
	    c = exgetc();
	while (isspace(c));
	if (c != ')') {
	    yyerror("bracket not paired in #if");
	    if (!c)
		myungetc('\0');
	}
    } else if (ispunct(c)) {
	x = optab1[c];
	if (!x) {
	    yyerror("illegal character in #if");
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
	    yyerror("illegal unary operator in #if");
	    return 0;
	}
    } else {
	int base;

	if (!isdigit(c)) {
	    if (!c) {
		yyerror("missing expression in #if");
		myungetc('\0');
	    } else
		yyerror("illegal character in #if");
	    return 0;
	}
	value = 0;
	if (c != '0')
	    base = 10;
	else {
	    c = mygetc();
	    if (c == 'x' || c == 'X') {
		base = 16;
		c = mygetc();
	    } else
		base = 8;
	}
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
	    if (x > base)
		break;
	    value = value * base + x;
	    c = mygetc();
	}
	myungetc(c);
    }
    for (;;) {
	do
	    c = exgetc();
	while (isspace(c));
	if (!ispunct(c))
	    break;
	x = optab1[c];
	if (!x)
	    break;
	value2 = mygetc();
	for (;; x += 3) {
	    if (!optab2[x]) {
		myungetc(value2);
		if (!optab2[x + 1]) {
		    yyerror("illegal operator use in #if");
		    return 0;
		}
		break;
	    }
	    if (value2 == optab2[x])
		break;
	}
	if (priority >= optab2[x + 2]) {
	    if (optab2[x])
		myungetc(value2);
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
		yyerror("division by 0 in #if");
	    break;
	case MOD:
	    if (value2)
		value %= value2;
	    else
		yyerror("modulo by 0 in #if");
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
	    do
		c = exgetc();
	    while (isspace(c));
	    if (c != ':') {
		yyerror("'?' without ':' in #if");
		myungetc(c);
		return 0;
	    }
	    if (value) {
		cond_get_exp(1);
		value = value2;
	    } else
		value = cond_get_exp(1);
	    break;
	}
    }
    myungetc(c);
    return value;
}

void set_inc_list P1(char *, list)
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
    inc_list = (char **) DXALLOC(size * sizeof(char *), 68, "set_inc_list");
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
}

char *main_file_name()
{
    struct incstate *is;

    if (inctop == 0)
	return current_file;
    is = inctop;
    while (is->next)
	is = is->next;
    return is->file;
}

/* identifier hash table stuff, size must be an even power of two */
#define IDENT_HASH_SIZE 1024
#define IdentHash(s) (whashstr((s), 20) & (IDENT_HASH_SIZE - 1))

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

struct ident_hash_elem *lookup_ident P1(char *, name) {
    int h = IdentHash(name);
    struct ident_hash_elem *hptr, *hptr2;

    if (hptr = ident_hash_table[h]) {
	CHECK_ELEM(hptr, name, return hptr;);
	hptr2 = hptr->next;
	while (hptr2 != hptr) {
	    CHECK_ELEM(hptr2, name, ident_hash_table[h] = hptr2; return hptr2;);
	    hptr2 = hptr2->next;
	}
    }
    /* efun and asm are special cases, as they are redefinable keywords */
    if (strcmp("efun", name)==0) return (struct ident_hash_elem *)&efun_keyword;
    if (strcmp("asm", name)==0) return (struct ident_hash_elem *)&asm_keyword;
    return 0;
}

struct ident_hash_elem *find_or_add_perm_ident P1(char *, name) {
    int h = IdentHash(name);
    struct ident_hash_elem *hptr, *hptr2;

    if (hptr = ident_hash_table[h]) {
	if (!strcmp(hptr->name, name)) return hptr;
	hptr2 = hptr->next;
	while (hptr2 != hptr) {
	    if (!strcmp(hptr->name, name)) return hptr;
	    hptr2 = hptr2->next;
	}
	hptr = (struct ident_hash_elem *)DMALLOC(sizeof(struct ident_hash_elem), 0, "find_or_add_perm_ident:1");
	hptr->next = ident_hash_head[h]->next;
	ident_hash_head[h]->next = hptr;
	if (ident_hash_head[h] == ident_hash_tail[h])
	    ident_hash_tail[h] = hptr;
    } else {
	hptr = (ident_hash_table[h] = (struct ident_hash_elem *)DMALLOC(sizeof(struct ident_hash_elem), 0, "find_or_add_perm_ident:2"));
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
    return hptr;
}

struct linked_buf {
    struct linked_buf *next;
    char block[4096];
} *lnamebuf = 0;

int lb_index = 4096;

static char *alloc_local_name P1(char *, name) {
    int len = strlen(name)+1;
    char *res;

    if (lb_index + len > 4096) {
	struct linked_buf *new_buf;
	new_buf = (struct linked_buf *)DMALLOC(sizeof(struct linked_buf), 0, "alloc_local_name");
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

struct ident_hash_elem_list {
    struct ident_hash_elem_list *next;
    struct ident_hash_elem items[128];
} *ihe_list = 0;

#ifdef DEBUG
void dump_ihe P2(struct ident_hash_elem *, ihe, int, noisy) {
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

void debug_dump_ident_hash_table P1(int, noisy) {
    int zeros = 0;
    int i;
    struct ident_hash_elem *ihe, *ihe2;

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
    struct ident_hash_elem_list *ihel, *next;
    struct linked_buf *lnb, *lnbn;
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
	ident_dirty_list = ident_dirty_list->next_dirty;
    }

    for (i = 0; i < IDENT_HASH_SIZE; i++)
	if (ident_hash_table[i] = ident_hash_head[i])
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
#ifdef DEBUG
    debug_dump_ident_hash_table(0);
#endif
}

static struct ident_hash_elem *quick_alloc_ident_entry() {
    if (num_free) {
	num_free--;
	return &(ihe_list->items[num_free]);
    } else {
	struct ident_hash_elem_list *ihel;
	ihel = (struct ident_hash_elem_list *)DMALLOC(sizeof(struct ident_hash_elem_list), 0, "quick_alloc_ident_entry");
	ihel->next = ihe_list;
	ihe_list = ihel;
	num_free = 127;
	return &(ihe_list->items[127]);
    }
}

struct ident_hash_elem *
find_or_add_ident P2(char *, name, int, flags) {
    int h = IdentHash(name);
    struct ident_hash_elem *hptr, *hptr2;

    if (hptr = ident_hash_table[h]) {
	if (!strcmp(hptr->name, name)) {
	    if ((hptr->token & IHE_PERMANENT) && (flags & FOA_GLOBAL_SCOPE)
		&& (hptr->dn.function_num==-1)&&(hptr->dn.global_num==-1)) {
		hptr->next_dirty = ident_dirty_list;
		ident_dirty_list = hptr;
	    }
	    return hptr;
	}
	hptr2 = hptr->next;
	while (hptr2 != hptr) {
	    if (!strcmp(hptr2->name, name)) {
		if ((hptr2->token & IHE_PERMANENT)&&(flags & FOA_GLOBAL_SCOPE)
		 && (hptr2->dn.function_num==-1)&&(hptr2->dn.global_num==-1)){
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
    return hptr;
}

static void add_keyword P2(char *, name, keyword *, entry) {
    int h = IdentHash(name);

    if (ident_hash_table[h]) {
	entry->next = ident_hash_head[h]->next;
	ident_hash_head[h]->next = (struct ident_hash_elem *)entry;
	if (ident_hash_head[h] == ident_hash_tail[h])
	    ident_hash_tail[h] = (struct ident_hash_elem *)entry;
    } else {
	ident_hash_head[h] = (struct ident_hash_elem *)entry;
	ident_hash_tail[h] = (struct ident_hash_elem *)entry;
	ident_hash_table[h] = (struct ident_hash_elem *)entry;
	entry->next = (struct ident_hash_elem *)entry;
    }
    entry->token |= IHE_RESWORD;
}

void init_identifiers() {
    int i;
    struct ident_hash_elem *ihe;
    
    /* allocate all three tables together */
    ident_hash_table = (struct ident_hash_elem **)
	DMALLOC(sizeof(struct ident_hash_elem *) * IDENT_HASH_SIZE * 3,
		0, "init_identifiers");
    ident_hash_head = (struct ident_hash_elem **)&ident_hash_table[IDENT_HASH_SIZE];
    ident_hash_tail = (struct ident_hash_elem **)&ident_hash_table[2*IDENT_HASH_SIZE];

    /* clean all three tables */
    for (i=0; i<IDENT_HASH_SIZE * 3; i++) {
	ident_hash_table[i]=0;
    }
    /* add the reserved words */
    for (i=0; i<NELEM(reswords); i++) {
	add_keyword(reswords[i].word, &reswords[i]);
    }
    /* add the efuns */
    for (i=0; i<NELEM(predefs); i++) {
	ihe = find_or_add_perm_ident(predefs[i].word);
	ihe->token |= IHE_EFUN;
	ihe->sem_value++;
	ihe->dn.efun_num = i;
    }
}

