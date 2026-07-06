/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_GRAMMAR_AUTOGEN_H_INCLUDED
# define YY_YY_GRAMMAR_AUTOGEN_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    L_STRING = 258,                /* L_STRING  */
    L_NUMBER = 259,                /* L_NUMBER  */
    L_REAL = 260,                  /* L_REAL  */
    L_TEMPLATE_HEAD = 261,         /* L_TEMPLATE_HEAD  */
    L_TEMPLATE_MIDDLE = 262,       /* L_TEMPLATE_MIDDLE  */
    L_TEMPLATE_TAIL = 263,         /* L_TEMPLATE_TAIL  */
    L_BASIC_TYPE = 264,            /* L_BASIC_TYPE  */
    L_TYPE_MODIFIER = 265,         /* L_TYPE_MODIFIER  */
    L_DEFINED_NAME = 266,          /* L_DEFINED_NAME  */
    L_IDENTIFIER = 267,            /* L_IDENTIFIER  */
    L_EFUN = 268,                  /* L_EFUN  */
    L_INC_DEC = 269,               /* L_INC_DEC  */
    L_ASSIGN = 270,                /* L_ASSIGN  */
    L_LAND = 271,                  /* L_LAND  */
    L_LOR = 272,                   /* L_LOR  */
    L_QUESTION_QUESTION = 273,     /* L_QUESTION_QUESTION  */
    L_SHIFT = 274,                 /* L_SHIFT  */
    L_EQ_NE = 275,                 /* L_EQ_NE  */
    L_ORDER = 276,                 /* L_ORDER  */
    L_IF = 277,                    /* L_IF  */
    L_ELSE = 278,                  /* L_ELSE  */
    L_SWITCH = 279,                /* L_SWITCH  */
    L_CASE = 280,                  /* L_CASE  */
    L_DEFAULT = 281,               /* L_DEFAULT  */
    L_RANGE = 282,                 /* L_RANGE  */
    L_DOT_DOT_DOT = 283,           /* L_DOT_DOT_DOT  */
    L_WHILE = 284,                 /* L_WHILE  */
    L_DO = 285,                    /* L_DO  */
    L_FOR = 286,                   /* L_FOR  */
    L_FOREACH = 287,               /* L_FOREACH  */
    L_IN = 288,                    /* L_IN  */
    L_BREAK = 289,                 /* L_BREAK  */
    L_CONTINUE = 290,              /* L_CONTINUE  */
    L_RETURN = 291,                /* L_RETURN  */
    L_ARROW = 292,                 /* L_ARROW  */
    L_INHERIT = 293,               /* L_INHERIT  */
    L_COLON_COLON = 294,           /* L_COLON_COLON  */
    L_OPTIONAL_DOT = 295,          /* L_OPTIONAL_DOT  */
    L_DOT_OPTIONAL = 296,          /* L_DOT_OPTIONAL  */
    L_FUNCTION_OPEN = 297,         /* L_FUNCTION_OPEN  */
    L_SSCANF = 298,                /* L_SSCANF  */
    L_CATCH = 299,                 /* L_CATCH  */
    L_ARRAY = 300,                 /* L_ARRAY  */
    L_REF = 301,                   /* L_REF  */
    L_PARSE_COMMAND = 302,         /* L_PARSE_COMMAND  */
    L_TIME_EXPRESSION = 303,       /* L_TIME_EXPRESSION  */
    L_CLASS = 304,                 /* L_CLASS  */
    L_NEW = 305,                   /* L_NEW  */
    L_PARAMETER = 306,             /* L_PARAMETER  */
    L_TREE = 307,                  /* L_TREE  */
    LOWER_THAN_ELSE = 308          /* LOWER_THAN_ELSE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 163 "$REPO_ROOT$/src/compiler/internal/grammar.y"

  LPC_INT number;              /* integers, opcodes, type flags */
  LPC_FLOAT real;              /* floating-point literals */
  char *string;                /* scratch-allocated string */
  argument_t argument;         /* function parameter list metadata */
  ident_hash_elem_t *ihe;      /* symbol-table entry for a known name */
  parse_node_t *node;          /* parse-tree node (most non-terminals) */
  function_context_t *contextp; /* saved function-context pointer */
  decl_t decl;                 /* block/declaration info (node + local count) */
  func_block_t func_block;     /* saved state for anonymous function body */
  struct {
    struct ident_hash_elem_t *ihe;
    LPC_INT classname_index;
  } class_header;              /* class name and index during class declaration */
  struct {
    parse_node_t *expr;
    LPC_INT saved_context;
    LPC_INT saved_cases_size;
  } switch_header;             /* switch discriminant and saved case-table state */
  struct {
    LPC_INT saved_context;
    LPC_INT saved_refs;
  } call_open;                 /* context/refs captured at the '(' of a call */

#line 142 "$REPO_ROOT$/build/src/grammar.autogen.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif




#ifndef YYPUSH_MORE_DEFINED
# define YYPUSH_MORE_DEFINED
enum { YYPUSH_MORE = 4 };
#endif

typedef struct yypstate yypstate;


int yypush_parse (yypstate *ps,
                  int pushed_char, YYSTYPE const *pushed_val, YYLTYPE *pushed_loc, void* yyscanner);

yypstate *yypstate_new (void);
void yypstate_delete (yypstate *ps);


#endif /* !YY_YY_GRAMMAR_AUTOGEN_H_INCLUDED  */
