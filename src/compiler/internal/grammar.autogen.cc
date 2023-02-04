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
#line 11 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"

#include "base/std.h"

#include "compiler/internal/grammar_rules.h"
#include "vm/vm.h"
#include "vm/internal/base/machine.h"
#include "compiler/internal/compiler.h"
#include "compiler/internal/lex.h"
#include "compiler/internal/scratchpad.h"
#include "compiler/internal/generate.h"

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


#line 107 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"

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
  YYSYMBOL_L_LSH = 16,                     /* L_LSH  */
  YYSYMBOL_L_RSH = 17,                     /* L_RSH  */
  YYSYMBOL_L_ORDER = 18,                   /* L_ORDER  */
  YYSYMBOL_L_NOT = 19,                     /* L_NOT  */
  YYSYMBOL_L_IF = 20,                      /* L_IF  */
  YYSYMBOL_L_ELSE = 21,                    /* L_ELSE  */
  YYSYMBOL_L_SWITCH = 22,                  /* L_SWITCH  */
  YYSYMBOL_L_CASE = 23,                    /* L_CASE  */
  YYSYMBOL_L_DEFAULT = 24,                 /* L_DEFAULT  */
  YYSYMBOL_L_RANGE = 25,                   /* L_RANGE  */
  YYSYMBOL_L_DOT_DOT_DOT = 26,             /* L_DOT_DOT_DOT  */
  YYSYMBOL_L_WHILE = 27,                   /* L_WHILE  */
  YYSYMBOL_L_DO = 28,                      /* L_DO  */
  YYSYMBOL_L_FOR = 29,                     /* L_FOR  */
  YYSYMBOL_L_FOREACH = 30,                 /* L_FOREACH  */
  YYSYMBOL_L_IN = 31,                      /* L_IN  */
  YYSYMBOL_L_BREAK = 32,                   /* L_BREAK  */
  YYSYMBOL_L_CONTINUE = 33,                /* L_CONTINUE  */
  YYSYMBOL_L_RETURN = 34,                  /* L_RETURN  */
  YYSYMBOL_L_ARROW = 35,                   /* L_ARROW  */
  YYSYMBOL_L_DOT = 36,                     /* L_DOT  */
  YYSYMBOL_L_INHERIT = 37,                 /* L_INHERIT  */
  YYSYMBOL_L_COLON_COLON = 38,             /* L_COLON_COLON  */
  YYSYMBOL_L_ARRAY_OPEN = 39,              /* L_ARRAY_OPEN  */
  YYSYMBOL_L_MAPPING_OPEN = 40,            /* L_MAPPING_OPEN  */
  YYSYMBOL_L_FUNCTION_OPEN = 41,           /* L_FUNCTION_OPEN  */
  YYSYMBOL_L_NEW_FUNCTION_OPEN = 42,       /* L_NEW_FUNCTION_OPEN  */
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
  YYSYMBOL_L_PREPROCESSOR_COMMAND = 53,    /* L_PREPROCESSOR_COMMAND  */
  YYSYMBOL_LOWER_THAN_ELSE = 54,           /* LOWER_THAN_ELSE  */
  YYSYMBOL_55_ = 55,                       /* '?'  */
  YYSYMBOL_56_ = 56,                       /* '|'  */
  YYSYMBOL_57_ = 57,                       /* '^'  */
  YYSYMBOL_58_ = 58,                       /* '&'  */
  YYSYMBOL_L_EQ = 59,                      /* L_EQ  */
  YYSYMBOL_L_NE = 60,                      /* L_NE  */
  YYSYMBOL_61_ = 61,                       /* '<'  */
  YYSYMBOL_62_ = 62,                       /* '+'  */
  YYSYMBOL_63_ = 63,                       /* '-'  */
  YYSYMBOL_64_ = 64,                       /* '*'  */
  YYSYMBOL_65_ = 65,                       /* '%'  */
  YYSYMBOL_66_ = 66,                       /* '/'  */
  YYSYMBOL_67_ = 67,                       /* '~'  */
  YYSYMBOL_68_ = 68,                       /* ';'  */
  YYSYMBOL_69_ = 69,                       /* '('  */
  YYSYMBOL_70_ = 70,                       /* ')'  */
  YYSYMBOL_71_ = 71,                       /* ':'  */
  YYSYMBOL_72_ = 72,                       /* ','  */
  YYSYMBOL_73_ = 73,                       /* '{'  */
  YYSYMBOL_74_ = 74,                       /* '}'  */
  YYSYMBOL_75_ = 75,                       /* '$'  */
  YYSYMBOL_76_ = 76,                       /* '['  */
  YYSYMBOL_77_ = 77,                       /* ']'  */
  YYSYMBOL_YYACCEPT = 78,                  /* $accept  */
  YYSYMBOL_all = 79,                       /* all  */
  YYSYMBOL_program = 80,                   /* program  */
  YYSYMBOL_possible_semi_colon = 81,       /* possible_semi_colon  */
  YYSYMBOL_inheritance = 82,               /* inheritance  */
  YYSYMBOL_real = 83,                      /* real  */
  YYSYMBOL_number = 84,                    /* number  */
  YYSYMBOL_optional_star = 85,             /* optional_star  */
  YYSYMBOL_block_or_semi = 86,             /* block_or_semi  */
  YYSYMBOL_identifier = 87,                /* identifier  */
  YYSYMBOL_function = 88,                  /* function  */
  YYSYMBOL_89_1 = 89,                      /* $@1  */
  YYSYMBOL_90_2 = 90,                      /* @2  */
  YYSYMBOL_def = 91,                       /* def  */
  YYSYMBOL_modifier_change = 92,           /* modifier_change  */
  YYSYMBOL_member_name = 93,               /* member_name  */
  YYSYMBOL_member_name_list = 94,          /* member_name_list  */
  YYSYMBOL_member_list = 95,               /* member_list  */
  YYSYMBOL_96_3 = 96,                      /* $@3  */
  YYSYMBOL_type_decl = 97,                 /* type_decl  */
  YYSYMBOL_98_4 = 98,                      /* @4  */
  YYSYMBOL_new_local_name = 99,            /* new_local_name  */
  YYSYMBOL_atomic_type = 100,              /* atomic_type  */
  YYSYMBOL_opt_atomic_type = 101,          /* opt_atomic_type  */
  YYSYMBOL_basic_type = 102,               /* basic_type  */
  YYSYMBOL_arg_type = 103,                 /* arg_type  */
  YYSYMBOL_new_arg = 104,                  /* new_arg  */
  YYSYMBOL_argument = 105,                 /* argument  */
  YYSYMBOL_argument_list = 106,            /* argument_list  */
  YYSYMBOL_type_modifier_list = 107,       /* type_modifier_list  */
  YYSYMBOL_type = 108,                     /* type  */
  YYSYMBOL_cast = 109,                     /* cast  */
  YYSYMBOL_opt_basic_type = 110,           /* opt_basic_type  */
  YYSYMBOL_name_list = 111,                /* name_list  */
  YYSYMBOL_new_name = 112,                 /* new_name  */
  YYSYMBOL_block = 113,                    /* block  */
  YYSYMBOL_decl_block = 114,               /* decl_block  */
  YYSYMBOL_local_declarations = 115,       /* local_declarations  */
  YYSYMBOL_116_5 = 116,                    /* $@5  */
  YYSYMBOL_new_local_def = 117,            /* new_local_def  */
  YYSYMBOL_single_new_local_def = 118,     /* single_new_local_def  */
  YYSYMBOL_single_new_local_def_with_init = 119, /* single_new_local_def_with_init  */
  YYSYMBOL_local_name_list = 120,          /* local_name_list  */
  YYSYMBOL_statements = 121,               /* statements  */
  YYSYMBOL_statement = 122,                /* statement  */
  YYSYMBOL_while = 123,                    /* while  */
  YYSYMBOL_124_6 = 124,                    /* $@6  */
  YYSYMBOL_do = 125,                       /* do  */
  YYSYMBOL_126_7 = 126,                    /* $@7  */
  YYSYMBOL_for = 127,                      /* for  */
  YYSYMBOL_128_8 = 128,                    /* $@8  */
  YYSYMBOL_foreach_var = 129,              /* foreach_var  */
  YYSYMBOL_foreach_vars = 130,             /* foreach_vars  */
  YYSYMBOL_foreach = 131,                  /* foreach  */
  YYSYMBOL_132_9 = 132,                    /* $@9  */
  YYSYMBOL_for_expr = 133,                 /* for_expr  */
  YYSYMBOL_first_for_expr = 134,           /* first_for_expr  */
  YYSYMBOL_switch = 135,                   /* switch  */
  YYSYMBOL_136_10 = 136,                   /* $@10  */
  YYSYMBOL_switch_block = 137,             /* switch_block  */
  YYSYMBOL_case = 138,                     /* case  */
  YYSYMBOL_case_label = 139,               /* case_label  */
  YYSYMBOL_constant = 140,                 /* constant  */
  YYSYMBOL_comma_expr = 141,               /* comma_expr  */
  YYSYMBOL_ref = 142,                      /* ref  */
  YYSYMBOL_expr0 = 143,                    /* expr0  */
  YYSYMBOL_return = 144,                   /* return  */
  YYSYMBOL_expr_list = 145,                /* expr_list  */
  YYSYMBOL_expr_list_node = 146,           /* expr_list_node  */
  YYSYMBOL_expr_list2 = 147,               /* expr_list2  */
  YYSYMBOL_expr_list3 = 148,               /* expr_list3  */
  YYSYMBOL_expr_list4 = 149,               /* expr_list4  */
  YYSYMBOL_assoc_pair = 150,               /* assoc_pair  */
  YYSYMBOL_lvalue = 151,                   /* lvalue  */
  YYSYMBOL_l_new_function_open = 152,      /* l_new_function_open  */
  YYSYMBOL_expr4 = 153,                    /* expr4  */
  YYSYMBOL_154_11 = 154,                   /* @11  */
  YYSYMBOL_155_12 = 155,                   /* @12  */
  YYSYMBOL_expr_or_block = 156,            /* expr_or_block  */
  YYSYMBOL_catch = 157,                    /* catch  */
  YYSYMBOL_158_13 = 158,                   /* @13  */
  YYSYMBOL_tree = 159,                     /* tree  */
  YYSYMBOL_sscanf = 160,                   /* sscanf  */
  YYSYMBOL_parse_command = 161,            /* parse_command  */
  YYSYMBOL_time_expression = 162,          /* time_expression  */
  YYSYMBOL_163_14 = 163,                   /* @14  */
  YYSYMBOL_lvalue_list = 164,              /* lvalue_list  */
  YYSYMBOL_string = 165,                   /* string  */
  YYSYMBOL_string_con1 = 166,              /* string_con1  */
  YYSYMBOL_string_con2 = 167,              /* string_con2  */
  YYSYMBOL_class_init = 168,               /* class_init  */
  YYSYMBOL_opt_class_init = 169,           /* opt_class_init  */
  YYSYMBOL_function_call = 170,            /* function_call  */
  YYSYMBOL_171_15 = 171,                   /* @15  */
  YYSYMBOL_172_16 = 172,                   /* @16  */
  YYSYMBOL_173_17 = 173,                   /* @17  */
  YYSYMBOL_174_18 = 174,                   /* @18  */
  YYSYMBOL_175_19 = 175,                   /* @19  */
  YYSYMBOL_176_20 = 176,                   /* @20  */
  YYSYMBOL_efun_override = 177,            /* efun_override  */
  YYSYMBOL_function_name = 178,            /* function_name  */
  YYSYMBOL_cond = 179,                     /* cond  */
  YYSYMBOL_optional_else_part = 180        /* optional_else_part  */
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
#define YYLAST   1867

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  78
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  103
/* YYNRULES -- Number of rules.  */
#define YYNRULES  262
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  494

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
       2,     2,     2,     2,     2,     2,    75,    65,    58,     2,
      69,    70,    64,    62,    72,    63,     2,    66,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    71,    68,
      61,     2,     2,    55,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    76,     2,    77,    57,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    73,    56,    74,    67,     2,     2,     2,
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
      59,    60
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   215,   215,   219,   220,   223,   225,   229,   233,   237,
     241,   242,   246,   253,   254,   258,   259,   263,   264,   263,
     269,   270,   276,   277,   278,   282,   299,   312,   313,   316,
     318,   318,   323,   323,   328,   329,   339,   340,   349,   357,
     358,   362,   363,   367,   368,   372,   378,   386,   399,   403,
     404,   422,   433,   447,   450,   467,   475,   482,   484,   490,
     491,   495,   520,   575,   584,   584,   584,   588,   593,   592,
     611,   622,   656,   667,   699,   704,   715,   718,   724,   731,
     743,   744,   745,   746,   747,   748,   753,   757,   779,   792,
     791,   805,   804,   818,   817,   842,   863,   873,   890,   895,
     906,   905,   925,   928,   932,   937,   946,   945,   984,   990,
     997,  1003,  1010,  1024,  1038,  1051,  1067,  1081,  1096,  1100,
    1104,  1108,  1112,  1116,  1124,  1128,  1132,  1136,  1140,  1144,
    1148,  1152,  1156,  1160,  1164,  1168,  1172,  1179,  1183,  1190,
    1194,  1221,  1246,  1251,  1275,  1281,  1287,  1312,  1316,  1339,
    1361,  1375,  1419,  1456,  1460,  1464,  1634,  1728,  1808,  1812,
    1907,  1928,  1949,  1971,  1980,  1991,  2015,  2037,  2058,  2059,
    2060,  2061,  2062,  2063,  2067,  2073,  2094,  2097,  2101,  2108,
    2112,  2119,  2124,  2137,  2141,  2145,  2152,  2162,  2180,  2187,
    2303,  2304,  2311,  2312,  2355,  2372,  2377,  2376,  2406,  2430,
    2454,  2465,  2469,  2476,  2483,  2487,  2491,  2536,  2592,  2593,
    2597,  2598,  2600,  2599,  2656,  2694,  2789,  2812,  2821,  2833,
    2837,  2845,  2844,  2857,  2864,  2874,  2883,  2894,  2893,  2907,
    2912,  2926,  2934,  2935,  2939,  2946,  2947,  2954,  2965,  2968,
    2977,  2976,  2990,  2989,  3020,  3055,  3074,  3073,  3134,  3133,
    3202,  3201,  3253,  3252,  3283,  3303,  3319,  3320,  3334,  3349,
    3364,  3398,  3402
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
  "L_LOR", "L_LSH", "L_RSH", "L_ORDER", "L_NOT", "L_IF", "L_ELSE",
  "L_SWITCH", "L_CASE", "L_DEFAULT", "L_RANGE", "L_DOT_DOT_DOT", "L_WHILE",
  "L_DO", "L_FOR", "L_FOREACH", "L_IN", "L_BREAK", "L_CONTINUE",
  "L_RETURN", "L_ARROW", "L_DOT", "L_INHERIT", "L_COLON_COLON",
  "L_ARRAY_OPEN", "L_MAPPING_OPEN", "L_FUNCTION_OPEN",
  "L_NEW_FUNCTION_OPEN", "L_SSCANF", "L_CATCH", "L_ARRAY", "L_REF",
  "L_PARSE_COMMAND", "L_TIME_EXPRESSION", "L_CLASS", "L_NEW",
  "L_PARAMETER", "L_TREE", "L_PREPROCESSOR_COMMAND", "LOWER_THAN_ELSE",
  "'?'", "'|'", "'^'", "'&'", "L_EQ", "L_NE", "'<'", "'+'", "'-'", "'*'",
  "'%'", "'/'", "'~'", "';'", "'('", "')'", "':'", "','", "'{'", "'}'",
  "'$'", "'['", "']'", "$accept", "all", "program", "possible_semi_colon",
  "inheritance", "real", "number", "optional_star", "block_or_semi",
  "identifier", "function", "$@1", "@2", "def", "modifier_change",
  "member_name", "member_name_list", "member_list", "$@3", "type_decl",
  "@4", "new_local_name", "atomic_type", "opt_atomic_type", "basic_type",
  "arg_type", "new_arg", "argument", "argument_list", "type_modifier_list",
  "type", "cast", "opt_basic_type", "name_list", "new_name", "block",
  "decl_block", "local_declarations", "$@5", "new_local_def",
  "single_new_local_def", "single_new_local_def_with_init",
  "local_name_list", "statements", "statement", "while", "$@6", "do",
  "$@7", "for", "$@8", "foreach_var", "foreach_vars", "foreach", "$@9",
  "for_expr", "first_for_expr", "switch", "$@10", "switch_block", "case",
  "case_label", "constant", "comma_expr", "ref", "expr0", "return",
  "expr_list", "expr_list_node", "expr_list2", "expr_list3", "expr_list4",
  "assoc_pair", "lvalue", "l_new_function_open", "expr4", "@11", "@12",
  "expr_or_block", "catch", "@13", "tree", "sscanf", "parse_command",
  "time_expression", "@14", "lvalue_list", "string", "string_con1",
  "string_con2", "class_init", "opt_class_init", "function_call", "@15",
  "@16", "@17", "@18", "@19", "@20", "efun_override", "function_name",
  "cond", "optional_else_part", YY_NULLPTR
  };
  return yy_sname[yysymbol];
}
#endif

#define YYPACT_NINF (-403)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-257)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -403,    55,    58,  -403,    54,  -403,  -403,    -4,  -403,  -403,
     125,     3,  -403,  -403,  -403,  -403,     8,   272,  -403,    29,
      87,  -403,  -403,  -403,   279,    79,    81,  -403,     8,   -12,
     156,    94,   104,   159,  -403,  -403,  -403,    15,  -403,     3,
     -23,     8,  -403,  -403,  -403,  1725,   139,   279,  -403,  -403,
    -403,  -403,   227,  -403,  -403,   232,    -1,     6,   235,   178,
     178,  1725,   279,  1212,   543,  1725,  -403,   276,  -403,  -403,
     297,  -403,   301,  -403,     7,  1725,  1725,   991,   305,  -403,
    -403,   258,  1725,   178,  1399,   167,   253,   239,  -403,  -403,
    -403,  -403,  -403,  -403,   156,  -403,   310,   322,   163,   288,
      24,  1725,   279,   323,  -403,   135,  1287,  -403,   -19,  -403,
    -403,  -403,  1326,   236,  -403,   328,   957,   318,   334,  -403,
     306,  1399,   310,  1725,    65,  1725,    65,   358,  1725,  -403,
    -403,  -403,  -403,    10,   374,  1725,     3,    53,  -403,   279,
    -403,  -403,  1725,  1725,  1725,  1725,  1725,  1725,  1725,  1725,
    1725,  1725,  1725,  1725,  1725,  1725,  1725,  1725,  1725,  -403,
    -403,  1725,   338,  1725,   279,   279,  1360,  -403,  -403,  -403,
    -403,  -403,   363,     3,  -403,   341,    27,  -403,  -403,  1399,
    -403,   163,  1433,  -403,  -403,  -403,   344,  1137,  1725,   345,
     618,   346,  1725,   433,  1725,  -403,  -403,   508,  -403,   377,
    1433,   128,   693,  -403,  -403,   153,   348,  -403,  1725,  -403,
     277,  1472,   202,   202,   364,  1030,   291,  1689,  1762,   197,
     197,   364,   212,   212,  -403,  -403,  -403,  1399,  -403,   316,
     350,  -403,  1725,    13,  1433,  1433,  -403,   381,  -403,  -403,
      37,     3,   352,   353,  -403,  -403,  1399,  -403,  -403,  -403,
    1399,  1725,   174,  1725,  -403,  -403,   372,  -403,    14,   351,
     383,   384,  -403,   386,   388,   378,   390,  1506,  -403,  -403,
    -403,  -403,   385,   843,  -403,  -403,  -403,  -403,  -403,   138,
    -403,  -403,   392,  -403,   185,  1725,   394,  1725,  -403,    16,
     393,  -403,   395,   396,  -403,    23,  -403,   279,   391,   399,
     402,  -403,   583,  -403,  1177,   241,   244,  -403,  -403,  1725,
    1725,  1725,   918,  1064,   133,  -403,  -403,  -403,   166,     3,
    -403,  -403,  -403,  -403,  -403,  1399,  -403,  1433,   468,  -403,
    1725,  -403,    77,  -403,  -403,  -403,  -403,  -403,  -403,  -403,
       3,  -403,  -403,   178,   411,  1725,  -403,   279,  -403,   251,
     295,   299,   455,     3,   470,  -403,  -403,   416,   413,  -403,
    -403,  -403,   414,   469,  -403,   381,   429,   434,  1433,   443,
    1725,  -403,    84,    88,  -403,  -403,   431,  -403,   583,   446,
    -403,   918,  -403,  -403,   435,   381,  1725,  1579,   133,  1725,
     514,     3,  -403,   451,  -403,    96,  -403,  -403,  -403,   458,
    1725,   509,   459,   918,  1725,  -403,  1399,   465,  -403,  1103,
    1725,  -403,  -403,  -403,  -403,  1399,   918,  -403,  -403,  -403,
     303,  1652,  -403,  1399,  -403,   248,   466,   471,   918,   223,
     467,   768,  -403,  -403,  -403,  -403,   532,   172,   535,   536,
     172,    26,  1543,   480,  -403,   768,   476,   768,   918,  -403,
     485,  -403,  -403,  1251,   132,  -403,    59,    59,    59,    59,
      59,    59,    59,    59,    59,    59,    59,    59,    59,    59,
    -403,  -403,  -403,  -403,  -403,  -403,  -403,   486,    59,   298,
     298,   588,  1616,  1743,  1773,   266,   266,   588,   240,   240,
    -403,  -403,  -403,  -403
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       4,     0,    53,     1,    53,    22,    20,     5,    24,    23,
      58,    10,    54,     6,     3,    36,     0,     0,    25,    41,
       0,    57,    55,    11,     0,     0,    59,   235,     0,     0,
     232,    37,    38,     0,    42,    15,    16,    61,    21,    10,
       0,     0,     7,   236,    32,     0,     0,     0,    60,   233,
     234,    29,     0,     9,     8,   212,   193,   194,     0,     0,
       0,     0,     0,     0,     0,     0,   190,     0,   221,   139,
       0,   227,     0,   195,     0,     0,     0,     0,     0,   173,
     172,     0,     0,     0,    62,     0,     0,   168,   210,   211,
     169,   170,   171,   208,   231,   192,     0,     0,    40,    61,
      40,     0,     0,     0,   246,     0,     0,   161,   189,   162,
     163,   257,   179,     0,   181,   177,     0,     0,   184,   186,
       0,   137,   191,     0,     0,     0,     0,   242,     0,    67,
     223,   165,   164,    36,     0,     0,    10,     0,   196,     0,
     160,   140,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   166,
     167,     0,     0,     0,     0,     0,     0,   240,   248,    35,
      34,    47,    43,    10,    51,     0,    49,    33,    30,   142,
     258,    40,     0,   255,   254,   180,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   219,   222,     0,   228,     0,
       0,     0,     0,    37,    38,     0,     0,   209,     0,   259,
     145,   144,   153,   154,   151,     0,   146,   147,   148,   149,
     150,   152,   155,   156,   157,   158,   159,   141,   214,     0,
     198,   199,     0,     0,     0,     0,    44,    45,    18,    50,
      40,    10,     0,     0,   218,   182,   188,   217,   187,   216,
     138,     0,     0,     0,   238,   238,     0,   224,     0,     0,
       0,     0,    91,     0,     0,     0,     0,     0,    86,    68,
      64,    85,     0,     0,    81,    82,    65,    66,    83,     0,
      84,    80,     0,    56,     0,     0,     0,     0,   250,     0,
       0,   207,     0,     0,    46,     0,    52,     0,    27,     0,
       0,   247,   229,   220,     0,     0,     0,   243,    78,     0,
       0,     0,     0,     0,    40,    87,    88,   174,     0,    10,
      63,    77,    79,   252,   197,   143,   215,     0,     0,   206,
       0,   204,     0,   241,   249,    14,    13,    19,    12,    26,
      10,    31,   213,     0,     0,     0,   244,     0,   245,     0,
       0,     0,     0,    10,     0,   105,   104,     0,   103,    95,
      97,    96,    98,     0,   175,     0,    74,     0,     0,     0,
       0,   205,     0,     0,   200,    28,   229,   225,   229,     0,
     239,     0,   106,    89,     0,     0,     0,     0,    40,     0,
      70,    10,    69,     0,   251,     0,   201,   203,   230,     0,
       0,   261,     0,     0,     0,    72,    73,     0,    99,     0,
       0,    75,   253,   202,   226,   237,     0,   260,    67,    90,
       0,     0,   100,    71,   262,    40,     0,     0,     0,     0,
       0,     0,    92,    93,   101,   133,     0,     0,     0,     0,
       0,     0,   116,   117,   115,     0,     0,     0,     0,   135,
       0,   134,   136,     0,     0,   111,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     109,   107,   108,    94,   114,   132,   113,     0,     0,   125,
     126,   123,   118,   119,   120,   121,   122,   124,   127,   128,
     129,   130,   131,   112
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -403,  -403,  -403,  -403,  -403,  -403,  -403,    -7,  -403,    25,
    -403,  -403,  -403,  -403,  -403,  -403,   218,  -403,  -403,  -403,
    -403,  -228,  -403,  -403,    -8,  -299,   319,   379,  -403,   557,
    -403,  -403,  -403,   537,  -403,   -64,  -403,   157,  -403,  -403,
     264,  -403,   187,   315,  -179,  -403,  -403,  -403,  -403,  -403,
    -403,   191,  -403,  -403,  -403,  -269,  -403,  -403,  -403,  -240,
     171,  -402,  1389,   -11,   420,   -42,  -403,  -164,  -167,   439,
    -403,  -403,   417,   -54,  -403,   -52,  -403,  -403,   477,  -403,
    -403,  -403,  -403,  -403,  -403,  -403,   -58,  -403,   -15,   -16,
    -403,   354,  -403,  -403,  -403,  -403,  -403,  -403,  -403,   546,
    -403,  -403,  -403
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,     2,    14,     5,    79,    80,   297,   337,    81,
       6,    46,   295,     7,     8,   298,   299,   100,   241,     9,
      51,   171,    19,    20,   172,   173,   174,   175,   176,    10,
      11,    82,    22,    25,    26,   270,   271,   202,   319,   366,
     361,   355,   367,   272,   445,   274,   403,   275,   312,   276,
     448,   362,   363,   277,   428,   356,   357,   278,   402,   446,
     447,   441,   442,   279,    83,   121,   280,   113,   114,   115,
     117,   118,   119,    85,    86,    87,   208,   103,   196,    88,
     124,    89,    90,    91,    92,   126,   344,    93,   443,    94,
     380,   305,    95,   234,   200,   182,   235,   327,   368,    96,
      97,   281,   417
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      30,    29,    21,    84,    24,   107,   109,   108,   108,   294,
     130,    27,    30,    40,   353,   353,   164,   165,   243,   110,
     245,   112,   116,   273,   335,    30,    50,   101,    45,   141,
      15,   108,    47,   131,   132,   450,   256,   -15,   290,    41,
     140,   328,    33,    15,   -16,   169,   170,    49,   102,    37,
      41,   454,   477,   239,   120,     3,    42,   166,    -2,   179,
     195,     4,   195,   435,    13,     4,   137,    23,   104,   136,
     292,   293,    99,   134,   -39,  -256,   128,    28,   436,  -212,
     129,   193,   308,   197,   -17,   192,   134,   111,   192,   353,
     291,   336,   178,   329,   273,   137,   129,   455,   177,   240,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   201,   407,   227,
     245,   112,   438,   207,   205,   192,   439,   180,   478,   206,
     184,    15,    34,   352,   194,    27,   435,   390,   129,    15,
     112,   359,   360,    35,    36,   112,   246,    38,   116,   192,
     250,   436,   427,    39,   374,   233,   192,   405,   112,    43,
     192,   396,    16,   369,   209,   397,   237,   -15,   192,    15,
     -40,   169,   170,   413,    17,    27,   435,   -16,   159,   160,
     161,    27,   134,   252,    55,   183,    56,    57,    58,   230,
     231,   436,   112,   112,   269,   438,    18,   284,   257,   439,
     192,   440,   401,   476,   393,   470,   322,   472,    98,   302,
     192,   304,   134,   144,   145,   146,    62,    63,    64,    65,
      66,   289,    68,   282,   419,   192,    27,   435,    72,    73,
      74,   338,    44,   -48,   364,   438,   342,   424,   192,   439,
     101,   440,   436,   325,   303,   112,   192,   106,   437,   434,
    -189,  -189,  -189,    78,    15,   324,   318,   192,   153,   154,
     155,   156,   157,   158,   154,   155,   156,   157,   158,   473,
     102,   429,   430,   105,   164,   165,   156,   157,   158,   332,
      31,    32,   456,   457,   458,   112,   438,    35,    36,   376,
     439,   108,   440,   144,   145,   146,   139,   134,   349,   350,
     351,    45,   358,   378,   467,   468,   469,   144,   145,   146,
     186,   346,   365,   347,   348,   166,   347,   372,   398,   373,
     399,   381,   339,   192,   162,   163,   112,   464,   465,   466,
     467,   468,   469,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   406,   123,   385,   409,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   415,   395,
     465,   466,   467,   468,   469,   382,   125,   192,   423,   383,
     127,   192,   379,   426,   138,   192,   358,   191,   192,   167,
     144,   145,   203,   204,   365,   254,   255,   286,   287,   169,
     170,   168,   181,   420,    52,   189,    27,    53,    54,    55,
     187,    56,    57,    58,    59,    60,   190,   199,   228,    69,
     358,   238,    61,    30,   244,   247,   249,   269,   283,   288,
     309,    30,   300,   301,    30,    40,   154,   155,   156,   157,
     158,    62,    63,    64,    65,    66,    67,    68,    30,    69,
      70,    71,   307,    72,    73,    74,   315,   142,   143,   144,
     145,   146,   310,   311,   330,   313,    75,   314,   316,   320,
      76,   323,    77,   340,   326,   333,   334,   341,    78,    52,
     331,    27,    53,    54,    55,   129,    56,    57,    58,    59,
      60,   377,   384,   386,   387,   192,   388,    61,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     389,   391,   392,   343,   404,   251,    62,    63,    64,    65,
      66,    67,    68,   394,    69,    70,    71,   400,    72,    73,
      74,   412,   142,   143,   144,   145,   146,   410,   414,   370,
     416,    75,   418,   421,   432,    76,   449,    77,   444,   451,
     452,   433,    41,    78,    52,   371,    27,    53,    54,    55,
     471,    56,    57,    58,    59,    60,   474,   493,   375,   296,
     242,    12,    61,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   425,    48,   354,   411,   408,
     253,    62,    63,    64,    65,    66,    67,    68,   321,    69,
      70,    71,   236,    72,    73,    74,   431,   142,   143,   144,
     145,   146,   229,   198,   456,   457,    75,   248,     0,   306,
      76,   122,    77,     0,     0,     0,     0,     0,    78,    52,
    -183,    27,    53,    54,    55,     0,    56,    57,    58,    59,
      60,     0,     0,     0,     0,     0,     0,    61,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     465,   466,   467,   468,   469,   343,    62,    63,    64,    65,
      66,    67,    68,     0,    69,    70,    71,     0,    72,    73,
      74,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,     0,     0,     0,    76,     0,    77,     0,     0,
       0,     0,     0,    78,   258,  -185,    27,    53,    54,   133,
       0,    56,    57,    58,    59,    60,     0,     0,     0,     0,
       0,     0,    61,   259,     0,   260,     0,     0,     0,     0,
     261,   262,   263,   264,     0,   265,   266,   267,     0,     0,
       0,    62,    63,    64,    65,    66,    67,    68,   -40,    69,
      70,    71,   134,    72,    73,    74,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
      76,   268,    77,     0,     0,     0,   129,   -76,    78,    52,
       0,    27,    53,    54,    55,     0,    56,    57,    58,    59,
      60,     0,     0,     0,     0,     0,     0,    61,   259,     0,
     260,   429,   430,     0,     0,   261,   262,   263,   264,     0,
     265,   266,   267,     0,     0,     0,    62,    63,    64,    65,
      66,    67,    68,     0,    69,    70,    71,     0,    72,    73,
      74,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,     0,     0,     0,    76,   268,    77,     0,     0,
       0,   129,  -110,    78,   258,     0,    27,    53,    54,    55,
       0,    56,    57,    58,    59,    60,     0,     0,     0,     0,
       0,     0,    61,   259,     0,   260,     0,     0,     0,     0,
     261,   262,   263,   264,     0,   265,   266,   267,     0,     0,
       0,    62,    63,    64,    65,    66,    67,    68,     0,    69,
      70,    71,     0,    72,    73,    74,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
      76,   268,    77,     0,     0,     0,   129,   -76,    78,    52,
       0,    27,    53,    54,    55,     0,    56,    57,    58,    59,
      60,     0,     0,     0,     0,     0,     0,    61,   259,     0,
     260,     0,     0,     0,     0,   261,   262,   263,   264,     0,
     265,   266,   267,     0,     0,     0,    62,    63,    64,    65,
      66,    67,    68,     0,    69,    70,    71,     0,    72,    73,
      74,   142,   143,   144,   145,   146,     0,     0,     0,     0,
       0,    75,     0,     0,     0,    76,   268,    77,     0,     0,
       0,   129,    52,    78,    27,    53,    54,   133,     0,    56,
      57,    58,    59,    60,     0,     0,     0,     0,     0,     0,
      61,     0,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,     0,     0,     0,     0,   188,    62,
      63,    64,    65,    66,    67,    68,   -40,    69,    70,    71,
     134,    72,    73,    74,   142,   143,   144,   145,   146,     0,
       0,     0,     0,     0,    75,   135,     0,     0,    76,     0,
      77,     0,     0,     0,     0,    52,    78,    27,    53,    54,
     133,     0,    56,    57,    58,    59,    60,     0,     0,     0,
       0,     0,     0,    61,     0,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,     0,     0,     0,
       0,   285,    62,    63,    64,    65,    66,    67,    68,   -40,
      69,    70,    71,   134,    72,    73,    74,   142,   143,   144,
     145,   146,     0,     0,     0,     0,     0,    75,     0,     0,
       0,    76,  -102,    77,     0,     0,     0,     0,    52,    78,
      27,    53,    54,    55,     0,    56,    57,    58,    59,    60,
       0,     0,     0,     0,     0,     0,    61,     0,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
       0,     0,     0,   422,     0,    62,    63,    64,    65,    66,
      67,    68,     0,    69,    70,    71,     0,    72,    73,    74,
       0,   142,   143,   144,   145,   146,     0,     0,     0,     0,
      75,     0,     0,     0,    76,     0,    77,  -178,     0,     0,
       0,  -178,    78,    52,     0,    27,    53,    54,    55,     0,
      56,    57,    58,    59,    60,     0,     0,     0,     0,     0,
       0,    61,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,     0,     0,     0,     0,     0,   345,
      62,    63,    64,    65,    66,    67,    68,     0,    69,    70,
      71,     0,    72,    73,    74,     0,     0,   456,   457,   458,
       0,     0,     0,     0,     0,    75,     0,     0,     0,    76,
       0,    77,     0,     0,     0,     0,  -176,    78,    52,     0,
      27,    53,    54,    55,     0,    56,    57,    58,    59,    60,
       0,     0,     0,     0,     0,     0,    61,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,     0,     0,
       0,   475,     0,     0,     0,    62,    63,    64,    65,    66,
      67,    68,     0,    69,    70,    71,     0,    72,    73,    74,
     142,   143,   144,   145,   146,     0,     0,     0,     0,     0,
      75,   135,   185,     0,    76,     0,    77,     0,     0,     0,
       0,    52,    78,    27,    53,    54,    55,     0,    56,    57,
      58,    59,    60,     0,     0,     0,     0,     0,     0,    61,
       0,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,     0,     0,     0,     0,     0,    62,    63,
      64,    65,    66,    67,    68,     0,    69,    70,    71,     0,
      72,    73,    74,   142,   143,   144,   145,   146,     0,     0,
       0,   232,     0,    75,     0,     0,     0,    76,     0,    77,
       0,     0,     0,     0,    52,    78,    27,    53,    54,    55,
       0,    56,    57,    58,    59,    60,     0,     0,     0,     0,
       0,     0,    61,     0,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,     0,     0,     0,     0,
       0,    62,    63,    64,    65,    66,    67,    68,     0,    69,
      70,    71,     0,    72,    73,    74,   142,     0,   144,   145,
     146,     0,     0,     0,     0,     0,    75,     0,     0,     0,
      76,     0,    77,  -176,     0,     0,     0,    52,    78,    27,
      53,    54,    55,     0,    56,    57,    58,    59,    60,     0,
       0,     0,     0,     0,     0,    61,     0,     0,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,     0,
       0,     0,     0,     0,    62,    63,    64,    65,    66,    67,
      68,     0,    69,    70,    71,     0,    72,    73,    74,   456,
     457,   458,     0,     0,     0,     0,     0,     0,     0,    75,
       0,     0,     0,    76,   317,    77,     0,     0,     0,     0,
      52,    78,    27,    53,    54,    55,     0,    56,    57,    58,
      59,    60,     0,     0,     0,     0,     0,     0,    61,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
       0,     0,     0,     0,     0,     0,     0,    62,    63,    64,
      65,    66,    67,    68,     0,    69,    70,    71,     0,    72,
      73,    74,   456,   457,   458,     0,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,    76,  -102,    77,     0,
       0,     0,     0,    52,    78,    27,    53,    54,    55,     0,
      56,    57,    58,    59,    60,     0,     0,     0,     0,     0,
       0,    61,     0,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,    65,    66,    67,    68,     0,    69,    70,
      71,     0,    72,    73,    74,   144,   145,   146,     0,     0,
       0,     0,     0,     0,     0,    75,     0,     0,     0,    76,
       0,    77,  -102,     0,     0,     0,    52,    78,    27,    53,
      54,    55,     0,    56,    57,    58,    59,    60,     0,     0,
       0,     0,     0,     0,    61,     0,     0,   150,   151,   152,
     153,   154,   155,   156,   157,   158,     0,     0,     0,   456,
     457,   458,     0,    62,    63,    64,    65,    66,    67,    68,
       0,    69,    70,    71,     0,    72,    73,    74,   144,   145,
     146,     0,     0,     0,     0,     0,     0,     0,    75,   456,
     457,   458,    76,     0,    77,     0,     0,     0,     0,     0,
      78,   461,   462,   463,   464,   465,   466,   467,   468,   469,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   151,   152,   153,   154,   155,   156,   157,   158,   453,
       0,     0,   462,   463,   464,   465,   466,   467,   468,   469,
       0,     0,     0,     0,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,     0,
       0,     0,     0,     0,     0,     0,     0,   453
};

static const yytype_int16 yycheck[] =
{
      16,    16,    10,    45,    11,    59,    60,    59,    60,   237,
      74,     3,    28,    28,   313,   314,    35,    36,   182,    61,
     187,    63,    64,   202,     1,    41,    41,    13,    13,    83,
       6,    83,    39,    75,    76,   437,   200,    38,    25,    62,
      82,    25,    17,     6,    38,     8,     9,    70,    38,    24,
      62,    25,   454,    26,    65,     0,    68,    76,     0,   101,
     124,     7,   126,     4,    68,     7,    77,    64,    69,    77,
     234,   235,    47,    49,    45,    69,    69,    69,    19,    69,
      73,   123,    68,   125,    69,    72,    49,    62,    72,   388,
      77,    68,   100,    77,   273,   106,    73,    71,    74,    72,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   128,   387,   161,
     287,   163,    63,    70,   135,    72,    67,   102,    69,   136,
     105,     6,    45,   312,    69,     3,     4,   365,    73,     6,
     182,     8,     9,     8,     9,   187,   188,    68,   190,    72,
     192,    19,   421,    72,    77,   166,    72,   385,   200,     3,
      72,    77,    37,   327,   139,    77,   173,    73,    72,     6,
      45,     8,     9,    77,    49,     3,     4,    73,    11,    12,
      13,     3,    49,   194,     6,    50,     8,     9,    10,   164,
     165,    19,   234,   235,   202,    63,    71,   208,    70,    67,
      72,    69,   381,    71,   368,   445,    68,   447,    69,   251,
      72,   253,    49,    16,    17,    18,    38,    39,    40,    41,
      42,   232,    44,    70,   403,    72,     3,     4,    50,    51,
      52,   295,    73,    70,    68,    63,   300,   416,    72,    67,
      13,    69,    19,   285,    70,   287,    72,    69,    25,   428,
      11,    12,    13,    75,     6,    70,   267,    72,    61,    62,
      63,    64,    65,    66,    62,    63,    64,    65,    66,   448,
      38,    23,    24,    38,    35,    36,    64,    65,    66,   290,
       8,     9,    16,    17,    18,   327,    63,     8,     9,   343,
      67,   343,    69,    16,    17,    18,    38,    49,   309,   310,
     311,    13,   313,   345,    64,    65,    66,    16,    17,    18,
      74,    70,   319,    72,    70,    76,    72,   328,   376,   330,
     378,    70,   297,    72,    71,    72,   368,    61,    62,    63,
      64,    65,    66,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,   386,    69,   353,   389,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,   400,   370,
      62,    63,    64,    65,    66,    70,    69,    72,   410,    70,
      69,    72,   347,    70,    69,    72,   387,    71,    72,    69,
      16,    17,     8,     9,   391,     8,     9,    71,    72,     8,
       9,    69,    69,   404,     1,    77,     3,     4,     5,     6,
      72,     8,     9,    10,    11,    12,    72,    49,    70,    46,
     421,    70,    19,   429,    70,    70,    70,   425,    70,    69,
      69,   437,    70,    70,   440,   440,    62,    63,    64,    65,
      66,    38,    39,    40,    41,    42,    43,    44,   454,    46,
      47,    48,    70,    50,    51,    52,    68,    14,    15,    16,
      17,    18,    69,    69,    61,    69,    63,    69,    68,    74,
      67,    69,    69,    72,    70,    70,    70,    68,    75,     1,
      77,     3,     4,     5,     6,    73,     8,     9,    10,    11,
      12,    70,    27,    13,    68,    72,    72,    19,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      31,    72,    68,    72,    69,    72,    38,    39,    40,    41,
      42,    43,    44,    70,    46,    47,    48,    71,    50,    51,
      52,    70,    14,    15,    16,    17,    18,    13,    70,    61,
      21,    63,    73,    68,    68,    67,     4,    69,    71,     4,
       4,    70,    62,    75,     1,    77,     3,     4,     5,     6,
      74,     8,     9,    10,    11,    12,    71,    71,   340,   240,
     181,     4,    19,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,   418,    39,   313,   391,   388,
      72,    38,    39,    40,    41,    42,    43,    44,   273,    46,
      47,    48,   172,    50,    51,    52,   425,    14,    15,    16,
      17,    18,   163,   126,    16,    17,    63,   190,    -1,   255,
      67,    65,    69,    -1,    -1,    -1,    -1,    -1,    75,     1,
      77,     3,     4,     5,     6,    -1,     8,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    19,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      62,    63,    64,    65,    66,    72,    38,    39,    40,    41,
      42,    43,    44,    -1,    46,    47,    48,    -1,    50,    51,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    63,    -1,    -1,    -1,    67,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    75,     1,    77,     3,     4,     5,     6,
      -1,     8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    19,    20,    -1,    22,    -1,    -1,    -1,    -1,
      27,    28,    29,    30,    -1,    32,    33,    34,    -1,    -1,
      -1,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,
      67,    68,    69,    -1,    -1,    -1,    73,    74,    75,     1,
      -1,     3,     4,     5,     6,    -1,     8,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,    -1,
      22,    23,    24,    -1,    -1,    27,    28,    29,    30,    -1,
      32,    33,    34,    -1,    -1,    -1,    38,    39,    40,    41,
      42,    43,    44,    -1,    46,    47,    48,    -1,    50,    51,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    63,    -1,    -1,    -1,    67,    68,    69,    -1,    -1,
      -1,    73,    74,    75,     1,    -1,     3,     4,     5,     6,
      -1,     8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    19,    20,    -1,    22,    -1,    -1,    -1,    -1,
      27,    28,    29,    30,    -1,    32,    33,    34,    -1,    -1,
      -1,    38,    39,    40,    41,    42,    43,    44,    -1,    46,
      47,    48,    -1,    50,    51,    52,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,
      67,    68,    69,    -1,    -1,    -1,    73,    74,    75,     1,
      -1,     3,     4,     5,     6,    -1,     8,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,    -1,
      22,    -1,    -1,    -1,    -1,    27,    28,    29,    30,    -1,
      32,    33,    34,    -1,    -1,    -1,    38,    39,    40,    41,
      42,    43,    44,    -1,    46,    47,    48,    -1,    50,    51,
      52,    14,    15,    16,    17,    18,    -1,    -1,    -1,    -1,
      -1,    63,    -1,    -1,    -1,    67,    68,    69,    -1,    -1,
      -1,    73,     1,    75,     3,     4,     5,     6,    -1,     8,
       9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      19,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    -1,    -1,    -1,    71,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    14,    15,    16,    17,    18,    -1,
      -1,    -1,    -1,    -1,    63,    64,    -1,    -1,    67,    -1,
      69,    -1,    -1,    -1,    -1,     1,    75,     3,     4,     5,
       6,    -1,     8,     9,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    19,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      -1,    71,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    14,    15,    16,
      17,    18,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,
      -1,    67,    68,    69,    -1,    -1,    -1,    -1,     1,    75,
       3,     4,     5,     6,    -1,     8,     9,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    19,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    -1,    -1,    70,    -1,    38,    39,    40,    41,    42,
      43,    44,    -1,    46,    47,    48,    -1,    50,    51,    52,
      -1,    14,    15,    16,    17,    18,    -1,    -1,    -1,    -1,
      63,    -1,    -1,    -1,    67,    -1,    69,    70,    -1,    -1,
      -1,    74,    75,     1,    -1,     3,     4,     5,     6,    -1,
       8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    -1,    -1,    -1,    -1,    72,
      38,    39,    40,    41,    42,    43,    44,    -1,    46,    47,
      48,    -1,    50,    51,    52,    -1,    -1,    16,    17,    18,
      -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,    67,
      -1,    69,    -1,    -1,    -1,    -1,    74,    75,     1,    -1,
       3,     4,     5,     6,    -1,     8,     9,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    19,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    -1,
      -1,    70,    -1,    -1,    -1,    38,    39,    40,    41,    42,
      43,    44,    -1,    46,    47,    48,    -1,    50,    51,    52,
      14,    15,    16,    17,    18,    -1,    -1,    -1,    -1,    -1,
      63,    64,    26,    -1,    67,    -1,    69,    -1,    -1,    -1,
      -1,     1,    75,     3,     4,     5,     6,    -1,     8,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    19,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    -1,    -1,    -1,    -1,    38,    39,
      40,    41,    42,    43,    44,    -1,    46,    47,    48,    -1,
      50,    51,    52,    14,    15,    16,    17,    18,    -1,    -1,
      -1,    61,    -1,    63,    -1,    -1,    -1,    67,    -1,    69,
      -1,    -1,    -1,    -1,     1,    75,     3,     4,     5,     6,
      -1,     8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    19,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    -1,    -1,
      -1,    38,    39,    40,    41,    42,    43,    44,    -1,    46,
      47,    48,    -1,    50,    51,    52,    14,    -1,    16,    17,
      18,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,
      67,    -1,    69,    70,    -1,    -1,    -1,     1,    75,     3,
       4,     5,     6,    -1,     8,     9,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    19,    -1,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      -1,    -1,    -1,    -1,    38,    39,    40,    41,    42,    43,
      44,    -1,    46,    47,    48,    -1,    50,    51,    52,    16,
      17,    18,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,
      -1,    -1,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,
       1,    75,     3,     4,     5,     6,    -1,     8,     9,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    19,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    39,    40,
      41,    42,    43,    44,    -1,    46,    47,    48,    -1,    50,
      51,    52,    16,    17,    18,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    63,    -1,    -1,    -1,    67,    68,    69,    -1,
      -1,    -1,    -1,     1,    75,     3,     4,     5,     6,    -1,
       8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    -1,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      38,    39,    40,    41,    42,    43,    44,    -1,    46,    47,
      48,    -1,    50,    51,    52,    16,    17,    18,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,    67,
      -1,    69,    70,    -1,    -1,    -1,     1,    75,     3,     4,
       5,     6,    -1,     8,     9,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    19,    -1,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    -1,    16,
      17,    18,    -1,    38,    39,    40,    41,    42,    43,    44,
      -1,    46,    47,    48,    -1,    50,    51,    52,    16,    17,
      18,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    16,
      17,    18,    67,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      75,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    59,    60,    61,    62,    63,    64,    65,    66,   440,
      -1,    -1,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    -1,    -1,    -1,    -1,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   478
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    79,    80,     0,     7,    82,    88,    91,    92,    97,
     107,   108,   107,    68,    81,     6,    37,    49,    71,   100,
     101,   102,   110,    64,    85,   111,   112,     3,    69,   166,
     167,     8,     9,    87,    45,     8,     9,    87,    68,    72,
     166,    62,    68,     3,    73,    13,    89,    85,   111,    70,
     166,    98,     1,     4,     5,     6,     8,     9,    10,    11,
      12,    19,    38,    39,    40,    41,    42,    43,    44,    46,
      47,    48,    50,    51,    52,    63,    67,    69,    75,    83,
      84,    87,   109,   142,   143,   151,   152,   153,   157,   159,
     160,   161,   162,   165,   167,   170,   177,   178,    69,    87,
      95,    13,    38,   155,    69,    38,    69,   151,   153,   151,
     143,    87,   143,   145,   146,   147,   143,   148,   149,   150,
     141,   143,   177,    69,   158,    69,   163,    69,    69,    73,
     113,   143,   143,     6,    49,    64,   102,   141,    69,    38,
     143,   151,    14,    15,    16,    17,    18,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    11,
      12,    13,    71,    72,    35,    36,    76,    69,    69,     8,
       9,    99,   102,   103,   104,   105,   106,    74,   102,   143,
      87,    69,   173,    50,    87,    26,    74,    72,    71,    77,
      72,    71,    72,   143,    69,   113,   156,   143,   156,    49,
     172,   141,   115,     8,     9,   141,    85,    70,   154,    87,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   143,   143,   143,   143,    70,   147,
      87,    87,    61,   141,   171,   174,   142,    85,    70,    26,
      72,    96,   105,   145,    70,   146,   143,    70,   150,    70,
     143,    72,   141,    72,     8,     9,   145,    70,     1,    20,
      22,    27,    28,    29,    30,    32,    33,    34,    68,   102,
     113,   114,   121,   122,   123,   125,   127,   131,   135,   141,
     144,   179,    70,    70,   141,    71,    71,    72,    69,   141,
      25,    77,   145,   145,    99,    90,   104,    85,    93,    94,
      70,    70,   143,    70,   143,   169,   169,    70,    68,    69,
      69,    69,   126,    69,    69,    68,    68,    68,   141,   116,
      74,   121,    68,    69,    70,   143,    70,   175,    25,    77,
      61,    77,   141,    70,    70,     1,    68,    86,   113,    87,
      72,    68,   113,    72,   164,    72,    70,    72,    70,   141,
     141,   141,   122,   103,   118,   119,   133,   134,   141,     8,
       9,   118,   129,   130,    68,    85,   117,   120,   176,   145,
      61,    77,   141,   141,    77,    94,   151,    70,   143,    87,
     168,    70,    70,    70,    27,    85,    13,    68,    72,    31,
      99,    72,    68,   145,    70,   141,    77,    77,   164,   164,
      71,   122,   136,   124,    69,    99,   143,   133,   129,   143,
      13,   120,    70,    77,    70,   143,    21,   180,    73,   122,
     141,    68,    70,   143,   122,   115,    70,   133,   132,    23,
      24,   138,    68,    70,   122,     4,    19,    25,    63,    67,
      69,   139,   140,   166,    71,   122,   137,   138,   128,     4,
     139,     4,     4,   140,    25,    71,    16,    17,    18,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
     137,    74,   137,   122,    71,    70,    71,   139,    69,   140,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   140,
     140,   140,   140,    71
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    78,    79,    80,    80,    81,    81,    82,    83,    84,
      85,    85,    86,    86,    86,    87,    87,    89,    90,    88,
      91,    91,    91,    91,    91,    92,    93,    94,    94,    95,
      96,    95,    98,    97,    99,    99,   100,   100,   100,   101,
     101,   102,   102,   103,   103,   104,   104,   104,   105,   105,
     105,   106,   106,   107,   107,   108,   109,   110,   110,   111,
     111,   112,   112,   113,   114,   114,   114,   115,   116,   115,
     117,   117,   118,   119,   120,   120,   121,   121,   121,   122,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   124,
     123,   126,   125,   128,   127,   129,   129,   129,   130,   130,
     132,   131,   133,   133,   134,   134,   136,   135,   137,   137,
     137,   138,   138,   138,   138,   138,   139,   139,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   141,   141,   142,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   144,   144,   145,   145,   145,   146,
     146,   147,   147,   148,   148,   148,   149,   149,   150,   151,
     152,   152,   153,   153,   153,   153,   154,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   155,   153,   153,   153,   153,   153,   153,   156,
     156,   158,   157,   159,   159,   160,   161,   163,   162,   164,
     164,   165,   166,   166,   166,   167,   167,   168,   169,   169,
     171,   170,   172,   170,   170,   170,   173,   170,   174,   170,
     175,   170,   176,   170,   177,   177,   178,   178,   178,   178,
     179,   180,   180
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     3,     0,     0,     1,     4,     1,     1,
       0,     1,     1,     1,     1,     1,     1,     0,     0,     9,
       1,     3,     1,     1,     1,     2,     2,     1,     3,     0,
       0,     5,     0,     7,     1,     1,     1,     2,     2,     1,
       0,     1,     2,     1,     2,     2,     3,     1,     0,     1,
       2,     1,     3,     0,     2,     2,     4,     1,     0,     1,
       3,     2,     4,     4,     1,     1,     1,     0,     0,     5,
       2,     4,     3,     3,     1,     3,     0,     2,     2,     2,
       1,     1,     1,     1,     1,     1,     1,     2,     2,     0,
       6,     0,     8,     0,    10,     1,     1,     1,     1,     3,
       0,     8,     0,     1,     1,     1,     0,    10,     2,     2,
       0,     3,     5,     4,     4,     2,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     2,     2,     2,     1,     3,     1,
       2,     3,     3,     5,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     2,     2,     2,     2,     2,     1,     1,
       1,     1,     1,     1,     2,     3,     0,     1,     2,     1,
       2,     1,     3,     0,     1,     2,     1,     3,     3,     1,
       1,     2,     1,     1,     1,     1,     0,     5,     3,     3,
       6,     7,     8,     7,     5,     6,     5,     4,     1,     3,
       1,     1,     0,     6,     3,     5,     4,     4,     4,     1,
       3,     0,     3,     2,     4,     7,     9,     0,     3,     0,
       3,     1,     1,     3,     3,     1,     2,     3,     0,     3,
       0,     5,     0,     5,     6,     6,     0,     5,     0,     5,
       0,     7,     0,     8,     3,     3,     1,     2,     3,     3,
       6,     0,     2
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
#line 215 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
          { rule_program((yyval.node)); }
#line 2375 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 3: /* program: program def possible_semi_colon  */
#line 219 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                  { CREATE_TWO_VALUES((yyval.node), 0, (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 2381 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 4: /* program: %empty  */
#line 220 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                        { (yyval.node) = 0; }
#line 2387 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 6: /* possible_semi_colon: ';'  */
#line 225 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
        { yywarn("Extra ';'. Ignored."); }
#line 2393 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 7: /* inheritance: type_modifier_list L_INHERIT string_con1 ';'  */
#line 229 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                               { if (rule_inheritence(&(yyval.node), (yyvsp[-3].number), (yyvsp[-1].string))) { YYACCEPT; } }
#line 2399 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 8: /* real: L_REAL  */
#line 233 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
         { CREATE_REAL((yyval.node), (yyvsp[0].real)); }
#line 2405 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 9: /* number: L_NUMBER  */
#line 237 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
           { CREATE_NUMBER((yyval.node), (yyvsp[0].number)); }
#line 2411 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 10: /* optional_star: %empty  */
#line 241 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                        { (yyval.number) = 0; }
#line 2417 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 11: /* optional_star: '*'  */
#line 242 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                { (yyval.number) = TYPE_MOD_ARRAY; }
#line 2423 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 12: /* block_or_semi: block  */
#line 247 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
          {
            (yyval.node) = (yyvsp[0].decl).node;
            if (!(yyval.node)) {
              CREATE_RETURN((yyval.node), 0);
            }
          }
#line 2434 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 13: /* block_or_semi: ';'  */
#line 253 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2440 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 14: /* block_or_semi: error  */
#line 254 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2446 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 15: /* identifier: L_DEFINED_NAME  */
#line 258 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                  { (yyval.string) = scratch_copy((yyvsp[0].ihe)->name); }
#line 2452 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 17: /* $@1: %empty  */
#line 263 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                  { (yyvsp[-2].number) = rule_func_type((yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].string)); }
#line 2458 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 18: /* @2: %empty  */
#line 264 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                  { (yyval.number) = rule_func_proto((yyvsp[-6].number), (yyvsp[-5].number), &(yyvsp[-4].string), (yyvsp[-1].argument)); }
#line 2464 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 19: /* function: type optional_star identifier $@1 '(' argument ')' @2 block_or_semi  */
#line 265 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                  { rule_func(&(yyval.node), (yyvsp[-8].number), (yyvsp[-7].number), (yyvsp[-6].string), (yyvsp[-3].argument), &(yyvsp[-1].number), &(yyvsp[0].node)); }
#line 2470 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 21: /* def: type name_list ';'  */
#line 271 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                  {
                                    if (!((yyvsp[-2].number) & ~(DECL_MODS)) && (pragmas & PRAGMA_STRICT_TYPES))
                                      yyerror("Missing type for global variable declaration");
                                    (yyval.node) = 0;
                                  }
#line 2480 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 25: /* modifier_change: type_modifier_list ':'  */
#line 283 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 2498 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 26: /* member_name: optional_star identifier  */
#line 300 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                  {
                                    /* At this point, the current_type here is only a basic_type */
                                    /* and cannot be unused yet - Sym */

                                    if (current_type == TYPE_VOID)
                                      yyerror("Illegal to declare class member of type void.");
                                    add_local_name((yyvsp[0].string), current_type | (yyvsp[-1].number));
                                    scratch_free((yyvsp[0].string));
                                  }
#line 2512 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 30: /* $@3: %empty  */
#line 318 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                           { current_type = (yyvsp[0].number); }
#line 2518 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 32: /* @4: %empty  */
#line 323 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                             { (yyvsp[-2].ihe) = rule_define_class(&(yyval.number), (yyvsp[-1].string)); }
#line 2524 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 33: /* type_decl: type_modifier_list L_CLASS identifier '{' @4 member_list '}'  */
#line 324 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                             { rule_define_class_members((yyvsp[-5].ihe), (yyvsp[-2].number)); (yyval.node) = 0; }
#line 2530 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 35: /* new_local_name: L_DEFINED_NAME  */
#line 330 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                            {
                                              if ((yyvsp[0].ihe)->dn.local_num != -1) {
                                                yyerror("Illegal to redeclare local name '%s'", (yyvsp[0].ihe)->name);
                                              }
                                              (yyval.string) = scratch_copy((yyvsp[0].ihe)->name);
                                            }
#line 2541 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 37: /* atomic_type: L_CLASS L_DEFINED_NAME  */
#line 341 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                            {
                                              if ((yyvsp[0].ihe)->dn.class_num == -1) {
                                                yyerror("Undefined class '%s'", (yyvsp[0].ihe)->name);
                                                (yyval.number) = TYPE_ANY;
                                              } else {
                                                (yyval.number) = (yyvsp[0].ihe)->dn.class_num | TYPE_MOD_CLASS;
                                              }
                                            }
#line 2554 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 38: /* atomic_type: L_CLASS L_IDENTIFIER  */
#line 350 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                            {
                                              yyerror("Undefined class '%s'", (yyvsp[0].string));
                                              (yyval.number) = TYPE_ANY;
                                            }
#line 2563 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 40: /* opt_atomic_type: %empty  */
#line 358 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                        { (yyval.number) = TYPE_ANY; }
#line 2569 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 42: /* basic_type: opt_atomic_type L_ARRAY  */
#line 363 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                            { (yyval.number) = (yyvsp[-1].number) | TYPE_MOD_ARRAY; }
#line 2575 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 44: /* arg_type: basic_type ref  */
#line 368 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                   { (yyval.number) = (yyvsp[-1].number) | LOCAL_MOD_REF; }
#line 2581 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 45: /* new_arg: arg_type optional_star  */
#line 373 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                              {
                                                (yyval.number) = (yyvsp[-1].number) | (yyvsp[0].number);
                                                if ((yyvsp[-1].number) != TYPE_VOID)
                                                  add_local_name("", (yyvsp[-1].number) | (yyvsp[0].number));
                                              }
#line 2591 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 46: /* new_arg: arg_type optional_star new_local_name  */
#line 379 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                              {
                                                if ((yyvsp[-2].number) == TYPE_VOID)
                                                  yyerror("Illegal to declare argument of type void.");
                                                add_local_name((yyvsp[0].string), (yyvsp[-2].number) | (yyvsp[-1].number));
                                                scratch_free((yyvsp[0].string));
                                                (yyval.number) = (yyvsp[-2].number) | (yyvsp[-1].number);
                                              }
#line 2603 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 47: /* new_arg: new_local_name  */
#line 387 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                              {
                                                if (exact_types) {
                                                  yyerror("Missing type for argument");
                                                }
                                                add_local_name((yyvsp[0].string), TYPE_ANY);
                                                scratch_free((yyvsp[0].string));
                                                (yyval.number) = TYPE_ANY;
                                              }
#line 2616 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 48: /* argument: %empty  */
#line 399 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.argument).num_arg = 0;
      (yyval.argument).flags = 0;
    }
#line 2625 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 50: /* argument: argument_list L_DOT_DOT_DOT  */
#line 405 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 2644 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 51: /* argument_list: new_arg  */
#line 423 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 2659 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 52: /* argument_list: argument_list ',' new_arg  */
#line 434 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if (!(yyval.argument).num_arg)    /* first arg was void w/no name */
        yyerror("argument of type void must be the only argument.");
      if (((yyvsp[0].number) & TYPE_MASK) == TYPE_VOID && !((yyvsp[0].number) & TYPE_MOD_CLASS))
        yyerror("Illegal to declare argument of type void.");

      (yyval.argument) = (yyvsp[-2].argument);
      (yyval.argument).num_arg++;
    }
#line 2673 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 53: /* type_modifier_list: %empty  */
#line 447 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.number) = 0;
    }
#line 2681 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 54: /* type_modifier_list: L_TYPE_MODIFIER type_modifier_list  */
#line 451 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 2699 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 55: /* type: type_modifier_list opt_basic_type  */
#line 468 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = ((yyvsp[-1].number) << 16) | (yyvsp[0].number);
      current_type = (yyval.number);
    }
#line 2708 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 56: /* cast: '(' basic_type optional_star ')'  */
#line 476 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) | (yyvsp[-1].number);
    }
#line 2716 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 58: /* opt_basic_type: %empty  */
#line 484 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.number) = TYPE_UNKNOWN;
    }
#line 2724 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 61: /* new_name: optional_star identifier  */
#line 496 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 2753 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 62: /* new_name: optional_star identifier L_ASSIGN expr0  */
#line 521 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 2809 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 63: /* block: '{' local_declarations statements '}'  */
#line 576 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-2].decl).node && (yyvsp[-1].node)) {
        CREATE_STATEMENTS((yyval.decl).node, (yyvsp[-2].decl).node, (yyvsp[-1].node));
      } else (yyval.decl).node = ((yyvsp[-2].decl).node ? (yyvsp[-2].decl).node : (yyvsp[-1].node));
      (yyval.decl).num = (yyvsp[-2].decl).num;
    }
#line 2820 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 67: /* local_declarations: %empty  */
#line 588 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.decl).node = 0;
      (yyval.decl).num = 0;
    }
#line 2829 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 68: /* $@5: %empty  */
#line 593 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[0].number) == TYPE_VOID)
        yyerror("Illegal to declare local variable of type void.");
      /* can't do this in basic_type b/c local_name_list contains
       * expr0 which contains cast which contains basic_type
       */
      current_type = (yyvsp[0].number);
    }
#line 2842 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 69: /* local_declarations: local_declarations basic_type $@5 local_name_list ';'  */
#line 602 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-4].decl).node && (yyvsp[-1].decl).node) {
        CREATE_STATEMENTS((yyval.decl).node, (yyvsp[-4].decl).node, (yyvsp[-1].decl).node);
      } else (yyval.decl).node = ((yyvsp[-4].decl).node ? (yyvsp[-4].decl).node : (yyvsp[-1].decl).node);
      (yyval.decl).num = (yyvsp[-4].decl).num + (yyvsp[-1].decl).num;
    }
#line 2853 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 70: /* new_local_def: optional_star new_local_name  */
#line 612 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if (current_type & LOCAL_MOD_REF) {
        yyerror("Illegal to declare local variable as reference");
        current_type &= ~LOCAL_MOD_REF;
      }
      add_local_name((yyvsp[0].string), current_type | (yyvsp[-1].number) | LOCAL_MOD_UNUSED);

      scratch_free((yyvsp[0].string));
      (yyval.node) = 0;
    }
#line 2868 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 71: /* new_local_def: optional_star new_local_name L_ASSIGN expr0  */
#line 623 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 2903 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 72: /* single_new_local_def: arg_type optional_star new_local_name  */
#line 657 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-2].number) == TYPE_VOID)
        yyerror("Illegal to declare local variable of type void.");

      (yyval.number) = add_local_name((yyvsp[0].string), (yyvsp[-2].number) | (yyvsp[-1].number));
      scratch_free((yyvsp[0].string));
    }
#line 2915 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 73: /* single_new_local_def_with_init: single_new_local_def L_ASSIGN expr0  */
#line 668 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 2948 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 74: /* local_name_list: new_local_def  */
#line 700 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.decl).node = (yyvsp[0].node);
      (yyval.decl).num = 1;
    }
#line 2957 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 75: /* local_name_list: new_local_def ',' local_name_list  */
#line 705 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-2].node) && (yyvsp[0].decl).node) {
        CREATE_STATEMENTS((yyval.decl).node, (yyvsp[-2].node), (yyvsp[0].decl).node);
      } else (yyval.decl).node = ((yyvsp[-2].node) ? (yyvsp[-2].node) : (yyvsp[0].decl).node);
      (yyval.decl).num = 1 + (yyvsp[0].decl).num;
    }
#line 2968 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 76: /* statements: %empty  */
#line 715 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.node) = 0;
    }
#line 2976 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 77: /* statements: statement statements  */
#line 719 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-1].node) && (yyvsp[0].node)) {
        CREATE_STATEMENTS((yyval.node), (yyvsp[-1].node), (yyvsp[0].node));
      } else (yyval.node) = ((yyvsp[-1].node) ? (yyvsp[-1].node) : (yyvsp[0].node));
    }
#line 2986 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 78: /* statements: error ';'  */
#line 725 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = 0;
    }
#line 2994 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 79: /* statement: comma_expr ';'  */
#line 732 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3010 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 85: /* statement: decl_block  */
#line 749 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[0].decl).node;
      pop_n_locals((yyvsp[0].decl).num);
    }
#line 3019 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 86: /* statement: ';'  */
#line 754 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = 0;
    }
#line 3027 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 87: /* statement: L_BREAK ';'  */
#line 758 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3053 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 88: /* statement: L_CONTINUE ';'  */
#line 780 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if (context & SPECIAL_CONTEXT)
        yyerror("Cannot continue out of catch { } or time_expression { }");
      else
        if (!(context & LOOP_CONTEXT))
          yyerror("continue statement outside loop");
      CREATE_CONTROL_JUMP((yyval.node), CJ_CONTINUE);
    }
#line 3066 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 89: /* $@6: %empty  */
#line 792 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[-3].number) = context;
      context = LOOP_CONTEXT;
    }
#line 3075 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 90: /* while: L_WHILE '(' comma_expr ')' $@6 statement  */
#line 797 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_LOOP((yyval.node), 1, (yyvsp[0].node), 0, optimize_loop_test((yyvsp[-3].node)));
      context = (yyvsp[-5].number);
    }
#line 3084 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 91: /* $@7: %empty  */
#line 805 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[0].number) = context;
      context = LOOP_CONTEXT;
    }
#line 3093 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 92: /* do: L_DO $@7 statement L_WHILE '(' comma_expr ')' ';'  */
#line 810 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_LOOP((yyval.node), 0, (yyvsp[-5].node), 0, optimize_loop_test((yyvsp[-2].node)));
      context = (yyvsp[-7].number);
    }
#line 3102 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 93: /* $@8: %empty  */
#line 818 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[-5].decl).node = pop_value((yyvsp[-5].decl).node);
      (yyvsp[-7].number) = context;
      context = LOOP_CONTEXT;
    }
#line 3112 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 94: /* for: L_FOR '(' first_for_expr ';' for_expr ';' for_expr ')' $@8 statement  */
#line 824 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3132 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 95: /* foreach_var: L_DEFINED_NAME  */
#line 843 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3157 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 96: /* foreach_var: single_new_local_def  */
#line 864 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if (type_of_locals_ptr[(yyvsp[0].number)] & LOCAL_MOD_REF) {
        CREATE_OPCODE_1((yyval.decl).node, F_REF_LVALUE, 0, (yyvsp[0].number));
      } else {
        CREATE_OPCODE_1((yyval.decl).node, F_LOCAL_LVALUE, 0, (yyvsp[0].number));
        type_of_locals_ptr[(yyvsp[0].number)] &= ~LOCAL_MOD_UNUSED;
      }
      (yyval.decl).num = 1;
    }
#line 3171 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 97: /* foreach_var: L_IDENTIFIER  */
#line 874 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3189 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 98: /* foreach_vars: foreach_var  */
#line 891 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_FOREACH((yyval.decl).node, (yyvsp[0].decl).node, 0);
      (yyval.decl).num = (yyvsp[0].decl).num;
    }
#line 3198 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 99: /* foreach_vars: foreach_var ',' foreach_var  */
#line 896 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_FOREACH((yyval.decl).node, (yyvsp[-2].decl).node, (yyvsp[0].decl).node);
      (yyval.decl).num = (yyvsp[-2].decl).num + (yyvsp[0].decl).num;
      if ((yyvsp[-2].decl).node->v.number == F_REF_LVALUE)
        yyerror("Mapping key may not be a reference in foreach()");
    }
#line 3209 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 100: /* $@9: %empty  */
#line 906 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[-3].decl).node->v.expr = (yyvsp[-1].node);
      (yyvsp[-5].number) = context;
      context = LOOP_CONTEXT | LOOP_FOREACH;
    }
#line 3219 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 101: /* foreach: L_FOREACH '(' foreach_vars L_IN expr0 ')' $@9 statement  */
#line 912 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.decl).num = (yyvsp[-5].decl).num;

      CREATE_STATEMENTS((yyval.decl).node, (yyvsp[-5].decl).node, 0);
      CREATE_LOOP((yyval.decl).node->r.expr, 2, (yyvsp[0].node), 0, 0);
      CREATE_OPCODE((yyval.decl).node->r.expr->r.expr, F_NEXT_FOREACH, 0);

      context = (yyvsp[-7].number);
    }
#line 3233 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 102: /* for_expr: %empty  */
#line 925 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.node) = 0;
    }
#line 3241 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 104: /* first_for_expr: for_expr  */
#line 933 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.decl).node = (yyvsp[0].node);
      (yyval.decl).num = 0;
    }
#line 3250 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 105: /* first_for_expr: single_new_local_def_with_init  */
#line 938 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.decl).node = (yyvsp[0].node);
      (yyval.decl).num = 1;
    }
#line 3259 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 106: /* $@10: %empty  */
#line 946 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[-3].number) = context;
      context &= LOOP_CONTEXT;
      context |= SWITCH_CONTEXT;
      (yyvsp[-2].number) = mem_block[A_CASES].current_size;
    }
#line 3270 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 107: /* switch: L_SWITCH '(' comma_expr ')' $@10 '{' local_declarations case switch_block '}'  */
#line 953 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3303 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 108: /* switch_block: case switch_block  */
#line 985 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[0].node)){
        CREATE_STATEMENTS((yyval.node), (yyvsp[-1].node), (yyvsp[0].node));
      } else (yyval.node) = (yyvsp[-1].node);
    }
#line 3313 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 109: /* switch_block: statement switch_block  */
#line 991 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[0].node)){
        CREATE_STATEMENTS((yyval.node), (yyvsp[-1].node), (yyvsp[0].node));
      } else (yyval.node) = (yyvsp[-1].node);
    }
#line 3323 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 110: /* switch_block: %empty  */
#line 997 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.node) = 0;
    }
#line 3331 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 111: /* case: L_CASE case_label ':'  */
#line 1004 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[-1].node);
      (yyval.node)->v.expr = 0;

      add_to_mem_block(A_CASES, (char *)&((yyvsp[-1].node)), sizeof((yyvsp[-1].node)));
    }
#line 3342 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 112: /* case: L_CASE case_label L_RANGE case_label ':'  */
#line 1011 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3360 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 113: /* case: L_CASE case_label L_RANGE ':'  */
#line 1025 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3378 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 114: /* case: L_CASE L_RANGE case_label ':'  */
#line 1039 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3395 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 115: /* case: L_DEFAULT ':'  */
#line 1052 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3412 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 116: /* case_label: constant  */
#line 1068 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3430 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 117: /* case_label: string_con1  */
#line 1082 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3446 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 118: /* constant: constant '|' constant  */
#line 1097 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) | (yyvsp[0].number);
    }
#line 3454 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 119: /* constant: constant '^' constant  */
#line 1101 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) ^ (yyvsp[0].number);
    }
#line 3462 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 120: /* constant: constant '&' constant  */
#line 1105 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) & (yyvsp[0].number);
    }
#line 3470 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 121: /* constant: constant L_EQ constant  */
#line 1109 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) == (yyvsp[0].number);
    }
#line 3478 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 122: /* constant: constant L_NE constant  */
#line 1113 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) != (yyvsp[0].number);
    }
#line 3486 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 123: /* constant: constant L_ORDER constant  */
#line 1117 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      switch((yyvsp[-1].number)){
        case F_GE: (yyval.number) = (yyvsp[-2].number) >= (yyvsp[0].number); break;
        case F_LE: (yyval.number) = (yyvsp[-2].number) <= (yyvsp[0].number); break;
        case F_GT: (yyval.number) = (yyvsp[-2].number) >  (yyvsp[0].number); break;
      }
    }
#line 3498 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 124: /* constant: constant '<' constant  */
#line 1125 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) < (yyvsp[0].number);
    }
#line 3506 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 125: /* constant: constant L_LSH constant  */
#line 1129 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) << (yyvsp[0].number);
    }
#line 3514 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 126: /* constant: constant L_RSH constant  */
#line 1133 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) >> (yyvsp[0].number);
    }
#line 3522 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 127: /* constant: constant '+' constant  */
#line 1137 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) + (yyvsp[0].number);
    }
#line 3530 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 128: /* constant: constant '-' constant  */
#line 1141 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) - (yyvsp[0].number);
    }
#line 3538 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 129: /* constant: constant '*' constant  */
#line 1145 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) * (yyvsp[0].number);
    }
#line 3546 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 130: /* constant: constant '%' constant  */
#line 1149 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[0].number)) (yyval.number) = (yyvsp[-2].number) % (yyvsp[0].number); else yyerror("Modulo by zero");
    }
#line 3554 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 131: /* constant: constant '/' constant  */
#line 1153 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[0].number)) (yyval.number) = (yyvsp[-2].number) / (yyvsp[0].number); else yyerror("Division by zero");
    }
#line 3562 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 132: /* constant: '(' constant ')'  */
#line 1157 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-1].number);
    }
#line 3570 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 133: /* constant: L_NUMBER  */
#line 1161 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[0].number);
    }
#line 3578 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 134: /* constant: '-' L_NUMBER  */
#line 1165 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = -(yyvsp[0].number);
    }
#line 3586 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 135: /* constant: L_NOT L_NUMBER  */
#line 1169 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = !(yyvsp[0].number);
    }
#line 3594 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 136: /* constant: '~' L_NUMBER  */
#line 1173 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = ~(yyvsp[0].number);
    }
#line 3602 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 137: /* comma_expr: expr0  */
#line 1180 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3610 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 138: /* comma_expr: comma_expr ',' expr0  */
#line 1184 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_TWO_VALUES((yyval.node), (yyvsp[0].node)->type, pop_value((yyvsp[-2].node)), (yyvsp[0].node));
    }
#line 3618 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 140: /* expr0: ref lvalue  */
#line 1195 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3649 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 141: /* expr0: lvalue L_ASSIGN expr0  */
#line 1222 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      parse_node_t *l = (yyvsp[-2].node), *r = (yyvsp[0].node);
      /* set this up here so we can change it below */
      /* assignments are backwards; rhs is evaluated before
         lhs, so put the RIGHT hand side on the LEFT hand
         side of the tree node. */
      CREATE_BINARY_OP((yyval.node), (yyvsp[-1].number), r->type, r, l);

      /* allow TYPE_STRING += TYPE_NUMBER | TYPE_OBJECT */
      if (exact_types && !compatible_types(r->type, l->type) &&
          !((yyvsp[-1].number) == F_ADD_EQ && l->type == TYPE_STRING &&
            ((COMP_TYPE(r->type, TYPE_NUMBER)) || r->type == TYPE_OBJECT))) {
        char buf[256];
        char *end = EndOf(buf);
        char *p;
        p = strput(buf, end, "Bad assignment ");
        p = get_two_types(p, end, l->type, r->type);
        p = strput(p, end, ".");
        yyerror(buf);
      }

      if ((yyvsp[-1].number) == F_ASSIGN)
        (yyval.node)->l.expr = do_promotions(r, l->type);
    }
#line 3678 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 142: /* expr0: error L_ASSIGN expr0  */
#line 1247 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      yyerror("Illegal LHS");
      CREATE_ERROR((yyval.node));
    }
#line 3687 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 143: /* expr0: expr0 '?' expr0 ':' expr0  */
#line 1252 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3715 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 144: /* expr0: expr0 L_LOR expr0  */
#line 1276 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_LAND_LOR((yyval.node), F_LOR, (yyvsp[-2].node), (yyvsp[0].node));
      if (IS_NODE((yyvsp[-2].node), NODE_LAND_LOR, F_LOR))
        (yyvsp[-2].node)->kind = NODE_BRANCH_LINK;
    }
#line 3725 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 145: /* expr0: expr0 L_LAND expr0  */
#line 1282 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_LAND_LOR((yyval.node), F_LAND, (yyvsp[-2].node), (yyvsp[0].node));
      if (IS_NODE((yyvsp[-2].node), NODE_LAND_LOR, F_LAND))
        (yyvsp[-2].node)->kind = NODE_BRANCH_LINK;
    }
#line 3735 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 146: /* expr0: expr0 '|' expr0  */
#line 1288 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3764 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 147: /* expr0: expr0 '^' expr0  */
#line 1313 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = binary_int_op((yyvsp[-2].node), (yyvsp[0].node), F_XOR, "^");
    }
#line 3772 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 148: /* expr0: expr0 '&' expr0  */
#line 1317 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3799 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 149: /* expr0: expr0 L_EQ expr0  */
#line 1340 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3825 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 150: /* expr0: expr0 L_NE expr0  */
#line 1362 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3843 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 151: /* expr0: expr0 L_ORDER expr0  */
#line 1376 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3891 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 152: /* expr0: expr0 '<' expr0  */
#line 1420 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3932 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 153: /* expr0: expr0 L_LSH expr0  */
#line 1457 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = binary_int_op((yyvsp[-2].node), (yyvsp[0].node), F_LSH, "<<");
    }
#line 3940 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 154: /* expr0: expr0 L_RSH expr0  */
#line 1461 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = binary_int_op((yyvsp[-2].node), (yyvsp[0].node), F_RSH, ">>");
    }
#line 3948 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 155: /* expr0: expr0 '+' expr0  */
#line 1465 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4122 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 156: /* expr0: expr0 '-' expr0  */
#line 1635 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4220 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 157: /* expr0: expr0 '*' expr0  */
#line 1729 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4304 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 158: /* expr0: expr0 '%' expr0  */
#line 1809 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = binary_int_op((yyvsp[-2].node), (yyvsp[0].node), F_MOD, "%");
    }
#line 4312 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 159: /* expr0: expr0 '/' expr0  */
#line 1813 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4411 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 160: /* expr0: cast expr0  */
#line 1908 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4436 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 161: /* expr0: L_INC lvalue  */
#line 1929 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4461 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 162: /* expr0: L_DEC lvalue  */
#line 1950 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4487 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 163: /* expr0: L_NOT expr0  */
#line 1972 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[0].node)->kind == NODE_NUMBER) {
        (yyval.node) = (yyvsp[0].node);
        (yyval.node)->v.number = !((yyval.node)->v.number);
      } else {
        CREATE_UNARY_OP((yyval.node), F_NOT, TYPE_NUMBER, (yyvsp[0].node));
      }
    }
#line 4500 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 164: /* expr0: '~' expr0  */
#line 1981 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4515 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 165: /* expr0: '-' expr0  */
#line 1992 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4543 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 166: /* expr0: lvalue L_INC  */
#line 2016 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4569 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 167: /* expr0: lvalue L_DEC  */
#line 2038 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4594 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 174: /* return: L_RETURN ';'  */
#line 2068 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if (exact_types && !IS_TYPE(exact_types, TYPE_VOID))
        yywarn("Non-void functions must return a value.");
      CREATE_RETURN((yyval.node), 0);
    }
#line 4604 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 175: /* return: L_RETURN comma_expr ';'  */
#line 2074 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4625 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 176: /* expr_list: %empty  */
#line 2094 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      CREATE_EXPR_LIST((yyval.node), 0);
    }
#line 4633 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 177: /* expr_list: expr_list2  */
#line 2098 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_EXPR_LIST((yyval.node), (yyvsp[0].node));
    }
#line 4641 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 178: /* expr_list: expr_list2 ','  */
#line 2102 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_EXPR_LIST((yyval.node), (yyvsp[-1].node));
    }
#line 4649 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 179: /* expr_list_node: expr0  */
#line 2109 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_EXPR_NODE((yyval.node), (yyvsp[0].node), 0);
    }
#line 4657 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 180: /* expr_list_node: expr0 L_DOT_DOT_DOT  */
#line 2113 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_EXPR_NODE((yyval.node), (yyvsp[-1].node), 1);
    }
#line 4665 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 181: /* expr_list2: expr_list_node  */
#line 2120 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[0].node)->kind = 1;
      (yyval.node) = (yyvsp[0].node);
    }
#line 4674 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 182: /* expr_list2: expr_list2 ',' expr_list_node  */
#line 2125 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[0].node)->kind = 0;

      (yyval.node) = (yyvsp[-2].node);
      (yyval.node)->kind++;
      (yyval.node)->l.expr->r.expr = (yyvsp[0].node);
      (yyval.node)->l.expr = (yyvsp[0].node);
    }
#line 4687 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 183: /* expr_list3: %empty  */
#line 2137 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      /* this is a dummy node */
      CREATE_EXPR_LIST((yyval.node), 0);
    }
#line 4696 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 184: /* expr_list3: expr_list4  */
#line 2142 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_EXPR_LIST((yyval.node), (yyvsp[0].node));
    }
#line 4704 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 185: /* expr_list3: expr_list4 ','  */
#line 2146 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_EXPR_LIST((yyval.node), (yyvsp[-1].node));
    }
#line 4712 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 186: /* expr_list4: assoc_pair  */
#line 2153 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = new_node_no_line();
      (yyval.node)->kind = 2;
      (yyval.node)->v.expr = (yyvsp[0].node);
      (yyval.node)->r.expr = 0;
      (yyval.node)->type = 0;
      /* we keep track of the end of the chain in the left nodes */
      (yyval.node)->l.expr = (yyval.node);
    }
#line 4726 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 187: /* expr_list4: expr_list4 ',' assoc_pair  */
#line 2163 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4745 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 188: /* assoc_pair: expr0 ':' expr0  */
#line 2181 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_TWO_VALUES((yyval.node), 0, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 4753 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 189: /* lvalue: expr4  */
#line 2188 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4870 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 191: /* l_new_function_open: L_FUNCTION_OPEN efun_override  */
#line 2305 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = ((yyvsp[0].number) << 8) | FP_EFUN;
    }
#line 4878 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 193: /* expr4: L_DEFINED_NAME  */
#line 2313 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4925 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 194: /* expr4: L_IDENTIFIER  */
#line 2356 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      char buf[256];
      char *end = EndOf(buf);
      char *p;

      auto max_local_variables = CFG_INT(__MAX_LOCAL_VARIABLES__);
      p = strput(buf, end, "Undefined variable '");
      p = strput(p, end, (yyvsp[0].string));
      p = strput(p, end, "'");
      if (current_number_of_locals < max_local_variables) {
        add_local_name((yyvsp[0].string), TYPE_ANY);
      }
      CREATE_ERROR((yyval.node));
      yyerror(buf);
      scratch_free((yyvsp[0].string));
    }
#line 4946 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 195: /* expr4: L_PARAMETER  */
#line 2373 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_PARAMETER((yyval.node), TYPE_ANY, (yyvsp[0].number));
    }
#line 4954 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 196: /* @11: %empty  */
#line 2377 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.contextp) = current_function_context;
      /* already flagged as an error */
      if (current_function_context)
        current_function_context = current_function_context->parent;
    }
#line 4965 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 197: /* expr4: '$' '(' @11 comma_expr ')'  */
#line 2384 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4992 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 198: /* expr4: expr4 L_ARROW identifier  */
#line 2407 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5020 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 199: /* expr4: expr4 L_DOT identifier  */
#line 2431 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5048 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 200: /* expr4: expr4 '[' comma_expr L_RANGE comma_expr ']'  */
#line 2455 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5063 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 201: /* expr4: expr4 '[' '<' comma_expr L_RANGE comma_expr ']'  */
#line 2466 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = make_range_node(F_RN_RANGE, (yyvsp[-6].node), (yyvsp[-3].node), (yyvsp[-1].node));
    }
#line 5071 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 202: /* expr4: expr4 '[' '<' comma_expr L_RANGE '<' comma_expr ']'  */
#line 2470 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-1].node)->kind == NODE_NUMBER && (yyvsp[-1].node)->v.number <= 1)
        (yyval.node) = make_range_node(F_RE_RANGE, (yyvsp[-7].node), (yyvsp[-4].node), 0);
      else
        (yyval.node) = make_range_node(F_RR_RANGE, (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-1].node));
    }
#line 5082 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 203: /* expr4: expr4 '[' comma_expr L_RANGE '<' comma_expr ']'  */
#line 2477 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-1].node)->kind == NODE_NUMBER && (yyvsp[-1].node)->v.number <= 1)
        (yyval.node) = make_range_node(F_NE_RANGE, (yyvsp[-6].node), (yyvsp[-4].node), 0);
      else
        (yyval.node) = make_range_node(F_NR_RANGE, (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-1].node));
    }
#line 5093 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 204: /* expr4: expr4 '[' comma_expr L_RANGE ']'  */
#line 2484 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = make_range_node(F_NE_RANGE, (yyvsp[-4].node), (yyvsp[-2].node), 0);
    }
#line 5101 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 205: /* expr4: expr4 '[' '<' comma_expr L_RANGE ']'  */
#line 2488 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = make_range_node(F_RE_RANGE, (yyvsp[-5].node), (yyvsp[-2].node), 0);
    }
#line 5109 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 206: /* expr4: expr4 '[' '<' comma_expr ']'  */
#line 2492 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5158 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 207: /* expr4: expr4 '[' comma_expr ']'  */
#line 2537 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5218 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 209: /* expr4: '(' comma_expr ')'  */
#line 2594 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[-1].node);
    }
#line 5226 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 212: /* @12: %empty  */
#line 2600 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5251 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 213: /* expr4: L_BASIC_TYPE @12 '(' argument ')' block  */
#line 2621 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5291 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 214: /* expr4: l_new_function_open ':' ')'  */
#line 2657 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5333 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 215: /* expr4: l_new_function_open ',' expr_list2 ':' ')'  */
#line 2695 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5432 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 216: /* expr4: L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 2790 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5459 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 217: /* expr4: L_MAPPING_OPEN expr_list3 ']' ')'  */
#line 2813 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if (CONFIG_INT(__RC_WOMBLES__)) {
        if(*(outp-2) != ']') {
          yyerror("End of mapping not found");
        }
      }
      CREATE_CALL((yyval.node), F_AGGREGATE_ASSOC, TYPE_MAPPING, (yyvsp[-2].node));
    }
#line 5472 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 218: /* expr4: L_ARRAY_OPEN expr_list '}' ')'  */
#line 2822 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if (CONFIG_INT(__RC_WOMBLES__)) {
        if(*(outp-2) != '}') {
          yyerror("End of array not found");
        }
      }
      CREATE_CALL((yyval.node), F_AGGREGATE, TYPE_ANY | TYPE_MOD_ARRAY, (yyvsp[-2].node));
    }
#line 5485 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 219: /* expr_or_block: block  */
#line 2834 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[0].decl).node;
    }
#line 5493 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 220: /* expr_or_block: '(' comma_expr ')'  */
#line 2838 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = insert_pop_value((yyvsp[-1].node));
    }
#line 5501 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 221: /* @13: %empty  */
#line 2845 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      context = SPECIAL_CONTEXT;
    }
#line 5510 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 222: /* catch: L_CATCH @13 expr_or_block  */
#line 2850 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_CATCH((yyval.node), (yyvsp[0].node));
      context = (yyvsp[-1].number);
    }
#line 5519 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 223: /* tree: L_TREE block  */
#line 2858 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
#ifdef DEBUG
      (yyval.node) = new_node_no_line();
      lpc_tree_form((yyvsp[0].decl).node, (yyval.node));
#endif
    }
#line 5530 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 224: /* tree: L_TREE '(' comma_expr ')'  */
#line 2865 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
#ifdef DEBUG
      (yyval.node) = new_node_no_line();
      lpc_tree_form((yyvsp[-1].node), (yyval.node));
#endif
    }
#line 5541 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 225: /* sscanf: L_SSCANF '(' expr0 ',' expr0 lvalue_list ')'  */
#line 2875 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      int p = (yyvsp[-1].node)->v.number;
      CREATE_LVALUE_EFUN((yyval.node), TYPE_NUMBER, (yyvsp[-1].node));
      CREATE_BINARY_OP_1((yyval.node)->l.expr, F_SSCANF, 0, (yyvsp[-4].node), (yyvsp[-2].node), p);
    }
#line 5551 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 226: /* parse_command: L_PARSE_COMMAND '(' expr0 ',' expr0 ',' expr0 lvalue_list ')'  */
#line 2884 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      int p = (yyvsp[-1].node)->v.number;
      CREATE_LVALUE_EFUN((yyval.node), TYPE_NUMBER, (yyvsp[-1].node));
      CREATE_TERNARY_OP_1((yyval.node)->l.expr, F_PARSE_COMMAND, 0,
          (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-2].node), p);
    }
#line 5562 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 227: /* @14: %empty  */
#line 2894 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      context = SPECIAL_CONTEXT;
    }
#line 5571 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 228: /* time_expression: L_TIME_EXPRESSION @14 expr_or_block  */
#line 2899 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_TIME_EXPRESSION((yyval.node), (yyvsp[0].node));
      context = (yyvsp[-1].number);
    }
#line 5580 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 229: /* lvalue_list: %empty  */
#line 2907 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.node) = new_node_no_line();
      (yyval.node)->r.expr = 0;
      (yyval.node)->v.number = 0;
    }
#line 5590 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 230: /* lvalue_list: ',' lvalue lvalue_list  */
#line 2913 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      parse_node_t *insert;

      (yyval.node) = (yyvsp[0].node);
      insert = new_node_no_line();
      insert->r.expr = (yyvsp[0].node)->r.expr;
      insert->l.expr = (yyvsp[-1].node);
      (yyvsp[0].node)->r.expr = insert;
      (yyval.node)->v.number++;
    }
#line 5605 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 231: /* string: string_con2  */
#line 2927 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_STRING((yyval.node), (yyvsp[0].string));
      scratch_free((yyvsp[0].string));
    }
#line 5614 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 233: /* string_con1: '(' string_con1 ')'  */
#line 2936 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.string) = (yyvsp[-1].string);
    }
#line 5622 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 234: /* string_con1: string_con1 '+' string_con1  */
#line 2940 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.string) = scratch_join((yyvsp[-2].string), (yyvsp[0].string));
    }
#line 5630 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 236: /* string_con2: string_con2 L_STRING  */
#line 2948 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.string) = scratch_join((yyvsp[-1].string), (yyvsp[0].string));
    }
#line 5638 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 237: /* class_init: identifier ':' expr0  */
#line 2955 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = new_node();
      (yyval.node)->l.expr = (parse_node_t *)(yyvsp[-2].string);
      (yyval.node)->v.expr = (yyvsp[0].node);
      (yyval.node)->r.expr = 0;
    }
#line 5649 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 238: /* opt_class_init: %empty  */
#line 2965 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.node) = 0;
    }
#line 5657 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 239: /* opt_class_init: opt_class_init ',' class_init  */
#line 2969 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->r.expr = (yyvsp[-2].node);
    }
#line 5666 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 240: /* @15: %empty  */
#line 2977 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      (yyvsp[0].number) = num_refs;
      context |= ARG_LIST;
    }
#line 5676 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 241: /* function_call: efun_override '(' @15 expr_list ')'  */
#line 2983 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      context = (yyvsp[-2].number);
      (yyval.node) = validate_efun_call((yyvsp[-4].number),(yyvsp[-1].node));
      (yyval.node) = check_refs(num_refs - (yyvsp[-3].number), (yyvsp[-1].node), (yyval.node));
      num_refs = (yyvsp[-3].number);
    }
#line 5687 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 242: /* @16: %empty  */
#line 2990 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      (yyvsp[0].number) = num_refs;
      context |= ARG_LIST;
    }
#line 5697 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 243: /* function_call: L_NEW '(' @16 expr_list ')'  */
#line 2996 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5726 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 244: /* function_call: L_NEW '(' L_CLASS L_DEFINED_NAME opt_class_init ')'  */
#line 3021 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5765 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 245: /* function_call: L_NEW '(' L_CLASS L_IDENTIFIER opt_class_init ')'  */
#line 3056 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5787 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 246: /* @17: %empty  */
#line 3074 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      (yyvsp[0].number) = num_refs;
      context |= ARG_LIST;
    }
#line 5797 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 247: /* function_call: L_DEFINED_NAME '(' @17 expr_list ')'  */
#line 3080 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      int f;

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
#line 5855 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 248: /* @18: %empty  */
#line 3134 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      (yyvsp[0].number) = num_refs;
      context |= ARG_LIST;
    }
#line 5865 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 249: /* function_call: function_name '(' @18 expr_list ')'  */
#line 3140 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5931 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 250: /* @19: %empty  */
#line 3202 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      (yyvsp[0].number) = num_refs;
      context |= ARG_LIST;
    }
#line 5941 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 251: /* function_call: expr4 L_ARROW identifier '(' @19 expr_list ')'  */
#line 3208 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5990 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 252: /* @20: %empty  */
#line 3253 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      (yyvsp[0].number) = num_refs;
      context |= ARG_LIST;
    }
#line 6000 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 253: /* function_call: '(' '*' comma_expr ')' '(' @20 expr_list ')'  */
#line 3259 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 6026 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 254: /* efun_override: L_EFUN L_COLON_COLON identifier  */
#line 3284 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 6050 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 255: /* efun_override: L_EFUN L_COLON_COLON L_NEW  */
#line 3304 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 6067 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 257: /* function_name: L_COLON_COLON identifier  */
#line 3321 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 6085 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 258: /* function_name: L_BASIC_TYPE L_COLON_COLON identifier  */
#line 3335 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 6104 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 259: /* function_name: identifier L_COLON_COLON identifier  */
#line 3350 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 6120 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 260: /* cond: L_IF '(' comma_expr ')' statement optional_else_part  */
#line 3365 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 6155 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 261: /* optional_else_part: %empty  */
#line 3399 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = 0;
    }
#line 6163 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;

  case 262: /* optional_else_part: L_ELSE statement  */
#line 3403 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 6171 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"
    break;


#line 6175 "/home/sunyc/src/fluffos/build/src/grammar.autogen.cc"

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

#line 3407 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"

