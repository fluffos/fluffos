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

#ifndef YY_YY_PROJECTS_GIT_FLUFFOS_BUILD_SRC_GRAMMAR_AUTOGEN_H_INCLUDED
# define YY_YY_PROJECTS_GIT_FLUFFOS_BUILD_SRC_GRAMMAR_AUTOGEN_H_INCLUDED
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
    L_LSH = 271,                   /* L_LSH  */
    L_RSH = 272,                   /* L_RSH  */
    L_ORDER = 273,                 /* L_ORDER  */
    L_NOT = 274,                   /* L_NOT  */
    L_IF = 275,                    /* L_IF  */
    L_ELSE = 276,                  /* L_ELSE  */
    L_SWITCH = 277,                /* L_SWITCH  */
    L_CASE = 278,                  /* L_CASE  */
    L_DEFAULT = 279,               /* L_DEFAULT  */
    L_RANGE = 280,                 /* L_RANGE  */
    L_DOT_DOT_DOT = 281,           /* L_DOT_DOT_DOT  */
    L_WHILE = 282,                 /* L_WHILE  */
    L_DO = 283,                    /* L_DO  */
    L_FOR = 284,                   /* L_FOR  */
    L_FOREACH = 285,               /* L_FOREACH  */
    L_IN = 286,                    /* L_IN  */
    L_BREAK = 287,                 /* L_BREAK  */
    L_CONTINUE = 288,              /* L_CONTINUE  */
    L_RETURN = 289,                /* L_RETURN  */
    L_ARROW = 290,                 /* L_ARROW  */
    L_DOT = 291,                   /* L_DOT  */
    L_OPTIONAL_DOT = 292,          /* L_OPTIONAL_DOT  */
    L_DOT_OPTIONAL = 293,          /* L_DOT_OPTIONAL  */
    L_INHERIT = 294,               /* L_INHERIT  */
    L_COLON_COLON = 295,           /* L_COLON_COLON  */
    L_ARRAY_OPEN = 296,            /* L_ARRAY_OPEN  */
    L_MAPPING_OPEN = 297,          /* L_MAPPING_OPEN  */
    L_FUNCTION_OPEN = 298,         /* L_FUNCTION_OPEN  */
    L_NEW_FUNCTION_OPEN = 299,     /* L_NEW_FUNCTION_OPEN  */
    L_SSCANF = 300,                /* L_SSCANF  */
    L_CATCH = 301,                 /* L_CATCH  */
    L_ARRAY = 302,                 /* L_ARRAY  */
    L_REF = 303,                   /* L_REF  */
    L_PARSE_COMMAND = 304,         /* L_PARSE_COMMAND  */
    L_TIME_EXPRESSION = 305,       /* L_TIME_EXPRESSION  */
    L_CLASS = 306,                 /* L_CLASS  */
    L_NEW = 307,                   /* L_NEW  */
    L_PARAMETER = 308,             /* L_PARAMETER  */
    L_TREE = 309,                  /* L_TREE  */
    L_PREPROCESSOR_COMMAND = 310,  /* L_PREPROCESSOR_COMMAND  */
    LOWER_THAN_ELSE = 311,         /* LOWER_THAN_ELSE  */
    L_EQ = 312,                    /* L_EQ  */
    L_NE = 313                     /* L_NE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 129 "/projects/git/fluffos/src/compiler/internal/grammar.y"

  LPC_INT number; /* 8 */
  LPC_FLOAT real; /* 8 */
  char *string;
  struct argument_t argument;
  ident_hash_elem_t *ihe;
  parse_node_t *node;
  function_context_t *contextp;
  struct {
    parse_node_t *node;
    char num;
  } decl; /* 9 */
  struct {
    uint8_t num_local;
    uint8_t max_num_locals;
    uint16_t context;
    uint16_t save_current_type;
    uint16_t save_exact_types;
  } func_block; /* 8 */

#line 143 "/projects/git/fluffos/build/src/grammar.autogen.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_PROJECTS_GIT_FLUFFOS_BUILD_SRC_GRAMMAR_AUTOGEN_H_INCLUDED  */
