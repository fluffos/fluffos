%{
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#ifdef __386BSD__
#include <stdlib.h>
#endif
#include "config.h"
#define _YACC_
#include "lint.h"
#include "mudlib_stats.h"
#include "interpret.h"
#include "mapping.h"

#ifndef BUFSIZ
#define BUFSIZ 		1024
#endif
#define NELEMS(arr) 	(sizeof arr / sizeof arr[0])

#define FUNC_SPEC "func_spec.cpp"

#define MAX_FUNC  	2048  /* If we need more than this we're in trouble! */
int num_buff;
int op_code, efun_code;

/* For quick sort purposes : */
char *key[MAX_FUNC], *buf[MAX_FUNC], has_token[MAX_FUNC];

char *oper_codes[MAX_FUNC], *efun_codes[MAX_FUNC];

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
int yylex();
int yyparse();
int ungetc PROT((int c, FILE *f));
char *type_str PROT((int)), *etype PROT((int)), *etype1 PROT((int)),
   *ctype PROT((int));
#ifndef toupper
int toupper PROT((int));
#endif

#define VOID		1
#define INT		2
#define STRING		3
#define OBJECT		4
#define MAPPING		5
#define MIXED		6
#define UNKNOWN		7
#define FLOAT		8
#define FUNCTION	9
#define BUFFER     10

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

void fatal(str)
    char *str;
{
    fprintf(stderr, "%s", str);
    exit(1);
}

%}
%union {
    int number;
    char *string;
}

%token ID DEFAULT OPERATOR

%type <number> type arg_list basic typel arg_type typel2

%type <string> ID optional_ID optional_default

%%

specs: /* empty */ | specs spec ;

spec: operator | func ;
    
operator: OPERATOR op_list ';' ;
    
op_list: op | op_list ',' op ;

op: ID
    {
	char f_name[500];
	int len, i;
	sprintf(f_name, "F_%s", $1);
	len = strlen(f_name);
	for (i=0; i < len; i++) {
	    if (islower(f_name[i]))
		f_name[i] = toupper(f_name[i]);
	}
	oper_codes[op_code] = (char *) malloc(len + 1);
	strcpy(oper_codes[op_code], f_name);
	op_code++;
    } ;

optional_ID: ID | /* empty */ { $$ = ""; } ;

optional_default: DEFAULT ':' ID { $$ = $3; } | /* empty */ { $$="0"; } ;

func: type ID optional_ID '(' arg_list optional_default ')' ';'
    {
	char buff[500];
	char f_name[500];
	int i, len;
	if (min_arg == -1)
	    min_arg = $5;
	if ($3[0] == '\0') {
	    if (strlen($2) + 1 + 2 > sizeof f_name)
		fatal("A local buffer was too small!(1)\n");
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
	    if (strlen($3) + 1 > sizeof f_name)
		fatal("A local buffer was too small(2)!\n");
	    sprintf(f_name, "F_%s", $3);
	    len = strlen(f_name);
	    for (i=0; i < len; i++) {
		if (islower(f_name[i]))
		    f_name[i] = toupper(f_name[i]);
	    }
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
		$2, f_name, min_arg, limit_max ? -1 : $5, ctype($1),
		etype(0), etype(1), i, $6);
	if (strlen(buff) > sizeof buff)
	    fatal("Local buffer overwritten !\n");
	key[num_buff] = (char *) malloc(strlen($2) + 1);
	strcpy(key[num_buff], $2);
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
	int i;
	$$ = 0;
	for (i=0; i < NELEMS(types); i++) {
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
    } ;

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

FILE *f;
int current_line = 1;

int main(argc, argv)
    int argc;
    char **argv;
{
    int i;
    void make_efun_tables();

    num_buff = op_code = efun_code = 0;

    if ((f = fopen(FUNC_SPEC, "r")) == NULL) { 
	perror(FUNC_SPEC);
	exit(1);
    }

    yyparse();

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
    fclose(f);
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
    int len;

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
    if (strcmp(buff, "default") == 0)
	return DEFAULT;
    if (strcmp(buff, "operator") == 0)
	return OPERATOR;
    yylval.string = (char *)malloc(strlen(buff)+1);
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
    return "What?";
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
/* no prototype in <stdio.h> *sigh* */
#if defined(sun) && !defined(SunOS_5)
	    extern int fscanf PROT((FILE *, char *, ...));
#endif
	    int line;
	    /* does any operating system support longer pathnames? */
	    char aBuf[2048];

		fgets(aBuf, 2047, f);
	    if (sscanf(aBuf, "%d", &line) == 2)
		current_line = line;
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
	return "0";	/* 0 means any type */
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
	fatal("Local buffer overwritten in ctype()");
    return buff;
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
	
	fprintf(fp, "\ntypedef void (*func_t) PROT((int, int));\n\n");
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
			fprintf(fp,"void f_%s PROT((int, int));\n",key[i]);
	}
	fprintf(fp3, "\n/* operators */\n\n");
	for (i = 0; i < op_code; i++) {
		fprintf(fp3,"#define %-30s %d\n", oper_codes[i], i+1);
	}
	fprintf(fp3,"\n/* efuns */\n#define BASE %d\n\n", op_code+1);
	for (i = 0; i < efun_code; i++) {
		fprintf(fp3,"#define %-30s %d\n", efun_codes[i], i+op_code+1);
	}
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
			fprintf(fp,"void f_%s PROT((int, int));\n",key[i]);
	}
	fclose(fp);
}
