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
#define T_PROMISE 11

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
int varargs_at = -1; /* the position declared `...` */

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

/*
 * The C++ signature of every efun (primary spec entries only: an alias
 * compiles to its primary's opcode and shares its implementation). The
 * generated thunk checks each declared argument against these types and
 * hands it to the efun as a typed parameter.
 */
struct efun_sig {
  int ret;     /* spec type of the result; T_VOID for a void efun */
  int nargs;   /* declared argument positions, including a trailing ... */
  int min_arg; /* positions below this are always present */
  int varargs; /* the last position is ... */
  int types[40];
  int ntypes; /* curr_arg_types, 0-delimited per position */
};
efun_sig efun_sigs[MAX_FUNC];

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
    {"buffer", T_BUFFER},
    {"promise", T_PROMISE}
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
  int nargs = $5;
  if (limit_max && varargs_at != nargs - 1) {
    char msg[300];
    snprintf(msg, sizeof msg,
             "efun '%s': `...` must be the last argument (nothing after it can be typed)", $2);
    yyerror(msg);
  }
  /* `f(void)` declares no arguments (as in C), not one optional argument
   * of no type: a position with an empty type list that is not `...`. */
  if (nargs == 1 && curr_arg_type_size == 1 && !limit_max) {
    nargs = 0;
    curr_arg_type_size = 0;
    min_arg = 0;
  }
  if (min_arg == -1) min_arg = nargs;
  if (min_arg > 127) yyerror("min_arg > 127\n");
  if ($3[0] == '\0') {
    if (strlen($2) + 1 + 2 > sizeof f_name) yyerror("A local buffer was too small!(1)\n");
    sprintf(f_name, "F_%s", $2);
    len = strlen(f_name);
    for (i = 0; i < len; i++) {
      if (islower(f_name[i])) f_name[i] = toupper(f_name[i]);
    }
    for (i = 0; i < efun_code; i++) {
      if (!strcmp(efun_names[i], $2)) {
        char msg[300];
        snprintf(msg, sizeof msg,
                 "efun '%s' is declared twice; declare it in one package's spec (or make "
                 "the second an alias)",
                 $2);
        yyerror(msg);
      }
    }
    efun_codes[efun_code] = (char *)malloc(len + 1);
    strcpy(efun_codes[efun_code], f_name);
    efun_names[efun_code] = (char *)malloc(len - 1);
    strcpy(efun_names[efun_code], $2);
    {
      efun_sig *sig = &efun_sigs[efun_code];
      sig->ret = $1;
      sig->nargs = nargs;
      sig->min_arg = min_arg;
      sig->varargs = limit_max;
      sig->ntypes = curr_arg_type_size;
      memcpy(sig->types, curr_arg_types, sizeof(int) * curr_arg_type_size);
    }
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
          limit_max ? -1 : nargs, $1 != T_VOID ? ctype($1) : "TYPE_NOVALUE",
          etype(0).c_str(), etype(1).c_str(), etype(2).c_str(), etype(3).c_str(), i, $6);

  if (strlen(buff) > sizeof buff) yyerror("Local buffer overwritten !\n");

  key[num_buff] = $2;
  buf[num_buff] = (char *)malloc(strlen(buff) + 1);
  strcpy((char *)buf[num_buff], buff);
  num_buff++;
  min_arg = -1;
  limit_max = 0;
  varargs_at = -1;
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
  varargs_at = 0;
  for (int k = 0; k < curr_arg_type_size; k++) {
    if (curr_arg_types[k] == 0) varargs_at++;
  }
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
      case T_PROMISE:
        /* promise<mixed>: the spec language has no payload syntax, and no
           efun declares one (issue #1319) */
        p = "(TYPE_MOD_PROMISE|TYPE_ANY)";
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
    case T_PROMISE:
      return "T_PROMISE";
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

/* ---- typed efun bindings ---- */

/* The non-void types declared at one argument position. */
struct arg_shape {
  int types[40];
  int n;
  int optional; /* the position may be absent */
};

static void arg_position(const efun_sig *sig, int pos, arg_shape *out) {
  int i = 0, p = 0;
  for (; i < sig->ntypes && p < pos; i++) {
    if (sig->types[i] == 0) p++;
  }
  out->n = 0;
  for (; i < sig->ntypes && sig->types[i] != 0; i++) {
    if (sig->types[i] != T_VOID) out->types[out->n++] = sig->types[i];
  }
  out->optional = pos >= sig->min_arg;
}

/* The C++ parameter type and vm::EfunArgs getter for a position of exactly
 * one type, or null for a union or mixed (which binds as a vm::Arg). */
static const char *scalar_cpp(int t, const char **getter) {
  if (t & 0x10000) {
    *getter = "array_arg";
    return "array_t*";
  }
  switch (t) {
    case T_INT:
      *getter = "int_arg";
      return "LPC_INT";
    case T_FLOAT:
      *getter = "real_arg";
      return "LPC_FLOAT";
    case T_STRING:
      *getter = "string_arg";
      return "vm::StringArg";
    case T_OBJECT:
      *getter = "object_arg";
      return "object_t*";
    case T_MAPPING:
      *getter = "mapping_arg";
      return "mapping_t*";
    case T_BUFFER:
      *getter = "buffer_arg";
      return "buffer_t*";
    case T_FUNCTION:
      *getter = "funptr_arg";
      return "funptr_t*";
    case T_PROMISE:
      *getter = "promise_arg";
      return "promise_t*";
    default:
      return NULL;
  }
}

/* A position binds as a typed scalar when every type declared there is
 * the same runtime type (`int * | float *` is just an array). */
static const char *shape_scalar(const arg_shape *a, const char **getter) {
  if (a->n == 0) return NULL;
  for (int i = 0; i < a->n; i++) {
    if (a->types[i] == T_MIXED) return NULL;
    if (strcmp(etype1(a->types[i]), etype1(a->types[0])) != 0) return NULL;
  }
  return scalar_cpp(a->types[0], getter);
}

/* The T_* mask a union position accepts. */
static std::string shape_mask(const arg_shape *a) {
  std::string m;
  for (int i = 0; i < a->n; i++) {
    if (a->types[i] == T_MIXED) return "T_ANY";
    if (!m.empty()) m += "|";
    m += etype1(a->types[i]);
  }
  if (m.empty()) return "T_ANY";
  return m;
}

static std::string param_type(const arg_shape *a) {
  const char *getter;
  const char *cpp = shape_scalar(a, &getter);
  if (!a->optional) {
    return cpp ? cpp : "vm::Arg";
  }
  std::string t = cpp ? cpp : "vm::Arg";
  return "std::optional<" + std::move(t) + ">";
}

/* A readable parameter name for the generated prototype. Specs have no
 * parameter names, so invent one from the C++ type; the second string is
 * str2, the second object ob2, etc. Declarations name them so link errors
 * and efuns.autogen.h document the shape; definitions may rename freely. */
static std::string param_name(const arg_shape *a, int *counts) {
  const char *getter;
  const char *cpp = shape_scalar(a, &getter);
  const char *base;
  int kind;
  if (!cpp) {
    base = "arg";
    kind = 0;
  } else if (!strcmp(cpp, "LPC_INT")) {
    base = "n";
    kind = 1;
  } else if (!strcmp(cpp, "LPC_FLOAT")) {
    base = "x";
    kind = 2;
  } else if (!strcmp(cpp, "vm::StringArg")) {
    base = "str";
    kind = 3;
  } else if (!strcmp(cpp, "object_t*")) {
    base = "ob";
    kind = 4;
  } else if (!strcmp(cpp, "array_t*")) {
    base = "arr";
    kind = 5;
  } else if (!strcmp(cpp, "mapping_t*")) {
    base = "map";
    kind = 6;
  } else if (!strcmp(cpp, "buffer_t*")) {
    base = "buf";
    kind = 7;
  } else if (!strcmp(cpp, "funptr_t*")) {
    base = "fun";
    kind = 8;
  } else if (!strcmp(cpp, "promise_t*")) {
    base = "prom";
    kind = 9;
  } else {
    base = "arg";
    kind = 0;
  }
  counts[kind]++;
  if (counts[kind] == 1) {
    return base;
  }
  char buf[32];
  snprintf(buf, sizeof buf, "%s%d", base, counts[kind]);
  return buf;
}

static std::string param_expr(const arg_shape *a, int pos) {
  char buf[200];
  const char *getter;
  const char *cpp = shape_scalar(a, &getter);
  if (cpp) {
    if (a->optional) {
      snprintf(buf, sizeof buf, "f.optional<&vm::EfunArgs::%s>(%d)", getter, pos);
    } else {
      snprintf(buf, sizeof buf, "f.%s(%d)", getter, pos);
    }
  } else if (shape_mask(a) == "T_ANY") {
    snprintf(buf, sizeof buf, a->optional ? "f.optional_mixed(%d)" : "f.mixed_arg(%d)", pos);
  } else {
    snprintf(buf, sizeof buf, a->optional ? "f.optional_any(%d, %s)" : "f.any_arg(%d, %s)", pos,
             shape_mask(a).c_str());
  }
  return buf;
}

/* Typed positions: every declared one but a trailing `...`. */
static int typed_positions(const efun_sig *sig) { return sig->nargs - (sig->varargs ? 1 : 0); }

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
  fprintf(f, "#include \"vm/internal/base/efun_args_inline.h\"\n");

  fprintf(f, "\n/* Operator names */\n");
  fprintf(f, "const char* operator_names[] = {\n");
  fprintf(f, "    \"INVALID OPCODE\", // 0\n");
  for (int i = 0; i < op_code; i++) {
    fprintf(f, "    \"%s\", // %s: %d \n ", oper_codes[i], oper_codes[i], i+1);
  }
  fprintf(f, "};\n");

  fprintf(f, "\n// EFUN thunks: each checks the arguments its spec types, passes them to the\n"
             "// efun as typed parameters, and replaces the arguments with its result.\n\n");
  for (int i = 0; i < efun_code; i++) {
    const efun_sig *sig = &efun_sigs[i];
    std::string call = std::string("f_") + efun_names[i] + "(f";
    for (int pos = 0; pos < typed_positions(sig); pos++) {
      arg_shape a;
      arg_position(sig, pos, &a);
      call += ", " + param_expr(&a, pos);
    }
    call += ")";
    /* The thunk builds the window itself, with its own opcode as a
     * constant: the dispatcher passes only the argument count. */
    fprintf(f, "static void efun_thunk_%s(int num_arg) {\n", efun_names[i]);
    if (!sig->varargs && sig->min_arg == sig->nargs) {
      /* fixed arity, which the dispatchers enforce: a constant count lets
       * the compiler fold the window release */
      fprintf(f, "  DEBUG_CHECK(num_arg != %d, \"%s: wrong argument count\\n\");\n", sig->nargs,
              efun_names[i]);
      fprintf(f, "  vm::EfunArgs f = vm::EfunArgs::over_top(%d, %s);\n", sig->nargs, efun_codes[i]);
    } else {
      fprintf(f, "  vm::EfunArgs f = vm::EfunArgs::over_top(num_arg, %s);\n", efun_codes[i]);
    }
    if (sig->ret == T_VOID) {
      fprintf(f, "  %s;\n  f.finish_void();\n}\n", call.c_str());
    } else {
      /* the efun's Value dies in its own statement, before finish() runs:
       * nothing can touch it in between, so it never needs memory */
      fprintf(f, "  svalue_t result = %s.release();\n  f.finish(result);\n}\n", call.c_str());
    }
  }

  fprintf(f, "\n// EFUN tables\n\n");
  fprintf(f, "func_t efun_table[] = {\n");
  for (int i = 0; i < efun_code; i++) {
    fprintf(f, "    efun_thunk_%s,\n", efun_names[i]);
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

  fprintf(f, "int efun_arg_etypes[] = {\n");
  for (int i = 0; i < last_current_type; i++) {
    if (arg_types[i] == 0) {
      fprintf(f, "0,\n");
    } else {
      fprintf(f, "%s, ", etype1(arg_types[i]));
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

  fprintf(f, "#include \"vm/internal/base/efun_args.h\"\n\n");
  fprintf(f, "\n/* EFUN implementations, one per primary spec entry, generated from the\n"
             " * spec: a void efun returns void, any other returns a vm::Value. Each\n"
             " * declared argument arrives typed (see efun_args.h): a single type as its\n"
             " * C++ type, a union or mixed as a vm::Arg, a position that may be absent as\n"
             " * a std::optional. A trailing ... is read through f.\n"
             " */\n\n");
  for (int i = 0; i < efun_code; i++) {
    const efun_sig *sig = &efun_sigs[i];
    std::string params;
    int name_counts[10] = {};
    for (int pos = 0; pos < typed_positions(sig); pos++) {
      arg_shape a;
      arg_position(sig, pos, &a);
      params += ", " + param_type(&a) + " " + param_name(&a, name_counts);
    }
    fprintf(f, "%s f_%s(vm::EfunArgs& f%s);\n", sig->ret == T_VOID ? "void" : "vm::Value",
            efun_names[i], params.c_str());
  }
  fprintf(f, "/* An efun's entry in efun_table: the generated thunk, given the number\n"
             " * of arguments on the stack. */\n");
  fprintf(f, "typedef void (*func_t)(int num_arg);\n\n");
  fprintf(f, "extern func_t efun_table[];\n");
  fprintf(f, "extern const char* operator_names[];\n");

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
