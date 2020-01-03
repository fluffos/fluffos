/* This is to make emacs edit this in C mode: -*-C-*- */
%{
#include "base/std.h"

#include <cstdlib>
#include <cstdio>
#include <cctype>

#define YYDEBUG 1
#define YYERROR_VERBOSE 1

#define IS_ID_CHAR(c) (uisalnum(c) || (c) == '_')

FILE *yyin = 0, *yyout = 0;
int yylex();

char *current_file = nullptr;
int current_line = 0;
void yyerror(const char *str) {
  fprintf(stderr, "%s:%d: %s\n", current_file, current_line, str);
  exit(1);
}

#define MAX_FUNC 2048 /* If we need more than this we're in trouble! */

#define T_VOID 1
#define T_INT 2
#define T_STRING 3
#define T_OBJECT 4
#define T_MAPPING 5
#define T_MIXED 6
#define T_UNKNOWN 7
#define T_FLOAT 8
#define T_FUNCTION 9
#define T_BUFFER 10

extern int num_buff;
extern int op_code, efun_code;
extern char *oper_codes[MAX_FUNC];
extern char *efun_codes[MAX_FUNC];
extern char *efun_names[MAX_FUNC];
extern const char *key[MAX_FUNC], *buf[MAX_FUNC];

extern int arg_types[1000], last_current_type;

const char *ctype(int);
std::string etype(int);

int num_buff = 0;
int op_code, efun_code;
char *oper_codes[MAX_FUNC];
char *efun_codes[MAX_FUNC];
char *efun_names[MAX_FUNC];
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
};

type types[] = {
    {"void", T_VOID},
    {"int", T_INT},
    {"string", T_STRING},
    {"object", T_OBJECT},
    {"mapping", T_MAPPING},
    {"mixed", T_MIXED},
    {"unknown", T_UNKNOWN},
    {"float", T_FLOAT},
    {"function", T_FUNCTION},
    {"buffer", T_BUFFER}
};

#define NELEMS(arr) (sizeof arr / sizeof arr[0])
%}

%union {
  int number;
  const char *string;
}

%token ID NUM DEFAULT OPERATOR

%type<number> type arg_list basic typel arg_type typel2 NUM

%type<string> ID optional_ID optional_default

%%

specs: /* empty */ | specs spec;

spec: operator | func;

operator: OPERATOR op_list ';';

op_list: op | op_list ',' op;

op: ID {
  char f_name[500], c;
  int i = 2;
  sprintf(f_name, "F_%s", $1);
  while ((c = f_name[i])) {
    if (islower(c))
      f_name[i++] = toupper(c);
    else
      i++;
  }
  oper_codes[op_code] = (char *)malloc(i + 1);
  strcpy(oper_codes[op_code], f_name);
  free((void *)$1);

  op_code++;
};

optional_ID: ID | /* empty */ { $$ = ""; };

optional_default : /* empty */ { $$ = "DEFAULT_NONE"; }
| DEFAULT ':' NUM {
  static char buf[40];
  sprintf(buf, "%i", $3);
  $$ = buf;
}
| DEFAULT ':' ID {
  if (strcmp($3, "F__THIS_OBJECT")) yyerror("Illegal default");
  $$ = "DEFAULT_THIS_OBJECT";
  free((void *)$3);
};

func: type ID optional_ID '(' arg_list optional_default ')' ';' {
  char buff[500];
  char f_name[500];
  int i, len;
  if (min_arg == -1) min_arg = $5;
  if (min_arg > 4) yyerror("min_arg > 4\n");
  if ($3[0] == '\0') {
    if (strlen($2) + 1 + 2 > sizeof f_name) yyerror("A local buffer was too small!(1)\n");
    sprintf(f_name, "F_%s", $2);
    len = strlen(f_name);
    for (i = 0; i < len; i++) {
      if (islower(f_name[i])) f_name[i] = toupper(f_name[i]);
    }
    efun_codes[efun_code] = (char *)malloc(len + 1);
    strcpy(efun_codes[efun_code], f_name);
    efun_names[efun_code] = (char *)malloc(len - 1);
    strcpy(efun_names[efun_code], $2);
    efun_code++;
  } else {
    if (strlen($3) + 1 + 17 > sizeof f_name) yyerror("A local buffer was too small(2)!\n");
    sprintf(f_name, "F_%s | F_ALIAS_FLAG", $3);
    len = strlen(f_name);
    for (i = 0; i < len; i++) {
      if (islower(f_name[i])) f_name[i] = toupper(f_name[i]);
    }
    free((void *)$3);
  }
  for (i = 0; i < last_current_type; i++) {
    int j;
    for (j = 0; j + i < last_current_type && j < curr_arg_type_size; j++) {
      if (curr_arg_types[j] != arg_types[i + j]) break;
    }
    if (j == curr_arg_type_size) break;
  }
  if (i == last_current_type) {
    int j;
    for (j = 0; j < curr_arg_type_size; j++) {
      arg_types[last_current_type++] = curr_arg_types[j];
      if (last_current_type == NELEMS(arg_types)) yyerror("Array 'arg_types' is too small");
    }
  }
#ifndef CAST_CALL_OTHER
  if (!strcmp($2, "call_other")) {
    $1 = T_MIXED;
  }
#endif
  sprintf(buff, "{\"%s\",%s,0,0,%d,%d,%s,%s,%s,%s,%s,%d,%s},\n", $2, f_name, min_arg,
          limit_max ? -1 : $5, $1 != T_VOID ? ctype($1) : "TYPE_NOVALUE",
          etype(0).c_str(), etype(1).c_str(), etype(2).c_str(), etype(3).c_str(), i, $6);

  if (strlen(buff) > sizeof buff) yyerror("Local buffer overwritten !\n");

  key[num_buff] = $2;
  buf[num_buff] = (char *)malloc(strlen(buff) + 1);
  strcpy((char *)buf[num_buff], buff);
  num_buff++;
  min_arg = -1;
  limit_max = 0;
  curr_arg_type_size = 0;
};

type: basic | basic '*' { $$ = $1 | 0x10000; };

basic: ID {
  int i = NELEMS(types);
  $$ = 0;
  while (i--) {
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

arg_list : /* empty */ { $$ = 0; }
         | typel2 {
  $$ = 1;
  if ($1) min_arg = 0;
}
         | arg_list ',' typel2 {
  $$ = $1 + 1;
  if ($3) min_arg = $$ - 1;
};

typel2: typel {
  $$ = $1;
  curr_arg_types[curr_arg_type_size++] = 0;
  if (curr_arg_type_size == NELEMS(curr_arg_types)) yyerror("Too many arguments");
};

arg_type: type {
  if ($1 != T_VOID) {
    curr_arg_types[curr_arg_type_size++] = $1;
    if (curr_arg_type_size == NELEMS(curr_arg_types)) yyerror("Too many arguments");
  }
  $$ = $1;
};

typel: arg_type { $$ = ($1 == T_VOID && min_arg == -1); }
| typel '|' arg_type { $$ = (min_arg == -1 && ($1 || $3 == T_VOID)); }
| '.' '.' '.' {
  $$ = min_arg == -1;
  limit_max = 1;
};

%%

const char *ctype(int n) {
static char buff[100]; /* 100 is such a comfortable size :-) */
const char *p = (char *)NULL;

if (n & 0x10000)
  strcpy(buff, "TYPE_MOD_ARRAY|");
else
    buff[0] = '\0';
  n &= ~0x10000;
  switch (n) {
    case T_FLOAT:
      p = "TYPE_REAL";
      break;
    case T_FUNCTION:
      p = "TYPE_FUNCTION";
      break;
    case T_VOID:
      p = "TYPE_VOID";
      break;
    case T_STRING:
      p = "TYPE_STRING";
      break;
    case T_INT:
      p = "TYPE_NUMBER";
      break;
    case T_OBJECT:
      p = "TYPE_OBJECT";
      break;
    case T_MAPPING:
      p = "TYPE_MAPPING";
      break;
    case T_BUFFER:
      p = "TYPE_BUFFER";
      break;
    case T_MIXED:
      p = "TYPE_ANY";
      break;
    case T_UNKNOWN:
      p = "TYPE_UNKNOWN";
      break;
    default:
      yyerror("Bad type!");
  }
  strcat(buff, p);
  if (strlen(buff) + 1 > sizeof buff) yyerror("Local buffer overwritten in ctype()");
  return buff;
}

const char *etype1(int n) {
  if (n & 0x10000) return "T_ARRAY";
  switch (n) {
    case T_FLOAT:
      return "T_REAL";
    case T_FUNCTION:
      return "T_FUNCTION";
    case T_INT:
      return "T_NUMBER";
    case T_OBJECT:
      return "T_OBJECT";
    case T_MAPPING:
      return "T_MAPPING";
    case T_STRING:
      return "T_STRING";
    case T_BUFFER:
      return "T_BUFFER";
    case T_MIXED:
      return "T_ANY"; /* 0 means any type */
    default:
      yyerror("Illegal type for argument");
  }
  return "What?";
}

std::string etype(int n) {
  int i;
  int local_size = 100;
  std::string buff = "";

  for (i = 0; i < curr_arg_type_size; i++) {
    if (n == 0) break;
    if (curr_arg_types[i] == 0) n--;
  }
  if (i == curr_arg_type_size) return "T_ANY";
  buff[0] = '\0';
  for (; curr_arg_types[i] != 0; i++) {
    const char *p;
    if (curr_arg_types[i] == T_VOID) continue;
    if (buff[0] != '\0') buff = buff + "|";
    p = etype1(curr_arg_types[i]);
    /*
     * The number 2 below is to include the zero-byte and the next
     * '|' (which may not come).
     */
    if (strlen(p) + buff.size() + 2 > local_size) {
      fprintf(stderr, "Buffer overflow!\n");
      exit(1);
    }
    buff += etype1(curr_arg_types[i]);
  }
  if (buff == "") buff = "T_ANY";
  return buff;
}

int ident(int);

int yylex() {
  int c;

  for (;;) {
    switch (c = getc(yyin)) {
      case ' ':
      case '\t':
      case '\r':
        continue;
      case '\n':
        current_line++;
        continue;
      case '!': {
        char buff[2048];
        fgets(buff, 2047, yyin);
        fprintf(stderr, "Configuration problem: %s\n", buff);
        exit(-1);
      }
      case '#': {
        // skip to the end of the line
        while ((c = getc(yyin)) != '\n') {
          ;  // empty
        }
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
        if (IS_ID_CHAR(c)) return ident(c);
        return c;
    }
  }
}

int ident(int c) {
  char buff[100];
  int len;

  for (len = 0; IS_ID_CHAR(c); c = getc(yyin)) {
    buff[len++] = c;
    if (len == sizeof buff - 1) {
      yyerror("Too long indentifier");
      break;
    }
  }
  (void)ungetc(c, yyin);
  buff[len] = '\0';
  if (!strcmp(buff, "default")) return DEFAULT;
  if (!strcmp(buff, "operator")) return OPERATOR;

  yylval.string = (char *)malloc(strlen(buff) + 1);
  strcpy((char *)yylval.string, buff);
  return ID;
}

void make_efun_tables();

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: make_func <spec_file>\n");
    return 1;
  }
  if ((yyin = fopen(argv[1], "r")) == NULL) {
    fprintf(stderr, "make_func: unable to open input file: %s\n", argv[1]);
    return 1;
  }
  current_file = new char[strlen(argv[1]) + 1];
  strcpy(current_file, argv[1]);
  current_line = 1;

  num_buff = op_code = efun_code = 0;
  yyparse();

  make_efun_tables();

  exit(0);
}

#define EFUN_CC "efuns.autogen.cc"
#define EFUN_H "efuns.autogen.h"

static const char HEADER[] =
    "// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
    "// !!! This file is automatically generated by make_func. !!!\n"
    "// !!! do not make any manual changes to this file.       !!!\n"
    "// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n";

static const char GUARD_TOP[] =
    "#ifndef __EFUNS_HH__\n#define __EFUNS_HH__\n\n";

static const char GUARD_BOTTOM[] = "\n\n#endif  // __EFUNS_HH__\n";

void make_efun_tables() {
  FILE* f = fopen(EFUN_CC, "w");

  if (!f) {
    fprintf(stderr, "make_func: unable to open %s\n", EFUN_CC);
    exit(-1);
  }

  fprintf(f, HEADER);
  fprintf(f, "#include \"base/std.h\"\n\n");
  fprintf(f, "#include \"" EFUN_H "\"\n");

  fprintf(f, "\n// EFUN tables\n\n");
  fprintf(f, "func_t efun_table[] = {\n");
  for (int i = 0; i < efun_code; i++) {
    fprintf(f, "    f_%s,\n", efun_names[i]);
  }
  fprintf(f, "};\n");

  fprintf(f, "struct keyword_t predefs[] = {\n");
  for (int i = 0; i < num_buff; i++) {
    fprintf(f, "    %s", buf[i]);
  }
  fprintf(f, "};\n");

  fprintf(f, "const int size_of_predefs = sizeof(predefs) / sizeof(predefs[0]);\n");

  // EFUN arg types

  fprintf(f, "int efun_arg_types[] = {\n");
  for (int i = 0; i < last_current_type; i++) {
    if (arg_types[i] == 0) {
      fprintf(f, "0,\n");
    } else {
      fprintf(f, "%s, ", ctype(arg_types[i]));
    }
  }
  fprintf(f, "};\n");

  fclose(f);

  // Now generating Header
  f = fopen(EFUN_H, "w");
  if (!f) {
    fprintf(stderr, "make_func: unable to open %s\n", EFUN_H);
    exit(-1);
  }

  fprintf(f, HEADER);
  fprintf(f, GUARD_TOP);
  fprintf(f, "\n/* operators */\n\n");

  int total_code = 0;
  for (int i = 0; i < op_code; i++) {
    fprintf(f, "#define %-30s %d\n", oper_codes[i], i + 1);
    total_code++;
  }
  fprintf(f, "\n/* efuns */\n");

  int efun_base = op_code + 1;
  fprintf(f, "#define EFUN_BASE %d\n\n", efun_base);
  for (int i = 0; i < efun_code; i++) {
    fprintf(f, "#define %-30s %d\n", efun_codes[i], i + efun_base);
    total_code++;
  }

  fprintf(f, "\n#define NUM_OPCODES %d\n", total_code + 1);

  fprintf(f, "\n/* EFUN prototypes */\n\n");
  for (int i = 0; i < efun_code; i++) {
    fprintf(f, "void f_%s (void);\n", efun_names[i]);
  }
  fprintf(f, "typedef void (*func_t) (void);\n\n");
  fprintf(f, "extern func_t efun_table[];");

  /* Now sort the main_list */
  for (int i = 0; i < num_buff; i++) {
    for (int j = 0; j < i; j++)
      if (strcmp(key[i], key[j]) < 0) {
        const char *tmp;
        tmp = key[i];
        key[i] = key[j];
        key[j] = tmp;
        tmp = buf[i];
        buf[i] = buf[j];
        buf[j] = tmp;
      }
  }

  fprintf(f, "\n // EFUN arguments list (used by compiler.cc) \n\n");

  fprintf(f, "#include \"compiler/internal/keyword.h\"\n");
  fprintf(f, "#include \"compiler/internal/compiler.h\"\n");
  fprintf(f, "#include \"vm/internal/base/svalue.h\"\n");
  fprintf(f, "\n\n");
  fprintf(f, "#define DEFAULT_NONE 0xff\n");
  fprintf(f, "#define DEFAULT_THIS_OBJECT 0xfe\n");
  fprintf(f, "// indicates that the instruction is only used at compile time\n");
  fprintf(f, "#define F_ALIAS_FLAG 1024\n");

  fprintf(f, "\n\n");

  fprintf(f, "extern struct keyword_t predefs[];\n");
  fprintf(f, "extern const int size_of_predefs;\n");

  fprintf(f, GUARD_BOTTOM);

  fclose(f);
}
