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
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 1

/* Pull parsers.  */
#define YYPULL 0




/* First part of user prologue.  */
#line 33 "$REPO_ROOT$/src/compiler/internal/grammar.y"

#include "base/std.h"
#include "compiler/internal/grammar_rules.h"

extern char *outp;

/*
 * LPC grammar — parse-tree generator.
 *
 * Rules are ordered top-down: the entry point and high-level structure
 * appear first; details and primitives are defined further below.
 * All bison rules may forward-reference nonterminals defined later.
 *
 * `context` tracks nesting state used for semantic error checking:
 *   SWITCH_CONTEXT, LOOP_CONTEXT, SPECIAL_CONTEXT, SWITCH_STRINGS,
 *   SWITCH_NUMBERS, SWITCH_RANGES, SWITCH_DEFAULT.
 */
int context;
int num_refs;
int func_present;

void yyerror(void *yyscanner, const char *msg);

#line 95 "$REPO_ROOT$/build/src/grammar.autogen.cc"

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
  YYSYMBOL_L_TEMPLATE_HEAD = 6,            /* L_TEMPLATE_HEAD  */
  YYSYMBOL_L_TEMPLATE_MIDDLE = 7,          /* L_TEMPLATE_MIDDLE  */
  YYSYMBOL_L_TEMPLATE_TAIL = 8,            /* L_TEMPLATE_TAIL  */
  YYSYMBOL_L_BASIC_TYPE = 9,               /* L_BASIC_TYPE  */
  YYSYMBOL_L_TYPE_MODIFIER = 10,           /* L_TYPE_MODIFIER  */
  YYSYMBOL_L_DEFINED_NAME = 11,            /* L_DEFINED_NAME  */
  YYSYMBOL_L_IDENTIFIER = 12,              /* L_IDENTIFIER  */
  YYSYMBOL_L_EFUN = 13,                    /* L_EFUN  */
  YYSYMBOL_L_INC_DEC = 14,                 /* L_INC_DEC  */
  YYSYMBOL_L_ASSIGN = 15,                  /* L_ASSIGN  */
  YYSYMBOL_L_LAND = 16,                    /* L_LAND  */
  YYSYMBOL_L_LOR = 17,                     /* L_LOR  */
  YYSYMBOL_L_QUESTION_QUESTION = 18,       /* L_QUESTION_QUESTION  */
  YYSYMBOL_L_SHIFT = 19,                   /* L_SHIFT  */
  YYSYMBOL_L_EQ_NE = 20,                   /* L_EQ_NE  */
  YYSYMBOL_L_ORDER = 21,                   /* L_ORDER  */
  YYSYMBOL_L_IF = 22,                      /* L_IF  */
  YYSYMBOL_L_ELSE = 23,                    /* L_ELSE  */
  YYSYMBOL_L_SWITCH = 24,                  /* L_SWITCH  */
  YYSYMBOL_L_CASE = 25,                    /* L_CASE  */
  YYSYMBOL_L_DEFAULT = 26,                 /* L_DEFAULT  */
  YYSYMBOL_L_RANGE = 27,                   /* L_RANGE  */
  YYSYMBOL_L_DOT_DOT_DOT = 28,             /* L_DOT_DOT_DOT  */
  YYSYMBOL_L_WHILE = 29,                   /* L_WHILE  */
  YYSYMBOL_L_DO = 30,                      /* L_DO  */
  YYSYMBOL_L_FOR = 31,                     /* L_FOR  */
  YYSYMBOL_L_FOREACH = 32,                 /* L_FOREACH  */
  YYSYMBOL_L_IN = 33,                      /* L_IN  */
  YYSYMBOL_L_BREAK = 34,                   /* L_BREAK  */
  YYSYMBOL_L_CONTINUE = 35,                /* L_CONTINUE  */
  YYSYMBOL_L_RETURN = 36,                  /* L_RETURN  */
  YYSYMBOL_L_ARROW = 37,                   /* L_ARROW  */
  YYSYMBOL_L_INHERIT = 38,                 /* L_INHERIT  */
  YYSYMBOL_L_COLON_COLON = 39,             /* L_COLON_COLON  */
  YYSYMBOL_L_OPTIONAL_DOT = 40,            /* L_OPTIONAL_DOT  */
  YYSYMBOL_L_DOT_OPTIONAL = 41,            /* L_DOT_OPTIONAL  */
  YYSYMBOL_L_FUNCTION_OPEN = 42,           /* L_FUNCTION_OPEN  */
  YYSYMBOL_L_SSCANF = 43,                  /* L_SSCANF  */
  YYSYMBOL_L_CATCH = 44,                   /* L_CATCH  */
  YYSYMBOL_L_ARRAY = 45,                   /* L_ARRAY  */
  YYSYMBOL_L_REF = 46,                     /* L_REF  */
  YYSYMBOL_L_PARSE_COMMAND = 47,           /* L_PARSE_COMMAND  */
  YYSYMBOL_L_TIME_EXPRESSION = 48,         /* L_TIME_EXPRESSION  */
  YYSYMBOL_L_CLASS = 49,                   /* L_CLASS  */
  YYSYMBOL_L_NEW = 50,                     /* L_NEW  */
  YYSYMBOL_L_PARAMETER = 51,               /* L_PARAMETER  */
  YYSYMBOL_L_TREE = 52,                    /* L_TREE  */
  YYSYMBOL_LOWER_THAN_ELSE = 53,           /* LOWER_THAN_ELSE  */
  YYSYMBOL_54_ = 54,                       /* '?'  */
  YYSYMBOL_55_ = 55,                       /* '|'  */
  YYSYMBOL_56_ = 56,                       /* '^'  */
  YYSYMBOL_57_ = 57,                       /* '&'  */
  YYSYMBOL_58_ = 58,                       /* '<'  */
  YYSYMBOL_59_ = 59,                       /* '+'  */
  YYSYMBOL_60_ = 60,                       /* '-'  */
  YYSYMBOL_61_ = 61,                       /* '*'  */
  YYSYMBOL_62_ = 62,                       /* '%'  */
  YYSYMBOL_63_ = 63,                       /* '/'  */
  YYSYMBOL_64_ = 64,                       /* '!'  */
  YYSYMBOL_65_ = 65,                       /* '~'  */
  YYSYMBOL_66_ = 66,                       /* ';'  */
  YYSYMBOL_67_ = 67,                       /* '('  */
  YYSYMBOL_68_ = 68,                       /* ')'  */
  YYSYMBOL_69_ = 69,                       /* '{'  */
  YYSYMBOL_70_ = 70,                       /* '}'  */
  YYSYMBOL_71_ = 71,                       /* ','  */
  YYSYMBOL_72_ = 72,                       /* ':'  */
  YYSYMBOL_73_ = 73,                       /* '$'  */
  YYSYMBOL_74_ = 74,                       /* '.'  */
  YYSYMBOL_75_ = 75,                       /* '['  */
  YYSYMBOL_76_ = 76,                       /* ']'  */
  YYSYMBOL_YYACCEPT = 77,                  /* $accept  */
  YYSYMBOL_loop_start = 78,                /* loop_start  */
  YYSYMBOL_foreach_start = 79,             /* foreach_start  */
  YYSYMBOL_block_start = 80,               /* block_start  */
  YYSYMBOL_special_context_start = 81,     /* special_context_start  */
  YYSYMBOL_dollar_start = 82,              /* dollar_start  */
  YYSYMBOL_all = 83,                       /* all  */
  YYSYMBOL_program = 84,                   /* program  */
  YYSYMBOL_opt_semicolon = 85,             /* opt_semicolon  */
  YYSYMBOL_def = 86,                       /* def  */
  YYSYMBOL_function = 87,                  /* function  */
  YYSYMBOL_88_1 = 88,                      /* @1  */
  YYSYMBOL_89_2 = 89,                      /* @2  */
  YYSYMBOL_block_or_semi = 90,             /* block_or_semi  */
  YYSYMBOL_statement = 91,                 /* statement  */
  YYSYMBOL_stmt_return = 92,               /* stmt_return  */
  YYSYMBOL_stmt_cond = 93,                 /* stmt_cond  */
  YYSYMBOL_optional_else_part = 94,        /* optional_else_part  */
  YYSYMBOL_stmt_while = 95,                /* stmt_while  */
  YYSYMBOL_stmt_do = 96,                   /* stmt_do  */
  YYSYMBOL_stmt_for = 97,                  /* stmt_for  */
  YYSYMBOL_stmt_foreach = 98,              /* stmt_foreach  */
  YYSYMBOL_stmt_switch = 99,               /* stmt_switch  */
  YYSYMBOL_block = 100,                    /* block  */
  YYSYMBOL_statements = 101,               /* statements  */
  YYSYMBOL_local_decl_statement_header = 102, /* local_decl_statement_header  */
  YYSYMBOL_local_declaration_statement = 103, /* local_declaration_statement  */
  YYSYMBOL_local_decl_header = 104,        /* local_decl_header  */
  YYSYMBOL_local_declarations = 105,       /* local_declarations  */
  YYSYMBOL_comma_expr = 106,               /* comma_expr  */
  YYSYMBOL_expr = 107,                     /* expr  */
  YYSYMBOL_lvalue = 108,                   /* lvalue  */
  YYSYMBOL_primary_expr = 109,             /* primary_expr  */
  YYSYMBOL_call_open = 110,                /* call_open  */
  YYSYMBOL_function_call = 111,            /* function_call  */
  YYSYMBOL_sscanf = 112,                   /* sscanf  */
  YYSYMBOL_parse_command = 113,            /* parse_command  */
  YYSYMBOL_catch = 114,                    /* catch  */
  YYSYMBOL_time_expression = 115,          /* time_expression  */
  YYSYMBOL_tree = 116,                     /* tree  */
  YYSYMBOL_switch_header = 117,            /* switch_header  */
  YYSYMBOL_switch_block = 118,             /* switch_block  */
  YYSYMBOL_case = 119,                     /* case  */
  YYSYMBOL_case_label = 120,               /* case_label  */
  YYSYMBOL_for_expr = 121,                 /* for_expr  */
  YYSYMBOL_for_init = 122,                 /* for_init  */
  YYSYMBOL_foreach_var = 123,              /* foreach_var  */
  YYSYMBOL_foreach_vars = 124,             /* foreach_vars  */
  YYSYMBOL_type_decl = 125,                /* type_decl  */
  YYSYMBOL_class_header = 126,             /* class_header  */
  YYSYMBOL_member_list = 127,              /* member_list  */
  YYSYMBOL_128_3 = 128,                    /* $@3  */
  YYSYMBOL_member_name_list = 129,         /* member_name_list  */
  YYSYMBOL_member_name = 130,              /* member_name  */
  YYSYMBOL_class_init = 131,               /* class_init  */
  YYSYMBOL_opt_class_init = 132,           /* opt_class_init  */
  YYSYMBOL_type = 133,                     /* type  */
  YYSYMBOL_type_modifier_list = 134,       /* type_modifier_list  */
  YYSYMBOL_opt_basic_type = 135,           /* opt_basic_type  */
  YYSYMBOL_basic_type = 136,               /* basic_type  */
  YYSYMBOL_opt_atomic_type = 137,          /* opt_atomic_type  */
  YYSYMBOL_atomic_type = 138,              /* atomic_type  */
  YYSYMBOL_arg_type = 139,                 /* arg_type  */
  YYSYMBOL_cast = 140,                     /* cast  */
  YYSYMBOL_optional_star = 141,            /* optional_star  */
  YYSYMBOL_identifier = 142,               /* identifier  */
  YYSYMBOL_new_local_name = 143,           /* new_local_name  */
  YYSYMBOL_efun_override = 144,            /* efun_override  */
  YYSYMBOL_function_name = 145,            /* function_name  */
  YYSYMBOL_inheritance = 146,              /* inheritance  */
  YYSYMBOL_modifier_change = 147,          /* modifier_change  */
  YYSYMBOL_argument = 148,                 /* argument  */
  YYSYMBOL_argument_list = 149,            /* argument_list  */
  YYSYMBOL_param_decl = 150,               /* param_decl  */
  YYSYMBOL_optional_default_arg_value = 151, /* optional_default_arg_value  */
  YYSYMBOL_local_name_list = 152,          /* local_name_list  */
  YYSYMBOL_new_local_def = 153,            /* new_local_def  */
  YYSYMBOL_single_new_local_def = 154,     /* single_new_local_def  */
  YYSYMBOL_single_new_local_def_with_init = 155, /* single_new_local_def_with_init  */
  YYSYMBOL_name_list = 156,                /* name_list  */
  YYSYMBOL_new_name = 157,                 /* new_name  */
  YYSYMBOL_lambda_return_type = 158,       /* lambda_return_type  */
  YYSYMBOL_functional_open = 159,          /* functional_open  */
  YYSYMBOL_assoc_pair = 160,               /* assoc_pair  */
  YYSYMBOL_spread_expr = 161,              /* spread_expr  */
  YYSYMBOL_arg_list = 162,                 /* arg_list  */
  YYSYMBOL_opt_arg_list = 163,             /* opt_arg_list  */
  YYSYMBOL_pair_list = 164,                /* pair_list  */
  YYSYMBOL_opt_pair_list = 165,            /* opt_pair_list  */
  YYSYMBOL_lvalue_list = 166,              /* lvalue_list  */
  YYSYMBOL_expr_or_block = 167,            /* expr_or_block  */
  YYSYMBOL_ref = 168,                      /* ref  */
  YYSYMBOL_string = 169,                   /* string  */
  YYSYMBOL_template_literal = 170,         /* template_literal  */
  YYSYMBOL_template_parts = 171,           /* template_parts  */
  YYSYMBOL_string_like = 172,              /* string_like  */
  YYSYMBOL_string_literal = 173,           /* string_literal  */
  YYSYMBOL_string_const = 174,             /* string_const  */
  YYSYMBOL_number = 175,                   /* number  */
  YYSYMBOL_real = 176,                     /* real  */
  YYSYMBOL_constant = 177                  /* constant  */
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
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

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
#define YYLAST   1832

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  77
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  101
/* YYNRULES -- Number of rules.  */
#define YYNRULES  269
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  530

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   308


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
       2,     2,     2,    64,     2,     2,    73,    62,    57,     2,
      67,    68,    61,    59,    71,    60,    74,    63,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    72,    66,
      58,     2,     2,    54,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    75,     2,    76,    56,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    69,    55,    70,    65,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50,    51,    52,    53
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   233,   233,   234,   235,   236,   237,   246,   251,   252,
     257,   258,   269,   270,   271,   272,   273,   290,   292,   289,
     299,   300,   301,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   328,   329,   334,   339,   340,
     345,   351,   357,   363,   369,   381,   387,   389,   391,   393,
     399,   403,   409,   413,   415,   434,   435,   439,   440,   441,
     443,   444,   445,   446,   448,   449,   450,   452,   453,   454,
     456,   458,   465,   466,   467,   468,   470,   471,   472,   473,
     474,   476,   478,   479,   480,   481,   482,   483,   488,   497,
     498,   499,   500,   501,   502,   503,   504,   507,   511,   512,
     517,   519,   521,   526,   528,   530,   532,   534,   536,   538,
     540,   542,   545,   547,   552,   559,   561,   570,   572,   574,
     578,   579,   589,   602,   604,   606,   608,   610,   612,   614,
     616,   618,   629,   635,   641,   647,   653,   655,   666,   672,
     673,   674,   680,   682,   684,   686,   688,   695,   696,   706,
     707,   712,   714,   720,   721,   722,   727,   729,   740,   746,
     752,   753,   753,   759,   760,   765,   770,   775,   776,   788,
     793,   794,   799,   800,   805,   806,   811,   812,   817,   818,
     819,   824,   825,   830,   835,   836,   846,   847,   852,   853,
     858,   859,   869,   870,   871,   872,   877,   883,   893,   895,
     896,   902,   904,   914,   916,   918,   924,   925,   930,   932,
     938,   940,   946,   952,   963,   964,   969,   970,   981,   989,
     999,  1004,  1005,  1010,  1012,  1019,  1020,  1021,  1026,  1028,
    1035,  1036,  1037,  1042,  1043,  1048,  1049,  1054,  1064,  1076,
    1080,  1081,  1090,  1091,  1092,  1093,  1100,  1101,  1107,  1108,
    1109,  1113,  1117,  1123,  1124,  1125,  1126,  1127,  1128,  1129,
    1130,  1131,  1132,  1133,  1134,  1135,  1136,  1137,  1138,  1139
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
  "L_REAL", "L_TEMPLATE_HEAD", "L_TEMPLATE_MIDDLE", "L_TEMPLATE_TAIL",
  "L_BASIC_TYPE", "L_TYPE_MODIFIER", "L_DEFINED_NAME", "L_IDENTIFIER",
  "L_EFUN", "L_INC_DEC", "L_ASSIGN", "L_LAND", "L_LOR",
  "L_QUESTION_QUESTION", "L_SHIFT", "L_EQ_NE", "L_ORDER", "L_IF", "L_ELSE",
  "L_SWITCH", "L_CASE", "L_DEFAULT", "L_RANGE", "L_DOT_DOT_DOT", "L_WHILE",
  "L_DO", "L_FOR", "L_FOREACH", "L_IN", "L_BREAK", "L_CONTINUE",
  "L_RETURN", "L_ARROW", "L_INHERIT", "L_COLON_COLON", "L_OPTIONAL_DOT",
  "L_DOT_OPTIONAL", "L_FUNCTION_OPEN", "L_SSCANF", "L_CATCH", "L_ARRAY",
  "L_REF", "L_PARSE_COMMAND", "L_TIME_EXPRESSION", "L_CLASS", "L_NEW",
  "L_PARAMETER", "L_TREE", "LOWER_THAN_ELSE", "'?'", "'|'", "'^'", "'&'",
  "'<'", "'+'", "'-'", "'*'", "'%'", "'/'", "'!'", "'~'", "';'", "'('",
  "')'", "'{'", "'}'", "','", "':'", "'$'", "'.'", "'['", "']'", "$accept",
  "loop_start", "foreach_start", "block_start", "special_context_start",
  "dollar_start", "all", "program", "opt_semicolon", "def", "function",
  "@1", "@2", "block_or_semi", "statement", "stmt_return", "stmt_cond",
  "optional_else_part", "stmt_while", "stmt_do", "stmt_for",
  "stmt_foreach", "stmt_switch", "block", "statements",
  "local_decl_statement_header", "local_declaration_statement",
  "local_decl_header", "local_declarations", "comma_expr", "expr",
  "lvalue", "primary_expr", "call_open", "function_call", "sscanf",
  "parse_command", "catch", "time_expression", "tree", "switch_header",
  "switch_block", "case", "case_label", "for_expr", "for_init",
  "foreach_var", "foreach_vars", "type_decl", "class_header",
  "member_list", "$@3", "member_name_list", "member_name", "class_init",
  "opt_class_init", "type", "type_modifier_list", "opt_basic_type",
  "basic_type", "opt_atomic_type", "atomic_type", "arg_type", "cast",
  "optional_star", "identifier", "new_local_name", "efun_override",
  "function_name", "inheritance", "modifier_change", "argument",
  "argument_list", "param_decl", "optional_default_arg_value",
  "local_name_list", "new_local_def", "single_new_local_def",
  "single_new_local_def_with_init", "name_list", "new_name",
  "lambda_return_type", "functional_open", "assoc_pair", "spread_expr",
  "arg_list", "opt_arg_list", "pair_list", "opt_pair_list", "lvalue_list",
  "expr_or_block", "ref", "string", "template_literal", "template_parts",
  "string_like", "string_literal", "string_const", "number", "real",
  "constant", YY_NULLPTR
  };
  return yy_sname[yysymbol];
}
#endif

#define YYPACT_NINF (-417)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-233)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -417,    31,    48,  -417,    35,    19,  -417,  -417,  -417,    47,
     190,  -417,  -417,  -417,  -417,  -417,    64,  -417,   355,    67,
      41,  -417,    25,   360,  -417,  -417,  -417,   185,   263,   381,
    -417,  -417,  -417,  -417,    49,  -417,    47,  -417,    25,   233,
      52,   216,   249,   258,  -417,  -417,  -417,    47,  1634,   266,
     355,  -417,   127,  -417,    25,  -417,  -417,   277,   186,   355,
     348,  -417,  -417,  1634,   344,   -14,    81,   369,   278,   355,
    1699,   320,  -417,  -417,   352,  -417,   372,  -417,   149,  1634,
    1634,  1634,   713,   384,  1671,   396,    54,  -417,  -417,  -417,
    -417,  -417,  -417,  1634,   373,   399,   399,   412,   343,   278,
    -417,  -417,    46,   233,  -417,  -417,    29,   421,  -417,  -417,
    -417,    47,  -417,  1634,  1760,   355,  -417,  1279,   133,   786,
    -417,   295,  -417,   199,   345,  1671,   399,  1634,   226,  1634,
     226,   431,  1279,  1634,  -417,  -417,  -417,  -417,  -417,   153,
    1634,  1350,   565,   125,    47,  -417,  1634,  1634,  1634,  1634,
    1634,  1634,  1634,  1634,  1634,  1634,  1634,  1634,  1634,  1634,
    1634,  1634,  -417,  1634,   355,    15,   408,   355,  1143,  -417,
     355,  1279,  1279,    29,  1634,   359,  -417,  -417,  -417,  -417,
    -417,   442,    47,  -417,   423,   104,  -417,  -417,  1671,  1634,
    -417,  -417,  -417,  1601,  -417,   418,   430,  -417,  -417,  1634,
     433,  1634,   463,  1245,  1634,  -417,  -417,  1316,  -417,   413,
     464,   169,   859,   175,   429,   750,  -417,   465,   471,  -417,
     472,  1634,   451,  1769,  1736,   279,   457,   191,  1038,   339,
     533,   614,   191,   319,   319,  -417,  -417,  -417,  1671,   399,
    1634,  -417,  1634,  -417,   490,  1634,   151,  -417,   482,   489,
     494,   378,  -417,  -417,   443,  -417,  -417,   192,  1760,  -417,
    1208,  -417,   386,  -417,  1671,  -417,  1634,   188,  1634,  -417,
    -417,  -417,  -417,     4,   496,   498,   505,  -417,   506,   508,
     514,   523,  1421,  -417,   859,  -417,  -417,  -417,  -417,  -417,
    -417,  -417,  -417,   535,    47,   859,   -32,   537,  -417,   399,
     542,  1634,   639,   546,  -417,   204,  1634,  1279,   -10,   135,
    1634,   158,   417,   399,  -417,  -417,   549,  1634,   558,   555,
      10,  -417,  -417,  -417,   560,  1387,  -417,  1458,   211,   215,
     859,  1634,  1634,  1634,  1001,  1072,   212,  -417,  -417,  -417,
     160,  -417,  -417,   443,   570,   566,  -417,  -417,  -417,  1279,
    -417,  1671,  -417,  -417,  -417,  1671,   563,  -417,  -417,   184,
     491,  -417,  1634,  -417,   202,  1279,  -417,  -417,   597,  -417,
    -417,  -417,  -417,  -417,  -417,   278,   578,  1634,  -417,   355,
    -417,  -417,   284,   285,   293,   618,   583,  -417,   589,    47,
     634,  -417,  -417,  -417,   585,   635,  -417,  -417,   652,  -417,
      47,    89,   601,  -417,  -417,  1634,  -417,   240,   603,   248,
    -417,   602,  1634,   600,  -417,  1387,  -417,   608,  1001,  -417,
    -417,   617,  1492,   443,  1634,   212,  1634,  1634,  -417,    17,
     616,    47,   930,  -417,  -417,   255,   677,  -417,  -417,  -417,
    -417,   402,  -417,   624,  1634,   678,  1001,  1634,   636,  -417,
    1671,  -417,  1529,  1671,  -417,   148,   696,   701,   706,   148,
      -5,   654,   466,  -417,   645,   930,   650,   930,  -417,  -417,
     653,  -417,  1671,  1001,  -417,  -417,   294,  1563,  -417,   651,
    -417,  -417,  -417,   385,    59,  -417,    39,    39,    39,    39,
      39,    39,    39,    39,    39,    39,    39,    39,  -417,  -417,
    -417,  -417,  -417,  -417,   662,   661,  1001,  -417,  -417,  -417,
     658,    39,   287,   782,   315,   525,   540,   688,   315,   328,
     328,  -417,  -417,  -417,  -417,  -417,  -417,  -417,  1001,  -417
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       9,     0,   170,     1,   170,    10,    12,    15,   160,   184,
     173,    14,    16,   171,    11,     8,   177,   185,     0,     0,
     214,   178,     0,     0,   197,   169,   172,     0,   174,     0,
     158,   161,   186,   187,   216,    13,   184,   246,     0,   248,
       0,   179,   180,     0,   175,   179,   180,   184,     0,     0,
       0,   215,     0,   247,     0,   196,   159,     0,   163,     0,
       0,   251,   252,     0,   218,    90,    91,     0,     0,     0,
       0,     0,     5,   237,     0,     5,     0,    92,     0,     0,
       0,     0,     0,     0,   217,     0,    82,    89,    83,    84,
      95,    85,    96,     0,     0,     0,     0,     0,     0,     0,
     242,   243,    93,   238,    86,    87,   177,   216,   249,   250,
     162,   184,   165,     0,     0,     0,   122,     0,     0,     0,
      77,    88,   193,    90,     0,    55,   219,     0,     0,     0,
       0,   122,     0,     0,     4,   136,    80,    78,    79,   178,
       0,     0,     0,     0,   184,     6,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,     0,     0,    76,
       0,     0,     0,   177,     0,     0,    57,   244,   245,   189,
     188,   181,   184,   205,     0,   199,   201,   164,    59,     0,
     240,   239,   194,   221,   223,   226,     0,   191,   190,     0,
       0,     0,     0,     0,     0,   235,   134,     0,   135,     0,
       0,     0,     0,     0,     0,     0,   228,   231,     0,    94,
       0,     0,    63,    62,    61,    70,    67,    68,     0,    64,
      65,    66,    69,    71,    72,    73,    74,    75,    58,    98,
       0,   100,     0,    99,     0,     0,     0,   195,     0,     0,
       0,     0,   115,   182,   203,    18,   200,   177,     0,   222,
       0,   127,     0,   117,    56,   119,     0,     0,     0,   167,
     167,   124,   137,     0,     0,     0,     0,     2,     0,     0,
       0,     0,     0,    32,     0,    30,    24,    25,    26,    27,
      28,    29,    31,     0,   184,     0,     0,     0,    50,     0,
       0,     0,     0,     0,   183,     0,     0,     0,     0,     0,
       0,     0,     0,   103,   123,   128,     0,     0,     0,   206,
       0,   202,   241,   224,     0,   233,   236,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   177,    33,    34,    35,
       0,    47,    45,     0,     0,   208,    48,    23,    53,     0,
     121,   220,   229,   120,    97,    60,     0,   101,   102,     0,
       0,   104,     0,   109,     0,     0,   114,   116,     0,   204,
      22,    21,    19,    20,   118,     0,     0,     0,   125,     0,
     126,    49,     0,     0,     0,     0,   150,   151,     0,   184,
       0,   152,   153,   155,   156,     0,   154,    36,   210,    51,
     184,   177,     0,   130,   111,     0,   110,     0,    55,     0,
     105,     0,     0,   233,   132,   233,   168,     0,     0,   138,
       2,     0,     0,     0,     0,   177,     0,     0,   209,     0,
       0,   184,     0,    52,   131,     0,    55,   106,   113,   108,
     129,     0,   234,     0,     0,    38,     0,     0,     0,   212,
     213,   157,     0,   211,   266,     0,     0,     0,     0,     0,
       0,   148,   147,   146,     0,     0,     0,     0,   107,   112,
       0,   133,   166,     0,    37,    40,     0,     0,     3,     0,
     267,   268,   269,     0,     0,   142,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    54,   140,
      44,   139,   207,    39,     0,     0,     0,   145,   265,   144,
       0,     0,   259,   256,   257,   253,   254,   255,   258,   260,
     261,   262,   263,   264,    41,     2,    43,   143,     0,    42
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -417,  -416,  -417,  -417,   666,  -417,  -417,  -417,  -417,  -417,
    -417,  -417,  -417,  -417,  -248,  -417,  -417,  -417,  -417,  -417,
    -417,  -417,  -417,   -71,  -154,  -417,  -417,  -417,  -417,   -68,
       8,   -63,   -62,   -66,  -417,  -417,  -417,  -417,  -417,  -417,
    -417,  -265,   341,  -338,  -372,  -417,   318,  -417,  -417,  -417,
    -417,  -417,   633,  -417,  -417,   475,  -417,   768,  -417,    -7,
    -417,  -417,  -323,  -417,    -1,    24,  -236,   684,  -417,  -417,
    -417,   606,  -417,   518,  -417,  -304,  -417,   441,  -417,   745,
    -417,  -417,  -417,   481,  -237,   -52,  -117,  -417,  -417,   -29,
     655,   612,   682,   692,   538,  -417,   -22,   -21,  -417,  -417,
    -190
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,   334,   506,   212,   128,   221,     1,     2,    15,     5,
       6,    49,   320,   372,   284,   285,   286,   474,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   431,   401,   296,
     125,    85,    86,   117,    87,    88,    89,    90,    91,    92,
     297,   466,   467,   460,   387,   388,   394,   395,     7,     8,
      16,    47,    57,    58,   416,   328,     9,    10,    25,   181,
      27,    28,   182,    93,   343,    94,   183,    95,    96,    11,
      12,   184,   185,   186,   369,   344,   345,   396,   391,    19,
      20,    97,    98,   216,   194,   195,   196,   217,   218,   376,
     206,    99,   100,   101,   191,   102,   103,   461,   104,   105,
     462
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      39,    40,   124,    26,   446,   120,   121,   135,    18,    31,
     132,   370,   389,   389,   143,   210,    39,    52,   319,   113,
      37,   454,   484,   323,   214,  -186,    32,    33,    37,   171,
     172,     3,    39,   109,   347,    50,   176,   121,    21,   201,
     179,   180,    34,   454,   455,     4,    59,    43,    -7,    37,
     448,   143,    63,   116,   248,   249,    84,   205,     4,   205,
     171,   201,    37,   454,    48,   211,   357,   485,   -88,   -88,
     330,   114,   213,    21,   107,   144,   371,   456,    29,   134,
     323,   457,   458,   112,   459,    14,   385,   136,   137,   138,
     240,   164,    38,   122,   165,   166,   428,  -198,    21,   456,
     246,   169,   389,   457,   458,   505,   511,   398,    17,   528,
      59,    54,    36,    29,   429,   430,   -17,   479,    55,   456,
    -187,   188,   251,   457,   458,   193,   459,   464,   167,   168,
     341,   509,   256,    35,    30,   203,   267,   207,    29,   192,
     193,   346,   198,   220,    32,    33,   510,   262,  -192,   193,
     215,    37,   454,   305,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     445,   238,   308,   307,   309,   257,   381,   311,   312,   193,
     193,   254,   193,   197,   465,   360,    54,   449,   239,   241,
     356,   243,   115,   219,   247,   108,   201,   258,   475,    21,
     499,    21,   501,   179,   180,   298,   201,   193,   456,   264,
     149,   358,   457,   458,   340,   459,   133,   465,   134,   465,
    -218,    21,   201,   392,   393,   503,   397,   313,    22,   201,
      44,   201,   402,   349,   361,  -177,    53,   272,  -186,    23,
     201,    29,   359,   299,   364,   366,   201,   365,   411,   373,
     157,   158,   159,   160,   161,   201,   326,   111,   526,   201,
     404,    29,    24,   382,   383,   384,   116,   386,   193,   483,
     199,   200,   354,   201,   325,   201,   327,   298,   410,   378,
     529,    37,   379,   380,    63,  -186,   379,    64,   298,    65,
      66,    67,   407,   204,   409,   134,   512,   513,   514,   515,
     516,   517,   518,   519,   520,   521,   522,   523,  -176,   351,
     215,   201,   413,   121,   355,   193,   437,    69,  -187,   201,
      70,   483,    72,   298,   439,   193,   201,    56,    76,    77,
      78,   468,   164,   106,   486,   165,   166,   435,   157,   158,
     159,   160,   161,   110,   441,   119,   493,   494,   495,   496,
     497,    83,   418,   419,   386,   201,   201,   193,   149,   150,
     151,   420,   504,   113,   201,   201,    32,    33,   408,   167,
     168,    41,    42,   193,   493,   494,   495,   496,   497,   476,
     159,   160,   161,   115,   442,   415,   443,   127,   423,   495,
     496,   497,    45,    46,   433,   154,   155,   156,   157,   158,
     159,   160,   161,   417,   486,   487,   488,    39,   118,   386,
     162,   163,   170,   436,   174,   175,   201,   202,    60,   129,
      37,    61,    62,    63,   269,   270,    64,   252,    65,    66,
      67,    68,   450,    39,   452,   453,    48,    39,    52,   131,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   317,
     318,   145,   472,   508,   179,   180,    69,   317,   324,    70,
      71,    72,    39,    73,    74,    75,   116,    76,    77,    78,
     149,   150,   151,   201,   470,   362,   149,    79,   151,   173,
     209,    80,    81,   242,    82,   486,   487,   488,    73,   260,
      83,   255,    60,   363,    37,    61,    62,    63,   261,   300,
      64,   263,    65,    66,    67,    68,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   156,   157,   158,   159,   160,
     161,   489,   490,   491,   492,   493,   494,   495,   496,   497,
      69,   265,   271,    70,    71,    72,   302,    73,    74,    75,
     304,    76,    77,    78,   486,   487,   488,   303,   310,   405,
     314,    79,   149,   150,   151,    80,    81,   315,    82,   486,
     487,   488,   316,   331,    83,   332,    60,   406,    37,    61,
      62,    63,   333,   335,    64,   336,    65,    66,    67,    68,
     337,   490,   491,   492,   493,   494,   495,   496,   497,   338,
     155,   156,   157,   158,   159,   160,   161,   491,   492,   493,
     494,   495,   496,   497,    69,   342,   348,    70,    71,    72,
     350,    73,    74,    75,   353,    76,    77,    78,   134,   146,
     147,   148,   149,   150,   151,    79,   367,   368,   374,    80,
      81,   403,    82,   149,   150,   151,   399,   400,    83,   412,
      60,  -230,    37,    61,    62,    63,   414,   421,    64,   424,
      65,    66,    67,    68,   201,   422,   425,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   427,   426,   434,
     440,   375,   156,   157,   158,   159,   160,   161,    69,   438,
     444,    70,    71,    72,   447,    73,    74,    75,   463,    76,
      77,    78,   471,   146,   147,   148,   149,   150,   151,    79,
     480,   473,   477,    80,    81,   481,    82,   486,   487,   488,
     482,   498,    83,    54,    60,  -232,    37,    61,    62,    63,
     500,   502,   139,   507,    65,    66,    67,    68,   524,   525,
     527,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   130,   432,   451,   187,   329,   492,   493,   494,   495,
     496,   497,    69,   469,   126,    70,    71,    72,  -177,    73,
      74,    75,    29,    76,    77,    78,   146,   147,   148,   149,
     150,   151,    13,    79,   140,   321,   390,    80,    81,   250,
      82,    51,   141,   352,   177,   208,    83,    60,   142,    37,
      61,    62,    63,   253,   178,    64,   322,    65,    66,    67,
      68,   486,     0,   488,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,     0,     0,     0,     0,     0,     0,
       0,     0,   301,     0,     0,    69,     0,     0,    70,    71,
      72,     0,    73,    74,    75,     0,    76,    77,    78,     0,
     492,   493,   494,   495,   496,   497,    79,   140,     0,     0,
      80,    81,     0,    82,     0,   141,     0,     0,     0,    83,
     273,   142,    37,    61,    62,    63,     0,     0,   139,     0,
      65,    66,    67,    68,     0,     0,     0,     0,     0,     0,
       0,   274,     0,   275,     0,     0,     0,     0,   276,   277,
     278,   279,     0,   280,   281,   282,     0,     0,    69,     0,
       0,    70,    71,    72,  -177,    73,    74,    75,    29,    76,
      77,    78,     0,     0,     0,     0,     0,     0,     0,    79,
       0,     0,     0,    80,    81,   283,    82,     0,   134,   -46,
       0,    60,    83,    37,    61,    62,    63,     0,     0,    64,
       0,    65,    66,    67,    68,     0,     0,     0,     0,     0,
       0,     0,   274,     0,   275,   429,   430,     0,     0,   276,
     277,   278,   279,     0,   280,   281,   282,     0,     0,    69,
       0,     0,    70,    71,    72,     0,    73,    74,    75,     0,
      76,    77,    78,     0,     0,     0,     0,     0,     0,     0,
      79,     0,     0,     0,    80,    81,   283,    82,     0,   134,
    -141,     0,    60,    83,    37,    61,    62,    63,     0,     0,
      64,     0,    65,    66,    67,    68,     0,     0,     0,     0,
       0,     0,     0,   274,     0,   275,     0,     0,     0,     0,
     276,   277,   278,   279,     0,   280,   281,   282,     0,     0,
      69,     0,     0,    70,    71,    72,     0,    73,    74,    75,
       0,    76,    77,    78,   146,   147,   148,   149,   150,   151,
       0,    79,     0,     0,     0,    80,    81,   283,    82,     0,
     134,     0,     0,    60,    83,    37,    61,    62,    63,     0,
       0,   139,     0,    65,    66,    67,    68,     0,     0,     0,
       0,     0,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,     0,     0,     0,     0,     0,     0,     0,     0,
     306,    69,     0,     0,    70,    71,    72,  -177,    73,    74,
      75,    29,    76,    77,    78,     0,     0,     0,     0,     0,
       0,     0,    79,     0,     0,     0,    80,    81,  -149,    82,
       0,     0,     0,     0,    60,    83,    37,    61,    62,    63,
       0,     0,    64,     0,    65,    66,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     244,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,     0,     0,    70,    71,    72,     0,    73,
      74,    75,     0,    76,    77,    78,     0,     0,     0,     0,
       0,   245,     0,    79,     0,     0,     0,    80,    81,    60,
      82,    37,    61,    62,    63,     0,    83,    64,     0,    65,
      66,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    69,     0,     0,
      70,    71,    72,     0,    73,    74,    75,     0,    76,    77,
      78,   146,   147,   148,   149,   150,   151,     0,    79,     0,
       0,     0,    80,    81,     0,    82,  -227,     0,  -227,     0,
      60,    83,    37,    61,    62,    63,     0,     0,    64,     0,
      65,    66,    67,    68,     0,     0,     0,     0,     0,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,     0,
       0,     0,     0,     0,     0,     0,   266,     0,    69,     0,
       0,    70,    71,    72,     0,    73,    74,    75,     0,    76,
      77,    78,   146,   147,   148,   149,   150,   151,     0,    79,
       0,     0,     0,    80,    81,     0,    82,  -225,     0,     0,
       0,    60,    83,    37,    61,    62,    63,     0,     0,    64,
       0,    65,    66,    67,    68,     0,     0,     0,     0,     0,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
       0,     0,     0,     0,     0,     0,     0,   268,     0,    69,
       0,     0,    70,    71,    72,     0,    73,    74,    75,     0,
      76,    77,    78,   146,   147,   148,   149,   150,   151,     0,
      79,     0,     0,     0,    80,    81,     0,    82,     0,     0,
    -225,     0,    60,    83,    37,    61,    62,    63,     0,     0,
      64,     0,    65,    66,    67,    68,     0,     0,     0,     0,
       0,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,     0,     0,     0,     0,     0,     0,     0,   375,     0,
      69,     0,     0,    70,    71,    72,     0,    73,    74,    75,
       0,    76,    77,    78,   146,   147,   148,   149,   150,   151,
       0,    79,     0,     0,     0,    80,    81,   339,    82,     0,
       0,     0,     0,    60,    83,    37,    61,    62,    63,     0,
       0,    64,     0,    65,    66,    67,    68,     0,     0,     0,
       0,     0,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,     0,     0,     0,     0,     0,     0,     0,   377,
       0,    69,     0,     0,    70,    71,    72,     0,    73,    74,
      75,     0,    76,    77,    78,   146,   147,   148,   149,   150,
     151,     0,    79,     0,     0,     0,    80,    81,  -149,    82,
       0,     0,     0,     0,    60,    83,    37,    61,    62,    63,
       0,     0,    64,     0,    65,    66,    67,    68,     0,     0,
       0,     0,     0,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,     0,     0,     0,     0,   478,     0,     0,
       0,     0,    69,     0,     0,    70,    71,    72,     0,    73,
      74,    75,     0,    76,    77,    78,     0,   146,   147,   148,
     149,   150,   151,    79,     0,     0,     0,    80,    81,   259,
      82,  -149,     0,     0,     0,    60,    83,    37,    61,    62,
      63,     0,     0,    64,     0,    65,    66,    67,    68,     0,
       0,     0,     0,     0,     0,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,     0,     0,     0,     0,     0,
       0,     0,     0,    69,     0,     0,    70,    71,    72,     0,
      73,    74,    75,     0,    76,    77,    78,   146,   147,   148,
     149,   150,   151,     0,    79,     0,     0,     0,    80,    81,
      60,    82,    37,    61,    62,    63,     0,    83,    64,     0,
     123,    66,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,     0,     0,     0,    69,     0,
       0,    70,    71,    72,     0,    73,    74,    75,     0,    76,
      77,    78,   146,   147,     0,   149,   150,   151,     0,    79,
       0,     0,     0,    80,    81,     0,    82,   189,   190,     0,
       0,     0,    83,     0,     0,     0,   146,   147,   148,   149,
     150,   151,     0,     0,     0,   146,     0,     0,   149,   150,
     151,   153,   154,   155,   156,   157,   158,   159,   160,   161,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   153,   154,   155,   156,   157,   158,
     159,   160,   161
};

static const yytype_int16 yycheck[] =
{
      22,    22,    70,    10,   420,    68,    68,    78,     9,    16,
      76,     1,   335,   336,    82,   132,    38,    38,   254,    15,
       3,     4,    27,   260,   141,    39,    11,    12,     3,    95,
      96,     0,    54,    54,    66,    36,    99,    99,     9,    71,
      11,    12,    18,     4,    27,    10,    47,    23,     0,     3,
     422,   119,     6,    67,   171,   172,    48,   128,    10,   130,
     126,    71,     3,     4,    15,   133,    76,    72,    14,    15,
      66,    63,   140,     9,    50,    82,    66,    60,    49,    69,
     317,    64,    65,    59,    67,    66,   334,    79,    80,    81,
      75,    37,    67,    69,    40,    41,   400,    68,     9,    60,
     168,    93,   425,    64,    65,   477,    67,   343,    61,   525,
     111,    59,    71,    49,    25,    26,    67,   455,    66,    60,
      39,   113,   174,    64,    65,   117,    67,   431,    74,    75,
     284,    72,    28,    66,    70,   127,   204,   129,    49,   115,
     132,   295,   118,   144,    11,    12,   484,   199,    67,   141,
     142,     3,     4,   221,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     418,   163,   240,   239,   242,    71,   330,   245,    27,   171,
     172,   182,   174,    50,   432,    27,    59,   423,   164,   165,
     307,   167,    39,    68,   170,    68,    71,   189,   446,     9,
     465,     9,   467,    11,    12,   212,    71,   199,    60,   201,
      19,    76,    64,    65,   282,    67,    67,   465,    69,   467,
      67,     9,    71,    11,    12,   473,    66,    76,    38,    71,
      45,    71,   349,   299,    76,    45,     3,    68,    39,    49,
      71,    49,   310,    68,   312,   316,    71,   313,   365,   320,
      59,    60,    61,    62,    63,    71,    68,    71,   506,    71,
      76,    49,    72,   331,   332,   333,    67,   335,   260,   459,
      71,    72,    68,    71,   266,    71,   268,   284,    76,    68,
     528,     3,    71,    68,     6,    69,    71,     9,   295,    11,
      12,    13,   360,    67,   362,    69,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,    45,   301,
     302,    71,   375,   375,   306,   307,    76,    39,    69,    71,
      42,   511,    44,   330,    76,   317,    71,    69,    50,    51,
      52,    76,    37,    67,    19,    40,    41,   405,    59,    60,
      61,    62,    63,    66,   412,    67,    59,    60,    61,    62,
      63,    73,    68,    68,   422,    71,    71,   349,    19,    20,
      21,    68,    68,    15,    71,    71,    11,    12,   360,    74,
      75,    11,    12,   365,    59,    60,    61,    62,    63,   447,
      61,    62,    63,    39,   413,   377,   415,    67,   389,    61,
      62,    63,    11,    12,   401,    56,    57,    58,    59,    60,
      61,    62,    63,   379,    19,    20,    21,   429,    39,   477,
      14,    15,    39,   405,    71,    72,    71,    72,     1,    67,
       3,     4,     5,     6,    11,    12,     9,    68,    11,    12,
      13,    14,   424,   455,   426,   427,    15,   459,   459,    67,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    71,
      72,    67,   444,    68,    11,    12,    39,    71,    72,    42,
      43,    44,   484,    46,    47,    48,    67,    50,    51,    52,
      19,    20,    21,    71,    72,    58,    19,    60,    21,    67,
      49,    64,    65,    75,    67,    19,    20,    21,    46,    71,
      73,    68,     1,    76,     3,     4,     5,     6,    68,    70,
       9,    68,    11,    12,    13,    14,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    58,    59,    60,    61,    62,
      63,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      39,    68,    68,    42,    43,    44,    71,    46,    47,    48,
      68,    50,    51,    52,    19,    20,    21,    76,    58,    58,
      68,    60,    19,    20,    21,    64,    65,    68,    67,    19,
      20,    21,    68,    67,    73,    67,     1,    76,     3,     4,
       5,     6,    67,    67,     9,    67,    11,    12,    13,    14,
      66,    56,    57,    58,    59,    60,    61,    62,    63,    66,
      57,    58,    59,    60,    61,    62,    63,    57,    58,    59,
      60,    61,    62,    63,    39,    70,    69,    42,    43,    44,
      68,    46,    47,    48,    68,    50,    51,    52,    69,    16,
      17,    18,    19,    20,    21,    60,    68,    72,    68,    64,
      65,    68,    67,    19,    20,    21,    66,    71,    73,    42,
       1,    76,     3,     4,     5,     6,    68,    29,     9,    15,
      11,    12,    13,    14,    71,    66,    71,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    15,    33,    68,
      68,    71,    58,    59,    60,    61,    62,    63,    39,    76,
      72,    42,    43,    44,    67,    46,    47,    48,    72,    50,
      51,    52,    68,    16,    17,    18,    19,    20,    21,    60,
       4,    23,    66,    64,    65,     4,    67,    19,    20,    21,
       4,    66,    73,    59,     1,    76,     3,     4,     5,     6,
      70,    68,     9,    72,    11,    12,    13,    14,    66,    68,
      72,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    75,   401,   425,   111,   270,    58,    59,    60,    61,
      62,    63,    39,    76,    70,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    16,    17,    18,    19,
      20,    21,     4,    60,    61,   257,   335,    64,    65,   173,
      67,    36,    69,   302,   102,   130,    73,     1,    75,     3,
       4,     5,     6,   181,   102,     9,   258,    11,    12,    13,
      14,    19,    -1,    21,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    72,    -1,    -1,    39,    -1,    -1,    42,    43,
      44,    -1,    46,    47,    48,    -1,    50,    51,    52,    -1,
      58,    59,    60,    61,    62,    63,    60,    61,    -1,    -1,
      64,    65,    -1,    67,    -1,    69,    -1,    -1,    -1,    73,
       1,    75,     3,     4,     5,     6,    -1,    -1,     9,    -1,
      11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    22,    -1,    24,    -1,    -1,    -1,    -1,    29,    30,
      31,    32,    -1,    34,    35,    36,    -1,    -1,    39,    -1,
      -1,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,
      -1,    -1,    -1,    64,    65,    66,    67,    -1,    69,    70,
      -1,     1,    73,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    22,    -1,    24,    25,    26,    -1,    -1,    29,
      30,    31,    32,    -1,    34,    35,    36,    -1,    -1,    39,
      -1,    -1,    42,    43,    44,    -1,    46,    47,    48,    -1,
      50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    -1,    -1,    -1,    64,    65,    66,    67,    -1,    69,
      70,    -1,     1,    73,     3,     4,     5,     6,    -1,    -1,
       9,    -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    22,    -1,    24,    -1,    -1,    -1,    -1,
      29,    30,    31,    32,    -1,    34,    35,    36,    -1,    -1,
      39,    -1,    -1,    42,    43,    44,    -1,    46,    47,    48,
      -1,    50,    51,    52,    16,    17,    18,    19,    20,    21,
      -1,    60,    -1,    -1,    -1,    64,    65,    66,    67,    -1,
      69,    -1,    -1,     1,    73,     3,     4,     5,     6,    -1,
      -1,     9,    -1,    11,    12,    13,    14,    -1,    -1,    -1,
      -1,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      72,    39,    -1,    -1,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    -1,    -1,    -1,    64,    65,    66,    67,
      -1,    -1,    -1,    -1,     1,    73,     3,     4,     5,     6,
      -1,    -1,     9,    -1,    11,    12,    13,    14,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    39,    -1,    -1,    42,    43,    44,    -1,    46,
      47,    48,    -1,    50,    51,    52,    -1,    -1,    -1,    -1,
      -1,    58,    -1,    60,    -1,    -1,    -1,    64,    65,     1,
      67,     3,     4,     5,     6,    -1,    73,     9,    -1,    11,
      12,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,    -1,
      42,    43,    44,    -1,    46,    47,    48,    -1,    50,    51,
      52,    16,    17,    18,    19,    20,    21,    -1,    60,    -1,
      -1,    -1,    64,    65,    -1,    67,    68,    -1,    70,    -1,
       1,    73,     3,     4,     5,     6,    -1,    -1,     9,    -1,
      11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,    39,    -1,
      -1,    42,    43,    44,    -1,    46,    47,    48,    -1,    50,
      51,    52,    16,    17,    18,    19,    20,    21,    -1,    60,
      -1,    -1,    -1,    64,    65,    -1,    67,    68,    -1,    -1,
      -1,     1,    73,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,    39,
      -1,    -1,    42,    43,    44,    -1,    46,    47,    48,    -1,
      50,    51,    52,    16,    17,    18,    19,    20,    21,    -1,
      60,    -1,    -1,    -1,    64,    65,    -1,    67,    -1,    -1,
      70,    -1,     1,    73,     3,     4,     5,     6,    -1,    -1,
       9,    -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,
      39,    -1,    -1,    42,    43,    44,    -1,    46,    47,    48,
      -1,    50,    51,    52,    16,    17,    18,    19,    20,    21,
      -1,    60,    -1,    -1,    -1,    64,    65,    66,    67,    -1,
      -1,    -1,    -1,     1,    73,     3,     4,     5,     6,    -1,
      -1,     9,    -1,    11,    12,    13,    14,    -1,    -1,    -1,
      -1,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,
      -1,    39,    -1,    -1,    42,    43,    44,    -1,    46,    47,
      48,    -1,    50,    51,    52,    16,    17,    18,    19,    20,
      21,    -1,    60,    -1,    -1,    -1,    64,    65,    66,    67,
      -1,    -1,    -1,    -1,     1,    73,     3,     4,     5,     6,
      -1,    -1,     9,    -1,    11,    12,    13,    14,    -1,    -1,
      -1,    -1,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    -1,    -1,    -1,    -1,    68,    -1,    -1,
      -1,    -1,    39,    -1,    -1,    42,    43,    44,    -1,    46,
      47,    48,    -1,    50,    51,    52,    -1,    16,    17,    18,
      19,    20,    21,    60,    -1,    -1,    -1,    64,    65,    28,
      67,    68,    -1,    -1,    -1,     1,    73,     3,     4,     5,
       6,    -1,    -1,     9,    -1,    11,    12,    13,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    39,    -1,    -1,    42,    43,    44,    -1,
      46,    47,    48,    -1,    50,    51,    52,    16,    17,    18,
      19,    20,    21,    -1,    60,    -1,    -1,    -1,    64,    65,
       1,    67,     3,     4,     5,     6,    -1,    73,     9,    -1,
      11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    -1,    -1,    -1,    39,    -1,
      -1,    42,    43,    44,    -1,    46,    47,    48,    -1,    50,
      51,    52,    16,    17,    -1,    19,    20,    21,    -1,    60,
      -1,    -1,    -1,    64,    65,    -1,    67,     7,     8,    -1,
      -1,    -1,    73,    -1,    -1,    -1,    16,    17,    18,    19,
      20,    21,    -1,    -1,    -1,    16,    -1,    -1,    19,    20,
      21,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    55,    56,    57,    58,    59,    60,
      61,    62,    63
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    83,    84,     0,    10,    86,    87,   125,   126,   133,
     134,   146,   147,   134,    66,    85,   127,    61,   141,   156,
     157,     9,    38,    49,    72,   135,   136,   137,   138,    49,
      70,   136,    11,    12,   142,    66,    71,     3,    67,   173,
     174,    11,    12,   142,    45,    11,    12,   128,    15,    88,
     141,   156,   174,     3,    59,    66,    69,   129,   130,   141,
       1,     4,     5,     6,     9,    11,    12,    13,    14,    39,
      42,    43,    44,    46,    47,    48,    50,    51,    52,    60,
      64,    65,    67,    73,   107,   108,   109,   111,   112,   113,
     114,   115,   116,   140,   142,   144,   145,   158,   159,   168,
     169,   170,   172,   173,   175,   176,    67,   142,    68,   174,
      66,    71,   142,    15,   107,    39,    67,   110,    39,    67,
     108,   109,   142,    11,   106,   107,   144,    67,    81,    67,
      81,    67,   110,    67,    69,   100,   107,   107,   107,     9,
      61,    69,    75,   106,   136,    67,    16,    17,    18,    19,
      20,    21,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    14,    15,    37,    40,    41,    74,    75,   107,
      39,   110,   110,    67,    71,    72,   108,   169,   170,    11,
      12,   136,   139,   143,   148,   149,   150,   129,   107,     7,
       8,   171,   142,   107,   161,   162,   163,    50,   142,    71,
      72,    71,    72,   107,    67,   100,   167,   107,   167,    49,
     163,   106,    80,   106,   163,   107,   160,   164,   165,    68,
     141,    82,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   142,
      75,   142,    75,   142,    27,    58,   106,   142,   163,   163,
     148,   162,    68,   168,   141,    68,    28,    71,   107,    28,
      71,    68,   162,    68,   107,    68,    71,   106,    71,    11,
      12,    68,    68,     1,    22,    24,    29,    30,    31,    32,
      34,    35,    36,    66,    91,    92,    93,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   106,   117,   136,    68,
      70,    72,    71,    76,    68,   106,    72,   110,   106,   106,
      58,   106,    27,    76,    68,    68,    68,    71,    72,   143,
      89,   150,   171,   161,    72,   107,    68,   107,   132,   132,
      66,    67,    67,    67,    78,    67,    67,    66,    66,    66,
     106,   101,    70,   141,   152,   153,   101,    66,    69,   110,
      68,   107,   160,    68,    68,   107,   163,    76,    76,   106,
      27,    76,    58,    76,   106,   110,   100,    68,    72,   151,
       1,    66,    90,   100,    68,    71,   166,    71,    68,    71,
      68,   101,   106,   106,   106,    91,   106,   121,   122,   139,
     154,   155,    11,    12,   123,   124,   154,    66,   143,    66,
      71,   105,   163,    68,    76,    58,    76,   106,   107,   106,
      76,   163,    42,   108,    68,   107,   131,   142,    68,    68,
      68,    29,    66,   141,    15,    71,    33,    15,   152,    25,
      26,   104,   119,   136,    68,   106,   107,    76,    76,    76,
      68,   106,   166,   166,    72,    91,    78,    67,   121,   143,
     107,   123,   107,   107,     4,    27,    60,    64,    65,    67,
     120,   174,   177,    72,   152,    91,   118,   119,    76,    76,
      72,    68,   107,    23,    94,    91,   106,    66,    68,   120,
       4,     4,     4,   177,    27,    72,    19,    20,    21,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    66,   118,
      70,   118,    68,    91,    68,   121,    79,    72,    68,    72,
     120,    67,   177,   177,   177,   177,   177,   177,   177,   177,
     177,   177,   177,   177,    66,    68,    91,    72,    78,    91
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    77,    78,    79,    80,    81,    82,    83,    84,    84,
      85,    85,    86,    86,    86,    86,    86,    88,    89,    87,
      90,    90,    90,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    92,    92,    93,    94,    94,
      95,    96,    97,    98,    99,   100,   101,   101,   101,   101,
     102,   103,   104,   105,   105,   106,   106,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   108,   109,
     109,   109,   109,   109,   109,   109,   109,   109,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   109,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   109,   109,   109,
     109,   109,   110,   111,   111,   111,   111,   111,   111,   111,
     111,   111,   112,   113,   114,   115,   116,   116,   117,   118,
     118,   118,   119,   119,   119,   119,   119,   120,   120,   121,
     121,   122,   122,   123,   123,   123,   124,   124,   125,   126,
     127,   128,   127,   129,   129,   130,   131,   132,   132,   133,
     134,   134,   135,   135,   136,   136,   137,   137,   138,   138,
     138,   139,   139,   140,   141,   141,   142,   142,   143,   143,
     144,   144,   145,   145,   145,   145,   146,   147,   148,   148,
     148,   149,   149,   150,   150,   150,   151,   151,   152,   152,
     153,   153,   154,   155,   156,   156,   157,   157,   158,   159,
     160,   161,   161,   162,   162,   163,   163,   163,   164,   164,
     165,   165,   165,   166,   166,   167,   167,   168,   169,   170,
     171,   171,   172,   172,   172,   172,   173,   173,   174,   174,
     174,   175,   176,   177,   177,   177,   177,   177,   177,   177,
     177,   177,   177,   177,   177,   177,   177,   177,   177,   177
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     0,     0,     0,     0,     1,     3,     0,
       0,     1,     1,     3,     1,     1,     1,     0,     0,     9,
       1,     1,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     2,     3,     6,     0,     2,
       6,     8,    10,     8,     6,     4,     0,     2,     2,     3,
       1,     3,     1,     0,     4,     1,     3,     2,     3,     3,
       5,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       2,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     1,     1,     5,     3,     3,
       3,     5,     5,     4,     5,     6,     7,     8,     7,     5,
       6,     6,     8,     7,     5,     3,     5,     4,     6,     4,
       5,     5,     1,     4,     4,     6,     6,     4,     4,     7,
       6,     7,     7,     9,     3,     3,     2,     4,     4,     2,
       2,     0,     3,     5,     4,     4,     2,     1,     1,     0,
       1,     1,     1,     1,     1,     1,     1,     3,     3,     4,
       0,     0,     5,     1,     3,     2,     3,     0,     3,     2,
       0,     2,     1,     0,     1,     2,     1,     0,     1,     2,
       2,     1,     2,     4,     0,     1,     1,     1,     1,     1,
       3,     3,     1,     2,     3,     3,     4,     2,     0,     1,
       2,     1,     3,     2,     4,     1,     0,     5,     1,     3,
       2,     4,     3,     3,     1,     3,     2,     4,     1,     2,
       3,     1,     2,     1,     3,     0,     1,     2,     1,     3,
       0,     1,     2,     0,     3,     1,     3,     1,     1,     3,
       1,     3,     1,     1,     2,     2,     1,     2,     1,     3,
       3,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     2,     2,     2
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
        yyerror (&yylloc, yyscanner, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


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


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location, yyscanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void* yyscanner)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  YY_USE (yyscanner);
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
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void* yyscanner)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp, yyscanner);
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule, void* yyscanner)
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
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]), yyscanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, yyscanner); \
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
/* Parser data structure.  */
struct yypstate
  {
    /* Number of syntax errors so far.  */
    int yynerrs;

    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    yy_state_t yyesa[20];
    yy_state_t *yyes;
    YYPTRDIFF_T yyes_capacity;
    /* Whether this instance has not started parsing yet.
     * If 2, it corresponds to a finished parsing.  */
    int yynew;
  };


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
  yypstate* yyps;
  yysymbol_kind_t yytoken;
  YYLTYPE *yylloc;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypstate_expected_tokens (yypstate *yyps,
                          yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;

  int yyx;
  for (yyx = 0; yyx < YYNTOKENS; ++yyx)
    {
      yysymbol_kind_t yysym = YY_CAST (yysymbol_kind_t, yyx);
      if (yysym != YYSYMBOL_YYerror && yysym != YYSYMBOL_YYUNDEF)
        switch (yy_lac (yyps->yyesa, &yyps->yyes, &yyps->yyes_capacity, yyps->yyssp, yysym))
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


/* Similar to the previous function.  */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  return yypstate_expected_tokens (yyctx->yyps, yyarg, yyargn);
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
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, void* yyscanner)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  YY_USE (yyscanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}





#define yynerrs yyps->yynerrs
#define yystate yyps->yystate
#define yyerrstatus yyps->yyerrstatus
#define yyssa yyps->yyssa
#define yyss yyps->yyss
#define yyssp yyps->yyssp
#define yyvsa yyps->yyvsa
#define yyvs yyps->yyvs
#define yyvsp yyps->yyvsp
#define yylsa yyps->yylsa
#define yyls yyps->yyls
#define yylsp yyps->yylsp
#define yystacksize yyps->yystacksize
#define yyesa yyps->yyesa
#define yyes yyps->yyes
#define yyes_capacity yyps->yyes_capacity

/* Initialize the parser data structure.  */
static void
yypstate_clear (yypstate *yyps)
{
  yynerrs = 0;
  yystate = 0;
  yyerrstatus = 0;

  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;

  /* Initialize the state stack, in case yypcontext_expected_tokens is
     called before the first call to yyparse. */
  *yyssp = 0;
  yyps->yynew = 1;
}

/* Initialize the parser data structure.  */
yypstate *
yypstate_new (void)
{
  yypstate *yyps;
  yyps = YY_CAST (yypstate *, YYMALLOC (sizeof *yyps));
  if (!yyps)
    return YY_NULLPTR;
  yystacksize = YYINITDEPTH;
  yyss = yyssa;
  yyvs = yyvsa;
  yyls = yylsa;
  yyes = yyesa;
  yyes_capacity = 20;
  if (YYMAXDEPTH < yyes_capacity)
    yyes_capacity = YYMAXDEPTH;
  yypstate_clear (yyps);
  return yyps;
}

void
yypstate_delete (yypstate *yyps)
{
  if (yyps)
    {
#ifndef yyoverflow
      /* If the stack was reallocated but the parse did not complete, then the
         stack still needs to be freed.  */
      if (yyss != yyssa)
        YYSTACK_FREE (yyss);
#endif
      if (yyes != yyesa)
        YYSTACK_FREE (yyes);
      YYFREE (yyps);
    }
}



/*---------------.
| yypush_parse.  |
`---------------*/

int
yypush_parse (yypstate *yyps,
              int yypushed_char, YYSTYPE const *yypushed_val, YYLTYPE *yypushed_loc, void* yyscanner)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

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
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  switch (yyps->yynew)
    {
    case 0:
      yyn = yypact[yystate];
      goto yyread_pushed_token;

    case 2:
      yypstate_clear (yyps);
      break;

    default:
      break;
    }

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = *yypushed_loc;
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
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
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
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

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
      if (!yyps->yynew)
        {
          YYDPRINTF ((stderr, "Return for a new token:\n"));
          yyresult = YYPUSH_MORE;
          goto yypushreturn;
        }
      yyps->yynew = 0;
yyread_pushed_token:
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yypushed_char;
      if (yypushed_val)
        yylval = *yypushed_val;
      if (yypushed_loc)
        yylloc = *yypushed_loc;
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
      yyerror_range[1] = yylloc;
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
  *++yylsp = yylloc;

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

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  {
    int yychar_backup = yychar;
    switch (yyn)
      {
  case 2: /* loop_start: %empty  */
#line 233 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { (yyval.number) = rule_loop_open(); }
#line 2603 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 3: /* foreach_start: %empty  */
#line 234 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                      { (yyval.number) = rule_foreach_open(); }
#line 2609 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 4: /* block_start: %empty  */
#line 235 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.number) = rule_block_open(); }
#line 2615 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 5: /* special_context_start: %empty  */
#line 236 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { (yyval.number) = rule_special_context_open(); }
#line 2621 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 6: /* dollar_start: %empty  */
#line 237 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                     { (yyval.contextp) = rule_dollar_open(); }
#line 2627 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 7: /* all: program  */
#line 246 "$REPO_ROOT$/src/compiler/internal/grammar.y"
           { rule_program((yyvsp[0].node)); }
#line 2633 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 8: /* program: program def opt_semicolon  */
#line 251 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_program_append(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 2639 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 9: /* program: %empty  */
#line 252 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.node) = 0; }
#line 2645 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 11: /* opt_semicolon: ';'  */
#line 258 "$REPO_ROOT$/src/compiler/internal/grammar.y"
         { rule_opt_semicolon(); }
#line 2651 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 13: /* def: type name_list ';'  */
#line 270 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_def_global_var((yyvsp[-2].number)); (yyval.node) = 0; }
#line 2657 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 17: /* @1: %empty  */
#line 290 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_func_type((yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].string)); }
#line 2663 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 18: /* @2: %empty  */
#line 292 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_func_proto((yyvsp[-3].number), (yyvsp[-5].number), &(yyvsp[-4].string), (yyvsp[-1].argument)); }
#line 2669 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 19: /* function: type optional_star identifier @1 '(' argument ')' @2 block_or_semi  */
#line 294 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_func(&(yyval.node), (yyvsp[-5].number), (yyvsp[-7].number), (yyvsp[-6].string), (yyvsp[-3].argument), &(yyvsp[-1].number), &(yyvsp[0].node)); }
#line 2675 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 20: /* block_or_semi: block  */
#line 299 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = rule_block_or_semi((yyvsp[0].decl).node); }
#line 2681 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 21: /* block_or_semi: ';'  */
#line 300 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2687 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 22: /* block_or_semi: error  */
#line 301 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2693 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 23: /* statement: comma_expr ';'  */
#line 311 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_expr((yyvsp[-1].node)); }
#line 2699 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 27: /* statement: stmt_for  */
#line 315 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2705 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 28: /* statement: stmt_foreach  */
#line 316 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2711 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 31: /* statement: block  */
#line 319 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2717 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 32: /* statement: ';'  */
#line 320 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = 0; }
#line 2723 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 33: /* statement: L_BREAK ';'  */
#line 321 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_break(); }
#line 2729 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 34: /* statement: L_CONTINUE ';'  */
#line 322 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_continue(); }
#line 2735 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 35: /* stmt_return: L_RETURN ';'  */
#line 328 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_return_void(&(yyval.node)); }
#line 2741 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 36: /* stmt_return: L_RETURN comma_expr ';'  */
#line 329 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_return_expr(&(yyval.node), (yyvsp[-1].node)); }
#line 2747 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 37: /* stmt_cond: L_IF '(' comma_expr ')' statement optional_else_part  */
#line 335 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_cond(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 2753 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 38: /* optional_else_part: %empty  */
#line 339 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                { (yyval.node) = 0; }
#line 2759 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 39: /* optional_else_part: L_ELSE statement  */
#line 340 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                 { (yyval.node) = (yyvsp[0].node); }
#line 2765 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 40: /* stmt_while: L_WHILE '(' comma_expr ')' loop_start statement  */
#line 346 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_while(&(yyval.node), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2771 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 41: /* stmt_do: L_DO loop_start statement L_WHILE '(' comma_expr ')' ';'  */
#line 352 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_do(&(yyval.node), (yyvsp[-5].node), (yyvsp[-2].node), (yyvsp[-6].number)); }
#line 2777 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 42: /* stmt_for: L_FOR '(' for_init ';' for_expr ';' for_expr ')' loop_start statement  */
#line 358 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for(&(yyval.decl), &(yyvsp[-7].decl), (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2783 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 43: /* stmt_foreach: L_FOREACH '(' foreach_vars L_IN expr ')' foreach_start statement  */
#line 364 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach(&(yyval.decl), &(yyvsp[-5].decl), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2789 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 44: /* stmt_switch: switch_header '{' local_declarations case switch_block '}'  */
#line 370 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_switch(&(yyval.node), (yyvsp[-5].switch_header).expr, &(yyvsp[-3].decl), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[-5].switch_header).saved_context, (yyvsp[-5].switch_header).saved_cases_size); }
#line 2795 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 45: /* block: '{' block_start statements '}'  */
#line 382 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block(&(yyval.decl), (yyvsp[-1].decl).node, (yyvsp[-2].number)); }
#line 2801 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 46: /* statements: %empty  */
#line 388 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_empty(&(yyval.decl)); }
#line 2807 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 47: /* statements: statement statements  */
#line 390 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_stmt(&(yyval.decl), (yyvsp[-1].node), &(yyvsp[0].decl)); }
#line 2813 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 48: /* statements: local_declaration_statement statements  */
#line 392 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_decl(&(yyval.decl), &(yyvsp[-1].decl), &(yyvsp[0].decl)); }
#line 2819 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 49: /* statements: error ';' statements  */
#line 394 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_error(&(yyval.decl), &(yyvsp[0].decl)); }
#line 2825 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 50: /* local_decl_statement_header: basic_type  */
#line 399 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { rule_local_declaration_statement_set_type((yyvsp[0].number)); (yyval.number) = (yyvsp[0].number); }
#line 2831 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 51: /* local_declaration_statement: local_decl_statement_header local_name_list ';'  */
#line 404 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_declaration_statement(&(yyval.decl), &(yyvsp[-1].decl)); }
#line 2837 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 52: /* local_decl_header: basic_type  */
#line 409 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { rule_local_declarations_set_type((yyvsp[0].number)); (yyval.number) = (yyvsp[0].number); }
#line 2843 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 53: /* local_declarations: %empty  */
#line 414 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.decl).node = 0; (yyval.decl).num = 0; }
#line 2849 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 54: /* local_declarations: local_declarations local_decl_header local_name_list ';'  */
#line 416 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_declarations(&(yyval.decl), &(yyvsp[-3].decl), &(yyvsp[-1].decl)); }
#line 2855 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 56: /* comma_expr: comma_expr ',' expr  */
#line 435 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                { rule_comma_expr(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2861 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 57: /* expr: ref lvalue  */
#line 439 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_ref(&(yyval.node), (yyvsp[0].node)); }
#line 2867 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 58: /* expr: lvalue L_ASSIGN expr  */
#line 440 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_assign(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 2873 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 59: /* expr: error L_ASSIGN expr  */
#line 441 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_assign_error(&(yyval.node), (yyvsp[0].node)); }
#line 2879 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 60: /* expr: expr '?' expr ':' expr  */
#line 443 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                            { rule_expr_ternary(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2885 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 61: /* expr: expr L_QUESTION_QUESTION expr  */
#line 444 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_nullish(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2891 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 62: /* expr: expr L_LOR expr  */
#line 445 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_lor(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2897 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 63: /* expr: expr L_LAND expr  */
#line 446 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_land(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2903 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 64: /* expr: expr '|' expr  */
#line 448 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_or(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2909 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 65: /* expr: expr '^' expr  */
#line 449 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_xor(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2915 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 66: /* expr: expr '&' expr  */
#line 450 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_and(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2921 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 67: /* expr: expr L_EQ_NE expr  */
#line 452 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_expr_eq_ne(&(yyval.node), (yyvsp[-1].number), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2927 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 68: /* expr: expr L_ORDER expr  */
#line 453 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_order(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 2933 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 69: /* expr: expr '<' expr  */
#line 454 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_lt(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2939 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 70: /* expr: expr L_SHIFT expr  */
#line 456 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_expr_shift(&(yyval.node), (yyvsp[-1].number), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2945 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 71: /* expr: expr '+' expr  */
#line 458 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             {
        rule_set_operand_ranges((yylsp[-2]).first_line, (yylsp[-2]).first_column, (yylsp[-2]).last_column,
                                (yylsp[-1]).first_line, (yylsp[-1]).first_column,
                                (yylsp[0]).first_line, (yylsp[0]).first_column, (yylsp[0]).last_column);
        rule_expr_add(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node));
        rule_clear_operand_ranges();
    }
#line 2957 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 72: /* expr: expr '-' expr  */
#line 465 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_sub(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2963 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 73: /* expr: expr '*' expr  */
#line 466 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_mul(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2969 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 74: /* expr: expr '%' expr  */
#line 467 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_mod(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2975 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 75: /* expr: expr '/' expr  */
#line 468 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_div(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2981 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 76: /* expr: cast expr  */
#line 470 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_cast(&(yyval.node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 2987 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 77: /* expr: L_INC_DEC lvalue  */
#line 471 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_expr_pre_incdec(&(yyval.node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 2993 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 78: /* expr: '!' expr  */
#line 472 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_expr_not(&(yyval.node), (yyvsp[0].node)); }
#line 2999 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 79: /* expr: '~' expr  */
#line 473 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_expr_compl(&(yyval.node), (yyvsp[0].node)); }
#line 3005 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 80: /* expr: '-' expr  */
#line 474 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_neg(&(yyval.node), (yyvsp[0].node)); }
#line 3011 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 81: /* expr: lvalue L_INC_DEC  */
#line 476 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_expr_post_incdec(&(yyval.node), (yyvsp[0].number), (yyvsp[-1].node)); }
#line 3017 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 88: /* lvalue: primary_expr  */
#line 488 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                { (yyval.node) = rule_lvalue((yyvsp[0].node)); }
#line 3023 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 90: /* primary_expr: L_DEFINED_NAME  */
#line 498 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_defined_name(&(yyval.node), (yyvsp[0].ihe)); }
#line 3029 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 91: /* primary_expr: L_IDENTIFIER  */
#line 499 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_identifier(&(yyval.node), (yyvsp[0].string)); }
#line 3035 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 92: /* primary_expr: L_PARAMETER  */
#line 500 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_parameter(&(yyval.node), (yyvsp[0].number)); }
#line 3041 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 94: /* primary_expr: '(' comma_expr ')'  */
#line 502 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { (yyval.node) = (yyvsp[-1].node); }
#line 3047 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 97: /* primary_expr: '$' '(' dollar_start comma_expr ')'  */
#line 508 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_dollar_expr(&(yyval.node), (yyvsp[-2].contextp), (yyvsp[-1].node)); }
#line 3053 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 98: /* primary_expr: primary_expr L_ARROW identifier  */
#line 511 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                 { rule_primary_expr_member_arrow(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 3059 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 99: /* primary_expr: primary_expr '.' identifier  */
#line 512 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                 { rule_primary_expr_member_dot(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 3065 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 100: /* primary_expr: primary_expr L_OPTIONAL_DOT identifier  */
#line 518 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_member_optional(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 3071 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 101: /* primary_expr: primary_expr L_OPTIONAL_DOT '[' comma_expr ']'  */
#line 520 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_optional(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3077 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 102: /* primary_expr: primary_expr L_DOT_OPTIONAL '[' comma_expr ']'  */
#line 522 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_optional(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3083 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 103: /* primary_expr: primary_expr '[' comma_expr ']'  */
#line 527 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3089 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 104: /* primary_expr: primary_expr '[' '<' comma_expr ']'  */
#line 529 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_r(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3095 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 105: /* primary_expr: primary_expr '[' comma_expr L_RANGE comma_expr ']'  */
#line 531 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nn(&(yyval.node), (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3101 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 106: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE comma_expr ']'  */
#line 533 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rn(&(yyval.node), (yyvsp[-6].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3107 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 107: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE '<' comma_expr ']'  */
#line 535 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rr(&(yyval.node), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3113 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 108: /* primary_expr: primary_expr '[' comma_expr L_RANGE '<' comma_expr ']'  */
#line 537 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nr(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3119 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 109: /* primary_expr: primary_expr '[' comma_expr L_RANGE ']'  */
#line 539 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_ne(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node)); }
#line 3125 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 110: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE ']'  */
#line 541 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_re(&(yyval.node), (yyvsp[-5].node), (yyvsp[-2].node)); }
#line 3131 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 111: /* primary_expr: primary_expr '[' L_RANGE '<' comma_expr ']'  */
#line 543 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nr(&(yyval.node), (yyvsp[-5].node), 0, (yyvsp[-1].node)); }
#line 3137 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 112: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE '<' expr ']'  */
#line 546 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rr(&(yyval.node), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3143 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 113: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE expr ']'  */
#line 548 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rn(&(yyval.node), (yyvsp[-6].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3149 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 114: /* primary_expr: lambda_return_type '(' argument ')' block  */
#line 553 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_anon_func(&(yyval.node), &(yyvsp[-4].func_block), &(yyvsp[-2].argument), &(yyvsp[0].decl)); }
#line 3155 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 115: /* primary_expr: functional_open ':' ')'  */
#line 560 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_1(&(yyval.node), (yyvsp[-2].number)); }
#line 3161 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 116: /* primary_expr: functional_open ',' arg_list ':' ')'  */
#line 562 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_2(&(yyval.node), (yyvsp[-4].number), (yyvsp[-2].node)); }
#line 3167 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 117: /* primary_expr: L_FUNCTION_OPEN L_DEFINED_NAME ':' ')'  */
#line 571 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_1(&(yyval.node), rule_functional_ref((yyvsp[-2].ihe))); }
#line 3173 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 118: /* primary_expr: L_FUNCTION_OPEN L_DEFINED_NAME ',' arg_list ':' ')'  */
#line 573 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_2(&(yyval.node), rule_functional_ref((yyvsp[-4].ihe)), (yyvsp[-2].node)); }
#line 3179 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 119: /* primary_expr: L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 575 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_3(&(yyval.node), (yyvsp[-2].node)); }
#line 3185 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 120: /* primary_expr: '(' '[' opt_pair_list ']' ')'  */
#line 578 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_primary_expr_mapping(&(yyval.node), (yyvsp[-2].node)); }
#line 3191 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 121: /* primary_expr: '(' '{' opt_arg_list '}' ')'  */
#line 579 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_primary_expr_array(&(yyval.node), (yyvsp[-2].node)); }
#line 3197 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 122: /* call_open: '('  */
#line 589 "$REPO_ROOT$/src/compiler/internal/grammar.y"
       { rule_call_open(&(yyval.call_open).saved_context, &(yyval.call_open).saved_refs); }
#line 3203 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 123: /* function_call: efun_override call_open opt_arg_list ')'  */
#line 603 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_efun(&(yyval.node), (yyvsp[-3].number), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3209 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 124: /* function_call: L_NEW call_open opt_arg_list ')'  */
#line 605 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new(&(yyval.node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3215 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 125: /* function_call: L_NEW '(' L_CLASS L_DEFINED_NAME opt_class_init ')'  */
#line 607 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new_class(&(yyval.node), (yyvsp[-2].ihe), (yyvsp[-1].node)); }
#line 3221 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 126: /* function_call: L_NEW '(' L_CLASS L_IDENTIFIER opt_class_init ')'  */
#line 609 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new_class_undef(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node)); }
#line 3227 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 127: /* function_call: L_DEFINED_NAME call_open opt_arg_list ')'  */
#line 611 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_defined_name(&(yyval.node), (yyvsp[-3].ihe), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3233 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 128: /* function_call: function_name call_open opt_arg_list ')'  */
#line 613 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_name(&(yyval.node), (yyvsp[-3].string), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3239 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 129: /* function_call: primary_expr '[' comma_expr ']' call_open opt_arg_list ')'  */
#line 615 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_indexed(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3245 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 130: /* function_call: primary_expr L_ARROW identifier call_open opt_arg_list ')'  */
#line 617 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_arrow(&(yyval.node), (yyvsp[-5].node), (yyvsp[-3].string), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3251 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 131: /* function_call: '(' '*' comma_expr ')' call_open opt_arg_list ')'  */
#line 619 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_star(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3257 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 132: /* sscanf: L_SSCANF '(' expr ',' expr lvalue_list ')'  */
#line 630 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_sscanf(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 3263 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 133: /* parse_command: L_PARSE_COMMAND '(' expr ',' expr ',' expr lvalue_list ')'  */
#line 636 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_parse_command(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 3269 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 134: /* catch: L_CATCH special_context_start expr_or_block  */
#line 642 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_catch(&(yyval.node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 3275 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 135: /* time_expression: L_TIME_EXPRESSION special_context_start expr_or_block  */
#line 648 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_time_expression(&(yyval.node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 3281 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 136: /* tree: L_TREE block  */
#line 654 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_tree_block(&(yyval.node), (yyvsp[0].decl).node); }
#line 3287 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 137: /* tree: L_TREE '(' comma_expr ')'  */
#line 656 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_tree_expr(&(yyval.node), (yyvsp[-1].node)); }
#line 3293 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 138: /* switch_header: L_SWITCH '(' comma_expr ')'  */
#line 667 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.switch_header).expr = (yyvsp[-1].node); rule_switch_pre(&(yyval.switch_header).saved_context, &(yyval.switch_header).saved_cases_size); }
#line 3299 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 139: /* switch_block: case switch_block  */
#line 672 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_switch_block_case(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3305 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 140: /* switch_block: statement switch_block  */
#line 673 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_switch_block_stmt(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3311 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 141: /* switch_block: %empty  */
#line 674 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { rule_switch_block_empty(&(yyval.node)); }
#line 3317 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 142: /* case: L_CASE case_label ':'  */
#line 681 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_single(&(yyval.node), (yyvsp[-1].node)); }
#line 3323 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 143: /* case: L_CASE case_label L_RANGE case_label ':'  */
#line 683 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3329 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 144: /* case: L_CASE case_label L_RANGE ':'  */
#line 685 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range_from(&(yyval.node), (yyvsp[-2].node)); }
#line 3335 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 145: /* case: L_CASE L_RANGE case_label ':'  */
#line 687 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range_to(&(yyval.node), (yyvsp[-1].node)); }
#line 3341 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 146: /* case: L_DEFAULT ':'  */
#line 689 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_default(&(yyval.node)); }
#line 3347 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 147: /* case_label: constant  */
#line 695 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_case_label_constant(&(yyval.node), (yyvsp[0].number)); }
#line 3353 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 148: /* case_label: string_const  */
#line 696 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_case_label_string(&(yyval.node), (yyvsp[0].string)); }
#line 3359 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 149: /* for_expr: %empty  */
#line 706 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { (yyval.node) = 0; }
#line 3365 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 151: /* for_init: for_expr  */
#line 713 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for_init_expr(&(yyval.decl), (yyvsp[0].node)); }
#line 3371 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 152: /* for_init: single_new_local_def_with_init  */
#line 715 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for_init_local(&(yyval.decl), (yyvsp[0].node)); }
#line 3377 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 153: /* foreach_var: L_DEFINED_NAME  */
#line 720 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_defined(&(yyval.decl), (yyvsp[0].ihe)); }
#line 3383 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 154: /* foreach_var: single_new_local_def  */
#line 721 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_new_local(&(yyval.decl), (yyvsp[0].number)); }
#line 3389 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 155: /* foreach_var: L_IDENTIFIER  */
#line 722 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_identifier(&(yyval.decl), (yyvsp[0].string)); }
#line 3395 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 156: /* foreach_vars: foreach_var  */
#line 728 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach_vars_single(&(yyval.decl), &(yyvsp[0].decl)); }
#line 3401 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 157: /* foreach_vars: foreach_var ',' foreach_var  */
#line 730 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach_vars_double(&(yyval.decl), &(yyvsp[-2].decl), &(yyvsp[0].decl)); }
#line 3407 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 158: /* type_decl: class_header member_list '}'  */
#line 741 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_define_class_members((yyvsp[-2].class_header).ihe, (yyvsp[-2].class_header).classname_index); (yyval.node) = 0; }
#line 3413 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 159: /* class_header: type_modifier_list L_CLASS identifier '{'  */
#line 747 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.class_header).ihe = rule_define_class(&(yyval.class_header).classname_index, (yyvsp[-1].string)); }
#line 3419 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 161: /* $@3: %empty  */
#line 753 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                           { rule_member_list_set_type((yyvsp[0].number)); }
#line 3425 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 165: /* member_name: optional_star identifier  */
#line 765 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { rule_member_name((yyvsp[-1].number), (yyvsp[0].string)); }
#line 3431 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 166: /* class_init: identifier ':' expr  */
#line 770 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                       { (yyval.node) = rule_class_init((yyvsp[-2].string), (yyvsp[0].node)); }
#line 3437 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 167: /* opt_class_init: %empty  */
#line 775 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.node) = rule_opt_class_init_empty(); }
#line 3443 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 168: /* opt_class_init: opt_class_init ',' class_init  */
#line 776 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { (yyval.node) = rule_opt_class_init((yyvsp[-2].node), (yyvsp[0].node)); }
#line 3449 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 169: /* type: type_modifier_list opt_basic_type  */
#line 788 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { (yyval.number) = rule_type((yyvsp[-1].number), (yyvsp[0].number)); }
#line 3455 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 170: /* type_modifier_list: %empty  */
#line 793 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { (yyval.number) = 0; }
#line 3461 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 171: /* type_modifier_list: L_TYPE_MODIFIER type_modifier_list  */
#line 794 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                               { (yyval.number) = rule_type_modifier_list((yyvsp[-1].number), (yyvsp[0].number)); }
#line 3467 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 173: /* opt_basic_type: %empty  */
#line 800 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { (yyval.number) = rule_opt_basic_type_empty(); }
#line 3473 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 175: /* basic_type: opt_atomic_type L_ARRAY  */
#line 806 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { (yyval.number) = (yyvsp[-1].number) | TYPE_MOD_ARRAY; }
#line 3479 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 177: /* opt_atomic_type: %empty  */
#line 812 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { (yyval.number) = rule_opt_basic_type_empty(); }
#line 3485 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 179: /* atomic_type: L_CLASS L_DEFINED_NAME  */
#line 818 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.number) = rule_atomic_type_class((yyvsp[0].ihe)); }
#line 3491 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 180: /* atomic_type: L_CLASS L_IDENTIFIER  */
#line 819 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.number) = rule_atomic_type_class_identifier((yyvsp[0].string)); }
#line 3497 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 182: /* arg_type: basic_type ref  */
#line 825 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.number) = (yyvsp[-1].number) | LOCAL_MOD_REF; }
#line 3503 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 183: /* cast: '(' basic_type optional_star ')'  */
#line 830 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { (yyval.number) = rule_cast((yyvsp[-2].number), (yyvsp[-1].number)); }
#line 3509 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 184: /* optional_star: %empty  */
#line 835 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.number) = 0; }
#line 3515 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 185: /* optional_star: '*'  */
#line 836 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.number) = TYPE_MOD_ARRAY; }
#line 3521 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 186: /* identifier: L_DEFINED_NAME  */
#line 846 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { (yyval.string) = rule_identifier_defined_name((yyvsp[0].ihe)); }
#line 3527 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 189: /* new_local_name: L_DEFINED_NAME  */
#line 853 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.string) = rule_new_local_name_redefine((yyvsp[0].ihe)); }
#line 3533 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 190: /* efun_override: L_EFUN L_COLON_COLON identifier  */
#line 858 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.number) = rule_efun_override((yyvsp[0].string)); }
#line 3539 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 191: /* efun_override: L_EFUN L_COLON_COLON L_NEW  */
#line 859 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.number) = rule_efun_override_new(); }
#line 3545 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 193: /* function_name: L_COLON_COLON identifier  */
#line 870 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { (yyval.string) = rule_function_name_colon_colon((yyvsp[0].string)); }
#line 3551 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 194: /* function_name: L_BASIC_TYPE L_COLON_COLON identifier  */
#line 871 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { (yyval.string) = rule_function_name_type((yyvsp[-2].number), (yyvsp[0].string)); }
#line 3557 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 195: /* function_name: identifier L_COLON_COLON identifier  */
#line 872 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                    { (yyval.string) = rule_function_name_obj((yyvsp[-2].string), (yyvsp[0].string)); }
#line 3563 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 196: /* inheritance: type_modifier_list L_INHERIT string_const ';'  */
#line 878 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { if (rule_inheritence(&(yyval.node), (yyvsp[-3].number), (yyvsp[-1].string))) { YYACCEPT; } }
#line 3569 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 197: /* modifier_change: type_modifier_list ':'  */
#line 883 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { (yyval.node) = rule_modifier_change((yyvsp[-1].number)); }
#line 3575 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 198: /* argument: %empty  */
#line 894 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.argument).num_arg = 0; (yyval.argument).flags = 0; }
#line 3581 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 200: /* argument: argument_list L_DOT_DOT_DOT  */
#line 897 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_varargs(&(yyval.argument), &(yyvsp[-1].argument)); }
#line 3587 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 201: /* argument_list: param_decl  */
#line 903 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_list_single(&(yyval.argument), (yyvsp[0].number)); }
#line 3593 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 202: /* argument_list: argument_list ',' param_decl  */
#line 905 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_list_multi(&(yyval.argument), &(yyvsp[-2].argument), (yyvsp[0].number)); }
#line 3599 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 203: /* param_decl: arg_type optional_star  */
#line 915 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_typed((yyvsp[-1].number) | (yyvsp[0].number)); }
#line 3605 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 204: /* param_decl: arg_type optional_star new_local_name optional_default_arg_value  */
#line 917 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_typed_name((yyvsp[-3].number) | (yyvsp[-2].number), (yyvsp[-1].string), (yyvsp[0].node)); }
#line 3611 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 205: /* param_decl: new_local_name  */
#line 919 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_untyped_name((yyvsp[0].string)); }
#line 3617 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 206: /* optional_default_arg_value: %empty  */
#line 924 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                          { (yyval.node) = 0; }
#line 3623 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 207: /* optional_default_arg_value: ':' L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 925 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                            { (yyval.node) = rule_default_arg_value((yyvsp[-2].node)); }
#line 3629 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 208: /* local_name_list: new_local_def  */
#line 931 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_name_list_single(&(yyval.decl), (yyvsp[0].node)); }
#line 3635 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 209: /* local_name_list: new_local_def ',' local_name_list  */
#line 933 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_name_list_multi(&(yyval.decl), (yyvsp[-2].node), &(yyvsp[0].decl)); }
#line 3641 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 210: /* new_local_def: optional_star new_local_name  */
#line 939 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_new_local_def((yyvsp[0].string), (yyvsp[-1].number)); }
#line 3647 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 211: /* new_local_def: optional_star new_local_name L_ASSIGN expr  */
#line 941 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_new_local_def_with_init((yyvsp[-2].string), (yyvsp[-3].number), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3653 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 212: /* single_new_local_def: arg_type optional_star new_local_name  */
#line 947 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_single_new_local_def(&(yyval.number), (yyvsp[-2].number) | (yyvsp[-1].number), (yyvsp[0].string)); }
#line 3659 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 213: /* single_new_local_def_with_init: single_new_local_def L_ASSIGN expr  */
#line 953 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_single_new_local_def_with_init((yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3665 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 216: /* new_name: optional_star identifier  */
#line 969 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { rule_new_name((yyvsp[-1].number), (yyvsp[0].string)); }
#line 3671 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 217: /* new_name: optional_star identifier L_ASSIGN expr  */
#line 970 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { rule_new_name_with_init((yyvsp[-3].number), (yyvsp[-2].string), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3677 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 218: /* lambda_return_type: L_BASIC_TYPE  */
#line 981 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                { rule_lambda_return_type(&(yyval.func_block), (yyvsp[0].number)); }
#line 3683 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 219: /* functional_open: L_FUNCTION_OPEN efun_override  */
#line 989 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                 { (yyval.number) = rule_functional_open((yyvsp[0].number)); }
#line 3689 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 220: /* assoc_pair: expr ':' expr  */
#line 999 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_assoc_pair(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3695 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 221: /* spread_expr: expr  */
#line 1004 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_spread_expr_normal(&(yyval.node), (yyvsp[0].node)); }
#line 3701 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 222: /* spread_expr: expr L_DOT_DOT_DOT  */
#line 1005 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_spread_expr_dots(&(yyval.node), (yyvsp[-1].node)); }
#line 3707 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 223: /* arg_list: spread_expr  */
#line 1011 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_arg_list_single(&(yyval.node), (yyvsp[0].node)); }
#line 3713 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 224: /* arg_list: arg_list ',' spread_expr  */
#line 1013 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_arg_list_multi(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3719 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 225: /* opt_arg_list: %empty  */
#line 1019 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list_empty(&(yyval.node)); }
#line 3725 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 226: /* opt_arg_list: arg_list  */
#line 1020 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list(&(yyval.node), (yyvsp[0].node)); }
#line 3731 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 227: /* opt_arg_list: arg_list ','  */
#line 1021 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list(&(yyval.node), (yyvsp[-1].node)); }
#line 3737 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 228: /* pair_list: assoc_pair  */
#line 1027 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_pair_list_single(&(yyval.node), (yyvsp[0].node)); }
#line 3743 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 229: /* pair_list: pair_list ',' assoc_pair  */
#line 1029 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_pair_list_multi(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3749 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 230: /* opt_pair_list: %empty  */
#line 1035 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list_empty(&(yyval.node)); }
#line 3755 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 231: /* opt_pair_list: pair_list  */
#line 1036 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list(&(yyval.node), (yyvsp[0].node)); }
#line 3761 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 232: /* opt_pair_list: pair_list ','  */
#line 1037 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list(&(yyval.node), (yyvsp[-1].node)); }
#line 3767 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 233: /* lvalue_list: %empty  */
#line 1042 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.node) = rule_lvalue_list_empty(); }
#line 3773 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 234: /* lvalue_list: ',' lvalue lvalue_list  */
#line 1043 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { (yyval.node) = rule_lvalue_list((yyvsp[-1].node), (yyvsp[0].node)); }
#line 3779 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 235: /* expr_or_block: block  */
#line 1048 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_expr_or_block_block((yyvsp[0].decl)); }
#line 3785 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 236: /* expr_or_block: '(' comma_expr ')'  */
#line 1049 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_expr_or_block_expr((yyvsp[-1].node)); }
#line 3791 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 238: /* string: string_literal  */
#line 1064 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_string(&(yyval.node), (yyvsp[0].string)); }
#line 3797 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 239: /* template_literal: L_TEMPLATE_HEAD expr template_parts  */
#line 1076 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_template_literal(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3803 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 240: /* template_parts: L_TEMPLATE_TAIL  */
#line 1080 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                      { rule_template_parts_tail(&(yyval.node), (yyvsp[0].string)); }
#line 3809 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 241: /* template_parts: L_TEMPLATE_MIDDLE expr template_parts  */
#line 1081 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                       { rule_template_parts_middle(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3815 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 244: /* string_like: string_like string  */
#line 1092 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                               { rule_string_like_concat(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3821 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 245: /* string_like: string_like template_literal  */
#line 1093 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_string_like_concat(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3827 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 247: /* string_literal: string_literal L_STRING  */
#line 1101 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.string) = rule_string_literal_concat((yyvsp[-1].string), (yyvsp[0].string)); }
#line 3833 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 249: /* string_const: '(' string_const ')'  */
#line 1108 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                        { (yyval.string) = (yyvsp[-1].string); }
#line 3839 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 250: /* string_const: string_const '+' string_const  */
#line 1109 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { (yyval.string) = rule_string_literal_concat((yyvsp[-2].string), (yyvsp[0].string)); }
#line 3845 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 251: /* number: L_NUMBER  */
#line 1113 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { rule_number(&(yyval.node), (yyvsp[0].number)); }
#line 3851 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 252: /* real: L_REAL  */
#line 1117 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { rule_real(&(yyval.node), (yyvsp[0].real)); }
#line 3857 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 253: /* constant: constant '|' constant  */
#line 1123 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_or(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3863 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 254: /* constant: constant '^' constant  */
#line 1124 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_xor(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3869 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 255: /* constant: constant '&' constant  */
#line 1125 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_and(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3875 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 256: /* constant: constant L_EQ_NE constant  */
#line 1126 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                             { rule_constant_eq_ne(&(yyval.number), (yyvsp[-1].number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3881 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 257: /* constant: constant L_ORDER constant  */
#line 1127 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { rule_constant_order(&(yyval.number), (yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].number)); }
#line 3887 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 258: /* constant: constant '<' constant  */
#line 1128 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_lt(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3893 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 259: /* constant: constant L_SHIFT constant  */
#line 1129 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                             { rule_constant_shift(&(yyval.number), (yyvsp[-1].number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3899 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 260: /* constant: constant '+' constant  */
#line 1130 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_add(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3905 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 261: /* constant: constant '-' constant  */
#line 1131 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_sub(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3911 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 262: /* constant: constant '*' constant  */
#line 1132 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_mul(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3917 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 263: /* constant: constant '%' constant  */
#line 1133 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_mod(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3923 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 264: /* constant: constant '/' constant  */
#line 1134 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_div(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3929 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 265: /* constant: '(' constant ')'  */
#line 1135 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { (yyval.number) = (yyvsp[-1].number); }
#line 3935 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 266: /* constant: L_NUMBER  */
#line 1136 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { (yyval.number) = (yyvsp[0].number); }
#line 3941 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 267: /* constant: '-' L_NUMBER  */
#line 1137 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_neg(&(yyval.number), (yyvsp[0].number)); }
#line 3947 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 268: /* constant: '!' L_NUMBER  */
#line 1138 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_not(&(yyval.number), (yyvsp[0].number)); }
#line 3953 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 269: /* constant: '~' L_NUMBER  */
#line 1139 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_compl(&(yyval.number), (yyvsp[0].number)); }
#line 3959 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;


#line 3963 "$REPO_ROOT$/build/src/grammar.autogen.cc"

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
  *++yylsp = yyloc;

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
          = {yyps, yytoken, &yylloc};
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
        yyerror (&yylloc, yyscanner, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  yyerror_range[1] = yylloc;
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
                      yytoken, &yylval, &yylloc, yyscanner);
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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp, yyscanner);
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

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

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
  yyerror (&yylloc, yyscanner, YY_("memory exhausted"));
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
                  yytoken, &yylval, &yylloc, yyscanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp, yyscanner);
      YYPOPSTACK (1);
    }
  yyps->yynew = 2;
  goto yypushreturn;


/*-------------------------.
| yypushreturn -- return.  |
`-------------------------*/
yypushreturn:
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}
#undef yynerrs
#undef yystate
#undef yyerrstatus
#undef yyssa
#undef yyss
#undef yyssp
#undef yyvsa
#undef yyvs
#undef yyvsp
#undef yylsa
#undef yyls
#undef yylsp
#undef yystacksize
#undef yyesa
#undef yyes
#undef yyes_capacity
#line 1142 "$REPO_ROOT$/src/compiler/internal/grammar.y"

