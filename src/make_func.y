%{
#define EDIT_SOURCE
#define NO_OPCODES
#include "std.h"
#include "make_func.h"
#include "lex.h"
#include "preprocess.h"
#include "edit_source.h"
#include <stdlib.h>

#ifdef WIN32
#define MSDOS
#include <process.h>
#endif

    void mf_fatal(const char *);
    void yyerror(char const *);

    int num_buff = 0;
    int op_code, efun_code, efun1_code;
    char *oper_codes[MAX_FUNC];
    char *efun_codes[MAX_FUNC], *efun1_codes[MAX_FUNC];
    char *efun_names[MAX_FUNC], *efun1_names[MAX_FUNC];
    const char *key[MAX_FUNC], *buf[MAX_FUNC];

    int min_arg = -1, limit_max = 0;

/*
 * arg_types is the types of all arguments. A 0 is used as a delimiter,
 * marking next argument. An argument can have several types.
 */
int arg_types[1000], last_current_type;

/*
 * Store the types of the current efun. They will be copied into the
 * arg_types list if they were not already there (to save memory).
 */
int curr_arg_types[40], curr_arg_type_size;

struct type {
    const char *name;
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

#define NELEMS(arr) 	(sizeof arr / sizeof arr[0])
%}

%union {
    int number;
    const char *string;
}

%token ID NUM DEFAULT OPERATOR

%type <number> type arg_list basic typel arg_type typel2 NUM

%type <string> ID optional_ID optional_default

%%

specs: /* empty */ | specs spec ;

spec: operator | func;

operator: OPERATOR op_list ';' ;

op_list: op | op_list ',' op ;

op: ID
    {
	char f_name[500],c;
	int i = 2;
	sprintf(f_name, "F_%s", $1);
	while ((c = f_name[i])) {
	    if (islower(c)) f_name[i++] = toupper(c);
	    else i++;
	}
	oper_codes[op_code] = (char *) malloc(i+1);
	strcpy(oper_codes[op_code], f_name);
        free((void *)$1);

	op_code++;
    } ;

optional_ID: ID | /* empty */ { $$ = ""; } ;

optional_default: /* empty */ { $$="DEFAULT_NONE"; }
                | DEFAULT ':' NUM
                  {
		      static char buf[40];
                      sprintf(buf, "%i", $3);
                      $$ = buf;
		  }
                | DEFAULT ':' ID
                  {
                      if (strcmp($3, "F__THIS_OBJECT"))
                          yyerror("Illegal default");
                      $$ = "DEFAULT_THIS_OBJECT";
                  } ;

func: type ID optional_ID '(' arg_list optional_default ')' ';'
    {
	char buff[500];
	char f_name[500];
	int i, len;
	if (min_arg == -1)
	    min_arg = $5;
        if (min_arg > 4) mf_fatal("min_arg > 4\n");
	if ($3[0] == '\0') {
	    if (strlen($2) + 1 + 2 > sizeof f_name)
		mf_fatal("A local buffer was too small!(1)\n");
	    sprintf(f_name, "F_%s", $2);
	    len = strlen(f_name);
	    for (i=0; i < len; i++) {
		if (islower(f_name[i]))
		    f_name[i] = toupper(f_name[i]);
	    }
            if (min_arg == 1 && !limit_max && $5 == 1) {
	        efun1_codes[efun1_code] = (char *) malloc(len + 1);
	        strcpy(efun1_codes[efun1_code], f_name);
		efun1_names[efun1_code] = (char *) malloc(len - 1);
		strcpy(efun1_names[efun1_code], $2);
	        efun1_code++;
	    } else {
	        efun_codes[efun_code] = (char *) malloc(len + 1);
	        strcpy(efun_codes[efun_code], f_name);
		efun_names[efun_code] = (char *) malloc(len - 1);
		strcpy(efun_names[efun_code], $2);
	        efun_code++;
	    }
	} else {
	    if (strlen($3) + 1 + 17 > sizeof f_name)
		mf_fatal("A local buffer was too small(2)!\n");
	    sprintf(f_name, "F_%s | F_ALIAS_FLAG", $3);
	    len = strlen(f_name);
	    for (i=0; i < len; i++) {
		if (islower(f_name[i]))
		    f_name[i] = toupper(f_name[i]);
	    }
	    free((void *)$3);
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
        if (!strcmp($2, "call_other") && !lookup_define("CAST_CALL_OTHERS")) {
	    $1 = MIXED;
	}
     	sprintf(buff, "{\"%s\",%s,0,0,%d,%d,%s,%s,%s,%s,%s,%d,%s},\n",
		$2, f_name, min_arg, limit_max ? -1 : $5,
		$1 != VOID ? ctype($1) : "TYPE_NOVALUE",
		etype(0), etype(1), etype(2), etype(3), i, $6);
	if (strlen(buff) > sizeof buff)
	    mf_fatal("Local buffer overwritten !\n");

        key[num_buff] = $2;
	buf[num_buff] = (char *) malloc(strlen(buff) + 1);
        strcpy((char *)buf[num_buff], buff);
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
        free((void *)$1);
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

const char *ctype (int n)
{
    static char buff[100];	/* 100 is such a comfortable size :-) */
    const char *p = (char *)NULL;

    if (n & 0x10000)
	strcpy(buff, "TYPE_MOD_ARRAY|");
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

const char *etype1 (int n)
{
    if (n & 0x10000)
	return "T_ARRAY";
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

const char *etype (int n)
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
	return "T_ANY";
    buff[0] = '\0';
    for(; curr_arg_types[i] != 0; i++) {
	const char *p;
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

int ident (int);

int yylex() {
    register int c;

    for(;;) {
	switch(c = getc(yyin)){
	case ' ':
	case '\t':
	    continue;
	case '\n':
	    current_line++;
	    continue;
	case '!':
	{
	    char buff[2048];
	    fgets(buff, 2047, yyin);
	    fprintf(stderr, "Configuration problem: %s\n", buff);
	    exit(-1);
	}
	case '#':
	{
	    int line;

	    char aBuf[2048], fname[2048];
	    fgets(aBuf, 2047, yyin);
	    if (sscanf(aBuf, "%d \"%[^\"]\"", &line, fname)) {
		current_line = line;
		if (current_file) free(current_file);
		current_file = (char*)malloc(strlen(fname) + 1);
		strcpy(current_file, fname);
	    } else
	    if (sscanf(aBuf, "%d", &line)) current_line = line;
            current_line++;
	    continue;
	}
	case EOF:
	    fclose(yyin);
	    return -1;
	default:
	    if ((c >= '0' && c <= '9') || c == '-') {
		int v;
		int neg = 0;
		v = 0;
		if (c == '-') {
		    neg = 1;
		    c = '0';
		}
		do {
		    v = v * 10 + (c - '0');
		    c = getc(yyin);
		} while (c >= '0' && c <= '9');
		ungetc(c, yyin);
		yylval.number = (neg ? -v : v);
		return NUM;
	    }
	    if (isalunum(c))
		return ident(c);
	    return c;
	}
    }
}

int ident (int c)
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

    yylval.string = (char *)malloc(strlen(buff)+1);
    strcpy((char *)yylval.string, buff);
    return ID;
}

