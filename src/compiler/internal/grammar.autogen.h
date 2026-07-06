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
    L_BASIC_TYPE = 261,            /* L_BASIC_TYPE  */
    L_TYPE_MODIFIER = 262,         /* L_TYPE_MODIFIER  */
    L_DEFINED_NAME = 263,          /* L_DEFINED_NAME  */
    L_IDENTIFIER = 264,            /* L_IDENTIFIER  */
    L_EFUN = 265,                  /* L_EFUN  */
    L_INC = 266,                   /* L_INC  */
    L_DEC = 267,                   /* L_DEC  */
    L_ASSIGN = 268,                /* L_ASSIGN  */
    L_LAND = 269,                  /* L_LAND  */
    L_LOR = 270,                   /* L_LOR  */
    L_QUESTION_QUESTION = 271,     /* L_QUESTION_QUESTION  */
    L_LSH = 272,                   /* L_LSH  */
    L_RSH = 273,                   /* L_RSH  */
    L_ORDER = 274,                 /* L_ORDER  */
    L_NOT = 275,                   /* L_NOT  */
    L_IF = 276,                    /* L_IF  */
    L_ELSE = 277,                  /* L_ELSE  */
    L_SWITCH = 278,                /* L_SWITCH  */
    L_CASE = 279,                  /* L_CASE  */
    L_DEFAULT = 280,               /* L_DEFAULT  */
    L_RANGE = 281,                 /* L_RANGE  */
    L_DOT_DOT_DOT = 282,           /* L_DOT_DOT_DOT  */
    L_WHILE = 283,                 /* L_WHILE  */
    L_DO = 284,                    /* L_DO  */
    L_FOR = 285,                   /* L_FOR  */
    L_FOREACH = 286,               /* L_FOREACH  */
    L_IN = 287,                    /* L_IN  */
    L_BREAK = 288,                 /* L_BREAK  */
    L_CONTINUE = 289,              /* L_CONTINUE  */
    L_RETURN = 290,                /* L_RETURN  */
    L_ARROW = 291,                 /* L_ARROW  */
    L_DOT = 292,                   /* L_DOT  */
    L_INHERIT = 293,               /* L_INHERIT  */
    L_COLON_COLON = 294,           /* L_COLON_COLON  */
    L_ARRAY_OPEN = 295,            /* L_ARRAY_OPEN  */
    L_MAPPING_OPEN = 296,          /* L_MAPPING_OPEN  */
    L_FUNCTION_OPEN = 297,         /* L_FUNCTION_OPEN  */
    L_NEW_FUNCTION_OPEN = 298,     /* L_NEW_FUNCTION_OPEN  */
    L_SSCANF = 299,                /* L_SSCANF  */
    L_CATCH = 300,                 /* L_CATCH  */
    L_ARRAY = 301,                 /* L_ARRAY  */
    L_REF = 302,                   /* L_REF  */
    L_PARSE_COMMAND = 303,         /* L_PARSE_COMMAND  */
    L_TIME_EXPRESSION = 304,       /* L_TIME_EXPRESSION  */
    L_CLASS = 305,                 /* L_CLASS  */
    L_NEW = 306,                   /* L_NEW  */
    L_PARAMETER = 307,             /* L_PARAMETER  */
    L_TREE = 308,                  /* L_TREE  */
    L_PREPROCESSOR_COMMAND = 309,  /* L_PREPROCESSOR_COMMAND  */
    LOWER_THAN_ELSE = 310,         /* LOWER_THAN_ELSE  */
    L_EQ = 311,                    /* L_EQ  */
    L_NE = 312                     /* L_NE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 137 "$REPO_ROOT$/src/compiler/internal/grammar.y"

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

#line 146 "$REPO_ROOT$/build/src/grammar.autogen.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_GRAMMAR_AUTOGEN_H_INCLUDED  */
