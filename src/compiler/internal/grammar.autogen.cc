/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 11 "/projects/git/fluffos/src/compiler/internal/grammar.y"

#include "base/std.h"

#include "compiler/internal/grammar_rules.h"
#include "vm/vm.h"
#include "vm/internal/base/machine.h"
#include "compiler/internal/compiler.h"
#include "compiler/internal/lex.h"
#include "compiler/internal/scratchpad.h"
#include "compiler/internal/generate.h"
#include "include/opcodes_extra.h"

extern char *outp;

/*
 * This is the grammar definition of LPC, and its parse tree generator.
 */

/* down to one global :)
   bits:
      SWITCH_CONTEXT     - we're inside a switch
      LOOP_CONTEXT       - we're inside a loop
      SWITCH_STRINGS     - a string case has been found
      SWITCH_NUMBERS     - a non-zero numeric case has been found
      SWITCH_RANGES      - a range has been found
      SWITCH_DEFAULT     - a default has been found
 */
int context;
int num_refs;
int func_present;
/*
 * bison & yacc don't prototype this in y.tab.h
 */
int yyparse (void);


#line 108 "/projects/git/fluffos/build/src/grammar.autogen.cc"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "grammar.autogen.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_L_STRING = 3,                   /* L_STRING  */
  YYSYMBOL_L_NUMBER = 4,                   /* L_NUMBER  */
  YYSYMBOL_L_REAL = 5,                     /* L_REAL  */
  YYSYMBOL_L_BASIC_TYPE = 6,               /* L_BASIC_TYPE  */
  YYSYMBOL_L_TYPE_MODIFIER = 7,            /* L_TYPE_MODIFIER  */
  YYSYMBOL_L_DEFINED_NAME = 8,             /* L_DEFINED_NAME  */
  YYSYMBOL_L_IDENTIFIER = 9,               /* L_IDENTIFIER  */
  YYSYMBOL_L_EFUN = 10,                    /* L_EFUN  */
  YYSYMBOL_L_INC = 11,                     /* L_INC  */
  YYSYMBOL_L_DEC = 12,                     /* L_DEC  */
  YYSYMBOL_L_ASSIGN = 13,                  /* L_ASSIGN  */
  YYSYMBOL_L_LAND = 14,                    /* L_LAND  */
  YYSYMBOL_L_LOR = 15,                     /* L_LOR  */
  YYSYMBOL_L_QUESTION_QUESTION = 16,       /* L_QUESTION_QUESTION  */
  YYSYMBOL_L_LSH = 17,                     /* L_LSH  */
  YYSYMBOL_L_RSH = 18,                     /* L_RSH  */
  YYSYMBOL_L_ORDER = 19,                   /* L_ORDER  */
  YYSYMBOL_L_NOT = 20,                     /* L_NOT  */
  YYSYMBOL_L_IF = 21,                      /* L_IF  */
  YYSYMBOL_L_ELSE = 22,                    /* L_ELSE  */
  YYSYMBOL_L_SWITCH = 23,                  /* L_SWITCH  */
  YYSYMBOL_L_CASE = 24,                    /* L_CASE  */
  YYSYMBOL_L_DEFAULT = 25,                 /* L_DEFAULT  */
  YYSYMBOL_L_RANGE = 26,                   /* L_RANGE  */
  YYSYMBOL_L_DOT_DOT_DOT = 27,             /* L_DOT_DOT_DOT  */
  YYSYMBOL_L_WHILE = 28,                   /* L_WHILE  */
  YYSYMBOL_L_DO = 29,                      /* L_DO  */
  YYSYMBOL_L_FOR = 30,                     /* L_FOR  */
  YYSYMBOL_L_FOREACH = 31,                 /* L_FOREACH  */
  YYSYMBOL_L_IN = 32,                      /* L_IN  */
  YYSYMBOL_L_BREAK = 33,                   /* L_BREAK  */
  YYSYMBOL_L_CONTINUE = 34,                /* L_CONTINUE  */
  YYSYMBOL_L_RETURN = 35,                  /* L_RETURN  */
  YYSYMBOL_L_ARROW = 36,                   /* L_ARROW  */
  YYSYMBOL_L_DOT = 37,                     /* L_DOT  */
  YYSYMBOL_L_INHERIT = 38,                 /* L_INHERIT  */
  YYSYMBOL_L_COLON_COLON = 39,             /* L_COLON_COLON  */
  YYSYMBOL_L_ARRAY_OPEN = 40,              /* L_ARRAY_OPEN  */
  YYSYMBOL_L_MAPPING_OPEN = 41,            /* L_MAPPING_OPEN  */
  YYSYMBOL_L_FUNCTION_OPEN = 42,           /* L_FUNCTION_OPEN  */
  YYSYMBOL_L_NEW_FUNCTION_OPEN = 43,       /* L_NEW_FUNCTION_OPEN  */
  YYSYMBOL_L_SSCANF = 44,                  /* L_SSCANF  */
  YYSYMBOL_L_CATCH = 45,                   /* L_CATCH  */
  YYSYMBOL_L_ARRAY = 46,                   /* L_ARRAY  */
  YYSYMBOL_L_REF = 47,                     /* L_REF  */
  YYSYMBOL_L_PARSE_COMMAND = 48,           /* L_PARSE_COMMAND  */
  YYSYMBOL_L_TIME_EXPRESSION = 49,         /* L_TIME_EXPRESSION  */
  YYSYMBOL_L_CLASS = 50,                   /* L_CLASS  */
  YYSYMBOL_L_NEW = 51,                     /* L_NEW  */
  YYSYMBOL_L_PARAMETER = 52,               /* L_PARAMETER  */
  YYSYMBOL_L_TREE = 53,                    /* L_TREE  */
  YYSYMBOL_L_TEMPLATE_HEAD = 54,           /* L_TEMPLATE_HEAD  */
  YYSYMBOL_L_TEMPLATE_MIDDLE = 55,         /* L_TEMPLATE_MIDDLE  */
  YYSYMBOL_L_TEMPLATE_TAIL = 56,           /* L_TEMPLATE_TAIL  */
  YYSYMBOL_L_PREPROCESSOR_COMMAND = 57,    /* L_PREPROCESSOR_COMMAND  */
  YYSYMBOL_LOWER_THAN_ELSE = 58,           /* LOWER_THAN_ELSE  */
  YYSYMBOL_59_ = 59,                       /* '?'  */
  YYSYMBOL_60_ = 60,                       /* '|'  */
  YYSYMBOL_61_ = 61,                       /* '^'  */
  YYSYMBOL_62_ = 62,                       /* '&'  */
  YYSYMBOL_L_EQ = 63,                      /* L_EQ  */
  YYSYMBOL_L_NE = 64,                      /* L_NE  */
  YYSYMBOL_65_ = 65,                       /* '<'  */
  YYSYMBOL_66_ = 66,                       /* '+'  */
  YYSYMBOL_67_ = 67,                       /* '-'  */
  YYSYMBOL_68_ = 68,                       /* '*'  */
  YYSYMBOL_69_ = 69,                       /* '%'  */
  YYSYMBOL_70_ = 70,                       /* '/'  */
  YYSYMBOL_71_ = 71,                       /* '~'  */
  YYSYMBOL_72_ = 72,                       /* ';'  */
  YYSYMBOL_73_ = 73,                       /* '('  */
  YYSYMBOL_74_ = 74,                       /* ')'  */
  YYSYMBOL_75_ = 75,                       /* ':'  */
  YYSYMBOL_76_ = 76,                       /* ','  */
  YYSYMBOL_77_ = 77,                       /* '{'  */
  YYSYMBOL_78_ = 78,                       /* '}'  */
  YYSYMBOL_79_ = 79,                       /* '$'  */
  YYSYMBOL_80_ = 80,                       /* '['  */
  YYSYMBOL_81_ = 81,                       /* ']'  */
  YYSYMBOL_YYACCEPT = 82,                  /* $accept  */
  YYSYMBOL_all = 83,                       /* all  */
  YYSYMBOL_program = 84,                   /* program  */
  YYSYMBOL_possible_semi_colon = 85,       /* possible_semi_colon  */
  YYSYMBOL_inheritance = 86,               /* inheritance  */
  YYSYMBOL_real = 87,                      /* real  */
  YYSYMBOL_number = 88,                    /* number  */
  YYSYMBOL_optional_star = 89,             /* optional_star  */
  YYSYMBOL_block_or_semi = 90,             /* block_or_semi  */
  YYSYMBOL_identifier = 91,                /* identifier  */
  YYSYMBOL_function = 92,                  /* function  */
  YYSYMBOL_93_1 = 93,                      /* $@1  */
  YYSYMBOL_94_2 = 94,                      /* @2  */
  YYSYMBOL_def = 95,                       /* def  */
  YYSYMBOL_modifier_change = 96,           /* modifier_change  */
  YYSYMBOL_member_name = 97,               /* member_name  */
  YYSYMBOL_member_name_list = 98,          /* member_name_list  */
  YYSYMBOL_member_list = 99,               /* member_list  */
  YYSYMBOL_100_3 = 100,                    /* $@3  */
  YYSYMBOL_type_decl = 101,                /* type_decl  */
  YYSYMBOL_102_4 = 102,                    /* @4  */
  YYSYMBOL_new_local_name = 103,           /* new_local_name  */
  YYSYMBOL_atomic_type = 104,              /* atomic_type  */
  YYSYMBOL_opt_atomic_type = 105,          /* opt_atomic_type  */
  YYSYMBOL_basic_type = 106,               /* basic_type  */
  YYSYMBOL_arg_type = 107,                 /* arg_type  */
  YYSYMBOL_optional_default_arg_value = 108, /* optional_default_arg_value  */
  YYSYMBOL_new_arg = 109,                  /* new_arg  */
  YYSYMBOL_argument = 110,                 /* argument  */
  YYSYMBOL_argument_list = 111,            /* argument_list  */
  YYSYMBOL_type_modifier_list = 112,       /* type_modifier_list  */
  YYSYMBOL_type = 113,                     /* type  */
  YYSYMBOL_cast = 114,                     /* cast  */
  YYSYMBOL_opt_basic_type = 115,           /* opt_basic_type  */
  YYSYMBOL_name_list = 116,                /* name_list  */
  YYSYMBOL_new_name = 117,                 /* new_name  */
  YYSYMBOL_block = 118,                    /* block  */
  YYSYMBOL_119_5 = 119,                    /* @5  */
  YYSYMBOL_decl_block = 120,               /* decl_block  */
  YYSYMBOL_local_declarations = 121,       /* local_declarations  */
  YYSYMBOL_122_6 = 122,                    /* $@6  */
  YYSYMBOL_new_local_def = 123,            /* new_local_def  */
  YYSYMBOL_single_new_local_def = 124,     /* single_new_local_def  */
  YYSYMBOL_single_new_local_def_with_init = 125, /* single_new_local_def_with_init  */
  YYSYMBOL_local_name_list = 126,          /* local_name_list  */
  YYSYMBOL_local_declaration_statement = 127, /* local_declaration_statement  */
  YYSYMBOL_128_7 = 128,                    /* $@7  */
  YYSYMBOL_block_statements = 129,         /* block_statements  */
  YYSYMBOL_statement = 130,                /* statement  */
  YYSYMBOL_while = 131,                    /* while  */
  YYSYMBOL_132_8 = 132,                    /* $@8  */
  YYSYMBOL_do = 133,                       /* do  */
  YYSYMBOL_134_9 = 134,                    /* $@9  */
  YYSYMBOL_for = 135,                      /* for  */
  YYSYMBOL_136_10 = 136,                   /* $@10  */
  YYSYMBOL_foreach_var = 137,              /* foreach_var  */
  YYSYMBOL_foreach_vars = 138,             /* foreach_vars  */
  YYSYMBOL_foreach = 139,                  /* foreach  */
  YYSYMBOL_140_11 = 140,                   /* $@11  */
  YYSYMBOL_for_expr = 141,                 /* for_expr  */
  YYSYMBOL_first_for_expr = 142,           /* first_for_expr  */
  YYSYMBOL_switch = 143,                   /* switch  */
  YYSYMBOL_144_12 = 144,                   /* $@12  */
  YYSYMBOL_switch_block = 145,             /* switch_block  */
  YYSYMBOL_case = 146,                     /* case  */
  YYSYMBOL_case_label = 147,               /* case_label  */
  YYSYMBOL_constant = 148,                 /* constant  */
  YYSYMBOL_comma_expr = 149,               /* comma_expr  */
  YYSYMBOL_ref = 150,                      /* ref  */
  YYSYMBOL_expr0 = 151,                    /* expr0  */
  YYSYMBOL_return = 152,                   /* return  */
  YYSYMBOL_expr_list = 153,                /* expr_list  */
  YYSYMBOL_expr_list_node = 154,           /* expr_list_node  */
  YYSYMBOL_expr_list2 = 155,               /* expr_list2  */
  YYSYMBOL_expr_list3 = 156,               /* expr_list3  */
  YYSYMBOL_expr_list4 = 157,               /* expr_list4  */
  YYSYMBOL_assoc_pair = 158,               /* assoc_pair  */
  YYSYMBOL_lvalue = 159,                   /* lvalue  */
  YYSYMBOL_l_new_function_open = 160,      /* l_new_function_open  */
  YYSYMBOL_expr4 = 161,                    /* expr4  */
  YYSYMBOL_162_13 = 162,                   /* @13  */
  YYSYMBOL_163_14 = 163,                   /* @14  */
  YYSYMBOL_expr_or_block = 164,            /* expr_or_block  */
  YYSYMBOL_catch = 165,                    /* catch  */
  YYSYMBOL_166_15 = 166,                   /* @15  */
  YYSYMBOL_tree = 167,                     /* tree  */
  YYSYMBOL_sscanf = 168,                   /* sscanf  */
  YYSYMBOL_parse_command = 169,            /* parse_command  */
  YYSYMBOL_time_expression = 170,          /* time_expression  */
  YYSYMBOL_171_16 = 171,                   /* @16  */
  YYSYMBOL_lvalue_list = 172,              /* lvalue_list  */
  YYSYMBOL_string = 173,                   /* string  */
  YYSYMBOL_string_con1 = 174,              /* string_con1  */
  YYSYMBOL_string_con2 = 175,              /* string_con2  */
  YYSYMBOL_template_literal = 176,         /* template_literal  */
  YYSYMBOL_template_parts = 177,           /* template_parts  */
  YYSYMBOL_string_like = 178,              /* string_like  */
  YYSYMBOL_class_init = 179,               /* class_init  */
  YYSYMBOL_opt_class_init = 180,           /* opt_class_init  */
  YYSYMBOL_function_call = 181,            /* function_call  */
  YYSYMBOL_182_17 = 182,                   /* @17  */
  YYSYMBOL_183_18 = 183,                   /* @18  */
  YYSYMBOL_184_19 = 184,                   /* @19  */
  YYSYMBOL_185_20 = 185,                   /* @20  */
  YYSYMBOL_186_21 = 186,                   /* @21  */
  YYSYMBOL_187_22 = 187,                   /* @22  */
  YYSYMBOL_188_23 = 188,                   /* @23  */
  YYSYMBOL_efun_override = 189,            /* efun_override  */
  YYSYMBOL_function_name = 190,            /* function_name  */
  YYSYMBOL_cond = 191,                     /* cond  */
  YYSYMBOL_optional_else_part = 192        /* optional_else_part  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#ifdef NDEBUG
# define YY_ASSERT(E) ((void) (0 && (E)))
#else
# include <assert.h> /* INFRINGES ON USER NAME SPACE */
# define YY_ASSERT(E) assert (E)
#endif


#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
# define YYCOPY_NEEDED 1
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1839

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  82
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  111
/* YYNRULES -- Number of rules.  */
#define YYNRULES  278
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  524

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   315


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    79,    69,    62,     2,
      73,    74,    68,    66,    76,    67,     2,    70,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    75,    72,
      65,     2,     2,    59,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    80,     2,    81,    61,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    77,    60,    78,    71,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    63,    64
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   223,   223,   227,   228,   231,   233,   237,   241,   245,
     249,   250,   254,   261,   262,   266,   267,   271,   272,   271,
     277,   278,   284,   285,   286,   290,   307,   320,   321,   324,
     326,   326,   331,   331,   336,   337,   347,   348,   357,   365,
     366,   370,   371,   375,   376,   380,   381,   402,   408,   416,
     429,   433,   434,   452,   463,   477,   480,   497,   505,   512,
     514,   520,   521,   525,   550,   606,   605,   614,   614,   614,
     618,   623,   622,   641,   652,   686,   697,   729,   734,   745,
     744,   759,   763,   770,   777,   785,   797,   798,   799,   800,
     801,   802,   807,   811,   833,   846,   845,   859,   858,   872,
     871,   896,   917,   927,   944,   949,   960,   959,   979,   982,
     986,   991,  1000,   999,  1038,  1044,  1051,  1057,  1064,  1078,
    1092,  1105,  1121,  1135,  1150,  1154,  1158,  1162,  1166,  1170,
    1178,  1182,  1186,  1190,  1194,  1198,  1202,  1206,  1210,  1214,
    1218,  1222,  1226,  1233,  1237,  1244,  1248,  1275,  1315,  1320,
    1344,  1350,  1356,  1362,  1387,  1391,  1414,  1436,  1450,  1494,
    1531,  1535,  1539,  1709,  1803,  1883,  1887,  1982,  2003,  2024,
    2046,  2055,  2066,  2090,  2112,  2133,  2134,  2135,  2136,  2137,
    2138,  2142,  2148,  2169,  2172,  2176,  2183,  2187,  2194,  2199,
    2212,  2216,  2220,  2227,  2237,  2255,  2262,  2378,  2379,  2386,
    2387,  2460,  2478,  2483,  2482,  2512,  2536,  2560,  2571,  2575,
    2582,  2589,  2593,  2597,  2642,  2698,  2699,  2703,  2704,  2706,
    2705,  2762,  2800,  2895,  2918,  2927,  2939,  2943,  2951,  2950,
    2963,  2970,  2980,  2989,  3000,  2999,  3013,  3018,  3032,  3040,
    3041,  3045,  3052,  3053,  3060,  3072,  3077,  3089,  3090,  3091,
    3095,  3102,  3113,  3116,  3125,  3124,  3138,  3137,  3168,  3203,
    3222,  3221,  3357,  3356,  3425,  3424,  3476,  3475,  3527,  3526,
    3557,  3577,  3593,  3594,  3608,  3623,  3638,  3672,  3676
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  static const char *const yy_sname[] =
  {
  "end of file", "error", "invalid token", "L_STRING", "L_NUMBER",
  "L_REAL", "L_BASIC_TYPE", "L_TYPE_MODIFIER", "L_DEFINED_NAME",
  "L_IDENTIFIER", "L_EFUN", "L_INC", "L_DEC", "L_ASSIGN", "L_LAND",
  "L_LOR", "L_QUESTION_QUESTION", "L_LSH", "L_RSH", "L_ORDER", "L_NOT",
  "L_IF", "L_ELSE", "L_SWITCH", "L_CASE", "L_DEFAULT", "L_RANGE",
  "L_DOT_DOT_DOT", "L_WHILE", "L_DO", "L_FOR", "L_FOREACH", "L_IN",
  "L_BREAK", "L_CONTINUE", "L_RETURN", "L_ARROW", "L_DOT", "L_INHERIT",
  "L_COLON_COLON", "L_ARRAY_OPEN", "L_MAPPING_OPEN", "L_FUNCTION_OPEN",
  "L_NEW_FUNCTION_OPEN", "L_SSCANF", "L_CATCH", "L_ARRAY", "L_REF",
  "L_PARSE_COMMAND", "L_TIME_EXPRESSION", "L_CLASS", "L_NEW",
  "L_PARAMETER", "L_TREE", "L_TEMPLATE_HEAD", "L_TEMPLATE_MIDDLE",
  "L_TEMPLATE_TAIL", "L_PREPROCESSOR_COMMAND", "LOWER_THAN_ELSE", "'?'",
  "'|'", "'^'", "'&'", "L_EQ", "L_NE", "'<'", "'+'", "'-'", "'*'", "'%'",
  "'/'", "'~'", "';'", "'('", "')'", "':'", "','", "'{'", "'}'", "'$'",
  "'['", "']'", "$accept", "all", "program", "possible_semi_colon",
  "inheritance", "real", "number", "optional_star", "block_or_semi",
  "identifier", "function", "$@1", "@2", "def", "modifier_change",
  "member_name", "member_name_list", "member_list", "$@3", "type_decl",
  "@4", "new_local_name", "atomic_type", "opt_atomic_type", "basic_type",
  "arg_type", "optional_default_arg_value", "new_arg", "argument",
  "argument_list", "type_modifier_list", "type", "cast", "opt_basic_type",
  "name_list", "new_name", "block", "@5", "decl_block",
  "local_declarations", "$@6", "new_local_def", "single_new_local_def",
  "single_new_local_def_with_init", "local_name_list",
  "local_declaration_statement", "$@7", "block_statements", "statement",
  "while", "$@8", "do", "$@9", "for", "$@10", "foreach_var",
  "foreach_vars", "foreach", "$@11", "for_expr", "first_for_expr",
  "switch", "$@12", "switch_block", "case", "case_label", "constant",
  "comma_expr", "ref", "expr0", "return", "expr_list", "expr_list_node",
  "expr_list2", "expr_list3", "expr_list4", "assoc_pair", "lvalue",
  "l_new_function_open", "expr4", "@13", "@14", "expr_or_block", "catch",
  "@15", "tree", "sscanf", "parse_command", "time_expression", "@16",
  "lvalue_list", "string", "string_con1", "string_con2",
  "template_literal", "template_parts", "string_like", "class_init",
  "opt_class_init", "function_call", "@17", "@18", "@19", "@20", "@21",
  "@22", "@23", "efun_override", "function_name", "cond",
  "optional_else_part", YY_NULLPTR
  };
  return yy_sname[yysymbol];
}
#endif

#define YYPACT_NINF (-435)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-273)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -435,    36,   129,  -435,    51,  -435,  -435,    -3,  -435,  -435,
      53,    74,  -435,  -435,  -435,  -435,    12,   271,  -435,   112,
     123,  -435,  -435,  -435,   334,   109,   122,  -435,    12,   -26,
     206,   144,   158,   161,  -435,  -435,  -435,    15,  -435,    74,
     -17,    12,  -435,  -435,  -435,  1528,   133,   334,  -435,  -435,
    -435,  -435,   216,  -435,  -435,   211,     0,   117,   235,   287,
     287,  1528,   334,  1081,   574,  1528,  -435,   184,  -435,  -435,
     215,  -435,   238,  -435,    -2,  1528,  1528,  1528,   894,   252,
    -435,  -435,   259,  1528,   287,  1656,   241,   283,    43,  -435,
    -435,  -435,  -435,  -435,  -435,   206,  -435,    40,  -435,   262,
     288,   125,   228,    14,  1528,   334,   312,  -435,   204,  1135,
    -435,    16,  -435,  -435,  -435,  1492,   266,  -435,   169,  1594,
     275,   243,  -435,   294,  1656,   262,  1528,   155,  1528,   155,
     353,  1528,  -435,  -435,  1309,  -435,  -435,   152,   338,  1528,
      74,   172,  -435,   334,  -435,  -435,  1528,  1528,  1528,  1528,
    1528,  1528,  1528,  1528,  1528,  1528,  1528,  1528,  1528,  1528,
    1528,  1528,  1528,  1528,  -435,  -435,  1528,   217,  1528,   334,
     334,  1189,  -435,  -435,  -435,  -435,  -435,  -435,  -435,   357,
      74,  -435,   342,     5,  -435,  -435,  1656,  -435,   125,  1266,
    -435,  -435,  -435,   344,  1002,  1528,   349,   628,   355,  1528,
     457,  1528,  -435,  -435,   536,  -435,   369,  1266,   202,   682,
    1528,  -435,  -435,  -435,  -435,   210,   363,  -435,  1528,  -435,
    1721,  1691,  1667,   248,   248,   306,  1632,   332,   859,   200,
     365,   365,   306,   191,   191,  -435,  -435,  -435,  1656,  -435,
     304,   367,  -435,  1528,    21,  1266,  1266,  -435,   379,  -435,
    -435,   165,    74,   364,   370,  -435,  -435,  1656,  -435,  -435,
    -435,  1656,  1528,   213,  1528,  -435,  -435,   371,  -435,    29,
     373,   374,   376,  -435,   377,   380,   382,   390,  1343,  -435,
    -435,  -435,  -435,   682,   399,   682,  -435,  -435,  -435,  -435,
    -435,   167,  -435,  -435,  1309,   405,  -435,   218,  1528,   406,
    1528,  -435,   119,   416,   409,   410,   411,   404,    10,  -435,
     334,   412,   414,   425,  -435,  1043,  -435,  1569,   225,   229,
    -435,   682,  1528,  1528,  1528,   840,   948,   176,  -435,  -435,
    -435,   168,    74,  -435,  -435,  -435,  -435,  -435,  -435,  -435,
    1656,  -435,  1266,   495,  -435,  1528,  -435,   -31,  -435,  -435,
    -435,   471,  -435,  -435,  -435,  -435,  -435,  -435,    74,  -435,
    -435,   287,   440,  1528,  -435,   334,  -435,  -435,   233,   236,
     246,   500,    74,   516,  -435,  -435,   458,   455,  -435,  -435,
    -435,   456,   509,  -435,   379,   469,   484,  1266,   483,  1528,
    -435,    -9,    62,  -435,  1266,  1528,  -435,   482,  -435,  1043,
     486,  -435,   840,  -435,  -435,   490,   379,  1528,  1397,   176,
    1528,   546,    74,  -435,   491,  -435,    78,  -435,  -435,   493,
     314,  -435,   496,  1528,   542,   492,   840,  1528,  -435,  1656,
     499,  -435,  1230,  1528,  -435,  -435,  -435,  -435,   498,  -435,
    1656,   840,  -435,  -435,  -435,   257,  1451,  -435,  1656,  -435,
    -435,   199,   501,   507,   840,   160,   512,  -435,   761,  -435,
    -435,  -435,  -435,   584,   163,   585,   586,   163,    52,  1745,
     525,  -435,    74,   761,   514,   761,   840,  -435,   518,  -435,
    -435,   757,    73,  -435,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   535,  -435,
    -435,  -435,  -435,  -435,  -435,  -435,   545,   121,   442,   442,
     424,   345,   913,   967,   591,   591,   424,   203,   203,  -435,
    -435,  -435,  -435,  -435
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       4,     0,    55,     1,    55,    22,    20,     5,    24,    23,
      60,    10,    56,     6,     3,    36,     0,     0,    25,    41,
       0,    59,    57,    11,     0,     0,    61,   242,     0,     0,
     239,    37,    38,     0,    42,    15,    16,    63,    21,    10,
       0,     0,     7,   243,    32,     0,     0,     0,    62,   240,
     241,    29,     0,     9,     8,   219,   200,   201,     0,     0,
       0,     0,     0,     0,     0,     0,   197,     0,   228,   145,
       0,   234,     0,   202,     0,     0,     0,     0,     0,     0,
     180,   179,     0,     0,     0,    64,     0,     0,   175,   217,
     218,   176,   177,   178,   247,   238,   248,   215,   199,     0,
       0,    40,    63,    40,     0,     0,     0,   260,     0,     0,
     168,   196,   169,   170,   273,   186,     0,   188,   184,     0,
       0,   191,   193,     0,   143,   198,     0,     0,     0,     0,
     256,     0,    65,   230,     0,   172,   171,    36,     0,     0,
      10,     0,   203,     0,   167,   146,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   173,   174,     0,     0,     0,     0,
       0,     0,   249,   250,   254,   262,    35,    34,    49,    43,
      10,    53,     0,    51,    33,    30,   148,   274,    40,     0,
     271,   270,   187,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   226,   229,     0,   235,     0,     0,     0,     0,
       0,   245,   244,    37,    38,     0,     0,   216,     0,   275,
     152,   151,   150,   160,   161,   158,     0,   153,   154,   155,
     156,   157,   159,   162,   163,   164,   165,   166,   147,   221,
       0,   205,   206,     0,     0,     0,     0,    44,    47,    18,
      52,    40,    10,     0,     0,   225,   189,   195,   224,   194,
     223,   144,     0,     0,     0,   252,   252,     0,   231,     0,
       0,     0,     0,    97,     0,     0,     0,     0,     0,    92,
      79,    67,    91,     0,     0,     0,    87,    88,    68,    69,
      89,     0,    90,    86,     0,     0,    58,     0,     0,     0,
       0,   266,     0,     0,   214,     0,     0,    45,     0,    54,
       0,    27,     0,     0,   261,   236,   227,     0,     0,     0,
     257,     0,     0,     0,     0,     0,     0,    40,    93,    94,
     181,     0,    10,    83,    66,    82,    85,   246,   268,   204,
     149,   222,     0,     0,   213,     0,   211,     0,   264,   255,
     263,     0,    48,    14,    13,    19,    12,    26,    10,    31,
     220,     0,     0,     0,   258,     0,   259,    84,     0,     0,
       0,     0,    10,     0,   111,   110,     0,   109,   101,   103,
     102,   104,     0,   182,     0,    77,     0,     0,     0,     0,
     212,     0,     0,   207,     0,     0,    28,   236,   232,   236,
       0,   253,     0,   112,    95,     0,     0,     0,     0,    40,
       0,    73,    10,    80,     0,   267,     0,   208,   210,     0,
       0,   237,     0,     0,   277,     0,     0,     0,    75,    76,
       0,   105,     0,     0,    78,   269,   209,   265,     0,   233,
     251,     0,   276,    70,    96,     0,     0,   106,    74,    46,
     278,    40,     0,     0,     0,     0,     0,    71,     0,    98,
      99,   107,   139,     0,     0,     0,     0,     0,     0,   122,
     123,   121,    10,     0,     0,     0,     0,   141,     0,   140,
     142,     0,     0,   117,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   115,
     113,   114,   100,   120,   138,   119,     0,     0,   131,   132,
     129,   124,   125,   126,   127,   128,   130,   133,   134,   135,
     136,   137,    72,   118
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -435,  -435,  -435,  -435,  -435,  -435,  -435,    -1,  -435,    27,
    -435,  -435,  -435,  -435,  -435,  -435,   253,  -435,  -435,  -435,
    -435,  -234,  -435,  -435,    -8,  -309,  -435,   384,   436,  -435,
     626,  -435,  -435,  -435,   603,  -435,   -66,  -435,  -435,  -435,
    -435,  -435,   317,  -435,  -389,  -435,  -435,  -134,  -265,  -435,
    -435,  -435,  -435,  -435,  -435,   237,  -435,  -435,  -435,  -230,
    -435,  -435,  -435,  -139,   193,  -434,  1332,   -41,   470,   -42,
    -435,  -180,  -178,   494,  -435,  -435,   453,   -55,  -435,   -53,
    -435,  -435,   522,  -435,  -435,  -435,  -435,  -435,  -435,  -435,
     -44,   555,   -15,   -16,   557,   372,  -435,  -435,   397,  -435,
    -435,  -435,  -435,  -435,  -435,  -435,  -435,   599,  -435,  -435,
    -435
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,     2,    14,     5,    80,    81,   384,   355,    82,
       6,    46,   308,     7,     8,   311,   312,   103,   252,     9,
      51,   178,    19,    20,   179,   180,   352,   181,   182,   183,
      10,    11,    83,    22,    25,    26,   281,   209,   282,   451,
     472,   385,   380,   374,   386,   283,   332,   284,   285,   286,
     426,   287,   325,   288,   476,   381,   382,   289,   454,   375,
     376,   290,   425,   474,   475,   468,   469,   291,    84,   124,
     292,   116,   117,   118,   120,   121,   122,    86,    87,    88,
     218,   106,   203,    89,   127,    90,    91,    92,    93,   129,
     362,    94,   470,    95,    96,   212,    97,   401,   318,    98,
     245,   207,   189,   246,   394,   342,   387,    99,   100,   293,
     442
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      30,    29,    21,    85,   110,   112,   111,   111,   133,   254,
      24,   353,    30,    40,   307,    27,   256,   372,   372,   113,
      15,   115,   119,   434,   123,    30,    50,   267,    45,   145,
     478,   111,   250,   134,   135,   136,     3,   141,    47,   -15,
      41,   144,   104,    27,    33,   199,    42,   303,   506,    41,
     393,    37,   169,   170,  -196,  -196,  -196,    49,     4,    15,
     371,   202,   186,   202,   138,   305,   306,   199,   141,    13,
     140,   131,   417,   107,   102,   132,    27,   462,   482,   169,
     170,   251,   354,   498,   200,    28,   204,   132,   -17,   114,
     208,    16,   184,   463,    75,   185,   171,   199,   215,   -40,
     372,   321,   304,    17,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   256,   171,   238,   462,   115,   483,    18,    -2,
     244,    15,   187,   176,   177,   191,     4,   424,   199,   216,
     465,   463,    23,   418,   466,   343,   467,   115,   505,   333,
     411,   335,   115,   257,   199,   119,   -16,   261,   -39,   436,
     263,   444,   388,    27,   462,   115,    27,   462,   294,    34,
     219,    15,   428,   176,   177,   138,   450,   297,   430,   248,
     463,    38,    15,   463,   378,   379,   464,   367,   465,   461,
    -272,   105,   466,   473,   507,   199,   241,   242,    39,   -50,
     344,   280,   302,   115,   115,    15,   101,   414,   473,    43,
     473,   502,    35,    36,   419,   138,   453,   149,   150,   151,
     315,   -15,   317,   455,   456,  -219,   138,   465,   201,   104,
     465,   466,   132,   467,   466,   -16,   467,   331,    44,   336,
     383,    45,   356,   199,   199,   194,   217,   360,   199,   138,
     105,   310,   164,   165,   166,   190,   340,   126,   115,   161,
     162,   163,   347,   156,   157,   158,   159,   160,   161,   162,
     163,   495,   496,   497,   108,   280,   268,   280,   199,    31,
      32,   368,   369,   370,   295,   377,   199,   316,   128,   199,
      27,   239,   339,    55,   199,    56,    57,    58,   143,   364,
     115,   365,   391,   366,   392,   365,   397,   402,   111,   199,
     403,   130,   199,   280,   159,   160,   161,   162,   163,   197,
     404,   399,   199,   149,   150,   142,    62,    63,    64,    65,
      66,   452,    68,   199,   499,   174,   501,   357,    72,    73,
      74,    75,    35,    36,   193,   115,   213,   214,   416,   149,
     150,   151,   115,   421,   420,   422,   196,   310,   167,   168,
     109,   175,   484,   485,   486,   429,    79,   377,   432,   198,
     199,   406,   159,   160,   161,   162,   163,   265,   266,   299,
     300,   440,   149,   150,   151,   188,   445,   176,   177,   438,
     199,   448,   400,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   206,    69,   377,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   249,    52,   255,    27,
      53,    54,    55,   258,    56,    57,    58,    59,    60,   260,
     158,   159,   160,   161,   162,   163,    61,   296,   313,    30,
     301,   484,   485,   457,   314,   320,   322,   323,    30,   324,
     326,    30,    40,   327,   328,    62,    63,    64,    65,    66,
      67,    68,   329,    69,    70,    71,    30,    72,    73,    74,
      75,   146,   147,   148,   149,   150,   151,   334,   338,   351,
     341,   345,   348,    76,   349,   350,   359,    77,   358,    78,
     493,   494,   495,   496,   497,    79,    52,   346,    27,    53,
      54,    55,   132,    56,    57,    58,    59,    60,   493,   494,
     495,   496,   497,   395,   398,    61,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   405,   407,
     408,   199,   409,   262,    62,    63,    64,    65,    66,    67,
      68,   410,    69,    70,    71,   412,    72,    73,    74,    75,
     146,   147,   148,   149,   150,   151,   413,   415,   361,   433,
     389,   423,    76,   427,   441,   435,    77,   437,    78,   443,
     439,   446,   449,   459,    79,    52,   390,    27,    53,    54,
      55,   460,    56,    57,    58,    59,    60,   471,   477,   479,
     480,    41,   500,   503,    61,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   522,   484,   485,
     486,   396,   264,    62,    63,    64,    65,    66,    67,    68,
     523,    69,    70,    71,   253,    72,    73,    74,    75,    52,
      12,    27,    53,    54,    55,   309,    56,    57,    58,    59,
      60,    76,    48,   373,   458,    77,   431,    78,    61,   247,
     259,   205,   172,    79,   173,  -190,   492,   493,   494,   495,
     496,   497,   240,   319,   125,     0,   337,    62,    63,    64,
      65,    66,    67,    68,     0,    69,    70,    71,     0,    72,
      73,    74,    75,   269,     0,    27,    53,    54,   137,     0,
      56,    57,    58,    59,    60,    76,     0,     0,     0,    77,
       0,    78,    61,   270,     0,   271,     0,    79,     0,  -192,
     272,   273,   274,   275,     0,   276,   277,   278,     0,     0,
       0,    62,    63,    64,    65,    66,    67,    68,   -40,    69,
      70,    71,   138,    72,    73,    74,    75,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    76,
       0,     0,     0,    77,   279,    78,     0,     0,     0,   132,
     -81,    79,    52,     0,    27,    53,    54,    55,     0,    56,
      57,    58,    59,    60,   484,   485,   486,     0,     0,     0,
       0,    61,   270,     0,   271,   455,   456,     0,     0,   272,
     273,   274,   275,     0,   276,   277,   278,     0,     0,     0,
      62,    63,    64,    65,    66,    67,    68,     0,    69,    70,
      71,     0,    72,    73,    74,    75,     0,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,    76,     0,
       0,   504,    77,   279,    78,     0,     0,     0,   132,  -116,
      79,    52,     0,    27,    53,    54,    55,     0,    56,    57,
      58,    59,    60,     0,     0,     0,     0,     0,     0,     0,
      61,   270,     0,   271,     0,     0,     0,     0,   272,   273,
     274,   275,     0,   276,   277,   278,   149,   150,   151,    62,
      63,    64,    65,    66,    67,    68,     0,    69,    70,    71,
       0,    72,    73,    74,    75,    52,     0,    27,    53,    54,
     137,     0,    56,    57,    58,    59,    60,    76,     0,     0,
       0,    77,   279,    78,    61,     0,     0,   132,     0,    79,
       0,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     484,   485,   486,    62,    63,    64,    65,    66,    67,    68,
     -40,    69,    70,    71,   138,    72,    73,    74,    75,    52,
       0,    27,    53,    54,   137,     0,    56,    57,    58,    59,
      60,    76,   139,     0,     0,    77,     0,    78,    61,     0,
       0,     0,     0,    79,     0,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   484,   485,   486,    62,    63,    64,
      65,    66,    67,    68,   -40,    69,    70,    71,   138,    72,
      73,    74,    75,    52,     0,    27,    53,    54,    55,     0,
      56,    57,    58,    59,    60,    76,     0,     0,     0,    77,
    -108,    78,    61,     0,     0,     0,     0,    79,     0,     0,
     490,   491,   492,   493,   494,   495,   496,   497,     0,     0,
       0,    62,    63,    64,    65,    66,    67,    68,     0,    69,
      70,    71,     0,    72,    73,    74,    75,   146,   147,   148,
     149,   150,   151,     0,     0,     0,     0,     0,     0,    76,
       0,     0,     0,    77,     0,    78,  -185,     0,     0,     0,
    -185,    79,    52,     0,    27,    53,    54,    55,     0,    56,
      57,    58,    59,    60,     0,     0,     0,     0,     0,     0,
       0,    61,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,     0,     0,     0,     0,     0,   361,
      62,    63,    64,    65,    66,    67,    68,     0,    69,    70,
      71,     0,    72,    73,    74,    75,    52,     0,    27,    53,
      54,    55,     0,    56,    57,    58,    59,    60,    76,     0,
       0,     0,    77,     0,    78,    61,     0,     0,     0,  -183,
      79,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,    65,    66,    67,
      68,     0,    69,    70,    71,     0,    72,    73,    74,    75,
      52,     0,    27,    53,    54,    55,     0,    56,    57,    58,
      59,    60,    76,   139,     0,     0,    77,     0,    78,    61,
       0,     0,     0,     0,    79,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,    65,    66,    67,    68,     0,    69,    70,    71,     0,
      72,    73,    74,    75,   146,   147,   148,   149,   150,   151,
       0,     0,     0,     0,   243,     0,    76,     0,     0,     0,
      77,     0,    78,     0,     0,     0,     0,    52,    79,    27,
      53,    54,    55,     0,    56,    57,    58,    59,    60,     0,
       0,     0,     0,     0,     0,     0,    61,     0,     0,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     163,     0,     0,     0,   447,    62,    63,    64,    65,    66,
      67,    68,     0,    69,    70,    71,     0,    72,    73,    74,
      75,     0,     0,   146,   147,   148,   149,   150,   151,     0,
       0,     0,     0,    76,     0,     0,     0,    77,     0,    78,
    -183,     0,     0,     0,    52,    79,    27,    53,    54,    55,
       0,    56,    57,    58,    59,    60,     0,     0,     0,     0,
       0,     0,     0,    61,   210,   211,     0,     0,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
       0,     0,    62,    63,    64,    65,    66,    67,    68,     0,
      69,    70,    71,     0,    72,    73,    74,    75,    52,     0,
      27,    53,    54,    55,     0,    56,    57,    58,    59,    60,
      76,     0,     0,     0,    77,   330,    78,    61,     0,     0,
       0,     0,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,    65,
      66,    67,    68,     0,    69,    70,    71,     0,    72,    73,
      74,    75,    52,     0,    27,    53,    54,    55,     0,    56,
      57,    58,    59,    60,    76,     0,     0,     0,    77,  -108,
      78,    61,     0,     0,     0,     0,    79,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,    65,    66,    67,    68,     0,    69,    70,
      71,     0,    72,    73,    74,    75,   146,   147,   148,   149,
     150,   151,     0,     0,     0,     0,     0,     0,    76,   192,
       0,     0,    77,     0,    78,  -108,     0,     0,     0,    52,
      79,    27,    53,    54,    55,     0,    56,    57,    58,    59,
      60,     0,     0,     0,     0,     0,     0,     0,    61,     0,
       0,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,     0,     0,     0,     0,    62,    63,    64,
      65,    66,    67,    68,     0,    69,    70,    71,     0,    72,
      73,    74,    75,   146,   147,   148,   149,   150,   151,     0,
       0,     0,     0,     0,     0,    76,     0,     0,     0,    77,
       0,    78,     0,     0,     0,     0,     0,    79,   146,   147,
     148,   149,   150,   151,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
       0,     0,     0,     0,     0,   363,   146,   147,   148,   149,
     150,   151,     0,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,     0,     0,     0,     0,   195,
     146,   147,   148,   149,   150,   151,     0,     0,     0,     0,
       0,   146,   147,     0,   149,   150,   151,     0,     0,     0,
       0,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,     0,     0,   146,     0,   298,   149,   150,
     151,     0,     0,     0,     0,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   149,   150,
     151,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   484,   485,   486,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,     0,     0,     0,     0,     0,     0,     0,   481,
       0,     0,     0,     0,     0,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   508,   509,   510,   511,
     512,   513,   514,   515,   516,   517,   518,   519,   520,   521,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   481
};

static const yytype_int16 yycheck[] =
{
      16,    16,    10,    45,    59,    60,    59,    60,    74,   189,
      11,     1,    28,    28,   248,     3,   194,   326,   327,    61,
       6,    63,    64,   412,    65,    41,    41,   207,    13,    84,
     464,    84,    27,    75,    76,    77,     0,    78,    39,    39,
      66,    83,    13,     3,    17,    76,    72,    26,   482,    66,
      81,    24,    36,    37,    11,    12,    13,    74,     7,     6,
     325,   127,   104,   129,    50,   245,   246,    76,   109,    72,
      78,    73,    81,    73,    47,    77,     3,     4,    26,    36,
      37,    76,    72,   472,   126,    73,   128,    77,    73,    62,
     131,    38,    78,    20,    54,   103,    80,    76,   139,    46,
     409,    72,    81,    50,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   300,    80,   166,     4,   168,    75,    75,     0,
     171,     6,   105,     8,     9,   108,     7,   402,    76,   140,
      67,    20,    68,    81,    71,    26,    73,   189,    75,   283,
     384,   285,   194,   195,    76,   197,    39,   199,    46,    81,
     201,   426,   342,     3,     4,   207,     3,     4,   210,    46,
     143,     6,   406,     8,     9,    50,   441,   218,   408,   180,
      20,    72,     6,    20,     8,     9,    26,   321,    67,   454,
      73,    39,    71,   458,    73,    76,   169,   170,    76,    74,
      81,   209,   243,   245,   246,     6,    73,   387,   473,     3,
     475,   476,     8,     9,   394,    50,   446,    17,    18,    19,
     262,    77,   264,    24,    25,    73,    50,    67,    73,    13,
      67,    71,    77,    73,    71,    77,    73,   278,    77,    72,
      72,    13,   308,    76,    76,    76,    74,   313,    76,    50,
      39,   252,    11,    12,    13,    51,   298,    73,   300,    68,
      69,    70,   303,    63,    64,    65,    66,    67,    68,    69,
      70,    68,    69,    70,    39,   283,    74,   285,    76,     8,
       9,   322,   323,   324,    74,   326,    76,    74,    73,    76,
       3,    74,    74,     6,    76,     8,     9,    10,    39,    74,
     342,    76,   343,    74,   345,    76,   361,    74,   361,    76,
      74,    73,    76,   321,    66,    67,    68,    69,    70,    76,
      74,   363,    76,    17,    18,    73,    39,    40,    41,    42,
      43,    74,    45,    76,   473,    73,   475,   310,    51,    52,
      53,    54,     8,     9,    78,   387,     8,     9,   389,    17,
      18,    19,   394,   397,   395,   399,    81,   358,    75,    76,
      73,    73,    17,    18,    19,   407,    79,   408,   410,    75,
      76,   372,    66,    67,    68,    69,    70,     8,     9,    75,
      76,   423,    17,    18,    19,    73,   427,     8,     9,    75,
      76,   433,   365,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    50,    47,   446,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    74,     1,    74,     3,
       4,     5,     6,    74,     8,     9,    10,    11,    12,    74,
      65,    66,    67,    68,    69,    70,    20,    74,    74,   455,
      73,    17,    18,   451,    74,    74,    73,    73,   464,    73,
      73,   467,   467,    73,    72,    39,    40,    41,    42,    43,
      44,    45,    72,    47,    48,    49,   482,    51,    52,    53,
      54,    14,    15,    16,    17,    18,    19,    78,    73,    75,
      74,    65,    73,    67,    74,    74,    72,    71,    76,    73,
      66,    67,    68,    69,    70,    79,     1,    81,     3,     4,
       5,     6,    77,     8,     9,    10,    11,    12,    66,    67,
      68,    69,    70,    42,    74,    20,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    28,    13,
      72,    76,    76,    76,    39,    40,    41,    42,    43,    44,
      45,    32,    47,    48,    49,    76,    51,    52,    53,    54,
      14,    15,    16,    17,    18,    19,    72,    74,    76,    13,
      65,    75,    67,    73,    22,    74,    71,    74,    73,    77,
      74,    72,    74,    72,    79,     1,    81,     3,     4,     5,
       6,    74,     8,     9,    10,    11,    12,    75,     4,     4,
       4,    66,    78,    75,    20,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    72,    17,    18,
      19,   358,    76,    39,    40,    41,    42,    43,    44,    45,
      75,    47,    48,    49,   188,    51,    52,    53,    54,     1,
       4,     3,     4,     5,     6,   251,     8,     9,    10,    11,
      12,    67,    39,   326,   451,    71,   409,    73,    20,   179,
     197,   129,    97,    79,    97,    81,    65,    66,    67,    68,
      69,    70,   168,   266,    65,    -1,   294,    39,    40,    41,
      42,    43,    44,    45,    -1,    47,    48,    49,    -1,    51,
      52,    53,    54,     1,    -1,     3,     4,     5,     6,    -1,
       8,     9,    10,    11,    12,    67,    -1,    -1,    -1,    71,
      -1,    73,    20,    21,    -1,    23,    -1,    79,    -1,    81,
      28,    29,    30,    31,    -1,    33,    34,    35,    -1,    -1,
      -1,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    71,    72,    73,    -1,    -1,    -1,    77,
      78,    79,     1,    -1,     3,     4,     5,     6,    -1,     8,
       9,    10,    11,    12,    17,    18,    19,    -1,    -1,    -1,
      -1,    20,    21,    -1,    23,    24,    25,    -1,    -1,    28,
      29,    30,    31,    -1,    33,    34,    35,    -1,    -1,    -1,
      39,    40,    41,    42,    43,    44,    45,    -1,    47,    48,
      49,    -1,    51,    52,    53,    54,    -1,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    67,    -1,
      -1,    74,    71,    72,    73,    -1,    -1,    -1,    77,    78,
      79,     1,    -1,     3,     4,     5,     6,    -1,     8,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      20,    21,    -1,    23,    -1,    -1,    -1,    -1,    28,    29,
      30,    31,    -1,    33,    34,    35,    17,    18,    19,    39,
      40,    41,    42,    43,    44,    45,    -1,    47,    48,    49,
      -1,    51,    52,    53,    54,     1,    -1,     3,     4,     5,
       6,    -1,     8,     9,    10,    11,    12,    67,    -1,    -1,
      -1,    71,    72,    73,    20,    -1,    -1,    77,    -1,    79,
      -1,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      17,    18,    19,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,     1,
      -1,     3,     4,     5,     6,    -1,     8,     9,    10,    11,
      12,    67,    68,    -1,    -1,    71,    -1,    73,    20,    -1,
      -1,    -1,    -1,    79,    -1,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    17,    18,    19,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,     1,    -1,     3,     4,     5,     6,    -1,
       8,     9,    10,    11,    12,    67,    -1,    -1,    -1,    71,
      72,    73,    20,    -1,    -1,    -1,    -1,    79,    -1,    -1,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    39,    40,    41,    42,    43,    44,    45,    -1,    47,
      48,    49,    -1,    51,    52,    53,    54,    14,    15,    16,
      17,    18,    19,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    71,    -1,    73,    74,    -1,    -1,    -1,
      78,    79,     1,    -1,     3,     4,     5,     6,    -1,     8,
       9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    20,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    -1,    -1,    76,
      39,    40,    41,    42,    43,    44,    45,    -1,    47,    48,
      49,    -1,    51,    52,    53,    54,     1,    -1,     3,     4,
       5,     6,    -1,     8,     9,    10,    11,    12,    67,    -1,
      -1,    -1,    71,    -1,    73,    20,    -1,    -1,    -1,    78,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    39,    40,    41,    42,    43,    44,
      45,    -1,    47,    48,    49,    -1,    51,    52,    53,    54,
       1,    -1,     3,     4,     5,     6,    -1,     8,     9,    10,
      11,    12,    67,    68,    -1,    -1,    71,    -1,    73,    20,
      -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,    40,
      41,    42,    43,    44,    45,    -1,    47,    48,    49,    -1,
      51,    52,    53,    54,    14,    15,    16,    17,    18,    19,
      -1,    -1,    -1,    -1,    65,    -1,    67,    -1,    -1,    -1,
      71,    -1,    73,    -1,    -1,    -1,    -1,     1,    79,     3,
       4,     5,     6,    -1,     8,     9,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    20,    -1,    -1,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    39,    40,    41,    42,    43,
      44,    45,    -1,    47,    48,    49,    -1,    51,    52,    53,
      54,    -1,    -1,    14,    15,    16,    17,    18,    19,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    71,    -1,    73,
      74,    -1,    -1,    -1,     1,    79,     3,     4,     5,     6,
      -1,     8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    20,    55,    56,    -1,    -1,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    -1,    39,    40,    41,    42,    43,    44,    45,    -1,
      47,    48,    49,    -1,    51,    52,    53,    54,     1,    -1,
       3,     4,     5,     6,    -1,     8,     9,    10,    11,    12,
      67,    -1,    -1,    -1,    71,    72,    73,    20,    -1,    -1,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    39,    40,    41,    42,
      43,    44,    45,    -1,    47,    48,    49,    -1,    51,    52,
      53,    54,     1,    -1,     3,     4,     5,     6,    -1,     8,
       9,    10,    11,    12,    67,    -1,    -1,    -1,    71,    72,
      73,    20,    -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      39,    40,    41,    42,    43,    44,    45,    -1,    47,    48,
      49,    -1,    51,    52,    53,    54,    14,    15,    16,    17,
      18,    19,    -1,    -1,    -1,    -1,    -1,    -1,    67,    27,
      -1,    -1,    71,    -1,    73,    74,    -1,    -1,    -1,     1,
      79,     3,     4,     5,     6,    -1,     8,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    -1,
      -1,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    -1,    39,    40,    41,
      42,    43,    44,    45,    -1,    47,    48,    49,    -1,    51,
      52,    53,    54,    14,    15,    16,    17,    18,    19,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    71,
      -1,    73,    -1,    -1,    -1,    -1,    -1,    79,    14,    15,
      16,    17,    18,    19,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    -1,    -1,    76,    14,    15,    16,    17,
      18,    19,    -1,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    -1,    75,
      14,    15,    16,    17,    18,    19,    -1,    -1,    -1,    -1,
      -1,    14,    15,    -1,    17,    18,    19,    -1,    -1,    -1,
      -1,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    -1,    14,    -1,    75,    17,    18,
      19,    -1,    -1,    -1,    -1,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    17,    18,
      19,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    17,    18,    19,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   467,
      -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   507
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    83,    84,     0,     7,    86,    92,    95,    96,   101,
     112,   113,   112,    72,    85,     6,    38,    50,    75,   104,
     105,   106,   115,    68,    89,   116,   117,     3,    73,   174,
     175,     8,     9,    91,    46,     8,     9,    91,    72,    76,
     174,    66,    72,     3,    77,    13,    93,    89,   116,    74,
     174,   102,     1,     4,     5,     6,     8,     9,    10,    11,
      12,    20,    39,    40,    41,    42,    43,    44,    45,    47,
      48,    49,    51,    52,    53,    54,    67,    71,    73,    79,
      87,    88,    91,   114,   150,   151,   159,   160,   161,   165,
     167,   168,   169,   170,   173,   175,   176,   178,   181,   189,
     190,    73,    91,    99,    13,    39,   163,    73,    39,    73,
     159,   161,   159,   151,    91,   151,   153,   154,   155,   151,
     156,   157,   158,   149,   151,   189,    73,   166,    73,   171,
      73,    73,    77,   118,   151,   151,   151,     6,    50,    68,
     106,   149,    73,    39,   151,   159,    14,    15,    16,    17,
      18,    19,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    11,    12,    13,    75,    76,    36,
      37,    80,   173,   176,    73,    73,     8,     9,   103,   106,
     107,   109,   110,   111,    78,   106,   151,    91,    73,   184,
      51,    91,    27,    78,    76,    75,    81,    76,    75,    76,
     151,    73,   118,   164,   151,   164,    50,   183,   149,   119,
      55,    56,   177,     8,     9,   149,    89,    74,   162,    91,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,    74,
     155,    91,    91,    65,   149,   182,   185,   150,    89,    74,
      27,    76,   100,   110,   153,    74,   154,   151,    74,   158,
      74,   151,    76,   149,    76,     8,     9,   153,    74,     1,
      21,    23,    28,    29,    30,    31,    33,    34,    35,    72,
     106,   118,   120,   127,   129,   130,   131,   133,   135,   139,
     143,   149,   152,   191,   151,    74,    74,   149,    75,    75,
      76,    73,   149,    26,    81,   153,   153,   103,    94,   109,
      89,    97,    98,    74,    74,   151,    74,   151,   180,   180,
      74,    72,    73,    73,    73,   134,    73,    73,    72,    72,
      72,   149,   128,   129,    78,   129,    72,   177,    73,    74,
     151,    74,   187,    26,    81,    65,    81,   149,    73,    74,
      74,    75,   108,     1,    72,    90,   118,    91,    76,    72,
     118,    76,   172,    76,    74,    76,    74,   129,   149,   149,
     149,   130,   107,   124,   125,   141,   142,   149,     8,     9,
     124,   137,   138,    72,    89,   123,   126,   188,   153,    65,
      81,   149,   149,    81,   186,    42,    98,   159,    74,   151,
      91,   179,    74,    74,    74,    28,    89,    13,    72,    76,
      32,   103,    76,    72,   153,    74,   149,    81,    81,   153,
     149,   172,   172,    75,   130,   144,   132,    73,   103,   151,
     141,   137,   151,    13,   126,    74,    81,    74,    75,    74,
     151,    22,   192,    77,   130,   149,    72,    74,   151,    74,
     130,   121,    74,   141,   140,    24,    25,   106,   146,    72,
      74,   130,     4,    20,    26,    67,    71,    73,   147,   148,
     174,    75,   122,   130,   145,   146,   136,     4,   147,     4,
       4,   148,    26,    75,    17,    18,    19,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,   126,   145,
      78,   145,   130,    75,    74,    75,   147,    73,   148,   148,
     148,   148,   148,   148,   148,   148,   148,   148,   148,   148,
     148,   148,    72,    75
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    82,    83,    84,    84,    85,    85,    86,    87,    88,
      89,    89,    90,    90,    90,    91,    91,    93,    94,    92,
      95,    95,    95,    95,    95,    96,    97,    98,    98,    99,
     100,    99,   102,   101,   103,   103,   104,   104,   104,   105,
     105,   106,   106,   107,   107,   108,   108,   109,   109,   109,
     110,   110,   110,   111,   111,   112,   112,   113,   114,   115,
     115,   116,   116,   117,   117,   119,   118,   120,   120,   120,
     121,   122,   121,   123,   123,   124,   125,   126,   126,   128,
     127,   129,   129,   129,   129,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   132,   131,   134,   133,   136,
     135,   137,   137,   137,   138,   138,   140,   139,   141,   141,
     142,   142,   144,   143,   145,   145,   145,   146,   146,   146,
     146,   146,   147,   147,   148,   148,   148,   148,   148,   148,
     148,   148,   148,   148,   148,   148,   148,   148,   148,   148,
     148,   148,   148,   149,   149,   150,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   151,
     151,   152,   152,   153,   153,   153,   154,   154,   155,   155,
     156,   156,   156,   157,   157,   158,   159,   160,   160,   161,
     161,   161,   161,   162,   161,   161,   161,   161,   161,   161,
     161,   161,   161,   161,   161,   161,   161,   161,   161,   163,
     161,   161,   161,   161,   161,   161,   164,   164,   166,   165,
     167,   167,   168,   169,   171,   170,   172,   172,   173,   174,
     174,   174,   175,   175,   176,   177,   177,   178,   178,   178,
     178,   179,   180,   180,   182,   181,   183,   181,   181,   181,
     184,   181,   185,   181,   186,   181,   187,   181,   188,   181,
     189,   189,   190,   190,   190,   190,   191,   192,   192
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     3,     0,     0,     1,     4,     1,     1,
       0,     1,     1,     1,     1,     1,     1,     0,     0,     9,
       1,     3,     1,     1,     1,     2,     2,     1,     3,     0,
       0,     5,     0,     7,     1,     1,     1,     2,     2,     1,
       0,     1,     2,     1,     2,     0,     5,     2,     4,     1,
       0,     1,     2,     1,     3,     0,     2,     2,     4,     1,
       0,     1,     3,     2,     4,     0,     4,     1,     1,     1,
       0,     0,     5,     2,     4,     3,     3,     1,     3,     0,
       4,     0,     2,     2,     3,     2,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     0,     6,     0,     8,     0,
      10,     1,     1,     1,     1,     3,     0,     8,     0,     1,
       1,     1,     0,    10,     2,     2,     0,     3,     5,     4,
       4,     2,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     1,
       2,     2,     2,     1,     3,     1,     2,     3,     3,     5,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     2,     2,     2,     2,     1,     1,     1,     1,     1,
       1,     2,     3,     0,     1,     2,     1,     2,     1,     3,
       0,     1,     2,     1,     3,     3,     1,     1,     2,     1,
       1,     1,     1,     0,     5,     3,     3,     6,     7,     8,
       7,     5,     6,     5,     4,     1,     3,     1,     1,     0,
       6,     3,     5,     4,     4,     4,     1,     3,     0,     3,
       2,     4,     7,     9,     0,     3,     0,     3,     1,     1,
       3,     3,     1,     2,     3,     1,     3,     1,     1,     2,
       2,     3,     0,     3,     0,     5,     0,     5,     6,     6,
       0,     5,     0,     5,     0,     8,     0,     7,     0,     8,
       3,     3,     1,     2,     3,     3,     6,     0,     2
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        YY_LAC_DISCARD ("YYBACKUP");                              \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Given a state stack such that *YYBOTTOM is its bottom, such that
   *YYTOP is either its top or is YYTOP_EMPTY to indicate an empty
   stack, and such that *YYCAPACITY is the maximum number of elements it
   can hold without a reallocation, make sure there is enough room to
   store YYADD more elements.  If not, allocate a new stack using
   YYSTACK_ALLOC, copy the existing elements, and adjust *YYBOTTOM,
   *YYTOP, and *YYCAPACITY to reflect the new capacity and memory
   location.  If *YYBOTTOM != YYBOTTOM_NO_FREE, then free the old stack
   using YYSTACK_FREE.  Return 0 if successful or if no reallocation is
   required.  Return YYENOMEM if memory is exhausted.  */
static int
yy_lac_stack_realloc (YYPTRDIFF_T *yycapacity, YYPTRDIFF_T yyadd,
#if YYDEBUG
                      char const *yydebug_prefix,
                      char const *yydebug_suffix,
#endif
                      yy_state_t **yybottom,
                      yy_state_t *yybottom_no_free,
                      yy_state_t **yytop, yy_state_t *yytop_empty)
{
  YYPTRDIFF_T yysize_old =
    *yytop == yytop_empty ? 0 : *yytop - *yybottom + 1;
  YYPTRDIFF_T yysize_new = yysize_old + yyadd;
  if (*yycapacity < yysize_new)
    {
      YYPTRDIFF_T yyalloc = 2 * yysize_new;
      yy_state_t *yybottom_new;
      /* Use YYMAXDEPTH for maximum stack size given that the stack
         should never need to grow larger than the main state stack
         needs to grow without LAC.  */
      if (YYMAXDEPTH < yysize_new)
        {
          YYDPRINTF ((stderr, "%smax size exceeded%s", yydebug_prefix,
                      yydebug_suffix));
          return YYENOMEM;
        }
      if (YYMAXDEPTH < yyalloc)
        yyalloc = YYMAXDEPTH;
      yybottom_new =
        YY_CAST (yy_state_t *,
                 YYSTACK_ALLOC (YY_CAST (YYSIZE_T,
                                         yyalloc * YYSIZEOF (*yybottom_new))));
      if (!yybottom_new)
        {
          YYDPRINTF ((stderr, "%srealloc failed%s", yydebug_prefix,
                      yydebug_suffix));
          return YYENOMEM;
        }
      if (*yytop != yytop_empty)
        {
          YYCOPY (yybottom_new, *yybottom, yysize_old);
          *yytop = yybottom_new + (yysize_old - 1);
        }
      if (*yybottom != yybottom_no_free)
        YYSTACK_FREE (*yybottom);
      *yybottom = yybottom_new;
      *yycapacity = yyalloc;
    }
  return 0;
}

/* Establish the initial context for the current lookahead if no initial
   context is currently established.

   We define a context as a snapshot of the parser stacks.  We define
   the initial context for a lookahead as the context in which the
   parser initially examines that lookahead in order to select a
   syntactic action.  Thus, if the lookahead eventually proves
   syntactically unacceptable (possibly in a later context reached via a
   series of reductions), the initial context can be used to determine
   the exact set of tokens that would be syntactically acceptable in the
   lookahead's place.  Moreover, it is the context after which any
   further semantic actions would be erroneous because they would be
   determined by a syntactically unacceptable token.

   YY_LAC_ESTABLISH should be invoked when a reduction is about to be
   performed in an inconsistent state (which, for the purposes of LAC,
   includes consistent states that don't know they're consistent because
   their default reductions have been disabled).  Iff there is a
   lookahead token, it should also be invoked before reporting a syntax
   error.  This latter case is for the sake of the debugging output.

   For parse.lac=full, the implementation of YY_LAC_ESTABLISH is as
   follows.  If no initial context is currently established for the
   current lookahead, then check if that lookahead can eventually be
   shifted if syntactic actions continue from the current context.
   Report a syntax error if it cannot.  */
#define YY_LAC_ESTABLISH                                                \
do {                                                                    \
  if (!yy_lac_established)                                              \
    {                                                                   \
      YYDPRINTF ((stderr,                                               \
                  "LAC: initial context established for %s\n",          \
                  yysymbol_name (yytoken)));                            \
      yy_lac_established = 1;                                           \
      switch (yy_lac (yyesa, &yyes, &yyes_capacity, yyssp, yytoken))    \
        {                                                               \
        case YYENOMEM:                                                  \
          YYNOMEM;                                                      \
        case 1:                                                         \
          goto yyerrlab;                                                \
        }                                                               \
    }                                                                   \
} while (0)

/* Discard any previous initial lookahead context because of Event,
   which may be a lookahead change or an invalidation of the currently
   established initial context for the current lookahead.

   The most common example of a lookahead change is a shift.  An example
   of both cases is syntax error recovery.  That is, a syntax error
   occurs when the lookahead is syntactically erroneous for the
   currently established initial context, so error recovery manipulates
   the parser stacks to try to find a new initial context in which the
   current lookahead is syntactically acceptable.  If it fails to find
   such a context, it discards the lookahead.  */
#if YYDEBUG
# define YY_LAC_DISCARD(Event)                                           \
do {                                                                     \
  if (yy_lac_established)                                                \
    {                                                                    \
      YYDPRINTF ((stderr, "LAC: initial context discarded due to "       \
                  Event "\n"));                                          \
      yy_lac_established = 0;                                            \
    }                                                                    \
} while (0)
#else
# define YY_LAC_DISCARD(Event) yy_lac_established = 0
#endif

/* Given the stack whose top is *YYSSP, return 0 iff YYTOKEN can
   eventually (after perhaps some reductions) be shifted, return 1 if
   not, or return YYENOMEM if memory is exhausted.  As preconditions and
   postconditions: *YYES_CAPACITY is the allocated size of the array to
   which *YYES points, and either *YYES = YYESA or *YYES points to an
   array allocated with YYSTACK_ALLOC.  yy_lac may overwrite the
   contents of either array, alter *YYES and *YYES_CAPACITY, and free
   any old *YYES other than YYESA.  */
static int
yy_lac (yy_state_t *yyesa, yy_state_t **yyes,
        YYPTRDIFF_T *yyes_capacity, yy_state_t *yyssp, yysymbol_kind_t yytoken)
{
  yy_state_t *yyes_prev = yyssp;
  yy_state_t *yyesp = yyes_prev;
  /* Reduce until we encounter a shift and thereby accept the token.  */
  YYDPRINTF ((stderr, "LAC: checking lookahead %s:", yysymbol_name (yytoken)));
  if (yytoken == YYSYMBOL_YYUNDEF)
    {
      YYDPRINTF ((stderr, " Always Err\n"));
      return 1;
    }
  while (1)
    {
      int yyrule = yypact[+*yyesp];
      if (yypact_value_is_default (yyrule)
          || (yyrule += yytoken) < 0 || YYLAST < yyrule
          || yycheck[yyrule] != yytoken)
        {
          /* Use the default action.  */
          yyrule = yydefact[+*yyesp];
          if (yyrule == 0)
            {
              YYDPRINTF ((stderr, " Err\n"));
              return 1;
            }
        }
      else
        {
          /* Use the action from yytable.  */
          yyrule = yytable[yyrule];
          if (yytable_value_is_error (yyrule))
            {
              YYDPRINTF ((stderr, " Err\n"));
              return 1;
            }
          if (0 < yyrule)
            {
              YYDPRINTF ((stderr, " S%d\n", yyrule));
              return 0;
            }
          yyrule = -yyrule;
        }
      /* By now we know we have to simulate a reduce.  */
      YYDPRINTF ((stderr, " R%d", yyrule - 1));
      {
        /* Pop the corresponding number of values from the stack.  */
        YYPTRDIFF_T yylen = yyr2[yyrule];
        /* First pop from the LAC stack as many tokens as possible.  */
        if (yyesp != yyes_prev)
          {
            YYPTRDIFF_T yysize = yyesp - *yyes + 1;
            if (yylen < yysize)
              {
                yyesp -= yylen;
                yylen = 0;
              }
            else
              {
                yyesp = yyes_prev;
                yylen -= yysize;
              }
          }
        /* Only afterwards look at the main stack.  */
        if (yylen)
          yyesp = yyes_prev -= yylen;
      }
      /* Push the resulting state of the reduction.  */
      {
        yy_state_fast_t yystate;
        {
          const int yylhs = yyr1[yyrule] - YYNTOKENS;
          const int yyi = yypgoto[yylhs] + *yyesp;
          yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyesp
                     ? yytable[yyi]
                     : yydefgoto[yylhs]);
        }
        if (yyesp == yyes_prev)
          {
            yyesp = *yyes;
            YY_IGNORE_USELESS_CAST_BEGIN
            *yyesp = YY_CAST (yy_state_t, yystate);
            YY_IGNORE_USELESS_CAST_END
          }
        else
          {
            if (yy_lac_stack_realloc (yyes_capacity, 1,
#if YYDEBUG
                                      " (", ")",
#endif
                                      yyes, yyesa, &yyesp, yyes_prev))
              {
                YYDPRINTF ((stderr, "\n"));
                return YYENOMEM;
              }
            YY_IGNORE_USELESS_CAST_BEGIN
            *++yyesp = YY_CAST (yy_state_t, yystate);
            YY_IGNORE_USELESS_CAST_END
          }
        YYDPRINTF ((stderr, " G%d", yystate));
      }
    }
}

/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yy_state_t *yyesa;
  yy_state_t **yyes;
  YYPTRDIFF_T *yyes_capacity;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;

  int yyx;
  for (yyx = 0; yyx < YYNTOKENS; ++yyx)
    {
      yysymbol_kind_t yysym = YY_CAST (yysymbol_kind_t, yyx);
      if (yysym != YYSYMBOL_YYerror && yysym != YYSYMBOL_YYUNDEF)
        switch (yy_lac (yyctx->yyesa, yyctx->yyes, yyctx->yyes_capacity, yyctx->yyssp, yysym))
          {
          case YYENOMEM:
            return YYENOMEM;
          case 1:
            continue;
          default:
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = yysym;
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif



static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
       In the first two cases, it might appear that the current syntax
       error should have been detected in the previous state when yy_lac
       was invoked.  However, at that time, there might have been a
       different syntax error that discarded a different initial context
       during error recovery, leaving behind the current lookahead.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      YYDPRINTF ((stderr, "Constructing syntax error message\n"));
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else if (yyn == 0)
        YYDPRINTF ((stderr, "No expected tokens.\n"));
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.  In order to see if a particular token T is a
   valid looakhead, invoke yy_lac (YYESA, YYES, YYES_CAPACITY, YYSSP, T).

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store or if
   yy_lac returned YYENOMEM.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yystrlen (yysymbol_name (yyarg[yyi]));
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp = yystpcpy (yyp, yysymbol_name (yyarg[yyi++]));
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    yy_state_t yyesa[20];
    yy_state_t *yyes = yyesa;
    YYPTRDIFF_T yyes_capacity = 20 < YYMAXDEPTH ? 20 : YYMAXDEPTH;

  /* Whether LAC context is established.  A Boolean.  */
  int yy_lac_established = 0;
  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    {
      YY_LAC_ESTABLISH;
      goto yydefault;
    }
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      YY_LAC_ESTABLISH;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  YY_LAC_DISCARD ("shift");
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  {
    int yychar_backup = yychar;
    switch (yyn)
      {
  case 2: /* all: program  */
#line 223 "/projects/git/fluffos/src/compiler/internal/grammar.y"
          { rule_program((yyval.node)); }
#line 2400 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 3: /* program: program def possible_semi_colon  */
#line 227 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                                  { CREATE_TWO_VALUES((yyval.node), 0, (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 2406 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 4: /* program: %empty  */
#line 228 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                        { (yyval.node) = 0; }
#line 2412 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 6: /* possible_semi_colon: ';'  */
#line 233 "/projects/git/fluffos/src/compiler/internal/grammar.y"
        { yywarn("Extra ';'. Ignored."); }
#line 2418 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 7: /* inheritance: type_modifier_list L_INHERIT string_con1 ';'  */
#line 237 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                                               { if (rule_inheritence(&(yyval.node), (yyvsp[-3].number), (yyvsp[-1].string))) { YYACCEPT; } }
#line 2424 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 8: /* real: L_REAL  */
#line 241 "/projects/git/fluffos/src/compiler/internal/grammar.y"
         { CREATE_REAL((yyval.node), (yyvsp[0].real)); }
#line 2430 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 9: /* number: L_NUMBER  */
#line 245 "/projects/git/fluffos/src/compiler/internal/grammar.y"
           { CREATE_NUMBER((yyval.node), (yyvsp[0].number)); }
#line 2436 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 10: /* optional_star: %empty  */
#line 249 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                        { (yyval.number) = 0; }
#line 2442 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 11: /* optional_star: '*'  */
#line 250 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                { (yyval.number) = TYPE_MOD_ARRAY; }
#line 2448 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 12: /* block_or_semi: block  */
#line 255 "/projects/git/fluffos/src/compiler/internal/grammar.y"
          {
            (yyval.node) = (yyvsp[0].decl).node;
            if (!(yyval.node)) {
              CREATE_RETURN((yyval.node), 0);
            }
          }
#line 2459 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 13: /* block_or_semi: ';'  */
#line 261 "/projects/git/fluffos/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2465 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 14: /* block_or_semi: error  */
#line 262 "/projects/git/fluffos/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2471 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 15: /* identifier: L_DEFINED_NAME  */
#line 266 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                                  { (yyval.string) = scratch_copy((yyvsp[0].ihe)->name); }
#line 2477 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 17: /* $@1: %empty  */
#line 271 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                                  { (yyvsp[-2].number) = rule_func_type((yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].string)); }
#line 2483 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 18: /* @2: %empty  */
#line 272 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                                  { (yyval.number) = rule_func_proto((yyvsp[-6].number), (yyvsp[-5].number), &(yyvsp[-4].string), (yyvsp[-1].argument)); }
#line 2489 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 19: /* function: type optional_star identifier $@1 '(' argument ')' @2 block_or_semi  */
#line 273 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                                  { rule_func(&(yyval.node), (yyvsp[-8].number), (yyvsp[-7].number), (yyvsp[-6].string), (yyvsp[-3].argument), &(yyvsp[-1].number), &(yyvsp[0].node)); }
#line 2495 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 21: /* def: type name_list ';'  */
#line 279 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                                  {
                                    if (!((yyvsp[-2].number) & ~(DECL_MODS)) && (pragmas & PRAGMA_STRICT_TYPES))
                                      yyerror("Missing type for global variable declaration");
                                    (yyval.node) = 0;
                                  }
#line 2505 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 25: /* modifier_change: type_modifier_list ':'  */
#line 291 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                                  {
                                    if (!(yyvsp[-1].number))
                                      yyerror("modifier list may not be empty.");

                                    if ((yyvsp[-1].number) & FUNC_VARARGS) {
                                      yyerror("Illegal modifier 'varargs' in global modifier list.");
                                      (yyvsp[-1].number) &= ~FUNC_VARARGS;
                                    }

                                    if (!((yyvsp[-1].number) & DECL_ACCESS)) (yyvsp[-1].number) |= DECL_PUBLIC;
                                    global_modifiers = (yyvsp[-1].number);
                                    (yyval.node) = 0;
                                  }
#line 2523 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 26: /* member_name: optional_star identifier  */
#line 308 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                                  {
                                    /* At this point, the current_type here is only a basic_type */
                                    /* and cannot be unused yet - Sym */

                                    if (current_type == TYPE_VOID)
                                      yyerror("Illegal to declare class member of type void.");
                                    add_local_name((yyvsp[0].string), current_type | (yyvsp[-1].number));
                                    scratch_free((yyvsp[0].string));
                                  }
#line 2537 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 30: /* $@3: %empty  */
#line 326 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                           { current_type = (yyvsp[0].number); }
#line 2543 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 32: /* @4: %empty  */
#line 331 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                                             { (yyvsp[-2].ihe) = rule_define_class(&(yyval.number), (yyvsp[-1].string)); }
#line 2549 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 33: /* type_decl: type_modifier_list L_CLASS identifier '{' @4 member_list '}'  */
#line 332 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                                             { rule_define_class_members((yyvsp[-5].ihe), (yyvsp[-2].number)); (yyval.node) = 0; }
#line 2555 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 35: /* new_local_name: L_DEFINED_NAME  */
#line 338 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                                            {
                                              if ((yyvsp[0].ihe)->dn.local_num != -1) {
                                                yyerror("Illegal to redeclare local name '%s'", (yyvsp[0].ihe)->name);
                                              }
                                              (yyval.string) = scratch_copy((yyvsp[0].ihe)->name);
                                            }
#line 2566 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 37: /* atomic_type: L_CLASS L_DEFINED_NAME  */
#line 349 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                                            {
                                              if ((yyvsp[0].ihe)->dn.class_num == -1) {
                                                yyerror("Undefined class '%s'", (yyvsp[0].ihe)->name);
                                                (yyval.number) = TYPE_ANY;
                                              } else {
                                                (yyval.number) = (yyvsp[0].ihe)->dn.class_num | TYPE_MOD_CLASS;
                                              }
                                            }
#line 2579 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 38: /* atomic_type: L_CLASS L_IDENTIFIER  */
#line 358 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                                            {
                                              yyerror("Undefined class '%s'", (yyvsp[0].string));
                                              (yyval.number) = TYPE_ANY;
                                            }
#line 2588 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 40: /* opt_atomic_type: %empty  */
#line 366 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                        { (yyval.number) = TYPE_ANY; }
#line 2594 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 42: /* basic_type: opt_atomic_type L_ARRAY  */
#line 371 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                            { (yyval.number) = (yyvsp[-1].number) | TYPE_MOD_ARRAY; }
#line 2600 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 44: /* arg_type: basic_type ref  */
#line 376 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                   { (yyval.number) = (yyvsp[-1].number) | LOCAL_MOD_REF; }
#line 2606 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 45: /* optional_default_arg_value: %empty  */
#line 380 "/projects/git/fluffos/src/compiler/internal/grammar.y"
         { (yyval.node) = 0; }
#line 2612 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 46: /* optional_default_arg_value: ':' L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 381 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                                          {
    if (CONFIG_INT(__RC_WOMBLES__)) {
        if(*(outp-2) != ':') {
          yyerror("End of functional not found");
        }
    }
    if (current_function_context->num_locals)
        yyerror("Illegal to use local variable in functional.");
    if (current_function_context->values_list->r.expr)
        current_function_context->values_list->r.expr->kind = current_function_context->values_list->kind;

    (yyval.node) = new_node();
    (yyval.node)->kind = NODE_FUNCTION_CONSTRUCTOR;
    (yyval.node)->type = TYPE_FUNCTION;
    (yyval.node)->l.expr = (yyvsp[-2].node);
    (yyval.node)->r.expr = nullptr; // no arguments
    (yyval.node)->v.number = FP_FUNCTIONAL + 0 /* args */;
    pop_function_context();
}
#line 2636 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 47: /* new_arg: arg_type optional_star  */
#line 403 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                                              {
                                                (yyval.number) = (yyvsp[-1].number) | (yyvsp[0].number);
                                                if ((yyvsp[-1].number) != TYPE_VOID)
                                                  add_local_name("", (yyvsp[-1].number) | (yyvsp[0].number));
                                              }
#line 2646 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 48: /* new_arg: arg_type optional_star new_local_name optional_default_arg_value  */
#line 409 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                                              {
                                                if ((yyvsp[-3].number) == TYPE_VOID)
                                                  yyerror("Illegal to declare argument of type void.");
                                                add_local_name((yyvsp[-1].string), (yyvsp[-3].number) | (yyvsp[-2].number), (yyvsp[0].node));
                                                scratch_free((yyvsp[-1].string));
                                                (yyval.number) = (yyvsp[-3].number) | (yyvsp[-2].number);
                                              }
#line 2658 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 49: /* new_arg: new_local_name  */
#line 417 "/projects/git/fluffos/src/compiler/internal/grammar.y"
                                              {
                                                if (exact_types) {
                                                  yyerror("Missing type for argument");
                                                }
                                                add_local_name((yyvsp[0].string), TYPE_ANY);
                                                scratch_free((yyvsp[0].string));
                                                (yyval.number) = TYPE_ANY;
                                              }
#line 2671 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 50: /* argument: %empty  */
#line 429 "/projects/git/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.argument).num_arg = 0;
      (yyval.argument).flags = 0;
    }
#line 2680 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 52: /* argument: argument_list L_DOT_DOT_DOT  */
#line 435 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      int x = type_of_locals_ptr[max_num_locals-1];
      int lt = x & ~LOCAL_MODS;

      (yyval.argument) = (yyvsp[-1].argument);
      (yyval.argument).flags |= ARG_IS_VARARGS;

      if (x & LOCAL_MOD_REF) {
        yyerror("Variable to hold remainder of args may not be a reference");
        x &= ~LOCAL_MOD_REF;
      }
      if (lt != TYPE_ANY && !(lt & TYPE_MOD_ARRAY))
        yywarn("Variable to hold remainder of arguments should be an array.");
    }
#line 2699 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 53: /* argument_list: new_arg  */
#line 453 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (((yyvsp[0].number) & TYPE_MASK) == TYPE_VOID && !((yyvsp[0].number) & TYPE_MOD_CLASS)) {
        if ((yyvsp[0].number) & ~TYPE_MASK)
          yyerror("Illegal to declare argument of type void.");
        (yyval.argument).num_arg = 0;
      } else {
        (yyval.argument).num_arg = 1;
      }
      (yyval.argument).flags = 0;
    }
#line 2714 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 54: /* argument_list: argument_list ',' new_arg  */
#line 464 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (!(yyval.argument).num_arg)    /* first arg was void w/no name */
        yyerror("argument of type void must be the only argument.");
      if (((yyvsp[0].number) & TYPE_MASK) == TYPE_VOID && !((yyvsp[0].number) & TYPE_MOD_CLASS))
        yyerror("Illegal to declare argument of type void.");

      (yyval.argument) = (yyvsp[-2].argument);
      (yyval.argument).num_arg++;
    }
#line 2728 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 55: /* type_modifier_list: %empty  */
#line 477 "/projects/git/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.number) = 0;
    }
#line 2736 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 56: /* type_modifier_list: L_TYPE_MODIFIER type_modifier_list  */
#line 481 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-1].number) | (yyvsp[0].number);
      int acc_mod = (yyval.number) & DECL_ACCESS;
#ifdef SENSIBLE_MODIFIERS
      if (acc_mod & (acc_mod - 1)) {
        char buf[256];
        char *end = EndOf(buf);
        get_type_modifiers(buf, end, acc_mod);
        yyerror("Multiple access modifiers (%s)", buf);
        (yyval.number) = DECL_PUBLIC;
      }
#endif
    }
#line 2754 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 57: /* type: type_modifier_list opt_basic_type  */
#line 498 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = ((yyvsp[-1].number) << 16) | (yyvsp[0].number);
      current_type = (yyval.number);
    }
#line 2763 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 58: /* cast: '(' basic_type optional_star ')'  */
#line 506 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) | (yyvsp[-1].number);
    }
#line 2771 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 60: /* opt_basic_type: %empty  */
#line 514 "/projects/git/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.number) = TYPE_UNKNOWN;
    }
#line 2779 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 63: /* new_name: optional_star identifier  */
#line 526 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (current_type & (FUNC_VARARGS << 16)){
        yyerror("Illegal to declare varargs variable.");
        current_type &= ~(FUNC_VARARGS << 16);
      }
      /* Now it is ok to merge the two
       * remember that class_num and varargs was the reason for above
       * Do the merging once only per row of decls
       */

      if (current_type & 0xffff0000){
        current_type = (current_type >> 16) | (current_type & 0xffff);
      }

      current_type |= global_modifiers;

      if (!(current_type & DECL_ACCESS)) current_type |= DECL_PUBLIC;

      if ((current_type & ~DECL_MODS) == TYPE_VOID)
        yyerror("Illegal to declare global variable of type void.");

      define_new_variable((yyvsp[0].string), current_type | (yyvsp[-1].number));
      scratch_free((yyvsp[0].string));
    }
#line 2808 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 64: /* new_name: optional_star identifier L_ASSIGN expr0  */
#line 551 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      parse_node_t *expr, *newnode;
      int type;

      if (current_type & (FUNC_VARARGS << 16)){
        yyerror("Illegal to declare varargs variable.");
        current_type &= ~(FUNC_VARARGS << 16);
      }

      if (current_type & 0xffff0000){
        current_type = (current_type >> 16) | (current_type & 0xffff);
      }

      current_type |= global_modifiers;

      if (!(current_type & DECL_ACCESS)) current_type |= DECL_PUBLIC;

      if ((current_type & ~DECL_MODS) == TYPE_VOID)
        yyerror("Illegal to declare global variable of type void.");

      if ((yyvsp[-1].number) != F_ASSIGN)
        yyerror("Only '=' is legal in initializers.");

      /* ignore current_type == 0, which gets a missing type error
         later anyway */
      if (current_type) {
        type = (current_type | (yyvsp[-3].number)) & ~DECL_MODS;
        if ((current_type & ~DECL_MODS) == TYPE_VOID)
          yyerror("Illegal to declare global variable of type void.");
        if (!compatible_types(type, (yyvsp[0].node)->type)) {
          char buff[256];
          char *end = EndOf(buff);
          char *p;

          p = strput(buff, end, "Type mismatch ");
          p = get_two_types(p, end, type, (yyvsp[0].node)->type);
          p = strput(p, end, " when initializing ");
          p = strput(p, end, (yyvsp[-2].string));
          yyerror(buff);
        }
      } else type = 0;
      (yyvsp[0].node) = do_promotions((yyvsp[0].node), type);

      CREATE_BINARY_OP(expr, F_VOID_ASSIGN, 0, (yyvsp[0].node), 0);
      CREATE_OPCODE_1(expr->r.expr, F_GLOBAL_LVALUE, 0,
          define_new_variable((yyvsp[-2].string), current_type | (yyvsp[-3].number)));
      newnode = comp_trees[TREE_INIT];
      CREATE_TWO_VALUES(comp_trees[TREE_INIT], 0,
          newnode, expr);
      scratch_free((yyvsp[-2].string));
    }
#line 2864 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 65: /* @5: %empty  */
#line 606 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    { (yyval.number) = current_number_of_locals; }
#line 2870 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 66: /* block: '{' @5 block_statements '}'  */
#line 608 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.decl).node = (yyvsp[-1].decl).node;
      (yyval.decl).num = current_number_of_locals - (yyvsp[-2].number);  /* calculate locals declared in this block */
    }
#line 2879 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 70: /* local_declarations: %empty  */
#line 618 "/projects/git/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.decl).node = 0;
      (yyval.decl).num = 0;
    }
#line 2888 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 71: /* $@6: %empty  */
#line 623 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[0].number) == TYPE_VOID)
        yyerror("Illegal to declare local variable of type void.");
      /* can't do this in basic_type b/c local_name_list contains
       * expr0 which contains cast which contains basic_type
       */
      current_type = (yyvsp[0].number);
    }
#line 2901 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 72: /* local_declarations: local_declarations basic_type $@6 local_name_list ';'  */
#line 632 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-4].decl).node && (yyvsp[-1].decl).node) {
        CREATE_STATEMENTS((yyval.decl).node, (yyvsp[-4].decl).node, (yyvsp[-1].decl).node);
      } else (yyval.decl).node = ((yyvsp[-4].decl).node ? (yyvsp[-4].decl).node : (yyvsp[-1].decl).node);
      (yyval.decl).num = (yyvsp[-4].decl).num + (yyvsp[-1].decl).num;
    }
#line 2912 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 73: /* new_local_def: optional_star new_local_name  */
#line 642 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (current_type & LOCAL_MOD_REF) {
        yyerror("Illegal to declare local variable as reference");
        current_type &= ~LOCAL_MOD_REF;
      }
      add_local_name((yyvsp[0].string), current_type | (yyvsp[-1].number) | LOCAL_MOD_UNUSED);

      scratch_free((yyvsp[0].string));
      (yyval.node) = 0;
    }
#line 2927 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 74: /* new_local_def: optional_star new_local_name L_ASSIGN expr0  */
#line 653 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      int type = (current_type | (yyvsp[-3].number)) & ~DECL_MODS;

      if (current_type & LOCAL_MOD_REF) {
        yyerror("Illegal to declare local variable as reference");
        current_type &= ~LOCAL_MOD_REF;
        type &= ~LOCAL_MOD_REF;
      }

      if ((yyvsp[-1].number) != F_ASSIGN)
        yyerror("Only '=' is allowed in initializers.");
      if (!compatible_types((yyvsp[0].node)->type, type)) {
        char buff[256];
        char *end = EndOf(buff);
        char *p;

        p = strput(buff, end, "Type mismatch ");
        p = get_two_types(p, end, type, (yyvsp[0].node)->type);
        p = strput(p, end, " when initializing ");
        p = strput(p, end, (yyvsp[-2].string));

        yyerror(buff);
      }

      (yyvsp[0].node) = do_promotions((yyvsp[0].node), type);

      CREATE_UNARY_OP_1((yyval.node), F_VOID_ASSIGN_LOCAL, 0, (yyvsp[0].node),
          add_local_name((yyvsp[-2].string), current_type | (yyvsp[-3].number) | LOCAL_MOD_UNUSED));
      scratch_free((yyvsp[-2].string));
    }
#line 2962 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 75: /* single_new_local_def: arg_type optional_star new_local_name  */
#line 687 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-2].number) == TYPE_VOID)
        yyerror("Illegal to declare local variable of type void.");

      (yyval.number) = add_local_name((yyvsp[0].string), (yyvsp[-2].number) | (yyvsp[-1].number));
      scratch_free((yyvsp[0].string));
    }
#line 2974 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 76: /* single_new_local_def_with_init: single_new_local_def L_ASSIGN expr0  */
#line 698 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      int type = type_of_locals_ptr[(yyvsp[-2].number)];

      if (type & LOCAL_MOD_REF) {
        yyerror("Illegal to declare local variable as reference");
        type_of_locals_ptr[(yyvsp[-2].number)] &= ~LOCAL_MOD_REF;
      }
      type &= ~LOCAL_MODS;

      if ((yyvsp[-1].number) != F_ASSIGN)
        yyerror("Only '=' is allowed in initializers.");
      if (!compatible_types((yyvsp[0].node)->type, type)) {
        char buff[256];
        char *end = EndOf(buff);
        char *p;

        p = strput(buff, end, "Type mismatch ");
        p = get_two_types(p, end, type, (yyvsp[0].node)->type);
        p = strput(p, end, " when initializing.");
        yyerror(buff);
      }

      (yyvsp[0].node) = do_promotions((yyvsp[0].node), type);

      /* this is an expression */
      CREATE_BINARY_OP((yyval.node), F_ASSIGN, 0, (yyvsp[0].node), 0);
      CREATE_OPCODE_1((yyval.node)->r.expr, F_LOCAL_LVALUE, 0, (yyvsp[-2].number));
    }
#line 3007 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 77: /* local_name_list: new_local_def  */
#line 730 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.decl).node = (yyvsp[0].node);
      (yyval.decl).num = 1;
    }
#line 3016 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 78: /* local_name_list: new_local_def ',' local_name_list  */
#line 735 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-2].node) && (yyvsp[0].decl).node) {
        CREATE_STATEMENTS((yyval.decl).node, (yyvsp[-2].node), (yyvsp[0].decl).node);
      } else (yyval.decl).node = ((yyvsp[-2].node) ? (yyvsp[-2].node) : (yyvsp[0].decl).node);
      (yyval.decl).num = 1 + (yyvsp[0].decl).num;
    }
#line 3027 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 79: /* $@7: %empty  */
#line 745 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[0].number) == TYPE_VOID)
        yyerror("Illegal to declare local variable of type void.");
      current_type = (yyvsp[0].number);
    }
#line 3037 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 80: /* local_declaration_statement: basic_type $@7 local_name_list ';'  */
#line 751 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.decl).node = (yyvsp[-1].decl).node;
      (yyval.decl).num = (yyvsp[-1].decl).num;
    }
#line 3046 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 81: /* block_statements: %empty  */
#line 759 "/projects/git/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.decl).node = 0;
      (yyval.decl).num = 0;
    }
#line 3055 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 82: /* block_statements: statement block_statements  */
#line 764 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-1].node) && (yyvsp[0].decl).node) {
        CREATE_STATEMENTS((yyval.decl).node, (yyvsp[-1].node), (yyvsp[0].decl).node);
      } else (yyval.decl).node = ((yyvsp[-1].node) ? (yyvsp[-1].node) : (yyvsp[0].decl).node);
      (yyval.decl).num = (yyvsp[0].decl).num;
    }
#line 3066 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 83: /* block_statements: local_declaration_statement block_statements  */
#line 771 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-1].decl).node && (yyvsp[0].decl).node) {
        CREATE_STATEMENTS((yyval.decl).node, (yyvsp[-1].decl).node, (yyvsp[0].decl).node);
      } else (yyval.decl).node = ((yyvsp[-1].decl).node ? (yyvsp[-1].decl).node : (yyvsp[0].decl).node);
      (yyval.decl).num = (yyvsp[-1].decl).num + (yyvsp[0].decl).num;
    }
#line 3077 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 84: /* block_statements: error ';' block_statements  */
#line 778 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.decl).node = (yyvsp[0].decl).node;
      (yyval.decl).num = (yyvsp[0].decl).num;
    }
#line 3086 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 85: /* statement: comma_expr ';'  */
#line 786 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = pop_value((yyvsp[-1].node));
#ifdef DEBUG
      {
        parse_node_t *replacement;
        CREATE_STATEMENTS(replacement, (yyval.node), 0);
        CREATE_OPCODE(replacement->r.expr, F_BREAK_POINT, 0);
        (yyval.node) = replacement;
      }
#endif
    }
#line 3102 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 91: /* statement: decl_block  */
#line 803 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[0].decl).node;
      pop_n_locals((yyvsp[0].decl).num);
    }
#line 3111 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 92: /* statement: ';'  */
#line 808 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = 0;
    }
#line 3119 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 93: /* statement: L_BREAK ';'  */
#line 812 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (context & SPECIAL_CONTEXT) {
        yyerror("Cannot break out of catch { } or time_expression { }");
        (yyval.node) = 0;
      } else
        if (context & SWITCH_CONTEXT) {
          CREATE_CONTROL_JUMP((yyval.node), CJ_BREAK_SWITCH);
        } else
          if (context & LOOP_CONTEXT) {
            CREATE_CONTROL_JUMP((yyval.node), CJ_BREAK);
            if (context & LOOP_FOREACH) {
              parse_node_t *replace;
              CREATE_STATEMENTS(replace, 0, (yyval.node));
              CREATE_OPCODE(replace->l.expr, F_EXIT_FOREACH, 0);
              (yyval.node) = replace;
            }
          } else {
            yyerror("break statement outside loop");
            (yyval.node) = 0;
          }
    }
#line 3145 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 94: /* statement: L_CONTINUE ';'  */
#line 834 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (context & SPECIAL_CONTEXT)
        yyerror("Cannot continue out of catch { } or time_expression { }");
      else
        if (!(context & LOOP_CONTEXT))
          yyerror("continue statement outside loop");
      CREATE_CONTROL_JUMP((yyval.node), CJ_CONTINUE);
    }
#line 3158 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 95: /* $@8: %empty  */
#line 846 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[-3].number) = context;
      context = LOOP_CONTEXT;
    }
#line 3167 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 96: /* while: L_WHILE '(' comma_expr ')' $@8 statement  */
#line 851 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_LOOP((yyval.node), 1, (yyvsp[0].node), 0, optimize_loop_test((yyvsp[-3].node)));
      context = (yyvsp[-5].number);
    }
#line 3176 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 97: /* $@9: %empty  */
#line 859 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[0].number) = context;
      context = LOOP_CONTEXT;
    }
#line 3185 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 98: /* do: L_DO $@9 statement L_WHILE '(' comma_expr ')' ';'  */
#line 864 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_LOOP((yyval.node), 0, (yyvsp[-5].node), 0, optimize_loop_test((yyvsp[-2].node)));
      context = (yyvsp[-7].number);
    }
#line 3194 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 99: /* $@10: %empty  */
#line 872 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[-5].decl).node = pop_value((yyvsp[-5].decl).node);
      (yyvsp[-7].number) = context;
      context = LOOP_CONTEXT;
    }
#line 3204 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 100: /* for: L_FOR '(' first_for_expr ';' for_expr ';' for_expr ')' $@10 statement  */
#line 878 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.decl).num = (yyvsp[-7].decl).num; /* number of declarations (0/1) */

      (yyvsp[-3].node) = pop_value((yyvsp[-3].node));
      if ((yyvsp[-3].node) && IS_NODE((yyvsp[-3].node), NODE_UNARY_OP, F_INC)
          && IS_NODE((yyvsp[-3].node)->r.expr, NODE_OPCODE_1, F_LOCAL_LVALUE)) {
        LPC_INT lvar = (yyvsp[-3].node)->r.expr->l.number;
        CREATE_OPCODE_1((yyvsp[-3].node), F_LOOP_INCR, 0, lvar);
      }

      CREATE_STATEMENTS((yyval.decl).node, (yyvsp[-7].decl).node, 0);
      CREATE_LOOP((yyval.decl).node->r.expr, 1, (yyvsp[0].node), (yyvsp[-3].node), optimize_loop_test((yyvsp[-5].node)));

      context = (yyvsp[-9].number);
    }
#line 3224 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 101: /* foreach_var: L_DEFINED_NAME  */
#line 897 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[0].ihe)->dn.local_num != -1) {
        CREATE_OPCODE_1((yyval.decl).node, F_LOCAL_LVALUE, 0, (yyvsp[0].ihe)->dn.local_num);
        type_of_locals_ptr[(yyvsp[0].ihe)->dn.local_num] &= ~LOCAL_MOD_UNUSED;
      } else
        if ((yyvsp[0].ihe)->dn.global_num != -1) {
          CREATE_OPCODE_1((yyval.decl).node, F_GLOBAL_LVALUE, 0, (yyvsp[0].ihe)->dn.global_num);
        } else {
          char buf[256];
          char *end = EndOf(buf);
          char *p;

          p = strput(buf, end, "'");
          p = strput(p, end, (yyvsp[0].ihe)->name);
          p = strput(p, end, "' is not a local or a global variable.");
          yyerror(buf);
          CREATE_OPCODE_1((yyval.decl).node, F_GLOBAL_LVALUE, 0, 0);
        }
      (yyval.decl).num = 0;
    }
#line 3249 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 102: /* foreach_var: single_new_local_def  */
#line 918 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (type_of_locals_ptr[(yyvsp[0].number)] & LOCAL_MOD_REF) {
        CREATE_OPCODE_1((yyval.decl).node, F_REF_LVALUE, 0, (yyvsp[0].number));
      } else {
        CREATE_OPCODE_1((yyval.decl).node, F_LOCAL_LVALUE, 0, (yyvsp[0].number));
        type_of_locals_ptr[(yyvsp[0].number)] &= ~LOCAL_MOD_UNUSED;
      }
      (yyval.decl).num = 1;
    }
#line 3263 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 103: /* foreach_var: L_IDENTIFIER  */
#line 928 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      char buf[256];
      char *end = EndOf(buf);
      char *p;

      p = strput(buf, end, "'");
      p = strput(p, end, (yyvsp[0].string));
      p = strput(p, end, "' is not a local or a global variable.");
      yyerror(buf);
      CREATE_OPCODE_1((yyval.decl).node, F_GLOBAL_LVALUE, 0, 0);
      scratch_free((yyvsp[0].string));
      (yyval.decl).num = 0;
    }
#line 3281 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 104: /* foreach_vars: foreach_var  */
#line 945 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_FOREACH((yyval.decl).node, (yyvsp[0].decl).node, 0);
      (yyval.decl).num = (yyvsp[0].decl).num;
    }
#line 3290 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 105: /* foreach_vars: foreach_var ',' foreach_var  */
#line 950 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_FOREACH((yyval.decl).node, (yyvsp[-2].decl).node, (yyvsp[0].decl).node);
      (yyval.decl).num = (yyvsp[-2].decl).num + (yyvsp[0].decl).num;
      if ((yyvsp[-2].decl).node->v.number == F_REF_LVALUE)
        yyerror("Mapping key may not be a reference in foreach()");
    }
#line 3301 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 106: /* $@11: %empty  */
#line 960 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[-3].decl).node->v.expr = (yyvsp[-1].node);
      (yyvsp[-5].number) = context;
      context = LOOP_CONTEXT | LOOP_FOREACH;
    }
#line 3311 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 107: /* foreach: L_FOREACH '(' foreach_vars L_IN expr0 ')' $@11 statement  */
#line 966 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.decl).num = (yyvsp[-5].decl).num;

      CREATE_STATEMENTS((yyval.decl).node, (yyvsp[-5].decl).node, 0);
      CREATE_LOOP((yyval.decl).node->r.expr, 2, (yyvsp[0].node), 0, 0);
      CREATE_OPCODE((yyval.decl).node->r.expr->r.expr, F_NEXT_FOREACH, 0);

      context = (yyvsp[-7].number);
    }
#line 3325 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 108: /* for_expr: %empty  */
#line 979 "/projects/git/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.node) = 0;
    }
#line 3333 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 110: /* first_for_expr: for_expr  */
#line 987 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.decl).node = (yyvsp[0].node);
      (yyval.decl).num = 0;
    }
#line 3342 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 111: /* first_for_expr: single_new_local_def_with_init  */
#line 992 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.decl).node = (yyvsp[0].node);
      (yyval.decl).num = 1;
    }
#line 3351 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 112: /* $@12: %empty  */
#line 1000 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[-3].number) = context;
      context &= LOOP_CONTEXT;
      context |= SWITCH_CONTEXT;
      (yyvsp[-2].number) = mem_block[A_CASES].current_size;
    }
#line 3362 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 113: /* switch: L_SWITCH '(' comma_expr ')' $@12 '{' local_declarations case switch_block '}'  */
#line 1007 "/projects/git/fluffos/src/compiler/internal/grammar.y"
      {
        parse_node_t *node1, *node2;

        if ((yyvsp[-1].node)) {
          CREATE_STATEMENTS(node1, (yyvsp[-2].node), (yyvsp[-1].node));
        } else node1 = (yyvsp[-2].node);

        if (context & SWITCH_STRINGS) {
          NODE_NO_LINE(node2, NODE_SWITCH_STRINGS);
        } else if (context & SWITCH_RANGES) {
          NODE_NO_LINE(node2, NODE_SWITCH_RANGES);
        } else if ((context & SWITCH_NUMBERS) ||
            (context & SWITCH_NOT_EMPTY)) {
          NODE_NO_LINE(node2, NODE_SWITCH_NUMBERS);
        } else {
          // to prevent crashing during the remaining parsing bits
          NODE_NO_LINE(node2, NODE_SWITCH_NUMBERS);

          yyerror("need case statements in switch/case, not just default:"); //just a default case present
        }

        node2->l.expr = (yyvsp[-7].node);
        node2->r.expr = node1;
        prepare_cases(node2, (yyvsp[-8].number));
        context = (yyvsp[-9].number);
        (yyval.node) = node2;
        pop_n_locals((yyvsp[-3].decl).num);
      }
#line 3395 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 114: /* switch_block: case switch_block  */
#line 1039 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[0].node)){
        CREATE_STATEMENTS((yyval.node), (yyvsp[-1].node), (yyvsp[0].node));
      } else (yyval.node) = (yyvsp[-1].node);
    }
#line 3405 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 115: /* switch_block: statement switch_block  */
#line 1045 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[0].node)){
        CREATE_STATEMENTS((yyval.node), (yyvsp[-1].node), (yyvsp[0].node));
      } else (yyval.node) = (yyvsp[-1].node);
    }
#line 3415 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 116: /* switch_block: %empty  */
#line 1051 "/projects/git/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.node) = 0;
    }
#line 3423 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 117: /* case: L_CASE case_label ':'  */
#line 1058 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[-1].node);
      (yyval.node)->v.expr = 0;

      add_to_mem_block(A_CASES, (char *)&((yyvsp[-1].node)), sizeof((yyvsp[-1].node)));
    }
#line 3434 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 118: /* case: L_CASE case_label L_RANGE case_label ':'  */
#line 1065 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ( (yyvsp[-3].node)->kind != NODE_CASE_NUMBER
          || (yyvsp[-1].node)->kind != NODE_CASE_NUMBER )
        yyerror("String case labels not allowed as range bounds");
      if ((yyvsp[-3].node)->r.number > (yyvsp[-1].node)->r.number) break;

      context |= SWITCH_RANGES;

      (yyval.node) = (yyvsp[-3].node);
      (yyval.node)->v.expr = (yyvsp[-1].node);

      add_to_mem_block(A_CASES, (char *)&((yyvsp[-3].node)), sizeof((yyvsp[-3].node)));
    }
#line 3452 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 119: /* case: L_CASE case_label L_RANGE ':'  */
#line 1079 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ( (yyvsp[-2].node)->kind != NODE_CASE_NUMBER )
        yyerror("String case labels not allowed as range bounds");

      context |= SWITCH_RANGES;

      (yyval.node) = (yyvsp[-2].node);
      (yyval.node)->v.expr = new_node();
      (yyval.node)->v.expr->kind = NODE_CASE_NUMBER;
      (yyval.node)->v.expr->r.number = LPC_INT_MAX; //maxint

      add_to_mem_block(A_CASES, (char *)&((yyvsp[-2].node)), sizeof((yyvsp[-2].node)));
    }
#line 3470 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 120: /* case: L_CASE L_RANGE case_label ':'  */
#line 1093 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ( (yyvsp[-1].node)->kind != NODE_CASE_NUMBER )
        yyerror("String case labels not allowed as range bounds");

      context |= SWITCH_RANGES;
      (yyval.node) = new_node();
      (yyval.node)->kind = NODE_CASE_NUMBER;
      (yyval.node)->r.number = LPC_INT_MIN; //maxint +1 wraps to min_int, on all computers i know, just not in the C standard iirc
      (yyval.node)->v.expr = (yyvsp[-1].node);

      add_to_mem_block(A_CASES, (char *)&((yyval.node)), sizeof((yyval.node)));
    }
#line 3487 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 121: /* case: L_DEFAULT ':'  */
#line 1106 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (context & SWITCH_DEFAULT) {
        yyerror("Duplicate default");
        (yyval.node) = 0;
        break;
      }
      (yyval.node) = new_node();
      (yyval.node)->kind = NODE_DEFAULT;
      (yyval.node)->v.expr = 0;
      add_to_mem_block(A_CASES, (char *)&((yyval.node)), sizeof((yyval.node)));
      context |= SWITCH_DEFAULT;
    }
#line 3504 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 122: /* case_label: constant  */
#line 1122 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ((context & SWITCH_STRINGS) && (yyvsp[0].number))
        yyerror("Mixed case label list not allowed");

      if ((yyvsp[0].number))
        context |= SWITCH_NUMBERS;
      else
        context |= SWITCH_NOT_EMPTY;

      (yyval.node) = new_node();
      (yyval.node)->kind = NODE_CASE_NUMBER;
      (yyval.node)->r.number = (LPC_INT)(yyvsp[0].number);
    }
#line 3522 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 123: /* case_label: string_con1  */
#line 1136 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      POINTER_INT str;
      str = store_prog_string((yyvsp[0].string));
      scratch_free((yyvsp[0].string));
      if (context & SWITCH_NUMBERS)
        yyerror("Mixed case label list not allowed");
      context |= SWITCH_STRINGS;
      (yyval.node) = new_node();
      (yyval.node)->kind = NODE_CASE_STRING;
      (yyval.node)->r.number = (LPC_INT)str;
    }
#line 3538 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 124: /* constant: constant '|' constant  */
#line 1151 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) | (yyvsp[0].number);
    }
#line 3546 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 125: /* constant: constant '^' constant  */
#line 1155 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) ^ (yyvsp[0].number);
    }
#line 3554 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 126: /* constant: constant '&' constant  */
#line 1159 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) & (yyvsp[0].number);
    }
#line 3562 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 127: /* constant: constant L_EQ constant  */
#line 1163 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) == (yyvsp[0].number);
    }
#line 3570 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 128: /* constant: constant L_NE constant  */
#line 1167 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) != (yyvsp[0].number);
    }
#line 3578 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 129: /* constant: constant L_ORDER constant  */
#line 1171 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      switch((yyvsp[-1].number)){
        case F_GE: (yyval.number) = (yyvsp[-2].number) >= (yyvsp[0].number); break;
        case F_LE: (yyval.number) = (yyvsp[-2].number) <= (yyvsp[0].number); break;
        case F_GT: (yyval.number) = (yyvsp[-2].number) >  (yyvsp[0].number); break;
      }
    }
#line 3590 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 130: /* constant: constant '<' constant  */
#line 1179 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) < (yyvsp[0].number);
    }
#line 3598 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 131: /* constant: constant L_LSH constant  */
#line 1183 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) << (yyvsp[0].number);
    }
#line 3606 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 132: /* constant: constant L_RSH constant  */
#line 1187 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) >> (yyvsp[0].number);
    }
#line 3614 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 133: /* constant: constant '+' constant  */
#line 1191 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) + (yyvsp[0].number);
    }
#line 3622 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 134: /* constant: constant '-' constant  */
#line 1195 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) - (yyvsp[0].number);
    }
#line 3630 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 135: /* constant: constant '*' constant  */
#line 1199 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) * (yyvsp[0].number);
    }
#line 3638 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 136: /* constant: constant '%' constant  */
#line 1203 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[0].number)) (yyval.number) = (yyvsp[-2].number) % (yyvsp[0].number); else yyerror("Modulo by zero");
    }
#line 3646 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 137: /* constant: constant '/' constant  */
#line 1207 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[0].number)) (yyval.number) = (yyvsp[-2].number) / (yyvsp[0].number); else yyerror("Division by zero");
    }
#line 3654 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 138: /* constant: '(' constant ')'  */
#line 1211 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-1].number);
    }
#line 3662 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 139: /* constant: L_NUMBER  */
#line 1215 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[0].number);
    }
#line 3670 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 140: /* constant: '-' L_NUMBER  */
#line 1219 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = -(yyvsp[0].number);
    }
#line 3678 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 141: /* constant: L_NOT L_NUMBER  */
#line 1223 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = !(yyvsp[0].number);
    }
#line 3686 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 142: /* constant: '~' L_NUMBER  */
#line 1227 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = ~(yyvsp[0].number);
    }
#line 3694 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 143: /* comma_expr: expr0  */
#line 1234 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3702 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 144: /* comma_expr: comma_expr ',' expr0  */
#line 1238 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_TWO_VALUES((yyval.node), (yyvsp[0].node)->type, pop_value((yyvsp[-2].node)), (yyvsp[0].node));
    }
#line 3710 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 146: /* expr0: ref lvalue  */
#line 1249 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      int op;

      if (!(context & ARG_LIST))
        yyerror("ref illegal outside function argument list");
      else
        num_refs++;

      switch ((yyvsp[0].node)->kind) {
        case NODE_PARAMETER_LVALUE:
          op = F_LOCAL_LVALUE;
          break;
        case NODE_TERNARY_OP:
        case NODE_OPCODE_1:
        case NODE_UNARY_OP_1:
        case NODE_BINARY_OP:
          op = (yyvsp[0].node)->v.number;
          if (op > F_RINDEX_LVALUE)
            yyerror("Illegal to make reference to range");
          break;
        default:
          op=0; //0 is harmless, i hope
          yyerror("unknown lvalue kind");
      }
      CREATE_UNARY_OP_1((yyval.node), F_MAKE_REF, TYPE_ANY, (yyvsp[0].node), op);
    }
#line 3741 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 147: /* expr0: lvalue L_ASSIGN expr0  */
#line 1276 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      parse_node_t *l = (yyvsp[-2].node), *r = (yyvsp[0].node);
      int opcode = (yyvsp[-1].number);

      if (opcode == F_LOR_EQ || opcode == F_LAND_EQ || opcode == F_NULLISH_EQ) {
        if (exact_types && !compatible_types(r->type, l->type)) {
          char buf[256];
          char *end = EndOf(buf);
          char *p;
          p = strput(buf, end, "Bad assignment ");
          p = get_two_types(p, end, l->type, r->type);
          p = strput(p, end, ".");
          yyerror(buf);
        }
        CREATE_LOGICAL_ASSIGN((yyval.node), opcode, l, r);
      } else {
        /* set this up here so we can change it below */
        /* assignments are backwards; rhs is evaluated before
           lhs, so put the RIGHT hand side on the LEFT hand
           side of the tree node. */
        CREATE_BINARY_OP((yyval.node), opcode, r->type, r, l);

        /* allow TYPE_STRING += TYPE_NUMBER | TYPE_OBJECT */
        if (exact_types && !compatible_types(r->type, l->type) &&
            !(opcode == F_ADD_EQ && l->type == TYPE_STRING &&
              ((COMP_TYPE(r->type, TYPE_NUMBER)) || r->type == TYPE_OBJECT))) {
          char buf[256];
          char *end = EndOf(buf);
          char *p;
          p = strput(buf, end, "Bad assignment ");
          p = get_two_types(p, end, l->type, r->type);
          p = strput(p, end, ".");
          yyerror(buf);
        }

        if (opcode == F_ASSIGN)
          (yyval.node)->l.expr = do_promotions(r, l->type);
      }
    }
#line 3785 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 148: /* expr0: error L_ASSIGN expr0  */
#line 1316 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      yyerror("Illegal LHS");
      CREATE_ERROR((yyval.node));
    }
#line 3794 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 149: /* expr0: expr0 '?' expr0 ':' expr0  */
#line 1321 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      parse_node_t *p1 = (yyvsp[-2].node), *p2 = (yyvsp[0].node);

      if (exact_types && !compatible_types2(p1->type, p2->type)) {
        char buf[256];
        char *end = EndOf(buf);
        char *p;

        p = strput(buf, end, "Types in ?: do not match ");
        p = get_two_types(p, end, p1->type, p2->type);
        p = strput(p, end, ".");
        yywarn(buf);
      }

      /* optimize if last expression did F_NOT */
      if (IS_NODE((yyvsp[-4].node), NODE_UNARY_OP, F_NOT)) {
        /* !a ? b : c  --> a ? c : b */
        CREATE_IF((yyval.node), (yyvsp[-4].node)->r.expr, p2, p1);
      } else {
        CREATE_IF((yyval.node), (yyvsp[-4].node), p1, p2);
      }
      (yyval.node)->type = ((p1->type == p2->type) ? p1->type : TYPE_ANY);
    }
#line 3822 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 150: /* expr0: expr0 L_QUESTION_QUESTION expr0  */
#line 1345 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      /* Nullish coalescing: left ?? right
       * Return left if defined, otherwise return right */
      CREATE_NULLISH((yyval.node), (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 3832 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 151: /* expr0: expr0 L_LOR expr0  */
#line 1351 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_LAND_LOR((yyval.node), F_LOR, (yyvsp[-2].node), (yyvsp[0].node));
      if (IS_NODE((yyvsp[-2].node), NODE_LAND_LOR, F_LOR))
        (yyvsp[-2].node)->kind = NODE_BRANCH_LINK;
    }
#line 3842 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 152: /* expr0: expr0 L_LAND expr0  */
#line 1357 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_LAND_LOR((yyval.node), F_LAND, (yyvsp[-2].node), (yyvsp[0].node));
      if (IS_NODE((yyvsp[-2].node), NODE_LAND_LOR, F_LAND))
        (yyvsp[-2].node)->kind = NODE_BRANCH_LINK;
    }
#line 3852 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 153: /* expr0: expr0 '|' expr0  */
#line 1363 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      int t1 = (yyvsp[-2].node)->type, t3 = (yyvsp[0].node)->type;

      if (is_boolean((yyvsp[-2].node)) && is_boolean((yyvsp[0].node)))
        yywarn("bitwise operation on boolean values.");
      if ((t1 & TYPE_MOD_ARRAY) || (t3 & TYPE_MOD_ARRAY)) {
        if (t1 != t3) {
          if ((t1 != TYPE_ANY) && (t3 != TYPE_ANY) &&
              !(t1 & t3 & TYPE_MOD_ARRAY)) {
            char buf[256];
            char *end = EndOf(buf);
            char *p;

            p = strput(buf, end, "Incompatible types for | ");
            p = get_two_types(p, end, t1, t3);
            p = strput(p, end, ".");
            yyerror(buf);
          }
          t1 = TYPE_ANY | TYPE_MOD_ARRAY;
        }
        CREATE_BINARY_OP((yyval.node), F_OR, t1, (yyvsp[-2].node), (yyvsp[0].node));
      }
      else (yyval.node) = binary_int_op((yyvsp[-2].node), (yyvsp[0].node), F_OR, "|");
    }
#line 3881 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 154: /* expr0: expr0 '^' expr0  */
#line 1388 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = binary_int_op((yyvsp[-2].node), (yyvsp[0].node), F_XOR, "^");
    }
#line 3889 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 155: /* expr0: expr0 '&' expr0  */
#line 1392 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      int t1 = (yyvsp[-2].node)->type, t3 = (yyvsp[0].node)->type;
      if (is_boolean((yyvsp[-2].node)) && is_boolean((yyvsp[0].node)))
        yywarn("bitwise operation on boolean values.");
      if ((t1 & TYPE_MOD_ARRAY) || (t3 & TYPE_MOD_ARRAY)) {
        if (t1 != t3) {
          if ((t1 != TYPE_ANY) && (t3 != TYPE_ANY) &&
              !(t1 & t3 & TYPE_MOD_ARRAY)) {
            char buf[256];
            char *end = EndOf(buf);
            char *p;

            p = strput(buf, end, "Incompatible types for & ");
            p = get_two_types(p, end, t1, t3);
            p = strput(p, end, ".");
            yyerror(buf);
          }
          t1 = TYPE_ANY | TYPE_MOD_ARRAY;
        }
        CREATE_BINARY_OP((yyval.node), F_AND, t1, (yyvsp[-2].node), (yyvsp[0].node));
      } else (yyval.node) = binary_int_op((yyvsp[-2].node), (yyvsp[0].node), F_AND, "&");
    }
#line 3916 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 156: /* expr0: expr0 L_EQ expr0  */
#line 1415 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (exact_types && !compatible_types2((yyvsp[-2].node)->type, (yyvsp[0].node)->type)){
        char buf[256];
        char *end = EndOf(buf);
        char *p;

        p = strput(buf, end, "== always false because of incompatible types ");
        p = get_two_types(p, end, (yyvsp[-2].node)->type, (yyvsp[0].node)->type);
        p = strput(p, end, ".");
        yyerror(buf);
      }
      /* x == 0 -> !x */
      if (IS_NODE((yyvsp[-2].node), NODE_NUMBER, 0)) {
        CREATE_UNARY_OP((yyval.node), F_NOT, TYPE_NUMBER, (yyvsp[0].node));
      } else
        if (IS_NODE((yyvsp[0].node), NODE_NUMBER, 0)) {
          CREATE_UNARY_OP((yyval.node), F_NOT, TYPE_NUMBER, (yyvsp[-2].node));
        } else {
          CREATE_BINARY_OP((yyval.node), F_EQ, TYPE_NUMBER, (yyvsp[-2].node), (yyvsp[0].node));
        }
    }
#line 3942 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 157: /* expr0: expr0 L_NE expr0  */
#line 1437 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (exact_types && !compatible_types2((yyvsp[-2].node)->type, (yyvsp[0].node)->type)){
        char buf[256];
        char *end = EndOf(buf);
        char *p;

        p = strput(buf, end, "!= always true because of incompatible types ");
        p = get_two_types(p, end, (yyvsp[-2].node)->type, (yyvsp[0].node)->type);
        p = strput(p, end, ".");
        yyerror(buf);
      }
      CREATE_BINARY_OP((yyval.node), F_NE, TYPE_NUMBER, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 3960 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 158: /* expr0: expr0 L_ORDER expr0  */
#line 1451 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (exact_types) {
        int t1 = (yyvsp[-2].node)->type;
        int t3 = (yyvsp[0].node)->type;

        if (!COMP_TYPE(t1, TYPE_NUMBER)
            && !COMP_TYPE(t1, TYPE_STRING)) {
          char buf[256];
          char *end = EndOf(buf);
          char *p;

          p = strput(buf, end, "Bad left argument to '");
          p = strput(p, end, query_instr_name((yyvsp[-1].number)));
          p = strput(p, end, "' : \"");
          p = get_type_name(p, end, t1);
          p = strput(p, end, "\"");
          yyerror(buf);
        } else if (!COMP_TYPE(t3, TYPE_NUMBER)
            && !COMP_TYPE(t3, TYPE_STRING)) {
          char buf[256];
          char *end = EndOf(buf);
          char *p;

          p = strput(buf, end, "Bad right argument to '");
          p = strput(p, end, query_instr_name((yyvsp[-1].number)));
          p = strput(p, end, "' : \"");
          p = get_type_name(p, end, t3);
          p = strput(p, end, "\"");
          yyerror(buf);
        } else if (!compatible_types2(t1,t3)) {
          char buf[256];
          char *end = EndOf(buf);
          char *p;

          p = strput(buf, end, "Arguments to ");
          p = strput(p, end, query_instr_name((yyvsp[-1].number)));
          p = strput(p, end, " do not have compatible types : ");
          p = get_two_types(p, end, t1, t3);
          yyerror(buf);
        }
      }
      CREATE_BINARY_OP((yyval.node), (yyvsp[-1].number), TYPE_NUMBER, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 4008 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 159: /* expr0: expr0 '<' expr0  */
#line 1495 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (exact_types) {
        int t1 = (yyvsp[-2].node)->type, t3 = (yyvsp[0].node)->type;

        if (!COMP_TYPE(t1, TYPE_NUMBER)
            && !COMP_TYPE(t1, TYPE_STRING)) {
          char buf[256];
          char *end = EndOf(buf);
          char *p;

          p = strput(buf, end, "Bad left argument to '<' : \"");
          p = get_type_name(p, end, t1);
          p = strput(p, end, "\"");
          yyerror(buf);
        } else if (!COMP_TYPE(t3, TYPE_NUMBER)
            && !COMP_TYPE(t3, TYPE_STRING)) {
          char buf[200];
          char *end = EndOf(buf);
          char *p;

          p = strput(buf, end, "Bad right argument to '<' : \"");
          p = get_type_name(p, end, t3);
          p = strput(p, end, "\"");
          yyerror(buf);
        } else if (!compatible_types2(t1,t3)) {
          char buf[256];
          char *end = EndOf(buf);
          char *p;

          p = strput(buf, end, "Arguments to < do not have compatible types : ");
          p = get_two_types(p, end, t1, t3);
          yyerror(buf);
        }
      }
      CREATE_BINARY_OP((yyval.node), F_LT, TYPE_NUMBER, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 4049 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 160: /* expr0: expr0 L_LSH expr0  */
#line 1532 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = binary_int_op((yyvsp[-2].node), (yyvsp[0].node), F_LSH, "<<");
    }
#line 4057 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 161: /* expr0: expr0 L_RSH expr0  */
#line 1536 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = binary_int_op((yyvsp[-2].node), (yyvsp[0].node), F_RSH, ">>");
    }
#line 4065 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 162: /* expr0: expr0 '+' expr0  */
#line 1540 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      int result_type;

      if (exact_types) {
        int t1 = (yyvsp[-2].node)->type, t3 = (yyvsp[0].node)->type;

        if (t1 == t3){
#ifdef CAST_CALL_OTHERS
          if (t1 == TYPE_UNKNOWN){
            yyerror("Bad arguments to '+' (unknown vs unknown)");
            result_type = TYPE_ANY;
          } else
#endif
            result_type = t1;
        }
        else if (t1 == TYPE_ANY) {
          if (t3 == TYPE_FUNCTION) {
            yyerror("Bad right argument to '+' (function)");
            result_type = TYPE_ANY;
          } else result_type = t3;
        } else if (t3 == TYPE_ANY) {
          if (t1 == TYPE_FUNCTION) {
            yyerror("Bad left argument to '+' (function)");
            result_type = TYPE_ANY;
          } else result_type = t1;
        } else {
          switch(t1) {
            case TYPE_OBJECT:
              if(t3 == TYPE_STRING){
                result_type = TYPE_STRING;
              } else goto add_error;
              break;
            case TYPE_STRING:
              {
                if (t3 == TYPE_REAL || t3 == TYPE_NUMBER || t3 == TYPE_OBJECT){
                  result_type = TYPE_STRING;
                } else goto add_error;
                break;
              }
            case TYPE_NUMBER:
              {
                if (t3 == TYPE_REAL || t3 == TYPE_STRING)
                  result_type = t3;
                else goto add_error;
                break;
              }
            case TYPE_REAL:
              {
                if (t3 == TYPE_NUMBER) result_type = TYPE_REAL;
                else if (t3 == TYPE_STRING) result_type = TYPE_STRING;
                else goto add_error;
                break;
              }
            default:
              {
                if (t1 & t3 & TYPE_MOD_ARRAY) {
                  result_type = TYPE_ANY|TYPE_MOD_ARRAY;
                  break;
                }
            add_error:
                {
                  char buf[256];
                  char *end = EndOf(buf);
                  char *p;

                  p = strput(buf, end, "Invalid argument types to '+' ");
                  p = get_two_types(p, end, t1, t3);
                  yyerror(buf);
                  result_type = TYPE_ANY;
                }
              }
          }
        }
      } else
        result_type = TYPE_ANY;

      /* TODO: perhaps we should do (string)+(number) and
       * (number)+(string) constant folding as well.
       *
       * codefor string x = "foo" + 1;
       *
       * 0000: push string 13, number 1
       * 0004: +
       * 0005: (void)assign_local LV0
       */
      switch ((yyvsp[-2].node)->kind) {
        case NODE_NUMBER:
          /* 0 + X */
          if ((yyvsp[-2].node)->v.number == 0 &&
              ((yyvsp[0].node)->type == TYPE_NUMBER || (yyvsp[0].node)->type == TYPE_REAL)) {
            (yyval.node) = (yyvsp[0].node);
            break;
          }
          if ((yyvsp[0].node)->kind == NODE_NUMBER) {
            (yyval.node) = (yyvsp[-2].node);
            (yyvsp[-2].node)->v.number += (yyvsp[0].node)->v.number;
            break;
          }
          if ((yyvsp[0].node)->kind == NODE_REAL) {
            (yyval.node) = (yyvsp[0].node);
            (yyvsp[0].node)->v.real += (yyvsp[-2].node)->v.number;
            break;
          }
          /* swapping the nodes may help later constant folding */
          if ((yyvsp[0].node)->type != TYPE_STRING && (yyvsp[0].node)->type != TYPE_ANY)
            CREATE_BINARY_OP((yyval.node), F_ADD, result_type, (yyvsp[0].node), (yyvsp[-2].node));
          else
            CREATE_BINARY_OP((yyval.node), F_ADD, result_type, (yyvsp[-2].node), (yyvsp[0].node));
          break;
        case NODE_REAL:
          if ((yyvsp[0].node)->kind == NODE_NUMBER) {
            (yyval.node) = (yyvsp[-2].node);
            (yyvsp[-2].node)->v.real += (yyvsp[0].node)->v.number;
            break;
          }
          if ((yyvsp[0].node)->kind == NODE_REAL) {
            (yyval.node) = (yyvsp[-2].node);
            (yyvsp[-2].node)->v.real += (yyvsp[0].node)->v.real;
            break;
          }
          /* swapping the nodes may help later constant folding */
          if ((yyvsp[0].node)->type != TYPE_STRING && (yyvsp[0].node)->type != TYPE_ANY)
            CREATE_BINARY_OP((yyval.node), F_ADD, result_type, (yyvsp[0].node), (yyvsp[-2].node));
          else
            CREATE_BINARY_OP((yyval.node), F_ADD, result_type, (yyvsp[-2].node), (yyvsp[0].node));
          break;
        case NODE_STRING:
          if ((yyvsp[0].node)->kind == NODE_STRING) {
            /* Combine strings */
            LPC_INT n1, n2;
            const char *s1, *s2;
            char *news;
            int l;

            n1 = (yyvsp[-2].node)->v.number;
            n2 = (yyvsp[0].node)->v.number;
            s1 = PROG_STRING(n1);
            s2 = PROG_STRING(n2);
            news = (char *)DMALLOC( (l = strlen(s1))+strlen(s2)+1, TAG_COMPILER, "combine string" );
            strcpy(news, s1);
            strcat(news + l, s2);
            /* free old strings (ordering may help shrink table) */
            if (n1 > n2) {
              free_prog_string(n1); free_prog_string(n2);
            } else {
              free_prog_string(n2); free_prog_string(n1);
            }
            (yyval.node) = (yyvsp[-2].node);
            (yyval.node)->v.number = store_prog_string(news);
            FREE(news);
            break;
          }
          /* Yes, this can actually happen for absurd code like:
           * (int)"foo" + 0
           * for which I guess we ought to generate (int)"foo"
           * in order to be consistent.  Then shoot the coder.
           */
          /* FALLTHROUGH */
        default:
          /* X + 0 */
          if (IS_NODE((yyvsp[0].node), NODE_NUMBER, 0) &&
              ((yyvsp[-2].node)->type == TYPE_NUMBER || (yyvsp[-2].node)->type == TYPE_REAL)) {
            (yyval.node) = (yyvsp[-2].node);
            break;
          }
          CREATE_BINARY_OP((yyval.node), F_ADD, result_type, (yyvsp[-2].node), (yyvsp[0].node));
          break;
      }
    }
#line 4239 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 163: /* expr0: expr0 '-' expr0  */
#line 1710 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      int result_type;

      if (exact_types) {
        int t1 = (yyvsp[-2].node)->type, t3 = (yyvsp[0].node)->type;

        if (t1 == t3){
          switch(t1){
            case TYPE_ANY:
            case TYPE_NUMBER:
            case TYPE_REAL:
              result_type = t1;
              break;
            default:
              if (!(t1 & TYPE_MOD_ARRAY)){
                type_error("Bad argument number 1 to '-'", t1);
                result_type = TYPE_ANY;
              } else result_type = t1;
          }
        } else if (t1 == TYPE_ANY){
          switch(t3){
            case TYPE_REAL:
            case TYPE_NUMBER:
              result_type = t3;
              break;
            default:
              if (!(t3 & TYPE_MOD_ARRAY)){
                type_error("Bad argument number 2 to '-'", t3);
                result_type = TYPE_ANY;
              } else result_type = t3;
          }
        } else if (t3 == TYPE_ANY){
          switch(t1){
            case TYPE_REAL:
            case TYPE_NUMBER:
              result_type = t1;
              break;
            default:
              if (!(t1 & TYPE_MOD_ARRAY)){
                type_error("Bad argument number 1 to '-'", t1);
                result_type = TYPE_ANY;
              } else result_type = t1;
          }
        } else if ((t1 == TYPE_REAL && t3 == TYPE_NUMBER) ||
            (t3 == TYPE_REAL && t1 == TYPE_NUMBER)){
          result_type = TYPE_REAL;
        } else if (t1 & t3 & TYPE_MOD_ARRAY){
          result_type = TYPE_MOD_ARRAY|TYPE_ANY;
        } else {
          char buf[256];
          char *end = EndOf(buf);
          char *p;

          p = strput(buf, end, "Invalid types to '-' ");
          p = get_two_types(p, end, t1, t3);
          yyerror(buf);
          result_type = TYPE_ANY;
        }
      } else result_type = TYPE_ANY;

      switch ((yyvsp[-2].node)->kind) {
        case NODE_NUMBER:
          if ((yyvsp[-2].node)->v.number == 0) {
            CREATE_UNARY_OP((yyval.node), F_NEGATE, (yyvsp[0].node)->type, (yyvsp[0].node));
          } else if ((yyvsp[0].node)->kind == NODE_NUMBER) {
            (yyval.node) = (yyvsp[-2].node);
            (yyvsp[-2].node)->v.number -= (yyvsp[0].node)->v.number;
          } else if ((yyvsp[0].node)->kind == NODE_REAL) {
            (yyval.node) = (yyvsp[0].node);
            (yyvsp[0].node)->v.real = (yyvsp[-2].node)->v.number - (yyvsp[0].node)->v.real;
          } else {
            CREATE_BINARY_OP((yyval.node), F_SUBTRACT, result_type, (yyvsp[-2].node), (yyvsp[0].node));
          }
          break;
        case NODE_REAL:
          if ((yyvsp[0].node)->kind == NODE_NUMBER) {
            (yyval.node) = (yyvsp[-2].node);
            (yyvsp[-2].node)->v.real -= (yyvsp[0].node)->v.number;
          } else if ((yyvsp[0].node)->kind == NODE_REAL) {
            (yyval.node) = (yyvsp[-2].node);
            (yyvsp[-2].node)->v.real -= (yyvsp[0].node)->v.real;
          } else {
            CREATE_BINARY_OP((yyval.node), F_SUBTRACT, result_type, (yyvsp[-2].node), (yyvsp[0].node));
          }
          break;
        default:
          /* optimize X-0 */
          if (IS_NODE((yyvsp[0].node), NODE_NUMBER, 0)) {
            (yyval.node) = (yyvsp[-2].node);
          }
          CREATE_BINARY_OP((yyval.node), F_SUBTRACT, result_type, (yyvsp[-2].node), (yyvsp[0].node));
      }
    }
#line 4337 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 164: /* expr0: expr0 '*' expr0  */
#line 1804 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      int result_type;

      if (exact_types){
        int t1 = (yyvsp[-2].node)->type, t3 = (yyvsp[0].node)->type;

        if (t1 == t3){
          switch(t1){
            case TYPE_MAPPING:
            case TYPE_ANY:
            case TYPE_NUMBER:
            case TYPE_REAL:
              result_type = t1;
              break;
            default:
              type_error("Bad argument number 1 to '*'", t1);
              result_type = TYPE_ANY;
          }
        } else if (t1 == TYPE_ANY || t3 == TYPE_ANY){
          int t = (t1 == TYPE_ANY) ? t3 : t1;
          switch(t){
            case TYPE_NUMBER:
            case TYPE_REAL:
            case TYPE_MAPPING:
              result_type = t;
              break;
            default:
              type_error((t1 == TYPE_ANY) ?
                  "Bad argument number 2 to '*'" :
                  "Bad argument number 1 to '*'",
                  t);
              result_type = TYPE_ANY;
          }
        } else if ((t1 == TYPE_NUMBER && t3 == TYPE_REAL) ||
            (t1 == TYPE_REAL && t3 == TYPE_NUMBER)){
          result_type = TYPE_REAL;
        } else {
          char buf[256];
          char *end = EndOf(buf);
          char *p;

          p = strput(buf, end, "Invalid types to '*' ");
          p = get_two_types(p, end, t1, t3);
          yyerror(buf);
          result_type = TYPE_ANY;
        }
      } else result_type = TYPE_ANY;

      switch ((yyvsp[-2].node)->kind) {
        case NODE_NUMBER:
          if ((yyvsp[0].node)->kind == NODE_NUMBER) {
            (yyval.node) = (yyvsp[-2].node);
            (yyval.node)->v.number *= (yyvsp[0].node)->v.number;
            break;
          }
          if ((yyvsp[0].node)->kind == NODE_REAL) {
            (yyval.node) = (yyvsp[0].node);
            (yyvsp[0].node)->v.real *= (yyvsp[-2].node)->v.number;
            break;
          }
          CREATE_BINARY_OP((yyval.node), F_MULTIPLY, result_type, (yyvsp[0].node), (yyvsp[-2].node));
          break;
        case NODE_REAL:
          if ((yyvsp[0].node)->kind == NODE_NUMBER) {
            (yyval.node) = (yyvsp[-2].node);
            (yyvsp[-2].node)->v.real *= (yyvsp[0].node)->v.number;
            break;
          }
          if ((yyvsp[0].node)->kind == NODE_REAL) {
            (yyval.node) = (yyvsp[-2].node);
            (yyvsp[-2].node)->v.real *= (yyvsp[0].node)->v.real;
            break;
          }
          CREATE_BINARY_OP((yyval.node), F_MULTIPLY, result_type, (yyvsp[0].node), (yyvsp[-2].node));
          break;
        default:
          CREATE_BINARY_OP((yyval.node), F_MULTIPLY, result_type, (yyvsp[-2].node), (yyvsp[0].node));
      }
    }
#line 4421 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 165: /* expr0: expr0 '%' expr0  */
#line 1884 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = binary_int_op((yyvsp[-2].node), (yyvsp[0].node), F_MOD, "%");
    }
#line 4429 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 166: /* expr0: expr0 '/' expr0  */
#line 1888 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      int result_type;

      if (exact_types){
        int t1 = (yyvsp[-2].node)->type, t3 = (yyvsp[0].node)->type;

        if (t1 == t3){
          switch(t1){
            case TYPE_NUMBER:
            case TYPE_REAL:
            case TYPE_ANY:
              result_type = t1;
              break;
            default:
              type_error("Bad argument 1 to '/'", t1);
              result_type = TYPE_ANY;
          }
        } else if (t1 == TYPE_ANY || t3 == TYPE_ANY){
          int t = (t1 == TYPE_ANY) ? t3 : t1;
          if (t == TYPE_REAL || t == TYPE_NUMBER)
            result_type = t;
          else {
            type_error(t1 == TYPE_ANY ?
                "Bad argument 2 to '/'" :
                "Bad argument 1 to '/'", t);
            result_type = TYPE_ANY;
          }
        } else if ((t1 == TYPE_NUMBER && t3 == TYPE_REAL) ||
            (t1 == TYPE_REAL && t3 == TYPE_NUMBER)) {
          result_type = TYPE_REAL;
        } else {
          char buf[256];
          char *end = EndOf(buf);
          char *p;

          p = strput(buf, end, "Invalid types to '/' ");
          p = get_two_types(p, end, t1, t3);
          yyerror(buf);
          result_type = TYPE_ANY;
        }
      } else result_type = TYPE_ANY;

      /* constant expressions */
      switch ((yyvsp[-2].node)->kind) {
        case NODE_NUMBER:
          if ((yyvsp[0].node)->kind == NODE_NUMBER) {
            if ((yyvsp[0].node)->v.number == 0) {
              yyerror("Divide by zero in constant");
              (yyval.node) = (yyvsp[-2].node);
              break;
            }
            (yyval.node) = (yyvsp[-2].node);
            (yyvsp[-2].node)->v.number /= (yyvsp[0].node)->v.number;
            break;
          }
          if ((yyvsp[0].node)->kind == NODE_REAL) {
            if ((yyvsp[0].node)->v.real == 0.0) {
              yyerror("Divide by zero in constant");
              (yyval.node) = (yyvsp[-2].node);
              break;
            }
            (yyval.node) = (yyvsp[0].node);
            (yyvsp[0].node)->v.real = ((yyvsp[-2].node)->v.number / (yyvsp[0].node)->v.real);
            break;
          }
          CREATE_BINARY_OP((yyval.node), F_DIVIDE, result_type, (yyvsp[-2].node), (yyvsp[0].node));
          break;
        case NODE_REAL:
          if ((yyvsp[0].node)->kind == NODE_NUMBER) {
            if ((yyvsp[0].node)->v.number == 0) {
              yyerror("Divide by zero in constant");
              (yyval.node) = (yyvsp[-2].node);
              break;
            }
            (yyval.node) = (yyvsp[-2].node);
            (yyvsp[-2].node)->v.real /= (yyvsp[0].node)->v.number;
            break;
          }
          if ((yyvsp[0].node)->kind == NODE_REAL) {
            if ((yyvsp[0].node)->v.real == 0.0) {
              yyerror("Divide by zero in constant");
              (yyval.node) = (yyvsp[-2].node);
              break;
            }
            (yyval.node) = (yyvsp[-2].node);
            (yyvsp[-2].node)->v.real /= (yyvsp[0].node)->v.real;
            break;
          }
          CREATE_BINARY_OP((yyval.node), F_DIVIDE, result_type, (yyvsp[-2].node), (yyvsp[0].node));
          break;
        default:
          CREATE_BINARY_OP((yyval.node), F_DIVIDE, result_type, (yyvsp[-2].node), (yyvsp[0].node));
      }
    }
#line 4528 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 167: /* expr0: cast expr0  */
#line 1983 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->type = (yyvsp[-1].number);

      if (exact_types &&
          (yyvsp[0].node)->type != (yyvsp[-1].number) &&
          (yyvsp[0].node)->type != TYPE_ANY &&
          (yyvsp[0].node)->type != TYPE_UNKNOWN &&
          (yyvsp[-1].number) != TYPE_VOID) {
        char buf[256];
        char *end = EndOf(buf);
        char *p;

        p = strput(buf, end, "Cannot cast ");
        p = get_type_name(p, end, (yyvsp[0].node)->type);
        p = strput(p, end, "to ");
        p = get_type_name(p, end, (yyvsp[-1].number));
        yyerror(buf);
      }
    }
#line 4553 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 168: /* expr0: L_INC lvalue  */
#line 2004 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_UNARY_OP((yyval.node), F_PRE_INC, 0, (yyvsp[0].node));
      if (exact_types){
        switch((yyvsp[0].node)->type){
          case TYPE_NUMBER:
          case TYPE_ANY:
          case TYPE_REAL:
            {
              (yyval.node)->type = (yyvsp[0].node)->type;
              break;
            }

          default:
            {
              (yyval.node)->type = TYPE_ANY;
              type_error("Bad argument 1 to ++x", (yyvsp[0].node)->type);
            }
        }
      } else (yyval.node)->type = TYPE_ANY;
    }
#line 4578 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 169: /* expr0: L_DEC lvalue  */
#line 2025 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_UNARY_OP((yyval.node), F_PRE_DEC, 0, (yyvsp[0].node));
      if (exact_types){
        switch((yyvsp[0].node)->type){
          case TYPE_NUMBER:
          case TYPE_ANY:
          case TYPE_REAL:
            {
              (yyval.node)->type = (yyvsp[0].node)->type;
              break;
            }

          default:
            {
              (yyval.node)->type = TYPE_ANY;
              type_error("Bad argument 1 to --x", (yyvsp[0].node)->type);
            }
        }
      } else (yyval.node)->type = TYPE_ANY;

    }
#line 4604 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 170: /* expr0: L_NOT expr0  */
#line 2047 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[0].node)->kind == NODE_NUMBER) {
        (yyval.node) = (yyvsp[0].node);
        (yyval.node)->v.number = !((yyval.node)->v.number);
      } else {
        CREATE_UNARY_OP((yyval.node), F_NOT, TYPE_NUMBER, (yyvsp[0].node));
      }
    }
#line 4617 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 171: /* expr0: '~' expr0  */
#line 2056 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (exact_types && !IS_TYPE((yyvsp[0].node)->type, TYPE_NUMBER))
        type_error("Bad argument to ~", (yyvsp[0].node)->type);
      if ((yyvsp[0].node)->kind == NODE_NUMBER) {
        (yyval.node) = (yyvsp[0].node);
        (yyval.node)->v.number = ~(yyval.node)->v.number;
      } else {
        CREATE_UNARY_OP((yyval.node), F_COMPL, TYPE_NUMBER, (yyvsp[0].node));
      }
    }
#line 4632 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 172: /* expr0: '-' expr0  */
#line 2067 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      int result_type;
      if (exact_types){
        int t = (yyvsp[0].node)->type;
        if (!COMP_TYPE(t, TYPE_NUMBER)){
          type_error("Bad argument to unary '-'", t);
          result_type = TYPE_ANY;
        } else result_type = t;
      } else result_type = TYPE_ANY;

      switch ((yyvsp[0].node)->kind) {
        case NODE_NUMBER:
          (yyval.node) = (yyvsp[0].node);
          (yyval.node)->v.number = -(yyval.node)->v.number;
          break;
        case NODE_REAL:
          (yyval.node) = (yyvsp[0].node);
          (yyval.node)->v.real = -(yyval.node)->v.real;
          break;
        default:
          CREATE_UNARY_OP((yyval.node), F_NEGATE, result_type, (yyvsp[0].node));
      }
    }
#line 4660 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 173: /* expr0: lvalue L_INC  */
#line 2091 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_UNARY_OP((yyval.node), F_POST_INC, 0, (yyvsp[-1].node));
      (yyval.node)->v.number = F_POST_INC;
      if (exact_types){
        switch((yyvsp[-1].node)->type){
          case TYPE_NUMBER:
          case TYPE_ANY:
          case TYPE_REAL:
            {
              (yyval.node)->type = (yyvsp[-1].node)->type;
              break;
            }

          default:
            {
              (yyval.node)->type = TYPE_ANY;
              type_error("Bad argument 1 to x++", (yyvsp[-1].node)->type);
            }
        }
      } else (yyval.node)->type = TYPE_ANY;
    }
#line 4686 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 174: /* expr0: lvalue L_DEC  */
#line 2113 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_UNARY_OP((yyval.node), F_POST_DEC, 0, (yyvsp[-1].node));
      if (exact_types){
        switch((yyvsp[-1].node)->type){
          case TYPE_NUMBER:
          case TYPE_ANY:
          case TYPE_REAL:
            {
              (yyval.node)->type = (yyvsp[-1].node)->type;
              break;
            }

          default:
            {
              (yyval.node)->type = TYPE_ANY;
              type_error("Bad argument 1 to x--", (yyvsp[-1].node)->type);
            }
        }
      } else (yyval.node)->type = TYPE_ANY;
    }
#line 4711 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 181: /* return: L_RETURN ';'  */
#line 2143 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (exact_types && !IS_TYPE(exact_types, TYPE_VOID))
        yywarn("Non-void functions must return a value.");
      CREATE_RETURN((yyval.node), 0);
    }
#line 4721 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 182: /* return: L_RETURN comma_expr ';'  */
#line 2149 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (exact_types && !compatible_types((yyvsp[-1].node)->type, exact_types)) {
        char buf[256];
        char *end = EndOf(buf);
        char *p;

        p = strput(buf, end, "Type of returned value doesn't match function return type ");
        p = get_two_types(p, end, (yyvsp[-1].node)->type, exact_types);
        yyerror(buf);
      }
      if (IS_NODE((yyvsp[-1].node), NODE_NUMBER, 0)) {
        CREATE_RETURN((yyval.node), 0);
      } else {
        CREATE_RETURN((yyval.node), (yyvsp[-1].node));
      }
    }
#line 4742 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 183: /* expr_list: %empty  */
#line 2169 "/projects/git/fluffos/src/compiler/internal/grammar.y"
            {
      CREATE_EXPR_LIST((yyval.node), 0);
    }
#line 4750 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 184: /* expr_list: expr_list2  */
#line 2173 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_EXPR_LIST((yyval.node), (yyvsp[0].node));
    }
#line 4758 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 185: /* expr_list: expr_list2 ','  */
#line 2177 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_EXPR_LIST((yyval.node), (yyvsp[-1].node));
    }
#line 4766 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 186: /* expr_list_node: expr0  */
#line 2184 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_EXPR_NODE((yyval.node), (yyvsp[0].node), 0);
    }
#line 4774 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 187: /* expr_list_node: expr0 L_DOT_DOT_DOT  */
#line 2188 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_EXPR_NODE((yyval.node), (yyvsp[-1].node), 1);
    }
#line 4782 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 188: /* expr_list2: expr_list_node  */
#line 2195 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[0].node)->kind = 1;
      (yyval.node) = (yyvsp[0].node);
    }
#line 4791 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 189: /* expr_list2: expr_list2 ',' expr_list_node  */
#line 2200 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[0].node)->kind = 0;

      (yyval.node) = (yyvsp[-2].node);
      (yyval.node)->kind++;
      (yyval.node)->l.expr->r.expr = (yyvsp[0].node);
      (yyval.node)->l.expr = (yyvsp[0].node);
    }
#line 4804 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 190: /* expr_list3: %empty  */
#line 2212 "/projects/git/fluffos/src/compiler/internal/grammar.y"
            {
      /* this is a dummy node */
      CREATE_EXPR_LIST((yyval.node), 0);
    }
#line 4813 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 191: /* expr_list3: expr_list4  */
#line 2217 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_EXPR_LIST((yyval.node), (yyvsp[0].node));
    }
#line 4821 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 192: /* expr_list3: expr_list4 ','  */
#line 2221 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_EXPR_LIST((yyval.node), (yyvsp[-1].node));
    }
#line 4829 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 193: /* expr_list4: assoc_pair  */
#line 2228 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = new_node_no_line();
      (yyval.node)->kind = 2;
      (yyval.node)->v.expr = (yyvsp[0].node);
      (yyval.node)->r.expr = 0;
      (yyval.node)->type = 0;
      /* we keep track of the end of the chain in the left nodes */
      (yyval.node)->l.expr = (yyval.node);
    }
#line 4843 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 194: /* expr_list4: expr_list4 ',' assoc_pair  */
#line 2238 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      parse_node_t *expr;

      expr = new_node_no_line();
      expr->kind = 0;
      expr->v.expr = (yyvsp[0].node);
      expr->r.expr = 0;
      expr->type = 0;

      (yyvsp[-2].node)->l.expr->r.expr = expr;
      (yyvsp[-2].node)->l.expr = expr;
      (yyvsp[-2].node)->kind += 2;
      (yyval.node) = (yyvsp[-2].node);
    }
#line 4862 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 195: /* assoc_pair: expr0 ':' expr0  */
#line 2256 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_TWO_VALUES((yyval.node), 0, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 4870 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 196: /* lvalue: expr4  */
#line 2263 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
#define LV_ILLEGAL 1
#define LV_RANGE 2
#define LV_INDEX 4
      /* Restrictive lvalues, but I think they make more sense :) */
      (yyval.node) = (yyvsp[0].node);
      if((yyval.node)->kind == NODE_BINARY_OP && (yyval.node)->v.number == F_TYPE_CHECK)
        (yyval.node) = (yyval.node)->l.expr;
      switch((yyval.node)->kind) {
        default:
          yyerror("Illegal lvalue");
          break;
        case NODE_PARAMETER:
          (yyval.node)->kind = NODE_PARAMETER_LVALUE;
          break;
        case NODE_TERNARY_OP:
          (yyval.node)->v.number = (yyval.node)->r.expr->v.number;
        case NODE_OPCODE_1:
        case NODE_UNARY_OP_1:
        case NODE_BINARY_OP:
          if ((yyval.node)->v.number >= F_LOCAL && (yyval.node)->v.number <= F_MEMBER)
            (yyval.node)->v.number++; /* make it an lvalue */
          else if ((yyval.node)->v.number >= F_INDEX
              && (yyval.node)->v.number <= F_RE_RANGE) {
            parse_node_t *node = (yyval.node);
            int flag = 0;
            do {
              switch(node->kind) {
                case NODE_PARAMETER:
                  node->kind = NODE_PARAMETER_LVALUE;
                  flag |= LV_ILLEGAL;
                  break;
                case NODE_TERNARY_OP:
                  node->v.number = node->r.expr->v.number;
                case NODE_OPCODE_1:
                case NODE_UNARY_OP_1:
                case NODE_BINARY_OP:
                  if(node->kind == NODE_BINARY_OP &&
                      node->v.number == F_TYPE_CHECK) {
                    node = node->l.expr;
                    continue;
                  }

                  if (node->v.number >= F_LOCAL
                      && node->v.number <= F_MEMBER) {
                    node->v.number++;
                    flag |= LV_ILLEGAL;
                    break;
                  } else if (node->v.number == F_INDEX ||
                      node->v.number == F_RINDEX) {
                    node->v.number++;
                    flag |= LV_INDEX;
                    break;
                  } else if (node->v.number >= F_ADD_EQ
                      && node->v.number <= F_ASSIGN) {
                    if (!(flag & LV_INDEX)) {
                      yyerror("Illegal lvalue, a possible lvalue is (x <assign> y)[a]");
                    }
                    if (node->r.expr->kind == NODE_BINARY_OP||
                        node->r.expr->kind == NODE_TERNARY_OP){
                      if (node->r.expr->v.number >= F_NN_RANGE_LVALUE && node->r.expr->v.number <= F_NR_RANGE_LVALUE)
                        yyerror("Illegal to have (x[a..b] <assign> y) to be the beginning of an lvalue");
                    }
                    flag = LV_ILLEGAL;
                    break;
                  } else if (node->v.number >= F_NN_RANGE
                      && node->v.number <= F_RE_RANGE) {
                    if (flag & LV_RANGE) {
                      yyerror("Can't do range lvalue of range lvalue.");
                      flag |= LV_ILLEGAL;
                      break;
                    }
                    if (flag & LV_INDEX){
                      yyerror("Can't do indexed lvalue of range lvalue.");
                      flag |= LV_ILLEGAL;
                      break;
                    }
                    if (node->v.number == F_NE_RANGE) {
                      /* x[foo..] -> x[foo..<1] */
                      parse_node_t *rchild = node->r.expr;
                      node->kind = NODE_TERNARY_OP;
                      CREATE_BINARY_OP(node->r.expr,
                          F_NR_RANGE_LVALUE,
                          0, 0, rchild);
                      CREATE_NUMBER(node->r.expr->l.expr, 1);
                    } else if (node->v.number == F_RE_RANGE) {
                      /* x[<foo..] -> x[<foo..<1] */
                      parse_node_t *rchild = node->r.expr;
                      node->kind = NODE_TERNARY_OP;
                      CREATE_BINARY_OP(node->r.expr,
                          F_RR_RANGE_LVALUE,
                          0, 0, rchild);
                      CREATE_NUMBER(node->r.expr->l.expr, 1);
                    } else
                      node->r.expr->v.number++;
                    flag |= LV_RANGE;
                    node = node->r.expr->r.expr;
                    continue;
                  }
                default:
                  yyerror("Illegal lvalue");
                  flag = LV_ILLEGAL;
                  break;
              }
              if ((flag & LV_ILLEGAL) || !(node = node->r.expr)) break;
            } while (1);
            break;
          } else
            yyerror("Illegal lvalue");
          break;
      }
    }
#line 4987 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 198: /* l_new_function_open: L_FUNCTION_OPEN efun_override  */
#line 2380 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = ((yyvsp[0].number) << 8) | FP_EFUN;
    }
#line 4995 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 200: /* expr4: L_DEFINED_NAME  */
#line 2388 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      int i;
      if ((i = (yyvsp[0].ihe)->dn.local_num) != -1) {
        type_of_locals_ptr[i] &= ~LOCAL_MOD_UNUSED;
        if (type_of_locals_ptr[i] & LOCAL_MOD_REF)
          CREATE_OPCODE_1((yyval.node), F_REF, type_of_locals_ptr[i] & ~LOCAL_MOD_REF,i & 0xff);
        else
          CREATE_OPCODE_1((yyval.node), F_LOCAL, type_of_locals_ptr[i], i & 0xff);
        if (current_function_context)
          current_function_context->num_locals++;
      } else
        if ((i = (yyvsp[0].ihe)->dn.global_num) != -1) {
          if (current_function_context)
            current_function_context->bindable = FP_NOT_BINDABLE;
          CREATE_OPCODE_1((yyval.node), F_GLOBAL,
              VAR_TEMP(i)->type & ~DECL_MODS, i);
          if (VAR_TEMP(i)->type & DECL_HIDDEN) {
            char buf[256];
            char *end = EndOf(buf);
            char *p;

            p = strput(buf, end, "Illegal to use private variable '");
            p = strput(p, end, (yyvsp[0].ihe)->name);
            p = strput(p, end, "'");
            yyerror(buf);
          }
        } else if ((yyvsp[0].ihe)->dn.function_num != -1) {
          /* Local function - create function pointer */
          (yyval.node) = new_node();
          (yyval.node)->kind = NODE_FUNCTION_CONSTRUCTOR;
          (yyval.node)->type = TYPE_FUNCTION;
          (yyval.node)->r.expr = 0;
          (yyval.node)->l.expr = 0;
          (yyval.node)->v.number = ((yyvsp[0].ihe)->dn.function_num << 8) | FP_LOCAL;
          
          if (current_function_context)
            current_function_context->bindable = FP_NOT_BINDABLE;
        } else if ((yyvsp[0].ihe)->dn.simul_num != -1) {
          /* Simul efun - create function pointer */
          (yyval.node) = new_node();
          (yyval.node)->kind = NODE_FUNCTION_CONSTRUCTOR;
          (yyval.node)->type = TYPE_FUNCTION;
          (yyval.node)->r.expr = 0;
          (yyval.node)->l.expr = 0;
          (yyval.node)->v.number = ((yyvsp[0].ihe)->dn.simul_num << 8) | FP_SIMUL;
          
          if (current_function_context)
            current_function_context->bindable = FP_NOT_BINDABLE;
        } else if ((yyvsp[0].ihe)->dn.efun_num != -1) {
          /* Efun - create function pointer */
          (yyval.node) = new_node();
          (yyval.node)->kind = NODE_FUNCTION_CONSTRUCTOR;
          (yyval.node)->type = TYPE_FUNCTION;
          (yyval.node)->r.expr = 0;
          (yyval.node)->l.expr = 0;
          (yyval.node)->v.number = ((yyvsp[0].ihe)->dn.efun_num << 8) | FP_EFUN;
        } else {
          char buf[256];
          char *end = EndOf(buf);
          char *p;

          auto max_local_variables = CFG_INT(__MAX_LOCAL_VARIABLES__);
          p = strput(buf, end, "Undefined variable '");
          p = strput(p, end, (yyvsp[0].ihe)->name);
          p = strput(p, end, "'");
          if (current_number_of_locals < max_local_variables) {
            add_local_name((yyvsp[0].ihe)->name, TYPE_ANY);
          }
          CREATE_ERROR((yyval.node));
          yyerror(buf);
        }
    }
#line 5072 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 201: /* expr4: L_IDENTIFIER  */
#line 2461 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      char buf[256];
      char *end = EndOf(buf);
      char *p;

      /* Treat bare identifiers as function pointers - let compiler resolve them later */
      (yyval.node) = new_node();
      (yyval.node)->kind = NODE_FUNCTION_CONSTRUCTOR;
      (yyval.node)->type = TYPE_FUNCTION;
      (yyval.node)->r.expr = 0;
      CREATE_STRING((yyval.node)->l.expr, (yyvsp[0].string));
      (yyval.node)->v.number = FP_FUNCTIONAL;
      
      /* Mark as not bindable - same as (: funcname :) syntax */
      if (current_function_context)
        current_function_context->bindable = FP_NOT_BINDABLE;
    }
#line 5094 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 202: /* expr4: L_PARAMETER  */
#line 2479 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_PARAMETER((yyval.node), TYPE_ANY, (yyvsp[0].number));
    }
#line 5102 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 203: /* @13: %empty  */
#line 2483 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.contextp) = current_function_context;
      /* already flagged as an error */
      if (current_function_context)
        current_function_context = current_function_context->parent;
    }
#line 5113 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 204: /* expr4: '$' '(' @13 comma_expr ')'  */
#line 2490 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      parse_node_t *node;

      current_function_context = (yyvsp[-2].contextp);

      if (!current_function_context || current_function_context->num_parameters < 0) {
        /* This was illegal, and error'ed when the '$' token
         * was returned.
         */
        CREATE_ERROR((yyval.node));
      } else {
        CREATE_OPCODE_1((yyval.node), F_LOCAL, (yyvsp[-1].node)->type,
            current_function_context->values_list->kind++);

        node = new_node_no_line();
        node->type = 0;
        current_function_context->values_list->l.expr->r.expr = node;
        current_function_context->values_list->l.expr = node;
        node->r.expr = 0;
        node->v.expr = (yyvsp[-1].node);
      }
    }
#line 5140 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 205: /* expr4: expr4 L_ARROW identifier  */
#line 2513 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-2].node)->type == TYPE_ANY) {
        int cmi;
        unsigned short tp;

        if ((cmi = lookup_any_class_member((yyvsp[0].string), &tp)) != -1) {
          CREATE_UNARY_OP_1((yyval.node), F_MEMBER, tp, (yyvsp[-2].node), 0);
          (yyval.node)->l.number = cmi;
        } else {
          CREATE_ERROR((yyval.node));
        }
      } else if (!IS_CLASS((yyvsp[-2].node)->type)) {
        yyerror("Left argument of -> is not a class");
        CREATE_ERROR((yyval.node));
      } else {
        CREATE_UNARY_OP_1((yyval.node), F_MEMBER, 0, (yyvsp[-2].node), 0);
        (yyval.node)->l.number = lookup_class_member(CLASS_IDX((yyvsp[-2].node)->type),
            (yyvsp[0].string),
            &((yyval.node)->type));
      }

      scratch_free((yyvsp[0].string));
    }
#line 5168 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 206: /* expr4: expr4 L_DOT identifier  */
#line 2537 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-2].node)->type == TYPE_ANY) {
        int cmi;
        unsigned short tp;

        if ((cmi = lookup_any_class_member((yyvsp[0].string), &tp)) != -1) {
          CREATE_UNARY_OP_1((yyval.node), F_MEMBER, tp, (yyvsp[-2].node), 0);
          (yyval.node)->l.number = cmi;
        } else {
          CREATE_ERROR((yyval.node));
        }
      } else if (!IS_CLASS((yyvsp[-2].node)->type)) {
        yyerror("Left argument of . is not a class");
        CREATE_ERROR((yyval.node));
      } else {
        CREATE_UNARY_OP_1((yyval.node), F_MEMBER, 0, (yyvsp[-2].node), 0);
        (yyval.node)->l.number = lookup_class_member(CLASS_IDX((yyvsp[-2].node)->type),
            (yyvsp[0].string),
            &((yyval.node)->type));
      }

      scratch_free((yyvsp[0].string));
    }
#line 5196 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 207: /* expr4: expr4 '[' comma_expr L_RANGE comma_expr ']'  */
#line 2561 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
    if (!CONFIG_INT(__RC_OLD_RANGE_BEHAVIOR__)) {
      if (CONFIG_INT(__RC_WARN_OLD_RANGE_BEHAVIOR__)) {
      if ((yyvsp[-5].node)->type != TYPE_MAPPING &&
          (yyvsp[-1].node)->kind == NODE_NUMBER && (yyvsp[-1].node)->v.number < 0)
        yywarn("A negative constant as the second element of arr[x..y] no longer means indexing from the end.  Use arr[x..<y]");
      }
    }
      (yyval.node) = make_range_node(F_NN_RANGE, (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[-1].node));
    }
#line 5211 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 208: /* expr4: expr4 '[' '<' comma_expr L_RANGE comma_expr ']'  */
#line 2572 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = make_range_node(F_RN_RANGE, (yyvsp[-6].node), (yyvsp[-3].node), (yyvsp[-1].node));
    }
#line 5219 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 209: /* expr4: expr4 '[' '<' comma_expr L_RANGE '<' comma_expr ']'  */
#line 2576 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-1].node)->kind == NODE_NUMBER && (yyvsp[-1].node)->v.number <= 1)
        (yyval.node) = make_range_node(F_RE_RANGE, (yyvsp[-7].node), (yyvsp[-4].node), 0);
      else
        (yyval.node) = make_range_node(F_RR_RANGE, (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-1].node));
    }
#line 5230 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 210: /* expr4: expr4 '[' comma_expr L_RANGE '<' comma_expr ']'  */
#line 2583 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-1].node)->kind == NODE_NUMBER && (yyvsp[-1].node)->v.number <= 1)
        (yyval.node) = make_range_node(F_NE_RANGE, (yyvsp[-6].node), (yyvsp[-4].node), 0);
      else
        (yyval.node) = make_range_node(F_NR_RANGE, (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-1].node));
    }
#line 5241 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 211: /* expr4: expr4 '[' comma_expr L_RANGE ']'  */
#line 2590 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = make_range_node(F_NE_RANGE, (yyvsp[-4].node), (yyvsp[-2].node), 0);
    }
#line 5249 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 212: /* expr4: expr4 '[' '<' comma_expr L_RANGE ']'  */
#line 2594 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = make_range_node(F_RE_RANGE, (yyvsp[-5].node), (yyvsp[-2].node), 0);
    }
#line 5257 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 213: /* expr4: expr4 '[' '<' comma_expr ']'  */
#line 2598 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (IS_NODE((yyvsp[-4].node), NODE_CALL, F_AGGREGATE)
          && (yyvsp[-1].node)->kind == NODE_NUMBER) {
        int i = (yyvsp[-1].node)->v.number;
        if (i < 1 || i > (yyvsp[-4].node)->l.number)
          yyerror("Illegal index to array constant.");
        else {
          parse_node_t *node = (yyvsp[-4].node)->r.expr;
          i = (yyvsp[-4].node)->l.number - i;
          while (i--)
            node = node->r.expr;
          (yyval.node) = node->v.expr;
          break;
        }
      }
      CREATE_BINARY_OP((yyval.node), F_RINDEX, 0, (yyvsp[-1].node), (yyvsp[-4].node));
      if (exact_types) {
        switch((yyvsp[-4].node)->type) {
          case TYPE_MAPPING:
            yyerror("Illegal index for mapping.");
          case TYPE_ANY:
            (yyval.node)->type = TYPE_ANY;
            break;
          case TYPE_STRING:
          case TYPE_BUFFER:
            (yyval.node)->type = TYPE_NUMBER;
            if (!IS_TYPE((yyvsp[-1].node)->type,TYPE_NUMBER))
              type_error("Bad type of index", (yyvsp[-1].node)->type);
            break;

          default:
            if ((yyvsp[-4].node)->type & TYPE_MOD_ARRAY) {
              (yyval.node)->type = (yyvsp[-4].node)->type & ~TYPE_MOD_ARRAY;
              if ((yyval.node)->type != TYPE_ANY)
                (yyval.node) = add_type_check((yyval.node), (yyval.node)->type);
              if (!IS_TYPE((yyvsp[-1].node)->type,TYPE_NUMBER))
                type_error("Bad type of index", (yyvsp[-1].node)->type);
            } else {
              type_error("Value indexed has a bad type ", (yyvsp[-4].node)->type);
              (yyval.node)->type = TYPE_ANY;
            }
        }
      } else (yyval.node)->type = TYPE_ANY;
    }
#line 5306 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 214: /* expr4: expr4 '[' comma_expr ']'  */
#line 2643 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      /* Something stupid like ({ 1, 2, 3 })[1]; we take the
       * time to optimize this because people who don't understand
       * the preprocessor often write things like:
       *
       * #define MY_ARRAY ({ "foo", "bar", "bazz" })
       * ...
       * ... MY_ARRAY[1] ...
       *
       * which of course expands to the above.
       */
      if (IS_NODE((yyvsp[-3].node), NODE_CALL, F_AGGREGATE) && (yyvsp[-1].node)->kind == NODE_NUMBER) {
        int i = (yyvsp[-1].node)->v.number;
        if (i < 0 || i >= (yyvsp[-3].node)->l.number)
          yyerror("Illegal index to array constant.");
        else {
          parse_node_t *node = (yyvsp[-3].node)->r.expr;
          while (i--)
            node = node->r.expr;
          (yyval.node) = node->v.expr;
          break;
        }
      }
#ifndef OLD_RANGE_BEHAVIOR
      if ((yyvsp[-1].node)->kind == NODE_NUMBER && (yyvsp[-1].node)->v.number < 0)
        yywarn("A negative constant in arr[x] no longer means indexing from the end.  Use arr[<x]");
#endif
      CREATE_BINARY_OP((yyval.node), F_INDEX, 0, (yyvsp[-1].node), (yyvsp[-3].node));
      if (exact_types) {
        switch((yyvsp[-3].node)->type) {
          case TYPE_MAPPING:
          case TYPE_ANY:
            (yyval.node)->type = TYPE_ANY;
            break;
          case TYPE_STRING:
          case TYPE_BUFFER:
            (yyval.node)->type = TYPE_NUMBER;
            if (!IS_TYPE((yyvsp[-1].node)->type,TYPE_NUMBER))
              type_error("Bad type of index", (yyvsp[-1].node)->type);
            break;

          default:
            if ((yyvsp[-3].node)->type & TYPE_MOD_ARRAY) {
              (yyval.node)->type = (yyvsp[-3].node)->type & ~TYPE_MOD_ARRAY;
              if((yyval.node)->type != TYPE_ANY)
                (yyval.node) = add_type_check((yyval.node), (yyval.node)->type);
              if (!IS_TYPE((yyvsp[-1].node)->type,TYPE_NUMBER))
                type_error("Bad type of index", (yyvsp[-1].node)->type);
            } else {
              type_error("Value indexed has a bad type ", (yyvsp[-3].node)->type);
              (yyval.node)->type = TYPE_ANY;
            }
        }
      } else (yyval.node)->type = TYPE_ANY;
    }
#line 5366 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 216: /* expr4: '(' comma_expr ')'  */
#line 2700 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[-1].node);
    }
#line 5374 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 219: /* @14: %empty  */
#line 2706 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      auto max_local_variables = CFG_INT(__MAX_LOCAL_VARIABLES__);

      if ((yyvsp[0].number) != TYPE_FUNCTION) yyerror("Reserved type name unexpected.");
      (yyval.func_block).num_local = current_number_of_locals;
      (yyval.func_block).max_num_locals = max_num_locals;
      (yyval.func_block).context = context;
      (yyval.func_block).save_current_type = current_type;
      (yyval.func_block).save_exact_types = exact_types;
      if (type_of_locals_ptr + max_num_locals + max_local_variables >= &type_of_locals[type_of_locals_size])
        reallocate_locals();
      deactivate_current_locals();
      locals_ptr += current_number_of_locals;
      type_of_locals_ptr += max_num_locals;
      max_num_locals = current_number_of_locals = 0;
      push_function_context();
      current_function_context->num_parameters = -1;
      exact_types = TYPE_ANY;
      context = 0;
    }
#line 5399 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 220: /* expr4: L_BASIC_TYPE @14 '(' argument ')' block  */
#line 2727 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-2].argument).flags & ARG_IS_VARARGS) {
        yyerror("Anonymous varargs functions aren't implemented");
      }
      if (!(yyvsp[0].decl).node) {
        CREATE_RETURN((yyvsp[0].decl).node, 0);
      } else if ((yyvsp[0].decl).node->kind != NODE_RETURN &&
          ((yyvsp[0].decl).node->kind != NODE_TWO_VALUES || (yyvsp[0].decl).node->r.expr->kind != NODE_RETURN)) {
        parse_node_t *replacement;
        CREATE_STATEMENTS(replacement, (yyvsp[0].decl).node, 0);
        CREATE_RETURN(replacement->r.expr, 0);
        (yyvsp[0].decl).node = replacement;
      }

      (yyval.node) = new_node();
      (yyval.node)->kind = NODE_ANON_FUNC;
      (yyval.node)->type = TYPE_FUNCTION;
      (yyval.node)->l.number = (max_num_locals - (yyvsp[-2].argument).num_arg);
      (yyval.node)->r.expr = (yyvsp[0].decl).node;
      (yyval.node)->v.number = (yyvsp[-2].argument).num_arg;
      if (current_function_context->bindable)
        (yyval.node)->v.number |= 0x10000;
      free_all_local_names(1);

      current_number_of_locals = (yyvsp[-4].func_block).num_local;
      max_num_locals = (yyvsp[-4].func_block).max_num_locals;
      context = (yyvsp[-4].func_block).context;
      current_type = (yyvsp[-4].func_block).save_current_type;
      exact_types = (yyvsp[-4].func_block).save_exact_types;
      pop_function_context();

      locals_ptr -= current_number_of_locals;
      type_of_locals_ptr -= max_num_locals;
      reactivate_current_locals();
    }
#line 5439 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 221: /* expr4: l_new_function_open ':' ')'  */
#line 2763 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (CONFIG_INT(__RC_WOMBLES__)) {
        if(*(outp-2) != ':') {
          yyerror("End of functional not found");
        }
      }
      (yyval.node) = new_node();
      (yyval.node)->kind = NODE_FUNCTION_CONSTRUCTOR;
      (yyval.node)->type = TYPE_FUNCTION;
      (yyval.node)->r.expr = 0;
      switch ((yyvsp[-2].number) & 0xff) {
        case FP_L_VAR:
          yyerror("Illegal to use local variable in a functional.");
          CREATE_NUMBER((yyval.node)->l.expr, 0);
          (yyval.node)->l.expr->r.expr = 0;
          (yyval.node)->l.expr->l.expr = 0;
          (yyval.node)->v.number = FP_FUNCTIONAL;
          break;
        case FP_G_VAR:
          CREATE_OPCODE_1((yyval.node)->l.expr, F_GLOBAL, 0, (yyvsp[-2].number) >> 8);
          (yyval.node)->v.number = FP_FUNCTIONAL | FP_NOT_BINDABLE;
          if (VAR_TEMP((yyval.node)->l.expr->l.number)->type & DECL_HIDDEN) {
            char buf[256];
            char *end = EndOf(buf);
            char *p;

            p = strput(buf, end, "Illegal to use private variable '");
            p = strput(p, end, VAR_TEMP((yyval.node)->l.expr->l.number)->name);
            p = strput(p, end, "'");
            yyerror(buf);
          }
          break;
        default:
          (yyval.node)->v.number = (yyvsp[-2].number);
          break;
      }
    }
#line 5481 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 222: /* expr4: l_new_function_open ',' expr_list2 ':' ')'  */
#line 2801 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (CONFIG_INT(__RC_WOMBLES__)) {
        if(*(outp-2) != ':') {
          yyerror("End of functional not found");
        }
      }
      (yyval.node) = new_node();
      (yyval.node)->kind = NODE_FUNCTION_CONSTRUCTOR;
      (yyval.node)->type = TYPE_FUNCTION;
      (yyval.node)->v.number = (yyvsp[-4].number);
      (yyval.node)->r.expr = (yyvsp[-2].node);

      switch ((yyvsp[-4].number) & 0xff) {
        case FP_EFUN: {
                        int *argp;
                        int f = (yyvsp[-4].number) >>8;
                        int num = (yyvsp[-2].node)->kind;
                        int max_arg = predefs[f].max_args;
                        if(f!=-1){
                          if (num > max_arg && max_arg != -1) {
                            parse_node_t *pn = (yyvsp[-2].node);

                            while (pn) {
                              if (pn->type & 1) break;
                              pn = pn->r.expr;
                            }

                            if (!pn) {
                              char bff[256];
                              char *end = EndOf(bff);
                              char *p;

                              p = strput(bff, end, "Too many arguments to ");
                              p = strput(p, end, predefs[f].word);
                              yyerror(bff);
                            }
                          } else if (max_arg != -1 && exact_types) {
                            /*
                             * Now check all types of arguments to efuns.
                             */
                            int i, argn, tmp;
                            parse_node_t *enode = (yyvsp[-2].node);
                            argp = &efun_arg_types[predefs[f].arg_index];

                            for (argn = 0; argn < num; argn++) {
                              if (enode->type & 1) break;

                              tmp = enode->v.expr->type;
                              for (i=0; !compatible_types(tmp, argp[i])
                                  && argp[i] != 0; i++)
                                ;
                              if (argp[i] == 0) {
                                char buf[256];
                                char *end = EndOf(buf);
                                char *p;

                                p = strput(buf, end, "Bad argument ");
                                p = strput_int(p, end, argn+1);
                                p = strput(p, end, " to efun ");
                                p = strput(p, end, predefs[f].word);
                                p = strput(p, end, "()");
                                yyerror(buf);
                              } else {
                                /* this little section necessary b/c in the
                                   case float | int we dont want to do
                                   promoting. */
                                if (tmp == TYPE_NUMBER && argp[i] == TYPE_REAL) {
                                  for (i++; argp[i] && argp[i] != TYPE_NUMBER; i++)
                                    ;
                                  if (!argp[i])
                                    enode->v.expr = promote_to_float(enode->v.expr);
                                }
                                if (tmp == TYPE_REAL && argp[i] == TYPE_NUMBER) {
                                  for (i++; argp[i] && argp[i] != TYPE_REAL; i++)
                                    ;
                                  if (!argp[i])
                                    enode->v.expr = promote_to_int(enode->v.expr);
                                }
                              }
                              while (argp[i] != 0)
                                i++;
                              argp += i + 1;
                              enode = enode->r.expr;
                            }
                          }
                        }
                        break;
                      }
        case FP_L_VAR:
        case FP_G_VAR:
                      yyerror("Can't give parameters to functional.");
                      break;
      }
    }
#line 5580 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 223: /* expr4: L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 2896 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (CONFIG_INT(__RC_WOMBLES__)) {
        if(*(outp-2) != ':') {
          yyerror("End of functional not found");
        }
      }
      if (current_function_context->num_locals)
        yyerror("Illegal to use local variable in functional.");
      if (current_function_context->values_list->r.expr)
        current_function_context->values_list->r.expr->kind = current_function_context->values_list->kind;

      (yyval.node) = new_node();
      (yyval.node)->kind = NODE_FUNCTION_CONSTRUCTOR;
      (yyval.node)->type = TYPE_FUNCTION;
      (yyval.node)->l.expr = (yyvsp[-2].node);
      if ((yyvsp[-2].node)->kind == NODE_STRING)
        yywarn("Function pointer returning string constant is NOT a function call");
      (yyval.node)->r.expr = current_function_context->values_list->r.expr;
      (yyval.node)->v.number = FP_FUNCTIONAL + current_function_context->bindable
        + (current_function_context->num_parameters << 8);
      pop_function_context();
    }
#line 5607 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 224: /* expr4: L_MAPPING_OPEN expr_list3 ']' ')'  */
#line 2919 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (CONFIG_INT(__RC_WOMBLES__)) {
        if(*(outp-2) != ']') {
          yyerror("End of mapping not found");
        }
      }
      CREATE_CALL((yyval.node), F_AGGREGATE_ASSOC, TYPE_MAPPING, (yyvsp[-2].node));
    }
#line 5620 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 225: /* expr4: L_ARRAY_OPEN expr_list '}' ')'  */
#line 2928 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      if (CONFIG_INT(__RC_WOMBLES__)) {
        if(*(outp-2) != '}') {
          yyerror("End of array not found");
        }
      }
      CREATE_CALL((yyval.node), F_AGGREGATE, TYPE_ANY | TYPE_MOD_ARRAY, (yyvsp[-2].node));
    }
#line 5633 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 226: /* expr_or_block: block  */
#line 2940 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[0].decl).node;
    }
#line 5641 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 227: /* expr_or_block: '(' comma_expr ')'  */
#line 2944 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = insert_pop_value((yyvsp[-1].node));
    }
#line 5649 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 228: /* @15: %empty  */
#line 2951 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      context = SPECIAL_CONTEXT;
    }
#line 5658 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 229: /* catch: L_CATCH @15 expr_or_block  */
#line 2956 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_CATCH((yyval.node), (yyvsp[0].node));
      context = (yyvsp[-1].number);
    }
#line 5667 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 230: /* tree: L_TREE block  */
#line 2964 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
#ifdef DEBUG
      (yyval.node) = new_node_no_line();
      lpc_tree_form((yyvsp[0].decl).node, (yyval.node));
#endif
    }
#line 5678 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 231: /* tree: L_TREE '(' comma_expr ')'  */
#line 2971 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
#ifdef DEBUG
      (yyval.node) = new_node_no_line();
      lpc_tree_form((yyvsp[-1].node), (yyval.node));
#endif
    }
#line 5689 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 232: /* sscanf: L_SSCANF '(' expr0 ',' expr0 lvalue_list ')'  */
#line 2981 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      int p = (yyvsp[-1].node)->v.number;
      CREATE_LVALUE_EFUN((yyval.node), TYPE_NUMBER, (yyvsp[-1].node));
      CREATE_BINARY_OP_1((yyval.node)->l.expr, F_SSCANF, 0, (yyvsp[-4].node), (yyvsp[-2].node), p);
    }
#line 5699 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 233: /* parse_command: L_PARSE_COMMAND '(' expr0 ',' expr0 ',' expr0 lvalue_list ')'  */
#line 2990 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      int p = (yyvsp[-1].node)->v.number;
      CREATE_LVALUE_EFUN((yyval.node), TYPE_NUMBER, (yyvsp[-1].node));
      CREATE_TERNARY_OP_1((yyval.node)->l.expr, F_PARSE_COMMAND, 0,
          (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-2].node), p);
    }
#line 5710 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 234: /* @16: %empty  */
#line 3000 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      context = SPECIAL_CONTEXT;
    }
#line 5719 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 235: /* time_expression: L_TIME_EXPRESSION @16 expr_or_block  */
#line 3005 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_TIME_EXPRESSION((yyval.node), (yyvsp[0].node));
      context = (yyvsp[-1].number);
    }
#line 5728 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 236: /* lvalue_list: %empty  */
#line 3013 "/projects/git/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.node) = new_node_no_line();
      (yyval.node)->r.expr = 0;
      (yyval.node)->v.number = 0;
    }
#line 5738 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 237: /* lvalue_list: ',' lvalue lvalue_list  */
#line 3019 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      parse_node_t *insert;

      (yyval.node) = (yyvsp[0].node);
      insert = new_node_no_line();
      insert->r.expr = (yyvsp[0].node)->r.expr;
      insert->l.expr = (yyvsp[-1].node);
      (yyvsp[0].node)->r.expr = insert;
      (yyval.node)->v.number++;
    }
#line 5753 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 238: /* string: string_con2  */
#line 3033 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_STRING((yyval.node), (yyvsp[0].string));
      scratch_free((yyvsp[0].string));
    }
#line 5762 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 240: /* string_con1: '(' string_con1 ')'  */
#line 3042 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.string) = (yyvsp[-1].string);
    }
#line 5770 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 241: /* string_con1: string_con1 '+' string_con1  */
#line 3046 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.string) = scratch_join((yyvsp[-2].string), (yyvsp[0].string));
    }
#line 5778 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 243: /* string_con2: string_con2 L_STRING  */
#line 3054 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.string) = scratch_join((yyvsp[-1].string), (yyvsp[0].string));
    }
#line 5786 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 244: /* template_literal: L_TEMPLATE_HEAD expr0 template_parts  */
#line 3061 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      parse_node_t *head, *tmp, *coerced;
      CREATE_STRING(head, (yyvsp[-2].string));
      scratch_free((yyvsp[-2].string));
      CREATE_UNARY_OP(coerced, F_TEMPLATE_COERCE, TYPE_STRING, (yyvsp[-1].node));
      CREATE_BINARY_OP(tmp, F_ADD, TYPE_STRING, head, coerced);
      CREATE_BINARY_OP((yyval.node), F_ADD, TYPE_STRING, tmp, (yyvsp[0].node));
    }
#line 5799 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 245: /* template_parts: L_TEMPLATE_TAIL  */
#line 3073 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_STRING((yyval.node), (yyvsp[0].string));
      scratch_free((yyvsp[0].string));
    }
#line 5808 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 246: /* template_parts: L_TEMPLATE_MIDDLE expr0 template_parts  */
#line 3078 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      parse_node_t *mid, *tmp, *coerced;
      CREATE_STRING(mid, (yyvsp[-2].string));
      scratch_free((yyvsp[-2].string));
      CREATE_UNARY_OP(coerced, F_TEMPLATE_COERCE, TYPE_STRING, (yyvsp[-1].node));
      CREATE_BINARY_OP(tmp, F_ADD, TYPE_STRING, mid, coerced);
      CREATE_BINARY_OP((yyval.node), F_ADD, TYPE_STRING, tmp, (yyvsp[0].node));
    }
#line 5821 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 249: /* string_like: string_like string  */
#line 3092 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_BINARY_OP((yyval.node), F_ADD, TYPE_STRING, (yyvsp[-1].node), (yyvsp[0].node));
    }
#line 5829 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 250: /* string_like: string_like template_literal  */
#line 3096 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_BINARY_OP((yyval.node), F_ADD, TYPE_STRING, (yyvsp[-1].node), (yyvsp[0].node));
    }
#line 5837 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 251: /* class_init: identifier ':' expr0  */
#line 3103 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = new_node();
      (yyval.node)->l.expr = (parse_node_t *)(yyvsp[-2].string);
      (yyval.node)->v.expr = (yyvsp[0].node);
      (yyval.node)->r.expr = 0;
    }
#line 5848 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 252: /* opt_class_init: %empty  */
#line 3113 "/projects/git/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.node) = 0;
    }
#line 5856 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 253: /* opt_class_init: opt_class_init ',' class_init  */
#line 3117 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->r.expr = (yyvsp[-2].node);
    }
#line 5865 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 254: /* @17: %empty  */
#line 3125 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      (yyvsp[0].number) = num_refs;
      context |= ARG_LIST;
    }
#line 5875 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 255: /* function_call: efun_override '(' @17 expr_list ')'  */
#line 3131 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      context = (yyvsp[-2].number);
      (yyval.node) = validate_efun_call((yyvsp[-4].number),(yyvsp[-1].node));
      (yyval.node) = check_refs(num_refs - (yyvsp[-3].number), (yyvsp[-1].node), (yyval.node));
      num_refs = (yyvsp[-3].number);
    }
#line 5886 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 256: /* @18: %empty  */
#line 3138 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      (yyvsp[0].number) = num_refs;
      context |= ARG_LIST;
    }
#line 5896 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 257: /* function_call: L_NEW '(' @18 expr_list ')'  */
#line 3144 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      ident_hash_elem_t *ihe;
      int f;

      context = (yyvsp[-2].number);
      ihe = lookup_ident("clone_object");

      if ((f = ihe->dn.simul_num) != -1) {
        (yyval.node) = (yyvsp[-1].node);
        (yyval.node)->kind = NODE_CALL_1;
        (yyval.node)->v.number = F_SIMUL_EFUN;
        (yyval.node)->l.number = f;
        (yyval.node)->type = (SIMUL(f)->type) & ~DECL_MODS;
      } else {
        (yyval.node) = validate_efun_call(lookup_predef("clone_object"), (yyvsp[-1].node));
#ifdef CAST_CALL_OTHERS
        (yyval.node)->type = TYPE_UNKNOWN;
#else
        (yyval.node)->type = TYPE_ANY;
#endif
      }
      (yyval.node) = check_refs(num_refs - (yyvsp[-3].number), (yyvsp[-1].node), (yyval.node));
      num_refs = (yyvsp[-3].number);
    }
#line 5925 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 258: /* function_call: L_NEW '(' L_CLASS L_DEFINED_NAME opt_class_init ')'  */
#line 3169 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      parse_node_t *node;

      if ((yyvsp[-2].ihe)->dn.class_num == -1) {
        char buf[256];
        char *end = EndOf(buf);
        char *p;

        p = strput(buf, end, "Undefined class '");
        p = strput(p, end, (yyvsp[-2].ihe)->name);
        p = strput(p, end, "'");
        yyerror(buf);
        CREATE_ERROR((yyval.node));
        node = (yyvsp[-1].node);
        while (node) {
          scratch_free((char *)node->l.expr);
          node = node->r.expr;
        }
      } else {
        int type = (yyvsp[-2].ihe)->dn.class_num | TYPE_MOD_CLASS;

        if ((node = (yyvsp[-1].node))) {
          CREATE_TWO_VALUES((yyval.node), type, 0, 0);
          (yyval.node)->l.expr = reorder_class_values((yyvsp[-2].ihe)->dn.class_num,
              node);
          CREATE_OPCODE_1((yyval.node)->r.expr, F_NEW_CLASS,
              type, (yyvsp[-2].ihe)->dn.class_num);

        } else {
          CREATE_OPCODE_1((yyval.node), F_NEW_EMPTY_CLASS,
              type, (yyvsp[-2].ihe)->dn.class_num);
        }
      }
    }
#line 5964 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 259: /* function_call: L_NEW '(' L_CLASS L_IDENTIFIER opt_class_init ')'  */
#line 3204 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      parse_node_t *node;
      char buf[256];
      char *end = EndOf(buf);
      char *p;

      p = strput(buf, end, "Undefined class '");
      p = strput(p, end, (yyvsp[-2].string));
      p = strput(p, end, "'");
      yyerror(buf);
      CREATE_ERROR((yyval.node));
      node = (yyvsp[-1].node);
      while (node) {
        scratch_free((char *)node->l.expr);
        node = node->r.expr;
      }
    }
#line 5986 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 260: /* @19: %empty  */
#line 3222 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      (yyvsp[0].number) = num_refs;
      context |= ARG_LIST;
    }
#line 5996 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 261: /* function_call: L_DEFINED_NAME '(' @19 expr_list ')'  */
#line 3228 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      int f;
      int i;

      context = (yyvsp[-2].number);
      (yyval.node) = (yyvsp[-1].node);
      if ((f = (yyvsp[-4].ihe)->dn.function_num) != -1) {
        if (current_function_context)
          current_function_context->bindable = FP_NOT_BINDABLE;

        (yyval.node)->kind = NODE_CALL_1;
        (yyval.node)->v.number = F_CALL_FUNCTION_BY_ADDRESS;
        (yyval.node)->l.number = f;
        (yyval.node)->type = validate_function_call(f, (yyvsp[-1].node)->r.expr);
      } else if ((f=(yyvsp[-4].ihe)->dn.simul_num) != -1) {
        (yyval.node)->kind = NODE_CALL_1;
        (yyval.node)->v.number = F_SIMUL_EFUN;
        (yyval.node)->l.number = f;
        (yyval.node)->type = (SIMUL(f)->type) & ~DECL_MODS;
      } else if ((f=(yyvsp[-4].ihe)->dn.efun_num) != -1) {
        (yyval.node) = validate_efun_call(f, (yyvsp[-1].node));
      } else if ((i = (yyvsp[-4].ihe)->dn.local_num) != -1 && 
                 ((type_of_locals_ptr[i] & ~LOCAL_MODS) == TYPE_FUNCTION ||
                  (type_of_locals_ptr[i] & ~LOCAL_MODS) == TYPE_ANY ||
                  (type_of_locals_ptr[i] & ~LOCAL_MODS) == TYPE_UNKNOWN)) {
        /* Local variable that may hold a function pointer - generate evaluate() call */
        parse_node_t *expr;
        parse_node_t *func_node;
        int local_type = type_of_locals_ptr[i] & ~LOCAL_MODS;
        
        type_of_locals_ptr[i] &= ~LOCAL_MOD_UNUSED;
        
        /* Create node to load the function variable */
        if (type_of_locals_ptr[i] & LOCAL_MOD_REF)
          CREATE_OPCODE_1(func_node, F_REF, local_type, i & 0xff);
        else
          CREATE_OPCODE_1(func_node, F_LOCAL, local_type, i & 0xff);
        
        /* Generate evaluate(func_var, args...) */
        (yyval.node)->kind = NODE_EFUN;
        (yyval.node)->l.number = (yyval.node)->v.number + 1;
        (yyval.node)->v.number = predefs[evaluate_efun].token;
#ifdef CAST_CALL_OTHERS
        (yyval.node)->type = TYPE_UNKNOWN;
#else
        (yyval.node)->type = TYPE_ANY;
#endif
        expr = new_node_no_line();
        expr->type = 0;
        expr->v.expr = func_node;
        expr->r.expr = (yyval.node)->r.expr;
        (yyval.node)->r.expr = expr;
        
        if (current_function_context)
          current_function_context->num_locals++;
      } else if ((i = (yyvsp[-4].ihe)->dn.global_num) != -1 && 
                 ((VAR_TEMP(i)->type & ~DECL_MODS) == TYPE_FUNCTION ||
                  (VAR_TEMP(i)->type & ~DECL_MODS) == TYPE_ANY ||
                  (VAR_TEMP(i)->type & ~DECL_MODS) == TYPE_UNKNOWN)) {
        /* Global variable that may hold a function pointer - generate evaluate() call */
        parse_node_t *expr;
        parse_node_t *func_node;
        int global_type = VAR_TEMP(i)->type & ~DECL_MODS;
        
        if (current_function_context)
          current_function_context->bindable = FP_NOT_BINDABLE;
        
        /* Create node to load the function variable */
        CREATE_OPCODE_1(func_node, F_GLOBAL, global_type, i);
        
        if (VAR_TEMP(i)->type & DECL_HIDDEN) {
          char buf[256];
          char *end = EndOf(buf);
          char *p;

          p = strput(buf, end, "Illegal to use private variable '");
          p = strput(p, end, (yyvsp[-4].ihe)->name);
          p = strput(p, end, "'");
          yyerror(buf);
        }
        
        /* Generate evaluate(func_var, args...) */
        (yyval.node)->kind = NODE_EFUN;
        (yyval.node)->l.number = (yyval.node)->v.number + 1;
        (yyval.node)->v.number = predefs[evaluate_efun].token;
#ifdef CAST_CALL_OTHERS
        (yyval.node)->type = TYPE_UNKNOWN;
#else
        (yyval.node)->type = TYPE_ANY;
#endif
        expr = new_node_no_line();
        expr->type = 0;
        expr->v.expr = func_node;
        expr->r.expr = (yyval.node)->r.expr;
        (yyval.node)->r.expr = expr;
      } else {
        /* This here is a really nasty case that only occurs with
         * exact_types off.  The user has done something gross like:
         *
         * func() { int f; f(); } // if f was prototyped we wouldn't
         * f() { }                // need this case
         */
        if (exact_types) {
          char buf[256];
          char *end = EndOf(buf);
          char *p;
          const char *n = (yyvsp[-4].ihe)->name;
          if (*n == ':') n++;
          p = strput(buf, end, "Undefined function ");
          p = strput(p, end, n);
          yyerror(buf);
        } else {
          /*
           * Don't complain, just grok it.
           */
          if (current_function_context)
            current_function_context->bindable = FP_NOT_BINDABLE;

          f = define_new_function((yyvsp[-4].ihe)->name, 0, 0, DECL_PUBLIC|FUNC_UNDEFINED, TYPE_ANY);
          (yyval.node)->kind = NODE_CALL_1;
          (yyval.node)->v.number = F_CALL_FUNCTION_BY_ADDRESS;
          (yyval.node)->l.number = f;
          (yyval.node)->type = TYPE_ANY; /* just a guess */
        }
      }
      (yyval.node) = check_refs(num_refs - (yyvsp[-3].number), (yyvsp[-1].node), (yyval.node));
      num_refs = (yyvsp[-3].number);
    }
#line 6129 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 262: /* @20: %empty  */
#line 3357 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      (yyvsp[0].number) = num_refs;
      context |= ARG_LIST;
    }
#line 6139 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 263: /* function_call: function_name '(' @20 expr_list ')'  */
#line 3363 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      char *name = (yyvsp[-4].string);

      context = (yyvsp[-2].number);
      (yyval.node) = (yyvsp[-1].node);

      if (current_function_context)
        current_function_context->bindable = FP_NOT_BINDABLE;

      if (*name == ':') {
        int f;

        if ((f = arrange_call_inherited(name + 1, (yyval.node))) != -1)
          /* Can't do this; f may not be the correct function
             entry.  It might be overloaded.

             validate_function_call(f, $$->r.expr)
             */
          ;
      } else {
        /* The only way this can happen is if function_name
         * below made the function name. (directly or inherited.)
         * The lexer would return L_DEFINED_NAME otherwise.
         */
        int f;
        ident_hash_elem_t *ihe;

        f = (ihe = lookup_ident(name)) ? ihe->dn.function_num : -1;

        // Funciton is not yet defined. for exact_types case, we simply return error,
        // otherwise attempt to create a function, hoping later it will be defined by inherit.
        if (f == -1) {
          if (exact_types) {
            char buf[256];
            char *end = EndOf(buf);
            char *p;
            char *n = (yyvsp[-4].string);
            if (*n == ':') n++;
            p = strput(buf, end, "Undefined function ");
            p = strput(p, end, n);
            yyerror(buf);
          } else {
            f = define_new_function(name, 0, 0, DECL_PUBLIC|FUNC_UNDEFINED, TYPE_ANY);
          }
        }

        if (f != -1) {
          (yyval.node)->kind = NODE_CALL_1;
          (yyval.node)->v.number = F_CALL_FUNCTION_BY_ADDRESS;
          (yyval.node)->l.number = f;
          if (FUNCTION_FLAGS(f) & FUNC_UNDEFINED) {
            (yyval.node)->type = TYPE_ANY;  /* Just a guess */
          } else {
            (yyval.node)->type = validate_function_call(f, (yyvsp[-1].node)->r.expr);
          }
        }
      }
      (yyval.node) = check_refs(num_refs - (yyvsp[-3].number), (yyvsp[-1].node), (yyval.node));
      num_refs = (yyvsp[-3].number);
      scratch_free(name);
    }
#line 6205 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 264: /* @21: %empty  */
#line 3425 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      (yyvsp[0].number) = num_refs;
      context |= ARG_LIST;
    }
#line 6215 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 265: /* function_call: expr4 '[' comma_expr ']' '(' @21 expr_list ')'  */
#line 3431 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      parse_node_t *expr;
      parse_node_t *index_expr;

      context = (yyvsp[-2].number);
      (yyval.node) = (yyvsp[-1].node);

      /* Create the indexing expression */
      CREATE_BINARY_OP(index_expr, F_INDEX, 0, (yyvsp[-5].node), (yyvsp[-7].node));
      if (exact_types) {
        switch((yyvsp[-7].node)->type) {
          case TYPE_MAPPING:
          case TYPE_ANY:
            index_expr->type = TYPE_ANY;
            break;
          default:
            if ((yyvsp[-7].node)->type & TYPE_MOD_ARRAY) {
              index_expr->type = (yyvsp[-7].node)->type & ~TYPE_MOD_ARRAY;
            } else {
              index_expr->type = TYPE_ANY;
            }
            break;
        }
      } else {
        index_expr->type = TYPE_ANY;
      }

      /* Generate evaluate(indexed_expr, args...) */
      (yyval.node)->kind = NODE_EFUN;
      (yyval.node)->l.number = (yyval.node)->v.number + 1;
      (yyval.node)->v.number = predefs[evaluate_efun].token;
#ifdef CAST_CALL_OTHERS
      (yyval.node)->type = TYPE_UNKNOWN;
#else
      (yyval.node)->type = TYPE_ANY;
#endif
      expr = new_node_no_line();
      expr->type = 0;
      expr->v.expr = index_expr;
      expr->r.expr = (yyval.node)->r.expr;
      (yyval.node)->r.expr = expr;
      (yyval.node) = check_refs(num_refs - (yyvsp[-3].number), (yyvsp[-1].node), (yyval.node));
      num_refs = (yyvsp[-3].number);
    }
#line 6264 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 266: /* @22: %empty  */
#line 3476 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      (yyvsp[0].number) = num_refs;
      context |= ARG_LIST;
    }
#line 6274 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 267: /* function_call: expr4 L_ARROW identifier '(' @22 expr_list ')'  */
#line 3482 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      ident_hash_elem_t *ihe;
      int f;
      parse_node_t *pn1, *pn2;

      (yyvsp[-1].node)->v.number += 2;

      pn1 = new_node_no_line();
      pn1->type = 0;
      pn1->v.expr = (yyvsp[-6].node);
      pn1->kind = (yyvsp[-1].node)->v.number;

      pn2 = new_node_no_line();
      pn2->type = 0;
      CREATE_STRING(pn2->v.expr, (yyvsp[-4].string));
      scratch_free((yyvsp[-4].string));

      /* insert the two nodes */
      pn2->r.expr = (yyvsp[-1].node)->r.expr;
      pn1->r.expr = pn2;
      (yyvsp[-1].node)->r.expr = pn1;

      if (!(yyvsp[-1].node)->l.expr) (yyvsp[-1].node)->l.expr = pn2;

      context = (yyvsp[-2].number);
      ihe = lookup_ident("call_other");

      if ((f = ihe->dn.simul_num) != -1) {
        (yyval.node) = (yyvsp[-1].node);
        (yyval.node)->kind = NODE_CALL_1;
        (yyval.node)->v.number = F_SIMUL_EFUN;
        (yyval.node)->l.number = f;
        (yyval.node)->type = (SIMUL(f)->type) & ~DECL_MODS;
      } else {
        (yyval.node) = validate_efun_call(arrow_efun, (yyvsp[-1].node));
#ifdef CAST_CALL_OTHERS
        (yyval.node)->type = TYPE_UNKNOWN;
#else
        (yyval.node)->type = TYPE_ANY;
#endif
      }
      (yyval.node) = check_refs(num_refs - (yyvsp[-3].number), (yyvsp[-1].node), (yyval.node));
      num_refs = (yyvsp[-3].number);
    }
#line 6323 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 268: /* @23: %empty  */
#line 3527 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      (yyvsp[0].number) = num_refs;
      context |= ARG_LIST;
    }
#line 6333 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 269: /* function_call: '(' '*' comma_expr ')' '(' @23 expr_list ')'  */
#line 3533 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      parse_node_t *expr;

      context = (yyvsp[-2].number);
      (yyval.node) = (yyvsp[-1].node);
      (yyval.node)->kind = NODE_EFUN;
      (yyval.node)->l.number = (yyval.node)->v.number + 1;
      (yyval.node)->v.number = predefs[evaluate_efun].token;
#ifdef CAST_CALL_OTHERS
      (yyval.node)->type = TYPE_UNKNOWN;
#else
      (yyval.node)->type = TYPE_ANY;
#endif
      expr = new_node_no_line();
      expr->type = 0;
      expr->v.expr = (yyvsp[-5].node);
      expr->r.expr = (yyval.node)->r.expr;
      (yyval.node)->r.expr = expr;
      (yyval.node) = check_refs(num_refs - (yyvsp[-3].number), (yyvsp[-1].node), (yyval.node));
      num_refs = (yyvsp[-3].number);
    }
#line 6359 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 270: /* efun_override: L_EFUN L_COLON_COLON identifier  */
#line 3558 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      svalue_t *res;
      ident_hash_elem_t *ihe;

      (yyval.number) = (ihe = lookup_ident((yyvsp[0].string))) ? ihe->dn.efun_num : -1;
      if ((yyval.number) == -1) {
        yyerror("Unknown efun: %s", (yyvsp[0].string));
      } else {
        push_malloced_string(the_file_name(current_file));
        share_and_push_string((yyvsp[0].string));
        push_malloced_string(add_slash(main_file_name()));
        res = safe_apply_master_ob(APPLY_VALID_OVERRIDE, 3);
        if (!MASTER_APPROVED(res)) {
          yyerror("Invalid simulated efunction override");
          (yyval.number) = -1;
        }
      }
      scratch_free((yyvsp[0].string));
    }
#line 6383 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 271: /* efun_override: L_EFUN L_COLON_COLON L_NEW  */
#line 3578 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      svalue_t *res;

      push_malloced_string(the_file_name(current_file));
      push_constant_string("new");
      push_malloced_string(add_slash(main_file_name()));
      res = safe_apply_master_ob(APPLY_VALID_OVERRIDE, 3);
      if (!MASTER_APPROVED(res)) {
        yyerror("Invalid simulated efunction override");
        (yyval.number) = -1;
      } else (yyval.number) = new_efun;
    }
#line 6400 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 273: /* function_name: L_COLON_COLON identifier  */
#line 3595 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      int l = strlen((yyvsp[0].string)) + 1;
      char *p;
      /* here we be a bit cute.  we put a : on the front so we
       * don't have to strchr for it.  Here we do:
       * "name" -> ":::name"
       */
      (yyval.string) = scratch_realloc((yyvsp[0].string), l + 3);
      p = (yyval.string) + l;
      while (p--,l--)
        *(p+3) = *p;
      strncpy((yyval.string), ":::", 3);
    }
#line 6418 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 274: /* function_name: L_BASIC_TYPE L_COLON_COLON identifier  */
#line 3609 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      int z, l = strlen((yyvsp[0].string)) + 1;
      char *p;
      /* <type> and "name" -> ":type::name" */
      z = strlen(compiler_type_names[(yyvsp[-2].number)]) + 3; /* length of :type:: */
      (yyval.string) = scratch_realloc((yyvsp[0].string), l + z);
      p = (yyval.string) + l;
      while (p--,l--)
        *(p+z) = *p;
      (yyval.string)[0] = ':';
      strncpy((yyval.string) + 1, compiler_type_names[(yyvsp[-2].number)], z - 3);
      (yyval.string)[z-2] = ':';
      (yyval.string)[z-1] = ':';
    }
#line 6437 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 275: /* function_name: identifier L_COLON_COLON identifier  */
#line 3624 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      int l = strlen((yyvsp[-2].string));
      /* "ob" and "name" -> ":ob::name" */
      (yyval.string) = scratch_alloc(l + strlen((yyvsp[0].string)) + 4);
      *((yyval.string)) = ':';
      strcpy((yyval.string) + 1, (yyvsp[-2].string));
      strcpy((yyval.string) + l + 1, "::");
      strcpy((yyval.string) + l + 3, (yyvsp[0].string));
      scratch_free((yyvsp[-2].string));
      scratch_free((yyvsp[0].string));
    }
#line 6453 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 276: /* cond: L_IF '(' comma_expr ')' statement optional_else_part  */
#line 3639 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      /* x != 0 -> x */
      if (IS_NODE((yyvsp[-3].node), NODE_BINARY_OP, F_NE)) {
        if (IS_NODE((yyvsp[-3].node)->r.expr, NODE_NUMBER, 0))
          (yyvsp[-3].node) = (yyvsp[-3].node)->l.expr;
        else if (IS_NODE((yyvsp[-3].node)->l.expr, NODE_NUMBER, 0))
          (yyvsp[-3].node) = (yyvsp[-3].node)->r.expr;
      }

      /* TODO: should optimize if (0), if (1) here.
       * Also generalize this.
       */

      if ((yyvsp[-1].node) == 0) {
        if ((yyvsp[0].node) == 0) {
          /* if (x) ; -> x; */
          (yyval.node) = pop_value((yyvsp[-3].node));
          break;
        } else {
          /* if (x) {} else y; -> if (!x) y; */
          parse_node_t *repl;

          CREATE_UNARY_OP(repl, F_NOT, TYPE_NUMBER, (yyvsp[-3].node));
          (yyvsp[-3].node) = repl;
          (yyvsp[-1].node) = (yyvsp[0].node);
          (yyvsp[0].node) = 0;
        }
      }
      CREATE_IF((yyval.node), (yyvsp[-3].node), (yyvsp[-1].node), (yyvsp[0].node));
    }
#line 6488 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 277: /* optional_else_part: %empty  */
#line 3673 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = 0;
    }
#line 6496 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;

  case 278: /* optional_else_part: L_ELSE statement  */
#line 3677 "/projects/git/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 6504 "/projects/git/fluffos/build/src/grammar.autogen.cc"
    break;


#line 6508 "/projects/git/fluffos/build/src/grammar.autogen.cc"

        default: break;
      }
    if (yychar_backup != yychar)
      YY_LAC_DISCARD ("yychar change");
  }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yyesa, &yyes, &yyes_capacity, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        if (yychar != YYEMPTY)
          YY_LAC_ESTABLISH;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  /* If the stack popping above didn't lose the initial context for the
     current lookahead token, the shift below will for sure.  */
  YY_LAC_DISCARD ("error recovery");

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yyes != yyesa)
    YYSTACK_FREE (yyes);
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 3681 "/projects/git/fluffos/src/compiler/internal/grammar.y"

