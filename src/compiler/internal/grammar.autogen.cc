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
  YYSYMBOL_dollar_start = 85,              /* dollar_start  */
  YYSYMBOL_all = 86,                       /* all  */
  YYSYMBOL_program = 87,                   /* program  */
  YYSYMBOL_opt_semicolon = 88,             /* opt_semicolon  */
  YYSYMBOL_def = 89,                       /* def  */
  YYSYMBOL_function = 90,                  /* function  */
  YYSYMBOL_91_1 = 91,                      /* @1  */
  YYSYMBOL_92_2 = 92,                      /* @2  */
  YYSYMBOL_block_or_semi = 93,             /* block_or_semi  */
  YYSYMBOL_statement = 94,                 /* statement  */
  YYSYMBOL_stmt_return = 95,               /* stmt_return  */
  YYSYMBOL_stmt_cond = 96,                 /* stmt_cond  */
  YYSYMBOL_optional_else_part = 97,        /* optional_else_part  */
  YYSYMBOL_stmt_while = 98,                /* stmt_while  */
  YYSYMBOL_stmt_do = 99,                   /* stmt_do  */
  YYSYMBOL_stmt_for = 100,                 /* stmt_for  */
  YYSYMBOL_stmt_foreach = 101,             /* stmt_foreach  */
  YYSYMBOL_stmt_switch = 102,              /* stmt_switch  */
  YYSYMBOL_block = 103,                    /* block  */
  YYSYMBOL_statements = 104,               /* statements  */
  YYSYMBOL_local_decl_statement_header = 105, /* local_decl_statement_header  */
  YYSYMBOL_local_declaration_statement = 106, /* local_declaration_statement  */
  YYSYMBOL_local_decl_header = 107,        /* local_decl_header  */
  YYSYMBOL_local_declarations = 108,       /* local_declarations  */
  YYSYMBOL_comma_expr = 109,               /* comma_expr  */
  YYSYMBOL_expr = 110,                     /* expr  */
  YYSYMBOL_lvalue = 111,                   /* lvalue  */
  YYSYMBOL_primary_expr = 112,             /* primary_expr  */
  YYSYMBOL_call_open = 113,                /* call_open  */
  YYSYMBOL_function_call = 114,            /* function_call  */
  YYSYMBOL_sscanf = 115,                   /* sscanf  */
  YYSYMBOL_parse_command = 116,            /* parse_command  */
  YYSYMBOL_catch = 117,                    /* catch  */
  YYSYMBOL_acatch = 118,                   /* acatch  */
  YYSYMBOL_acatch_context_start = 119,     /* acatch_context_start  */
  YYSYMBOL_time_expression = 120,          /* time_expression  */
  YYSYMBOL_tree = 121,                     /* tree  */
  YYSYMBOL_switch_header = 122,            /* switch_header  */
  YYSYMBOL_switch_block = 123,             /* switch_block  */
  YYSYMBOL_case = 124,                     /* case  */
  YYSYMBOL_case_label = 125,               /* case_label  */
  YYSYMBOL_for_expr = 126,                 /* for_expr  */
  YYSYMBOL_for_init = 127,                 /* for_init  */
  YYSYMBOL_foreach_var = 128,              /* foreach_var  */
  YYSYMBOL_foreach_vars = 129,             /* foreach_vars  */
  YYSYMBOL_type_decl = 130,                /* type_decl  */
  YYSYMBOL_class_header = 131,             /* class_header  */
  YYSYMBOL_member_list = 132,              /* member_list  */
  YYSYMBOL_133_3 = 133,                    /* $@3  */
  YYSYMBOL_member_name_list = 134,         /* member_name_list  */
  YYSYMBOL_member_name = 135,              /* member_name  */
  YYSYMBOL_class_init = 136,               /* class_init  */
  YYSYMBOL_opt_class_init = 137,           /* opt_class_init  */
  YYSYMBOL_type = 138,                     /* type  */
  YYSYMBOL_type_modifier_list = 139,       /* type_modifier_list  */
  YYSYMBOL_opt_basic_type = 140,           /* opt_basic_type  */
  YYSYMBOL_basic_type = 141,               /* basic_type  */
  YYSYMBOL_opt_atomic_type = 142,          /* opt_atomic_type  */
  YYSYMBOL_atomic_type = 143,              /* atomic_type  */
  YYSYMBOL_arg_type = 144,                 /* arg_type  */
  YYSYMBOL_cast = 145,                     /* cast  */
  YYSYMBOL_optional_star = 146,            /* optional_star  */
  YYSYMBOL_identifier = 147,               /* identifier  */
  YYSYMBOL_new_local_name = 148,           /* new_local_name  */
  YYSYMBOL_efun_override = 149,            /* efun_override  */
  YYSYMBOL_function_name = 150,            /* function_name  */
  YYSYMBOL_inheritance = 151,              /* inheritance  */
  YYSYMBOL_modifier_change = 152,          /* modifier_change  */
  YYSYMBOL_argument = 153,                 /* argument  */
  YYSYMBOL_argument_list = 154,            /* argument_list  */
  YYSYMBOL_param_decl = 155,               /* param_decl  */
  YYSYMBOL_optional_default_arg_value = 156, /* optional_default_arg_value  */
  YYSYMBOL_local_name_list = 157,          /* local_name_list  */
  YYSYMBOL_new_local_def = 158,            /* new_local_def  */
  YYSYMBOL_single_new_local_def = 159,     /* single_new_local_def  */
  YYSYMBOL_single_new_local_def_with_init = 160, /* single_new_local_def_with_init  */
  YYSYMBOL_name_list = 161,                /* name_list  */
  YYSYMBOL_new_name = 162,                 /* new_name  */
  YYSYMBOL_lambda_return_type = 163,       /* lambda_return_type  */
  YYSYMBOL_functional_open = 164,          /* functional_open  */
  YYSYMBOL_assoc_pair = 165,               /* assoc_pair  */
  YYSYMBOL_spread_expr = 166,              /* spread_expr  */
  YYSYMBOL_arg_list = 167,                 /* arg_list  */
  YYSYMBOL_opt_arg_list = 168,             /* opt_arg_list  */
  YYSYMBOL_pair_list = 169,                /* pair_list  */
  YYSYMBOL_opt_pair_list = 170,            /* opt_pair_list  */
  YYSYMBOL_lvalue_list = 171,              /* lvalue_list  */
  YYSYMBOL_expr_or_block = 172,            /* expr_or_block  */
  YYSYMBOL_ref = 173,                      /* ref  */
  YYSYMBOL_string = 174,                   /* string  */
  YYSYMBOL_template_literal = 175,         /* template_literal  */
  YYSYMBOL_template_parts = 176,           /* template_parts  */
  YYSYMBOL_string_like = 177,              /* string_like  */
  YYSYMBOL_string_literal = 178,           /* string_literal  */
  YYSYMBOL_string_const = 179,             /* string_const  */
  YYSYMBOL_number = 180,                   /* number  */
  YYSYMBOL_real = 181,                     /* real  */
  YYSYMBOL_constant = 182                  /* constant  */
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
#define YYLAST   2107

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  80
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  103
/* YYNRULES -- Number of rules.  */
#define YYNRULES  276
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  542

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
       0,   248,   248,   249,   250,   251,   252,   261,   266,   267,
     272,   273,   284,   285,   286,   287,   288,   311,   313,   310,
     320,   321,   322,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   349,   350,   355,   360,   361,
     366,   372,   378,   384,   390,   402,   408,   410,   412,   414,
     420,   424,   430,   434,   436,   455,   456,   460,   461,   462,
     464,   465,   466,   467,   469,   470,   471,   473,   474,   475,
     477,   479,   486,   487,   488,   489,   491,   492,   493,   494,
     495,   496,   498,   500,   501,   502,   503,   504,   505,   510,
     519,   520,   521,   522,   523,   524,   525,   526,   527,   530,
     534,   535,   540,   542,   544,   549,   551,   553,   555,   557,
     559,   561,   563,   565,   568,   570,   575,   582,   584,   593,
     595,   597,   601,   602,   612,   625,   627,   629,   631,   633,
     635,   637,   639,   641,   652,   658,   664,   673,   677,   681,
     687,   689,   700,   706,   707,   708,   714,   716,   718,   720,
     722,   729,   730,   740,   741,   746,   748,   754,   755,   756,
     761,   763,   774,   780,   786,   787,   787,   793,   794,   799,
     804,   809,   810,   822,   827,   828,   833,   834,   839,   840,
     845,   846,   851,   852,   853,   857,   858,   864,   865,   870,
     875,   876,   886,   887,   892,   893,   898,   899,   909,   910,
     911,   912,   917,   923,   933,   935,   936,   942,   944,   954,
     956,   958,   964,   965,   970,   972,   978,   980,   986,   992,
    1003,  1004,  1009,  1010,  1021,  1029,  1039,  1044,  1045,  1050,
    1052,  1059,  1060,  1061,  1066,  1068,  1075,  1076,  1077,  1082,
    1083,  1088,  1089,  1094,  1095,  1105,  1117,  1121,  1122,  1131,
    1132,  1133,  1134,  1141,  1142,  1148,  1149,  1150,  1154,  1158,
    1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,  1178,  1179,  1180
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
  "foreach_start", "block_start", "special_context_start", "dollar_start",
  "all", "program", "opt_semicolon", "def", "function", "@1", "@2",
  "block_or_semi", "statement", "stmt_return", "stmt_cond",
  "optional_else_part", "stmt_while", "stmt_do", "stmt_for",
  "stmt_foreach", "stmt_switch", "block", "statements",
  "local_decl_statement_header", "local_declaration_statement",
  "local_decl_header", "local_declarations", "comma_expr", "expr",
  "lvalue", "primary_expr", "call_open", "function_call", "sscanf",
  "parse_command", "catch", "acatch", "acatch_context_start",
  "time_expression", "tree", "switch_header", "switch_block", "case",
  "case_label", "for_expr", "for_init", "foreach_var", "foreach_vars",
  "type_decl", "class_header", "member_list", "$@3", "member_name_list",
  "member_name", "class_init", "opt_class_init", "type",
  "type_modifier_list", "opt_basic_type", "basic_type", "opt_atomic_type",
  "atomic_type", "arg_type", "cast", "optional_star", "identifier",
  "new_local_name", "efun_override", "function_name", "inheritance",
  "modifier_change", "argument", "argument_list", "param_decl",
  "optional_default_arg_value", "local_name_list", "new_local_def",
  "single_new_local_def", "single_new_local_def_with_init", "name_list",
  "new_name", "lambda_return_type", "functional_open", "assoc_pair",
  "spread_expr", "arg_list", "opt_arg_list", "pair_list", "opt_pair_list",
  "lvalue_list", "expr_or_block", "ref", "string", "template_literal",
  "template_parts", "string_like", "string_literal", "string_const",
  "number", "real", "constant", YY_NULLPTR
  };
  return yy_sname[yysymbol];
}
#endif

#define YYPACT_NINF (-424)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-239)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -424,    28,    50,  -424,    53,    15,  -424,  -424,  -424,    24,
      82,  -424,  -424,  -424,  -424,  -424,    76,  -424,   169,    32,
      73,  -424,    33,    92,   299,  -424,  -424,  -424,   111,   139,
     345,  -424,  -424,  -424,  -424,    -1,  -424,    24,  -424,    33,
     255,    93,   126,   102,   148,   190,  -424,  -424,  -424,    24,
    1670,   207,   169,  -424,   -23,  -424,    33,  -424,    24,  -424,
     250,   232,   169,   323,  -424,  -424,  1670,   308,    -9,    37,
     331,   254,   169,  1738,   316,  -424,  1670,  -424,  -424,   346,
    -424,   385,  -424,    52,  -424,  1670,  1670,  1670,   710,   392,
    2016,   337,   202,  -424,  -424,  -424,  -424,  -424,  -424,  -424,
    1670,   370,   400,   400,   407,   317,   254,  -424,  -424,   322,
     255,  -424,  -424,   274,   466,  -424,  -424,   468,  -424,    24,
    -424,  1670,  1832,   169,  -424,  1300,    55,   786,  -424,   204,
    -424,   220,   359,  2016,   400,  1670,   142,  -424,   142,  1670,
     142,   448,  1300,  1670,  -424,  -424,  -424,  -424,  -424,    61,
    1670,  1374,   556,   230,    24,  -424,  1670,  1670,  1670,  1670,
    1670,  1670,  1670,  1670,  1670,  1670,  1670,  1670,  1670,  1670,
    1670,  1670,  -424,  1670,   169,    21,   423,   169,  1158,  -424,
     169,  1300,  1300,   274,  1670,   443,  -424,  -424,  -424,  -424,
    -424,   -11,    24,  -424,   444,    -3,  -424,  -424,  -424,  2016,
    1670,  -424,  -424,  -424,  2041,  -424,   442,   446,  -424,  -424,
    1670,   449,  1670,   455,   315,  1670,  -424,  -424,  -424,  1914,
    -424,   429,   460,   275,   862,   297,   465,  1863,  -424,   445,
     462,  -424,   472,  1670,   747,   678,   601,   298,   278,   251,
    1883,  1055,  1195,  1271,   251,   171,   171,  -424,  -424,  -424,
    2016,   400,  1670,  -424,  1670,  -424,   483,  1670,    38,  -424,
     474,   477,   480,   377,  -424,  -424,   447,  -424,  -424,   217,
    1832,  -424,  1226,  -424,   412,  -424,  2016,  -424,  1670,   311,
    1670,  -424,  -424,  -424,  -424,    11,   482,   484,   486,  -424,
     493,   494,   481,   497,  1448,  -424,   862,  -424,  -424,  -424,
    -424,  -424,  -424,  -424,  -424,   498,    24,   862,   -18,   500,
    -424,   400,   502,  1670,   633,   512,  -424,   313,  1670,  1300,
      77,    97,  1670,    84,   393,   400,  -424,  -424,   513,  1670,
     515,   478,    26,  -424,  -424,  -424,   517,  1934,  -424,  1965,
     329,   339,   862,  1670,  1670,  1670,  1010,  1084,   237,  -424,
    -424,  -424,   115,  -424,  -424,   447,   520,   510,  -424,  -424,
    -424,  1300,  -424,  2016,  -424,  -424,  -424,  2016,   519,  -424,
    -424,   116,   479,  -424,  1670,  -424,   144,  1300,  -424,  -424,
     549,  -424,  -424,  -424,  -424,  -424,  -424,   254,   521,  1670,
    -424,   169,  -424,  -424,   340,   344,   356,   564,   522,  -424,
     525,    24,   582,  -424,  -424,  -424,   529,   571,  -424,  -424,
     593,  -424,    24,   341,   541,  -424,  -424,  1670,  -424,   159,
    1799,   182,  -424,   542,  1670,   540,  -424,  1934,  -424,   550,
    1010,  -424,  -424,   545,  1522,   447,  1670,   237,  1670,  1670,
    -424,    69,   552,    24,   936,  -424,  -424,   213,  1809,  -424,
    -424,  -424,  -424,   420,  -424,   557,  1670,   606,  1010,  1670,
     561,  -424,  2016,  -424,  1985,  2016,  -424,    75,   627,   629,
     636,    75,    -7,   579,  1047,  -424,   574,   936,   575,   936,
    -424,  -424,   578,  -424,  2016,  1010,  -424,  -424,   357,  1596,
    -424,   576,  -424,  -424,  -424,   516,    19,  -424,   164,   164,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   164,
    -424,  -424,  -424,  -424,  -424,  -424,   581,   583,  1010,  -424,
    -424,  -424,   577,   164,   360,  1343,   402,  1129,  1263,  1337,
     402,   433,   433,  -424,  -424,  -424,  -424,  -424,  -424,  -424,
    1010,  -424
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       9,     0,   174,     1,   174,    10,    12,    15,   164,   190,
     177,    14,    16,   175,    11,     8,   181,   191,     0,     0,
     220,   182,     0,   185,     0,   203,   173,   176,     0,   178,
       0,   162,   165,   192,   193,   222,    13,   190,   253,     0,
     255,     0,   181,   183,   184,     0,   179,   183,   184,   190,
       0,     0,     0,   221,     0,   254,     0,   202,   190,   163,
       0,   167,     0,     0,   258,   259,     0,   224,    91,    92,
       0,     0,     0,     0,     0,     5,     0,   138,   243,     0,
       5,     0,    93,     0,   244,     0,     0,     0,     0,     0,
     223,     0,    83,    90,    84,    85,    96,    97,    86,    98,
       0,     0,     0,     0,     0,     0,     0,   249,   250,    94,
     245,    87,    88,   181,   222,   256,   257,     0,   166,   190,
     169,     0,     0,     0,   124,     0,     0,     0,    77,    89,
     199,    91,     0,    55,   225,     0,     0,    81,     0,     0,
       0,   124,     0,     0,     4,   140,    80,    78,    79,   182,
       0,     0,     0,     0,   190,     6,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    82,     0,     0,     0,     0,     0,     0,    76,
       0,     0,     0,   181,     0,     0,    57,   251,   252,   195,
     194,   187,   190,   211,     0,   205,   207,   186,   168,    59,
       0,   247,   246,   200,   227,   229,   232,     0,   197,   196,
       0,     0,     0,     0,     0,     0,   241,   136,   137,     0,
     139,     0,     0,     0,     0,     0,     0,     0,   234,   237,
       0,    95,     0,     0,    63,    62,    61,    70,    67,    68,
       0,    64,    65,    66,    69,    71,    72,    73,    74,    75,
      58,   100,     0,   102,     0,   101,     0,     0,     0,   201,
       0,     0,     0,     0,   117,   188,   209,    18,   206,   181,
       0,   228,     0,   129,     0,   119,    56,   121,     0,     0,
       0,   171,   171,   126,   141,     0,     0,     0,     0,     2,
       0,     0,     0,     0,     0,    32,     0,    30,    24,    25,
      26,    27,    28,    29,    31,     0,   190,     0,     0,     0,
      50,     0,     0,     0,     0,     0,   189,     0,     0,     0,
       0,     0,     0,     0,     0,   105,   125,   130,     0,     0,
       0,   212,     0,   208,   248,   230,     0,   239,   242,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   181,    33,
      34,    35,     0,    47,    45,     0,     0,   214,    48,    23,
      53,     0,   123,   226,   235,   122,    99,    60,     0,   103,
     104,     0,     0,   106,     0,   111,     0,     0,   116,   118,
       0,   210,    22,    21,    19,    20,   120,     0,     0,     0,
     127,     0,   128,    49,     0,     0,     0,     0,   154,   155,
       0,   190,     0,   156,   157,   159,   160,     0,   158,    36,
     216,    51,   190,   181,     0,   132,   113,     0,   112,     0,
      55,     0,   107,     0,     0,   239,   134,   239,   172,     0,
       0,   142,     2,     0,     0,     0,     0,   181,     0,     0,
     215,     0,     0,   190,     0,    52,   133,     0,    55,   108,
     115,   110,   131,     0,   240,     0,     0,    38,     0,     0,
       0,   218,   219,   161,     0,   217,   273,     0,     0,     0,
       0,     0,     0,   152,   151,   150,     0,     0,     0,     0,
     109,   114,     0,   135,   170,     0,    37,    40,     0,     0,
       3,     0,   274,   275,   276,     0,     0,   146,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      54,   144,    44,   143,   213,    39,     0,     0,     0,   149,
     272,   148,     0,     0,   266,   263,   264,   260,   261,   262,
     265,   267,   268,   269,   270,   271,    41,     2,    43,   147,
       0,    42
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -424,  -423,  -424,  -424,   573,  -424,  -424,  -424,  -424,  -424,
    -424,  -424,  -424,  -424,  -325,  -424,  -424,  -424,  -424,  -424,
    -424,  -424,  -424,   -81,  -267,  -424,  -424,  -424,  -424,   -69,
      40,   -65,   -64,   -57,  -424,  -424,  -424,  -424,  -424,  -424,
    -424,  -424,  -424,  -206,   242,  -346,  -419,  -424,   219,  -424,
    -424,  -424,  -424,  -424,   538,  -424,  -424,   376,  -424,   664,
    -424,    -5,  -424,  -424,  -335,  -424,    -6,    -8,  -258,   596,
    -424,  -424,  -424,   487,  -424,   404,  -424,  -302,  -424,   324,
    -424,   637,  -424,  -424,  -424,   366,  -225,   -40,   -89,  -424,
    -424,   -98,   333,   490,   580,   586,   415,  -424,   -22,   -21,
    -424,  -424,     4
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,   346,   518,   224,   136,   233,     1,     2,    15,     5,
       6,    51,   332,   384,   296,   297,   298,   486,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   443,   413,   308,
     133,    91,    92,   125,    93,    94,    95,    96,    97,   138,
      98,    99,   309,   478,   479,   472,   399,   400,   406,   407,
       7,     8,    16,    49,    60,    61,   428,   340,     9,    10,
      26,   191,    28,    29,   192,   100,   355,   101,   193,   102,
     103,    11,    12,   194,   195,   196,   381,   356,   357,   408,
     403,    19,    20,   104,   105,   228,   205,   206,   207,   229,
     230,   388,   217,   106,   107,   108,   202,   109,   110,   473,
     111,   112,   474
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      40,    41,   145,    18,   132,    27,   128,   129,   331,   458,
      35,    32,   401,   401,    50,   460,    45,    40,    54,   153,
     496,   397,    38,   466,   142,   268,   121,   382,     3,   353,
    -192,    52,    33,    34,    40,   116,    38,    58,    78,    56,
     358,   186,   129,    62,   114,   181,   182,   335,   115,    84,
      -7,   359,   117,   222,   120,   216,   212,   216,   153,   216,
       4,   124,   226,     4,   130,   324,    33,    34,   497,   -17,
     517,   269,    38,   466,   223,   393,  -193,   181,    38,   466,
     342,   225,   468,   154,    14,    21,   469,   470,    17,   471,
      90,    21,   260,   261,   521,   383,   467,   410,   144,   252,
     123,    36,   401,    39,   335,   457,   122,  -198,   208,   258,
     440,   372,   212,    62,   540,   203,   137,   325,   209,   477,
      22,   491,   143,    23,   144,   146,   147,   148,    30,    23,
    -181,  -224,   468,   487,    24,    21,   469,   470,   468,   471,
     179,   476,   469,   470,   263,   471,   279,    37,   232,    31,
     522,   212,   477,    42,   477,    56,   369,    25,   212,    46,
     515,   199,    57,   373,   317,   204,   251,   253,   466,   255,
     274,   212,   259,    23,  -192,   214,   370,   461,    30,   219,
      33,    34,   204,   320,   409,   321,   266,  -180,   323,   212,
     212,   204,   227,   538,   319,   416,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   215,   250,   144,   541,   -89,   -89,   212,   310,
    -193,   204,   204,   422,   204,   352,    21,   468,   189,   190,
     368,   469,   470,   212,   523,   169,   170,   171,   449,   174,
     270,   174,   175,   176,   175,   176,    21,   378,   404,   405,
     204,   385,   276,   371,   361,   376,   212,    38,    55,  -192,
      66,   451,    59,    67,    23,    68,    69,    70,   377,    30,
     159,   511,   414,   513,   394,   395,   396,   113,   398,   177,
     178,   177,   178,    21,    23,   189,   190,   212,   423,    30,
     124,   310,   480,    72,   210,   211,    73,   159,    75,   161,
      77,   231,   310,   419,   212,   421,   119,    81,    82,    83,
      43,    44,   204,   167,   168,   169,   170,   171,   337,   118,
     339,    23,   425,   129,   127,    38,    30,   454,    66,   455,
      89,   156,   157,   158,   159,   160,   161,   310,   121,   166,
     167,   168,   169,   170,   171,  -204,   284,   123,   447,   212,
      21,   172,   173,   363,   227,   453,    47,    48,   367,   204,
     167,   168,   169,   170,   171,   398,   441,   442,   311,   204,
     126,   212,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   338,   429,   366,   212,   135,   212,    23,   278,
     488,   184,   185,    30,    63,   435,    38,    64,    65,    66,
     390,   204,    67,   391,    68,    69,    70,    71,   445,   180,
     392,   430,   420,   391,   212,   431,   139,   204,   212,    40,
     398,   498,   505,   506,   507,   508,   509,   432,   516,   427,
     212,   212,    72,   212,   213,    73,    74,    75,    76,    77,
     281,   282,    78,    79,    80,    40,    81,    82,    83,    40,
      54,   329,   330,    84,   374,   141,    85,   448,   189,   190,
      86,    87,   155,    88,   505,   506,   507,   508,   509,    89,
     124,   218,   375,   220,    40,   495,   462,   183,   464,   465,
      63,    50,    38,    64,    65,    66,   329,   336,    67,   197,
      68,    69,    70,    71,   212,   482,   484,   507,   508,   509,
     221,   254,   524,   525,   526,   527,   528,   529,   530,   531,
     532,   533,   534,   535,   264,   267,   272,   273,    72,   314,
     275,    73,    74,    75,    76,    77,   277,   495,    78,    79,
      80,   283,    81,    82,    83,   498,   499,   500,   312,    84,
     417,   315,    85,   316,   322,   326,    86,    87,   327,    88,
     349,   328,   343,   380,   344,    89,   345,    63,   418,    38,
      64,    65,    66,   347,   348,    67,   350,    68,    69,    70,
      71,   354,   360,   362,   501,   502,   503,   504,   505,   506,
     507,   508,   509,   365,   412,   144,   379,   520,   386,   411,
     415,   424,   426,   433,   434,    72,   212,   436,    73,    74,
      75,    76,    77,   437,   438,    78,    79,    80,   439,    81,
      82,    83,   446,   452,   387,   459,    84,   156,   157,    85,
     159,   160,   161,    86,    87,   456,    88,   475,   483,   485,
     489,   492,    89,   493,    63,  -236,    38,    64,    65,    66,
     494,    56,    67,   510,    68,    69,    70,    71,   512,   514,
     536,   519,   539,   140,   537,   444,   463,   198,   341,   163,
     164,   165,   166,   167,   168,   169,   170,   171,    13,   134,
     262,   402,    72,   333,    53,    73,    74,    75,    76,    77,
     364,   265,    78,    79,    80,   334,    81,    82,    83,   187,
       0,     0,     0,    84,   156,   188,    85,   159,   160,   161,
      86,    87,     0,    88,     0,     0,     0,     0,     0,    89,
       0,    63,  -238,    38,    64,    65,    66,     0,     0,   149,
       0,    68,    69,    70,    71,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   163,   164,   165,   166,
     167,   168,   169,   170,   171,     0,     0,     0,     0,    72,
       0,     0,    73,    74,    75,    76,    77,    23,  -181,    78,
      79,    80,    30,    81,    82,    83,   159,   160,   161,     0,
      84,     0,     0,    85,   150,     0,     0,    86,    87,     0,
      88,     0,   151,     0,     0,     0,    89,    63,   152,    38,
      64,    65,    66,     0,     0,    67,     0,    68,    69,    70,
      71,     0,     0,     0,     0,   163,   164,   165,   166,   167,
     168,   169,   170,   171,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    72,     0,     0,    73,    74,
      75,    76,    77,     0,     0,    78,    79,    80,     0,    81,
      82,    83,     0,     0,     0,     0,    84,     0,     0,    85,
     150,     0,     0,    86,    87,     0,    88,     0,   151,     0,
       0,     0,    89,   285,   152,    38,    64,    65,    66,     0,
       0,   149,     0,    68,    69,    70,    71,     0,     0,     0,
       0,     0,     0,     0,   286,     0,   287,     0,     0,     0,
       0,   288,   289,   290,   291,     0,   292,   293,   294,     0,
       0,    72,     0,     0,    73,    74,    75,    76,    77,    23,
    -181,    78,    79,    80,    30,    81,    82,    83,     0,     0,
       0,     0,    84,     0,     0,    85,     0,     0,     0,    86,
      87,   295,    88,     0,   144,   -46,     0,    63,    89,    38,
      64,    65,    66,     0,     0,    67,     0,    68,    69,    70,
      71,     0,     0,     0,     0,     0,     0,     0,   286,     0,
     287,   441,   442,     0,     0,   288,   289,   290,   291,     0,
     292,   293,   294,     0,     0,    72,     0,     0,    73,    74,
      75,    76,    77,     0,     0,    78,    79,    80,     0,    81,
      82,    83,     0,     0,     0,     0,    84,     0,     0,    85,
       0,     0,     0,    86,    87,   295,    88,     0,   144,  -145,
       0,    63,    89,    38,    64,    65,    66,     0,     0,    67,
       0,    68,    69,    70,    71,     0,     0,     0,     0,     0,
       0,     0,   286,     0,   287,     0,     0,     0,     0,   288,
     289,   290,   291,     0,   292,   293,   294,     0,     0,    72,
       0,     0,    73,    74,    75,    76,    77,     0,     0,    78,
      79,    80,     0,    81,    82,    83,   498,   499,   500,     0,
      84,     0,     0,    85,   159,   160,   161,    86,    87,   295,
      88,     0,   144,     0,     0,    63,    89,    38,    64,    65,
      66,     0,     0,   149,     0,    68,    69,    70,    71,     0,
       0,     0,     0,     0,     0,   501,   502,   503,   504,   505,
     506,   507,   508,   509,   164,   165,   166,   167,   168,   169,
     170,   171,     0,    72,     0,     0,    73,    74,    75,    76,
      77,    23,  -181,    78,    79,    80,    30,    81,    82,    83,
       0,     0,     0,     0,    84,     0,     0,    85,   498,   499,
     500,    86,    87,  -153,    88,     0,     0,     0,     0,    63,
      89,    38,    64,    65,    66,     0,     0,    67,     0,    68,
      69,    70,    71,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   256,     0,     0,   502,   503,
     504,   505,   506,   507,   508,   509,     0,    72,     0,     0,
      73,    74,    75,    76,    77,     0,     0,    78,    79,    80,
       0,    81,    82,    83,   159,   160,   161,     0,    84,   257,
       0,    85,     0,     0,     0,    86,    87,    63,    88,    38,
      64,    65,    66,     0,    89,    67,     0,    68,    69,    70,
      71,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   165,   166,   167,   168,   169,
     170,   171,     0,     0,     0,    72,     0,     0,    73,    74,
      75,    76,    77,     0,     0,    78,    79,    80,     0,    81,
      82,    83,   498,   499,   500,     0,    84,     0,     0,    85,
     159,   160,   161,    86,    87,     0,    88,  -233,     0,  -233,
       0,    63,    89,    38,    64,    65,    66,     0,     0,    67,
       0,    68,    69,    70,    71,     0,     0,     0,     0,     0,
       0,     0,     0,   503,   504,   505,   506,   507,   508,   509,
       0,     0,   166,   167,   168,   169,   170,   171,     0,    72,
       0,     0,    73,    74,    75,    76,    77,     0,     0,    78,
      79,    80,     0,    81,    82,    83,   498,   499,   500,     0,
      84,     0,   498,    85,   500,     0,     0,    86,    87,     0,
      88,  -231,     0,     0,     0,    63,    89,    38,    64,    65,
      66,     0,     0,    67,     0,    68,    69,    70,    71,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   504,   505,
     506,   507,   508,   509,   504,   505,   506,   507,   508,   509,
       0,     0,     0,    72,     0,     0,    73,    74,    75,    76,
      77,     0,     0,    78,    79,    80,     0,    81,    82,    83,
       0,     0,     0,     0,    84,     0,     0,    85,     0,     0,
       0,    86,    87,     0,    88,     0,     0,  -231,     0,    63,
      89,    38,    64,    65,    66,     0,     0,    67,     0,    68,
      69,    70,    71,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    72,     0,     0,
      73,    74,    75,    76,    77,     0,     0,    78,    79,    80,
       0,    81,    82,    83,     0,     0,     0,     0,    84,     0,
       0,    85,     0,     0,     0,    86,    87,   351,    88,     0,
       0,     0,     0,    63,    89,    38,    64,    65,    66,     0,
       0,    67,     0,    68,    69,    70,    71,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    72,     0,     0,    73,    74,    75,    76,    77,     0,
       0,    78,    79,    80,     0,    81,    82,    83,     0,     0,
       0,     0,    84,     0,     0,    85,     0,     0,     0,    86,
      87,  -153,    88,     0,     0,     0,     0,    63,    89,    38,
      64,    65,    66,     0,     0,    67,     0,    68,    69,    70,
      71,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    72,     0,     0,    73,    74,
      75,    76,    77,     0,     0,    78,    79,    80,     0,    81,
      82,    83,     0,     0,     0,     0,    84,     0,     0,    85,
       0,     0,     0,    86,    87,     0,    88,  -153,     0,     0,
       0,    63,    89,    38,    64,    65,    66,     0,     0,    67,
       0,    68,    69,    70,    71,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    72,
       0,     0,    73,    74,    75,    76,    77,     0,     0,    78,
      79,    80,     0,    81,    82,    83,     0,     0,     0,     0,
      84,     0,     0,    85,     0,     0,     0,    86,    87,    63,
      88,    38,    64,    65,    66,     0,    89,    67,     0,   131,
      69,    70,    71,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    72,     0,     0,
      73,    74,    75,    76,    77,     0,     0,    78,    79,    80,
       0,    81,    82,    83,     0,     0,     0,     0,    84,     0,
       0,    85,     0,     0,     0,    86,    87,     0,    88,     0,
       0,     0,     0,     0,    89,   156,   157,   158,   159,   160,
     161,     0,     0,     0,     0,   156,   157,   158,   159,   160,
     161,     0,     0,     0,     0,     0,     0,     0,     0,   200,
     201,     0,     0,     0,     0,     0,     0,     0,   156,   157,
     158,   159,   160,   161,     0,     0,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,     0,     0,   450,   156,
     157,   158,   159,   160,   161,     0,     0,     0,   481,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   156,
     157,   158,   159,   160,   161,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     156,   157,   158,   159,   160,   161,     0,     0,   313,     0,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     156,   157,   158,   159,   160,   161,     0,     0,   318,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   156,   157,   158,   159,   160,   161,     0,   280,     0,
       0,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   156,   157,   158,   159,   160,   161,     0,   387,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   156,   157,   158,   159,   160,   161,     0,   389,
       0,     0,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,     0,     0,     0,     0,   490,   156,   157,   158,
     159,   160,   161,     0,     0,     0,     0,     0,     0,   271,
       0,     0,     0,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171
};

static const yytype_int16 yycheck[] =
{
      22,    22,    83,     9,    73,    10,    71,    71,   266,   432,
      18,    16,   347,   348,    15,   434,    24,    39,    39,    88,
      27,   346,     3,     4,    81,    28,    15,     1,     0,   296,
      39,    37,    11,    12,    56,    56,     3,    42,    49,    62,
     307,   106,   106,    49,    52,   102,   103,   272,    71,    60,
       0,    69,    58,   142,    62,   136,    74,   138,   127,   140,
      10,    70,   151,    10,    72,    27,    11,    12,    75,    70,
     489,    74,     3,     4,   143,   342,    39,   134,     3,     4,
      69,   150,    63,    88,    69,     9,    67,    68,    64,    70,
      50,     9,   181,   182,    75,    69,    27,   355,    72,    78,
      39,    69,   437,    70,   329,   430,    66,    70,    53,   178,
     412,    27,    74,   119,   537,   123,    76,    79,   126,   444,
      38,   467,    70,    47,    72,    85,    86,    87,    52,    47,
      48,    70,    63,   458,    52,     9,    67,    68,    63,    70,
     100,   443,    67,    68,   184,    70,   215,    74,   154,    73,
     496,    74,   477,    61,   479,    62,    79,    75,    74,    48,
     485,   121,    69,    79,   233,   125,   174,   175,     4,   177,
     210,    74,   180,    47,    72,   135,    79,   435,    52,   139,
      11,    12,   142,   252,    69,   254,   192,    48,   257,    74,
      74,   151,   152,   518,   251,    79,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,    70,   173,    72,   540,    14,    15,    74,   224,
      72,   181,   182,    79,   184,   294,     9,    63,    11,    12,
     319,    67,    68,    74,    70,    64,    65,    66,    79,    37,
     200,    37,    40,    41,    40,    41,     9,   328,    11,    12,
     210,   332,   212,   322,   311,   324,    74,     3,     3,    39,
       6,    79,    72,     9,    47,    11,    12,    13,   325,    52,
      19,   477,   361,   479,   343,   344,   345,    70,   347,    77,
      78,    77,    78,     9,    47,    11,    12,    74,   377,    52,
      70,   296,    79,    39,    74,    75,    42,    19,    44,    21,
      46,    71,   307,   372,    74,   374,    74,    53,    54,    55,
      11,    12,   272,    62,    63,    64,    65,    66,   278,    69,
     280,    47,   387,   387,    70,     3,    52,   425,     6,   427,
      76,    16,    17,    18,    19,    20,    21,   342,    15,    61,
      62,    63,    64,    65,    66,    71,    71,    39,   417,    74,
       9,    14,    15,   313,   314,   424,    11,    12,   318,   319,
      62,    63,    64,    65,    66,   434,    25,    26,    71,   329,
      39,    74,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    71,   391,    71,    74,    70,    74,    47,    74,
     459,    74,    75,    52,     1,   401,     3,     4,     5,     6,
      71,   361,     9,    74,    11,    12,    13,    14,   413,    39,
      71,    71,   372,    74,    74,    71,    70,   377,    74,   441,
     489,    19,    62,    63,    64,    65,    66,    71,    71,   389,
      74,    74,    39,    74,    75,    42,    43,    44,    45,    46,
      11,    12,    49,    50,    51,   467,    53,    54,    55,   471,
     471,    74,    75,    60,    61,    70,    63,   417,    11,    12,
      67,    68,    70,    70,    62,    63,    64,    65,    66,    76,
      70,   138,    79,   140,   496,   471,   436,    70,   438,   439,
       1,    15,     3,     4,     5,     6,    74,    75,     9,    21,
      11,    12,    13,    14,    74,    75,   456,    64,    65,    66,
      52,    78,   498,   499,   500,   501,   502,   503,   504,   505,
     506,   507,   508,   509,    71,    71,    74,    71,    39,    74,
      71,    42,    43,    44,    45,    46,    71,   523,    49,    50,
      51,    71,    53,    54,    55,    19,    20,    21,    73,    60,
      61,    79,    63,    71,    61,    71,    67,    68,    71,    70,
      69,    71,    70,    75,    70,    76,    70,     1,    79,     3,
       4,     5,     6,    70,    70,     9,    69,    11,    12,    13,
      14,    73,    72,    71,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    71,    74,    72,    71,    71,    71,    69,
      71,    42,    71,    29,    69,    39,    74,    15,    42,    43,
      44,    45,    46,    74,    33,    49,    50,    51,    15,    53,
      54,    55,    71,    71,    74,    70,    60,    16,    17,    63,
      19,    20,    21,    67,    68,    75,    70,    75,    71,    23,
      69,     4,    76,     4,     1,    79,     3,     4,     5,     6,
       4,    62,     9,    69,    11,    12,    13,    14,    73,    71,
      69,    75,    75,    80,    71,   413,   437,   119,   282,    58,
      59,    60,    61,    62,    63,    64,    65,    66,     4,    73,
     183,   347,    39,   269,    37,    42,    43,    44,    45,    46,
     314,   191,    49,    50,    51,   270,    53,    54,    55,   109,
      -1,    -1,    -1,    60,    16,   109,    63,    19,    20,    21,
      67,    68,    -1,    70,    -1,    -1,    -1,    -1,    -1,    76,
      -1,     1,    79,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    -1,    -1,    -1,    39,
      -1,    -1,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    19,    20,    21,    -1,
      60,    -1,    -1,    63,    64,    -1,    -1,    67,    68,    -1,
      70,    -1,    72,    -1,    -1,    -1,    76,     1,    78,     3,
       4,     5,     6,    -1,    -1,     9,    -1,    11,    12,    13,
      14,    -1,    -1,    -1,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    39,    -1,    -1,    42,    43,
      44,    45,    46,    -1,    -1,    49,    50,    51,    -1,    53,
      54,    55,    -1,    -1,    -1,    -1,    60,    -1,    -1,    63,
      64,    -1,    -1,    67,    68,    -1,    70,    -1,    72,    -1,
      -1,    -1,    76,     1,    78,     3,     4,     5,     6,    -1,
      -1,     9,    -1,    11,    12,    13,    14,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    22,    -1,    24,    -1,    -1,    -1,
      -1,    29,    30,    31,    32,    -1,    34,    35,    36,    -1,
      -1,    39,    -1,    -1,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,
      68,    69,    70,    -1,    72,    73,    -1,     1,    76,     3,
       4,     5,     6,    -1,    -1,     9,    -1,    11,    12,    13,
      14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    -1,
      24,    25,    26,    -1,    -1,    29,    30,    31,    32,    -1,
      34,    35,    36,    -1,    -1,    39,    -1,    -1,    42,    43,
      44,    45,    46,    -1,    -1,    49,    50,    51,    -1,    53,
      54,    55,    -1,    -1,    -1,    -1,    60,    -1,    -1,    63,
      -1,    -1,    -1,    67,    68,    69,    70,    -1,    72,    73,
      -1,     1,    76,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    22,    -1,    24,    -1,    -1,    -1,    -1,    29,
      30,    31,    32,    -1,    34,    35,    36,    -1,    -1,    39,
      -1,    -1,    42,    43,    44,    45,    46,    -1,    -1,    49,
      50,    51,    -1,    53,    54,    55,    19,    20,    21,    -1,
      60,    -1,    -1,    63,    19,    20,    21,    67,    68,    69,
      70,    -1,    72,    -1,    -1,     1,    76,     3,     4,     5,
       6,    -1,    -1,     9,    -1,    11,    12,    13,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    39,    -1,    -1,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    60,    -1,    -1,    63,    19,    20,
      21,    67,    68,    69,    70,    -1,    -1,    -1,    -1,     1,
      76,     3,     4,     5,     6,    -1,    -1,     9,    -1,    11,
      12,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    39,    -1,    -1,
      42,    43,    44,    45,    46,    -1,    -1,    49,    50,    51,
      -1,    53,    54,    55,    19,    20,    21,    -1,    60,    61,
      -1,    63,    -1,    -1,    -1,    67,    68,     1,    70,     3,
       4,     5,     6,    -1,    76,     9,    -1,    11,    12,    13,
      14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,
      65,    66,    -1,    -1,    -1,    39,    -1,    -1,    42,    43,
      44,    45,    46,    -1,    -1,    49,    50,    51,    -1,    53,
      54,    55,    19,    20,    21,    -1,    60,    -1,    -1,    63,
      19,    20,    21,    67,    68,    -1,    70,    71,    -1,    73,
      -1,     1,    76,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    60,    61,    62,    63,    64,    65,    66,
      -1,    -1,    61,    62,    63,    64,    65,    66,    -1,    39,
      -1,    -1,    42,    43,    44,    45,    46,    -1,    -1,    49,
      50,    51,    -1,    53,    54,    55,    19,    20,    21,    -1,
      60,    -1,    19,    63,    21,    -1,    -1,    67,    68,    -1,
      70,    71,    -1,    -1,    -1,     1,    76,     3,     4,     5,
       6,    -1,    -1,     9,    -1,    11,    12,    13,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    61,    62,
      63,    64,    65,    66,    61,    62,    63,    64,    65,    66,
      -1,    -1,    -1,    39,    -1,    -1,    42,    43,    44,    45,
      46,    -1,    -1,    49,    50,    51,    -1,    53,    54,    55,
      -1,    -1,    -1,    -1,    60,    -1,    -1,    63,    -1,    -1,
      -1,    67,    68,    -1,    70,    -1,    -1,    73,    -1,     1,
      76,     3,     4,     5,     6,    -1,    -1,     9,    -1,    11,
      12,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,    -1,
      42,    43,    44,    45,    46,    -1,    -1,    49,    50,    51,
      -1,    53,    54,    55,    -1,    -1,    -1,    -1,    60,    -1,
      -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,    -1,
      -1,    -1,    -1,     1,    76,     3,     4,     5,     6,    -1,
      -1,     9,    -1,    11,    12,    13,    14,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    39,    -1,    -1,    42,    43,    44,    45,    46,    -1,
      -1,    49,    50,    51,    -1,    53,    54,    55,    -1,    -1,
      -1,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,
      68,    69,    70,    -1,    -1,    -1,    -1,     1,    76,     3,
       4,     5,     6,    -1,    -1,     9,    -1,    11,    12,    13,
      14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    39,    -1,    -1,    42,    43,
      44,    45,    46,    -1,    -1,    49,    50,    51,    -1,    53,
      54,    55,    -1,    -1,    -1,    -1,    60,    -1,    -1,    63,
      -1,    -1,    -1,    67,    68,    -1,    70,    71,    -1,    -1,
      -1,     1,    76,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,
      -1,    -1,    42,    43,    44,    45,    46,    -1,    -1,    49,
      50,    51,    -1,    53,    54,    55,    -1,    -1,    -1,    -1,
      60,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,     1,
      70,     3,     4,     5,     6,    -1,    76,     9,    -1,    11,
      12,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,    -1,
      42,    43,    44,    45,    46,    -1,    -1,    49,    50,    51,
      -1,    53,    54,    55,    -1,    -1,    -1,    -1,    60,    -1,
      -1,    63,    -1,    -1,    -1,    67,    68,    -1,    70,    -1,
      -1,    -1,    -1,    -1,    76,    16,    17,    18,    19,    20,
      21,    -1,    -1,    -1,    -1,    16,    17,    18,    19,    20,
      21,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     7,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    16,    17,
      18,    19,    20,    21,    -1,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    79,    16,
      17,    18,    19,    20,    21,    -1,    -1,    -1,    79,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    16,
      17,    18,    19,    20,    21,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      16,    17,    18,    19,    20,    21,    -1,    -1,    75,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      16,    17,    18,    19,    20,    21,    -1,    -1,    75,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    16,    17,    18,    19,    20,    21,    -1,    74,    -1,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    16,    17,    18,    19,    20,    21,    -1,    74,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    16,    17,    18,    19,    20,    21,    -1,    74,
      -1,    -1,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    -1,    -1,    -1,    71,    16,    17,    18,
      19,    20,    21,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      -1,    -1,    -1,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    86,    87,     0,    10,    89,    90,   130,   131,   138,
     139,   151,   152,   139,    69,    88,   132,    64,   146,   161,
     162,     9,    38,    47,    52,    75,   140,   141,   142,   143,
      52,    73,   141,    11,    12,   147,    69,    74,     3,    70,
     178,   179,    61,    11,    12,   147,    48,    11,    12,   133,
      15,    91,   146,   161,   179,     3,    62,    69,   141,    72,
     134,   135,   146,     1,     4,     5,     6,     9,    11,    12,
      13,    14,    39,    42,    43,    44,    45,    46,    49,    50,
      51,    53,    54,    55,    60,    63,    67,    68,    70,    76,
     110,   111,   112,   114,   115,   116,   117,   118,   120,   121,
     145,   147,   149,   150,   163,   164,   173,   174,   175,   177,
     178,   180,   181,    70,   147,    71,   179,   146,    69,    74,
     147,    15,   110,    39,    70,   113,    39,    70,   111,   112,
     147,    11,   109,   110,   149,    70,    84,   110,   119,    70,
      84,    70,   113,    70,    72,   103,   110,   110,   110,     9,
      64,    72,    78,   109,   141,    70,    16,    17,    18,    19,
      20,    21,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    14,    15,    37,    40,    41,    77,    78,   110,
      39,   113,   113,    70,    74,    75,   111,   174,   175,    11,
      12,   141,   144,   148,   153,   154,   155,    21,   134,   110,
       7,     8,   176,   147,   110,   166,   167,   168,    53,   147,
      74,    75,    74,    75,   110,    70,   103,   172,   172,   110,
     172,    52,   168,   109,    83,   109,   168,   110,   165,   169,
     170,    71,   146,    85,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   147,    78,   147,    78,   147,    27,    61,   109,   147,
     168,   168,   153,   167,    71,   173,   146,    71,    28,    74,
     110,    28,    74,    71,   167,    71,   110,    71,    74,   109,
      74,    11,    12,    71,    71,     1,    22,    24,    29,    30,
      31,    32,    34,    35,    36,    69,    94,    95,    96,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   109,   122,
     141,    71,    73,    75,    74,    79,    71,   109,    75,   113,
     109,   109,    61,   109,    27,    79,    71,    71,    71,    74,
      75,   148,    92,   155,   176,   166,    75,   110,    71,   110,
     137,   137,    69,    70,    70,    70,    81,    70,    70,    69,
      69,    69,   109,   104,    73,   146,   157,   158,   104,    69,
      72,   113,    71,   110,   165,    71,    71,   110,   168,    79,
      79,   109,    27,    79,    61,    79,   109,   113,   103,    71,
      75,   156,     1,    69,    93,   103,    71,    74,   171,    74,
      71,    74,    71,   104,   109,   109,   109,    94,   109,   126,
     127,   144,   159,   160,    11,    12,   128,   129,   159,    69,
     148,    69,    74,   108,   168,    71,    79,    61,    79,   109,
     110,   109,    79,   168,    42,   111,    71,   110,   136,   147,
      71,    71,    71,    29,    69,   146,    15,    74,    33,    15,
     157,    25,    26,   107,   124,   141,    71,   109,   110,    79,
      79,    79,    71,   109,   171,   171,    75,    94,    81,    70,
     126,   148,   110,   128,   110,   110,     4,    27,    63,    67,
      68,    70,   125,   179,   182,    75,   157,    94,   123,   124,
      79,    79,    75,    71,   110,    23,    97,    94,   109,    69,
      71,   125,     4,     4,     4,   182,    27,    75,    19,    20,
      21,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      69,   123,    73,   123,    71,    94,    71,   126,    82,    75,
      71,    75,   125,    70,   182,   182,   182,   182,   182,   182,
     182,   182,   182,   182,   182,   182,    69,    71,    94,    75,
      81,    94
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    80,    81,    82,    83,    84,    85,    86,    87,    87,
      88,    88,    89,    89,    89,    89,    89,    91,    92,    90,
      93,    93,    93,    94,    94,    94,    94,    94,    94,    94,
      94,    94,    94,    94,    94,    95,    95,    96,    97,    97,
      98,    99,   100,   101,   102,   103,   104,   104,   104,   104,
     105,   106,   107,   108,   108,   109,   109,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   111,
     112,   112,   112,   112,   112,   112,   112,   112,   112,   112,
     112,   112,   112,   112,   112,   112,   112,   112,   112,   112,
     112,   112,   112,   112,   112,   112,   112,   112,   112,   112,
     112,   112,   112,   112,   113,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   115,   116,   117,   118,   119,   120,
     121,   121,   122,   123,   123,   123,   124,   124,   124,   124,
     124,   125,   125,   126,   126,   127,   127,   128,   128,   128,
     129,   129,   130,   131,   132,   133,   132,   134,   134,   135,
     136,   137,   137,   138,   139,   139,   140,   140,   141,   141,
     142,   142,   143,   143,   143,   143,   143,   144,   144,   145,
     146,   146,   147,   147,   148,   148,   149,   149,   150,   150,
     150,   150,   151,   152,   153,   153,   153,   154,   154,   155,
     155,   155,   156,   156,   157,   157,   158,   158,   159,   160,
     161,   161,   162,   162,   163,   164,   165,   166,   166,   167,
     167,   168,   168,   168,   169,   169,   170,   170,   170,   171,
     171,   172,   172,   173,   173,   174,   175,   176,   176,   177,
     177,   177,   177,   178,   178,   179,   179,   179,   180,   181,
     182,   182,   182,   182,   182,   182,   182,   182,   182,   182,
     182,   182,   182,   182,   182,   182,   182
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
       1,     0,     1,     2,     2,     1,     5,     1,     2,     4,
       0,     1,     1,     1,     1,     1,     3,     3,     1,     2,
       3,     3,     4,     2,     0,     1,     2,     1,     3,     2,
       4,     1,     0,     5,     1,     3,     2,     4,     3,     3,
       1,     3,     2,     4,     1,     2,     3,     1,     2,     1,
       3,     0,     1,     2,     1,     3,     0,     1,     2,     0,
       3,     1,     3,     1,     1,     1,     3,     1,     3,     1,
       1,     2,     2,     1,     2,     1,     3,     3,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     2,     2,     2
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
#line 248 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { (yyval.number) = rule_loop_open(); }
#line 2671 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 3: /* foreach_start: %empty  */
#line 249 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                      { (yyval.number) = rule_foreach_open(); }
#line 2677 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 4: /* block_start: %empty  */
#line 250 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.number) = rule_block_open(); }
#line 2683 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 5: /* special_context_start: %empty  */
#line 251 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { (yyval.number) = rule_special_context_open(); }
#line 2689 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 6: /* dollar_start: %empty  */
#line 252 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                     { (yyval.contextp) = rule_dollar_open(); }
#line 2695 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 7: /* all: program  */
#line 261 "$REPO_ROOT$/src/compiler/internal/grammar.y"
           { rule_program((yyvsp[0].node)); }
#line 2701 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 8: /* program: program def opt_semicolon  */
#line 266 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_program_append(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 2707 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 9: /* program: %empty  */
#line 267 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.node) = 0; }
#line 2713 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 11: /* opt_semicolon: ';'  */
#line 273 "$REPO_ROOT$/src/compiler/internal/grammar.y"
         { rule_opt_semicolon(); }
#line 2719 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 13: /* def: type name_list ';'  */
#line 285 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_def_global_var((yyvsp[-2].number)); (yyval.node) = 0; }
#line 2725 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 17: /* @1: %empty  */
#line 311 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_func_type((yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].string)); }
#line 2731 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 18: /* @2: %empty  */
#line 313 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_func_proto((yyvsp[-3].number), (yyvsp[-5].number), (yyvsp[-4].string), &(yyvsp[-4].shared_string), (yyvsp[-1].argument)); }
#line 2737 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 19: /* function: type optional_star identifier @1 '(' argument ')' @2 block_or_semi  */
#line 315 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_func(&(yyval.node), (yyvsp[-5].number), (yyvsp[-7].number), (yyvsp[-6].shared_string), (yyvsp[-3].argument), &(yyvsp[-1].number), &(yyvsp[0].node)); }
#line 2743 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 20: /* block_or_semi: block  */
#line 320 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = rule_block_or_semi((yyvsp[0].decl).node); }
#line 2749 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 21: /* block_or_semi: ';'  */
#line 321 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2755 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 22: /* block_or_semi: error  */
#line 322 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2761 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 23: /* statement: comma_expr ';'  */
#line 332 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_expr((yyvsp[-1].node)); }
#line 2767 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 27: /* statement: stmt_for  */
#line 336 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2773 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 28: /* statement: stmt_foreach  */
#line 337 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2779 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 31: /* statement: block  */
#line 340 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2785 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 32: /* statement: ';'  */
#line 341 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = 0; }
#line 2791 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 33: /* statement: L_BREAK ';'  */
#line 342 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_break(); }
#line 2797 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 34: /* statement: L_CONTINUE ';'  */
#line 343 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_continue(); }
#line 2803 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 35: /* stmt_return: L_RETURN ';'  */
#line 349 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_return_void(&(yyval.node)); }
#line 2809 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 36: /* stmt_return: L_RETURN comma_expr ';'  */
#line 350 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_return_expr(&(yyval.node), (yyvsp[-1].node)); }
#line 2815 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 37: /* stmt_cond: L_IF '(' comma_expr ')' statement optional_else_part  */
#line 356 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_cond(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 2821 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 38: /* optional_else_part: %empty  */
#line 360 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                { (yyval.node) = 0; }
#line 2827 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 39: /* optional_else_part: L_ELSE statement  */
#line 361 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                 { (yyval.node) = (yyvsp[0].node); }
#line 2833 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 40: /* stmt_while: L_WHILE '(' comma_expr ')' loop_start statement  */
#line 367 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_while(&(yyval.node), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2839 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 41: /* stmt_do: L_DO loop_start statement L_WHILE '(' comma_expr ')' ';'  */
#line 373 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_do(&(yyval.node), (yyvsp[-5].node), (yyvsp[-2].node), (yyvsp[-6].number)); }
#line 2845 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 42: /* stmt_for: L_FOR '(' for_init ';' for_expr ';' for_expr ')' loop_start statement  */
#line 379 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for(&(yyval.decl), &(yyvsp[-7].decl), (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2851 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 43: /* stmt_foreach: L_FOREACH '(' foreach_vars L_IN expr ')' foreach_start statement  */
#line 385 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach(&(yyval.decl), &(yyvsp[-5].decl), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2857 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 44: /* stmt_switch: switch_header '{' local_declarations case switch_block '}'  */
#line 391 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_switch(&(yyval.node), (yyvsp[-5].switch_header).expr, &(yyvsp[-3].decl), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[-5].switch_header).saved_context, (yyvsp[-5].switch_header).saved_cases_size); }
#line 2863 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 45: /* block: '{' block_start statements '}'  */
#line 403 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block(&(yyval.decl), (yyvsp[-1].decl).node, (yyvsp[-2].number)); }
#line 2869 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 46: /* statements: %empty  */
#line 409 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_empty(&(yyval.decl)); }
#line 2875 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 47: /* statements: statement statements  */
#line 411 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_stmt(&(yyval.decl), (yyvsp[-1].node), &(yyvsp[0].decl)); }
#line 2881 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 48: /* statements: local_declaration_statement statements  */
#line 413 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_decl(&(yyval.decl), &(yyvsp[-1].decl), &(yyvsp[0].decl)); }
#line 2887 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 49: /* statements: error ';' statements  */
#line 415 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_error(&(yyval.decl), &(yyvsp[0].decl)); }
#line 2893 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 50: /* local_decl_statement_header: basic_type  */
#line 420 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { rule_local_declaration_statement_set_type((yyvsp[0].number)); (yyval.number) = (yyvsp[0].number); }
#line 2899 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 51: /* local_declaration_statement: local_decl_statement_header local_name_list ';'  */
#line 425 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_declaration_statement(&(yyval.decl), &(yyvsp[-1].decl)); }
#line 2905 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 52: /* local_decl_header: basic_type  */
#line 430 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { rule_local_declarations_set_type((yyvsp[0].number)); (yyval.number) = (yyvsp[0].number); }
#line 2911 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 53: /* local_declarations: %empty  */
#line 435 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.decl).node = 0; (yyval.decl).num = 0; }
#line 2917 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 54: /* local_declarations: local_declarations local_decl_header local_name_list ';'  */
#line 437 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_declarations(&(yyval.decl), &(yyvsp[-3].decl), &(yyvsp[-1].decl)); }
#line 2923 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 56: /* comma_expr: comma_expr ',' expr  */
#line 456 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                { rule_comma_expr(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2929 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 57: /* expr: ref lvalue  */
#line 460 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_ref(&(yyval.node), (yyvsp[0].node)); }
#line 2935 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 58: /* expr: lvalue L_ASSIGN expr  */
#line 461 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_assign(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 2941 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 59: /* expr: error L_ASSIGN expr  */
#line 462 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_assign_error(&(yyval.node), (yyvsp[0].node)); }
#line 2947 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 60: /* expr: expr '?' expr ':' expr  */
#line 464 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                            { rule_expr_ternary(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2953 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 61: /* expr: expr L_QUESTION_QUESTION expr  */
#line 465 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_nullish(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2959 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 62: /* expr: expr L_LOR expr  */
#line 466 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_lor(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2965 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 63: /* expr: expr L_LAND expr  */
#line 467 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_land(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2971 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 64: /* expr: expr '|' expr  */
#line 469 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_or(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2977 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 65: /* expr: expr '^' expr  */
#line 470 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_xor(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2983 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 66: /* expr: expr '&' expr  */
#line 471 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_and(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2989 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 67: /* expr: expr L_EQ_NE expr  */
#line 473 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_expr_eq_ne(&(yyval.node), (yyvsp[-1].number), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2995 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 68: /* expr: expr L_ORDER expr  */
#line 474 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_order(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3001 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 69: /* expr: expr '<' expr  */
#line 475 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_lt(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3007 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 70: /* expr: expr L_SHIFT expr  */
#line 477 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_expr_shift(&(yyval.node), (yyvsp[-1].number), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3013 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 71: /* expr: expr '+' expr  */
#line 479 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             {
        rule_set_operand_ranges((yylsp[-2]).first_line, (yylsp[-2]).first_column, (yylsp[-2]).last_column,
                                (yylsp[-1]).first_line, (yylsp[-1]).first_column,
                                (yylsp[0]).first_line, (yylsp[0]).first_column, (yylsp[0]).last_column);
        rule_expr_add(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node));
        rule_clear_operand_ranges();
    }
#line 3025 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 72: /* expr: expr '-' expr  */
#line 486 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_sub(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3031 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 73: /* expr: expr '*' expr  */
#line 487 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_mul(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3037 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 74: /* expr: expr '%' expr  */
#line 488 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_mod(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3043 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 75: /* expr: expr '/' expr  */
#line 489 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_div(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3049 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 76: /* expr: cast expr  */
#line 491 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_cast(&(yyval.node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3055 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 77: /* expr: L_INC_DEC lvalue  */
#line 492 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_expr_pre_incdec(&(yyval.node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3061 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 78: /* expr: '!' expr  */
#line 493 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_expr_not(&(yyval.node), (yyvsp[0].node)); }
#line 3067 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 79: /* expr: '~' expr  */
#line 494 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_expr_compl(&(yyval.node), (yyvsp[0].node)); }
#line 3073 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 80: /* expr: '-' expr  */
#line 495 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_neg(&(yyval.node), (yyvsp[0].node)); }
#line 3079 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 81: /* expr: L_AWAIT expr  */
#line 496 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_await(&(yyval.node), (yyvsp[0].node)); }
#line 3085 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 82: /* expr: lvalue L_INC_DEC  */
#line 498 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_expr_post_incdec(&(yyval.node), (yyvsp[0].number), (yyvsp[-1].node)); }
#line 3091 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 89: /* lvalue: primary_expr  */
#line 510 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                { (yyval.node) = rule_lvalue((yyvsp[0].node)); }
#line 3097 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 91: /* primary_expr: L_DEFINED_NAME  */
#line 520 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_defined_name(&(yyval.node), (yyvsp[0].ihe)); }
#line 3103 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 92: /* primary_expr: L_IDENTIFIER  */
#line 521 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_identifier(&(yyval.node), (yyvsp[0].string)); }
#line 3109 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 93: /* primary_expr: L_PARAMETER  */
#line 522 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_parameter(&(yyval.node), (yyvsp[0].number)); }
#line 3115 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 95: /* primary_expr: '(' comma_expr ')'  */
#line 524 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { (yyval.node) = (yyvsp[-1].node); }
#line 3121 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 99: /* primary_expr: '$' '(' dollar_start comma_expr ')'  */
#line 531 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_dollar_expr(&(yyval.node), (yyvsp[-2].contextp), (yyvsp[-1].node)); }
#line 3127 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 100: /* primary_expr: primary_expr L_ARROW identifier  */
#line 534 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                 { rule_primary_expr_member_arrow(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 3133 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 101: /* primary_expr: primary_expr '.' identifier  */
#line 535 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                 { rule_primary_expr_member_dot(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 3139 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 102: /* primary_expr: primary_expr L_OPTIONAL_DOT identifier  */
#line 541 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_member_optional(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 3145 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 103: /* primary_expr: primary_expr L_OPTIONAL_DOT '[' comma_expr ']'  */
#line 543 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_optional(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3151 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 104: /* primary_expr: primary_expr L_DOT_OPTIONAL '[' comma_expr ']'  */
#line 545 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_optional(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3157 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 105: /* primary_expr: primary_expr '[' comma_expr ']'  */
#line 550 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3163 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 106: /* primary_expr: primary_expr '[' '<' comma_expr ']'  */
#line 552 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_r(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3169 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 107: /* primary_expr: primary_expr '[' comma_expr L_RANGE comma_expr ']'  */
#line 554 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nn(&(yyval.node), (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3175 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 108: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE comma_expr ']'  */
#line 556 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rn(&(yyval.node), (yyvsp[-6].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3181 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 109: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE '<' comma_expr ']'  */
#line 558 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rr(&(yyval.node), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3187 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 110: /* primary_expr: primary_expr '[' comma_expr L_RANGE '<' comma_expr ']'  */
#line 560 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nr(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3193 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 111: /* primary_expr: primary_expr '[' comma_expr L_RANGE ']'  */
#line 562 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_ne(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node)); }
#line 3199 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 112: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE ']'  */
#line 564 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_re(&(yyval.node), (yyvsp[-5].node), (yyvsp[-2].node)); }
#line 3205 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 113: /* primary_expr: primary_expr '[' L_RANGE '<' comma_expr ']'  */
#line 566 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nr(&(yyval.node), (yyvsp[-5].node), 0, (yyvsp[-1].node)); }
#line 3211 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 114: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE '<' expr ']'  */
#line 569 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rr(&(yyval.node), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 3217 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 115: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE expr ']'  */
#line 571 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rn(&(yyval.node), (yyvsp[-6].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3223 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 116: /* primary_expr: lambda_return_type '(' argument ')' block  */
#line 576 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_anon_func(&(yyval.node), &(yyvsp[-4].func_block), &(yyvsp[-2].argument), &(yyvsp[0].decl)); }
#line 3229 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 117: /* primary_expr: functional_open ':' ')'  */
#line 583 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_1(&(yyval.node), (yyvsp[-2].number)); }
#line 3235 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 118: /* primary_expr: functional_open ',' arg_list ':' ')'  */
#line 585 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_2(&(yyval.node), (yyvsp[-4].number), (yyvsp[-2].node)); }
#line 3241 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 119: /* primary_expr: L_FUNCTION_OPEN L_DEFINED_NAME ':' ')'  */
#line 594 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_1(&(yyval.node), rule_functional_ref((yyvsp[-2].ihe))); }
#line 3247 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 120: /* primary_expr: L_FUNCTION_OPEN L_DEFINED_NAME ',' arg_list ':' ')'  */
#line 596 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_2(&(yyval.node), rule_functional_ref((yyvsp[-4].ihe)), (yyvsp[-2].node)); }
#line 3253 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 121: /* primary_expr: L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 598 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_3(&(yyval.node), (yyvsp[-2].node)); }
#line 3259 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 122: /* primary_expr: '(' '[' opt_pair_list ']' ')'  */
#line 601 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_primary_expr_mapping(&(yyval.node), (yyvsp[-2].node)); }
#line 3265 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 123: /* primary_expr: '(' '{' opt_arg_list '}' ')'  */
#line 602 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_primary_expr_array(&(yyval.node), (yyvsp[-2].node)); }
#line 3271 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 124: /* call_open: '('  */
#line 612 "$REPO_ROOT$/src/compiler/internal/grammar.y"
       { rule_call_open(&(yyval.call_open).saved_context, &(yyval.call_open).saved_refs); }
#line 3277 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 125: /* function_call: efun_override call_open opt_arg_list ')'  */
#line 626 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_efun(&(yyval.node), (yyvsp[-3].number), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3283 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 126: /* function_call: L_NEW call_open opt_arg_list ')'  */
#line 628 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new(&(yyval.node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3289 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 127: /* function_call: L_NEW '(' L_CLASS L_DEFINED_NAME opt_class_init ')'  */
#line 630 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new_class(&(yyval.node), (yyvsp[-2].ihe), (yyvsp[-1].node)); }
#line 3295 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 128: /* function_call: L_NEW '(' L_CLASS L_IDENTIFIER opt_class_init ')'  */
#line 632 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new_class_undef(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node)); }
#line 3301 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 129: /* function_call: L_DEFINED_NAME call_open opt_arg_list ')'  */
#line 634 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_defined_name(&(yyval.node), (yyvsp[-3].ihe), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3307 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 130: /* function_call: function_name call_open opt_arg_list ')'  */
#line 636 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_name(&(yyval.node), (yyvsp[-3].string), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3313 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 131: /* function_call: primary_expr '[' comma_expr ']' call_open opt_arg_list ')'  */
#line 638 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_indexed(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3319 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 132: /* function_call: primary_expr L_ARROW identifier call_open opt_arg_list ')'  */
#line 640 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_arrow(&(yyval.node), (yyvsp[-5].node), (yyvsp[-3].string), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3325 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 133: /* function_call: '(' '*' comma_expr ')' call_open opt_arg_list ')'  */
#line 642 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_star(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3331 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 134: /* sscanf: L_SSCANF '(' expr ',' expr lvalue_list ')'  */
#line 653 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_sscanf(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 3337 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 135: /* parse_command: L_PARSE_COMMAND '(' expr ',' expr ',' expr lvalue_list ')'  */
#line 659 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_parse_command(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 3343 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 136: /* catch: L_CATCH special_context_start expr_or_block  */
#line 665 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_catch(&(yyval.node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 3349 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 137: /* acatch: L_ACATCH acatch_context_start expr_or_block  */
#line 674 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_acatch(&(yyval.node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 3355 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 138: /* acatch_context_start: %empty  */
#line 677 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { (yyval.number) = rule_acatch_context_open(); }
#line 3361 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 139: /* time_expression: L_TIME_EXPRESSION special_context_start expr_or_block  */
#line 682 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_time_expression(&(yyval.node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 3367 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 140: /* tree: L_TREE block  */
#line 688 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_tree_block(&(yyval.node), (yyvsp[0].decl).node); }
#line 3373 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 141: /* tree: L_TREE '(' comma_expr ')'  */
#line 690 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_tree_expr(&(yyval.node), (yyvsp[-1].node)); }
#line 3379 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 142: /* switch_header: L_SWITCH '(' comma_expr ')'  */
#line 701 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.switch_header).expr = (yyvsp[-1].node); rule_switch_pre(&(yyval.switch_header).saved_context, &(yyval.switch_header).saved_cases_size); }
#line 3385 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 143: /* switch_block: case switch_block  */
#line 706 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_switch_block_case(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3391 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 144: /* switch_block: statement switch_block  */
#line 707 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_switch_block_stmt(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3397 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 145: /* switch_block: %empty  */
#line 708 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { rule_switch_block_empty(&(yyval.node)); }
#line 3403 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 146: /* case: L_CASE case_label ':'  */
#line 715 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_single(&(yyval.node), (yyvsp[-1].node)); }
#line 3409 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 147: /* case: L_CASE case_label L_RANGE case_label ':'  */
#line 717 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3415 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 148: /* case: L_CASE case_label L_RANGE ':'  */
#line 719 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range_from(&(yyval.node), (yyvsp[-2].node)); }
#line 3421 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 149: /* case: L_CASE L_RANGE case_label ':'  */
#line 721 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range_to(&(yyval.node), (yyvsp[-1].node)); }
#line 3427 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 150: /* case: L_DEFAULT ':'  */
#line 723 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_default(&(yyval.node)); }
#line 3433 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 151: /* case_label: constant  */
#line 729 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_case_label_constant(&(yyval.node), (yyvsp[0].number)); }
#line 3439 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 152: /* case_label: string_const  */
#line 730 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_case_label_string(&(yyval.node), (yyvsp[0].string)); }
#line 3445 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 153: /* for_expr: %empty  */
#line 740 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { (yyval.node) = 0; }
#line 3451 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 155: /* for_init: for_expr  */
#line 747 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for_init_expr(&(yyval.decl), (yyvsp[0].node)); }
#line 3457 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 156: /* for_init: single_new_local_def_with_init  */
#line 749 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for_init_local(&(yyval.decl), (yyvsp[0].node)); }
#line 3463 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 157: /* foreach_var: L_DEFINED_NAME  */
#line 754 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_defined(&(yyval.decl), (yyvsp[0].ihe)); }
#line 3469 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 158: /* foreach_var: single_new_local_def  */
#line 755 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_new_local(&(yyval.decl), (yyvsp[0].number)); }
#line 3475 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 159: /* foreach_var: L_IDENTIFIER  */
#line 756 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_identifier(&(yyval.decl), (yyvsp[0].string)); }
#line 3481 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 160: /* foreach_vars: foreach_var  */
#line 762 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach_vars_single(&(yyval.decl), &(yyvsp[0].decl)); }
#line 3487 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 161: /* foreach_vars: foreach_var ',' foreach_var  */
#line 764 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach_vars_double(&(yyval.decl), &(yyvsp[-2].decl), &(yyvsp[0].decl)); }
#line 3493 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 162: /* type_decl: class_header member_list '}'  */
#line 775 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_define_class_members((yyvsp[-2].class_header).ihe, (yyvsp[-2].class_header).classname_index); (yyval.node) = 0; }
#line 3499 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 163: /* class_header: type_modifier_list L_CLASS identifier '{'  */
#line 781 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.class_header).ihe = rule_define_class(&(yyval.class_header).classname_index, (yyvsp[-1].string)); }
#line 3505 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 165: /* $@3: %empty  */
#line 787 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                           { rule_member_list_set_type((yyvsp[0].number)); }
#line 3511 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 169: /* member_name: optional_star identifier  */
#line 799 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { rule_member_name((yyvsp[-1].number), (yyvsp[0].string)); }
#line 3517 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 170: /* class_init: identifier ':' expr  */
#line 804 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                       { (yyval.node) = rule_class_init((yyvsp[-2].string), (yyvsp[0].node)); }
#line 3523 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 171: /* opt_class_init: %empty  */
#line 809 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.node) = rule_opt_class_init_empty(); }
#line 3529 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 172: /* opt_class_init: opt_class_init ',' class_init  */
#line 810 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { (yyval.node) = rule_opt_class_init((yyvsp[-2].node), (yyvsp[0].node)); }
#line 3535 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 173: /* type: type_modifier_list opt_basic_type  */
#line 822 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { (yyval.number) = rule_type((yyvsp[-1].number), (yyvsp[0].number)); }
#line 3541 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 174: /* type_modifier_list: %empty  */
#line 827 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { (yyval.number) = 0; }
#line 3547 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 175: /* type_modifier_list: L_TYPE_MODIFIER type_modifier_list  */
#line 828 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                               { (yyval.number) = rule_type_modifier_list((yyvsp[-1].number), (yyvsp[0].number)); }
#line 3553 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 177: /* opt_basic_type: %empty  */
#line 834 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { (yyval.number) = rule_opt_basic_type_empty(); }
#line 3559 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 179: /* basic_type: opt_atomic_type L_ARRAY  */
#line 840 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { (yyval.number) = (yyvsp[-1].number) | TYPE_MOD_ARRAY; }
#line 3565 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 181: /* opt_atomic_type: %empty  */
#line 846 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { (yyval.number) = rule_opt_basic_type_empty(); }
#line 3571 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 183: /* atomic_type: L_CLASS L_DEFINED_NAME  */
#line 852 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.number) = rule_atomic_type_class((yyvsp[0].ihe)); }
#line 3577 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 184: /* atomic_type: L_CLASS L_IDENTIFIER  */
#line 853 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.number) = rule_atomic_type_class_identifier((yyvsp[0].string)); }
#line 3583 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 185: /* atomic_type: L_PROMISE  */
#line 857 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { (yyval.number) = rule_atomic_type_promise(); }
#line 3589 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 186: /* atomic_type: L_PROMISE '<' basic_type optional_star L_ORDER  */
#line 859 "$REPO_ROOT$/src/compiler/internal/grammar.y"
      { (yyval.number) = rule_atomic_type_promise_of((yyvsp[-2].number) | (yyvsp[-1].number), (yyvsp[0].number)); }
#line 3595 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 188: /* arg_type: basic_type ref  */
#line 865 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.number) = (yyvsp[-1].number) | LOCAL_MOD_REF; }
#line 3601 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 189: /* cast: '(' basic_type optional_star ')'  */
#line 870 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { (yyval.number) = rule_cast((yyvsp[-2].number), (yyvsp[-1].number)); }
#line 3607 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 190: /* optional_star: %empty  */
#line 875 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.number) = 0; }
#line 3613 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 191: /* optional_star: '*'  */
#line 876 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.number) = TYPE_MOD_ARRAY; }
#line 3619 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 192: /* identifier: L_DEFINED_NAME  */
#line 886 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { (yyval.string) = rule_identifier_defined_name((yyvsp[0].ihe)); }
#line 3625 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 195: /* new_local_name: L_DEFINED_NAME  */
#line 893 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.string) = rule_new_local_name_redefine((yyvsp[0].ihe)); }
#line 3631 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 196: /* efun_override: L_EFUN L_COLON_COLON identifier  */
#line 898 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.number) = rule_efun_override((yyvsp[0].string)); }
#line 3637 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 197: /* efun_override: L_EFUN L_COLON_COLON L_NEW  */
#line 899 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.number) = rule_efun_override_new(); }
#line 3643 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 199: /* function_name: L_COLON_COLON identifier  */
#line 910 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { (yyval.string) = rule_function_name_colon_colon((yyvsp[0].string)); }
#line 3649 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 200: /* function_name: L_BASIC_TYPE L_COLON_COLON identifier  */
#line 911 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { (yyval.string) = rule_function_name_type((yyvsp[-2].number), (yyvsp[0].string)); }
#line 3655 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 201: /* function_name: identifier L_COLON_COLON identifier  */
#line 912 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                    { (yyval.string) = rule_function_name_obj((yyvsp[-2].string), (yyvsp[0].string)); }
#line 3661 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 202: /* inheritance: type_modifier_list L_INHERIT string_const ';'  */
#line 918 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { if (rule_inheritence(&(yyval.node), (yyvsp[-3].number), (yyvsp[-1].string))) { YYACCEPT; } }
#line 3667 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 203: /* modifier_change: type_modifier_list ':'  */
#line 923 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { (yyval.node) = rule_modifier_change((yyvsp[-1].number)); }
#line 3673 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 204: /* argument: %empty  */
#line 934 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.argument).num_arg = 0; (yyval.argument).flags = 0; }
#line 3679 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 206: /* argument: argument_list L_DOT_DOT_DOT  */
#line 937 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_varargs(&(yyval.argument), &(yyvsp[-1].argument)); }
#line 3685 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 207: /* argument_list: param_decl  */
#line 943 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_list_single(&(yyval.argument), (yyvsp[0].number)); }
#line 3691 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 208: /* argument_list: argument_list ',' param_decl  */
#line 945 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_list_multi(&(yyval.argument), &(yyvsp[-2].argument), (yyvsp[0].number)); }
#line 3697 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 209: /* param_decl: arg_type optional_star  */
#line 955 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_typed((yyvsp[-1].number) | (yyvsp[0].number)); }
#line 3703 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 210: /* param_decl: arg_type optional_star new_local_name optional_default_arg_value  */
#line 957 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_typed_name((yyvsp[-3].number) | (yyvsp[-2].number), (yyvsp[-1].string), (yyvsp[0].node)); }
#line 3709 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 211: /* param_decl: new_local_name  */
#line 959 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_untyped_name((yyvsp[0].string)); }
#line 3715 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 212: /* optional_default_arg_value: %empty  */
#line 964 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                          { (yyval.node) = 0; }
#line 3721 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 213: /* optional_default_arg_value: ':' L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 965 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                            { (yyval.node) = rule_default_arg_value((yyvsp[-2].node)); }
#line 3727 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 214: /* local_name_list: new_local_def  */
#line 971 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_name_list_single(&(yyval.decl), (yyvsp[0].node)); }
#line 3733 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 215: /* local_name_list: new_local_def ',' local_name_list  */
#line 973 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_name_list_multi(&(yyval.decl), (yyvsp[-2].node), &(yyvsp[0].decl)); }
#line 3739 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 216: /* new_local_def: optional_star new_local_name  */
#line 979 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_new_local_def((yyvsp[0].string), (yyvsp[-1].number)); }
#line 3745 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 217: /* new_local_def: optional_star new_local_name L_ASSIGN expr  */
#line 981 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_new_local_def_with_init((yyvsp[-2].string), (yyvsp[-3].number), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3751 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 218: /* single_new_local_def: arg_type optional_star new_local_name  */
#line 987 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_single_new_local_def(&(yyval.number), (yyvsp[-2].number) | (yyvsp[-1].number), (yyvsp[0].string)); }
#line 3757 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 219: /* single_new_local_def_with_init: single_new_local_def L_ASSIGN expr  */
#line 993 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_single_new_local_def_with_init((yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3763 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 222: /* new_name: optional_star identifier  */
#line 1009 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { rule_new_name((yyvsp[-1].number), (yyvsp[0].string)); }
#line 3769 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 223: /* new_name: optional_star identifier L_ASSIGN expr  */
#line 1010 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { rule_new_name_with_init((yyvsp[-3].number), (yyvsp[-2].string), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3775 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 224: /* lambda_return_type: L_BASIC_TYPE  */
#line 1021 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                { rule_lambda_return_type(&(yyval.func_block), (yyvsp[0].number)); }
#line 3781 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 225: /* functional_open: L_FUNCTION_OPEN efun_override  */
#line 1029 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                 { (yyval.number) = rule_functional_open((yyvsp[0].number)); }
#line 3787 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 226: /* assoc_pair: expr ':' expr  */
#line 1039 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_assoc_pair(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3793 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 227: /* spread_expr: expr  */
#line 1044 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_spread_expr_normal(&(yyval.node), (yyvsp[0].node)); }
#line 3799 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 228: /* spread_expr: expr L_DOT_DOT_DOT  */
#line 1045 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_spread_expr_dots(&(yyval.node), (yyvsp[-1].node)); }
#line 3805 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 229: /* arg_list: spread_expr  */
#line 1051 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_arg_list_single(&(yyval.node), (yyvsp[0].node)); }
#line 3811 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 230: /* arg_list: arg_list ',' spread_expr  */
#line 1053 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_arg_list_multi(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3817 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 231: /* opt_arg_list: %empty  */
#line 1059 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list_empty(&(yyval.node)); }
#line 3823 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 232: /* opt_arg_list: arg_list  */
#line 1060 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list(&(yyval.node), (yyvsp[0].node)); }
#line 3829 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 233: /* opt_arg_list: arg_list ','  */
#line 1061 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list(&(yyval.node), (yyvsp[-1].node)); }
#line 3835 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 234: /* pair_list: assoc_pair  */
#line 1067 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_pair_list_single(&(yyval.node), (yyvsp[0].node)); }
#line 3841 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 235: /* pair_list: pair_list ',' assoc_pair  */
#line 1069 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_pair_list_multi(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3847 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 236: /* opt_pair_list: %empty  */
#line 1075 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list_empty(&(yyval.node)); }
#line 3853 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 237: /* opt_pair_list: pair_list  */
#line 1076 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list(&(yyval.node), (yyvsp[0].node)); }
#line 3859 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 238: /* opt_pair_list: pair_list ','  */
#line 1077 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list(&(yyval.node), (yyvsp[-1].node)); }
#line 3865 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 239: /* lvalue_list: %empty  */
#line 1082 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.node) = rule_lvalue_list_empty(); }
#line 3871 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 240: /* lvalue_list: ',' lvalue lvalue_list  */
#line 1083 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { (yyval.node) = rule_lvalue_list((yyvsp[-1].node), (yyvsp[0].node)); }
#line 3877 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 241: /* expr_or_block: block  */
#line 1088 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_expr_or_block_block((yyvsp[0].decl)); }
#line 3883 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 242: /* expr_or_block: '(' comma_expr ')'  */
#line 1089 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_expr_or_block_expr((yyvsp[-1].node)); }
#line 3889 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 245: /* string: string_literal  */
#line 1105 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_string(&(yyval.node), (yyvsp[0].string)); }
#line 3895 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 246: /* template_literal: L_TEMPLATE_HEAD expr template_parts  */
#line 1117 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_template_literal(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3901 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 247: /* template_parts: L_TEMPLATE_TAIL  */
#line 1121 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                      { rule_template_parts_tail(&(yyval.node), (yyvsp[0].string)); }
#line 3907 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 248: /* template_parts: L_TEMPLATE_MIDDLE expr template_parts  */
#line 1122 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                       { rule_template_parts_middle(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3913 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 251: /* string_like: string_like string  */
#line 1133 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                               { rule_string_like_concat(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3919 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 252: /* string_like: string_like template_literal  */
#line 1134 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_string_like_concat(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3925 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 254: /* string_literal: string_literal L_STRING  */
#line 1142 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.string) = rule_string_literal_concat((yyvsp[-1].string), (yyvsp[0].string)); }
#line 3931 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 256: /* string_const: '(' string_const ')'  */
#line 1149 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                        { (yyval.string) = (yyvsp[-1].string); }
#line 3937 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 257: /* string_const: string_const '+' string_const  */
#line 1150 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { (yyval.string) = rule_string_literal_concat((yyvsp[-2].string), (yyvsp[0].string)); }
#line 3943 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 258: /* number: L_NUMBER  */
#line 1154 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { rule_number(&(yyval.node), (yyvsp[0].number)); }
#line 3949 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 259: /* real: L_REAL  */
#line 1158 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { rule_real(&(yyval.node), (yyvsp[0].real)); }
#line 3955 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 260: /* constant: constant '|' constant  */
#line 1164 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_or(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3961 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 261: /* constant: constant '^' constant  */
#line 1165 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_xor(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3967 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 262: /* constant: constant '&' constant  */
#line 1166 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_and(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3973 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 263: /* constant: constant L_EQ_NE constant  */
#line 1167 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                             { rule_constant_eq_ne(&(yyval.number), (yyvsp[-1].number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3979 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 264: /* constant: constant L_ORDER constant  */
#line 1168 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { rule_constant_order(&(yyval.number), (yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].number)); }
#line 3985 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 265: /* constant: constant '<' constant  */
#line 1169 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_lt(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3991 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 266: /* constant: constant L_SHIFT constant  */
#line 1170 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                             { rule_constant_shift(&(yyval.number), (yyvsp[-1].number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3997 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 267: /* constant: constant '+' constant  */
#line 1171 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_add(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 4003 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 268: /* constant: constant '-' constant  */
#line 1172 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_sub(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 4009 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 269: /* constant: constant '*' constant  */
#line 1173 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_mul(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 4015 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 270: /* constant: constant '%' constant  */
#line 1174 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_mod(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 4021 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 271: /* constant: constant '/' constant  */
#line 1175 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_div(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 4027 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 272: /* constant: '(' constant ')'  */
#line 1176 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { (yyval.number) = (yyvsp[-1].number); }
#line 4033 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 273: /* constant: L_NUMBER  */
#line 1177 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { (yyval.number) = (yyvsp[0].number); }
#line 4039 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 274: /* constant: '-' L_NUMBER  */
#line 1178 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_neg(&(yyval.number), (yyvsp[0].number)); }
#line 4045 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 275: /* constant: '!' L_NUMBER  */
#line 1179 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_not(&(yyval.number), (yyvsp[0].number)); }
#line 4051 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;

  case 276: /* constant: '~' L_NUMBER  */
#line 1180 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_compl(&(yyval.number), (yyvsp[0].number)); }
#line 4057 "$BUILD_ROOT$/src/grammar.autogen.cc"
    break;


#line 4061 "$BUILD_ROOT$/src/grammar.autogen.cc"

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
#line 1183 "$REPO_ROOT$/src/compiler/internal/grammar.y"


// Public accessor for the parser's symbol-name table: maps a raw yylex
// token number (what lpcc --tokens prints) to its grammar spelling
// ("L_IDENTIFIER", "'{'"). Lives in the epilogue because yysymbol_name()
// and YYTRANSLATE are file-static in the generated parser; consumed by
// the lpcc --json staged outputs (stage_output.cc).
const char* lpc_token_name(int token) {
  return yysymbol_name(YY_CAST(yysymbol_kind_t, YYTRANSLATE(token)));
}
/* FluffOS generated-from grammar.y sha256=20b45d6abb52bd9c7193b0b7ff8f2799c556ca60daa7b14d1b0b304eaf864e38 */
