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
#line 39 "$REPO_ROOT$/src/compiler/internal/grammar.y"

#include "base/std.h"
#include "compiler/internal/grammar_rules.h"

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

#line 93 "$BUILD_ROOT$/src/grammar.autogen.cc"

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
#define YYLAST   2078

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  77
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  101
/* YYNRULES -- Number of rules.  */
#define YYNRULES  270
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  531

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
       0,   243,   243,   244,   245,   246,   247,   256,   261,   262,
     267,   268,   279,   280,   281,   282,   283,   306,   308,   305,
     315,   316,   317,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,   344,   345,   350,   355,   356,
     361,   367,   373,   379,   385,   397,   403,   405,   407,   409,
     415,   419,   425,   429,   431,   450,   451,   455,   456,   457,
     459,   460,   461,   462,   464,   465,   466,   468,   469,   470,
     472,   474,   481,   482,   483,   484,   486,   487,   488,   489,
     490,   492,   494,   495,   496,   497,   498,   499,   504,   513,
     514,   515,   516,   517,   518,   519,   520,   523,   527,   528,
     533,   535,   537,   542,   544,   546,   548,   550,   552,   554,
     556,   558,   561,   563,   568,   575,   577,   586,   588,   590,
     594,   595,   605,   618,   620,   622,   624,   626,   628,   630,
     632,   634,   645,   651,   657,   663,   669,   671,   682,   688,
     689,   690,   696,   698,   700,   702,   704,   711,   712,   722,
     723,   728,   730,   736,   737,   738,   743,   745,   756,   762,
     768,   769,   769,   775,   776,   781,   786,   791,   792,   804,
     809,   810,   815,   816,   821,   822,   827,   828,   833,   834,
     835,   840,   841,   846,   851,   852,   862,   863,   868,   869,
     874,   875,   885,   886,   887,   888,   893,   899,   909,   911,
     912,   918,   920,   930,   932,   934,   940,   941,   946,   948,
     954,   956,   962,   968,   979,   980,   985,   986,   997,  1005,
    1015,  1020,  1021,  1026,  1028,  1035,  1036,  1037,  1042,  1044,
    1051,  1052,  1053,  1058,  1059,  1064,  1065,  1070,  1071,  1081,
    1093,  1097,  1098,  1107,  1108,  1109,  1110,  1117,  1118,  1124,
    1125,  1126,  1130,  1134,  1140,  1141,  1142,  1143,  1144,  1145,
    1146,  1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,
    1156
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

#define YYPACT_NINF (-410)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-233)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -410,    65,   123,  -410,   118,    68,  -410,  -410,  -410,    84,
      59,  -410,  -410,  -410,  -410,  -410,    29,  -410,   213,   147,
     160,  -410,     3,   229,  -410,  -410,  -410,   204,   227,   361,
    -410,  -410,  -410,  -410,    39,  -410,    84,  -410,     3,   283,
     155,   248,   267,   275,  -410,  -410,  -410,    84,  1635,   342,
     213,  -410,    71,  -410,     3,  -410,  -410,   317,   349,   213,
     396,  -410,  -410,  1635,   389,     0,   109,   398,   279,   213,
    1700,   385,  -410,  -410,   391,  -410,   392,  -410,   -46,  -410,
    1635,  1635,  1635,   714,   400,  2015,   373,   182,  -410,  -410,
    -410,  -410,  -410,  -410,  1635,   401,   407,   407,   413,   322,
     279,  -410,  -410,   113,   283,  -410,  -410,    13,   466,  -410,
    -410,  -410,    84,  -410,  1635,  1829,   213,  -410,  1280,   103,
     787,  -410,   164,  -410,   270,   341,  2015,   407,  1635,   -26,
    1635,   -26,   435,  1280,  1635,  -410,  -410,  -410,  -410,  -410,
     149,  1635,  1351,   566,   132,    84,  -410,  1635,  1635,  1635,
    1635,  1635,  1635,  1635,  1635,  1635,  1635,  1635,  1635,  1635,
    1635,  1635,  1635,  -410,  1635,   213,    17,   415,   213,  1144,
    -410,   213,  1280,  1280,    13,  1635,   421,  -410,  -410,  -410,
    -410,  -410,    -2,    84,  -410,   424,     6,  -410,  -410,  2015,
    1635,  -410,  -410,  -410,  2002,  -410,   428,   432,  -410,  -410,
    1635,   434,  1635,   464,  1887,  1635,  -410,  -410,  1906,  -410,
     404,   465,   141,   860,   212,   471,  1839,  -410,   477,   461,
    -410,   483,  1635,   452,   682,   608,   288,   458,   192,  1858,
     340,   534,   615,   192,   199,   199,  -410,  -410,  -410,  2015,
     407,  1635,  -410,  1635,  -410,   500,  1635,    42,  -410,   495,
     496,   498,   346,  -410,  -410,   414,  -410,  -410,   218,  1829,
    -410,  1209,  -410,   379,  -410,  2015,  -410,  1635,   216,  1635,
    -410,  -410,  -410,  -410,    30,   506,   507,   509,  -410,   514,
     523,   540,   545,  1422,  -410,   860,  -410,  -410,  -410,  -410,
    -410,  -410,  -410,  -410,   537,    84,   860,   176,   546,  -410,
     407,   551,  1635,   640,   552,  -410,   272,  1635,  1280,   187,
     200,  1635,   108,   418,   407,  -410,  -410,   553,  1635,   564,
     549,    25,  -410,  -410,  -410,   569,  1935,  -410,  1954,   285,
     286,   860,  1635,  1635,  1635,  1002,  1073,   224,  -410,  -410,
    -410,   208,  -410,  -410,   414,   572,   576,  -410,  -410,  -410,
    1280,  -410,  2015,  -410,  -410,  -410,  2015,   580,  -410,  -410,
     243,   492,  -410,  1635,  -410,   249,  1280,  -410,  -410,   598,
    -410,  -410,  -410,  -410,  -410,  -410,   279,   582,  1635,  -410,
     213,  -410,  -410,   299,   300,   313,   626,   585,  -410,   591,
      84,   643,  -410,  -410,  -410,   588,   627,  -410,  -410,   646,
    -410,    84,   168,   594,  -410,  -410,  1635,  -410,   256,  1758,
     257,  -410,   604,  1635,   609,  -410,  1935,  -410,   613,  1002,
    -410,  -410,   614,  1493,   414,  1635,   224,  1635,  1635,  -410,
      45,   617,    84,   931,  -410,  -410,   263,  1768,  -410,  -410,
    -410,  -410,   384,  -410,   625,  1635,   671,  1002,  1635,   629,
    -410,  2015,  -410,  1983,  2015,  -410,    82,   692,   695,   702,
      82,    26,   652,   467,  -410,   648,   931,   642,   931,  -410,
    -410,   653,  -410,  2015,  1002,  -410,  -410,   314,  1564,  -410,
     650,  -410,  -410,  -410,   386,    60,  -410,    15,    15,    15,
      15,    15,    15,    15,    15,    15,    15,    15,    15,  -410,
    -410,  -410,  -410,  -410,  -410,   658,   661,  1002,  -410,  -410,
    -410,   659,    15,   303,   748,   316,   526,   541,   689,   316,
     329,   329,  -410,  -410,  -410,  -410,  -410,  -410,  -410,  1002,
    -410
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       9,     0,   170,     1,   170,    10,    12,    15,   160,   184,
     173,    14,    16,   171,    11,     8,   177,   185,     0,     0,
     214,   178,     0,     0,   197,   169,   172,     0,   174,     0,
     158,   161,   186,   187,   216,    13,   184,   247,     0,   249,
       0,   179,   180,     0,   175,   179,   180,   184,     0,     0,
       0,   215,     0,   248,     0,   196,   159,     0,   163,     0,
       0,   252,   253,     0,   218,    90,    91,     0,     0,     0,
       0,     0,     5,   237,     0,     5,     0,    92,     0,   238,
       0,     0,     0,     0,     0,   217,     0,    82,    89,    83,
      84,    95,    85,    96,     0,     0,     0,     0,     0,     0,
       0,   243,   244,    93,   239,    86,    87,   177,   216,   250,
     251,   162,   184,   165,     0,     0,     0,   122,     0,     0,
       0,    77,    88,   193,    90,     0,    55,   219,     0,     0,
       0,     0,   122,     0,     0,     4,   136,    80,    78,    79,
     178,     0,     0,     0,     0,   184,     6,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    81,     0,     0,     0,     0,     0,     0,
      76,     0,     0,     0,   177,     0,     0,    57,   245,   246,
     189,   188,   181,   184,   205,     0,   199,   201,   164,    59,
       0,   241,   240,   194,   221,   223,   226,     0,   191,   190,
       0,     0,     0,     0,     0,     0,   235,   134,     0,   135,
       0,     0,     0,     0,     0,     0,     0,   228,   231,     0,
      94,     0,     0,    63,    62,    61,    70,    67,    68,     0,
      64,    65,    66,    69,    71,    72,    73,    74,    75,    58,
      98,     0,   100,     0,    99,     0,     0,     0,   195,     0,
       0,     0,     0,   115,   182,   203,    18,   200,   177,     0,
     222,     0,   127,     0,   117,    56,   119,     0,     0,     0,
     167,   167,   124,   137,     0,     0,     0,     0,     2,     0,
       0,     0,     0,     0,    32,     0,    30,    24,    25,    26,
      27,    28,    29,    31,     0,   184,     0,     0,     0,    50,
       0,     0,     0,     0,     0,   183,     0,     0,     0,     0,
       0,     0,     0,     0,   103,   123,   128,     0,     0,     0,
     206,     0,   202,   242,   224,     0,   233,   236,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   177,    33,    34,
      35,     0,    47,    45,     0,     0,   208,    48,    23,    53,
       0,   121,   220,   229,   120,    97,    60,     0,   101,   102,
       0,     0,   104,     0,   109,     0,     0,   114,   116,     0,
     204,    22,    21,    19,    20,   118,     0,     0,     0,   125,
       0,   126,    49,     0,     0,     0,     0,   150,   151,     0,
     184,     0,   152,   153,   155,   156,     0,   154,    36,   210,
      51,   184,   177,     0,   130,   111,     0,   110,     0,    55,
       0,   105,     0,     0,   233,   132,   233,   168,     0,     0,
     138,     2,     0,     0,     0,     0,   177,     0,     0,   209,
       0,     0,   184,     0,    52,   131,     0,    55,   106,   113,
     108,   129,     0,   234,     0,     0,    38,     0,     0,     0,
     212,   213,   157,     0,   211,   267,     0,     0,     0,     0,
       0,     0,   148,   147,   146,     0,     0,     0,     0,   107,
     112,     0,   133,   166,     0,    37,    40,     0,     0,     3,
       0,   268,   269,   270,     0,     0,   142,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    54,
     140,    44,   139,   207,    39,     0,     0,     0,   145,   266,
     144,     0,     0,   260,   257,   258,   254,   255,   256,   259,
     261,   262,   263,   264,   265,    41,     2,    43,   143,     0,
      42
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -410,  -409,  -410,  -410,   655,  -410,  -410,  -410,  -410,  -410,
    -410,  -410,  -410,  -410,  -248,  -410,  -410,  -410,  -410,  -410,
    -410,  -410,  -410,   -71,   -94,  -410,  -410,  -410,  -410,   -68,
       8,   -64,   -63,   -66,  -410,  -410,  -410,  -410,  -410,  -410,
    -410,  -172,   330,  -356,  -383,  -410,   307,  -410,  -410,  -410,
    -410,  -410,   622,  -410,  -410,   475,  -410,   731,  -410,    -7,
    -410,  -410,  -323,  -410,    -1,    24,  -237,   666,  -410,  -410,
    -410,   581,  -410,   510,  -410,  -280,  -410,   436,  -410,   718,
    -410,  -410,  -410,   470,  -234,    32,  -122,  -410,  -410,  -357,
     639,   595,   673,   677,   525,  -410,   -22,   -21,  -410,  -410,
    -190
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,   335,   507,   213,   129,   222,     1,     2,    15,     5,
       6,    49,   321,   373,   285,   286,   287,   475,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   432,   402,   297,
     126,    86,    87,   118,    88,    89,    90,    91,    92,    93,
     298,   467,   468,   461,   388,   389,   395,   396,     7,     8,
      16,    47,    57,    58,   417,   329,     9,    10,    25,   182,
      27,    28,   183,    94,   344,    95,   184,    96,    97,    11,
      12,   185,   186,   187,   370,   345,   346,   397,   392,    19,
      20,    98,    99,   217,   195,   196,   197,   218,   219,   377,
     207,   100,   101,   102,   192,   103,   104,   462,   105,   106,
     463
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      39,    40,   125,    26,   121,   122,    37,   136,    18,    31,
     133,   211,   447,   390,   390,   144,    39,    52,   320,   455,
     215,   134,    21,   135,   180,   181,   371,   324,    32,    33,
     172,   173,    39,   110,   257,    50,   177,   122,    21,  -186,
     449,   205,    34,   135,    73,   114,    59,    43,    37,   455,
     249,   250,   144,   485,    48,    79,    85,   443,   206,   444,
     206,   172,    29,    37,   455,     3,   212,   117,    21,   313,
      38,   115,   456,   214,   108,   457,   145,   258,    29,   458,
     459,  -198,   512,   113,   324,    37,   455,   386,   137,   138,
     139,   372,   241,   123,   135,   506,   331,    22,   486,    30,
     480,   247,   170,   390,  -177,   457,   -17,   399,    23,   458,
     459,    59,   460,   202,    32,    33,    37,   529,   314,    63,
     457,   429,   189,    -7,   458,   459,   194,   460,     4,   511,
      54,    24,   510,     4,    14,   361,   204,   268,   208,   109,
     193,   194,   457,   199,   221,    17,   458,   459,  -187,   460,
     194,   216,   465,   198,   306,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   446,   239,   309,   308,   310,  -192,    21,   312,   202,
     194,   194,   255,   194,   362,   466,   357,   450,   116,   240,
     242,   342,   244,   430,   431,   248,   -88,   -88,   259,   476,
     220,   165,   347,   202,   166,   167,   299,   252,   194,   273,
     265,   150,   202,    35,    54,   341,  -218,    29,   466,   165,
     466,    55,   166,   167,    32,    33,   504,    21,   403,   180,
     181,    36,   263,    21,   350,   393,   394,   382,   168,   169,
      41,    42,   348,   360,   412,   365,   367,   202,   366,    44,
     374,   158,   159,   160,   161,   162,   168,   169,   202,   527,
     160,   161,   162,   358,   383,   384,   385,    29,   387,   194,
     484,   202,  -176,    29,   398,   326,   359,   328,   299,   202,
     300,   530,    37,   202,   327,    63,    53,   202,    64,   299,
      65,    66,    67,   408,   500,   410,   502,   513,   514,   515,
     516,   517,   518,   519,   520,   521,   522,   523,   524,  -186,
     352,   216,   414,   122,   202,   356,   194,  -186,    69,   405,
     202,    70,   484,    72,   299,   411,   194,   202,   202,    76,
      77,    78,   438,   440,   202,   487,  -187,   117,   436,   469,
     355,   200,   201,   202,    56,   442,   120,   158,   159,   160,
     161,   162,    84,   379,   381,   387,   380,   380,   194,   150,
     151,   152,   494,   495,   496,   497,   498,   419,   420,   409,
     202,   202,    45,    46,   194,   494,   495,   496,   497,   498,
     477,   421,   505,   111,   202,   202,   416,   163,   164,   424,
     496,   497,   498,   175,   176,   434,   155,   156,   157,   158,
     159,   160,   161,   162,   418,   487,   488,   489,    39,   107,
     387,   114,   202,   203,   437,   270,   271,   318,   319,    60,
     112,    37,    61,    62,    63,   180,   181,    64,   116,    65,
      66,    67,    68,   451,    39,   453,   454,   119,    39,    52,
     171,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     318,   325,   128,   473,   509,   202,   471,    69,   130,   132,
      70,    71,    72,    39,    73,    74,    75,   146,    76,    77,
      78,   150,   151,   152,   117,    79,   363,   150,    80,   152,
     174,    48,    81,    82,   210,    83,   487,   488,   489,   253,
     243,    84,   256,    60,   364,    37,    61,    62,    63,   261,
     262,    64,   264,    65,    66,    67,    68,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   157,   158,   159,   160,
     161,   162,   490,   491,   492,   493,   494,   495,   496,   497,
     498,    69,   266,   272,    70,    71,    72,   304,    73,    74,
      75,   301,    76,    77,    78,   487,   488,   489,   303,    79,
     406,   305,    80,   150,   151,   152,    81,    82,   311,    83,
     487,   488,   489,   315,   316,    84,   317,    60,   407,    37,
      61,    62,    63,   332,   333,    64,   334,    65,    66,    67,
      68,   336,   491,   492,   493,   494,   495,   496,   497,   498,
     337,   156,   157,   158,   159,   160,   161,   162,   492,   493,
     494,   495,   496,   497,   498,    69,   338,   343,    70,    71,
      72,   339,    73,    74,    75,   349,    76,    77,    78,   351,
     354,   369,   135,    79,   147,   148,    80,   150,   151,   152,
      81,    82,   368,    83,   150,   151,   152,   375,   400,    84,
     413,    60,  -230,    37,    61,    62,    63,   401,   404,    64,
     415,    65,    66,    67,    68,   422,   202,   423,   425,   426,
     427,   428,   435,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   441,   157,   158,   159,   160,   161,   162,    69,
     376,   448,    70,    71,    72,   445,    73,    74,    75,   464,
      76,    77,    78,   472,   474,   478,   481,    79,   147,   482,
      80,   150,   151,   152,    81,    82,   483,    83,   487,   488,
     489,    54,   501,    84,   499,    60,  -232,    37,    61,    62,
      63,   503,   508,   140,   525,    65,    66,    67,    68,   526,
     131,   528,   433,   452,   188,    13,   127,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   330,   493,   494,   495,
     496,   497,   498,    69,    51,   251,    70,    71,    72,  -177,
      73,    74,    75,    29,    76,    77,    78,   487,   322,   489,
     209,    79,   391,   353,    80,   141,   178,   254,    81,    82,
     179,    83,     0,   142,   323,     0,     0,    84,    60,   143,
      37,    61,    62,    63,     0,     0,    64,     0,    65,    66,
      67,    68,     0,     0,     0,     0,   493,   494,   495,   496,
     497,   498,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    69,     0,     0,    70,
      71,    72,     0,    73,    74,    75,     0,    76,    77,    78,
       0,     0,     0,     0,    79,     0,     0,    80,   141,     0,
       0,    81,    82,     0,    83,     0,   142,     0,     0,     0,
      84,   274,   143,    37,    61,    62,    63,     0,     0,   140,
       0,    65,    66,    67,    68,     0,     0,     0,     0,     0,
       0,     0,   275,     0,   276,     0,     0,     0,     0,   277,
     278,   279,   280,     0,   281,   282,   283,     0,     0,    69,
       0,     0,    70,    71,    72,  -177,    73,    74,    75,    29,
      76,    77,    78,     0,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,    81,    82,   284,    83,     0,   135,
     -46,     0,    60,    84,    37,    61,    62,    63,     0,     0,
      64,     0,    65,    66,    67,    68,     0,     0,     0,     0,
       0,     0,     0,   275,     0,   276,   430,   431,     0,     0,
     277,   278,   279,   280,     0,   281,   282,   283,     0,     0,
      69,     0,     0,    70,    71,    72,     0,    73,    74,    75,
       0,    76,    77,    78,     0,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,    81,    82,   284,    83,     0,
     135,  -141,     0,    60,    84,    37,    61,    62,    63,     0,
       0,    64,     0,    65,    66,    67,    68,     0,     0,     0,
       0,     0,     0,     0,   275,     0,   276,     0,     0,     0,
       0,   277,   278,   279,   280,     0,   281,   282,   283,     0,
       0,    69,     0,     0,    70,    71,    72,     0,    73,    74,
      75,     0,    76,    77,    78,     0,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,    81,    82,   284,    83,
       0,   135,     0,     0,    60,    84,    37,    61,    62,    63,
       0,     0,   140,     0,    65,    66,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,     0,     0,    70,    71,    72,  -177,    73,
      74,    75,    29,    76,    77,    78,     0,     0,     0,     0,
      79,     0,     0,    80,     0,     0,     0,    81,    82,  -149,
      83,     0,     0,     0,     0,    60,    84,    37,    61,    62,
      63,     0,     0,    64,     0,    65,    66,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   245,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    69,     0,     0,    70,    71,    72,     0,
      73,    74,    75,     0,    76,    77,    78,     0,     0,     0,
       0,    79,   246,     0,    80,     0,     0,     0,    81,    82,
      60,    83,    37,    61,    62,    63,     0,    84,    64,     0,
      65,    66,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    69,     0,
       0,    70,    71,    72,     0,    73,    74,    75,     0,    76,
      77,    78,     0,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,    81,    82,     0,    83,  -227,     0,  -227,
       0,    60,    84,    37,    61,    62,    63,     0,     0,    64,
       0,    65,    66,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    69,
       0,     0,    70,    71,    72,     0,    73,    74,    75,     0,
      76,    77,    78,     0,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,    81,    82,     0,    83,  -225,     0,
       0,     0,    60,    84,    37,    61,    62,    63,     0,     0,
      64,     0,    65,    66,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      69,     0,     0,    70,    71,    72,     0,    73,    74,    75,
       0,    76,    77,    78,     0,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,    81,    82,     0,    83,     0,
       0,  -225,     0,    60,    84,    37,    61,    62,    63,     0,
       0,    64,     0,    65,    66,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    69,     0,     0,    70,    71,    72,     0,    73,    74,
      75,     0,    76,    77,    78,     0,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,    81,    82,   340,    83,
       0,     0,     0,     0,    60,    84,    37,    61,    62,    63,
       0,     0,    64,     0,    65,    66,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,     0,     0,    70,    71,    72,     0,    73,
      74,    75,     0,    76,    77,    78,     0,     0,     0,     0,
      79,     0,     0,    80,     0,     0,     0,    81,    82,  -149,
      83,     0,     0,     0,     0,    60,    84,    37,    61,    62,
      63,     0,     0,    64,     0,    65,    66,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    69,     0,     0,    70,    71,    72,     0,
      73,    74,    75,     0,    76,    77,    78,     0,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,    81,    82,
       0,    83,  -149,     0,     0,     0,    60,    84,    37,    61,
      62,    63,     0,     0,    64,     0,    65,    66,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    69,     0,     0,    70,    71,    72,
       0,    73,    74,    75,     0,    76,    77,    78,     0,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,    81,
      82,    60,    83,    37,    61,    62,    63,     0,    84,    64,
       0,   124,    66,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    69,
       0,     0,    70,    71,    72,     0,    73,    74,    75,     0,
      76,    77,    78,     0,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,    81,    82,     0,    83,     0,     0,
       0,     0,     0,    84,   147,   148,   149,   150,   151,   152,
       0,     0,     0,     0,   147,   148,   149,   150,   151,   152,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,     0,     0,   439,     0,   190,   191,     0,     0,
       0,     0,     0,     0,   470,   147,   148,   149,   150,   151,
     152,     0,     0,     0,     0,   147,   148,   149,   150,   151,
     152,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   147,   148,   149,   150,   151,   152,
       0,     0,     0,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   147,   148,   149,   150,   151,   152,     0,
       0,   302,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   147,   148,   149,   150,   151,   152,     0,     0,
     307,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   147,   148,   149,   150,   151,   152,     0,   267,     0,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     147,   148,   149,   150,   151,   152,     0,   269,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   147,
     148,   149,   150,   151,   152,     0,   376,     0,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   147,   148,
     149,   150,   151,   152,     0,   378,     0,     0,     0,     0,
     260,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,     0,     0,     0,
       0,   479,     0,     0,     0,     0,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,     0,     0,     0,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162
};

static const yytype_int16 yycheck[] =
{
      22,    22,    70,    10,    68,    68,     3,    78,     9,    16,
      76,   133,   421,   336,   337,    83,    38,    38,   255,     4,
     142,    67,     9,    69,    11,    12,     1,   261,    11,    12,
      96,    97,    54,    54,    28,    36,   100,   100,     9,    39,
     423,    67,    18,    69,    46,    15,    47,    23,     3,     4,
     172,   173,   120,    27,    15,    57,    48,   414,   129,   416,
     131,   127,    49,     3,     4,     0,   134,    67,     9,    27,
      67,    63,    27,   141,    50,    60,    83,    71,    49,    64,
      65,    68,    67,    59,   318,     3,     4,   335,    80,    81,
      82,    66,    75,    69,    69,   478,    66,    38,    72,    70,
     456,   169,    94,   426,    45,    60,    67,   344,    49,    64,
      65,   112,    67,    71,    11,    12,     3,   526,    76,     6,
      60,   401,   114,     0,    64,    65,   118,    67,    10,   485,
      59,    72,    72,    10,    66,    27,   128,   205,   130,    68,
     116,   133,    60,   119,   145,    61,    64,    65,    39,    67,
     142,   143,   432,    50,   222,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   419,   164,   241,   240,   243,    67,     9,   246,    71,
     172,   173,   183,   175,    76,   433,   308,   424,    39,   165,
     166,   285,   168,    25,    26,   171,    14,    15,   190,   447,
      68,    37,   296,    71,    40,    41,   213,   175,   200,    68,
     202,    19,    71,    66,    59,   283,    67,    49,   466,    37,
     468,    66,    40,    41,    11,    12,   474,     9,   350,    11,
      12,    71,   200,     9,   300,    11,    12,   331,    74,    75,
      11,    12,    66,   311,   366,   313,   317,    71,   314,    45,
     321,    59,    60,    61,    62,    63,    74,    75,    71,   507,
      61,    62,    63,    76,   332,   333,   334,    49,   336,   261,
     460,    71,    45,    49,    66,   267,    76,   269,   285,    71,
      68,   529,     3,    71,    68,     6,     3,    71,     9,   296,
      11,    12,    13,   361,   466,   363,   468,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,    39,
     302,   303,   376,   376,    71,   307,   308,    69,    39,    76,
      71,    42,   512,    44,   331,    76,   318,    71,    71,    50,
      51,    52,    76,    76,    71,    19,    69,    67,   406,    76,
      68,    71,    72,    71,    69,   413,    67,    59,    60,    61,
      62,    63,    73,    68,    68,   423,    71,    71,   350,    19,
      20,    21,    59,    60,    61,    62,    63,    68,    68,   361,
      71,    71,    11,    12,   366,    59,    60,    61,    62,    63,
     448,    68,    68,    66,    71,    71,   378,    14,    15,   390,
      61,    62,    63,    71,    72,   402,    56,    57,    58,    59,
      60,    61,    62,    63,   380,    19,    20,    21,   430,    67,
     478,    15,    71,    72,   406,    11,    12,    71,    72,     1,
      71,     3,     4,     5,     6,    11,    12,     9,    39,    11,
      12,    13,    14,   425,   456,   427,   428,    39,   460,   460,
      39,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      71,    72,    67,   445,    68,    71,    72,    39,    67,    67,
      42,    43,    44,   485,    46,    47,    48,    67,    50,    51,
      52,    19,    20,    21,    67,    57,    58,    19,    60,    21,
      67,    15,    64,    65,    49,    67,    19,    20,    21,    68,
      75,    73,    68,     1,    76,     3,     4,     5,     6,    71,
      68,     9,    68,    11,    12,    13,    14,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    58,    59,    60,    61,
      62,    63,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    39,    68,    68,    42,    43,    44,    76,    46,    47,
      48,    70,    50,    51,    52,    19,    20,    21,    71,    57,
      58,    68,    60,    19,    20,    21,    64,    65,    58,    67,
      19,    20,    21,    68,    68,    73,    68,     1,    76,     3,
       4,     5,     6,    67,    67,     9,    67,    11,    12,    13,
      14,    67,    56,    57,    58,    59,    60,    61,    62,    63,
      67,    57,    58,    59,    60,    61,    62,    63,    57,    58,
      59,    60,    61,    62,    63,    39,    66,    70,    42,    43,
      44,    66,    46,    47,    48,    69,    50,    51,    52,    68,
      68,    72,    69,    57,    16,    17,    60,    19,    20,    21,
      64,    65,    68,    67,    19,    20,    21,    68,    66,    73,
      42,     1,    76,     3,     4,     5,     6,    71,    68,     9,
      68,    11,    12,    13,    14,    29,    71,    66,    15,    71,
      33,    15,    68,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    68,    58,    59,    60,    61,    62,    63,    39,
      71,    67,    42,    43,    44,    72,    46,    47,    48,    72,
      50,    51,    52,    68,    23,    66,     4,    57,    16,     4,
      60,    19,    20,    21,    64,    65,     4,    67,    19,    20,
      21,    59,    70,    73,    66,     1,    76,     3,     4,     5,
       6,    68,    72,     9,    66,    11,    12,    13,    14,    68,
      75,    72,   402,   426,   112,     4,    70,    55,    56,    57,
      58,    59,    60,    61,    62,    63,   271,    58,    59,    60,
      61,    62,    63,    39,    36,   174,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    19,   258,    21,
     131,    57,   336,   303,    60,    61,   103,   182,    64,    65,
     103,    67,    -1,    69,   259,    -1,    -1,    73,     1,    75,
       3,     4,     5,     6,    -1,    -1,     9,    -1,    11,    12,
      13,    14,    -1,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    63,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,    -1,    42,
      43,    44,    -1,    46,    47,    48,    -1,    50,    51,    52,
      -1,    -1,    -1,    -1,    57,    -1,    -1,    60,    61,    -1,
      -1,    64,    65,    -1,    67,    -1,    69,    -1,    -1,    -1,
      73,     1,    75,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    22,    -1,    24,    -1,    -1,    -1,    -1,    29,
      30,    31,    32,    -1,    34,    35,    36,    -1,    -1,    39,
      -1,    -1,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      60,    -1,    -1,    -1,    64,    65,    66,    67,    -1,    69,
      70,    -1,     1,    73,     3,     4,     5,     6,    -1,    -1,
       9,    -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    22,    -1,    24,    25,    26,    -1,    -1,
      29,    30,    31,    32,    -1,    34,    35,    36,    -1,    -1,
      39,    -1,    -1,    42,    43,    44,    -1,    46,    47,    48,
      -1,    50,    51,    52,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    60,    -1,    -1,    -1,    64,    65,    66,    67,    -1,
      69,    70,    -1,     1,    73,     3,     4,     5,     6,    -1,
      -1,     9,    -1,    11,    12,    13,    14,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    22,    -1,    24,    -1,    -1,    -1,
      -1,    29,    30,    31,    32,    -1,    34,    35,    36,    -1,
      -1,    39,    -1,    -1,    42,    43,    44,    -1,    46,    47,
      48,    -1,    50,    51,    52,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    60,    -1,    -1,    -1,    64,    65,    66,    67,
      -1,    69,    -1,    -1,     1,    73,     3,     4,     5,     6,
      -1,    -1,     9,    -1,    11,    12,    13,    14,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    39,    -1,    -1,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,
      57,    -1,    -1,    60,    -1,    -1,    -1,    64,    65,    66,
      67,    -1,    -1,    -1,    -1,     1,    73,     3,     4,     5,
       6,    -1,    -1,     9,    -1,    11,    12,    13,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    39,    -1,    -1,    42,    43,    44,    -1,
      46,    47,    48,    -1,    50,    51,    52,    -1,    -1,    -1,
      -1,    57,    58,    -1,    60,    -1,    -1,    -1,    64,    65,
       1,    67,     3,     4,     5,     6,    -1,    73,     9,    -1,
      11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,
      -1,    42,    43,    44,    -1,    46,    47,    48,    -1,    50,
      51,    52,    -1,    -1,    -1,    -1,    57,    -1,    -1,    60,
      -1,    -1,    -1,    64,    65,    -1,    67,    68,    -1,    70,
      -1,     1,    73,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,
      -1,    -1,    42,    43,    44,    -1,    46,    47,    48,    -1,
      50,    51,    52,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      60,    -1,    -1,    -1,    64,    65,    -1,    67,    68,    -1,
      -1,    -1,     1,    73,     3,     4,     5,     6,    -1,    -1,
       9,    -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      39,    -1,    -1,    42,    43,    44,    -1,    46,    47,    48,
      -1,    50,    51,    52,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    60,    -1,    -1,    -1,    64,    65,    -1,    67,    -1,
      -1,    70,    -1,     1,    73,     3,     4,     5,     6,    -1,
      -1,     9,    -1,    11,    12,    13,    14,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    39,    -1,    -1,    42,    43,    44,    -1,    46,    47,
      48,    -1,    50,    51,    52,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    60,    -1,    -1,    -1,    64,    65,    66,    67,
      -1,    -1,    -1,    -1,     1,    73,     3,     4,     5,     6,
      -1,    -1,     9,    -1,    11,    12,    13,    14,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    39,    -1,    -1,    42,    43,    44,    -1,    46,
      47,    48,    -1,    50,    51,    52,    -1,    -1,    -1,    -1,
      57,    -1,    -1,    60,    -1,    -1,    -1,    64,    65,    66,
      67,    -1,    -1,    -1,    -1,     1,    73,     3,     4,     5,
       6,    -1,    -1,     9,    -1,    11,    12,    13,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    39,    -1,    -1,    42,    43,    44,    -1,
      46,    47,    48,    -1,    50,    51,    52,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    60,    -1,    -1,    -1,    64,    65,
      -1,    67,    68,    -1,    -1,    -1,     1,    73,     3,     4,
       5,     6,    -1,    -1,     9,    -1,    11,    12,    13,    14,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    39,    -1,    -1,    42,    43,    44,
      -1,    46,    47,    48,    -1,    50,    51,    52,    -1,    -1,
      -1,    -1,    57,    -1,    -1,    60,    -1,    -1,    -1,    64,
      65,     1,    67,     3,     4,     5,     6,    -1,    73,     9,
      -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,
      -1,    -1,    42,    43,    44,    -1,    46,    47,    48,    -1,
      50,    51,    52,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      60,    -1,    -1,    -1,    64,    65,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    73,    16,    17,    18,    19,    20,    21,
      -1,    -1,    -1,    -1,    16,    17,    18,    19,    20,    21,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    -1,    -1,    76,    -1,     7,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    76,    16,    17,    18,    19,    20,
      21,    -1,    -1,    -1,    -1,    16,    17,    18,    19,    20,
      21,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    16,    17,    18,    19,    20,    21,
      -1,    -1,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    16,    17,    18,    19,    20,    21,    -1,
      -1,    72,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    16,    17,    18,    19,    20,    21,    -1,    -1,
      72,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    16,    17,    18,    19,    20,    21,    -1,    71,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      16,    17,    18,    19,    20,    21,    -1,    71,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    16,
      17,    18,    19,    20,    21,    -1,    71,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    16,    17,
      18,    19,    20,    21,    -1,    71,    -1,    -1,    -1,    -1,
      28,    16,    17,    18,    19,    20,    21,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    -1,    -1,    -1,
      -1,    68,    -1,    -1,    -1,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    -1,    -1,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63
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
      42,    43,    44,    46,    47,    48,    50,    51,    52,    57,
      60,    64,    65,    67,    73,   107,   108,   109,   111,   112,
     113,   114,   115,   116,   140,   142,   144,   145,   158,   159,
     168,   169,   170,   172,   173,   175,   176,    67,   142,    68,
     174,    66,    71,   142,    15,   107,    39,    67,   110,    39,
      67,   108,   109,   142,    11,   106,   107,   144,    67,    81,
      67,    81,    67,   110,    67,    69,   100,   107,   107,   107,
       9,    61,    69,    75,   106,   136,    67,    16,    17,    18,
      19,    20,    21,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    14,    15,    37,    40,    41,    74,    75,
     107,    39,   110,   110,    67,    71,    72,   108,   169,   170,
      11,    12,   136,   139,   143,   148,   149,   150,   129,   107,
       7,     8,   171,   142,   107,   161,   162,   163,    50,   142,
      71,    72,    71,    72,   107,    67,   100,   167,   107,   167,
      49,   163,   106,    80,   106,   163,   107,   160,   164,   165,
      68,   141,    82,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     142,    75,   142,    75,   142,    27,    58,   106,   142,   163,
     163,   148,   162,    68,   168,   141,    68,    28,    71,   107,
      28,    71,    68,   162,    68,   107,    68,    71,   106,    71,
      11,    12,    68,    68,     1,    22,    24,    29,    30,    31,
      32,    34,    35,    36,    66,    91,    92,    93,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   106,   117,   136,
      68,    70,    72,    71,    76,    68,   106,    72,   110,   106,
     106,    58,   106,    27,    76,    68,    68,    68,    71,    72,
     143,    89,   150,   171,   161,    72,   107,    68,   107,   132,
     132,    66,    67,    67,    67,    78,    67,    67,    66,    66,
      66,   106,   101,    70,   141,   152,   153,   101,    66,    69,
     110,    68,   107,   160,    68,    68,   107,   163,    76,    76,
     106,    27,    76,    58,    76,   106,   110,   100,    68,    72,
     151,     1,    66,    90,   100,    68,    71,   166,    71,    68,
      71,    68,   101,   106,   106,   106,    91,   106,   121,   122,
     139,   154,   155,    11,    12,   123,   124,   154,    66,   143,
      66,    71,   105,   163,    68,    76,    58,    76,   106,   107,
     106,    76,   163,    42,   108,    68,   107,   131,   142,    68,
      68,    68,    29,    66,   141,    15,    71,    33,    15,   152,
      25,    26,   104,   119,   136,    68,   106,   107,    76,    76,
      76,    68,   106,   166,   166,    72,    91,    78,    67,   121,
     143,   107,   123,   107,   107,     4,    27,    60,    64,    65,
      67,   120,   174,   177,    72,   152,    91,   118,   119,    76,
      76,    72,    68,   107,    23,    94,    91,   106,    66,    68,
     120,     4,     4,     4,   177,    27,    72,    19,    20,    21,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    66,
     118,    70,   118,    68,    91,    68,   121,    79,    72,    68,
      72,   120,    67,   177,   177,   177,   177,   177,   177,   177,
     177,   177,   177,   177,   177,    66,    68,    91,    72,    78,
      91
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
     165,   165,   165,   166,   166,   167,   167,   168,   168,   169,
     170,   171,   171,   172,   172,   172,   172,   173,   173,   174,
     174,   174,   175,   176,   177,   177,   177,   177,   177,   177,
     177,   177,   177,   177,   177,   177,   177,   177,   177,   177,
     177
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
       0,     1,     2,     0,     3,     1,     3,     1,     1,     1,
       3,     1,     3,     1,     1,     2,     2,     1,     2,     1,
       3,     3,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     2,     2,
       2
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
    int yynerrs YY_ATTRIBUTE_UNUSED;

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
#line 243 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { (yyval.number) = rule_loop_open(); }
#line 2655 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 3: /* foreach_start: %empty  */
#line 244 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                      { (yyval.number) = rule_foreach_open(); }
#line 2661 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 4: /* block_start: %empty  */
#line 245 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.number) = rule_block_open(); }
#line 2667 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 5: /* special_context_start: %empty  */
#line 246 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { (yyval.number) = rule_special_context_open(); }
#line 2673 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 6: /* dollar_start: %empty  */
#line 247 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                     { (yyval.contextp) = rule_dollar_open(); }
#line 2679 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 7: /* all: program  */
#line 256 "$REPO_ROOT$/src/compiler/internal/grammar.y"
           { rule_program((yyvsp[0].node)); }
#line 2685 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 8: /* program: program def opt_semicolon  */
#line 261 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_program_append(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 2691 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 9: /* program: %empty  */
#line 262 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.node) = 0; }
#line 2697 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 11: /* opt_semicolon: ';'  */
#line 268 "$REPO_ROOT$/src/compiler/internal/grammar.y"
         { rule_opt_semicolon(); }
#line 2703 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 13: /* def: type name_list ';'  */
#line 280 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_def_global_var((yyvsp[-2].number)); (yyval.node) = 0; }
#line 2709 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 17: /* @1: %empty  */
#line 306 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_func_type((yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].string)); }
#line 2715 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 18: /* @2: %empty  */
#line 308 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_func_proto((yyvsp[-3].number), (yyvsp[-5].number), (yyvsp[-4].string), &(yyvsp[-4].shared_string), (yyvsp[-1].argument)); }
#line 2721 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 19: /* function: type optional_star identifier @1 '(' argument ')' @2 block_or_semi  */
#line 310 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_func(&(yyval.node), (yyvsp[-5].number), (yyvsp[-7].number), (yyvsp[-6].shared_string), (yyvsp[-3].argument), &(yyvsp[-1].number), &(yyvsp[0].node)); }
#line 2727 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 20: /* block_or_semi: block  */
#line 315 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = rule_block_or_semi((yyvsp[0].decl).node); }
#line 2733 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 21: /* block_or_semi: ';'  */
#line 316 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2739 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 22: /* block_or_semi: error  */
#line 317 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2745 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 23: /* statement: comma_expr ';'  */
#line 327 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_expr((yyvsp[-1].node)); }
#line 2751 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 27: /* statement: stmt_for  */
#line 331 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2757 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 28: /* statement: stmt_foreach  */
#line 332 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2763 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 31: /* statement: block  */
#line 335 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2769 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 32: /* statement: ';'  */
#line 336 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = 0; }
#line 2775 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 33: /* statement: L_BREAK ';'  */
#line 337 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_break(); }
#line 2781 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 34: /* statement: L_CONTINUE ';'  */
#line 338 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_continue(); }
#line 2787 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 35: /* stmt_return: L_RETURN ';'  */
#line 344 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_return_void(&(yyval.node)); }
#line 2793 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 36: /* stmt_return: L_RETURN comma_expr ';'  */
#line 345 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_return_expr(&(yyval.node), (yyvsp[-1].node)); }
#line 2799 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 37: /* stmt_cond: L_IF '(' comma_expr ')' statement optional_else_part  */
#line 351 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_cond(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 2805 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 38: /* optional_else_part: %empty  */
#line 355 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                { (yyval.node) = 0; }
#line 2811 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 39: /* optional_else_part: L_ELSE statement  */
#line 356 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                 { (yyval.node) = (yyvsp[0].node); }
#line 2817 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 40: /* stmt_while: L_WHILE '(' comma_expr ')' loop_start statement  */
#line 362 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_while(&(yyval.node), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2823 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 41: /* stmt_do: L_DO loop_start statement L_WHILE '(' comma_expr ')' ';'  */
#line 368 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_do(&(yyval.node), (yyvsp[-5].node), (yyvsp[-2].node), (yyvsp[-6].number)); }
#line 2829 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 42: /* stmt_for: L_FOR '(' for_init ';' for_expr ';' for_expr ')' loop_start statement  */
#line 374 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for(&(yyval.decl), &(yyvsp[-7].decl), (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2835 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 43: /* stmt_foreach: L_FOREACH '(' foreach_vars L_IN expr ')' foreach_start statement  */
#line 380 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach(&(yyval.decl), &(yyvsp[-5].decl), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2841 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 44: /* stmt_switch: switch_header '{' local_declarations case switch_block '}'  */
#line 386 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_switch(&(yyval.node), (yyvsp[-5].switch_header).expr, &(yyvsp[-3].decl), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[-5].switch_header).saved_context, (yyvsp[-5].switch_header).saved_cases_size); }
#line 2847 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 45: /* block: '{' block_start statements '}'  */
#line 398 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block(&(yyval.decl), (yyvsp[-1].decl).node, (yyvsp[-2].number)); }
#line 2853 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 46: /* statements: %empty  */
#line 404 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_empty(&(yyval.decl)); }
#line 2859 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 47: /* statements: statement statements  */
#line 406 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_stmt(&(yyval.decl), (yyvsp[-1].node), &(yyvsp[0].decl)); }
#line 2865 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 48: /* statements: local_declaration_statement statements  */
#line 408 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_decl(&(yyval.decl), &(yyvsp[-1].decl), &(yyvsp[0].decl)); }
#line 2871 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 49: /* statements: error ';' statements  */
#line 410 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_error(&(yyval.decl), &(yyvsp[0].decl)); }
#line 2877 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 50: /* local_decl_statement_header: basic_type  */
#line 415 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { rule_local_declaration_statement_set_type((yyvsp[0].number)); (yyval.number) = (yyvsp[0].number); }
#line 2883 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 51: /* local_declaration_statement: local_decl_statement_header local_name_list ';'  */
#line 420 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_declaration_statement(&(yyval.decl), &(yyvsp[-1].decl)); }
#line 2889 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 52: /* local_decl_header: basic_type  */
#line 425 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { rule_local_declarations_set_type((yyvsp[0].number)); (yyval.number) = (yyvsp[0].number); }
#line 2895 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 53: /* local_declarations: %empty  */
#line 430 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.decl).node = 0; (yyval.decl).num = 0; }
#line 2901 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 54: /* local_declarations: local_declarations local_decl_header local_name_list ';'  */
#line 432 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_declarations(&(yyval.decl), &(yyvsp[-3].decl), &(yyvsp[-1].decl)); }
#line 2907 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 56: /* comma_expr: comma_expr ',' expr  */
#line 451 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                { rule_comma_expr(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2913 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 57: /* expr: ref lvalue  */
#line 455 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_ref(&(yyval.node), (yyvsp[0].node)); }
#line 2919 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 58: /* expr: lvalue L_ASSIGN expr  */
#line 456 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_assign(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 2925 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 59: /* expr: error L_ASSIGN expr  */
#line 457 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_assign_error(&(yyval.node), (yyvsp[0].node)); }
#line 2931 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 60: /* expr: expr '?' expr ':' expr  */
#line 459 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                            { rule_expr_ternary(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2937 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 61: /* expr: expr L_QUESTION_QUESTION expr  */
#line 460 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_nullish(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2943 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 62: /* expr: expr L_LOR expr  */
#line 461 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_lor(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2949 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 63: /* expr: expr L_LAND expr  */
#line 462 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_land(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2955 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 64: /* expr: expr '|' expr  */
#line 464 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_or(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2961 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 65: /* expr: expr '^' expr  */
#line 465 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_xor(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2967 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 66: /* expr: expr '&' expr  */
#line 466 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_and(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2973 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 67: /* expr: expr L_EQ_NE expr  */
#line 468 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_expr_eq_ne(&(yyval.node), (yyvsp[-1].number), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2979 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 68: /* expr: expr L_ORDER expr  */
#line 469 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_order(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 2985 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 69: /* expr: expr '<' expr  */
#line 470 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_lt(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2991 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 70: /* expr: expr L_SHIFT expr  */
#line 472 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_expr_shift(&(yyval.node), (yyvsp[-1].number), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2997 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 71: /* expr: expr '+' expr  */
#line 474 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             {
        rule_set_operand_ranges((yylsp[-2]).first_line, (yylsp[-2]).first_column, (yylsp[-2]).last_column,
                                (yylsp[-1]).first_line, (yylsp[-1]).first_column,
                                (yylsp[0]).first_line, (yylsp[0]).first_column, (yylsp[0]).last_column);
        rule_expr_add(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node));
        rule_clear_operand_ranges();
    }
#line 3009 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 72: /* expr: expr '-' expr  */
#line 481 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_sub(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3015 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 73: /* expr: expr '*' expr  */
#line 482 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_mul(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3021 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 74: /* expr: expr '%' expr  */
#line 483 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_mod(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3027 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 75: /* expr: expr '/' expr  */
#line 484 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_div(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3033 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 76: /* expr: cast expr  */
#line 486 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_cast(&(yyval.node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3039 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 77: /* expr: L_INC_DEC lvalue  */
#line 487 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_expr_pre_incdec(&(yyval.node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3045 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 78: /* expr: '!' expr  */
#line 488 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_expr_not(&(yyval.node), (yyvsp[0].node)); }
#line 3051 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 79: /* expr: '~' expr  */
#line 489 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_expr_compl(&(yyval.node), (yyvsp[0].node)); }
#line 3057 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 80: /* expr: '-' expr  */
#line 490 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_neg(&(yyval.node), (yyvsp[0].node)); }
#line 3063 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 81: /* expr: lvalue L_INC_DEC  */
#line 492 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_expr_post_incdec(&(yyval.node), (yyvsp[0].number), (yyvsp[-1].node)); }
#line 3069 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 88: /* lvalue: primary_expr  */
#line 504 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                { (yyval.node) = rule_lvalue((yyvsp[0].node)); }
#line 3075 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 90: /* primary_expr: L_DEFINED_NAME  */
#line 514 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_defined_name(&(yyval.node), (yyvsp[0].ihe)); }
#line 3081 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 91: /* primary_expr: L_IDENTIFIER  */
#line 515 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_identifier(&(yyval.node), (yyvsp[0].string)); }
#line 3087 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 92: /* primary_expr: L_PARAMETER  */
#line 516 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_parameter(&(yyval.node), (yyvsp[0].number)); }
#line 3093 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 94: /* primary_expr: '(' comma_expr ')'  */
#line 518 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { (yyval.node) = (yyvsp[-1].node); }
#line 3099 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 97: /* primary_expr: '$' '(' dollar_start comma_expr ')'  */
#line 524 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_dollar_expr(&(yyval.node), (yyvsp[-2].contextp), (yyvsp[-1].node)); }
#line 3105 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 98: /* primary_expr: primary_expr L_ARROW identifier  */
#line 527 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                 { rule_primary_expr_member_arrow(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 3111 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 99: /* primary_expr: primary_expr '.' identifier  */
#line 528 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                 { rule_primary_expr_member_dot(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 3117 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 100: /* primary_expr: primary_expr L_OPTIONAL_DOT identifier  */
#line 534 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_member_optional(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 3123 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 101: /* primary_expr: primary_expr L_OPTIONAL_DOT '[' comma_expr ']'  */
#line 536 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_optional(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3129 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 102: /* primary_expr: primary_expr L_DOT_OPTIONAL '[' comma_expr ']'  */
#line 538 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_optional(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3135 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 103: /* primary_expr: primary_expr '[' comma_expr ']'  */
#line 543 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3141 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 104: /* primary_expr: primary_expr '[' '<' comma_expr ']'  */
#line 545 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_r(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3147 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 105: /* primary_expr: primary_expr '[' comma_expr L_RANGE comma_expr ']'  */
#line 547 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nn(&(yyval.node), (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3153 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 106: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE comma_expr ']'  */
#line 549 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rn(&(yyval.node), (yyvsp[-6].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3159 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 107: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE '<' comma_expr ']'  */
#line 551 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rr(&(yyval.node), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3165 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 108: /* primary_expr: primary_expr '[' comma_expr L_RANGE '<' comma_expr ']'  */
#line 553 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nr(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3171 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 109: /* primary_expr: primary_expr '[' comma_expr L_RANGE ']'  */
#line 555 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_ne(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node)); }
#line 3177 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 110: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE ']'  */
#line 557 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_re(&(yyval.node), (yyvsp[-5].node), (yyvsp[-2].node)); }
#line 3183 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 111: /* primary_expr: primary_expr '[' L_RANGE '<' comma_expr ']'  */
#line 559 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nr(&(yyval.node), (yyvsp[-5].node), 0, (yyvsp[-1].node)); }
#line 3189 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 112: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE '<' expr ']'  */
#line 562 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rr(&(yyval.node), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3195 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 113: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE expr ']'  */
#line 564 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rn(&(yyval.node), (yyvsp[-6].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3201 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 114: /* primary_expr: lambda_return_type '(' argument ')' block  */
#line 569 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_anon_func(&(yyval.node), &(yyvsp[-4].func_block), &(yyvsp[-2].argument), &(yyvsp[0].decl)); }
#line 3207 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 115: /* primary_expr: functional_open ':' ')'  */
#line 576 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_1(&(yyval.node), (yyvsp[-2].number)); }
#line 3213 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 116: /* primary_expr: functional_open ',' arg_list ':' ')'  */
#line 578 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_2(&(yyval.node), (yyvsp[-4].number), (yyvsp[-2].node)); }
#line 3219 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 117: /* primary_expr: L_FUNCTION_OPEN L_DEFINED_NAME ':' ')'  */
#line 587 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_1(&(yyval.node), rule_functional_ref((yyvsp[-2].ihe))); }
#line 3225 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 118: /* primary_expr: L_FUNCTION_OPEN L_DEFINED_NAME ',' arg_list ':' ')'  */
#line 589 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_2(&(yyval.node), rule_functional_ref((yyvsp[-4].ihe)), (yyvsp[-2].node)); }
#line 3231 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 119: /* primary_expr: L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 591 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_3(&(yyval.node), (yyvsp[-2].node)); }
#line 3237 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 120: /* primary_expr: '(' '[' opt_pair_list ']' ')'  */
#line 594 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_primary_expr_mapping(&(yyval.node), (yyvsp[-2].node)); }
#line 3243 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 121: /* primary_expr: '(' '{' opt_arg_list '}' ')'  */
#line 595 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_primary_expr_array(&(yyval.node), (yyvsp[-2].node)); }
#line 3249 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 122: /* call_open: '('  */
#line 605 "$REPO_ROOT$/src/compiler/internal/grammar.y"
       { rule_call_open(&(yyval.call_open).saved_context, &(yyval.call_open).saved_refs); }
#line 3255 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 123: /* function_call: efun_override call_open opt_arg_list ')'  */
#line 619 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_efun(&(yyval.node), (yyvsp[-3].number), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3261 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 124: /* function_call: L_NEW call_open opt_arg_list ')'  */
#line 621 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new(&(yyval.node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3267 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 125: /* function_call: L_NEW '(' L_CLASS L_DEFINED_NAME opt_class_init ')'  */
#line 623 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new_class(&(yyval.node), (yyvsp[-2].ihe), (yyvsp[-1].node)); }
#line 3273 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 126: /* function_call: L_NEW '(' L_CLASS L_IDENTIFIER opt_class_init ')'  */
#line 625 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new_class_undef(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node)); }
#line 3279 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 127: /* function_call: L_DEFINED_NAME call_open opt_arg_list ')'  */
#line 627 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_defined_name(&(yyval.node), (yyvsp[-3].ihe), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3285 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 128: /* function_call: function_name call_open opt_arg_list ')'  */
#line 629 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_name(&(yyval.node), (yyvsp[-3].string), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3291 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 129: /* function_call: primary_expr '[' comma_expr ']' call_open opt_arg_list ')'  */
#line 631 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_indexed(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3297 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 130: /* function_call: primary_expr L_ARROW identifier call_open opt_arg_list ')'  */
#line 633 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_arrow(&(yyval.node), (yyvsp[-5].node), (yyvsp[-3].string), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3303 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 131: /* function_call: '(' '*' comma_expr ')' call_open opt_arg_list ')'  */
#line 635 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_star(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3309 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 132: /* sscanf: L_SSCANF '(' expr ',' expr lvalue_list ')'  */
#line 646 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_sscanf(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 3315 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 133: /* parse_command: L_PARSE_COMMAND '(' expr ',' expr ',' expr lvalue_list ')'  */
#line 652 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_parse_command(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 3321 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 134: /* catch: L_CATCH special_context_start expr_or_block  */
#line 658 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_catch(&(yyval.node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 3327 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 135: /* time_expression: L_TIME_EXPRESSION special_context_start expr_or_block  */
#line 664 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_time_expression(&(yyval.node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 3333 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 136: /* tree: L_TREE block  */
#line 670 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_tree_block(&(yyval.node), (yyvsp[0].decl).node); }
#line 3339 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 137: /* tree: L_TREE '(' comma_expr ')'  */
#line 672 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_tree_expr(&(yyval.node), (yyvsp[-1].node)); }
#line 3345 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 138: /* switch_header: L_SWITCH '(' comma_expr ')'  */
#line 683 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.switch_header).expr = (yyvsp[-1].node); rule_switch_pre(&(yyval.switch_header).saved_context, &(yyval.switch_header).saved_cases_size); }
#line 3351 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 139: /* switch_block: case switch_block  */
#line 688 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_switch_block_case(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3357 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 140: /* switch_block: statement switch_block  */
#line 689 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_switch_block_stmt(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3363 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 141: /* switch_block: %empty  */
#line 690 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { rule_switch_block_empty(&(yyval.node)); }
#line 3369 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 142: /* case: L_CASE case_label ':'  */
#line 697 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_single(&(yyval.node), (yyvsp[-1].node)); }
#line 3375 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 143: /* case: L_CASE case_label L_RANGE case_label ':'  */
#line 699 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3381 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 144: /* case: L_CASE case_label L_RANGE ':'  */
#line 701 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range_from(&(yyval.node), (yyvsp[-2].node)); }
#line 3387 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 145: /* case: L_CASE L_RANGE case_label ':'  */
#line 703 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range_to(&(yyval.node), (yyvsp[-1].node)); }
#line 3393 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 146: /* case: L_DEFAULT ':'  */
#line 705 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_default(&(yyval.node)); }
#line 3399 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 147: /* case_label: constant  */
#line 711 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_case_label_constant(&(yyval.node), (yyvsp[0].number)); }
#line 3405 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 148: /* case_label: string_const  */
#line 712 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_case_label_string(&(yyval.node), (yyvsp[0].string)); }
#line 3411 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 149: /* for_expr: %empty  */
#line 722 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { (yyval.node) = 0; }
#line 3417 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 151: /* for_init: for_expr  */
#line 729 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for_init_expr(&(yyval.decl), (yyvsp[0].node)); }
#line 3423 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 152: /* for_init: single_new_local_def_with_init  */
#line 731 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for_init_local(&(yyval.decl), (yyvsp[0].node)); }
#line 3429 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 153: /* foreach_var: L_DEFINED_NAME  */
#line 736 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_defined(&(yyval.decl), (yyvsp[0].ihe)); }
#line 3435 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 154: /* foreach_var: single_new_local_def  */
#line 737 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_new_local(&(yyval.decl), (yyvsp[0].number)); }
#line 3441 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 155: /* foreach_var: L_IDENTIFIER  */
#line 738 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_identifier(&(yyval.decl), (yyvsp[0].string)); }
#line 3447 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 156: /* foreach_vars: foreach_var  */
#line 744 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach_vars_single(&(yyval.decl), &(yyvsp[0].decl)); }
#line 3453 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 157: /* foreach_vars: foreach_var ',' foreach_var  */
#line 746 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach_vars_double(&(yyval.decl), &(yyvsp[-2].decl), &(yyvsp[0].decl)); }
#line 3459 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 158: /* type_decl: class_header member_list '}'  */
#line 757 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_define_class_members((yyvsp[-2].class_header).ihe, (yyvsp[-2].class_header).classname_index); (yyval.node) = 0; }
#line 3465 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 159: /* class_header: type_modifier_list L_CLASS identifier '{'  */
#line 763 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.class_header).ihe = rule_define_class(&(yyval.class_header).classname_index, (yyvsp[-1].string)); }
#line 3471 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 161: /* $@3: %empty  */
#line 769 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                           { rule_member_list_set_type((yyvsp[0].number)); }
#line 3477 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 165: /* member_name: optional_star identifier  */
#line 781 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { rule_member_name((yyvsp[-1].number), (yyvsp[0].string)); }
#line 3483 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 166: /* class_init: identifier ':' expr  */
#line 786 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                       { (yyval.node) = rule_class_init((yyvsp[-2].string), (yyvsp[0].node)); }
#line 3489 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 167: /* opt_class_init: %empty  */
#line 791 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.node) = rule_opt_class_init_empty(); }
#line 3495 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 168: /* opt_class_init: opt_class_init ',' class_init  */
#line 792 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { (yyval.node) = rule_opt_class_init((yyvsp[-2].node), (yyvsp[0].node)); }
#line 3501 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 169: /* type: type_modifier_list opt_basic_type  */
#line 804 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { (yyval.number) = rule_type((yyvsp[-1].number), (yyvsp[0].number)); }
#line 3507 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 170: /* type_modifier_list: %empty  */
#line 809 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { (yyval.number) = 0; }
#line 3513 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 171: /* type_modifier_list: L_TYPE_MODIFIER type_modifier_list  */
#line 810 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                               { (yyval.number) = rule_type_modifier_list((yyvsp[-1].number), (yyvsp[0].number)); }
#line 3519 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 173: /* opt_basic_type: %empty  */
#line 816 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { (yyval.number) = rule_opt_basic_type_empty(); }
#line 3525 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 175: /* basic_type: opt_atomic_type L_ARRAY  */
#line 822 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { (yyval.number) = (yyvsp[-1].number) | TYPE_MOD_ARRAY; }
#line 3531 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 177: /* opt_atomic_type: %empty  */
#line 828 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { (yyval.number) = rule_opt_basic_type_empty(); }
#line 3537 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 179: /* atomic_type: L_CLASS L_DEFINED_NAME  */
#line 834 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.number) = rule_atomic_type_class((yyvsp[0].ihe)); }
#line 3543 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 180: /* atomic_type: L_CLASS L_IDENTIFIER  */
#line 835 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.number) = rule_atomic_type_class_identifier((yyvsp[0].string)); }
#line 3549 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 182: /* arg_type: basic_type ref  */
#line 841 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.number) = (yyvsp[-1].number) | LOCAL_MOD_REF; }
#line 3555 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 183: /* cast: '(' basic_type optional_star ')'  */
#line 846 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { (yyval.number) = rule_cast((yyvsp[-2].number), (yyvsp[-1].number)); }
#line 3561 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 184: /* optional_star: %empty  */
#line 851 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.number) = 0; }
#line 3567 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 185: /* optional_star: '*'  */
#line 852 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.number) = TYPE_MOD_ARRAY; }
#line 3573 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 186: /* identifier: L_DEFINED_NAME  */
#line 862 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { (yyval.string) = rule_identifier_defined_name((yyvsp[0].ihe)); }
#line 3579 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 189: /* new_local_name: L_DEFINED_NAME  */
#line 869 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.string) = rule_new_local_name_redefine((yyvsp[0].ihe)); }
#line 3585 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 190: /* efun_override: L_EFUN L_COLON_COLON identifier  */
#line 874 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.number) = rule_efun_override((yyvsp[0].string)); }
#line 3591 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 191: /* efun_override: L_EFUN L_COLON_COLON L_NEW  */
#line 875 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.number) = rule_efun_override_new(); }
#line 3597 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 193: /* function_name: L_COLON_COLON identifier  */
#line 886 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { (yyval.string) = rule_function_name_colon_colon((yyvsp[0].string)); }
#line 3603 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 194: /* function_name: L_BASIC_TYPE L_COLON_COLON identifier  */
#line 887 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { (yyval.string) = rule_function_name_type((yyvsp[-2].number), (yyvsp[0].string)); }
#line 3609 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 195: /* function_name: identifier L_COLON_COLON identifier  */
#line 888 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                    { (yyval.string) = rule_function_name_obj((yyvsp[-2].string), (yyvsp[0].string)); }
#line 3615 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 196: /* inheritance: type_modifier_list L_INHERIT string_const ';'  */
#line 894 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { if (rule_inheritence(&(yyval.node), (yyvsp[-3].number), (yyvsp[-1].string))) { YYACCEPT; } }
#line 3621 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 197: /* modifier_change: type_modifier_list ':'  */
#line 899 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { (yyval.node) = rule_modifier_change((yyvsp[-1].number)); }
#line 3627 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 198: /* argument: %empty  */
#line 910 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.argument).num_arg = 0; (yyval.argument).flags = 0; }
#line 3633 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 200: /* argument: argument_list L_DOT_DOT_DOT  */
#line 913 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_varargs(&(yyval.argument), &(yyvsp[-1].argument)); }
#line 3639 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 201: /* argument_list: param_decl  */
#line 919 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_list_single(&(yyval.argument), (yyvsp[0].number)); }
#line 3645 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 202: /* argument_list: argument_list ',' param_decl  */
#line 921 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_list_multi(&(yyval.argument), &(yyvsp[-2].argument), (yyvsp[0].number)); }
#line 3651 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 203: /* param_decl: arg_type optional_star  */
#line 931 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_typed((yyvsp[-1].number) | (yyvsp[0].number)); }
#line 3657 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 204: /* param_decl: arg_type optional_star new_local_name optional_default_arg_value  */
#line 933 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_typed_name((yyvsp[-3].number) | (yyvsp[-2].number), (yyvsp[-1].string), (yyvsp[0].node)); }
#line 3663 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 205: /* param_decl: new_local_name  */
#line 935 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_untyped_name((yyvsp[0].string)); }
#line 3669 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 206: /* optional_default_arg_value: %empty  */
#line 940 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                          { (yyval.node) = 0; }
#line 3675 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 207: /* optional_default_arg_value: ':' L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 941 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                            { (yyval.node) = rule_default_arg_value((yyvsp[-2].node)); }
#line 3681 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 208: /* local_name_list: new_local_def  */
#line 947 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_name_list_single(&(yyval.decl), (yyvsp[0].node)); }
#line 3687 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 209: /* local_name_list: new_local_def ',' local_name_list  */
#line 949 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_name_list_multi(&(yyval.decl), (yyvsp[-2].node), &(yyvsp[0].decl)); }
#line 3693 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 210: /* new_local_def: optional_star new_local_name  */
#line 955 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_new_local_def((yyvsp[0].string), (yyvsp[-1].number)); }
#line 3699 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 211: /* new_local_def: optional_star new_local_name L_ASSIGN expr  */
#line 957 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_new_local_def_with_init((yyvsp[-2].string), (yyvsp[-3].number), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3705 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 212: /* single_new_local_def: arg_type optional_star new_local_name  */
#line 963 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_single_new_local_def(&(yyval.number), (yyvsp[-2].number) | (yyvsp[-1].number), (yyvsp[0].string)); }
#line 3711 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 213: /* single_new_local_def_with_init: single_new_local_def L_ASSIGN expr  */
#line 969 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_single_new_local_def_with_init((yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3717 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 216: /* new_name: optional_star identifier  */
#line 985 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { rule_new_name((yyvsp[-1].number), (yyvsp[0].string)); }
#line 3723 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 217: /* new_name: optional_star identifier L_ASSIGN expr  */
#line 986 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { rule_new_name_with_init((yyvsp[-3].number), (yyvsp[-2].string), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3729 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 218: /* lambda_return_type: L_BASIC_TYPE  */
#line 997 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                { rule_lambda_return_type(&(yyval.func_block), (yyvsp[0].number)); }
#line 3735 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 219: /* functional_open: L_FUNCTION_OPEN efun_override  */
#line 1005 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                 { (yyval.number) = rule_functional_open((yyvsp[0].number)); }
#line 3741 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 220: /* assoc_pair: expr ':' expr  */
#line 1015 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_assoc_pair(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3747 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 221: /* spread_expr: expr  */
#line 1020 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_spread_expr_normal(&(yyval.node), (yyvsp[0].node)); }
#line 3753 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 222: /* spread_expr: expr L_DOT_DOT_DOT  */
#line 1021 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_spread_expr_dots(&(yyval.node), (yyvsp[-1].node)); }
#line 3759 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 223: /* arg_list: spread_expr  */
#line 1027 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_arg_list_single(&(yyval.node), (yyvsp[0].node)); }
#line 3765 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 224: /* arg_list: arg_list ',' spread_expr  */
#line 1029 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_arg_list_multi(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3771 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 225: /* opt_arg_list: %empty  */
#line 1035 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list_empty(&(yyval.node)); }
#line 3777 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 226: /* opt_arg_list: arg_list  */
#line 1036 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list(&(yyval.node), (yyvsp[0].node)); }
#line 3783 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 227: /* opt_arg_list: arg_list ','  */
#line 1037 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list(&(yyval.node), (yyvsp[-1].node)); }
#line 3789 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 228: /* pair_list: assoc_pair  */
#line 1043 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_pair_list_single(&(yyval.node), (yyvsp[0].node)); }
#line 3795 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 229: /* pair_list: pair_list ',' assoc_pair  */
#line 1045 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_pair_list_multi(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3801 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 230: /* opt_pair_list: %empty  */
#line 1051 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list_empty(&(yyval.node)); }
#line 3807 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 231: /* opt_pair_list: pair_list  */
#line 1052 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list(&(yyval.node), (yyvsp[0].node)); }
#line 3813 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 232: /* opt_pair_list: pair_list ','  */
#line 1053 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list(&(yyval.node), (yyvsp[-1].node)); }
#line 3819 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 233: /* lvalue_list: %empty  */
#line 1058 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.node) = rule_lvalue_list_empty(); }
#line 3825 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 234: /* lvalue_list: ',' lvalue lvalue_list  */
#line 1059 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { (yyval.node) = rule_lvalue_list((yyvsp[-1].node), (yyvsp[0].node)); }
#line 3831 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 235: /* expr_or_block: block  */
#line 1064 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_expr_or_block_block((yyvsp[0].decl)); }
#line 3837 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 236: /* expr_or_block: '(' comma_expr ')'  */
#line 1065 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_expr_or_block_expr((yyvsp[-1].node)); }
#line 3843 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 239: /* string: string_literal  */
#line 1081 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_string(&(yyval.node), (yyvsp[0].string)); }
#line 3849 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 240: /* template_literal: L_TEMPLATE_HEAD expr template_parts  */
#line 1093 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_template_literal(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3855 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 241: /* template_parts: L_TEMPLATE_TAIL  */
#line 1097 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                      { rule_template_parts_tail(&(yyval.node), (yyvsp[0].string)); }
#line 3861 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 242: /* template_parts: L_TEMPLATE_MIDDLE expr template_parts  */
#line 1098 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                       { rule_template_parts_middle(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3867 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 245: /* string_like: string_like string  */
#line 1109 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                               { rule_string_like_concat(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3873 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 246: /* string_like: string_like template_literal  */
#line 1110 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_string_like_concat(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3879 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 248: /* string_literal: string_literal L_STRING  */
#line 1118 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.string) = rule_string_literal_concat((yyvsp[-1].string), (yyvsp[0].string)); }
#line 3885 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 250: /* string_const: '(' string_const ')'  */
#line 1125 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                        { (yyval.string) = (yyvsp[-1].string); }
#line 3891 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 251: /* string_const: string_const '+' string_const  */
#line 1126 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { (yyval.string) = rule_string_literal_concat((yyvsp[-2].string), (yyvsp[0].string)); }
#line 3897 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 252: /* number: L_NUMBER  */
#line 1130 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { rule_number(&(yyval.node), (yyvsp[0].number)); }
#line 3903 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 253: /* real: L_REAL  */
#line 1134 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { rule_real(&(yyval.node), (yyvsp[0].real)); }
#line 3909 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 254: /* constant: constant '|' constant  */
#line 1140 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_or(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3915 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 255: /* constant: constant '^' constant  */
#line 1141 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_xor(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3921 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 256: /* constant: constant '&' constant  */
#line 1142 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_and(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3927 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 257: /* constant: constant L_EQ_NE constant  */
#line 1143 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                             { rule_constant_eq_ne(&(yyval.number), (yyvsp[-1].number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3933 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 258: /* constant: constant L_ORDER constant  */
#line 1144 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { rule_constant_order(&(yyval.number), (yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].number)); }
#line 3939 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 259: /* constant: constant '<' constant  */
#line 1145 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_lt(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3945 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 260: /* constant: constant L_SHIFT constant  */
#line 1146 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                             { rule_constant_shift(&(yyval.number), (yyvsp[-1].number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3951 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 261: /* constant: constant '+' constant  */
#line 1147 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_add(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3957 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 262: /* constant: constant '-' constant  */
#line 1148 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_sub(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3963 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 263: /* constant: constant '*' constant  */
#line 1149 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_mul(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3969 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 264: /* constant: constant '%' constant  */
#line 1150 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_mod(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3975 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 265: /* constant: constant '/' constant  */
#line 1151 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_div(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3981 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 266: /* constant: '(' constant ')'  */
#line 1152 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { (yyval.number) = (yyvsp[-1].number); }
#line 3987 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 267: /* constant: L_NUMBER  */
#line 1153 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { (yyval.number) = (yyvsp[0].number); }
#line 3993 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 268: /* constant: '-' L_NUMBER  */
#line 1154 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_neg(&(yyval.number), (yyvsp[0].number)); }
#line 3999 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 269: /* constant: '!' L_NUMBER  */
#line 1155 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_not(&(yyval.number), (yyvsp[0].number)); }
#line 4005 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 270: /* constant: '~' L_NUMBER  */
#line 1156 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_compl(&(yyval.number), (yyvsp[0].number)); }
#line 4011 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;


#line 4015 "$BUILD_ROOT$/src/grammar.autogen.cc"

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
#line 1159 "$REPO_ROOT$/src/compiler/internal/grammar.y"

