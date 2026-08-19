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
  YYSYMBOL_tree_context_start = 82,        /* tree_context_start  */
  YYSYMBOL_dollar_start = 83,              /* dollar_start  */
  YYSYMBOL_all = 84,                       /* all  */
  YYSYMBOL_program = 85,                   /* program  */
  YYSYMBOL_opt_semicolon = 86,             /* opt_semicolon  */
  YYSYMBOL_def = 87,                       /* def  */
  YYSYMBOL_function = 88,                  /* function  */
  YYSYMBOL_89_1 = 89,                      /* @1  */
  YYSYMBOL_90_2 = 90,                      /* @2  */
  YYSYMBOL_block_or_semi = 91,             /* block_or_semi  */
  YYSYMBOL_statement = 92,                 /* statement  */
  YYSYMBOL_stmt_return = 93,               /* stmt_return  */
  YYSYMBOL_stmt_cond = 94,                 /* stmt_cond  */
  YYSYMBOL_optional_else_part = 95,        /* optional_else_part  */
  YYSYMBOL_stmt_while = 96,                /* stmt_while  */
  YYSYMBOL_stmt_do = 97,                   /* stmt_do  */
  YYSYMBOL_stmt_for = 98,                  /* stmt_for  */
  YYSYMBOL_stmt_foreach = 99,              /* stmt_foreach  */
  YYSYMBOL_stmt_switch = 100,              /* stmt_switch  */
  YYSYMBOL_block = 101,                    /* block  */
  YYSYMBOL_statements = 102,               /* statements  */
  YYSYMBOL_local_decl_statement_header = 103, /* local_decl_statement_header  */
  YYSYMBOL_local_declaration_statement = 104, /* local_declaration_statement  */
  YYSYMBOL_local_decl_header = 105,        /* local_decl_header  */
  YYSYMBOL_local_declarations = 106,       /* local_declarations  */
  YYSYMBOL_comma_expr = 107,               /* comma_expr  */
  YYSYMBOL_expr = 108,                     /* expr  */
  YYSYMBOL_lvalue = 109,                   /* lvalue  */
  YYSYMBOL_primary_expr = 110,             /* primary_expr  */
  YYSYMBOL_call_open = 111,                /* call_open  */
  YYSYMBOL_function_call = 112,            /* function_call  */
  YYSYMBOL_sscanf = 113,                   /* sscanf  */
  YYSYMBOL_parse_command = 114,            /* parse_command  */
  YYSYMBOL_catch = 115,                    /* catch  */
  YYSYMBOL_time_expression = 116,          /* time_expression  */
  YYSYMBOL_tree = 117,                     /* tree  */
  YYSYMBOL_switch_header = 118,            /* switch_header  */
  YYSYMBOL_switch_block = 119,             /* switch_block  */
  YYSYMBOL_case = 120,                     /* case  */
  YYSYMBOL_case_label = 121,               /* case_label  */
  YYSYMBOL_for_expr = 122,                 /* for_expr  */
  YYSYMBOL_for_init = 123,                 /* for_init  */
  YYSYMBOL_foreach_var = 124,              /* foreach_var  */
  YYSYMBOL_foreach_vars = 125,             /* foreach_vars  */
  YYSYMBOL_type_decl = 126,                /* type_decl  */
  YYSYMBOL_class_header = 127,             /* class_header  */
  YYSYMBOL_member_list = 128,              /* member_list  */
  YYSYMBOL_129_3 = 129,                    /* $@3  */
  YYSYMBOL_member_name_list = 130,         /* member_name_list  */
  YYSYMBOL_member_name = 131,              /* member_name  */
  YYSYMBOL_class_init = 132,               /* class_init  */
  YYSYMBOL_opt_class_init = 133,           /* opt_class_init  */
  YYSYMBOL_type = 134,                     /* type  */
  YYSYMBOL_type_modifier_list = 135,       /* type_modifier_list  */
  YYSYMBOL_opt_basic_type = 136,           /* opt_basic_type  */
  YYSYMBOL_basic_type = 137,               /* basic_type  */
  YYSYMBOL_opt_atomic_type = 138,          /* opt_atomic_type  */
  YYSYMBOL_atomic_type = 139,              /* atomic_type  */
  YYSYMBOL_arg_type = 140,                 /* arg_type  */
  YYSYMBOL_cast = 141,                     /* cast  */
  YYSYMBOL_optional_star = 142,            /* optional_star  */
  YYSYMBOL_identifier = 143,               /* identifier  */
  YYSYMBOL_new_local_name = 144,           /* new_local_name  */
  YYSYMBOL_efun_override = 145,            /* efun_override  */
  YYSYMBOL_function_name = 146,            /* function_name  */
  YYSYMBOL_inheritance = 147,              /* inheritance  */
  YYSYMBOL_modifier_change = 148,          /* modifier_change  */
  YYSYMBOL_argument = 149,                 /* argument  */
  YYSYMBOL_argument_list = 150,            /* argument_list  */
  YYSYMBOL_param_decl = 151,               /* param_decl  */
  YYSYMBOL_optional_default_arg_value = 152, /* optional_default_arg_value  */
  YYSYMBOL_local_name_list = 153,          /* local_name_list  */
  YYSYMBOL_new_local_def = 154,            /* new_local_def  */
  YYSYMBOL_single_new_local_def = 155,     /* single_new_local_def  */
  YYSYMBOL_single_new_local_def_with_init = 156, /* single_new_local_def_with_init  */
  YYSYMBOL_name_list = 157,                /* name_list  */
  YYSYMBOL_new_name = 158,                 /* new_name  */
  YYSYMBOL_lambda_return_type = 159,       /* lambda_return_type  */
  YYSYMBOL_functional_open = 160,          /* functional_open  */
  YYSYMBOL_assoc_pair = 161,               /* assoc_pair  */
  YYSYMBOL_spread_expr = 162,              /* spread_expr  */
  YYSYMBOL_arg_list = 163,                 /* arg_list  */
  YYSYMBOL_opt_arg_list = 164,             /* opt_arg_list  */
  YYSYMBOL_pair_list = 165,                /* pair_list  */
  YYSYMBOL_opt_pair_list = 166,            /* opt_pair_list  */
  YYSYMBOL_lvalue_list = 167,              /* lvalue_list  */
  YYSYMBOL_expr_or_block = 168,            /* expr_or_block  */
  YYSYMBOL_ref = 169,                      /* ref  */
  YYSYMBOL_string = 170,                   /* string  */
  YYSYMBOL_template_literal = 171,         /* template_literal  */
  YYSYMBOL_template_parts = 172,           /* template_parts  */
  YYSYMBOL_string_like = 173,              /* string_like  */
  YYSYMBOL_string_literal = 174,           /* string_literal  */
  YYSYMBOL_string_const = 175,             /* string_const  */
  YYSYMBOL_number = 176,                   /* number  */
  YYSYMBOL_real = 177,                     /* real  */
  YYSYMBOL_constant = 178                  /* constant  */
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
#define YYLAST   2024

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  77
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  102
/* YYNRULES -- Number of rules.  */
#define YYNRULES  271
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  532

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
       0,   243,   243,   244,   245,   246,   247,   248,   257,   262,
     263,   268,   269,   280,   281,   282,   283,   284,   307,   309,
     306,   316,   317,   318,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,   339,   345,   346,   351,   356,
     357,   362,   368,   374,   380,   386,   398,   404,   406,   408,
     410,   416,   420,   426,   430,   432,   451,   452,   456,   457,
     458,   460,   461,   462,   463,   465,   466,   467,   469,   470,
     471,   473,   475,   482,   483,   484,   485,   487,   488,   489,
     490,   491,   493,   495,   496,   497,   498,   499,   500,   505,
     514,   515,   516,   517,   518,   519,   520,   521,   524,   528,
     529,   534,   536,   538,   543,   545,   547,   549,   551,   553,
     555,   557,   559,   562,   564,   569,   576,   578,   587,   589,
     591,   595,   596,   606,   619,   621,   623,   625,   627,   629,
     631,   633,   635,   646,   652,   658,   664,   670,   672,   683,
     689,   690,   691,   697,   699,   701,   703,   705,   712,   713,
     723,   724,   729,   731,   737,   738,   739,   744,   746,   757,
     763,   769,   770,   770,   776,   777,   782,   787,   792,   793,
     805,   810,   811,   816,   817,   822,   823,   828,   829,   834,
     835,   836,   841,   842,   847,   852,   853,   863,   864,   869,
     870,   875,   876,   886,   887,   888,   889,   894,   900,   910,
     912,   913,   919,   921,   931,   933,   935,   941,   942,   947,
     949,   955,   957,   963,   969,   980,   981,   986,   987,   998,
    1006,  1016,  1021,  1022,  1027,  1029,  1036,  1037,  1038,  1043,
    1045,  1052,  1053,  1054,  1059,  1060,  1065,  1066,  1071,  1072,
    1082,  1094,  1098,  1099,  1108,  1109,  1110,  1111,  1118,  1119,
    1125,  1126,  1127,  1131,  1135,  1141,  1142,  1143,  1144,  1145,
    1146,  1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,
    1156,  1157
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
  "tree_context_start", "dollar_start", "all", "program", "opt_semicolon",
  "def", "function", "@1", "@2", "block_or_semi", "statement",
  "stmt_return", "stmt_cond", "optional_else_part", "stmt_while",
  "stmt_do", "stmt_for", "stmt_foreach", "stmt_switch", "block",
  "statements", "local_decl_statement_header",
  "local_declaration_statement", "local_decl_header", "local_declarations",
  "comma_expr", "expr", "lvalue", "primary_expr", "call_open",
  "function_call", "sscanf", "parse_command", "catch", "time_expression",
  "tree", "switch_header", "switch_block", "case", "case_label",
  "for_expr", "for_init", "foreach_var", "foreach_vars", "type_decl",
  "class_header", "member_list", "$@3", "member_name_list", "member_name",
  "class_init", "opt_class_init", "type", "type_modifier_list",
  "opt_basic_type", "basic_type", "opt_atomic_type", "atomic_type",
  "arg_type", "cast", "optional_star", "identifier", "new_local_name",
  "efun_override", "function_name", "inheritance", "modifier_change",
  "argument", "argument_list", "param_decl", "optional_default_arg_value",
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

#define YYPACT_NINF (-420)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-234)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -420,   172,   150,  -420,    86,    61,  -420,  -420,  -420,   115,
     119,  -420,  -420,  -420,  -420,  -420,    45,  -420,   331,   131,
     114,  -420,    36,   387,  -420,  -420,  -420,   170,   187,   465,
    -420,  -420,  -420,  -420,     9,  -420,   115,  -420,    36,   238,
      -3,   180,   218,   227,  -420,  -420,  -420,   115,  1634,   205,
     331,  -420,    43,  -420,    36,  -420,  -420,   249,   229,   331,
     294,  -420,  -420,  1634,   285,    14,    99,   289,   234,   331,
    1699,   280,  -420,  -420,   295,  -420,   297,  -420,   337,  -420,
    1634,  1634,  1634,   713,   342,  1961,   449,   216,  -420,  -420,
    -420,  -420,  -420,  -420,  1634,   378,   368,   368,   380,   415,
     234,  -420,  -420,   153,   238,  -420,  -420,   128,   436,  -420,
    -420,  -420,   115,  -420,  1634,   332,   331,  -420,  1279,    39,
     786,  -420,   304,  -420,    74,   445,  1961,   368,  1634,    96,
    1634,    96,   409,  1279,  1634,   392,  -420,  -420,  -420,   122,
    1634,  1350,   565,   190,   115,  -420,  1634,  1634,  1634,  1634,
    1634,  1634,  1634,  1634,  1634,  1634,  1634,  1634,  1634,  1634,
    1634,  1634,  -420,  1634,   331,    24,   395,   331,  1143,  -420,
     331,  1279,  1279,   128,  1634,   406,  -420,  -420,  -420,  -420,
    -420,    -9,   115,  -420,   411,    -7,  -420,  -420,  1961,  1634,
    -420,  -420,  -420,  1948,  -420,   418,   425,  -420,  -420,  1634,
     430,  1634,   433,  1834,  1634,  -420,  -420,  -420,  1852,  -420,
     517,   438,   203,  -420,   221,   466,  1767,  -420,   479,   478,
    -420,   494,  1634,   540,   452,   681,   296,   207,   220,  1786,
     747,   313,   688,   220,   305,   305,  -420,  -420,  -420,  1961,
     368,  1634,  -420,  1634,  -420,   505,  1634,    38,  -420,   497,
     504,   507,   460,  -420,  -420,   533,  -420,  -420,   120,   332,
    -420,  1208,  -420,   475,  -420,  1961,  -420,  1634,   242,   859,
    1634,  -420,  -420,  -420,  -420,   368,   512,  1634,   639,   513,
    -420,   258,  1634,  1279,   103,   158,  1634,    41,   402,   368,
    -420,  -420,   392,  1634,   526,   501,     4,  -420,  -420,  -420,
     537,  1882,  -420,    40,   543,   547,   554,  -420,   556,   557,
     560,   561,  1421,  -420,   859,  -420,  -420,  -420,  -420,  -420,
    -420,  -420,  -420,   558,   115,   859,   127,   562,  -420,  1900,
     292,   309,  1279,  -420,  1961,  -420,  -420,  -420,  1961,   568,
    -420,  -420,   162,   491,  -420,  1634,  -420,   184,  1279,  -420,
    -420,   595,  -420,  -420,  -420,  -420,  -420,  -420,   234,   571,
     859,  1634,  1634,  1634,  1001,  1072,   239,  -420,  -420,  -420,
     193,  -420,  -420,   533,   580,   576,  -420,  -420,  -420,  1634,
    -420,   331,  -420,   581,  -420,  -420,  1634,  -420,   240,   464,
     241,  -420,   596,  1634,   592,  -420,  -420,   329,   363,   365,
     636,   597,  -420,   600,   115,   664,  -420,  -420,  -420,   609,
     651,  -420,  -420,   673,  -420,   115,   312,  1882,  -420,   620,
    -420,   251,  1757,  -420,  -420,  -420,  -420,   481,  -420,  1001,
    -420,  -420,   626,  1492,   533,  1634,   239,  1634,  1634,  -420,
      58,   622,   115,   930,  -420,   627,  1634,  -420,  -420,   637,
     687,  1001,  1634,   645,  -420,  1961,  -420,  1930,  1961,  -420,
      84,   709,   716,   717,    84,    -1,   669,   614,  -420,   657,
     930,   659,   930,  -420,  1961,  -420,  1001,  -420,  -420,   369,
    1563,  -420,   658,  -420,  -420,  -420,   599,    26,  -420,    15,
      15,    15,    15,    15,    15,    15,    15,    15,    15,    15,
      15,  -420,  -420,  -420,  -420,  -420,   665,   666,  1001,  -420,
    -420,  -420,   660,    15,   367,   760,   243,  1035,   362,  1043,
     243,   394,   394,  -420,  -420,  -420,  -420,  -420,  -420,  -420,
    1001,  -420
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
      10,     0,   171,     1,   171,    11,    13,    16,   161,   185,
     174,    15,    17,   172,    12,     9,   178,   186,     0,     0,
     215,   179,     0,     0,   198,   170,   173,     0,   175,     0,
     159,   162,   187,   188,   217,    14,   185,   248,     0,   250,
       0,   180,   181,     0,   176,   180,   181,   185,     0,     0,
       0,   216,     0,   249,     0,   197,   160,     0,   164,     0,
       0,   253,   254,     0,   219,    91,    92,     0,     0,     0,
       0,     0,     5,   238,     0,     5,     0,    93,     6,   239,
       0,     0,     0,     0,     0,   218,     0,    83,    90,    84,
      85,    96,    86,    97,     0,     0,     0,     0,     0,     0,
       0,   244,   245,    94,   240,    87,    88,   178,   217,   251,
     252,   163,   185,   166,     0,     0,     0,   123,     0,     0,
       0,    78,    89,   194,    91,     0,    56,   220,     0,     0,
       0,     0,   123,     0,     0,     0,    81,    79,    80,   179,
       0,     0,     0,     0,   185,     7,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    82,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,   178,     0,     0,    58,   246,   247,   190,
     189,   182,   185,   206,     0,   200,   202,   165,    60,     0,
     242,   241,   195,   222,   224,   227,     0,   192,   191,     0,
       0,     0,     0,     0,     0,     4,   236,   135,     0,   136,
       0,     0,     0,   137,     0,     0,     0,   229,   232,     0,
      95,     0,     0,    64,    63,    62,    71,    68,    69,     0,
      65,    66,    67,    70,    72,    73,    74,    75,    76,    59,
      99,     0,   101,     0,   100,     0,     0,     0,   196,     0,
       0,     0,     0,   116,   183,   204,    19,   201,   178,     0,
     223,     0,   128,     0,   118,    57,   120,     0,     0,     0,
       0,   168,   168,   125,   138,     0,     0,     0,     0,     0,
     184,     0,     0,     0,     0,     0,     0,     0,     0,   104,
     124,   129,     0,     0,     0,   207,     0,   203,   243,   225,
       0,   234,   237,     0,     0,     0,     0,     2,     0,     0,
       0,     0,     0,    33,     0,    31,    25,    26,    27,    28,
      29,    30,    32,     0,   185,     0,     0,     0,    51,     0,
       0,     0,     0,   122,   221,   230,   121,    98,    61,     0,
     102,   103,     0,     0,   105,     0,   110,     0,     0,   115,
     117,     0,   205,    23,    22,    20,    21,   119,     0,     0,
       0,     0,     0,     0,     0,     0,   178,    34,    35,    36,
       0,    48,    46,     0,     0,   209,    49,    24,    54,     0,
     126,     0,   127,     0,   131,   112,     0,   111,     0,    56,
       0,   106,     0,     0,   234,   133,    50,     0,     0,     0,
       0,   151,   152,     0,   185,     0,   153,   154,   156,   157,
       0,   155,    37,   211,    52,   185,   178,   234,   169,     0,
     132,     0,    56,   107,   114,   109,   130,     0,   235,     0,
     139,     2,     0,     0,     0,     0,   178,     0,     0,   210,
       0,     0,   185,     0,    53,     0,     0,   108,   113,     0,
      39,     0,     0,     0,   213,   214,   158,     0,   212,   268,
       0,     0,     0,     0,     0,     0,   149,   148,   147,     0,
       0,     0,     0,   134,   167,   208,     0,    38,    41,     0,
       0,     3,     0,   269,   270,   271,     0,     0,   143,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    55,   141,    45,   140,    40,     0,     0,     0,   146,
     267,   145,     0,     0,   261,   258,   259,   255,   256,   257,
     260,   262,   263,   264,   265,   266,    42,     2,    44,   144,
       0,    43
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -420,  -419,  -420,  -420,   670,  -420,  -420,  -420,  -420,  -420,
    -420,  -420,  -420,  -420,  -420,  -346,  -420,  -420,  -420,  -420,
    -420,  -420,  -420,  -420,  -104,  -265,  -420,  -420,  -420,  -420,
     -68,    53,   -60,   -59,   -53,  -420,  -420,  -420,  -420,  -420,
    -420,  -420,  -249,   317,  -368,  -411,  -420,   299,  -420,  -420,
    -420,  -420,  -420,   641,  -420,  -420,   482,  -420,   765,  -420,
      -6,  -420,  -420,  -352,  -420,    -2,   -12,  -252,   701,  -420,
    -420,  -420,   602,  -420,   514,  -420,  -225,  -420,   419,  -420,
     740,  -420,  -420,  -420,   515,  -215,  -132,  -113,  -420,  -420,
    -274,   652,   604,   691,   693,   542,  -420,   -22,   -21,  -420,
    -420,    93
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,   364,   508,   269,   129,   135,   222,     1,     2,    15,
       5,     6,    49,   296,   355,   314,   315,   316,   477,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   442,   416,
     326,   126,    86,    87,   118,    88,    89,    90,    91,    92,
      93,   327,   471,   472,   465,   402,   403,   409,   410,     7,
       8,    16,    47,    57,    58,   418,   330,     9,    10,    25,
     181,    27,    28,   182,    94,   373,    95,   183,    96,    97,
      11,    12,   184,   185,   186,   352,   374,   375,   411,   406,
      19,    20,    98,    99,   217,   194,   195,   196,   218,   219,
     359,   207,   100,   101,   102,   191,   103,   104,   466,   105,
     106,   467
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      39,    40,   125,   295,    26,   353,    34,    18,   121,   122,
      31,    43,   451,   404,   404,   143,    39,    52,   400,   459,
     211,   257,   453,   133,    48,   206,   487,   206,   215,    37,
     459,   213,    39,   110,    50,    32,    33,    73,   108,    37,
     176,   122,   252,   171,   172,    59,   299,   113,    79,   371,
      32,    33,   143,  -187,    21,   114,    54,   123,   249,   250,
     376,    37,   459,    55,   258,   288,   212,   263,   343,   507,
     354,   488,   214,   205,   171,   461,   -18,   144,   299,   462,
     463,   117,   513,   450,   404,   460,   461,    37,   459,   197,
     462,   463,   482,   464,    29,   396,     4,   470,   511,   241,
     247,    85,    54,    38,   192,   478,   360,   198,   530,   201,
      59,   109,   201,  -187,   289,    30,   115,   344,   461,   512,
     428,   413,   462,   463,   470,   464,   470,    14,    21,    21,
     505,   179,   180,   136,   137,   138,   268,    21,  -188,   179,
     180,   117,   221,   445,   461,   199,   200,   169,   462,   463,
      -8,   464,   240,   242,   281,   244,    37,    22,   248,    63,
       4,   116,   528,   204,  -178,   205,  -193,   188,    23,    29,
     339,   193,     3,   284,   201,   285,    17,    29,   287,   340,
     255,   203,   454,   208,   531,    36,   193,   283,   349,  -219,
     439,    24,   356,   377,   193,   216,  -199,    35,   201,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,    44,   239,   469,   342,   383,
     347,   502,   332,   504,   193,   193,   149,   193,   151,   201,
     -89,   -89,  -177,   201,   341,   392,   348,    37,   385,   149,
      63,    53,   259,    64,   370,    65,    66,    67,    21,  -187,
     407,   408,   193,   164,   265,   201,   165,   166,   220,   412,
     391,   201,   489,   328,   201,   156,   157,   158,   159,   160,
     161,   274,   107,    69,   201,   388,    70,   390,    72,   157,
     158,   159,   160,   161,    76,    77,    78,  -188,    29,   275,
     167,   168,   201,   397,   398,   399,    56,   401,   394,   122,
     112,   120,   496,   497,   498,   499,   500,    84,   328,   114,
     302,   201,   201,   201,   193,   111,   423,   425,   421,   328,
     301,    21,   201,   329,   116,   427,   337,   447,   119,   201,
     334,   216,   149,   150,   151,   338,   193,   440,   441,   189,
     190,   164,    32,    33,   165,   166,   193,   128,   146,   147,
     148,   149,   150,   151,   328,   157,   158,   159,   160,   161,
     380,    29,   130,   381,   132,   401,   159,   160,   161,   419,
     155,   156,   157,   158,   159,   160,   161,   382,   167,   168,
     381,   489,   490,   491,   479,   193,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   389,   429,    41,    42,
     201,   193,   434,    60,   134,    37,    61,    62,    63,   145,
     444,    64,   401,    65,    66,    67,    68,   170,    39,   494,
     495,   496,   497,   498,   499,   500,   496,   497,   498,   499,
     500,   430,   417,   431,   201,   117,   201,   506,    39,   422,
     201,    69,    39,    52,    70,    71,    72,   173,    73,    74,
      75,    48,    76,    77,    78,   498,   499,   500,   210,    79,
     345,   205,    80,   162,   163,    39,    81,    82,   146,    83,
     243,   149,   150,   151,   253,    84,    45,    46,   346,   256,
     146,   147,   148,   149,   150,   151,   174,   175,   455,   261,
     457,   458,    60,   262,    37,    61,    62,    63,   264,   474,
      64,   266,    65,    66,    67,    68,   273,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   201,   202,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   271,   272,
      69,   293,   294,    70,    71,    72,   276,    73,    74,    75,
     424,    76,    77,    78,   179,   180,   293,   300,    79,   386,
     278,    80,   201,   449,   279,    81,    82,   486,    83,   149,
     150,   151,   280,   286,    84,   290,    60,   387,    37,    61,
      62,    63,   291,   351,    64,   292,    65,    66,    67,    68,
     333,   336,   514,   515,   516,   517,   518,   519,   520,   521,
     522,   523,   524,   525,   350,   153,   154,   155,   156,   157,
     158,   159,   160,   161,    69,   357,   486,    70,    71,    72,
     361,    73,    74,    75,   362,    76,    77,    78,   489,   490,
     491,   363,    79,   365,   366,    80,   367,   368,   372,    81,
      82,   378,    83,   489,   490,   491,   384,   393,    84,   395,
      60,  -231,    37,    61,    62,    63,   414,   415,    64,   420,
      65,    66,    67,    68,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   358,   426,   432,   433,   510,   201,   492,
     493,   494,   495,   496,   497,   498,   499,   500,    69,   435,
     436,    70,    71,    72,   437,    73,    74,    75,   438,    76,
      77,    78,   446,   452,   468,   473,    79,   146,   147,    80,
     149,   150,   151,    81,    82,   475,    83,   149,   150,   151,
     476,   480,    84,   483,    60,  -233,    37,    61,    62,    63,
     484,   485,   139,   501,    65,    66,    67,    68,    54,   503,
     509,   526,   529,   443,   527,   456,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   131,   156,   157,   158,   159,
     160,   161,    69,   187,   331,    70,    71,    72,  -178,    73,
      74,    75,    29,    76,    77,    78,   149,   150,   151,    13,
      79,   127,   297,    80,   140,   251,    51,    81,    82,   489,
      83,   491,   141,   209,   405,   254,    84,    60,   142,    37,
      61,    62,    63,   335,   177,    64,   178,    65,    66,    67,
      68,   298,     0,   154,   155,   156,   157,   158,   159,   160,
     161,     0,     0,     0,     0,     0,     0,     0,   495,   496,
     497,   498,   499,   500,     0,    69,     0,     0,    70,    71,
      72,     0,    73,    74,    75,     0,    76,    77,    78,     0,
       0,     0,     0,    79,     0,     0,    80,   140,     0,     0,
      81,    82,     0,    83,     0,   141,     0,     0,     0,    84,
     303,   142,    37,    61,    62,    63,     0,     0,   139,     0,
      65,    66,    67,    68,     0,     0,     0,     0,     0,     0,
       0,   304,     0,   305,     0,     0,     0,     0,   306,   307,
     308,   309,     0,   310,   311,   312,     0,     0,    69,     0,
       0,    70,    71,    72,  -178,    73,    74,    75,    29,    76,
      77,    78,     0,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,    81,    82,   313,    83,     0,   205,   -47,
       0,    60,    84,    37,    61,    62,    63,     0,     0,    64,
       0,    65,    66,    67,    68,     0,     0,     0,     0,     0,
       0,     0,   304,     0,   305,   440,   441,     0,     0,   306,
     307,   308,   309,     0,   310,   311,   312,     0,     0,    69,
       0,     0,    70,    71,    72,     0,    73,    74,    75,     0,
      76,    77,    78,     0,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,    81,    82,   313,    83,     0,   205,
    -142,     0,    60,    84,    37,    61,    62,    63,     0,     0,
      64,     0,    65,    66,    67,    68,     0,     0,     0,     0,
       0,     0,     0,   304,     0,   305,     0,     0,     0,     0,
     306,   307,   308,   309,     0,   310,   311,   312,     0,     0,
      69,     0,     0,    70,    71,    72,     0,    73,    74,    75,
       0,    76,    77,    78,   489,   490,   491,     0,    79,     0,
       0,    80,   489,   490,   491,    81,    82,   313,    83,     0,
     205,     0,     0,    60,    84,    37,    61,    62,    63,     0,
       0,   139,     0,    65,    66,    67,    68,     0,     0,     0,
       0,   493,   494,   495,   496,   497,   498,   499,   500,     0,
       0,   495,   496,   497,   498,   499,   500,     0,     0,     0,
       0,    69,     0,     0,    70,    71,    72,  -178,    73,    74,
      75,    29,    76,    77,    78,     0,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,    81,    82,  -150,    83,
       0,     0,     0,     0,    60,    84,    37,    61,    62,    63,
       0,     0,    64,     0,    65,    66,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     245,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,     0,     0,    70,    71,    72,     0,    73,
      74,    75,     0,    76,    77,    78,     0,     0,     0,     0,
      79,   246,     0,    80,     0,     0,     0,    81,    82,    60,
      83,    37,    61,    62,    63,     0,    84,    64,     0,    65,
      66,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    69,     0,     0,
      70,    71,    72,     0,    73,    74,    75,     0,    76,    77,
      78,     0,     0,     0,     0,    79,     0,     0,    80,     0,
       0,     0,    81,    82,     0,    83,  -228,     0,  -228,     0,
      60,    84,    37,    61,    62,    63,     0,     0,    64,     0,
      65,    66,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    69,     0,
       0,    70,    71,    72,     0,    73,    74,    75,     0,    76,
      77,    78,     0,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,    81,    82,     0,    83,  -226,     0,     0,
       0,    60,    84,    37,    61,    62,    63,     0,     0,    64,
       0,    65,    66,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    69,
       0,     0,    70,    71,    72,     0,    73,    74,    75,     0,
      76,    77,    78,     0,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,    81,    82,     0,    83,     0,     0,
    -226,     0,    60,    84,    37,    61,    62,    63,     0,     0,
      64,     0,    65,    66,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      69,     0,     0,    70,    71,    72,     0,    73,    74,    75,
       0,    76,    77,    78,     0,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,    81,    82,   369,    83,     0,
       0,     0,     0,    60,    84,    37,    61,    62,    63,     0,
       0,    64,     0,    65,    66,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    69,     0,     0,    70,    71,    72,     0,    73,    74,
      75,     0,    76,    77,    78,     0,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,    81,    82,  -150,    83,
       0,     0,     0,     0,    60,    84,    37,    61,    62,    63,
       0,     0,    64,     0,    65,    66,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,     0,     0,    70,    71,    72,     0,    73,
      74,    75,     0,    76,    77,    78,     0,     0,     0,     0,
      79,     0,     0,    80,     0,     0,     0,    81,    82,     0,
      83,  -150,     0,     0,     0,    60,    84,    37,    61,    62,
      63,     0,     0,    64,     0,    65,    66,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    69,     0,     0,    70,    71,    72,     0,
      73,    74,    75,     0,    76,    77,    78,     0,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,    81,    82,
      60,    83,    37,    61,    62,    63,     0,    84,    64,     0,
     124,    66,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    69,     0,
       0,    70,    71,    72,     0,    73,    74,    75,     0,    76,
      77,    78,     0,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,    81,    82,     0,    83,     0,     0,     0,
       0,     0,    84,   146,   147,   148,   149,   150,   151,     0,
       0,     0,     0,   146,   147,   148,   149,   150,   151,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   146,   147,   148,   149,   150,   151,     0,     0,
       0,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,     0,     0,   448,     0,     0,     0,     0,     0,   277,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     146,   147,   148,   149,   150,   151,     0,     0,   282,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   146,   147,
     148,   149,   150,   151,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   146,   147,
     148,   149,   150,   151,     0,   267,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   146,   147,   148,   149,
     150,   151,     0,   270,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   146,   147,   148,   149,
     150,   151,     0,   358,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   146,   147,   148,   149,   150,   151,
       0,   379,     0,     0,     0,     0,   260,   146,   147,   148,
     149,   150,   151,     0,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,     0,     0,     0,     0,   481,     0,
       0,     0,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,     0,     0,     0,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161
};

static const yytype_int16 yycheck[] =
{
      22,    22,    70,   255,    10,     1,    18,     9,    68,    68,
      16,    23,   431,   365,   366,    83,    38,    38,   364,     4,
     133,    28,   433,    76,    15,   129,    27,   131,   141,     3,
       4,   135,    54,    54,    36,    11,    12,    46,    50,     3,
     100,   100,   174,    96,    97,    47,   261,    59,    57,   314,
      11,    12,   120,    39,     9,    15,    59,    69,   171,   172,
     325,     3,     4,    66,    71,    27,   134,   199,    27,   480,
      66,    72,   140,    69,   127,    60,    67,    83,   293,    64,
      65,    67,    67,   429,   436,    27,    60,     3,     4,    50,
      64,    65,   460,    67,    49,   360,    10,   443,    72,    75,
     168,    48,    59,    67,   116,   451,    66,   119,   527,    71,
     112,    68,    71,    39,    76,    70,    63,    76,    60,   487,
     394,   373,    64,    65,   470,    67,   472,    66,     9,     9,
     476,    11,    12,    80,    81,    82,   204,     9,    39,    11,
      12,    67,   144,   417,    60,    71,    72,    94,    64,    65,
       0,    67,   164,   165,   222,   167,     3,    38,   170,     6,
      10,    39,   508,    67,    45,    69,    67,   114,    49,    49,
     283,   118,     0,   241,    71,   243,    61,    49,   246,    76,
     182,   128,   434,   130,   530,    71,   133,   240,   292,    67,
     415,    72,   296,    66,   141,   142,    68,    66,    71,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,    45,   163,   442,   286,   332,
     288,   470,   275,   472,   171,   172,    19,   174,    21,    71,
      14,    15,    45,    71,    76,   348,   289,     3,    76,    19,
       6,     3,   189,     9,   312,    11,    12,    13,     9,    69,
      11,    12,   199,    37,   201,    71,    40,    41,    68,    66,
      76,    71,    19,   269,    71,    58,    59,    60,    61,    62,
      63,    68,    67,    39,    71,   343,    42,   345,    44,    59,
      60,    61,    62,    63,    50,    51,    52,    69,    49,    68,
      74,    75,    71,   361,   362,   363,    69,   365,   358,   358,
      71,    67,    59,    60,    61,    62,    63,    73,   314,    15,
      68,    71,    71,    71,   261,    66,    76,    76,   386,   325,
     267,     9,    71,   270,    39,   393,    68,    76,    39,    71,
     277,   278,    19,    20,    21,   282,   283,    25,    26,     7,
       8,    37,    11,    12,    40,    41,   293,    67,    16,    17,
      18,    19,    20,    21,   360,    59,    60,    61,    62,    63,
      68,    49,    67,    71,    67,   433,    61,    62,    63,   381,
      57,    58,    59,    60,    61,    62,    63,    68,    74,    75,
      71,    19,    20,    21,   452,   332,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,   343,    68,    11,    12,
      71,   348,   404,     1,    67,     3,     4,     5,     6,    67,
     416,     9,   480,    11,    12,    13,    14,    39,   440,    57,
      58,    59,    60,    61,    62,    63,    59,    60,    61,    62,
      63,    68,   379,    68,    71,    67,    71,    68,   460,   386,
      71,    39,   464,   464,    42,    43,    44,    67,    46,    47,
      48,    15,    50,    51,    52,    61,    62,    63,    49,    57,
      58,    69,    60,    14,    15,   487,    64,    65,    16,    67,
      75,    19,    20,    21,    68,    73,    11,    12,    76,    68,
      16,    17,    18,    19,    20,    21,    71,    72,   435,    71,
     437,   438,     1,    68,     3,     4,     5,     6,    68,   446,
       9,    68,    11,    12,    13,    14,    68,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    71,    72,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    11,    12,
      39,    71,    72,    42,    43,    44,    70,    46,    47,    48,
      76,    50,    51,    52,    11,    12,    71,    72,    57,    58,
      71,    60,    71,    72,    76,    64,    65,   464,    67,    19,
      20,    21,    68,    58,    73,    68,     1,    76,     3,     4,
       5,     6,    68,    72,     9,    68,    11,    12,    13,    14,
      68,    68,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,    68,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    39,    68,   513,    42,    43,    44,
      67,    46,    47,    48,    67,    50,    51,    52,    19,    20,
      21,    67,    57,    67,    67,    60,    66,    66,    70,    64,
      65,    69,    67,    19,    20,    21,    68,    42,    73,    68,
       1,    76,     3,     4,     5,     6,    66,    71,     9,    68,
      11,    12,    13,    14,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    71,    68,    29,    66,    68,    71,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    39,    15,
      71,    42,    43,    44,    33,    46,    47,    48,    15,    50,
      51,    52,    72,    67,    72,    68,    57,    16,    17,    60,
      19,    20,    21,    64,    65,    68,    67,    19,    20,    21,
      23,    66,    73,     4,     1,    76,     3,     4,     5,     6,
       4,     4,     9,    66,    11,    12,    13,    14,    59,    70,
      72,    66,    72,   416,    68,   436,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    75,    58,    59,    60,    61,
      62,    63,    39,   112,   272,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    19,    20,    21,     4,
      57,    70,   258,    60,    61,   173,    36,    64,    65,    19,
      67,    21,    69,   131,   365,   181,    73,     1,    75,     3,
       4,     5,     6,   278,   103,     9,   103,    11,    12,    13,
      14,   259,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    58,    59,
      60,    61,    62,    63,    -1,    39,    -1,    -1,    42,    43,
      44,    -1,    46,    47,    48,    -1,    50,    51,    52,    -1,
      -1,    -1,    -1,    57,    -1,    -1,    60,    61,    -1,    -1,
      64,    65,    -1,    67,    -1,    69,    -1,    -1,    -1,    73,
       1,    75,     3,     4,     5,     6,    -1,    -1,     9,    -1,
      11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    22,    -1,    24,    -1,    -1,    -1,    -1,    29,    30,
      31,    32,    -1,    34,    35,    36,    -1,    -1,    39,    -1,
      -1,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    -1,    -1,    -1,    57,    -1,    -1,    60,
      -1,    -1,    -1,    64,    65,    66,    67,    -1,    69,    70,
      -1,     1,    73,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    22,    -1,    24,    25,    26,    -1,    -1,    29,
      30,    31,    32,    -1,    34,    35,    36,    -1,    -1,    39,
      -1,    -1,    42,    43,    44,    -1,    46,    47,    48,    -1,
      50,    51,    52,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      60,    -1,    -1,    -1,    64,    65,    66,    67,    -1,    69,
      70,    -1,     1,    73,     3,     4,     5,     6,    -1,    -1,
       9,    -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    22,    -1,    24,    -1,    -1,    -1,    -1,
      29,    30,    31,    32,    -1,    34,    35,    36,    -1,    -1,
      39,    -1,    -1,    42,    43,    44,    -1,    46,    47,    48,
      -1,    50,    51,    52,    19,    20,    21,    -1,    57,    -1,
      -1,    60,    19,    20,    21,    64,    65,    66,    67,    -1,
      69,    -1,    -1,     1,    73,     3,     4,     5,     6,    -1,
      -1,     9,    -1,    11,    12,    13,    14,    -1,    -1,    -1,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    -1,
      -1,    58,    59,    60,    61,    62,    63,    -1,    -1,    -1,
      -1,    39,    -1,    -1,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    60,    -1,    -1,    -1,    64,    65,    66,    67,
      -1,    -1,    -1,    -1,     1,    73,     3,     4,     5,     6,
      -1,    -1,     9,    -1,    11,    12,    13,    14,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    39,    -1,    -1,    42,    43,    44,    -1,    46,
      47,    48,    -1,    50,    51,    52,    -1,    -1,    -1,    -1,
      57,    58,    -1,    60,    -1,    -1,    -1,    64,    65,     1,
      67,     3,     4,     5,     6,    -1,    73,     9,    -1,    11,
      12,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,    -1,
      42,    43,    44,    -1,    46,    47,    48,    -1,    50,    51,
      52,    -1,    -1,    -1,    -1,    57,    -1,    -1,    60,    -1,
      -1,    -1,    64,    65,    -1,    67,    68,    -1,    70,    -1,
       1,    73,     3,     4,     5,     6,    -1,    -1,     9,    -1,
      11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,
      -1,    42,    43,    44,    -1,    46,    47,    48,    -1,    50,
      51,    52,    -1,    -1,    -1,    -1,    57,    -1,    -1,    60,
      -1,    -1,    -1,    64,    65,    -1,    67,    68,    -1,    -1,
      -1,     1,    73,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,
      -1,    -1,    42,    43,    44,    -1,    46,    47,    48,    -1,
      50,    51,    52,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      60,    -1,    -1,    -1,    64,    65,    -1,    67,    -1,    -1,
      70,    -1,     1,    73,     3,     4,     5,     6,    -1,    -1,
       9,    -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      39,    -1,    -1,    42,    43,    44,    -1,    46,    47,    48,
      -1,    50,    51,    52,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    60,    -1,    -1,    -1,    64,    65,    66,    67,    -1,
      -1,    -1,    -1,     1,    73,     3,     4,     5,     6,    -1,
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
      57,    -1,    -1,    60,    -1,    -1,    -1,    64,    65,    -1,
      67,    68,    -1,    -1,    -1,     1,    73,     3,     4,     5,
       6,    -1,    -1,     9,    -1,    11,    12,    13,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    39,    -1,    -1,    42,    43,    44,    -1,
      46,    47,    48,    -1,    50,    51,    52,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    60,    -1,    -1,    -1,    64,    65,
       1,    67,     3,     4,     5,     6,    -1,    73,     9,    -1,
      11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,
      -1,    42,    43,    44,    -1,    46,    47,    48,    -1,    50,
      51,    52,    -1,    -1,    -1,    -1,    57,    -1,    -1,    60,
      -1,    -1,    -1,    64,    65,    -1,    67,    -1,    -1,    -1,
      -1,    -1,    73,    16,    17,    18,    19,    20,    21,    -1,
      -1,    -1,    -1,    16,    17,    18,    19,    20,    21,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    16,    17,    18,    19,    20,    21,    -1,    -1,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    -1,    -1,    76,    -1,    -1,    -1,    -1,    -1,    72,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      16,    17,    18,    19,    20,    21,    -1,    -1,    72,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    16,    17,
      18,    19,    20,    21,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    16,    17,
      18,    19,    20,    21,    -1,    71,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    16,    17,    18,    19,
      20,    21,    -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    16,    17,    18,    19,
      20,    21,    -1,    71,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    16,    17,    18,    19,    20,    21,
      -1,    71,    -1,    -1,    -1,    -1,    28,    16,    17,    18,
      19,    20,    21,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    -1,    -1,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    84,    85,     0,    10,    87,    88,   126,   127,   134,
     135,   147,   148,   135,    66,    86,   128,    61,   142,   157,
     158,     9,    38,    49,    72,   136,   137,   138,   139,    49,
      70,   137,    11,    12,   143,    66,    71,     3,    67,   174,
     175,    11,    12,   143,    45,    11,    12,   129,    15,    89,
     142,   157,   175,     3,    59,    66,    69,   130,   131,   142,
       1,     4,     5,     6,     9,    11,    12,    13,    14,    39,
      42,    43,    44,    46,    47,    48,    50,    51,    52,    57,
      60,    64,    65,    67,    73,   108,   109,   110,   112,   113,
     114,   115,   116,   117,   141,   143,   145,   146,   159,   160,
     169,   170,   171,   173,   174,   176,   177,    67,   143,    68,
     175,    66,    71,   143,    15,   108,    39,    67,   111,    39,
      67,   109,   110,   143,    11,   107,   108,   145,    67,    81,
      67,    81,    67,   111,    67,    82,   108,   108,   108,     9,
      61,    69,    75,   107,   137,    67,    16,    17,    18,    19,
      20,    21,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    14,    15,    37,    40,    41,    74,    75,   108,
      39,   111,   111,    67,    71,    72,   109,   170,   171,    11,
      12,   137,   140,   144,   149,   150,   151,   130,   108,     7,
       8,   172,   143,   108,   162,   163,   164,    50,   143,    71,
      72,    71,    72,   108,    67,    69,   101,   168,   108,   168,
      49,   164,   107,   101,   107,   164,   108,   161,   165,   166,
      68,   142,    83,   108,   108,   108,   108,   108,   108,   108,
     108,   108,   108,   108,   108,   108,   108,   108,   108,   108,
     143,    75,   143,    75,   143,    27,    58,   107,   143,   164,
     164,   149,   163,    68,   169,   142,    68,    28,    71,   108,
      28,    71,    68,   163,    68,   108,    68,    71,   107,    80,
      71,    11,    12,    68,    68,    68,    70,    72,    71,    76,
      68,   107,    72,   111,   107,   107,    58,   107,    27,    76,
      68,    68,    68,    71,    72,   144,    90,   151,   172,   162,
      72,   108,    68,     1,    22,    24,    29,    30,    31,    32,
      34,    35,    36,    66,    92,    93,    94,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   107,   118,   137,   108,
     133,   133,   111,    68,   108,   161,    68,    68,   108,   164,
      76,    76,   107,    27,    76,    58,    76,   107,   111,   101,
      68,    72,   152,     1,    66,    91,   101,    68,    71,   167,
      66,    67,    67,    67,    78,    67,    67,    66,    66,    66,
     107,   102,    70,   142,   153,   154,   102,    66,    69,    71,
      68,    71,    68,   164,    68,    76,    58,    76,   107,   108,
     107,    76,   164,    42,   109,    68,   102,   107,   107,   107,
      92,   107,   122,   123,   140,   155,   156,    11,    12,   124,
     125,   155,    66,   144,    66,    71,   106,   108,   132,   143,
      68,   107,   108,    76,    76,    76,    68,   107,   167,    68,
      68,    68,    29,    66,   142,    15,    71,    33,    15,   153,
      25,    26,   105,   120,   137,   167,    72,    76,    76,    72,
      92,    78,    67,   122,   144,   108,   124,   108,   108,     4,
      27,    60,    64,    65,    67,   121,   175,   178,    72,   153,
      92,   119,   120,    68,   108,    68,    23,    95,    92,   107,
      66,    68,   121,     4,     4,     4,   178,    27,    72,    19,
      20,    21,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    66,   119,    70,   119,    92,    68,   122,    79,    72,
      68,    72,   121,    67,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,    66,    68,    92,    72,
      78,    92
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      85,    86,    86,    87,    87,    87,    87,    87,    89,    90,
      88,    91,    91,    91,    92,    92,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    93,    93,    94,    95,
      95,    96,    97,    98,    99,   100,   101,   102,   102,   102,
     102,   103,   104,   105,   106,   106,   107,   107,   108,   108,
     108,   108,   108,   108,   108,   108,   108,   108,   108,   108,
     108,   108,   108,   108,   108,   108,   108,   108,   108,   108,
     108,   108,   108,   108,   108,   108,   108,   108,   108,   109,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   111,   112,   112,   112,   112,   112,   112,
     112,   112,   112,   113,   114,   115,   116,   117,   117,   118,
     119,   119,   119,   120,   120,   120,   120,   120,   121,   121,
     122,   122,   123,   123,   124,   124,   124,   125,   125,   126,
     127,   128,   129,   128,   130,   130,   131,   132,   133,   133,
     134,   135,   135,   136,   136,   137,   137,   138,   138,   139,
     139,   139,   140,   140,   141,   142,   142,   143,   143,   144,
     144,   145,   145,   146,   146,   146,   146,   147,   148,   149,
     149,   149,   150,   150,   151,   151,   151,   152,   152,   153,
     153,   154,   154,   155,   156,   157,   157,   158,   158,   159,
     160,   161,   162,   162,   163,   163,   164,   164,   164,   165,
     165,   166,   166,   166,   167,   167,   168,   168,   169,   169,
     170,   171,   172,   172,   173,   173,   173,   173,   174,   174,
     175,   175,   175,   176,   177,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     0,     0,     0,     0,     0,     1,     3,
       0,     0,     1,     1,     3,     1,     1,     1,     0,     0,
       9,     1,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     2,     3,     6,     0,
       2,     6,     8,    10,     8,     6,     4,     0,     2,     2,
       3,     1,     3,     1,     0,     4,     1,     3,     2,     3,
       3,     5,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     1,     5,     3,
       3,     3,     5,     5,     4,     5,     6,     7,     8,     7,
       5,     6,     6,     8,     7,     5,     3,     5,     4,     6,
       4,     5,     5,     1,     4,     4,     6,     6,     4,     4,
       7,     6,     7,     7,     9,     3,     3,     3,     4,     4,
       2,     2,     0,     3,     5,     4,     4,     2,     1,     1,
       0,     1,     1,     1,     1,     1,     1,     1,     3,     3,
       4,     0,     0,     5,     1,     3,     2,     3,     0,     3,
       2,     0,     2,     1,     0,     1,     2,     1,     0,     1,
       2,     2,     1,     2,     4,     0,     1,     1,     1,     1,
       1,     3,     3,     1,     2,     3,     3,     4,     2,     0,
       1,     2,     1,     3,     2,     4,     1,     0,     5,     1,
       3,     2,     4,     3,     3,     1,     3,     2,     4,     1,
       2,     3,     1,     2,     1,     3,     0,     1,     2,     1,
       3,     0,     1,     2,     0,     3,     1,     3,     1,     1,
       1,     3,     1,     3,     1,     1,     2,     2,     1,     2,
       1,     3,     3,     1,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     1,     2,
       2,     2
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
#line 243 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { (yyval.number) = rule_loop_open(); }
#line 2646 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 3: /* foreach_start: %empty  */
#line 244 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                      { (yyval.number) = rule_foreach_open(); }
#line 2652 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 4: /* block_start: %empty  */
#line 245 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.number) = rule_block_open(); }
#line 2658 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 5: /* special_context_start: %empty  */
#line 246 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { (yyval.number) = rule_special_context_open(); }
#line 2664 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 6: /* tree_context_start: %empty  */
#line 247 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                           { (yyval.number) = rule_tree_context_open(); }
#line 2670 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 7: /* dollar_start: %empty  */
#line 248 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                     { (yyval.contextp) = rule_dollar_open(); }
#line 2676 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 8: /* all: program  */
#line 257 "$REPO_ROOT$/src/compiler/internal/grammar.y"
           { rule_program((yyvsp[0].node)); }
#line 2682 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 9: /* program: program def opt_semicolon  */
#line 262 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_program_append(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 2688 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 10: /* program: %empty  */
#line 263 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.node) = 0; }
#line 2694 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 12: /* opt_semicolon: ';'  */
#line 269 "$REPO_ROOT$/src/compiler/internal/grammar.y"
         { rule_opt_semicolon(); }
#line 2700 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 14: /* def: type name_list ';'  */
#line 281 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_def_global_var((yyvsp[-2].number)); (yyval.node) = 0; }
#line 2706 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 18: /* @1: %empty  */
#line 307 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_func_type((yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].string)); }
#line 2712 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 19: /* @2: %empty  */
#line 309 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_func_proto((yyvsp[-3].number), (yyvsp[-5].number), (yyvsp[-4].string), &(yyvsp[-4].shared_string), (yyvsp[-1].argument)); }
#line 2718 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 20: /* function: type optional_star identifier @1 '(' argument ')' @2 block_or_semi  */
#line 311 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_func(&(yyval.node), (yyvsp[-5].number), (yyvsp[-7].number), (yyvsp[-6].shared_string), (yyvsp[-3].argument), &(yyvsp[-1].number), &(yyvsp[0].node)); }
#line 2724 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 21: /* block_or_semi: block  */
#line 316 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = rule_block_or_semi((yyvsp[0].decl).node); }
#line 2730 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 22: /* block_or_semi: ';'  */
#line 317 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2736 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 23: /* block_or_semi: error  */
#line 318 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2742 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 24: /* statement: comma_expr ';'  */
#line 328 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_expr((yyvsp[-1].node)); }
#line 2748 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 28: /* statement: stmt_for  */
#line 332 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2754 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 29: /* statement: stmt_foreach  */
#line 333 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2760 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 32: /* statement: block  */
#line 336 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2766 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 33: /* statement: ';'  */
#line 337 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = 0; }
#line 2772 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 34: /* statement: L_BREAK ';'  */
#line 338 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_break(); }
#line 2778 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 35: /* statement: L_CONTINUE ';'  */
#line 339 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_continue(); }
#line 2784 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 36: /* stmt_return: L_RETURN ';'  */
#line 345 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_return_void(&(yyval.node)); }
#line 2790 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 37: /* stmt_return: L_RETURN comma_expr ';'  */
#line 346 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_return_expr(&(yyval.node), (yyvsp[-1].node)); }
#line 2796 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 38: /* stmt_cond: L_IF '(' comma_expr ')' statement optional_else_part  */
#line 352 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_cond(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 2802 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 39: /* optional_else_part: %empty  */
#line 356 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                { (yyval.node) = 0; }
#line 2808 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 40: /* optional_else_part: L_ELSE statement  */
#line 357 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                 { (yyval.node) = (yyvsp[0].node); }
#line 2814 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 41: /* stmt_while: L_WHILE '(' comma_expr ')' loop_start statement  */
#line 363 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_while(&(yyval.node), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2820 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 42: /* stmt_do: L_DO loop_start statement L_WHILE '(' comma_expr ')' ';'  */
#line 369 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_do(&(yyval.node), (yyvsp[-5].node), (yyvsp[-2].node), (yyvsp[-6].number)); }
#line 2826 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 43: /* stmt_for: L_FOR '(' for_init ';' for_expr ';' for_expr ')' loop_start statement  */
#line 375 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for(&(yyval.decl), &(yyvsp[-7].decl), (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2832 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 44: /* stmt_foreach: L_FOREACH '(' foreach_vars L_IN expr ')' foreach_start statement  */
#line 381 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach(&(yyval.decl), &(yyvsp[-5].decl), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2838 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 45: /* stmt_switch: switch_header '{' local_declarations case switch_block '}'  */
#line 387 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_switch(&(yyval.node), (yyvsp[-5].switch_header).expr, &(yyvsp[-3].decl), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[-5].switch_header).saved_context, (yyvsp[-5].switch_header).saved_cases_size); }
#line 2844 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 46: /* block: '{' block_start statements '}'  */
#line 399 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block(&(yyval.decl), (yyvsp[-1].decl).node, (yyvsp[-2].number)); }
#line 2850 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 47: /* statements: %empty  */
#line 405 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_empty(&(yyval.decl)); }
#line 2856 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 48: /* statements: statement statements  */
#line 407 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_stmt(&(yyval.decl), (yyvsp[-1].node), &(yyvsp[0].decl)); }
#line 2862 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 49: /* statements: local_declaration_statement statements  */
#line 409 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_decl(&(yyval.decl), &(yyvsp[-1].decl), &(yyvsp[0].decl)); }
#line 2868 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 50: /* statements: error ';' statements  */
#line 411 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_error(&(yyval.decl), &(yyvsp[0].decl)); }
#line 2874 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 51: /* local_decl_statement_header: basic_type  */
#line 416 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { rule_local_declaration_statement_set_type((yyvsp[0].number)); (yyval.number) = (yyvsp[0].number); }
#line 2880 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 52: /* local_declaration_statement: local_decl_statement_header local_name_list ';'  */
#line 421 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_declaration_statement(&(yyval.decl), &(yyvsp[-1].decl)); }
#line 2886 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 53: /* local_decl_header: basic_type  */
#line 426 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { rule_local_declarations_set_type((yyvsp[0].number)); (yyval.number) = (yyvsp[0].number); }
#line 2892 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 54: /* local_declarations: %empty  */
#line 431 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.decl).node = 0; (yyval.decl).num = 0; }
#line 2898 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 55: /* local_declarations: local_declarations local_decl_header local_name_list ';'  */
#line 433 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_declarations(&(yyval.decl), &(yyvsp[-3].decl), &(yyvsp[-1].decl)); }
#line 2904 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 57: /* comma_expr: comma_expr ',' expr  */
#line 452 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                { rule_comma_expr(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2910 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 58: /* expr: ref lvalue  */
#line 456 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_ref(&(yyval.node), (yyvsp[0].node)); }
#line 2916 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 59: /* expr: lvalue L_ASSIGN expr  */
#line 457 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_assign(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 2922 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 60: /* expr: error L_ASSIGN expr  */
#line 458 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_assign_error(&(yyval.node), (yyvsp[0].node)); }
#line 2928 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 61: /* expr: expr '?' expr ':' expr  */
#line 460 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                            { rule_expr_ternary(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2934 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 62: /* expr: expr L_QUESTION_QUESTION expr  */
#line 461 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_nullish(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2940 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 63: /* expr: expr L_LOR expr  */
#line 462 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_lor(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2946 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 64: /* expr: expr L_LAND expr  */
#line 463 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_land(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2952 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 65: /* expr: expr '|' expr  */
#line 465 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_or(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2958 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 66: /* expr: expr '^' expr  */
#line 466 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_xor(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2964 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 67: /* expr: expr '&' expr  */
#line 467 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_and(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2970 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 68: /* expr: expr L_EQ_NE expr  */
#line 469 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_expr_eq_ne(&(yyval.node), (yyvsp[-1].number), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2976 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 69: /* expr: expr L_ORDER expr  */
#line 470 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_order(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 2982 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 70: /* expr: expr '<' expr  */
#line 471 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_lt(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2988 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 71: /* expr: expr L_SHIFT expr  */
#line 473 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_expr_shift(&(yyval.node), (yyvsp[-1].number), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2994 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 72: /* expr: expr '+' expr  */
#line 475 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             {
        rule_set_operand_ranges((yylsp[-2]).first_line, (yylsp[-2]).first_column, (yylsp[-2]).last_column,
                                (yylsp[-1]).first_line, (yylsp[-1]).first_column,
                                (yylsp[0]).first_line, (yylsp[0]).first_column, (yylsp[0]).last_column);
        rule_expr_add(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node));
        rule_clear_operand_ranges();
    }
#line 3006 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 73: /* expr: expr '-' expr  */
#line 482 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_sub(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3012 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 74: /* expr: expr '*' expr  */
#line 483 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_mul(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3018 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 75: /* expr: expr '%' expr  */
#line 484 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_mod(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3024 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 76: /* expr: expr '/' expr  */
#line 485 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_div(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3030 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 77: /* expr: cast expr  */
#line 487 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_cast(&(yyval.node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3036 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 78: /* expr: L_INC_DEC lvalue  */
#line 488 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_expr_pre_incdec(&(yyval.node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3042 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 79: /* expr: '!' expr  */
#line 489 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_expr_not(&(yyval.node), (yyvsp[0].node)); }
#line 3048 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 80: /* expr: '~' expr  */
#line 490 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_expr_compl(&(yyval.node), (yyvsp[0].node)); }
#line 3054 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 81: /* expr: '-' expr  */
#line 491 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_neg(&(yyval.node), (yyvsp[0].node)); }
#line 3060 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 82: /* expr: lvalue L_INC_DEC  */
#line 493 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_expr_post_incdec(&(yyval.node), (yyvsp[0].number), (yyvsp[-1].node)); }
#line 3066 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 89: /* lvalue: primary_expr  */
#line 505 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                { (yyval.node) = rule_lvalue((yyvsp[0].node)); }
#line 3072 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 91: /* primary_expr: L_DEFINED_NAME  */
#line 515 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_defined_name(&(yyval.node), (yyvsp[0].ihe)); }
#line 3078 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 92: /* primary_expr: L_IDENTIFIER  */
#line 516 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_identifier(&(yyval.node), (yyvsp[0].string)); }
#line 3084 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 93: /* primary_expr: L_PARAMETER  */
#line 517 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_parameter(&(yyval.node), (yyvsp[0].number)); }
#line 3090 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 95: /* primary_expr: '(' comma_expr ')'  */
#line 519 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { (yyval.node) = (yyvsp[-1].node); }
#line 3096 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 98: /* primary_expr: '$' '(' dollar_start comma_expr ')'  */
#line 525 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_dollar_expr(&(yyval.node), (yyvsp[-2].contextp), (yyvsp[-1].node)); }
#line 3102 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 99: /* primary_expr: primary_expr L_ARROW identifier  */
#line 528 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                 { rule_primary_expr_member_arrow(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 3108 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 100: /* primary_expr: primary_expr '.' identifier  */
#line 529 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                 { rule_primary_expr_member_dot(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 3114 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 101: /* primary_expr: primary_expr L_OPTIONAL_DOT identifier  */
#line 535 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_member_optional(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 3120 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 102: /* primary_expr: primary_expr L_OPTIONAL_DOT '[' comma_expr ']'  */
#line 537 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_optional(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3126 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 103: /* primary_expr: primary_expr L_DOT_OPTIONAL '[' comma_expr ']'  */
#line 539 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_optional(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3132 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 104: /* primary_expr: primary_expr '[' comma_expr ']'  */
#line 544 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3138 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 105: /* primary_expr: primary_expr '[' '<' comma_expr ']'  */
#line 546 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_r(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3144 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 106: /* primary_expr: primary_expr '[' comma_expr L_RANGE comma_expr ']'  */
#line 548 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nn(&(yyval.node), (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3150 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 107: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE comma_expr ']'  */
#line 550 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rn(&(yyval.node), (yyvsp[-6].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3156 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 108: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE '<' comma_expr ']'  */
#line 552 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rr(&(yyval.node), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3162 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 109: /* primary_expr: primary_expr '[' comma_expr L_RANGE '<' comma_expr ']'  */
#line 554 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nr(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3168 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 110: /* primary_expr: primary_expr '[' comma_expr L_RANGE ']'  */
#line 556 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_ne(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node)); }
#line 3174 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 111: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE ']'  */
#line 558 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_re(&(yyval.node), (yyvsp[-5].node), (yyvsp[-2].node)); }
#line 3180 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 112: /* primary_expr: primary_expr '[' L_RANGE '<' comma_expr ']'  */
#line 560 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nr(&(yyval.node), (yyvsp[-5].node), 0, (yyvsp[-1].node)); }
#line 3186 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 113: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE '<' expr ']'  */
#line 563 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rr(&(yyval.node), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3192 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 114: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE expr ']'  */
#line 565 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rn(&(yyval.node), (yyvsp[-6].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3198 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 115: /* primary_expr: lambda_return_type '(' argument ')' block  */
#line 570 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_anon_func(&(yyval.node), &(yyvsp[-4].func_block), &(yyvsp[-2].argument), &(yyvsp[0].decl)); }
#line 3204 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 116: /* primary_expr: functional_open ':' ')'  */
#line 577 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_1(&(yyval.node), (yyvsp[-2].number)); }
#line 3210 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 117: /* primary_expr: functional_open ',' arg_list ':' ')'  */
#line 579 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_2(&(yyval.node), (yyvsp[-4].number), (yyvsp[-2].node)); }
#line 3216 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 118: /* primary_expr: L_FUNCTION_OPEN L_DEFINED_NAME ':' ')'  */
#line 588 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_1(&(yyval.node), rule_functional_ref((yyvsp[-2].ihe))); }
#line 3222 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 119: /* primary_expr: L_FUNCTION_OPEN L_DEFINED_NAME ',' arg_list ':' ')'  */
#line 590 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_2(&(yyval.node), rule_functional_ref((yyvsp[-4].ihe)), (yyvsp[-2].node)); }
#line 3228 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 120: /* primary_expr: L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 592 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_3(&(yyval.node), (yyvsp[-2].node)); }
#line 3234 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 121: /* primary_expr: '(' '[' opt_pair_list ']' ')'  */
#line 595 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_primary_expr_mapping(&(yyval.node), (yyvsp[-2].node)); }
#line 3240 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 122: /* primary_expr: '(' '{' opt_arg_list '}' ')'  */
#line 596 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_primary_expr_array(&(yyval.node), (yyvsp[-2].node)); }
#line 3246 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 123: /* call_open: '('  */
#line 606 "$REPO_ROOT$/src/compiler/internal/grammar.y"
       { rule_call_open(&(yyval.call_open).saved_context, &(yyval.call_open).saved_refs); }
#line 3252 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 124: /* function_call: efun_override call_open opt_arg_list ')'  */
#line 620 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_efun(&(yyval.node), (yyvsp[-3].number), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3258 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 125: /* function_call: L_NEW call_open opt_arg_list ')'  */
#line 622 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new(&(yyval.node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3264 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 126: /* function_call: L_NEW '(' L_CLASS L_DEFINED_NAME opt_class_init ')'  */
#line 624 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new_class(&(yyval.node), (yyvsp[-2].ihe), (yyvsp[-1].node)); }
#line 3270 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 127: /* function_call: L_NEW '(' L_CLASS L_IDENTIFIER opt_class_init ')'  */
#line 626 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new_class_undef(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node)); }
#line 3276 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 128: /* function_call: L_DEFINED_NAME call_open opt_arg_list ')'  */
#line 628 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_defined_name(&(yyval.node), (yyvsp[-3].ihe), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3282 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 129: /* function_call: function_name call_open opt_arg_list ')'  */
#line 630 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_name(&(yyval.node), (yyvsp[-3].string), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3288 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 130: /* function_call: primary_expr '[' comma_expr ']' call_open opt_arg_list ')'  */
#line 632 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_indexed(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3294 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 131: /* function_call: primary_expr L_ARROW identifier call_open opt_arg_list ')'  */
#line 634 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_arrow(&(yyval.node), (yyvsp[-5].node), (yyvsp[-3].string), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3300 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 132: /* function_call: '(' '*' comma_expr ')' call_open opt_arg_list ')'  */
#line 636 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_star(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3306 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 133: /* sscanf: L_SSCANF '(' expr ',' expr lvalue_list ')'  */
#line 647 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_sscanf(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 3312 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 134: /* parse_command: L_PARSE_COMMAND '(' expr ',' expr ',' expr lvalue_list ')'  */
#line 653 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_parse_command(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 3318 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 135: /* catch: L_CATCH special_context_start expr_or_block  */
#line 659 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_catch(&(yyval.node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 3324 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 136: /* time_expression: L_TIME_EXPRESSION special_context_start expr_or_block  */
#line 665 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_time_expression(&(yyval.node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 3330 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 137: /* tree: L_TREE tree_context_start block  */
#line 671 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_tree_block(&(yyval.node), (yyvsp[0].decl), (yyvsp[-1].number)); }
#line 3336 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 138: /* tree: L_TREE '(' comma_expr ')'  */
#line 673 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_tree_expr(&(yyval.node), (yyvsp[-1].node)); }
#line 3342 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 139: /* switch_header: L_SWITCH '(' comma_expr ')'  */
#line 684 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.switch_header).expr = (yyvsp[-1].node); rule_switch_pre(&(yyval.switch_header).saved_context, &(yyval.switch_header).saved_cases_size); }
#line 3348 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 140: /* switch_block: case switch_block  */
#line 689 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_switch_block_case(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3354 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 141: /* switch_block: statement switch_block  */
#line 690 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_switch_block_stmt(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3360 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 142: /* switch_block: %empty  */
#line 691 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { rule_switch_block_empty(&(yyval.node)); }
#line 3366 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 143: /* case: L_CASE case_label ':'  */
#line 698 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_single(&(yyval.node), (yyvsp[-1].node)); }
#line 3372 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 144: /* case: L_CASE case_label L_RANGE case_label ':'  */
#line 700 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3378 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 145: /* case: L_CASE case_label L_RANGE ':'  */
#line 702 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range_from(&(yyval.node), (yyvsp[-2].node)); }
#line 3384 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 146: /* case: L_CASE L_RANGE case_label ':'  */
#line 704 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range_to(&(yyval.node), (yyvsp[-1].node)); }
#line 3390 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 147: /* case: L_DEFAULT ':'  */
#line 706 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_default(&(yyval.node)); }
#line 3396 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 148: /* case_label: constant  */
#line 712 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_case_label_constant(&(yyval.node), (yyvsp[0].number)); }
#line 3402 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 149: /* case_label: string_const  */
#line 713 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_case_label_string(&(yyval.node), (yyvsp[0].string)); }
#line 3408 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 150: /* for_expr: %empty  */
#line 723 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { (yyval.node) = 0; }
#line 3414 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 152: /* for_init: for_expr  */
#line 730 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for_init_expr(&(yyval.decl), (yyvsp[0].node)); }
#line 3420 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 153: /* for_init: single_new_local_def_with_init  */
#line 732 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for_init_local(&(yyval.decl), (yyvsp[0].node)); }
#line 3426 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 154: /* foreach_var: L_DEFINED_NAME  */
#line 737 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_defined(&(yyval.decl), (yyvsp[0].ihe)); }
#line 3432 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 155: /* foreach_var: single_new_local_def  */
#line 738 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_new_local(&(yyval.decl), (yyvsp[0].number)); }
#line 3438 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 156: /* foreach_var: L_IDENTIFIER  */
#line 739 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_identifier(&(yyval.decl), (yyvsp[0].string)); }
#line 3444 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 157: /* foreach_vars: foreach_var  */
#line 745 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach_vars_single(&(yyval.decl), &(yyvsp[0].decl)); }
#line 3450 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 158: /* foreach_vars: foreach_var ',' foreach_var  */
#line 747 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach_vars_double(&(yyval.decl), &(yyvsp[-2].decl), &(yyvsp[0].decl)); }
#line 3456 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 159: /* type_decl: class_header member_list '}'  */
#line 758 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_define_class_members((yyvsp[-2].class_header).ihe, (yyvsp[-2].class_header).classname_index); (yyval.node) = 0; }
#line 3462 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 160: /* class_header: type_modifier_list L_CLASS identifier '{'  */
#line 764 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.class_header).ihe = rule_define_class(&(yyval.class_header).classname_index, (yyvsp[-1].string)); }
#line 3468 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 162: /* $@3: %empty  */
#line 770 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                           { rule_member_list_set_type((yyvsp[0].number)); }
#line 3474 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 166: /* member_name: optional_star identifier  */
#line 782 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { rule_member_name((yyvsp[-1].number), (yyvsp[0].string)); }
#line 3480 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 167: /* class_init: identifier ':' expr  */
#line 787 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                       { (yyval.node) = rule_class_init((yyvsp[-2].string), (yyvsp[0].node)); }
#line 3486 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 168: /* opt_class_init: %empty  */
#line 792 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.node) = rule_opt_class_init_empty(); }
#line 3492 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 169: /* opt_class_init: opt_class_init ',' class_init  */
#line 793 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { (yyval.node) = rule_opt_class_init((yyvsp[-2].node), (yyvsp[0].node)); }
#line 3498 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 170: /* type: type_modifier_list opt_basic_type  */
#line 805 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { (yyval.number) = rule_type((yyvsp[-1].number), (yyvsp[0].number)); }
#line 3504 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 171: /* type_modifier_list: %empty  */
#line 810 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { (yyval.number) = 0; }
#line 3510 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 172: /* type_modifier_list: L_TYPE_MODIFIER type_modifier_list  */
#line 811 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                               { (yyval.number) = rule_type_modifier_list((yyvsp[-1].number), (yyvsp[0].number)); }
#line 3516 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 174: /* opt_basic_type: %empty  */
#line 817 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { (yyval.number) = rule_opt_basic_type_empty(); }
#line 3522 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 176: /* basic_type: opt_atomic_type L_ARRAY  */
#line 823 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { (yyval.number) = (yyvsp[-1].number) | TYPE_MOD_ARRAY; }
#line 3528 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 178: /* opt_atomic_type: %empty  */
#line 829 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { (yyval.number) = rule_opt_basic_type_empty(); }
#line 3534 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 180: /* atomic_type: L_CLASS L_DEFINED_NAME  */
#line 835 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.number) = rule_atomic_type_class((yyvsp[0].ihe)); }
#line 3540 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 181: /* atomic_type: L_CLASS L_IDENTIFIER  */
#line 836 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.number) = rule_atomic_type_class_identifier((yyvsp[0].string)); }
#line 3546 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 183: /* arg_type: basic_type ref  */
#line 842 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.number) = (yyvsp[-1].number) | LOCAL_MOD_REF; }
#line 3552 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 184: /* cast: '(' basic_type optional_star ')'  */
#line 847 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { (yyval.number) = rule_cast((yyvsp[-2].number), (yyvsp[-1].number)); }
#line 3558 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 185: /* optional_star: %empty  */
#line 852 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.number) = 0; }
#line 3564 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 186: /* optional_star: '*'  */
#line 853 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.number) = TYPE_MOD_ARRAY; }
#line 3570 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 187: /* identifier: L_DEFINED_NAME  */
#line 863 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { (yyval.string) = rule_identifier_defined_name((yyvsp[0].ihe)); }
#line 3576 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 190: /* new_local_name: L_DEFINED_NAME  */
#line 870 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.string) = rule_new_local_name_redefine((yyvsp[0].ihe)); }
#line 3582 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 191: /* efun_override: L_EFUN L_COLON_COLON identifier  */
#line 875 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.number) = rule_efun_override((yyvsp[0].string)); }
#line 3588 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 192: /* efun_override: L_EFUN L_COLON_COLON L_NEW  */
#line 876 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.number) = rule_efun_override_new(); }
#line 3594 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 194: /* function_name: L_COLON_COLON identifier  */
#line 887 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { (yyval.string) = rule_function_name_colon_colon((yyvsp[0].string)); }
#line 3600 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 195: /* function_name: L_BASIC_TYPE L_COLON_COLON identifier  */
#line 888 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { (yyval.string) = rule_function_name_type((yyvsp[-2].number), (yyvsp[0].string)); }
#line 3606 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 196: /* function_name: identifier L_COLON_COLON identifier  */
#line 889 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                    { (yyval.string) = rule_function_name_obj((yyvsp[-2].string), (yyvsp[0].string)); }
#line 3612 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 197: /* inheritance: type_modifier_list L_INHERIT string_const ';'  */
#line 895 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { if (rule_inheritence(&(yyval.node), (yyvsp[-3].number), (yyvsp[-1].string))) { YYACCEPT; } }
#line 3618 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 198: /* modifier_change: type_modifier_list ':'  */
#line 900 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { (yyval.node) = rule_modifier_change((yyvsp[-1].number)); }
#line 3624 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 199: /* argument: %empty  */
#line 911 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.argument).num_arg = 0; (yyval.argument).flags = 0; }
#line 3630 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 201: /* argument: argument_list L_DOT_DOT_DOT  */
#line 914 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_varargs(&(yyval.argument), &(yyvsp[-1].argument)); }
#line 3636 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 202: /* argument_list: param_decl  */
#line 920 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_list_single(&(yyval.argument), (yyvsp[0].number)); }
#line 3642 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 203: /* argument_list: argument_list ',' param_decl  */
#line 922 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_list_multi(&(yyval.argument), &(yyvsp[-2].argument), (yyvsp[0].number)); }
#line 3648 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 204: /* param_decl: arg_type optional_star  */
#line 932 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_typed((yyvsp[-1].number) | (yyvsp[0].number)); }
#line 3654 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 205: /* param_decl: arg_type optional_star new_local_name optional_default_arg_value  */
#line 934 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_typed_name((yyvsp[-3].number) | (yyvsp[-2].number), (yyvsp[-1].string), (yyvsp[0].node)); }
#line 3660 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 206: /* param_decl: new_local_name  */
#line 936 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_untyped_name((yyvsp[0].string)); }
#line 3666 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 207: /* optional_default_arg_value: %empty  */
#line 941 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                          { (yyval.node) = 0; }
#line 3672 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 208: /* optional_default_arg_value: ':' L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 942 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                            { (yyval.node) = rule_default_arg_value((yyvsp[-2].node)); }
#line 3678 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 209: /* local_name_list: new_local_def  */
#line 948 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_name_list_single(&(yyval.decl), (yyvsp[0].node)); }
#line 3684 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 210: /* local_name_list: new_local_def ',' local_name_list  */
#line 950 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_name_list_multi(&(yyval.decl), (yyvsp[-2].node), &(yyvsp[0].decl)); }
#line 3690 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 211: /* new_local_def: optional_star new_local_name  */
#line 956 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_new_local_def((yyvsp[0].string), (yyvsp[-1].number)); }
#line 3696 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 212: /* new_local_def: optional_star new_local_name L_ASSIGN expr  */
#line 958 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_new_local_def_with_init((yyvsp[-2].string), (yyvsp[-3].number), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3702 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 213: /* single_new_local_def: arg_type optional_star new_local_name  */
#line 964 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_single_new_local_def(&(yyval.number), (yyvsp[-2].number) | (yyvsp[-1].number), (yyvsp[0].string)); }
#line 3708 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 214: /* single_new_local_def_with_init: single_new_local_def L_ASSIGN expr  */
#line 970 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_single_new_local_def_with_init((yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3714 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 217: /* new_name: optional_star identifier  */
#line 986 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { rule_new_name((yyvsp[-1].number), (yyvsp[0].string)); }
#line 3720 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 218: /* new_name: optional_star identifier L_ASSIGN expr  */
#line 987 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { rule_new_name_with_init((yyvsp[-3].number), (yyvsp[-2].string), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3726 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 219: /* lambda_return_type: L_BASIC_TYPE  */
#line 998 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                { rule_lambda_return_type(&(yyval.func_block), (yyvsp[0].number)); }
#line 3732 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 220: /* functional_open: L_FUNCTION_OPEN efun_override  */
#line 1006 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                 { (yyval.number) = rule_functional_open((yyvsp[0].number)); }
#line 3738 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 221: /* assoc_pair: expr ':' expr  */
#line 1016 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_assoc_pair(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3744 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 222: /* spread_expr: expr  */
#line 1021 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_spread_expr_normal(&(yyval.node), (yyvsp[0].node)); }
#line 3750 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 223: /* spread_expr: expr L_DOT_DOT_DOT  */
#line 1022 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_spread_expr_dots(&(yyval.node), (yyvsp[-1].node)); }
#line 3756 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 224: /* arg_list: spread_expr  */
#line 1028 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_arg_list_single(&(yyval.node), (yyvsp[0].node)); }
#line 3762 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 225: /* arg_list: arg_list ',' spread_expr  */
#line 1030 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_arg_list_multi(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3768 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 226: /* opt_arg_list: %empty  */
#line 1036 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list_empty(&(yyval.node)); }
#line 3774 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 227: /* opt_arg_list: arg_list  */
#line 1037 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list(&(yyval.node), (yyvsp[0].node)); }
#line 3780 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 228: /* opt_arg_list: arg_list ','  */
#line 1038 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list(&(yyval.node), (yyvsp[-1].node)); }
#line 3786 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 229: /* pair_list: assoc_pair  */
#line 1044 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_pair_list_single(&(yyval.node), (yyvsp[0].node)); }
#line 3792 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 230: /* pair_list: pair_list ',' assoc_pair  */
#line 1046 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_pair_list_multi(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3798 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 231: /* opt_pair_list: %empty  */
#line 1052 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list_empty(&(yyval.node)); }
#line 3804 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 232: /* opt_pair_list: pair_list  */
#line 1053 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list(&(yyval.node), (yyvsp[0].node)); }
#line 3810 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 233: /* opt_pair_list: pair_list ','  */
#line 1054 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list(&(yyval.node), (yyvsp[-1].node)); }
#line 3816 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 234: /* lvalue_list: %empty  */
#line 1059 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.node) = rule_lvalue_list_empty(); }
#line 3822 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 235: /* lvalue_list: ',' lvalue lvalue_list  */
#line 1060 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { (yyval.node) = rule_lvalue_list((yyvsp[-1].node), (yyvsp[0].node)); }
#line 3828 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 236: /* expr_or_block: block  */
#line 1065 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_expr_or_block_block((yyvsp[0].decl)); }
#line 3834 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 237: /* expr_or_block: '(' comma_expr ')'  */
#line 1066 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_expr_or_block_expr((yyvsp[-1].node)); }
#line 3840 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 240: /* string: string_literal  */
#line 1082 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_string(&(yyval.node), (yyvsp[0].string)); }
#line 3846 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 241: /* template_literal: L_TEMPLATE_HEAD expr template_parts  */
#line 1094 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_template_literal(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3852 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 242: /* template_parts: L_TEMPLATE_TAIL  */
#line 1098 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                      { rule_template_parts_tail(&(yyval.node), (yyvsp[0].string)); }
#line 3858 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 243: /* template_parts: L_TEMPLATE_MIDDLE expr template_parts  */
#line 1099 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                       { rule_template_parts_middle(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3864 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 246: /* string_like: string_like string  */
#line 1110 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                               { rule_string_like_concat(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3870 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 247: /* string_like: string_like template_literal  */
#line 1111 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_string_like_concat(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3876 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 249: /* string_literal: string_literal L_STRING  */
#line 1119 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.string) = rule_string_literal_concat((yyvsp[-1].string), (yyvsp[0].string)); }
#line 3882 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 251: /* string_const: '(' string_const ')'  */
#line 1126 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                        { (yyval.string) = (yyvsp[-1].string); }
#line 3888 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 252: /* string_const: string_const '+' string_const  */
#line 1127 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { (yyval.string) = rule_string_literal_concat((yyvsp[-2].string), (yyvsp[0].string)); }
#line 3894 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 253: /* number: L_NUMBER  */
#line 1131 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { rule_number(&(yyval.node), (yyvsp[0].number)); }
#line 3900 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 254: /* real: L_REAL  */
#line 1135 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { rule_real(&(yyval.node), (yyvsp[0].real)); }
#line 3906 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 255: /* constant: constant '|' constant  */
#line 1141 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_or(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3912 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 256: /* constant: constant '^' constant  */
#line 1142 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_xor(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3918 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 257: /* constant: constant '&' constant  */
#line 1143 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_and(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3924 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 258: /* constant: constant L_EQ_NE constant  */
#line 1144 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                             { rule_constant_eq_ne(&(yyval.number), (yyvsp[-1].number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3930 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 259: /* constant: constant L_ORDER constant  */
#line 1145 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { rule_constant_order(&(yyval.number), (yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].number)); }
#line 3936 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 260: /* constant: constant '<' constant  */
#line 1146 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_lt(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3942 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 261: /* constant: constant L_SHIFT constant  */
#line 1147 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                             { rule_constant_shift(&(yyval.number), (yyvsp[-1].number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3948 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 262: /* constant: constant '+' constant  */
#line 1148 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_add(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3954 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 263: /* constant: constant '-' constant  */
#line 1149 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_sub(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3960 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 264: /* constant: constant '*' constant  */
#line 1150 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_mul(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3966 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 265: /* constant: constant '%' constant  */
#line 1151 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_mod(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3972 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 266: /* constant: constant '/' constant  */
#line 1152 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_div(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3978 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 267: /* constant: '(' constant ')'  */
#line 1153 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { (yyval.number) = (yyvsp[-1].number); }
#line 3984 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 268: /* constant: L_NUMBER  */
#line 1154 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { (yyval.number) = (yyvsp[0].number); }
#line 3990 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 269: /* constant: '-' L_NUMBER  */
#line 1155 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_neg(&(yyval.number), (yyvsp[0].number)); }
#line 3996 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 270: /* constant: '!' L_NUMBER  */
#line 1156 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_not(&(yyval.number), (yyvsp[0].number)); }
#line 4002 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 271: /* constant: '~' L_NUMBER  */
#line 1157 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_compl(&(yyval.number), (yyvsp[0].number)); }
#line 4008 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;


#line 4012 "$BUILD_ROOT$/src/grammar.autogen.cc"

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
#line 1160 "$REPO_ROOT$/src/compiler/internal/grammar.y"


// Public accessor for the parser's symbol-name table: maps a raw yylex
// token number (what lpcc --tokens prints) to its grammar spelling
// ("L_IDENTIFIER", "'{'"). Lives in the epilogue because yysymbol_name()
// and YYTRANSLATE are file-static in the generated parser; consumed by
// the lpcc --json staged outputs (stage_output.cc).
const char* lpc_token_name(int token) {
  return yysymbol_name(YY_CAST(yysymbol_kind_t, YYTRANSLATE(token)));
}
/* FluffOS generated-from grammar.y sha256=9915989013c5561244dc8f5dca29b32351de8145eb2c910aac08038fe0697f99 */
