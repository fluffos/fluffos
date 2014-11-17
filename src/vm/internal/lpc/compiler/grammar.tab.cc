/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 3 "lpc/compiler/grammar.y" /* yacc.c:339  */


extern char *outp;
#include "base/std.h"

#include "lpc/compiler/compiler.h"
#include "lpc/compiler/lex.h"
#include "lpc/compiler/scratchpad.h"

#include "lpc_incl.h"
#include "simul_efun.h"
#include "lpc/compiler/generate.h"
#include "master.h"

#define YYSTACK_USE_ALLOCA 0
#line 17 "lpc/compiler/grammar.y.pre"
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


#line 105 "y.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    L_STRING = 258,
    L_NUMBER = 259,
    L_REAL = 260,
    L_BASIC_TYPE = 261,
    L_TYPE_MODIFIER = 262,
    L_DEFINED_NAME = 263,
    L_IDENTIFIER = 264,
    L_EFUN = 265,
    L_INC = 266,
    L_DEC = 267,
    L_ASSIGN = 268,
    L_LAND = 269,
    L_LOR = 270,
    L_LSH = 271,
    L_RSH = 272,
    L_ORDER = 273,
    L_NOT = 274,
    L_IF = 275,
    L_ELSE = 276,
    L_SWITCH = 277,
    L_CASE = 278,
    L_DEFAULT = 279,
    L_RANGE = 280,
    L_DOT_DOT_DOT = 281,
    L_WHILE = 282,
    L_DO = 283,
    L_FOR = 284,
    L_FOREACH = 285,
    L_IN = 286,
    L_BREAK = 287,
    L_CONTINUE = 288,
    L_RETURN = 289,
    L_ARROW = 290,
    L_INHERIT = 291,
    L_COLON_COLON = 292,
    L_ARRAY_OPEN = 293,
    L_MAPPING_OPEN = 294,
    L_FUNCTION_OPEN = 295,
    L_NEW_FUNCTION_OPEN = 296,
    L_SSCANF = 297,
    L_CATCH = 298,
    L_TREE = 299,
    L_REF = 300,
    L_PARSE_COMMAND = 301,
    L_TIME_EXPRESSION = 302,
    L_CLASS = 303,
    L_NEW = 304,
    L_PARAMETER = 305,
    LOWER_THAN_ELSE = 306,
    L_EQ = 307,
    L_NE = 308
  };
#endif
/* Tokens.  */
#define L_STRING 258
#define L_NUMBER 259
#define L_REAL 260
#define L_BASIC_TYPE 261
#define L_TYPE_MODIFIER 262
#define L_DEFINED_NAME 263
#define L_IDENTIFIER 264
#define L_EFUN 265
#define L_INC 266
#define L_DEC 267
#define L_ASSIGN 268
#define L_LAND 269
#define L_LOR 270
#define L_LSH 271
#define L_RSH 272
#define L_ORDER 273
#define L_NOT 274
#define L_IF 275
#define L_ELSE 276
#define L_SWITCH 277
#define L_CASE 278
#define L_DEFAULT 279
#define L_RANGE 280
#define L_DOT_DOT_DOT 281
#define L_WHILE 282
#define L_DO 283
#define L_FOR 284
#define L_FOREACH 285
#define L_IN 286
#define L_BREAK 287
#define L_CONTINUE 288
#define L_RETURN 289
#define L_ARROW 290
#define L_INHERIT 291
#define L_COLON_COLON 292
#define L_ARRAY_OPEN 293
#define L_MAPPING_OPEN 294
#define L_FUNCTION_OPEN 295
#define L_NEW_FUNCTION_OPEN 296
#define L_SSCANF 297
#define L_CATCH 298
#define L_TREE 299
#define L_REF 300
#define L_PARSE_COMMAND 301
#define L_TIME_EXPRESSION 302
#define L_CLASS 303
#define L_NEW 304
#define L_PARAMETER 305
#define LOWER_THAN_ELSE 306
#define L_EQ 307
#define L_NE 308

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 121 "lpc/compiler/grammar.y" /* yacc.c:355  */

    LPC_INT number; /* 8 */
    LPC_FLOAT real; /* 8 */
    char *string;
    struct { short num_arg; char flags; } argument;
    ident_hash_elem_t *ihe;
    parse_node_t *node;
    function_context_t *contextp;
    struct {
	parse_node_t *node;
        char num;
    } decl; /* 5 */
    struct {
	char num_local;
	char max_num_locals; 
	short context; 
	unsigned short save_current_type;
	unsigned short save_exact_types;
    } func_block; /* 8 */

#line 272 "y.tab.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 287 "y.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

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

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
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


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

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
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
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
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
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
#define YYLAST   1803

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  75
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  101
/* YYNRULES -- Number of rules.  */
#define YYNRULES  257
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  489

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   308

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    72,    62,    55,     2,
      66,    67,    61,    59,    69,    60,     2,    63,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    68,    65,
      58,     2,     2,    52,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    73,     2,    74,    54,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    70,    53,    71,    64,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50,    51,    56,    57
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   207,   207,   215,   221,   227,   229,   239,   324,   332,
     341,   345,   353,   361,   366,   374,   379,   384,   426,   383,
     484,   491,   492,   493,   496,   514,   528,   529,   532,   535,
     534,   544,   543,   624,   625,   643,   644,   661,   678,   682,
     683,   691,   698,   707,   721,   726,   727,   746,   758,   773,
     777,   800,   809,   817,   819,   826,   827,   831,   857,   913,
     923,   923,   923,   927,   933,   932,   954,   966,  1001,  1013,
    1046,  1052,  1064,  1068,  1075,  1083,  1096,  1097,  1098,  1099,
    1100,  1101,  1107,  1112,  1135,  1149,  1148,  1164,  1163,  1179,
    1178,  1204,  1226,  1237,  1255,  1261,  1273,  1272,  1294,  1298,
    1302,  1308,  1318,  1317,  1358,  1365,  1373,  1381,  1389,  1404,
    1419,  1433,  1450,  1465,  1481,  1486,  1491,  1496,  1501,  1506,
    1515,  1520,  1525,  1530,  1535,  1540,  1545,  1550,  1555,  1560,
    1565,  1570,  1575,  1583,  1588,  1596,  1600,  1629,  1655,  1661,
    1686,  1693,  1700,  1726,  1731,  1755,  1778,  1793,  1838,  1876,
    1881,  1886,  2057,  2152,  2233,  2238,  2334,  2356,  2378,  2401,
    2411,  2423,  2448,  2471,  2493,  2494,  2495,  2496,  2497,  2498,
    2502,  2509,  2531,  2535,  2540,  2548,  2553,  2561,  2568,  2582,
    2587,  2592,  2600,  2611,  2630,  2638,  2754,  2755,  2764,  2765,
    2808,  2825,  2831,  2830,  2862,  2887,  2892,  2897,  2905,  2913,
    2918,  2923,  2969,  3024,  3025,  3030,  3031,  3033,  3032,  3089,
    3127,  3222,  3245,  3254,  3266,  3271,  3280,  3279,  3294,  3301,
    3310,  3320,  3332,  3331,  3347,  3353,  3368,  3377,  3378,  3383,
    3391,  3392,  3399,  3411,  3415,  3426,  3425,  3441,  3440,  3473,
    3509,  3529,  3528,  3598,  3597,  3671,  3670,  3724,  3723,  3755,
    3781,  3797,  3798,  3813,  3829,  3845,  3880,  3885
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "L_STRING", "L_NUMBER", "L_REAL",
  "L_BASIC_TYPE", "L_TYPE_MODIFIER", "L_DEFINED_NAME", "L_IDENTIFIER",
  "L_EFUN", "L_INC", "L_DEC", "L_ASSIGN", "L_LAND", "L_LOR", "L_LSH",
  "L_RSH", "L_ORDER", "L_NOT", "L_IF", "L_ELSE", "L_SWITCH", "L_CASE",
  "L_DEFAULT", "L_RANGE", "L_DOT_DOT_DOT", "L_WHILE", "L_DO", "L_FOR",
  "L_FOREACH", "L_IN", "L_BREAK", "L_CONTINUE", "L_RETURN", "L_ARROW",
  "L_INHERIT", "L_COLON_COLON", "L_ARRAY_OPEN", "L_MAPPING_OPEN",
  "L_FUNCTION_OPEN", "L_NEW_FUNCTION_OPEN", "L_SSCANF", "L_CATCH",
  "L_TREE", "L_REF", "L_PARSE_COMMAND", "L_TIME_EXPRESSION", "L_CLASS",
  "L_NEW", "L_PARAMETER", "LOWER_THAN_ELSE", "'?'", "'|'", "'^'", "'&'",
  "L_EQ", "L_NE", "'<'", "'+'", "'-'", "'*'", "'%'", "'/'", "'~'", "';'",
  "'('", "')'", "':'", "','", "'{'", "'}'", "'$'", "'['", "']'", "$accept",
  "all", "program", "possible_semi_colon", "inheritance", "real", "number",
  "optional_star", "block_or_semi", "identifier", "def", "$@1", "@2",
  "modifier_change", "member_name", "member_name_list", "member_list",
  "$@3", "type_decl", "@4", "new_local_name", "atomic_type", "basic_type",
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
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,    63,   124,    94,    38,   307,   308,    60,    43,
      45,    42,    37,    47,   126,    59,    40,    41,    58,    44,
     123,   125,    36,    91,    93
};
# endif

#define YYPACT_NINF -379

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-379)))

#define YYTABLE_NINF -252

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -379,    60,   131,  -379,    56,  -379,     3,  -379,  -379,   126,
      36,  -379,  -379,  -379,  -379,    29,   117,  -379,  -379,  -379,
    -379,  -379,   187,    78,   157,  -379,    29,    83,   208,   177,
     195,   229,  -379,  -379,    28,  -379,    36,   -12,    29,  -379,
    -379,  -379,  1592,   260,   187,  -379,  -379,  -379,  -379,   292,
    -379,  -379,   317,     7,     9,   331,   127,   127,  1592,   187,
    1100,   306,  1592,  -379,   319,  -379,   174,  -379,   322,  -379,
     342,  -379,  1592,  1592,   888,   350,  -379,  -379,   336,  1592,
     127,  1140,   266,   254,    15,  -379,  -379,  -379,  -379,  -379,
    -379,   208,  -379,   352,   353,   172,   370,     8,  1592,   187,
     356,  -379,    44,  1172,  -379,   125,  -379,  -379,  -379,  1071,
     182,  -379,   368,   856,   364,   371,  -379,   272,  1140,   352,
    1592,   180,  1592,  -379,  -379,  1592,   180,   391,  -379,  -379,
      12,   276,  1592,    36,   106,  -379,   187,  -379,  -379,  1592,
    1592,  1592,  1592,  1592,  1592,  1592,  1592,  1592,  1592,  1592,
    1592,  1592,  1592,  1592,  1592,  1592,  -379,  -379,  1592,   378,
    1592,   187,  1242,  -379,  -379,  -379,  -379,  -379,   420,    36,
    -379,   384,    16,  -379,  -379,  1140,  -379,   172,  1312,  -379,
    -379,  -379,   404,   958,  1592,   405,   530,   407,  1592,  1629,
    1592,  -379,  -379,   116,   674,  1651,  -379,   398,  1312,  -379,
    -379,   150,   408,  -379,  1592,  -379,   425,   274,    91,    91,
     196,   926,  1208,  1347,  1487,   341,   341,   196,   232,   232,
    -379,  -379,  -379,  1140,  -379,   343,   410,  1592,    -8,  1312,
    1312,  -379,   406,  -379,  -379,   172,    36,   439,   442,  -379,
    -379,  1140,  -379,  -379,  -379,  1140,  1592,   170,  -379,    59,
     444,   445,   446,  -379,   451,   455,   460,   461,  1382,  -379,
    -379,  -379,  -379,   402,   746,  -379,  -379,  -379,  -379,  -379,
     183,  -379,  -379,  1592,  -379,  -379,   456,   462,  -379,   194,
    1592,   470,  1592,  -379,    13,   386,  -379,   476,   477,  -379,
      19,  -379,   187,   478,   464,   457,  -379,  1679,  -379,  -379,
    1592,  1592,  1592,   818,  1030,   216,  -379,  -379,  -379,   217,
      36,  -379,  -379,  -379,  1701,   239,   293,  -379,  -379,  -379,
    1140,  -379,  1312,   458,  -379,  1592,  -379,   110,  -379,  -379,
    -379,  -379,  -379,  -379,  -379,    36,  -379,  -379,   127,   479,
     296,   302,   307,   518,    36,   535,  -379,  -379,   496,   509,
    -379,  -379,  -379,   512,   551,  -379,   406,   514,   519,  1592,
    -379,   187,  -379,  1312,   520,  1592,  -379,   140,   147,  -379,
    -379,   516,  -379,   818,  -379,  -379,   522,   406,  1592,  1452,
     216,  1592,   573,    36,  -379,  1679,   521,  -379,   524,  -379,
     158,  -379,  -379,  -379,   571,   523,   818,  1592,  -379,  1140,
     532,  -379,   998,  1592,  -379,   528,  1592,  -379,  -379,   818,
    -379,  -379,  -379,   312,  1522,  -379,  1140,  -379,  1140,  -379,
     212,   533,   534,   818,    32,   531,   602,  -379,  -379,  -379,
    -379,   596,   185,   605,   611,   185,   104,   497,   557,  -379,
     602,   546,   602,   818,  -379,   550,  -379,  -379,   213,   137,
    -379,   142,   142,   142,   142,   142,   142,   142,   142,   142,
     142,   142,   142,   142,   142,  -379,  -379,  -379,  -379,  -379,
    -379,  -379,   552,   142,   394,   394,   503,  1278,  1417,  1557,
     431,   431,   503,   240,   240,  -379,  -379,  -379,  -379
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       4,     0,    49,     1,    49,    21,     5,    23,    22,    54,
      10,    50,     6,     3,    35,     0,     0,    24,    38,    53,
      51,    11,     0,     0,    55,   230,     0,     0,   227,    36,
      37,     0,    15,    16,    57,    20,    10,     0,     0,     7,
     231,    31,     0,     0,     0,    56,   228,   229,    28,     0,
       9,     8,   207,   189,   190,     0,     0,     0,     0,     0,
       0,     0,     0,   186,     0,   216,     0,   135,     0,   222,
       0,   191,     0,     0,     0,     0,   169,   168,     0,     0,
       0,    58,     0,     0,   164,   205,   206,   165,   166,   167,
     203,   226,   188,     0,     0,    44,    57,     0,     0,     0,
       0,   241,     0,     0,   157,   185,   158,   159,   252,   175,
       0,   177,   173,     0,     0,   180,   182,     0,   133,   187,
       0,     0,     0,    63,   218,     0,     0,   237,   161,   160,
      35,     0,     0,    10,     0,   192,     0,   156,   136,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   162,   163,     0,     0,
       0,     0,     0,   235,   243,    34,    33,    43,    39,    10,
      47,     0,    45,    32,    29,   138,   253,    44,     0,   250,
     249,   176,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   214,   217,     0,     0,     0,   223,     0,     0,    36,
      37,     0,     0,   204,     0,   254,   141,   140,   149,   150,
     147,     0,   142,   143,   144,   145,   146,   148,   151,   152,
     153,   154,   155,   137,   209,     0,   194,     0,     0,     0,
       0,    40,    41,    18,    46,     0,    10,     0,     0,   213,
     178,   184,   212,   183,   211,   134,     0,     0,   219,     0,
       0,     0,     0,    87,     0,     0,     0,     0,     0,    82,
      64,    60,    81,     0,     0,    77,    78,    61,    62,    79,
       0,    80,    76,     0,   233,   233,     0,     0,    52,     0,
       0,     0,     0,   245,     0,     0,   202,     0,     0,    42,
       0,    48,     0,    26,     0,     0,   242,   224,   215,    74,
       0,     0,     0,     0,     0,     0,    83,    84,   170,     0,
      10,    59,    73,    75,     0,     0,     0,   238,   247,   193,
     139,   210,     0,     0,   201,     0,   199,     0,   236,   244,
      14,    13,    19,    12,    25,    10,    30,   208,     0,     0,
       0,     0,     0,     0,    10,     0,   101,   100,     0,    99,
      91,    93,    92,    94,     0,   171,     0,    70,     0,     0,
     239,     0,   240,     0,     0,     0,   200,     0,     0,   195,
      27,   224,   220,     0,   102,    85,     0,     0,     0,     0,
       0,     0,    66,    10,    65,   224,     0,   234,     0,   246,
       0,   196,   198,   225,   256,     0,     0,     0,    68,    69,
       0,    95,     0,     0,    71,     0,     0,   248,   197,     0,
     255,    63,    86,     0,     0,    96,    67,   221,   232,   257,
       0,     0,     0,     0,     0,     0,     0,    88,    89,    97,
     129,     0,     0,     0,     0,     0,     0,   112,   113,   111,
       0,     0,     0,     0,   131,     0,   130,   132,     0,     0,
     107,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   105,   103,   104,    90,   110,
     128,   109,     0,     0,   121,   122,   119,   114,   115,   116,
     117,   118,   120,   123,   124,   125,   126,   127,   108
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -379,  -379,  -379,  -379,  -379,  -379,  -379,    -6,  -379,    21,
    -379,  -379,  -379,  -379,  -379,   284,  -379,  -379,  -379,  -379,
    -222,  -379,    -7,  -289,   388,   450,  -379,   624,  -379,  -379,
    -379,   597,  -379,   -57,  -379,   226,  -379,  -379,   334,  -379,
     267,   389,  -181,  -379,  -379,  -379,  -379,  -379,  -379,   275,
    -379,  -379,  -379,  -340,  -379,  -379,  -379,   -56,   234,  -378,
    1330,    -4,   488,   -39,  -379,  -153,  -165,   498,  -379,  -379,
     471,   -51,  -379,   -49,  -379,  -379,   537,  -379,  -379,  -379,
    -379,  -379,  -379,  -379,  -323,  -379,   -14,   -15,  -379,   385,
    -379,  -379,  -379,  -379,  -379,  -379,  -379,   599,  -379,  -379,
    -379
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,    13,     5,    76,    77,   292,   332,    78,
       6,    43,   290,     7,   293,   294,    97,   236,     8,    48,
     167,    18,   168,   169,   170,   171,   172,     9,    10,    79,
      20,    23,    24,   261,   262,   194,   310,   357,   352,   346,
     358,   263,   440,   265,   396,   266,   303,   267,   443,   353,
     354,   268,   423,   347,   348,   269,   395,   441,   442,   436,
     437,   270,    80,   118,   271,   110,   111,   112,   114,   115,
     116,    82,    83,    84,   204,   100,   192,    85,   121,    86,
      87,    88,    89,   126,   339,    90,   438,    91,   387,   315,
      92,   229,   198,   178,   230,   322,   363,    93,    94,   272,
     410
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      28,    27,    19,    81,    22,   104,   106,   105,   105,   124,
     289,    28,    37,   264,    14,   344,   344,   285,   240,   107,
     330,   109,   113,    28,    47,   238,  -185,  -185,  -185,   138,
      44,   105,    25,   128,   129,    25,   430,    31,   323,   400,
     137,    42,   234,    34,   -15,   276,   -16,    38,   393,    99,
     161,   431,    32,    33,   445,    46,   131,   432,   117,   175,
       3,   188,   405,     4,   191,    96,   286,   133,    12,   191,
     134,   472,    98,   101,   422,  -251,   287,   288,  -207,   173,
     108,   189,   188,   264,   331,   235,   195,   324,   162,   123,
     174,   344,   433,   179,   -17,    26,   434,    21,   435,   134,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   240,   193,   223,
     176,   109,   343,   180,   299,    29,    30,   202,   201,   449,
      25,    -2,    14,    52,   382,    53,    54,    55,     4,   109,
      25,   430,    38,    35,   109,   241,   430,   113,    39,   245,
     151,   152,   153,   154,   155,   398,   431,   205,   228,   109,
     161,   431,    15,   232,    59,    60,    61,    62,    63,   364,
      65,    66,   450,   203,    16,   188,    70,    71,    14,   188,
     165,   166,   226,   248,   369,   188,   247,   260,    25,   430,
     109,   109,   394,   103,    17,    32,    33,   433,   162,    75,
     279,   434,   433,   435,   431,   471,   434,   297,   473,   188,
     388,    40,   141,   142,   391,   412,   188,   277,    14,   188,
     131,   392,    14,   284,   350,   351,    36,   188,   419,   451,
     452,   453,   408,   333,   314,   424,   425,   298,   337,   188,
     122,   320,   429,   109,   123,   433,   190,   -15,   313,   434,
     123,   435,   188,   182,   309,   151,   152,   153,   154,   155,
     131,   319,   468,   188,   131,   -16,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   156,   157,   158,
     470,   327,   355,   109,   199,   200,   188,   371,   139,   105,
     141,   142,   143,   153,   154,   155,   340,   341,   342,    41,
     349,   462,   463,   464,   356,    98,   360,    49,   361,    25,
      50,    51,    52,   334,    53,    54,    55,    56,    57,   367,
     385,   368,   159,   160,   109,    58,    95,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   377,   399,
     187,   188,   402,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    99,    70,    71,   141,   142,   143,
     362,   390,   361,   373,   416,   188,    72,   418,   102,   374,
      73,   188,    74,   136,   375,   349,   188,   356,    75,   421,
    -179,   188,   386,    42,   465,   120,   467,    49,   125,    25,
      50,    51,    52,   413,    53,    54,    55,    56,    57,   150,
     151,   152,   153,   154,   155,    58,   274,   275,   127,    28,
     349,   281,   282,   260,   165,   166,   135,    28,   163,   164,
      28,    37,   177,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    28,    70,    71,   183,   185,   197,
     186,   141,   142,   143,   325,   224,    72,   451,   452,   453,
      73,   233,    74,   460,   461,   462,   463,   464,    75,    49,
     326,    25,    50,    51,    52,    67,    53,    54,    55,    56,
      57,   239,   242,   311,   244,   278,   283,    58,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   459,
     460,   461,   462,   463,   464,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,   295,    70,    71,   296,
     300,   301,   302,   451,   452,   453,   365,   304,    72,   451,
     452,   305,    73,   317,    74,   306,   307,   123,   318,   336,
      75,    49,   366,    25,    50,    51,    52,   321,    53,    54,
      55,    56,    57,   328,   329,   376,   372,   335,   378,    58,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   379,   460,   461,   462,   463,   464,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,   188,    70,
      71,   380,   381,   383,   384,   338,   403,   389,   397,   406,
      72,   407,   409,   411,    73,   417,    74,   414,   427,   439,
     444,   428,    75,    49,  -181,    25,    50,    51,    52,   446,
      53,    54,    55,    56,    57,   447,    38,   466,   469,   370,
     488,    58,   250,   291,   251,   424,   425,   237,    11,   252,
     253,   254,   255,    45,   256,   257,   258,   420,   345,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
     404,    70,    71,   312,   426,   401,   231,   243,   225,     0,
     316,   119,    72,   196,     0,     0,    73,   259,    74,     0,
       0,     0,   123,  -106,    75,   249,     0,    25,    50,    51,
     130,     0,    53,    54,    55,    56,    57,     0,     0,     0,
       0,     0,     0,    58,   250,     0,   251,     0,     0,     0,
       0,   252,   253,   254,   255,     0,   256,   257,   258,     0,
       0,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,   131,    70,    71,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    72,     0,     0,     0,    73,   259,
      74,     0,     0,     0,   123,   -72,    75,   249,     0,    25,
      50,    51,    52,     0,    53,    54,    55,    56,    57,     0,
       0,     0,     0,     0,     0,    58,   250,     0,   251,     0,
       0,     0,     0,   252,   253,   254,   255,     0,   256,   257,
     258,     0,     0,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,     0,    70,    71,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    72,     0,     0,     0,
      73,   259,    74,     0,     0,     0,   123,   -72,    75,    49,
       0,    25,    50,    51,    52,     0,    53,    54,    55,    56,
      57,     0,     0,     0,     0,     0,     0,    58,   250,     0,
     251,     0,     0,     0,     0,   252,   253,   254,   255,     0,
     256,   257,   258,     0,     0,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,     0,    70,    71,     0,
     139,   140,   141,   142,   143,     0,     0,     0,    72,     0,
       0,     0,    73,   259,    74,     0,     0,     0,   123,    49,
      75,    25,    50,    51,   130,     0,    53,    54,    55,    56,
      57,     0,     0,     0,     0,     0,     0,    58,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
       0,     0,     0,     0,   184,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,   131,    70,    71,     0,
     139,   140,   141,   142,   143,     0,     0,     0,    72,   132,
       0,     0,    73,     0,    74,     0,     0,     0,     0,    49,
      75,    25,    50,    51,    52,     0,    53,    54,    55,    56,
      57,     0,     0,     0,     0,     0,     0,    58,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
       0,     0,     0,     0,   280,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,     0,    70,    71,     0,
       0,     0,   139,   140,   141,   142,   143,     0,    72,     0,
       0,     0,    73,     0,    74,  -174,     0,     0,     0,  -174,
      75,    49,     0,    25,    50,    51,   130,     0,    53,    54,
      55,    56,    57,     0,     0,     0,     0,     0,     0,    58,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,     0,     0,     0,   415,     0,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,   131,    70,
      71,     0,     0,     0,     0,   139,   140,   141,   142,   143,
      72,     0,     0,     0,    73,   -98,    74,   181,     0,     0,
       0,    49,    75,    25,    50,    51,    52,     0,    53,    54,
      55,    56,    57,     0,     0,     0,     0,     0,     0,    58,
       0,     0,     0,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,     0,     0,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,     0,    70,
      71,     0,     0,     0,   139,   140,   141,   142,   143,     0,
      72,     0,     0,     0,    73,     0,    74,     0,     0,     0,
       0,  -172,    75,    49,     0,    25,    50,    51,    52,     0,
      53,    54,    55,    56,    57,     0,     0,     0,     0,     0,
       0,    58,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,     0,     0,     0,     0,     0,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
       0,    70,    71,     0,   141,   142,   143,     0,     0,     0,
       0,     0,    72,   132,     0,     0,    73,     0,    74,     0,
       0,     0,     0,    49,    75,    25,    50,    51,    52,     0,
      53,    54,    55,    56,    57,     0,     0,     0,     0,     0,
       0,    58,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,     0,     0,     0,     0,     0,     0,     0,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
       0,    70,    71,     0,   451,   452,   453,     0,     0,     0,
     227,     0,    72,     0,     0,     0,    73,     0,    74,     0,
       0,     0,     0,    49,    75,    25,    50,    51,    52,     0,
      53,    54,    55,    56,    57,     0,     0,     0,     0,     0,
       0,    58,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,     0,     0,     0,     0,     0,     0,     0,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
       0,    70,    71,   141,   142,   143,     0,     0,     0,     0,
       0,     0,    72,     0,     0,     0,    73,     0,    74,  -172,
       0,     0,     0,    49,    75,    25,    50,    51,    52,     0,
      53,    54,    55,    56,    57,     0,     0,     0,     0,     0,
       0,    58,   147,   148,   149,   150,   151,   152,   153,   154,
     155,     0,     0,     0,     0,     0,     0,     0,     0,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
       0,    70,    71,   451,   452,   453,     0,     0,     0,     0,
       0,     0,    72,     0,     0,     0,    73,   308,    74,     0,
       0,     0,     0,    49,    75,    25,    50,    51,    52,     0,
      53,    54,    55,    56,    57,     0,     0,     0,     0,     0,
       0,    58,   456,   457,   458,   459,   460,   461,   462,   463,
     464,     0,     0,     0,     0,     0,     0,     0,     0,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
       0,    70,    71,   141,   142,   143,     0,     0,     0,     0,
       0,     0,    72,     0,     0,     0,    73,   -98,    74,     0,
       0,     0,     0,    49,    75,    25,    50,    51,    52,     0,
      53,    54,    55,    56,    57,     0,     0,     0,     0,     0,
       0,    58,     0,   148,   149,   150,   151,   152,   153,   154,
     155,     0,     0,     0,     0,     0,     0,     0,     0,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
       0,    70,    71,   451,   452,   453,     0,     0,     0,     0,
       0,     0,    72,     0,     0,     0,    73,     0,    74,   -98,
       0,     0,     0,    49,    75,    25,    50,    51,    52,     0,
      53,    54,    55,    56,    57,     0,     0,     0,     0,     0,
       0,    58,     0,   457,   458,   459,   460,   461,   462,   463,
     464,     0,     0,     0,     0,     0,     0,     0,     0,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
       0,    70,    71,   139,   140,   141,   142,   143,     0,     0,
       0,     0,    72,     0,     0,     0,    73,     0,    74,     0,
       0,     0,     0,     0,    75,   139,   140,   141,   142,   143,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   139,   140,   141,   142,   143,   246,     0,
       0,     0,     0,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   139,   140,   141,   142,   143,
     273,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,     0,     0,     0,     0,     0,   338,     0,
       0,     0,     0,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   448,     0,     0,     0,     0,
     359,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   474,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,     0,     0,     0,     0,     0,
       0,     0,     0,   448
};

static const yytype_int16 yycheck[] =
{
      15,    15,     9,    42,    10,    56,    57,    56,    57,    66,
     232,    26,    26,   194,     6,   304,   305,    25,   183,    58,
       1,    60,    61,    38,    38,   178,    11,    12,    13,    80,
      36,    80,     3,    72,    73,     3,     4,    16,    25,   379,
      79,    13,    26,    22,    37,   198,    37,    59,   371,    37,
      35,    19,     8,     9,   432,    67,    48,    25,    62,    98,
       0,    69,   385,     7,   121,    44,    74,    74,    65,   126,
      74,   449,    13,    66,   414,    66,   229,   230,    66,    71,
      59,   120,    69,   264,    65,    69,   125,    74,    73,    70,
      97,   380,    60,    49,    66,    66,    64,    61,    66,   103,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   282,   122,   158,
      99,   160,   303,   102,    65,     8,     9,   133,   132,    25,
       3,     0,     6,     6,   356,     8,     9,    10,     7,   178,
       3,     4,    59,    65,   183,   184,     4,   186,    65,   188,
      59,    60,    61,    62,    63,   377,    19,   136,   162,   198,
      35,    19,    36,   169,    37,    38,    39,    40,    41,   322,
      43,    44,    68,    67,    48,    69,    49,    50,     6,    69,
       8,     9,   161,    67,    74,    69,   190,   194,     3,     4,
     229,   230,   373,    66,    68,     8,     9,    60,    73,    72,
     204,    64,    60,    66,    19,    68,    64,   246,    66,    69,
     363,     3,    16,    17,    74,   396,    69,    67,     6,    69,
      48,    74,     6,   227,     8,     9,    69,    69,   409,    16,
      17,    18,    74,   290,   273,    23,    24,    67,   295,    69,
      66,   280,   423,   282,    70,    60,    66,    70,    65,    64,
      70,    66,    69,    71,   258,    59,    60,    61,    62,    63,
      48,    67,   443,    69,    48,    70,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    11,    12,    13,
      67,   285,    65,   322,     8,     9,    69,   338,    14,   338,
      16,    17,    18,    61,    62,    63,   300,   301,   302,    70,
     304,    61,    62,    63,   310,    13,    67,     1,    69,     3,
       4,     5,     6,   292,     8,     9,    10,    11,    12,   323,
     359,   325,    68,    69,   363,    19,    66,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,   344,   378,
      68,    69,   381,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    37,    49,    50,    16,    17,    18,
      67,   365,    69,    67,   403,    69,    60,   406,    37,    67,
      64,    69,    66,    37,    67,   379,    69,   383,    72,    67,
      74,    69,   361,    13,   440,    66,   442,     1,    66,     3,
       4,     5,     6,   397,     8,     9,    10,    11,    12,    58,
      59,    60,    61,    62,    63,    19,     8,     9,    66,   424,
     414,    68,    69,   420,     8,     9,    66,   432,    66,    66,
     435,   435,    66,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,   449,    49,    50,    69,    74,    48,
      69,    16,    17,    18,    58,    67,    60,    16,    17,    18,
      64,    67,    66,    59,    60,    61,    62,    63,    72,     1,
      74,     3,     4,     5,     6,    45,     8,     9,    10,    11,
      12,    67,    67,    71,    67,    67,    66,    19,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    58,
      59,    60,    61,    62,    63,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    67,    49,    50,    67,
      66,    66,    66,    16,    17,    18,    58,    66,    60,    16,
      17,    66,    64,    67,    66,    65,    65,    70,    66,    65,
      72,     1,    74,     3,     4,     5,     6,    67,     8,     9,
      10,    11,    12,    67,    67,    27,    67,    69,    13,    19,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    65,    59,    60,    61,    62,    63,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    69,    49,
      50,    69,    31,    69,    65,    69,    13,    67,    66,    68,
      60,    67,    21,    70,    64,    67,    66,    65,    65,    68,
       4,    67,    72,     1,    74,     3,     4,     5,     6,     4,
       8,     9,    10,    11,    12,     4,    59,    71,    68,   335,
      68,    19,    20,   235,    22,    23,    24,   177,     4,    27,
      28,    29,    30,    36,    32,    33,    34,   411,   304,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
     383,    49,    50,   264,   420,   380,   168,   186,   160,    -1,
     275,    62,    60,   126,    -1,    -1,    64,    65,    66,    -1,
      -1,    -1,    70,    71,    72,     1,    -1,     3,     4,     5,
       6,    -1,     8,     9,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    19,    20,    -1,    22,    -1,    -1,    -1,
      -1,    27,    28,    29,    30,    -1,    32,    33,    34,    -1,
      -1,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    60,    -1,    -1,    -1,    64,    65,
      66,    -1,    -1,    -1,    70,    71,    72,     1,    -1,     3,
       4,     5,     6,    -1,     8,     9,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    19,    20,    -1,    22,    -1,
      -1,    -1,    -1,    27,    28,    29,    30,    -1,    32,    33,
      34,    -1,    -1,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    60,    -1,    -1,    -1,
      64,    65,    66,    -1,    -1,    -1,    70,    71,    72,     1,
      -1,     3,     4,     5,     6,    -1,     8,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,    -1,
      22,    -1,    -1,    -1,    -1,    27,    28,    29,    30,    -1,
      32,    33,    34,    -1,    -1,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    50,    -1,
      14,    15,    16,    17,    18,    -1,    -1,    -1,    60,    -1,
      -1,    -1,    64,    65,    66,    -1,    -1,    -1,    70,     1,
      72,     3,     4,     5,     6,    -1,     8,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    19,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      -1,    -1,    -1,    -1,    68,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    -1,
      14,    15,    16,    17,    18,    -1,    -1,    -1,    60,    61,
      -1,    -1,    64,    -1,    66,    -1,    -1,    -1,    -1,     1,
      72,     3,     4,     5,     6,    -1,     8,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    19,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      -1,    -1,    -1,    -1,    68,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    50,    -1,
      -1,    -1,    14,    15,    16,    17,    18,    -1,    60,    -1,
      -1,    -1,    64,    -1,    66,    67,    -1,    -1,    -1,    71,
      72,     1,    -1,     3,     4,     5,     6,    -1,     8,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    19,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    -1,    -1,    -1,    67,    -1,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      60,    -1,    -1,    -1,    64,    65,    66,    26,    -1,    -1,
      -1,     1,    72,     3,     4,     5,     6,    -1,     8,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    19,
      -1,    -1,    -1,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    -1,    -1,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      50,    -1,    -1,    -1,    14,    15,    16,    17,    18,    -1,
      60,    -1,    -1,    -1,    64,    -1,    66,    -1,    -1,    -1,
      -1,    71,    72,     1,    -1,     3,     4,     5,     6,    -1,
       8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    -1,    -1,    -1,    -1,    -1,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    50,    -1,    16,    17,    18,    -1,    -1,    -1,
      -1,    -1,    60,    61,    -1,    -1,    64,    -1,    66,    -1,
      -1,    -1,    -1,     1,    72,     3,     4,     5,     6,    -1,
       8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    50,    -1,    16,    17,    18,    -1,    -1,    -1,
      58,    -1,    60,    -1,    -1,    -1,    64,    -1,    66,    -1,
      -1,    -1,    -1,     1,    72,     3,     4,     5,     6,    -1,
       8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    50,    16,    17,    18,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    -1,    -1,    -1,    64,    -1,    66,    67,
      -1,    -1,    -1,     1,    72,     3,     4,     5,     6,    -1,
       8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    50,    16,    17,    18,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    -1,    -1,    -1,    64,    65,    66,    -1,
      -1,    -1,    -1,     1,    72,     3,     4,     5,     6,    -1,
       8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    50,    16,    17,    18,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    -1,    -1,    -1,    64,    65,    66,    -1,
      -1,    -1,    -1,     1,    72,     3,     4,     5,     6,    -1,
       8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    50,    16,    17,    18,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    -1,    -1,    -1,    64,    -1,    66,    67,
      -1,    -1,    -1,     1,    72,     3,     4,     5,     6,    -1,
       8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    50,    14,    15,    16,    17,    18,    -1,    -1,
      -1,    -1,    60,    -1,    -1,    -1,    64,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    72,    14,    15,    16,    17,    18,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    14,    15,    16,    17,    18,    69,    -1,
      -1,    -1,    -1,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    14,    15,    16,    17,    18,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,   435,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   473
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    76,    77,     0,     7,    79,    85,    88,    93,   102,
     103,   102,    65,    78,     6,    36,    48,    68,    96,    97,
     105,    61,    82,   106,   107,     3,    66,   161,   162,     8,
       9,    84,     8,     9,    84,    65,    69,   161,    59,    65,
       3,    70,    13,    86,    82,   106,    67,   161,    94,     1,
       4,     5,     6,     8,     9,    10,    11,    12,    19,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      49,    50,    60,    64,    66,    72,    80,    81,    84,   104,
     137,   138,   146,   147,   148,   152,   154,   155,   156,   157,
     160,   162,   165,   172,   173,    66,    84,    91,    13,    37,
     150,    66,    37,    66,   146,   148,   146,   138,    84,   138,
     140,   141,   142,   138,   143,   144,   145,   136,   138,   172,
      66,   153,    66,    70,   108,    66,   158,    66,   138,   138,
       6,    48,    61,    97,   136,    66,    37,   138,   146,    14,
      15,    16,    17,    18,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    11,    12,    13,    68,
      69,    35,    73,    66,    66,     8,     9,    95,    97,    98,
      99,   100,   101,    71,    97,   138,    84,    66,   168,    49,
      84,    26,    71,    69,    68,    74,    69,    68,    69,   138,
      66,   108,   151,   136,   110,   138,   151,    48,   167,     8,
       9,   136,    82,    67,   149,    84,   138,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     138,   138,   138,   138,    67,   142,    84,    58,   136,   166,
     169,   137,    82,    67,    26,    69,    92,   100,   140,    67,
     141,   138,    67,   145,    67,   138,    69,   136,    67,     1,
      20,    22,    27,    28,    29,    30,    32,    33,    34,    65,
      97,   108,   109,   116,   117,   118,   120,   122,   126,   130,
     136,   139,   174,    69,     8,     9,   140,    67,    67,   136,
      68,    68,    69,    66,   136,    25,    74,   140,   140,    95,
      87,    99,    82,    89,    90,    67,    67,   138,    67,    65,
      66,    66,    66,   121,    66,    66,    65,    65,    65,   136,
     111,    71,   116,    65,   138,   164,   164,    67,    66,    67,
     138,    67,   170,    25,    74,    58,    74,   136,    67,    67,
       1,    65,    83,   108,    84,    69,    65,   108,    69,   159,
     136,   136,   136,   117,    98,   113,   114,   128,   129,   136,
       8,     9,   113,   124,   125,    65,    82,   112,   115,    69,
      67,    69,    67,   171,   140,    58,    74,   136,   136,    74,
      90,   146,    67,    67,    67,    67,    27,    82,    13,    65,
      69,    31,    95,    69,    65,   138,    84,   163,   140,    67,
     136,    74,    74,   159,   117,   131,   119,    66,    95,   138,
     128,   124,   138,    13,   115,   159,    68,    67,    74,    21,
     175,    70,   117,   136,    65,    67,   138,    67,   138,   117,
     110,    67,   128,   127,    23,    24,   133,    65,    67,   117,
       4,    19,    25,    60,    64,    66,   134,   135,   161,    68,
     117,   132,   133,   123,     4,   134,     4,     4,   135,    25,
      68,    16,    17,    18,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,   132,    71,   132,   117,    68,
      67,    68,   134,    66,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   135,    68
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    75,    76,    77,    77,    78,    78,    79,    80,    81,
      82,    82,    83,    83,    83,    84,    84,    86,    87,    85,
      85,    85,    85,    85,    88,    89,    90,    90,    91,    92,
      91,    94,    93,    95,    95,    96,    96,    96,    97,    98,
      98,    99,    99,    99,   100,   100,   100,   101,   101,   102,
     102,   103,   104,   105,   105,   106,   106,   107,   107,   108,
     109,   109,   109,   110,   111,   110,   112,   112,   113,   114,
     115,   115,   116,   116,   116,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   119,   118,   121,   120,   123,
     122,   124,   124,   124,   125,   125,   127,   126,   128,   128,
     129,   129,   131,   130,   132,   132,   132,   133,   133,   133,
     133,   133,   134,   134,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   136,   136,   137,   138,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     139,   139,   140,   140,   140,   141,   141,   142,   142,   143,
     143,   143,   144,   144,   145,   146,   147,   147,   148,   148,
     148,   148,   149,   148,   148,   148,   148,   148,   148,   148,
     148,   148,   148,   148,   148,   148,   148,   150,   148,   148,
     148,   148,   148,   148,   151,   151,   153,   152,   154,   154,
     155,   156,   158,   157,   159,   159,   160,   161,   161,   161,
     162,   162,   163,   164,   164,   166,   165,   167,   165,   165,
     165,   168,   165,   169,   165,   170,   165,   171,   165,   172,
     172,   173,   173,   173,   173,   174,   175,   175
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     3,     0,     0,     1,     4,     1,     1,
       0,     1,     1,     1,     1,     1,     1,     0,     0,     9,
       3,     1,     1,     1,     2,     2,     1,     3,     0,     0,
       5,     0,     7,     1,     1,     1,     2,     2,     1,     1,
       2,     2,     3,     1,     0,     1,     2,     1,     3,     0,
       2,     2,     4,     1,     0,     1,     3,     2,     4,     4,
       1,     1,     1,     0,     0,     5,     2,     4,     3,     3,
       1,     3,     0,     2,     2,     2,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     0,     6,     0,     8,     0,
      10,     1,     1,     1,     1,     3,     0,     8,     0,     1,
       1,     1,     0,    10,     2,     2,     0,     3,     5,     4,
       4,     2,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     1,
       2,     2,     2,     1,     3,     1,     2,     3,     3,     5,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       2,     2,     2,     2,     1,     1,     1,     1,     1,     1,
       2,     3,     0,     1,     2,     1,     2,     1,     3,     0,
       1,     2,     1,     3,     3,     1,     1,     2,     1,     1,
       1,     1,     0,     5,     3,     6,     7,     8,     7,     5,
       6,     5,     4,     1,     3,     1,     1,     0,     6,     3,
       5,     4,     4,     4,     1,     3,     0,     3,     2,     4,
       7,     9,     0,     3,     0,     3,     1,     1,     3,     3,
       1,     2,     3,     0,     3,     0,     5,     0,     5,     6,
       6,     0,     5,     0,     5,     0,     7,     0,     8,     3,
       3,     1,     2,     3,     3,     6,     0,     2
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



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

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
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
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
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


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
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
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
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
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
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
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
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
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

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

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
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
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
| yyreduce -- Do a reduction.  |
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
  switch (yyn)
    {
        case 2:
#line 208 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 224 "lpc/compiler/grammar.y.pre"
	    comp_trees[TREE_MAIN] = (yyval.node);
	}
#line 1995 "y.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 216 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 231 "lpc/compiler/grammar.y.pre"
	    CREATE_TWO_VALUES((yyval.node), 0, (yyvsp[-2].node), (yyvsp[-1].node));
	}
#line 2004 "y.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 221 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 235 "lpc/compiler/grammar.y.pre"
	    (yyval.node) = 0;
	}
#line 2013 "y.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 230 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 243 "lpc/compiler/grammar.y.pre"

		yywarn("Extra ';'. Ignored.");
	    }
#line 2023 "y.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 240 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 252 "lpc/compiler/grammar.y.pre"
		object_t *ob;
		inherit_t inherit;
		int initializer;
		int acc_mod;
		
		(yyvsp[-3].number) |= global_modifiers;

                acc_mod = ((yyvsp[-3].number) & DECL_ACCESS) & ~global_modifiers;
		if (acc_mod & (acc_mod - 1)) {
		    char buf[256];
		    char *end = EndOf(buf);
		    char *p;
		    
		    p = strput(buf, end, "Multiple access modifiers (");
		    p = get_type_modifiers(p, end, (yyvsp[-3].number));
		    p = strput(p, end, ") for inheritance");
		    yyerror(buf);
		}	       
		
		if (!((yyvsp[-3].number) & DECL_ACCESS)) (yyvsp[-3].number) |= DECL_PUBLIC;
		if (var_defined)
		    yyerror("Illegal to inherit after defining global variables.");
		if (var_defined){
		  inherit_file = 0;
		  YYACCEPT;
		}
		ob = find_object2((yyvsp[-1].string));
		if (ob == 0) {
		    inherit_file = alloc_cstring((yyvsp[-1].string), "inherit");
		    /* Return back to load_object() */
		    YYACCEPT;
		}
		scratch_free((yyvsp[-1].string));
		inherit.prog = ob->prog;

		if (mem_block[A_INHERITS].current_size){
		    inherit_t *prev_inherit = INHERIT(NUM_INHERITS - 1);
		    
		    inherit.function_index_offset 
			= prev_inherit->function_index_offset
			+ prev_inherit->prog->num_functions_defined
			+ prev_inherit->prog->last_inherited;
		    if (prev_inherit->prog->num_functions_defined &&
			prev_inherit->prog->function_table[prev_inherit->prog->num_functions_defined - 1].funcname[0] == APPLY___INIT_SPECIAL_CHAR)
			inherit.function_index_offset--;
		} else inherit.function_index_offset = 0;
		
		inherit.variable_index_offset =
		    mem_block[A_VAR_TEMP].current_size /
		    sizeof (variable_t);
		inherit.type_mod = (yyvsp[-3].number);
		add_to_mem_block(A_INHERITS, (char *)&inherit, sizeof inherit);

		/* The following has to come before copy_vars - Sym */
		copy_structures(ob->prog);
		copy_variables(ob->prog, (yyvsp[-3].number));
		initializer = copy_functions(ob->prog, (yyvsp[-3].number));
		if (initializer >= 0) {
		    parse_node_t *node, *newnode;
		    /* initializer is an index into the object we're
		       inheriting's function table; this finds the
		       appropriate entry in our table and generates
		       a call to it */
		    node = new_node_no_line();
		    node->kind = NODE_CALL_2;
		    node->r.expr = 0;
		    node->v.number = F_CALL_INHERITED;
		    node->l.number = initializer | ((NUM_INHERITS - 1) << 16);
		    node->type = TYPE_ANY;
		    
		    /* The following illustrates a distinction between */
		    /* macros and funcs...newnode is needed here - Sym */
		    newnode = comp_trees[TREE_INIT];
		    CREATE_TWO_VALUES(comp_trees[TREE_INIT],0, newnode, node);
		    comp_trees[TREE_INIT] = pop_value(comp_trees[TREE_INIT]);
		    
		} 
		(yyval.node) = 0;
	    }
#line 2109 "y.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 325 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 340 "lpc/compiler/grammar.y.pre"
		CREATE_REAL((yyval.node), (yyvsp[0].real));
	    }
#line 2118 "y.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 333 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 347 "lpc/compiler/grammar.y.pre"
		CREATE_NUMBER((yyval.node), (yyvsp[0].number));
	    }
#line 2127 "y.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 341 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 354 "lpc/compiler/grammar.y.pre"
		(yyval.number) = 0;
	    }
#line 2136 "y.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 346 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 358 "lpc/compiler/grammar.y.pre"
		(yyval.number) = TYPE_MOD_ARRAY;
	    }
#line 2145 "y.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 354 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 365 "lpc/compiler/grammar.y.pre"
		(yyval.node) = (yyvsp[0].decl).node;
		if (!(yyval.node)) {
		    CREATE_RETURN((yyval.node), 0);
		}
            }
#line 2157 "y.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 362 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 372 "lpc/compiler/grammar.y.pre"
		(yyval.node) = 0;
	    }
#line 2166 "y.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 367 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 376 "lpc/compiler/grammar.y.pre"
		(yyval.node) = 0;
	    }
#line 2175 "y.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 375 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 383 "lpc/compiler/grammar.y.pre"
		(yyval.string) = scratch_copy((yyvsp[0].ihe)->name);
	    }
#line 2184 "y.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 384 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 391 "lpc/compiler/grammar.y.pre"
		int flags;
                int acc_mod;
                func_present = 1;
		flags = ((yyvsp[-2].number) >> 16);
		
		flags |= global_modifiers;

                acc_mod = (flags & DECL_ACCESS) & ~global_modifiers;
		if (acc_mod & (acc_mod - 1)) {
		    char buf[256];
		    char *end = EndOf(buf);
		    char *p;
		    
		    p = strput(buf, end, "Multiple access modifiers (");
		    p = get_type_modifiers(p, end, flags);
		    p = strput(p, end, ") for function");
		    yyerror(buf);
		}	 

		if (!(flags & DECL_ACCESS)) flags |= DECL_PUBLIC;
		if (flags & DECL_NOSAVE) {
		    yywarn("Illegal to declare nosave function.");
		    flags &= ~DECL_NOSAVE;
		}
                (yyvsp[-2].number) = (flags << 16) | ((yyvsp[-2].number) & 0xffff);
		/* Handle type checking here so we know whether to typecheck
		   'argument' */
		if ((yyvsp[-2].number) & 0xffff) {
		    exact_types = ((yyvsp[-2].number)& 0xffff) | (yyvsp[-1].number);
		} else {
		    if (pragmas & PRAGMA_STRICT_TYPES) {
			if (strcmp((yyvsp[0].string), "create") != 0)
			    yyerror("\"#pragma strict_types\" requires type of function");
			else
			    exact_types = TYPE_VOID; /* default for create() */
		    } else
			exact_types = 0;
		}
	    }
#line 2230 "y.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 426 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 442 "lpc/compiler/grammar.y.pre"
		char *p = (yyvsp[-4].string);
		(yyvsp[-4].string) = make_shared_string((yyvsp[-4].string));
		scratch_free(p);

		/* If we had nested functions, we would need to check */
		/* here if we have enough space for locals */
		
		/*
		 * Define a prototype. If it is a real function, then the
		 * prototype will be replaced below.
		 */

		(yyval.number) = FUNC_PROTOTYPE;
		if ((yyvsp[-1].argument).flags & ARG_IS_VARARGS) {
		    (yyval.number) |= (FUNC_TRUE_VARARGS | FUNC_VARARGS);
		}
		(yyval.number) |= ((yyvsp[-6].number) >> 16);

		define_new_function((yyvsp[-4].string), (yyvsp[-1].argument).num_arg, 0, (yyval.number), ((yyvsp[-6].number) & 0xffff)| (yyvsp[-5].number));
		/* This is safe since it is guaranteed to be in the
		   function table, so it can't be dangling */
		free_string((yyvsp[-4].string)); 
		context = 0;
	    }
#line 2261 "y.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 453 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 468 "lpc/compiler/grammar.y.pre"
		/* Either a prototype or a block */
		if ((yyvsp[0].node)) {
		    int fun;

		    (yyvsp[-1].number) &= ~FUNC_PROTOTYPE;
		    if ((yyvsp[0].node)->kind != NODE_RETURN &&
			((yyvsp[0].node)->kind != NODE_TWO_VALUES
			 || (yyvsp[0].node)->r.expr->kind != NODE_RETURN)) {
			parse_node_t *replacement;
			CREATE_STATEMENTS(replacement, (yyvsp[0].node), 0);
			CREATE_RETURN(replacement->r.expr, 0);
			(yyvsp[0].node) = replacement;
		    }

		    fun = define_new_function((yyvsp[-6].string), (yyvsp[-3].argument).num_arg, 
					      max_num_locals - (yyvsp[-3].argument).num_arg,
					      (yyvsp[-1].number), ((yyvsp[-8].number) & 0xffff) | (yyvsp[-7].number));
		    if (fun != -1) {
			(yyval.node) = new_node_no_line();
			(yyval.node)->kind = NODE_FUNCTION;
			(yyval.node)->v.number = fun;
			(yyval.node)->l.number = max_num_locals;
			(yyval.node)->r.expr = (yyvsp[0].node);
		    } else 
			(yyval.node) = 0;
		} else
		    (yyval.node) = 0;
		free_all_local_names(!!(yyvsp[0].node));
	    }
#line 2297 "y.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 485 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 499 "lpc/compiler/grammar.y.pre"
		if (!((yyvsp[-2].number) & ~(DECL_MODS)) && (pragmas & PRAGMA_STRICT_TYPES))
		    yyerror("Missing type for global variable declaration");
		(yyval.node) = 0;
	    }
#line 2308 "y.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 497 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 510 "lpc/compiler/grammar.y.pre"
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
#line 2327 "y.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 515 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 527 "lpc/compiler/grammar.y.pre"
		/* At this point, the current_type here is only a basic_type */
		/* and cannot be unused yet - Sym */
		
		if (current_type == TYPE_VOID)
		    yyerror("Illegal to declare class member of type void.");
		add_local_name((yyvsp[0].string), current_type | (yyvsp[-1].number));
		scratch_free((yyvsp[0].string));
	    }
#line 2342 "y.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 535 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 546 "lpc/compiler/grammar.y.pre"
	      current_type = (yyvsp[0].number);
	  }
#line 2351 "y.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 544 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 554 "lpc/compiler/grammar.y.pre"
		ident_hash_elem_t *ihe;

		ihe = find_or_add_ident(
			   PROG_STRING((yyval.number) = store_prog_string((yyvsp[-1].string))),
			   FOA_GLOBAL_SCOPE);
		if (ihe->dn.class_num == -1) {
		    ihe->sem_value++;
		    ihe->dn.class_num = mem_block[A_CLASS_DEF].current_size / sizeof(class_def_t);
		    if (ihe->dn.class_num > CLASS_NUM_MASK){
			char buf[256];
			char *p;

			p = buf;
			sprintf(p, "Too many classes, max is %d.\n", CLASS_NUM_MASK + 1);
			yyerror(buf);
		    }

		    scratch_free((yyvsp[-1].string));
		    (yyvsp[-2].ihe) = 0;
		}
		else {
		    (yyvsp[-2].ihe) = ihe;
		}
	    }
#line 2382 "y.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 571 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 580 "lpc/compiler/grammar.y.pre"
		class_def_t *sd;
		class_member_entry_t *sme;
		int i, raise_error = 0;
		
		/* check for a redefinition */
		if ((yyvsp[-5].ihe) != 0) {
		    sd = CLASS((yyvsp[-5].ihe)->dn.class_num);
		    if (sd->size != current_number_of_locals)
			raise_error = 1;
		    else {
			i = sd->size;
			sme = (class_member_entry_t *)mem_block[A_CLASS_MEMBER].block + sd->index;
			while (i--) {
			    /* check for matching names and types */
			    if (strcmp(PROG_STRING(sme[i].membername), locals_ptr[i].ihe->name) != 0 ||
				sme[i].type != (type_of_locals_ptr[i] & ~LOCAL_MODS)) {
				raise_error = 1;
				break;
			    }
			}
		    }
		}

		if (raise_error) {
		    char buf[512];
		    char *end = EndOf(buf);
		    char *p;

		    p = strput(buf, end, "Illegal to redefine class ");
		    p = strput(p, end, PROG_STRING((yyval.number)));
		    yyerror(buf);
		} else {
		    sd = (class_def_t *)allocate_in_mem_block(A_CLASS_DEF, sizeof(class_def_t));
		    i = sd->size = current_number_of_locals;
		    sd->index = mem_block[A_CLASS_MEMBER].current_size / sizeof(class_member_entry_t);
		    sd->classname = (yyvsp[-2].number);

		    sme = (class_member_entry_t *)allocate_in_mem_block(A_CLASS_MEMBER, sizeof(class_member_entry_t) * current_number_of_locals);

		    while (i--) {
			sme[i].membername = store_prog_string(locals_ptr[i].ihe->name);
			sme[i].type = type_of_locals_ptr[i] & ~LOCAL_MODS;
		    }
		}

		free_all_local_names(0);
		(yyval.node) = 0;
	    }
#line 2437 "y.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 626 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 634 "lpc/compiler/grammar.y.pre"
		if ((yyvsp[0].ihe)->dn.local_num != -1) {
		    char buff[256];
		    char *end = EndOf(buff);
		    char *p;
		    
		    p = strput(buff, end, "Illegal to redeclare local name '");
		    p = strput(p, end, (yyvsp[0].ihe)->name);
		    p = strput(p, end, "'");
		    yyerror(buff);
		}
		(yyval.string) = scratch_copy((yyvsp[0].ihe)->name);
	    }
#line 2456 "y.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 645 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 652 "lpc/compiler/grammar.y.pre"
		if ((yyvsp[0].ihe)->dn.class_num == -1) {
		    char buf[256];
		    char *end = EndOf(buf);
		    char *p;
		    
		    p = strput(buf, end, "Undefined class '");
		    p = strput(p, end, (yyvsp[0].ihe)->name);
		    p = strput(p, end, "'");
		    yyerror(buf);
		    (yyval.number) = TYPE_ANY;
		} else {
		    (yyval.number) = (yyvsp[0].ihe)->dn.class_num | TYPE_MOD_CLASS;
		}
	    }
#line 2477 "y.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 662 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 668 "lpc/compiler/grammar.y.pre"
		char buf[256];
		char *end = EndOf(buf);
		char *p;

		p = strput(buf, end, "Undefined class '");
		p = strput(p, end, (yyvsp[0].string));
		p = strput(p, end, "'");
		yyerror(buf);
		(yyval.number) = TYPE_ANY;
	    }
#line 2494 "y.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 684 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 705 "lpc/compiler/grammar.y.pre"
		(yyval.number) = (yyvsp[-1].number) | LOCAL_MOD_REF;
	    }
#line 2503 "y.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 692 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 713 "lpc/compiler/grammar.y.pre"
                (yyval.number) = (yyvsp[-1].number) | (yyvsp[0].number);
		if ((yyvsp[-1].number) != TYPE_VOID)
		    add_local_name("", (yyvsp[-1].number) | (yyvsp[0].number));
            }
#line 2514 "y.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 699 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 719 "lpc/compiler/grammar.y.pre"
		if ((yyvsp[-2].number) == TYPE_VOID)
		    yyerror("Illegal to declare argument of type void.");
                add_local_name((yyvsp[0].string), (yyvsp[-2].number) | (yyvsp[-1].number));
		scratch_free((yyvsp[0].string));
                (yyval.number) = (yyvsp[-2].number) | (yyvsp[-1].number);
	    }
#line 2527 "y.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 708 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 727 "lpc/compiler/grammar.y.pre"
		if (exact_types) {
		    yyerror("Missing type for argument");
		}
		add_local_name((yyvsp[0].string), TYPE_ANY);
		scratch_free((yyvsp[0].string));
		(yyval.number) = TYPE_ANY;
            }
#line 2541 "y.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 721 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 739 "lpc/compiler/grammar.y.pre"
		(yyval.argument).num_arg = 0;
                (yyval.argument).flags = 0;
	    }
#line 2551 "y.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 728 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 745 "lpc/compiler/grammar.y.pre"
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
#line 2571 "y.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 747 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 763 "lpc/compiler/grammar.y.pre"
		if (((yyvsp[0].number) & TYPE_MASK) == TYPE_VOID && !((yyvsp[0].number) & TYPE_MOD_CLASS)) {
		    if ((yyvsp[0].number) & ~TYPE_MASK)
			yyerror("Illegal to declare argument of type void.");
		    (yyval.argument).num_arg = 0;
		} else {
		    (yyval.argument).num_arg = 1;
		}
                (yyval.argument).flags = 0;
	    }
#line 2587 "y.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 759 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 774 "lpc/compiler/grammar.y.pre"
		if (!(yyval.argument).num_arg)    /* first arg was void w/no name */
		    yyerror("argument of type void must be the only argument.");
		if (((yyvsp[0].number) & TYPE_MASK) == TYPE_VOID && !((yyvsp[0].number) & TYPE_MOD_CLASS))
		    yyerror("Illegal to declare argument of type void.");

                (yyval.argument) = (yyvsp[-2].argument);
		(yyval.argument).num_arg++;
	    }
#line 2602 "y.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 773 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 787 "lpc/compiler/grammar.y.pre"
		(yyval.number) = 0;
	    }
#line 2611 "y.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 778 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 791 "lpc/compiler/grammar.y.pre"
		int acc_mod;
		
		(yyval.number) = (yyvsp[-1].number) | (yyvsp[0].number);
		
                acc_mod = ((yyval.number) & DECL_ACCESS) & ~global_modifiers;
		if (acc_mod & (acc_mod - 1)) {
		    char buf[256];
		    char *end = EndOf(buf);
		    char *p;
		    
		    p = strput(buf, end, "Multiple access modifiers (");
		    p = get_type_modifiers(p, end, (yyval.number));
		    p = strput(p, end, ") ");
		    yyerror(buf);
		    (yyval.number) = DECL_PUBLIC;
		}
	    }
#line 2635 "y.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 801 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 817 "lpc/compiler/grammar.y.pre"
		(yyval.number) = ((yyvsp[-1].number) << 16) | (yyvsp[0].number);
		current_type = (yyval.number);
	    }
#line 2645 "y.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 810 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 825 "lpc/compiler/grammar.y.pre"
		(yyval.number) = (yyvsp[-2].number) | (yyvsp[-1].number);
	    }
#line 2654 "y.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 819 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 833 "lpc/compiler/grammar.y.pre"
		(yyval.number) = TYPE_UNKNOWN;
	    }
#line 2663 "y.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 832 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 845 "lpc/compiler/grammar.y.pre"
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
#line 2693 "y.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 858 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 870 "lpc/compiler/grammar.y.pre"
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
#line 2750 "y.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 914 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 925 "lpc/compiler/grammar.y.pre"
		if ((yyvsp[-2].decl).node && (yyvsp[-1].node)) {
		    CREATE_STATEMENTS((yyval.decl).node, (yyvsp[-2].decl).node, (yyvsp[-1].node));
		} else (yyval.decl).node = ((yyvsp[-2].decl).node ? (yyvsp[-2].decl).node : (yyvsp[-1].node));
                (yyval.decl).num = (yyvsp[-2].decl).num;
            }
#line 2762 "y.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 927 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 937 "lpc/compiler/grammar.y.pre"
                (yyval.decl).node = 0;
                (yyval.decl).num = 0;
            }
#line 2772 "y.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 933 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 942 "lpc/compiler/grammar.y.pre"
		if ((yyvsp[0].number) == TYPE_VOID)
		    yyerror("Illegal to declare local variable of type void.");
                /* can't do this in basic_type b/c local_name_list contains
                 * expr0 which contains cast which contains basic_type
                 */
                current_type = (yyvsp[0].number);
            }
#line 2786 "y.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 943 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 951 "lpc/compiler/grammar.y.pre"
                if ((yyvsp[-4].decl).node && (yyvsp[-1].decl).node) {
		    CREATE_STATEMENTS((yyval.decl).node, (yyvsp[-4].decl).node, (yyvsp[-1].decl).node);
                } else (yyval.decl).node = ((yyvsp[-4].decl).node ? (yyvsp[-4].decl).node : (yyvsp[-1].decl).node);
                (yyval.decl).num = (yyvsp[-4].decl).num + (yyvsp[-1].decl).num;
            }
#line 2798 "y.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 955 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 962 "lpc/compiler/grammar.y.pre"
		if (current_type & LOCAL_MOD_REF) {
		    yyerror("Illegal to declare local variable as reference");
		    current_type &= ~LOCAL_MOD_REF;
		}
		add_local_name((yyvsp[0].string), current_type | (yyvsp[-1].number) | LOCAL_MOD_UNUSED);

		scratch_free((yyvsp[0].string));
		(yyval.node) = 0;
	    }
#line 2814 "y.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 967 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 973 "lpc/compiler/grammar.y.pre"
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
#line 2850 "y.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 1002 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1007 "lpc/compiler/grammar.y.pre"
		if ((yyvsp[-2].number) == TYPE_VOID)
		    yyerror("Illegal to declare local variable of type void.");

		(yyval.number) = add_local_name((yyvsp[0].string), (yyvsp[-2].number) | (yyvsp[-1].number));
		scratch_free((yyvsp[0].string));
	    }
#line 2863 "y.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 1014 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1018 "lpc/compiler/grammar.y.pre"
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
#line 2897 "y.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 1047 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1050 "lpc/compiler/grammar.y.pre"
                (yyval.decl).node = (yyvsp[0].node);
                (yyval.decl).num = 1;
            }
#line 2907 "y.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 1053 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1055 "lpc/compiler/grammar.y.pre"
                if ((yyvsp[-2].node) && (yyvsp[0].decl).node) {
		    CREATE_STATEMENTS((yyval.decl).node, (yyvsp[-2].node), (yyvsp[0].decl).node);
                } else (yyval.decl).node = ((yyvsp[-2].node) ? (yyvsp[-2].node) : (yyvsp[0].decl).node);
                (yyval.decl).num = 1 + (yyvsp[0].decl).num;
            }
#line 2919 "y.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 1064 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1065 "lpc/compiler/grammar.y.pre"
		(yyval.node) = 0;
	    }
#line 2928 "y.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 1069 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1069 "lpc/compiler/grammar.y.pre"
		if ((yyvsp[-1].node) && (yyvsp[0].node)) {
		    CREATE_STATEMENTS((yyval.node), (yyvsp[-1].node), (yyvsp[0].node));
		} else (yyval.node) = ((yyvsp[-1].node) ? (yyvsp[-1].node) : (yyvsp[0].node));
            }
#line 2939 "y.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 1076 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1075 "lpc/compiler/grammar.y.pre"
		(yyval.node) = 0;
            }
#line 2948 "y.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 1084 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1082 "lpc/compiler/grammar.y.pre"
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
#line 2965 "y.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 1102 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1099 "lpc/compiler/grammar.y.pre"
                (yyval.node) = (yyvsp[0].decl).node;
                pop_n_locals((yyvsp[0].decl).num);
            }
#line 2975 "y.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 1108 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1104 "lpc/compiler/grammar.y.pre"
		(yyval.node) = 0;
	    }
#line 2984 "y.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 1113 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1108 "lpc/compiler/grammar.y.pre"
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
#line 3011 "y.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 1136 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1130 "lpc/compiler/grammar.y.pre"
		if (context & SPECIAL_CONTEXT)
		    yyerror("Cannot continue out of catch { } or time_expression { }");
		else
		if (!(context & LOOP_CONTEXT))
		    yyerror("continue statement outside loop");
		CREATE_CONTROL_JUMP((yyval.node), CJ_CONTINUE);
	    }
#line 3025 "y.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 1149 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1142 "lpc/compiler/grammar.y.pre"
		(yyvsp[-3].number) = context;
		context = LOOP_CONTEXT;
	    }
#line 3035 "y.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 1155 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1147 "lpc/compiler/grammar.y.pre"
		CREATE_LOOP((yyval.node), 1, (yyvsp[0].node), 0, optimize_loop_test((yyvsp[-3].node)));
		context = (yyvsp[-5].number);
	    }
#line 3045 "y.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 1164 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1155 "lpc/compiler/grammar.y.pre"
		(yyvsp[0].number) = context;
		context = LOOP_CONTEXT;
	    }
#line 3055 "y.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 1170 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1160 "lpc/compiler/grammar.y.pre"
		CREATE_LOOP((yyval.node), 0, (yyvsp[-5].node), 0, optimize_loop_test((yyvsp[-2].node)));
		context = (yyvsp[-7].number);
	    }
#line 3065 "y.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 1179 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1168 "lpc/compiler/grammar.y.pre"
		(yyvsp[-5].decl).node = pop_value((yyvsp[-5].decl).node);
		(yyvsp[-7].number) = context;
		context = LOOP_CONTEXT;
	    }
#line 3076 "y.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 1186 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1174 "lpc/compiler/grammar.y.pre"
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
#line 3097 "y.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 1205 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1192 "lpc/compiler/grammar.y.pre"
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
#line 3123 "y.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 1227 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1213 "lpc/compiler/grammar.y.pre"
		if (type_of_locals_ptr[(yyvsp[0].number)] & LOCAL_MOD_REF) {
		    CREATE_OPCODE_1((yyval.decl).node, F_REF_LVALUE, 0, (yyvsp[0].number));
		} else {
		    CREATE_OPCODE_1((yyval.decl).node, F_LOCAL_LVALUE, 0, (yyvsp[0].number));
		    type_of_locals_ptr[(yyvsp[0].number)] &= ~LOCAL_MOD_UNUSED;
		}
		(yyval.decl).num = 1;
            }
#line 3138 "y.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 1238 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1223 "lpc/compiler/grammar.y.pre"
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
#line 3157 "y.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 1256 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1240 "lpc/compiler/grammar.y.pre"
		CREATE_FOREACH((yyval.decl).node, (yyvsp[0].decl).node, 0);
		(yyval.decl).num = (yyvsp[0].decl).num;
            }
#line 3167 "y.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 1262 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1245 "lpc/compiler/grammar.y.pre"
		CREATE_FOREACH((yyval.decl).node, (yyvsp[-2].decl).node, (yyvsp[0].decl).node);
		(yyval.decl).num = (yyvsp[-2].decl).num + (yyvsp[0].decl).num;
		if ((yyvsp[-2].decl).node->v.number == F_REF_LVALUE)
		    yyerror("Mapping key may not be a reference in foreach()");
            }
#line 3179 "y.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 1273 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1255 "lpc/compiler/grammar.y.pre"
		(yyvsp[-3].decl).node->v.expr = (yyvsp[-1].node);
		(yyvsp[-5].number) = context;
		context = LOOP_CONTEXT | LOOP_FOREACH;
            }
#line 3190 "y.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 1280 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1261 "lpc/compiler/grammar.y.pre"
		(yyval.decl).num = (yyvsp[-5].decl).num;

		CREATE_STATEMENTS((yyval.decl).node, (yyvsp[-5].decl).node, 0);
		CREATE_LOOP((yyval.decl).node->r.expr, 2, (yyvsp[0].node), 0, 0);
		CREATE_OPCODE((yyval.decl).node->r.expr->r.expr, F_NEXT_FOREACH, 0);
		
		context = (yyvsp[-7].number);
	    }
#line 3205 "y.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 1294 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1274 "lpc/compiler/grammar.y.pre"
		(yyval.node) = 0;
	    }
#line 3214 "y.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 1303 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1282 "lpc/compiler/grammar.y.pre"
	 	(yyval.decl).node = (yyvsp[0].node);
		(yyval.decl).num = 0;
	    }
#line 3224 "y.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 1309 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1287 "lpc/compiler/grammar.y.pre"
		(yyval.decl).node = (yyvsp[0].node);
		(yyval.decl).num = 1;
	    }
#line 3234 "y.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 1318 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1295 "lpc/compiler/grammar.y.pre"
                (yyvsp[-3].number) = context;
                context &= LOOP_CONTEXT;
                context |= SWITCH_CONTEXT;
                (yyvsp[-2].number) = mem_block[A_CASES].current_size;
            }
#line 3246 "y.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 1326 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1302 "lpc/compiler/grammar.y.pre"
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
#line 3280 "y.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 1359 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1334 "lpc/compiler/grammar.y.pre"
               if ((yyvsp[0].node)){
		   CREATE_STATEMENTS((yyval.node), (yyvsp[-1].node), (yyvsp[0].node));
               } else (yyval.node) = (yyvsp[-1].node);
           }
#line 3291 "y.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 1366 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1340 "lpc/compiler/grammar.y.pre"
               if ((yyvsp[0].node)){
		   CREATE_STATEMENTS((yyval.node), (yyvsp[-1].node), (yyvsp[0].node));
               } else (yyval.node) = (yyvsp[-1].node);
           }
#line 3302 "y.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 1373 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1346 "lpc/compiler/grammar.y.pre"
               (yyval.node) = 0;
           }
#line 3311 "y.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 1382 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1354 "lpc/compiler/grammar.y.pre"
                (yyval.node) = (yyvsp[-1].node);
                (yyval.node)->v.expr = 0;

                add_to_mem_block(A_CASES, (char *)&((yyvsp[-1].node)), sizeof((yyvsp[-1].node)));
            }
#line 3323 "y.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 1390 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1361 "lpc/compiler/grammar.y.pre"
                if ( (yyvsp[-3].node)->kind != NODE_CASE_NUMBER
                    || (yyvsp[-1].node)->kind != NODE_CASE_NUMBER )
                    yyerror("String case labels not allowed as range bounds");
                if ((yyvsp[-3].node)->r.number > (yyvsp[-1].node)->r.number) break;

		context |= SWITCH_RANGES;

                (yyval.node) = (yyvsp[-3].node);
                (yyval.node)->v.expr = (yyvsp[-1].node);

                add_to_mem_block(A_CASES, (char *)&((yyvsp[-3].node)), sizeof((yyvsp[-3].node)));
            }
#line 3342 "y.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 1405 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1375 "lpc/compiler/grammar.y.pre"
	        if ( (yyvsp[-2].node)->kind != NODE_CASE_NUMBER )
                    yyerror("String case labels not allowed as range bounds");

		context |= SWITCH_RANGES;

                (yyval.node) = (yyvsp[-2].node);
                (yyval.node)->v.expr = new_node();
		(yyval.node)->v.expr->kind = NODE_CASE_NUMBER;
		(yyval.node)->v.expr->r.number = LPC_INT_MAX; //maxint

                add_to_mem_block(A_CASES, (char *)&((yyvsp[-2].node)), sizeof((yyvsp[-2].node)));
            }
#line 3361 "y.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 1420 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1389 "lpc/compiler/grammar.y.pre"
	      if ( (yyvsp[-1].node)->kind != NODE_CASE_NUMBER )
                    yyerror("String case labels not allowed as range bounds");

		context |= SWITCH_RANGES;
		(yyval.node) = new_node();
		(yyval.node)->kind = NODE_CASE_NUMBER;
                (yyval.node)->r.number = LPC_INT_MIN; //maxint +1 wraps to min_int, on all computers i know, just not in the C standard iirc 
                (yyval.node)->v.expr = (yyvsp[-1].node);

                add_to_mem_block(A_CASES, (char *)&((yyval.node)), sizeof((yyval.node)));
            }
#line 3379 "y.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 1434 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1402 "lpc/compiler/grammar.y.pre"
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
#line 3397 "y.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 1451 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1418 "lpc/compiler/grammar.y.pre"
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
#line 3416 "y.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 1466 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1432 "lpc/compiler/grammar.y.pre"
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
#line 3433 "y.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 1482 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1447 "lpc/compiler/grammar.y.pre"
                (yyval.number) = (yyvsp[-2].number) | (yyvsp[0].number);
            }
#line 3442 "y.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 1487 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1451 "lpc/compiler/grammar.y.pre"
                (yyval.number) = (yyvsp[-2].number) ^ (yyvsp[0].number);
            }
#line 3451 "y.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 1492 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1455 "lpc/compiler/grammar.y.pre"
                (yyval.number) = (yyvsp[-2].number) & (yyvsp[0].number);
            }
#line 3460 "y.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 1497 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1459 "lpc/compiler/grammar.y.pre"
                (yyval.number) = (yyvsp[-2].number) == (yyvsp[0].number);
            }
#line 3469 "y.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 1502 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1463 "lpc/compiler/grammar.y.pre"
                (yyval.number) = (yyvsp[-2].number) != (yyvsp[0].number);
            }
#line 3478 "y.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 1507 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1467 "lpc/compiler/grammar.y.pre"
                switch((yyvsp[-1].number)){
                    case F_GE: (yyval.number) = (yyvsp[-2].number) >= (yyvsp[0].number); break;
                    case F_LE: (yyval.number) = (yyvsp[-2].number) <= (yyvsp[0].number); break;
                    case F_GT: (yyval.number) = (yyvsp[-2].number) >  (yyvsp[0].number); break;
                }
            }
#line 3491 "y.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 1516 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1475 "lpc/compiler/grammar.y.pre"
                (yyval.number) = (yyvsp[-2].number) < (yyvsp[0].number);
            }
#line 3500 "y.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 1521 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1479 "lpc/compiler/grammar.y.pre"
                (yyval.number) = (yyvsp[-2].number) << (yyvsp[0].number);
            }
#line 3509 "y.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 1526 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1483 "lpc/compiler/grammar.y.pre"
                (yyval.number) = (yyvsp[-2].number) >> (yyvsp[0].number);
            }
#line 3518 "y.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 1531 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1487 "lpc/compiler/grammar.y.pre"
                (yyval.number) = (yyvsp[-2].number) + (yyvsp[0].number);
            }
#line 3527 "y.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 1536 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1491 "lpc/compiler/grammar.y.pre"
                (yyval.number) = (yyvsp[-2].number) - (yyvsp[0].number);
            }
#line 3536 "y.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 1541 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1495 "lpc/compiler/grammar.y.pre"
                (yyval.number) = (yyvsp[-2].number) * (yyvsp[0].number);
            }
#line 3545 "y.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 1546 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1499 "lpc/compiler/grammar.y.pre"
                if ((yyvsp[0].number)) (yyval.number) = (yyvsp[-2].number) % (yyvsp[0].number); else yyerror("Modulo by zero");
            }
#line 3554 "y.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 1551 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1503 "lpc/compiler/grammar.y.pre"
                if ((yyvsp[0].number)) (yyval.number) = (yyvsp[-2].number) / (yyvsp[0].number); else yyerror("Division by zero");
            }
#line 3563 "y.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 1556 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1507 "lpc/compiler/grammar.y.pre"
                (yyval.number) = (yyvsp[-1].number);
            }
#line 3572 "y.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 1561 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1511 "lpc/compiler/grammar.y.pre"
		(yyval.number) = (yyvsp[0].number);
	    }
#line 3581 "y.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 1566 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1515 "lpc/compiler/grammar.y.pre"
                (yyval.number) = -(yyvsp[0].number);
            }
#line 3590 "y.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 1571 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1519 "lpc/compiler/grammar.y.pre"
                (yyval.number) = !(yyvsp[0].number);
            }
#line 3599 "y.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 1576 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1523 "lpc/compiler/grammar.y.pre"
                (yyval.number) = ~(yyvsp[0].number);
            }
#line 3608 "y.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 1584 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1530 "lpc/compiler/grammar.y.pre"
		(yyval.node) = (yyvsp[0].node);
	    }
#line 3617 "y.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 1589 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1534 "lpc/compiler/grammar.y.pre"
		CREATE_TWO_VALUES((yyval.node), (yyvsp[0].node)->type, pop_value((yyvsp[-2].node)), (yyvsp[0].node));
	    }
#line 3626 "y.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 1601 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1551 "lpc/compiler/grammar.y.pre"
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
#line 3658 "y.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 1630 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1580 "lpc/compiler/grammar.y.pre"
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
#line 3688 "y.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 1656 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1605 "lpc/compiler/grammar.y.pre"
		yyerror("Illegal LHS");
		CREATE_ERROR((yyval.node));
	    }
#line 3698 "y.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 1662 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1610 "lpc/compiler/grammar.y.pre"
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
#line 3727 "y.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 1687 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1634 "lpc/compiler/grammar.y.pre"
		CREATE_LAND_LOR((yyval.node), F_LOR, (yyvsp[-2].node), (yyvsp[0].node));
		if (IS_NODE((yyvsp[-2].node), NODE_LAND_LOR, F_LOR))
		    (yyvsp[-2].node)->kind = NODE_BRANCH_LINK;
	    }
#line 3738 "y.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1694 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1640 "lpc/compiler/grammar.y.pre"
		CREATE_LAND_LOR((yyval.node), F_LAND, (yyvsp[-2].node), (yyvsp[0].node));
		if (IS_NODE((yyvsp[-2].node), NODE_LAND_LOR, F_LAND))
		    (yyvsp[-2].node)->kind = NODE_BRANCH_LINK;
	    }
#line 3749 "y.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1701 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1646 "lpc/compiler/grammar.y.pre"
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
#line 3779 "y.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1727 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1671 "lpc/compiler/grammar.y.pre"
		(yyval.node) = binary_int_op((yyvsp[-2].node), (yyvsp[0].node), F_XOR, "^");
	    }
#line 3788 "y.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1732 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1675 "lpc/compiler/grammar.y.pre"
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
#line 3816 "y.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1756 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1698 "lpc/compiler/grammar.y.pre"
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
#line 3843 "y.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1779 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1720 "lpc/compiler/grammar.y.pre"
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
#line 3862 "y.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1794 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1734 "lpc/compiler/grammar.y.pre"
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
#line 3911 "y.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1839 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1778 "lpc/compiler/grammar.y.pre"
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
#line 3953 "y.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1877 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1815 "lpc/compiler/grammar.y.pre"
		(yyval.node) = binary_int_op((yyvsp[-2].node), (yyvsp[0].node), F_LSH, "<<");
	    }
#line 3962 "y.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1882 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1819 "lpc/compiler/grammar.y.pre"
		(yyval.node) = binary_int_op((yyvsp[-2].node), (yyvsp[0].node), F_RSH, ">>");
	    }
#line 3971 "y.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1887 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1823 "lpc/compiler/grammar.y.pre"
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
#line 4146 "y.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 2058 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 1993 "lpc/compiler/grammar.y.pre"
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
#line 4245 "y.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 2153 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2087 "lpc/compiler/grammar.y.pre"
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
#line 4330 "y.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 2234 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2167 "lpc/compiler/grammar.y.pre"
		(yyval.node) = binary_int_op((yyvsp[-2].node), (yyvsp[0].node), F_MOD, "%");
	    }
#line 4339 "y.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 2239 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2171 "lpc/compiler/grammar.y.pre"
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
#line 4439 "y.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 2335 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2266 "lpc/compiler/grammar.y.pre"
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
#line 4465 "y.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 2357 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2287 "lpc/compiler/grammar.y.pre"
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
#line 4491 "y.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 2379 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2308 "lpc/compiler/grammar.y.pre"
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
#line 4518 "y.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 2402 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2330 "lpc/compiler/grammar.y.pre"
		if ((yyvsp[0].node)->kind == NODE_NUMBER) {
		    (yyval.node) = (yyvsp[0].node);
		    (yyval.node)->v.number = !((yyval.node)->v.number);
		} else {
		    CREATE_UNARY_OP((yyval.node), F_NOT, TYPE_NUMBER, (yyvsp[0].node));
		}
	    }
#line 4532 "y.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 2412 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2339 "lpc/compiler/grammar.y.pre"
		if (exact_types && !IS_TYPE((yyvsp[0].node)->type, TYPE_NUMBER))
		    type_error("Bad argument to ~", (yyvsp[0].node)->type);
		if ((yyvsp[0].node)->kind == NODE_NUMBER) {
		    (yyval.node) = (yyvsp[0].node);
		    (yyval.node)->v.number = ~(yyval.node)->v.number;
		} else {
		    CREATE_UNARY_OP((yyval.node), F_COMPL, TYPE_NUMBER, (yyvsp[0].node));
		}
	    }
#line 4548 "y.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 2424 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2350 "lpc/compiler/grammar.y.pre"
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
#line 4577 "y.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 2449 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2374 "lpc/compiler/grammar.y.pre"
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
#line 4604 "y.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 2472 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2396 "lpc/compiler/grammar.y.pre"
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
#line 4630 "y.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 2503 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2426 "lpc/compiler/grammar.y.pre"
		if (exact_types && !IS_TYPE(exact_types, TYPE_VOID))
		    yywarn("Non-void functions must return a value.");
		CREATE_RETURN((yyval.node), 0);
	    }
#line 4641 "y.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 2510 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2432 "lpc/compiler/grammar.y.pre"
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
#line 4663 "y.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 2531 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2452 "lpc/compiler/grammar.y.pre"
		CREATE_EXPR_LIST((yyval.node), 0);
	    }
#line 4672 "y.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 2536 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2456 "lpc/compiler/grammar.y.pre"
		CREATE_EXPR_LIST((yyval.node), (yyvsp[0].node));
	    }
#line 4681 "y.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 2541 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2460 "lpc/compiler/grammar.y.pre"
		CREATE_EXPR_LIST((yyval.node), (yyvsp[-1].node));
	    }
#line 4690 "y.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 2549 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2467 "lpc/compiler/grammar.y.pre"
		CREATE_EXPR_NODE((yyval.node), (yyvsp[0].node), 0);
	    }
#line 4699 "y.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 2554 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2471 "lpc/compiler/grammar.y.pre"
		CREATE_EXPR_NODE((yyval.node), (yyvsp[-1].node), 1);
	    }
#line 4708 "y.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 2562 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2478 "lpc/compiler/grammar.y.pre"
		(yyvsp[0].node)->kind = 1;

		(yyval.node) = (yyvsp[0].node);
	    }
#line 4719 "y.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 2569 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2484 "lpc/compiler/grammar.y.pre"
		(yyvsp[0].node)->kind = 0;

		(yyval.node) = (yyvsp[-2].node);
		(yyval.node)->kind++;
		(yyval.node)->l.expr->r.expr = (yyvsp[0].node);
		(yyval.node)->l.expr = (yyvsp[0].node);
	    }
#line 4733 "y.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 2582 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2496 "lpc/compiler/grammar.y.pre"
		/* this is a dummy node */
		CREATE_EXPR_LIST((yyval.node), 0);
	    }
#line 4743 "y.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 2588 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2501 "lpc/compiler/grammar.y.pre"
		CREATE_EXPR_LIST((yyval.node), (yyvsp[0].node));
	    }
#line 4752 "y.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 2593 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2505 "lpc/compiler/grammar.y.pre"
		CREATE_EXPR_LIST((yyval.node), (yyvsp[-1].node));
	    }
#line 4761 "y.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 2601 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2512 "lpc/compiler/grammar.y.pre"
		(yyval.node) = new_node_no_line();
		(yyval.node)->kind = 2;
		(yyval.node)->v.expr = (yyvsp[0].node);
		(yyval.node)->r.expr = 0;
		(yyval.node)->type = 0;
		/* we keep track of the end of the chain in the left nodes */
		(yyval.node)->l.expr = (yyval.node);
            }
#line 4776 "y.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 2612 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2522 "lpc/compiler/grammar.y.pre"
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
#line 4796 "y.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 2631 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2540 "lpc/compiler/grammar.y.pre"
		CREATE_TWO_VALUES((yyval.node), 0, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 4805 "y.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 2639 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2547 "lpc/compiler/grammar.y.pre"
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
#line 4923 "y.tab.c" /* yacc.c:1646  */
    break;

  case 187:
#line 2756 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2663 "lpc/compiler/grammar.y.pre"
		(yyval.number) = ((yyvsp[0].number) << 8) | FP_EFUN;
	    }
#line 4932 "y.tab.c" /* yacc.c:1646  */
    break;

  case 189:
#line 2766 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2695 "lpc/compiler/grammar.y.pre"
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
		      
		      p = strput(buf, end, "Undefined variable '");
		      p = strput(p, end, (yyvsp[0].ihe)->name);
		      p = strput(p, end, "'");
		      if (current_number_of_locals < CFG_MAX_LOCAL_VARIABLES) {
			  add_local_name((yyvsp[0].ihe)->name, TYPE_ANY);
		      }
		      CREATE_ERROR((yyval.node));
		      yyerror(buf);
		  }
	    }
#line 4979 "y.tab.c" /* yacc.c:1646  */
    break;

  case 190:
#line 2809 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2737 "lpc/compiler/grammar.y.pre"
		char buf[256];
		char *end = EndOf(buf);
		char *p;
		
		p = strput(buf, end, "Undefined variable '");
		p = strput(p, end, (yyvsp[0].string));
		p = strput(p, end, "'");
                if (current_number_of_locals < CFG_MAX_LOCAL_VARIABLES) {
                    add_local_name((yyvsp[0].string), TYPE_ANY);
                }
                CREATE_ERROR((yyval.node));
                yyerror(buf);
                scratch_free((yyvsp[0].string));
            }
#line 5000 "y.tab.c" /* yacc.c:1646  */
    break;

  case 191:
#line 2826 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2753 "lpc/compiler/grammar.y.pre"
		CREATE_PARAMETER((yyval.node), TYPE_ANY, (yyvsp[0].number));
            }
#line 5009 "y.tab.c" /* yacc.c:1646  */
    break;

  case 192:
#line 2831 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2757 "lpc/compiler/grammar.y.pre"
		(yyval.contextp) = current_function_context;
		/* already flagged as an error */
		if (current_function_context)
		    current_function_context = current_function_context->parent;
            }
#line 5021 "y.tab.c" /* yacc.c:1646  */
    break;

  case 193:
#line 2839 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2764 "lpc/compiler/grammar.y.pre"
		parse_node_t *node;

		current_function_context = (yyvsp[-2].contextp);

		if (!current_function_context || current_function_context->num_parameters == -2) {
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
#line 5049 "y.tab.c" /* yacc.c:1646  */
    break;

  case 194:
#line 2863 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2787 "lpc/compiler/grammar.y.pre"
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
#line 5078 "y.tab.c" /* yacc.c:1646  */
    break;

  case 195:
#line 2888 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2811 "lpc/compiler/grammar.y.pre"
                (yyval.node) = make_range_node(F_NN_RANGE, (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[-1].node));
            }
#line 5087 "y.tab.c" /* yacc.c:1646  */
    break;

  case 196:
#line 2893 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2822 "lpc/compiler/grammar.y.pre"
                (yyval.node) = make_range_node(F_RN_RANGE, (yyvsp[-6].node), (yyvsp[-3].node), (yyvsp[-1].node));
            }
#line 5096 "y.tab.c" /* yacc.c:1646  */
    break;

  case 197:
#line 2898 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2826 "lpc/compiler/grammar.y.pre"
		if ((yyvsp[-1].node)->kind == NODE_NUMBER && (yyvsp[-1].node)->v.number <= 1)
		    (yyval.node) = make_range_node(F_RE_RANGE, (yyvsp[-7].node), (yyvsp[-4].node), 0);
		else
		    (yyval.node) = make_range_node(F_RR_RANGE, (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-1].node));
            }
#line 5108 "y.tab.c" /* yacc.c:1646  */
    break;

  case 198:
#line 2906 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2833 "lpc/compiler/grammar.y.pre"
		if ((yyvsp[-1].node)->kind == NODE_NUMBER && (yyvsp[-1].node)->v.number <= 1)
		    (yyval.node) = make_range_node(F_NE_RANGE, (yyvsp[-6].node), (yyvsp[-4].node), 0);
		else
		    (yyval.node) = make_range_node(F_NR_RANGE, (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-1].node));
            }
#line 5120 "y.tab.c" /* yacc.c:1646  */
    break;

  case 199:
#line 2914 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2840 "lpc/compiler/grammar.y.pre"
                (yyval.node) = make_range_node(F_NE_RANGE, (yyvsp[-4].node), (yyvsp[-2].node), 0);
            }
#line 5129 "y.tab.c" /* yacc.c:1646  */
    break;

  case 200:
#line 2919 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2844 "lpc/compiler/grammar.y.pre"
                (yyval.node) = make_range_node(F_RE_RANGE, (yyvsp[-5].node), (yyvsp[-2].node), 0);
            }
#line 5138 "y.tab.c" /* yacc.c:1646  */
    break;

  case 201:
#line 2924 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2848 "lpc/compiler/grammar.y.pre"
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
#line 5188 "y.tab.c" /* yacc.c:1646  */
    break;

  case 202:
#line 2970 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2893 "lpc/compiler/grammar.y.pre"
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
                if ((yyvsp[-1].node)->kind == NODE_NUMBER && (yyvsp[-1].node)->v.number < 0)
		    yywarn("A negative constant in arr[x] no longer means indexing from the end.  Use arr[<x]");
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
#line 5247 "y.tab.c" /* yacc.c:1646  */
    break;

  case 204:
#line 3026 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2950 "lpc/compiler/grammar.y.pre"
		(yyval.node) = (yyvsp[-1].node);
	    }
#line 5256 "y.tab.c" /* yacc.c:1646  */
    break;

  case 207:
#line 3033 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2958 "lpc/compiler/grammar.y.pre"
	        if ((yyvsp[0].number) != TYPE_FUNCTION) yyerror("Reserved type name unexpected.");
		(yyval.func_block).num_local = current_number_of_locals;
		(yyval.func_block).max_num_locals = max_num_locals;
		(yyval.func_block).context = context;
		(yyval.func_block).save_current_type = current_type;
		(yyval.func_block).save_exact_types = exact_types;
	        if (type_of_locals_ptr + max_num_locals + CFG_MAX_LOCAL_VARIABLES >= &type_of_locals[type_of_locals_size])
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
#line 5280 "y.tab.c" /* yacc.c:1646  */
    break;

  case 208:
#line 3053 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 2977 "lpc/compiler/grammar.y.pre"
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
#line 5321 "y.tab.c" /* yacc.c:1646  */
    break;

  case 209:
#line 3090 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3017 "lpc/compiler/grammar.y.pre"
#ifdef WOMBLES
	        if(*(outp-2) != ':')
		  yyerror("End of functional not found");
#endif
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
#line 5363 "y.tab.c" /* yacc.c:1646  */
    break;

  case 210:
#line 3128 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3054 "lpc/compiler/grammar.y.pre"
#ifdef WOMBLES
	        if(*(outp-2) != ':')
		  yyerror("End of functional not found");
#endif
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
#line 5462 "y.tab.c" /* yacc.c:1646  */
    break;

  case 211:
#line 3223 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3148 "lpc/compiler/grammar.y.pre"
#ifdef WOMBLES
	         if(*(outp-2) != ':')
		   yyerror("End of functional not found");
#endif
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
#line 5489 "y.tab.c" /* yacc.c:1646  */
    break;

  case 212:
#line 3246 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3170 "lpc/compiler/grammar.y.pre"
#ifdef WOMBLES
	        if(*(outp-2) != ']')
		  yyerror("End of mapping not found");
#endif
		CREATE_CALL((yyval.node), F_AGGREGATE_ASSOC, TYPE_MAPPING, (yyvsp[-2].node));
	    }
#line 5502 "y.tab.c" /* yacc.c:1646  */
    break;

  case 213:
#line 3255 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3178 "lpc/compiler/grammar.y.pre"
#ifdef WOMBLES
	        if(*(outp-2) != '}')
		  yyerror("End of array not found");
#endif  
		CREATE_CALL((yyval.node), F_AGGREGATE, TYPE_ANY | TYPE_MOD_ARRAY, (yyvsp[-2].node));
	    }
#line 5515 "y.tab.c" /* yacc.c:1646  */
    break;

  case 214:
#line 3267 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3189 "lpc/compiler/grammar.y.pre"
		(yyval.node) = (yyvsp[0].decl).node;
	    }
#line 5524 "y.tab.c" /* yacc.c:1646  */
    break;

  case 215:
#line 3272 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3193 "lpc/compiler/grammar.y.pre"
		(yyval.node) = insert_pop_value((yyvsp[-1].node));
	    }
#line 5533 "y.tab.c" /* yacc.c:1646  */
    break;

  case 216:
#line 3280 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3200 "lpc/compiler/grammar.y.pre"
		(yyval.number) = context;
		context = SPECIAL_CONTEXT;
	    }
#line 5543 "y.tab.c" /* yacc.c:1646  */
    break;

  case 217:
#line 3286 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3205 "lpc/compiler/grammar.y.pre"
		CREATE_CATCH((yyval.node), (yyvsp[0].node));
		context = (yyvsp[-1].number);
	    }
#line 5553 "y.tab.c" /* yacc.c:1646  */
    break;

  case 218:
#line 3295 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3214 "lpc/compiler/grammar.y.pre"
		(yyval.node) = new_node_no_line();
		lpc_tree_form((yyvsp[0].decl).node, (yyval.node));
	    }
#line 5563 "y.tab.c" /* yacc.c:1646  */
    break;

  case 219:
#line 3302 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3220 "lpc/compiler/grammar.y.pre"
		(yyval.node) = new_node_no_line();
		lpc_tree_form((yyvsp[-1].node), (yyval.node));
	    }
#line 5573 "y.tab.c" /* yacc.c:1646  */
    break;

  case 220:
#line 3311 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3229 "lpc/compiler/grammar.y.pre"
		int p = (yyvsp[-1].node)->v.number;
		CREATE_LVALUE_EFUN((yyval.node), TYPE_NUMBER, (yyvsp[-1].node));
		CREATE_BINARY_OP_1((yyval.node)->l.expr, F_SSCANF, 0, (yyvsp[-4].node), (yyvsp[-2].node), p);
	    }
#line 5584 "y.tab.c" /* yacc.c:1646  */
    break;

  case 221:
#line 3321 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3238 "lpc/compiler/grammar.y.pre"
		int p = (yyvsp[-1].node)->v.number;
		CREATE_LVALUE_EFUN((yyval.node), TYPE_NUMBER, (yyvsp[-1].node));
		CREATE_TERNARY_OP_1((yyval.node)->l.expr, F_PARSE_COMMAND, 0, 
				    (yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-2].node), p);
	    }
#line 5596 "y.tab.c" /* yacc.c:1646  */
    break;

  case 222:
#line 3332 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3248 "lpc/compiler/grammar.y.pre"
		(yyval.number) = context;
		context = SPECIAL_CONTEXT;
	    }
#line 5606 "y.tab.c" /* yacc.c:1646  */
    break;

  case 223:
#line 3338 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3253 "lpc/compiler/grammar.y.pre"
		CREATE_TIME_EXPRESSION((yyval.node), (yyvsp[0].node));
		context = (yyvsp[-1].number);
	    }
#line 5616 "y.tab.c" /* yacc.c:1646  */
    break;

  case 224:
#line 3347 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3261 "lpc/compiler/grammar.y.pre"
	        (yyval.node) = new_node_no_line();
		(yyval.node)->r.expr = 0;
	        (yyval.node)->v.number = 0;
	    }
#line 5627 "y.tab.c" /* yacc.c:1646  */
    break;

  case 225:
#line 3354 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3267 "lpc/compiler/grammar.y.pre"
		parse_node_t *insert;
		
		(yyval.node) = (yyvsp[0].node);
		insert = new_node_no_line();
		insert->r.expr = (yyvsp[0].node)->r.expr;
		insert->l.expr = (yyvsp[-1].node);
		(yyvsp[0].node)->r.expr = insert;
		(yyval.node)->v.number++;
	    }
#line 5643 "y.tab.c" /* yacc.c:1646  */
    break;

  case 226:
#line 3369 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3281 "lpc/compiler/grammar.y.pre"
		CREATE_STRING((yyval.node), (yyvsp[0].string));
		scratch_free((yyvsp[0].string));
	    }
#line 5653 "y.tab.c" /* yacc.c:1646  */
    break;

  case 228:
#line 3379 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3290 "lpc/compiler/grammar.y.pre"
		(yyval.string) = (yyvsp[-1].string);
	    }
#line 5662 "y.tab.c" /* yacc.c:1646  */
    break;

  case 229:
#line 3384 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3294 "lpc/compiler/grammar.y.pre"
		(yyval.string) = scratch_join((yyvsp[-2].string), (yyvsp[0].string));
	    }
#line 5671 "y.tab.c" /* yacc.c:1646  */
    break;

  case 231:
#line 3393 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3302 "lpc/compiler/grammar.y.pre"
		(yyval.string) = scratch_join((yyvsp[-1].string), (yyvsp[0].string));
	    }
#line 5680 "y.tab.c" /* yacc.c:1646  */
    break;

  case 232:
#line 3400 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3308 "lpc/compiler/grammar.y.pre"
	(yyval.node) = new_node();
	(yyval.node)->l.expr = (parse_node_t *)(yyvsp[-2].string);
	(yyval.node)->v.expr = (yyvsp[0].node);
	(yyval.node)->r.expr = 0;
    }
#line 5692 "y.tab.c" /* yacc.c:1646  */
    break;

  case 233:
#line 3411 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3318 "lpc/compiler/grammar.y.pre"
	(yyval.node) = 0;
    }
#line 5701 "y.tab.c" /* yacc.c:1646  */
    break;

  case 234:
#line 3416 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3322 "lpc/compiler/grammar.y.pre"
	(yyval.node) = (yyvsp[0].node);
	(yyval.node)->r.expr = (yyvsp[-2].node);
    }
#line 5711 "y.tab.c" /* yacc.c:1646  */
    break;

  case 235:
#line 3426 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3331 "lpc/compiler/grammar.y.pre"
		(yyval.number) = context;
		(yyvsp[0].number) = num_refs;
		context |= ARG_LIST; 
	    }
#line 5722 "y.tab.c" /* yacc.c:1646  */
    break;

  case 236:
#line 3433 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3337 "lpc/compiler/grammar.y.pre"
		context = (yyvsp[-2].number);
		(yyval.node) = validate_efun_call((yyvsp[-4].number),(yyvsp[-1].node));
		(yyval.node) = check_refs(num_refs - (yyvsp[-3].number), (yyvsp[-1].node), (yyval.node));
		num_refs = (yyvsp[-3].number);
	    }
#line 5734 "y.tab.c" /* yacc.c:1646  */
    break;

  case 237:
#line 3441 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3344 "lpc/compiler/grammar.y.pre"
		(yyval.number) = context;
		(yyvsp[0].number) = num_refs;
		context |= ARG_LIST;
	    }
#line 5745 "y.tab.c" /* yacc.c:1646  */
    break;

  case 238:
#line 3448 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3350 "lpc/compiler/grammar.y.pre"
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
#line 5775 "y.tab.c" /* yacc.c:1646  */
    break;

  case 239:
#line 3474 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3375 "lpc/compiler/grammar.y.pre"
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
#line 5815 "y.tab.c" /* yacc.c:1646  */
    break;

  case 240:
#line 3510 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3410 "lpc/compiler/grammar.y.pre"
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
#line 5838 "y.tab.c" /* yacc.c:1646  */
    break;

  case 241:
#line 3529 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3428 "lpc/compiler/grammar.y.pre"
		(yyval.number) = context;
		(yyvsp[0].number) = num_refs;
		context |= ARG_LIST;
	    }
#line 5849 "y.tab.c" /* yacc.c:1646  */
    break;

  case 242:
#line 3536 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3434 "lpc/compiler/grammar.y.pre"
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
		     *
		     * Don't complain, just grok it.
		     */
		    
		    if (current_function_context)
			current_function_context->bindable = FP_NOT_BINDABLE;
		    
		    f = define_new_function((yyvsp[-4].ihe)->name, 0, 0, 
					    DECL_PUBLIC|FUNC_UNDEFINED, TYPE_ANY);
		    (yyval.node)->kind = NODE_CALL_1;
		    (yyval.node)->v.number = F_CALL_FUNCTION_BY_ADDRESS;
		    (yyval.node)->l.number = f;
		    (yyval.node)->type = TYPE_ANY; /* just a guess */
		    if (exact_types) {
			char buf[256];
			char *end = EndOf(buf);
			char *p;
			const char *n = (yyvsp[-4].ihe)->name;
			if (*n == ':') n++;
			/* prevent some errors; by making it look like an
			 * inherited function we prevent redeclaration errors
			 * if it shows up later
			 */
			
			FUNCTION_FLAGS(f) &= ~FUNC_UNDEFINED;
			FUNCTION_FLAGS(f) |= (FUNC_INHERITED | FUNC_VARARGS);
			p = strput(buf, end, "Undefined function ");
			p = strput(p, end, n);
			yyerror(buf);
		    }
		}
		(yyval.node) = check_refs(num_refs - (yyvsp[-3].number), (yyvsp[-1].node), (yyval.node));
		num_refs = (yyvsp[-3].number);
	    }
#line 5915 "y.tab.c" /* yacc.c:1646  */
    break;

  case 243:
#line 3598 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3495 "lpc/compiler/grammar.y.pre"
		(yyval.number) = context;
		(yyvsp[0].number) = num_refs;
		context |= ARG_LIST;
	    }
#line 5926 "y.tab.c" /* yacc.c:1646  */
    break;

  case 244:
#line 3605 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3501 "lpc/compiler/grammar.y.pre"
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
		  int f;
		  ident_hash_elem_t *ihe;
		  
		  f = (ihe = lookup_ident(name)) ? ihe->dn.function_num : -1;
		  (yyval.node)->kind = NODE_CALL_1;
		  (yyval.node)->v.number = F_CALL_FUNCTION_BY_ADDRESS;
		  if (f!=-1) {
		      /* The only way this can happen is if function_name
		       * below made the function name.  The lexer would
		       * return L_DEFINED_NAME instead.
		       */
		      (yyval.node)->type = validate_function_call(f, (yyvsp[-1].node)->r.expr);
		  } else {
		      f = define_new_function(name, 0, 0, 
					      DECL_PUBLIC|FUNC_UNDEFINED, TYPE_ANY);
		  }
		  (yyval.node)->l.number = f;
		  /*
		   * Check if this function has been defined.
		   * But, don't complain yet about functions defined
		   * by inheritance.
		   */
		  if (exact_types && (FUNCTION_FLAGS(f) & FUNC_UNDEFINED)) {
		      char buf[256];
		      char *end = EndOf(buf);
		      char *p;
		      char *n = (yyvsp[-4].string);
		      if (*n == ':') n++;
		      /* prevent some errors */
		      FUNCTION_FLAGS(f) &= ~FUNC_UNDEFINED;
		      FUNCTION_FLAGS(f) |= (FUNC_INHERITED | FUNC_VARARGS);
		      p = strput(buf, end, "Undefined function ");
		      p = strput(p, end, n);
		      yyerror(buf);
		  }
		  if (!(FUNCTION_FLAGS(f) & FUNC_UNDEFINED))
		      (yyval.node)->type = FUNCTION_DEF(f)->type;
		  else
		      (yyval.node)->type = TYPE_ANY;  /* Just a guess */
	      }
	      (yyval.node) = check_refs(num_refs - (yyvsp[-3].number), (yyvsp[-1].node), (yyval.node));
	      num_refs = (yyvsp[-3].number);
	      scratch_free(name);
	  }
#line 5996 "y.tab.c" /* yacc.c:1646  */
    break;

  case 245:
#line 3671 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3566 "lpc/compiler/grammar.y.pre"
		(yyval.number) = context;
		(yyvsp[0].number) = num_refs;
		context |= ARG_LIST;
	    }
#line 6007 "y.tab.c" /* yacc.c:1646  */
    break;

  case 246:
#line 3678 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3572 "lpc/compiler/grammar.y.pre"
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
#line 6057 "y.tab.c" /* yacc.c:1646  */
    break;

  case 247:
#line 3724 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3617 "lpc/compiler/grammar.y.pre"
		(yyval.number) = context;
		(yyvsp[0].number) = num_refs;
		context |= ARG_LIST;
	    }
#line 6068 "y.tab.c" /* yacc.c:1646  */
    break;

  case 248:
#line 3731 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3623 "lpc/compiler/grammar.y.pre"
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
#line 6095 "y.tab.c" /* yacc.c:1646  */
    break;

  case 249:
#line 3755 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3646 "lpc/compiler/grammar.y.pre"
	svalue_t *res;
	ident_hash_elem_t *ihe;

	(yyval.number) = (ihe = lookup_ident((yyvsp[0].string))) ? ihe->dn.efun_num : -1;
	if ((yyval.number) == -1) {
	    char buf[256];
	    char *end = EndOf(buf);
	    char *p;
	    
	    p = strput(buf, end, "Unknown efun: ");
	    p = strput(p, end, (yyvsp[0].string));
	    yyerror(buf);
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
#line 6126 "y.tab.c" /* yacc.c:1646  */
    break;

  case 250:
#line 3781 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3671 "lpc/compiler/grammar.y.pre"
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
#line 6144 "y.tab.c" /* yacc.c:1646  */
    break;

  case 252:
#line 3799 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3688 "lpc/compiler/grammar.y.pre"
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
#line 6163 "y.tab.c" /* yacc.c:1646  */
    break;

  case 253:
#line 3814 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3702 "lpc/compiler/grammar.y.pre"
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
#line 6183 "y.tab.c" /* yacc.c:1646  */
    break;

  case 254:
#line 3830 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3717 "lpc/compiler/grammar.y.pre"
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
#line 6200 "y.tab.c" /* yacc.c:1646  */
    break;

  case 255:
#line 3846 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3732 "lpc/compiler/grammar.y.pre"
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
#line 6236 "y.tab.c" /* yacc.c:1646  */
    break;

  case 256:
#line 3881 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3766 "lpc/compiler/grammar.y.pre"
		(yyval.node) = 0;
	    }
#line 6245 "y.tab.c" /* yacc.c:1646  */
    break;

  case 257:
#line 3886 "lpc/compiler/grammar.y" /* yacc.c:1646  */
    {
#line 3770 "lpc/compiler/grammar.y.pre"
		(yyval.node) = (yyvsp[0].node);
            }
#line 6254 "y.tab.c" /* yacc.c:1646  */
    break;


#line 6258 "y.tab.c" /* yacc.c:1646  */
      default: break;
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
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
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

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

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

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
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
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
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
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 3891 "lpc/compiler/grammar.y" /* yacc.c:1906  */



#line 3775 "lpc/compiler/grammar.y.pre"
