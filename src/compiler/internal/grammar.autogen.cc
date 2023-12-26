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


#line 107 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"

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
  YYSYMBOL_optional_default_arg_value = 104, /* optional_default_arg_value  */
  YYSYMBOL_new_arg = 105,                  /* new_arg  */
  YYSYMBOL_argument = 106,                 /* argument  */
  YYSYMBOL_argument_list = 107,            /* argument_list  */
  YYSYMBOL_type_modifier_list = 108,       /* type_modifier_list  */
  YYSYMBOL_type = 109,                     /* type  */
  YYSYMBOL_cast = 110,                     /* cast  */
  YYSYMBOL_opt_basic_type = 111,           /* opt_basic_type  */
  YYSYMBOL_name_list = 112,                /* name_list  */
  YYSYMBOL_new_name = 113,                 /* new_name  */
  YYSYMBOL_block = 114,                    /* block  */
  YYSYMBOL_decl_block = 115,               /* decl_block  */
  YYSYMBOL_local_declarations = 116,       /* local_declarations  */
  YYSYMBOL_117_5 = 117,                    /* $@5  */
  YYSYMBOL_new_local_def = 118,            /* new_local_def  */
  YYSYMBOL_single_new_local_def = 119,     /* single_new_local_def  */
  YYSYMBOL_single_new_local_def_with_init = 120, /* single_new_local_def_with_init  */
  YYSYMBOL_local_name_list = 121,          /* local_name_list  */
  YYSYMBOL_statements = 122,               /* statements  */
  YYSYMBOL_statement = 123,                /* statement  */
  YYSYMBOL_while = 124,                    /* while  */
  YYSYMBOL_125_6 = 125,                    /* $@6  */
  YYSYMBOL_do = 126,                       /* do  */
  YYSYMBOL_127_7 = 127,                    /* $@7  */
  YYSYMBOL_for = 128,                      /* for  */
  YYSYMBOL_129_8 = 129,                    /* $@8  */
  YYSYMBOL_foreach_var = 130,              /* foreach_var  */
  YYSYMBOL_foreach_vars = 131,             /* foreach_vars  */
  YYSYMBOL_foreach = 132,                  /* foreach  */
  YYSYMBOL_133_9 = 133,                    /* $@9  */
  YYSYMBOL_for_expr = 134,                 /* for_expr  */
  YYSYMBOL_first_for_expr = 135,           /* first_for_expr  */
  YYSYMBOL_switch = 136,                   /* switch  */
  YYSYMBOL_137_10 = 137,                   /* $@10  */
  YYSYMBOL_switch_block = 138,             /* switch_block  */
  YYSYMBOL_case = 139,                     /* case  */
  YYSYMBOL_case_label = 140,               /* case_label  */
  YYSYMBOL_constant = 141,                 /* constant  */
  YYSYMBOL_comma_expr = 142,               /* comma_expr  */
  YYSYMBOL_ref = 143,                      /* ref  */
  YYSYMBOL_expr0 = 144,                    /* expr0  */
  YYSYMBOL_return = 145,                   /* return  */
  YYSYMBOL_expr_list = 146,                /* expr_list  */
  YYSYMBOL_expr_list_node = 147,           /* expr_list_node  */
  YYSYMBOL_expr_list2 = 148,               /* expr_list2  */
  YYSYMBOL_expr_list3 = 149,               /* expr_list3  */
  YYSYMBOL_expr_list4 = 150,               /* expr_list4  */
  YYSYMBOL_assoc_pair = 151,               /* assoc_pair  */
  YYSYMBOL_lvalue = 152,                   /* lvalue  */
  YYSYMBOL_l_new_function_open = 153,      /* l_new_function_open  */
  YYSYMBOL_expr4 = 154,                    /* expr4  */
  YYSYMBOL_155_11 = 155,                   /* @11  */
  YYSYMBOL_156_12 = 156,                   /* @12  */
  YYSYMBOL_expr_or_block = 157,            /* expr_or_block  */
  YYSYMBOL_catch = 158,                    /* catch  */
  YYSYMBOL_159_13 = 159,                   /* @13  */
  YYSYMBOL_tree = 160,                     /* tree  */
  YYSYMBOL_sscanf = 161,                   /* sscanf  */
  YYSYMBOL_parse_command = 162,            /* parse_command  */
  YYSYMBOL_time_expression = 163,          /* time_expression  */
  YYSYMBOL_164_14 = 164,                   /* @14  */
  YYSYMBOL_lvalue_list = 165,              /* lvalue_list  */
  YYSYMBOL_string = 166,                   /* string  */
  YYSYMBOL_string_con1 = 167,              /* string_con1  */
  YYSYMBOL_string_con2 = 168,              /* string_con2  */
  YYSYMBOL_class_init = 169,               /* class_init  */
  YYSYMBOL_opt_class_init = 170,           /* opt_class_init  */
  YYSYMBOL_function_call = 171,            /* function_call  */
  YYSYMBOL_172_15 = 172,                   /* @15  */
  YYSYMBOL_173_16 = 173,                   /* @16  */
  YYSYMBOL_174_17 = 174,                   /* @17  */
  YYSYMBOL_175_18 = 175,                   /* @18  */
  YYSYMBOL_176_19 = 176,                   /* @19  */
  YYSYMBOL_177_20 = 177,                   /* @20  */
  YYSYMBOL_efun_override = 178,            /* efun_override  */
  YYSYMBOL_function_name = 179,            /* function_name  */
  YYSYMBOL_cond = 180,                     /* cond  */
  YYSYMBOL_optional_else_part = 181        /* optional_else_part  */
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
#define YYLAST   1813

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  78
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  104
/* YYNRULES -- Number of rules.  */
#define YYNRULES  264
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  500

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
       0,   216,   216,   220,   221,   224,   226,   230,   234,   238,
     242,   243,   247,   254,   255,   259,   260,   264,   265,   264,
     270,   271,   277,   278,   279,   283,   300,   313,   314,   317,
     319,   319,   324,   324,   329,   330,   340,   341,   350,   358,
     359,   363,   364,   368,   369,   373,   374,   394,   400,   408,
     421,   425,   426,   444,   455,   469,   472,   489,   497,   504,
     506,   512,   513,   517,   542,   597,   606,   606,   606,   610,
     615,   614,   633,   644,   678,   689,   721,   726,   737,   740,
     746,   753,   765,   766,   767,   768,   769,   770,   775,   779,
     801,   814,   813,   827,   826,   840,   839,   864,   885,   895,
     912,   917,   928,   927,   947,   950,   954,   959,   968,   967,
    1006,  1012,  1019,  1025,  1032,  1046,  1060,  1073,  1089,  1103,
    1118,  1122,  1126,  1130,  1134,  1138,  1146,  1150,  1154,  1158,
    1162,  1166,  1170,  1174,  1178,  1182,  1186,  1190,  1194,  1201,
    1205,  1212,  1216,  1243,  1268,  1273,  1297,  1303,  1309,  1334,
    1338,  1361,  1383,  1397,  1441,  1478,  1482,  1486,  1656,  1750,
    1830,  1834,  1929,  1950,  1971,  1993,  2002,  2013,  2037,  2059,
    2080,  2081,  2082,  2083,  2084,  2085,  2089,  2095,  2116,  2119,
    2123,  2130,  2134,  2141,  2146,  2159,  2163,  2167,  2174,  2184,
    2202,  2209,  2325,  2326,  2333,  2334,  2377,  2394,  2399,  2398,
    2428,  2452,  2476,  2487,  2491,  2498,  2505,  2509,  2513,  2558,
    2614,  2615,  2619,  2620,  2622,  2621,  2678,  2716,  2811,  2834,
    2843,  2855,  2859,  2867,  2866,  2879,  2886,  2896,  2905,  2916,
    2915,  2929,  2934,  2948,  2956,  2957,  2961,  2968,  2969,  2976,
    2987,  2990,  2999,  2998,  3012,  3011,  3042,  3077,  3096,  3095,
    3156,  3155,  3224,  3223,  3275,  3274,  3305,  3325,  3341,  3342,
    3356,  3371,  3386,  3420,  3424
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
  "arg_type", "optional_default_arg_value", "new_arg", "argument",
  "argument_list", "type_modifier_list", "type", "cast", "opt_basic_type",
  "name_list", "new_name", "block", "decl_block", "local_declarations",
  "$@5", "new_local_def", "single_new_local_def",
  "single_new_local_def_with_init", "local_name_list", "statements",
  "statement", "while", "$@6", "do", "$@7", "for", "$@8", "foreach_var",
  "foreach_vars", "foreach", "$@9", "for_expr", "first_for_expr", "switch",
  "$@10", "switch_block", "case", "case_label", "constant", "comma_expr",
  "ref", "expr0", "return", "expr_list", "expr_list_node", "expr_list2",
  "expr_list3", "expr_list4", "assoc_pair", "lvalue",
  "l_new_function_open", "expr4", "@11", "@12", "expr_or_block", "catch",
  "@13", "tree", "sscanf", "parse_command", "time_expression", "@14",
  "lvalue_list", "string", "string_con1", "string_con2", "class_init",
  "opt_class_init", "function_call", "@15", "@16", "@17", "@18", "@19",
  "@20", "efun_override", "function_name", "cond", "optional_else_part", YY_NULLPTR
  };
  return yy_sname[yysymbol];
}
#endif

#define YYPACT_NINF (-403)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-259)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -403,    23,    64,  -403,    63,  -403,  -403,     5,  -403,  -403,
     112,    98,  -403,  -403,  -403,  -403,     8,    41,  -403,    39,
     126,  -403,  -403,  -403,   334,   120,   105,  -403,     8,   -14,
     212,   161,   183,   191,  -403,  -403,  -403,     3,  -403,    98,
     -18,     8,  -403,  -403,  -403,  1652,   242,   334,  -403,  -403,
    -403,  -403,   307,  -403,  -403,   288,    -3,    30,   292,   166,
     166,  1652,   334,  1139,   470,  1652,  -403,   265,  -403,  -403,
     275,  -403,   289,  -403,   151,  1652,  1652,   918,   297,  -403,
    -403,   335,  1652,   166,  1326,   247,   176,   239,  -403,  -403,
    -403,  -403,  -403,  -403,   212,  -403,   310,   312,    89,   373,
      12,  1652,   334,   320,  -403,    38,  1214,  -403,   186,  -403,
    -403,  -403,  1253,   301,  -403,   325,   884,   336,   330,  -403,
     266,  1326,   310,  1652,   159,  1652,   159,   368,  1652,  -403,
    -403,  -403,  -403,    85,   360,  1652,    98,    61,  -403,   334,
    -403,  -403,  1652,  1652,  1652,  1652,  1652,  1652,  1652,  1652,
    1652,  1652,  1652,  1652,  1652,  1652,  1652,  1652,  1652,  -403,
    -403,  1652,   348,  1652,   334,   334,  1287,  -403,  -403,  -403,
    -403,  -403,   374,    98,  -403,   351,     4,  -403,  -403,  1326,
    -403,    89,  1360,  -403,  -403,  -403,   352,  1064,  1652,   354,
     545,   355,  1652,   435,  1652,  -403,  -403,   510,  -403,   362,
    1360,   108,   620,  -403,  -403,   130,   356,  -403,  1652,  -403,
    1470,  1399,   214,   214,   329,   957,  1616,  1689,  1708,  1476,
    1476,   329,   217,   217,  -403,  -403,  -403,  1326,  -403,   306,
     359,  -403,  1652,    55,  1360,  1360,  -403,   375,  -403,  -403,
     135,    98,   370,   378,  -403,  -403,  1326,  -403,  -403,  -403,
    1326,  1652,   142,  1652,  -403,  -403,   384,  -403,     7,   363,
     386,   388,  -403,   390,   391,   361,   393,  1433,  -403,  -403,
    -403,  -403,   389,   770,  -403,  -403,  -403,  -403,  -403,   170,
    -403,  -403,   396,  -403,   167,  1652,   397,  1652,  -403,   109,
     313,  -403,   398,   399,   406,     6,  -403,   334,   394,   415,
     411,  -403,  1104,  -403,  1179,   195,   201,  -403,  -403,  1652,
    1652,  1652,   845,   991,   181,  -403,  -403,  -403,   172,    98,
    -403,  -403,  -403,  -403,  -403,  1326,  -403,  1360,   395,  -403,
    1652,  -403,    65,  -403,  -403,   444,  -403,  -403,  -403,  -403,
    -403,  -403,    98,  -403,  -403,   166,   416,  1652,  -403,   334,
    -403,   220,   234,   257,   460,    98,   475,  -403,  -403,   434,
     431,  -403,  -403,  -403,   432,   474,  -403,   375,   443,   438,
    1360,   449,  1652,  -403,    75,    93,  -403,  1652,  -403,   451,
    -403,  1104,   458,  -403,   845,  -403,  -403,   461,   375,  1652,
    1506,   181,  1652,   521,    98,  -403,   465,  -403,    96,  -403,
    -403,   344,  -403,   466,  1652,   517,   467,   845,  1652,  -403,
    1326,   473,  -403,  1030,  1652,  -403,  -403,  -403,   472,  -403,
    1326,   845,  -403,  -403,  -403,   263,  1579,  -403,  1326,  -403,
    -403,    33,   476,   482,   845,   160,   487,   695,  -403,  -403,
    -403,  -403,   539,   194,   555,   556,   194,    20,  1543,   499,
    -403,   695,   488,   695,   845,  -403,   492,  -403,  -403,   237,
     132,  -403,    59,    59,    59,    59,    59,    59,    59,    59,
      59,    59,    59,    59,    59,    59,  -403,  -403,  -403,  -403,
    -403,  -403,  -403,   519,    59,   346,   346,   515,  1671,  1700,
    1722,  1549,  1549,   515,   244,   244,  -403,  -403,  -403,  -403
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       4,     0,    55,     1,    55,    22,    20,     5,    24,    23,
      60,    10,    56,     6,     3,    36,     0,     0,    25,    41,
       0,    59,    57,    11,     0,     0,    61,   237,     0,     0,
     234,    37,    38,     0,    42,    15,    16,    63,    21,    10,
       0,     0,     7,   238,    32,     0,     0,     0,    62,   235,
     236,    29,     0,     9,     8,   214,   195,   196,     0,     0,
       0,     0,     0,     0,     0,     0,   192,     0,   223,   141,
       0,   229,     0,   197,     0,     0,     0,     0,     0,   175,
     174,     0,     0,     0,    64,     0,     0,   170,   212,   213,
     171,   172,   173,   210,   233,   194,     0,     0,    40,    63,
      40,     0,     0,     0,   248,     0,     0,   163,   191,   164,
     165,   259,   181,     0,   183,   179,     0,     0,   186,   188,
       0,   139,   193,     0,     0,     0,     0,   244,     0,    69,
     225,   167,   166,    36,     0,     0,    10,     0,   198,     0,
     162,   142,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   168,
     169,     0,     0,     0,     0,     0,     0,   242,   250,    35,
      34,    49,    43,    10,    53,     0,    51,    33,    30,   144,
     260,    40,     0,   257,   256,   182,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   221,   224,     0,   230,     0,
       0,     0,     0,    37,    38,     0,     0,   211,     0,   261,
     147,   146,   155,   156,   153,     0,   148,   149,   150,   151,
     152,   154,   157,   158,   159,   160,   161,   143,   216,     0,
     200,   201,     0,     0,     0,     0,    44,    47,    18,    52,
      40,    10,     0,     0,   220,   184,   190,   219,   189,   218,
     140,     0,     0,     0,   240,   240,     0,   226,     0,     0,
       0,     0,    93,     0,     0,     0,     0,     0,    88,    70,
      66,    87,     0,     0,    83,    84,    67,    68,    85,     0,
      86,    82,     0,    58,     0,     0,     0,     0,   252,     0,
       0,   209,     0,     0,    45,     0,    54,     0,    27,     0,
       0,   249,   231,   222,     0,     0,     0,   245,    80,     0,
       0,     0,     0,     0,    40,    89,    90,   176,     0,    10,
      65,    79,    81,   254,   199,   145,   217,     0,     0,   208,
       0,   206,     0,   243,   251,     0,    48,    14,    13,    19,
      12,    26,    10,    31,   215,     0,     0,     0,   246,     0,
     247,     0,     0,     0,     0,    10,     0,   107,   106,     0,
     105,    97,    99,    98,   100,     0,   177,     0,    76,     0,
       0,     0,     0,   207,     0,     0,   202,     0,    28,   231,
     227,   231,     0,   241,     0,   108,    91,     0,     0,     0,
       0,    40,     0,    72,    10,    71,     0,   253,     0,   203,
     205,     0,   232,     0,     0,   263,     0,     0,     0,    74,
      75,     0,   101,     0,     0,    77,   255,   204,     0,   228,
     239,     0,   262,    69,    92,     0,     0,   102,    73,    46,
     264,    40,     0,     0,     0,     0,     0,     0,    94,    95,
     103,   135,     0,     0,     0,     0,     0,     0,   118,   119,
     117,     0,     0,     0,     0,   137,     0,   136,   138,     0,
       0,   113,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   111,   109,   110,    96,
     116,   134,   115,     0,     0,   127,   128,   125,   120,   121,
     122,   123,   124,   126,   129,   130,   131,   132,   133,   114
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -403,  -403,  -403,  -403,  -403,  -403,  -403,    -7,  -403,    -9,
    -403,  -403,  -403,  -403,  -403,  -403,   252,  -403,  -403,  -403,
    -403,  -228,  -403,  -403,    -8,  -271,  -403,   358,   418,  -403,
     596,  -403,  -403,  -403,   562,  -403,   -64,  -403,   179,  -403,
    -403,   290,  -403,   210,   332,  -188,  -403,  -403,  -403,  -403,
    -403,  -403,   215,  -403,  -403,  -403,  -359,  -403,  -403,  -403,
    -167,   178,  -402,  1329,   -41,   439,   -42,  -403,  -145,  -170,
     447,  -403,  -403,   417,   -54,  -403,   -32,  -403,  -403,   489,
    -403,  -403,  -403,  -403,  -403,  -403,  -403,   -40,  -403,   -15,
     -16,  -403,   364,  -403,  -403,  -403,  -403,  -403,  -403,  -403,
     548,  -403,  -403,  -403
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,     2,    14,     5,    79,    80,   297,   339,    81,
       6,    46,   295,     7,     8,   298,   299,   100,   241,     9,
      51,   171,    19,    20,   172,   173,   336,   174,   175,   176,
      10,    11,    82,    22,    25,    26,   270,   271,   202,   319,
     368,   363,   357,   369,   272,   451,   274,   407,   275,   312,
     276,   454,   364,   365,   277,   434,   358,   359,   278,   406,
     452,   453,   447,   448,   279,    83,   121,   280,   113,   114,
     115,   117,   118,   119,    85,    86,    87,   208,   103,   196,
      88,   124,    89,    90,    91,    92,   126,   346,    93,   449,
      94,   383,   305,    95,   234,   200,   182,   235,   327,   370,
      96,    97,   281,   422
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      30,    29,    21,    84,    24,   107,   109,   337,    33,   294,
     130,    27,    30,    40,   273,    37,    45,   245,    15,   110,
     101,   112,   116,     3,   120,    30,    50,   108,   108,   141,
     239,   411,    47,   131,   132,   -15,   137,   243,    99,    15,
     140,   456,   355,   355,    41,   460,    35,    36,    41,    31,
      32,   108,    49,   111,    42,   256,   435,   436,   483,   179,
     195,   134,   195,   441,    -2,   137,   104,   433,   -16,   136,
       4,     4,   -17,    13,   338,   308,   240,    28,   442,   129,
     290,   193,   134,   197,   -39,   273,   177,   201,   183,   292,
     293,   461,   178,   180,   205,    15,   184,   169,   170,  -258,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   245,    15,   227,
     355,   112,   444,   102,   354,   233,   445,   192,   484,   206,
     209,   207,   291,   192,   328,    27,   441,   192,   134,   393,
     112,    15,   376,   169,   170,   112,   246,   192,   116,    16,
     250,   442,   399,   252,  -214,   230,   231,   -40,   112,   -50,
     409,    17,    23,    27,   441,   192,   237,   284,   192,    27,
     400,    34,    55,   417,    56,    57,    58,    39,   257,   442,
     192,   192,   371,    18,   134,   443,   329,    15,    38,   361,
     362,   289,   112,   112,   269,   444,   405,    27,   441,   445,
     282,   446,   192,   482,    62,    63,    64,    65,    66,   302,
      68,   304,   303,   442,   192,    43,    72,    73,    74,   424,
     128,   164,   165,   444,   129,   396,   318,   445,   194,   446,
     134,   340,   129,   430,   -15,   106,   344,   324,   322,   192,
     366,    78,   192,   325,   192,   112,   440,   162,   163,   332,
    -191,  -191,  -191,   462,   463,   464,   -16,   444,   159,   160,
     161,   445,   166,   446,    44,   348,   479,   349,   351,   352,
     353,   350,   360,   349,   164,   165,   154,   155,   156,   157,
     158,   156,   157,   158,   476,   112,   478,   374,   341,   375,
     384,   379,   192,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   385,   381,   192,   481,   473,   474,
     475,    98,   367,   108,    52,   166,    27,    53,    54,    55,
     101,    56,    57,    58,    59,    60,   102,   386,   112,   192,
     105,   398,    61,   432,   123,   192,   401,   191,   192,   402,
     382,   403,    35,    36,   125,   144,   145,   410,   388,   360,
     413,    62,    63,    64,    65,    66,    67,    68,   127,    69,
      70,    71,   420,    72,    73,    74,   138,   425,   203,   204,
     254,   255,   428,   139,   330,   186,    75,   286,   287,   167,
      76,   168,    77,   169,   170,   360,    45,   367,    78,   181,
     331,   154,   155,   156,   157,   158,    52,   187,    27,    53,
      54,    55,   190,    56,    57,    58,    59,    60,   471,   472,
     473,   474,   475,   189,    61,   418,   192,   199,   228,    30,
      69,   238,   244,   269,   247,   249,   283,    30,   288,   315,
      30,    40,   309,    62,    63,    64,    65,    66,    67,    68,
     300,    69,    70,    71,    30,    72,    73,    74,   301,   142,
     143,   144,   145,   146,   307,   310,   372,   311,    75,   313,
     314,   316,    76,   320,    77,   323,   342,   326,   333,   334,
      78,    52,   373,    27,    53,    54,    55,   335,    56,    57,
      58,    59,    60,   343,   129,   377,   380,   387,   389,    61,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   390,   192,   391,   392,   395,   251,    62,    63,
      64,    65,    66,    67,    68,   394,    69,    70,    71,   397,
      72,    73,    74,   345,   142,   143,   144,   145,   146,   404,
     408,   462,   463,    75,   414,   416,   419,    76,   421,    77,
     423,   426,   429,   455,   438,    78,    52,  -185,    27,    53,
      54,    55,   439,    56,    57,    58,    59,    60,   450,   457,
     458,    41,   477,   480,    61,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   471,   472,   473,
     474,   475,   253,    62,    63,    64,    65,    66,    67,    68,
     499,    69,    70,    71,   378,    72,    73,    74,   296,   242,
      12,    48,   431,   356,   415,   321,   412,   248,    75,   437,
     229,   236,    76,   122,    77,   198,     0,     0,     0,   306,
      78,   258,  -187,    27,    53,    54,   133,     0,    56,    57,
      58,    59,    60,     0,     0,     0,     0,     0,     0,    61,
     259,     0,   260,     0,     0,     0,     0,   261,   262,   263,
     264,     0,   265,   266,   267,     0,     0,     0,    62,    63,
      64,    65,    66,    67,    68,   -40,    69,    70,    71,   134,
      72,    73,    74,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,     0,     0,     0,    76,   268,    77,
       0,     0,     0,   129,   -78,    78,    52,     0,    27,    53,
      54,    55,     0,    56,    57,    58,    59,    60,     0,     0,
       0,     0,     0,     0,    61,   259,     0,   260,   435,   436,
       0,     0,   261,   262,   263,   264,     0,   265,   266,   267,
       0,     0,     0,    62,    63,    64,    65,    66,    67,    68,
       0,    69,    70,    71,     0,    72,    73,    74,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,    76,   268,    77,     0,     0,     0,   129,  -112,
      78,   258,     0,    27,    53,    54,    55,     0,    56,    57,
      58,    59,    60,     0,     0,     0,     0,     0,     0,    61,
     259,     0,   260,     0,     0,     0,     0,   261,   262,   263,
     264,     0,   265,   266,   267,     0,     0,     0,    62,    63,
      64,    65,    66,    67,    68,     0,    69,    70,    71,     0,
      72,    73,    74,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,     0,     0,     0,    76,   268,    77,
       0,     0,     0,   129,   -78,    78,    52,     0,    27,    53,
      54,    55,     0,    56,    57,    58,    59,    60,     0,     0,
       0,     0,     0,     0,    61,   259,     0,   260,     0,     0,
       0,     0,   261,   262,   263,   264,     0,   265,   266,   267,
       0,     0,     0,    62,    63,    64,    65,    66,    67,    68,
       0,    69,    70,    71,     0,    72,    73,    74,   142,   143,
     144,   145,   146,     0,     0,     0,     0,     0,    75,     0,
       0,     0,    76,   268,    77,     0,     0,     0,   129,    52,
      78,    27,    53,    54,   133,     0,    56,    57,    58,    59,
      60,     0,     0,     0,     0,     0,     0,    61,     0,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,     0,     0,     0,     0,   188,    62,    63,    64,    65,
      66,    67,    68,   -40,    69,    70,    71,   134,    72,    73,
      74,   142,   143,   144,   145,   146,     0,     0,     0,     0,
       0,    75,   135,     0,     0,    76,     0,    77,     0,     0,
       0,     0,    52,    78,    27,    53,    54,   133,     0,    56,
      57,    58,    59,    60,     0,     0,     0,     0,     0,     0,
      61,     0,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,     0,     0,     0,     0,   285,    62,
      63,    64,    65,    66,    67,    68,   -40,    69,    70,    71,
     134,    72,    73,    74,   142,   143,   144,   145,   146,     0,
       0,     0,     0,     0,    75,     0,     0,     0,    76,  -104,
      77,     0,     0,     0,     0,    52,    78,    27,    53,    54,
      55,     0,    56,    57,    58,    59,    60,     0,     0,     0,
       0,     0,     0,    61,     0,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,     0,     0,     0,
     427,     0,    62,    63,    64,    65,    66,    67,    68,     0,
      69,    70,    71,     0,    72,    73,    74,     0,   142,   143,
     144,   145,   146,     0,     0,     0,     0,    75,     0,     0,
       0,    76,     0,    77,  -180,     0,     0,     0,  -180,    78,
      52,     0,    27,    53,    54,    55,     0,    56,    57,    58,
      59,    60,     0,     0,     0,     0,     0,     0,    61,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,     0,     0,     0,     0,     0,   345,    62,    63,    64,
      65,    66,    67,    68,     0,    69,    70,    71,     0,    72,
      73,    74,     0,   142,   143,   144,   145,   146,     0,     0,
       0,     0,    75,     0,     0,     0,    76,     0,    77,     0,
       0,     0,     0,  -178,    78,    52,     0,    27,    53,    54,
      55,     0,    56,    57,    58,    59,    60,     0,     0,     0,
       0,     0,     0,    61,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,     0,     0,     0,     0,
       0,   347,    62,    63,    64,    65,    66,    67,    68,     0,
      69,    70,    71,     0,    72,    73,    74,   142,   143,   144,
     145,   146,     0,     0,     0,     0,     0,    75,   135,   185,
       0,    76,     0,    77,     0,     0,     0,     0,    52,    78,
      27,    53,    54,    55,     0,    56,    57,    58,    59,    60,
       0,     0,     0,     0,     0,     0,    61,     0,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
       0,     0,     0,     0,     0,    62,    63,    64,    65,    66,
      67,    68,     0,    69,    70,    71,     0,    72,    73,    74,
     142,   143,   144,   145,   146,     0,     0,     0,   232,     0,
      75,     0,     0,     0,    76,     0,    77,     0,     0,     0,
       0,    52,    78,    27,    53,    54,    55,     0,    56,    57,
      58,    59,    60,     0,     0,     0,     0,     0,     0,    61,
       0,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,     0,     0,     0,     0,     0,    62,    63,
      64,    65,    66,    67,    68,     0,    69,    70,    71,     0,
      72,    73,    74,   142,     0,   144,   145,   146,     0,     0,
       0,     0,     0,    75,     0,     0,     0,    76,     0,    77,
    -178,     0,     0,     0,    52,    78,    27,    53,    54,    55,
       0,    56,    57,    58,    59,    60,     0,     0,     0,     0,
       0,     0,    61,     0,     0,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,     0,     0,     0,     0,
       0,    62,    63,    64,    65,    66,    67,    68,     0,    69,
      70,    71,     0,    72,    73,    74,   144,   145,   146,     0,
       0,     0,   144,   145,   146,     0,    75,     0,     0,     0,
      76,   317,    77,     0,     0,     0,     0,    52,    78,    27,
      53,    54,    55,     0,    56,    57,    58,    59,    60,     0,
       0,     0,     0,     0,     0,    61,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   153,   154,   155,
     156,   157,   158,     0,    62,    63,    64,    65,    66,    67,
      68,     0,    69,    70,    71,     0,    72,    73,    74,   462,
     463,   464,     0,     0,     0,   462,   463,   464,     0,    75,
       0,     0,     0,    76,  -104,    77,     0,     0,     0,     0,
      52,    78,    27,    53,    54,    55,     0,    56,    57,    58,
      59,    60,     0,     0,     0,     0,     0,     0,    61,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     470,   471,   472,   473,   474,   475,     0,    62,    63,    64,
      65,    66,    67,    68,     0,    69,    70,    71,     0,    72,
      73,    74,   144,   145,   146,     0,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,    76,     0,    77,  -104,
       0,     0,     0,    52,    78,    27,    53,    54,    55,     0,
      56,    57,    58,    59,    60,     0,     0,     0,     0,     0,
       0,    61,     0,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,     0,     0,     0,     0,   462,   463,   464,
      62,    63,    64,    65,    66,    67,    68,     0,    69,    70,
      71,     0,    72,    73,    74,   144,   145,   146,     0,     0,
       0,     0,     0,     0,     0,    75,   462,   463,   464,    76,
       0,    77,     0,     0,   144,   145,   146,    78,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   462,   463,
     464,     0,     0,     0,     0,     0,     0,   150,   151,   152,
     153,   154,   155,   156,   157,   158,     0,     0,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   151,   152,   153,
     154,   155,   156,   157,   158,   459,     0,     0,     0,     0,
       0,   468,   469,   470,   471,   472,   473,   474,   475,     0,
       0,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,     0,     0,     0,     0,     0,
       0,     0,     0,   459
};

static const yytype_int16 yycheck[] =
{
      16,    16,    10,    45,    11,    59,    60,     1,    17,   237,
      74,     3,    28,    28,   202,    24,    13,   187,     6,    61,
      13,    63,    64,     0,    65,    41,    41,    59,    60,    83,
      26,   390,    39,    75,    76,    38,    77,   182,    47,     6,
      82,   443,   313,   314,    62,    25,     8,     9,    62,     8,
       9,    83,    70,    62,    68,   200,    23,    24,   460,   101,
     124,    49,   126,     4,     0,   106,    69,   426,    38,    77,
       7,     7,    69,    68,    68,    68,    72,    69,    19,    73,
      25,   123,    49,   125,    45,   273,    74,   128,    50,   234,
     235,    71,   100,   102,   135,     6,   105,     8,     9,    69,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   287,     6,   161,
     391,   163,    63,    38,   312,   166,    67,    72,    69,   136,
     139,    70,    77,    72,    25,     3,     4,    72,    49,   367,
     182,     6,    77,     8,     9,   187,   188,    72,   190,    37,
     192,    19,    77,   194,    69,   164,   165,    45,   200,    70,
     388,    49,    64,     3,     4,    72,   173,   208,    72,     3,
      77,    45,     6,    77,     8,     9,    10,    72,    70,    19,
      72,    72,   327,    71,    49,    25,    77,     6,    68,     8,
       9,   232,   234,   235,   202,    63,   384,     3,     4,    67,
      70,    69,    72,    71,    38,    39,    40,    41,    42,   251,
      44,   253,    70,    19,    72,     3,    50,    51,    52,   407,
      69,    35,    36,    63,    73,   370,   267,    67,    69,    69,
      49,   295,    73,   421,    73,    69,   300,    70,    68,    72,
      68,    75,    72,   285,    72,   287,   434,    71,    72,   290,
      11,    12,    13,    16,    17,    18,    73,    63,    11,    12,
      13,    67,    76,    69,    73,    70,   454,    72,   309,   310,
     311,    70,   313,    72,    35,    36,    62,    63,    64,    65,
      66,    64,    65,    66,   451,   327,   453,   328,   297,   330,
      70,   345,    72,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    70,   347,    72,    70,    64,    65,
      66,    69,   319,   345,     1,    76,     3,     4,     5,     6,
      13,     8,     9,    10,    11,    12,    38,    70,   370,    72,
      38,   372,    19,    70,    69,    72,   377,    71,    72,   379,
     349,   381,     8,     9,    69,    16,    17,   389,   355,   390,
     392,    38,    39,    40,    41,    42,    43,    44,    69,    46,
      47,    48,   404,    50,    51,    52,    69,   408,     8,     9,
       8,     9,   414,    38,    61,    74,    63,    71,    72,    69,
      67,    69,    69,     8,     9,   426,    13,   394,    75,    69,
      77,    62,    63,    64,    65,    66,     1,    72,     3,     4,
       5,     6,    72,     8,     9,    10,    11,    12,    62,    63,
      64,    65,    66,    77,    19,    71,    72,    49,    70,   435,
      46,    70,    70,   431,    70,    70,    70,   443,    69,    68,
     446,   446,    69,    38,    39,    40,    41,    42,    43,    44,
      70,    46,    47,    48,   460,    50,    51,    52,    70,    14,
      15,    16,    17,    18,    70,    69,    61,    69,    63,    69,
      69,    68,    67,    74,    69,    69,    72,    70,    70,    70,
      75,     1,    77,     3,     4,     5,     6,    71,     8,     9,
      10,    11,    12,    68,    73,    41,    70,    27,    13,    19,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    68,    72,    72,    31,    68,    72,    38,    39,
      40,    41,    42,    43,    44,    72,    46,    47,    48,    70,
      50,    51,    52,    72,    14,    15,    16,    17,    18,    71,
      69,    16,    17,    63,    13,    70,    70,    67,    21,    69,
      73,    68,    70,     4,    68,    75,     1,    77,     3,     4,
       5,     6,    70,     8,     9,    10,    11,    12,    71,     4,
       4,    62,    74,    71,    19,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    62,    63,    64,
      65,    66,    72,    38,    39,    40,    41,    42,    43,    44,
      71,    46,    47,    48,   342,    50,    51,    52,   240,   181,
       4,    39,   423,   313,   394,   273,   391,   190,    63,   431,
     163,   172,    67,    65,    69,   126,    -1,    -1,    -1,   255,
      75,     1,    77,     3,     4,     5,     6,    -1,     8,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    19,
      20,    -1,    22,    -1,    -1,    -1,    -1,    27,    28,    29,
      30,    -1,    32,    33,    34,    -1,    -1,    -1,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,    69,
      -1,    -1,    -1,    73,    74,    75,     1,    -1,     3,     4,
       5,     6,    -1,     8,     9,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    19,    20,    -1,    22,    23,    24,
      -1,    -1,    27,    28,    29,    30,    -1,    32,    33,    34,
      -1,    -1,    -1,    38,    39,    40,    41,    42,    43,    44,
      -1,    46,    47,    48,    -1,    50,    51,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,
      -1,    -1,    67,    68,    69,    -1,    -1,    -1,    73,    74,
      75,     1,    -1,     3,     4,     5,     6,    -1,     8,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    19,
      20,    -1,    22,    -1,    -1,    -1,    -1,    27,    28,    29,
      30,    -1,    32,    33,    34,    -1,    -1,    -1,    38,    39,
      40,    41,    42,    43,    44,    -1,    46,    47,    48,    -1,
      50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,    69,
      -1,    -1,    -1,    73,    74,    75,     1,    -1,     3,     4,
       5,     6,    -1,     8,     9,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    19,    20,    -1,    22,    -1,    -1,
      -1,    -1,    27,    28,    29,    30,    -1,    32,    33,    34,
      -1,    -1,    -1,    38,    39,    40,    41,    42,    43,    44,
      -1,    46,    47,    48,    -1,    50,    51,    52,    14,    15,
      16,    17,    18,    -1,    -1,    -1,    -1,    -1,    63,    -1,
      -1,    -1,    67,    68,    69,    -1,    -1,    -1,    73,     1,
      75,     3,     4,     5,     6,    -1,     8,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    19,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    -1,    -1,    -1,    71,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    14,    15,    16,    17,    18,    -1,    -1,    -1,    -1,
      -1,    63,    64,    -1,    -1,    67,    -1,    69,    -1,    -1,
      -1,    -1,     1,    75,     3,     4,     5,     6,    -1,     8,
       9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      19,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    -1,    -1,    -1,    71,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    14,    15,    16,    17,    18,    -1,
      -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,
      69,    -1,    -1,    -1,    -1,     1,    75,     3,     4,     5,
       6,    -1,     8,     9,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    19,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    -1,    38,    39,    40,    41,    42,    43,    44,    -1,
      46,    47,    48,    -1,    50,    51,    52,    -1,    14,    15,
      16,    17,    18,    -1,    -1,    -1,    -1,    63,    -1,    -1,
      -1,    67,    -1,    69,    70,    -1,    -1,    -1,    74,    75,
       1,    -1,     3,     4,     5,     6,    -1,     8,     9,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    19,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    -1,    -1,    -1,    -1,    72,    38,    39,    40,
      41,    42,    43,    44,    -1,    46,    47,    48,    -1,    50,
      51,    52,    -1,    14,    15,    16,    17,    18,    -1,    -1,
      -1,    -1,    63,    -1,    -1,    -1,    67,    -1,    69,    -1,
      -1,    -1,    -1,    74,    75,     1,    -1,     3,     4,     5,
       6,    -1,     8,     9,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    19,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    -1,    -1,
      -1,    72,    38,    39,    40,    41,    42,    43,    44,    -1,
      46,    47,    48,    -1,    50,    51,    52,    14,    15,    16,
      17,    18,    -1,    -1,    -1,    -1,    -1,    63,    64,    26,
      -1,    67,    -1,    69,    -1,    -1,    -1,    -1,     1,    75,
       3,     4,     5,     6,    -1,     8,     9,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    19,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    -1,    -1,    -1,    -1,    38,    39,    40,    41,    42,
      43,    44,    -1,    46,    47,    48,    -1,    50,    51,    52,
      14,    15,    16,    17,    18,    -1,    -1,    -1,    61,    -1,
      63,    -1,    -1,    -1,    67,    -1,    69,    -1,    -1,    -1,
      -1,     1,    75,     3,     4,     5,     6,    -1,     8,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    19,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    -1,    -1,    -1,    -1,    38,    39,
      40,    41,    42,    43,    44,    -1,    46,    47,    48,    -1,
      50,    51,    52,    14,    -1,    16,    17,    18,    -1,    -1,
      -1,    -1,    -1,    63,    -1,    -1,    -1,    67,    -1,    69,
      70,    -1,    -1,    -1,     1,    75,     3,     4,     5,     6,
      -1,     8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    19,    -1,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    -1,    -1,
      -1,    38,    39,    40,    41,    42,    43,    44,    -1,    46,
      47,    48,    -1,    50,    51,    52,    16,    17,    18,    -1,
      -1,    -1,    16,    17,    18,    -1,    63,    -1,    -1,    -1,
      67,    68,    69,    -1,    -1,    -1,    -1,     1,    75,     3,
       4,     5,     6,    -1,     8,     9,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    19,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    61,    62,    63,
      64,    65,    66,    -1,    38,    39,    40,    41,    42,    43,
      44,    -1,    46,    47,    48,    -1,    50,    51,    52,    16,
      17,    18,    -1,    -1,    -1,    16,    17,    18,    -1,    63,
      -1,    -1,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,
       1,    75,     3,     4,     5,     6,    -1,     8,     9,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    19,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      61,    62,    63,    64,    65,    66,    -1,    38,    39,    40,
      41,    42,    43,    44,    -1,    46,    47,    48,    -1,    50,
      51,    52,    16,    17,    18,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    63,    -1,    -1,    -1,    67,    -1,    69,    70,
      -1,    -1,    -1,     1,    75,     3,     4,     5,     6,    -1,
       8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    -1,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    -1,    -1,    -1,    16,    17,    18,
      38,    39,    40,    41,    42,    43,    44,    -1,    46,    47,
      48,    -1,    50,    51,    52,    16,    17,    18,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    63,    16,    17,    18,    67,
      -1,    69,    -1,    -1,    16,    17,    18,    75,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    16,    17,
      18,    -1,    -1,    -1,    -1,    -1,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    59,    60,    61,
      62,    63,    64,    65,    66,   446,    -1,    -1,    -1,    -1,
      -1,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      -1,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   484
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    79,    80,     0,     7,    82,    88,    91,    92,    97,
     108,   109,   108,    68,    81,     6,    37,    49,    71,   100,
     101,   102,   111,    64,    85,   112,   113,     3,    69,   167,
     168,     8,     9,    87,    45,     8,     9,    87,    68,    72,
     167,    62,    68,     3,    73,    13,    89,    85,   112,    70,
     167,    98,     1,     4,     5,     6,     8,     9,    10,    11,
      12,    19,    38,    39,    40,    41,    42,    43,    44,    46,
      47,    48,    50,    51,    52,    63,    67,    69,    75,    83,
      84,    87,   110,   143,   144,   152,   153,   154,   158,   160,
     161,   162,   163,   166,   168,   171,   178,   179,    69,    87,
      95,    13,    38,   156,    69,    38,    69,   152,   154,   152,
     144,    87,   144,   146,   147,   148,   144,   149,   150,   151,
     142,   144,   178,    69,   159,    69,   164,    69,    69,    73,
     114,   144,   144,     6,    49,    64,   102,   142,    69,    38,
     144,   152,    14,    15,    16,    17,    18,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    11,
      12,    13,    71,    72,    35,    36,    76,    69,    69,     8,
       9,    99,   102,   103,   105,   106,   107,    74,   102,   144,
      87,    69,   174,    50,    87,    26,    74,    72,    71,    77,
      72,    71,    72,   144,    69,   114,   157,   144,   157,    49,
     173,   142,   116,     8,     9,   142,    85,    70,   155,    87,
     144,   144,   144,   144,   144,   144,   144,   144,   144,   144,
     144,   144,   144,   144,   144,   144,   144,   144,    70,   148,
      87,    87,    61,   142,   172,   175,   143,    85,    70,    26,
      72,    96,   106,   146,    70,   147,   144,    70,   151,    70,
     144,    72,   142,    72,     8,     9,   146,    70,     1,    20,
      22,    27,    28,    29,    30,    32,    33,    34,    68,   102,
     114,   115,   122,   123,   124,   126,   128,   132,   136,   142,
     145,   180,    70,    70,   142,    71,    71,    72,    69,   142,
      25,    77,   146,   146,    99,    90,   105,    85,    93,    94,
      70,    70,   144,    70,   144,   170,   170,    70,    68,    69,
      69,    69,   127,    69,    69,    68,    68,    68,   142,   117,
      74,   122,    68,    69,    70,   144,    70,   176,    25,    77,
      61,    77,   142,    70,    70,    71,   104,     1,    68,    86,
     114,    87,    72,    68,   114,    72,   165,    72,    70,    72,
      70,   142,   142,   142,   123,   103,   119,   120,   134,   135,
     142,     8,     9,   119,   130,   131,    68,    85,   118,   121,
     177,   146,    61,    77,   142,   142,    77,    41,    94,   152,
      70,   144,    87,   169,    70,    70,    70,    27,    85,    13,
      68,    72,    31,    99,    72,    68,   146,    70,   142,    77,
      77,   142,   165,   165,    71,   123,   137,   125,    69,    99,
     144,   134,   130,   144,    13,   121,    70,    77,    71,    70,
     144,    21,   181,    73,   123,   142,    68,    70,   144,    70,
     123,   116,    70,   134,   133,    23,    24,   139,    68,    70,
     123,     4,    19,    25,    63,    67,    69,   140,   141,   167,
      71,   123,   138,   139,   129,     4,   140,     4,     4,   141,
      25,    71,    16,    17,    18,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,   138,    74,   138,   123,
      71,    70,    71,   140,    69,   141,   141,   141,   141,   141,
     141,   141,   141,   141,   141,   141,   141,   141,   141,    71
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    78,    79,    80,    80,    81,    81,    82,    83,    84,
      85,    85,    86,    86,    86,    87,    87,    89,    90,    88,
      91,    91,    91,    91,    91,    92,    93,    94,    94,    95,
      96,    95,    98,    97,    99,    99,   100,   100,   100,   101,
     101,   102,   102,   103,   103,   104,   104,   105,   105,   105,
     106,   106,   106,   107,   107,   108,   108,   109,   110,   111,
     111,   112,   112,   113,   113,   114,   115,   115,   115,   116,
     117,   116,   118,   118,   119,   120,   121,   121,   122,   122,
     122,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   125,   124,   127,   126,   129,   128,   130,   130,   130,
     131,   131,   133,   132,   134,   134,   135,   135,   137,   136,
     138,   138,   138,   139,   139,   139,   139,   139,   140,   140,
     141,   141,   141,   141,   141,   141,   141,   141,   141,   141,
     141,   141,   141,   141,   141,   141,   141,   141,   141,   142,
     142,   143,   144,   144,   144,   144,   144,   144,   144,   144,
     144,   144,   144,   144,   144,   144,   144,   144,   144,   144,
     144,   144,   144,   144,   144,   144,   144,   144,   144,   144,
     144,   144,   144,   144,   144,   144,   145,   145,   146,   146,
     146,   147,   147,   148,   148,   149,   149,   149,   150,   150,
     151,   152,   153,   153,   154,   154,   154,   154,   155,   154,
     154,   154,   154,   154,   154,   154,   154,   154,   154,   154,
     154,   154,   154,   154,   156,   154,   154,   154,   154,   154,
     154,   157,   157,   159,   158,   160,   160,   161,   162,   164,
     163,   165,   165,   166,   167,   167,   167,   168,   168,   169,
     170,   170,   172,   171,   173,   171,   171,   171,   174,   171,
     175,   171,   176,   171,   177,   171,   178,   178,   179,   179,
     179,   179,   180,   181,   181
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
       0,     1,     3,     2,     4,     4,     1,     1,     1,     0,
       0,     5,     2,     4,     3,     3,     1,     3,     0,     2,
       2,     2,     1,     1,     1,     1,     1,     1,     1,     2,
       2,     0,     6,     0,     8,     0,    10,     1,     1,     1,
       1,     3,     0,     8,     0,     1,     1,     1,     0,    10,
       2,     2,     0,     3,     5,     4,     4,     2,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     1,     2,     2,     2,     1,
       3,     1,     2,     3,     3,     5,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     2,     2,     2,     2,
       1,     1,     1,     1,     1,     1,     2,     3,     0,     1,
       2,     1,     2,     1,     3,     0,     1,     2,     1,     3,
       3,     1,     1,     2,     1,     1,     1,     1,     0,     5,
       3,     3,     6,     7,     8,     7,     5,     6,     5,     4,
       1,     3,     1,     1,     0,     6,     3,     5,     4,     4,
       4,     1,     3,     0,     3,     2,     4,     7,     9,     0,
       3,     0,     3,     1,     1,     3,     3,     1,     2,     3,
       0,     3,     0,     5,     0,     5,     6,     6,     0,     5,
       0,     5,     0,     7,     0,     8,     3,     3,     1,     2,
       3,     3,     6,     0,     2
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
#line 216 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
          { rule_program((yyval.node)); }
#line 2366 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 3: /* program: program def possible_semi_colon  */
#line 220 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                  { CREATE_TWO_VALUES((yyval.node), 0, (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 2372 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 4: /* program: %empty  */
#line 221 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                        { (yyval.node) = 0; }
#line 2378 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 6: /* possible_semi_colon: ';'  */
#line 226 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
        { yywarn("Extra ';'. Ignored."); }
#line 2384 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 7: /* inheritance: type_modifier_list L_INHERIT string_con1 ';'  */
#line 230 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                               { if (rule_inheritence(&(yyval.node), (yyvsp[-3].number), (yyvsp[-1].string))) { YYACCEPT; } }
#line 2390 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 8: /* real: L_REAL  */
#line 234 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
         { CREATE_REAL((yyval.node), (yyvsp[0].real)); }
#line 2396 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 9: /* number: L_NUMBER  */
#line 238 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
           { CREATE_NUMBER((yyval.node), (yyvsp[0].number)); }
#line 2402 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 10: /* optional_star: %empty  */
#line 242 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                        { (yyval.number) = 0; }
#line 2408 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 11: /* optional_star: '*'  */
#line 243 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                { (yyval.number) = TYPE_MOD_ARRAY; }
#line 2414 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 12: /* block_or_semi: block  */
#line 248 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
          {
            (yyval.node) = (yyvsp[0].decl).node;
            if (!(yyval.node)) {
              CREATE_RETURN((yyval.node), 0);
            }
          }
#line 2425 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 13: /* block_or_semi: ';'  */
#line 254 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2431 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 14: /* block_or_semi: error  */
#line 255 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
          { (yyval.node) = 0; }
#line 2437 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 15: /* identifier: L_DEFINED_NAME  */
#line 259 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                  { (yyval.string) = scratch_copy((yyvsp[0].ihe)->name); }
#line 2443 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 17: /* $@1: %empty  */
#line 264 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                  { (yyvsp[-2].number) = rule_func_type((yyvsp[-2].number), (yyvsp[-1].number), (yyvsp[0].string)); }
#line 2449 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 18: /* @2: %empty  */
#line 265 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                  { (yyval.number) = rule_func_proto((yyvsp[-6].number), (yyvsp[-5].number), &(yyvsp[-4].string), (yyvsp[-1].argument)); }
#line 2455 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 19: /* function: type optional_star identifier $@1 '(' argument ')' @2 block_or_semi  */
#line 266 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                  { rule_func(&(yyval.node), (yyvsp[-8].number), (yyvsp[-7].number), (yyvsp[-6].string), (yyvsp[-3].argument), &(yyvsp[-1].number), &(yyvsp[0].node)); }
#line 2461 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 21: /* def: type name_list ';'  */
#line 272 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                  {
                                    if (!((yyvsp[-2].number) & ~(DECL_MODS)) && (pragmas & PRAGMA_STRICT_TYPES))
                                      yyerror("Missing type for global variable declaration");
                                    (yyval.node) = 0;
                                  }
#line 2471 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 25: /* modifier_change: type_modifier_list ':'  */
#line 284 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 2489 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 26: /* member_name: optional_star identifier  */
#line 301 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                  {
                                    /* At this point, the current_type here is only a basic_type */
                                    /* and cannot be unused yet - Sym */

                                    if (current_type == TYPE_VOID)
                                      yyerror("Illegal to declare class member of type void.");
                                    add_local_name((yyvsp[0].string), current_type | (yyvsp[-1].number));
                                    scratch_free((yyvsp[0].string));
                                  }
#line 2503 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 30: /* $@3: %empty  */
#line 319 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                           { current_type = (yyvsp[0].number); }
#line 2509 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 32: /* @4: %empty  */
#line 324 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                             { (yyvsp[-2].ihe) = rule_define_class(&(yyval.number), (yyvsp[-1].string)); }
#line 2515 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 33: /* type_decl: type_modifier_list L_CLASS identifier '{' @4 member_list '}'  */
#line 325 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                             { rule_define_class_members((yyvsp[-5].ihe), (yyvsp[-2].number)); (yyval.node) = 0; }
#line 2521 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 35: /* new_local_name: L_DEFINED_NAME  */
#line 331 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                            {
                                              if ((yyvsp[0].ihe)->dn.local_num != -1) {
                                                yyerror("Illegal to redeclare local name '%s'", (yyvsp[0].ihe)->name);
                                              }
                                              (yyval.string) = scratch_copy((yyvsp[0].ihe)->name);
                                            }
#line 2532 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 37: /* atomic_type: L_CLASS L_DEFINED_NAME  */
#line 342 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                            {
                                              if ((yyvsp[0].ihe)->dn.class_num == -1) {
                                                yyerror("Undefined class '%s'", (yyvsp[0].ihe)->name);
                                                (yyval.number) = TYPE_ANY;
                                              } else {
                                                (yyval.number) = (yyvsp[0].ihe)->dn.class_num | TYPE_MOD_CLASS;
                                              }
                                            }
#line 2545 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 38: /* atomic_type: L_CLASS L_IDENTIFIER  */
#line 351 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                            {
                                              yyerror("Undefined class '%s'", (yyvsp[0].string));
                                              (yyval.number) = TYPE_ANY;
                                            }
#line 2554 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 40: /* opt_atomic_type: %empty  */
#line 359 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                        { (yyval.number) = TYPE_ANY; }
#line 2560 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 42: /* basic_type: opt_atomic_type L_ARRAY  */
#line 364 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                            { (yyval.number) = (yyvsp[-1].number) | TYPE_MOD_ARRAY; }
#line 2566 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 44: /* arg_type: basic_type ref  */
#line 369 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                   { (yyval.number) = (yyvsp[-1].number) | LOCAL_MOD_REF; }
#line 2572 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 45: /* optional_default_arg_value: %empty  */
#line 373 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
         { (yyval.node) = 0; }
#line 2578 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 46: /* optional_default_arg_value: ':' L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 374 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
}
#line 2601 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 47: /* new_arg: arg_type optional_star  */
#line 395 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                              {
                                                (yyval.number) = (yyvsp[-1].number) | (yyvsp[0].number);
                                                if ((yyvsp[-1].number) != TYPE_VOID)
                                                  add_local_name("", (yyvsp[-1].number) | (yyvsp[0].number));
                                              }
#line 2611 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 48: /* new_arg: arg_type optional_star new_local_name optional_default_arg_value  */
#line 401 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                              {
                                                if ((yyvsp[-3].number) == TYPE_VOID)
                                                  yyerror("Illegal to declare argument of type void.");
                                                add_local_name((yyvsp[-1].string), (yyvsp[-3].number) | (yyvsp[-2].number), (yyvsp[0].node));
                                                scratch_free((yyvsp[-1].string));
                                                (yyval.number) = (yyvsp[-3].number) | (yyvsp[-2].number);
                                              }
#line 2623 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 49: /* new_arg: new_local_name  */
#line 409 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
                                              {
                                                if (exact_types) {
                                                  yyerror("Missing type for argument");
                                                }
                                                add_local_name((yyvsp[0].string), TYPE_ANY);
                                                scratch_free((yyvsp[0].string));
                                                (yyval.number) = TYPE_ANY;
                                              }
#line 2636 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 50: /* argument: %empty  */
#line 421 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.argument).num_arg = 0;
      (yyval.argument).flags = 0;
    }
#line 2645 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 52: /* argument: argument_list L_DOT_DOT_DOT  */
#line 427 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 2664 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 53: /* argument_list: new_arg  */
#line 445 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 2679 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 54: /* argument_list: argument_list ',' new_arg  */
#line 456 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if (!(yyval.argument).num_arg)    /* first arg was void w/no name */
        yyerror("argument of type void must be the only argument.");
      if (((yyvsp[0].number) & TYPE_MASK) == TYPE_VOID && !((yyvsp[0].number) & TYPE_MOD_CLASS))
        yyerror("Illegal to declare argument of type void.");

      (yyval.argument) = (yyvsp[-2].argument);
      (yyval.argument).num_arg++;
    }
#line 2693 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 55: /* type_modifier_list: %empty  */
#line 469 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.number) = 0;
    }
#line 2701 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 56: /* type_modifier_list: L_TYPE_MODIFIER type_modifier_list  */
#line 473 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 2719 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 57: /* type: type_modifier_list opt_basic_type  */
#line 490 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = ((yyvsp[-1].number) << 16) | (yyvsp[0].number);
      current_type = (yyval.number);
    }
#line 2728 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 58: /* cast: '(' basic_type optional_star ')'  */
#line 498 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) | (yyvsp[-1].number);
    }
#line 2736 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 60: /* opt_basic_type: %empty  */
#line 506 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.number) = TYPE_UNKNOWN;
    }
#line 2744 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 63: /* new_name: optional_star identifier  */
#line 518 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 2773 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 64: /* new_name: optional_star identifier L_ASSIGN expr0  */
#line 543 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 2829 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 65: /* block: '{' local_declarations statements '}'  */
#line 598 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-2].decl).node && (yyvsp[-1].node)) {
        CREATE_STATEMENTS((yyval.decl).node, (yyvsp[-2].decl).node, (yyvsp[-1].node));
      } else (yyval.decl).node = ((yyvsp[-2].decl).node ? (yyvsp[-2].decl).node : (yyvsp[-1].node));
      (yyval.decl).num = (yyvsp[-2].decl).num;
    }
#line 2840 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 69: /* local_declarations: %empty  */
#line 610 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.decl).node = 0;
      (yyval.decl).num = 0;
    }
#line 2849 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 70: /* $@5: %empty  */
#line 615 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[0].number) == TYPE_VOID)
        yyerror("Illegal to declare local variable of type void.");
      /* can't do this in basic_type b/c local_name_list contains
       * expr0 which contains cast which contains basic_type
       */
      current_type = (yyvsp[0].number);
    }
#line 2862 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 71: /* local_declarations: local_declarations basic_type $@5 local_name_list ';'  */
#line 624 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-4].decl).node && (yyvsp[-1].decl).node) {
        CREATE_STATEMENTS((yyval.decl).node, (yyvsp[-4].decl).node, (yyvsp[-1].decl).node);
      } else (yyval.decl).node = ((yyvsp[-4].decl).node ? (yyvsp[-4].decl).node : (yyvsp[-1].decl).node);
      (yyval.decl).num = (yyvsp[-4].decl).num + (yyvsp[-1].decl).num;
    }
#line 2873 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 72: /* new_local_def: optional_star new_local_name  */
#line 634 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if (current_type & LOCAL_MOD_REF) {
        yyerror("Illegal to declare local variable as reference");
        current_type &= ~LOCAL_MOD_REF;
      }
      add_local_name((yyvsp[0].string), current_type | (yyvsp[-1].number) | LOCAL_MOD_UNUSED);

      scratch_free((yyvsp[0].string));
      (yyval.node) = 0;
    }
#line 2888 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 73: /* new_local_def: optional_star new_local_name L_ASSIGN expr0  */
#line 645 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 2923 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 74: /* single_new_local_def: arg_type optional_star new_local_name  */
#line 679 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-2].number) == TYPE_VOID)
        yyerror("Illegal to declare local variable of type void.");

      (yyval.number) = add_local_name((yyvsp[0].string), (yyvsp[-2].number) | (yyvsp[-1].number));
      scratch_free((yyvsp[0].string));
    }
#line 2935 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 75: /* single_new_local_def_with_init: single_new_local_def L_ASSIGN expr0  */
#line 690 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 2968 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 76: /* local_name_list: new_local_def  */
#line 722 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.decl).node = (yyvsp[0].node);
      (yyval.decl).num = 1;
    }
#line 2977 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 77: /* local_name_list: new_local_def ',' local_name_list  */
#line 727 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-2].node) && (yyvsp[0].decl).node) {
        CREATE_STATEMENTS((yyval.decl).node, (yyvsp[-2].node), (yyvsp[0].decl).node);
      } else (yyval.decl).node = ((yyvsp[-2].node) ? (yyvsp[-2].node) : (yyvsp[0].decl).node);
      (yyval.decl).num = 1 + (yyvsp[0].decl).num;
    }
#line 2988 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 78: /* statements: %empty  */
#line 737 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.node) = 0;
    }
#line 2996 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 79: /* statements: statement statements  */
#line 741 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-1].node) && (yyvsp[0].node)) {
        CREATE_STATEMENTS((yyval.node), (yyvsp[-1].node), (yyvsp[0].node));
      } else (yyval.node) = ((yyvsp[-1].node) ? (yyvsp[-1].node) : (yyvsp[0].node));
    }
#line 3006 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 80: /* statements: error ';'  */
#line 747 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = 0;
    }
#line 3014 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 81: /* statement: comma_expr ';'  */
#line 754 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3030 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 87: /* statement: decl_block  */
#line 771 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[0].decl).node;
      pop_n_locals((yyvsp[0].decl).num);
    }
#line 3039 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 88: /* statement: ';'  */
#line 776 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = 0;
    }
#line 3047 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 89: /* statement: L_BREAK ';'  */
#line 780 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3073 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 90: /* statement: L_CONTINUE ';'  */
#line 802 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if (context & SPECIAL_CONTEXT)
        yyerror("Cannot continue out of catch { } or time_expression { }");
      else
        if (!(context & LOOP_CONTEXT))
          yyerror("continue statement outside loop");
      CREATE_CONTROL_JUMP((yyval.node), CJ_CONTINUE);
    }
#line 3086 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 91: /* $@6: %empty  */
#line 814 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[-3].number) = context;
      context = LOOP_CONTEXT;
    }
#line 3095 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 92: /* while: L_WHILE '(' comma_expr ')' $@6 statement  */
#line 819 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_LOOP((yyval.node), 1, (yyvsp[0].node), 0, optimize_loop_test((yyvsp[-3].node)));
      context = (yyvsp[-5].number);
    }
#line 3104 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 93: /* $@7: %empty  */
#line 827 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[0].number) = context;
      context = LOOP_CONTEXT;
    }
#line 3113 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 94: /* do: L_DO $@7 statement L_WHILE '(' comma_expr ')' ';'  */
#line 832 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_LOOP((yyval.node), 0, (yyvsp[-5].node), 0, optimize_loop_test((yyvsp[-2].node)));
      context = (yyvsp[-7].number);
    }
#line 3122 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 95: /* $@8: %empty  */
#line 840 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[-5].decl).node = pop_value((yyvsp[-5].decl).node);
      (yyvsp[-7].number) = context;
      context = LOOP_CONTEXT;
    }
#line 3132 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 96: /* for: L_FOR '(' first_for_expr ';' for_expr ';' for_expr ')' $@8 statement  */
#line 846 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3152 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 97: /* foreach_var: L_DEFINED_NAME  */
#line 865 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3177 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 98: /* foreach_var: single_new_local_def  */
#line 886 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if (type_of_locals_ptr[(yyvsp[0].number)] & LOCAL_MOD_REF) {
        CREATE_OPCODE_1((yyval.decl).node, F_REF_LVALUE, 0, (yyvsp[0].number));
      } else {
        CREATE_OPCODE_1((yyval.decl).node, F_LOCAL_LVALUE, 0, (yyvsp[0].number));
        type_of_locals_ptr[(yyvsp[0].number)] &= ~LOCAL_MOD_UNUSED;
      }
      (yyval.decl).num = 1;
    }
#line 3191 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 99: /* foreach_var: L_IDENTIFIER  */
#line 896 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3209 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 100: /* foreach_vars: foreach_var  */
#line 913 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_FOREACH((yyval.decl).node, (yyvsp[0].decl).node, 0);
      (yyval.decl).num = (yyvsp[0].decl).num;
    }
#line 3218 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 101: /* foreach_vars: foreach_var ',' foreach_var  */
#line 918 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_FOREACH((yyval.decl).node, (yyvsp[-2].decl).node, (yyvsp[0].decl).node);
      (yyval.decl).num = (yyvsp[-2].decl).num + (yyvsp[0].decl).num;
      if ((yyvsp[-2].decl).node->v.number == F_REF_LVALUE)
        yyerror("Mapping key may not be a reference in foreach()");
    }
#line 3229 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 102: /* $@9: %empty  */
#line 928 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[-3].decl).node->v.expr = (yyvsp[-1].node);
      (yyvsp[-5].number) = context;
      context = LOOP_CONTEXT | LOOP_FOREACH;
    }
#line 3239 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 103: /* foreach: L_FOREACH '(' foreach_vars L_IN expr0 ')' $@9 statement  */
#line 934 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.decl).num = (yyvsp[-5].decl).num;

      CREATE_STATEMENTS((yyval.decl).node, (yyvsp[-5].decl).node, 0);
      CREATE_LOOP((yyval.decl).node->r.expr, 2, (yyvsp[0].node), 0, 0);
      CREATE_OPCODE((yyval.decl).node->r.expr->r.expr, F_NEXT_FOREACH, 0);

      context = (yyvsp[-7].number);
    }
#line 3253 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 104: /* for_expr: %empty  */
#line 947 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.node) = 0;
    }
#line 3261 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 106: /* first_for_expr: for_expr  */
#line 955 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.decl).node = (yyvsp[0].node);
      (yyval.decl).num = 0;
    }
#line 3270 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 107: /* first_for_expr: single_new_local_def_with_init  */
#line 960 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.decl).node = (yyvsp[0].node);
      (yyval.decl).num = 1;
    }
#line 3279 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 108: /* $@10: %empty  */
#line 968 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[-3].number) = context;
      context &= LOOP_CONTEXT;
      context |= SWITCH_CONTEXT;
      (yyvsp[-2].number) = mem_block[A_CASES].current_size;
    }
#line 3290 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 109: /* switch: L_SWITCH '(' comma_expr ')' $@10 '{' local_declarations case switch_block '}'  */
#line 975 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3323 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 110: /* switch_block: case switch_block  */
#line 1007 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[0].node)){
        CREATE_STATEMENTS((yyval.node), (yyvsp[-1].node), (yyvsp[0].node));
      } else (yyval.node) = (yyvsp[-1].node);
    }
#line 3333 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 111: /* switch_block: statement switch_block  */
#line 1013 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[0].node)){
        CREATE_STATEMENTS((yyval.node), (yyvsp[-1].node), (yyvsp[0].node));
      } else (yyval.node) = (yyvsp[-1].node);
    }
#line 3343 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 112: /* switch_block: %empty  */
#line 1019 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.node) = 0;
    }
#line 3351 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 113: /* case: L_CASE case_label ':'  */
#line 1026 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[-1].node);
      (yyval.node)->v.expr = 0;

      add_to_mem_block(A_CASES, (char *)&((yyvsp[-1].node)), sizeof((yyvsp[-1].node)));
    }
#line 3362 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 114: /* case: L_CASE case_label L_RANGE case_label ':'  */
#line 1033 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3380 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 115: /* case: L_CASE case_label L_RANGE ':'  */
#line 1047 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3398 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 116: /* case: L_CASE L_RANGE case_label ':'  */
#line 1061 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3415 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 117: /* case: L_DEFAULT ':'  */
#line 1074 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3432 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 118: /* case_label: constant  */
#line 1090 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3450 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 119: /* case_label: string_con1  */
#line 1104 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3466 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 120: /* constant: constant '|' constant  */
#line 1119 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) | (yyvsp[0].number);
    }
#line 3474 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 121: /* constant: constant '^' constant  */
#line 1123 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) ^ (yyvsp[0].number);
    }
#line 3482 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 122: /* constant: constant '&' constant  */
#line 1127 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) & (yyvsp[0].number);
    }
#line 3490 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 123: /* constant: constant L_EQ constant  */
#line 1131 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) == (yyvsp[0].number);
    }
#line 3498 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 124: /* constant: constant L_NE constant  */
#line 1135 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) != (yyvsp[0].number);
    }
#line 3506 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 125: /* constant: constant L_ORDER constant  */
#line 1139 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      switch((yyvsp[-1].number)){
        case F_GE: (yyval.number) = (yyvsp[-2].number) >= (yyvsp[0].number); break;
        case F_LE: (yyval.number) = (yyvsp[-2].number) <= (yyvsp[0].number); break;
        case F_GT: (yyval.number) = (yyvsp[-2].number) >  (yyvsp[0].number); break;
      }
    }
#line 3518 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 126: /* constant: constant '<' constant  */
#line 1147 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) < (yyvsp[0].number);
    }
#line 3526 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 127: /* constant: constant L_LSH constant  */
#line 1151 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) << (yyvsp[0].number);
    }
#line 3534 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 128: /* constant: constant L_RSH constant  */
#line 1155 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) >> (yyvsp[0].number);
    }
#line 3542 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 129: /* constant: constant '+' constant  */
#line 1159 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) + (yyvsp[0].number);
    }
#line 3550 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 130: /* constant: constant '-' constant  */
#line 1163 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) - (yyvsp[0].number);
    }
#line 3558 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 131: /* constant: constant '*' constant  */
#line 1167 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-2].number) * (yyvsp[0].number);
    }
#line 3566 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 132: /* constant: constant '%' constant  */
#line 1171 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[0].number)) (yyval.number) = (yyvsp[-2].number) % (yyvsp[0].number); else yyerror("Modulo by zero");
    }
#line 3574 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 133: /* constant: constant '/' constant  */
#line 1175 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[0].number)) (yyval.number) = (yyvsp[-2].number) / (yyvsp[0].number); else yyerror("Division by zero");
    }
#line 3582 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 134: /* constant: '(' constant ')'  */
#line 1179 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[-1].number);
    }
#line 3590 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 135: /* constant: L_NUMBER  */
#line 1183 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = (yyvsp[0].number);
    }
#line 3598 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 136: /* constant: '-' L_NUMBER  */
#line 1187 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = -(yyvsp[0].number);
    }
#line 3606 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 137: /* constant: L_NOT L_NUMBER  */
#line 1191 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = !(yyvsp[0].number);
    }
#line 3614 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 138: /* constant: '~' L_NUMBER  */
#line 1195 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = ~(yyvsp[0].number);
    }
#line 3622 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 139: /* comma_expr: expr0  */
#line 1202 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 3630 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 140: /* comma_expr: comma_expr ',' expr0  */
#line 1206 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_TWO_VALUES((yyval.node), (yyvsp[0].node)->type, pop_value((yyvsp[-2].node)), (yyvsp[0].node));
    }
#line 3638 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 142: /* expr0: ref lvalue  */
#line 1217 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3669 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 143: /* expr0: lvalue L_ASSIGN expr0  */
#line 1244 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3698 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 144: /* expr0: error L_ASSIGN expr0  */
#line 1269 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      yyerror("Illegal LHS");
      CREATE_ERROR((yyval.node));
    }
#line 3707 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 145: /* expr0: expr0 '?' expr0 ':' expr0  */
#line 1274 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3735 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 146: /* expr0: expr0 L_LOR expr0  */
#line 1298 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_LAND_LOR((yyval.node), F_LOR, (yyvsp[-2].node), (yyvsp[0].node));
      if (IS_NODE((yyvsp[-2].node), NODE_LAND_LOR, F_LOR))
        (yyvsp[-2].node)->kind = NODE_BRANCH_LINK;
    }
#line 3745 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 147: /* expr0: expr0 L_LAND expr0  */
#line 1304 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_LAND_LOR((yyval.node), F_LAND, (yyvsp[-2].node), (yyvsp[0].node));
      if (IS_NODE((yyvsp[-2].node), NODE_LAND_LOR, F_LAND))
        (yyvsp[-2].node)->kind = NODE_BRANCH_LINK;
    }
#line 3755 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 148: /* expr0: expr0 '|' expr0  */
#line 1310 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3784 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 149: /* expr0: expr0 '^' expr0  */
#line 1335 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = binary_int_op((yyvsp[-2].node), (yyvsp[0].node), F_XOR, "^");
    }
#line 3792 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 150: /* expr0: expr0 '&' expr0  */
#line 1339 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3819 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 151: /* expr0: expr0 L_EQ expr0  */
#line 1362 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3845 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 152: /* expr0: expr0 L_NE expr0  */
#line 1384 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3863 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 153: /* expr0: expr0 L_ORDER expr0  */
#line 1398 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3911 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 154: /* expr0: expr0 '<' expr0  */
#line 1442 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 3952 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 155: /* expr0: expr0 L_LSH expr0  */
#line 1479 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = binary_int_op((yyvsp[-2].node), (yyvsp[0].node), F_LSH, "<<");
    }
#line 3960 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 156: /* expr0: expr0 L_RSH expr0  */
#line 1483 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = binary_int_op((yyvsp[-2].node), (yyvsp[0].node), F_RSH, ">>");
    }
#line 3968 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 157: /* expr0: expr0 '+' expr0  */
#line 1487 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4142 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 158: /* expr0: expr0 '-' expr0  */
#line 1657 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4240 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 159: /* expr0: expr0 '*' expr0  */
#line 1751 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4324 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 160: /* expr0: expr0 '%' expr0  */
#line 1831 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = binary_int_op((yyvsp[-2].node), (yyvsp[0].node), F_MOD, "%");
    }
#line 4332 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 161: /* expr0: expr0 '/' expr0  */
#line 1835 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4431 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 162: /* expr0: cast expr0  */
#line 1930 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4456 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 163: /* expr0: L_INC lvalue  */
#line 1951 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4481 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 164: /* expr0: L_DEC lvalue  */
#line 1972 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4507 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 165: /* expr0: L_NOT expr0  */
#line 1994 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[0].node)->kind == NODE_NUMBER) {
        (yyval.node) = (yyvsp[0].node);
        (yyval.node)->v.number = !((yyval.node)->v.number);
      } else {
        CREATE_UNARY_OP((yyval.node), F_NOT, TYPE_NUMBER, (yyvsp[0].node));
      }
    }
#line 4520 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 166: /* expr0: '~' expr0  */
#line 2003 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4535 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 167: /* expr0: '-' expr0  */
#line 2014 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4563 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 168: /* expr0: lvalue L_INC  */
#line 2038 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4589 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 169: /* expr0: lvalue L_DEC  */
#line 2060 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4614 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 176: /* return: L_RETURN ';'  */
#line 2090 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if (exact_types && !IS_TYPE(exact_types, TYPE_VOID))
        yywarn("Non-void functions must return a value.");
      CREATE_RETURN((yyval.node), 0);
    }
#line 4624 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 177: /* return: L_RETURN comma_expr ';'  */
#line 2096 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4645 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 178: /* expr_list: %empty  */
#line 2116 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      CREATE_EXPR_LIST((yyval.node), 0);
    }
#line 4653 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 179: /* expr_list: expr_list2  */
#line 2120 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_EXPR_LIST((yyval.node), (yyvsp[0].node));
    }
#line 4661 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 180: /* expr_list: expr_list2 ','  */
#line 2124 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_EXPR_LIST((yyval.node), (yyvsp[-1].node));
    }
#line 4669 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 181: /* expr_list_node: expr0  */
#line 2131 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_EXPR_NODE((yyval.node), (yyvsp[0].node), 0);
    }
#line 4677 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 182: /* expr_list_node: expr0 L_DOT_DOT_DOT  */
#line 2135 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_EXPR_NODE((yyval.node), (yyvsp[-1].node), 1);
    }
#line 4685 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 183: /* expr_list2: expr_list_node  */
#line 2142 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[0].node)->kind = 1;
      (yyval.node) = (yyvsp[0].node);
    }
#line 4694 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 184: /* expr_list2: expr_list2 ',' expr_list_node  */
#line 2147 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyvsp[0].node)->kind = 0;

      (yyval.node) = (yyvsp[-2].node);
      (yyval.node)->kind++;
      (yyval.node)->l.expr->r.expr = (yyvsp[0].node);
      (yyval.node)->l.expr = (yyvsp[0].node);
    }
#line 4707 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 185: /* expr_list3: %empty  */
#line 2159 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      /* this is a dummy node */
      CREATE_EXPR_LIST((yyval.node), 0);
    }
#line 4716 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 186: /* expr_list3: expr_list4  */
#line 2164 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_EXPR_LIST((yyval.node), (yyvsp[0].node));
    }
#line 4724 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 187: /* expr_list3: expr_list4 ','  */
#line 2168 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_EXPR_LIST((yyval.node), (yyvsp[-1].node));
    }
#line 4732 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 188: /* expr_list4: assoc_pair  */
#line 2175 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = new_node_no_line();
      (yyval.node)->kind = 2;
      (yyval.node)->v.expr = (yyvsp[0].node);
      (yyval.node)->r.expr = 0;
      (yyval.node)->type = 0;
      /* we keep track of the end of the chain in the left nodes */
      (yyval.node)->l.expr = (yyval.node);
    }
#line 4746 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 189: /* expr_list4: expr_list4 ',' assoc_pair  */
#line 2185 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4765 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 190: /* assoc_pair: expr0 ':' expr0  */
#line 2203 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_TWO_VALUES((yyval.node), 0, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 4773 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 191: /* lvalue: expr4  */
#line 2210 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4890 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 193: /* l_new_function_open: L_FUNCTION_OPEN efun_override  */
#line 2327 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = ((yyvsp[0].number) << 8) | FP_EFUN;
    }
#line 4898 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 195: /* expr4: L_DEFINED_NAME  */
#line 2335 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4945 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 196: /* expr4: L_IDENTIFIER  */
#line 2378 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 4966 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 197: /* expr4: L_PARAMETER  */
#line 2395 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_PARAMETER((yyval.node), TYPE_ANY, (yyvsp[0].number));
    }
#line 4974 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 198: /* @11: %empty  */
#line 2399 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.contextp) = current_function_context;
      /* already flagged as an error */
      if (current_function_context)
        current_function_context = current_function_context->parent;
    }
#line 4985 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 199: /* expr4: '$' '(' @11 comma_expr ')'  */
#line 2406 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5012 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 200: /* expr4: expr4 L_ARROW identifier  */
#line 2429 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5040 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 201: /* expr4: expr4 L_DOT identifier  */
#line 2453 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5068 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 202: /* expr4: expr4 '[' comma_expr L_RANGE comma_expr ']'  */
#line 2477 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5083 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 203: /* expr4: expr4 '[' '<' comma_expr L_RANGE comma_expr ']'  */
#line 2488 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = make_range_node(F_RN_RANGE, (yyvsp[-6].node), (yyvsp[-3].node), (yyvsp[-1].node));
    }
#line 5091 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 204: /* expr4: expr4 '[' '<' comma_expr L_RANGE '<' comma_expr ']'  */
#line 2492 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-1].node)->kind == NODE_NUMBER && (yyvsp[-1].node)->v.number <= 1)
        (yyval.node) = make_range_node(F_RE_RANGE, (yyvsp[-7].node), (yyvsp[-4].node), 0);
      else
        (yyval.node) = make_range_node(F_RR_RANGE, (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-1].node));
    }
#line 5102 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 205: /* expr4: expr4 '[' comma_expr L_RANGE '<' comma_expr ']'  */
#line 2499 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if ((yyvsp[-1].node)->kind == NODE_NUMBER && (yyvsp[-1].node)->v.number <= 1)
        (yyval.node) = make_range_node(F_NE_RANGE, (yyvsp[-6].node), (yyvsp[-4].node), 0);
      else
        (yyval.node) = make_range_node(F_NR_RANGE, (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-1].node));
    }
#line 5113 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 206: /* expr4: expr4 '[' comma_expr L_RANGE ']'  */
#line 2506 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = make_range_node(F_NE_RANGE, (yyvsp[-4].node), (yyvsp[-2].node), 0);
    }
#line 5121 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 207: /* expr4: expr4 '[' '<' comma_expr L_RANGE ']'  */
#line 2510 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = make_range_node(F_RE_RANGE, (yyvsp[-5].node), (yyvsp[-2].node), 0);
    }
#line 5129 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 208: /* expr4: expr4 '[' '<' comma_expr ']'  */
#line 2514 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5178 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 209: /* expr4: expr4 '[' comma_expr ']'  */
#line 2559 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5238 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 211: /* expr4: '(' comma_expr ')'  */
#line 2616 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[-1].node);
    }
#line 5246 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 214: /* @12: %empty  */
#line 2622 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5271 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 215: /* expr4: L_BASIC_TYPE @12 '(' argument ')' block  */
#line 2643 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5311 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 216: /* expr4: l_new_function_open ':' ')'  */
#line 2679 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5353 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 217: /* expr4: l_new_function_open ',' expr_list2 ':' ')'  */
#line 2717 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5452 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 218: /* expr4: L_FUNCTION_OPEN comma_expr ':' ')'  */
#line 2812 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5479 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 219: /* expr4: L_MAPPING_OPEN expr_list3 ']' ')'  */
#line 2835 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if (CONFIG_INT(__RC_WOMBLES__)) {
        if(*(outp-2) != ']') {
          yyerror("End of mapping not found");
        }
      }
      CREATE_CALL((yyval.node), F_AGGREGATE_ASSOC, TYPE_MAPPING, (yyvsp[-2].node));
    }
#line 5492 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 220: /* expr4: L_ARRAY_OPEN expr_list '}' ')'  */
#line 2844 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      if (CONFIG_INT(__RC_WOMBLES__)) {
        if(*(outp-2) != '}') {
          yyerror("End of array not found");
        }
      }
      CREATE_CALL((yyval.node), F_AGGREGATE, TYPE_ANY | TYPE_MOD_ARRAY, (yyvsp[-2].node));
    }
#line 5505 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 221: /* expr_or_block: block  */
#line 2856 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[0].decl).node;
    }
#line 5513 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 222: /* expr_or_block: '(' comma_expr ')'  */
#line 2860 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = insert_pop_value((yyvsp[-1].node));
    }
#line 5521 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 223: /* @13: %empty  */
#line 2867 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      context = SPECIAL_CONTEXT;
    }
#line 5530 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 224: /* catch: L_CATCH @13 expr_or_block  */
#line 2872 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_CATCH((yyval.node), (yyvsp[0].node));
      context = (yyvsp[-1].number);
    }
#line 5539 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 225: /* tree: L_TREE block  */
#line 2880 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
#ifdef DEBUG
      (yyval.node) = new_node_no_line();
      lpc_tree_form((yyvsp[0].decl).node, (yyval.node));
#endif
    }
#line 5550 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 226: /* tree: L_TREE '(' comma_expr ')'  */
#line 2887 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
#ifdef DEBUG
      (yyval.node) = new_node_no_line();
      lpc_tree_form((yyvsp[-1].node), (yyval.node));
#endif
    }
#line 5561 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 227: /* sscanf: L_SSCANF '(' expr0 ',' expr0 lvalue_list ')'  */
#line 2897 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      int p = (yyvsp[-1].node)->v.number;
      CREATE_LVALUE_EFUN((yyval.node), TYPE_NUMBER, (yyvsp[-1].node));
      CREATE_BINARY_OP_1((yyval.node)->l.expr, F_SSCANF, 0, (yyvsp[-4].node), (yyvsp[-2].node), p);
    }
#line 5571 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 228: /* parse_command: L_PARSE_COMMAND '(' expr0 ',' expr0 ',' expr0 lvalue_list ')'  */
#line 2906 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      int p = (yyvsp[-1].node)->v.number;
      CREATE_LVALUE_EFUN((yyval.node), TYPE_NUMBER, (yyvsp[-1].node));
      CREATE_TERNARY_OP_1((yyval.node)->l.expr, F_PARSE_COMMAND, 0,
          (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-2].node), p);
    }
#line 5582 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 229: /* @14: %empty  */
#line 2916 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      context = SPECIAL_CONTEXT;
    }
#line 5591 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 230: /* time_expression: L_TIME_EXPRESSION @14 expr_or_block  */
#line 2921 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_TIME_EXPRESSION((yyval.node), (yyvsp[0].node));
      context = (yyvsp[-1].number);
    }
#line 5600 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 231: /* lvalue_list: %empty  */
#line 2929 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.node) = new_node_no_line();
      (yyval.node)->r.expr = 0;
      (yyval.node)->v.number = 0;
    }
#line 5610 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 232: /* lvalue_list: ',' lvalue lvalue_list  */
#line 2935 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      parse_node_t *insert;

      (yyval.node) = (yyvsp[0].node);
      insert = new_node_no_line();
      insert->r.expr = (yyvsp[0].node)->r.expr;
      insert->l.expr = (yyvsp[-1].node);
      (yyvsp[0].node)->r.expr = insert;
      (yyval.node)->v.number++;
    }
#line 5625 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 233: /* string: string_con2  */
#line 2949 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      CREATE_STRING((yyval.node), (yyvsp[0].string));
      scratch_free((yyvsp[0].string));
    }
#line 5634 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 235: /* string_con1: '(' string_con1 ')'  */
#line 2958 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.string) = (yyvsp[-1].string);
    }
#line 5642 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 236: /* string_con1: string_con1 '+' string_con1  */
#line 2962 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.string) = scratch_join((yyvsp[-2].string), (yyvsp[0].string));
    }
#line 5650 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 238: /* string_con2: string_con2 L_STRING  */
#line 2970 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.string) = scratch_join((yyvsp[-1].string), (yyvsp[0].string));
    }
#line 5658 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 239: /* class_init: identifier ':' expr0  */
#line 2977 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = new_node();
      (yyval.node)->l.expr = (parse_node_t *)(yyvsp[-2].string);
      (yyval.node)->v.expr = (yyvsp[0].node);
      (yyval.node)->r.expr = 0;
    }
#line 5669 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 240: /* opt_class_init: %empty  */
#line 2987 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
            {
      (yyval.node) = 0;
    }
#line 5677 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 241: /* opt_class_init: opt_class_init ',' class_init  */
#line 2991 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->r.expr = (yyvsp[-2].node);
    }
#line 5686 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 242: /* @15: %empty  */
#line 2999 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      (yyvsp[0].number) = num_refs;
      context |= ARG_LIST;
    }
#line 5696 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 243: /* function_call: efun_override '(' @15 expr_list ')'  */
#line 3005 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      context = (yyvsp[-2].number);
      (yyval.node) = validate_efun_call((yyvsp[-4].number),(yyvsp[-1].node));
      (yyval.node) = check_refs(num_refs - (yyvsp[-3].number), (yyvsp[-1].node), (yyval.node));
      num_refs = (yyvsp[-3].number);
    }
#line 5707 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 244: /* @16: %empty  */
#line 3012 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      (yyvsp[0].number) = num_refs;
      context |= ARG_LIST;
    }
#line 5717 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 245: /* function_call: L_NEW '(' @16 expr_list ')'  */
#line 3018 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5746 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 246: /* function_call: L_NEW '(' L_CLASS L_DEFINED_NAME opt_class_init ')'  */
#line 3043 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5785 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 247: /* function_call: L_NEW '(' L_CLASS L_IDENTIFIER opt_class_init ')'  */
#line 3078 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5807 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 248: /* @17: %empty  */
#line 3096 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      (yyvsp[0].number) = num_refs;
      context |= ARG_LIST;
    }
#line 5817 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 249: /* function_call: L_DEFINED_NAME '(' @17 expr_list ')'  */
#line 3102 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5875 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 250: /* @18: %empty  */
#line 3156 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      (yyvsp[0].number) = num_refs;
      context |= ARG_LIST;
    }
#line 5885 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 251: /* function_call: function_name '(' @18 expr_list ')'  */
#line 3162 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 5951 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 252: /* @19: %empty  */
#line 3224 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      (yyvsp[0].number) = num_refs;
      context |= ARG_LIST;
    }
#line 5961 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 253: /* function_call: expr4 L_ARROW identifier '(' @19 expr_list ')'  */
#line 3230 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 6010 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 254: /* @20: %empty  */
#line 3275 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.number) = context;
      (yyvsp[0].number) = num_refs;
      context |= ARG_LIST;
    }
#line 6020 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 255: /* function_call: '(' '*' comma_expr ')' '(' @20 expr_list ')'  */
#line 3281 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 6046 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 256: /* efun_override: L_EFUN L_COLON_COLON identifier  */
#line 3306 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 6070 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 257: /* efun_override: L_EFUN L_COLON_COLON L_NEW  */
#line 3326 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 6087 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 259: /* function_name: L_COLON_COLON identifier  */
#line 3343 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 6105 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 260: /* function_name: L_BASIC_TYPE L_COLON_COLON identifier  */
#line 3357 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 6124 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 261: /* function_name: identifier L_COLON_COLON identifier  */
#line 3372 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 6140 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 262: /* cond: L_IF '(' comma_expr ')' statement optional_else_part  */
#line 3387 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
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
#line 6175 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 263: /* optional_else_part: %empty  */
#line 3421 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = 0;
    }
#line 6183 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;

  case 264: /* optional_else_part: L_ELSE statement  */
#line 3425 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"
    {
      (yyval.node) = (yyvsp[0].node);
    }
#line 6191 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"
    break;


#line 6195 "/home/sunyc/src/fluffos/cmake-build-debug-asan-wsl-clang/src/grammar.autogen.cc"

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

#line 3429 "/home/sunyc/src/fluffos/src/compiler/internal/grammar.y"

