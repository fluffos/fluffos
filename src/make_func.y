%{
#define NO_SOCKETS
#define NO_OPCODES
#include "std.h"
#include "config.h"
#define _YACC_
#include "lint.h"
#include "lex.h"
#include "mudlib_stats.h"
#include "interpret.h"
#include "mapping.h"
#include "hash.h"
#include "cc.h"

#ifndef BUFSIZ
#define BUFSIZ 		1024
#endif
#define NELEMS(arr) 	(sizeof arr / sizeof arr[0])

#define FUNC_SPEC "func_spec.cpp"
#define OLD_COMPILER_FILE  "grammar.pre"
#define OPTIONS "options.h"
#define OPTION_DEFINES "option_defs.c"
#define COMPILER_FILE "grammar.y"

#define MAX_FUNC  	2048  /* If we need more than this we're in trouble! */

int num_buff;
int op_code, efun_code;

int braces;

/* For quick sort purposes : */
char *key[MAX_FUNC], *buf[MAX_FUNC], has_token[MAX_FUNC];

char *oper_codes[MAX_FUNC], *efun_codes[MAX_FUNC];

int num_packages = 0;
char *packages[100];

static struct ifstate {
    struct ifstate *next;
    int state;
}      *iftop = 0;

#define EFUN_TABLE "efunctions.h"
#define EFUN_PROTO "efun_protos.h"
#define OPC_PROF "opc.h"
#define OPCODES "opcodes.h"

int min_arg = -1, limit_max = 0;

/*
 * arg_types is the types of all arguments. A 0 is used as a delimiter,
 * marking next argument. An argument can have several types.
 */
int arg_types[400], last_current_type;
/*
 * Store the types of the current efun. They will be copied into the
 * arg_types list if they were not already there (to save memory).
 */
int curr_arg_types[MAX_LOCAL], curr_arg_type_size;

void yyerror PROT((char *));
int yylex1 PROT((void));
int yylex PROT((void));
int ident PROT((int c));
int yyparse PROT((void));
void llparse PROT((void));
void make_efun_tables PROT((void));
void make_package_makefile PROT((void));
int ungetc PROT((int c, FILE *f));
char *type_str PROT((int)), *etype PROT((int)), *etype1 PROT((int)),
   *ctype PROT((int));
#ifndef toupper
int toupper PROT((int));
#endif
static void handle_define PROT((void));
static int expand_define PROT((void));
static void add_define PROT((char *, int, char *));
static void add_input PROT((char *));
static int cond_get_exp PROT((int));
static struct defn *lookup_define PROT((char *s));
static void handle_cond PROT((int));
static void handle_include PROT((char *));
static char *skip_comment PROT((char *, int));
static void deltrail PROT((void));
static int cmygetc PROT((void));
static void refill PROT((void));
static int exgetc PROT((void));
static int skip_to PROT((char *, char *));
static void create_option_defines PROT((void));

void yyerrorp PROT((char *));

struct defn {
    struct defn *next;
    char *name;
    int undef;
    char *exps;
    int nargs;
};

typedef struct incstate_t {
    struct incstate_t *next;
    FILE *yyin;
    int line;
    char *file;
} incstate;

static incstate *inctop = 0;

#define VOID 		1
#define INT		2
#define STRING		3
#define OBJECT		4
#define MAPPING		5
#define MIXED		6
#define UNKNOWN		7
#define FLOAT		8
#define FUNCTION	9
#define BUFFER         10

struct type {
    char *name;
    int num;
} types[] = {
{ "void", VOID },
{ "int", INT },
{ "string", STRING },
{ "object", OBJECT },
{ "mapping", MAPPING },
{ "mixed", MIXED },
{ "unknown", UNKNOWN },
{ "float", FLOAT},
{ "function", FUNCTION},
{ "buffer", BUFFER}
};

void mf_fatal P1(char *, str)
{
    fprintf(stderr, "%s", str);
    exit(1);
}

%}
%union {
    int number;
    char *string;
}

%token ID DEFAULT OPERATOR PACKAGE

%type <number> type arg_list basic typel arg_type typel2

%type <string> ID optional_ID optional_default

%%

specs: /* empty */ | specs spec ;

spec: operator | func | package;
    
operator: OPERATOR op_list ';' ;
    
op_list: op | op_list ',' op ;

op: ID
    {
	char f_name[500],c;
	int i = 2;
	sprintf(f_name, "F_%s", $1);
	while (c = f_name[i]){
	    if (islower(c)) f_name[i++] = toupper(c);
	    else i++;
	}
	oper_codes[op_code] = (char *) malloc(i+1);
	strcpy(oper_codes[op_code], f_name);
        free($1);

	op_code++;
    } ;

optional_ID: ID | /* empty */ { $$ = ""; } ;

optional_default: /* empty */ { $$="0"; } 
                | DEFAULT ':' ID { 
                        static char buf[40];
                        sprintf(buf, $3);
                        free($3); $$ = buf; }
                | DEFAULT ':' ID ID { 
    /* Static buffer is safe here */
    static char buf[40];

    strcpy(buf, "-((");
    strcpy(buf+3, $3);
    strcat(buf, " << 8) + ");
    strcat(buf, $4);
    strcat(buf, ")");
    free($3);
    free($4);
    $$ = buf;
} ;

package: PACKAGE ID ';' {
    if (num_packages == 100) {
	fprintf(stderr, "Too many packages.\n");
	exit(-1);
    }
    packages[num_packages++] = $2;
}

func: type ID optional_ID '(' arg_list optional_default ')' ';'
    {
	char buff[500];
	char f_name[500];
	int i, len;
	if (min_arg == -1)
	    min_arg = $5;
	if ($3[0] == '\0') {
	    if (strlen($2) + 1 + 2 > sizeof f_name)
		mf_fatal("A local buffer was too small!(1)\n");
	    sprintf(f_name, "F_%s", $2);
	    len = strlen(f_name);
	    for (i=0; i < len; i++) {
		if (islower(f_name[i]))
		    f_name[i] = toupper(f_name[i]);
	    }
	    has_token[num_buff]=1;
	    efun_codes[efun_code] = (char *) malloc(len + 1);
	    strcpy(efun_codes[efun_code], f_name);
	    efun_code++;
	} else {
	    if (strlen($3) + 1 + 17 > sizeof f_name)
		mf_fatal("A local buffer was too small(2)!\n");
	    sprintf(f_name, "F_%s | F_ALIAS_FLAG", $3);
	    len = strlen(f_name);
	    for (i=0; i < len; i++) {
		if (islower(f_name[i]))
		    f_name[i] = toupper(f_name[i]);
	    }
	    has_token[num_buff]=0;
	    free($3);
	}
	for(i=0; i < last_current_type; i++) {
	    int j;
	    for (j = 0; j+i<last_current_type && j < curr_arg_type_size; j++)
	    {
		if (curr_arg_types[j] != arg_types[i+j])
		    break;
	    }
	    if (j == curr_arg_type_size)
		break;
	}
	if (i == last_current_type) {
	    int j;
	    for (j=0; j < curr_arg_type_size; j++) {
		arg_types[last_current_type++] = curr_arg_types[j];
		if (last_current_type == NELEMS(arg_types))
		    yyerror("Array 'arg_types' is too small");
	    }
	}
        if (!strcmp($2, "call_other") && !lookup_define("CAST_CALL_OTHERS")){
	    $1 = MIXED;
	}
     	sprintf(buff, "{\"%s\",%s,0,0,%d,%d,%s,%s,%s,%d,%s},\n",
		$2, f_name, min_arg, limit_max ? -1 : $5, ctype($1),
		etype(0), etype(1), i, $6);
	if (strlen(buff) > sizeof buff)
	    mf_fatal("Local buffer overwritten !\n");
	key[num_buff] = $2;
	buf[num_buff] = (char *) malloc(strlen(buff) + 1);
        strcpy(buf[num_buff], buff);
        num_buff++;
	min_arg = -1;
	limit_max = 0;
	curr_arg_type_size = 0;
    } ;

type: basic | basic '*' { $$ = $1 | 0x10000; };

basic: ID
    {
	int i = NELEMS(types);
	$$ = 0;
	while (i--){
	    if (strcmp($1, types[i].name) == 0) {
		$$ = types[i].num;
		break;
	    }
	}
	if (!$$) {
		char buf[256];
		sprintf(buf, "Invalid type: %s", $1);
		yyerror(buf);
	}
        free($1);
    };

arg_list: /* empty */		{ $$ = 0; }
	| typel2			{ $$ = 1; if ($1) min_arg = 0; }
	| arg_list ',' typel2 	{ $$ = $1 + 1; if ($3) min_arg = $$ - 1; } ;

typel2: typel
    {
	$$ = $1;
	curr_arg_types[curr_arg_type_size++] = 0;
	if (curr_arg_type_size == NELEMS(curr_arg_types))
	    yyerror("Too many arguments");
    } ;

arg_type: type
    {
	if ($1 != VOID) {
	    curr_arg_types[curr_arg_type_size++] = $1;
	    if (curr_arg_type_size == NELEMS(curr_arg_types))
		yyerror("Too many arguments");
	}
	$$ = $1;
    } ;

typel: arg_type			{ $$ = ($1 == VOID && min_arg == -1); }
     | typel '|' arg_type 	{ $$ = (min_arg == -1 && ($1 || $3 == VOID));}
     | '.' '.' '.'		{ $$ = min_arg == -1 ; limit_max = 1; } ;

%%

FILE *yyin, *yyout;
char *current_file;
int current_line = 0, nexpands = 0, buffered = 0;
char yytext[MAXLINE];
static char defbuf[DEFMAX]; /* DEFMAX should be at least 2 * MAXLINE */
char *outp, ppchar = 0;
int in_c_case, cquote, pragmas, block_nest;
#define CHAR_QUOTE 1
#define STRING_QUOTE 2
#define PRAGMA_NOTE_CASE_START 1

int grammar_mode = 0;

int main P2(int, argc, char **, argv)
{
    int i;

    num_buff = op_code = efun_code = 0;

    ppchar = '#';
    if ((yyin = fopen(OPTIONS, "r")) == NULL){
      perror(OPTIONS);
      exit(1);
    }

    current_file = (char *) malloc(strlen(OPTIONS) + 1);
    current_line = 0;
    strcpy(current_file, OPTIONS);
    llparse();
    create_option_defines();

    if ((yyin = fopen(FUNC_SPEC, "r")) == NULL) { 
	perror(FUNC_SPEC);
	exit(1);
    }

    current_line = 0;
    current_file = (char *) malloc(strlen(FUNC_SPEC)+1);
    strcpy(current_file, FUNC_SPEC);

    yyparse();

    free((char *) current_file);

    make_package_makefile();
    make_efun_tables();

    /* Now sort the main_list */
    for (i = 0; i < num_buff; i++) {
       int j;
       for (j = 0; j < i; j++)
	   if (strcmp(key[i], key[j]) < 0) {
	      char *tmp;
	      int tmpi;
	      tmp = key[i]; key[i] = key[j]; key[j] = tmp;
	      tmp = buf[i]; buf[i] = buf[j]; buf[j] = tmp;
	      tmpi = has_token[i];
	      has_token[i] = has_token[j]; has_token[j] = tmpi;
	   }
    }
    /* Now display it... */
    printf("{\n");
    for (i = 0; i < num_buff; i++)
	printf("%s", buf[i]);
    printf("\n};\nint efun_arg_types[] = {\n");
    for (i=0; i < last_current_type; i++) {
	if (arg_types[i] == 0)
	    printf("0,\n");
	else
	    printf("%s,", ctype(arg_types[i]));
    }
    printf("};\n");
    fflush(stdout);
    fclose(yyin);

    ppchar = '%';
    in_c_case = block_nest = cquote = pragmas = 0;
    if ((yyin = fopen(OLD_COMPILER_FILE, "r")) == NULL){
      perror(OLD_COMPILER_FILE);
      exit(1);
    }

    if ((yyout = fopen(COMPILER_FILE, "w")) == NULL){
      perror(COMPILER_FILE);
      exit(1);
    }

    current_file = (char *) malloc(strlen(OLD_COMPILER_FILE) + 1);
    current_line = 0;
    braces = 0;
    strcpy(current_file, OLD_COMPILER_FILE);
    llparse();
    grammar_mode = 0;

    return 0;
}

#define SKPW while (isspace(*outp)) outp++

#define DEFHASH 33
static struct defn *defns[DEFHASH];

#define defhash(s) hashstr(s, 10, DEFHASH)

static struct defn *
     lookup_define P1(char *, s)
{
    struct defn *p;
    int h;
    h = defhash(s);
    for (p = defns[h]; p; p = p->next)
        if (!p->undef && strcmp(s, p->name) == 0)
            return p;
    return 0;
}

static void add_define P3(char *, name, int, nargs, char *, exps)
{
    struct defn *p;
    int h;

    if ((p = lookup_define(name))) {
        if (nargs != p->nargs || strcmp(exps, p->exps) != 0) {
            char buf[200 + NSIZE];

            sprintf(buf, "Warning: redefinition of %s\n", name);
            fprintf(stderr, "%s", buf);
      }
        p->nargs = nargs;
        p->exps = (char *) realloc(p->exps, strlen(exps) + 1 /*, 65, "add_define: 3" */);
        strcpy(p->exps, exps);
    } else {
        p = (struct defn *) malloc(sizeof(struct defn) /*, 65, "add_define: 1" */);
        p->name = (char *) malloc(strlen(name) + 1 /*, 66, "add_define: 2" */);
        strcpy(p->name, name);
        p->undef = 0;
        p->nargs = nargs;
        p->exps = (char *) malloc(strlen(exps) + 1 /*, 67, "add_define: 3" */);
        strcpy(p->exps, exps);
        h = defhash(name);
        p->next = defns[h];
        defns[h] = p;
    }
}

static char *protect P1(char *, p) {
    static char buf[1024];
    char *bufp = buf;

    while (*p) {
	if (*p=='\"' || *p == '\\') *bufp++ = '\\';
	*bufp++ = *p++;
    }
    *bufp = 0;
    return buf;
}

static void
create_option_defines() {
    FILE *out;
    struct defn *p;
    int count = 0;
    int i;

    out = fopen(OPTION_DEFINES, "w");
    if (!out) {
	perror(OPTION_DEFINES);
	exit(1);
    }
    fprintf(out, "{\n");
    for (i = 0; i < DEFHASH; i++) {
	for (p = defns[i]; p; p = p->next) {
	    fprintf(out, "  \"__%s__\", \"%s\",\n", p->name, protect(p->exps));
	    count++;
	}
    }
    fprintf(out,"};\n\n#define NUM_OPTION_DEFS %d\n\n", count);
    fclose(out);
}

static void
handle_include P1(char *, name)
{
    char *p;
    static char buf[1024];
    FILE *f;
    incstate *is;

    if (*name != '"') {
        struct defn *d;

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
        current_file = (char *) malloc(strlen(name) + 1 /*, 62, "handle_include: 2" */);
        strcpy(current_file, name);
        yyin = f;
    } else {
        sprintf(buf, "Cannot %cinclude %s", ppchar, name);
        yyerror(buf);
    }
}

static void
handle_pragma P1(char *, name)
{
    if (!strcmp(name, "auto_note_compiler_case_start"))
        pragmas |= PRAGMA_NOTE_CASE_START;
    else if (!strcmp(name, "no_auto_note_compiler_case_start"))
        pragmas &= ~PRAGMA_NOTE_CASE_START;
    else if (!strncmp(name, "ppchar:", 7) && *(name + 8))
        ppchar = *(name + 8);
    else yyerrorp("Unidentified %cpragma");
}

static char *skip_comment(tmp, flag)
    char *tmp;
    int flag;
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

    if (fgets(p = yyp = defbuf + (DEFMAX >> 1), MAXLINE - 1, yyin)){
      while (((c = *yyp++) != '\n') && (c != EOF)){
          if (c == '/'){
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

static void deltrail(){
    register char *p;

    p = outp;
    while (*p && !isspace(*p) && *p != '\n'){
      p++;
    }
    *p = 0;
}

void
echo_line P3( FILE *, f, char *, line, int, mode ) {
    fprintf(f, "%s", line);
    if (mode == 1) {
	int old_braces = braces;
	char *p = line;

	while (p = strchr(p, '{')) { if (p[-1] != '\'') braces++; p++; }
	p = line;
	while (p = strchr(p, '}')) { if (p[-1] != '\'') braces--; p++; }
	if (braces < 0)
	    yyerror("braces went negative\n");
	if (braces && !old_braces) {
	  /* temporary kludge */
	    fprintf(f, "#line %d \"%s\"\n", current_line - 3, current_file);
	}
    }
}

void
llparse() {
    register char *yyp, *yyp2;
    int c;
    int cond;

    while (buffered ? (yyp = yyp2 = outp) : fgets(yyp = yyp2 = defbuf + (DEFMAX >> 1), MAXLINE-1, yyin)){
	if (!buffered) current_line++;
	else buffered = 0;
	while (isspace(*yyp2)) yyp2++;
	if ((c = *yyp2) == ppchar){
	    int quote = 0;
	    char sp_buf = 0, *oldoutp;

	    if (c == '%' && yyp2[1] == '%')
		grammar_mode++;
	    outp = 0;
	    if (yyp != yyp2) yyerrorp("Misplaced '%c'.\n");
	    while (isspace(*++yyp2));
	    yyp++;
	    for (;;){
		if ((c = *yyp2++) == '"') quote ^= 1;
		else{
		    if (!quote && c == '/'){
			if (*yyp2 == '*'){
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

	    if (!strcmp("define", yyp)){
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
		if (iftop) {
		    if (iftop->state == EXPECT_ELSE) {
			/* last cond was false... */
			int cond;
			struct ifstate *p = iftop;
			
			/* pop previous condition */
			iftop = p->next;
			free((char *) p);
			
			cond = cond_get_exp(0);
			if (*outp != '\n') {
			    yyerror("Condition too complex in #elif");
			} else handle_cond(cond);
		    } else {/* EXPECT_ENDIF */
			/*
			 * last cond was true...skip to end of
			 * conditional
			 */
			skip_to("endif", (char *) 0);
		    }
		} else yyerrorp("Unexpected %celif");
	    } else if (!strcmp("else", yyp)) {
		if (iftop) {
		    if (iftop->state == EXPECT_ELSE) {
			iftop->state = EXPECT_ENDIF;
                  } else {
                      skip_to("endif", (char *) 0);
                  }
		} else yyerrorp("Unexpected %celse");
	    } else if (!strcmp("endif", yyp)) {
		if (iftop && (iftop->state == EXPECT_ENDIF ||
			      iftop->state == EXPECT_ELSE)) {
		    struct ifstate *p = iftop;
		    
		    iftop = p->next;
		    free((char *) p);
		} else {
		    yyerrorp("Unexpected %cendif");
		}
	    } else if (!strcmp("undef", yyp)) {
		struct defn *d;
		
		deltrail();
		if ((d = lookup_define(outp)))
		    d->undef++;
	    } else if (!strcmp("echo", yyp)) {
		fprintf(stderr, "echo at line %d of %s: %s\n", current_line, current_file, outp);
	    } else if (!strcmp("include", yyp)) {
		handle_include(outp);
	    } else if (!strcmp("pragma", yyp)) {
		handle_pragma(outp);
	    } else if (yyout){
		if (!strcmp("line", yyp)){
		    fprintf(yyout, "#line %d \"%s\"\n", current_line,
			    current_file);
		} else {
		    if (sp_buf) *oldoutp = sp_buf;
		    if (pragmas & PRAGMA_NOTE_CASE_START){
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
	else if (c == '/'){
	    if ((c = *++yyp2) == '*'){
		if (yyout) fputs(yyp, yyout);
		yyp2 = skip_comment(yyp2, 1);
	    } else if (c == '/' && !yyout) continue;
	    else if (yyout){
		fprintf(yyout, "%s", yyp);
	    }
	}
	else if (yyout){
	    fprintf(yyout, "%s", yyp);
	    if (pragmas & PRAGMA_NOTE_CASE_START){
		static int line_to_print;
		
		line_to_print = 0;
		
		if (!in_c_case){
		    while (isalunum(*yyp2)) yyp2++;
		    while (isspace(*yyp2)) yyp2++;
		    if (*yyp2 == ':'){
			in_c_case = 1;
			yyp2++;
		    }
		}
		
		if (in_c_case){
		    while (c = *yyp2++){
			switch(c){
			  case '{':
			    {
				if (!cquote && (++block_nest == 1))
				    line_to_print = 1;
				break;
			    }
			    
			  case '}':
			    {
				if (!cquote){
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
                            if (!cquote){
                                if ((c = *yyp2) == '*'){
                                    yyp2 = skip_comment(yyp2, 1);
                                } else if (c == '/'){
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
    if (iftop){
      struct ifstate *p = iftop;

      while (iftop){
          p = iftop;
          iftop = p->next;
          free(p);
      }
      yyerrorp("Missing %cendif");
    }
    fclose(yyin);
    free(current_file);
    nexpands = 0;
    if (inctop){
      incstate *p = inctop;

      current_file = p->file;
      current_line = p->line;
      yyin = p->yyin;
      inctop = p->next;
      free((char *) p);
      llparse();
    } else yyout = 0;
}

static int skip_to(token, atoken)
    char *token, *atoken;
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

static void handle_cond(c)
    int c;
{
    struct ifstate *p;

    if (!c) skip_to("else", "endif");
    p = (struct ifstate *) malloc(sizeof(struct ifstate) /*, 60, "handle_cond" */);
    p->next = iftop;
    iftop = p;
    p->state = c ? EXPECT_ENDIF : EXPECT_ELSE;
}

static void add_input(p)
    char *p;
{
    int l = strlen(p);

    if (outp - l < defbuf) yyerror("Macro expansion buffer overflow.\n");
    strncpy(outp - l, p, l);
    outp -= l;
}

static int cmygetc(){
    int c;

    for (;;){
      if ((c = *outp++) == '/'){
          if ((c = *outp) == '*') outp = skip_comment(outp, 0);
          else if (c == '/') return -1;
          else return c;
      } else return c;
    }
}

/* Check if yytext is a macro and expand if it is. */
static int expand_define()
{
    struct defn *p;
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
                if (!squote && !dquote){
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
    while (isalpha(c = *outp) || c == '_'){
      yyp = yytext;
      do {
          *yyp++ = c;
      } while (isalnum(c = *++outp) || (c == '_'));
      *yyp = '\0';
      if (!strcmp(yytext, "defined")) {
          /* handle the defined "function" in #/%if */
          SKPW;
          if (*outp != '(') yyerror("Missing ( after 'defined'");
          SKPW;
          yyp = yytext;
          if (isalpha(c = *outp) || c == '_'){
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


static int cond_get_exp(priority)
    int priority;
{
    int c;
    int value, value2, x;

    if ((c = exgetc()) == '(') {
        value = cond_get_exp(0);
        if ((c = exgetc()) != ')') yyerrorp("bracket not paired in %cif");
    } else if (ispunct(c)) {
        if (!(x = optab1[c])) yyerrorp("illegal character in %cif");
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
        if (c != '0')
            base = 10;
        else {
            c = *outp++;
            if (c == 'x' || c == 'X') {
                base = 16;
                c = *outp++;
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
            c = *outp++;
	}
        outp--;
    }
    for (;;) {
        if (!ispunct(c = exgetc()))
            break;
        if (!(x = optab1[c]))
            break;
        value2 = *outp++;
        for (;; x += 3) {
            if (!optab2[x]) {
		outp--;
                if (!optab2[x + 1]) {
                    yyerrorp("illegal operator use in %cif");
                    return 0;
		}
                break;
	    }
            if (value2 == optab2[x])
                break;
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
    outp--;
    return value;
}


#define SKIPW(foo) while (isspace(*foo)) foo++;

static void handle_define()
{
    char namebuf[NSIZE];
    char args[NARGS][NSIZE];
    char mtext[MLEN];
    char *end;
    register char *tmp = outp, *q;

    q = namebuf;
    end = q + NSIZE - 1;
    while (isalunum(*tmp)){
	if (q < end) *q++ = *tmp++;
	else yyerror("Name too long.\n");
    }
    if (q == namebuf) yyerror("Macro name missing.\n");
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
		while (isalunum(*tmp) || (*tmp == '#')){
		    if (q < end) *q++ = *tmp++;
		    else yyerror("Name too long.\n");
		}
		if (q == args[arg]){
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


void yyerror P1(char *, str)
{
    fprintf(stderr, "%s:%d: %s\n", current_file, current_line, str);
    exit(1);
}

void yyerrorp P1(char *, str)
{
    char buff[200];
    sprintf(buff, str, ppchar);
    fprintf(stderr, "%s:%d: %s\n", current_file, current_line, buff);
    exit(1);
}

int ident P1(int, c)
{
    char buff[100];
    int len;

    for (len = 0; isalunum(c); c = getc(yyin)) {
	buff[len++] = c;
	if (len == sizeof buff - 1) {
	    yyerror("Too long indentifier");
	    break;
	}
    }
    (void)ungetc(c, yyin);
    buff[len] = '\0';
    if (!strcmp(buff, "default"))
	return DEFAULT;
    if (!strcmp(buff, "operator"))
	return OPERATOR;
    if (!strcmp(buff, "package"))
	return PACKAGE;

    yylval.string = (char *)malloc(strlen(buff)+1);
    strcpy(yylval.string, buff);
    return ID;
}

char *type_str P1(int, n)
{
    int i, type = n & 0xffff;

    for (i=0; i < NELEMS(types); i++) {
	if (types[i].num == type) {
	    if (n & 0x10000) {
		static char buff[100];
		if (strlen(types[i].name) + 3 > sizeof buff)
		    mf_fatal("Local buffer too small in type_str()!\n");
		sprintf(buff, "%s *", types[i].name);
		return buff;
	    }
	    return types[i].name;
	}
    }
    return "What?";
}

int yylex1() {
    register int c;
    
    for(;;) {
	switch(c = getc(yyin)){
	case ' ':
	case '\t':
	    continue;
	case '\n':
	    current_line++;
	    continue;
	case '#':
	{
	    int line;

	    char aBuf[2048];
	    fgets(aBuf, 2047, yyin);
	    if (sscanf(aBuf, "%d", &line)) current_line = line;
            current_line++;
	    continue;
	}
	case EOF:
	    return -1;
	default:
	    if (isalunum(c))
		return ident(c);
	    return c;
	}
    }
}

int yylex() {
    return yylex1();
}

char *etype1(n)
    int n;
{
    if (n & 0x10000)
	return "T_POINTER";
    switch(n) {
    case FLOAT:
    return "T_REAL";
    case FUNCTION:
    return "T_FUNCTION";
    case INT:
	return "T_NUMBER";
    case OBJECT:
	return "T_OBJECT";
    case MAPPING:
	return "T_MAPPING";
    case STRING:
	return "T_STRING";
    case BUFFER:
	return "T_BUFFER";
    case MIXED:
	return "T_ANY";	/* 0 means any type */
    default:
	yyerror("Illegal type for argument");
    }
    return "What?";
}

char *etype(n)
    int n;
{
    int i;
    int local_size = 100;
    char *buff = (char *)malloc(local_size);

    for (i=0; i < curr_arg_type_size; i++) {
	if (n == 0)
	    break;
	if (curr_arg_types[i] == 0)
	    n--;
    }
    if (i == curr_arg_type_size)
	return "0";
    buff[0] = '\0';
    for(; curr_arg_types[i] != 0; i++) {
	char *p;
	if (curr_arg_types[i] == VOID)
	    continue;
	if (buff[0] != '\0')
	    strcat(buff, "|");
	p = etype1(curr_arg_types[i]);
	/*
	 * The number 2 below is to include the zero-byte and the next
	 * '|' (which may not come).
	 */
	if (strlen(p) + strlen(buff) + 2 > local_size) {
	    fprintf(stderr, "Buffer overflow!\n");
	    exit(1);
	}
	strcat(buff, etype1(curr_arg_types[i]));
    }
    if (!strcmp(buff, "")) 
      strcpy(buff, "T_ANY");
    return buff;
}

char *ctype(n)
    int n;
{
    static char buff[100];	/* 100 is such a comfortable size :-) */
    char *p = (char *)NULL;

    if (n & 0x10000)
	strcpy(buff, "TYPE_MOD_POINTER|");
    else
	buff[0] = '\0';
    n &= ~0x10000;
    switch(n) {
	case FLOAT: p = "TYPE_REAL"; break;
	case FUNCTION: p = "TYPE_FUNCTION"; break;
	case VOID: p = "TYPE_VOID"; break;
	case STRING: p = "TYPE_STRING"; break;
	case INT: p = "TYPE_NUMBER"; break;
	case OBJECT: p = "TYPE_OBJECT"; break;
	case MAPPING: p = "TYPE_MAPPING"; break;
	case BUFFER: p = "TYPE_BUFFER"; break;
	case MIXED: p = "TYPE_ANY"; break;
	case UNKNOWN: p = "TYPE_UNKNOWN"; break;
	default: yyerror("Bad type!");
    }
    strcat(buff, p);
    if (strlen(buff) + 1 > sizeof buff)
	mf_fatal("Local buffer overwritten in ctype()");
    return buff;
}

void make_package_makefile() {
    FILE *f1, *f2, *f3;
    int i;

    f1 = fopen("packages/Makefile", "w");
    if (!f1) {
	fprintf(stderr, "make_func: unable to open packages/Makefile");
	exit(-1);
    }
    f2 = fopen("packages/GNUmakefile", "w");
    if (!f2) {
	fprintf(stderr, "make_func: unable to open packages/GNUmakefile");
	exit(-1);
    }
    f3 = fopen("packages/SMakefile", "w");
    if (!f3) {
	fprintf(stderr, "make_func: unable to open packages/SMakefile");
	exit(-1);
    }
    fprintf(f1, "# ****** This Makefile generated by make_func\n\nCC=" COMPILER "\nCFLAGS=" CFLAGS "\n\n.c.o:\n\t$(CC) $(CFLAGS) -c $*.c\n\n");
    fprintf(f2, "# ****** This Makefile generated by make_func\n\nCC=" COMPILER "\nCFLAGS=" CFLAGS "\nOBJDIR= ../" OBJDIR "/packages\n\n.c.o:\n\t$(CC) $(CFLAGS) -c $*.c -o $(OBJDIR)/$*.o\n\n");
    fprintf(f3, "# ****** This Makefile generated by make_func\n\nCC=" COMPILER "\nCFLAGS=" CFLAGS "\n\n.c.o:\n\t$(CC) $(CFLAGS) $<\n\n");
    if (num_packages) {
	fprintf(f1, "OBJ=");
	fprintf(f2, "OBJ=");
	fprintf(f3, "OBJ=");
	for (i=0; i<num_packages; i++) {
	    fprintf(f1, "%s.o ", packages[i]);
	    fprintf(f2, "%s.o ", packages[i]);
	    fprintf(f3, "%s.o ", packages[i]);
	}
	fprintf(f1, "\n\n");
	fprintf(f2, "\n\n");
	fprintf(f3, "\n\n");
    }
    /* Have to make clean first so that the Makefile can use packages/*.o */
    fprintf(f1, "all: clean $(OBJ)\n\nclean:\n\t-rm *.o\n");
    fprintf(f2, "all: clean $(OBJDIR) $(OBJ)\n\n$(OBJDIR):\n\t-mkdir $(OBJDIR)\n\nclean:\n\t-rm $(OBJDIR)/*.o\n");
    fprintf(f3, "all: $(OBJ)\n\nclean:\n\t-delete \\#?.o\n");
    fclose(f1);
    fclose(f2);
    fclose(f3);
}

void make_efun_tables()
{
	FILE *fp, *fp2, *fp3;
	int i;

	fp = fopen(EFUN_TABLE,"w");
	if (!fp) {
		fprintf(stderr,"make_func: unable to open %s\n",EFUN_TABLE);
		exit(-1);
	}
	fp2 = fopen(OPC_PROF, "w");
	if (!fp2) {
		fprintf(stderr,"make_func: unable to open %s\n",OPC_PROF);
		exit(-2);
	}
	fp3 = fopen(OPCODES, "w");
	if (!fp3) {
		fprintf(stderr,"make_func: unable to open %s\n",OPCODES);
		exit(-3);
	}
	
	fprintf(fp,"/*\n\tThis file is automatically generated by make_func.\n");
	fprintf(fp,"\tdo not make any manual changes to this file.\n*/\n\n");
	fprintf(fp2,"/*\n\tThis file is automatically generated by make_func.\n");
	fprintf(fp2,"\tdo not make any manual changes to this file.\n*/\n\n");
	fprintf(fp3,"/*\n\tThis file is automatically generated by make_func.\n");
	fprintf(fp3,"\tdo not make any manual changes to this file.\n*/\n\n");
	
	fprintf(fp, "\ntypedef void (*func_t) PROT((void));\n\n");
	fprintf(fp2,"\ntypedef struct opc_s { char *name; int count; } opc_t;\n\n");
	fprintf(fp,"func_t efun_table[] = {\n");
	fprintf(fp2,"opc_t opc_efun[] = {\n");
	for (i = 0; i < (num_buff - 1); i++) {
		if (has_token[i]) {
			fprintf(fp,"\tf_%s,\n",key[i]);
			fprintf(fp2,"{\"%s\", 0},\n",key[i]);
		}
	}
	fprintf(fp,"\tf_%s};\n",key[num_buff - 1]);
	fprintf(fp2,"{\"%s\", 0}};\n",key[num_buff - 1]);
	for (i = 0; i < num_buff; i++) {
	    if (has_token[i])
		fprintf(fp,"void f_%s PROT((void));\n",key[i]);
	}
	fprintf(fp3, "\n/* operators */\n\n");
	for (i = 0; i < op_code; i++) {
	    fprintf(fp3,"#define %-30s %d\n", oper_codes[i], i+1);
	}
	fprintf(fp3,"\n/* efuns */\n#define BASE %d\n\n", op_code+1);
	for (i = 0; i < efun_code; i++) {
	    fprintf(fp3,"#define %-30s %d\n", efun_codes[i], i+op_code+1);
	}
	if (efun_code + op_code < 256) {
	    fprintf(fp3,"#undef NEEDS_CALL_EXTRA\n");
	} else {
	    fprintf(fp3,"#define NEEDS_CALL_EXTRA\n");
	    if (efun_code + op_code > 510) {
		fprintf(stderr, "You have way too many efuns.  Contact the MudOS developers if you really need this many.\n");
	    }
	}
	fprintf(fp3,"\n/* efuns */\n#define NUM_OPCODES %d\n\n", efun_code + op_code);
	fclose(fp);
	fclose(fp2);
	fclose(fp3);
	fp = fopen(EFUN_PROTO,"w");
	fprintf(fp,"/*\n\tThis file is automatically generated by make_func.\n");
	fprintf(fp,"\tdo not make any manual changes to this file.\n*/\n\n");
	if (!fp) {
		fprintf(stderr,"make_func: unable to open %s\n",EFUN_TABLE);
		exit(-1);
	}
	for (i = 0; i < num_buff; i++) {
		if (has_token[i])
			fprintf(fp,"void f_%s PROT((void));\n",key[i]);
	}
	fclose(fp);
}
