
# line 2 "make_func.y"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include "lint.h"
#include "config.h"

#if !defined(FUNC_SPEC)
#define FUNC_SPEC 	"make list_funcs"
#endif
#define FUNC_TOKENS 	"efun_tokens.y"
#define PRE_LANG        "prelang.y"
#define POST_LANG       "postlang.y"
#define THE_LANG        "lang.y"
#ifndef BUFSIZ
#define BUFSIZ 		1024
#endif
#define NELEMS(arr) 	(sizeof arr / sizeof arr[0])

#define MAX_FUNC  	2048  /* If we need more than this we're in trouble! */
int num_buff;
/* For quick sort purposes : */
char *key[MAX_FUNC], *buf[MAX_FUNC], has_token[MAX_FUNC];

int min_arg = -1, limit_max = 0;

/*
 * arg_types is the types of all arguments. A 0 is used as a delimiter,
 * marking next argument. An argument can have several types.
 */
int arg_types[200], last_current_type;
/*
 * Store the types of the current efun. They will be copied into the
 * arg_types list if they were not already there (to save memory).
 */
int curr_arg_types[MAX_LOCAL], curr_arg_type_size;

void yyerror PROT((char *));
int yylex();
int yyparse();
int ungetc PROT((int c, FILE *f));
char *type_str PROT((int)), *etype PROT((int)), *etype1 PROT((int)),
   *ctype PROT((int));
#ifndef toupper
int toupper PROT((int));
#endif

void fatal(str)
    char *str;
{
    fprintf(stderr, "%s", str);
    exit(1);
}


# line 57 "make_func.y"
typedef union  {
    int number;
    char *string;
} YYSTYPE;
# define ID 257
# define VOID 258
# define INT 259
# define STRING 260
# define OBJECT 261
# define MAPPING 262
# define MIXED 263
# define UNKNOWN 264
# define DEFAULT 265
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern short yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

# line 168 "make_func.y"


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
{ "unknown", UNKNOWN }
};

FILE *f;
int current_line = 1;

int main(argc, argv)
    int argc;
    char **argv;
{
    int i, fdr, fdw;
    char buffer[BUFSIZ + 1];

    if ((f = popen(FUNC_SPEC, "r")) == NULL) { 
	perror(FUNC_SPEC);
	exit(1);
    }
    yyparse();
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
    pclose(f);
    /*
     * Write all the tokens out.  Do this by copying the
     * pre-include portion of lang.y to lang.y, appending
     * this information, then appending the post-include
     * portion of lang.y.  It's done this way because I don't
     * know how to get YACC to #include %token files.  *grin*
     */
    if ((fdr = open(PRE_LANG, O_RDONLY)) < 0) {
       perror(PRE_LANG);
       exit(1);
    }
    unlink(THE_LANG);
    if ((fdw = open(THE_LANG, O_CREAT | O_WRONLY, 0600)) < 0) {
       perror(THE_LANG);
       exit(1);
    }
    while (i = read(fdr, buffer, BUFSIZ))
       write(fdw, buffer, i);
    close(fdr);
    for (i = 0; i < num_buff; i++) {
       if (has_token[i]) {
          char *str;   /* It's okay to mung key[*] now */
          for (str = key[i]; *str; str++)
   	     if (islower(*str)) *str = toupper(*str);
          sprintf(buffer, "%%token F_%s\n", key[i]);
          write(fdw, buffer, strlen(buffer));
       }
    }
    if ((fdr = open(POST_LANG, O_RDONLY)) < 0) {
       perror(POST_LANG);
       exit(1);
    }
    while (i = read(fdr, buffer, BUFSIZ))
       write(fdw, buffer, i);
    close(fdr), close(fdw);
    return 0;
}

void yyerror(str)
    char *str;
{
    fprintf(stderr, "%s:%d: %s\n", FUNC_SPEC, current_line, str);
    exit(1);
}

int ident(c)
    int c;
{
    char buff[100];
    int len, i;

    for (len=0; isalnum(c) || c == '_'; c = getc(f)) {
	buff[len++] = c;
	if (len + 1 >= sizeof buff)
	    fatal("Local buffer in ident() too small!\n");
	if (len == sizeof buff - 1) {
	    yyerror("Too long indentifier");
	    break;
	}
    }
    (void)ungetc(c, f);
    buff[len] = '\0';
    for (i=0; i < NELEMS(types); i++) {
	if (strcmp(buff, types[i].name) == 0) {
	    yylval.number = types[i].num;
	    return types[i].num;
	}
    }
    if (strcmp(buff, "default") == 0)
	return DEFAULT;
    yylval.string = (char *)MALLOC(strlen(buff)+1);
    strcpy(yylval.string, buff);
    return ID;
}

char *type_str(n)
    int n;
{
    int i, type = n & 0xffff;

    for (i=0; i < NELEMS(types); i++) {
	if (types[i].num == type) {
	    if (n & 0x10000) {
		static char buff[100];
		if (strlen(types[i].name) + 3 > sizeof buff)
		    fatal("Local buffer too small in type_str()!\n");
		sprintf(buff, "%s *", types[i].name);
		return buff;
	    }
	    return types[i].name;
	}
    }
    return "What ?";
}

int yylex1() {
    register int c;
    
    for(;;) {
	switch(c = getc(f)) {
	case ' ':
	case '\t':
	    continue;
	case '#':
	{
#ifdef sun /* no prototype in <stdio.h> *sigh* */
	    extern int fscanf PROT((FILE *, char *, ...));
#endif
	    int line;
	    char file[2048]; /* does any operating system support
				longer pathnames? */
	    if ( fscanf(f,"%d \"%s\"",&line,file ) == 2 )
		current_line = line;
	    while(c != '\n' && c != EOF)
		c = getc(f);
	    current_line++;
	    continue;
	}
	case '\n':
	    current_line++;
	    continue;
	case EOF:
	    return -1;
	default:
	    if (isalpha(c))
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
    case INT:
	return "T_NUMBER";
    case OBJECT:
	return "T_OBJECT";
    case MAPPING:
        return "T_MAPPING";
    case STRING:
	return "T_STRING";
    case MIXED:
	return "0";	/* 0 means any type */
    default:
	yyerror("Illegal type for argument");
    }
    return "What ?";
}

char *etype(n)
    int n;
{
    int i;
    int local_size = 100;
    char *buff = (char *)MALLOC(local_size);

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
    char *p;

    if (n & 0x10000)
	strcpy(buff, "TYPE_MOD_POINTER|");
    else
	buff[0] = '\0';
    n &= ~0x10000;
    switch(n) {
    case VOID: p = "TYPE_VOID"; break;
    case STRING: p = "TYPE_STRING"; break;
    case INT: p = "TYPE_NUMBER"; break;
    case OBJECT: p = "TYPE_OBJECT"; break;
    case MAPPING: p = "TYPE_MAPPING"; break;
    case MIXED: p = "TYPE_ANY"; break;
    case UNKNOWN: p = "TYPE_UNKNOWN"; break;
    default: yyerror("Bad type!");
    }
    strcat(buff, p);
    if (strlen(buff) + 1 > sizeof buff)
	fatal("Local buffer overwritten in ctype()");
    return buff;
}
short yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 25
# define YYLAST 230
short yyact[]={

  21,   5,   6,   7,  10,  11,   8,   9,  24,  34,
  15,  12,  26,  32,  33,  30,  27,  13,  28,  16,
  20,  18,  22,   2,   3,   1,  23,  14,  19,   4,
  17,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,  29,  31,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   5,   6,   7,  10,  11,   8,   9,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,  25 };
short yypact[]={

-1000,-257,-1000,-246, -25,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-247,-1000, -21,-1000, -46, -36,-1000,-112,
-1000, -30,-1000, -23, -46, -43,-257, -33, -45,-1000,
-248,-1000,-1000,-1000,-1000 };
short yypgo[]={

   0,  22,  30,  29,  28,  20,  21,  27,  26,  25,
  23 };
short yyr1[]={

   0,   9,   9,   7,   7,   8,   8,  10,   1,   1,
   3,   3,   3,   3,   3,   3,   3,   2,   2,   2,
   6,   5,   4,   4,   4 };
short yyr2[]={

   0,   0,   2,   1,   0,   3,   0,   8,   1,   2,
   1,   1,   1,   1,   1,   1,   1,   0,   1,   3,
   1,   1,   1,   3,   3 };
short yychk[]={

-1000,  -9, -10,  -1,  -3, 258, 259, 260, 263, 264,
 261, 262, 257,  42,  -7, 257,  40,  -2,  -6,  -4,
  -5,  46,  -1,  -8,  44, 265, 124,  46,  41,  -6,
  58,  -5,  46,  59, 257 };
short yydef[]={

   1,  -2,   2,   0,   8,  10,  11,  12,  13,  14,
  15,  16,   4,   9,   0,   3,  17,   6,  18,  20,
  22,   0,  21,   0,   0,   0,   0,   0,   0,  19,
   0,  23,  24,   7,   5 };
# line 1 "/usr/lib/yaccpar"
#ifndef lint
static char yaccpar_sccsid[] = "@(#)yaccpar	4.1	(Berkeley)	2/11/83";
#endif not lint

# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

/*	parser for yacc output	*/

#ifdef YYDEBUG
int yydebug = 0; /* 1 for debugging */
#endif
YYSTYPE yyv[YYMAXDEPTH]; /* where the values are stored */
int yychar = -1; /* current input token number */
int yynerrs = 0;  /* number of errors */
short yyerrflag = 0;  /* error recovery flag */

yyparse() {

	short yys[YYMAXDEPTH];
	short yyj, yym;
	register YYSTYPE *yypvt;
	register short yystate, *yyps, yyn;
	register YYSTYPE *yypv;
	register short *yyxi;

	yystate = 0;
	yychar = -1;
	yynerrs = 0;
	yyerrflag = 0;
	yyps= &yys[-1];
	yypv= &yyv[-1];

 yystack:    /* put a state and value onto the stack */

#ifdef YYDEBUG
	if( yydebug  ) printf( "state %d, char 0%o\n", yystate, yychar );
#endif
		if( ++yyps>= &yys[YYMAXDEPTH] ) { yyerror( "yacc stack overflow" ); return(1); }
		*yyps = yystate;
		++yypv;
		*yypv = yyval;

 yynewstate:

	yyn = yypact[yystate];

	if( yyn<= YYFLAG ) goto yydefault; /* simple state */

	if( yychar<0 ) if( (yychar=yylex())<0 ) yychar=0;
	if( (yyn += yychar)<0 || yyn >= YYLAST ) goto yydefault;

	if( yychk[ yyn=yyact[ yyn ] ] == yychar ){ /* valid shift */
		yychar = -1;
		yyval = yylval;
		yystate = yyn;
		if( yyerrflag > 0 ) --yyerrflag;
		goto yystack;
		}

 yydefault:
	/* default state action */

	if( (yyn=yydef[yystate]) == -2 ) {
		if( yychar<0 ) if( (yychar=yylex())<0 ) yychar = 0;
		/* look through exception table */

		for( yyxi=yyexca; (*yyxi!= (-1)) || (yyxi[1]!=yystate) ; yyxi += 2 ) ; /* VOID */

		while( *(yyxi+=2) >= 0 ){
			if( *yyxi == yychar ) break;
			}
		if( (yyn = yyxi[1]) < 0 ) return(0);   /* accept */
		}

	if( yyn == 0 ){ /* error */
		/* error ... attempt to resume parsing */

		switch( yyerrflag ){

		case 0:   /* brand new error */

			yyerror( "syntax error" );
		yyerrlab:
			++yynerrs;

		case 1:
		case 2: /* incompletely recovered error ... try again */

			yyerrflag = 3;

			/* find a state where "error" is a legal shift action */

			while ( yyps >= yys ) {
			   yyn = yypact[*yyps] + YYERRCODE;
			   if( yyn>= 0 && yyn < YYLAST && yychk[yyact[yyn]] == YYERRCODE ){
			      yystate = yyact[yyn];  /* simulate a shift of "error" */
			      goto yystack;
			      }
			   yyn = yypact[*yyps];

			   /* the current yyps has no shift onn "error", pop stack */

#ifdef YYDEBUG
			   if( yydebug ) printf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1] );
#endif
			   --yyps;
			   --yypv;
			   }

			/* there is no state on the stack with an error shift ... abort */

	yyabort:
			return(1);


		case 3:  /* no shift yet; clobber input char */

#ifdef YYDEBUG
			if( yydebug ) printf( "error recovery discards char %d\n", yychar );
#endif

			if( yychar == 0 ) goto yyabort; /* don't discard EOF, quit */
			yychar = -1;
			goto yynewstate;   /* try again in the same state */

			}

		}

	/* reduction by production yyn */

#ifdef YYDEBUG
		if( yydebug ) printf("reduce %d\n",yyn);
#endif
		yyps -= yyr2[yyn];
		yypvt = yypv;
		yypv -= yyr2[yyn];
		yyval = yypv[1];
		yym=yyn;
			/* consult goto table to find next state */
		yyn = yyr1[yyn];
		yyj = yypgo[yyn] + *yyps + 1;
		if( yyj>=YYLAST || yychk[ yystate = yyact[yyj] ] != -yyn ) yystate = yyact[yypgo[yyn]];
		switch(yym){
			
case 4:
# line 77 "make_func.y"
{ yyval.string = ""; } break;
case 5:
# line 79 "make_func.y"
{ yyval.string = yypvt[-0].string; } break;
case 6:
# line 79 "make_func.y"
{ yyval.string="0"; } break;
case 7:
# line 82 "make_func.y"
{
	char buff[500];
	char f_name[500];
	int i;
	if (min_arg == -1)
	    min_arg = yypvt[-3].number;
	if (yypvt[-5].string[0] == '\0') {
	    int len;
	    if (strlen(yypvt[-6].string) + 1 + 2 > sizeof f_name)
		fatal("A local buffer was too small!(1)\n");
	    sprintf(f_name, "F_%s", yypvt[-6].string);
	    len = strlen(f_name);
	    for (i=0; i < len; i++) {
		if (islower(f_name[i]))
		    f_name[i] = toupper(f_name[i]);
	    }
	    has_token[num_buff]=1;
	} else {
	    if (strlen(yypvt[-5].string) + 1 > sizeof f_name)
		fatal("A local buffer was too small(2)!\n");
	    strcpy(f_name, yypvt[-5].string);
	    has_token[num_buff]=0;
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
	sprintf(buff, "{\"%s\",%s,%d,%d,%s,%s,%s,%d,%s},\n",
		yypvt[-6].string, f_name, min_arg, limit_max ? -1 : yypvt[-3].number, ctype(yypvt[-7].number),
		etype(0), etype(1), i, yypvt[-2].string);
        if (strlen(buff) > sizeof buff)
     	    fatal("Local buffer overwritten !\n");
        key[num_buff] = (char *) MALLOC(strlen(yypvt[-6].string) + 1);
	strcpy(key[num_buff], yypvt[-6].string);
	buf[num_buff] = (char *) MALLOC(strlen(buff) + 1);
	strcpy(buf[num_buff], buff);
        num_buff++;
	min_arg = -1;
	limit_max = 0;
	curr_arg_type_size = 0;
    } break;
case 9:
# line 138 "make_func.y"
{ yyval.number = yypvt[-1].number | 0x10000; } break;
case 17:
# line 142 "make_func.y"
{ yyval.number = 0; } break;
case 18:
# line 143 "make_func.y"
{ yyval.number = 1; if (yypvt[-0].number) min_arg = 0; } break;
case 19:
# line 144 "make_func.y"
{ yyval.number = yypvt[-2].number + 1; if (yypvt[-0].number) min_arg = yyval.number - 1; } break;
case 20:
# line 147 "make_func.y"
{
	yyval.number = yypvt[-0].number;
	curr_arg_types[curr_arg_type_size++] = 0;
	if (curr_arg_type_size == NELEMS(curr_arg_types))
	    yyerror("Too many arguments");
    } break;
case 21:
# line 155 "make_func.y"
{
	if (yypvt[-0].number != VOID) {
	    curr_arg_types[curr_arg_type_size++] = yypvt[-0].number;
	    if (curr_arg_type_size == NELEMS(curr_arg_types))
		yyerror("Too many arguments");
	}
	yyval.number = yypvt[-0].number;
    } break;
case 22:
# line 164 "make_func.y"
{ yyval.number = (yypvt[-0].number == VOID && min_arg == -1); } break;
case 23:
# line 165 "make_func.y"
{ yyval.number = (min_arg == -1 && (yypvt[-2].number || yypvt[-0].number == VOID));} break;
case 24:
# line 166 "make_func.y"
{ yyval.number = min_arg == -1 ; limit_max = 1; } break;
# line 148 "/usr/lib/yaccpar"

		}
		goto yystack;  /* stack new state and value */

	}
