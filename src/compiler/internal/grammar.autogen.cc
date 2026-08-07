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
  YYSYMBOL_L_AWAIT = 45,                   /* L_AWAIT  */
  YYSYMBOL_L_ACATCH = 46,                  /* L_ACATCH  */
  YYSYMBOL_L_ARRAY = 47,                   /* L_ARRAY  */
  YYSYMBOL_L_REF = 48,                     /* L_REF  */
  YYSYMBOL_L_PARSE_COMMAND = 49,           /* L_PARSE_COMMAND  */
  YYSYMBOL_L_TIME_EXPRESSION = 50,         /* L_TIME_EXPRESSION  */
  YYSYMBOL_L_CLASS = 51,                   /* L_CLASS  */
  YYSYMBOL_L_NEW = 52,                     /* L_NEW  */
  YYSYMBOL_L_PARAMETER = 53,               /* L_PARAMETER  */
  YYSYMBOL_L_TREE = 54,                    /* L_TREE  */
  YYSYMBOL_LOWER_THAN_ELSE = 55,           /* LOWER_THAN_ELSE  */
  YYSYMBOL_56_ = 56,                       /* '?'  */
  YYSYMBOL_57_ = 57,                       /* '|'  */
  YYSYMBOL_58_ = 58,                       /* '^'  */
  YYSYMBOL_59_ = 59,                       /* '&'  */
  YYSYMBOL_60_ = 60,                       /* '<'  */
  YYSYMBOL_61_ = 61,                       /* '+'  */
  YYSYMBOL_62_ = 62,                       /* '-'  */
  YYSYMBOL_63_ = 63,                       /* '*'  */
  YYSYMBOL_64_ = 64,                       /* '%'  */
  YYSYMBOL_65_ = 65,                       /* '/'  */
  YYSYMBOL_66_ = 66,                       /* '!'  */
  YYSYMBOL_67_ = 67,                       /* '~'  */
  YYSYMBOL_68_ = 68,                       /* ';'  */
  YYSYMBOL_69_ = 69,                       /* '('  */
  YYSYMBOL_70_ = 70,                       /* ')'  */
  YYSYMBOL_71_ = 71,                       /* '{'  */
  YYSYMBOL_72_ = 72,                       /* '}'  */
  YYSYMBOL_73_ = 73,                       /* ','  */
  YYSYMBOL_74_ = 74,                       /* ':'  */
  YYSYMBOL_75_ = 75,                       /* '$'  */
  YYSYMBOL_76_ = 76,                       /* '.'  */
  YYSYMBOL_77_ = 77,                       /* '['  */
  YYSYMBOL_78_ = 78,                       /* ']'  */
  YYSYMBOL_YYACCEPT = 79,                  /* $accept  */
  YYSYMBOL_loop_start = 80,                /* loop_start  */
  YYSYMBOL_foreach_start = 81,             /* foreach_start  */
  YYSYMBOL_block_start = 82,               /* block_start  */
  YYSYMBOL_special_context_start = 83,     /* special_context_start  */
  YYSYMBOL_dollar_start = 84,              /* dollar_start  */
  YYSYMBOL_all = 85,                       /* all  */
  YYSYMBOL_program = 86,                   /* program  */
  YYSYMBOL_opt_semicolon = 87,             /* opt_semicolon  */
  YYSYMBOL_def = 88,                       /* def  */
  YYSYMBOL_function = 89,                  /* function  */
  YYSYMBOL_90_1 = 90,                      /* @1  */
  YYSYMBOL_91_2 = 91,                      /* @2  */
  YYSYMBOL_block_or_semi = 92,             /* block_or_semi  */
  YYSYMBOL_statement = 93,                 /* statement  */
  YYSYMBOL_stmt_return = 94,               /* stmt_return  */
  YYSYMBOL_stmt_cond = 95,                 /* stmt_cond  */
  YYSYMBOL_optional_else_part = 96,        /* optional_else_part  */
  YYSYMBOL_stmt_while = 97,                /* stmt_while  */
  YYSYMBOL_stmt_do = 98,                   /* stmt_do  */
  YYSYMBOL_stmt_for = 99,                  /* stmt_for  */
  YYSYMBOL_stmt_foreach = 100,             /* stmt_foreach  */
  YYSYMBOL_stmt_switch = 101,              /* stmt_switch  */
  YYSYMBOL_block = 102,                    /* block  */
  YYSYMBOL_statements = 103,               /* statements  */
  YYSYMBOL_local_decl_statement_header = 104, /* local_decl_statement_header  */
  YYSYMBOL_local_declaration_statement = 105, /* local_declaration_statement  */
  YYSYMBOL_local_decl_header = 106,        /* local_decl_header  */
  YYSYMBOL_local_declarations = 107,       /* local_declarations  */
  YYSYMBOL_comma_expr = 108,               /* comma_expr  */
  YYSYMBOL_expr = 109,                     /* expr  */
  YYSYMBOL_lvalue = 110,                   /* lvalue  */
  YYSYMBOL_primary_expr = 111,             /* primary_expr  */
  YYSYMBOL_call_open = 112,                /* call_open  */
  YYSYMBOL_function_call = 113,            /* function_call  */
  YYSYMBOL_sscanf = 114,                   /* sscanf  */
  YYSYMBOL_parse_command = 115,            /* parse_command  */
  YYSYMBOL_catch = 116,                    /* catch  */
  YYSYMBOL_acatch = 117,                   /* acatch  */
  YYSYMBOL_acatch_context_start = 118,     /* acatch_context_start  */
  YYSYMBOL_time_expression = 119,          /* time_expression  */
  YYSYMBOL_tree = 120,                     /* tree  */
  YYSYMBOL_switch_header = 121,            /* switch_header  */
  YYSYMBOL_switch_block = 122,             /* switch_block  */
  YYSYMBOL_case = 123,                     /* case  */
  YYSYMBOL_case_label = 124,               /* case_label  */
  YYSYMBOL_for_expr = 125,                 /* for_expr  */
  YYSYMBOL_for_init = 126,                 /* for_init  */
  YYSYMBOL_foreach_var = 127,              /* foreach_var  */
  YYSYMBOL_foreach_vars = 128,             /* foreach_vars  */
  YYSYMBOL_type_decl = 129,                /* type_decl  */
  YYSYMBOL_class_header = 130,             /* class_header  */
  YYSYMBOL_member_list = 131,              /* member_list  */
  YYSYMBOL_132_3 = 132,                    /* $@3  */
  YYSYMBOL_member_name_list = 133,         /* member_name_list  */
  YYSYMBOL_member_name = 134,              /* member_name  */
  YYSYMBOL_class_init = 135,               /* class_init  */
  YYSYMBOL_opt_class_init = 136,           /* opt_class_init  */
  YYSYMBOL_type = 137,                     /* type  */
  YYSYMBOL_type_modifier_list = 138,       /* type_modifier_list  */
  YYSYMBOL_opt_basic_type = 139,           /* opt_basic_type  */
  YYSYMBOL_basic_type = 140,               /* basic_type  */
  YYSYMBOL_opt_atomic_type = 141,          /* opt_atomic_type  */
  YYSYMBOL_atomic_type = 142,              /* atomic_type  */
  YYSYMBOL_arg_type = 143,                 /* arg_type  */
  YYSYMBOL_cast = 144,                     /* cast  */
  YYSYMBOL_optional_star = 145,            /* optional_star  */
  YYSYMBOL_identifier = 146,               /* identifier  */
  YYSYMBOL_new_local_name = 147,           /* new_local_name  */
  YYSYMBOL_efun_override = 148,            /* efun_override  */
  YYSYMBOL_function_name = 149,            /* function_name  */
  YYSYMBOL_inheritance = 150,              /* inheritance  */
  YYSYMBOL_modifier_change = 151,          /* modifier_change  */
  YYSYMBOL_argument = 152,                 /* argument  */
  YYSYMBOL_argument_list = 153,            /* argument_list  */
  YYSYMBOL_param_decl = 154,               /* param_decl  */
  YYSYMBOL_optional_default_arg_value = 155, /* optional_default_arg_value  */
  YYSYMBOL_local_name_list = 156,          /* local_name_list  */
  YYSYMBOL_new_local_def = 157,            /* new_local_def  */
  YYSYMBOL_single_new_local_def = 158,     /* single_new_local_def  */
  YYSYMBOL_single_new_local_def_with_init = 159, /* single_new_local_def_with_init  */
  YYSYMBOL_name_list = 160,                /* name_list  */
  YYSYMBOL_new_name = 161,                 /* new_name  */
  YYSYMBOL_lambda_return_type = 162,       /* lambda_return_type  */
  YYSYMBOL_functional_open = 163,          /* functional_open  */
  YYSYMBOL_assoc_pair = 164,               /* assoc_pair  */
  YYSYMBOL_spread_expr = 165,              /* spread_expr  */
  YYSYMBOL_arg_list = 166,                 /* arg_list  */
  YYSYMBOL_opt_arg_list = 167,             /* opt_arg_list  */
  YYSYMBOL_pair_list = 168,                /* pair_list  */
  YYSYMBOL_opt_pair_list = 169,            /* opt_pair_list  */
  YYSYMBOL_lvalue_list = 170,              /* lvalue_list  */
  YYSYMBOL_expr_or_block = 171,            /* expr_or_block  */
  YYSYMBOL_ref = 172,                      /* ref  */
  YYSYMBOL_string = 173,                   /* string  */
  YYSYMBOL_template_literal = 174,         /* template_literal  */
  YYSYMBOL_template_parts = 175,           /* template_parts  */
  YYSYMBOL_string_like = 176,              /* string_like  */
  YYSYMBOL_string_literal = 177,           /* string_literal  */
  YYSYMBOL_string_const = 178,             /* string_const  */
  YYSYMBOL_number = 179,                   /* number  */
  YYSYMBOL_real = 180,                     /* real  */
  YYSYMBOL_constant = 181                  /* constant  */
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
#define YYLAST   2103

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  79
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  103
/* YYNRULES -- Number of rules.  */
#define YYNRULES  274
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  537

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   310


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
       2,     2,     2,    66,     2,     2,    75,    64,    59,     2,
      69,    70,    63,    61,    73,    62,    76,    65,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    74,    68,
      60,     2,     2,    56,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    77,     2,    78,    58,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    71,    57,    72,    67,     2,     2,     2,
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
      55
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   247,   247,   248,   249,   250,   251,   260,   265,   266,
     271,   272,   283,   284,   285,   286,   287,   310,   312,   309,
     319,   320,   321,   331,   332,   333,   334,   335,   336,   337,
     338,   339,   340,   341,   342,   348,   349,   354,   359,   360,
     365,   371,   377,   383,   389,   401,   407,   409,   411,   413,
     419,   423,   429,   433,   435,   454,   455,   459,   460,   461,
     463,   464,   465,   466,   468,   469,   470,   472,   473,   474,
     476,   478,   485,   486,   487,   488,   490,   491,   492,   493,
     494,   495,   497,   499,   500,   501,   502,   503,   504,   509,
     518,   519,   520,   521,   522,   523,   524,   525,   526,   529,
     533,   534,   539,   541,   543,   548,   550,   552,   554,   556,
     558,   560,   562,   564,   567,   569,   574,   581,   583,   592,
     594,   596,   600,   601,   611,   624,   626,   628,   630,   632,
     634,   636,   638,   640,   651,   657,   663,   672,   676,   680,
     686,   688,   699,   705,   706,   707,   713,   715,   717,   719,
     721,   728,   729,   739,   740,   745,   747,   753,   754,   755,
     760,   762,   773,   779,   785,   786,   786,   792,   793,   798,
     803,   808,   809,   821,   826,   827,   832,   833,   838,   839,
     844,   845,   850,   851,   852,   857,   858,   863,   868,   869,
     879,   880,   885,   886,   891,   892,   902,   903,   904,   905,
     910,   916,   926,   928,   929,   935,   937,   947,   949,   951,
     957,   958,   963,   965,   971,   973,   979,   985,   996,   997,
    1002,  1003,  1014,  1022,  1032,  1037,  1038,  1043,  1045,  1052,
    1053,  1054,  1059,  1061,  1068,  1069,  1070,  1075,  1076,  1081,
    1082,  1087,  1088,  1098,  1110,  1114,  1115,  1124,  1125,  1126,
    1127,  1134,  1135,  1141,  1142,  1143,  1147,  1151,  1157,  1158,
    1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173
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
  "L_DOT_OPTIONAL", "L_FUNCTION_OPEN", "L_SSCANF", "L_CATCH", "L_AWAIT",
  "L_ACATCH", "L_ARRAY", "L_REF", "L_PARSE_COMMAND", "L_TIME_EXPRESSION",
  "L_CLASS", "L_NEW", "L_PARAMETER", "L_TREE", "LOWER_THAN_ELSE", "'?'",
  "'|'", "'^'", "'&'", "'<'", "'+'", "'-'", "'*'", "'%'", "'/'", "'!'",
  "'~'", "';'", "'('", "')'", "'{'", "'}'", "','", "':'", "'$'", "'.'",
  "'['", "']'", "$accept", "loop_start", "foreach_start", "block_start",
  "special_context_start", "dollar_start", "all", "program",
  "opt_semicolon", "def", "function", "@1", "@2", "block_or_semi",
  "statement", "stmt_return", "stmt_cond", "optional_else_part",
  "stmt_while", "stmt_do", "stmt_for", "stmt_foreach", "stmt_switch",
  "block", "statements", "local_decl_statement_header",
  "local_declaration_statement", "local_decl_header", "local_declarations",
  "comma_expr", "expr", "lvalue", "primary_expr", "call_open",
  "function_call", "sscanf", "parse_command", "catch", "acatch",
  "acatch_context_start", "time_expression", "tree", "switch_header",
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

#define YYPACT_NINF (-419)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-237)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -419,    35,   105,  -419,    37,   -17,  -419,  -419,  -419,    -4,
     141,  -419,  -419,  -419,  -419,  -419,    48,  -419,    89,     5,
      71,  -419,    24,   300,  -419,  -419,  -419,   109,   124,   408,
    -419,  -419,  -419,  -419,     7,  -419,    -4,  -419,    24,   184,
     175,    81,   120,   130,  -419,  -419,  -419,    -4,  1673,   126,
      89,  -419,   160,  -419,    24,  -419,  -419,   139,   155,    89,
     222,  -419,  -419,  1673,   212,    28,    73,   225,   295,    89,
    1740,   234,  -419,  1673,  -419,  -419,   266,  -419,   271,  -419,
     275,  -419,  1673,  1673,  1673,   726,   304,  2013,   445,   149,
    -419,  -419,  -419,  -419,  -419,  -419,  -419,  1673,   239,   321,
     321,   334,   389,   295,  -419,  -419,   221,   184,  -419,  -419,
     163,   282,  -419,  -419,  -419,    -4,  -419,  1673,   390,    89,
    -419,  1308,   146,   801,  -419,   291,  -419,   303,   404,  2013,
     321,  1673,   330,  -419,   330,  1673,   330,   314,  1308,  1673,
    -419,  -419,  -419,  -419,  -419,   100,  1673,  1381,   574,   226,
      -4,  -419,  1673,  1673,  1673,  1673,  1673,  1673,  1673,  1673,
    1673,  1673,  1673,  1673,  1673,  1673,  1673,  1673,  -419,  1673,
      89,    14,   292,    89,  1168,  -419,    89,  1308,  1308,   163,
    1673,   354,  -419,  -419,  -419,  -419,  -419,   183,    -4,  -419,
     359,     1,  -419,  -419,  2013,  1673,  -419,  -419,  -419,  2038,
    -419,   311,   372,  -419,  -419,  1673,   387,  1673,   403,  1883,
    1673,  -419,  -419,  -419,  1913,  -419,   468,   413,   263,   876,
     280,   423,  1833,  -419,   425,   418,  -419,   438,  1673,   762,
     694,   618,   317,   471,   197,  1863,   228,  1139,   296,   197,
     259,   259,  -419,  -419,  -419,  2013,   321,  1673,  -419,  1673,
    -419,   469,  1673,    43,  -419,   443,   458,   460,   412,  -419,
    -419,   482,  -419,  -419,    34,   390,  -419,  1235,  -419,   432,
    -419,  2013,  -419,  1673,   281,  1673,  -419,  -419,  -419,  -419,
      22,   476,   480,   487,  -419,   490,   492,   470,   494,  1454,
    -419,   876,  -419,  -419,  -419,  -419,  -419,  -419,  -419,  -419,
     491,    -4,   876,   172,   499,  -419,   321,   501,  1673,   650,
     502,  -419,   293,  1673,  1308,    95,   203,  1673,    44,   422,
     321,  -419,  -419,   503,  1673,   511,   495,    10,  -419,  -419,
    -419,   512,  1933,  -419,  1963,   301,   315,   876,  1673,  1673,
    1673,  1022,  1095,   223,  -419,  -419,  -419,   209,  -419,  -419,
     482,   516,   517,  -419,  -419,  -419,  1308,  -419,  2013,  -419,
    -419,  -419,  2013,   519,  -419,  -419,   232,   498,  -419,  1673,
    -419,   236,  1308,  -419,  -419,   558,  -419,  -419,  -419,  -419,
    -419,  -419,   295,   531,  1673,  -419,    89,  -419,  -419,   316,
     342,   343,   573,   530,  -419,   537,    -4,   591,  -419,  -419,
    -419,   541,   582,  -419,  -419,   606,  -419,    -4,   168,   555,
    -419,  -419,  1673,  -419,   240,  1800,   248,  -419,   559,  1673,
     557,  -419,  1933,  -419,   568,  1022,  -419,  -419,   562,  1527,
     482,  1673,   223,  1673,  1673,  -419,   143,   570,    -4,   949,
    -419,  -419,   252,  1810,  -419,  -419,  -419,  -419,   453,  -419,
     575,  1673,   609,  1022,  1673,   578,  -419,  2013,  -419,  1983,
    2013,  -419,   151,   643,   644,   646,   151,     9,   596,  1058,
    -419,   590,   949,   588,   949,  -419,  -419,   595,  -419,  2013,
    1022,  -419,  -419,   348,  1600,  -419,   592,  -419,  -419,  -419,
     534,    20,  -419,   137,   137,   137,   137,   137,   137,   137,
     137,   137,   137,   137,   137,  -419,  -419,  -419,  -419,  -419,
    -419,   599,   598,  1022,  -419,  -419,  -419,   597,   137,   331,
     547,   204,  1066,  1204,  1271,   204,   374,   374,  -419,  -419,
    -419,  -419,  -419,  -419,  -419,  1022,  -419
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       9,     0,   174,     1,   174,    10,    12,    15,   164,   188,
     177,    14,    16,   175,    11,     8,   181,   189,     0,     0,
     218,   182,     0,     0,   201,   173,   176,     0,   178,     0,
     162,   165,   190,   191,   220,    13,   188,   251,     0,   253,
       0,   183,   184,     0,   179,   183,   184,   188,     0,     0,
       0,   219,     0,   252,     0,   200,   163,     0,   167,     0,
       0,   256,   257,     0,   222,    91,    92,     0,     0,     0,
       0,     0,     5,     0,   138,   241,     0,     5,     0,    93,
       0,   242,     0,     0,     0,     0,     0,   221,     0,    83,
      90,    84,    85,    96,    97,    86,    98,     0,     0,     0,
       0,     0,     0,     0,   247,   248,    94,   243,    87,    88,
     181,   220,   254,   255,   166,   188,   169,     0,     0,     0,
     124,     0,     0,     0,    77,    89,   197,    91,     0,    55,
     223,     0,     0,    81,     0,     0,     0,   124,     0,     0,
       4,   140,    80,    78,    79,   182,     0,     0,     0,     0,
     188,     6,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    82,     0,
       0,     0,     0,     0,     0,    76,     0,     0,     0,   181,
       0,     0,    57,   249,   250,   193,   192,   185,   188,   209,
       0,   203,   205,   168,    59,     0,   245,   244,   198,   225,
     227,   230,     0,   195,   194,     0,     0,     0,     0,     0,
       0,   239,   136,   137,     0,   139,     0,     0,     0,     0,
       0,     0,     0,   232,   235,     0,    95,     0,     0,    63,
      62,    61,    70,    67,    68,     0,    64,    65,    66,    69,
      71,    72,    73,    74,    75,    58,   100,     0,   102,     0,
     101,     0,     0,     0,   199,     0,     0,     0,     0,   117,
     186,   207,    18,   204,   181,     0,   226,     0,   129,     0,
     119,    56,   121,     0,     0,     0,   171,   171,   126,   141,
       0,     0,     0,     0,     2,     0,     0,     0,     0,     0,
      32,     0,    30,    24,    25,    26,    27,    28,    29,    31,
       0,   188,     0,     0,     0,    50,     0,     0,     0,     0,
       0,   187,     0,     0,     0,     0,     0,     0,     0,     0,
     105,   125,   130,     0,     0,     0,   210,     0,   206,   246,
     228,     0,   237,   240,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   181,    33,    34,    35,     0,    47,    45,
       0,     0,   212,    48,    23,    53,     0,   123,   224,   233,
     122,    99,    60,     0,   103,   104,     0,     0,   106,     0,
     111,     0,     0,   116,   118,     0,   208,    22,    21,    19,
      20,   120,     0,     0,     0,   127,     0,   128,    49,     0,
       0,     0,     0,   154,   155,     0,   188,     0,   156,   157,
     159,   160,     0,   158,    36,   214,    51,   188,   181,     0,
     132,   113,     0,   112,     0,    55,     0,   107,     0,     0,
     237,   134,   237,   172,     0,     0,   142,     2,     0,     0,
       0,     0,   181,     0,     0,   213,     0,     0,   188,     0,
      52,   133,     0,    55,   108,   115,   110,   131,     0,   238,
       0,     0,    38,     0,     0,     0,   216,   217,   161,     0,
     215,   271,     0,     0,     0,     0,     0,     0,   152,   151,
     150,     0,     0,     0,     0,   109,   114,     0,   135,   170,
       0,    37,    40,     0,     0,     3,     0,   272,   273,   274,
       0,     0,   146,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    54,   144,    44,   143,   211,
      39,     0,     0,     0,   149,   270,   148,     0,     0,   264,
     261,   262,   258,   259,   260,   263,   265,   266,   267,   268,
     269,    41,     2,    43,   147,     0,    42
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -419,  -418,  -419,  -419,   593,  -419,  -419,  -419,  -419,  -419,
    -419,  -419,  -419,  -419,  -272,  -419,  -419,  -419,  -419,  -419,
    -419,  -419,  -419,   -70,  -239,  -419,  -419,  -419,  -419,   -67,
     -29,   -63,   -62,   -50,  -419,  -419,  -419,  -419,  -419,  -419,
    -419,  -419,  -419,   -42,   261,  -378,  -409,  -419,   241,  -419,
    -419,  -419,  -419,  -419,   569,  -419,  -419,   395,  -419,   670,
    -419,    -8,  -419,  -419,  -329,  -419,    -5,   -11,  -246,   615,
    -419,  -419,  -419,   507,  -419,   424,  -419,  -342,  -419,   345,
    -419,   654,  -419,  -419,  -419,   382,  -229,    49,  -117,  -419,
    -419,    36,   307,   510,   600,   601,   436,  -419,   -22,   -21,
    -419,  -419,    21
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,   341,   513,   219,   132,   228,     1,     2,    15,     5,
       6,    49,   327,   379,   291,   292,   293,   481,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   438,   408,   303,
     129,    88,    89,   121,    90,    91,    92,    93,    94,   134,
      95,    96,   304,   473,   474,   467,   394,   395,   401,   402,
       7,     8,    16,    47,    57,    58,   423,   335,     9,    10,
      25,   187,    27,    28,   188,    97,   350,    98,   189,    99,
     100,    11,    12,   190,   191,   192,   376,   351,   352,   403,
     398,    19,    20,   101,   102,   223,   200,   201,   202,   224,
     225,   383,   212,   103,   104,   105,   197,   106,   107,   468,
     108,   109,   469
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      39,    40,    26,   128,    18,   124,   125,    34,    31,   453,
     141,   377,    43,   396,   396,   326,    39,    52,   149,    87,
     455,   217,    48,    37,   461,    32,    33,    37,   138,   263,
     221,    50,    39,   113,   118,     3,   491,   117,   330,   111,
     182,   125,    59,    21,   133,   185,   186,     4,   116,   177,
     178,    14,   348,   142,   143,   144,   149,    21,   126,    17,
     255,   256,   211,   353,   211,   435,   211,  -190,   175,   392,
     319,   367,   218,    35,   264,   512,   -17,   150,   378,   220,
     177,   140,   463,   492,   486,    29,   464,   465,   194,   466,
     337,   247,   199,    38,   516,   330,   471,   120,   388,    29,
      32,    33,   209,   396,   405,    -7,   214,   253,   198,   199,
      59,   204,  -191,   517,   535,     4,   207,   207,   199,   222,
      30,   320,   368,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   119,
     245,   461,  -196,   274,    36,   227,    37,   461,   199,   199,
      21,   199,  -190,   452,    37,   461,    44,    32,    33,   246,
     248,   312,   250,   -89,   -89,   254,   265,   472,   207,  -222,
     462,  -180,    21,   364,   185,   186,   199,    21,   271,    22,
     315,   482,   316,   261,   456,   318,   170,    53,  -181,   171,
     172,  -191,    23,   436,   437,   110,   314,   363,   203,   463,
     472,    56,   472,   464,   465,   463,   518,   114,   510,   464,
     465,   305,   466,   463,    29,    24,   155,   464,   465,    29,
     466,    54,   347,   493,    37,   173,   174,    63,   115,   258,
     112,    75,    21,  -202,   399,   400,    54,   117,   199,   409,
     354,   533,    81,    55,   332,   207,   334,   155,   156,   157,
     366,   119,   371,   373,   269,   418,   356,   380,   163,   164,
     165,   166,   167,   536,   122,   500,   501,   502,   503,   504,
     372,   389,   390,   391,    29,   393,   207,   404,   176,   358,
     222,   365,   207,   305,   362,   199,   160,   161,   162,   163,
     164,   165,   166,   167,   305,   199,   226,    48,    37,   207,
     414,    63,   416,   131,    64,   207,    65,    66,    67,   207,
     411,    41,    42,   207,   417,   155,   156,   157,   444,   420,
     125,   207,   165,   166,   167,   207,   446,   199,   170,   305,
     475,   171,   172,   279,    69,   135,   207,    70,   415,    72,
     137,    74,  -190,   199,   139,   442,   140,    78,    79,    80,
     306,   333,   448,   207,   207,   422,   162,   163,   164,   165,
     166,   167,   393,   361,   123,   216,   207,   173,   174,   249,
      86,   385,   120,   151,   386,   424,   205,   206,   163,   164,
     165,   166,   167,   443,   267,   387,   425,   483,   386,   207,
     120,   430,   500,   501,   502,   503,   504,   195,   196,   210,
     440,   140,   457,   179,   459,   460,   152,   153,   154,   155,
     156,   157,   426,   427,    39,   207,   207,   393,   511,    45,
      46,   207,   479,    60,   259,    37,    61,    62,    63,   262,
     506,    64,   508,    65,    66,    67,    68,   502,   503,   504,
      39,   213,   268,   215,    39,    52,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   449,   270,   450,   168,
     169,    69,   180,   181,    70,    71,    72,    73,    74,    39,
      75,    76,    77,   272,    78,    79,    80,   207,   208,   276,
     277,    81,   369,   278,    82,   324,   325,   490,    83,    84,
     155,    85,   157,   185,   186,   307,   310,    86,   309,    60,
     370,    37,    61,    62,    63,   324,   331,    64,   311,    65,
      66,    67,    68,   321,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   530,   207,   477,   322,   317,
     323,   162,   163,   164,   165,   166,   167,    69,   344,   490,
      70,    71,    72,    73,    74,   338,    75,    76,    77,   339,
      78,    79,    80,   493,   494,   495,   340,    81,   412,   342,
      82,   343,   345,   349,    83,    84,   493,    85,   495,   375,
     355,   357,   360,    86,   140,    60,   413,    37,    61,    62,
      63,   374,   381,    64,   406,    65,    66,    67,    68,   410,
     407,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     419,   421,   428,   207,   515,   429,   431,   499,   500,   501,
     502,   503,   504,    69,   432,   433,    70,    71,    72,    73,
      74,   434,    75,    76,    77,   441,    78,    79,    80,   447,
     382,   454,   480,    81,   152,   153,    82,   155,   156,   157,
      83,    84,   451,    85,   470,   478,   484,   487,   488,    86,
     489,    60,  -234,    37,    61,    62,    63,    54,   505,    64,
     507,    65,    66,    67,    68,   509,   514,   531,   532,   439,
     136,   534,   336,   458,    13,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   193,   130,   257,   397,   328,    69,
      51,   359,    70,    71,    72,    73,    74,   260,    75,    76,
      77,   329,    78,    79,    80,     0,   183,   184,     0,    81,
     152,     0,    82,   155,   156,   157,    83,    84,     0,    85,
       0,     0,     0,     0,     0,    86,     0,    60,  -236,    37,
      61,    62,    63,     0,     0,   145,     0,    65,    66,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   159,   160,   161,   162,   163,   164,   165,   166,   167,
       0,     0,     0,     0,     0,    69,     0,     0,    70,    71,
      72,    73,    74,  -181,    75,    76,    77,    29,    78,    79,
      80,   155,   156,   157,     0,    81,     0,     0,    82,   146,
       0,     0,    83,    84,     0,    85,     0,   147,     0,     0,
       0,    86,    60,   148,    37,    61,    62,    63,     0,     0,
      64,     0,    65,    66,    67,    68,     0,     0,     0,   159,
     160,   161,   162,   163,   164,   165,   166,   167,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      69,     0,     0,    70,    71,    72,    73,    74,     0,    75,
      76,    77,     0,    78,    79,    80,     0,     0,     0,     0,
      81,     0,     0,    82,   146,     0,     0,    83,    84,     0,
      85,     0,   147,     0,     0,     0,    86,   280,   148,    37,
      61,    62,    63,     0,     0,   145,     0,    65,    66,    67,
      68,     0,     0,     0,     0,     0,     0,     0,   281,     0,
     282,     0,     0,     0,     0,   283,   284,   285,   286,     0,
     287,   288,   289,     0,     0,    69,     0,     0,    70,    71,
      72,    73,    74,  -181,    75,    76,    77,    29,    78,    79,
      80,     0,     0,     0,     0,    81,     0,     0,    82,     0,
       0,     0,    83,    84,   290,    85,     0,   140,   -46,     0,
      60,    86,    37,    61,    62,    63,     0,     0,    64,     0,
      65,    66,    67,    68,     0,     0,     0,     0,     0,     0,
       0,   281,     0,   282,   436,   437,     0,     0,   283,   284,
     285,   286,     0,   287,   288,   289,     0,     0,    69,     0,
       0,    70,    71,    72,    73,    74,     0,    75,    76,    77,
       0,    78,    79,    80,     0,     0,     0,     0,    81,     0,
       0,    82,     0,     0,     0,    83,    84,   290,    85,     0,
     140,  -145,     0,    60,    86,    37,    61,    62,    63,     0,
       0,    64,     0,    65,    66,    67,    68,     0,     0,     0,
       0,     0,     0,     0,   281,     0,   282,     0,     0,     0,
       0,   283,   284,   285,   286,     0,   287,   288,   289,     0,
       0,    69,     0,     0,    70,    71,    72,    73,    74,     0,
      75,    76,    77,     0,    78,    79,    80,   493,   494,   495,
       0,    81,     0,     0,    82,   493,   494,   495,    83,    84,
     290,    85,     0,   140,     0,     0,    60,    86,    37,    61,
      62,    63,     0,     0,   145,     0,    65,    66,    67,    68,
       0,     0,     0,     0,     0,   496,   497,   498,   499,   500,
     501,   502,   503,   504,   497,   498,   499,   500,   501,   502,
     503,   504,     0,     0,    69,     0,     0,    70,    71,    72,
      73,    74,  -181,    75,    76,    77,    29,    78,    79,    80,
       0,     0,     0,     0,    81,     0,     0,    82,   155,   156,
     157,    83,    84,  -153,    85,     0,     0,     0,     0,    60,
      86,    37,    61,    62,    63,     0,     0,    64,     0,    65,
      66,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   251,     0,     0,   161,   162,
     163,   164,   165,   166,   167,     0,     0,    69,     0,     0,
      70,    71,    72,    73,    74,     0,    75,    76,    77,     0,
      78,    79,    80,   493,   494,   495,     0,    81,   252,     0,
      82,     0,     0,     0,    83,    84,    60,    85,    37,    61,
      62,    63,     0,    86,    64,     0,    65,    66,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   498,   499,   500,   501,   502,   503,   504,
       0,     0,     0,     0,    69,     0,     0,    70,    71,    72,
      73,    74,     0,    75,    76,    77,     0,    78,    79,    80,
     493,   494,   495,     0,    81,     0,     0,    82,     0,     0,
       0,    83,    84,     0,    85,  -231,     0,  -231,     0,    60,
      86,    37,    61,    62,    63,     0,     0,    64,     0,    65,
      66,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,   499,   500,   501,   502,   503,   504,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    69,     0,     0,
      70,    71,    72,    73,    74,     0,    75,    76,    77,     0,
      78,    79,    80,     0,     0,     0,     0,    81,     0,     0,
      82,     0,     0,     0,    83,    84,     0,    85,  -229,     0,
       0,     0,    60,    86,    37,    61,    62,    63,     0,     0,
      64,     0,    65,    66,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      69,     0,     0,    70,    71,    72,    73,    74,     0,    75,
      76,    77,     0,    78,    79,    80,     0,     0,     0,     0,
      81,     0,     0,    82,     0,     0,     0,    83,    84,     0,
      85,     0,     0,  -229,     0,    60,    86,    37,    61,    62,
      63,     0,     0,    64,     0,    65,    66,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    69,     0,     0,    70,    71,    72,    73,
      74,     0,    75,    76,    77,     0,    78,    79,    80,     0,
       0,     0,     0,    81,     0,     0,    82,     0,     0,     0,
      83,    84,   346,    85,     0,     0,     0,     0,    60,    86,
      37,    61,    62,    63,     0,     0,    64,     0,    65,    66,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    69,     0,     0,    70,
      71,    72,    73,    74,     0,    75,    76,    77,     0,    78,
      79,    80,     0,     0,     0,     0,    81,     0,     0,    82,
       0,     0,     0,    83,    84,  -153,    85,     0,     0,     0,
       0,    60,    86,    37,    61,    62,    63,     0,     0,    64,
       0,    65,    66,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    69,
       0,     0,    70,    71,    72,    73,    74,     0,    75,    76,
      77,     0,    78,    79,    80,     0,     0,     0,     0,    81,
       0,     0,    82,     0,     0,     0,    83,    84,     0,    85,
    -153,     0,     0,     0,    60,    86,    37,    61,    62,    63,
       0,     0,    64,     0,    65,    66,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,     0,     0,    70,    71,    72,    73,    74,
       0,    75,    76,    77,     0,    78,    79,    80,     0,     0,
       0,     0,    81,     0,     0,    82,     0,     0,     0,    83,
      84,    60,    85,    37,    61,    62,    63,     0,    86,    64,
       0,   127,    66,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    69,
       0,     0,    70,    71,    72,    73,    74,     0,    75,    76,
      77,     0,    78,    79,    80,     0,     0,     0,     0,    81,
       0,     0,    82,     0,     0,     0,    83,    84,     0,    85,
       0,     0,     0,     0,     0,    86,   152,   153,   154,   155,
     156,   157,     0,     0,     0,     0,   152,   153,   154,   155,
     156,   157,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   152,
     153,   154,   155,   156,   157,     0,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,     0,     0,   445,   152,
     153,   154,   155,   156,   157,     0,     0,     0,   476,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   152,
     153,   154,   155,   156,   157,     0,     0,   308,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   152,
     153,   154,   155,   156,   157,     0,     0,   313,     0,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   152,
     153,   154,   155,   156,   157,     0,   273,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   152,
     153,   154,   155,   156,   157,     0,   275,     0,     0,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   152,
     153,   154,   155,   156,   157,     0,   382,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   152,
     153,   154,   155,   156,   157,     0,   384,     0,     0,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,     0,
       0,     0,     0,   485,   152,   153,   154,   155,   156,   157,
       0,     0,     0,     0,     0,     0,   266,     0,     0,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167
};

static const yytype_int16 yycheck[] =
{
      22,    22,    10,    70,     9,    68,    68,    18,    16,   427,
      80,     1,    23,   342,   343,   261,    38,    38,    85,    48,
     429,   138,    15,     3,     4,    11,    12,     3,    78,    28,
     147,    36,    54,    54,    63,     0,    27,    15,   267,    50,
     103,   103,    47,     9,    73,    11,    12,    10,    59,    99,
     100,    68,   291,    82,    83,    84,   123,     9,    69,    63,
     177,   178,   132,   302,   134,   407,   136,    39,    97,   341,
      27,    27,   139,    68,    73,   484,    69,    85,    68,   146,
     130,    71,    62,    74,   462,    51,    66,    67,   117,    69,
      68,    77,   121,    69,    74,   324,   438,    69,   337,    51,
      11,    12,   131,   432,   350,     0,   135,   174,   119,   138,
     115,   122,    39,   491,   532,    10,    73,    73,   147,   148,
      72,    78,    78,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,    39,
     169,     4,    69,   210,    73,   150,     3,     4,   177,   178,
       9,   180,    71,   425,     3,     4,    47,    11,    12,   170,
     171,   228,   173,    14,    15,   176,   195,   439,    73,    69,
      27,    47,     9,    78,    11,    12,   205,     9,   207,    38,
     247,   453,   249,   188,   430,   252,    37,     3,    47,    40,
      41,    71,    51,    25,    26,    69,   246,   314,    52,    62,
     472,    71,   474,    66,    67,    62,    69,    68,   480,    66,
      67,   219,    69,    62,    51,    74,    19,    66,    67,    51,
      69,    61,   289,    19,     3,    76,    77,     6,    73,   180,
      70,    48,     9,    70,    11,    12,    61,    15,   267,   356,
      68,   513,    59,    68,   273,    73,   275,    19,    20,    21,
     317,    39,   319,   323,   205,   372,   306,   327,    61,    62,
      63,    64,    65,   535,    39,    61,    62,    63,    64,    65,
     320,   338,   339,   340,    51,   342,    73,    68,    39,   308,
     309,    78,    73,   291,   313,   314,    58,    59,    60,    61,
      62,    63,    64,    65,   302,   324,    70,    15,     3,    73,
     367,     6,   369,    69,     9,    73,    11,    12,    13,    73,
      78,    11,    12,    73,    78,    19,    20,    21,    78,   382,
     382,    73,    63,    64,    65,    73,    78,   356,    37,   337,
      78,    40,    41,    70,    39,    69,    73,    42,   367,    44,
      69,    46,    39,   372,    69,   412,    71,    52,    53,    54,
      70,    70,   419,    73,    73,   384,    60,    61,    62,    63,
      64,    65,   429,    70,    69,    51,    73,    76,    77,    77,
      75,    70,    69,    69,    73,   386,    73,    74,    61,    62,
      63,    64,    65,   412,    73,    70,    70,   454,    73,    73,
      69,   396,    61,    62,    63,    64,    65,     7,     8,    69,
     408,    71,   431,    69,   433,   434,    16,    17,    18,    19,
      20,    21,    70,    70,   436,    73,    73,   484,    70,    11,
      12,    73,   451,     1,    70,     3,     4,     5,     6,    70,
     472,     9,   474,    11,    12,    13,    14,    63,    64,    65,
     462,   134,    70,   136,   466,   466,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,   420,    70,   422,    14,
      15,    39,    73,    74,    42,    43,    44,    45,    46,   491,
      48,    49,    50,    70,    52,    53,    54,    73,    74,    11,
      12,    59,    60,    70,    62,    73,    74,   466,    66,    67,
      19,    69,    21,    11,    12,    72,    78,    75,    73,     1,
      78,     3,     4,     5,     6,    73,    74,     9,    70,    11,
      12,    13,    14,    70,   493,   494,   495,   496,   497,   498,
     499,   500,   501,   502,   503,   504,    73,    74,    70,    60,
      70,    60,    61,    62,    63,    64,    65,    39,    68,   518,
      42,    43,    44,    45,    46,    69,    48,    49,    50,    69,
      52,    53,    54,    19,    20,    21,    69,    59,    60,    69,
      62,    69,    68,    72,    66,    67,    19,    69,    21,    74,
      71,    70,    70,    75,    71,     1,    78,     3,     4,     5,
       6,    70,    70,     9,    68,    11,    12,    13,    14,    70,
      73,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      42,    70,    29,    73,    70,    68,    15,    60,    61,    62,
      63,    64,    65,    39,    73,    33,    42,    43,    44,    45,
      46,    15,    48,    49,    50,    70,    52,    53,    54,    70,
      73,    69,    23,    59,    16,    17,    62,    19,    20,    21,
      66,    67,    74,    69,    74,    70,    68,     4,     4,    75,
       4,     1,    78,     3,     4,     5,     6,    61,    68,     9,
      72,    11,    12,    13,    14,    70,    74,    68,    70,   408,
      77,    74,   277,   432,     4,    57,    58,    59,    60,    61,
      62,    63,    64,    65,   115,    70,   179,   342,   264,    39,
      36,   309,    42,    43,    44,    45,    46,   187,    48,    49,
      50,   265,    52,    53,    54,    -1,   106,   106,    -1,    59,
      16,    -1,    62,    19,    20,    21,    66,    67,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    75,    -1,     1,    78,     3,
       4,     5,     6,    -1,    -1,     9,    -1,    11,    12,    13,
      14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    -1,    -1,    39,    -1,    -1,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    19,    20,    21,    -1,    59,    -1,    -1,    62,    63,
      -1,    -1,    66,    67,    -1,    69,    -1,    71,    -1,    -1,
      -1,    75,     1,    77,     3,     4,     5,     6,    -1,    -1,
       9,    -1,    11,    12,    13,    14,    -1,    -1,    -1,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      39,    -1,    -1,    42,    43,    44,    45,    46,    -1,    48,
      49,    50,    -1,    52,    53,    54,    -1,    -1,    -1,    -1,
      59,    -1,    -1,    62,    63,    -1,    -1,    66,    67,    -1,
      69,    -1,    71,    -1,    -1,    -1,    75,     1,    77,     3,
       4,     5,     6,    -1,    -1,     9,    -1,    11,    12,    13,
      14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    -1,
      24,    -1,    -1,    -1,    -1,    29,    30,    31,    32,    -1,
      34,    35,    36,    -1,    -1,    39,    -1,    -1,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    -1,    -1,    -1,    59,    -1,    -1,    62,    -1,
      -1,    -1,    66,    67,    68,    69,    -1,    71,    72,    -1,
       1,    75,     3,     4,     5,     6,    -1,    -1,     9,    -1,
      11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    22,    -1,    24,    25,    26,    -1,    -1,    29,    30,
      31,    32,    -1,    34,    35,    36,    -1,    -1,    39,    -1,
      -1,    42,    43,    44,    45,    46,    -1,    48,    49,    50,
      -1,    52,    53,    54,    -1,    -1,    -1,    -1,    59,    -1,
      -1,    62,    -1,    -1,    -1,    66,    67,    68,    69,    -1,
      71,    72,    -1,     1,    75,     3,     4,     5,     6,    -1,
      -1,     9,    -1,    11,    12,    13,    14,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    22,    -1,    24,    -1,    -1,    -1,
      -1,    29,    30,    31,    32,    -1,    34,    35,    36,    -1,
      -1,    39,    -1,    -1,    42,    43,    44,    45,    46,    -1,
      48,    49,    50,    -1,    52,    53,    54,    19,    20,    21,
      -1,    59,    -1,    -1,    62,    19,    20,    21,    66,    67,
      68,    69,    -1,    71,    -1,    -1,     1,    75,     3,     4,
       5,     6,    -1,    -1,     9,    -1,    11,    12,    13,    14,
      -1,    -1,    -1,    -1,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    39,    -1,    -1,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    -1,    -1,    -1,    59,    -1,    -1,    62,    19,    20,
      21,    66,    67,    68,    69,    -1,    -1,    -1,    -1,     1,
      75,     3,     4,     5,     6,    -1,    -1,     9,    -1,    11,
      12,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    39,    -1,    -1,
      42,    43,    44,    45,    46,    -1,    48,    49,    50,    -1,
      52,    53,    54,    19,    20,    21,    -1,    59,    60,    -1,
      62,    -1,    -1,    -1,    66,    67,     1,    69,     3,     4,
       5,     6,    -1,    75,     9,    -1,    11,    12,    13,    14,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    -1,    39,    -1,    -1,    42,    43,    44,
      45,    46,    -1,    48,    49,    50,    -1,    52,    53,    54,
      19,    20,    21,    -1,    59,    -1,    -1,    62,    -1,    -1,
      -1,    66,    67,    -1,    69,    70,    -1,    72,    -1,     1,
      75,     3,     4,     5,     6,    -1,    -1,     9,    -1,    11,
      12,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,    -1,
      42,    43,    44,    45,    46,    -1,    48,    49,    50,    -1,
      52,    53,    54,    -1,    -1,    -1,    -1,    59,    -1,    -1,
      62,    -1,    -1,    -1,    66,    67,    -1,    69,    70,    -1,
      -1,    -1,     1,    75,     3,     4,     5,     6,    -1,    -1,
       9,    -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      39,    -1,    -1,    42,    43,    44,    45,    46,    -1,    48,
      49,    50,    -1,    52,    53,    54,    -1,    -1,    -1,    -1,
      59,    -1,    -1,    62,    -1,    -1,    -1,    66,    67,    -1,
      69,    -1,    -1,    72,    -1,     1,    75,     3,     4,     5,
       6,    -1,    -1,     9,    -1,    11,    12,    13,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    39,    -1,    -1,    42,    43,    44,    45,
      46,    -1,    48,    49,    50,    -1,    52,    53,    54,    -1,
      -1,    -1,    -1,    59,    -1,    -1,    62,    -1,    -1,    -1,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,     1,    75,
       3,     4,     5,     6,    -1,    -1,     9,    -1,    11,    12,
      13,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,    -1,    42,
      43,    44,    45,    46,    -1,    48,    49,    50,    -1,    52,
      53,    54,    -1,    -1,    -1,    -1,    59,    -1,    -1,    62,
      -1,    -1,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,     1,    75,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,
      -1,    -1,    42,    43,    44,    45,    46,    -1,    48,    49,
      50,    -1,    52,    53,    54,    -1,    -1,    -1,    -1,    59,
      -1,    -1,    62,    -1,    -1,    -1,    66,    67,    -1,    69,
      70,    -1,    -1,    -1,     1,    75,     3,     4,     5,     6,
      -1,    -1,     9,    -1,    11,    12,    13,    14,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    39,    -1,    -1,    42,    43,    44,    45,    46,
      -1,    48,    49,    50,    -1,    52,    53,    54,    -1,    -1,
      -1,    -1,    59,    -1,    -1,    62,    -1,    -1,    -1,    66,
      67,     1,    69,     3,     4,     5,     6,    -1,    75,     9,
      -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,
      -1,    -1,    42,    43,    44,    45,    46,    -1,    48,    49,
      50,    -1,    52,    53,    54,    -1,    -1,    -1,    -1,    59,
      -1,    -1,    62,    -1,    -1,    -1,    66,    67,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    75,    16,    17,    18,    19,
      20,    21,    -1,    -1,    -1,    -1,    16,    17,    18,    19,
      20,    21,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    16,
      17,    18,    19,    20,    21,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    78,    16,
      17,    18,    19,    20,    21,    -1,    -1,    -1,    78,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    16,
      17,    18,    19,    20,    21,    -1,    -1,    74,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    16,
      17,    18,    19,    20,    21,    -1,    -1,    74,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    16,
      17,    18,    19,    20,    21,    -1,    73,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    16,
      17,    18,    19,    20,    21,    -1,    73,    -1,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    16,
      17,    18,    19,    20,    21,    -1,    73,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    16,
      17,    18,    19,    20,    21,    -1,    73,    -1,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    -1,    70,    16,    17,    18,    19,    20,    21,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    85,    86,     0,    10,    88,    89,   129,   130,   137,
     138,   150,   151,   138,    68,    87,   131,    63,   145,   160,
     161,     9,    38,    51,    74,   139,   140,   141,   142,    51,
      72,   140,    11,    12,   146,    68,    73,     3,    69,   177,
     178,    11,    12,   146,    47,    11,    12,   132,    15,    90,
     145,   160,   178,     3,    61,    68,    71,   133,   134,   145,
       1,     4,     5,     6,     9,    11,    12,    13,    14,    39,
      42,    43,    44,    45,    46,    48,    49,    50,    52,    53,
      54,    59,    62,    66,    67,    69,    75,   109,   110,   111,
     113,   114,   115,   116,   117,   119,   120,   144,   146,   148,
     149,   162,   163,   172,   173,   174,   176,   177,   179,   180,
      69,   146,    70,   178,    68,    73,   146,    15,   109,    39,
      69,   112,    39,    69,   110,   111,   146,    11,   108,   109,
     148,    69,    83,   109,   118,    69,    83,    69,   112,    69,
      71,   102,   109,   109,   109,     9,    63,    71,    77,   108,
     140,    69,    16,    17,    18,    19,    20,    21,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    14,    15,
      37,    40,    41,    76,    77,   109,    39,   112,   112,    69,
      73,    74,   110,   173,   174,    11,    12,   140,   143,   147,
     152,   153,   154,   133,   109,     7,     8,   175,   146,   109,
     165,   166,   167,    52,   146,    73,    74,    73,    74,   109,
      69,   102,   171,   171,   109,   171,    51,   167,   108,    82,
     108,   167,   109,   164,   168,   169,    70,   145,    84,   109,
     109,   109,   109,   109,   109,   109,   109,   109,   109,   109,
     109,   109,   109,   109,   109,   109,   146,    77,   146,    77,
     146,    27,    60,   108,   146,   167,   167,   152,   166,    70,
     172,   145,    70,    28,    73,   109,    28,    73,    70,   166,
      70,   109,    70,    73,   108,    73,    11,    12,    70,    70,
       1,    22,    24,    29,    30,    31,    32,    34,    35,    36,
      68,    93,    94,    95,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   108,   121,   140,    70,    72,    74,    73,
      78,    70,   108,    74,   112,   108,   108,    60,   108,    27,
      78,    70,    70,    70,    73,    74,   147,    91,   154,   175,
     165,    74,   109,    70,   109,   136,   136,    68,    69,    69,
      69,    80,    69,    69,    68,    68,    68,   108,   103,    72,
     145,   156,   157,   103,    68,    71,   112,    70,   109,   164,
      70,    70,   109,   167,    78,    78,   108,    27,    78,    60,
      78,   108,   112,   102,    70,    74,   155,     1,    68,    92,
     102,    70,    73,   170,    73,    70,    73,    70,   103,   108,
     108,   108,    93,   108,   125,   126,   143,   158,   159,    11,
      12,   127,   128,   158,    68,   147,    68,    73,   107,   167,
      70,    78,    60,    78,   108,   109,   108,    78,   167,    42,
     110,    70,   109,   135,   146,    70,    70,    70,    29,    68,
     145,    15,    73,    33,    15,   156,    25,    26,   106,   123,
     140,    70,   108,   109,    78,    78,    78,    70,   108,   170,
     170,    74,    93,    80,    69,   125,   147,   109,   127,   109,
     109,     4,    27,    62,    66,    67,    69,   124,   178,   181,
      74,   156,    93,   122,   123,    78,    78,    74,    70,   109,
      23,    96,    93,   108,    68,    70,   124,     4,     4,     4,
     181,    27,    74,    19,    20,    21,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    68,   122,    72,   122,    70,
      93,    70,   125,    81,    74,    70,    74,   124,    69,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,    68,    70,    93,    74,    80,    93
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    79,    80,    81,    82,    83,    84,    85,    86,    86,
      87,    87,    88,    88,    88,    88,    88,    90,    91,    89,
      92,    92,    92,    93,    93,    93,    93,    93,    93,    93,
      93,    93,    93,    93,    93,    94,    94,    95,    96,    96,
      97,    98,    99,   100,   101,   102,   103,   103,   103,   103,
     104,   105,   106,   107,   107,   108,   108,   109,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   109,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   109,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   109,   109,   110,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
     111,   111,   111,   111,   112,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   114,   115,   116,   117,   118,   119,
     120,   120,   121,   122,   122,   122,   123,   123,   123,   123,
     123,   124,   124,   125,   125,   126,   126,   127,   127,   127,
     128,   128,   129,   130,   131,   132,   131,   133,   133,   134,
     135,   136,   136,   137,   138,   138,   139,   139,   140,   140,
     141,   141,   142,   142,   142,   143,   143,   144,   145,   145,
     146,   146,   147,   147,   148,   148,   149,   149,   149,   149,
     150,   151,   152,   152,   152,   153,   153,   154,   154,   154,
     155,   155,   156,   156,   157,   157,   158,   159,   160,   160,
     161,   161,   162,   163,   164,   165,   165,   166,   166,   167,
     167,   167,   168,   168,   169,   169,   169,   170,   170,   171,
     171,   172,   172,   173,   174,   175,   175,   176,   176,   176,
     176,   177,   177,   178,   178,   178,   179,   180,   181,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   181
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
       2,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     1,     1,     5,
       3,     3,     3,     5,     5,     4,     5,     6,     7,     8,
       7,     5,     6,     6,     8,     7,     5,     3,     5,     4,
       6,     4,     5,     5,     1,     4,     4,     6,     6,     4,
       4,     7,     6,     7,     7,     9,     3,     3,     0,     3,
       2,     4,     4,     2,     2,     0,     3,     5,     4,     4,
       2,     1,     1,     0,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     4,     0,     0,     5,     1,     3,     2,
       3,     0,     3,     2,     0,     2,     1,     0,     1,     2,
       1,     0,     1,     2,     2,     1,     2,     4,     0,     1,
       1,     1,     1,     1,     3,     3,     1,     2,     3,     3,
       4,     2,     0,     1,     2,     1,     3,     2,     4,     1,
       0,     5,     1,     3,     2,     4,     3,     3,     1,     3,
       2,     4,     1,     2,     3,     1,     2,     1,     3,     0,
       1,     2,     1,     3,     0,     1,     2,     0,     3,     1,
       3,     1,     1,     1,     3,     1,     3,     1,     1,     2,
       2,     1,     2,     1,     3,     3,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     2,     2,     2
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
#line 247 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { (yyval.number) = rule_loop_open(); }
#line 2667 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 3: /* foreach_start: %empty  */
#line 248 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                      { (yyval.number) = rule_foreach_open(); }
#line 2673 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 4: /* block_start: %empty  */
#line 249 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.number) = rule_block_open(); }
#line 2679 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 5: /* special_context_start: %empty  */
#line 250 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { (yyval.number) = rule_special_context_open(); }
#line 2685 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 6: /* dollar_start: %empty  */
#line 251 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                     { (yyval.contextp) = rule_dollar_open(); }
#line 2691 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 7: /* all: program  */
#line 260 "$REPO_ROOT$/src/compiler/internal/grammar.y"
           { rule_program((yyvsp[0].node)); }
#line 2697 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 8: /* program: program def opt_semicolon  */
#line 265 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_program_append(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 2703 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 9: /* program: %empty  */
#line 266 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.node) = 0; }
#line 2709 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 11: /* opt_semicolon: ';'  */
#line 272 "$REPO_ROOT$/src/compiler/internal/grammar.y"
         { rule_opt_semicolon(); }
#line 2715 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 13: /* def: type name_list ';'  */
#line 284 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_def_global_var((yyvsp[-2].number)); (yyval.node) = 0; }
#line 2721 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 17: /* @1: %empty  */
#line 310 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_func_type((yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].string)); }
#line 2727 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 18: /* @2: %empty  */
#line 312 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_func_proto((yyvsp[-3].number), (yyvsp[-5].number), (yyvsp[-4].string), &(yyvsp[-4].shared_string), (yyvsp[-1].argument)); }
#line 2733 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 19: /* function: type optional_star identifier @1 '(' argument ')' @2 block_or_semi  */
#line 314 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_func(&(yyval.node), (yyvsp[-5].number), (yyvsp[-7].number), (yyvsp[-6].shared_string), (yyvsp[-3].argument), &(yyvsp[-1].number), &(yyvsp[0].node)); }
#line 2739 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 20: /* block_or_semi: block  */
#line 319 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = rule_block_or_semi((yyvsp[0].decl).node); }
#line 2745 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 21: /* block_or_semi: ';'  */
#line 320 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2751 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 22: /* block_or_semi: error  */
#line 321 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2757 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 23: /* statement: comma_expr ';'  */
#line 331 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_expr((yyvsp[-1].node)); }
#line 2763 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 27: /* statement: stmt_for  */
#line 335 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2769 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 28: /* statement: stmt_foreach  */
#line 336 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2775 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 31: /* statement: block  */
#line 339 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2781 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 32: /* statement: ';'  */
#line 340 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = 0; }
#line 2787 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 33: /* statement: L_BREAK ';'  */
#line 341 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_break(); }
#line 2793 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 34: /* statement: L_CONTINUE ';'  */
#line 342 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_continue(); }
#line 2799 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 35: /* stmt_return: L_RETURN ';'  */
#line 348 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_return_void(&(yyval.node)); }
#line 2805 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 36: /* stmt_return: L_RETURN comma_expr ';'  */
#line 349 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_return_expr(&(yyval.node), (yyvsp[-1].node)); }
#line 2811 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 37: /* stmt_cond: L_IF '(' comma_expr ')' statement optional_else_part  */
#line 355 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_cond(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 2817 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 38: /* optional_else_part: %empty  */
#line 359 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                { (yyval.node) = 0; }
#line 2823 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 39: /* optional_else_part: L_ELSE statement  */
#line 360 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                 { (yyval.node) = (yyvsp[0].node); }
#line 2829 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 40: /* stmt_while: L_WHILE '(' comma_expr ')' loop_start statement  */
#line 366 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_while(&(yyval.node), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2835 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 41: /* stmt_do: L_DO loop_start statement L_WHILE '(' comma_expr ')' ';'  */
#line 372 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_do(&(yyval.node), (yyvsp[-5].node), (yyvsp[-2].node), (yyvsp[-6].number)); }
#line 2841 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 42: /* stmt_for: L_FOR '(' for_init ';' for_expr ';' for_expr ')' loop_start statement  */
#line 378 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for(&(yyval.decl), &(yyvsp[-7].decl), (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2847 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 43: /* stmt_foreach: L_FOREACH '(' foreach_vars L_IN expr ')' foreach_start statement  */
#line 384 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach(&(yyval.decl), &(yyvsp[-5].decl), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2853 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 44: /* stmt_switch: switch_header '{' local_declarations case switch_block '}'  */
#line 390 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_switch(&(yyval.node), (yyvsp[-5].switch_header).expr, &(yyvsp[-3].decl), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[-5].switch_header).saved_context, (yyvsp[-5].switch_header).saved_cases_size); }
#line 2859 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 45: /* block: '{' block_start statements '}'  */
#line 402 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block(&(yyval.decl), (yyvsp[-1].decl).node, (yyvsp[-2].number)); }
#line 2865 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 46: /* statements: %empty  */
#line 408 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_empty(&(yyval.decl)); }
#line 2871 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 47: /* statements: statement statements  */
#line 410 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_stmt(&(yyval.decl), (yyvsp[-1].node), &(yyvsp[0].decl)); }
#line 2877 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 48: /* statements: local_declaration_statement statements  */
#line 412 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_decl(&(yyval.decl), &(yyvsp[-1].decl), &(yyvsp[0].decl)); }
#line 2883 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 49: /* statements: error ';' statements  */
#line 414 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_error(&(yyval.decl), &(yyvsp[0].decl)); }
#line 2889 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 50: /* local_decl_statement_header: basic_type  */
#line 419 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { rule_local_declaration_statement_set_type((yyvsp[0].number)); (yyval.number) = (yyvsp[0].number); }
#line 2895 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 51: /* local_declaration_statement: local_decl_statement_header local_name_list ';'  */
#line 424 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_declaration_statement(&(yyval.decl), &(yyvsp[-1].decl)); }
#line 2901 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 52: /* local_decl_header: basic_type  */
#line 429 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { rule_local_declarations_set_type((yyvsp[0].number)); (yyval.number) = (yyvsp[0].number); }
#line 2907 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 53: /* local_declarations: %empty  */
#line 434 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.decl).node = 0; (yyval.decl).num = 0; }
#line 2913 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 54: /* local_declarations: local_declarations local_decl_header local_name_list ';'  */
#line 436 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_declarations(&(yyval.decl), &(yyvsp[-3].decl), &(yyvsp[-1].decl)); }
#line 2919 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 56: /* comma_expr: comma_expr ',' expr  */
#line 455 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                { rule_comma_expr(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2925 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 57: /* expr: ref lvalue  */
#line 459 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_ref(&(yyval.node), (yyvsp[0].node)); }
#line 2931 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 58: /* expr: lvalue L_ASSIGN expr  */
#line 460 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_assign(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 2937 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 59: /* expr: error L_ASSIGN expr  */
#line 461 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_assign_error(&(yyval.node), (yyvsp[0].node)); }
#line 2943 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 60: /* expr: expr '?' expr ':' expr  */
#line 463 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                            { rule_expr_ternary(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2949 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 61: /* expr: expr L_QUESTION_QUESTION expr  */
#line 464 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_nullish(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2955 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 62: /* expr: expr L_LOR expr  */
#line 465 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_lor(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2961 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 63: /* expr: expr L_LAND expr  */
#line 466 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_land(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2967 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 64: /* expr: expr '|' expr  */
#line 468 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_or(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2973 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 65: /* expr: expr '^' expr  */
#line 469 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_xor(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2979 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 66: /* expr: expr '&' expr  */
#line 470 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_and(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2985 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 67: /* expr: expr L_EQ_NE expr  */
#line 472 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_expr_eq_ne(&(yyval.node), (yyvsp[-1].number), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2991 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 68: /* expr: expr L_ORDER expr  */
#line 473 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_order(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 2997 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 69: /* expr: expr '<' expr  */
#line 474 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_lt(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3003 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 70: /* expr: expr L_SHIFT expr  */
#line 476 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_expr_shift(&(yyval.node), (yyvsp[-1].number), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3009 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 71: /* expr: expr '+' expr  */
#line 478 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             {
        rule_set_operand_ranges((yylsp[-2]).first_line, (yylsp[-2]).first_column, (yylsp[-2]).last_column,
                                (yylsp[-1]).first_line, (yylsp[-1]).first_column,
                                (yylsp[0]).first_line, (yylsp[0]).first_column, (yylsp[0]).last_column);
        rule_expr_add(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node));
        rule_clear_operand_ranges();
    }
#line 3021 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 72: /* expr: expr '-' expr  */
#line 485 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_sub(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3027 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 73: /* expr: expr '*' expr  */
#line 486 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_mul(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3033 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 74: /* expr: expr '%' expr  */
#line 487 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_mod(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3039 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 75: /* expr: expr '/' expr  */
#line 488 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_div(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3045 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 76: /* expr: cast expr  */
#line 490 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_cast(&(yyval.node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3051 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 77: /* expr: L_INC_DEC lvalue  */
#line 491 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_expr_pre_incdec(&(yyval.node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3057 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 78: /* expr: '!' expr  */
#line 492 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_expr_not(&(yyval.node), (yyvsp[0].node)); }
#line 3063 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 79: /* expr: '~' expr  */
#line 493 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_expr_compl(&(yyval.node), (yyvsp[0].node)); }
#line 3069 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 80: /* expr: '-' expr  */
#line 494 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_neg(&(yyval.node), (yyvsp[0].node)); }
#line 3075 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 81: /* expr: L_AWAIT expr  */
#line 495 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_await(&(yyval.node), (yyvsp[0].node)); }
#line 3081 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 82: /* expr: lvalue L_INC_DEC  */
#line 497 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_expr_post_incdec(&(yyval.node), (yyvsp[0].number), (yyvsp[-1].node)); }
#line 3087 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 89: /* lvalue: primary_expr  */
#line 509 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                { (yyval.node) = rule_lvalue((yyvsp[0].node)); }
#line 3093 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 91: /* primary_expr: L_DEFINED_NAME  */
#line 519 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_defined_name(&(yyval.node), (yyvsp[0].ihe)); }
#line 3099 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 92: /* primary_expr: L_IDENTIFIER  */
#line 520 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_identifier(&(yyval.node), (yyvsp[0].string)); }
#line 3105 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 93: /* primary_expr: L_PARAMETER  */
#line 521 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_parameter(&(yyval.node), (yyvsp[0].number)); }
#line 3111 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 95: /* primary_expr: '(' comma_expr ')'  */
#line 523 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { (yyval.node) = (yyvsp[-1].node); }
#line 3117 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 99: /* primary_expr: '$' '(' dollar_start comma_expr ')'  */
#line 530 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_dollar_expr(&(yyval.node), (yyvsp[-2].contextp), (yyvsp[-1].node)); }
#line 3123 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 100: /* primary_expr: primary_expr L_ARROW identifier  */
#line 533 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                 { rule_primary_expr_member_arrow(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 3129 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 101: /* primary_expr: primary_expr '.' identifier  */
#line 534 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                 { rule_primary_expr_member_dot(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 3135 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 102: /* primary_expr: primary_expr L_OPTIONAL_DOT identifier  */
#line 540 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_member_optional(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 3141 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 103: /* primary_expr: primary_expr L_OPTIONAL_DOT '[' comma_expr ']'  */
#line 542 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_optional(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3147 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 104: /* primary_expr: primary_expr L_DOT_OPTIONAL '[' comma_expr ']'  */
#line 544 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_optional(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3153 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 105: /* primary_expr: primary_expr '[' comma_expr ']'  */
#line 549 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3159 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 106: /* primary_expr: primary_expr '[' '<' comma_expr ']'  */
#line 551 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_r(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3165 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 107: /* primary_expr: primary_expr '[' comma_expr L_RANGE comma_expr ']'  */
#line 553 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nn(&(yyval.node), (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3171 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 108: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE comma_expr ']'  */
#line 555 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rn(&(yyval.node), (yyvsp[-6].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3177 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 109: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE '<' comma_expr ']'  */
#line 557 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rr(&(yyval.node), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3183 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 110: /* primary_expr: primary_expr '[' comma_expr L_RANGE '<' comma_expr ']'  */
#line 559 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nr(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3189 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 111: /* primary_expr: primary_expr '[' comma_expr L_RANGE ']'  */
#line 561 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_ne(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node)); }
#line 3195 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 112: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE ']'  */
#line 563 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_re(&(yyval.node), (yyvsp[-5].node), (yyvsp[-2].node)); }
#line 3201 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 113: /* primary_expr: primary_expr '[' L_RANGE '<' comma_expr ']'  */
#line 565 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nr(&(yyval.node), (yyvsp[-5].node), 0, (yyvsp[-1].node)); }
#line 3207 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 114: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE '<' expr ']'  */
#line 568 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rr(&(yyval.node), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3213 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 115: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE expr ']'  */
#line 570 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rn(&(yyval.node), (yyvsp[-6].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3219 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 116: /* primary_expr: lambda_return_type '(' argument ')' block  */
#line 575 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_anon_func(&(yyval.node), &(yyvsp[-4].func_block), &(yyvsp[-2].argument), &(yyvsp[0].decl)); }
#line 3225 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 117: /* primary_expr: functional_open ':' ')'  */
#line 582 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_1(&(yyval.node), (yyvsp[-2].number)); }
#line 3231 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 118: /* primary_expr: functional_open ',' arg_list ':' ')'  */
#line 584 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_2(&(yyval.node), (yyvsp[-4].number), (yyvsp[-2].node)); }
#line 3237 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 119: /* primary_expr: L_FUNCTION_OPEN L_DEFINED_NAME ':' ')'  */
#line 593 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_1(&(yyval.node), rule_functional_ref((yyvsp[-2].ihe))); }
#line 3243 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 120: /* primary_expr: L_FUNCTION_OPEN L_DEFINED_NAME ',' arg_list ':' ')'  */
#line 595 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_2(&(yyval.node), rule_functional_ref((yyvsp[-4].ihe)), (yyvsp[-2].node)); }
#line 3249 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 121: /* primary_expr: L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 597 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_3(&(yyval.node), (yyvsp[-2].node)); }
#line 3255 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 122: /* primary_expr: '(' '[' opt_pair_list ']' ')'  */
#line 600 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_primary_expr_mapping(&(yyval.node), (yyvsp[-2].node)); }
#line 3261 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 123: /* primary_expr: '(' '{' opt_arg_list '}' ')'  */
#line 601 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_primary_expr_array(&(yyval.node), (yyvsp[-2].node)); }
#line 3267 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 124: /* call_open: '('  */
#line 611 "$REPO_ROOT$/src/compiler/internal/grammar.y"
       { rule_call_open(&(yyval.call_open).saved_context, &(yyval.call_open).saved_refs); }
#line 3273 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 125: /* function_call: efun_override call_open opt_arg_list ')'  */
#line 625 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_efun(&(yyval.node), (yyvsp[-3].number), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3279 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 126: /* function_call: L_NEW call_open opt_arg_list ')'  */
#line 627 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new(&(yyval.node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3285 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 127: /* function_call: L_NEW '(' L_CLASS L_DEFINED_NAME opt_class_init ')'  */
#line 629 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new_class(&(yyval.node), (yyvsp[-2].ihe), (yyvsp[-1].node)); }
#line 3291 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 128: /* function_call: L_NEW '(' L_CLASS L_IDENTIFIER opt_class_init ')'  */
#line 631 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new_class_undef(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node)); }
#line 3297 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 129: /* function_call: L_DEFINED_NAME call_open opt_arg_list ')'  */
#line 633 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_defined_name(&(yyval.node), (yyvsp[-3].ihe), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3303 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 130: /* function_call: function_name call_open opt_arg_list ')'  */
#line 635 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_name(&(yyval.node), (yyvsp[-3].string), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3309 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 131: /* function_call: primary_expr '[' comma_expr ']' call_open opt_arg_list ')'  */
#line 637 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_indexed(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3315 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 132: /* function_call: primary_expr L_ARROW identifier call_open opt_arg_list ')'  */
#line 639 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_arrow(&(yyval.node), (yyvsp[-5].node), (yyvsp[-3].string), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3321 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 133: /* function_call: '(' '*' comma_expr ')' call_open opt_arg_list ')'  */
#line 641 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_star(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3327 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 134: /* sscanf: L_SSCANF '(' expr ',' expr lvalue_list ')'  */
#line 652 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_sscanf(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 3333 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 135: /* parse_command: L_PARSE_COMMAND '(' expr ',' expr ',' expr lvalue_list ')'  */
#line 658 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_parse_command(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 3339 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 136: /* catch: L_CATCH special_context_start expr_or_block  */
#line 664 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_catch(&(yyval.node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 3345 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 137: /* acatch: L_ACATCH acatch_context_start expr_or_block  */
#line 673 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_acatch(&(yyval.node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 3351 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 138: /* acatch_context_start: %empty  */
#line 676 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { (yyval.number) = rule_acatch_context_open(); }
#line 3357 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 139: /* time_expression: L_TIME_EXPRESSION special_context_start expr_or_block  */
#line 681 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_time_expression(&(yyval.node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 3363 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 140: /* tree: L_TREE block  */
#line 687 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_tree_block(&(yyval.node), (yyvsp[0].decl).node); }
#line 3369 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 141: /* tree: L_TREE '(' comma_expr ')'  */
#line 689 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_tree_expr(&(yyval.node), (yyvsp[-1].node)); }
#line 3375 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 142: /* switch_header: L_SWITCH '(' comma_expr ')'  */
#line 700 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.switch_header).expr = (yyvsp[-1].node); rule_switch_pre(&(yyval.switch_header).saved_context, &(yyval.switch_header).saved_cases_size); }
#line 3381 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 143: /* switch_block: case switch_block  */
#line 705 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_switch_block_case(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3387 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 144: /* switch_block: statement switch_block  */
#line 706 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_switch_block_stmt(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3393 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 145: /* switch_block: %empty  */
#line 707 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { rule_switch_block_empty(&(yyval.node)); }
#line 3399 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 146: /* case: L_CASE case_label ':'  */
#line 714 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_single(&(yyval.node), (yyvsp[-1].node)); }
#line 3405 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 147: /* case: L_CASE case_label L_RANGE case_label ':'  */
#line 716 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3411 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 148: /* case: L_CASE case_label L_RANGE ':'  */
#line 718 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range_from(&(yyval.node), (yyvsp[-2].node)); }
#line 3417 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 149: /* case: L_CASE L_RANGE case_label ':'  */
#line 720 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range_to(&(yyval.node), (yyvsp[-1].node)); }
#line 3423 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 150: /* case: L_DEFAULT ':'  */
#line 722 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_default(&(yyval.node)); }
#line 3429 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 151: /* case_label: constant  */
#line 728 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_case_label_constant(&(yyval.node), (yyvsp[0].number)); }
#line 3435 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 152: /* case_label: string_const  */
#line 729 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_case_label_string(&(yyval.node), (yyvsp[0].string)); }
#line 3441 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 153: /* for_expr: %empty  */
#line 739 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { (yyval.node) = 0; }
#line 3447 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 155: /* for_init: for_expr  */
#line 746 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for_init_expr(&(yyval.decl), (yyvsp[0].node)); }
#line 3453 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 156: /* for_init: single_new_local_def_with_init  */
#line 748 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for_init_local(&(yyval.decl), (yyvsp[0].node)); }
#line 3459 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 157: /* foreach_var: L_DEFINED_NAME  */
#line 753 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_defined(&(yyval.decl), (yyvsp[0].ihe)); }
#line 3465 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 158: /* foreach_var: single_new_local_def  */
#line 754 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_new_local(&(yyval.decl), (yyvsp[0].number)); }
#line 3471 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 159: /* foreach_var: L_IDENTIFIER  */
#line 755 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_identifier(&(yyval.decl), (yyvsp[0].string)); }
#line 3477 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 160: /* foreach_vars: foreach_var  */
#line 761 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach_vars_single(&(yyval.decl), &(yyvsp[0].decl)); }
#line 3483 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 161: /* foreach_vars: foreach_var ',' foreach_var  */
#line 763 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach_vars_double(&(yyval.decl), &(yyvsp[-2].decl), &(yyvsp[0].decl)); }
#line 3489 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 162: /* type_decl: class_header member_list '}'  */
#line 774 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_define_class_members((yyvsp[-2].class_header).ihe, (yyvsp[-2].class_header).classname_index); (yyval.node) = 0; }
#line 3495 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 163: /* class_header: type_modifier_list L_CLASS identifier '{'  */
#line 780 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.class_header).ihe = rule_define_class(&(yyval.class_header).classname_index, (yyvsp[-1].string)); }
#line 3501 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 165: /* $@3: %empty  */
#line 786 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                           { rule_member_list_set_type((yyvsp[0].number)); }
#line 3507 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 169: /* member_name: optional_star identifier  */
#line 798 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { rule_member_name((yyvsp[-1].number), (yyvsp[0].string)); }
#line 3513 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 170: /* class_init: identifier ':' expr  */
#line 803 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                       { (yyval.node) = rule_class_init((yyvsp[-2].string), (yyvsp[0].node)); }
#line 3519 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 171: /* opt_class_init: %empty  */
#line 808 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.node) = rule_opt_class_init_empty(); }
#line 3525 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 172: /* opt_class_init: opt_class_init ',' class_init  */
#line 809 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { (yyval.node) = rule_opt_class_init((yyvsp[-2].node), (yyvsp[0].node)); }
#line 3531 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 173: /* type: type_modifier_list opt_basic_type  */
#line 821 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { (yyval.number) = rule_type((yyvsp[-1].number), (yyvsp[0].number)); }
#line 3537 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 174: /* type_modifier_list: %empty  */
#line 826 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { (yyval.number) = 0; }
#line 3543 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 175: /* type_modifier_list: L_TYPE_MODIFIER type_modifier_list  */
#line 827 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                               { (yyval.number) = rule_type_modifier_list((yyvsp[-1].number), (yyvsp[0].number)); }
#line 3549 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 177: /* opt_basic_type: %empty  */
#line 833 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { (yyval.number) = rule_opt_basic_type_empty(); }
#line 3555 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 179: /* basic_type: opt_atomic_type L_ARRAY  */
#line 839 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { (yyval.number) = (yyvsp[-1].number) | TYPE_MOD_ARRAY; }
#line 3561 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 181: /* opt_atomic_type: %empty  */
#line 845 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { (yyval.number) = rule_opt_basic_type_empty(); }
#line 3567 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 183: /* atomic_type: L_CLASS L_DEFINED_NAME  */
#line 851 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.number) = rule_atomic_type_class((yyvsp[0].ihe)); }
#line 3573 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 184: /* atomic_type: L_CLASS L_IDENTIFIER  */
#line 852 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.number) = rule_atomic_type_class_identifier((yyvsp[0].string)); }
#line 3579 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 186: /* arg_type: basic_type ref  */
#line 858 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.number) = (yyvsp[-1].number) | LOCAL_MOD_REF; }
#line 3585 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 187: /* cast: '(' basic_type optional_star ')'  */
#line 863 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { (yyval.number) = rule_cast((yyvsp[-2].number), (yyvsp[-1].number)); }
#line 3591 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 188: /* optional_star: %empty  */
#line 868 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.number) = 0; }
#line 3597 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 189: /* optional_star: '*'  */
#line 869 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.number) = TYPE_MOD_ARRAY; }
#line 3603 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 190: /* identifier: L_DEFINED_NAME  */
#line 879 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { (yyval.string) = rule_identifier_defined_name((yyvsp[0].ihe)); }
#line 3609 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 193: /* new_local_name: L_DEFINED_NAME  */
#line 886 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.string) = rule_new_local_name_redefine((yyvsp[0].ihe)); }
#line 3615 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 194: /* efun_override: L_EFUN L_COLON_COLON identifier  */
#line 891 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.number) = rule_efun_override((yyvsp[0].string)); }
#line 3621 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 195: /* efun_override: L_EFUN L_COLON_COLON L_NEW  */
#line 892 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.number) = rule_efun_override_new(); }
#line 3627 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 197: /* function_name: L_COLON_COLON identifier  */
#line 903 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { (yyval.string) = rule_function_name_colon_colon((yyvsp[0].string)); }
#line 3633 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 198: /* function_name: L_BASIC_TYPE L_COLON_COLON identifier  */
#line 904 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { (yyval.string) = rule_function_name_type((yyvsp[-2].number), (yyvsp[0].string)); }
#line 3639 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 199: /* function_name: identifier L_COLON_COLON identifier  */
#line 905 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                    { (yyval.string) = rule_function_name_obj((yyvsp[-2].string), (yyvsp[0].string)); }
#line 3645 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 200: /* inheritance: type_modifier_list L_INHERIT string_const ';'  */
#line 911 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { if (rule_inheritence(&(yyval.node), (yyvsp[-3].number), (yyvsp[-1].string))) { YYACCEPT; } }
#line 3651 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 201: /* modifier_change: type_modifier_list ':'  */
#line 916 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { (yyval.node) = rule_modifier_change((yyvsp[-1].number)); }
#line 3657 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 202: /* argument: %empty  */
#line 927 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.argument).num_arg = 0; (yyval.argument).flags = 0; }
#line 3663 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 204: /* argument: argument_list L_DOT_DOT_DOT  */
#line 930 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_varargs(&(yyval.argument), &(yyvsp[-1].argument)); }
#line 3669 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 205: /* argument_list: param_decl  */
#line 936 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_list_single(&(yyval.argument), (yyvsp[0].number)); }
#line 3675 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 206: /* argument_list: argument_list ',' param_decl  */
#line 938 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_list_multi(&(yyval.argument), &(yyvsp[-2].argument), (yyvsp[0].number)); }
#line 3681 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 207: /* param_decl: arg_type optional_star  */
#line 948 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_typed((yyvsp[-1].number) | (yyvsp[0].number)); }
#line 3687 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 208: /* param_decl: arg_type optional_star new_local_name optional_default_arg_value  */
#line 950 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_typed_name((yyvsp[-3].number) | (yyvsp[-2].number), (yyvsp[-1].string), (yyvsp[0].node)); }
#line 3693 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 209: /* param_decl: new_local_name  */
#line 952 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_untyped_name((yyvsp[0].string)); }
#line 3699 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 210: /* optional_default_arg_value: %empty  */
#line 957 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                          { (yyval.node) = 0; }
#line 3705 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 211: /* optional_default_arg_value: ':' L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 958 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                            { (yyval.node) = rule_default_arg_value((yyvsp[-2].node)); }
#line 3711 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 212: /* local_name_list: new_local_def  */
#line 964 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_name_list_single(&(yyval.decl), (yyvsp[0].node)); }
#line 3717 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 213: /* local_name_list: new_local_def ',' local_name_list  */
#line 966 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_name_list_multi(&(yyval.decl), (yyvsp[-2].node), &(yyvsp[0].decl)); }
#line 3723 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 214: /* new_local_def: optional_star new_local_name  */
#line 972 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_new_local_def((yyvsp[0].string), (yyvsp[-1].number)); }
#line 3729 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 215: /* new_local_def: optional_star new_local_name L_ASSIGN expr  */
#line 974 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_new_local_def_with_init((yyvsp[-2].string), (yyvsp[-3].number), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3735 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 216: /* single_new_local_def: arg_type optional_star new_local_name  */
#line 980 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_single_new_local_def(&(yyval.number), (yyvsp[-2].number) | (yyvsp[-1].number), (yyvsp[0].string)); }
#line 3741 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 217: /* single_new_local_def_with_init: single_new_local_def L_ASSIGN expr  */
#line 986 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_single_new_local_def_with_init((yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3747 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 220: /* new_name: optional_star identifier  */
#line 1002 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { rule_new_name((yyvsp[-1].number), (yyvsp[0].string)); }
#line 3753 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 221: /* new_name: optional_star identifier L_ASSIGN expr  */
#line 1003 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { rule_new_name_with_init((yyvsp[-3].number), (yyvsp[-2].string), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3759 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 222: /* lambda_return_type: L_BASIC_TYPE  */
#line 1014 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                { rule_lambda_return_type(&(yyval.func_block), (yyvsp[0].number)); }
#line 3765 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 223: /* functional_open: L_FUNCTION_OPEN efun_override  */
#line 1022 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                 { (yyval.number) = rule_functional_open((yyvsp[0].number)); }
#line 3771 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 224: /* assoc_pair: expr ':' expr  */
#line 1032 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_assoc_pair(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3777 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 225: /* spread_expr: expr  */
#line 1037 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_spread_expr_normal(&(yyval.node), (yyvsp[0].node)); }
#line 3783 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 226: /* spread_expr: expr L_DOT_DOT_DOT  */
#line 1038 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_spread_expr_dots(&(yyval.node), (yyvsp[-1].node)); }
#line 3789 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 227: /* arg_list: spread_expr  */
#line 1044 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_arg_list_single(&(yyval.node), (yyvsp[0].node)); }
#line 3795 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 228: /* arg_list: arg_list ',' spread_expr  */
#line 1046 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_arg_list_multi(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3801 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 229: /* opt_arg_list: %empty  */
#line 1052 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list_empty(&(yyval.node)); }
#line 3807 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 230: /* opt_arg_list: arg_list  */
#line 1053 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list(&(yyval.node), (yyvsp[0].node)); }
#line 3813 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 231: /* opt_arg_list: arg_list ','  */
#line 1054 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list(&(yyval.node), (yyvsp[-1].node)); }
#line 3819 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 232: /* pair_list: assoc_pair  */
#line 1060 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_pair_list_single(&(yyval.node), (yyvsp[0].node)); }
#line 3825 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 233: /* pair_list: pair_list ',' assoc_pair  */
#line 1062 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_pair_list_multi(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3831 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 234: /* opt_pair_list: %empty  */
#line 1068 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list_empty(&(yyval.node)); }
#line 3837 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 235: /* opt_pair_list: pair_list  */
#line 1069 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list(&(yyval.node), (yyvsp[0].node)); }
#line 3843 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 236: /* opt_pair_list: pair_list ','  */
#line 1070 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list(&(yyval.node), (yyvsp[-1].node)); }
#line 3849 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 237: /* lvalue_list: %empty  */
#line 1075 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.node) = rule_lvalue_list_empty(); }
#line 3855 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 238: /* lvalue_list: ',' lvalue lvalue_list  */
#line 1076 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { (yyval.node) = rule_lvalue_list((yyvsp[-1].node), (yyvsp[0].node)); }
#line 3861 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 239: /* expr_or_block: block  */
#line 1081 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_expr_or_block_block((yyvsp[0].decl)); }
#line 3867 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 240: /* expr_or_block: '(' comma_expr ')'  */
#line 1082 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_expr_or_block_expr((yyvsp[-1].node)); }
#line 3873 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 243: /* string: string_literal  */
#line 1098 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_string(&(yyval.node), (yyvsp[0].string)); }
#line 3879 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 244: /* template_literal: L_TEMPLATE_HEAD expr template_parts  */
#line 1110 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_template_literal(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3885 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 245: /* template_parts: L_TEMPLATE_TAIL  */
#line 1114 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                      { rule_template_parts_tail(&(yyval.node), (yyvsp[0].string)); }
#line 3891 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 246: /* template_parts: L_TEMPLATE_MIDDLE expr template_parts  */
#line 1115 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                       { rule_template_parts_middle(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3897 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 249: /* string_like: string_like string  */
#line 1126 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                               { rule_string_like_concat(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3903 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 250: /* string_like: string_like template_literal  */
#line 1127 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_string_like_concat(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3909 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 252: /* string_literal: string_literal L_STRING  */
#line 1135 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.string) = rule_string_literal_concat((yyvsp[-1].string), (yyvsp[0].string)); }
#line 3915 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 254: /* string_const: '(' string_const ')'  */
#line 1142 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                        { (yyval.string) = (yyvsp[-1].string); }
#line 3921 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 255: /* string_const: string_const '+' string_const  */
#line 1143 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { (yyval.string) = rule_string_literal_concat((yyvsp[-2].string), (yyvsp[0].string)); }
#line 3927 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 256: /* number: L_NUMBER  */
#line 1147 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { rule_number(&(yyval.node), (yyvsp[0].number)); }
#line 3933 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 257: /* real: L_REAL  */
#line 1151 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { rule_real(&(yyval.node), (yyvsp[0].real)); }
#line 3939 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 258: /* constant: constant '|' constant  */
#line 1157 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_or(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3945 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 259: /* constant: constant '^' constant  */
#line 1158 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_xor(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3951 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 260: /* constant: constant '&' constant  */
#line 1159 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_and(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3957 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 261: /* constant: constant L_EQ_NE constant  */
#line 1160 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                             { rule_constant_eq_ne(&(yyval.number), (yyvsp[-1].number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3963 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 262: /* constant: constant L_ORDER constant  */
#line 1161 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { rule_constant_order(&(yyval.number), (yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].number)); }
#line 3969 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 263: /* constant: constant '<' constant  */
#line 1162 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_lt(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3975 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 264: /* constant: constant L_SHIFT constant  */
#line 1163 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                             { rule_constant_shift(&(yyval.number), (yyvsp[-1].number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3981 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 265: /* constant: constant '+' constant  */
#line 1164 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_add(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3987 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 266: /* constant: constant '-' constant  */
#line 1165 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_sub(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3993 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 267: /* constant: constant '*' constant  */
#line 1166 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_mul(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3999 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 268: /* constant: constant '%' constant  */
#line 1167 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_mod(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 4005 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 269: /* constant: constant '/' constant  */
#line 1168 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_div(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 4011 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 270: /* constant: '(' constant ')'  */
#line 1169 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { (yyval.number) = (yyvsp[-1].number); }
#line 4017 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 271: /* constant: L_NUMBER  */
#line 1170 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { (yyval.number) = (yyvsp[0].number); }
#line 4023 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 272: /* constant: '-' L_NUMBER  */
#line 1171 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_neg(&(yyval.number), (yyvsp[0].number)); }
#line 4029 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 273: /* constant: '!' L_NUMBER  */
#line 1172 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_not(&(yyval.number), (yyvsp[0].number)); }
#line 4035 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 274: /* constant: '~' L_NUMBER  */
#line 1173 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_compl(&(yyval.number), (yyvsp[0].number)); }
#line 4041 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;


#line 4045 "$BUILD_ROOT$/src/grammar.autogen.cc"

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
#line 1176 "$REPO_ROOT$/src/compiler/internal/grammar.y"


// Public accessor for the parser's symbol-name table: maps a raw yylex
// token number (what lpcc --tokens prints) to its grammar spelling
// ("L_IDENTIFIER", "'{'"). Lives in the epilogue because yysymbol_name()
// and YYTRANSLATE are file-static in the generated parser; consumed by
// the lpcc --json staged outputs (stage_output.cc).
const char* lpc_token_name(int token) {
  return yysymbol_name(YY_CAST(yysymbol_kind_t, YYTRANSLATE(token)));
}
/* FluffOS generated-from grammar.y sha256=7f218a9d8f63bb51a434adc504bcfd5285a376dad0db4af38d603b79c05b6ef6 */
