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

#include "config.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#ifdef SunOS_5
#include <stdlib.h>
#endif

#include "instrs.h"
#include "arch.h" /* put after config.h */
#include "lint.h"
#include "interpret.h"
#include "compiler.tab.h"
#include "opcodes.h"
#include "string.h"
#include "exec.h"
#include "lex.h"
#include "cc.h"

#define isalunum(c) (isalnum(c) || (c) == '_')
#define NELEM(a) (sizeof (a) / sizeof((a)[0]))

int current_line;
int total_lines;	/* Used to compute average compiled lines/s */
char *current_file;
int pragma_strict_types;	/* Force usage of strict types. */
int pragma_save_types;		/* Save argument types after compilation */
#ifdef SAVE_BINARIES
int pragma_save_binaries;	/* Save program as binary file */
#endif
struct lpc_predef_s *lpc_predefs=NULL;
extern char *argument_name;
extern char *xalloc();
static int number PROT((int)), ident PROT((char *)), string PROT((char *));
static int real PROT((double));
static int islocal PROT((char *));
static void handle_define PROT((char *));
static void free_defines PROT((void)), add_define PROT((char *, int, char *));
static int expand_define PROT((void));
static void add_input PROT((char *));
static void myungetc PROT((int));
static int lookup_resword PROT((char *));
static int cond_get_exp PROT((int));
static int exgetc();
static FILE *yyin;
static int lex_fatal;
static char **inc_list;
static int inc_list_size;
static int defines_need_freed = 0;

#define EXPANDMAX 25000
static int nexpands;

extern char *local_names[];
extern int current_number_of_locals;

extern char *string_copy();

#ifndef tolower
extern int tolower PROT((int));
#endif

void yyerror(), error();

#define MAXLINE 1024
static char yytext[MAXLINE];

/* static char partial[MAXLINE]; 
char *partp; */

static int slast, lastchar;

struct defn {
    struct defn *next;
    char *name;
    int undef;
    char *exps;
    int nargs;
};
struct defn *lookup_define();

static struct ifstate {
    struct ifstate *next;
    int state;
} *iftop = 0;
#define EXPECT_ELSE 1
#define EXPECT_ENDIF 2

static struct incstate {
    struct incstate *next;
    FILE *yyin;
    int line;
    char *file;
    int slast, lastchar;
    int pragma_strict_types;
} *inctop = 0;

#define DEFMAX 10000
static char defbuf[DEFMAX];
static int nbuf;
static char *outp;

void merge(name, dest)
    char *name, *dest;
{
    char *from;

    strcpy(dest, current_file);
    if ((from = strrchr(dest, '/')))   /* strip filename */
	*from = 0;
    else
	/* current_file was the file_name */
	/* include from the root directory */
	*dest = 0;

    from = name;
    while (*from == '/') {
	from++;
	*dest=0;	/* absolute path */
    }

    while(*from) {
	if(!strncmp(from, "../", 3)) {
	    char *tmp;
	    
	    if(*dest == 0) /* including from above mudlib is NOT allowed */
		break;
	    tmp = strrchr(dest, '/');
	    if(tmp == NULL) /* 1 component in dest */
		*dest = 0;
	    else
		*tmp = 0;
	    from +=3;   /* skip "../" */
	}
	else if(!strncmp(from, "./", 2)) {
	    from += 2;
	}
	else { /* append first component to dest */
	    char *q;
	    
	    if(*dest)
		strcat(dest, "/");    /* only if dest is not empty !! */
	    q = strchr(from, '/');
	    
	    if(q) { /* from has 2 or more components */
		while(*from=='/') /* find the start */
		    from++;
		strncat(dest, from, q - from);
		for(from = q+1;*from=='/';from++)
		    ;
	    }
	    else {
		/* this was the last component */
		strcat(dest, from);
		break;
	    }
	}
    }
}

static INLINE int
mygetc()
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
gobble(c)
int c;
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
lexerror(s)
char *s;
{
    yyerror(s);
    lex_fatal++;
}

static int
skip_to(token, atoken)
char *token, *atoken;
{
    char b[20], *p;
    int c;
    int nest;

    for(nest = 0;;) {
	c = mygetc();
	if (c == '#') {
	    do {
		c = mygetc();
	    } while(isspace(c));
	    for(p = b; c != '\n' && !isspace(c) && c != EOF; ) {
		if (p < b+sizeof b-1)
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
		    myungetc(c); add_input(b); myungetc('#');
		    return 1;
		} else if (atoken && strcmp(b, atoken) == 0) {
		    myungetc(c); add_input(b); myungetc('#');
		    return 0;
                } else if (strcmp(b, "elif") == 0) {
		    myungetc(c); add_input(b); myungetc('#');
                    return (atoken == 0);
		}
	    }
	} else {
/*fprintf(stderr, "skipping (%d) %c", c, c);*/
            while (c != '\n' && c != EOF) {
		c = mygetc();
/*fprintf(stderr, "%c", c);*/
	    } 
	    if (c == EOF) {
		lexerror("Unexpected end of file while skipping");
		return 1;
	    }
	}
	if (inctop == 0)
	    store_line_number_info();
	current_line++;
	total_lines++;
    }
}

static void
handle_cond(c)
int c;
{
    struct ifstate *p;

/*fprintf(stderr, "cond %d\n", c);*/
    if (!c)
        skip_to("else", "endif");
    p = (struct ifstate *)DXALLOC(sizeof(struct ifstate), 60, "handle_cond");
    p->next = iftop;
    iftop = p;
    p->state = c ? EXPECT_ENDIF : EXPECT_ELSE;
}

static FILE *
inc_open(buf, name)
    char *buf, *name;
{
    extern save_include();
    FILE *f;
    int i;
    char *p;

    merge(name, buf);
    if ((f = fopen(buf, "r")) != NULL) {
	save_include(buf);
	return f;
    }
    /*
     * Search all include dirs specified.
     */
    for (p=strchr(name, '.'); p; p = strchr(p+1, '.')) {
	if (p[1] == '.')
	    return NULL;
    }
    for (i=0; i < inc_list_size; i++) {
      sprintf(buf,"%s/%s",inc_list[i],name);
      if ((f = fopen(buf, "r"))) {
	save_include(buf);
	return f;
      }
    }
    return NULL;
}

static void
handle_include(name)
char *name;
{
    char *p;
    char buf[1024];
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
	    while(isspace(*q))
		q++;
	    handle_include(q);
	} else {
	    yyerror("Missing leading \" or < in #include");
	}
	return;
    }
    delim = *name++ == '"' ? '"' : '>';
    for(p = name; *p && *p != delim; p++)
	;
    if (!*p) {
	yyerror("Missing trailing \" or > in #include");
	return;
    }
    if (strlen(name) > sizeof(buf) - 100) {
	yyerror("Include name too long.");
	return;
    }
    *p = 0;
    if ((f = inc_open(buf, name)) != NULL) {
	is = (struct incstate *)
		DXALLOC(sizeof(struct incstate), 61, "handle_include: 1");
	is->yyin = yyin;
	is->line = current_line;
	is->file = current_file;
	is->slast = slast;
	is->lastchar = lastchar;
	is->next = inctop;
	is->pragma_strict_types = pragma_strict_types;
	pragma_strict_types = 0;
	inctop = is;
	current_line = 1;
	current_file = DXALLOC(strlen(buf)+1, 62, "handle_include: 2");
	strcpy(current_file, buf);
	slast = lastchar = '\n';
	yyin = f;
/*fprintf(stderr, "pushed to %s\n", buf);*/
    } else {
	sprintf(buf, "Cannot #include %s\n", name);
	yyerror(buf);
    }
}

static int
get_terminator(terminator)
char *terminator;
{
	int c, j = 0;

	while (((c = mygetc()) != '\n') && (c != EOF)) {
		terminator[j++] = c;
	}
	if (c == EOF) {
		return 0;
	}
	current_line++;
	terminator[j] = '\0';
	if (inctop == 0) {
		store_line_number_info();
	}
	return j;
}

#define MAXBLOCK (MAXLINE*4)
/* better solution needs to be found for large blocks than just
 * increasing this larger and larger.
 */

static int
get_array_block(text, term)
char *text;
char *term;
{
        int c, len, finished = 0, j, k;
        int total_len = 0;
        char *beg;
        char array_line[MAXBLOCK + 1];

        if (!strlen(term)) {
                return 0;
        }
        beg = text;
        array_line[0] = '(';
        array_line[1] = '{';
        array_line[2] = '"';
        k = j = len = 3;
        while (!finished) {
                while (((c = mygetc()) != '\n') && (c != EOF)) {
                        if (len++ == MAXBLOCK) {
                                break;
                        }
                        array_line[j++] = c;
                }
                if (c == EOF) {
                        return -1;
                }
                current_line++;
                if (inctop == 0) {
                        store_line_number_info();
                }
                array_line[j] = '\0';
                if (!strcmp(array_line + k, term)) {
                        if (beg == text) {
                                *text++ = '(';
                                *text++ = '{';
                        }
                        *text++ = '}';
                        *text++ = ')';
                        *text = '\0';
                        finished = 1;
                } else {
                        if ((total_len + len) >= MAXBLOCK) {
                                return -2;
                        }
                        strcpy(text, array_line);
                        text += len;
                        *text++ = '"';
                        *text = '\0';

                        array_line[0] = ',';
                        array_line[1] = '"';
                        k = j = 2;
                        
                        total_len += (len + 1);
                        len = 2;
                }
        }
        return 1;
}

static int
get_text_block(text, term)
char *text;
char *term;
{
        int c, len, finished = 0, j, k;
	int total_len = 0;
	char *beg;
	static char text_line[MAXBLOCK + 1];

	if (!strlen(term)) {
		return 0;
	}
        beg = text;
	text_line[0] = '"';
        k = j = len = 1;
	while (!finished) {
		while (((c = mygetc()) != '\n') && (c != EOF)) {
			if (len++ == MAXBLOCK) {
				break;
			}
			text_line[j++] = c;
		}
		if (c == EOF) {
			return -1;
		}
		current_line++;
		if (inctop == 0) {
			store_line_number_info();
		}
		text_line[j] = '\0';
                if (!strcmp(text_line + k, term)) {
                        if (beg == text) {
                                *text++ = '"';
                        }
			*text++ = '"';
			*text = '\0';
			finished = 1;
		} else {
			if ((total_len + len) >= MAXBLOCK) {
				return -2;
			}
			strcpy(text, text_line);
			text += len;
			*text++ = '\n';
			*text = '\0';

                        k = j = 0;

			total_len += (len + 1);
                        len = 0;
		}
	}
	return 1;
}

static void
skip_line()
{
  int c;

    while (((c = mygetc()) != '\n') && (c != EOF))
      {
	/* empty while */
      }        
    if (inctop == 0)        
      store_line_number_info();
    if (c == '\n') {
		myungetc(c);
	}
}

static void
skip_comment()
{
    int c;

    for(;;) {
	while((c = mygetc()) != '*') {
	    if (c == EOF) {
	        lexerror("End of file in a comment");
		return;
	    }
	    if (c == '\n') {
		if (inctop == 0)
		    store_line_number_info();
		nexpands=0;
		current_line++;
	    }
	}
	do {
	    if ((c = mygetc()) == '/')
		return;
	    if (c == '\n') {
		if (inctop == 0)
		    store_line_number_info();
		nexpands=0;
		current_line++;
	    }
	} while(c == '*');
    }
}

#define TRY(c, t) if (gobble(c)) return t

static void
deltrail(sp)
char *sp;
{
    char *p;
    p = sp;
    if (!*p) {
	lexerror("Illegal # command");
    } else {
	while(*p && !isspace(*p))
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

static void handle_pragma(str)
    char *str;
{
    extern int code_size();
    if (strcmp(str, "strict_types") == 0) {
#ifndef IGNORE_STRICT_PRAGMA
	pragma_strict_types = 1;
#endif
    } else if (strcmp(str, "save_types") == 0) {
	pragma_save_types = 1;
#ifdef SAVE_BINARIES
    } else if (strcmp(str, "save_binary") == 0) {
	pragma_save_binaries = 1;
#endif
    }
}

static int
yylex1()
{
  static char partial[MAXLINE+5]; /* extra 5 for safety buffer */
  static char terminator[MAXLINE+5];
  int is_float;
  float myreal;
  char *partp;

  register char *yyp;	/* Xeno */
  register int c;	/* Xeno */

  partp=partial;	/* Xeno */
  partial[0]=0;		/* Xeno */

  for(;;) {
    if (lex_fatal) {
	return -1;
    }
    switch(c = mygetc()) {
    case EOF:
	if (inctop) {
	    struct incstate *p;
	    p = inctop;
	    fclose(yyin);
/*fprintf(stderr, "popping to %s\n", p->file);*/
	    FREE(current_file);
	    nexpands=0;
	    current_file = p->file;
	    current_line = p->line + 1;
	    pragma_strict_types = p->pragma_strict_types;
	    yyin = p->yyin;
	    slast = p->slast;
	    lastchar = p->lastchar;
	    inctop = p->next;
	    if (inctop == 0)
		store_line_number_info();
	    FREE((char *)p);
	    break;
	}
	if (iftop) {
	    struct ifstate *p = iftop;
            yyerror(p->state == EXPECT_ENDIF ? "Missing #endif" : "Missing #else/#elif");
	    while(iftop) {
		p = iftop;
		iftop = p->next;
		FREE((char *)p);
	    }
	}
	return -1;
    case '\n':
	{
	    if (inctop == 0)
		store_line_number_info();
	    nexpands=0;
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
	TRY('=', L_ADD_EQ);
	return c;
    case '-':
	TRY('>', L_ARROW);
	TRY('-', L_DEC);
	TRY('=', L_SUB_EQ);
	return c;
    case '&':
	TRY('&', L_LAND);
	TRY('=', L_AND_EQ);
	return c;
    case '|':
	TRY('|', L_LOR);
	TRY('=', L_OR_EQ);
	return c;
    case '^':
	TRY('=', L_XOR_EQ);
	return c;
    case '<':
	if (gobble('<')) {
	    TRY('=', L_LSH_EQ);
	    return L_LSH;
	}
	TRY('=', L_LE);
	return c;
    case '>':
	if (gobble('>')) {
	    TRY('=', L_RSH_EQ);
	    return L_RSH;
	}
	TRY('=', L_GE);
	return c;
    case '*':
	TRY('=', L_MULT_EQ);
	return c;
    case '%':
	TRY('=', L_MOD_EQ);
	return c;
    case '/':
	if (gobble('*')) {
	    skip_comment();
	    break;
    }
	else if (gobble('/')) {
		skip_line();
		break;
	}
	TRY('=', L_DIV_EQ);
	return c;
    case '=':
	TRY('=', L_EQ);
	return c;
    case ';':
    case '(':
    case ')':
    case ',':
    case '{':
    case '}':
    case '~':
    case '[':
    case ']':
    case '?':
	return c;
    case '!':
	TRY('=', L_NE);
	return L_NOT;
    case ':':
	TRY(':', L_COLON_COLON);
	return ':';
    case '.':
	TRY('.',L_RANGE);
	goto badlex;
    case '#':
	if (lastchar == '\n') {
	    char *sp = 0;
	    int quote;

	    yyp = yytext;
	    do {
		c = mygetc();
	    } while (isspace(c));
	    for(quote = 0;;) {

		if (c == '"')
		    quote ^= 1;
		if (!quote && c == '/') { /*gc - handle comments cpp-like! 1.6.91 */
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
		while(isspace(*sp))
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
		cond=cond_get_exp(0);
		if (mygetc()) {
		    yyerror("Condition too complex in #if");
		    while ( mygetc() ) ;
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
                        FREE((char *)p);

                        myungetc(0);
                        add_input(sp);
                        cond = cond_get_exp(0);
                        if (mygetc()) {
                            yyerror("Condition too complex in #elif");
                            while ( mygetc() );
                        } else {
                            handle_cond(cond);
                        }
                    } else { /* EXPECT_ENDIF */
                        /* last cond was true...skip to end of conditional */
                        skip_to("endif", (char *)0);
                    }
                } else {
                    yyerror("Unexpected #elif");
                }
	    } else if (strcmp("else", yytext) == 0) {
                if (iftop) {
                    if (iftop->state == EXPECT_ELSE) {
                        iftop->state = EXPECT_ENDIF;
                    } else {
                        skip_to("endif", (char *)0);
                    }
		} else {
		    yyerror("Unexpected #else");
		}
	    } else if (strcmp("endif", yytext) == 0) {
		if (iftop && (iftop->state == EXPECT_ENDIF ||
			      iftop->state == EXPECT_ELSE)) {
		    struct ifstate *p = iftop;

		    iftop = p->next;
		    FREE((char *)p);
		} else {
		    yyerror("Unexpected #endif");
		}
	    } else if (strcmp("undef", yytext) == 0) {
		struct defn *d;

		deltrail(sp);
		if ((d = lookup_define(sp)))
		    d->undef++;
	    } else if (strcmp("echo", yytext) == 0) {
		fprintf(stderr, "%s\n", sp);
	    } else if (strcmp("include", yytext) == 0) {
                handle_include(sp);
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
		if (tmp == 'n') { /* fix from Wing@TMI 92/08/21 */
			yylval.number = '\n';
		} else if (tmp == 't') {
			yylval.number = '\t';
		} else if (tmp == 'r') {
			yylval.number = '\r';
		} else if (tmp == 'b') {
			yylval.number = '\b';
		} else
			yylval.number = tmp;
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
                        rc = get_array_block(yytext, terminator);

                        if (rc > 0) {
                                add_input(yytext);
                                return mygetc();
                        } else if (rc == -1) {
                                yyerror("End of file in array block");
                        } else {
                                yyerror("Array block exceeded maximum length");
                        }
                } else {
                        rc = get_text_block(yytext, terminator);

                        if (rc > 0) {
                                return string(yytext);
                        } else if (rc == -1) {
                                yyerror("End of file in text block");
                        } else /* if (rc == -2) */ {
                                yyerror("Text block exceeded maximum length");
                        }
		}
	}
    break;
    case '"':
	yyp = yytext;
	*yyp++ = c;
	for(;;) {
	    c = mygetc();
	    if (c == EOF) {
 		lexerror("End of file in string");
		return string("\"\"");
	    }
#if 0
	else if (c == '\n') {
 		lexerror("Newline in string");
		return string("\"\"");
	    }
#endif
	    SAVEC;
	    if (c == '"')
		break;
	    if (c == '\\') {
		c = mygetc();
		if ( c == '\n' ) {
		    yyp--;
		    if (inctop == 0)
		        store_line_number_info();
		    current_line++;
		    total_lines++;
		} else if ( c == EOF ) {
		    myungetc(c); /* some operating systems give EOF only once */
		} else *yyp++ = c;
	    }
	}
	*yyp = 0;
	return string(yytext);

    case '0':
	c = mygetc();
	if ( c == 'X' || c == 'x' ) {
	    yyp=yytext;
	    for(;;) {
		c = mygetc();
		SAVEC;
		if (!isxdigit(c))
		    break;
	    }
	    myungetc(c);
	    return number( (int)strtol(yytext,(char**)NULL,0x10) );
	}
	myungetc(c);
	c = '0';
	/* fall through */
    case '1':case '2':case '3':case '4':
    case '5':case '6':case '7':case '8':case '9':
    is_float = 0;
	yyp = yytext;
	*yyp++ = c;
	for(;;) {
	    c = mygetc();
        if (c == '.') {
            if (!is_float) {
                is_float = 1;
            } else {
                is_float = 0;
                myungetc(c);
                break;
            }
        }
	    else if (!isdigit(c))
		   break;
	    SAVEC;
	}
	myungetc(c);
	*yyp = 0;
    if (is_float) {
       sscanf(yytext, "%f", &myreal);
       return real(myreal);
    } else {
	   return number(atoi(yytext));
    }
    default:
	if (isalpha(c) || c == '_') {
	    int r;

	    yyp = yytext;
	    *yyp++ = c;
	    for(;;) {
		c = mygetc();
		if (!isalunum(c))
		    break;
		SAVEC;
	      }
#ifdef WANT_MISSING_SPACE_BUG
        while (c==' ') c=mygetc();
#endif
	if (c=='#') {
		if (mygetc()!='#')
		  lexerror("Single '#' in identifier -- use '##' for token pasting");
		*yyp=0;
		myungetc(c);
		myungetc(c);
/*		fprintf(stderr,"Check point 1\n"); */
		if (!expand_define()) {
/*		  fprintf(stderr,"Check point 2\n"); */
			if (partp+strlen(yytext)-partial>MAXLINE)
				lexerror("Pasted token is too long");
			strcpy(partp,yytext);
			partp+=strlen(yytext);
			mygetc();
		        mygetc();
/*		  fprintf(stderr,"yytext = '%s'\npartial = '%s'\npartp='%s'\n",yytext,partial,partp);
		  fprintf(stderr,"*outp = '%c'",*outp); */
		      }
	      }
	else if (partp!=partial) {
/*	  fprintf(stderr,"Check point 3\n"); */
	  *yyp=0;
	  myungetc(c);
          if (!expand_define())
	    add_input(yytext);
	  while ((c=mygetc())==' ');
          myungetc(c);
	  add_input(partial);
/*	  fprintf(stderr,"partial='%s'\nyytext='%s'\n",partial,yytext);
          fprintf(stderr,"outp='%20s'\n",outp); */
	  partp=partial;
	  partial[0]=0;
	}
	else {

	    *yyp = 0;

	    myungetc(c);
	    if (!expand_define()) {
		r = lookup_resword(yytext);
		if (r >= 0) {
		    return r;
		} else
		    return ident(yytext);
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

    sprintf(buff, "Illegal character (hex %02x) '%c'", (unsigned)c, (char)c);
    fprintf(stderr,"partial = %s\n",partial);
    yyerror(buff);
#endif
    return ' '; }
  }

int
yylex()
{
    int r;

    yytext[0] = 0;
    r = yylex1();
/*    fprintf(stderr, "lex=%d(%s) ", r, yytext);*/
    return r;
}

extern YYSTYPE yylval;

static int islocal(str)
    char *str;
{
    int i;

    for (i=current_number_of_locals-1; i>=0; i--) {
	if (strcmp(local_names[i], str) == 0)
            return i;
    }
    return -1;
}

static int ident(str)
    char *str;
{
    int i;
   
    i = islocal(str);
    if (i >= 0) {
        yylval.number = i;
        return L_LOCAL_NAME;
    }
    yylval.string = string_copy(str);
    return L_IDENTIFIER;
}

static int string(str)
    char *str;
{
    char *p;

    if (!*str) {
	str = "\"\"";
    }
    p = DXALLOC(strlen(str), 63, "string: p");
    yylval.string = p;
    for (str++; str[0] && str[1] ; str++, p++) {
	if (str[0] == '\\') {
	    if (str[1] == 'n') {
		*p = '\n';
	    } else if (str[1] == 't') {
		*p = '\t';
	    } else if (str[1] == 'r') {
		*p = '\r';
	    } else if (str[1] == 'b') {
		*p = '\b';
	    } else
		*p = str[1];
	    str++;
	} else
	    *p = *str;
    }
    *p = '\0';
    return L_STRING;
}

static int number(i)
    int i;
{
    yylval.number = i;
    return L_NUMBER;
}

static int real(i)
    double i;
{
    yylval.real = (float)i;
    return L_REAL;
}

void end_new_file()
{
    while (inctop) {
	struct incstate *p;
	p = inctop;
	fclose(yyin);
	FREE(current_file);
	current_file = p->file;
	yyin = p->yyin;
	inctop = p->next;
	FREE((char *)p);
    }
    while(iftop) {
	struct ifstate *p;

	p = iftop;
	iftop = p->next;
	FREE((char *)p);
    }
	if (defines_need_freed) {
		free_defines();
		defines_need_freed = 0;
	}
}

void
add_quoted_define(def, val)
char *def, *val;
{
	char save_buf[1024];

    strcpy(save_buf, "\"");
    strcat(save_buf, val);
    strcat(save_buf, "\"");
    add_define(def, -1, save_buf);
}

void
add_predefines()
{
	char save_buf[80];
	char *tmp, *dir;

	add_define("LPC3", -1, "");
	add_define("MUDOS", -1, "");
	add_quoted_define("SAVE_EXTENSION", SAVE_EXTENSION);
	add_quoted_define("__ARCH__", ARCH);
#ifdef COMPILER
	add_quoted_define("__COMPILER__", COMPILER);
#endif
#ifdef __VERSION__
	add_quoted_define("__COMPILER_VERSION__", __VERSION__);
#endif
#ifdef OPTIMIZE
	add_quoted_define("__OPTIMIZATION__", OPTIMIZE);
#endif
#ifndef CDLIB
	add_quoted_define("MUD_NAME", MUD_NAME);
#endif
	get_version(save_buf);
	add_quoted_define("__VERSION__", save_buf);
	sprintf(save_buf,"%d", PORTNO);
	add_define("__PORT__", -1, save_buf);
	if (current_file) {
		dir = (char *)DMALLOC(strlen(current_file)+3, 64, "start_new_file");
		sprintf (dir,"\"%s",current_file);
		tmp = strrchr (dir,'/');
		if (tmp) {
			tmp[1] = '"';
			tmp[2] = 0;
		}
		add_define("__DIR__",-1,dir);
		FREE(dir);
	}
	add_define("USE_EUID", -1, "");
#ifdef SOCKET_EFUNS
	add_define("HAS_SOCKETS", -1, "");
#endif
#ifndef NO_SHADOWS 
	add_define("HAS_SHADOWS", -1, "");
#endif
#if (defined(DEBUGMALLOC) && defined(DEBUGMALLOC_EXTENSIONS))
	add_define("HAS_DEBUGMALLOC", -1, "");
#endif
#ifdef MATH
	add_define("HAS_MATH", -1, "");
#endif
#ifdef MATRIX
	add_define("HAS_MATRIX", -1, "");
#endif
#ifdef ED
	add_define("HAS_ED", -1, "");
#endif
#ifdef PRINTF
	add_define("HAS_PRINTF", -1, "");
#endif
#ifdef PRIVS
	add_define("HAS_PRIVS", -1, "");
#endif
#ifdef EACH
	add_define("HAS_EACH", -1, "");
#endif
#ifdef CACHE_STATS
	add_define("HAS_CACHE_STATS", -1, "");
#endif
#if (defined(RUSAGE) || defined(GET_PROCESS_STATS) || defined(TIMES))
	add_define("HAS_RUSAGE", -1, "");
#endif
#ifdef DEBUG_MACRO
	add_define("HAS_DEBUG_LEVEL", -1, "");
#endif
#ifdef OPCPROF
	add_define("HAS_OPCPROF", -1, "");
#endif
}

void start_new_file(f)
    FILE *f;
{
    struct lpc_predef_s *tmpf;

	if (defines_need_freed) {
		free_defines();
	}
	defines_need_freed = 1;
    add_predefines();
    for (tmpf=lpc_predefs; tmpf; tmpf=tmpf->next) {
    char namebuf[NSIZE];
    char mtext[MLEN];

	*mtext='\0';
	sscanf(tmpf->flag,"%[^=]=%[ -~=]",namebuf,mtext);
	if ( strlen(namebuf) >= NSIZE ) fatal("NSIZE exceeded");
	if ( strlen(mtext) >= MLEN ) fatal("MLEN exceeded");
	add_define(namebuf,-1,mtext);
    }
    yyin = f;
    slast = '\n';
    lastchar = '\n';
    current_line = 1;
    lex_fatal = 0;
    nbuf = 0;
    outp = defbuf+DEFMAX;
    pragma_strict_types = 0;		/* I would prefer !o_flag   /Lars */
#ifdef SAVE_BINARIES
    pragma_save_binaries = 0;
#endif
    nexpands = 0;
}

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
static struct keyword {
    char *word;
    short  token;
    short min_args;	/* Minimum number of arguments. */
    short max_args;	/* Maximum number of arguments. */
    short ret_type;	/* The return type used by the compiler. */
    unsigned char arg_type1;	/* Type of argument 1 */
    unsigned char arg_type2;	/* Type of argument 2 */
    short arg_index;	/* Index pointing to where to find arg type */
    short Default;      /* an efun to use as default for last argument */
} predefs[] =
#include "efun_defs.c"

static struct keyword reswords[] = {
{ "break",		L_BREAK, },
{ "case",		L_CASE, },
{ "catch",		L_CATCH, },
{ "continue",		L_CONTINUE, },
{ "default",		L_DEFAULT, },
{ "do",			L_DO, },
{ "else",		L_ELSE, },
{ "float",      L_FLOAT, },
{ "for",		L_FOR, },
{ "function",   L_FUNCTION, },
{ "if",			L_IF, },
{ "inherit",		L_INHERIT, },
{ "int",		L_INT, },
{ "mapping",            L_MAPPING, },
{ "mixed",		L_MIXED, },
{ "nomask",		L_NO_MASK, },
{ "object",		L_OBJECT, },
{ "parse_command",	L_PARSE_COMMAND, },
{ "private",		L_PRIVATE, },
{ "protected",		L_PROTECTED, },
{ "public",		L_PUBLIC, },
{ "return",		L_RETURN, },
{ "sscanf",		L_SSCANF, },
{ "static",		L_STATIC, },
{ "status",		L_STATUS, },
{ "string",		L_STRING_DECL, },
{ "switch",		L_SWITCH, },
{ "time_expression",	L_TIME_EXPRESSION, },
{ "varargs",		L_VARARGS, },
{ "void",		L_VOID, },
{ "while",		L_WHILE, },
};

struct instr instrs[MAX_INSTRS];
static char num_buf[20];

char *query_instr_name(instr)
int instr;
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

static void add_instr_name(name, n)
    char *name;
    int n;
{
    instrs[n].name = name;
}

void init_num_args()
{
    int i, n;

    for(i=0; i<NELEM(predefs); i++) {
	n = predefs[i].token;
	if (n < 0 || n > NELEM(instrs))
	    fatal("Token %s has illegal value %d.\n", predefs[i].word, n);
	instrs[n].min_arg = predefs[i].min_args;
	instrs[n].max_arg = predefs[i].max_args;
	instrs[n].name = predefs[i].word;
	instrs[n].type[0] = predefs[i].arg_type1;
	instrs[n].type[1] = predefs[i].arg_type2;
	instrs[n].Default = predefs[i].Default;
	instrs[n].ret_type = predefs[i].ret_type;
	instrs[n].arg_index = predefs[i].arg_index;
    }
    add_instr_name("<", F_LT);
    add_instr_name(">", F_GT);
    add_instr_name("<=", F_LE);
    add_instr_name(">=", F_GE);
    add_instr_name("==", F_EQ);
    add_instr_name("+=", F_ADD_EQ);
    add_instr_name("(void)+=", F_VOID_ADD_EQ);
    add_instr_name("!", F_NOT);
    add_instr_name("&", F_AND);
    add_instr_name("&=", F_AND_EQ);
    add_instr_name("index", F_INDEX);
#ifdef LPC_OPTIMIZE_LOOPS
    add_instr_name("loop_cond", F_LOOP_COND);
    add_instr_name("loop_incr", F_LOOP_INCR);
    add_instr_name("while_dec", F_WHILE_DEC);
#endif
    add_instr_name("push_indexed_lvalue", F_PUSH_INDEXED_LVALUE);
    add_instr_name("identifier", F_IDENTIFIER);
    add_instr_name("local", F_LOCAL_NAME);
    add_instr_name("number", F_NUMBER);
    add_instr_name("real", F_REAL);
    add_instr_name("push_local_variable_lvalue", F_PUSH_LOCAL_VARIABLE_LVALUE);
    add_instr_name("const1", F_CONST1);
    add_instr_name("subtract", F_SUBTRACT);
    add_instr_name("(void)assign", F_VOID_ASSIGN);
    add_instr_name("assign", F_ASSIGN);
    add_instr_name("branch", F_BRANCH);
    add_instr_name("byte", F_BYTE);
    add_instr_name("-byte", F_NBYTE);
    add_instr_name("bbranch_when_zero", F_BBRANCH_WHEN_ZERO);
    add_instr_name("bbranch_when_non_zero", F_BBRANCH_WHEN_NON_ZERO);
    add_instr_name("branch_when_zero", F_BRANCH_WHEN_ZERO);
    add_instr_name("branch_when_non_zero", F_BRANCH_WHEN_NON_ZERO);
    add_instr_name("pop", F_POP_VALUE);
    add_instr_name("const0", F_CONST0);
#ifdef F_JUMP_WHEN_ZERO
    add_instr_name("jump_when_zero", F_JUMP_WHEN_ZERO);
    add_instr_name("jump_when_non_zero", F_JUMP_WHEN_NON_ZERO);
#endif
#ifdef F_LOR
    add_instr_name("||", F_LOR);
    add_instr_name("&&", F_LAND);
#endif
    add_instr_name("-=", F_SUB_EQ);
    add_instr_name("jump", F_JUMP);
    add_instr_name("return", F_RETURN);
    add_instr_name("sscanf", F_SSCANF);
    add_instr_name("parse_command", F_PARSE_COMMAND);
    add_instr_name("string", F_STRING);
    add_instr_name("call", F_CALL_FUNCTION_BY_ADDRESS);
    add_instr_name("aggregate_assoc", F_AGGREGATE_ASSOC);
    add_instr_name("break_point", F_BREAK_POINT);
    add_instr_name("call_extra", F_CALL_EXTRA);
    add_instr_name("aggregate", F_AGGREGATE);
    add_instr_name("(::)", F_FUNCTION_CONSTRUCTOR);
    add_instr_name("(*)", F_FUNCTION_SPLIT);
    add_instr_name("push_identifier_lvalue", F_PUSH_IDENTIFIER_LVALUE);
	add_instr_name("|", F_OR);
	add_instr_name("<<", F_LSH);
	add_instr_name(">>", F_RSH);
	add_instr_name(">>=", F_RSH_EQ);
	add_instr_name("<<=", F_LSH_EQ);
	add_instr_name("^", F_XOR);
	add_instr_name("^=", F_XOR_EQ);
	add_instr_name("|=", F_OR_EQ);
    add_instr_name("+", F_ADD);
    add_instr_name("!=", F_NE);
    add_instr_name("dup", F_DUP);
    add_instr_name("catch", F_CATCH);
    add_instr_name("end_catch", F_END_CATCH);
    add_instr_name("!", F_NEGATE);
    add_instr_name("~", F_COMPL);
	add_instr_name("++x", F_PRE_INC);
	add_instr_name("--x", F_PRE_DEC);
	add_instr_name("*", F_MULTIPLY);
	add_instr_name("*=", F_MULT_EQ);
	add_instr_name("/", F_DIVIDE);
	add_instr_name("/=", F_DIV_EQ);
	add_instr_name("%", F_MOD);
	add_instr_name("%=", F_MOD_EQ);
	add_instr_name("inc(x)", F_INC);
	add_instr_name("dec(x)", F_DEC);
    add_instr_name("x++", F_POST_INC);
    add_instr_name("x--", F_POST_DEC);
    add_instr_name("switch",F_SWITCH);
    add_instr_name("break",F_BREAK);
    add_instr_name("pop_break",F_POP_BREAK);
    add_instr_name("range",F_RANGE);
    add_instr_name("time_expression",F_TIME_EXPRESSION);
    add_instr_name("end_time_expression",F_END_TIME_EXPRESSION);
    instrs[F_RANGE].type[0] = T_POINTER|T_STRING;
}

char *get_f_name(n)
    int n;
{
    if (instrs[n].name)
	return instrs[n].name;
    else {
	static char buf[30];
	sprintf(buf, "<OTHER %d>", n);
	return buf;
    }
}

static int
lookupword(s, words, h)
char *s;
struct keyword *words;
int h;
{
    int i, l, r;

    l = 0;
    for(;;) {
      i = (l+h)/2;
      r = strcmp(s, words[i].word);
      if (r == 0)
          return words[i].token;
      else if (l == i)
          return -1;
      else if (r < 0)
          h = i;
      else
          l = i;
    }
}

static int lookup_resword(s)
    char *s;
{
    return lookupword(s, reswords, NELEM(reswords));
}

int lookup_predef(s)
    char *s;
{
    return lookupword(s, predefs, NELEM(predefs));
}

#define NARGS 25
#define MARKS '@'

#define SKIPWHITE while(isspace(*p)) p++
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

static int
cmygetc()
{
    int c;

    for(;;) {
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

static void
refill()
{
    char *p;
    int c;

    p = yytext;
    do {
	c = cmygetc();
	if (p < yytext+MAXLINE-5)
	    *p++ = c;
	else {
	    lexerror("Line too long");
	    break;
	}
    } while(c != '\n' && c != EOF);
    p[-1] = ' ';
    *p = 0;
    nexpands=0;
    current_line++;
    if (inctop == 0)
	store_line_number_info();
}

static void
handle_define(yyt)
char *yyt;
{
    char namebuf[NSIZE];
    char args[NARGS][NSIZE];
    char mtext[MLEN];
    char *p, *q;

    p = yyt;
    strcat(p, " ");
    q = namebuf;
    GETALPHA(p, q, namebuf+NSIZE-1);
    if (*p == '(') {		/* if "function macro" */
	int arg;
	int inid;
	char *ids = (char *)NULL;

	p++;			/* skip '(' */
	SKIPWHITE;
	if (*p == ')') {
	    arg = 0;
	} else {
	    for(arg = 0; arg < NARGS; ) {
		q = args[arg];
		GETDEFINE(p, q, args[arg]+NSIZE-1);
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
	for(inid = 0, q = mtext; *p; ) {
	    if (isalunum(*p)) {
		if (!inid) {
		    inid++;
		    ids = p;
		}
	    } else {
		if (inid) {
		    int idlen = p - ids;
		    int n, l;
		    for(n = 0; n < arg; n++) {
			l = strlen(args[n]);
			if (l == idlen && strncmp(args[n], ids, l) == 0) {
			    q -= idlen;
			    *q++ = MARKS;
			    *q++ = n+MARKS+1;
			    break;
			}
		    }
		    inid = 0;
		}
	    }
	    *q = *p;
	    if (*p++ == MARKS)
		*++q = MARKS;
	    if (q < mtext+MLEN-2)
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
    } else {
	for(q = mtext; *p; ) {
	    *q = *p++;
	    if (q < mtext+MLEN-2)
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
    }
    return;
}

static void
myungetc(c)
int c;
{
    *--outp = c;
    nbuf++;
}

static void
add_input(p)
char *p;
{
    int l = strlen(p);

/*if (l > 2)
fprintf(stderr, "add '%s'\n", p);*/
    if (nbuf+l >= DEFMAX-10) {
	lexerror("Macro expansion buffer overflow");
	return;
    }
    outp -= l;
    nbuf += l;
    strncpy(outp, p, l);
}

#define DEFHASH 33
struct defn *defns[DEFHASH];
#define defhash(s) hashstr(s, 10, DEFHASH)

static void
add_define(name, nargs, exps)
char *name, *exps;
int nargs;
{
	struct defn *p;
	int h;

	if ((p = lookup_define(name))) {
		if (nargs != p->nargs || strcmp(exps, p->exps) != 0) {
			char buf[200+NSIZE];
			sprintf(buf, "Warning: redefinition of #define %s", name);
			add_message(buf);
		}
		p->nargs = nargs;
		p->exps = (char *)DREALLOC(p->exps, strlen(exps)+1,65,"add_define: 3");
		strcpy(p->exps, exps);
	} else {
		p = (struct defn *)DXALLOC(sizeof(struct defn), 65, "add_define: 1");
		p->name = DXALLOC(strlen(name)+1, 66, "add_define: 2");
		strcpy(p->name, name);
		p->undef = 0;
		p->nargs = nargs;
		p->exps = DXALLOC(strlen(exps)+1, 67, "add_define: 3");
		strcpy(p->exps, exps);
		h = defhash(name);
		p->next = defns[h];
		defns[h] = p;
	}
	/* fprintf(stderr, "define '%s' %d '%s'\n", name, nargs, exps); */
}

static void
free_defines()
{
    struct defn *p, *q;
    int i;

    for(i = 0; i < DEFHASH; i++) {
	for(p = defns[i]; p; p = q) {
	    q = p->next;
	    FREE(p->name);
	    FREE(p->exps);
	    FREE((char *)p);
	}
	defns[i] = 0;
    }
    nexpands = 0;
}

struct defn *
lookup_define(s)
char *s;
{
    struct defn *p;
    int h;

    h = defhash(s);
    for(p = defns[h]; p; p = p->next)
	if (!p->undef && strcmp(s, p->name) == 0)
	    return p;
    return 0;
}

#define SKIPW \
        do {\
	    c = cmygetc();\
	} while(isspace(c));


/* Check if yytext is a macro and expand if it is. */
static int
expand_define()
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
	    for(n = 0; n < NARGS; ) {
		switch(c) {
		case '"': if (!squote) dquote ^= 1; break;
		case '\'': if (!dquote) squote ^= 1; break;
		case '(': if (!squote && !dquote) parcnt++; break;
		case ')': if (!squote && !dquote) parcnt--; break;
                case '#': if (!squote && !dquote) {
                             *q++ = c;
                             if (mygetc() != '#') {
                                lexerror("'#' expected");
                                return 0;
                             }
                          }
                          break;
		case '\\': if (squote || dquote) { *q++ = c; c = mygetc();} break;
		case '\n': if (squote || dquote) { lexerror("Newline in string"); return 0; } break;
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
		if (!squote && ! dquote)
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
	while(*e) {
            if (*e == '#' && *(e + 1) == '#') e += 2;
	    if (*e == MARKS) {
		if (*++e == MARKS)
		    *b++ = *e++;
		else {
		    for(q = args[*e++ - MARKS - 1]; *q; ) {
			*b++ = *q++;
			if (b >= buf+DEFMAX) {
			    lexerror("Macro expansion overflow");
			    return 0;
			}
		    }
		}
	    } else {
		*b++ = *e++;
		if (b >= buf+DEFMAX) {
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

static int exgetc() {
  register char c,*yyp;

  c=mygetc();
  while ( isalpha(c) || c=='_' ) {
    yyp=yytext;
    do {
      SAVEC;
      c=mygetc();
    } while ( isalunum(c) );
    myungetc(c);
    *yyp='\0';
    if (strcmp(yytext, "defined") == 0) {
	/* handle the defined "function" in #if */
	do c = mygetc(); while(isspace(c));
	if (c != '(') {
	    yyerror("Missing ( in defined");
	    continue;
	}
	do c = mygetc(); while(isspace(c));
	yyp=yytext;
	while ( isalunum(c) ) {
	    SAVEC;
	    c=mygetc();
	}
	*yyp='\0';
	while(isspace(c)) c = mygetc();
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
	if (!expand_define()) add_input(" 0 ");
    }
    c=mygetc();
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

static char _optab[]=
{0,4,0,0,0,26,56,0,0,0,18,14,0,10,0,22,0,0,0,0,0,0,0,0,0,0,0,0,30,50,40,74,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,0,1};
static char optab2[]=
{BNOT,0,0,LNOT,'=',NEQ,7,0,0,UMINUS,0,BMINUS,10,UPLUS,0,BPLUS,10,
0,0,MULT,11,0,0,DIV,11,0,0,MOD,11,
0,'<',LSHIFT,9,'=',LEQ,8,0,LESS,8,0,'>',RSHIFT,9,'=',GEQ,8,0,GREAT,8,
0,'=',EQ,7,0,0,0,'&',LAND,3,0,BAND,6,0,'|',LOR,2,0,BOR,4,
0,0,XOR,5,0,0,QMARK,1};
#define optab1 (_optab-' ')

static int cond_get_exp(priority)
int priority;
{
  int c;
  int value,value2,x;

  do c=exgetc(); while ( isspace(c) );
  if ( c=='(' ) {

    value=cond_get_exp(0);
    do c=exgetc(); while ( isspace(c) );
    if ( c!=')' ) {
      yyerror("bracket not paired in #if");
      if (!c) myungetc('\0');
    }
  } else if ( ispunct(c) ) {
    x=optab1[c];
    if (!x) {
      yyerror("illegal character in #if");
      return 0;
    }
    value=cond_get_exp(12);
    switch ( optab2[x-1] ) {
      case BNOT  : value = ~value; break;
      case LNOT  : value = !value; break;
      case UMINUS: value = -value; break;
      case UPLUS : value =  value; break;
      default :
	yyerror("illegal unary operator in #if");
	return 0;
    }
  } else {
    int base;

    if ( !isdigit(c) ) {
      if (!c) {
	yyerror("missing expression in #if");
	myungetc('\0');
      } else yyerror("illegal character in #if");
      return 0;
    }
    value=0;
    if ( c!='0' ) base=10;
    else {
      c=mygetc();
      if ( c=='x' || c=='X' ) {
	base=16;
	c=mygetc();
      } else base=8;
    }
    for(;;) {
      if ( isdigit(c) ) x = -'0';
      else if ( isupper(c) ) x = -'A'+10;
      else if ( islower(c) ) x = -'a'+10;
      else break;
      x+=c;
      if ( x > base ) break;
      value=value*base+x;
      c=mygetc();
    }
    myungetc(c);
  }
  for (;;) {
    do c=exgetc(); while ( isspace(c) );
    if ( !ispunct(c) ) break;
    x=optab1[c];
    if (!x) break;
    value2=mygetc();
    for(;;x+=3) {
      if ( !optab2[x] ) {
	myungetc(value2);
	if ( !optab2[x+1] ) {
	  yyerror("illegal operator use in #if");
	  return 0;
	}
	break;
      }
      if ( value2==optab2[x] ) break;
    }
    if ( priority >= optab2[x+2] ) {
      if( optab2[x] ) myungetc(value2);
      break;
    }
    value2=cond_get_exp(optab2[x+2]);
    switch ( optab2[x+1] ) {
      case MULT : value *= value2;	break;
      case DIV  :
           if (value2)
              value /= value2;
           else
              yyerror("division by 0 in #if");
           break;
      case MOD  :
           if (value2)
              value %= value2;
           else
              yyerror("modulo by 0 in #if");
           break;
      case BPLUS  : value += value2;	break;
      case BMINUS : value -= value2;	break;
      case LSHIFT : value <<= value2;	break;
      case RSHIFT : value >>= value2;	break;
      case LESS   : value = value <  value2;	break;
      case LEQ    : value = value <= value2;	break;
      case GREAT  : value = value >  value2;	break;
      case GEQ    : value = value >= value2;	break;
      case EQ     : value = value == value2;	break;
      case NEQ    : value = value != value2;	break;
      case BAND   : value &= value2;	break;
      case XOR    : value ^= value2;	break;
      case BOR    : value |= value2;	break;
      case LAND   : value = value && value2;	break;
      case LOR    : value = value || value2;	break;
      case QMARK  :
	do c=exgetc(); while( isspace(c) );
	if ( c!=':' ) {
	  yyerror("'?' without ':' in #if");
	  myungetc(c);
	  return 0;
	}
	if ( value ) {
	  cond_get_exp(1);
	  value=value2;
	}
	else value=cond_get_exp(1);
	break;
    }
  }
  myungetc(c);
  return value;
}

void set_inc_list (list)
     char *list;
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
  while (1)
    {
      p = strchr (p,':');
      if (!p)
	break;
      size++;
      p++;
    }
  inc_list = (char **)DXALLOC(size * sizeof (char *), 68, "set_inc_list");
  inc_list_size = size;
  for (i=size-1; i >= 0; i--) 
    {
      p = strrchr (list,':');
      if (p)
	{
	  *p = '\0';
	  p++;
	}
      else
	{
	  if (i) 
	    {
	      fprintf (stderr,"Fatal error in set_inc_list: bad state.\n");
	      exit (1);
	    }
	  p = list;
	}
      if (*p == '/')
	p++;
      /*
       * Even make sure that the mud administrator has not made an error.
       */
      if (!legal_path(p)) 
	{
	  fprintf(stderr, "'include dirs' must give paths without any '..'\n");
	  exit(-1);
	}
      inc_list[i] = make_shared_string(p);
    }
}
