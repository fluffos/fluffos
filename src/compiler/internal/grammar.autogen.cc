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
#line 11 "$REPO_ROOT$/src/compiler/internal/grammar.y"

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

int yyparse(void);

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
  YYSYMBOL_L_PREPROCESSOR_COMMAND = 54,    /* L_PREPROCESSOR_COMMAND  */
  YYSYMBOL_LOWER_THAN_ELSE = 55,           /* LOWER_THAN_ELSE  */
  YYSYMBOL_56_ = 56,                       /* '?'  */
  YYSYMBOL_57_ = 57,                       /* '|'  */
  YYSYMBOL_58_ = 58,                       /* '^'  */
  YYSYMBOL_59_ = 59,                       /* '&'  */
  YYSYMBOL_L_EQ = 60,                      /* L_EQ  */
  YYSYMBOL_L_NE = 61,                      /* L_NE  */
  YYSYMBOL_62_ = 62,                       /* '<'  */
  YYSYMBOL_63_ = 63,                       /* '+'  */
  YYSYMBOL_64_ = 64,                       /* '-'  */
  YYSYMBOL_65_ = 65,                       /* '*'  */
  YYSYMBOL_66_ = 66,                       /* '%'  */
  YYSYMBOL_67_ = 67,                       /* '/'  */
  YYSYMBOL_68_ = 68,                       /* '~'  */
  YYSYMBOL_69_ = 69,                       /* ';'  */
  YYSYMBOL_70_ = 70,                       /* '('  */
  YYSYMBOL_71_ = 71,                       /* ')'  */
  YYSYMBOL_72_ = 72,                       /* '{'  */
  YYSYMBOL_73_ = 73,                       /* '}'  */
  YYSYMBOL_74_ = 74,                       /* ','  */
  YYSYMBOL_75_ = 75,                       /* ':'  */
  YYSYMBOL_76_ = 76,                       /* '$'  */
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
  YYSYMBOL_time_expression = 117,          /* time_expression  */
  YYSYMBOL_tree = 118,                     /* tree  */
  YYSYMBOL_switch_header = 119,            /* switch_header  */
  YYSYMBOL_switch_block = 120,             /* switch_block  */
  YYSYMBOL_case = 121,                     /* case  */
  YYSYMBOL_case_label = 122,               /* case_label  */
  YYSYMBOL_for_expr = 123,                 /* for_expr  */
  YYSYMBOL_for_init = 124,                 /* for_init  */
  YYSYMBOL_foreach_var = 125,              /* foreach_var  */
  YYSYMBOL_foreach_vars = 126,             /* foreach_vars  */
  YYSYMBOL_type_decl = 127,                /* type_decl  */
  YYSYMBOL_class_header = 128,             /* class_header  */
  YYSYMBOL_member_list = 129,              /* member_list  */
  YYSYMBOL_130_3 = 130,                    /* $@3  */
  YYSYMBOL_member_name_list = 131,         /* member_name_list  */
  YYSYMBOL_member_name = 132,              /* member_name  */
  YYSYMBOL_class_init = 133,               /* class_init  */
  YYSYMBOL_opt_class_init = 134,           /* opt_class_init  */
  YYSYMBOL_type = 135,                     /* type  */
  YYSYMBOL_type_modifier_list = 136,       /* type_modifier_list  */
  YYSYMBOL_opt_basic_type = 137,           /* opt_basic_type  */
  YYSYMBOL_basic_type = 138,               /* basic_type  */
  YYSYMBOL_opt_atomic_type = 139,          /* opt_atomic_type  */
  YYSYMBOL_atomic_type = 140,              /* atomic_type  */
  YYSYMBOL_arg_type = 141,                 /* arg_type  */
  YYSYMBOL_cast = 142,                     /* cast  */
  YYSYMBOL_optional_star = 143,            /* optional_star  */
  YYSYMBOL_identifier = 144,               /* identifier  */
  YYSYMBOL_new_local_name = 145,           /* new_local_name  */
  YYSYMBOL_efun_override = 146,            /* efun_override  */
  YYSYMBOL_function_name = 147,            /* function_name  */
  YYSYMBOL_inheritance = 148,              /* inheritance  */
  YYSYMBOL_modifier_change = 149,          /* modifier_change  */
  YYSYMBOL_argument = 150,                 /* argument  */
  YYSYMBOL_argument_list = 151,            /* argument_list  */
  YYSYMBOL_param_decl = 152,               /* param_decl  */
  YYSYMBOL_optional_default_arg_value = 153, /* optional_default_arg_value  */
  YYSYMBOL_local_name_list = 154,          /* local_name_list  */
  YYSYMBOL_new_local_def = 155,            /* new_local_def  */
  YYSYMBOL_single_new_local_def = 156,     /* single_new_local_def  */
  YYSYMBOL_single_new_local_def_with_init = 157, /* single_new_local_def_with_init  */
  YYSYMBOL_name_list = 158,                /* name_list  */
  YYSYMBOL_new_name = 159,                 /* new_name  */
  YYSYMBOL_lambda_return_type = 160,       /* lambda_return_type  */
  YYSYMBOL_functional_open = 161,          /* functional_open  */
  YYSYMBOL_assoc_pair = 162,               /* assoc_pair  */
  YYSYMBOL_spread_expr = 163,              /* spread_expr  */
  YYSYMBOL_arg_list = 164,                 /* arg_list  */
  YYSYMBOL_opt_arg_list = 165,             /* opt_arg_list  */
  YYSYMBOL_pair_list = 166,                /* pair_list  */
  YYSYMBOL_opt_pair_list = 167,            /* opt_pair_list  */
  YYSYMBOL_lvalue_list = 168,              /* lvalue_list  */
  YYSYMBOL_expr_or_block = 169,            /* expr_or_block  */
  YYSYMBOL_ref = 170,                      /* ref  */
  YYSYMBOL_string = 171,                   /* string  */
  YYSYMBOL_string_literal = 172,           /* string_literal  */
  YYSYMBOL_string_const = 173,             /* string_const  */
  YYSYMBOL_number = 174,                   /* number  */
  YYSYMBOL_real = 175,                     /* real  */
  YYSYMBOL_constant = 176                  /* constant  */
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
#define YYLAST   1899

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  79
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  98
/* YYNRULES -- Number of rules.  */
#define YYNRULES  264
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  515

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   312


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
       2,     2,     2,     2,     2,     2,    76,    66,    59,     2,
      70,    71,    65,    63,    74,    64,     2,    67,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    75,    69,
      62,     2,     2,    56,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    77,     2,    78,    58,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    72,    57,    73,    68,     2,     2,     2,
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
      55,    60,    61
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   206,   206,   207,   208,   209,   210,   219,   224,   225,
     230,   231,   242,   243,   244,   245,   246,   263,   265,   262,
     272,   273,   274,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   301,   302,   307,   312,   313,
     318,   324,   330,   336,   342,   354,   360,   362,   364,   366,
     372,   376,   382,   386,   388,   407,   408,   412,   413,   414,
     416,   417,   418,   419,   421,   422,   423,   425,   426,   427,
     428,   430,   431,   433,   434,   435,   436,   437,   439,   440,
     441,   442,   443,   444,   446,   447,   449,   450,   451,   452,
     453,   454,   459,   468,   469,   470,   471,   472,   473,   474,
     475,   478,   482,   483,   487,   489,   491,   493,   495,   497,
     499,   501,   503,   506,   508,   513,   520,   522,   524,   528,
     529,   539,   552,   554,   556,   558,   560,   562,   564,   566,
     568,   579,   585,   591,   597,   603,   605,   616,   622,   623,
     624,   630,   632,   634,   636,   638,   645,   646,   656,   657,
     662,   664,   670,   671,   672,   677,   679,   690,   696,   702,
     703,   703,   709,   710,   715,   720,   725,   726,   738,   743,
     744,   749,   750,   755,   756,   761,   762,   767,   768,   769,
     774,   775,   780,   785,   786,   796,   797,   802,   803,   808,
     809,   819,   820,   821,   822,   827,   833,   843,   845,   846,
     852,   854,   864,   866,   868,   874,   875,   880,   882,   888,
     890,   896,   902,   913,   914,   919,   920,   931,   939,   940,
     950,   955,   956,   961,   963,   970,   971,   972,   977,   979,
     986,   987,   988,   993,   994,   999,  1000,  1005,  1015,  1022,
    1023,  1029,  1030,  1031,  1035,  1039,  1045,  1046,  1047,  1048,
    1049,  1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,
    1059,  1060,  1061,  1062,  1063
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
  "L_PARAMETER", "L_TREE", "L_PREPROCESSOR_COMMAND", "LOWER_THAN_ELSE",
  "'?'", "'|'", "'^'", "'&'", "L_EQ", "L_NE", "'<'", "'+'", "'-'", "'*'",
  "'%'", "'/'", "'~'", "';'", "'('", "')'", "'{'", "'}'", "','", "':'",
  "'$'", "'['", "']'", "$accept", "loop_start", "foreach_start",
  "block_start", "special_context_start", "dollar_start", "all", "program",
  "opt_semicolon", "def", "function", "@1", "@2", "block_or_semi",
  "statement", "stmt_return", "stmt_cond", "optional_else_part",
  "stmt_while", "stmt_do", "stmt_for", "stmt_foreach", "stmt_switch",
  "block", "statements", "local_decl_statement_header",
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
  "expr_or_block", "ref", "string", "string_literal", "string_const",
  "number", "real", "constant", YY_NULLPTR
  };
  return yy_sname[yysymbol];
}
#endif

#define YYPACT_NINF (-404)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-233)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -404,    36,   195,  -404,    40,   -12,  -404,  -404,  -404,    41,
      57,  -404,  -404,  -404,  -404,  -404,    93,  -404,   252,    22,
      56,  -404,     9,   292,  -404,  -404,  -404,    94,   119,   370,
    -404,  -404,  -404,  -404,     2,  -404,    41,  -404,     9,   176,
     -18,   120,   141,   162,  -404,  -404,  -404,    41,  1654,   197,
     252,  -404,   114,  -404,     9,  -404,  -404,   199,   214,   252,
     294,  -404,  -404,   290,   -15,    30,   295,   296,   296,  1654,
     252,  1210,   540,  1654,  -404,   275,  -404,  -404,   282,  -404,
     300,  -404,    -6,  1654,  1654,   914,   306,  1176,   193,   233,
    -404,  -404,  -404,  -404,  -404,  -404,  1654,   323,   310,   310,
     315,   167,   296,  -404,   176,  -404,  -404,    81,   352,  -404,
    -404,  -404,    41,  -404,  1654,   252,  -404,  1284,   189,  1358,
    -404,   132,  -404,  -404,  -404,   954,  -404,   316,   322,  1694,
    -404,   339,   336,   200,  1176,   310,  1654,   224,  1654,   224,
     366,  1284,  1654,  -404,  -404,  -404,  -404,   106,  1654,    75,
      41,  -404,  1654,  1654,  1654,  1654,  1654,  1654,  1654,  1654,
    1654,  1654,  1654,  1654,  1654,  1654,  1654,  1654,  1654,  1654,
    -404,  -404,  1654,   252,   252,  1062,  -404,   252,  1284,  1284,
      81,  1654,   346,  -404,  -404,  -404,   378,    41,  -404,   355,
      31,  -404,  -404,  1176,  -404,   363,  -404,  -404,  -404,  1136,
     367,  1654,   616,   371,  1654,   380,  1748,  1654,  -404,  -404,
    1771,  -404,   374,   382,   191,   692,   192,  -404,   383,  1654,
    1322,  1250,  1032,   254,   254,   338,  1717,   219,   688,  1428,
     836,   836,   338,   293,   293,  -404,  -404,  -404,  1176,   310,
    -404,   393,  1654,   112,  -404,   386,   389,   390,   332,  -404,
    -404,   401,  -404,  -404,   166,  -404,  -404,  -404,  1176,  -404,
    -404,  1176,  -404,  1654,   218,  1654,  -404,  -404,  -404,  -404,
      33,   392,   407,   408,  -404,   409,   410,   394,   402,  1432,
    -404,   692,  -404,  -404,  -404,  -404,  -404,  -404,  -404,  -404,
     412,    41,   692,   138,   387,  -404,   310,  -404,   237,  1654,
    1284,  1654,   113,   388,   310,  -404,  -404,   411,  1654,   427,
     406,    29,  -404,  1802,  -404,  1825,   238,   257,   692,  1654,
    1654,  1654,   840,   988,   175,  -404,  -404,  -404,   155,  -404,
    -404,   401,   413,   425,  -404,  -404,  -404,  1284,  -404,  1176,
     429,    85,   464,  -404,  1654,  -404,   115,  1284,  -404,  -404,
     460,  -404,  -404,  -404,  -404,  -404,   296,   439,  1654,  -404,
     252,  -404,  -404,   259,   269,   279,   486,   444,  -404,   450,
      41,   507,  -404,  -404,  -404,   447,   490,  -404,  -404,   510,
    -404,    41,   266,   453,  -404,  -404,  1654,  -404,   153,   584,
     179,  -404,   454,  1654,   455,  -404,  1802,  -404,   452,   840,
    -404,  -404,   461,  1506,   401,  1654,   175,  1654,  1654,  -404,
     147,   458,    41,   766,  -404,  -404,   180,  1104,  -404,  -404,
    -404,  -404,   337,  -404,   459,  1654,   513,   840,  1654,   468,
    -404,  1176,  -404,   430,  1176,  -404,   534,   150,   535,   543,
     150,     3,   491,  1396,  -404,   484,   766,   482,   766,  -404,
    -404,   485,  -404,  1176,   840,  -404,  -404,   280,  1580,  -404,
    -404,   483,  -404,  -404,   878,    18,  -404,   158,   158,   158,
     158,   158,   158,   158,   158,   158,   158,   158,   158,   158,
     158,  -404,  -404,  -404,  -404,  -404,  -404,   488,   506,   840,
    -404,  -404,  -404,   487,   158,   260,   260,   357,   612,   762,
    1502,  1280,  1280,   357,   302,   302,  -404,  -404,  -404,  -404,
    -404,  -404,  -404,   840,  -404
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       9,     0,   169,     1,   169,    10,    12,    15,   159,   183,
     172,    14,    16,   170,    11,     8,   176,   184,     0,     0,
     213,   177,     0,     0,   196,   168,   171,     0,   173,     0,
     157,   160,   185,   186,   215,    13,   183,   239,     0,   241,
       0,   178,   179,     0,   174,   178,   179,   183,     0,     0,
       0,   214,     0,   240,     0,   195,   158,     0,   162,     0,
       0,   244,   245,   217,    94,    95,     0,     0,     0,     0,
       0,     0,     0,     0,   218,     0,     5,   237,     0,     5,
       0,    96,     0,     0,     0,     0,     0,   216,     0,    86,
      93,    87,    88,    99,    89,   100,     0,     0,     0,     0,
       0,     0,     0,    97,   238,    90,    91,   176,   215,   242,
     243,   161,   183,   164,     0,     0,   121,     0,     0,     0,
      79,    92,    80,    81,   192,   221,   223,   226,     0,     0,
     228,   231,     0,     0,    55,   219,     0,     0,     0,     0,
     121,     0,     0,     4,   135,    83,    82,   177,     0,     0,
     183,     6,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      84,    85,     0,     0,     0,     0,    78,     0,     0,     0,
     176,     0,     0,    57,   188,   187,   180,   183,   204,     0,
     198,   200,   163,    59,   193,     0,   190,   189,   222,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   235,   133,
       0,   134,     0,     0,     0,     0,     0,    98,     0,     0,
      63,    62,    61,    71,    72,    69,     0,    64,    65,    66,
      67,    68,    70,    73,    74,    75,    76,    77,    58,   102,
     103,     0,     0,     0,   194,     0,     0,     0,     0,   116,
     181,   202,    18,   199,   176,   126,   224,   120,   220,   229,
     119,    56,   118,     0,     0,     0,   166,   166,   123,   136,
       0,     0,     0,     0,     2,     0,     0,     0,     0,     0,
      32,     0,    30,    24,    25,    26,    27,    28,    29,    31,
       0,   183,     0,     0,     0,    50,     0,   182,     0,     0,
       0,     0,     0,     0,   104,   122,   127,     0,     0,     0,
     205,     0,   201,   233,   236,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   176,    33,    34,    35,     0,    47,
      45,     0,     0,   207,    48,    23,    53,     0,   101,    60,
       0,     0,     0,   105,     0,   110,     0,     0,   115,   117,
       0,   203,    22,    21,    19,    20,     0,     0,     0,   124,
       0,   125,    49,     0,     0,     0,     0,   149,   150,     0,
     183,     0,   151,   152,   154,   155,     0,   153,    36,   209,
      51,   183,   176,     0,   129,   112,     0,   111,     0,    55,
       0,   106,     0,     0,   233,   131,   233,   167,     0,     0,
     137,     2,     0,     0,     0,     0,   176,     0,     0,   208,
       0,     0,   183,     0,    52,   130,     0,    55,   107,   114,
     109,   128,     0,   234,     0,     0,    38,     0,     0,     0,
     211,   212,   156,     0,   210,   261,     0,     0,     0,     0,
       0,     0,   147,   146,   145,     0,     0,     0,     0,   108,
     113,     0,   132,   165,     0,    37,    40,     0,     0,     3,
     263,     0,   262,   264,     0,     0,   141,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    54,   139,    44,   138,   206,    39,     0,     0,     0,
     144,   260,   143,     0,     0,   253,   254,   251,   246,   247,
     248,   249,   250,   252,   255,   256,   257,   258,   259,    41,
       2,    43,   142,     0,    42
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -404,  -383,  -404,  -404,   480,  -404,  -404,  -404,  -404,  -404,
    -404,  -404,  -404,  -404,  -266,  -404,  -404,  -404,  -404,  -404,
    -404,  -404,  -404,   -72,  -250,  -404,  -404,  -404,  -404,   -71,
     -44,   -61,   -59,   -45,  -404,  -404,  -404,  -404,  -404,  -404,
    -404,  -102,   196,  -403,  -377,  -404,   188,  -404,  -404,  -404,
    -404,  -404,   449,  -404,  -404,   319,  -404,   591,  -404,    -5,
    -404,  -404,  -264,  -404,    14,    26,  -248,   523,  -404,  -404,
    -404,   417,  -404,   351,  -404,  -328,  -404,   283,  -404,   571,
    -404,  -404,  -404,   421,  -179,   428,  -104,  -404,  -404,   -23,
     472,   426,  -404,   -22,   -21,  -404,  -404,    96
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,   322,   489,   215,   137,   219,     1,     2,    15,     5,
       6,    49,   311,   354,   281,   282,   283,   455,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   412,   382,   293,
     134,    88,    89,   178,    90,    91,    92,    93,    94,    95,
     294,   447,   448,   441,   368,   369,   375,   376,     7,     8,
      16,    47,    57,    58,   397,   316,     9,    10,    25,   186,
      27,    28,   187,    96,   331,    97,   188,    98,    99,    11,
      12,   189,   190,   191,   351,   332,   333,   377,   372,    19,
      20,   100,   101,   130,   126,   127,   128,   131,   132,   357,
     209,   102,   103,   104,   442,   105,   106,   443
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      39,    40,   133,   310,    87,    26,   120,   122,   121,   121,
     144,    31,    37,   195,   149,    48,    39,    52,   427,   117,
     256,    37,   435,    18,  -185,   123,   429,   125,   129,   465,
     352,   329,    39,   110,   461,   141,     3,   213,   436,   145,
     146,   183,   334,   121,    34,    54,   114,     4,   149,    43,
      50,    55,   176,   409,   179,   116,   366,    14,   253,   370,
     370,    59,   493,    21,   142,   208,   143,   208,   362,  -186,
     193,   214,   -17,   125,   245,   246,   108,   216,   466,    38,
     150,   488,   438,   379,   445,   113,   439,    21,   440,   184,
     185,    35,   206,   492,   210,    22,   124,   125,   353,    21,
    -191,   143,   318,  -176,   243,   254,    17,    23,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,    59,   513,   238,   256,
      36,    29,    24,   426,   125,   125,   264,   125,   303,   342,
      44,   194,   370,    29,   197,   115,   217,   446,   298,   204,
      37,   435,  -197,    37,   435,   125,   430,   258,   129,   204,
     261,   456,   435,   385,   218,  -175,    30,   436,   173,   174,
     436,   302,    21,   437,   184,   185,  -217,    54,   436,    53,
     446,    21,   446,   373,   374,   109,   204,   204,   486,   204,
     304,   343,  -185,   391,   300,    -7,   340,    32,    33,   239,
     240,   251,     4,   244,   170,   171,   172,   335,   328,   175,
     295,   438,   204,  -186,   438,   439,    29,   440,   439,   313,
     440,   315,   438,   511,   378,    29,   439,   204,   494,   204,
     341,   418,   346,   383,    56,   348,   155,   156,   157,   355,
     196,   181,   182,   392,   -92,   -92,   -92,   514,   363,   364,
     365,   337,   367,   204,   204,   339,   125,   420,   449,   347,
      32,    33,   269,   296,   125,   204,   204,   107,   111,   173,
     174,   388,    21,   390,   204,   205,   295,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   295,   112,   314,
     410,   411,   204,   125,   207,   394,   143,   121,   389,    37,
      41,    42,    63,   125,    64,    65,    66,   114,   338,   359,
     175,   204,   360,   295,   396,   416,    29,   165,   166,   167,
     168,   169,   422,   476,   477,   478,   479,   480,   361,   115,
     399,   360,   367,   204,   118,    70,    71,    72,    73,    74,
     400,    76,   417,   204,   482,   136,   484,    80,    81,    82,
     401,   487,   138,   204,   204,   155,   156,   457,   167,   168,
     169,   431,   177,   433,   434,    48,   119,   478,   479,   480,
     140,   423,    86,   424,   467,   468,   151,   414,    45,    46,
     116,   453,   266,   267,   404,   180,   398,   367,    39,    60,
     199,    37,    61,    62,    63,   200,    64,    65,    66,    67,
      68,   165,   166,   167,   168,   169,   308,   309,    69,   184,
     185,   204,   451,   202,   203,    39,   212,   249,    39,    52,
     476,   477,   478,   479,   480,    77,   252,    70,    71,    72,
      73,    74,    75,    76,   255,    77,    78,    79,   257,    80,
      81,    82,   260,    39,   152,   153,   154,   155,   156,   157,
     344,   262,    83,   268,   297,   301,    84,   305,    85,   336,
     306,   307,   319,   325,    86,    60,   345,    37,    61,    62,
      63,   326,    64,    65,    66,    67,    68,   320,   321,   323,
     324,   350,   380,   143,    69,   330,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   349,   381,
     384,   459,   393,    70,    71,    72,    73,    74,    75,    76,
     395,    77,    78,    79,   402,    80,    81,    82,   204,   403,
     405,   406,   407,   408,   415,   421,   386,   425,    83,   356,
     452,   428,    84,   444,    85,   454,   464,   458,   460,   462,
      86,    60,   387,    37,    61,    62,    63,   463,    64,    65,
      66,    67,    68,   481,    54,   483,   485,   509,   490,   139,
      69,   192,   512,   495,   496,   497,   498,   499,   500,   501,
     502,   503,   504,   505,   506,   507,   508,   510,   413,    70,
      71,    72,    73,    74,    75,    76,   317,    77,    78,    79,
     464,    80,    81,    82,   432,    13,   135,   247,   152,   153,
     154,   155,   156,   157,    83,   312,   371,    51,    84,   248,
      85,   211,   250,     0,     0,     0,    86,    60,  -230,    37,
      61,    62,    63,   259,    64,    65,    66,    67,    68,   467,
     468,   469,     0,     0,     0,     0,    69,     0,     0,     0,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,     0,     0,     0,    70,    71,    72,    73,    74,
      75,    76,   419,    77,    78,    79,     0,    80,    81,    82,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
      83,     0,     0,     0,    84,     0,    85,     0,     0,     0,
       0,     0,    86,   270,  -232,    37,    61,    62,   147,     0,
      64,    65,    66,    67,    68,   155,   156,   157,     0,     0,
       0,     0,    69,   271,     0,   272,     0,     0,     0,     0,
     273,   274,   275,   276,     0,   277,   278,   279,     0,     0,
       0,    70,    71,    72,    73,    74,    75,    76,  -176,    77,
      78,    79,    29,    80,    81,    82,     0,   161,   162,   163,
     164,   165,   166,   167,   168,   169,    83,     0,     0,     0,
      84,   280,    85,     0,   143,   -46,     0,    60,    86,    37,
      61,    62,    63,     0,    64,    65,    66,    67,    68,   467,
     468,   469,     0,     0,     0,     0,    69,   271,     0,   272,
     410,   411,     0,     0,   273,   274,   275,   276,     0,   277,
     278,   279,     0,     0,     0,    70,    71,    72,    73,    74,
      75,    76,     0,    77,    78,    79,     0,    80,    81,    82,
       0,   472,   473,   474,   475,   476,   477,   478,   479,   480,
      83,     0,     0,     0,    84,   280,    85,     0,   143,  -140,
       0,    60,    86,    37,    61,    62,    63,     0,    64,    65,
      66,    67,    68,   155,   156,   157,     0,     0,     0,     0,
      69,   271,     0,   272,     0,     0,     0,     0,   273,   274,
     275,   276,     0,   277,   278,   279,     0,     0,     0,    70,
      71,    72,    73,    74,    75,    76,     0,    77,    78,    79,
       0,    80,    81,    82,     0,   467,   468,   469,   164,   165,
     166,   167,   168,   169,    83,     0,     0,     0,    84,   280,
      85,     0,   143,     0,     0,    60,    86,    37,    61,    62,
     147,     0,    64,    65,    66,    67,    68,     0,     0,     0,
       0,     0,     0,     0,    69,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,     0,     0,     0,   491,
       0,     0,     0,    70,    71,    72,    73,    74,    75,    76,
    -176,    77,    78,    79,    29,    80,    81,    82,   152,   153,
     154,   155,   156,   157,     0,     0,     0,     0,    83,   148,
       0,   198,    84,     0,    85,     0,     0,     0,     0,    60,
      86,    37,    61,    62,   147,     0,    64,    65,    66,    67,
      68,     0,     0,     0,     0,     0,     0,     0,    69,     0,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,     0,     0,     0,     0,     0,    70,    71,    72,
      73,    74,    75,    76,  -176,    77,    78,    79,    29,    80,
      81,    82,     0,     0,     0,     0,   152,   153,     0,   155,
     156,   157,    83,     0,     0,     0,    84,  -148,    85,     0,
       0,     0,     0,    60,    86,    37,    61,    62,    63,     0,
      64,    65,    66,    67,    68,     0,     0,     0,     0,     0,
       0,     0,    69,     0,     0,     0,     0,     0,   241,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
       0,    70,    71,    72,    73,    74,    75,    76,     0,    77,
      78,    79,     0,    80,    81,    82,     0,     0,   152,   153,
     154,   155,   156,   157,   242,     0,    83,     0,     0,     0,
      84,     0,    85,     0,     0,     0,     0,    60,    86,    37,
      61,    62,    63,     0,    64,    65,    66,    67,    68,     0,
       0,     0,     0,     0,     0,     0,    69,     0,     0,     0,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,     0,     0,     0,    70,    71,    72,    73,    74,
      75,    76,   450,    77,    78,    79,     0,    80,    81,    82,
     152,   153,   154,   155,   156,   157,     0,     0,     0,     0,
      83,     0,     0,     0,    84,     0,    85,  -227,     0,  -227,
       0,    60,    86,    37,    61,    62,    63,     0,    64,    65,
      66,    67,    68,     0,     0,     0,     0,     0,     0,     0,
      69,     0,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,     0,     0,     0,     0,     0,    70,
      71,    72,    73,    74,    75,    76,     0,    77,    78,    79,
       0,    80,    81,    82,   152,     0,     0,   155,   156,   157,
       0,     0,     0,     0,    83,     0,     0,     0,    84,     0,
      85,     0,     0,  -225,     0,    60,    86,    37,    61,    62,
      63,     0,    64,    65,    66,    67,    68,   467,   468,   469,
       0,     0,     0,     0,    69,     0,     0,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,     0,     0,
       0,     0,     0,    70,    71,    72,    73,    74,    75,    76,
       0,    77,    78,    79,     0,    80,    81,    82,     0,   155,
     156,   157,   475,   476,   477,   478,   479,   480,    83,     0,
       0,     0,    84,     0,    85,  -225,     0,     0,     0,    60,
      86,    37,    61,    62,    63,     0,    64,    65,    66,    67,
      68,     0,     0,     0,     0,     0,     0,     0,    69,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
       0,     0,     0,     0,     0,     0,     0,    70,    71,    72,
      73,    74,    75,    76,     0,    77,    78,    79,     0,    80,
      81,    82,     0,   467,   468,   469,     0,     0,     0,     0,
       0,     0,    83,   148,     0,     0,    84,     0,    85,     0,
       0,     0,     0,    60,    86,    37,    61,    62,    63,     0,
      64,    65,    66,    67,    68,   155,   156,   157,     0,     0,
       0,     0,    69,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,     0,     0,     0,     0,     0,
       0,    70,    71,    72,    73,    74,    75,    76,     0,    77,
      78,    79,     0,    80,    81,    82,     0,     0,   162,   163,
     164,   165,   166,   167,   168,   169,    83,     0,     0,     0,
      84,   327,    85,     0,     0,     0,     0,    60,    86,    37,
      61,    62,    63,     0,    64,    65,    66,    67,    68,   467,
     468,   469,     0,     0,     0,     0,    69,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    70,    71,    72,    73,    74,
      75,    76,     0,    77,    78,    79,     0,    80,    81,    82,
       0,     0,   473,   474,   475,   476,   477,   478,   479,   480,
      83,     0,     0,     0,    84,  -148,    85,     0,     0,     0,
       0,    60,    86,    37,    61,    62,    63,     0,    64,    65,
      66,    67,    68,     0,     0,     0,     0,     0,     0,     0,
      69,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    70,
      71,    72,    73,    74,    75,    76,     0,    77,    78,    79,
       0,    80,    81,    82,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    83,     0,     0,     0,    84,     0,
      85,  -148,     0,     0,     0,    60,    86,    37,    61,    62,
      63,     0,    64,    65,    66,    67,    68,     0,     0,     0,
       0,     0,     0,     0,    69,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    70,    71,    72,    73,    74,    75,    76,
       0,    77,    78,    79,     0,    80,    81,    82,   152,   153,
     154,   155,   156,   157,     0,     0,     0,     0,    83,     0,
       0,     0,    84,     0,    85,     0,     0,     0,     0,     0,
      86,   152,   153,   154,   155,   156,   157,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   152,   153,   154,   155,   156,   157,     0,   201,
       0,     0,     0,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   152,   153,   154,   155,   156,
     157,     0,   299,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   152,   153,   154,   155,
     156,   157,   263,     0,     0,     0,     0,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   152,
     153,   154,   155,   156,   157,   265,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
       0,     0,     0,     0,     0,     0,   356,     0,     0,     0,
       0,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,     0,     0,     0,     0,     0,     0,   358
};

static const yytype_int16 yycheck[] =
{
      22,    22,    73,   251,    48,    10,    67,    68,    67,    68,
      82,    16,     3,   117,    85,    13,    38,    38,   401,    64,
     199,     3,     4,     9,    39,    69,   403,    71,    72,    26,
       1,   281,    54,    54,   437,    80,     0,   141,    20,    83,
      84,   102,   292,   102,    18,    63,    13,     7,   119,    23,
      36,    69,    96,   381,    99,    70,   322,    69,    27,   323,
     324,    47,   465,     6,    70,   137,    72,   139,   318,    39,
     114,   142,    70,   117,   178,   179,    50,   148,    75,    70,
      85,   458,    64,   331,   412,    59,    68,     6,    70,     8,
       9,    69,   136,    75,   138,    38,    70,   141,    69,     6,
      70,    72,    69,    46,   175,    74,    65,    50,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   112,   510,   172,   308,
      74,    50,    75,   399,   178,   179,   207,   181,    26,    26,
      46,   115,   406,    50,   118,    39,    71,   413,   219,    74,
       3,     4,    71,     3,     4,   199,   404,   201,   202,    74,
     204,   427,     4,    78,   150,    46,    73,    20,    36,    37,
      20,   242,     6,    26,     8,     9,    70,    63,    20,     3,
     446,     6,   448,     8,     9,    71,    74,    74,   454,    74,
      78,    78,    72,    78,   239,     0,   300,     8,     9,   173,
     174,   187,     7,   177,    11,    12,    13,    69,   279,    77,
     215,    64,    74,    72,    64,    68,    50,    70,    68,   263,
      70,   265,    64,   489,    69,    50,    68,    74,    70,    74,
     301,    78,   303,   337,    72,   307,    17,    18,    19,   311,
      51,    74,    75,   347,    11,    12,    13,   513,   319,   320,
     321,   296,   323,    74,    74,   299,   300,    78,    78,   304,
       8,     9,    71,    71,   308,    74,    74,    70,    69,    36,
      37,   342,     6,   344,    74,    75,   281,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,   292,    74,    71,
      24,    25,    74,   337,    70,   356,    72,   356,   342,     3,
       8,     9,     6,   347,     8,     9,    10,    13,    71,    71,
      77,    74,    74,   318,   358,   386,    50,    63,    64,    65,
      66,    67,   393,    63,    64,    65,    66,    67,    71,    39,
      71,    74,   403,    74,    39,    39,    40,    41,    42,    43,
      71,    45,   386,    74,   446,    70,   448,    51,    52,    53,
      71,    71,    70,    74,    74,    17,    18,   428,    65,    66,
      67,   405,    39,   407,   408,    13,    70,    65,    66,    67,
      70,   394,    76,   396,    17,    18,    70,   382,     8,     9,
      70,   425,     8,     9,   370,    70,   360,   458,   410,     1,
      74,     3,     4,     5,     6,    73,     8,     9,    10,    11,
      12,    63,    64,    65,    66,    67,    74,    75,    20,     8,
       9,    74,    75,    74,    78,   437,    50,    71,   440,   440,
      63,    64,    65,    66,    67,    47,    71,    39,    40,    41,
      42,    43,    44,    45,    71,    47,    48,    49,    71,    51,
      52,    53,    71,   465,    14,    15,    16,    17,    18,    19,
      62,    71,    64,    71,    71,    62,    68,    71,    70,    72,
      71,    71,    70,    69,    76,     1,    78,     3,     4,     5,
       6,    69,     8,     9,    10,    11,    12,    70,    70,    70,
      70,    75,    69,    72,    20,    73,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    71,    74,
      71,    71,    42,    39,    40,    41,    42,    43,    44,    45,
      71,    47,    48,    49,    28,    51,    52,    53,    74,    69,
      13,    74,    32,    13,    71,    71,    62,    75,    64,    74,
      71,    70,    68,    75,    70,    22,   440,    69,     4,     4,
      76,     1,    78,     3,     4,     5,     6,     4,     8,     9,
      10,    11,    12,    69,    63,    73,    71,    69,    75,    79,
      20,   112,    75,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,    71,   382,    39,
      40,    41,    42,    43,    44,    45,   267,    47,    48,    49,
     494,    51,    52,    53,   406,     4,    73,   180,    14,    15,
      16,    17,    18,    19,    64,   254,   323,    36,    68,   181,
      70,   139,   186,    -1,    -1,    -1,    76,     1,    78,     3,
       4,     5,     6,   202,     8,     9,    10,    11,    12,    17,
      18,    19,    -1,    -1,    -1,    -1,    20,    -1,    -1,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    -1,    -1,    39,    40,    41,    42,    43,
      44,    45,    78,    47,    48,    49,    -1,    51,    52,    53,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      64,    -1,    -1,    -1,    68,    -1,    70,    -1,    -1,    -1,
      -1,    -1,    76,     1,    78,     3,     4,     5,     6,    -1,
       8,     9,    10,    11,    12,    17,    18,    19,    -1,    -1,
      -1,    -1,    20,    21,    -1,    23,    -1,    -1,    -1,    -1,
      28,    29,    30,    31,    -1,    33,    34,    35,    -1,    -1,
      -1,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    64,    -1,    -1,    -1,
      68,    69,    70,    -1,    72,    73,    -1,     1,    76,     3,
       4,     5,     6,    -1,     8,     9,    10,    11,    12,    17,
      18,    19,    -1,    -1,    -1,    -1,    20,    21,    -1,    23,
      24,    25,    -1,    -1,    28,    29,    30,    31,    -1,    33,
      34,    35,    -1,    -1,    -1,    39,    40,    41,    42,    43,
      44,    45,    -1,    47,    48,    49,    -1,    51,    52,    53,
      -1,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      64,    -1,    -1,    -1,    68,    69,    70,    -1,    72,    73,
      -1,     1,    76,     3,     4,     5,     6,    -1,     8,     9,
      10,    11,    12,    17,    18,    19,    -1,    -1,    -1,    -1,
      20,    21,    -1,    23,    -1,    -1,    -1,    -1,    28,    29,
      30,    31,    -1,    33,    34,    35,    -1,    -1,    -1,    39,
      40,    41,    42,    43,    44,    45,    -1,    47,    48,    49,
      -1,    51,    52,    53,    -1,    17,    18,    19,    62,    63,
      64,    65,    66,    67,    64,    -1,    -1,    -1,    68,    69,
      70,    -1,    72,    -1,    -1,     1,    76,     3,     4,     5,
       6,    -1,     8,     9,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    20,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    -1,    -1,    71,
      -1,    -1,    -1,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    14,    15,
      16,    17,    18,    19,    -1,    -1,    -1,    -1,    64,    65,
      -1,    27,    68,    -1,    70,    -1,    -1,    -1,    -1,     1,
      76,     3,     4,     5,     6,    -1,     8,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    -1,    -1,    -1,    -1,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    -1,    -1,    -1,    14,    15,    -1,    17,
      18,    19,    64,    -1,    -1,    -1,    68,    69,    70,    -1,
      -1,    -1,    -1,     1,    76,     3,     4,     5,     6,    -1,
       8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    20,    -1,    -1,    -1,    -1,    -1,    26,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      -1,    39,    40,    41,    42,    43,    44,    45,    -1,    47,
      48,    49,    -1,    51,    52,    53,    -1,    -1,    14,    15,
      16,    17,    18,    19,    62,    -1,    64,    -1,    -1,    -1,
      68,    -1,    70,    -1,    -1,    -1,    -1,     1,    76,     3,
       4,     5,     6,    -1,     8,     9,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    20,    -1,    -1,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    -1,    -1,    39,    40,    41,    42,    43,
      44,    45,    78,    47,    48,    49,    -1,    51,    52,    53,
      14,    15,    16,    17,    18,    19,    -1,    -1,    -1,    -1,
      64,    -1,    -1,    -1,    68,    -1,    70,    71,    -1,    73,
      -1,     1,    76,     3,     4,     5,     6,    -1,     8,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      20,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    -1,    -1,    -1,    -1,    39,
      40,    41,    42,    43,    44,    45,    -1,    47,    48,    49,
      -1,    51,    52,    53,    14,    -1,    -1,    17,    18,    19,
      -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,    68,    -1,
      70,    -1,    -1,    73,    -1,     1,    76,     3,     4,     5,
       6,    -1,     8,     9,    10,    11,    12,    17,    18,    19,
      -1,    -1,    -1,    -1,    20,    -1,    -1,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    -1,
      -1,    -1,    -1,    39,    40,    41,    42,    43,    44,    45,
      -1,    47,    48,    49,    -1,    51,    52,    53,    -1,    17,
      18,    19,    62,    63,    64,    65,    66,    67,    64,    -1,
      -1,    -1,    68,    -1,    70,    71,    -1,    -1,    -1,     1,
      76,     3,     4,     5,     6,    -1,     8,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,    40,    41,
      42,    43,    44,    45,    -1,    47,    48,    49,    -1,    51,
      52,    53,    -1,    17,    18,    19,    -1,    -1,    -1,    -1,
      -1,    -1,    64,    65,    -1,    -1,    68,    -1,    70,    -1,
      -1,    -1,    -1,     1,    76,     3,     4,     5,     6,    -1,
       8,     9,    10,    11,    12,    17,    18,    19,    -1,    -1,
      -1,    -1,    20,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    39,    40,    41,    42,    43,    44,    45,    -1,    47,
      48,    49,    -1,    51,    52,    53,    -1,    -1,    60,    61,
      62,    63,    64,    65,    66,    67,    64,    -1,    -1,    -1,
      68,    69,    70,    -1,    -1,    -1,    -1,     1,    76,     3,
       4,     5,     6,    -1,     8,     9,    10,    11,    12,    17,
      18,    19,    -1,    -1,    -1,    -1,    20,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    39,    40,    41,    42,    43,
      44,    45,    -1,    47,    48,    49,    -1,    51,    52,    53,
      -1,    -1,    60,    61,    62,    63,    64,    65,    66,    67,
      64,    -1,    -1,    -1,    68,    69,    70,    -1,    -1,    -1,
      -1,     1,    76,     3,     4,     5,     6,    -1,     8,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,
      40,    41,    42,    43,    44,    45,    -1,    47,    48,    49,
      -1,    51,    52,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,    68,    -1,
      70,    71,    -1,    -1,    -1,     1,    76,     3,     4,     5,
       6,    -1,     8,     9,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    20,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    39,    40,    41,    42,    43,    44,    45,
      -1,    47,    48,    49,    -1,    51,    52,    53,    14,    15,
      16,    17,    18,    19,    -1,    -1,    -1,    -1,    64,    -1,
      -1,    -1,    68,    -1,    70,    -1,    -1,    -1,    -1,    -1,
      76,    14,    15,    16,    17,    18,    19,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    14,    15,    16,    17,    18,    19,    -1,    75,
      -1,    -1,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    14,    15,    16,    17,    18,
      19,    -1,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    14,    15,    16,    17,
      18,    19,    74,    -1,    -1,    -1,    -1,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    14,
      15,    16,    17,    18,    19,    74,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    -1,    -1,    -1,    -1,    -1,    -1,    74
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    85,    86,     0,     7,    88,    89,   127,   128,   135,
     136,   148,   149,   136,    69,    87,   129,    65,   143,   158,
     159,     6,    38,    50,    75,   137,   138,   139,   140,    50,
      73,   138,     8,     9,   144,    69,    74,     3,    70,   172,
     173,     8,     9,   144,    46,     8,     9,   130,    13,    90,
     143,   158,   173,     3,    63,    69,    72,   131,   132,   143,
       1,     4,     5,     6,     8,     9,    10,    11,    12,    20,
      39,    40,    41,    42,    43,    44,    45,    47,    48,    49,
      51,    52,    53,    64,    68,    70,    76,   109,   110,   111,
     113,   114,   115,   116,   117,   118,   142,   144,   146,   147,
     160,   161,   170,   171,   172,   174,   175,    70,   144,    71,
     173,    69,    74,   144,    13,    39,    70,   112,    39,    70,
     110,   111,   110,   109,   144,   109,   163,   164,   165,   109,
     162,   166,   167,   108,   109,   146,    70,    83,    70,    83,
      70,   112,    70,    72,   102,   109,   109,     6,    65,   108,
     138,    70,    14,    15,    16,    17,    18,    19,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      11,    12,    13,    36,    37,    77,   109,    39,   112,   112,
      70,    74,    75,   110,     8,     9,   138,   141,   145,   150,
     151,   152,   131,   109,   144,   165,    51,   144,    27,    74,
      73,    75,    74,    78,    74,    75,   109,    70,   102,   169,
     109,   169,    50,   165,   108,    82,   108,    71,   143,    84,
     109,   109,   109,   109,   109,   109,   109,   109,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   109,   109,   144,
     144,    26,    62,   108,   144,   165,   165,   150,   164,    71,
     170,   143,    71,    27,    74,    71,   163,    71,   109,   162,
      71,   109,    71,    74,   108,    74,     8,     9,    71,    71,
       1,    21,    23,    28,    29,    30,    31,    33,    34,    35,
      69,    93,    94,    95,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   108,   119,   138,    71,    71,   108,    75,
     112,    62,   108,    26,    78,    71,    71,    71,    74,    75,
     145,    91,   152,   109,    71,   109,   134,   134,    69,    70,
      70,    70,    80,    70,    70,    69,    69,    69,   108,   103,
      73,   143,   154,   155,   103,    69,    72,   112,    71,   109,
     165,   108,    26,    78,    62,    78,   108,   112,   102,    71,
      75,   153,     1,    69,    92,   102,    74,   168,    74,    71,
      74,    71,   103,   108,   108,   108,    93,   108,   123,   124,
     141,   156,   157,     8,     9,   125,   126,   156,    69,   145,
      69,    74,   107,   165,    71,    78,    62,    78,   108,   109,
     108,    78,   165,    42,   110,    71,   109,   133,   144,    71,
      71,    71,    28,    69,   143,    13,    74,    32,    13,   154,
      24,    25,   106,   121,   138,    71,   108,   109,    78,    78,
      78,    71,   108,   168,   168,    75,    93,    80,    70,   123,
     145,   109,   125,   109,   109,     4,    20,    26,    64,    68,
      70,   122,   173,   176,    75,   154,    93,   120,   121,    78,
      78,    75,    71,   109,    22,    96,    93,   108,    69,    71,
       4,   122,     4,     4,   176,    26,    75,    17,    18,    19,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    69,   120,    73,   120,    71,    93,    71,   123,    81,
      75,    71,    75,   122,    70,   176,   176,   176,   176,   176,
     176,   176,   176,   176,   176,   176,   176,   176,   176,    69,
      71,    93,    75,    80,    93
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
     109,   109,   109,   109,   109,   109,   109,   109,   109,   109,
     109,   109,   110,   111,   111,   111,   111,   111,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
     111,   112,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   114,   115,   116,   117,   118,   118,   119,   120,   120,
     120,   121,   121,   121,   121,   121,   122,   122,   123,   123,
     124,   124,   125,   125,   125,   126,   126,   127,   128,   129,
     130,   129,   131,   131,   132,   133,   134,   134,   135,   136,
     136,   137,   137,   138,   138,   139,   139,   140,   140,   140,
     141,   141,   142,   143,   143,   144,   144,   145,   145,   146,
     146,   147,   147,   147,   147,   148,   149,   150,   150,   150,
     151,   151,   152,   152,   152,   153,   153,   154,   154,   155,
     155,   156,   157,   158,   158,   159,   159,   160,   161,   161,
     162,   163,   163,   164,   164,   165,   165,   165,   166,   166,
     167,   167,   167,   168,   168,   169,   169,   170,   171,   172,
     172,   173,   173,   173,   174,   175,   176,   176,   176,   176,
     176,   176,   176,   176,   176,   176,   176,   176,   176,   176,
     176,   176,   176,   176,   176
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
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       1,     5,     3,     3,     4,     5,     6,     7,     8,     7,
       5,     6,     6,     8,     7,     5,     3,     5,     4,     4,
       4,     1,     4,     4,     6,     6,     4,     4,     7,     6,
       7,     7,     9,     3,     3,     2,     4,     4,     2,     2,
       0,     3,     5,     4,     4,     2,     1,     1,     0,     1,
       1,     1,     1,     1,     1,     1,     3,     3,     4,     0,
       0,     5,     1,     3,     2,     3,     0,     3,     2,     0,
       2,     1,     0,     1,     2,     1,     0,     1,     2,     2,
       1,     2,     4,     0,     1,     1,     1,     1,     1,     3,
       3,     1,     2,     3,     3,     4,     2,     0,     1,     2,
       1,     3,     2,     4,     1,     0,     5,     1,     3,     2,
       4,     3,     3,     1,     3,     2,     4,     1,     1,     2,
       3,     1,     2,     1,     3,     0,     1,     2,     1,     3,
       0,     1,     2,     0,     3,     1,     3,     1,     1,     1,
       2,     1,     3,     3,     1,     1,     3,     3,     3,     3,
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
  case 2: /* loop_start: %empty  */
#line 206 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { (yyval.number) = rule_loop_open(); }
#line 2372 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 3: /* foreach_start: %empty  */
#line 207 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                      { (yyval.number) = rule_foreach_open(); }
#line 2378 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 4: /* block_start: %empty  */
#line 208 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.number) = rule_block_open(); }
#line 2384 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 5: /* special_context_start: %empty  */
#line 209 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { (yyval.number) = rule_special_context_open(); }
#line 2390 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 6: /* dollar_start: %empty  */
#line 210 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                     { (yyval.contextp) = rule_dollar_open(); }
#line 2396 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 7: /* all: program  */
#line 219 "$REPO_ROOT$/src/compiler/internal/grammar.y"
           { rule_program((yyvsp[0].node)); }
#line 2402 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 8: /* program: program def opt_semicolon  */
#line 224 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_program_append(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 2408 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 9: /* program: %empty  */
#line 225 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.node) = 0; }
#line 2414 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 11: /* opt_semicolon: ';'  */
#line 231 "$REPO_ROOT$/src/compiler/internal/grammar.y"
         { rule_opt_semicolon(); }
#line 2420 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 13: /* def: type name_list ';'  */
#line 243 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_def_global_var((yyvsp[-2].number)); (yyval.node) = 0; }
#line 2426 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 17: /* @1: %empty  */
#line 263 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_func_type((yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].string)); }
#line 2432 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 18: /* @2: %empty  */
#line 265 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_func_proto((yyvsp[-3].number), (yyvsp[-5].number), &(yyvsp[-4].string), (yyvsp[-1].argument)); }
#line 2438 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 19: /* function: type optional_star identifier @1 '(' argument ')' @2 block_or_semi  */
#line 267 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_func(&(yyval.node), (yyvsp[-5].number), (yyvsp[-7].number), (yyvsp[-6].string), (yyvsp[-3].argument), &(yyvsp[-1].number), &(yyvsp[0].node)); }
#line 2444 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 20: /* block_or_semi: block  */
#line 272 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = rule_block_or_semi((yyvsp[0].decl).node); }
#line 2450 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 21: /* block_or_semi: ';'  */
#line 273 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2456 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 22: /* block_or_semi: error  */
#line 274 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2462 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 23: /* statement: comma_expr ';'  */
#line 284 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_expr((yyvsp[-1].node)); }
#line 2468 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 27: /* statement: stmt_for  */
#line 288 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2474 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 28: /* statement: stmt_foreach  */
#line 289 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2480 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 31: /* statement: block  */
#line 292 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2486 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 32: /* statement: ';'  */
#line 293 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = 0; }
#line 2492 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 33: /* statement: L_BREAK ';'  */
#line 294 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_break(); }
#line 2498 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 34: /* statement: L_CONTINUE ';'  */
#line 295 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_continue(); }
#line 2504 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 35: /* stmt_return: L_RETURN ';'  */
#line 301 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_return_void(&(yyval.node)); }
#line 2510 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 36: /* stmt_return: L_RETURN comma_expr ';'  */
#line 302 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_return_expr(&(yyval.node), (yyvsp[-1].node)); }
#line 2516 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 37: /* stmt_cond: L_IF '(' comma_expr ')' statement optional_else_part  */
#line 308 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_cond(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 2522 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 38: /* optional_else_part: %empty  */
#line 312 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                { (yyval.node) = 0; }
#line 2528 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 39: /* optional_else_part: L_ELSE statement  */
#line 313 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                 { (yyval.node) = (yyvsp[0].node); }
#line 2534 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 40: /* stmt_while: L_WHILE '(' comma_expr ')' loop_start statement  */
#line 319 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_while(&(yyval.node), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2540 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 41: /* stmt_do: L_DO loop_start statement L_WHILE '(' comma_expr ')' ';'  */
#line 325 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_do(&(yyval.node), (yyvsp[-5].node), (yyvsp[-2].node), (yyvsp[-6].number)); }
#line 2546 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 42: /* stmt_for: L_FOR '(' for_init ';' for_expr ';' for_expr ')' loop_start statement  */
#line 331 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for(&(yyval.decl), &(yyvsp[-7].decl), (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2552 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 43: /* stmt_foreach: L_FOREACH '(' foreach_vars L_IN expr ')' foreach_start statement  */
#line 337 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach(&(yyval.decl), &(yyvsp[-5].decl), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2558 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 44: /* stmt_switch: switch_header '{' local_declarations case switch_block '}'  */
#line 343 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_switch(&(yyval.node), (yyvsp[-5].switch_header).expr, &(yyvsp[-3].decl), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[-5].switch_header).saved_context, (yyvsp[-5].switch_header).saved_cases_size); }
#line 2564 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 45: /* block: '{' block_start statements '}'  */
#line 355 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block(&(yyval.decl), (yyvsp[-1].decl).node, (yyvsp[-2].number)); }
#line 2570 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 46: /* statements: %empty  */
#line 361 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_empty(&(yyval.decl)); }
#line 2576 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 47: /* statements: statement statements  */
#line 363 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_stmt(&(yyval.decl), (yyvsp[-1].node), &(yyvsp[0].decl)); }
#line 2582 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 48: /* statements: local_declaration_statement statements  */
#line 365 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_decl(&(yyval.decl), &(yyvsp[-1].decl), &(yyvsp[0].decl)); }
#line 2588 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 49: /* statements: error ';' statements  */
#line 367 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_error(&(yyval.decl), &(yyvsp[0].decl)); }
#line 2594 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 50: /* local_decl_statement_header: basic_type  */
#line 372 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { rule_local_declaration_statement_set_type((yyvsp[0].number)); (yyval.number) = (yyvsp[0].number); }
#line 2600 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 51: /* local_declaration_statement: local_decl_statement_header local_name_list ';'  */
#line 377 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_declaration_statement(&(yyval.decl), &(yyvsp[-1].decl)); }
#line 2606 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 52: /* local_decl_header: basic_type  */
#line 382 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { rule_local_declarations_set_type((yyvsp[0].number)); (yyval.number) = (yyvsp[0].number); }
#line 2612 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 53: /* local_declarations: %empty  */
#line 387 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.decl).node = 0; (yyval.decl).num = 0; }
#line 2618 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 54: /* local_declarations: local_declarations local_decl_header local_name_list ';'  */
#line 389 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_declarations(&(yyval.decl), &(yyvsp[-3].decl), &(yyvsp[-1].decl)); }
#line 2624 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 56: /* comma_expr: comma_expr ',' expr  */
#line 408 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                { rule_comma_expr(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2630 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 57: /* expr: ref lvalue  */
#line 412 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_ref(&(yyval.node), (yyvsp[0].node)); }
#line 2636 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 58: /* expr: lvalue L_ASSIGN expr  */
#line 413 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_assign(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 2642 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 59: /* expr: error L_ASSIGN expr  */
#line 414 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_assign_error(&(yyval.node), (yyvsp[0].node)); }
#line 2648 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 60: /* expr: expr '?' expr ':' expr  */
#line 416 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                            { rule_expr_ternary(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2654 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 61: /* expr: expr L_QUESTION_QUESTION expr  */
#line 417 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_nullish(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2660 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 62: /* expr: expr L_LOR expr  */
#line 418 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_lor(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2666 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 63: /* expr: expr L_LAND expr  */
#line 419 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_land(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2672 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 64: /* expr: expr '|' expr  */
#line 421 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_or(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2678 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 65: /* expr: expr '^' expr  */
#line 422 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_xor(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2684 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 66: /* expr: expr '&' expr  */
#line 423 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_and(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2690 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 67: /* expr: expr L_EQ expr  */
#line 425 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_eq(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2696 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 68: /* expr: expr L_NE expr  */
#line 426 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_ne(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2702 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 69: /* expr: expr L_ORDER expr  */
#line 427 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_order(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 2708 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 70: /* expr: expr '<' expr  */
#line 428 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_lt(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2714 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 71: /* expr: expr L_LSH expr  */
#line 430 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                               { rule_expr_lsh(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2720 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 72: /* expr: expr L_RSH expr  */
#line 431 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                               { rule_expr_rsh(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2726 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 73: /* expr: expr '+' expr  */
#line 433 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_add(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2732 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 74: /* expr: expr '-' expr  */
#line 434 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_sub(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2738 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 75: /* expr: expr '*' expr  */
#line 435 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_mul(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2744 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 76: /* expr: expr '%' expr  */
#line 436 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_mod(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2750 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 77: /* expr: expr '/' expr  */
#line 437 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_div(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2756 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 78: /* expr: cast expr  */
#line 439 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_expr_cast(&(yyval.node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 2762 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 79: /* expr: L_INC lvalue  */
#line 440 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                               { rule_expr_pre_inc(&(yyval.node), (yyvsp[0].node)); }
#line 2768 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 80: /* expr: L_DEC lvalue  */
#line 441 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                               { rule_expr_pre_dec(&(yyval.node), (yyvsp[0].node)); }
#line 2774 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 81: /* expr: L_NOT expr  */
#line 442 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_expr_not(&(yyval.node), (yyvsp[0].node)); }
#line 2780 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 82: /* expr: '~' expr  */
#line 443 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_expr_compl(&(yyval.node), (yyvsp[0].node)); }
#line 2786 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 83: /* expr: '-' expr  */
#line 444 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_expr_neg(&(yyval.node), (yyvsp[0].node)); }
#line 2792 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 84: /* expr: lvalue L_INC  */
#line 446 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_expr_post_inc(&(yyval.node), (yyvsp[-1].node)); }
#line 2798 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 85: /* expr: lvalue L_DEC  */
#line 447 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_expr_post_dec(&(yyval.node), (yyvsp[-1].node)); }
#line 2804 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 92: /* lvalue: primary_expr  */
#line 459 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                { (yyval.node) = rule_lvalue((yyvsp[0].node)); }
#line 2810 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 94: /* primary_expr: L_DEFINED_NAME  */
#line 469 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_defined_name(&(yyval.node), (yyvsp[0].ihe)); }
#line 2816 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 95: /* primary_expr: L_IDENTIFIER  */
#line 470 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_identifier(&(yyval.node), (yyvsp[0].string)); }
#line 2822 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 96: /* primary_expr: L_PARAMETER  */
#line 471 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_parameter(&(yyval.node), (yyvsp[0].number)); }
#line 2828 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 98: /* primary_expr: '(' comma_expr ')'  */
#line 473 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { (yyval.node) = (yyvsp[-1].node); }
#line 2834 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 101: /* primary_expr: '$' '(' dollar_start comma_expr ')'  */
#line 479 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_dollar_expr(&(yyval.node), (yyvsp[-2].contextp), (yyvsp[-1].node)); }
#line 2840 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 102: /* primary_expr: primary_expr L_ARROW identifier  */
#line 482 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                 { rule_primary_expr_member_arrow(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 2846 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 103: /* primary_expr: primary_expr L_DOT identifier  */
#line 483 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                 { rule_primary_expr_member_dot(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 2852 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 104: /* primary_expr: primary_expr '[' comma_expr ']'  */
#line 488 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2858 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 105: /* primary_expr: primary_expr '[' '<' comma_expr ']'  */
#line 490 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_r(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 2864 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 106: /* primary_expr: primary_expr '[' comma_expr L_RANGE comma_expr ']'  */
#line 492 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nn(&(yyval.node), (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2870 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 107: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE comma_expr ']'  */
#line 494 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rn(&(yyval.node), (yyvsp[-6].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2876 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 108: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE '<' comma_expr ']'  */
#line 496 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rr(&(yyval.node), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 2882 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 109: /* primary_expr: primary_expr '[' comma_expr L_RANGE '<' comma_expr ']'  */
#line 498 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nr(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 2888 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 110: /* primary_expr: primary_expr '[' comma_expr L_RANGE ']'  */
#line 500 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_ne(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node)); }
#line 2894 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 111: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE ']'  */
#line 502 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_re(&(yyval.node), (yyvsp[-5].node), (yyvsp[-2].node)); }
#line 2900 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 112: /* primary_expr: primary_expr '[' L_RANGE '<' comma_expr ']'  */
#line 504 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nr(&(yyval.node), (yyvsp[-5].node), 0, (yyvsp[-1].node)); }
#line 2906 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 113: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE '<' expr ']'  */
#line 507 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rr(&(yyval.node), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 2912 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 114: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE expr ']'  */
#line 509 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rn(&(yyval.node), (yyvsp[-6].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2918 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 115: /* primary_expr: lambda_return_type '(' argument ')' block  */
#line 514 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_anon_func(&(yyval.node), &(yyvsp[-4].func_block), &(yyvsp[-2].argument), &(yyvsp[0].decl)); }
#line 2924 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 116: /* primary_expr: functional_open ':' ')'  */
#line 521 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_1(&(yyval.node), (yyvsp[-2].number)); }
#line 2930 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 117: /* primary_expr: functional_open ',' arg_list ':' ')'  */
#line 523 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_2(&(yyval.node), (yyvsp[-4].number), (yyvsp[-2].node)); }
#line 2936 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 118: /* primary_expr: L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 525 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_3(&(yyval.node), (yyvsp[-2].node)); }
#line 2942 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 119: /* primary_expr: L_MAPPING_OPEN opt_pair_list ']' ')'  */
#line 528 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { rule_primary_expr_mapping(&(yyval.node), (yyvsp[-2].node)); }
#line 2948 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 120: /* primary_expr: L_ARRAY_OPEN opt_arg_list '}' ')'  */
#line 529 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { rule_primary_expr_array(&(yyval.node), (yyvsp[-2].node)); }
#line 2954 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 121: /* call_open: '('  */
#line 539 "$REPO_ROOT$/src/compiler/internal/grammar.y"
       { rule_call_open(&(yyval.call_open).saved_context, &(yyval.call_open).saved_refs); }
#line 2960 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 122: /* function_call: efun_override call_open opt_arg_list ')'  */
#line 553 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_efun(&(yyval.node), (yyvsp[-3].number), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 2966 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 123: /* function_call: L_NEW call_open opt_arg_list ')'  */
#line 555 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new(&(yyval.node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 2972 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 124: /* function_call: L_NEW '(' L_CLASS L_DEFINED_NAME opt_class_init ')'  */
#line 557 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new_class(&(yyval.node), (yyvsp[-2].ihe), (yyvsp[-1].node)); }
#line 2978 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 125: /* function_call: L_NEW '(' L_CLASS L_IDENTIFIER opt_class_init ')'  */
#line 559 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new_class_undef(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node)); }
#line 2984 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 126: /* function_call: L_DEFINED_NAME call_open opt_arg_list ')'  */
#line 561 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_defined_name(&(yyval.node), (yyvsp[-3].ihe), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 2990 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 127: /* function_call: function_name call_open opt_arg_list ')'  */
#line 563 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_name(&(yyval.node), (yyvsp[-3].string), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 2996 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 128: /* function_call: primary_expr '[' comma_expr ']' call_open opt_arg_list ')'  */
#line 565 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_indexed(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3002 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 129: /* function_call: primary_expr L_ARROW identifier call_open opt_arg_list ')'  */
#line 567 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_arrow(&(yyval.node), (yyvsp[-5].node), (yyvsp[-3].string), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3008 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 130: /* function_call: '(' '*' comma_expr ')' call_open opt_arg_list ')'  */
#line 569 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_star(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3014 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 131: /* sscanf: L_SSCANF '(' expr ',' expr lvalue_list ')'  */
#line 580 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_sscanf(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 3020 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 132: /* parse_command: L_PARSE_COMMAND '(' expr ',' expr ',' expr lvalue_list ')'  */
#line 586 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_parse_command(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 3026 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 133: /* catch: L_CATCH special_context_start expr_or_block  */
#line 592 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_catch(&(yyval.node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 3032 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 134: /* time_expression: L_TIME_EXPRESSION special_context_start expr_or_block  */
#line 598 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_time_expression(&(yyval.node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 3038 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 135: /* tree: L_TREE block  */
#line 604 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_tree_block(&(yyval.node), (yyvsp[0].decl).node); }
#line 3044 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 136: /* tree: L_TREE '(' comma_expr ')'  */
#line 606 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_tree_expr(&(yyval.node), (yyvsp[-1].node)); }
#line 3050 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 137: /* switch_header: L_SWITCH '(' comma_expr ')'  */
#line 617 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.switch_header).expr = (yyvsp[-1].node); rule_switch_pre(&(yyval.switch_header).saved_context, &(yyval.switch_header).saved_cases_size); }
#line 3056 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 138: /* switch_block: case switch_block  */
#line 622 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_switch_block_case(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3062 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 139: /* switch_block: statement switch_block  */
#line 623 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_switch_block_stmt(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3068 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 140: /* switch_block: %empty  */
#line 624 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { rule_switch_block_empty(&(yyval.node)); }
#line 3074 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 141: /* case: L_CASE case_label ':'  */
#line 631 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_single(&(yyval.node), (yyvsp[-1].node)); }
#line 3080 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 142: /* case: L_CASE case_label L_RANGE case_label ':'  */
#line 633 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3086 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 143: /* case: L_CASE case_label L_RANGE ':'  */
#line 635 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range_from(&(yyval.node), (yyvsp[-2].node)); }
#line 3092 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 144: /* case: L_CASE L_RANGE case_label ':'  */
#line 637 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range_to(&(yyval.node), (yyvsp[-1].node)); }
#line 3098 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 145: /* case: L_DEFAULT ':'  */
#line 639 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_default(&(yyval.node)); }
#line 3104 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 146: /* case_label: constant  */
#line 645 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_case_label_constant(&(yyval.node), (yyvsp[0].number)); }
#line 3110 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 147: /* case_label: string_const  */
#line 646 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_case_label_string(&(yyval.node), (yyvsp[0].string)); }
#line 3116 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 148: /* for_expr: %empty  */
#line 656 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { (yyval.node) = 0; }
#line 3122 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 150: /* for_init: for_expr  */
#line 663 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for_init_expr(&(yyval.decl), (yyvsp[0].node)); }
#line 3128 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 151: /* for_init: single_new_local_def_with_init  */
#line 665 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for_init_local(&(yyval.decl), (yyvsp[0].node)); }
#line 3134 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 152: /* foreach_var: L_DEFINED_NAME  */
#line 670 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_defined(&(yyval.decl), (yyvsp[0].ihe)); }
#line 3140 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 153: /* foreach_var: single_new_local_def  */
#line 671 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_new_local(&(yyval.decl), (yyvsp[0].number)); }
#line 3146 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 154: /* foreach_var: L_IDENTIFIER  */
#line 672 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_identifier(&(yyval.decl), (yyvsp[0].string)); }
#line 3152 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 155: /* foreach_vars: foreach_var  */
#line 678 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach_vars_single(&(yyval.decl), &(yyvsp[0].decl)); }
#line 3158 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 156: /* foreach_vars: foreach_var ',' foreach_var  */
#line 680 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach_vars_double(&(yyval.decl), &(yyvsp[-2].decl), &(yyvsp[0].decl)); }
#line 3164 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 157: /* type_decl: class_header member_list '}'  */
#line 691 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_define_class_members((yyvsp[-2].class_header).ihe, (yyvsp[-2].class_header).classname_index); (yyval.node) = 0; }
#line 3170 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 158: /* class_header: type_modifier_list L_CLASS identifier '{'  */
#line 697 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.class_header).ihe = rule_define_class(&(yyval.class_header).classname_index, (yyvsp[-1].string)); }
#line 3176 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 160: /* $@3: %empty  */
#line 703 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                           { rule_member_list_set_type((yyvsp[0].number)); }
#line 3182 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 164: /* member_name: optional_star identifier  */
#line 715 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { rule_member_name((yyvsp[-1].number), (yyvsp[0].string)); }
#line 3188 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 165: /* class_init: identifier ':' expr  */
#line 720 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                       { (yyval.node) = rule_class_init((yyvsp[-2].string), (yyvsp[0].node)); }
#line 3194 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 166: /* opt_class_init: %empty  */
#line 725 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.node) = rule_opt_class_init_empty(); }
#line 3200 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 167: /* opt_class_init: opt_class_init ',' class_init  */
#line 726 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { (yyval.node) = rule_opt_class_init((yyvsp[-2].node), (yyvsp[0].node)); }
#line 3206 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 168: /* type: type_modifier_list opt_basic_type  */
#line 738 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { (yyval.number) = rule_type((yyvsp[-1].number), (yyvsp[0].number)); }
#line 3212 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 169: /* type_modifier_list: %empty  */
#line 743 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { (yyval.number) = 0; }
#line 3218 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 170: /* type_modifier_list: L_TYPE_MODIFIER type_modifier_list  */
#line 744 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                               { (yyval.number) = rule_type_modifier_list((yyvsp[-1].number), (yyvsp[0].number)); }
#line 3224 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 172: /* opt_basic_type: %empty  */
#line 750 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { (yyval.number) = rule_opt_basic_type_empty(); }
#line 3230 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 174: /* basic_type: opt_atomic_type L_ARRAY  */
#line 756 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { (yyval.number) = (yyvsp[-1].number) | TYPE_MOD_ARRAY; }
#line 3236 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 176: /* opt_atomic_type: %empty  */
#line 762 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { (yyval.number) = rule_opt_basic_type_empty(); }
#line 3242 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 178: /* atomic_type: L_CLASS L_DEFINED_NAME  */
#line 768 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.number) = rule_atomic_type_class((yyvsp[0].ihe)); }
#line 3248 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 179: /* atomic_type: L_CLASS L_IDENTIFIER  */
#line 769 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.number) = rule_atomic_type_class_identifier((yyvsp[0].string)); }
#line 3254 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 181: /* arg_type: basic_type ref  */
#line 775 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.number) = (yyvsp[-1].number) | LOCAL_MOD_REF; }
#line 3260 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 182: /* cast: '(' basic_type optional_star ')'  */
#line 780 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { (yyval.number) = rule_cast((yyvsp[-2].number), (yyvsp[-1].number)); }
#line 3266 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 183: /* optional_star: %empty  */
#line 785 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.number) = 0; }
#line 3272 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 184: /* optional_star: '*'  */
#line 786 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.number) = TYPE_MOD_ARRAY; }
#line 3278 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 185: /* identifier: L_DEFINED_NAME  */
#line 796 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { (yyval.string) = rule_identifier_defined_name((yyvsp[0].ihe)); }
#line 3284 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 188: /* new_local_name: L_DEFINED_NAME  */
#line 803 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.string) = rule_new_local_name_redefine((yyvsp[0].ihe)); }
#line 3290 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 189: /* efun_override: L_EFUN L_COLON_COLON identifier  */
#line 808 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.number) = rule_efun_override((yyvsp[0].string)); }
#line 3296 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 190: /* efun_override: L_EFUN L_COLON_COLON L_NEW  */
#line 809 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.number) = rule_efun_override_new(); }
#line 3302 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 192: /* function_name: L_COLON_COLON identifier  */
#line 820 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { (yyval.string) = rule_function_name_colon_colon((yyvsp[0].string)); }
#line 3308 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 193: /* function_name: L_BASIC_TYPE L_COLON_COLON identifier  */
#line 821 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { (yyval.string) = rule_function_name_type((yyvsp[-2].number), (yyvsp[0].string)); }
#line 3314 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 194: /* function_name: identifier L_COLON_COLON identifier  */
#line 822 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                    { (yyval.string) = rule_function_name_obj((yyvsp[-2].string), (yyvsp[0].string)); }
#line 3320 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 195: /* inheritance: type_modifier_list L_INHERIT string_const ';'  */
#line 828 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { if (rule_inheritence(&(yyval.node), (yyvsp[-3].number), (yyvsp[-1].string))) { YYACCEPT; } }
#line 3326 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 196: /* modifier_change: type_modifier_list ':'  */
#line 833 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { (yyval.node) = rule_modifier_change((yyvsp[-1].number)); }
#line 3332 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 197: /* argument: %empty  */
#line 844 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.argument).num_arg = 0; (yyval.argument).flags = 0; }
#line 3338 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 199: /* argument: argument_list L_DOT_DOT_DOT  */
#line 847 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_varargs(&(yyval.argument), &(yyvsp[-1].argument)); }
#line 3344 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 200: /* argument_list: param_decl  */
#line 853 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_list_single(&(yyval.argument), (yyvsp[0].number)); }
#line 3350 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 201: /* argument_list: argument_list ',' param_decl  */
#line 855 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_list_multi(&(yyval.argument), &(yyvsp[-2].argument), (yyvsp[0].number)); }
#line 3356 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 202: /* param_decl: arg_type optional_star  */
#line 865 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_typed((yyvsp[-1].number) | (yyvsp[0].number)); }
#line 3362 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 203: /* param_decl: arg_type optional_star new_local_name optional_default_arg_value  */
#line 867 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_typed_name((yyvsp[-3].number) | (yyvsp[-2].number), (yyvsp[-1].string), (yyvsp[0].node)); }
#line 3368 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 204: /* param_decl: new_local_name  */
#line 869 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_untyped_name((yyvsp[0].string)); }
#line 3374 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 205: /* optional_default_arg_value: %empty  */
#line 874 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                          { (yyval.node) = 0; }
#line 3380 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 206: /* optional_default_arg_value: ':' L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 875 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                            { (yyval.node) = rule_default_arg_value((yyvsp[-2].node)); }
#line 3386 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 207: /* local_name_list: new_local_def  */
#line 881 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_name_list_single(&(yyval.decl), (yyvsp[0].node)); }
#line 3392 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 208: /* local_name_list: new_local_def ',' local_name_list  */
#line 883 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_name_list_multi(&(yyval.decl), (yyvsp[-2].node), &(yyvsp[0].decl)); }
#line 3398 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 209: /* new_local_def: optional_star new_local_name  */
#line 889 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_new_local_def((yyvsp[0].string), (yyvsp[-1].number)); }
#line 3404 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 210: /* new_local_def: optional_star new_local_name L_ASSIGN expr  */
#line 891 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_new_local_def_with_init((yyvsp[-2].string), (yyvsp[-3].number), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3410 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 211: /* single_new_local_def: arg_type optional_star new_local_name  */
#line 897 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_single_new_local_def(&(yyval.number), (yyvsp[-2].number) | (yyvsp[-1].number), (yyvsp[0].string)); }
#line 3416 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 212: /* single_new_local_def_with_init: single_new_local_def L_ASSIGN expr  */
#line 903 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_single_new_local_def_with_init((yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3422 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 215: /* new_name: optional_star identifier  */
#line 919 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { rule_new_name((yyvsp[-1].number), (yyvsp[0].string)); }
#line 3428 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 216: /* new_name: optional_star identifier L_ASSIGN expr  */
#line 920 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { rule_new_name_with_init((yyvsp[-3].number), (yyvsp[-2].string), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3434 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 217: /* lambda_return_type: L_BASIC_TYPE  */
#line 931 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                { rule_lambda_return_type(&(yyval.func_block), (yyvsp[0].number)); }
#line 3440 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 219: /* functional_open: L_FUNCTION_OPEN efun_override  */
#line 940 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.number) = rule_functional_open((yyvsp[0].number)); }
#line 3446 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 220: /* assoc_pair: expr ':' expr  */
#line 950 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_assoc_pair(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3452 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 221: /* spread_expr: expr  */
#line 955 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_spread_expr_normal(&(yyval.node), (yyvsp[0].node)); }
#line 3458 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 222: /* spread_expr: expr L_DOT_DOT_DOT  */
#line 956 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_spread_expr_dots(&(yyval.node), (yyvsp[-1].node)); }
#line 3464 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 223: /* arg_list: spread_expr  */
#line 962 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_arg_list_single(&(yyval.node), (yyvsp[0].node)); }
#line 3470 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 224: /* arg_list: arg_list ',' spread_expr  */
#line 964 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_arg_list_multi(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3476 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 225: /* opt_arg_list: %empty  */
#line 970 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list_empty(&(yyval.node)); }
#line 3482 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 226: /* opt_arg_list: arg_list  */
#line 971 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list(&(yyval.node), (yyvsp[0].node)); }
#line 3488 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 227: /* opt_arg_list: arg_list ','  */
#line 972 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list(&(yyval.node), (yyvsp[-1].node)); }
#line 3494 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 228: /* pair_list: assoc_pair  */
#line 978 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_pair_list_single(&(yyval.node), (yyvsp[0].node)); }
#line 3500 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 229: /* pair_list: pair_list ',' assoc_pair  */
#line 980 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_pair_list_multi(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3506 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 230: /* opt_pair_list: %empty  */
#line 986 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list_empty(&(yyval.node)); }
#line 3512 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 231: /* opt_pair_list: pair_list  */
#line 987 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list(&(yyval.node), (yyvsp[0].node)); }
#line 3518 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 232: /* opt_pair_list: pair_list ','  */
#line 988 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list(&(yyval.node), (yyvsp[-1].node)); }
#line 3524 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 233: /* lvalue_list: %empty  */
#line 993 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.node) = rule_lvalue_list_empty(); }
#line 3530 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 234: /* lvalue_list: ',' lvalue lvalue_list  */
#line 994 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { (yyval.node) = rule_lvalue_list((yyvsp[-1].node), (yyvsp[0].node)); }
#line 3536 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 235: /* expr_or_block: block  */
#line 999 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_expr_or_block_block((yyvsp[0].decl)); }
#line 3542 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 236: /* expr_or_block: '(' comma_expr ')'  */
#line 1000 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_expr_or_block_expr((yyvsp[-1].node)); }
#line 3548 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 238: /* string: string_literal  */
#line 1015 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_string(&(yyval.node), (yyvsp[0].string)); }
#line 3554 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 240: /* string_literal: string_literal L_STRING  */
#line 1023 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.string) = rule_string_literal_concat((yyvsp[-1].string), (yyvsp[0].string)); }
#line 3560 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 242: /* string_const: '(' string_const ')'  */
#line 1030 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                        { (yyval.string) = (yyvsp[-1].string); }
#line 3566 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 243: /* string_const: string_const '+' string_const  */
#line 1031 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { (yyval.string) = rule_string_literal_concat((yyvsp[-2].string), (yyvsp[0].string)); }
#line 3572 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 244: /* number: L_NUMBER  */
#line 1035 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { rule_number(&(yyval.node), (yyvsp[0].number)); }
#line 3578 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 245: /* real: L_REAL  */
#line 1039 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { rule_real(&(yyval.node), (yyvsp[0].real)); }
#line 3584 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 246: /* constant: constant '|' constant  */
#line 1045 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_or(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3590 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 247: /* constant: constant '^' constant  */
#line 1046 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_xor(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3596 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 248: /* constant: constant '&' constant  */
#line 1047 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_and(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3602 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 249: /* constant: constant L_EQ constant  */
#line 1048 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_eq(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3608 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 250: /* constant: constant L_NE constant  */
#line 1049 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_ne(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3614 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 251: /* constant: constant L_ORDER constant  */
#line 1050 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { rule_constant_order(&(yyval.number), (yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].number)); }
#line 3620 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 252: /* constant: constant '<' constant  */
#line 1051 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_lt(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3626 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 253: /* constant: constant L_LSH constant  */
#line 1052 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_lsh(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3632 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 254: /* constant: constant L_RSH constant  */
#line 1053 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_rsh(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3638 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 255: /* constant: constant '+' constant  */
#line 1054 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_add(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3644 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 256: /* constant: constant '-' constant  */
#line 1055 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_sub(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3650 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 257: /* constant: constant '*' constant  */
#line 1056 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_mul(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3656 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 258: /* constant: constant '%' constant  */
#line 1057 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_mod(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3662 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 259: /* constant: constant '/' constant  */
#line 1058 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_div(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3668 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 260: /* constant: '(' constant ')'  */
#line 1059 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { (yyval.number) = (yyvsp[-1].number); }
#line 3674 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 261: /* constant: L_NUMBER  */
#line 1060 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { (yyval.number) = (yyvsp[0].number); }
#line 3680 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 262: /* constant: '-' L_NUMBER  */
#line 1061 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_neg(&(yyval.number), (yyvsp[0].number)); }
#line 3686 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 263: /* constant: L_NOT L_NUMBER  */
#line 1062 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_not(&(yyval.number), (yyvsp[0].number)); }
#line 3692 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 264: /* constant: '~' L_NUMBER  */
#line 1063 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_compl(&(yyval.number), (yyvsp[0].number)); }
#line 3698 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;


#line 3702 "$REPO_ROOT$/build/src/grammar.autogen.cc"

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

#line 1066 "$REPO_ROOT$/src/compiler/internal/grammar.y"

