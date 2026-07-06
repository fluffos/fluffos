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
#line 18 "$REPO_ROOT$/src/compiler/internal/grammar.y"

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
  YYSYMBOL_L_TEMPLATE_HEAD = 6,            /* L_TEMPLATE_HEAD  */
  YYSYMBOL_L_TEMPLATE_MIDDLE = 7,          /* L_TEMPLATE_MIDDLE  */
  YYSYMBOL_L_TEMPLATE_TAIL = 8,            /* L_TEMPLATE_TAIL  */
  YYSYMBOL_L_BASIC_TYPE = 9,               /* L_BASIC_TYPE  */
  YYSYMBOL_L_TYPE_MODIFIER = 10,           /* L_TYPE_MODIFIER  */
  YYSYMBOL_L_DEFINED_NAME = 11,            /* L_DEFINED_NAME  */
  YYSYMBOL_L_IDENTIFIER = 12,              /* L_IDENTIFIER  */
  YYSYMBOL_L_EFUN = 13,                    /* L_EFUN  */
  YYSYMBOL_L_INC = 14,                     /* L_INC  */
  YYSYMBOL_L_DEC = 15,                     /* L_DEC  */
  YYSYMBOL_L_ASSIGN = 16,                  /* L_ASSIGN  */
  YYSYMBOL_L_LAND = 17,                    /* L_LAND  */
  YYSYMBOL_L_LOR = 18,                     /* L_LOR  */
  YYSYMBOL_L_QUESTION_QUESTION = 19,       /* L_QUESTION_QUESTION  */
  YYSYMBOL_L_LSH = 20,                     /* L_LSH  */
  YYSYMBOL_L_RSH = 21,                     /* L_RSH  */
  YYSYMBOL_L_ORDER = 22,                   /* L_ORDER  */
  YYSYMBOL_L_NOT = 23,                     /* L_NOT  */
  YYSYMBOL_L_IF = 24,                      /* L_IF  */
  YYSYMBOL_L_ELSE = 25,                    /* L_ELSE  */
  YYSYMBOL_L_SWITCH = 26,                  /* L_SWITCH  */
  YYSYMBOL_L_CASE = 27,                    /* L_CASE  */
  YYSYMBOL_L_DEFAULT = 28,                 /* L_DEFAULT  */
  YYSYMBOL_L_RANGE = 29,                   /* L_RANGE  */
  YYSYMBOL_L_DOT_DOT_DOT = 30,             /* L_DOT_DOT_DOT  */
  YYSYMBOL_L_WHILE = 31,                   /* L_WHILE  */
  YYSYMBOL_L_DO = 32,                      /* L_DO  */
  YYSYMBOL_L_FOR = 33,                     /* L_FOR  */
  YYSYMBOL_L_FOREACH = 34,                 /* L_FOREACH  */
  YYSYMBOL_L_IN = 35,                      /* L_IN  */
  YYSYMBOL_L_BREAK = 36,                   /* L_BREAK  */
  YYSYMBOL_L_CONTINUE = 37,                /* L_CONTINUE  */
  YYSYMBOL_L_RETURN = 38,                  /* L_RETURN  */
  YYSYMBOL_L_ARROW = 39,                   /* L_ARROW  */
  YYSYMBOL_L_DOT = 40,                     /* L_DOT  */
  YYSYMBOL_L_INHERIT = 41,                 /* L_INHERIT  */
  YYSYMBOL_L_COLON_COLON = 42,             /* L_COLON_COLON  */
  YYSYMBOL_L_OPTIONAL_DOT = 43,            /* L_OPTIONAL_DOT  */
  YYSYMBOL_L_DOT_OPTIONAL = 44,            /* L_DOT_OPTIONAL  */
  YYSYMBOL_L_ARRAY_OPEN = 45,              /* L_ARRAY_OPEN  */
  YYSYMBOL_L_MAPPING_OPEN = 46,            /* L_MAPPING_OPEN  */
  YYSYMBOL_L_FUNCTION_OPEN = 47,           /* L_FUNCTION_OPEN  */
  YYSYMBOL_L_NEW_FUNCTION_OPEN = 48,       /* L_NEW_FUNCTION_OPEN  */
  YYSYMBOL_L_SSCANF = 49,                  /* L_SSCANF  */
  YYSYMBOL_L_CATCH = 50,                   /* L_CATCH  */
  YYSYMBOL_L_ARRAY = 51,                   /* L_ARRAY  */
  YYSYMBOL_L_REF = 52,                     /* L_REF  */
  YYSYMBOL_L_PARSE_COMMAND = 53,           /* L_PARSE_COMMAND  */
  YYSYMBOL_L_TIME_EXPRESSION = 54,         /* L_TIME_EXPRESSION  */
  YYSYMBOL_L_CLASS = 55,                   /* L_CLASS  */
  YYSYMBOL_L_NEW = 56,                     /* L_NEW  */
  YYSYMBOL_L_PARAMETER = 57,               /* L_PARAMETER  */
  YYSYMBOL_L_TREE = 58,                    /* L_TREE  */
  YYSYMBOL_L_PREPROCESSOR_COMMAND = 59,    /* L_PREPROCESSOR_COMMAND  */
  YYSYMBOL_LOWER_THAN_ELSE = 60,           /* LOWER_THAN_ELSE  */
  YYSYMBOL_61_ = 61,                       /* '?'  */
  YYSYMBOL_62_ = 62,                       /* '|'  */
  YYSYMBOL_63_ = 63,                       /* '^'  */
  YYSYMBOL_64_ = 64,                       /* '&'  */
  YYSYMBOL_L_EQ = 65,                      /* L_EQ  */
  YYSYMBOL_L_NE = 66,                      /* L_NE  */
  YYSYMBOL_67_ = 67,                       /* '<'  */
  YYSYMBOL_68_ = 68,                       /* '+'  */
  YYSYMBOL_69_ = 69,                       /* '-'  */
  YYSYMBOL_70_ = 70,                       /* '*'  */
  YYSYMBOL_71_ = 71,                       /* '%'  */
  YYSYMBOL_72_ = 72,                       /* '/'  */
  YYSYMBOL_73_ = 73,                       /* '~'  */
  YYSYMBOL_74_ = 74,                       /* ';'  */
  YYSYMBOL_75_ = 75,                       /* '('  */
  YYSYMBOL_76_ = 76,                       /* ')'  */
  YYSYMBOL_77_ = 77,                       /* '{'  */
  YYSYMBOL_78_ = 78,                       /* '}'  */
  YYSYMBOL_79_ = 79,                       /* ','  */
  YYSYMBOL_80_ = 80,                       /* ':'  */
  YYSYMBOL_81_ = 81,                       /* '$'  */
  YYSYMBOL_82_ = 82,                       /* '['  */
  YYSYMBOL_83_ = 83,                       /* ']'  */
  YYSYMBOL_YYACCEPT = 84,                  /* $accept  */
  YYSYMBOL_loop_start = 85,                /* loop_start  */
  YYSYMBOL_foreach_start = 86,             /* foreach_start  */
  YYSYMBOL_block_start = 87,               /* block_start  */
  YYSYMBOL_special_context_start = 88,     /* special_context_start  */
  YYSYMBOL_dollar_start = 89,              /* dollar_start  */
  YYSYMBOL_all = 90,                       /* all  */
  YYSYMBOL_program = 91,                   /* program  */
  YYSYMBOL_opt_semicolon = 92,             /* opt_semicolon  */
  YYSYMBOL_def = 93,                       /* def  */
  YYSYMBOL_function = 94,                  /* function  */
  YYSYMBOL_95_1 = 95,                      /* @1  */
  YYSYMBOL_96_2 = 96,                      /* @2  */
  YYSYMBOL_block_or_semi = 97,             /* block_or_semi  */
  YYSYMBOL_statement = 98,                 /* statement  */
  YYSYMBOL_stmt_return = 99,               /* stmt_return  */
  YYSYMBOL_stmt_cond = 100,                /* stmt_cond  */
  YYSYMBOL_optional_else_part = 101,       /* optional_else_part  */
  YYSYMBOL_stmt_while = 102,               /* stmt_while  */
  YYSYMBOL_stmt_do = 103,                  /* stmt_do  */
  YYSYMBOL_stmt_for = 104,                 /* stmt_for  */
  YYSYMBOL_stmt_foreach = 105,             /* stmt_foreach  */
  YYSYMBOL_stmt_switch = 106,              /* stmt_switch  */
  YYSYMBOL_block = 107,                    /* block  */
  YYSYMBOL_statements = 108,               /* statements  */
  YYSYMBOL_local_decl_statement_header = 109, /* local_decl_statement_header  */
  YYSYMBOL_local_declaration_statement = 110, /* local_declaration_statement  */
  YYSYMBOL_local_decl_header = 111,        /* local_decl_header  */
  YYSYMBOL_local_declarations = 112,       /* local_declarations  */
  YYSYMBOL_comma_expr = 113,               /* comma_expr  */
  YYSYMBOL_expr = 114,                     /* expr  */
  YYSYMBOL_lvalue = 115,                   /* lvalue  */
  YYSYMBOL_primary_expr = 116,             /* primary_expr  */
  YYSYMBOL_call_open = 117,                /* call_open  */
  YYSYMBOL_function_call = 118,            /* function_call  */
  YYSYMBOL_sscanf = 119,                   /* sscanf  */
  YYSYMBOL_parse_command = 120,            /* parse_command  */
  YYSYMBOL_catch = 121,                    /* catch  */
  YYSYMBOL_time_expression = 122,          /* time_expression  */
  YYSYMBOL_tree = 123,                     /* tree  */
  YYSYMBOL_switch_header = 124,            /* switch_header  */
  YYSYMBOL_switch_block = 125,             /* switch_block  */
  YYSYMBOL_case = 126,                     /* case  */
  YYSYMBOL_case_label = 127,               /* case_label  */
  YYSYMBOL_for_expr = 128,                 /* for_expr  */
  YYSYMBOL_for_init = 129,                 /* for_init  */
  YYSYMBOL_foreach_var = 130,              /* foreach_var  */
  YYSYMBOL_foreach_vars = 131,             /* foreach_vars  */
  YYSYMBOL_type_decl = 132,                /* type_decl  */
  YYSYMBOL_class_header = 133,             /* class_header  */
  YYSYMBOL_member_list = 134,              /* member_list  */
  YYSYMBOL_135_3 = 135,                    /* $@3  */
  YYSYMBOL_member_name_list = 136,         /* member_name_list  */
  YYSYMBOL_member_name = 137,              /* member_name  */
  YYSYMBOL_class_init = 138,               /* class_init  */
  YYSYMBOL_opt_class_init = 139,           /* opt_class_init  */
  YYSYMBOL_type = 140,                     /* type  */
  YYSYMBOL_type_modifier_list = 141,       /* type_modifier_list  */
  YYSYMBOL_opt_basic_type = 142,           /* opt_basic_type  */
  YYSYMBOL_basic_type = 143,               /* basic_type  */
  YYSYMBOL_opt_atomic_type = 144,          /* opt_atomic_type  */
  YYSYMBOL_atomic_type = 145,              /* atomic_type  */
  YYSYMBOL_arg_type = 146,                 /* arg_type  */
  YYSYMBOL_cast = 147,                     /* cast  */
  YYSYMBOL_optional_star = 148,            /* optional_star  */
  YYSYMBOL_identifier = 149,               /* identifier  */
  YYSYMBOL_new_local_name = 150,           /* new_local_name  */
  YYSYMBOL_efun_override = 151,            /* efun_override  */
  YYSYMBOL_function_name = 152,            /* function_name  */
  YYSYMBOL_inheritance = 153,              /* inheritance  */
  YYSYMBOL_modifier_change = 154,          /* modifier_change  */
  YYSYMBOL_argument = 155,                 /* argument  */
  YYSYMBOL_argument_list = 156,            /* argument_list  */
  YYSYMBOL_param_decl = 157,               /* param_decl  */
  YYSYMBOL_optional_default_arg_value = 158, /* optional_default_arg_value  */
  YYSYMBOL_local_name_list = 159,          /* local_name_list  */
  YYSYMBOL_new_local_def = 160,            /* new_local_def  */
  YYSYMBOL_single_new_local_def = 161,     /* single_new_local_def  */
  YYSYMBOL_single_new_local_def_with_init = 162, /* single_new_local_def_with_init  */
  YYSYMBOL_name_list = 163,                /* name_list  */
  YYSYMBOL_new_name = 164,                 /* new_name  */
  YYSYMBOL_lambda_return_type = 165,       /* lambda_return_type  */
  YYSYMBOL_functional_open = 166,          /* functional_open  */
  YYSYMBOL_assoc_pair = 167,               /* assoc_pair  */
  YYSYMBOL_spread_expr = 168,              /* spread_expr  */
  YYSYMBOL_arg_list = 169,                 /* arg_list  */
  YYSYMBOL_opt_arg_list = 170,             /* opt_arg_list  */
  YYSYMBOL_pair_list = 171,                /* pair_list  */
  YYSYMBOL_opt_pair_list = 172,            /* opt_pair_list  */
  YYSYMBOL_lvalue_list = 173,              /* lvalue_list  */
  YYSYMBOL_expr_or_block = 174,            /* expr_or_block  */
  YYSYMBOL_ref = 175,                      /* ref  */
  YYSYMBOL_string = 176,                   /* string  */
  YYSYMBOL_template_literal = 177,         /* template_literal  */
  YYSYMBOL_template_parts = 178,           /* template_parts  */
  YYSYMBOL_string_like = 179,              /* string_like  */
  YYSYMBOL_string_literal = 180,           /* string_literal  */
  YYSYMBOL_string_const = 181,             /* string_const  */
  YYSYMBOL_number = 182,                   /* number  */
  YYSYMBOL_real = 183,                     /* real  */
  YYSYMBOL_constant = 184                  /* constant  */
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
#define YYLAST   1940

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  84
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  101
/* YYNRULES -- Number of rules.  */
#define YYNRULES  274
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  535

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   317


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
       2,     2,     2,     2,     2,     2,    81,    71,    64,     2,
      75,    76,    70,    68,    79,    69,     2,    72,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    80,    74,
      67,     2,     2,    61,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    82,     2,    83,    63,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    77,    62,    78,    73,     2,     2,     2,
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
      55,    56,    57,    58,    59,    60,    65,    66
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   220,   220,   221,   222,   223,   224,   233,   238,   239,
     244,   245,   256,   257,   258,   259,   260,   277,   279,   276,
     286,   287,   288,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   315,   316,   321,   326,   327,
     332,   338,   344,   350,   356,   368,   374,   376,   378,   380,
     386,   390,   396,   400,   402,   421,   422,   426,   427,   428,
     430,   431,   432,   433,   435,   436,   437,   439,   440,   441,
     442,   444,   445,   447,   448,   449,   450,   451,   453,   454,
     455,   456,   457,   458,   460,   461,   463,   464,   465,   466,
     467,   468,   473,   482,   483,   484,   485,   486,   487,   488,
     489,   492,   496,   497,   502,   504,   506,   511,   513,   515,
     517,   519,   521,   523,   525,   527,   530,   532,   537,   544,
     546,   548,   552,   553,   563,   576,   578,   580,   582,   584,
     586,   588,   590,   592,   603,   609,   615,   621,   627,   629,
     640,   646,   647,   648,   654,   656,   658,   660,   662,   669,
     670,   680,   681,   686,   688,   694,   695,   696,   701,   703,
     714,   720,   726,   727,   727,   733,   734,   739,   744,   749,
     750,   762,   767,   768,   773,   774,   779,   780,   785,   786,
     791,   792,   793,   798,   799,   804,   809,   810,   820,   821,
     826,   827,   832,   833,   843,   844,   845,   846,   851,   857,
     867,   869,   870,   876,   878,   888,   890,   892,   898,   899,
     904,   906,   912,   914,   920,   926,   937,   938,   943,   944,
     955,   963,   964,   974,   979,   980,   985,   987,   994,   995,
     996,  1001,  1003,  1010,  1011,  1012,  1017,  1018,  1023,  1024,
    1029,  1039,  1051,  1055,  1056,  1065,  1066,  1067,  1068,  1075,
    1076,  1082,  1083,  1084,  1088,  1092,  1098,  1099,  1100,  1101,
    1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116
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
  "L_EFUN", "L_INC", "L_DEC", "L_ASSIGN", "L_LAND", "L_LOR",
  "L_QUESTION_QUESTION", "L_LSH", "L_RSH", "L_ORDER", "L_NOT", "L_IF",
  "L_ELSE", "L_SWITCH", "L_CASE", "L_DEFAULT", "L_RANGE", "L_DOT_DOT_DOT",
  "L_WHILE", "L_DO", "L_FOR", "L_FOREACH", "L_IN", "L_BREAK", "L_CONTINUE",
  "L_RETURN", "L_ARROW", "L_DOT", "L_INHERIT", "L_COLON_COLON",
  "L_OPTIONAL_DOT", "L_DOT_OPTIONAL", "L_ARRAY_OPEN", "L_MAPPING_OPEN",
  "L_FUNCTION_OPEN", "L_NEW_FUNCTION_OPEN", "L_SSCANF", "L_CATCH",
  "L_ARRAY", "L_REF", "L_PARSE_COMMAND", "L_TIME_EXPRESSION", "L_CLASS",
  "L_NEW", "L_PARAMETER", "L_TREE", "L_PREPROCESSOR_COMMAND",
  "LOWER_THAN_ELSE", "'?'", "'|'", "'^'", "'&'", "L_EQ", "L_NE", "'<'",
  "'+'", "'-'", "'*'", "'%'", "'/'", "'~'", "';'", "'('", "')'", "'{'",
  "'}'", "','", "':'", "'$'", "'['", "']'", "$accept", "loop_start",
  "foreach_start", "block_start", "special_context_start", "dollar_start",
  "all", "program", "opt_semicolon", "def", "function", "@1", "@2",
  "block_or_semi", "statement", "stmt_return", "stmt_cond",
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

#define YYPACT_NINF (-399)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-236)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -399,    52,    53,  -399,    63,    25,  -399,  -399,  -399,   116,
      55,  -399,  -399,  -399,  -399,  -399,   135,  -399,   137,   118,
     133,  -399,    22,   167,  -399,  -399,  -399,   172,   204,   311,
    -399,  -399,  -399,  -399,    19,  -399,   116,  -399,    22,   215,
     -18,   184,   187,   212,  -399,  -399,  -399,   116,  1709,   206,
     137,  -399,    -7,  -399,    22,  -399,  -399,   225,   228,   137,
     324,  -399,  -399,  1709,   291,   -15,    12,   295,   297,   297,
    1709,   137,  1241,   554,  1709,  -399,   271,  -399,  -399,   290,
    -399,   296,  -399,   -10,  1709,  1709,   937,   302,   596,   334,
     160,  -399,  -399,  -399,  -399,  -399,  -399,  1709,   340,   312,
     312,   327,    77,   297,  -399,  -399,   226,   215,  -399,  -399,
     208,   336,  -399,  -399,  -399,   116,  -399,  1709,  1841,   137,
    -399,  1320,    37,  1399,  -399,   317,  -399,  -399,  -399,   523,
    -399,   305,   318,   900,  -399,   325,   322,   279,   596,   312,
    1709,   170,  1709,   170,   352,  1320,  1709,  -399,  -399,  -399,
    -399,    20,  1709,    62,   116,  -399,  1709,  1709,  1709,  1709,
    1709,  1709,  1709,  1709,  1709,  1709,  1709,  1709,  1709,  1709,
    1709,  1709,  1709,  1709,  -399,  -399,  1709,   137,   137,    72,
     328,  1089,  -399,   137,  1320,  1320,   208,  1709,   339,  -399,
    -399,  -399,  -399,  -399,   364,   116,  -399,   342,    -1,  -399,
    -399,   596,  1709,  -399,  -399,  -399,   353,  -399,  -399,  -399,
    1162,   354,  1709,   627,   357,  1709,   358,  1284,  1709,  -399,
    -399,  1363,  -399,   351,   360,    87,   700,   161,  -399,   361,
    1709,  1438,  1131,   979,   356,   356,   258,  1204,   623,   775,
    1759,   373,   373,   258,   303,   303,  -399,  -399,  -399,   596,
     312,  -399,  1709,  -399,  1709,   365,  1709,    29,  -399,   370,
     371,   372,   310,  -399,  -399,   380,  -399,  -399,   232,  1841,
    -399,  -399,  -399,   596,  -399,  -399,   596,  -399,  1709,   183,
    1709,  -399,  -399,  -399,  -399,    -3,   374,   376,   377,  -399,
     384,   392,   397,   398,  1472,  -399,   700,  -399,  -399,  -399,
    -399,  -399,  -399,  -399,  -399,   395,   116,   700,    98,   399,
    -399,   312,  -399,   209,  1709,  1320,    86,   115,  1709,    76,
     408,   312,  -399,  -399,   401,  1709,   403,   394,    14,  -399,
    -399,  1515,  -399,  1594,   217,   218,   700,  1709,  1709,  1709,
     858,  1010,   240,  -399,  -399,  -399,   128,  -399,  -399,   380,
     406,   409,  -399,  -399,  -399,  1320,  -399,   596,   426,  -399,
    -399,   131,   481,  -399,  1709,  -399,   132,  1320,  -399,  -399,
     456,  -399,  -399,  -399,  -399,  -399,   297,   430,  1709,  -399,
     137,  -399,  -399,   222,   236,   245,   474,   428,  -399,   434,
     116,   493,  -399,  -399,  -399,   442,   487,  -399,  -399,   509,
    -399,   116,   277,   460,  -399,  -399,  1709,  -399,   145,  1751,
     151,  -399,   470,  1709,   453,  -399,  1515,  -399,   467,   858,
    -399,  -399,   476,  1551,   380,  1709,   240,  1709,  1709,  -399,
     158,   469,   116,   779,  -399,  -399,   156,  1774,  -399,  -399,
    -399,  -399,   321,  -399,   485,  1709,   527,   858,  1709,   496,
    -399,   596,  -399,  1672,   596,  -399,   567,   185,   568,   569,
     185,    10,   506,  1868,  -399,   501,   779,   498,   779,  -399,
    -399,   502,  -399,   596,   858,  -399,  -399,   259,  1630,  -399,
    -399,   499,  -399,  -399,   448,   202,  -399,    78,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    78,    78,    78,
      78,  -399,  -399,  -399,  -399,  -399,  -399,   507,   504,   858,
    -399,  -399,  -399,   503,    78,   429,   429,   298,   696,  1006,
    1849,  1395,  1395,   298,   309,   309,  -399,  -399,  -399,  -399,
    -399,  -399,  -399,   858,  -399
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       9,     0,   172,     1,   172,    10,    12,    15,   162,   186,
     175,    14,    16,   173,    11,     8,   179,   187,     0,     0,
     216,   180,     0,     0,   199,   171,   174,     0,   176,     0,
     160,   163,   188,   189,   218,    13,   186,   249,     0,   251,
       0,   181,   182,     0,   177,   181,   182,   186,     0,     0,
       0,   217,     0,   250,     0,   198,   161,     0,   165,     0,
       0,   254,   255,     0,   220,    94,    95,     0,     0,     0,
       0,     0,     0,     0,     0,   221,     0,     5,   240,     0,
       5,     0,    96,     0,     0,     0,     0,     0,   219,     0,
      86,    93,    87,    88,    99,    89,   100,     0,     0,     0,
       0,     0,     0,     0,   245,   246,    97,   241,    90,    91,
     179,   218,   252,   253,   164,   186,   167,     0,     0,     0,
     124,     0,     0,     0,    79,    92,    80,    81,   195,   224,
     226,   229,     0,     0,   231,   234,     0,     0,    55,   222,
       0,     0,     0,     0,   124,     0,     0,     4,   138,    83,
      82,   180,     0,     0,   186,     6,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    84,    85,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,   179,     0,     0,    57,
     247,   248,   191,   190,   183,   186,   207,     0,   201,   203,
     166,    59,     0,   243,   242,   196,     0,   193,   192,   225,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   238,
     136,     0,   137,     0,     0,     0,     0,     0,    98,     0,
       0,    63,    62,    61,    71,    72,    69,     0,    64,    65,
      66,    67,    68,    70,    73,    74,    75,    76,    77,    58,
     102,   103,     0,   104,     0,     0,     0,     0,   197,     0,
       0,     0,     0,   119,   184,   205,    18,   202,   179,     0,
     129,   227,   123,   223,   232,   122,    56,   121,     0,     0,
       0,   169,   169,   126,   139,     0,     0,     0,     0,     2,
       0,     0,     0,     0,     0,    32,     0,    30,    24,    25,
      26,    27,    28,    29,    31,     0,   186,     0,     0,     0,
      50,     0,   185,     0,     0,     0,     0,     0,     0,     0,
       0,   107,   125,   130,     0,     0,     0,   208,     0,   204,
     244,   236,   239,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   179,    33,    34,    35,     0,    47,    45,     0,
       0,   210,    48,    23,    53,     0,   101,    60,     0,   105,
     106,     0,     0,   108,     0,   113,     0,     0,   118,   120,
       0,   206,    22,    21,    19,    20,     0,     0,     0,   127,
       0,   128,    49,     0,     0,     0,     0,   152,   153,     0,
     186,     0,   154,   155,   157,   158,     0,   156,    36,   212,
      51,   186,   179,     0,   132,   115,     0,   114,     0,    55,
       0,   109,     0,     0,   236,   134,   236,   170,     0,     0,
     140,     2,     0,     0,     0,     0,   179,     0,     0,   211,
       0,     0,   186,     0,    52,   133,     0,    55,   110,   117,
     112,   131,     0,   237,     0,     0,    38,     0,     0,     0,
     214,   215,   159,     0,   213,   271,     0,     0,     0,     0,
       0,     0,   150,   149,   148,     0,     0,     0,     0,   111,
     116,     0,   135,   168,     0,    37,    40,     0,     0,     3,
     273,     0,   272,   274,     0,     0,   144,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    54,   142,    44,   141,   209,    39,     0,     0,     0,
     147,   270,   146,     0,     0,   263,   264,   261,   256,   257,
     258,   259,   260,   262,   265,   266,   267,   268,   269,    41,
       2,    43,   145,     0,    42
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -399,  -398,  -399,  -399,   517,  -399,  -399,  -399,  -399,  -399,
    -399,  -399,  -399,  -399,  -283,  -399,  -399,  -399,  -399,  -399,
    -399,  -399,  -399,   -71,  -270,  -399,  -399,  -399,  -399,   -72,
     -42,   -65,   -59,   -41,  -399,  -399,  -399,  -399,  -399,  -399,
    -399,  -152,   180,  -346,  -376,  -399,   179,  -399,  -399,  -399,
    -399,  -399,   483,  -399,  -399,   337,  -399,   605,  -399,    -5,
    -399,  -399,  -322,  -399,    -2,    18,  -257,   546,  -399,  -399,
    -399,   435,  -399,   366,  -399,  -325,  -399,   281,  -399,   588,
    -399,  -399,  -399,   412,  -192,   439,   -99,  -399,  -399,  -140,
     505,   452,   541,   543,   382,  -399,   -22,   -21,  -399,  -399,
    1387
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,   340,   509,   226,   141,   230,     1,     2,    15,     5,
       6,    49,   328,   374,   296,   297,   298,   475,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   432,   402,   308,
     138,    89,    90,   184,    91,    92,    93,    94,    95,    96,
     309,   467,   468,   461,   388,   389,   395,   396,     7,     8,
      16,    47,    57,    58,   417,   334,     9,    10,    25,   194,
      27,    28,   195,    97,   349,    98,   196,    99,   100,    11,
      12,   197,   198,   199,   371,   350,   351,   397,   392,    19,
      20,   101,   102,   134,   130,   131,   132,   135,   136,   377,
     220,   103,   104,   105,   204,   106,   107,   462,   108,   109,
     463
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      39,    40,   137,   124,   126,    26,    88,    18,   327,   125,
     125,    31,   148,   117,   153,   372,    39,    52,   271,   390,
     390,   118,   206,   447,   121,    37,   347,  -188,   127,   267,
     129,   133,    39,   113,    50,    48,    34,   352,   189,   485,
     145,    43,   149,   150,   125,    59,   224,   449,    32,    33,
      54,   153,     3,    -7,  -189,   182,    55,   386,   320,   185,
     120,    54,   119,     4,    21,   146,   382,   147,   111,   112,
     219,   336,   219,     4,   225,   201,   429,   116,   268,   129,
     227,   154,   455,    32,    33,   259,   260,  -194,   373,   128,
     486,   147,   399,   207,   -17,  -220,    22,    38,   217,    14,
     221,   456,   508,   129,   390,   362,  -179,   465,   215,   257,
      23,   481,   321,    59,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   533,   271,   249,    24,   446,   205,   228,   513,
     208,   215,   129,   129,    21,   129,   279,   458,    32,    33,
     466,   459,   229,   514,   252,   215,   187,   188,   313,   363,
     269,    37,   455,   284,   476,   215,   215,   450,   129,   359,
     273,   133,   353,   276,   -92,   -92,   -92,   215,    41,    42,
     316,   456,   317,   466,   319,   466,    17,   457,    37,   455,
      29,   506,    35,   265,   215,   250,   251,   253,   360,   177,
     178,   258,   398,   179,   180,    37,   455,   215,   456,   315,
     215,   215,    36,    30,   405,   411,   358,    21,    53,   192,
     193,   310,   346,    44,   215,   456,   531,   458,   438,    37,
     215,   459,    63,   460,   440,   215,   331,   311,   333,   469,
     215,    21,   181,   192,   193,   218,   361,   147,   366,    21,
     534,   393,   394,   368,   458,  -178,   403,   375,   459,   332,
     460,  -188,   215,    29,  -189,   383,   384,   385,   412,   387,
     355,   458,   357,   129,   443,   459,   444,   460,   159,   160,
     367,   110,   512,   129,  -200,   356,    21,    29,   215,    56,
     408,   310,   410,   379,   381,    29,   380,   380,   419,   114,
      37,   215,   310,    63,   430,   431,    64,   115,    65,    66,
      67,   414,   420,   129,   502,   215,   504,   125,   487,   488,
     409,   421,    45,    46,   215,   129,   169,   170,   171,   172,
     173,   310,    29,   119,   436,   507,   416,   122,   215,    71,
     117,   442,    72,    73,    74,    75,   140,    77,   174,   175,
     176,   387,    48,    81,    82,    83,   177,   178,   215,   216,
     179,   180,   281,   282,   437,   142,   496,   497,   498,   499,
     500,   144,   123,   171,   172,   173,   477,   155,    87,   498,
     499,   500,   183,   451,   210,   453,   454,   120,   424,   325,
     326,   192,   193,   159,   160,   161,   211,   434,   418,   181,
     215,   471,   186,   473,   213,   214,   387,   223,    39,    60,
     254,    37,    61,    62,    63,   263,    78,    64,   266,    65,
      66,    67,    68,    69,   169,   170,   171,   172,   173,   270,
     272,    70,   318,   275,   277,    39,   283,   312,    39,    52,
     168,   169,   170,   171,   172,   173,   322,   323,   324,   337,
      71,   338,   339,    72,    73,    74,    75,    76,    77,   341,
      78,    79,    80,    39,    81,    82,    83,   342,   487,   488,
     489,   343,   344,   348,   370,   364,   354,    84,   147,   369,
     400,    85,    60,    86,    37,    61,    62,    63,   401,    87,
      64,   365,    65,    66,    67,    68,    69,   496,   497,   498,
     499,   500,   404,   413,    70,   422,   415,   215,   423,   425,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   426,   427,    71,   511,   428,    72,    73,    74,    75,
      76,    77,   376,    78,    79,    80,   435,    81,    82,    83,
     156,   157,   158,   159,   160,   161,   441,   445,   406,   464,
      84,   448,   474,   209,    85,    60,    86,    37,    61,    62,
      63,   472,    87,    64,   407,    65,    66,    67,    68,    69,
     478,   480,   482,   483,    54,   501,   503,    70,   505,   510,
     530,   529,   433,   532,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,    71,   143,   200,    72,
      73,    74,    75,    76,    77,   452,    78,    79,    80,    13,
      81,    82,    83,   156,   157,   158,   159,   160,   161,   335,
     139,   261,   391,    84,    51,   274,   262,    85,    60,    86,
      37,    61,    62,    63,   329,    87,    64,  -233,    65,    66,
      67,    68,    69,   159,   160,   161,   264,   190,   222,   191,
      70,   330,     0,     0,     0,     0,     0,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,    71,
       0,     0,    72,    73,    74,    75,    76,    77,     0,    78,
      79,    80,     0,    81,    82,    83,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,    84,     0,     0,     0,
      85,   285,    86,    37,    61,    62,    63,     0,    87,   151,
    -235,    65,    66,    67,    68,    69,   487,   488,   489,     0,
       0,     0,     0,    70,   286,     0,   287,     0,     0,     0,
       0,   288,   289,   290,   291,     0,   292,   293,   294,     0,
       0,     0,    71,     0,     0,    72,    73,    74,    75,    76,
      77,  -179,    78,    79,    80,    29,    81,    82,    83,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,    84,
       0,     0,     0,    85,   295,    86,     0,   147,   -46,     0,
      60,    87,    37,    61,    62,    63,     0,     0,    64,     0,
      65,    66,    67,    68,    69,   159,   160,   161,     0,     0,
       0,     0,    70,   286,     0,   287,   430,   431,     0,     0,
     288,   289,   290,   291,     0,   292,   293,   294,     0,     0,
       0,    71,     0,     0,    72,    73,    74,    75,    76,    77,
       0,    78,    79,    80,     0,    81,    82,    83,     0,   165,
     166,   167,   168,   169,   170,   171,   172,   173,    84,     0,
       0,     0,    85,   295,    86,     0,   147,  -143,     0,    60,
      87,    37,    61,    62,    63,     0,     0,    64,     0,    65,
      66,    67,    68,    69,     0,     0,     0,     0,     0,     0,
       0,    70,   286,     0,   287,     0,     0,     0,     0,   288,
     289,   290,   291,     0,   292,   293,   294,     0,     0,     0,
      71,     0,     0,    72,    73,    74,    75,    76,    77,     0,
      78,    79,    80,     0,    81,    82,    83,   156,   157,   158,
     159,   160,   161,     0,     0,     0,     0,    84,     0,     0,
       0,    85,   295,    86,     0,   147,     0,     0,    60,    87,
      37,    61,    62,    63,     0,     0,   151,     0,    65,    66,
      67,    68,    69,     0,     0,     0,     0,     0,     0,     0,
      70,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,     0,     0,     0,     0,     0,     0,    71,
     212,     0,    72,    73,    74,    75,    76,    77,  -179,    78,
      79,    80,    29,    81,    82,    83,   156,   157,     0,   159,
     160,   161,     0,     0,     0,     0,    84,   152,     0,     0,
      85,    60,    86,    37,    61,    62,    63,     0,    87,   151,
       0,    65,    66,    67,    68,    69,   487,   488,   489,     0,
       0,     0,     0,    70,     0,     0,     0,     0,     0,     0,
       0,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,    71,     0,     0,    72,    73,    74,    75,    76,
      77,  -179,    78,    79,    80,    29,    81,    82,    83,     0,
     492,   493,   494,   495,   496,   497,   498,   499,   500,    84,
       0,     0,     0,    85,  -151,    86,     0,     0,     0,     0,
      60,    87,    37,    61,    62,    63,     0,     0,    64,     0,
      65,    66,    67,    68,    69,     0,     0,     0,     0,     0,
       0,     0,    70,     0,     0,     0,     0,     0,   255,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    71,     0,     0,    72,    73,    74,    75,    76,    77,
       0,    78,    79,    80,     0,    81,    82,    83,   156,     0,
       0,   159,   160,   161,     0,     0,   256,     0,    84,     0,
       0,     0,    85,    60,    86,    37,    61,    62,    63,     0,
      87,    64,     0,    65,    66,    67,    68,    69,     0,     0,
       0,     0,     0,     0,     0,    70,     0,     0,     0,     0,
       0,     0,     0,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,    71,     0,     0,    72,    73,    74,
      75,    76,    77,     0,    78,    79,    80,     0,    81,    82,
      83,   156,   157,   158,   159,   160,   161,     0,     0,     0,
       0,    84,     0,     0,     0,    85,     0,    86,  -230,     0,
    -230,     0,    60,    87,    37,    61,    62,    63,     0,     0,
      64,     0,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,     0,     0,    70,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,     0,     0,     0,
       0,     0,     0,    71,   314,     0,    72,    73,    74,    75,
      76,    77,     0,    78,    79,    80,     0,    81,    82,    83,
       0,   156,   157,   158,   159,   160,   161,     0,     0,     0,
      84,     0,     0,     0,    85,     0,    86,     0,     0,  -228,
       0,    60,    87,    37,    61,    62,    63,     0,     0,    64,
       0,    65,    66,    67,    68,    69,     0,     0,     0,     0,
       0,     0,     0,    70,     0,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,     0,     0,     0,
       0,     0,    71,   278,     0,    72,    73,    74,    75,    76,
      77,     0,    78,    79,    80,     0,    81,    82,    83,     0,
     156,   157,   158,   159,   160,   161,     0,     0,     0,    84,
       0,     0,     0,    85,     0,    86,  -228,     0,     0,     0,
      60,    87,    37,    61,    62,    63,     0,     0,    64,     0,
      65,    66,    67,    68,    69,   487,   488,   489,     0,     0,
       0,     0,    70,     0,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,     0,     0,     0,     0,
       0,    71,   280,     0,    72,    73,    74,    75,    76,    77,
       0,    78,    79,    80,     0,    81,    82,    83,   159,   160,
     161,     0,   495,   496,   497,   498,   499,   500,    84,   152,
       0,     0,    85,    60,    86,    37,    61,    62,    63,     0,
      87,    64,     0,    65,    66,    67,    68,    69,     0,     0,
       0,     0,     0,     0,     0,    70,     0,     0,     0,     0,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,     0,     0,     0,    71,     0,     0,    72,    73,    74,
      75,    76,    77,     0,    78,    79,    80,     0,    81,    82,
      83,     0,   156,   157,   158,   159,   160,   161,     0,     0,
       0,    84,     0,     0,     0,    85,   345,    86,     0,     0,
       0,     0,    60,    87,    37,    61,    62,    63,     0,     0,
      64,     0,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,     0,     0,    70,     0,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,     0,     0,
       0,     0,     0,    71,   376,     0,    72,    73,    74,    75,
      76,    77,     0,    78,    79,    80,     0,    81,    82,    83,
       0,   156,   157,   158,   159,   160,   161,     0,     0,     0,
      84,     0,     0,     0,    85,  -151,    86,     0,     0,     0,
       0,    60,    87,    37,    61,    62,    63,     0,     0,    64,
       0,    65,    66,    67,    68,    69,     0,     0,     0,     0,
       0,     0,     0,    70,     0,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,     0,     0,     0,
       0,     0,    71,   378,     0,    72,    73,    74,    75,    76,
      77,     0,    78,    79,    80,     0,    81,    82,    83,   156,
     157,   158,   159,   160,   161,     0,     0,     0,     0,    84,
       0,     0,     0,    85,     0,    86,  -151,     0,     0,     0,
      60,    87,    37,    61,    62,    63,     0,     0,    64,     0,
      65,    66,    67,    68,    69,     0,     0,     0,     0,     0,
       0,     0,    70,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,     0,     0,     0,   479,     0,
       0,    71,     0,     0,    72,    73,    74,    75,    76,    77,
       0,    78,    79,    80,     0,    81,    82,    83,   156,   157,
     158,   159,   160,   161,     0,     0,     0,     0,    84,   159,
     160,   161,    85,     0,    86,     0,     0,     0,     0,     0,
      87,   156,   157,   158,   159,   160,   161,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   166,   167,   168,   169,   170,   171,
     172,   173,     0,     0,   439,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   484,   202,   203,
       0,     0,     0,     0,     0,     0,     0,   470,   156,   157,
     158,   159,   160,   161,     0,     0,     0,     0,     0,   487,
     488,   489,     0,     0,   515,   516,   517,   518,   519,   520,
     521,   522,   523,   524,   525,   526,   527,   528,   487,   488,
     489,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   484,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   493,   494,   495,   496,   497,   498,
     499,   500,     0,     0,     0,     0,     0,     0,     0,     0,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500
};

static const yytype_int16 yycheck[] =
{
      22,    22,    74,    68,    69,    10,    48,     9,   265,    68,
      69,    16,    83,    16,    86,     1,    38,    38,   210,   341,
     342,    63,   121,   421,    65,     3,   296,    42,    70,    30,
      72,    73,    54,    54,    36,    16,    18,   307,   103,    29,
      81,    23,    84,    85,   103,    47,   145,   423,    11,    12,
      68,   123,     0,     0,    42,    97,    74,   340,    29,   100,
      75,    68,    42,    10,     9,    75,   336,    77,    50,    76,
     141,    74,   143,    10,   146,   117,   401,    59,    79,   121,
     152,    86,     4,    11,    12,   184,   185,    75,    74,    71,
      80,    77,   349,    56,    75,    75,    41,    75,   140,    74,
     142,    23,   478,   145,   426,    29,    51,   432,    79,   181,
      55,   457,    83,   115,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   530,   325,   176,    80,   419,   119,    76,   485,
     122,    79,   184,   185,     9,   187,   218,    69,    11,    12,
     433,    73,   154,    75,    82,    79,    79,    80,   230,    83,
     202,     3,     4,    76,   447,    79,    79,   424,   210,    83,
     212,   213,    74,   215,    14,    15,    16,    79,    11,    12,
     252,    23,   254,   466,   256,   468,    70,    29,     3,     4,
      55,   474,    74,   195,    79,   177,   178,   179,    83,    39,
      40,   183,    74,    43,    44,     3,     4,    79,    23,   250,
      79,    79,    79,    78,    83,    83,   315,     9,     3,    11,
      12,   226,   294,    51,    79,    23,   509,    69,    83,     3,
      79,    73,     6,    75,    83,    79,   278,    76,   280,    83,
      79,     9,    82,    11,    12,    75,   318,    77,   320,     9,
     533,    11,    12,   324,    69,    51,   355,   328,    73,    76,
      75,    77,    79,    55,    77,   337,   338,   339,   367,   341,
     311,    69,   314,   315,   414,    73,   416,    75,    20,    21,
     321,    75,    80,   325,    76,    76,     9,    55,    79,    77,
     362,   296,   364,    76,    76,    55,    79,    79,    76,    74,
       3,    79,   307,     6,    27,    28,     9,    79,    11,    12,
      13,   376,    76,   355,   466,    79,   468,   376,    20,    21,
     362,    76,    11,    12,    79,   367,    68,    69,    70,    71,
      72,   336,    55,    42,   406,    76,   378,    42,    79,    42,
      16,   413,    45,    46,    47,    48,    75,    50,    14,    15,
      16,   423,    16,    56,    57,    58,    39,    40,    79,    80,
      43,    44,    11,    12,   406,    75,    68,    69,    70,    71,
      72,    75,    75,    70,    71,    72,   448,    75,    81,    70,
      71,    72,    42,   425,    79,   427,   428,    75,   390,    79,
      80,    11,    12,    20,    21,    22,    78,   402,   380,    82,
      79,    80,    75,   445,    79,    83,   478,    55,   430,     1,
      82,     3,     4,     5,     6,    76,    52,     9,    76,    11,
      12,    13,    14,    15,    68,    69,    70,    71,    72,    76,
      76,    23,    67,    76,    76,   457,    76,    76,   460,   460,
      67,    68,    69,    70,    71,    72,    76,    76,    76,    75,
      42,    75,    75,    45,    46,    47,    48,    49,    50,    75,
      52,    53,    54,   485,    56,    57,    58,    75,    20,    21,
      22,    74,    74,    78,    80,    67,    77,    69,    77,    76,
      74,    73,     1,    75,     3,     4,     5,     6,    79,    81,
       9,    83,    11,    12,    13,    14,    15,    68,    69,    70,
      71,    72,    76,    47,    23,    31,    76,    79,    74,    16,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    79,    35,    42,    76,    16,    45,    46,    47,    48,
      49,    50,    79,    52,    53,    54,    76,    56,    57,    58,
      17,    18,    19,    20,    21,    22,    76,    80,    67,    80,
      69,    75,    25,    30,    73,     1,    75,     3,     4,     5,
       6,    76,    81,     9,    83,    11,    12,    13,    14,    15,
      74,     4,     4,     4,    68,    74,    78,    23,    76,    80,
      76,    74,   402,    80,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    42,    80,   115,    45,
      46,    47,    48,    49,    50,   426,    52,    53,    54,     4,
      56,    57,    58,    17,    18,    19,    20,    21,    22,   282,
      74,   186,   341,    69,    36,   213,   187,    73,     1,    75,
       3,     4,     5,     6,   268,    81,     9,    83,    11,    12,
      13,    14,    15,    20,    21,    22,   194,   106,   143,   106,
      23,   269,    -1,    -1,    -1,    -1,    -1,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    42,
      -1,    -1,    45,    46,    47,    48,    49,    50,    -1,    52,
      53,    54,    -1,    56,    57,    58,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    69,    -1,    -1,    -1,
      73,     1,    75,     3,     4,     5,     6,    -1,    81,     9,
      83,    11,    12,    13,    14,    15,    20,    21,    22,    -1,
      -1,    -1,    -1,    23,    24,    -1,    26,    -1,    -1,    -1,
      -1,    31,    32,    33,    34,    -1,    36,    37,    38,    -1,
      -1,    -1,    42,    -1,    -1,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    69,
      -1,    -1,    -1,    73,    74,    75,    -1,    77,    78,    -1,
       1,    81,     3,     4,     5,     6,    -1,    -1,     9,    -1,
      11,    12,    13,    14,    15,    20,    21,    22,    -1,    -1,
      -1,    -1,    23,    24,    -1,    26,    27,    28,    -1,    -1,
      31,    32,    33,    34,    -1,    36,    37,    38,    -1,    -1,
      -1,    42,    -1,    -1,    45,    46,    47,    48,    49,    50,
      -1,    52,    53,    54,    -1,    56,    57,    58,    -1,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    69,    -1,
      -1,    -1,    73,    74,    75,    -1,    77,    78,    -1,     1,
      81,     3,     4,     5,     6,    -1,    -1,     9,    -1,    11,
      12,    13,    14,    15,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    23,    24,    -1,    26,    -1,    -1,    -1,    -1,    31,
      32,    33,    34,    -1,    36,    37,    38,    -1,    -1,    -1,
      42,    -1,    -1,    45,    46,    47,    48,    49,    50,    -1,
      52,    53,    54,    -1,    56,    57,    58,    17,    18,    19,
      20,    21,    22,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    73,    74,    75,    -1,    77,    -1,    -1,     1,    81,
       3,     4,     5,     6,    -1,    -1,     9,    -1,    11,    12,
      13,    14,    15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      23,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      80,    -1,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    17,    18,    -1,    20,
      21,    22,    -1,    -1,    -1,    -1,    69,    70,    -1,    -1,
      73,     1,    75,     3,     4,     5,     6,    -1,    81,     9,
      -1,    11,    12,    13,    14,    15,    20,    21,    22,    -1,
      -1,    -1,    -1,    23,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    42,    -1,    -1,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    -1,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    69,
      -1,    -1,    -1,    73,    74,    75,    -1,    -1,    -1,    -1,
       1,    81,     3,     4,     5,     6,    -1,    -1,     9,    -1,
      11,    12,    13,    14,    15,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    23,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    -1,    -1,    45,    46,    47,    48,    49,    50,
      -1,    52,    53,    54,    -1,    56,    57,    58,    17,    -1,
      -1,    20,    21,    22,    -1,    -1,    67,    -1,    69,    -1,
      -1,    -1,    73,     1,    75,     3,     4,     5,     6,    -1,
      81,     9,    -1,    11,    12,    13,    14,    15,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    23,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    42,    -1,    -1,    45,    46,    47,
      48,    49,    50,    -1,    52,    53,    54,    -1,    56,    57,
      58,    17,    18,    19,    20,    21,    22,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    73,    -1,    75,    76,    -1,
      78,    -1,     1,    81,     3,     4,     5,     6,    -1,    -1,
       9,    -1,    11,    12,    13,    14,    15,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    23,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    80,    -1,    45,    46,    47,    48,
      49,    50,    -1,    52,    53,    54,    -1,    56,    57,    58,
      -1,    17,    18,    19,    20,    21,    22,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    73,    -1,    75,    -1,    -1,    78,
      -1,     1,    81,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    11,    12,    13,    14,    15,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    23,    -1,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    -1,    -1,    -1,
      -1,    -1,    42,    79,    -1,    45,    46,    47,    48,    49,
      50,    -1,    52,    53,    54,    -1,    56,    57,    58,    -1,
      17,    18,    19,    20,    21,    22,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    73,    -1,    75,    76,    -1,    -1,    -1,
       1,    81,     3,     4,     5,     6,    -1,    -1,     9,    -1,
      11,    12,    13,    14,    15,    20,    21,    22,    -1,    -1,
      -1,    -1,    23,    -1,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    -1,    -1,    -1,    -1,
      -1,    42,    79,    -1,    45,    46,    47,    48,    49,    50,
      -1,    52,    53,    54,    -1,    56,    57,    58,    20,    21,
      22,    -1,    67,    68,    69,    70,    71,    72,    69,    70,
      -1,    -1,    73,     1,    75,     3,     4,     5,     6,    -1,
      81,     9,    -1,    11,    12,    13,    14,    15,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    23,    -1,    -1,    -1,    -1,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    -1,    -1,    -1,    42,    -1,    -1,    45,    46,    47,
      48,    49,    50,    -1,    52,    53,    54,    -1,    56,    57,
      58,    -1,    17,    18,    19,    20,    21,    22,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    73,    74,    75,    -1,    -1,
      -1,    -1,     1,    81,     3,     4,     5,     6,    -1,    -1,
       9,    -1,    11,    12,    13,    14,    15,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    23,    -1,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    -1,    -1,
      -1,    -1,    -1,    42,    79,    -1,    45,    46,    47,    48,
      49,    50,    -1,    52,    53,    54,    -1,    56,    57,    58,
      -1,    17,    18,    19,    20,    21,    22,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    73,    74,    75,    -1,    -1,    -1,
      -1,     1,    81,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    11,    12,    13,    14,    15,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    23,    -1,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    -1,    -1,    -1,
      -1,    -1,    42,    79,    -1,    45,    46,    47,    48,    49,
      50,    -1,    52,    53,    54,    -1,    56,    57,    58,    17,
      18,    19,    20,    21,    22,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    73,    -1,    75,    76,    -1,    -1,    -1,
       1,    81,     3,     4,     5,     6,    -1,    -1,     9,    -1,
      11,    12,    13,    14,    15,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    23,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    -1,    -1,    -1,    76,    -1,
      -1,    42,    -1,    -1,    45,    46,    47,    48,    49,    50,
      -1,    52,    53,    54,    -1,    56,    57,    58,    17,    18,
      19,    20,    21,    22,    -1,    -1,    -1,    -1,    69,    20,
      21,    22,    73,    -1,    75,    -1,    -1,    -1,    -1,    -1,
      81,    17,    18,    19,    20,    21,    22,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    65,    66,    67,    68,    69,    70,
      71,    72,    -1,    -1,    83,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,   460,     7,     8,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    83,    17,    18,
      19,    20,    21,    22,    -1,    -1,    -1,    -1,    -1,    20,
      21,    22,    -1,    -1,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,    20,    21,
      22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   514,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    65,    66,    67,    68,    69,    70,
      71,    72,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    90,    91,     0,    10,    93,    94,   132,   133,   140,
     141,   153,   154,   141,    74,    92,   134,    70,   148,   163,
     164,     9,    41,    55,    80,   142,   143,   144,   145,    55,
      78,   143,    11,    12,   149,    74,    79,     3,    75,   180,
     181,    11,    12,   149,    51,    11,    12,   135,    16,    95,
     148,   163,   181,     3,    68,    74,    77,   136,   137,   148,
       1,     4,     5,     6,     9,    11,    12,    13,    14,    15,
      23,    42,    45,    46,    47,    48,    49,    50,    52,    53,
      54,    56,    57,    58,    69,    73,    75,    81,   114,   115,
     116,   118,   119,   120,   121,   122,   123,   147,   149,   151,
     152,   165,   166,   175,   176,   177,   179,   180,   182,   183,
      75,   149,    76,   181,    74,    79,   149,    16,   114,    42,
      75,   117,    42,    75,   115,   116,   115,   114,   149,   114,
     168,   169,   170,   114,   167,   171,   172,   113,   114,   151,
      75,    88,    75,    88,    75,   117,    75,    77,   107,   114,
     114,     9,    70,   113,   143,    75,    17,    18,    19,    20,
      21,    22,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    14,    15,    16,    39,    40,    43,
      44,    82,   114,    42,   117,   117,    75,    79,    80,   115,
     176,   177,    11,    12,   143,   146,   150,   155,   156,   157,
     136,   114,     7,     8,   178,   149,   170,    56,   149,    30,
      79,    78,    80,    79,    83,    79,    80,   114,    75,   107,
     174,   114,   174,    55,   170,   113,    87,   113,    76,   148,
      89,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     149,   149,    82,   149,    82,    29,    67,   113,   149,   170,
     170,   155,   169,    76,   175,   148,    76,    30,    79,   114,
      76,   168,    76,   114,   167,    76,   114,    76,    79,   113,
      79,    11,    12,    76,    76,     1,    24,    26,    31,    32,
      33,    34,    36,    37,    38,    74,    98,    99,   100,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   113,   124,
     143,    76,    76,   113,    80,   117,   113,   113,    67,   113,
      29,    83,    76,    76,    76,    79,    80,   150,    96,   157,
     178,   114,    76,   114,   139,   139,    74,    75,    75,    75,
      85,    75,    75,    74,    74,    74,   113,   108,    78,   148,
     159,   160,   108,    74,    77,   117,    76,   114,   170,    83,
      83,   113,    29,    83,    67,    83,   113,   117,   107,    76,
      80,   158,     1,    74,    97,   107,    79,   173,    79,    76,
      79,    76,   108,   113,   113,   113,    98,   113,   128,   129,
     146,   161,   162,    11,    12,   130,   131,   161,    74,   150,
      74,    79,   112,   170,    76,    83,    67,    83,   113,   114,
     113,    83,   170,    47,   115,    76,   114,   138,   149,    76,
      76,    76,    31,    74,   148,    16,    79,    35,    16,   159,
      27,    28,   111,   126,   143,    76,   113,   114,    83,    83,
      83,    76,   113,   173,   173,    80,    98,    85,    75,   128,
     150,   114,   130,   114,   114,     4,    23,    29,    69,    73,
      75,   127,   181,   184,    80,   159,    98,   125,   126,    83,
      83,    80,    76,   114,    25,   101,    98,   113,    74,    76,
       4,   127,     4,     4,   184,    29,    80,    20,    21,    22,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    74,   125,    78,   125,    76,    98,    76,   128,    86,
      80,    76,    80,   127,    75,   184,   184,   184,   184,   184,
     184,   184,   184,   184,   184,   184,   184,   184,   184,    74,
      76,    98,    80,    85,    98
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    84,    85,    86,    87,    88,    89,    90,    91,    91,
      92,    92,    93,    93,    93,    93,    93,    95,    96,    94,
      97,    97,    97,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    99,    99,   100,   101,   101,
     102,   103,   104,   105,   106,   107,   108,   108,   108,   108,
     109,   110,   111,   112,   112,   113,   113,   114,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   115,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   117,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   119,   120,   121,   122,   123,   123,
     124,   125,   125,   125,   126,   126,   126,   126,   126,   127,
     127,   128,   128,   129,   129,   130,   130,   130,   131,   131,
     132,   133,   134,   135,   134,   136,   136,   137,   138,   139,
     139,   140,   141,   141,   142,   142,   143,   143,   144,   144,
     145,   145,   145,   146,   146,   147,   148,   148,   149,   149,
     150,   150,   151,   151,   152,   152,   152,   152,   153,   154,
     155,   155,   155,   156,   156,   157,   157,   157,   158,   158,
     159,   159,   160,   160,   161,   162,   163,   163,   164,   164,
     165,   166,   166,   167,   168,   168,   169,   169,   170,   170,
     170,   171,   171,   172,   172,   172,   173,   173,   174,   174,
     175,   176,   177,   178,   178,   179,   179,   179,   179,   180,
     180,   181,   181,   181,   182,   183,   184,   184,   184,   184,
     184,   184,   184,   184,   184,   184,   184,   184,   184,   184,
     184,   184,   184,   184,   184
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
       1,     5,     3,     3,     3,     5,     5,     4,     5,     6,
       7,     8,     7,     5,     6,     6,     8,     7,     5,     3,
       5,     4,     4,     4,     1,     4,     4,     6,     6,     4,
       4,     7,     6,     7,     7,     9,     3,     3,     2,     4,
       4,     2,     2,     0,     3,     5,     4,     4,     2,     1,
       1,     0,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     4,     0,     0,     5,     1,     3,     2,     3,     0,
       3,     2,     0,     2,     1,     0,     1,     2,     1,     0,
       1,     2,     2,     1,     2,     4,     0,     1,     1,     1,
       1,     1,     3,     3,     1,     2,     3,     3,     4,     2,
       0,     1,     2,     1,     3,     2,     4,     1,     0,     5,
       1,     3,     2,     4,     3,     3,     1,     3,     2,     4,
       1,     1,     2,     3,     1,     2,     1,     3,     0,     1,
       2,     1,     3,     0,     1,     2,     0,     3,     1,     3,
       1,     1,     3,     1,     3,     1,     1,     2,     2,     1,
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
#line 220 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { (yyval.number) = rule_loop_open(); }
#line 2404 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 3: /* foreach_start: %empty  */
#line 221 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                      { (yyval.number) = rule_foreach_open(); }
#line 2410 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 4: /* block_start: %empty  */
#line 222 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.number) = rule_block_open(); }
#line 2416 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 5: /* special_context_start: %empty  */
#line 223 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { (yyval.number) = rule_special_context_open(); }
#line 2422 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 6: /* dollar_start: %empty  */
#line 224 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                     { (yyval.contextp) = rule_dollar_open(); }
#line 2428 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 7: /* all: program  */
#line 233 "$REPO_ROOT$/src/compiler/internal/grammar.y"
           { rule_program((yyvsp[0].node)); }
#line 2434 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 8: /* program: program def opt_semicolon  */
#line 238 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_program_append(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 2440 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 9: /* program: %empty  */
#line 239 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.node) = 0; }
#line 2446 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 11: /* opt_semicolon: ';'  */
#line 245 "$REPO_ROOT$/src/compiler/internal/grammar.y"
         { rule_opt_semicolon(); }
#line 2452 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 13: /* def: type name_list ';'  */
#line 257 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_def_global_var((yyvsp[-2].number)); (yyval.node) = 0; }
#line 2458 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 17: /* @1: %empty  */
#line 277 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_func_type((yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].string)); }
#line 2464 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 18: /* @2: %empty  */
#line 279 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_func_proto((yyvsp[-3].number), (yyvsp[-5].number), &(yyvsp[-4].string), (yyvsp[-1].argument)); }
#line 2470 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 19: /* function: type optional_star identifier @1 '(' argument ')' @2 block_or_semi  */
#line 281 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_func(&(yyval.node), (yyvsp[-5].number), (yyvsp[-7].number), (yyvsp[-6].string), (yyvsp[-3].argument), &(yyvsp[-1].number), &(yyvsp[0].node)); }
#line 2476 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 20: /* block_or_semi: block  */
#line 286 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = rule_block_or_semi((yyvsp[0].decl).node); }
#line 2482 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 21: /* block_or_semi: ';'  */
#line 287 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2488 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 22: /* block_or_semi: error  */
#line 288 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2494 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 23: /* statement: comma_expr ';'  */
#line 298 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_expr((yyvsp[-1].node)); }
#line 2500 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 27: /* statement: stmt_for  */
#line 302 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2506 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 28: /* statement: stmt_foreach  */
#line 303 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2512 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 31: /* statement: block  */
#line 306 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_compound_stmt((yyvsp[0].decl)); }
#line 2518 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 32: /* statement: ';'  */
#line 307 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = 0; }
#line 2524 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 33: /* statement: L_BREAK ';'  */
#line 308 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_break(); }
#line 2530 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 34: /* statement: L_CONTINUE ';'  */
#line 309 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_statement_continue(); }
#line 2536 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 35: /* stmt_return: L_RETURN ';'  */
#line 315 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_return_void(&(yyval.node)); }
#line 2542 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 36: /* stmt_return: L_RETURN comma_expr ';'  */
#line 316 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_return_expr(&(yyval.node), (yyvsp[-1].node)); }
#line 2548 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 37: /* stmt_cond: L_IF '(' comma_expr ')' statement optional_else_part  */
#line 322 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_cond(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 2554 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 38: /* optional_else_part: %empty  */
#line 326 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                { (yyval.node) = 0; }
#line 2560 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 39: /* optional_else_part: L_ELSE statement  */
#line 327 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                 { (yyval.node) = (yyvsp[0].node); }
#line 2566 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 40: /* stmt_while: L_WHILE '(' comma_expr ')' loop_start statement  */
#line 333 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_while(&(yyval.node), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2572 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 41: /* stmt_do: L_DO loop_start statement L_WHILE '(' comma_expr ')' ';'  */
#line 339 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_do(&(yyval.node), (yyvsp[-5].node), (yyvsp[-2].node), (yyvsp[-6].number)); }
#line 2578 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 42: /* stmt_for: L_FOR '(' for_init ';' for_expr ';' for_expr ')' loop_start statement  */
#line 345 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for(&(yyval.decl), &(yyvsp[-7].decl), (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2584 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 43: /* stmt_foreach: L_FOREACH '(' foreach_vars L_IN expr ')' foreach_start statement  */
#line 351 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach(&(yyval.decl), &(yyvsp[-5].decl), (yyvsp[-3].node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 2590 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 44: /* stmt_switch: switch_header '{' local_declarations case switch_block '}'  */
#line 357 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_switch(&(yyval.node), (yyvsp[-5].switch_header).expr, &(yyvsp[-3].decl), (yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[-5].switch_header).saved_context, (yyvsp[-5].switch_header).saved_cases_size); }
#line 2596 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 45: /* block: '{' block_start statements '}'  */
#line 369 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block(&(yyval.decl), (yyvsp[-1].decl).node, (yyvsp[-2].number)); }
#line 2602 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 46: /* statements: %empty  */
#line 375 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_empty(&(yyval.decl)); }
#line 2608 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 47: /* statements: statement statements  */
#line 377 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_stmt(&(yyval.decl), (yyvsp[-1].node), &(yyvsp[0].decl)); }
#line 2614 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 48: /* statements: local_declaration_statement statements  */
#line 379 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_decl(&(yyval.decl), &(yyvsp[-1].decl), &(yyvsp[0].decl)); }
#line 2620 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 49: /* statements: error ';' statements  */
#line 381 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_block_statements_error(&(yyval.decl), &(yyvsp[0].decl)); }
#line 2626 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 50: /* local_decl_statement_header: basic_type  */
#line 386 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { rule_local_declaration_statement_set_type((yyvsp[0].number)); (yyval.number) = (yyvsp[0].number); }
#line 2632 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 51: /* local_declaration_statement: local_decl_statement_header local_name_list ';'  */
#line 391 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_declaration_statement(&(yyval.decl), &(yyvsp[-1].decl)); }
#line 2638 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 52: /* local_decl_header: basic_type  */
#line 396 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { rule_local_declarations_set_type((yyvsp[0].number)); (yyval.number) = (yyvsp[0].number); }
#line 2644 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 53: /* local_declarations: %empty  */
#line 401 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.decl).node = 0; (yyval.decl).num = 0; }
#line 2650 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 54: /* local_declarations: local_declarations local_decl_header local_name_list ';'  */
#line 403 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_declarations(&(yyval.decl), &(yyvsp[-3].decl), &(yyvsp[-1].decl)); }
#line 2656 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 56: /* comma_expr: comma_expr ',' expr  */
#line 422 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                { rule_comma_expr(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2662 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 57: /* expr: ref lvalue  */
#line 426 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_ref(&(yyval.node), (yyvsp[0].node)); }
#line 2668 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 58: /* expr: lvalue L_ASSIGN expr  */
#line 427 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_assign(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 2674 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 59: /* expr: error L_ASSIGN expr  */
#line 428 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                      { rule_expr_assign_error(&(yyval.node), (yyvsp[0].node)); }
#line 2680 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 60: /* expr: expr '?' expr ':' expr  */
#line 430 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                            { rule_expr_ternary(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2686 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 61: /* expr: expr L_QUESTION_QUESTION expr  */
#line 431 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_nullish(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2692 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 62: /* expr: expr L_LOR expr  */
#line 432 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_lor(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2698 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 63: /* expr: expr L_LAND expr  */
#line 433 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_expr_land(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2704 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 64: /* expr: expr '|' expr  */
#line 435 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_or(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2710 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 65: /* expr: expr '^' expr  */
#line 436 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_xor(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2716 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 66: /* expr: expr '&' expr  */
#line 437 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                              { rule_expr_and(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2722 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 67: /* expr: expr L_EQ expr  */
#line 439 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_eq(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2728 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 68: /* expr: expr L_NE expr  */
#line 440 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_ne(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2734 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 69: /* expr: expr L_ORDER expr  */
#line 441 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_order(&(yyval.node), (yyvsp[-2].node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 2740 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 70: /* expr: expr '<' expr  */
#line 442 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { rule_expr_lt(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2746 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 71: /* expr: expr L_LSH expr  */
#line 444 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                               { rule_expr_lsh(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2752 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 72: /* expr: expr L_RSH expr  */
#line 445 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                               { rule_expr_rsh(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2758 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 73: /* expr: expr '+' expr  */
#line 447 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_add(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2764 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 74: /* expr: expr '-' expr  */
#line 448 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_sub(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2770 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 75: /* expr: expr '*' expr  */
#line 449 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_mul(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2776 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 76: /* expr: expr '%' expr  */
#line 450 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_mod(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2782 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 77: /* expr: expr '/' expr  */
#line 451 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_expr_div(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2788 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 78: /* expr: cast expr  */
#line 453 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_expr_cast(&(yyval.node), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 2794 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 79: /* expr: L_INC lvalue  */
#line 454 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                               { rule_expr_pre_inc(&(yyval.node), (yyvsp[0].node)); }
#line 2800 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 80: /* expr: L_DEC lvalue  */
#line 455 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                               { rule_expr_pre_dec(&(yyval.node), (yyvsp[0].node)); }
#line 2806 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 81: /* expr: L_NOT expr  */
#line 456 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_expr_not(&(yyval.node), (yyvsp[0].node)); }
#line 2812 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 82: /* expr: '~' expr  */
#line 457 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_expr_compl(&(yyval.node), (yyvsp[0].node)); }
#line 2818 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 83: /* expr: '-' expr  */
#line 458 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { rule_expr_neg(&(yyval.node), (yyvsp[0].node)); }
#line 2824 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 84: /* expr: lvalue L_INC  */
#line 460 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_expr_post_inc(&(yyval.node), (yyvsp[-1].node)); }
#line 2830 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 85: /* expr: lvalue L_DEC  */
#line 461 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_expr_post_dec(&(yyval.node), (yyvsp[-1].node)); }
#line 2836 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 92: /* lvalue: primary_expr  */
#line 473 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                { (yyval.node) = rule_lvalue((yyvsp[0].node)); }
#line 2842 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 94: /* primary_expr: L_DEFINED_NAME  */
#line 483 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_defined_name(&(yyval.node), (yyvsp[0].ihe)); }
#line 2848 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 95: /* primary_expr: L_IDENTIFIER  */
#line 484 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_identifier(&(yyval.node), (yyvsp[0].string)); }
#line 2854 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 96: /* primary_expr: L_PARAMETER  */
#line 485 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_primary_expr_parameter(&(yyval.node), (yyvsp[0].number)); }
#line 2860 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 98: /* primary_expr: '(' comma_expr ')'  */
#line 487 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { (yyval.node) = (yyvsp[-1].node); }
#line 2866 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 101: /* primary_expr: '$' '(' dollar_start comma_expr ')'  */
#line 493 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_dollar_expr(&(yyval.node), (yyvsp[-2].contextp), (yyvsp[-1].node)); }
#line 2872 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 102: /* primary_expr: primary_expr L_ARROW identifier  */
#line 496 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                 { rule_primary_expr_member_arrow(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 2878 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 103: /* primary_expr: primary_expr L_DOT identifier  */
#line 497 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                 { rule_primary_expr_member_dot(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 2884 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 104: /* primary_expr: primary_expr L_OPTIONAL_DOT identifier  */
#line 503 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_member_optional(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].string)); }
#line 2890 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 105: /* primary_expr: primary_expr L_OPTIONAL_DOT '[' comma_expr ']'  */
#line 505 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_optional(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 2896 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 106: /* primary_expr: primary_expr L_DOT_OPTIONAL '[' comma_expr ']'  */
#line 507 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_optional(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 2902 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 107: /* primary_expr: primary_expr '[' comma_expr ']'  */
#line 512 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2908 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 108: /* primary_expr: primary_expr '[' '<' comma_expr ']'  */
#line 514 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_index_r(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 2914 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 109: /* primary_expr: primary_expr '[' comma_expr L_RANGE comma_expr ']'  */
#line 516 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nn(&(yyval.node), (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2920 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 110: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE comma_expr ']'  */
#line 518 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rn(&(yyval.node), (yyvsp[-6].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2926 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 111: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE '<' comma_expr ']'  */
#line 520 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rr(&(yyval.node), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 2932 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 112: /* primary_expr: primary_expr '[' comma_expr L_RANGE '<' comma_expr ']'  */
#line 522 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nr(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 2938 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 113: /* primary_expr: primary_expr '[' comma_expr L_RANGE ']'  */
#line 524 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_ne(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node)); }
#line 2944 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 114: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE ']'  */
#line 526 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_re(&(yyval.node), (yyvsp[-5].node), (yyvsp[-2].node)); }
#line 2950 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 115: /* primary_expr: primary_expr '[' L_RANGE '<' comma_expr ']'  */
#line 528 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_nr(&(yyval.node), (yyvsp[-5].node), 0, (yyvsp[-1].node)); }
#line 2956 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 116: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE '<' expr ']'  */
#line 531 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rr(&(yyval.node), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-1].node)); }
#line 2962 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 117: /* primary_expr: primary_expr '[' '<' comma_expr L_RANGE expr ']'  */
#line 533 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_range_rn(&(yyval.node), (yyvsp[-6].node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2968 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 118: /* primary_expr: lambda_return_type '(' argument ')' block  */
#line 538 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_anon_func(&(yyval.node), &(yyvsp[-4].func_block), &(yyvsp[-2].argument), &(yyvsp[0].decl)); }
#line 2974 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 119: /* primary_expr: functional_open ':' ')'  */
#line 545 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_1(&(yyval.node), (yyvsp[-2].number)); }
#line 2980 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 120: /* primary_expr: functional_open ',' arg_list ':' ')'  */
#line 547 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_2(&(yyval.node), (yyvsp[-4].number), (yyvsp[-2].node)); }
#line 2986 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 121: /* primary_expr: L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 549 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_primary_expr_functional_3(&(yyval.node), (yyvsp[-2].node)); }
#line 2992 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 122: /* primary_expr: L_MAPPING_OPEN opt_pair_list ']' ')'  */
#line 552 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { rule_primary_expr_mapping(&(yyval.node), (yyvsp[-2].node)); }
#line 2998 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 123: /* primary_expr: L_ARRAY_OPEN opt_arg_list '}' ')'  */
#line 553 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { rule_primary_expr_array(&(yyval.node), (yyvsp[-2].node)); }
#line 3004 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 124: /* call_open: '('  */
#line 563 "$REPO_ROOT$/src/compiler/internal/grammar.y"
       { rule_call_open(&(yyval.call_open).saved_context, &(yyval.call_open).saved_refs); }
#line 3010 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 125: /* function_call: efun_override call_open opt_arg_list ')'  */
#line 577 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_efun(&(yyval.node), (yyvsp[-3].number), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3016 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 126: /* function_call: L_NEW call_open opt_arg_list ')'  */
#line 579 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new(&(yyval.node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3022 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 127: /* function_call: L_NEW '(' L_CLASS L_DEFINED_NAME opt_class_init ')'  */
#line 581 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new_class(&(yyval.node), (yyvsp[-2].ihe), (yyvsp[-1].node)); }
#line 3028 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 128: /* function_call: L_NEW '(' L_CLASS L_IDENTIFIER opt_class_init ')'  */
#line 583 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_new_class_undef(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node)); }
#line 3034 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 129: /* function_call: L_DEFINED_NAME call_open opt_arg_list ')'  */
#line 585 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_defined_name(&(yyval.node), (yyvsp[-3].ihe), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3040 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 130: /* function_call: function_name call_open opt_arg_list ')'  */
#line 587 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_name(&(yyval.node), (yyvsp[-3].string), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3046 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 131: /* function_call: primary_expr '[' comma_expr ']' call_open opt_arg_list ')'  */
#line 589 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_indexed(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3052 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 132: /* function_call: primary_expr L_ARROW identifier call_open opt_arg_list ')'  */
#line 591 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_arrow(&(yyval.node), (yyvsp[-5].node), (yyvsp[-3].string), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3058 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 133: /* function_call: '(' '*' comma_expr ')' call_open opt_arg_list ')'  */
#line 593 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_function_call_star(&(yyval.node), (yyvsp[-4].node), (yyvsp[-1].node), (yyvsp[-2].call_open).saved_context, (yyvsp[-2].call_open).saved_refs); }
#line 3064 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 134: /* sscanf: L_SSCANF '(' expr ',' expr lvalue_list ')'  */
#line 604 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_sscanf(&(yyval.node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 3070 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 135: /* parse_command: L_PARSE_COMMAND '(' expr ',' expr ',' expr lvalue_list ')'  */
#line 610 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_parse_command(&(yyval.node), (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 3076 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 136: /* catch: L_CATCH special_context_start expr_or_block  */
#line 616 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_catch(&(yyval.node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 3082 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 137: /* time_expression: L_TIME_EXPRESSION special_context_start expr_or_block  */
#line 622 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_time_expression(&(yyval.node), (yyvsp[0].node), (yyvsp[-1].number)); }
#line 3088 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 138: /* tree: L_TREE block  */
#line 628 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_tree_block(&(yyval.node), (yyvsp[0].decl).node); }
#line 3094 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 139: /* tree: L_TREE '(' comma_expr ')'  */
#line 630 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_tree_expr(&(yyval.node), (yyvsp[-1].node)); }
#line 3100 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 140: /* switch_header: L_SWITCH '(' comma_expr ')'  */
#line 641 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.switch_header).expr = (yyvsp[-1].node); rule_switch_pre(&(yyval.switch_header).saved_context, &(yyval.switch_header).saved_cases_size); }
#line 3106 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 141: /* switch_block: case switch_block  */
#line 646 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_switch_block_case(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3112 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 142: /* switch_block: statement switch_block  */
#line 647 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { rule_switch_block_stmt(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3118 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 143: /* switch_block: %empty  */
#line 648 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { rule_switch_block_empty(&(yyval.node)); }
#line 3124 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 144: /* case: L_CASE case_label ':'  */
#line 655 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_single(&(yyval.node), (yyvsp[-1].node)); }
#line 3130 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 145: /* case: L_CASE case_label L_RANGE case_label ':'  */
#line 657 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range(&(yyval.node), (yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3136 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 146: /* case: L_CASE case_label L_RANGE ':'  */
#line 659 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range_from(&(yyval.node), (yyvsp[-2].node)); }
#line 3142 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 147: /* case: L_CASE L_RANGE case_label ':'  */
#line 661 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_range_to(&(yyval.node), (yyvsp[-1].node)); }
#line 3148 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 148: /* case: L_DEFAULT ':'  */
#line 663 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_case_default(&(yyval.node)); }
#line 3154 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 149: /* case_label: constant  */
#line 669 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_case_label_constant(&(yyval.node), (yyvsp[0].number)); }
#line 3160 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 150: /* case_label: string_const  */
#line 670 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_case_label_string(&(yyval.node), (yyvsp[0].string)); }
#line 3166 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 151: /* for_expr: %empty  */
#line 680 "$REPO_ROOT$/src/compiler/internal/grammar.y"
              { (yyval.node) = 0; }
#line 3172 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 153: /* for_init: for_expr  */
#line 687 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for_init_expr(&(yyval.decl), (yyvsp[0].node)); }
#line 3178 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 154: /* for_init: single_new_local_def_with_init  */
#line 689 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_for_init_local(&(yyval.decl), (yyvsp[0].node)); }
#line 3184 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 155: /* foreach_var: L_DEFINED_NAME  */
#line 694 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_defined(&(yyval.decl), (yyvsp[0].ihe)); }
#line 3190 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 156: /* foreach_var: single_new_local_def  */
#line 695 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_new_local(&(yyval.decl), (yyvsp[0].number)); }
#line 3196 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 157: /* foreach_var: L_IDENTIFIER  */
#line 696 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { rule_foreach_var_identifier(&(yyval.decl), (yyvsp[0].string)); }
#line 3202 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 158: /* foreach_vars: foreach_var  */
#line 702 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach_vars_single(&(yyval.decl), &(yyvsp[0].decl)); }
#line 3208 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 159: /* foreach_vars: foreach_var ',' foreach_var  */
#line 704 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_foreach_vars_double(&(yyval.decl), &(yyvsp[-2].decl), &(yyvsp[0].decl)); }
#line 3214 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 160: /* type_decl: class_header member_list '}'  */
#line 715 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_define_class_members((yyvsp[-2].class_header).ihe, (yyvsp[-2].class_header).classname_index); (yyval.node) = 0; }
#line 3220 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 161: /* class_header: type_modifier_list L_CLASS identifier '{'  */
#line 721 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.class_header).ihe = rule_define_class(&(yyval.class_header).classname_index, (yyvsp[-1].string)); }
#line 3226 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 163: /* $@3: %empty  */
#line 727 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                           { rule_member_list_set_type((yyvsp[0].number)); }
#line 3232 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 167: /* member_name: optional_star identifier  */
#line 739 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { rule_member_name((yyvsp[-1].number), (yyvsp[0].string)); }
#line 3238 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 168: /* class_init: identifier ':' expr  */
#line 744 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                       { (yyval.node) = rule_class_init((yyvsp[-2].string), (yyvsp[0].node)); }
#line 3244 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 169: /* opt_class_init: %empty  */
#line 749 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.node) = rule_opt_class_init_empty(); }
#line 3250 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 170: /* opt_class_init: opt_class_init ',' class_init  */
#line 750 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { (yyval.node) = rule_opt_class_init((yyvsp[-2].node), (yyvsp[0].node)); }
#line 3256 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 171: /* type: type_modifier_list opt_basic_type  */
#line 762 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { (yyval.number) = rule_type((yyvsp[-1].number), (yyvsp[0].number)); }
#line 3262 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 172: /* type_modifier_list: %empty  */
#line 767 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { (yyval.number) = 0; }
#line 3268 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 173: /* type_modifier_list: L_TYPE_MODIFIER type_modifier_list  */
#line 768 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                               { (yyval.number) = rule_type_modifier_list((yyvsp[-1].number), (yyvsp[0].number)); }
#line 3274 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 175: /* opt_basic_type: %empty  */
#line 774 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { (yyval.number) = rule_opt_basic_type_empty(); }
#line 3280 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 177: /* basic_type: opt_atomic_type L_ARRAY  */
#line 780 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { (yyval.number) = (yyvsp[-1].number) | TYPE_MOD_ARRAY; }
#line 3286 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 179: /* opt_atomic_type: %empty  */
#line 786 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { (yyval.number) = rule_opt_basic_type_empty(); }
#line 3292 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 181: /* atomic_type: L_CLASS L_DEFINED_NAME  */
#line 792 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.number) = rule_atomic_type_class((yyvsp[0].ihe)); }
#line 3298 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 182: /* atomic_type: L_CLASS L_IDENTIFIER  */
#line 793 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.number) = rule_atomic_type_class_identifier((yyvsp[0].string)); }
#line 3304 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 184: /* arg_type: basic_type ref  */
#line 799 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.number) = (yyvsp[-1].number) | LOCAL_MOD_REF; }
#line 3310 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 185: /* cast: '(' basic_type optional_star ')'  */
#line 804 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                    { (yyval.number) = rule_cast((yyvsp[-2].number), (yyvsp[-1].number)); }
#line 3316 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 186: /* optional_star: %empty  */
#line 809 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.number) = 0; }
#line 3322 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 187: /* optional_star: '*'  */
#line 810 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { (yyval.number) = TYPE_MOD_ARRAY; }
#line 3328 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 188: /* identifier: L_DEFINED_NAME  */
#line 820 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { (yyval.string) = rule_identifier_defined_name((yyvsp[0].ihe)); }
#line 3334 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 191: /* new_local_name: L_DEFINED_NAME  */
#line 827 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                    { (yyval.string) = rule_new_local_name_redefine((yyvsp[0].ihe)); }
#line 3340 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 192: /* efun_override: L_EFUN L_COLON_COLON identifier  */
#line 832 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.number) = rule_efun_override((yyvsp[0].string)); }
#line 3346 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 193: /* efun_override: L_EFUN L_COLON_COLON L_NEW  */
#line 833 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.number) = rule_efun_override_new(); }
#line 3352 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 195: /* function_name: L_COLON_COLON identifier  */
#line 844 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { (yyval.string) = rule_function_name_colon_colon((yyvsp[0].string)); }
#line 3358 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 196: /* function_name: L_BASIC_TYPE L_COLON_COLON identifier  */
#line 845 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { (yyval.string) = rule_function_name_type((yyvsp[-2].number), (yyvsp[0].string)); }
#line 3364 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 197: /* function_name: identifier L_COLON_COLON identifier  */
#line 846 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                    { (yyval.string) = rule_function_name_obj((yyvsp[-2].string), (yyvsp[0].string)); }
#line 3370 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 198: /* inheritance: type_modifier_list L_INHERIT string_const ';'  */
#line 852 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { if (rule_inheritence(&(yyval.node), (yyvsp[-3].number), (yyvsp[-1].string))) { YYACCEPT; } }
#line 3376 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 199: /* modifier_change: type_modifier_list ':'  */
#line 857 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                          { (yyval.node) = rule_modifier_change((yyvsp[-1].number)); }
#line 3382 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 200: /* argument: %empty  */
#line 868 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.argument).num_arg = 0; (yyval.argument).flags = 0; }
#line 3388 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 202: /* argument: argument_list L_DOT_DOT_DOT  */
#line 871 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_varargs(&(yyval.argument), &(yyvsp[-1].argument)); }
#line 3394 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 203: /* argument_list: param_decl  */
#line 877 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_list_single(&(yyval.argument), (yyvsp[0].number)); }
#line 3400 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 204: /* argument_list: argument_list ',' param_decl  */
#line 879 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_argument_list_multi(&(yyval.argument), &(yyvsp[-2].argument), (yyvsp[0].number)); }
#line 3406 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 205: /* param_decl: arg_type optional_star  */
#line 889 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_typed((yyvsp[-1].number) | (yyvsp[0].number)); }
#line 3412 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 206: /* param_decl: arg_type optional_star new_local_name optional_default_arg_value  */
#line 891 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_typed_name((yyvsp[-3].number) | (yyvsp[-2].number), (yyvsp[-1].string), (yyvsp[0].node)); }
#line 3418 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 207: /* param_decl: new_local_name  */
#line 893 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.number) = rule_param_decl_untyped_name((yyvsp[0].string)); }
#line 3424 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 208: /* optional_default_arg_value: %empty  */
#line 898 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                          { (yyval.node) = 0; }
#line 3430 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 209: /* optional_default_arg_value: ':' L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 899 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                            { (yyval.node) = rule_default_arg_value((yyvsp[-2].node)); }
#line 3436 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 210: /* local_name_list: new_local_def  */
#line 905 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_name_list_single(&(yyval.decl), (yyvsp[0].node)); }
#line 3442 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 211: /* local_name_list: new_local_def ',' local_name_list  */
#line 907 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_local_name_list_multi(&(yyval.decl), (yyvsp[-2].node), &(yyvsp[0].decl)); }
#line 3448 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 212: /* new_local_def: optional_star new_local_name  */
#line 913 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_new_local_def((yyvsp[0].string), (yyvsp[-1].number)); }
#line 3454 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 213: /* new_local_def: optional_star new_local_name L_ASSIGN expr  */
#line 915 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_new_local_def_with_init((yyvsp[-2].string), (yyvsp[-3].number), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3460 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 214: /* single_new_local_def: arg_type optional_star new_local_name  */
#line 921 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_single_new_local_def(&(yyval.number), (yyvsp[-2].number) | (yyvsp[-1].number), (yyvsp[0].string)); }
#line 3466 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 215: /* single_new_local_def_with_init: single_new_local_def L_ASSIGN expr  */
#line 927 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { (yyval.node) = rule_single_new_local_def_with_init((yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3472 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 218: /* new_name: optional_star identifier  */
#line 943 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { rule_new_name((yyvsp[-1].number), (yyvsp[0].string)); }
#line 3478 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 219: /* new_name: optional_star identifier L_ASSIGN expr  */
#line 944 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                           { rule_new_name_with_init((yyvsp[-3].number), (yyvsp[-2].string), (yyvsp[-1].number), (yyvsp[0].node)); }
#line 3484 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 220: /* lambda_return_type: L_BASIC_TYPE  */
#line 955 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                { rule_lambda_return_type(&(yyval.func_block), (yyvsp[0].number)); }
#line 3490 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 222: /* functional_open: L_FUNCTION_OPEN efun_override  */
#line 964 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.number) = rule_functional_open((yyvsp[0].number)); }
#line 3496 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 223: /* assoc_pair: expr ':' expr  */
#line 974 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_assoc_pair(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3502 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 224: /* spread_expr: expr  */
#line 979 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_spread_expr_normal(&(yyval.node), (yyvsp[0].node)); }
#line 3508 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 225: /* spread_expr: expr L_DOT_DOT_DOT  */
#line 980 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                        { rule_spread_expr_dots(&(yyval.node), (yyvsp[-1].node)); }
#line 3514 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 226: /* arg_list: spread_expr  */
#line 986 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_arg_list_single(&(yyval.node), (yyvsp[0].node)); }
#line 3520 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 227: /* arg_list: arg_list ',' spread_expr  */
#line 988 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_arg_list_multi(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3526 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 228: /* opt_arg_list: %empty  */
#line 994 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list_empty(&(yyval.node)); }
#line 3532 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 229: /* opt_arg_list: arg_list  */
#line 995 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list(&(yyval.node), (yyvsp[0].node)); }
#line 3538 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 230: /* opt_arg_list: arg_list ','  */
#line 996 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_opt_arg_list(&(yyval.node), (yyvsp[-1].node)); }
#line 3544 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 231: /* pair_list: assoc_pair  */
#line 1002 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_pair_list_single(&(yyval.node), (yyvsp[0].node)); }
#line 3550 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 232: /* pair_list: pair_list ',' assoc_pair  */
#line 1004 "$REPO_ROOT$/src/compiler/internal/grammar.y"
    { rule_pair_list_multi(&(yyval.node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3556 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 233: /* opt_pair_list: %empty  */
#line 1010 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list_empty(&(yyval.node)); }
#line 3562 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 234: /* opt_pair_list: pair_list  */
#line 1011 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list(&(yyval.node), (yyvsp[0].node)); }
#line 3568 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 235: /* opt_pair_list: pair_list ','  */
#line 1012 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                   { rule_opt_pair_list(&(yyval.node), (yyvsp[-1].node)); }
#line 3574 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 236: /* lvalue_list: %empty  */
#line 1017 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                            { (yyval.node) = rule_lvalue_list_empty(); }
#line 3580 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 237: /* lvalue_list: ',' lvalue lvalue_list  */
#line 1018 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { (yyval.node) = rule_lvalue_list((yyvsp[-1].node), (yyvsp[0].node)); }
#line 3586 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 238: /* expr_or_block: block  */
#line 1023 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_expr_or_block_block((yyvsp[0].decl)); }
#line 3592 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 239: /* expr_or_block: '(' comma_expr ')'  */
#line 1024 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                         { (yyval.node) = rule_expr_or_block_expr((yyvsp[-1].node)); }
#line 3598 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 241: /* string: string_literal  */
#line 1039 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                  { rule_string(&(yyval.node), (yyvsp[0].string)); }
#line 3604 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 242: /* template_literal: L_TEMPLATE_HEAD expr template_parts  */
#line 1051 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_template_literal(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3610 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 243: /* template_parts: L_TEMPLATE_TAIL  */
#line 1055 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                      { rule_template_parts_tail(&(yyval.node), (yyvsp[0].string)); }
#line 3616 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 244: /* template_parts: L_TEMPLATE_MIDDLE expr template_parts  */
#line 1056 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                       { rule_template_parts_middle(&(yyval.node), (yyvsp[-2].string), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3622 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 247: /* string_like: string_like string  */
#line 1067 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                               { rule_string_like_concat(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3628 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 248: /* string_like: string_like template_literal  */
#line 1068 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { rule_string_like_concat(&(yyval.node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 3634 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 250: /* string_literal: string_literal L_STRING  */
#line 1076 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                   { (yyval.string) = rule_string_literal_concat((yyvsp[-1].string), (yyvsp[0].string)); }
#line 3640 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 252: /* string_const: '(' string_const ')'  */
#line 1083 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                        { (yyval.string) = (yyvsp[-1].string); }
#line 3646 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 253: /* string_const: string_const '+' string_const  */
#line 1084 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                                { (yyval.string) = rule_string_literal_concat((yyvsp[-2].string), (yyvsp[0].string)); }
#line 3652 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 254: /* number: L_NUMBER  */
#line 1088 "$REPO_ROOT$/src/compiler/internal/grammar.y"
            { rule_number(&(yyval.node), (yyvsp[0].number)); }
#line 3658 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 255: /* real: L_REAL  */
#line 1092 "$REPO_ROOT$/src/compiler/internal/grammar.y"
          { rule_real(&(yyval.node), (yyvsp[0].real)); }
#line 3664 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 256: /* constant: constant '|' constant  */
#line 1098 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_or(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3670 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 257: /* constant: constant '^' constant  */
#line 1099 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_xor(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3676 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 258: /* constant: constant '&' constant  */
#line 1100 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                     { rule_constant_and(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3682 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 259: /* constant: constant L_EQ constant  */
#line 1101 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_eq(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3688 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 260: /* constant: constant L_NE constant  */
#line 1102 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_ne(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3694 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 261: /* constant: constant L_ORDER constant  */
#line 1103 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                         { rule_constant_order(&(yyval.number), (yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].number)); }
#line 3700 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 262: /* constant: constant '<' constant  */
#line 1104 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_lt(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3706 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 263: /* constant: constant L_LSH constant  */
#line 1105 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_lsh(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3712 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 264: /* constant: constant L_RSH constant  */
#line 1106 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_rsh(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3718 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 265: /* constant: constant '+' constant  */
#line 1107 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_add(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3724 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 266: /* constant: constant '-' constant  */
#line 1108 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_sub(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3730 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 267: /* constant: constant '*' constant  */
#line 1109 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_mul(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3736 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 268: /* constant: constant '%' constant  */
#line 1110 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_mod(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3742 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 269: /* constant: constant '/' constant  */
#line 1111 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                       { rule_constant_div(&(yyval.number), (yyvsp[-2].number), (yyvsp[0].number)); }
#line 3748 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 270: /* constant: '(' constant ')'  */
#line 1112 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                                  { (yyval.number) = (yyvsp[-1].number); }
#line 3754 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 271: /* constant: L_NUMBER  */
#line 1113 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { (yyval.number) = (yyvsp[0].number); }
#line 3760 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 272: /* constant: '-' L_NUMBER  */
#line 1114 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_neg(&(yyval.number), (yyvsp[0].number)); }
#line 3766 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 273: /* constant: L_NOT L_NUMBER  */
#line 1115 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_not(&(yyval.number), (yyvsp[0].number)); }
#line 3772 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;

  case 274: /* constant: '~' L_NUMBER  */
#line 1116 "$REPO_ROOT$/src/compiler/internal/grammar.y"
                             { rule_constant_compl(&(yyval.number), (yyvsp[0].number)); }
#line 3778 "$REPO_ROOT$/build/src/grammar.autogen.cc"
    break;


#line 3782 "$REPO_ROOT$/build/src/grammar.autogen.cc"

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

#line 1119 "$REPO_ROOT$/src/compiler/internal/grammar.y"

