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
  YYSYMBOL_L_PROMISE = 47,                 /* L_PROMISE  */
  YYSYMBOL_L_ARRAY = 48,                   /* L_ARRAY  */
  YYSYMBOL_L_REF = 49,                     /* L_REF  */
  YYSYMBOL_L_PARSE_COMMAND = 50,           /* L_PARSE_COMMAND  */
  YYSYMBOL_L_TIME_EXPRESSION = 51,         /* L_TIME_EXPRESSION  */
  YYSYMBOL_L_CLASS = 52,                   /* L_CLASS  */
  YYSYMBOL_L_NEW = 53,                     /* L_NEW  */
  YYSYMBOL_L_PARAMETER = 54,               /* L_PARAMETER  */
  YYSYMBOL_L_TREE = 55,                    /* L_TREE  */
  YYSYMBOL_LOWER_THAN_ELSE = 56,           /* LOWER_THAN_ELSE  */
  YYSYMBOL_57_ = 57,                       /* '?'  */
  YYSYMBOL_58_ = 58,                       /* '|'  */
  YYSYMBOL_59_ = 59,                       /* '^'  */
  YYSYMBOL_60_ = 60,                       /* '&'  */
  YYSYMBOL_61_ = 61,                       /* '<'  */
  YYSYMBOL_62_ = 62,                       /* '+'  */
  YYSYMBOL_63_ = 63,                       /* '-'  */
  YYSYMBOL_64_ = 64,                       /* '*'  */
  YYSYMBOL_65_ = 65,                       /* '%'  */
  YYSYMBOL_66_ = 66,                       /* '/'  */
  YYSYMBOL_67_ = 67,                       /* '!'  */
  YYSYMBOL_68_ = 68,                       /* '~'  */
  YYSYMBOL_69_ = 69,                       /* ';'  */
  YYSYMBOL_70_ = 70,                       /* '('  */
  YYSYMBOL_71_ = 71,                       /* ')'  */
  YYSYMBOL_72_ = 72,                       /* '{'  */
  YYSYMBOL_73_ = 73,                       /* '}'  */
  YYSYMBOL_74_ = 74,                       /* ','  */
  YYSYMBOL_75_ = 75,                       /* ':'  */
  YYSYMBOL_76_ = 76,                       /* '$'  */
  YYSYMBOL_77_ = 77,                       /* '.'  */
  YYSYMBOL_78_ = 78,                       /* '['  */
  YYSYMBOL_79_ = 79,                       /* ']'  */
  YYSYMBOL_YYACCEPT = 80,                  /* $accept  */
  YYSYMBOL_loop_start = 81,                /* loop_start  */
  YYSYMBOL_foreach_start = 82,             /* foreach_start  */
  YYSYMBOL_block_start = 83,               /* block_start  */
  YYSYMBOL_special_context_start = 84,     /* special_context_start  */
  YYSYMBOL_tree_context_start = 85,        /* tree_context_start  */
  YYSYMBOL_dollar_start = 86,              /* dollar_start  */
  YYSYMBOL_all = 87,                       /* all  */
  YYSYMBOL_program = 88,                   /* program  */
  YYSYMBOL_opt_semicolon = 89,             /* opt_semicolon  */
  YYSYMBOL_def = 90,                       /* def  */
  YYSYMBOL_function = 91,                  /* function  */
  YYSYMBOL_92_1 = 92,                      /* @1  */
  YYSYMBOL_93_2 = 93,                      /* @2  */
  YYSYMBOL_block_or_semi = 94,             /* block_or_semi  */
  YYSYMBOL_statement = 95,                 /* statement  */
  YYSYMBOL_stmt_return = 96,               /* stmt_return  */
  YYSYMBOL_stmt_cond = 97,                 /* stmt_cond  */
  YYSYMBOL_optional_else_part = 98,        /* optional_else_part  */
  YYSYMBOL_stmt_while = 99,                /* stmt_while  */
  YYSYMBOL_stmt_do = 100,                  /* stmt_do  */
  YYSYMBOL_stmt_for = 101,                 /* stmt_for  */
  YYSYMBOL_stmt_foreach = 102,             /* stmt_foreach  */
  YYSYMBOL_stmt_switch = 103,              /* stmt_switch  */
  YYSYMBOL_block = 104,                    /* block  */
  YYSYMBOL_statements = 105,               /* statements  */
  YYSYMBOL_local_decl_statement_header = 106, /* local_decl_statement_header  */
  YYSYMBOL_local_declaration_statement = 107, /* local_declaration_statement  */
  YYSYMBOL_local_decl_header = 108,        /* local_decl_header  */
  YYSYMBOL_local_declarations = 109,       /* local_declarations  */
  YYSYMBOL_comma_expr = 110,               /* comma_expr  */
  YYSYMBOL_expr = 111,                     /* expr  */
  YYSYMBOL_lvalue = 112,                   /* lvalue  */
  YYSYMBOL_primary_expr = 113,             /* primary_expr  */
  YYSYMBOL_call_open = 114,                /* call_open  */
  YYSYMBOL_function_call = 115,            /* function_call  */
  YYSYMBOL_sscanf = 116,                   /* sscanf  */
  YYSYMBOL_parse_command = 117,            /* parse_command  */
  YYSYMBOL_catch = 118,                    /* catch  */
  YYSYMBOL_acatch = 119,                   /* acatch  */
  YYSYMBOL_acatch_context_start = 120,     /* acatch_context_start  */
  YYSYMBOL_time_expression = 121,          /* time_expression  */
  YYSYMBOL_tree = 122,                     /* tree  */
  YYSYMBOL_switch_header = 123,            /* switch_header  */
  YYSYMBOL_switch_block = 124,             /* switch_block  */
  YYSYMBOL_case = 125,                     /* case  */
  YYSYMBOL_case_label = 126,               /* case_label  */
  YYSYMBOL_for_expr = 127,                 /* for_expr  */
  YYSYMBOL_for_init = 128,                 /* for_init  */
  YYSYMBOL_foreach_var = 129,              /* foreach_var  */
  YYSYMBOL_foreach_vars = 130,             /* foreach_vars  */
  YYSYMBOL_type_decl = 131,                /* type_decl  */
  YYSYMBOL_class_header = 132,             /* class_header  */
  YYSYMBOL_member_list = 133,              /* member_list  */
  YYSYMBOL_134_3 = 134,                    /* $@3  */
  YYSYMBOL_member_name_list = 135,         /* member_name_list  */
  YYSYMBOL_member_name = 136,              /* member_name  */
  YYSYMBOL_class_init = 137,               /* class_init  */
  YYSYMBOL_opt_class_init = 138,           /* opt_class_init  */
  YYSYMBOL_type = 139,                     /* type  */
  YYSYMBOL_type_modifier_list = 140,       /* type_modifier_list  */
  YYSYMBOL_opt_basic_type = 141,           /* opt_basic_type  */
  YYSYMBOL_basic_type = 142,               /* basic_type  */
  YYSYMBOL_opt_atomic_type = 143,          /* opt_atomic_type  */
  YYSYMBOL_atomic_type = 144,              /* atomic_type  */
  YYSYMBOL_arg_type = 145,                 /* arg_type  */
  YYSYMBOL_cast = 146,                     /* cast  */
  YYSYMBOL_optional_star = 147,            /* optional_star  */
  YYSYMBOL_identifier = 148,               /* identifier  */
  YYSYMBOL_new_local_name = 149,           /* new_local_name  */
  YYSYMBOL_efun_override = 150,            /* efun_override  */
  YYSYMBOL_function_name = 151,            /* function_name  */
  YYSYMBOL_inheritance = 152,              /* inheritance  */
  YYSYMBOL_modifier_change = 153,          /* modifier_change  */
  YYSYMBOL_argument = 154,                 /* argument  */
  YYSYMBOL_argument_list = 155,            /* argument_list  */
  YYSYMBOL_param_decl = 156,               /* param_decl  */
  YYSYMBOL_optional_default_arg_value = 157, /* optional_default_arg_value  */
  YYSYMBOL_local_name_list = 158,          /* local_name_list  */
  YYSYMBOL_new_local_def = 159,            /* new_local_def  */
  YYSYMBOL_single_new_local_def = 160,     /* single_new_local_def  */
  YYSYMBOL_single_new_local_def_with_init = 161, /* single_new_local_def_with_init  */
  YYSYMBOL_name_list = 162,                /* name_list  */
  YYSYMBOL_new_name = 163,                 /* new_name  */
  YYSYMBOL_lambda_return_type = 164,       /* lambda_return_type  */
  YYSYMBOL_functional_open = 165,          /* functional_open  */
  YYSYMBOL_assoc_pair = 166,               /* assoc_pair  */
  YYSYMBOL_spread_expr = 167,              /* spread_expr  */
  YYSYMBOL_arg_list = 168,                 /* arg_list  */
  YYSYMBOL_opt_arg_list = 169,             /* opt_arg_list  */
  YYSYMBOL_pair_list = 170,                /* pair_list  */
  YYSYMBOL_opt_pair_list = 171,            /* opt_pair_list  */
  YYSYMBOL_lvalue_list = 172,              /* lvalue_list  */
  YYSYMBOL_expr_or_block = 173,            /* expr_or_block  */
  YYSYMBOL_ref = 174,                      /* ref  */
  YYSYMBOL_string = 175,                   /* string  */
  YYSYMBOL_template_literal = 176,         /* template_literal  */
  YYSYMBOL_template_parts = 177,           /* template_parts  */
  YYSYMBOL_string_like = 178,              /* string_like  */
  YYSYMBOL_string_literal = 179,           /* string_literal  */
  YYSYMBOL_string_const = 180,             /* string_const  */
  YYSYMBOL_number = 181,                   /* number  */
  YYSYMBOL_real = 182,                     /* real  */
  YYSYMBOL_constant = 183                  /* constant  */
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
#define YYLAST   2168

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  80
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  104
/* YYNRULES -- Number of rules.  */
#define YYNRULES  277
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  543

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   311


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
       2,     2,     2,    67,     2,     2,    76,    65,    60,     2,
      70,    71,    64,    62,    74,    63,    77,    66,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    75,    69,
      61,     2,     2,    57,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    78,     2,    79,    59,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    72,    58,    73,    68,     2,     2,     2,
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
      55,    56
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   249,   249,   250,   251,   252,   253,   254,   263,   268,
     269,   274,   275,   286,   287,   288,   289,   290,   313,   315,
     312,   322,   323,   324,   334,   335,   336,   337,   338,   339,
     340,   341,   342,   343,   344,   345,   351,   352,   357,   362,
     363,   368,   374,   380,   386,   392,   404,   410,   412,   414,
     416,   422,   426,   432,   436,   438,   457,   458,   462,   463,
     464,   466,   467,   468,   469,   471,   472,   473,   475,   476,
     477,   479,   481,   488,   489,   490,   491,   493,   494,   495,
     496,   497,   498,   500,   502,   503,   504,   505,   506,   507,
     512,   521,   522,   523,   524,   525,   526,   527,   528,   529,
     532,   536,   537,   542,   544,   546,   551,   553,   555,   557,
     559,   561,   563,   565,   567,   570,   572,   577,   584,   586,
     595,   597,   599,   603,   604,   614,   627,   629,   631,   633,
     635,   637,   639,   641,   643,   654,   660,   666,   675,   679,
     683,   689,   691,   702,   708,   709,   710,   716,   718,   720,
     722,   724,   731,   732,   742,   743,   748,   750,   756,   757,
     758,   763,   765,   776,   782,   788,   789,   789,   795,   796,
     801,   806,   811,   812,   824,   829,   830,   835,   836,   841,
     842,   847,   848,   853,   854,   855,   859,   860,   866,   867,
     872,   877,   878,   888,   889,   894,   895,   900,   901,   911,
     912,   913,   914,   919,   925,   935,   937,   938,   944,   946,
     956,   958,   960,   966,   967,   972,   974,   980,   982,   988,
     994,  1005,  1006,  1011,  1012,  1023,  1031,  1041,  1046,  1047,
    1052,  1054,  1061,  1062,  1063,  1068,  1070,  1077,  1078,  1079,
    1084,  1085,  1090,  1091,  1096,  1097,  1107,  1119,  1123,  1124,
    1133,  1134,  1135,  1136,  1143,  1144,  1150,  1151,  1152,  1156,
    1160,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182
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
  "L_ACATCH", "L_PROMISE", "L_ARRAY", "L_REF", "L_PARSE_COMMAND",
  "L_TIME_EXPRESSION", "L_CLASS", "L_NEW", "L_PARAMETER", "L_TREE",
  "LOWER_THAN_ELSE", "'?'", "'|'", "'^'", "'&'", "'<'", "'+'", "'-'",
  "'*'", "'%'", "'/'", "'!'", "'~'", "';'", "'('", "')'", "'{'", "'}'",
  "','", "':'", "'$'", "'.'", "'['", "']'", "$accept", "loop_start",
  "foreach_start", "block_start", "special_context_start",
  "tree_context_start", "dollar_start", "all", "program", "opt_semicolon",
  "def", "function", "@1", "@2", "block_or_semi", "statement",
  "stmt_return", "stmt_cond", "optional_else_part", "stmt_while",
  "stmt_do", "stmt_for", "stmt_foreach", "stmt_switch", "block",
  "statements", "local_decl_statement_header",
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

#define YYPACT_NINF (-435)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-240)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -435,   116,    40,  -435,   114,    81,  -435,  -435,  -435,    70,
       7,  -435,  -435,  -435,  -435,  -435,    58,  -435,    30,    96,
     135,  -435,    19,   152,   304,  -435,  -435,  -435,   176,   208,
     326,  -435,  -435,  -435,  -435,    11,  -435,    70,  -435,    19,
     241,    78,   108,   180,   194,   214,  -435,  -435,  -435,    70,
    1723,   232,    30,  -435,   -15,  -435,    19,  -435,    70,  -435,
     237,   253,    30,   296,  -435,  -435,  1723,   320,    -9,    59,
     329,   330,    30,  1791,   300,  -435,  1723,  -435,  -435,   312,
    -435,   317,  -435,   327,  -435,  1723,  1723,  1723,   763,   335,
    2102,   410,   196,  -435,  -435,  -435,  -435,  -435,  -435,  -435,
    1723,   369,   345,   345,   351,   356,   330,  -435,  -435,   228,
     241,  -435,  -435,   230,   423,  -435,  -435,   407,  -435,    70,
    -435,  1723,  1885,    30,  -435,  1353,    61,   839,  -435,   449,
    -435,   183,   388,  2102,   345,  1723,   192,  -435,   192,  1723,
     192,   400,  1353,  1723,   383,  -435,  -435,  -435,    88,  1723,
    1427,   609,   204,    70,  -435,  1723,  1723,  1723,  1723,  1723,
    1723,  1723,  1723,  1723,  1723,  1723,  1723,  1723,  1723,  1723,
    1723,  -435,  1723,    30,    16,   379,    30,  1211,  -435,    30,
    1353,  1353,   230,  1723,   394,  -435,  -435,  -435,  -435,  -435,
      66,    70,  -435,   404,    32,  -435,  -435,  -435,  2102,  1723,
    -435,  -435,  -435,  2089,  -435,   409,   420,  -435,  -435,  1723,
     421,  1723,   424,  1967,  1723,  -435,  -435,  -435,  -435,  1987,
    -435,   459,   431,   209,  -435,   210,   430,  1916,  -435,   422,
     428,  -435,   443,  1723,   569,   654,   382,   502,   206,   347,
    1936,   229,   731,  1100,   347,   280,   280,  -435,  -435,  -435,
    2102,   345,  1723,  -435,  1723,  -435,   456,  1723,    77,  -435,
     448,   450,   453,   399,  -435,  -435,   467,  -435,  -435,   137,
    1885,  -435,  1279,  -435,   406,  -435,  2102,  -435,  1723,   225,
     915,  1723,  -435,  -435,  -435,  -435,   345,   457,  1723,   686,
     458,  -435,   239,  1723,  1353,    51,    64,  1723,    80,   455,
     345,  -435,  -435,   383,  1723,   461,   445,    24,  -435,  -435,
    -435,   468,  2018,  -435,    28,   460,   470,   472,  -435,   477,
     478,   480,   490,  1501,  -435,   915,  -435,  -435,  -435,  -435,
    -435,  -435,  -435,  -435,   487,    70,   915,    94,   489,  -435,
    2038,   252,   260,  1353,  -435,  2102,  -435,  -435,  -435,  2102,
     491,  -435,  -435,   100,   532,  -435,  1723,  -435,   103,  1353,
    -435,  -435,   527,  -435,  -435,  -435,  -435,  -435,  -435,   330,
     499,   915,  1723,  1723,  1723,  1063,  1137,   380,  -435,  -435,
    -435,   138,  -435,  -435,   467,   503,   505,  -435,  -435,  -435,
    1723,  -435,    30,  -435,   509,  -435,  -435,  1723,  -435,   164,
    1852,   172,  -435,   513,  1723,   517,  -435,  -435,   306,   307,
     322,   544,   520,  -435,   540,    70,   586,  -435,  -435,  -435,
     542,   584,  -435,  -435,   604,  -435,    70,   425,  2018,  -435,
     549,  -435,   181,  1862,  -435,  -435,  -435,  -435,   413,  -435,
    1063,  -435,  -435,   555,  1575,   467,  1723,   380,  1723,  1723,
    -435,    65,   551,    70,   989,  -435,   573,  1723,  -435,  -435,
     574,   623,  1063,  1723,   578,  -435,  2102,  -435,  2069,  2102,
    -435,    74,   645,   646,   652,    74,   -24,   595,   723,  -435,
     592,   989,   593,   989,  -435,  2102,  -435,  1063,  -435,  -435,
     333,  1649,  -435,   590,  -435,  -435,  -435,   492,    20,  -435,
      99,    99,    99,    99,    99,    99,    99,    99,    99,    99,
      99,    99,  -435,  -435,  -435,  -435,  -435,   598,   597,  1063,
    -435,  -435,  -435,   596,    99,   541,   299,   354,   577,   800,
    1108,   354,   371,   371,  -435,  -435,  -435,  -435,  -435,  -435,
    -435,  1063,  -435
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
      10,     0,   175,     1,   175,    11,    13,    16,   165,   191,
     178,    15,    17,   176,    12,     9,   182,   192,     0,     0,
     221,   183,     0,   186,     0,   204,   174,   177,     0,   179,
       0,   163,   166,   193,   194,   223,    14,   191,   254,     0,
     256,     0,   182,   184,   185,     0,   180,   184,   185,   191,
       0,     0,     0,   222,     0,   255,     0,   203,   191,   164,
       0,   168,     0,     0,   259,   260,     0,   225,    92,    93,
       0,     0,     0,     0,     0,     5,     0,   139,   244,     0,
       5,     0,    94,     6,   245,     0,     0,     0,     0,     0,
     224,     0,    84,    91,    85,    86,    97,    98,    87,    99,
       0,     0,     0,     0,     0,     0,     0,   250,   251,    95,
     246,    88,    89,   182,   223,   257,   258,     0,   167,   191,
     170,     0,     0,     0,   125,     0,     0,     0,    78,    90,
     200,    92,     0,    56,   226,     0,     0,    82,     0,     0,
       0,   125,     0,     0,     0,    81,    79,    80,   183,     0,
       0,     0,     0,   191,     7,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    83,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,   182,     0,     0,    58,   252,   253,   196,   195,
     188,   191,   212,     0,   206,   208,   187,   169,    60,     0,
     248,   247,   201,   228,   230,   233,     0,   198,   197,     0,
       0,     0,     0,     0,     0,     4,   242,   137,   138,     0,
     140,     0,     0,     0,   141,     0,     0,     0,   235,   238,
       0,    96,     0,     0,    64,    63,    62,    71,    68,    69,
       0,    65,    66,    67,    70,    72,    73,    74,    75,    76,
      59,   101,     0,   103,     0,   102,     0,     0,     0,   202,
       0,     0,     0,     0,   118,   189,   210,    19,   207,   182,
       0,   229,     0,   130,     0,   120,    57,   122,     0,     0,
       0,     0,   172,   172,   127,   142,     0,     0,     0,     0,
       0,   190,     0,     0,     0,     0,     0,     0,     0,     0,
     106,   126,   131,     0,     0,     0,   213,     0,   209,   249,
     231,     0,   240,   243,     0,     0,     0,     0,     2,     0,
       0,     0,     0,     0,    33,     0,    31,    25,    26,    27,
      28,    29,    30,    32,     0,   191,     0,     0,     0,    51,
       0,     0,     0,     0,   124,   227,   236,   123,   100,    61,
       0,   104,   105,     0,     0,   107,     0,   112,     0,     0,
     117,   119,     0,   211,    23,    22,    20,    21,   121,     0,
       0,     0,     0,     0,     0,     0,     0,   182,    34,    35,
      36,     0,    48,    46,     0,     0,   215,    49,    24,    54,
       0,   128,     0,   129,     0,   133,   114,     0,   113,     0,
      56,     0,   108,     0,     0,   240,   135,    50,     0,     0,
       0,     0,   155,   156,     0,   191,     0,   157,   158,   160,
     161,     0,   159,    37,   217,    52,   191,   182,   240,   173,
       0,   134,     0,    56,   109,   116,   111,   132,     0,   241,
       0,   143,     2,     0,     0,     0,     0,   182,     0,     0,
     216,     0,     0,   191,     0,    53,     0,     0,   110,   115,
       0,    39,     0,     0,     0,   219,   220,   162,     0,   218,
     274,     0,     0,     0,     0,     0,     0,   153,   152,   151,
       0,     0,     0,     0,   136,   171,   214,     0,    38,    41,
       0,     0,     3,     0,   275,   276,   277,     0,     0,   147,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    55,   145,    45,   144,    40,     0,     0,     0,
     150,   273,   149,     0,     0,   267,   264,   265,   261,   262,
     263,   266,   268,   269,   270,   271,   272,    42,     2,    44,
     148,     0,    43
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -435,  -427,  -435,  -435,   600,  -435,  -435,  -435,  -435,  -435,
    -435,  -435,  -435,  -435,  -435,  -222,  -435,  -435,  -435,  -435,
    -435,  -435,  -435,  -435,  -131,  -292,  -435,  -435,  -435,  -435,
     -69,    36,   -60,   -57,   -71,  -435,  -435,  -435,  -435,  -435,
    -435,  -435,  -435,  -435,  -183,   251,  -434,  -415,  -435,   234,
    -435,  -435,  -435,  -435,  -435,   563,  -435,  -435,   401,  -435,
     679,  -435,    -4,  -435,  -435,  -356,  -435,    -1,    47,  -264,
     613,  -435,  -435,  -435,   511,  -435,   427,  -435,  -387,  -435,
     318,  -435,   664,  -435,  -435,  -435,   414,  -219,   -70,   -80,
    -435,  -435,  -353,   248,   512,   599,   601,   434,  -435,   -22,
     -21,  -435,  -435,  -153
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,   375,   519,   280,   136,   144,   233,     1,     2,    15,
       5,     6,    51,   307,   366,   325,   326,   327,   488,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   453,   427,
     337,   133,    91,    92,   125,    93,    94,    95,    96,    97,
     138,    98,    99,   338,   482,   483,   476,   413,   414,   420,
     421,     7,     8,    16,    49,    60,    61,   429,   341,     9,
      10,    26,   190,    28,    29,   191,   100,   384,   101,   192,
     102,   103,    11,    12,   193,   194,   195,   363,   385,   386,
     422,   417,    19,    20,   104,   105,   228,   204,   205,   206,
     229,   230,   370,   217,   106,   107,   108,   201,   109,   110,
     477,   111,   112,   478
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      40,    41,   306,   498,   132,   216,    27,   216,    18,   216,
     142,   128,    32,   224,   129,   462,    21,    40,    54,   152,
     415,   415,    38,    38,   470,   364,    50,    33,    34,   464,
    -193,   180,   181,   382,    40,   116,    52,   493,    58,   450,
      -8,    33,    34,   121,   387,    22,   185,    56,    62,   129,
       4,   499,   439,   310,    23,  -182,   115,   117,   152,    24,
     268,   124,   222,   180,   523,    35,   480,    21,    38,   470,
     226,    45,    33,    34,   223,   456,   518,    38,   470,   407,
     225,   -18,    25,   472,   153,   310,    90,   473,   474,    39,
     475,   415,   471,   365,   252,   522,   215,   371,  -194,   114,
     260,   261,   122,   470,   299,    23,   269,   354,   258,   120,
      30,   541,   137,   263,   207,    78,     3,    21,    62,   130,
     424,   145,   146,   147,     4,   211,    84,   123,   472,  -199,
     351,    31,   473,   474,    17,   475,   178,   472,   211,   274,
      56,   473,   474,   352,   475,   279,    21,    57,   188,   189,
      14,   211,   232,   411,   211,    23,   300,   198,  -225,   355,
      30,   203,   472,   388,   292,    36,   473,   474,   211,   524,
     202,   213,   360,   208,   211,   219,   367,   211,   203,   396,
     294,   465,   402,   295,    23,   296,   203,   227,   298,    30,
     266,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   423,   250,    37,
     -90,   -90,   211,    42,   350,   343,   203,   203,   461,   203,
     251,   253,  -193,   255,    46,   158,   259,   160,   353,   359,
     358,    38,   481,   173,    66,   270,   174,   175,   211,    21,
     489,   188,   189,   434,    55,   203,   211,   276,   158,   159,
     160,   436,  -193,   124,   381,   211,  -181,   209,   210,   481,
     458,   481,   214,   394,   215,   516,  -194,   165,   166,   167,
     168,   169,   170,   176,   177,   231,   339,    23,   211,   403,
     285,   286,    30,   211,   211,   399,    59,   401,   163,   164,
     165,   166,   167,   168,   169,   170,   313,   539,   513,   211,
     515,  -205,   113,   408,   409,   410,   118,   412,   203,   405,
     348,   121,   129,   211,   312,    43,    44,   340,   500,   542,
     502,   339,   497,   391,   345,   227,   392,   119,   432,   349,
     203,   393,   339,    38,   392,   438,    66,    47,    48,    67,
     203,    68,    69,    70,   168,   169,   170,   525,   526,   527,
     528,   529,   530,   531,   532,   533,   534,   535,   536,   123,
     506,   507,   508,   509,   510,   511,   158,   339,   126,    72,
     135,   497,    73,   500,    75,   412,    77,   440,   441,   203,
     211,   211,   139,    81,    82,    83,   218,   141,   220,    21,
     400,   418,   419,   442,   490,   203,   211,   143,   155,   156,
     127,   158,   159,   160,   517,   154,    89,   211,   179,   166,
     167,   168,   169,   170,   445,   124,   507,   508,   509,   510,
     511,   182,   412,   455,   171,   172,   428,    23,   196,    40,
     183,   184,    30,   433,    21,   509,   510,   511,    50,   430,
     162,   163,   164,   165,   166,   167,   168,   169,   170,    40,
     451,   452,   221,    40,    54,   215,    63,   254,    38,    64,
      65,    66,   211,   212,    67,   264,    68,    69,    70,    71,
     282,   283,    23,   304,   305,   267,    40,    30,   188,   189,
     304,   311,   466,   272,   468,   469,   173,   211,   460,   174,
     175,   273,   275,   485,    72,   277,   289,    73,    74,    75,
      76,    77,   284,   287,    78,    79,    80,   290,    81,    82,
      83,   500,   501,   502,   291,    84,   356,   297,    85,   301,
     362,   302,    86,    87,   303,    88,   176,   177,   344,   347,
     372,    89,   361,    63,   357,    38,    64,    65,    66,   368,
     373,    67,   374,    68,    69,    70,    71,   376,   377,   378,
     503,   504,   505,   506,   507,   508,   509,   510,   511,   379,
     383,   389,   395,   521,   166,   167,   168,   169,   170,   404,
     406,    72,   425,   443,    73,    74,    75,    76,    77,   426,
     431,    78,    79,    80,   437,    81,    82,    83,   158,   159,
     160,   369,    84,   397,   211,    85,   500,   501,   502,    86,
      87,   446,    88,   507,   508,   509,   510,   511,    89,   444,
      63,   398,    38,    64,    65,    66,   447,   448,    67,   449,
      68,    69,    70,    71,   457,   463,   479,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   504,   505,   506,   507,
     508,   509,   510,   511,   484,   486,   487,   491,    72,   494,
     495,    73,    74,    75,    76,    77,   496,    56,    78,    79,
      80,   512,    81,    82,    83,   520,   514,   537,   538,    84,
     155,   540,    85,   158,   159,   160,    86,    87,   454,    88,
     140,   467,   197,    13,   342,    89,   134,    63,  -237,    38,
      64,    65,    66,   262,   416,    67,   308,    68,    69,    70,
      71,    53,   265,   346,   309,     0,     0,     0,   186,     0,
     187,     0,   162,   163,   164,   165,   166,   167,   168,   169,
     170,     0,     0,     0,     0,    72,     0,     0,    73,    74,
      75,    76,    77,     0,     0,    78,    79,    80,     0,    81,
      82,    83,   500,   501,   502,     0,    84,     0,     0,    85,
     158,   159,   160,    86,    87,     0,    88,     0,     0,     0,
       0,     0,    89,     0,    63,  -239,    38,    64,    65,    66,
       0,     0,   148,     0,    68,    69,    70,    71,     0,     0,
       0,   503,   504,   505,   506,   507,   508,   509,   510,   511,
       0,   164,   165,   166,   167,   168,   169,   170,     0,     0,
       0,     0,    72,     0,     0,    73,    74,    75,    76,    77,
      23,  -182,    78,    79,    80,    30,    81,    82,    83,   500,
     501,   502,     0,    84,     0,     0,    85,   149,     0,     0,
      86,    87,     0,    88,     0,   150,     0,     0,     0,    89,
      63,   151,    38,    64,    65,    66,     0,     0,    67,     0,
      68,    69,    70,    71,     0,     0,     0,     0,     0,     0,
     505,   506,   507,   508,   509,   510,   511,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    72,     0,
       0,    73,    74,    75,    76,    77,     0,     0,    78,    79,
      80,     0,    81,    82,    83,     0,     0,     0,     0,    84,
       0,     0,    85,   149,     0,     0,    86,    87,     0,    88,
       0,   150,     0,     0,     0,    89,   314,   151,    38,    64,
      65,    66,     0,     0,   148,     0,    68,    69,    70,    71,
       0,     0,     0,     0,     0,     0,     0,   315,     0,   316,
       0,     0,     0,     0,   317,   318,   319,   320,     0,   321,
     322,   323,     0,     0,    72,     0,     0,    73,    74,    75,
      76,    77,    23,  -182,    78,    79,    80,    30,    81,    82,
      83,     0,     0,     0,     0,    84,     0,     0,    85,     0,
       0,     0,    86,    87,   324,    88,     0,   215,   -47,     0,
      63,    89,    38,    64,    65,    66,     0,     0,    67,     0,
      68,    69,    70,    71,     0,     0,     0,     0,     0,     0,
       0,   315,     0,   316,   451,   452,     0,     0,   317,   318,
     319,   320,     0,   321,   322,   323,     0,     0,    72,     0,
       0,    73,    74,    75,    76,    77,     0,     0,    78,    79,
      80,     0,    81,    82,    83,     0,     0,     0,     0,    84,
       0,     0,    85,     0,     0,     0,    86,    87,   324,    88,
       0,   215,  -146,     0,    63,    89,    38,    64,    65,    66,
       0,     0,    67,     0,    68,    69,    70,    71,     0,     0,
       0,     0,     0,     0,     0,   315,     0,   316,     0,     0,
       0,     0,   317,   318,   319,   320,     0,   321,   322,   323,
       0,     0,    72,     0,     0,    73,    74,    75,    76,    77,
       0,     0,    78,    79,    80,     0,    81,    82,    83,   158,
     159,   160,     0,    84,     0,     0,    85,   500,   501,   502,
      86,    87,   324,    88,     0,   215,     0,     0,    63,    89,
      38,    64,    65,    66,     0,     0,   148,     0,    68,    69,
      70,    71,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   165,   166,   167,   168,   169,   170,     0,     0,   506,
     507,   508,   509,   510,   511,     0,    72,     0,     0,    73,
      74,    75,    76,    77,    23,  -182,    78,    79,    80,    30,
      81,    82,    83,     0,     0,     0,     0,    84,     0,     0,
      85,     0,     0,     0,    86,    87,  -154,    88,     0,     0,
       0,     0,    63,    89,    38,    64,    65,    66,     0,     0,
      67,     0,    68,    69,    70,    71,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   256,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      72,     0,     0,    73,    74,    75,    76,    77,     0,     0,
      78,    79,    80,     0,    81,    82,    83,     0,     0,     0,
       0,    84,   257,     0,    85,     0,     0,     0,    86,    87,
      63,    88,    38,    64,    65,    66,     0,    89,    67,     0,
      68,    69,    70,    71,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    72,     0,
       0,    73,    74,    75,    76,    77,     0,     0,    78,    79,
      80,     0,    81,    82,    83,     0,     0,     0,     0,    84,
       0,     0,    85,     0,     0,     0,    86,    87,     0,    88,
    -234,     0,  -234,     0,    63,    89,    38,    64,    65,    66,
       0,     0,    67,     0,    68,    69,    70,    71,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    72,     0,     0,    73,    74,    75,    76,    77,
       0,     0,    78,    79,    80,     0,    81,    82,    83,     0,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
      86,    87,     0,    88,  -232,     0,     0,     0,    63,    89,
      38,    64,    65,    66,     0,     0,    67,     0,    68,    69,
      70,    71,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    72,     0,     0,    73,
      74,    75,    76,    77,     0,     0,    78,    79,    80,     0,
      81,    82,    83,     0,     0,     0,     0,    84,     0,     0,
      85,     0,     0,     0,    86,    87,     0,    88,     0,     0,
    -232,     0,    63,    89,    38,    64,    65,    66,     0,     0,
      67,     0,    68,    69,    70,    71,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      72,     0,     0,    73,    74,    75,    76,    77,     0,     0,
      78,    79,    80,     0,    81,    82,    83,     0,     0,     0,
       0,    84,     0,     0,    85,     0,     0,     0,    86,    87,
     380,    88,     0,     0,     0,     0,    63,    89,    38,    64,
      65,    66,     0,     0,    67,     0,    68,    69,    70,    71,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    72,     0,     0,    73,    74,    75,
      76,    77,     0,     0,    78,    79,    80,     0,    81,    82,
      83,     0,     0,     0,     0,    84,     0,     0,    85,     0,
       0,     0,    86,    87,  -154,    88,     0,     0,     0,     0,
      63,    89,    38,    64,    65,    66,     0,     0,    67,     0,
      68,    69,    70,    71,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    72,     0,
       0,    73,    74,    75,    76,    77,     0,     0,    78,    79,
      80,     0,    81,    82,    83,     0,     0,     0,     0,    84,
       0,     0,    85,     0,     0,     0,    86,    87,     0,    88,
    -154,     0,     0,     0,    63,    89,    38,    64,    65,    66,
       0,     0,    67,     0,    68,    69,    70,    71,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    72,     0,     0,    73,    74,    75,    76,    77,
       0,     0,    78,    79,    80,     0,    81,    82,    83,     0,
       0,     0,     0,    84,     0,     0,    85,     0,     0,     0,
      86,    87,    63,    88,    38,    64,    65,    66,     0,    89,
      67,     0,   131,    69,    70,    71,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      72,     0,     0,    73,    74,    75,    76,    77,     0,     0,
      78,    79,    80,     0,    81,    82,    83,     0,     0,     0,
       0,    84,     0,     0,    85,     0,     0,     0,    86,    87,
       0,    88,     0,     0,     0,     0,     0,    89,   155,   156,
     157,   158,   159,   160,     0,     0,     0,     0,   155,   156,
     157,   158,   159,   160,     0,     0,     0,     0,     0,     0,
       0,     0,   199,   200,     0,     0,     0,     0,     0,     0,
       0,   155,   156,   157,   158,   159,   160,     0,     0,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,     0,
       0,   435,   155,   156,   157,   158,   159,   160,     0,     0,
       0,   459,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   155,   156,   157,   158,   159,   160,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   155,   156,   157,   158,   159,   160,     0,
       0,   288,     0,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   155,   156,   157,   158,   159,   160,     0,
       0,   293,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   155,   156,   157,   158,   159,   160,
       0,   278,     0,     0,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   155,   156,   157,   158,   159,   160,
       0,   281,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   155,   156,   157,   158,   159,
     160,     0,   369,     0,     0,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   155,   156,   157,   158,   159,
     160,     0,   390,     0,     0,     0,     0,   271,   155,   156,
     157,   158,   159,   160,     0,     0,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,     0,     0,     0,     0,
     492,     0,     0,     0,     0,     0,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,     0,     0,     0,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170
};

static const yytype_int16 yycheck[] =
{
      22,    22,   266,    27,    73,   136,    10,   138,     9,   140,
      81,    71,    16,   144,    71,   442,     9,    39,    39,    88,
     376,   377,     3,     3,     4,     1,    15,    11,    12,   444,
      39,   102,   103,   325,    56,    56,    37,   471,    42,   426,
       0,    11,    12,    15,   336,    38,   106,    62,    49,   106,
      10,    75,   405,   272,    47,    48,    71,    58,   127,    52,
      28,    70,   142,   134,   498,    18,   453,     9,     3,     4,
     150,    24,    11,    12,   143,   428,   491,     3,     4,   371,
     149,    70,    75,    63,    88,   304,    50,    67,    68,    70,
      70,   447,    27,    69,    78,    75,    72,    69,    39,    52,
     180,   181,    66,     4,    27,    47,    74,    27,   177,    62,
      52,   538,    76,   183,    53,    49,     0,     9,   119,    72,
     384,    85,    86,    87,    10,    74,    60,    39,    63,    70,
      79,    73,    67,    68,    64,    70,   100,    63,    74,   209,
      62,    67,    68,    79,    70,   214,     9,    69,    11,    12,
      69,    74,   153,   375,    74,    47,    79,   121,    70,    79,
      52,   125,    63,    69,   233,    69,    67,    68,    74,    70,
     123,   135,   303,   126,    74,   139,   307,    74,   142,    79,
     251,   445,    79,   252,    47,   254,   150,   151,   257,    52,
     191,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,    69,   172,    74,
      14,    15,    74,    61,   294,   286,   180,   181,   440,   183,
     173,   174,    39,   176,    48,    19,   179,    21,   297,   300,
     299,     3,   454,    37,     6,   199,    40,    41,    74,     9,
     462,    11,    12,    79,     3,   209,    74,   211,    19,    20,
      21,    79,    72,    70,   323,    74,    48,    74,    75,   481,
      79,   483,    70,   343,    72,   487,    72,    61,    62,    63,
      64,    65,    66,    77,    78,    71,   280,    47,    74,   359,
      71,    71,    52,    74,    74,   354,    72,   356,    59,    60,
      61,    62,    63,    64,    65,    66,    71,   519,   481,    74,
     483,    71,    70,   372,   373,   374,    69,   376,   272,   369,
      71,    15,   369,    74,   278,    11,    12,   281,    19,   541,
      21,   325,   475,    71,   288,   289,    74,    74,   397,   293,
     294,    71,   336,     3,    74,   404,     6,    11,    12,     9,
     304,    11,    12,    13,    64,    65,    66,   500,   501,   502,
     503,   504,   505,   506,   507,   508,   509,   510,   511,    39,
      61,    62,    63,    64,    65,    66,    19,   371,    39,    39,
      70,   524,    42,    19,    44,   444,    46,    71,    71,   343,
      74,    74,    70,    53,    54,    55,   138,    70,   140,     9,
     354,    11,    12,    71,   463,   359,    74,    70,    16,    17,
      70,    19,    20,    21,    71,    70,    76,    74,    39,    62,
      63,    64,    65,    66,   415,    70,    62,    63,    64,    65,
      66,    70,   491,   427,    14,    15,   390,    47,    21,   451,
      74,    75,    52,   397,     9,    64,    65,    66,    15,   392,
      58,    59,    60,    61,    62,    63,    64,    65,    66,   471,
      25,    26,    52,   475,   475,    72,     1,    78,     3,     4,
       5,     6,    74,    75,     9,    71,    11,    12,    13,    14,
      11,    12,    47,    74,    75,    71,   498,    52,    11,    12,
      74,    75,   446,    74,   448,   449,    37,    74,    75,    40,
      41,    71,    71,   457,    39,    71,    74,    42,    43,    44,
      45,    46,    71,    73,    49,    50,    51,    79,    53,    54,
      55,    19,    20,    21,    71,    60,    61,    61,    63,    71,
      75,    71,    67,    68,    71,    70,    77,    78,    71,    71,
      70,    76,    71,     1,    79,     3,     4,     5,     6,    71,
      70,     9,    70,    11,    12,    13,    14,    70,    70,    69,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    69,
      73,    72,    71,    71,    62,    63,    64,    65,    66,    42,
      71,    39,    69,    29,    42,    43,    44,    45,    46,    74,
      71,    49,    50,    51,    71,    53,    54,    55,    19,    20,
      21,    74,    60,    61,    74,    63,    19,    20,    21,    67,
      68,    15,    70,    62,    63,    64,    65,    66,    76,    69,
       1,    79,     3,     4,     5,     6,    74,    33,     9,    15,
      11,    12,    13,    14,    75,    70,    75,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    59,    60,    61,    62,
      63,    64,    65,    66,    71,    71,    23,    69,    39,     4,
       4,    42,    43,    44,    45,    46,     4,    62,    49,    50,
      51,    69,    53,    54,    55,    75,    73,    69,    71,    60,
      16,    75,    63,    19,    20,    21,    67,    68,   427,    70,
      80,   447,   119,     4,   283,    76,    73,     1,    79,     3,
       4,     5,     6,   182,   376,     9,   269,    11,    12,    13,
      14,    37,   190,   289,   270,    -1,    -1,    -1,   109,    -1,
     109,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    -1,    -1,    -1,    39,    -1,    -1,    42,    43,
      44,    45,    46,    -1,    -1,    49,    50,    51,    -1,    53,
      54,    55,    19,    20,    21,    -1,    60,    -1,    -1,    63,
      19,    20,    21,    67,    68,    -1,    70,    -1,    -1,    -1,
      -1,    -1,    76,    -1,     1,    79,     3,     4,     5,     6,
      -1,    -1,     9,    -1,    11,    12,    13,    14,    -1,    -1,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    60,    61,    62,    63,    64,    65,    66,    -1,    -1,
      -1,    -1,    39,    -1,    -1,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    19,
      20,    21,    -1,    60,    -1,    -1,    63,    64,    -1,    -1,
      67,    68,    -1,    70,    -1,    72,    -1,    -1,    -1,    76,
       1,    78,     3,     4,     5,     6,    -1,    -1,     9,    -1,
      11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,
      -1,    42,    43,    44,    45,    46,    -1,    -1,    49,    50,
      51,    -1,    53,    54,    55,    -1,    -1,    -1,    -1,    60,
      -1,    -1,    63,    64,    -1,    -1,    67,    68,    -1,    70,
      -1,    72,    -1,    -1,    -1,    76,     1,    78,     3,     4,
       5,     6,    -1,    -1,     9,    -1,    11,    12,    13,    14,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    -1,    24,
      -1,    -1,    -1,    -1,    29,    30,    31,    32,    -1,    34,
      35,    36,    -1,    -1,    39,    -1,    -1,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    60,    -1,    -1,    63,    -1,
      -1,    -1,    67,    68,    69,    70,    -1,    72,    73,    -1,
       1,    76,     3,     4,     5,     6,    -1,    -1,     9,    -1,
      11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    22,    -1,    24,    25,    26,    -1,    -1,    29,    30,
      31,    32,    -1,    34,    35,    36,    -1,    -1,    39,    -1,
      -1,    42,    43,    44,    45,    46,    -1,    -1,    49,    50,
      51,    -1,    53,    54,    55,    -1,    -1,    -1,    -1,    60,
      -1,    -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,
      -1,    72,    73,    -1,     1,    76,     3,     4,     5,     6,
      -1,    -1,     9,    -1,    11,    12,    13,    14,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    22,    -1,    24,    -1,    -1,
      -1,    -1,    29,    30,    31,    32,    -1,    34,    35,    36,
      -1,    -1,    39,    -1,    -1,    42,    43,    44,    45,    46,
      -1,    -1,    49,    50,    51,    -1,    53,    54,    55,    19,
      20,    21,    -1,    60,    -1,    -1,    63,    19,    20,    21,
      67,    68,    69,    70,    -1,    72,    -1,    -1,     1,    76,
       3,     4,     5,     6,    -1,    -1,     9,    -1,    11,    12,
      13,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    61,    62,    63,    64,    65,    66,    -1,    -1,    61,
      62,    63,    64,    65,    66,    -1,    39,    -1,    -1,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    60,    -1,    -1,
      63,    -1,    -1,    -1,    67,    68,    69,    70,    -1,    -1,
      -1,    -1,     1,    76,     3,     4,     5,     6,    -1,    -1,
       9,    -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      39,    -1,    -1,    42,    43,    44,    45,    46,    -1,    -1,
      49,    50,    51,    -1,    53,    54,    55,    -1,    -1,    -1,
      -1,    60,    61,    -1,    63,    -1,    -1,    -1,    67,    68,
       1,    70,     3,     4,     5,     6,    -1,    76,     9,    -1,
      11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,
      -1,    42,    43,    44,    45,    46,    -1,    -1,    49,    50,
      51,    -1,    53,    54,    55,    -1,    -1,    -1,    -1,    60,
      -1,    -1,    63,    -1,    -1,    -1,    67,    68,    -1,    70,
      71,    -1,    73,    -1,     1,    76,     3,     4,     5,     6,
      -1,    -1,     9,    -1,    11,    12,    13,    14,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    39,    -1,    -1,    42,    43,    44,    45,    46,
      -1,    -1,    49,    50,    51,    -1,    53,    54,    55,    -1,
      -1,    -1,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,
      67,    68,    -1,    70,    71,    -1,    -1,    -1,     1,    76,
       3,     4,     5,     6,    -1,    -1,     9,    -1,    11,    12,
      13,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,    -1,    42,
      43,    44,    45,    46,    -1,    -1,    49,    50,    51,    -1,
      53,    54,    55,    -1,    -1,    -1,    -1,    60,    -1,    -1,
      63,    -1,    -1,    -1,    67,    68,    -1,    70,    -1,    -1,
      73,    -1,     1,    76,     3,     4,     5,     6,    -1,    -1,
       9,    -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      39,    -1,    -1,    42,    43,    44,    45,    46,    -1,    -1,
      49,    50,    51,    -1,    53,    54,    55,    -1,    -1,    -1,
      -1,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,
      69,    70,    -1,    -1,    -1,    -1,     1,    76,     3,     4,
       5,     6,    -1,    -1,     9,    -1,    11,    12,    13,    14,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    39,    -1,    -1,    42,    43,    44,
      45,    46,    -1,    -1,    49,    50,    51,    -1,    53,    54,
      55,    -1,    -1,    -1,    -1,    60,    -1,    -1,    63,    -1,
      -1,    -1,    67,    68,    69,    70,    -1,    -1,    -1,    -1,
       1,    76,     3,     4,     5,     6,    -1,    -1,     9,    -1,
      11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,
      -1,    42,    43,    44,    45,    46,    -1,    -1,    49,    50,
      51,    -1,    53,    54,    55,    -1,    -1,    -1,    -1,    60,
      -1,    -1,    63,    -1,    -1,    -1,    67,    68,    -1,    70,
      71,    -1,    -1,    -1,     1,    76,     3,     4,     5,     6,
      -1,    -1,     9,    -1,    11,    12,    13,    14,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    39,    -1,    -1,    42,    43,    44,    45,    46,
      -1,    -1,    49,    50,    51,    -1,    53,    54,    55,    -1,
      -1,    -1,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,
      67,    68,     1,    70,     3,     4,     5,     6,    -1,    76,
       9,    -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      39,    -1,    -1,    42,    43,    44,    45,    46,    -1,    -1,
      49,    50,    51,    -1,    53,    54,    55,    -1,    -1,    -1,
      -1,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,
      -1,    70,    -1,    -1,    -1,    -1,    -1,    76,    16,    17,
      18,    19,    20,    21,    -1,    -1,    -1,    -1,    16,    17,
      18,    19,    20,    21,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     7,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    16,    17,    18,    19,    20,    21,    -1,    -1,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      -1,    79,    16,    17,    18,    19,    20,    21,    -1,    -1,
      -1,    79,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    16,    17,    18,    19,    20,    21,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    16,    17,    18,    19,    20,    21,    -1,
      -1,    75,    -1,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    16,    17,    18,    19,    20,    21,    -1,
      -1,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    16,    17,    18,    19,    20,    21,
      -1,    74,    -1,    -1,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    16,    17,    18,    19,    20,    21,
      -1,    74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    16,    17,    18,    19,    20,
      21,    -1,    74,    -1,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    16,    17,    18,    19,    20,
      21,    -1,    74,    -1,    -1,    -1,    -1,    28,    16,    17,
      18,    19,    20,    21,    -1,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    -1,    -1,
      71,    -1,    -1,    -1,    -1,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    -1,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    87,    88,     0,    10,    90,    91,   131,   132,   139,
     140,   152,   153,   140,    69,    89,   133,    64,   147,   162,
     163,     9,    38,    47,    52,    75,   141,   142,   143,   144,
      52,    73,   142,    11,    12,   148,    69,    74,     3,    70,
     179,   180,    61,    11,    12,   148,    48,    11,    12,   134,
      15,    92,   147,   162,   180,     3,    62,    69,   142,    72,
     135,   136,   147,     1,     4,     5,     6,     9,    11,    12,
      13,    14,    39,    42,    43,    44,    45,    46,    49,    50,
      51,    53,    54,    55,    60,    63,    67,    68,    70,    76,
     111,   112,   113,   115,   116,   117,   118,   119,   121,   122,
     146,   148,   150,   151,   164,   165,   174,   175,   176,   178,
     179,   181,   182,    70,   148,    71,   180,   147,    69,    74,
     148,    15,   111,    39,    70,   114,    39,    70,   112,   113,
     148,    11,   110,   111,   150,    70,    84,   111,   120,    70,
      84,    70,   114,    70,    85,   111,   111,   111,     9,    64,
      72,    78,   110,   142,    70,    16,    17,    18,    19,    20,
      21,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    14,    15,    37,    40,    41,    77,    78,   111,    39,
     114,   114,    70,    74,    75,   112,   175,   176,    11,    12,
     142,   145,   149,   154,   155,   156,    21,   135,   111,     7,
       8,   177,   148,   111,   167,   168,   169,    53,   148,    74,
      75,    74,    75,   111,    70,    72,   104,   173,   173,   111,
     173,    52,   169,   110,   104,   110,   169,   111,   166,   170,
     171,    71,   147,    86,   111,   111,   111,   111,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
     111,   148,    78,   148,    78,   148,    27,    61,   110,   148,
     169,   169,   154,   168,    71,   174,   147,    71,    28,    74,
     111,    28,    74,    71,   168,    71,   111,    71,    74,   110,
      83,    74,    11,    12,    71,    71,    71,    73,    75,    74,
      79,    71,   110,    75,   114,   110,   110,    61,   110,    27,
      79,    71,    71,    71,    74,    75,   149,    93,   156,   177,
     167,    75,   111,    71,     1,    22,    24,    29,    30,    31,
      32,    34,    35,    36,    69,    95,    96,    97,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   110,   123,   142,
     111,   138,   138,   114,    71,   111,   166,    71,    71,   111,
     169,    79,    79,   110,    27,    79,    61,    79,   110,   114,
     104,    71,    75,   157,     1,    69,    94,   104,    71,    74,
     172,    69,    70,    70,    70,    81,    70,    70,    69,    69,
      69,   110,   105,    73,   147,   158,   159,   105,    69,    72,
      74,    71,    74,    71,   169,    71,    79,    61,    79,   110,
     111,   110,    79,   169,    42,   112,    71,   105,   110,   110,
     110,    95,   110,   127,   128,   145,   160,   161,    11,    12,
     129,   130,   160,    69,   149,    69,    74,   109,   111,   137,
     148,    71,   110,   111,    79,    79,    79,    71,   110,   172,
      71,    71,    71,    29,    69,   147,    15,    74,    33,    15,
     158,    25,    26,   108,   125,   142,   172,    75,    79,    79,
      75,    95,    81,    70,   127,   149,   111,   129,   111,   111,
       4,    27,    63,    67,    68,    70,   126,   180,   183,    75,
     158,    95,   124,   125,    71,   111,    71,    23,    98,    95,
     110,    69,    71,   126,     4,     4,     4,   183,    27,    75,
      19,    20,    21,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    69,   124,    73,   124,    95,    71,   127,    82,
      75,    71,    75,   126,    70,   183,   183,   183,   183,   183,
     183,   183,   183,   183,   183,   183,   183,    69,    71,    95,
      75,    81,    95
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      88,    89,    89,    90,    90,    90,    90,    90,    92,    93,
      91,    94,    94,    94,    95,    95,    95,    95,    95,    95,
      95,    95,    95,    95,    95,    95,    96,    96,    97,    98,
      98,    99,   100,   101,   102,   103,   104,   105,   105,   105,
     105,   106,   107,   108,   109,   109,   110,   110,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
     112,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   114,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   116,   117,   118,   119,   120,
     121,   122,   122,   123,   124,   124,   124,   125,   125,   125,
     125,   125,   126,   126,   127,   127,   128,   128,   129,   129,
     129,   130,   130,   131,   132,   133,   134,   133,   135,   135,
     136,   137,   138,   138,   139,   140,   140,   141,   141,   142,
     142,   143,   143,   144,   144,   144,   144,   144,   145,   145,
     146,   147,   147,   148,   148,   149,   149,   150,   150,   151,
     151,   151,   151,   152,   153,   154,   154,   154,   155,   155,
     156,   156,   156,   157,   157,   158,   158,   159,   159,   160,
     161,   162,   162,   163,   163,   164,   165,   166,   167,   167,
     168,   168,   169,   169,   169,   170,   170,   171,   171,   171,
     172,   172,   173,   173,   174,   174,   175,   176,   177,   177,
     178,   178,   178,   178,   179,   179,   180,   180,   180,   181,
     182,   183,   183,   183,   183,   183,   183,   183,   183,   183,
     183,   183,   183,   183,   183,   183,   183,   183
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
       2,     2,     2,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       5,     3,     3,     3,     5,     5,     4,     5,     6,     7,
       8,     7,     5,     6,     6,     8,     7,     5,     3,     5,
       4,     6,     4,     5,     5,     1,     4,     4,     6,     6,
       4,     4,     7,     6,     7,     7,     9,     3,     3,     0,
       3,     3,     4,     4,     2,     2,     0,     3,     5,     4,
       4,     2,     1,     1,     0,     1,     1,     1,     1,     1,
       1,     1,     3,     3,     4,     0,     0,     5,     1,     3,
       2,     3,     0,     3,     2,     0,     2,     1,     0,     1,
       2,     1,     0,     1,     2,     2,     1,     5,     1,     2,
       4,     0,     1,     1,     1,     1,     1,     3,     3,     1,
       2,     3,     3,     4,     2,     0,     1,     2,     1,     3,
       2,     4,     1,     0,     5,     1,     3,     2,     4,     3,
       3,     1,     3,     2,     4,     1,     2,     3,     1,     2,
       1,     3,     0,     1,     2,     1,     3,     0,     1,     2,
       0,     3,     1,     3,     1,     1,     1,     3,     1,     3,
       1,     1,     2,     2,     1,     2,     1,     3,     3,     1,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     2,     2,     2
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
#line 249 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { (yyval.number) = rule_loop_open(); }
#line 2685 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 3: /* foreach_start: %empty  */
#line 250 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                      { (yyval.number) = rule_foreach_open(); }
#line 2691 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 4: /* block_start: %empty  */
#line 251 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.number) = rule_block_open(); }
#line 2697 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 5: /* special_context_start: %empty  */
#line 252 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { (yyval.number) = rule_special_context_open(); }
#line 2703 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 6: /* tree_context_start: %empty  */
#line 253 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                           { (yyval.number) = rule_tree_context_open(); }
#line 2709 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 7: /* dollar_start: %empty  */
#line 254 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                     { (yyval.contextp) = rule_dollar_open(); }
#line 2715 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 8: /* all: program  */
#line 263 "$REPO_ROOT$/src/compiler/internal/grammar.y"
           { rule_program((yyvsp[0].node)); }
#line 2721 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 9: /* program: program def opt_semicolon  */
#line 268 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_program_append(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 2727 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 10: /* program: %empty  */
#line 269 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.node) = 0; }
#line 2733 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 12: /* opt_semicolon: ';'  */
#line 275 "$REPO_ROOT$/src/compiler/internal/grammar.y"
         { rule_opt_semicolon(); }
#line 2739 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 14: /* def: type name_list ';'  */
#line 287 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_def_global_var((yyvsp[-2].number)); (yyval.node) = 0; }
#line 2745 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 18: /* @1: %empty  */
#line 313 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_func_type((yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].string)); }
#line 2751 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 19: /* @2: %empty  */
#line 315 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_func_proto((yyvsp[-3].number), (yyvsp[-5].number), (yyvsp[-4].string), &(yyvsp[-4].shared_string), (yyvsp[-1].argument)); }
#line 2757 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 20: /* function: type optional_star identifier @1 '(' argument ')' @2 block_or_semi  */
#line 317 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_func(&(yyval.node), (yyvsp[-5].number), (yyvsp[-7].number), (yyvsp[-6].shared_string), (yyvsp[-3].argument), &(yyvsp[-1].number), &(yyvsp[0].node)); }
#line 2763 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 21: /* block_or_semi: block  */
#line 322 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = rule_block_or_semi((yyvsp[0].decl).node); }
#line 2769 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 22: /* block_or_semi: ';'  */
#line 323 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2775 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 23: /* block_or_semi: error  */
#line 324 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2781 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 24: /* statement: comma_expr ';'  */
#line 334 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_expr((yyvsp[-1].node)); }
#line 2787 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 28: /* statement: stmt_for  */
#line 338 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2793 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 29: /* statement: stmt_foreach  */
#line 339 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2799 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 32: /* statement: block  */
#line 342 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2805 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 33: /* statement: ';'  */
#line 343 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = 0; }
#line 2811 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 34: /* statement: L_BREAK ';'  */
#line 344 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_break(); }
#line 2817 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 35: /* statement: L_CONTINUE ';'  */
#line 345 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_continue(); }
#line 2823 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 36: /* stmt_return: L_RETURN ';'  */
#line 351 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_return_void(&(yyval.node)); }
#line 2829 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 37: /* stmt_return: L_RETURN comma_expr ';'  */
#line 352 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_return_expr(&(yyval.node), (yyvsp[-1].node)); }
#line 2835 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 38: /* stmt_cond: L_IF '(' comma_expr ')' statement optional_else_part  */
#line 358 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_cond(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 2841 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 39: /* optional_else_part: %empty  */
#line 362 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                { (yyval.node) = 0; }
#line 2847 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 40: /* optional_else_part: L_ELSE statement  */
#line 363 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                 { (yyval.node) = (yyvsp[0].node); }
#line 2853 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 41: /* stmt_while: L_WHILE '(' comma_expr ')' loop_start statement  */
#line 369 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_while(&(yyval.node), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2859 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 42: /* stmt_do: L_DO loop_start statement L_WHILE '(' comma_expr ')' ';'  */
#line 375 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_do(&(yyval.node), (yyvsp[-5].node), (yyvsp[-2].node), (yyvsp[-6].number)); }
#line 2865 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 43: /* stmt_for: L_FOR '(' for_init ';' for_expr ';' for_expr ')' loop_start statement  */
#line 381 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for(&(yyval.decl), &(yyvsp[-7].decl), (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2871 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 44: /* stmt_foreach: L_FOREACH '(' foreach_vars L_IN expr ')' foreach_start statement  */
#line 387 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach(&(yyval.decl), &(yyvsp[-5].decl), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2877 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 45: /* stmt_switch: switch_header '{' local_declarations case switch_block '}'  */
#line 393 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_switch(&(yyval.node), (yyvsp[-5].switch_header).expr, &(yyvsp[-3].decl), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[-5].switch_header).saved_context, (yyvsp[-5].switch_header).saved_cases_size); }
#line 2883 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 46: /* block: '{' block_start statements '}'  */
#line 405 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block(&(yyval.decl), (yyvsp[-1].decl).node, (yyvsp[-2].number)); }
#line 2889 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 47: /* statements: %empty  */
#line 411 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_empty(&(yyval.decl)); }
#line 2895 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 48: /* statements: statement statements  */
#line 413 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_stmt(&(yyval.decl), (yyvsp[-1].node), &(yyvsp[0].decl)); }
#line 2901 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 49: /* statements: local_declaration_statement statements  */
#line 415 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_decl(&(yyval.decl), &(yyvsp[-1].decl), &(yyvsp[0].decl)); }
#line 2907 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 50: /* statements: error ';' statements  */
#line 417 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_error(&(yyval.decl), &(yyvsp[0].decl)); }
#line 2913 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 51: /* local_decl_statement_header: basic_type  */
#line 422 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { rule_local_declaration_statement_set_type((yyvsp[0].number)); (yyval.number) = (yyvsp[0].number); }
#line 2919 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 52: /* local_declaration_statement: local_decl_statement_header local_name_list ';'  */
#line 427 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_declaration_statement(&(yyval.decl), &(yyvsp[-1].decl)); }
#line 2925 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 53: /* local_decl_header: basic_type  */
#line 432 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { rule_local_declarations_set_type((yyvsp[0].number)); (yyval.number) = (yyvsp[0].number); }
#line 2931 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 54: /* local_declarations: %empty  */
#line 437 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.decl).node = 0; (yyval.decl).num = 0; }
#line 2937 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 55: /* local_declarations: local_declarations local_decl_header local_name_list ';'  */
#line 439 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_declarations(&(yyval.decl), &(yyvsp[-3].decl), &(yyvsp[-1].decl)); }
#line 2943 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 57: /* comma_expr: comma_expr ',' expr  */
#line 458 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                { rule_comma_expr(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2949 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 58: /* expr: ref lvalue  */
#line 462 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_ref(&(yyval.node), (yyvsp[0].node)); }
#line 2955 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 59: /* expr: lvalue L_ASSIGN expr  */
#line 463 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_assign(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 2961 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 60: /* expr: error L_ASSIGN expr  */
#line 464 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_assign_error(&(yyval.node), (yyvsp[0].node)); }
#line 2967 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 61: /* expr: expr '?' expr ':' expr  */
#line 466 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                            { rule_expr_ternary(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2973 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 62: /* expr: expr L_QUESTION_QUESTION expr  */
#line 467 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_nullish(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2979 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 63: /* expr: expr L_LOR expr  */
#line 468 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_lor(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2985 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 64: /* expr: expr L_LAND expr  */
#line 469 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_land(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2991 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 65: /* expr: expr '|' expr  */
#line 471 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_or(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2997 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 66: /* expr: expr '^' expr  */
#line 472 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_xor(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3003 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 67: /* expr: expr '&' expr  */
#line 473 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_and(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3009 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 68: /* expr: expr L_EQ_NE expr  */
#line 475 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_expr_eq_ne(&(yyval.node), (yyvsp[-1].number), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3015 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 69: /* expr: expr L_ORDER expr  */
#line 476 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_order(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3021 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 70: /* expr: expr '<' expr  */
#line 477 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_lt(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3027 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 71: /* expr: expr L_SHIFT expr  */
#line 479 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_expr_shift(&(yyval.node), (yyvsp[-1].number), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3033 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 72: /* expr: expr '+' expr  */
#line 481 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             {
        rule_set_operand_ranges((yylsp[-2]).first_line, (yylsp[-2]).first_column, (yylsp[-2]).last_column,
                                (yylsp[-1]).first_line, (yylsp[-1]).first_column,
                                (yylsp[0]).first_line, (yylsp[0]).first_column, (yylsp[0]).last_column);
        rule_expr_add(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node));
        rule_clear_operand_ranges();
    }
#line 3045 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 73: /* expr: expr '-' expr  */
#line 488 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_sub(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3051 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 74: /* expr: expr '*' expr  */
#line 489 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_mul(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3057 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 75: /* expr: expr '%' expr  */
#line 490 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_mod(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3063 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 76: /* expr: expr '/' expr  */
#line 491 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_div(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3069 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 77: /* expr: cast expr  */
#line 493 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_cast(&(yyval.node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3075 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 78: /* expr: L_INC_DEC lvalue  */
#line 494 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_expr_pre_incdec(&(yyval.node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3081 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 79: /* expr: '!' expr  */
#line 495 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_expr_not(&(yyval.node), (yyvsp[0].node)); }
#line 3087 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 80: /* expr: '~' expr  */
#line 496 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_expr_compl(&(yyval.node), (yyvsp[0].node)); }
#line 3093 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 81: /* expr: '-' expr  */
#line 497 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_neg(&(yyval.node), (yyvsp[0].node)); }
#line 3099 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 82: /* expr: L_AWAIT expr  */
#line 498 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_await(&(yyval.node), (yyvsp[0].node)); }
#line 3105 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 83: /* expr: lvalue L_INC_DEC  */
#line 500 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_expr_post_incdec(&(yyval.node), (yyvsp[0].number), (yyvsp[-1].node)); }
#line 3111 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 90: /* lvalue: primary_expr  */
#line 512 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                { (yyval.node) = rule_lvalue((yyvsp[0].node)); }
#line 3117 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 92: /* primary_expr: L_DEFINED_NAME  */
#line 522 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_defined_name(&(yyval.node), (yyvsp[0].ihe)); }
#line 3123 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 93: /* primary_expr: L_IDENTIFIER  */
#line 523 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_identifier(&(yyval.node), (yyvsp[0].string)); }
#line 3129 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 94: /* primary_expr: L_PARAMETER  */
#line 524 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_parameter(&(yyval.node), (yyvsp[0].number)); }
#line 3135 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 96: /* primary_expr: '(' comma_expr ')'  */
#line 526 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { (yyval.node) = (yyvsp[-1].node); }
#line 3141 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 100: /* primary_expr: '$' '(' dollar_start comma_expr ')'  */
#line 533 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_dollar_expr(&(yyval.node), (yyvsp[-2].contextp), (yyvsp[-1].node)); }
#line 3147 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 101: /* primary_expr: primary_expr L_ARROW identifier  */
#line 536 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                 { rule_primary_expr_member_arrow(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 3153 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 102: /* primary_expr: primary_expr '.' identifier  */
#line 537 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                 { rule_primary_expr_member_dot(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 3159 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 103: /* primary_expr: primary_expr L_OPTIONAL_DOT identifier  */
#line 543 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_member_optional(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 3165 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 104: /* primary_expr: primary_expr L_OPTIONAL_DOT '[' comma_expr ']'  */
#line 545 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_optional(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3171 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 105: /* primary_expr: primary_expr L_DOT_OPTIONAL '[' comma_expr ']'  */
#line 547 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_optional(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3177 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 106: /* primary_expr: primary_expr '[' comma_expr ']'  */
#line 552 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3183 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 107: /* primary_expr: primary_expr '[' '<' comma_expr ']'  */
#line 554 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_r(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3189 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 108: /* primary_expr: primary_expr '[' comma_expr L_RANGE comma_expr ']'  */
#line 556 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nn(&(yyval.node), (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3195 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 109: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE comma_expr ']'  */
#line 558 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rn(&(yyval.node), (yyvsp[-6].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3201 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 110: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE '<' comma_expr ']'  */
#line 560 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rr(&(yyval.node), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3207 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 111: /* primary_expr: primary_expr '[' comma_expr L_RANGE '<' comma_expr ']'  */
#line 562 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nr(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3213 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 112: /* primary_expr: primary_expr '[' comma_expr L_RANGE ']'  */
#line 564 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_ne(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node)); }
#line 3219 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 113: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE ']'  */
#line 566 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_re(&(yyval.node), (yyvsp[-5].node), (yyvsp[-2].node)); }
#line 3225 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 114: /* primary_expr: primary_expr '[' L_RANGE '<' comma_expr ']'  */
#line 568 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nr(&(yyval.node), (yyvsp[-5].node), 0, (yyvsp[-1].node)); }
#line 3231 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 115: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE '<' expr ']'  */
#line 571 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rr(&(yyval.node), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3237 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 116: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE expr ']'  */
#line 573 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rn(&(yyval.node), (yyvsp[-6].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3243 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 117: /* primary_expr: lambda_return_type '(' argument ')' block  */
#line 578 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_anon_func(&(yyval.node), &(yyvsp[-4].func_block), &(yyvsp[-2].argument), &(yyvsp[0].decl)); }
#line 3249 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 118: /* primary_expr: functional_open ':' ')'  */
#line 585 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_1(&(yyval.node), (yyvsp[-2].number)); }
#line 3255 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 119: /* primary_expr: functional_open ',' arg_list ':' ')'  */
#line 587 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_2(&(yyval.node), (yyvsp[-4].number), (yyvsp[-2].node)); }
#line 3261 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 120: /* primary_expr: L_FUNCTION_OPEN L_DEFINED_NAME ':' ')'  */
#line 596 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_1(&(yyval.node), rule_functional_ref((yyvsp[-2].ihe))); }
#line 3267 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 121: /* primary_expr: L_FUNCTION_OPEN L_DEFINED_NAME ',' arg_list ':' ')'  */
#line 598 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_2(&(yyval.node), rule_functional_ref((yyvsp[-4].ihe)), (yyvsp[-2].node)); }
#line 3273 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 122: /* primary_expr: L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 600 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_3(&(yyval.node), (yyvsp[-2].node)); }
#line 3279 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 123: /* primary_expr: '(' '[' opt_pair_list ']' ')'  */
#line 603 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_primary_expr_mapping(&(yyval.node), (yyvsp[-2].node)); }
#line 3285 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 124: /* primary_expr: '(' '{' opt_arg_list '}' ')'  */
#line 604 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_primary_expr_array(&(yyval.node), (yyvsp[-2].node)); }
#line 3291 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 125: /* call_open: '('  */
#line 614 "$REPO_ROOT$/src/compiler/internal/grammar.y"
       { rule_call_open(&(yyval.call_open).saved_context, &(yyval.call_open).saved_refs); }
#line 3297 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 126: /* function_call: efun_override call_open opt_arg_list ')'  */
#line 628 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_efun(&(yyval.node), (yyvsp[-3].number), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3303 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 127: /* function_call: L_NEW call_open opt_arg_list ')'  */
#line 630 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new(&(yyval.node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3309 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 128: /* function_call: L_NEW '(' L_CLASS L_DEFINED_NAME opt_class_init ')'  */
#line 632 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new_class(&(yyval.node), (yyvsp[-2].ihe), (yyvsp[-1].node)); }
#line 3315 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 129: /* function_call: L_NEW '(' L_CLASS L_IDENTIFIER opt_class_init ')'  */
#line 634 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new_class_undef(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node)); }
#line 3321 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 130: /* function_call: L_DEFINED_NAME call_open opt_arg_list ')'  */
#line 636 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_defined_name(&(yyval.node), (yyvsp[-3].ihe), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3327 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 131: /* function_call: function_name call_open opt_arg_list ')'  */
#line 638 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_name(&(yyval.node), (yyvsp[-3].string), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3333 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 132: /* function_call: primary_expr '[' comma_expr ']' call_open opt_arg_list ')'  */
#line 640 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_indexed(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3339 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 133: /* function_call: primary_expr L_ARROW identifier call_open opt_arg_list ')'  */
#line 642 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_arrow(&(yyval.node), (yyvsp[-5].node), (yyvsp[-3].string), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3345 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 134: /* function_call: '(' '*' comma_expr ')' call_open opt_arg_list ')'  */
#line 644 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_star(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3351 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 135: /* sscanf: L_SSCANF '(' expr ',' expr lvalue_list ')'  */
#line 655 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_sscanf(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 3357 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 136: /* parse_command: L_PARSE_COMMAND '(' expr ',' expr ',' expr lvalue_list ')'  */
#line 661 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_parse_command(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 3363 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 137: /* catch: L_CATCH special_context_start expr_or_block  */
#line 667 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_catch(&(yyval.node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 3369 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 138: /* acatch: L_ACATCH acatch_context_start expr_or_block  */
#line 676 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_acatch(&(yyval.node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 3375 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 139: /* acatch_context_start: %empty  */
#line 679 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { (yyval.number) = rule_acatch_context_open(); }
#line 3381 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 140: /* time_expression: L_TIME_EXPRESSION special_context_start expr_or_block  */
#line 684 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_time_expression(&(yyval.node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 3387 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 141: /* tree: L_TREE tree_context_start block  */
#line 690 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_tree_block(&(yyval.node), (yyvsp[0].decl), (yyvsp[-1].number)); }
#line 3393 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 142: /* tree: L_TREE '(' comma_expr ')'  */
#line 692 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_tree_expr(&(yyval.node), (yyvsp[-1].node)); }
#line 3399 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 143: /* switch_header: L_SWITCH '(' comma_expr ')'  */
#line 703 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.switch_header).expr = (yyvsp[-1].node); rule_switch_pre(&(yyval.switch_header).saved_context, &(yyval.switch_header).saved_cases_size); }
#line 3405 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 144: /* switch_block: case switch_block  */
#line 708 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_switch_block_case(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3411 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 145: /* switch_block: statement switch_block  */
#line 709 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_switch_block_stmt(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3417 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 146: /* switch_block: %empty  */
#line 710 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { rule_switch_block_empty(&(yyval.node)); }
#line 3423 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 147: /* case: L_CASE case_label ':'  */
#line 717 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_single(&(yyval.node), (yyvsp[-1].node)); }
#line 3429 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 148: /* case: L_CASE case_label L_RANGE case_label ':'  */
#line 719 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3435 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 149: /* case: L_CASE case_label L_RANGE ':'  */
#line 721 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range_from(&(yyval.node), (yyvsp[-2].node)); }
#line 3441 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 150: /* case: L_CASE L_RANGE case_label ':'  */
#line 723 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range_to(&(yyval.node), (yyvsp[-1].node)); }
#line 3447 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 151: /* case: L_DEFAULT ':'  */
#line 725 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_default(&(yyval.node)); }
#line 3453 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 152: /* case_label: constant  */
#line 731 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_case_label_constant(&(yyval.node), (yyvsp[0].number)); }
#line 3459 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 153: /* case_label: string_const  */
#line 732 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_case_label_string(&(yyval.node), (yyvsp[0].string)); }
#line 3465 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 154: /* for_expr: %empty  */
#line 742 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { (yyval.node) = 0; }
#line 3471 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 156: /* for_init: for_expr  */
#line 749 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for_init_expr(&(yyval.decl), (yyvsp[0].node)); }
#line 3477 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 157: /* for_init: single_new_local_def_with_init  */
#line 751 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for_init_local(&(yyval.decl), (yyvsp[0].node)); }
#line 3483 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 158: /* foreach_var: L_DEFINED_NAME  */
#line 756 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_defined(&(yyval.decl), (yyvsp[0].ihe)); }
#line 3489 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 159: /* foreach_var: single_new_local_def  */
#line 757 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_new_local(&(yyval.decl), (yyvsp[0].number)); }
#line 3495 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 160: /* foreach_var: L_IDENTIFIER  */
#line 758 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_identifier(&(yyval.decl), (yyvsp[0].string)); }
#line 3501 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 161: /* foreach_vars: foreach_var  */
#line 764 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach_vars_single(&(yyval.decl), &(yyvsp[0].decl)); }
#line 3507 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 162: /* foreach_vars: foreach_var ',' foreach_var  */
#line 766 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach_vars_double(&(yyval.decl), &(yyvsp[-2].decl), &(yyvsp[0].decl)); }
#line 3513 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 163: /* type_decl: class_header member_list '}'  */
#line 777 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_define_class_members((yyvsp[-2].class_header).ihe, (yyvsp[-2].class_header).classname_index); (yyval.node) = 0; }
#line 3519 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 164: /* class_header: type_modifier_list L_CLASS identifier '{'  */
#line 783 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.class_header).ihe = rule_define_class(&(yyval.class_header).classname_index, (yyvsp[-1].string)); }
#line 3525 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 166: /* $@3: %empty  */
#line 789 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                           { rule_member_list_set_type((yyvsp[0].number)); }
#line 3531 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 170: /* member_name: optional_star identifier  */
#line 801 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { rule_member_name((yyvsp[-1].number), (yyvsp[0].string)); }
#line 3537 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 171: /* class_init: identifier ':' expr  */
#line 806 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                       { (yyval.node) = rule_class_init((yyvsp[-2].string), (yyvsp[0].node)); }
#line 3543 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 172: /* opt_class_init: %empty  */
#line 811 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.node) = rule_opt_class_init_empty(); }
#line 3549 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 173: /* opt_class_init: opt_class_init ',' class_init  */
#line 812 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { (yyval.node) = rule_opt_class_init((yyvsp[-2].node), (yyvsp[0].node)); }
#line 3555 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 174: /* type: type_modifier_list opt_basic_type  */
#line 824 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { (yyval.number) = rule_type((yyvsp[-1].number), (yyvsp[0].number)); }
#line 3561 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 175: /* type_modifier_list: %empty  */
#line 829 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { (yyval.number) = 0; }
#line 3567 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 176: /* type_modifier_list: L_TYPE_MODIFIER type_modifier_list  */
#line 830 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                               { (yyval.number) = rule_type_modifier_list((yyvsp[-1].number), (yyvsp[0].number)); }
#line 3573 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 178: /* opt_basic_type: %empty  */
#line 836 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { (yyval.number) = rule_opt_basic_type_empty(); }
#line 3579 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 180: /* basic_type: opt_atomic_type L_ARRAY  */
#line 842 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { (yyval.number) = (yyvsp[-1].number) | TYPE_MOD_ARRAY; }
#line 3585 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 182: /* opt_atomic_type: %empty  */
#line 848 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { (yyval.number) = rule_opt_basic_type_empty(); }
#line 3591 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 184: /* atomic_type: L_CLASS L_DEFINED_NAME  */
#line 854 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.number) = rule_atomic_type_class((yyvsp[0].ihe)); }
#line 3597 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 185: /* atomic_type: L_CLASS L_IDENTIFIER  */
#line 855 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.number) = rule_atomic_type_class_identifier((yyvsp[0].string)); }
#line 3603 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 186: /* atomic_type: L_PROMISE  */
#line 859 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { (yyval.number) = rule_atomic_type_promise(); }
#line 3609 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 187: /* atomic_type: L_PROMISE '<' basic_type optional_star L_ORDER  */
#line 861 "$REPO_ROOT$/src/compiler/internal/grammar.y"
      { (yyval.number) = rule_atomic_type_promise_of((yyvsp[-2].number) | (yyvsp[-1].number), (yyvsp[0].number)); }
#line 3615 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 189: /* arg_type: basic_type ref  */
#line 867 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.number) = (yyvsp[-1].number) | LOCAL_MOD_REF; }
#line 3621 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 190: /* cast: '(' basic_type optional_star ')'  */
#line 872 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { (yyval.number) = rule_cast((yyvsp[-2].number), (yyvsp[-1].number)); }
#line 3627 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 191: /* optional_star: %empty  */
#line 877 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.number) = 0; }
#line 3633 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 192: /* optional_star: '*'  */
#line 878 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.number) = TYPE_MOD_ARRAY; }
#line 3639 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 193: /* identifier: L_DEFINED_NAME  */
#line 888 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { (yyval.string) = rule_identifier_defined_name((yyvsp[0].ihe)); }
#line 3645 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 196: /* new_local_name: L_DEFINED_NAME  */
#line 895 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.string) = rule_new_local_name_redefine((yyvsp[0].ihe)); }
#line 3651 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 197: /* efun_override: L_EFUN L_COLON_COLON identifier  */
#line 900 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.number) = rule_efun_override((yyvsp[0].string)); }
#line 3657 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 198: /* efun_override: L_EFUN L_COLON_COLON L_NEW  */
#line 901 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.number) = rule_efun_override_new(); }
#line 3663 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 200: /* function_name: L_COLON_COLON identifier  */
#line 912 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { (yyval.string) = rule_function_name_colon_colon((yyvsp[0].string)); }
#line 3669 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 201: /* function_name: L_BASIC_TYPE L_COLON_COLON identifier  */
#line 913 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { (yyval.string) = rule_function_name_type((yyvsp[-2].number), (yyvsp[0].string)); }
#line 3675 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 202: /* function_name: identifier L_COLON_COLON identifier  */
#line 914 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                    { (yyval.string) = rule_function_name_obj((yyvsp[-2].string), (yyvsp[0].string)); }
#line 3681 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 203: /* inheritance: type_modifier_list L_INHERIT string_const ';'  */
#line 920 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { if (rule_inheritence(&(yyval.node), (yyvsp[-3].number), (yyvsp[-1].string))) { YYACCEPT; } }
#line 3687 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 204: /* modifier_change: type_modifier_list ':'  */
#line 925 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { (yyval.node) = rule_modifier_change((yyvsp[-1].number)); }
#line 3693 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 205: /* argument: %empty  */
#line 936 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.argument).num_arg = 0; (yyval.argument).flags = 0; }
#line 3699 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 207: /* argument: argument_list L_DOT_DOT_DOT  */
#line 939 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_varargs(&(yyval.argument), &(yyvsp[-1].argument)); }
#line 3705 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 208: /* argument_list: param_decl  */
#line 945 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_list_single(&(yyval.argument), (yyvsp[0].number)); }
#line 3711 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 209: /* argument_list: argument_list ',' param_decl  */
#line 947 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_list_multi(&(yyval.argument), &(yyvsp[-2].argument), (yyvsp[0].number)); }
#line 3717 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 210: /* param_decl: arg_type optional_star  */
#line 957 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_typed((yyvsp[-1].number) | (yyvsp[0].number)); }
#line 3723 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 211: /* param_decl: arg_type optional_star new_local_name optional_default_arg_value  */
#line 959 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_typed_name((yyvsp[-3].number) | (yyvsp[-2].number), (yyvsp[-1].string), (yyvsp[0].node)); }
#line 3729 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 212: /* param_decl: new_local_name  */
#line 961 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_untyped_name((yyvsp[0].string)); }
#line 3735 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 213: /* optional_default_arg_value: %empty  */
#line 966 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                          { (yyval.node) = 0; }
#line 3741 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 214: /* optional_default_arg_value: ':' L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 967 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                            { (yyval.node) = rule_default_arg_value((yyvsp[-2].node)); }
#line 3747 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 215: /* local_name_list: new_local_def  */
#line 973 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_name_list_single(&(yyval.decl), (yyvsp[0].node)); }
#line 3753 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 216: /* local_name_list: new_local_def ',' local_name_list  */
#line 975 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_name_list_multi(&(yyval.decl), (yyvsp[-2].node), &(yyvsp[0].decl)); }
#line 3759 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 217: /* new_local_def: optional_star new_local_name  */
#line 981 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_new_local_def((yyvsp[0].string), (yyvsp[-1].number)); }
#line 3765 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 218: /* new_local_def: optional_star new_local_name L_ASSIGN expr  */
#line 983 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_new_local_def_with_init((yyvsp[-2].string), (yyvsp[-3].number), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3771 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 219: /* single_new_local_def: arg_type optional_star new_local_name  */
#line 989 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_single_new_local_def(&(yyval.number), (yyvsp[-2].number) | (yyvsp[-1].number), (yyvsp[0].string)); }
#line 3777 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 220: /* single_new_local_def_with_init: single_new_local_def L_ASSIGN expr  */
#line 995 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_single_new_local_def_with_init((yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3783 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 223: /* new_name: optional_star identifier  */
#line 1011 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { rule_new_name((yyvsp[-1].number), (yyvsp[0].string)); }
#line 3789 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 224: /* new_name: optional_star identifier L_ASSIGN expr  */
#line 1012 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { rule_new_name_with_init((yyvsp[-3].number), (yyvsp[-2].string), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3795 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 225: /* lambda_return_type: L_BASIC_TYPE  */
#line 1023 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                { rule_lambda_return_type(&(yyval.func_block), (yyvsp[0].number)); }
#line 3801 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 226: /* functional_open: L_FUNCTION_OPEN efun_override  */
#line 1031 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                 { (yyval.number) = rule_functional_open((yyvsp[0].number)); }
#line 3807 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 227: /* assoc_pair: expr ':' expr  */
#line 1041 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_assoc_pair(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3813 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 228: /* spread_expr: expr  */
#line 1046 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_spread_expr_normal(&(yyval.node), (yyvsp[0].node)); }
#line 3819 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 229: /* spread_expr: expr L_DOT_DOT_DOT  */
#line 1047 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_spread_expr_dots(&(yyval.node), (yyvsp[-1].node)); }
#line 3825 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 230: /* arg_list: spread_expr  */
#line 1053 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_arg_list_single(&(yyval.node), (yyvsp[0].node)); }
#line 3831 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 231: /* arg_list: arg_list ',' spread_expr  */
#line 1055 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_arg_list_multi(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3837 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 232: /* opt_arg_list: %empty  */
#line 1061 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list_empty(&(yyval.node)); }
#line 3843 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 233: /* opt_arg_list: arg_list  */
#line 1062 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list(&(yyval.node), (yyvsp[0].node)); }
#line 3849 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 234: /* opt_arg_list: arg_list ','  */
#line 1063 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list(&(yyval.node), (yyvsp[-1].node)); }
#line 3855 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 235: /* pair_list: assoc_pair  */
#line 1069 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_pair_list_single(&(yyval.node), (yyvsp[0].node)); }
#line 3861 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 236: /* pair_list: pair_list ',' assoc_pair  */
#line 1071 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_pair_list_multi(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3867 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 237: /* opt_pair_list: %empty  */
#line 1077 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list_empty(&(yyval.node)); }
#line 3873 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 238: /* opt_pair_list: pair_list  */
#line 1078 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list(&(yyval.node), (yyvsp[0].node)); }
#line 3879 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 239: /* opt_pair_list: pair_list ','  */
#line 1079 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list(&(yyval.node), (yyvsp[-1].node)); }
#line 3885 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 240: /* lvalue_list: %empty  */
#line 1084 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.node) = rule_lvalue_list_empty(); }
#line 3891 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 241: /* lvalue_list: ',' lvalue lvalue_list  */
#line 1085 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { (yyval.node) = rule_lvalue_list((yyvsp[-1].node), (yyvsp[0].node)); }
#line 3897 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 242: /* expr_or_block: block  */
#line 1090 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_expr_or_block_block((yyvsp[0].decl)); }
#line 3903 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 243: /* expr_or_block: '(' comma_expr ')'  */
#line 1091 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_expr_or_block_expr((yyvsp[-1].node)); }
#line 3909 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 246: /* string: string_literal  */
#line 1107 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_string(&(yyval.node), (yyvsp[0].string)); }
#line 3915 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 247: /* template_literal: L_TEMPLATE_HEAD expr template_parts  */
#line 1119 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_template_literal(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3921 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 248: /* template_parts: L_TEMPLATE_TAIL  */
#line 1123 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                      { rule_template_parts_tail(&(yyval.node), (yyvsp[0].string)); }
#line 3927 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 249: /* template_parts: L_TEMPLATE_MIDDLE expr template_parts  */
#line 1124 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                       { rule_template_parts_middle(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3933 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 252: /* string_like: string_like string  */
#line 1135 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                               { rule_string_like_concat(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3939 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 253: /* string_like: string_like template_literal  */
#line 1136 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_string_like_concat(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3945 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 255: /* string_literal: string_literal L_STRING  */
#line 1144 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.string) = rule_string_literal_concat((yyvsp[-1].string), (yyvsp[0].string)); }
#line 3951 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 257: /* string_const: '(' string_const ')'  */
#line 1151 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                        { (yyval.string) = (yyvsp[-1].string); }
#line 3957 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 258: /* string_const: string_const '+' string_const  */
#line 1152 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { (yyval.string) = rule_string_literal_concat((yyvsp[-2].string), (yyvsp[0].string)); }
#line 3963 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 259: /* number: L_NUMBER  */
#line 1156 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { rule_number(&(yyval.node), (yyvsp[0].number)); }
#line 3969 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 260: /* real: L_REAL  */
#line 1160 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { rule_real(&(yyval.node), (yyvsp[0].real)); }
#line 3975 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 261: /* constant: constant '|' constant  */
#line 1166 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_or(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3981 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 262: /* constant: constant '^' constant  */
#line 1167 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_xor(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3987 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 263: /* constant: constant '&' constant  */
#line 1168 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_and(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3993 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 264: /* constant: constant L_EQ_NE constant  */
#line 1169 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                             { rule_constant_eq_ne(&(yyval.number), (yyvsp[-1].number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3999 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 265: /* constant: constant L_ORDER constant  */
#line 1170 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { rule_constant_order(&(yyval.number), (yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].number)); }
#line 4005 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 266: /* constant: constant '<' constant  */
#line 1171 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_lt(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 4011 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 267: /* constant: constant L_SHIFT constant  */
#line 1172 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                             { rule_constant_shift(&(yyval.number), (yyvsp[-1].number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 4017 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 268: /* constant: constant '+' constant  */
#line 1173 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_add(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 4023 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 269: /* constant: constant '-' constant  */
#line 1174 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_sub(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 4029 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 270: /* constant: constant '*' constant  */
#line 1175 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_mul(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 4035 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 271: /* constant: constant '%' constant  */
#line 1176 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_mod(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 4041 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 272: /* constant: constant '/' constant  */
#line 1177 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_div(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 4047 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 273: /* constant: '(' constant ')'  */
#line 1178 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { (yyval.number) = (yyvsp[-1].number); }
#line 4053 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 274: /* constant: L_NUMBER  */
#line 1179 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { (yyval.number) = (yyvsp[0].number); }
#line 4059 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 275: /* constant: '-' L_NUMBER  */
#line 1180 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_neg(&(yyval.number), (yyvsp[0].number)); }
#line 4065 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 276: /* constant: '!' L_NUMBER  */
#line 1181 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_not(&(yyval.number), (yyvsp[0].number)); }
#line 4071 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 277: /* constant: '~' L_NUMBER  */
#line 1182 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_compl(&(yyval.number), (yyvsp[0].number)); }
#line 4077 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;


#line 4081 "$BUILD_ROOT$/src/grammar.autogen.cc"

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
#line 1185 "$REPO_ROOT$/src/compiler/internal/grammar.y"


// Public accessor for the parser's symbol-name table: maps a raw yylex
// token number (what lpcc --tokens prints) to its grammar spelling
// ("L_IDENTIFIER", "'{'"). Lives in the epilogue because yysymbol_name()
// and YYTRANSLATE are file-static in the generated parser; consumed by
// the lpcc --json staged outputs (stage_output.cc).
const char* lpc_token_name(int token) {
  return yysymbol_name(YY_CAST(yysymbol_kind_t, YYTRANSLATE(token)));
}
/* FluffOS generated-from grammar.y sha256=d24fef6a097ead9873e55993e9e1dc55af517ccc91b32a044d0570f3d8d7128c */
