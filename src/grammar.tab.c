/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 3 "grammar.y"


extern char *outp;
#include "std.h"
#include "compiler.h"
#include "lex.h"
#include "scratchpad.h"

#include "lpc_incl.h"
#include "simul_efun.h"
#include "generate.h"
#include "master.h"

/* gross. Necessary? - Beek */
#ifdef WIN32
#define MSDOS
#endif
#define YYSTACK_USE_ALLOCA 0
#line 20 "grammar.y.pre"
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



/* Line 268 of yacc.c  */
#line 114 "y.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
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
     L_REF = 299,
     L_PARSE_COMMAND = 300,
     L_TIME_EXPRESSION = 301,
     L_CLASS = 302,
     L_NEW = 303,
     L_PARAMETER = 304,
     L_LAMBDA = 305,
     LOWER_THAN_ELSE = 306,
     L_NE = 307,
     L_EQ = 308
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
#define L_REF 299
#define L_PARSE_COMMAND 300
#define L_TIME_EXPRESSION 301
#define L_CLASS 302
#define L_NEW 303
#define L_PARAMETER 304
#define L_LAMBDA 305
#define LOWER_THAN_ELSE 306
#define L_NE 307
#define L_EQ 308




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 293 of yacc.c  */
#line 124 "grammar.y"

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



/* Line 293 of yacc.c  */
#line 279 "y.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 291 "y.tab.c"

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
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
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
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
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
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1971

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  75
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  101
/* YYNRULES -- Number of rules.  */
#define YYNRULES  257
/* YYNRULES -- Number of states.  */
#define YYNSTATES  487

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   308

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
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
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     9,    10,    11,    13,    18,    20,
      22,    23,    25,    27,    29,    31,    33,    35,    36,    37,
      47,    51,    53,    55,    57,    60,    63,    65,    69,    70,
      71,    77,    78,    86,    88,    90,    92,    95,    98,   100,
     102,   105,   108,   112,   114,   115,   117,   120,   122,   126,
     127,   130,   133,   138,   140,   141,   143,   147,   150,   155,
     160,   162,   164,   166,   167,   168,   174,   177,   182,   186,
     190,   192,   196,   197,   200,   203,   206,   208,   210,   212,
     214,   216,   218,   220,   223,   226,   227,   234,   235,   244,
     245,   256,   258,   260,   262,   264,   268,   269,   278,   279,
     281,   283,   285,   286,   297,   300,   303,   304,   308,   314,
     319,   324,   327,   329,   331,   335,   339,   343,   347,   351,
     355,   359,   363,   367,   371,   375,   379,   383,   387,   391,
     393,   396,   399,   402,   404,   408,   410,   412,   415,   419,
     423,   429,   433,   437,   441,   445,   449,   453,   457,   461,
     465,   469,   473,   477,   481,   485,   489,   493,   496,   499,
     502,   505,   508,   511,   514,   517,   519,   521,   523,   525,
     527,   529,   532,   536,   537,   539,   542,   544,   547,   549,
     553,   554,   556,   559,   561,   565,   569,   571,   573,   576,
     580,   583,   585,   587,   589,   591,   592,   598,   602,   609,
     617,   626,   634,   640,   647,   653,   658,   660,   664,   666,
     667,   674,   676,   682,   687,   692,   697,   699,   703,   704,
     708,   716,   726,   727,   731,   732,   736,   738,   740,   744,
     748,   750,   753,   757,   758,   762,   763,   769,   770,   776,
     783,   790,   791,   797,   798,   804,   805,   813,   814,   823,
     827,   831,   833,   836,   840,   844,   851,   852
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      76,     0,    -1,    77,    -1,    77,    85,    78,    -1,    -1,
      -1,    65,    -1,   102,    36,   161,    65,    -1,     5,    -1,
       4,    -1,    -1,    61,    -1,   108,    -1,    65,    -1,     1,
      -1,     8,    -1,     9,    -1,    -1,    -1,   103,    82,    84,
      86,    66,   100,    67,    87,    83,    -1,   103,   106,    65,
      -1,    79,    -1,    93,    -1,    88,    -1,   102,    68,    -1,
      82,    84,    -1,    89,    -1,    89,    69,    90,    -1,    -1,
      -1,    91,    97,    92,    90,    65,    -1,    -1,   102,    47,
      84,    70,    94,    91,    71,    -1,     9,    -1,     8,    -1,
       6,    -1,    47,     8,    -1,    47,     9,    -1,    96,    -1,
      97,    -1,    97,   137,    -1,    98,    82,    -1,    98,    82,
      95,    -1,    95,    -1,    -1,   101,    -1,   101,    26,    -1,
      99,    -1,   101,    69,    99,    -1,    -1,     7,   102,    -1,
     102,   105,    -1,    66,    97,    82,    67,    -1,    97,    -1,
      -1,   107,    -1,   107,    69,   106,    -1,    82,    84,    -1,
      82,    84,    13,   138,    -1,    70,   110,   116,    71,    -1,
     108,    -1,   122,    -1,   126,    -1,    -1,    -1,   110,    97,
     111,   115,    65,    -1,    82,    95,    -1,    82,    95,    13,
     138,    -1,    98,    82,    95,    -1,   113,    13,   138,    -1,
     112,    -1,   112,    69,   115,    -1,    -1,   117,   116,    -1,
       1,    65,    -1,   136,    65,    -1,   174,    -1,   118,    -1,
     120,    -1,   130,    -1,   139,    -1,   109,    -1,    65,    -1,
      32,    65,    -1,    33,    65,    -1,    -1,    27,    66,   136,
      67,   119,   117,    -1,    -1,    28,   121,   117,    27,    66,
     136,    67,    65,    -1,    -1,    29,    66,   129,    65,   128,
      65,   128,    67,   123,   117,    -1,     8,    -1,   113,    -1,
       9,    -1,   124,    -1,   124,    69,   124,    -1,    -1,    30,
      66,   125,    31,   138,    67,   127,   117,    -1,    -1,   136,
      -1,   128,    -1,   114,    -1,    -1,    22,    66,   136,    67,
     131,    70,   110,   133,   132,    71,    -1,   133,   132,    -1,
     117,   132,    -1,    -1,    23,   134,    68,    -1,    23,   134,
      25,   134,    68,    -1,    23,   134,    25,    68,    -1,    23,
      25,   134,    68,    -1,    24,    68,    -1,   135,    -1,   161,
      -1,   135,    53,   135,    -1,   135,    54,   135,    -1,   135,
      55,   135,    -1,   135,    57,   135,    -1,   135,    56,   135,
      -1,   135,    18,   135,    -1,   135,    58,   135,    -1,   135,
      16,   135,    -1,   135,    17,   135,    -1,   135,    59,   135,
      -1,   135,    60,   135,    -1,   135,    61,   135,    -1,   135,
      62,   135,    -1,   135,    63,   135,    -1,    66,   135,    67,
      -1,     4,    -1,    60,     4,    -1,    19,     4,    -1,    64,
       4,    -1,   138,    -1,   136,    69,   138,    -1,    44,    -1,
      55,    -1,   137,   146,    -1,   146,    13,   138,    -1,     1,
      13,   138,    -1,   138,    52,   138,    68,   138,    -1,   138,
      15,   138,    -1,   138,    14,   138,    -1,   138,    53,   138,
      -1,   138,    54,   138,    -1,   138,    55,   138,    -1,   138,
      57,   138,    -1,   138,    56,   138,    -1,   138,    18,   138,
      -1,   138,    58,   138,    -1,   138,    16,   138,    -1,   138,
      17,   138,    -1,   138,    59,   138,    -1,   138,    60,   138,
      -1,   138,    61,   138,    -1,   138,    62,   138,    -1,   138,
      63,   138,    -1,   104,   138,    -1,    11,   146,    -1,    12,
     146,    -1,    19,   138,    -1,    64,   138,    -1,    60,   138,
      -1,   146,    11,    -1,   146,    12,    -1,   149,    -1,   155,
      -1,   156,    -1,   157,    -1,    81,    -1,    80,    -1,    34,
      65,    -1,    34,   136,    65,    -1,    -1,   142,    -1,   142,
      69,    -1,   138,    -1,   138,    26,    -1,   141,    -1,   142,
      69,   141,    -1,    -1,   144,    -1,   144,    69,    -1,   145,
      -1,   144,    69,   145,    -1,   138,    68,   138,    -1,   149,
      -1,    41,    -1,    40,   172,    -1,   147,    68,    67,    -1,
      50,     8,    -1,   165,    -1,     8,    -1,     9,    -1,    49,
      -1,    -1,    72,    66,   150,   136,    67,    -1,   149,    35,
      84,    -1,   149,    73,   136,    25,   136,    74,    -1,   149,
      73,    58,   136,    25,   136,    74,    -1,   149,    73,    58,
     136,    25,    58,   136,    74,    -1,   149,    73,   136,    25,
      58,   136,    74,    -1,   149,    73,   136,    25,    74,    -1,
     149,    73,    58,   136,    25,    74,    -1,   149,    73,    58,
     136,    74,    -1,   149,    73,   136,    74,    -1,   160,    -1,
      66,   136,    67,    -1,   153,    -1,    -1,     6,   151,    66,
     100,    67,   108,    -1,   148,    -1,   147,    69,   142,    68,
      67,    -1,    40,   136,    68,    67,    -1,    39,   143,    74,
      67,    -1,    38,   140,    71,    67,    -1,   108,    -1,    66,
     136,    67,    -1,    -1,    43,   154,   152,    -1,    42,    66,
     138,    69,   138,   159,    67,    -1,    45,    66,   138,    69,
     138,    69,   138,   159,    67,    -1,    -1,    46,   158,   152,
      -1,    -1,    69,   146,   159,    -1,   162,    -1,   162,    -1,
      66,   161,    67,    -1,   161,    59,   161,    -1,     3,    -1,
     162,     3,    -1,    84,    68,   138,    -1,    -1,   164,    69,
     163,    -1,    -1,   172,    66,   166,   140,    67,    -1,    -1,
      48,    66,   167,   140,    67,    -1,    48,    66,    47,     8,
     164,    67,    -1,    48,    66,    47,     9,   164,    67,    -1,
      -1,     8,    66,   168,   140,    67,    -1,    -1,   173,    66,
     169,   140,    67,    -1,    -1,   149,    35,    84,    66,   170,
     140,    67,    -1,    -1,    66,    61,   136,    67,    66,   171,
     140,    67,    -1,    10,    37,    84,    -1,    10,    37,    48,
      -1,     9,    -1,    37,    84,    -1,     6,    37,    84,    -1,
      84,    37,    84,    -1,    20,    66,   136,    67,   117,   175,
      -1,    -1,    21,   117,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   210,   210,   218,   224,   230,   232,   242,   338,   346,
     355,   359,   367,   375,   380,   388,   393,   398,   440,   397,
     498,   505,   506,   507,   510,   528,   542,   543,   546,   549,
     548,   558,   557,   638,   639,   657,   658,   675,   692,   696,
     697,   705,   712,   721,   735,   740,   741,   760,   772,   787,
     791,   814,   823,   831,   833,   840,   841,   845,   871,   927,
     937,   937,   937,   941,   947,   946,   968,   980,  1015,  1027,
    1060,  1066,  1078,  1082,  1089,  1097,  1110,  1111,  1112,  1113,
    1114,  1115,  1121,  1126,  1149,  1163,  1162,  1178,  1177,  1193,
    1192,  1218,  1240,  1251,  1269,  1275,  1287,  1286,  1308,  1312,
    1316,  1322,  1332,  1331,  1372,  1379,  1387,  1395,  1403,  1418,
    1433,  1447,  1464,  1479,  1495,  1500,  1505,  1510,  1515,  1520,
    1529,  1534,  1539,  1544,  1549,  1554,  1559,  1564,  1569,  1574,
    1579,  1584,  1589,  1597,  1602,  1610,  1611,  1615,  1644,  1670,
    1676,  1701,  1708,  1715,  1741,  1746,  1770,  1793,  1808,  1853,
    1891,  1896,  1901,  2072,  2167,  2248,  2253,  2349,  2371,  2393,
    2416,  2426,  2438,  2463,  2486,  2508,  2509,  2510,  2511,  2512,
    2513,  2517,  2524,  2546,  2550,  2555,  2563,  2568,  2576,  2583,
    2597,  2602,  2607,  2615,  2626,  2645,  2653,  2769,  2770,  2777,
    2782,  2802,  2803,  2846,  2863,  2869,  2868,  2900,  2925,  2930,
    2935,  2943,  2951,  2956,  2961,  3007,  3062,  3063,  3068,  3070,
    3069,  3126,  3164,  3259,  3282,  3291,  3303,  3308,  3317,  3316,
    3332,  3342,  3354,  3353,  3369,  3375,  3390,  3399,  3400,  3405,
    3413,  3414,  3421,  3433,  3437,  3448,  3447,  3463,  3462,  3495,
    3531,  3551,  3550,  3620,  3619,  3693,  3692,  3746,  3745,  3777,
    3803,  3819,  3820,  3835,  3851,  3867,  3902,  3907
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
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
  "L_FUNCTION_OPEN", "L_NEW_FUNCTION_OPEN", "L_SSCANF", "L_CATCH", "L_REF",
  "L_PARSE_COMMAND", "L_TIME_EXPRESSION", "L_CLASS", "L_NEW",
  "L_PARAMETER", "L_LAMBDA", "LOWER_THAN_ELSE", "'?'", "'|'", "'^'", "'&'",
  "L_NE", "L_EQ", "'<'", "'+'", "'-'", "'*'", "'%'", "'/'", "'~'", "';'",
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
  "assoc_pair", "lvalue", "l_new_function_open", "simple_function_pointer",
  "expr4", "@11", "@12", "expr_or_block", "catch", "@13", "sscanf",
  "parse_command", "time_expression", "@14", "lvalue_list", "string",
  "string_con1", "string_con2", "class_init", "opt_class_init",
  "function_call", "@15", "@16", "@17", "@18", "@19", "@20",
  "efun_override", "function_name", "cond", "optional_else_part", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
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
     135,   135,   135,   136,   136,   137,   137,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     138,   139,   139,   140,   140,   140,   141,   141,   142,   142,
     143,   143,   143,   144,   144,   145,   146,   147,   147,   148,
     148,   149,   149,   149,   149,   150,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   149,   151,
     149,   149,   149,   149,   149,   149,   152,   152,   154,   153,
     155,   156,   158,   157,   159,   159,   160,   161,   161,   161,
     162,   162,   163,   164,   164,   166,   165,   167,   165,   165,
     165,   168,   165,   169,   165,   170,   165,   171,   165,   172,
     172,   173,   173,   173,   173,   174,   175,   175
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
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
       2,     2,     2,     1,     3,     1,     1,     2,     3,     3,
       5,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     2,     2,     2,     2,     1,     1,     1,     1,     1,
       1,     2,     3,     0,     1,     2,     1,     2,     1,     3,
       0,     1,     2,     1,     3,     3,     1,     1,     2,     3,
       2,     1,     1,     1,     1,     0,     5,     3,     6,     7,
       8,     7,     5,     6,     5,     4,     1,     3,     1,     0,
       6,     1,     5,     4,     4,     4,     1,     3,     0,     3,
       7,     9,     0,     3,     0,     3,     1,     1,     3,     3,
       1,     2,     3,     0,     3,     0,     5,     0,     5,     6,
       6,     0,     5,     0,     5,     0,     7,     0,     8,     3,
       3,     1,     2,     3,     3,     6,     0,     2
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       4,     0,    49,     1,    49,    21,     5,    23,    22,    54,
      10,    50,     6,     3,    35,     0,     0,    24,    38,    53,
      51,    11,     0,     0,    55,   230,     0,     0,   227,    36,
      37,     0,    15,    16,    57,    20,    10,     0,     0,     7,
     231,    31,     0,     0,     0,    56,   228,   229,    28,     0,
       9,     8,   209,   192,   193,     0,     0,     0,     0,     0,
       0,     0,     0,   187,     0,   218,   135,     0,   222,     0,
     194,     0,   136,     0,     0,     0,     0,   170,   169,     0,
       0,     0,    58,     0,     0,   211,   165,   208,   166,   167,
     168,   206,   226,   191,     0,     0,    44,    57,     0,     0,
       0,     0,   241,     0,     0,   158,   186,   159,   160,   252,
     176,     0,   178,   174,     0,     0,   181,   183,     0,   133,
     188,     0,     0,     0,     0,   237,   190,   162,   161,    35,
       0,     0,    10,     0,   195,     0,   157,   137,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   163,   164,     0,     0,     0,
       0,     0,   235,   243,    34,    33,    43,    39,    10,    47,
       0,    45,    32,    29,   139,   253,    44,     0,   250,   249,
     177,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      63,   216,   219,     0,   223,     0,     0,    36,    37,     0,
       0,   207,     0,   254,   142,   141,   150,   151,   148,     0,
     143,   144,   145,   147,   146,   149,   152,   153,   154,   155,
     156,   138,   189,     0,   197,     0,     0,     0,     0,    40,
      41,    18,    46,     0,    10,     0,     0,   215,   179,   185,
     214,   184,   213,   134,     0,     0,     0,     0,   233,   233,
       0,     0,    52,     0,     0,     0,     0,   245,     0,     0,
     205,     0,     0,    42,     0,    48,     0,    26,     0,     0,
     242,   224,   217,     0,     0,     0,     0,    87,     0,     0,
       0,     0,     0,    82,    64,    60,    81,     0,     0,    77,
      78,    61,    62,    79,     0,    80,    76,     0,     0,     0,
     238,   247,   196,   140,   212,     0,     0,   204,     0,   202,
       0,   236,   244,    14,    13,    19,    12,    25,    10,    30,
     210,     0,     0,    74,     0,     0,     0,     0,     0,     0,
      83,    84,   171,     0,    10,    59,    73,    75,     0,   239,
       0,   240,     0,     0,     0,   203,     0,     0,   198,    27,
     224,   220,     0,     0,     0,     0,    10,     0,   101,   100,
       0,    99,    91,    93,    92,    94,     0,   172,     0,    70,
       0,   224,     0,   234,     0,   246,     0,   199,   201,   225,
       0,   102,    85,     0,     0,     0,     0,     0,     0,    66,
      10,    65,     0,     0,   248,   200,   256,     0,     0,     0,
      68,    69,     0,    95,     0,     0,    71,   221,   232,     0,
     255,    63,    86,     0,     0,    96,    67,   257,     0,     0,
       0,     0,     0,     0,     0,    88,    89,    97,   129,     0,
       0,     0,     0,     0,     0,   112,   113,   111,     0,     0,
       0,     0,   131,     0,   130,   132,     0,     0,   107,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   105,   103,   104,    90,   110,   128,   109,
       0,     0,   121,   122,   119,   114,   115,   116,   118,   117,
     120,   123,   124,   125,   126,   127,   108
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,    13,     5,    77,    78,   266,   315,    79,
       6,    43,   264,     7,   267,   268,    98,   234,     8,    48,
     166,    18,   167,   168,   169,   170,   171,     9,    10,    80,
      20,    23,    24,   285,   286,   246,   334,   369,   364,   358,
     370,   287,   438,   289,   398,   290,   327,   291,   441,   365,
     366,   292,   421,   359,   360,   293,   397,   439,   440,   434,
     435,   294,    81,   119,   295,   111,   112,   113,   115,   116,
     117,    83,    84,    85,    86,   202,   101,   192,    87,   122,
      88,    89,    90,   124,   322,    91,   436,    92,   373,   298,
      93,   227,   196,   177,   228,   305,   342,    94,    95,   296,
     410
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -384
static const yytype_int16 yypact[] =
{
    -384,    59,   178,  -384,    76,  -384,    72,  -384,  -384,    26,
      64,  -384,  -384,  -384,  -384,    22,    36,  -384,  -384,  -384,
    -384,  -384,   164,    82,    89,  -384,    22,    70,   183,   124,
     142,   156,  -384,  -384,    23,  -384,    64,    63,    22,  -384,
    -384,  -384,  1591,   137,   164,  -384,  -384,  -384,  -384,   222,
    -384,  -384,   208,    24,    79,   211,  1639,  1639,  1591,   164,
    1099,   457,  1591,  -384,   206,  -384,  -384,   226,  -384,   229,
    -384,   267,  -384,  1591,  1591,   887,   244,  -384,  -384,   297,
    1591,  1639,  1885,   230,   186,  -384,   128,  -384,  -384,  -384,
    -384,  -384,   183,  -384,   249,   273,    46,   335,     4,  1591,
     164,   286,  -384,    48,  1171,  -384,    60,  -384,  -384,  -384,
    1853,   314,  -384,   323,  1776,   326,   332,  -384,   261,  1885,
     249,  1591,   104,  1591,   104,   356,  -384,  -384,  -384,    86,
     210,  1591,    64,   133,  -384,   164,  -384,  -384,  1591,  1591,
    1591,  1591,  1591,  1591,  1591,  1591,  1591,  1591,  1591,  1591,
    1591,  1591,  1591,  1591,  1591,  -384,  -384,  1591,   338,  1591,
     164,  1241,  -384,  -384,  -384,  -384,  -384,    87,    64,  -384,
     339,     0,  -384,  -384,  1885,  -384,    46,  1311,  -384,  -384,
    -384,   341,   957,  1591,   342,   529,   343,  1591,  1676,  1591,
    -384,  -384,  -384,  1698,  -384,   355,  1311,  -384,  -384,   209,
     345,  -384,  1591,  -384,   497,   430,   245,   245,   190,  1798,
     923,   320,  1135,  1070,  1070,   190,   227,   227,  -384,  -384,
    -384,  1885,  -384,   304,   347,  1591,    92,  1311,  1311,  -384,
     390,  -384,  -384,    46,    64,   349,   350,  -384,  -384,  1885,
    -384,  -384,  -384,  1885,  1591,   217,   673,  1591,  -384,  -384,
     353,   348,  -384,   242,  1591,   354,  1591,  -384,   102,   219,
    -384,   369,   370,  -384,    16,  -384,   164,   372,   373,   380,
    -384,  1726,  -384,     2,   376,   386,   387,  -384,   388,   389,
     374,   391,  1381,  -384,  -384,  -384,  -384,   393,   745,  -384,
    -384,  -384,  -384,  -384,   167,  -384,  -384,  1748,   266,   278,
    -384,  -384,  -384,  1885,  -384,  1311,   385,  -384,  1591,  -384,
     -21,  -384,  -384,  -384,  -384,  -384,  -384,  -384,    64,  -384,
    -384,  1639,   403,  -384,  1591,  1591,  1591,   817,  1029,   187,
    -384,  -384,  -384,   168,    64,  -384,  -384,  -384,  1591,  -384,
     164,  -384,  1311,   404,  1591,  -384,    90,   113,  -384,  -384,
     405,  -384,   284,   293,   298,   445,    64,   460,  -384,  -384,
     410,   408,  -384,  -384,  -384,   409,   448,  -384,   390,   411,
     416,  1726,   414,  -384,   437,  -384,   135,  -384,  -384,  -384,
     817,  -384,  -384,   442,   390,  1591,  1451,   187,  1591,   496,
      64,  -384,   443,  1591,  -384,  -384,   490,   446,   817,  1591,
    -384,  1885,   453,  -384,  1831,  1591,  -384,  -384,  1885,   817,
    -384,  -384,  -384,   301,  1521,  -384,  1885,  -384,   174,   454,
     455,   817,   150,   452,   601,  -384,  -384,  -384,  -384,   520,
     180,   521,   522,   180,    17,   857,   468,  -384,   601,   465,
     601,   817,  -384,   474,  -384,  -384,   264,    68,  -384,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,  -384,  -384,  -384,  -384,  -384,  -384,  -384,
     475,    10,   296,   296,   281,   997,  1064,  1144,  1206,  1206,
     281,   239,   239,  -384,  -384,  -384,  -384
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -384,  -384,  -384,  -384,  -384,  -384,  -384,    -6,  -384,    21,
    -384,  -384,  -384,  -384,  -384,   228,  -384,  -384,  -384,  -384,
    -217,  -384,    -7,  -289,   295,   368,  -384,   541,  -384,  -384,
    -384,   511,  -384,  -104,  -384,   138,  -384,  -384,   233,  -384,
     172,   275,  -230,  -384,  -384,  -384,  -384,  -384,  -384,   177,
    -384,  -384,  -384,  -348,  -384,  -384,  -384,   -69,   147,  -383,
    1500,   -12,   413,   -39,  -384,  -150,  -177,   417,  -384,  -384,
     396,   -50,  -384,  -384,   -48,  -384,  -384,   458,  -384,  -384,
    -384,  -384,  -384,  -384,  -322,  -384,   -14,   -15,  -384,   334,
    -384,  -384,  -384,  -384,  -384,  -384,  -384,   523,  -384,  -384,
    -384
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -252
static const yytype_int16 yytable[] =
{
      28,    27,    19,    82,    22,   238,   105,   107,   106,   106,
      14,    28,    37,   263,   428,    99,   288,   313,   191,   108,
     191,   110,   114,    28,    47,    25,   232,   236,   379,   429,
      44,   137,    14,   106,   127,   128,    42,    31,   402,   356,
     356,   136,   447,    34,    29,    30,   250,   443,   187,   392,
     118,   130,    14,   348,   164,   165,    32,    33,   288,     3,
     174,   -15,    15,   133,   470,    97,   420,   323,   132,   233,
     431,    25,   428,    16,   432,   172,   471,   261,   262,   238,
     109,   314,   188,     4,   193,   448,   190,   429,    26,   -17,
     102,   173,   133,   130,    17,   160,   178,   355,   356,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   -16,   259,   221,   199,
     110,   175,    38,   100,   179,    21,   200,   306,   431,    38,
      46,    66,   432,   161,   433,    39,   469,    12,   110,  -186,
    -186,  -186,    72,   110,   239,  -251,   114,    35,   243,   226,
     396,   389,  -209,    25,   428,   343,   203,   110,    36,   187,
     316,   187,   230,   160,   377,   320,   260,   400,   412,   429,
     189,   187,    32,    33,   190,   430,   307,   245,    -2,   417,
      14,   224,   187,    25,   428,     4,    40,   378,   110,   110,
     253,   427,   374,    14,   -15,   362,   363,   422,   423,   429,
     201,   161,   187,    96,   187,   271,   140,   141,   297,   395,
     431,   466,   -16,   258,   432,   303,   433,   110,   197,   198,
      49,   130,    25,    50,    51,    52,    41,    53,    54,    55,
      56,    57,   337,   367,   130,    99,   187,   187,    58,   284,
     431,   155,   156,   157,   432,   100,   433,   310,   103,   150,
     151,   152,   153,   154,   158,   159,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,   110,    69,    70,    71,
     333,   350,   121,   106,    72,   126,   251,   308,   187,    73,
     449,   450,   451,    74,   272,    75,   187,   317,   152,   153,
     154,    76,   123,   309,   346,   125,   347,   449,   450,   371,
     460,   461,   462,   110,   150,   151,   152,   153,   154,   302,
     134,   187,   352,   353,   354,   162,   361,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   368,   186,
     187,   468,   376,   339,   135,   340,   140,   141,   142,   163,
     458,   459,   460,   461,   462,   341,   401,   340,    42,   404,
     384,   380,   176,   187,   408,   458,   459,   460,   461,   462,
     381,   372,   187,   248,   249,   382,   416,   187,   419,   463,
     187,   465,   255,   256,   361,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   368,   181,    49,   413,    25,    50,
      51,    52,   182,    53,    54,    55,    56,    57,   164,   165,
     184,   185,   361,   195,    58,   222,   231,    28,   237,   240,
     242,   284,   252,   257,   301,    28,   269,   270,    28,    37,
     300,   304,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    28,    69,    70,    71,   311,   312,   319,   330,
      72,   318,   324,   344,   138,    73,   140,   141,   142,    74,
     190,    75,   325,   326,   328,   329,   331,    76,    49,   345,
      25,    50,    51,    52,   335,    53,    54,    55,    56,    57,
     351,   375,   383,   385,   321,   386,    58,   187,   387,   388,
     390,   391,   393,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,   394,    69,    70,    71,   399,   405,
     407,   409,    72,   140,   141,   142,   411,    73,   414,   425,
     437,    74,   426,    75,   442,   444,   445,    38,   265,    76,
      49,  -180,    25,    50,    51,    52,   464,    53,    54,    55,
      56,    57,   467,   486,   235,    11,   349,    45,    58,   418,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   357,   406,   336,   403,   424,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,   223,    69,    70,    71,
     229,   241,   194,   299,    72,   120,     0,     0,     0,    73,
       0,     0,     0,    74,     0,    75,     0,     0,     0,     0,
       0,    76,    49,  -182,    25,    50,    51,    52,     0,    53,
      54,    55,    56,    57,     0,     0,     0,     0,     0,     0,
      58,   274,     0,   275,   422,   423,     0,     0,   276,   277,
     278,   279,     0,   280,   281,   282,     0,     0,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,     0,    69,
      70,    71,     0,     0,     0,     0,    72,     0,     0,     0,
       0,    73,     0,     0,     0,    74,   283,    75,     0,     0,
       0,   190,  -106,    76,   273,     0,    25,    50,    51,   129,
       0,    53,    54,    55,    56,    57,     0,     0,     0,     0,
       0,     0,    58,   274,     0,   275,     0,     0,     0,     0,
     276,   277,   278,   279,     0,   280,   281,   282,     0,     0,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
     130,    69,    70,    71,     0,     0,     0,     0,    72,     0,
       0,     0,     0,    73,     0,     0,     0,    74,   283,    75,
       0,     0,     0,   190,   -72,    76,   273,     0,    25,    50,
      51,    52,     0,    53,    54,    55,    56,    57,     0,     0,
       0,     0,     0,     0,    58,   274,     0,   275,     0,     0,
       0,     0,   276,   277,   278,   279,     0,   280,   281,   282,
       0,     0,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,     0,    69,    70,    71,     0,     0,     0,     0,
      72,     0,     0,     0,     0,    73,     0,     0,     0,    74,
     283,    75,     0,     0,     0,   190,   -72,    76,    49,     0,
      25,    50,    51,    52,     0,    53,    54,    55,    56,    57,
       0,     0,     0,     0,     0,     0,    58,   274,     0,   275,
       0,     0,     0,     0,   276,   277,   278,   279,     0,   280,
     281,   282,     0,     0,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,     0,    69,    70,    71,     0,     0,
       0,     0,    72,   449,   450,   451,     0,    73,     0,     0,
       0,    74,   283,    75,     0,     0,     0,   190,    49,    76,
      25,    50,    51,   129,     0,    53,    54,    55,    56,    57,
       0,     0,     0,     0,     0,     0,    58,     0,     0,     0,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,     0,     0,     0,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,   130,    69,    70,    71,     0,   140,
     141,   142,    72,     0,     0,     0,     0,    73,   131,     0,
       0,    74,     0,    75,     0,     0,     0,     0,    49,    76,
      25,    50,    51,    52,     0,    53,    54,    55,    56,    57,
       0,     0,     0,     0,     0,     0,    58,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,     0,     0,     0,
       0,     0,     0,     0,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,     0,    69,    70,    71,     0,     0,
       0,     0,    72,   449,   450,   451,     0,    73,     0,     0,
       0,    74,     0,    75,  -175,     0,     0,     0,  -175,    76,
      49,     0,    25,    50,    51,   129,     0,    53,    54,    55,
      56,    57,     0,     0,     0,     0,     0,     0,    58,     0,
       0,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,     0,     0,     0,     0,     0,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,   130,    69,    70,    71,
     449,   450,   451,     0,    72,     0,   140,   141,   142,    73,
       0,     0,     0,    74,   -98,    75,     0,     0,     0,     0,
      49,    76,    25,    50,    51,    52,     0,    53,    54,    55,
      56,    57,     0,     0,     0,     0,     0,     0,    58,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   149,   150,
     151,   152,   153,   154,     0,     0,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,     0,    69,    70,    71,
       0,   140,   141,   142,    72,     0,     0,     0,     0,    73,
     449,   450,   451,    74,     0,    75,     0,     0,     0,     0,
    -173,    76,    49,     0,    25,    50,    51,    52,     0,    53,
      54,    55,    56,    57,     0,     0,     0,     0,     0,     0,
      58,   147,   148,   149,   150,   151,   152,   153,   154,     0,
     455,   456,   457,   458,   459,   460,   461,   462,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,     0,    69,
      70,    71,   449,   450,   451,     0,    72,     0,     0,     0,
       0,    73,   131,     0,     0,    74,     0,    75,     0,     0,
       0,     0,    49,    76,    25,    50,    51,    52,     0,    53,
      54,    55,    56,    57,     0,     0,     0,     0,     0,     0,
      58,     0,     0,     0,   457,   458,   459,   460,   461,   462,
       0,     0,     0,     0,     0,     0,     0,     0,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,     0,    69,
      70,    71,     0,     0,     0,     0,    72,     0,     0,   225,
       0,    73,     0,     0,     0,    74,     0,    75,     0,     0,
       0,     0,    49,    76,    25,    50,    51,    52,     0,    53,
      54,    55,    56,    57,     0,     0,     0,     0,     0,     0,
      58,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,     0,    69,
      70,    71,     0,     0,     0,     0,    72,     0,     0,     0,
       0,    73,     0,     0,     0,    74,     0,    75,  -173,     0,
       0,     0,    49,    76,    25,    50,    51,    52,     0,    53,
      54,    55,    56,    57,     0,     0,     0,     0,     0,     0,
      58,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,     0,    69,
      70,    71,     0,     0,     0,     0,    72,     0,     0,     0,
       0,    73,     0,     0,     0,    74,   332,    75,     0,     0,
       0,     0,    49,    76,    25,    50,    51,    52,     0,    53,
      54,    55,    56,    57,     0,     0,     0,     0,     0,     0,
      58,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,     0,    69,
      70,    71,     0,     0,     0,     0,    72,     0,     0,     0,
       0,    73,     0,     0,     0,    74,   -98,    75,     0,     0,
       0,     0,    49,    76,    25,    50,    51,    52,     0,    53,
      54,    55,    56,    57,     0,     0,     0,     0,     0,     0,
      58,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,     0,    69,
      70,    71,     0,     0,     0,     0,    72,     0,     0,     0,
       0,    73,     0,     0,     0,    74,     0,    75,   -98,     0,
       0,     0,    49,    76,    25,    50,    51,    52,     0,    53,
      54,    55,    56,    57,     0,     0,     0,     0,     0,     0,
      58,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,     0,    69,
      70,    71,    25,     0,     0,    52,    72,    53,    54,    55,
       0,    73,     0,     0,     0,    74,     0,    75,     0,     0,
       0,     0,     0,    76,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    59,    60,    61,    62,
      63,     0,    65,     0,     0,     0,     0,    69,    70,    71,
     138,   139,   140,   141,   142,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   104,     0,     0,     0,     0,
       0,    76,   138,   139,   140,   141,   142,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     138,   139,   140,   141,   142,   244,     0,     0,     0,     0,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   138,   139,   140,   141,   142,   247,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     138,   139,   140,   141,   142,   321,     0,     0,     0,     0,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   138,   139,   140,   141,   142,   338,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
       0,     0,     0,     0,   183,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,     0,     0,     0,     0,   254,   138,   139,   140,
     141,   142,     0,     0,     0,     0,     0,     0,     0,   180,
       0,     0,     0,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,     0,     0,     0,   415,   138,
     139,   140,   141,   142,     0,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   446,     0,     0,     0,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   472,
     473,   474,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,     0,     0,     0,     0,     0,     0,     0,
       0,   446
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-384))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
      15,    15,     9,    42,    10,   182,    56,    57,    56,    57,
       6,    26,    26,   230,     4,    13,   246,     1,   122,    58,
     124,    60,    61,    38,    38,     3,    26,   177,   350,    19,
      36,    81,     6,    81,    73,    74,    13,    16,   386,   328,
     329,    80,    25,    22,     8,     9,   196,   430,    69,   371,
      62,    47,     6,    74,     8,     9,     8,     9,   288,     0,
      99,    37,    36,    75,   447,    44,   414,    65,    75,    69,
      60,     3,     4,    47,    64,    71,    66,   227,   228,   256,
      59,    65,   121,     7,   123,    68,    70,    19,    66,    66,
      66,    98,   104,    47,    68,    35,    48,   327,   387,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,    37,    25,   157,   131,
     159,   100,    59,    37,   103,    61,   132,    25,    60,    59,
      67,    44,    64,    73,    66,    65,    68,    65,   177,    11,
      12,    13,    55,   182,   183,    66,   185,    65,   187,   161,
     380,   368,    66,     3,     4,   305,   135,   196,    69,    69,
     264,    69,   168,    35,    74,   269,    74,   384,   398,    19,
      66,    69,     8,     9,    70,    25,    74,   189,     0,   409,
       6,   160,    69,     3,     4,     7,     3,    74,   227,   228,
     202,   421,   342,     6,    70,     8,     9,    23,    24,    19,
      67,    73,    69,    66,    69,   244,    16,    17,   247,    74,
      60,   441,    70,   225,    64,   254,    66,   256,     8,     9,
       1,    47,     3,     4,     5,     6,    70,     8,     9,    10,
      11,    12,    65,    65,    47,    13,    69,    69,    19,   246,
      60,    11,    12,    13,    64,    37,    66,   259,    37,    59,
      60,    61,    62,    63,    68,    69,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,   305,    48,    49,    50,
     282,   321,    66,   321,    55,     8,    67,    58,    69,    60,
      16,    17,    18,    64,    67,    66,    69,   266,    61,    62,
      63,    72,    66,    74,   306,    66,   308,    16,    17,   338,
      61,    62,    63,   342,    59,    60,    61,    62,    63,    67,
      66,    69,   324,   325,   326,    66,   328,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,   334,    68,
      69,    67,   344,    67,    37,    69,    16,    17,    18,    66,
      59,    60,    61,    62,    63,    67,   385,    69,    13,   388,
     356,    67,    66,    69,   393,    59,    60,    61,    62,    63,
      67,   340,    69,     8,     9,    67,   405,    69,    67,   438,
      69,   440,    68,    69,   386,    55,    56,    57,    58,    59,
      60,    61,    62,    63,   390,    71,     1,   399,     3,     4,
       5,     6,    69,     8,     9,    10,    11,    12,     8,     9,
      74,    69,   414,    47,    19,    67,    67,   422,    67,    67,
      67,   418,    67,    66,    66,   430,    67,    67,   433,   433,
      67,    67,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,   447,    48,    49,    50,    67,    67,    65,    65,
      55,    69,    66,    58,    14,    60,    16,    17,    18,    64,
      70,    66,    66,    66,    66,    66,    65,    72,     1,    74,
       3,     4,     5,     6,    71,     8,     9,    10,    11,    12,
      67,    67,    27,    13,    69,    65,    19,    69,    69,    31,
      69,    65,    68,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    67,    48,    49,    50,    66,    13,
      67,    21,    55,    16,    17,    18,    70,    60,    65,    65,
      68,    64,    67,    66,     4,     4,     4,    59,   233,    72,
       1,    74,     3,     4,     5,     6,    71,     8,     9,    10,
      11,    12,    68,    68,   176,     4,   318,    36,    19,   411,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,   328,   390,   288,   387,   418,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,   159,    48,    49,    50,
     167,   185,   124,   249,    55,    62,    -1,    -1,    -1,    60,
      -1,    -1,    -1,    64,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    72,     1,    74,     3,     4,     5,     6,    -1,     8,
       9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      19,    20,    -1,    22,    23,    24,    -1,    -1,    27,    28,
      29,    30,    -1,    32,    33,    34,    -1,    -1,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    -1,    48,
      49,    50,    -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,
      -1,    60,    -1,    -1,    -1,    64,    65,    66,    -1,    -1,
      -1,    70,    71,    72,     1,    -1,     3,     4,     5,     6,
      -1,     8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    19,    20,    -1,    22,    -1,    -1,    -1,    -1,
      27,    28,    29,    30,    -1,    32,    33,    34,    -1,    -1,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    -1,    -1,    60,    -1,    -1,    -1,    64,    65,    66,
      -1,    -1,    -1,    70,    71,    72,     1,    -1,     3,     4,
       5,     6,    -1,     8,     9,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    19,    20,    -1,    22,    -1,    -1,
      -1,    -1,    27,    28,    29,    30,    -1,    32,    33,    34,
      -1,    -1,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    -1,    48,    49,    50,    -1,    -1,    -1,    -1,
      55,    -1,    -1,    -1,    -1,    60,    -1,    -1,    -1,    64,
      65,    66,    -1,    -1,    -1,    70,    71,    72,     1,    -1,
       3,     4,     5,     6,    -1,     8,     9,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    19,    20,    -1,    22,
      -1,    -1,    -1,    -1,    27,    28,    29,    30,    -1,    32,
      33,    34,    -1,    -1,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    -1,    48,    49,    50,    -1,    -1,
      -1,    -1,    55,    16,    17,    18,    -1,    60,    -1,    -1,
      -1,    64,    65,    66,    -1,    -1,    -1,    70,     1,    72,
       3,     4,     5,     6,    -1,     8,     9,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    19,    -1,    -1,    -1,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    -1,    -1,    -1,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    16,
      17,    18,    55,    -1,    -1,    -1,    -1,    60,    61,    -1,
      -1,    64,    -1,    66,    -1,    -1,    -1,    -1,     1,    72,
       3,     4,     5,     6,    -1,     8,     9,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    19,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    -1,    48,    49,    50,    -1,    -1,
      -1,    -1,    55,    16,    17,    18,    -1,    60,    -1,    -1,
      -1,    64,    -1,    66,    67,    -1,    -1,    -1,    71,    72,
       1,    -1,     3,     4,     5,     6,    -1,     8,     9,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    19,    -1,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    -1,    -1,    -1,    -1,    -1,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      16,    17,    18,    -1,    55,    -1,    16,    17,    18,    60,
      -1,    -1,    -1,    64,    65,    66,    -1,    -1,    -1,    -1,
       1,    72,     3,     4,     5,     6,    -1,     8,     9,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    19,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    58,    59,
      60,    61,    62,    63,    -1,    -1,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    -1,    48,    49,    50,
      -1,    16,    17,    18,    55,    -1,    -1,    -1,    -1,    60,
      16,    17,    18,    64,    -1,    66,    -1,    -1,    -1,    -1,
      71,    72,     1,    -1,     3,     4,     5,     6,    -1,     8,
       9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      19,    56,    57,    58,    59,    60,    61,    62,    63,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    -1,    48,
      49,    50,    16,    17,    18,    -1,    55,    -1,    -1,    -1,
      -1,    60,    61,    -1,    -1,    64,    -1,    66,    -1,    -1,
      -1,    -1,     1,    72,     3,     4,     5,     6,    -1,     8,
       9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      19,    -1,    -1,    -1,    58,    59,    60,    61,    62,    63,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    -1,    48,
      49,    50,    -1,    -1,    -1,    -1,    55,    -1,    -1,    58,
      -1,    60,    -1,    -1,    -1,    64,    -1,    66,    -1,    -1,
      -1,    -1,     1,    72,     3,     4,     5,     6,    -1,     8,
       9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      19,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    -1,    48,
      49,    50,    -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,
      -1,    60,    -1,    -1,    -1,    64,    -1,    66,    67,    -1,
      -1,    -1,     1,    72,     3,     4,     5,     6,    -1,     8,
       9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      19,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    -1,    48,
      49,    50,    -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,
      -1,    60,    -1,    -1,    -1,    64,    65,    66,    -1,    -1,
      -1,    -1,     1,    72,     3,     4,     5,     6,    -1,     8,
       9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      19,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    -1,    48,
      49,    50,    -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,
      -1,    60,    -1,    -1,    -1,    64,    65,    66,    -1,    -1,
      -1,    -1,     1,    72,     3,     4,     5,     6,    -1,     8,
       9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      19,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    -1,    48,
      49,    50,    -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,
      -1,    60,    -1,    -1,    -1,    64,    -1,    66,    67,    -1,
      -1,    -1,     1,    72,     3,     4,     5,     6,    -1,     8,
       9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      19,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    -1,    48,
      49,    50,     3,    -1,    -1,     6,    55,     8,     9,    10,
      -1,    60,    -1,    -1,    -1,    64,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    37,    38,    39,    40,
      41,    -1,    43,    -1,    -1,    -1,    -1,    48,    49,    50,
      14,    15,    16,    17,    18,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    72,    14,    15,    16,    17,    18,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      14,    15,    16,    17,    18,    69,    -1,    -1,    -1,    -1,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    14,    15,    16,    17,    18,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      14,    15,    16,    17,    18,    69,    -1,    -1,    -1,    -1,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    14,    15,    16,    17,    18,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      -1,    -1,    -1,    -1,    68,    14,    15,    16,    17,    18,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    -1,    -1,    -1,    -1,    68,    14,    15,    16,
      17,    18,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
      -1,    -1,    -1,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    -1,    -1,    -1,    67,    14,
      15,    16,    17,    18,    -1,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   433,    -1,    -1,    -1,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   471
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    76,    77,     0,     7,    79,    85,    88,    93,   102,
     103,   102,    65,    78,     6,    36,    47,    68,    96,    97,
     105,    61,    82,   106,   107,     3,    66,   161,   162,     8,
       9,    84,     8,     9,    84,    65,    69,   161,    59,    65,
       3,    70,    13,    86,    82,   106,    67,   161,    94,     1,
       4,     5,     6,     8,     9,    10,    11,    12,    19,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    48,
      49,    50,    55,    60,    64,    66,    72,    80,    81,    84,
     104,   137,   138,   146,   147,   148,   149,   153,   155,   156,
     157,   160,   162,   165,   172,   173,    66,    84,    91,    13,
      37,   151,    66,    37,    66,   146,   149,   146,   138,    84,
     138,   140,   141,   142,   138,   143,   144,   145,   136,   138,
     172,    66,   154,    66,   158,    66,     8,   138,   138,     6,
      47,    61,    97,   136,    66,    37,   138,   146,    14,    15,
      16,    17,    18,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    11,    12,    13,    68,    69,
      35,    73,    66,    66,     8,     9,    95,    97,    98,    99,
     100,   101,    71,    97,   138,    84,    66,   168,    48,    84,
      26,    71,    69,    68,    74,    69,    68,    69,   138,    66,
      70,   108,   152,   138,   152,    47,   167,     8,     9,   136,
      82,    67,   150,    84,   138,   138,   138,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     138,   138,    67,   142,    84,    58,   136,   166,   169,   137,
      82,    67,    26,    69,    92,   100,   140,    67,   141,   138,
      67,   145,    67,   138,    69,   136,   110,    69,     8,     9,
     140,    67,    67,   136,    68,    68,    69,    66,   136,    25,
      74,   140,   140,    95,    87,    99,    82,    89,    90,    67,
      67,   138,    67,     1,    20,    22,    27,    28,    29,    30,
      32,    33,    34,    65,    97,   108,   109,   116,   117,   118,
     120,   122,   126,   130,   136,   139,   174,   138,   164,   164,
      67,    66,    67,   138,    67,   170,    25,    74,    58,    74,
     136,    67,    67,     1,    65,    83,   108,    84,    69,    65,
     108,    69,   159,    65,    66,    66,    66,   121,    66,    66,
      65,    65,    65,   136,   111,    71,   116,    65,    69,    67,
      69,    67,   171,   140,    58,    74,   136,   136,    74,    90,
     146,    67,   136,   136,   136,   117,    98,   113,   114,   128,
     129,   136,     8,     9,   113,   124,   125,    65,    82,   112,
     115,   138,    84,   163,   140,    67,   136,    74,    74,   159,
      67,    67,    67,    27,    82,    13,    65,    69,    31,    95,
      69,    65,   159,    68,    67,    74,   117,   131,   119,    66,
      95,   138,   128,   124,   138,    13,   115,    67,   138,    21,
     175,    70,   117,   136,    65,    67,   138,   117,   110,    67,
     128,   127,    23,    24,   133,    65,    67,   117,     4,    19,
      25,    60,    64,    66,   134,   135,   161,    68,   117,   132,
     133,   123,     4,   134,     4,     4,   135,    25,    68,    16,
      17,    18,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,   132,    71,   132,   117,    68,    67,    68,
     134,    66,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   135,    68
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

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
#ifndef	YYINITDEPTH
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
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
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
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
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
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

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

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

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
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
  int yytoken;
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

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

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
      yychar = YYLEX;
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
  *++yyvsp = yylval;

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
     `$$ = $1'.

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

/* Line 1806 of yacc.c  */
#line 211 "grammar.y"
    {
#line 227 "grammar.y.pre"
	    comp_trees[TREE_MAIN] = (yyval.node);
	}
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 219 "grammar.y"
    {
#line 234 "grammar.y.pre"
	    CREATE_TWO_VALUES((yyval.node), 0, (yyvsp[(1) - (3)].node), (yyvsp[(2) - (3)].node));
	}
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 224 "grammar.y"
    {
#line 238 "grammar.y.pre"
	    (yyval.node) = 0;
	}
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 233 "grammar.y"
    {
#line 246 "grammar.y.pre"

		yywarn("Extra ';'. Ignored.");
	    }
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 243 "grammar.y"
    {
#line 255 "grammar.y.pre"
		object_t *ob;
		inherit_t inherit;
		int initializer;
		int acc_mod;
		
		(yyvsp[(1) - (4)].number) |= global_modifiers;

                acc_mod = ((yyvsp[(1) - (4)].number) & DECL_ACCESS) & ~global_modifiers;
		if (acc_mod & (acc_mod - 1)) {
		    char buf[256];
		    char *end = EndOf(buf);
		    char *p;
		    
		    p = strput(buf, end, "Multiple access modifiers (");
		    p = get_type_modifiers(p, end, (yyvsp[(1) - (4)].number));
		    p = strput(p, end, ") for inheritance");
		    yyerror(buf);
		}	       
		
		if (!((yyvsp[(1) - (4)].number) & DECL_ACCESS)) (yyvsp[(1) - (4)].number) |= DECL_PUBLIC;
#ifndef ALLOW_INHERIT_AFTER_FUNCTION
		if (func_present)
		    yyerror("Illegal to inherit after defining functions.");
#endif
		if (var_defined)
		    yyerror("Illegal to inherit after defining global variables.");
#ifndef ALLOW_INHERIT_AFTER_FUNCTION
		if (func_present || var_defined){
#else
		if (var_defined){
#endif
		  inherit_file = 0;
		  YYACCEPT;
		}
#ifdef NEVER
		} //stupid bison
#endif
		ob = find_object2((yyvsp[(3) - (4)].string));
		if (ob == 0) {
		    inherit_file = alloc_cstring((yyvsp[(3) - (4)].string), "inherit");
		    /* Return back to load_object() */
		    YYACCEPT;
		}
		scratch_free((yyvsp[(3) - (4)].string));
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
		inherit.type_mod = (yyvsp[(1) - (4)].number);
		add_to_mem_block(A_INHERITS, (char *)&inherit, sizeof inherit);

		/* The following has to come before copy_vars - Sym */
		copy_structures(ob->prog);
		copy_variables(ob->prog, (yyvsp[(1) - (4)].number));
		initializer = copy_functions(ob->prog, (yyvsp[(1) - (4)].number));
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
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 339 "grammar.y"
    {
#line 354 "grammar.y.pre"
		CREATE_REAL((yyval.node), (yyvsp[(1) - (1)].real));
	    }
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 347 "grammar.y"
    {
#line 361 "grammar.y.pre"
		CREATE_NUMBER((yyval.node), (yyvsp[(1) - (1)].number));
	    }
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 355 "grammar.y"
    {
#line 368 "grammar.y.pre"
		(yyval.number) = 0;
	    }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 360 "grammar.y"
    {
#line 372 "grammar.y.pre"
		(yyval.number) = TYPE_MOD_ARRAY;
	    }
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 368 "grammar.y"
    {
#line 379 "grammar.y.pre"
		(yyval.node) = (yyvsp[(1) - (1)].decl).node;
		if (!(yyval.node)) {
		    CREATE_RETURN((yyval.node), 0);
		}
            }
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 376 "grammar.y"
    {
#line 386 "grammar.y.pre"
		(yyval.node) = 0;
	    }
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 381 "grammar.y"
    {
#line 390 "grammar.y.pre"
		(yyval.node) = 0;
	    }
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 389 "grammar.y"
    {
#line 397 "grammar.y.pre"
		(yyval.string) = scratch_copy((yyvsp[(1) - (1)].ihe)->name);
	    }
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 398 "grammar.y"
    {
#line 405 "grammar.y.pre"
		int flags;
                int acc_mod;
                func_present = 1;
		flags = ((yyvsp[(1) - (3)].number) >> 16);
		
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
                (yyvsp[(1) - (3)].number) = (flags << 16) | ((yyvsp[(1) - (3)].number) & 0xffff);
		/* Handle type checking here so we know whether to typecheck
		   'argument' */
		if ((yyvsp[(1) - (3)].number) & 0xffff) {
		    exact_types = ((yyvsp[(1) - (3)].number)& 0xffff) | (yyvsp[(2) - (3)].number);
		} else {
		    if (pragmas & PRAGMA_STRICT_TYPES) {
			if (strcmp((yyvsp[(3) - (3)].string), "create") != 0)
			    yyerror("\"#pragma strict_types\" requires type of function");
			else
			    exact_types = TYPE_VOID; /* default for create() */
		    } else
			exact_types = 0;
		}
	    }
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 440 "grammar.y"
    {
#line 456 "grammar.y.pre"
		char *p = (yyvsp[(3) - (7)].string);
		(yyvsp[(3) - (7)].string) = make_shared_string((yyvsp[(3) - (7)].string));
		scratch_free(p);

		/* If we had nested functions, we would need to check */
		/* here if we have enough space for locals */
		
		/*
		 * Define a prototype. If it is a real function, then the
		 * prototype will be replaced below.
		 */

		(yyval.number) = FUNC_PROTOTYPE;
		if ((yyvsp[(6) - (7)].argument).flags & ARG_IS_VARARGS) {
		    (yyval.number) |= (FUNC_TRUE_VARARGS | FUNC_VARARGS);
		}
		(yyval.number) |= ((yyvsp[(1) - (7)].number) >> 16);

		define_new_function((yyvsp[(3) - (7)].string), (yyvsp[(6) - (7)].argument).num_arg, 0, (yyval.number), ((yyvsp[(1) - (7)].number) & 0xffff)| (yyvsp[(2) - (7)].number));
		/* This is safe since it is guaranteed to be in the
		   function table, so it can't be dangling */
		free_string((yyvsp[(3) - (7)].string)); 
		context = 0;
	    }
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 467 "grammar.y"
    {
#line 482 "grammar.y.pre"
		/* Either a prototype or a block */
		if ((yyvsp[(9) - (9)].node)) {
		    int fun;

		    (yyvsp[(8) - (9)].number) &= ~FUNC_PROTOTYPE;
		    if ((yyvsp[(9) - (9)].node)->kind != NODE_RETURN &&
			((yyvsp[(9) - (9)].node)->kind != NODE_TWO_VALUES
			 || (yyvsp[(9) - (9)].node)->r.expr->kind != NODE_RETURN)) {
			parse_node_t *replacement;
			CREATE_STATEMENTS(replacement, (yyvsp[(9) - (9)].node), 0);
			CREATE_RETURN(replacement->r.expr, 0);
			(yyvsp[(9) - (9)].node) = replacement;
		    }

		    fun = define_new_function((yyvsp[(3) - (9)].string), (yyvsp[(6) - (9)].argument).num_arg, 
					      max_num_locals - (yyvsp[(6) - (9)].argument).num_arg,
					      (yyvsp[(8) - (9)].number), ((yyvsp[(1) - (9)].number) & 0xffff) | (yyvsp[(2) - (9)].number));
		    if (fun != -1) {
			(yyval.node) = new_node_no_line();
			(yyval.node)->kind = NODE_FUNCTION;
			(yyval.node)->v.number = fun;
			(yyval.node)->l.number = max_num_locals;
			(yyval.node)->r.expr = (yyvsp[(9) - (9)].node);
		    } else 
			(yyval.node) = 0;
		} else
		    (yyval.node) = 0;
		free_all_local_names(!!(yyvsp[(9) - (9)].node));
	    }
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 499 "grammar.y"
    {
#line 513 "grammar.y.pre"
		if (!((yyvsp[(1) - (3)].number) & ~(DECL_MODS)) && (pragmas & PRAGMA_STRICT_TYPES))
		    yyerror("Missing type for global variable declaration");
		(yyval.node) = 0;
	    }
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 511 "grammar.y"
    {
#line 524 "grammar.y.pre"
		if (!(yyvsp[(1) - (2)].number)) 
		    yyerror("modifier list may not be empty.");
		
		if ((yyvsp[(1) - (2)].number) & FUNC_VARARGS) {
		    yyerror("Illegal modifier 'varargs' in global modifier list.");
		    (yyvsp[(1) - (2)].number) &= ~FUNC_VARARGS;
		}

		if (!((yyvsp[(1) - (2)].number) & DECL_ACCESS)) (yyvsp[(1) - (2)].number) |= DECL_PUBLIC;
		global_modifiers = (yyvsp[(1) - (2)].number);
		(yyval.node) = 0;
	    }
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 529 "grammar.y"
    {
#line 541 "grammar.y.pre"
		/* At this point, the current_type here is only a basic_type */
		/* and cannot be unused yet - Sym */
		
		if (current_type == TYPE_VOID)
		    yyerror("Illegal to declare class member of type void.");
		add_local_name((yyvsp[(2) - (2)].string), current_type | (yyvsp[(1) - (2)].number));
		scratch_free((yyvsp[(2) - (2)].string));
	    }
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 549 "grammar.y"
    {
#line 560 "grammar.y.pre"
	      current_type = (yyvsp[(2) - (2)].number);
	  }
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 558 "grammar.y"
    {
#line 568 "grammar.y.pre"
		ident_hash_elem_t *ihe;

		ihe = find_or_add_ident(
			   PROG_STRING((yyval.number) = store_prog_string((yyvsp[(3) - (4)].string))),
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

		    scratch_free((yyvsp[(3) - (4)].string));
		    (yyvsp[(2) - (4)].ihe) = 0;
		}
		else {
		    (yyvsp[(2) - (4)].ihe) = ihe;
		}
	    }
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 585 "grammar.y"
    {
#line 594 "grammar.y.pre"
		class_def_t *sd;
		class_member_entry_t *sme;
		int i, raise_error = 0;
		
		/* check for a redefinition */
		if ((yyvsp[(2) - (7)].ihe) != 0) {
		    sd = CLASS((yyvsp[(2) - (7)].ihe)->dn.class_num);
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
		    sd->classname = (yyvsp[(5) - (7)].number);

		    sme = (class_member_entry_t *)allocate_in_mem_block(A_CLASS_MEMBER, sizeof(class_member_entry_t) * current_number_of_locals);

		    while (i--) {
			sme[i].membername = store_prog_string(locals_ptr[i].ihe->name);
			sme[i].type = type_of_locals_ptr[i] & ~LOCAL_MODS;
		    }
		}

		free_all_local_names(0);
		(yyval.node) = 0;
	    }
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 640 "grammar.y"
    {
#line 648 "grammar.y.pre"
		if ((yyvsp[(1) - (1)].ihe)->dn.local_num != -1) {
		    char buff[256];
		    char *end = EndOf(buff);
		    char *p;
		    
		    p = strput(buff, end, "Illegal to redeclare local name '");
		    p = strput(p, end, (yyvsp[(1) - (1)].ihe)->name);
		    p = strput(p, end, "'");
		    yyerror(buff);
		}
		(yyval.string) = scratch_copy((yyvsp[(1) - (1)].ihe)->name);
	    }
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 659 "grammar.y"
    {
#line 666 "grammar.y.pre"
		if ((yyvsp[(2) - (2)].ihe)->dn.class_num == -1) {
		    char buf[256];
		    char *end = EndOf(buf);
		    char *p;
		    
		    p = strput(buf, end, "Undefined class '");
		    p = strput(p, end, (yyvsp[(2) - (2)].ihe)->name);
		    p = strput(p, end, "'");
		    yyerror(buf);
		    (yyval.number) = TYPE_ANY;
		} else {
		    (yyval.number) = (yyvsp[(2) - (2)].ihe)->dn.class_num | TYPE_MOD_CLASS;
		}
	    }
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 676 "grammar.y"
    {
#line 682 "grammar.y.pre"
		char buf[256];
		char *end = EndOf(buf);
		char *p;

		p = strput(buf, end, "Undefined class '");
		p = strput(p, end, (yyvsp[(2) - (2)].string));
		p = strput(p, end, "'");
		yyerror(buf);
		(yyval.number) = TYPE_ANY;
	    }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 698 "grammar.y"
    {
#line 719 "grammar.y.pre"
		(yyval.number) = (yyvsp[(1) - (2)].number) | LOCAL_MOD_REF;
	    }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 706 "grammar.y"
    {
#line 727 "grammar.y.pre"
                (yyval.number) = (yyvsp[(1) - (2)].number) | (yyvsp[(2) - (2)].number);
		if ((yyvsp[(1) - (2)].number) != TYPE_VOID)
		    add_local_name("", (yyvsp[(1) - (2)].number) | (yyvsp[(2) - (2)].number));
            }
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 713 "grammar.y"
    {
#line 733 "grammar.y.pre"
		if ((yyvsp[(1) - (3)].number) == TYPE_VOID)
		    yyerror("Illegal to declare argument of type void.");
                add_local_name((yyvsp[(3) - (3)].string), (yyvsp[(1) - (3)].number) | (yyvsp[(2) - (3)].number));
		scratch_free((yyvsp[(3) - (3)].string));
                (yyval.number) = (yyvsp[(1) - (3)].number) | (yyvsp[(2) - (3)].number);
	    }
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 722 "grammar.y"
    {
#line 741 "grammar.y.pre"
		if (exact_types) {
		    yyerror("Missing type for argument");
		}
		add_local_name((yyvsp[(1) - (1)].string), TYPE_ANY);
		scratch_free((yyvsp[(1) - (1)].string));
		(yyval.number) = TYPE_ANY;
            }
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 735 "grammar.y"
    {
#line 753 "grammar.y.pre"
		(yyval.argument).num_arg = 0;
                (yyval.argument).flags = 0;
	    }
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 742 "grammar.y"
    {
#line 759 "grammar.y.pre"
		int x = type_of_locals_ptr[max_num_locals-1];
		int lt = x & ~LOCAL_MODS;
		
		(yyval.argument) = (yyvsp[(1) - (2)].argument);
		(yyval.argument).flags |= ARG_IS_VARARGS;

		if (x & LOCAL_MOD_REF) {
		    yyerror("Variable to hold remainder of args may not be a reference");
		    x &= ~LOCAL_MOD_REF;
		}
		if (lt != TYPE_ANY && !(lt & TYPE_MOD_ARRAY))
		    yywarn("Variable to hold remainder of arguments should be an array.");
	    }
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 761 "grammar.y"
    {
#line 777 "grammar.y.pre"
		if (((yyvsp[(1) - (1)].number) & TYPE_MASK) == TYPE_VOID && !((yyvsp[(1) - (1)].number) & TYPE_MOD_CLASS)) {
		    if ((yyvsp[(1) - (1)].number) & ~TYPE_MASK)
			yyerror("Illegal to declare argument of type void.");
		    (yyval.argument).num_arg = 0;
		} else {
		    (yyval.argument).num_arg = 1;
		}
                (yyval.argument).flags = 0;
	    }
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 773 "grammar.y"
    {
#line 788 "grammar.y.pre"
		if (!(yyval.argument).num_arg)    /* first arg was void w/no name */
		    yyerror("argument of type void must be the only argument.");
		if (((yyvsp[(3) - (3)].number) & TYPE_MASK) == TYPE_VOID && !((yyvsp[(3) - (3)].number) & TYPE_MOD_CLASS))
		    yyerror("Illegal to declare argument of type void.");

                (yyval.argument) = (yyvsp[(1) - (3)].argument);
		(yyval.argument).num_arg++;
	    }
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 787 "grammar.y"
    {
#line 801 "grammar.y.pre"
		(yyval.number) = 0;
	    }
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 792 "grammar.y"
    {
#line 805 "grammar.y.pre"
		int acc_mod;
		
		(yyval.number) = (yyvsp[(1) - (2)].number) | (yyvsp[(2) - (2)].number);
		
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
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 815 "grammar.y"
    {
#line 831 "grammar.y.pre"
		(yyval.number) = ((yyvsp[(1) - (2)].number) << 16) | (yyvsp[(2) - (2)].number);
		current_type = (yyval.number);
	    }
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 824 "grammar.y"
    {
#line 839 "grammar.y.pre"
		(yyval.number) = (yyvsp[(2) - (4)].number) | (yyvsp[(3) - (4)].number);
	    }
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 833 "grammar.y"
    {
#line 847 "grammar.y.pre"
		(yyval.number) = TYPE_UNKNOWN;
	    }
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 846 "grammar.y"
    {
#line 859 "grammar.y.pre"
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

		define_new_variable((yyvsp[(2) - (2)].string), current_type | (yyvsp[(1) - (2)].number));
		scratch_free((yyvsp[(2) - (2)].string));
	    }
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 872 "grammar.y"
    {
#line 884 "grammar.y.pre"
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

		if ((yyvsp[(3) - (4)].number) != F_ASSIGN)
		    yyerror("Only '=' is legal in initializers.");

		/* ignore current_type == 0, which gets a missing type error
		   later anyway */
		if (current_type) {
		    type = (current_type | (yyvsp[(1) - (4)].number)) & ~DECL_MODS;
		    if ((current_type & ~DECL_MODS) == TYPE_VOID)
			yyerror("Illegal to declare global variable of type void.");
		    if (!compatible_types(type, (yyvsp[(4) - (4)].node)->type)) {
			char buff[256];
			char *end = EndOf(buff);
			char *p;
			
			p = strput(buff, end, "Type mismatch ");
			p = get_two_types(p, end, type, (yyvsp[(4) - (4)].node)->type);
			p = strput(p, end, " when initializing ");
			p = strput(p, end, (yyvsp[(2) - (4)].string));
			yyerror(buff);
		    }
		} else type = 0;
		(yyvsp[(4) - (4)].node) = do_promotions((yyvsp[(4) - (4)].node), type);

		CREATE_BINARY_OP(expr, F_VOID_ASSIGN, 0, (yyvsp[(4) - (4)].node), 0);
		CREATE_OPCODE_1(expr->r.expr, F_GLOBAL_LVALUE, 0,
				define_new_variable((yyvsp[(2) - (4)].string), current_type | (yyvsp[(1) - (4)].number)));
		newnode = comp_trees[TREE_INIT];
		CREATE_TWO_VALUES(comp_trees[TREE_INIT], 0,
				  newnode, expr);
		scratch_free((yyvsp[(2) - (4)].string));
	    }
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 928 "grammar.y"
    {
#line 939 "grammar.y.pre"
		if ((yyvsp[(2) - (4)].decl).node && (yyvsp[(3) - (4)].node)) {
		    CREATE_STATEMENTS((yyval.decl).node, (yyvsp[(2) - (4)].decl).node, (yyvsp[(3) - (4)].node));
		} else (yyval.decl).node = ((yyvsp[(2) - (4)].decl).node ? (yyvsp[(2) - (4)].decl).node : (yyvsp[(3) - (4)].node));
                (yyval.decl).num = (yyvsp[(2) - (4)].decl).num;
            }
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 941 "grammar.y"
    {
#line 951 "grammar.y.pre"
                (yyval.decl).node = 0;
                (yyval.decl).num = 0;
            }
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 947 "grammar.y"
    {
#line 956 "grammar.y.pre"
		if ((yyvsp[(2) - (2)].number) == TYPE_VOID)
		    yyerror("Illegal to declare local variable of type void.");
                /* can't do this in basic_type b/c local_name_list contains
                 * expr0 which contains cast which contains basic_type
                 */
                current_type = (yyvsp[(2) - (2)].number);
            }
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 957 "grammar.y"
    {
#line 965 "grammar.y.pre"
                if ((yyvsp[(1) - (5)].decl).node && (yyvsp[(4) - (5)].decl).node) {
		    CREATE_STATEMENTS((yyval.decl).node, (yyvsp[(1) - (5)].decl).node, (yyvsp[(4) - (5)].decl).node);
                } else (yyval.decl).node = ((yyvsp[(1) - (5)].decl).node ? (yyvsp[(1) - (5)].decl).node : (yyvsp[(4) - (5)].decl).node);
                (yyval.decl).num = (yyvsp[(1) - (5)].decl).num + (yyvsp[(4) - (5)].decl).num;
            }
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 969 "grammar.y"
    {
#line 976 "grammar.y.pre"
		if (current_type & LOCAL_MOD_REF) {
		    yyerror("Illegal to declare local variable as reference");
		    current_type &= ~LOCAL_MOD_REF;
		}
		add_local_name((yyvsp[(2) - (2)].string), current_type | (yyvsp[(1) - (2)].number) | LOCAL_MOD_UNUSED);

		scratch_free((yyvsp[(2) - (2)].string));
		(yyval.node) = 0;
	    }
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 981 "grammar.y"
    {
#line 987 "grammar.y.pre"
		int type = (current_type | (yyvsp[(1) - (4)].number)) & ~DECL_MODS;

		if (current_type & LOCAL_MOD_REF) {
		    yyerror("Illegal to declare local variable as reference");
		    current_type &= ~LOCAL_MOD_REF;
		    type &= ~LOCAL_MOD_REF;
		}

		if ((yyvsp[(3) - (4)].number) != F_ASSIGN)
		    yyerror("Only '=' is allowed in initializers.");
		if (!compatible_types((yyvsp[(4) - (4)].node)->type, type)) {
		    char buff[256];
		    char *end = EndOf(buff);
		    char *p;
		    
		    p = strput(buff, end, "Type mismatch ");
		    p = get_two_types(p, end, type, (yyvsp[(4) - (4)].node)->type);
		    p = strput(p, end, " when initializing ");
		    p = strput(p, end, (yyvsp[(2) - (4)].string));

		    yyerror(buff);
		}
		
		(yyvsp[(4) - (4)].node) = do_promotions((yyvsp[(4) - (4)].node), type);

		CREATE_UNARY_OP_1((yyval.node), F_VOID_ASSIGN_LOCAL, 0, (yyvsp[(4) - (4)].node),
				  add_local_name((yyvsp[(2) - (4)].string), current_type | (yyvsp[(1) - (4)].number) | LOCAL_MOD_UNUSED));
		scratch_free((yyvsp[(2) - (4)].string));
	    }
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 1016 "grammar.y"
    {
#line 1021 "grammar.y.pre"
		if ((yyvsp[(1) - (3)].number) == TYPE_VOID)
		    yyerror("Illegal to declare local variable of type void.");

		(yyval.number) = add_local_name((yyvsp[(3) - (3)].string), (yyvsp[(1) - (3)].number) | (yyvsp[(2) - (3)].number));
		scratch_free((yyvsp[(3) - (3)].string));
	    }
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 1028 "grammar.y"
    {
#line 1032 "grammar.y.pre"
                int type = type_of_locals_ptr[(yyvsp[(1) - (3)].number)];

		if (type & LOCAL_MOD_REF) {
		    yyerror("Illegal to declare local variable as reference");
		    type_of_locals_ptr[(yyvsp[(1) - (3)].number)] &= ~LOCAL_MOD_REF;
		}
		type &= ~LOCAL_MODS;

		if ((yyvsp[(2) - (3)].number) != F_ASSIGN)
		    yyerror("Only '=' is allowed in initializers.");
		if (!compatible_types((yyvsp[(3) - (3)].node)->type, type)) {
		    char buff[256];
		    char *end = EndOf(buff);
		    char *p;
		    
		    p = strput(buff, end, "Type mismatch ");
		    p = get_two_types(p, end, type, (yyvsp[(3) - (3)].node)->type);
		    p = strput(p, end, " when initializing.");
		    yyerror(buff);
		}

		(yyvsp[(3) - (3)].node) = do_promotions((yyvsp[(3) - (3)].node), type);

		/* this is an expression */
		CREATE_BINARY_OP((yyval.node), F_ASSIGN, 0, (yyvsp[(3) - (3)].node), 0);
                CREATE_OPCODE_1((yyval.node)->r.expr, F_LOCAL_LVALUE, 0, (yyvsp[(1) - (3)].number));
	    }
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 1061 "grammar.y"
    {
#line 1064 "grammar.y.pre"
                (yyval.decl).node = (yyvsp[(1) - (1)].node);
                (yyval.decl).num = 1;
            }
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 1067 "grammar.y"
    {
#line 1069 "grammar.y.pre"
                if ((yyvsp[(1) - (3)].node) && (yyvsp[(3) - (3)].decl).node) {
		    CREATE_STATEMENTS((yyval.decl).node, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].decl).node);
                } else (yyval.decl).node = ((yyvsp[(1) - (3)].node) ? (yyvsp[(1) - (3)].node) : (yyvsp[(3) - (3)].decl).node);
                (yyval.decl).num = 1 + (yyvsp[(3) - (3)].decl).num;
            }
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 1078 "grammar.y"
    {
#line 1079 "grammar.y.pre"
		(yyval.node) = 0;
	    }
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 1083 "grammar.y"
    {
#line 1083 "grammar.y.pre"
		if ((yyvsp[(1) - (2)].node) && (yyvsp[(2) - (2)].node)) {
		    CREATE_STATEMENTS((yyval.node), (yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));
		} else (yyval.node) = ((yyvsp[(1) - (2)].node) ? (yyvsp[(1) - (2)].node) : (yyvsp[(2) - (2)].node));
            }
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 1090 "grammar.y"
    {
#line 1089 "grammar.y.pre"
		(yyval.node) = 0;
            }
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 1098 "grammar.y"
    {
#line 1096 "grammar.y.pre"
		(yyval.node) = pop_value((yyvsp[(1) - (2)].node));
#ifdef DEBUG
		{
		    parse_node_t *replacement;
		    CREATE_STATEMENTS(replacement, (yyval.node), 0);
		    CREATE_OPCODE(replacement->r.expr, F_BREAK_POINT, 0);
		    (yyval.node) = replacement;
		}
#endif
	    }
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 1116 "grammar.y"
    {
#line 1113 "grammar.y.pre"
                (yyval.node) = (yyvsp[(1) - (1)].decl).node;
                pop_n_locals((yyvsp[(1) - (1)].decl).num);
            }
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 1122 "grammar.y"
    {
#line 1118 "grammar.y.pre"
		(yyval.node) = 0;
	    }
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 1127 "grammar.y"
    {
#line 1122 "grammar.y.pre"
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
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 1150 "grammar.y"
    {
#line 1144 "grammar.y.pre"
		if (context & SPECIAL_CONTEXT)
		    yyerror("Cannot continue out of catch { } or time_expression { }");
		else
		if (!(context & LOOP_CONTEXT))
		    yyerror("continue statement outside loop");
		CREATE_CONTROL_JUMP((yyval.node), CJ_CONTINUE);
	    }
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 1163 "grammar.y"
    {
#line 1156 "grammar.y.pre"
		(yyvsp[(1) - (4)].number) = context;
		context = LOOP_CONTEXT;
	    }
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 1169 "grammar.y"
    {
#line 1161 "grammar.y.pre"
		CREATE_LOOP((yyval.node), 1, (yyvsp[(6) - (6)].node), 0, optimize_loop_test((yyvsp[(3) - (6)].node)));
		context = (yyvsp[(1) - (6)].number);
	    }
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 1178 "grammar.y"
    {
#line 1169 "grammar.y.pre"
		(yyvsp[(1) - (1)].number) = context;
		context = LOOP_CONTEXT;
	    }
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 1184 "grammar.y"
    {
#line 1174 "grammar.y.pre"
		CREATE_LOOP((yyval.node), 0, (yyvsp[(3) - (8)].node), 0, optimize_loop_test((yyvsp[(6) - (8)].node)));
		context = (yyvsp[(1) - (8)].number);
	    }
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 1193 "grammar.y"
    {
#line 1182 "grammar.y.pre"
		(yyvsp[(3) - (8)].decl).node = pop_value((yyvsp[(3) - (8)].decl).node);
		(yyvsp[(1) - (8)].number) = context;
		context = LOOP_CONTEXT;
	    }
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 1200 "grammar.y"
    {
#line 1188 "grammar.y.pre"
		(yyval.decl).num = (yyvsp[(3) - (10)].decl).num; /* number of declarations (0/1) */
		
		(yyvsp[(7) - (10)].node) = pop_value((yyvsp[(7) - (10)].node));
		if ((yyvsp[(7) - (10)].node) && IS_NODE((yyvsp[(7) - (10)].node), NODE_UNARY_OP, F_INC)
		    && IS_NODE((yyvsp[(7) - (10)].node)->r.expr, NODE_OPCODE_1, F_LOCAL_LVALUE)) {
		    LPC_INT lvar = (yyvsp[(7) - (10)].node)->r.expr->l.number;
		    CREATE_OPCODE_1((yyvsp[(7) - (10)].node), F_LOOP_INCR, 0, lvar);
		}

		CREATE_STATEMENTS((yyval.decl).node, (yyvsp[(3) - (10)].decl).node, 0);
		CREATE_LOOP((yyval.decl).node->r.expr, 1, (yyvsp[(10) - (10)].node), (yyvsp[(7) - (10)].node), optimize_loop_test((yyvsp[(5) - (10)].node)));

		context = (yyvsp[(1) - (10)].number);
	      }
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 1219 "grammar.y"
    {
#line 1206 "grammar.y.pre"
		if ((yyvsp[(1) - (1)].ihe)->dn.local_num != -1) {
		    CREATE_OPCODE_1((yyval.decl).node, F_LOCAL_LVALUE, 0, (yyvsp[(1) - (1)].ihe)->dn.local_num);
		    type_of_locals_ptr[(yyvsp[(1) - (1)].ihe)->dn.local_num] &= ~LOCAL_MOD_UNUSED;
		} else
		if ((yyvsp[(1) - (1)].ihe)->dn.global_num != -1) {
		    CREATE_OPCODE_1((yyval.decl).node, F_GLOBAL_LVALUE, 0, (yyvsp[(1) - (1)].ihe)->dn.global_num);
		} else {
		    char buf[256];
		    char *end = EndOf(buf);
		    char *p;

		    p = strput(buf, end, "'");
		    p = strput(p, end, (yyvsp[(1) - (1)].ihe)->name);
		    p = strput(p, end, "' is not a local or a global variable.");
		    yyerror(buf);
		    CREATE_OPCODE_1((yyval.decl).node, F_GLOBAL_LVALUE, 0, 0);
		}
		(yyval.decl).num = 0;
	    }
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 1241 "grammar.y"
    {
#line 1227 "grammar.y.pre"
		if (type_of_locals_ptr[(yyvsp[(1) - (1)].number)] & LOCAL_MOD_REF) {
		    CREATE_OPCODE_1((yyval.decl).node, F_REF_LVALUE, 0, (yyvsp[(1) - (1)].number));
		} else {
		    CREATE_OPCODE_1((yyval.decl).node, F_LOCAL_LVALUE, 0, (yyvsp[(1) - (1)].number));
		    type_of_locals_ptr[(yyvsp[(1) - (1)].number)] &= ~LOCAL_MOD_UNUSED;
		}
		(yyval.decl).num = 1;
            }
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 1252 "grammar.y"
    {
#line 1237 "grammar.y.pre"
		char buf[256];
		char *end = EndOf(buf);
		char *p;
		
		p = strput(buf, end, "'");
		p = strput(p, end, (yyvsp[(1) - (1)].string));
		p = strput(p, end, "' is not a local or a global variable.");
		yyerror(buf);
		CREATE_OPCODE_1((yyval.decl).node, F_GLOBAL_LVALUE, 0, 0);
		scratch_free((yyvsp[(1) - (1)].string));
		(yyval.decl).num = 0;
	    }
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 1270 "grammar.y"
    {
#line 1254 "grammar.y.pre"
		CREATE_FOREACH((yyval.decl).node, (yyvsp[(1) - (1)].decl).node, 0);
		(yyval.decl).num = (yyvsp[(1) - (1)].decl).num;
            }
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 1276 "grammar.y"
    {
#line 1259 "grammar.y.pre"
		CREATE_FOREACH((yyval.decl).node, (yyvsp[(1) - (3)].decl).node, (yyvsp[(3) - (3)].decl).node);
		(yyval.decl).num = (yyvsp[(1) - (3)].decl).num + (yyvsp[(3) - (3)].decl).num;
		if ((yyvsp[(1) - (3)].decl).node->v.number == F_REF_LVALUE)
		    yyerror("Mapping key may not be a reference in foreach()");
            }
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 1287 "grammar.y"
    {
#line 1269 "grammar.y.pre"
		(yyvsp[(3) - (6)].decl).node->v.expr = (yyvsp[(5) - (6)].node);
		(yyvsp[(1) - (6)].number) = context;
		context = LOOP_CONTEXT | LOOP_FOREACH;
            }
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 1294 "grammar.y"
    {
#line 1275 "grammar.y.pre"
		(yyval.decl).num = (yyvsp[(3) - (8)].decl).num;

		CREATE_STATEMENTS((yyval.decl).node, (yyvsp[(3) - (8)].decl).node, 0);
		CREATE_LOOP((yyval.decl).node->r.expr, 2, (yyvsp[(8) - (8)].node), 0, 0);
		CREATE_OPCODE((yyval.decl).node->r.expr->r.expr, F_NEXT_FOREACH, 0);
		
		context = (yyvsp[(1) - (8)].number);
	    }
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 1308 "grammar.y"
    {
#line 1288 "grammar.y.pre"
		(yyval.node) = 0;
	    }
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 1317 "grammar.y"
    {
#line 1296 "grammar.y.pre"
	 	(yyval.decl).node = (yyvsp[(1) - (1)].node);
		(yyval.decl).num = 0;
	    }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 1323 "grammar.y"
    {
#line 1301 "grammar.y.pre"
		(yyval.decl).node = (yyvsp[(1) - (1)].node);
		(yyval.decl).num = 1;
	    }
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 1332 "grammar.y"
    {
#line 1309 "grammar.y.pre"
                (yyvsp[(1) - (4)].number) = context;
                context &= LOOP_CONTEXT;
                context |= SWITCH_CONTEXT;
                (yyvsp[(2) - (4)].number) = mem_block[A_CASES].current_size;
            }
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 1340 "grammar.y"
    {
#line 1316 "grammar.y.pre"
                parse_node_t *node1, *node2;

                if ((yyvsp[(9) - (10)].node)) {
		    CREATE_STATEMENTS(node1, (yyvsp[(8) - (10)].node), (yyvsp[(9) - (10)].node));
                } else node1 = (yyvsp[(8) - (10)].node);

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

                node2->l.expr = (yyvsp[(3) - (10)].node);
                node2->r.expr = node1;
                prepare_cases(node2, (yyvsp[(2) - (10)].number));
                context = (yyvsp[(1) - (10)].number);
		(yyval.node) = node2;
		pop_n_locals((yyvsp[(7) - (10)].decl).num);
            }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 1373 "grammar.y"
    {
#line 1348 "grammar.y.pre"
               if ((yyvsp[(2) - (2)].node)){
		   CREATE_STATEMENTS((yyval.node), (yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));
               } else (yyval.node) = (yyvsp[(1) - (2)].node);
           }
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 1380 "grammar.y"
    {
#line 1354 "grammar.y.pre"
               if ((yyvsp[(2) - (2)].node)){
		   CREATE_STATEMENTS((yyval.node), (yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));
               } else (yyval.node) = (yyvsp[(1) - (2)].node);
           }
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 1387 "grammar.y"
    {
#line 1360 "grammar.y.pre"
               (yyval.node) = 0;
           }
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 1396 "grammar.y"
    {
#line 1368 "grammar.y.pre"
                (yyval.node) = (yyvsp[(2) - (3)].node);
                (yyval.node)->v.expr = 0;

                add_to_mem_block(A_CASES, (char *)&((yyvsp[(2) - (3)].node)), sizeof((yyvsp[(2) - (3)].node)));
            }
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 1404 "grammar.y"
    {
#line 1375 "grammar.y.pre"
                if ( (yyvsp[(2) - (5)].node)->kind != NODE_CASE_NUMBER
                    || (yyvsp[(4) - (5)].node)->kind != NODE_CASE_NUMBER )
                    yyerror("String case labels not allowed as range bounds");
                if ((yyvsp[(2) - (5)].node)->r.number > (yyvsp[(4) - (5)].node)->r.number) break;

		context |= SWITCH_RANGES;

                (yyval.node) = (yyvsp[(2) - (5)].node);
                (yyval.node)->v.expr = (yyvsp[(4) - (5)].node);

                add_to_mem_block(A_CASES, (char *)&((yyvsp[(2) - (5)].node)), sizeof((yyvsp[(2) - (5)].node)));
            }
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 1419 "grammar.y"
    {
#line 1389 "grammar.y.pre"
	        if ( (yyvsp[(2) - (4)].node)->kind != NODE_CASE_NUMBER )
                    yyerror("String case labels not allowed as range bounds");

		context |= SWITCH_RANGES;

                (yyval.node) = (yyvsp[(2) - (4)].node);
                (yyval.node)->v.expr = new_node();
		(yyval.node)->v.expr->kind = NODE_CASE_NUMBER;
		(yyval.node)->v.expr->r.number = LPC_INT_MAX; //maxint

                add_to_mem_block(A_CASES, (char *)&((yyvsp[(2) - (4)].node)), sizeof((yyvsp[(2) - (4)].node)));
            }
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 1434 "grammar.y"
    {
#line 1403 "grammar.y.pre"
	      if ( (yyvsp[(3) - (4)].node)->kind != NODE_CASE_NUMBER )
                    yyerror("String case labels not allowed as range bounds");

		context |= SWITCH_RANGES;
		(yyval.node) = new_node();
		(yyval.node)->kind = NODE_CASE_NUMBER;
                (yyval.node)->r.number = LPC_INT_MIN; //maxint +1 wraps to min_int, on all computers i know, just not in the C standard iirc 
                (yyval.node)->v.expr = (yyvsp[(3) - (4)].node);

                add_to_mem_block(A_CASES, (char *)&((yyval.node)), sizeof((yyval.node)));
            }
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 1448 "grammar.y"
    {
#line 1416 "grammar.y.pre"
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
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 1465 "grammar.y"
    {
#line 1432 "grammar.y.pre"
                if ((context & SWITCH_STRINGS) && (yyvsp[(1) - (1)].number))
                    yyerror("Mixed case label list not allowed");

                if ((yyvsp[(1) - (1)].number))
		  context |= SWITCH_NUMBERS;
		else
		  context |= SWITCH_NOT_EMPTY;

		(yyval.node) = new_node();
		(yyval.node)->kind = NODE_CASE_NUMBER;
                (yyval.node)->r.number = (LPC_INT)(yyvsp[(1) - (1)].number);
            }
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 1480 "grammar.y"
    {
#line 1446 "grammar.y.pre"
		POINTER_INT str;
		str = store_prog_string((yyvsp[(1) - (1)].string));
                scratch_free((yyvsp[(1) - (1)].string));
                if (context & SWITCH_NUMBERS)
                    yyerror("Mixed case label list not allowed");
                context |= SWITCH_STRINGS;
		(yyval.node) = new_node();
		(yyval.node)->kind = NODE_CASE_STRING;
                (yyval.node)->r.number = (LPC_INT)str;
            }
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 1496 "grammar.y"
    {
#line 1461 "grammar.y.pre"
                (yyval.number) = (yyvsp[(1) - (3)].number) | (yyvsp[(3) - (3)].number);
            }
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 1501 "grammar.y"
    {
#line 1465 "grammar.y.pre"
                (yyval.number) = (yyvsp[(1) - (3)].number) ^ (yyvsp[(3) - (3)].number);
            }
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 1506 "grammar.y"
    {
#line 1469 "grammar.y.pre"
                (yyval.number) = (yyvsp[(1) - (3)].number) & (yyvsp[(3) - (3)].number);
            }
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 1511 "grammar.y"
    {
#line 1473 "grammar.y.pre"
                (yyval.number) = (yyvsp[(1) - (3)].number) == (yyvsp[(3) - (3)].number);
            }
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 1516 "grammar.y"
    {
#line 1477 "grammar.y.pre"
                (yyval.number) = (yyvsp[(1) - (3)].number) != (yyvsp[(3) - (3)].number);
            }
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 1521 "grammar.y"
    {
#line 1481 "grammar.y.pre"
                switch((yyvsp[(2) - (3)].number)){
                    case F_GE: (yyval.number) = (yyvsp[(1) - (3)].number) >= (yyvsp[(3) - (3)].number); break;
                    case F_LE: (yyval.number) = (yyvsp[(1) - (3)].number) <= (yyvsp[(3) - (3)].number); break;
                    case F_GT: (yyval.number) = (yyvsp[(1) - (3)].number) >  (yyvsp[(3) - (3)].number); break;
                }
            }
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 1530 "grammar.y"
    {
#line 1489 "grammar.y.pre"
                (yyval.number) = (yyvsp[(1) - (3)].number) < (yyvsp[(3) - (3)].number);
            }
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 1535 "grammar.y"
    {
#line 1493 "grammar.y.pre"
                (yyval.number) = (yyvsp[(1) - (3)].number) << (yyvsp[(3) - (3)].number);
            }
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 1540 "grammar.y"
    {
#line 1497 "grammar.y.pre"
                (yyval.number) = (yyvsp[(1) - (3)].number) >> (yyvsp[(3) - (3)].number);
            }
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 1545 "grammar.y"
    {
#line 1501 "grammar.y.pre"
                (yyval.number) = (yyvsp[(1) - (3)].number) + (yyvsp[(3) - (3)].number);
            }
    break;

  case 124:

/* Line 1806 of yacc.c  */
#line 1550 "grammar.y"
    {
#line 1505 "grammar.y.pre"
                (yyval.number) = (yyvsp[(1) - (3)].number) - (yyvsp[(3) - (3)].number);
            }
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 1555 "grammar.y"
    {
#line 1509 "grammar.y.pre"
                (yyval.number) = (yyvsp[(1) - (3)].number) * (yyvsp[(3) - (3)].number);
            }
    break;

  case 126:

/* Line 1806 of yacc.c  */
#line 1560 "grammar.y"
    {
#line 1513 "grammar.y.pre"
                if ((yyvsp[(3) - (3)].number)) (yyval.number) = (yyvsp[(1) - (3)].number) % (yyvsp[(3) - (3)].number); else yyerror("Modulo by zero");
            }
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 1565 "grammar.y"
    {
#line 1517 "grammar.y.pre"
                if ((yyvsp[(3) - (3)].number)) (yyval.number) = (yyvsp[(1) - (3)].number) / (yyvsp[(3) - (3)].number); else yyerror("Division by zero");
            }
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 1570 "grammar.y"
    {
#line 1521 "grammar.y.pre"
                (yyval.number) = (yyvsp[(2) - (3)].number);
            }
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 1575 "grammar.y"
    {
#line 1525 "grammar.y.pre"
		(yyval.number) = (yyvsp[(1) - (1)].number);
	    }
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 1580 "grammar.y"
    {
#line 1529 "grammar.y.pre"
                (yyval.number) = -(yyvsp[(2) - (2)].number);
            }
    break;

  case 131:

/* Line 1806 of yacc.c  */
#line 1585 "grammar.y"
    {
#line 1533 "grammar.y.pre"
                (yyval.number) = !(yyvsp[(2) - (2)].number);
            }
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 1590 "grammar.y"
    {
#line 1537 "grammar.y.pre"
                (yyval.number) = ~(yyvsp[(2) - (2)].number);
            }
    break;

  case 133:

/* Line 1806 of yacc.c  */
#line 1598 "grammar.y"
    {
#line 1544 "grammar.y.pre"
		(yyval.node) = (yyvsp[(1) - (1)].node);
	    }
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 1603 "grammar.y"
    {
#line 1548 "grammar.y.pre"
		CREATE_TWO_VALUES((yyval.node), (yyvsp[(3) - (3)].node)->type, pop_value((yyvsp[(1) - (3)].node)), (yyvsp[(3) - (3)].node));
	    }
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 1616 "grammar.y"
    {
#line 1565 "grammar.y.pre"
		int op;

		if (!(context & ARG_LIST))
		    yyerror("ref illegal outside function argument list");
		else
		    num_refs++;
		
		switch ((yyvsp[(2) - (2)].node)->kind) {
		case NODE_PARAMETER_LVALUE:
		    op = F_LOCAL_LVALUE;
		    break;
		case NODE_TERNARY_OP:
		case NODE_OPCODE_1:
		case NODE_UNARY_OP_1:
		case NODE_BINARY_OP:
		    op = (yyvsp[(2) - (2)].node)->v.number;
		    if (op > F_RINDEX_LVALUE) 
			yyerror("Illegal to make reference to range");
		    break;
		default:
		    op=0; //0 is harmless, i hope
		    yyerror("unknown lvalue kind");
		}
		CREATE_UNARY_OP_1((yyval.node), F_MAKE_REF, TYPE_ANY, (yyvsp[(2) - (2)].node), op);
	    }
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 1645 "grammar.y"
    {
#line 1594 "grammar.y.pre"
	        parse_node_t *l = (yyvsp[(1) - (3)].node), *r = (yyvsp[(3) - (3)].node);
		/* set this up here so we can change it below */
		/* assignments are backwards; rhs is evaluated before
		   lhs, so put the RIGHT hand side on the LEFT hand
		   side of the tree node. */
		CREATE_BINARY_OP((yyval.node), (yyvsp[(2) - (3)].number), r->type, r, l);

    /* allow TYPE_STRING += TYPE_NUMBER | TYPE_OBJECT */
		if (exact_types && !compatible_types(r->type, l->type) &&
		    !((yyvsp[(2) - (3)].number) == F_ADD_EQ && l->type == TYPE_STRING &&
        ((COMP_TYPE(r->type, TYPE_NUMBER)) || r->type == TYPE_OBJECT))) {
		    char buf[256];
		    char *end = EndOf(buf);
		    char *p;
		    p = strput(buf, end, "Bad assignment ");
		    p = get_two_types(p, end, l->type, r->type);
		    p = strput(p, end, ".");
		    yyerror(buf);
		}
		
		if ((yyvsp[(2) - (3)].number) == F_ASSIGN)
		    (yyval.node)->l.expr = do_promotions(r, l->type);
	    }
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 1671 "grammar.y"
    {
#line 1619 "grammar.y.pre"
		yyerror("Illegal LHS");
		CREATE_ERROR((yyval.node));
	    }
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 1677 "grammar.y"
    {
#line 1624 "grammar.y.pre"
		parse_node_t *p1 = (yyvsp[(3) - (5)].node), *p2 = (yyvsp[(5) - (5)].node);

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
		if (IS_NODE((yyvsp[(1) - (5)].node), NODE_UNARY_OP, F_NOT)) {
		    /* !a ? b : c  --> a ? c : b */
		    CREATE_IF((yyval.node), (yyvsp[(1) - (5)].node)->r.expr, p2, p1);
		} else {
		    CREATE_IF((yyval.node), (yyvsp[(1) - (5)].node), p1, p2);
		}
		(yyval.node)->type = ((p1->type == p2->type) ? p1->type : TYPE_ANY);
	    }
    break;

  case 141:

/* Line 1806 of yacc.c  */
#line 1702 "grammar.y"
    {
#line 1648 "grammar.y.pre"
		CREATE_LAND_LOR((yyval.node), F_LOR, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
		if (IS_NODE((yyvsp[(1) - (3)].node), NODE_LAND_LOR, F_LOR))
		    (yyvsp[(1) - (3)].node)->kind = NODE_BRANCH_LINK;
	    }
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 1709 "grammar.y"
    {
#line 1654 "grammar.y.pre"
		CREATE_LAND_LOR((yyval.node), F_LAND, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
		if (IS_NODE((yyvsp[(1) - (3)].node), NODE_LAND_LOR, F_LAND))
		    (yyvsp[(1) - (3)].node)->kind = NODE_BRANCH_LINK;
	    }
    break;

  case 143:

/* Line 1806 of yacc.c  */
#line 1716 "grammar.y"
    {
#line 1660 "grammar.y.pre"
		int t1 = (yyvsp[(1) - (3)].node)->type, t3 = (yyvsp[(3) - (3)].node)->type;
		
		if (is_boolean((yyvsp[(1) - (3)].node)) && is_boolean((yyvsp[(3) - (3)].node)))
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
		    CREATE_BINARY_OP((yyval.node), F_OR, t1, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
		}
		else (yyval.node) = binary_int_op((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), F_OR, "|");		
	    }
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 1742 "grammar.y"
    {
#line 1685 "grammar.y.pre"
		(yyval.node) = binary_int_op((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), F_XOR, "^");
	    }
    break;

  case 145:

/* Line 1806 of yacc.c  */
#line 1747 "grammar.y"
    {
#line 1689 "grammar.y.pre"
		int t1 = (yyvsp[(1) - (3)].node)->type, t3 = (yyvsp[(3) - (3)].node)->type;
		if (is_boolean((yyvsp[(1) - (3)].node)) && is_boolean((yyvsp[(3) - (3)].node)))
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
		    CREATE_BINARY_OP((yyval.node), F_AND, t1, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
		} else (yyval.node) = binary_int_op((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), F_AND, "&");
	    }
    break;

  case 146:

/* Line 1806 of yacc.c  */
#line 1771 "grammar.y"
    {
#line 1712 "grammar.y.pre"
		if (exact_types && !compatible_types2((yyvsp[(1) - (3)].node)->type, (yyvsp[(3) - (3)].node)->type)){
		    char buf[256];
		    char *end = EndOf(buf);
		    char *p;
		    
		    p = strput(buf, end, "== always false because of incompatible types ");
		    p = get_two_types(p, end, (yyvsp[(1) - (3)].node)->type, (yyvsp[(3) - (3)].node)->type);
		    p = strput(p, end, ".");
		    yyerror(buf);
		}
		/* x == 0 -> !x */
		if (IS_NODE((yyvsp[(1) - (3)].node), NODE_NUMBER, 0)) {
		    CREATE_UNARY_OP((yyval.node), F_NOT, TYPE_NUMBER, (yyvsp[(3) - (3)].node));
		} else
		if (IS_NODE((yyvsp[(3) - (3)].node), NODE_NUMBER, 0)) {
		    CREATE_UNARY_OP((yyval.node), F_NOT, TYPE_NUMBER, (yyvsp[(1) - (3)].node));
		} else {
		    CREATE_BINARY_OP((yyval.node), F_EQ, TYPE_NUMBER, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
		}
	    }
    break;

  case 147:

/* Line 1806 of yacc.c  */
#line 1794 "grammar.y"
    {
#line 1734 "grammar.y.pre"
		if (exact_types && !compatible_types2((yyvsp[(1) - (3)].node)->type, (yyvsp[(3) - (3)].node)->type)){
		    char buf[256];
		    char *end = EndOf(buf);
		    char *p;

		    p = strput(buf, end, "!= always true because of incompatible types ");
		    p = get_two_types(p, end, (yyvsp[(1) - (3)].node)->type, (yyvsp[(3) - (3)].node)->type);
		    p = strput(p, end, ".");
		    yyerror(buf);
		}
                CREATE_BINARY_OP((yyval.node), F_NE, TYPE_NUMBER, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
	    }
    break;

  case 148:

/* Line 1806 of yacc.c  */
#line 1809 "grammar.y"
    {
#line 1748 "grammar.y.pre"
		if (exact_types) {
		    int t1 = (yyvsp[(1) - (3)].node)->type;
		    int t3 = (yyvsp[(3) - (3)].node)->type;

		    if (!COMP_TYPE(t1, TYPE_NUMBER) 
			&& !COMP_TYPE(t1, TYPE_STRING)) {
			char buf[256];
			char *end = EndOf(buf);
			char *p;
			
			p = strput(buf, end, "Bad left argument to '");
			p = strput(p, end, query_instr_name((yyvsp[(2) - (3)].number)));
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
                        p = strput(p, end, query_instr_name((yyvsp[(2) - (3)].number)));
                        p = strput(p, end, "' : \"");
                        p = get_type_name(p, end, t3);
			p = strput(p, end, "\"");
			yyerror(buf);
		    } else if (!compatible_types2(t1,t3)) {
			char buf[256];
			char *end = EndOf(buf);
			char *p;
			
			p = strput(buf, end, "Arguments to ");
			p = strput(p, end, query_instr_name((yyvsp[(2) - (3)].number)));
			p = strput(p, end, " do not have compatible types : ");
			p = get_two_types(p, end, t1, t3);
			yyerror(buf);
		    }
		}
                CREATE_BINARY_OP((yyval.node), (yyvsp[(2) - (3)].number), TYPE_NUMBER, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
	    }
    break;

  case 149:

/* Line 1806 of yacc.c  */
#line 1854 "grammar.y"
    {
#line 1792 "grammar.y.pre"
                if (exact_types) {
                    int t1 = (yyvsp[(1) - (3)].node)->type, t3 = (yyvsp[(3) - (3)].node)->type;

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
                CREATE_BINARY_OP((yyval.node), F_LT, TYPE_NUMBER, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
            }
    break;

  case 150:

/* Line 1806 of yacc.c  */
#line 1892 "grammar.y"
    {
#line 1829 "grammar.y.pre"
		(yyval.node) = binary_int_op((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), F_LSH, "<<");
	    }
    break;

  case 151:

/* Line 1806 of yacc.c  */
#line 1897 "grammar.y"
    {
#line 1833 "grammar.y.pre"
		(yyval.node) = binary_int_op((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), F_RSH, ">>");
	    }
    break;

  case 152:

/* Line 1806 of yacc.c  */
#line 1902 "grammar.y"
    {
#line 1837 "grammar.y.pre"
		int result_type;

		if (exact_types) {
		    int t1 = (yyvsp[(1) - (3)].node)->type, t3 = (yyvsp[(3) - (3)].node)->type;

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
		switch ((yyvsp[(1) - (3)].node)->kind) {
		case NODE_NUMBER:
		    /* 0 + X */
		    if ((yyvsp[(1) - (3)].node)->v.number == 0 &&
			((yyvsp[(3) - (3)].node)->type == TYPE_NUMBER || (yyvsp[(3) - (3)].node)->type == TYPE_REAL)) {
			(yyval.node) = (yyvsp[(3) - (3)].node);
			break;
		    }
		    if ((yyvsp[(3) - (3)].node)->kind == NODE_NUMBER) {
			(yyval.node) = (yyvsp[(1) - (3)].node);
			(yyvsp[(1) - (3)].node)->v.number += (yyvsp[(3) - (3)].node)->v.number;
			break;
		    }
		    if ((yyvsp[(3) - (3)].node)->kind == NODE_REAL) {
			(yyval.node) = (yyvsp[(3) - (3)].node);
			(yyvsp[(3) - (3)].node)->v.real += (yyvsp[(1) - (3)].node)->v.number;
			break;
		    }
		    /* swapping the nodes may help later constant folding */
		    if ((yyvsp[(3) - (3)].node)->type != TYPE_STRING && (yyvsp[(3) - (3)].node)->type != TYPE_ANY)
			CREATE_BINARY_OP((yyval.node), F_ADD, result_type, (yyvsp[(3) - (3)].node), (yyvsp[(1) - (3)].node));
		    else
			CREATE_BINARY_OP((yyval.node), F_ADD, result_type, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
		    break;
		case NODE_REAL:
		    if ((yyvsp[(3) - (3)].node)->kind == NODE_NUMBER) {
			(yyval.node) = (yyvsp[(1) - (3)].node);
			(yyvsp[(1) - (3)].node)->v.real += (yyvsp[(3) - (3)].node)->v.number;
			break;
		    }
		    if ((yyvsp[(3) - (3)].node)->kind == NODE_REAL) {
			(yyval.node) = (yyvsp[(1) - (3)].node);
			(yyvsp[(1) - (3)].node)->v.real += (yyvsp[(3) - (3)].node)->v.real;
			break;
		    }
		    /* swapping the nodes may help later constant folding */
		    if ((yyvsp[(3) - (3)].node)->type != TYPE_STRING && (yyvsp[(3) - (3)].node)->type != TYPE_ANY)
			CREATE_BINARY_OP((yyval.node), F_ADD, result_type, (yyvsp[(3) - (3)].node), (yyvsp[(1) - (3)].node));
		    else
			CREATE_BINARY_OP((yyval.node), F_ADD, result_type, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
		    break;
		case NODE_STRING:
		    if ((yyvsp[(3) - (3)].node)->kind == NODE_STRING) {
			/* Combine strings */
			LPC_INT n1, n2;
			const char *s1, *s2;
			char *news;
			int l;

			n1 = (yyvsp[(1) - (3)].node)->v.number;
			n2 = (yyvsp[(3) - (3)].node)->v.number;
			s1 = PROG_STRING(n1);
			s2 = PROG_STRING(n2);
			news = (char *)DXALLOC( (l = strlen(s1))+strlen(s2)+1, TAG_COMPILER, "combine string" );
			strcpy(news, s1);
			strcat(news + l, s2);
			/* free old strings (ordering may help shrink table) */
			if (n1 > n2) {
			    free_prog_string(n1); free_prog_string(n2);
			} else {
			    free_prog_string(n2); free_prog_string(n1);
			}
			(yyval.node) = (yyvsp[(1) - (3)].node);
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
		    if (IS_NODE((yyvsp[(3) - (3)].node), NODE_NUMBER, 0) &&
			((yyvsp[(1) - (3)].node)->type == TYPE_NUMBER || (yyvsp[(1) - (3)].node)->type == TYPE_REAL)) {
			(yyval.node) = (yyvsp[(1) - (3)].node);
			break;
		    }
		    CREATE_BINARY_OP((yyval.node), F_ADD, result_type, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
		    break;
		}
	    }
    break;

  case 153:

/* Line 1806 of yacc.c  */
#line 2073 "grammar.y"
    {
#line 2007 "grammar.y.pre"
		int result_type;

		if (exact_types) {
		    int t1 = (yyvsp[(1) - (3)].node)->type, t3 = (yyvsp[(3) - (3)].node)->type;

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
		
		switch ((yyvsp[(1) - (3)].node)->kind) {
		case NODE_NUMBER:
		    if ((yyvsp[(1) - (3)].node)->v.number == 0) {
			CREATE_UNARY_OP((yyval.node), F_NEGATE, (yyvsp[(3) - (3)].node)->type, (yyvsp[(3) - (3)].node));
		    } else if ((yyvsp[(3) - (3)].node)->kind == NODE_NUMBER) {
			(yyval.node) = (yyvsp[(1) - (3)].node);
			(yyvsp[(1) - (3)].node)->v.number -= (yyvsp[(3) - (3)].node)->v.number;
		    } else if ((yyvsp[(3) - (3)].node)->kind == NODE_REAL) {
			(yyval.node) = (yyvsp[(3) - (3)].node);
			(yyvsp[(3) - (3)].node)->v.real = (yyvsp[(1) - (3)].node)->v.number - (yyvsp[(3) - (3)].node)->v.real;
		    } else {
			CREATE_BINARY_OP((yyval.node), F_SUBTRACT, result_type, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
		    }
		    break;
		case NODE_REAL:
		    if ((yyvsp[(3) - (3)].node)->kind == NODE_NUMBER) {
			(yyval.node) = (yyvsp[(1) - (3)].node);
			(yyvsp[(1) - (3)].node)->v.real -= (yyvsp[(3) - (3)].node)->v.number;
		    } else if ((yyvsp[(3) - (3)].node)->kind == NODE_REAL) {
			(yyval.node) = (yyvsp[(1) - (3)].node);
			(yyvsp[(1) - (3)].node)->v.real -= (yyvsp[(3) - (3)].node)->v.real;
		    } else {
			CREATE_BINARY_OP((yyval.node), F_SUBTRACT, result_type, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
		    }
		    break;
		default:
		    /* optimize X-0 */
		    if (IS_NODE((yyvsp[(3) - (3)].node), NODE_NUMBER, 0)) {
			(yyval.node) = (yyvsp[(1) - (3)].node);
		    } 
		    CREATE_BINARY_OP((yyval.node), F_SUBTRACT, result_type, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
		}
	    }
    break;

  case 154:

/* Line 1806 of yacc.c  */
#line 2168 "grammar.y"
    {
#line 2101 "grammar.y.pre"
		int result_type;

		if (exact_types){
		    int t1 = (yyvsp[(1) - (3)].node)->type, t3 = (yyvsp[(3) - (3)].node)->type;

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

		switch ((yyvsp[(1) - (3)].node)->kind) {
		case NODE_NUMBER:
		    if ((yyvsp[(3) - (3)].node)->kind == NODE_NUMBER) {
			(yyval.node) = (yyvsp[(1) - (3)].node);
			(yyval.node)->v.number *= (yyvsp[(3) - (3)].node)->v.number;
			break;
		    }
		    if ((yyvsp[(3) - (3)].node)->kind == NODE_REAL) {
			(yyval.node) = (yyvsp[(3) - (3)].node);
			(yyvsp[(3) - (3)].node)->v.real *= (yyvsp[(1) - (3)].node)->v.number;
			break;
		    }
		    CREATE_BINARY_OP((yyval.node), F_MULTIPLY, result_type, (yyvsp[(3) - (3)].node), (yyvsp[(1) - (3)].node));
		    break;
		case NODE_REAL:
		    if ((yyvsp[(3) - (3)].node)->kind == NODE_NUMBER) {
			(yyval.node) = (yyvsp[(1) - (3)].node);
			(yyvsp[(1) - (3)].node)->v.real *= (yyvsp[(3) - (3)].node)->v.number;
			break;
		    }
		    if ((yyvsp[(3) - (3)].node)->kind == NODE_REAL) {
			(yyval.node) = (yyvsp[(1) - (3)].node);
			(yyvsp[(1) - (3)].node)->v.real *= (yyvsp[(3) - (3)].node)->v.real;
			break;
		    }
		    CREATE_BINARY_OP((yyval.node), F_MULTIPLY, result_type, (yyvsp[(3) - (3)].node), (yyvsp[(1) - (3)].node));
		    break;
		default:
		    CREATE_BINARY_OP((yyval.node), F_MULTIPLY, result_type, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
		}
	    }
    break;

  case 155:

/* Line 1806 of yacc.c  */
#line 2249 "grammar.y"
    {
#line 2181 "grammar.y.pre"
		(yyval.node) = binary_int_op((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), F_MOD, "%");
	    }
    break;

  case 156:

/* Line 1806 of yacc.c  */
#line 2254 "grammar.y"
    {
#line 2185 "grammar.y.pre"
		int result_type;

		if (exact_types){
		    int t1 = (yyvsp[(1) - (3)].node)->type, t3 = (yyvsp[(3) - (3)].node)->type;

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
		switch ((yyvsp[(1) - (3)].node)->kind) {
		case NODE_NUMBER:
		    if ((yyvsp[(3) - (3)].node)->kind == NODE_NUMBER) {
			if ((yyvsp[(3) - (3)].node)->v.number == 0) {
			    yyerror("Divide by zero in constant");
			    (yyval.node) = (yyvsp[(1) - (3)].node);
			    break;
			}
			(yyval.node) = (yyvsp[(1) - (3)].node);
			(yyvsp[(1) - (3)].node)->v.number /= (yyvsp[(3) - (3)].node)->v.number;
			break;
		    }
		    if ((yyvsp[(3) - (3)].node)->kind == NODE_REAL) {
			if ((yyvsp[(3) - (3)].node)->v.real == 0.0) {
			    yyerror("Divide by zero in constant");
			    (yyval.node) = (yyvsp[(1) - (3)].node);
			    break;
			}
			(yyval.node) = (yyvsp[(3) - (3)].node);
			(yyvsp[(3) - (3)].node)->v.real = ((yyvsp[(1) - (3)].node)->v.number / (yyvsp[(3) - (3)].node)->v.real);
			break;
		    }
		    CREATE_BINARY_OP((yyval.node), F_DIVIDE, result_type, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
		    break;
		case NODE_REAL:
		    if ((yyvsp[(3) - (3)].node)->kind == NODE_NUMBER) {
			if ((yyvsp[(3) - (3)].node)->v.number == 0) {
			    yyerror("Divide by zero in constant");
			    (yyval.node) = (yyvsp[(1) - (3)].node);
			    break;
			}
			(yyval.node) = (yyvsp[(1) - (3)].node);
			(yyvsp[(1) - (3)].node)->v.real /= (yyvsp[(3) - (3)].node)->v.number;
			break;
		    }
		    if ((yyvsp[(3) - (3)].node)->kind == NODE_REAL) {
			if ((yyvsp[(3) - (3)].node)->v.real == 0.0) {
			    yyerror("Divide by zero in constant");
			    (yyval.node) = (yyvsp[(1) - (3)].node);
			    break;
			}
			(yyval.node) = (yyvsp[(1) - (3)].node);
			(yyvsp[(1) - (3)].node)->v.real /= (yyvsp[(3) - (3)].node)->v.real;
			break;
		    }
		    CREATE_BINARY_OP((yyval.node), F_DIVIDE, result_type, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
		    break;
		default:
		    CREATE_BINARY_OP((yyval.node), F_DIVIDE, result_type, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
		}
	    }
    break;

  case 157:

/* Line 1806 of yacc.c  */
#line 2350 "grammar.y"
    {
#line 2280 "grammar.y.pre"
		(yyval.node) = (yyvsp[(2) - (2)].node);
		(yyval.node)->type = (yyvsp[(1) - (2)].number);

		if (exact_types &&
		    (yyvsp[(2) - (2)].node)->type != (yyvsp[(1) - (2)].number) &&
		    (yyvsp[(2) - (2)].node)->type != TYPE_ANY && 
		    (yyvsp[(2) - (2)].node)->type != TYPE_UNKNOWN &&
		    (yyvsp[(1) - (2)].number) != TYPE_VOID) {
		    char buf[256];
		    char *end = EndOf(buf);
		    char *p;
		    
		    p = strput(buf, end, "Cannot cast ");
		    p = get_type_name(p, end, (yyvsp[(2) - (2)].node)->type);
		    p = strput(p, end, "to ");
		    p = get_type_name(p, end, (yyvsp[(1) - (2)].number));
		    yyerror(buf);
		}
	    }
    break;

  case 158:

/* Line 1806 of yacc.c  */
#line 2372 "grammar.y"
    {
#line 2301 "grammar.y.pre"
		CREATE_UNARY_OP((yyval.node), F_PRE_INC, 0, (yyvsp[(2) - (2)].node));
                if (exact_types){
                    switch((yyvsp[(2) - (2)].node)->type){
                        case TYPE_NUMBER:
                        case TYPE_ANY:
                        case TYPE_REAL:
                        {
                            (yyval.node)->type = (yyvsp[(2) - (2)].node)->type;
                            break;
                        }

                        default:
                        {
                            (yyval.node)->type = TYPE_ANY;
                            type_error("Bad argument 1 to ++x", (yyvsp[(2) - (2)].node)->type);
                        }
                    }
                } else (yyval.node)->type = TYPE_ANY;
	    }
    break;

  case 159:

/* Line 1806 of yacc.c  */
#line 2394 "grammar.y"
    {
#line 2322 "grammar.y.pre"
		CREATE_UNARY_OP((yyval.node), F_PRE_DEC, 0, (yyvsp[(2) - (2)].node));
                if (exact_types){
                    switch((yyvsp[(2) - (2)].node)->type){
                        case TYPE_NUMBER:
                        case TYPE_ANY:
                        case TYPE_REAL:
                        {
                            (yyval.node)->type = (yyvsp[(2) - (2)].node)->type;
                            break;
                        }

                        default:
                        {
                            (yyval.node)->type = TYPE_ANY;
                            type_error("Bad argument 1 to --x", (yyvsp[(2) - (2)].node)->type);
                        }
                    }
                } else (yyval.node)->type = TYPE_ANY;

	    }
    break;

  case 160:

/* Line 1806 of yacc.c  */
#line 2417 "grammar.y"
    {
#line 2344 "grammar.y.pre"
		if ((yyvsp[(2) - (2)].node)->kind == NODE_NUMBER) {
		    (yyval.node) = (yyvsp[(2) - (2)].node);
		    (yyval.node)->v.number = !((yyval.node)->v.number);
		} else {
		    CREATE_UNARY_OP((yyval.node), F_NOT, TYPE_NUMBER, (yyvsp[(2) - (2)].node));
		}
	    }
    break;

  case 161:

/* Line 1806 of yacc.c  */
#line 2427 "grammar.y"
    {
#line 2353 "grammar.y.pre"
		if (exact_types && !IS_TYPE((yyvsp[(2) - (2)].node)->type, TYPE_NUMBER))
		    type_error("Bad argument to ~", (yyvsp[(2) - (2)].node)->type);
		if ((yyvsp[(2) - (2)].node)->kind == NODE_NUMBER) {
		    (yyval.node) = (yyvsp[(2) - (2)].node);
		    (yyval.node)->v.number = ~(yyval.node)->v.number;
		} else {
		    CREATE_UNARY_OP((yyval.node), F_COMPL, TYPE_NUMBER, (yyvsp[(2) - (2)].node));
		}
	    }
    break;

  case 162:

/* Line 1806 of yacc.c  */
#line 2439 "grammar.y"
    {
#line 2364 "grammar.y.pre"
		int result_type;
                if (exact_types){
		    int t = (yyvsp[(2) - (2)].node)->type;
		    if (!COMP_TYPE(t, TYPE_NUMBER)){
			type_error("Bad argument to unary '-'", t);
			result_type = TYPE_ANY;
		    } else result_type = t;
		} else result_type = TYPE_ANY;

		switch ((yyvsp[(2) - (2)].node)->kind) {
		case NODE_NUMBER:
		    (yyval.node) = (yyvsp[(2) - (2)].node);
		    (yyval.node)->v.number = -(yyval.node)->v.number;
		    break;
		case NODE_REAL:
		    (yyval.node) = (yyvsp[(2) - (2)].node);
		    (yyval.node)->v.real = -(yyval.node)->v.real;
		    break;
		default:
		    CREATE_UNARY_OP((yyval.node), F_NEGATE, result_type, (yyvsp[(2) - (2)].node));
		}
	    }
    break;

  case 163:

/* Line 1806 of yacc.c  */
#line 2464 "grammar.y"
    {
#line 2388 "grammar.y.pre"
		CREATE_UNARY_OP((yyval.node), F_POST_INC, 0, (yyvsp[(1) - (2)].node));
		(yyval.node)->v.number = F_POST_INC;
                if (exact_types){
                    switch((yyvsp[(1) - (2)].node)->type){
                        case TYPE_NUMBER:
		    case TYPE_ANY:
                        case TYPE_REAL:
                        {
                            (yyval.node)->type = (yyvsp[(1) - (2)].node)->type;
                            break;
                        }

                        default:
                        {
                            (yyval.node)->type = TYPE_ANY;
                            type_error("Bad argument 1 to x++", (yyvsp[(1) - (2)].node)->type);
                        }
                    }
                } else (yyval.node)->type = TYPE_ANY;
	    }
    break;

  case 164:

/* Line 1806 of yacc.c  */
#line 2487 "grammar.y"
    {
#line 2410 "grammar.y.pre"
		CREATE_UNARY_OP((yyval.node), F_POST_DEC, 0, (yyvsp[(1) - (2)].node));
                if (exact_types){
                    switch((yyvsp[(1) - (2)].node)->type){
		    case TYPE_NUMBER:
		    case TYPE_ANY:
		    case TYPE_REAL:
		    {
			(yyval.node)->type = (yyvsp[(1) - (2)].node)->type;
			break;
		    }

		    default:
		    {
			(yyval.node)->type = TYPE_ANY;
			type_error("Bad argument 1 to x--", (yyvsp[(1) - (2)].node)->type);
		    }
                    }
                } else (yyval.node)->type = TYPE_ANY;
	    }
    break;

  case 171:

/* Line 1806 of yacc.c  */
#line 2518 "grammar.y"
    {
#line 2440 "grammar.y.pre"
		if (exact_types && !IS_TYPE(exact_types, TYPE_VOID))
		    yywarn("Non-void functions must return a value.");
		CREATE_RETURN((yyval.node), 0);
	    }
    break;

  case 172:

/* Line 1806 of yacc.c  */
#line 2525 "grammar.y"
    {
#line 2446 "grammar.y.pre"
		if (exact_types && !compatible_types((yyvsp[(2) - (3)].node)->type, exact_types)) {
		    char buf[256];
		    char *end = EndOf(buf);
		    char *p;
		    
		    p = strput(buf, end, "Type of returned value doesn't match function return type ");
		    p = get_two_types(p, end, (yyvsp[(2) - (3)].node)->type, exact_types);
		    yyerror(buf);
		}
		if (IS_NODE((yyvsp[(2) - (3)].node), NODE_NUMBER, 0)) {
		    CREATE_RETURN((yyval.node), 0);
		} else {
		    CREATE_RETURN((yyval.node), (yyvsp[(2) - (3)].node));
		}
	    }
    break;

  case 173:

/* Line 1806 of yacc.c  */
#line 2546 "grammar.y"
    {
#line 2466 "grammar.y.pre"
		CREATE_EXPR_LIST((yyval.node), 0);
	    }
    break;

  case 174:

/* Line 1806 of yacc.c  */
#line 2551 "grammar.y"
    {
#line 2470 "grammar.y.pre"
		CREATE_EXPR_LIST((yyval.node), (yyvsp[(1) - (1)].node));
	    }
    break;

  case 175:

/* Line 1806 of yacc.c  */
#line 2556 "grammar.y"
    {
#line 2474 "grammar.y.pre"
		CREATE_EXPR_LIST((yyval.node), (yyvsp[(1) - (2)].node));
	    }
    break;

  case 176:

/* Line 1806 of yacc.c  */
#line 2564 "grammar.y"
    {
#line 2481 "grammar.y.pre"
		CREATE_EXPR_NODE((yyval.node), (yyvsp[(1) - (1)].node), 0);
	    }
    break;

  case 177:

/* Line 1806 of yacc.c  */
#line 2569 "grammar.y"
    {
#line 2485 "grammar.y.pre"
		CREATE_EXPR_NODE((yyval.node), (yyvsp[(1) - (2)].node), 1);
	    }
    break;

  case 178:

/* Line 1806 of yacc.c  */
#line 2577 "grammar.y"
    {
#line 2492 "grammar.y.pre"
		(yyvsp[(1) - (1)].node)->kind = 1;

		(yyval.node) = (yyvsp[(1) - (1)].node);
	    }
    break;

  case 179:

/* Line 1806 of yacc.c  */
#line 2584 "grammar.y"
    {
#line 2498 "grammar.y.pre"
		(yyvsp[(3) - (3)].node)->kind = 0;

		(yyval.node) = (yyvsp[(1) - (3)].node);
		(yyval.node)->kind++;
		(yyval.node)->l.expr->r.expr = (yyvsp[(3) - (3)].node);
		(yyval.node)->l.expr = (yyvsp[(3) - (3)].node);
	    }
    break;

  case 180:

/* Line 1806 of yacc.c  */
#line 2597 "grammar.y"
    {
#line 2510 "grammar.y.pre"
		/* this is a dummy node */
		CREATE_EXPR_LIST((yyval.node), 0);
	    }
    break;

  case 181:

/* Line 1806 of yacc.c  */
#line 2603 "grammar.y"
    {
#line 2515 "grammar.y.pre"
		CREATE_EXPR_LIST((yyval.node), (yyvsp[(1) - (1)].node));
	    }
    break;

  case 182:

/* Line 1806 of yacc.c  */
#line 2608 "grammar.y"
    {
#line 2519 "grammar.y.pre"
		CREATE_EXPR_LIST((yyval.node), (yyvsp[(1) - (2)].node));
	    }
    break;

  case 183:

/* Line 1806 of yacc.c  */
#line 2616 "grammar.y"
    {
#line 2526 "grammar.y.pre"
		(yyval.node) = new_node_no_line();
		(yyval.node)->kind = 2;
		(yyval.node)->v.expr = (yyvsp[(1) - (1)].node);
		(yyval.node)->r.expr = 0;
		(yyval.node)->type = 0;
		/* we keep track of the end of the chain in the left nodes */
		(yyval.node)->l.expr = (yyval.node);
            }
    break;

  case 184:

/* Line 1806 of yacc.c  */
#line 2627 "grammar.y"
    {
#line 2536 "grammar.y.pre"
		parse_node_t *expr;

		expr = new_node_no_line();
		expr->kind = 0;
		expr->v.expr = (yyvsp[(3) - (3)].node);
		expr->r.expr = 0;
		expr->type = 0;
		
		(yyvsp[(1) - (3)].node)->l.expr->r.expr = expr;
		(yyvsp[(1) - (3)].node)->l.expr = expr;
		(yyvsp[(1) - (3)].node)->kind += 2;
		(yyval.node) = (yyvsp[(1) - (3)].node);
	    }
    break;

  case 185:

/* Line 1806 of yacc.c  */
#line 2646 "grammar.y"
    {
#line 2554 "grammar.y.pre"
		CREATE_TWO_VALUES((yyval.node), 0, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
            }
    break;

  case 186:

/* Line 1806 of yacc.c  */
#line 2654 "grammar.y"
    {
#line 2561 "grammar.y.pre"
#define LV_ILLEGAL 1
#define LV_RANGE 2
#define LV_INDEX 4
                /* Restrictive lvalues, but I think they make more sense :) */
                (yyval.node) = (yyvsp[(1) - (1)].node);
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
    break;

  case 188:

/* Line 1806 of yacc.c  */
#line 2771 "grammar.y"
    {
#line 2677 "grammar.y.pre"
		(yyval.number) = ((yyvsp[(2) - (2)].number) << 8) | FP_EFUN;
	    }
    break;

  case 189:

/* Line 1806 of yacc.c  */
#line 2778 "grammar.y"
    { 
#line 2684 "grammar.y.pre"
		(yyval.number) = (yyvsp[(1) - (3)].number);
	    }
    break;

  case 190:

/* Line 1806 of yacc.c  */
#line 2783 "grammar.y"
    {
#line 2688 "grammar.y.pre"
		int val;
		
		if ((val=(yyvsp[(2) - (2)].ihe)->dn.local_num) >= 0){
		    (yyval.number) = (val << 8) | FP_L_VAR;
		} else if ((val=(yyvsp[(2) - (2)].ihe)->dn.global_num) >= 0) {
		    (yyval.number) = (val << 8) | FP_G_VAR;
		} else if ((val=(yyvsp[(2) - (2)].ihe)->dn.function_num) >=0) {
		    (yyval.number) = (val << 8)|FP_LOCAL;
		} else if ((val=(yyvsp[(2) - (2)].ihe)->dn.simul_num) >=0) {
		    (yyval.number) = (val << 8)|FP_SIMUL;
		} else if ((val=(yyvsp[(2) - (2)].ihe)->dn.efun_num) >=0) {
		    (yyval.number) = (val << 8)|FP_EFUN;
		}
	    }
    break;

  case 192:

/* Line 1806 of yacc.c  */
#line 2804 "grammar.y"
    {
#line 2709 "grammar.y.pre"
              int i;
              if ((i = (yyvsp[(1) - (1)].ihe)->dn.local_num) != -1) {
		  type_of_locals_ptr[i] &= ~LOCAL_MOD_UNUSED;
		  if (type_of_locals_ptr[i] & LOCAL_MOD_REF)
		      CREATE_OPCODE_1((yyval.node), F_REF, type_of_locals_ptr[i] & ~LOCAL_MOD_REF,i & 0xff);
		  else
		      CREATE_OPCODE_1((yyval.node), F_LOCAL, type_of_locals_ptr[i], i & 0xff);
		  if (current_function_context)
		      current_function_context->num_locals++;
              } else
		  if ((i = (yyvsp[(1) - (1)].ihe)->dn.global_num) != -1) {
		      if (current_function_context)
			  current_function_context->bindable = FP_NOT_BINDABLE;
                          CREATE_OPCODE_1((yyval.node), F_GLOBAL,
				      VAR_TEMP(i)->type & ~DECL_MODS, i);
		      if (VAR_TEMP(i)->type & DECL_HIDDEN) {
			  char buf[256];
			  char *end = EndOf(buf);
			  char *p;

			  p = strput(buf, end, "Illegal to use private variable '");
			  p = strput(p, end, (yyvsp[(1) - (1)].ihe)->name);
			  p = strput(p, end, "'");
			  yyerror(buf);
		      }
		  } else {
		      char buf[256];
		      char *end = EndOf(buf);
		      char *p;
		      
		      p = strput(buf, end, "Undefined variable '");
		      p = strput(p, end, (yyvsp[(1) - (1)].ihe)->name);
		      p = strput(p, end, "'");
		      if (current_number_of_locals < CFG_MAX_LOCAL_VARIABLES) {
			  add_local_name((yyvsp[(1) - (1)].ihe)->name, TYPE_ANY);
		      }
		      CREATE_ERROR((yyval.node));
		      yyerror(buf);
		  }
	    }
    break;

  case 193:

/* Line 1806 of yacc.c  */
#line 2847 "grammar.y"
    {
#line 2751 "grammar.y.pre"
		char buf[256];
		char *end = EndOf(buf);
		char *p;
		
		p = strput(buf, end, "Undefined variable '");
		p = strput(p, end, (yyvsp[(1) - (1)].string));
		p = strput(p, end, "'");
                if (current_number_of_locals < CFG_MAX_LOCAL_VARIABLES) {
                    add_local_name((yyvsp[(1) - (1)].string), TYPE_ANY);
                }
                CREATE_ERROR((yyval.node));
                yyerror(buf);
                scratch_free((yyvsp[(1) - (1)].string));
            }
    break;

  case 194:

/* Line 1806 of yacc.c  */
#line 2864 "grammar.y"
    {
#line 2767 "grammar.y.pre"
		CREATE_PARAMETER((yyval.node), TYPE_ANY, (yyvsp[(1) - (1)].number));
            }
    break;

  case 195:

/* Line 1806 of yacc.c  */
#line 2869 "grammar.y"
    {
#line 2771 "grammar.y.pre"
		(yyval.contextp) = current_function_context;
		/* already flagged as an error */
		if (current_function_context)
		    current_function_context = current_function_context->parent;
            }
    break;

  case 196:

/* Line 1806 of yacc.c  */
#line 2877 "grammar.y"
    {
#line 2778 "grammar.y.pre"
		parse_node_t *node;

		current_function_context = (yyvsp[(3) - (5)].contextp);

		if (!current_function_context || current_function_context->num_parameters == -2) {
		    /* This was illegal, and error'ed when the '$' token
		     * was returned.
		     */
		    CREATE_ERROR((yyval.node));
		} else {
		    CREATE_OPCODE_1((yyval.node), F_LOCAL, (yyvsp[(4) - (5)].node)->type,
				    current_function_context->values_list->kind++);

		    node = new_node_no_line();
		    node->type = 0;
		    current_function_context->values_list->l.expr->r.expr = node;
		    current_function_context->values_list->l.expr = node;
		    node->r.expr = 0;
		    node->v.expr = (yyvsp[(4) - (5)].node);
		}
	    }
    break;

  case 197:

/* Line 1806 of yacc.c  */
#line 2901 "grammar.y"
    {
#line 2801 "grammar.y.pre"
		if ((yyvsp[(1) - (3)].node)->type == TYPE_ANY) {
		    int cmi;
		    unsigned short tp;
		    
		    if ((cmi = lookup_any_class_member((yyvsp[(3) - (3)].string), &tp)) != -1) {
			CREATE_UNARY_OP_1((yyval.node), F_MEMBER, tp, (yyvsp[(1) - (3)].node), 0);
			(yyval.node)->l.number = cmi;
		    } else {
			CREATE_ERROR((yyval.node));
		    }
		} else if (!IS_CLASS((yyvsp[(1) - (3)].node)->type)) {
		    yyerror("Left argument of -> is not a class");
		    CREATE_ERROR((yyval.node));
		} else {
		    CREATE_UNARY_OP_1((yyval.node), F_MEMBER, 0, (yyvsp[(1) - (3)].node), 0);
		    (yyval.node)->l.number = lookup_class_member(CLASS_IDX((yyvsp[(1) - (3)].node)->type),
						       (yyvsp[(3) - (3)].string),
						       &((yyval.node)->type));
		}
		    
		scratch_free((yyvsp[(3) - (3)].string));
            }
    break;

  case 198:

/* Line 1806 of yacc.c  */
#line 2926 "grammar.y"
    {
#line 2825 "grammar.y.pre"
                (yyval.node) = make_range_node(F_NN_RANGE, (yyvsp[(1) - (6)].node), (yyvsp[(3) - (6)].node), (yyvsp[(5) - (6)].node));
            }
    break;

  case 199:

/* Line 1806 of yacc.c  */
#line 2931 "grammar.y"
    {
#line 2836 "grammar.y.pre"
                (yyval.node) = make_range_node(F_RN_RANGE, (yyvsp[(1) - (7)].node), (yyvsp[(4) - (7)].node), (yyvsp[(6) - (7)].node));
            }
    break;

  case 200:

/* Line 1806 of yacc.c  */
#line 2936 "grammar.y"
    {
#line 2840 "grammar.y.pre"
		if ((yyvsp[(7) - (8)].node)->kind == NODE_NUMBER && (yyvsp[(7) - (8)].node)->v.number <= 1)
		    (yyval.node) = make_range_node(F_RE_RANGE, (yyvsp[(1) - (8)].node), (yyvsp[(4) - (8)].node), 0);
		else
		    (yyval.node) = make_range_node(F_RR_RANGE, (yyvsp[(1) - (8)].node), (yyvsp[(4) - (8)].node), (yyvsp[(7) - (8)].node));
            }
    break;

  case 201:

/* Line 1806 of yacc.c  */
#line 2944 "grammar.y"
    {
#line 2847 "grammar.y.pre"
		if ((yyvsp[(6) - (7)].node)->kind == NODE_NUMBER && (yyvsp[(6) - (7)].node)->v.number <= 1)
		    (yyval.node) = make_range_node(F_NE_RANGE, (yyvsp[(1) - (7)].node), (yyvsp[(3) - (7)].node), 0);
		else
		    (yyval.node) = make_range_node(F_NR_RANGE, (yyvsp[(1) - (7)].node), (yyvsp[(3) - (7)].node), (yyvsp[(6) - (7)].node));
            }
    break;

  case 202:

/* Line 1806 of yacc.c  */
#line 2952 "grammar.y"
    {
#line 2854 "grammar.y.pre"
                (yyval.node) = make_range_node(F_NE_RANGE, (yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].node), 0);
            }
    break;

  case 203:

/* Line 1806 of yacc.c  */
#line 2957 "grammar.y"
    {
#line 2858 "grammar.y.pre"
                (yyval.node) = make_range_node(F_RE_RANGE, (yyvsp[(1) - (6)].node), (yyvsp[(4) - (6)].node), 0);
            }
    break;

  case 204:

/* Line 1806 of yacc.c  */
#line 2962 "grammar.y"
    {
#line 2862 "grammar.y.pre"
                if (IS_NODE((yyvsp[(1) - (5)].node), NODE_CALL, F_AGGREGATE)
		    && (yyvsp[(4) - (5)].node)->kind == NODE_NUMBER) {
                    int i = (yyvsp[(4) - (5)].node)->v.number;
                    if (i < 1 || i > (yyvsp[(1) - (5)].node)->l.number)
                        yyerror("Illegal index to array constant.");
                    else {
                        parse_node_t *node = (yyvsp[(1) - (5)].node)->r.expr;
                        i = (yyvsp[(1) - (5)].node)->l.number - i;
                        while (i--)
                            node = node->r.expr;
                        (yyval.node) = node->v.expr;
                        break;
                    }
                }
		CREATE_BINARY_OP((yyval.node), F_RINDEX, 0, (yyvsp[(4) - (5)].node), (yyvsp[(1) - (5)].node));
                if (exact_types) {
		    switch((yyvsp[(1) - (5)].node)->type) {
		    case TYPE_MAPPING:
			yyerror("Illegal index for mapping.");
		    case TYPE_ANY:
			(yyval.node)->type = TYPE_ANY;
			break;
		    case TYPE_STRING:
		    case TYPE_BUFFER:
			(yyval.node)->type = TYPE_NUMBER;
			if (!IS_TYPE((yyvsp[(4) - (5)].node)->type,TYPE_NUMBER))
			    type_error("Bad type of index", (yyvsp[(4) - (5)].node)->type);
			break;
			
		    default:
			if ((yyvsp[(1) - (5)].node)->type & TYPE_MOD_ARRAY) {
			    (yyval.node)->type = (yyvsp[(1) - (5)].node)->type & ~TYPE_MOD_ARRAY;
			    if ((yyval.node)->type != TYPE_ANY)
			        (yyval.node) = add_type_check((yyval.node), (yyval.node)->type); 
			    if (!IS_TYPE((yyvsp[(4) - (5)].node)->type,TYPE_NUMBER))
				type_error("Bad type of index", (yyvsp[(4) - (5)].node)->type);
			} else {
			    type_error("Value indexed has a bad type ", (yyvsp[(1) - (5)].node)->type);
			    (yyval.node)->type = TYPE_ANY;
			}
		    }
		} else (yyval.node)->type = TYPE_ANY;
            }
    break;

  case 205:

/* Line 1806 of yacc.c  */
#line 3008 "grammar.y"
    {
#line 2907 "grammar.y.pre"
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
                if (IS_NODE((yyvsp[(1) - (4)].node), NODE_CALL, F_AGGREGATE) && (yyvsp[(3) - (4)].node)->kind == NODE_NUMBER) {
                    int i = (yyvsp[(3) - (4)].node)->v.number;
                    if (i < 0 || i >= (yyvsp[(1) - (4)].node)->l.number)
                        yyerror("Illegal index to array constant.");
                    else {
                        parse_node_t *node = (yyvsp[(1) - (4)].node)->r.expr;
                        while (i--)
                            node = node->r.expr;
                        (yyval.node) = node->v.expr;
                        break;
                    }
                }
                if ((yyvsp[(3) - (4)].node)->kind == NODE_NUMBER && (yyvsp[(3) - (4)].node)->v.number < 0)
		    yywarn("A negative constant in arr[x] no longer means indexing from the end.  Use arr[<x]");
                CREATE_BINARY_OP((yyval.node), F_INDEX, 0, (yyvsp[(3) - (4)].node), (yyvsp[(1) - (4)].node));
                if (exact_types) {
		    switch((yyvsp[(1) - (4)].node)->type) {
		    case TYPE_MAPPING:
		    case TYPE_ANY:
			(yyval.node)->type = TYPE_ANY;
			break;
		    case TYPE_STRING:
		    case TYPE_BUFFER:
			(yyval.node)->type = TYPE_NUMBER;
			if (!IS_TYPE((yyvsp[(3) - (4)].node)->type,TYPE_NUMBER))
			    type_error("Bad type of index", (yyvsp[(3) - (4)].node)->type);
			break;
			
		    default:
			if ((yyvsp[(1) - (4)].node)->type & TYPE_MOD_ARRAY) {
			    (yyval.node)->type = (yyvsp[(1) - (4)].node)->type & ~TYPE_MOD_ARRAY;
			    if((yyval.node)->type != TYPE_ANY)
			        (yyval.node) = add_type_check((yyval.node), (yyval.node)->type);
			    if (!IS_TYPE((yyvsp[(3) - (4)].node)->type,TYPE_NUMBER))
				type_error("Bad type of index", (yyvsp[(3) - (4)].node)->type);
			} else {
			    type_error("Value indexed has a bad type ", (yyvsp[(1) - (4)].node)->type);
			    (yyval.node)->type = TYPE_ANY;
			}
                    }
                } else (yyval.node)->type = TYPE_ANY;
            }
    break;

  case 207:

/* Line 1806 of yacc.c  */
#line 3064 "grammar.y"
    {
#line 2964 "grammar.y.pre"
		(yyval.node) = (yyvsp[(2) - (3)].node);
	    }
    break;

  case 209:

/* Line 1806 of yacc.c  */
#line 3070 "grammar.y"
    {
#line 2972 "grammar.y.pre"
	        if ((yyvsp[(1) - (1)].number) != TYPE_FUNCTION) yyerror("Reserved type name unexpected.");
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
    break;

  case 210:

/* Line 1806 of yacc.c  */
#line 3090 "grammar.y"
    {
#line 2991 "grammar.y.pre"
		if ((yyvsp[(4) - (6)].argument).flags & ARG_IS_VARARGS) {
		    yyerror("Anonymous varargs functions aren't implemented");
		}
		if (!(yyvsp[(6) - (6)].decl).node) {
		    CREATE_RETURN((yyvsp[(6) - (6)].decl).node, 0);
		} else if ((yyvsp[(6) - (6)].decl).node->kind != NODE_RETURN &&
			   ((yyvsp[(6) - (6)].decl).node->kind != NODE_TWO_VALUES || (yyvsp[(6) - (6)].decl).node->r.expr->kind != NODE_RETURN)) {
		    parse_node_t *replacement;
		    CREATE_STATEMENTS(replacement, (yyvsp[(6) - (6)].decl).node, 0);
		    CREATE_RETURN(replacement->r.expr, 0);
		    (yyvsp[(6) - (6)].decl).node = replacement;
		}
		
		(yyval.node) = new_node();
		(yyval.node)->kind = NODE_ANON_FUNC;
		(yyval.node)->type = TYPE_FUNCTION;
		(yyval.node)->l.number = (max_num_locals - (yyvsp[(4) - (6)].argument).num_arg);
		(yyval.node)->r.expr = (yyvsp[(6) - (6)].decl).node;
		(yyval.node)->v.number = (yyvsp[(4) - (6)].argument).num_arg;
		if (current_function_context->bindable)
		    (yyval.node)->v.number |= 0x10000;
		free_all_local_names(1);
		
		current_number_of_locals = (yyvsp[(2) - (6)].func_block).num_local;
		max_num_locals = (yyvsp[(2) - (6)].func_block).max_num_locals;
		context = (yyvsp[(2) - (6)].func_block).context;
		current_type = (yyvsp[(2) - (6)].func_block).save_current_type;
		exact_types = (yyvsp[(2) - (6)].func_block).save_exact_types;
		pop_function_context();
		
		locals_ptr -= current_number_of_locals;
		type_of_locals_ptr -= max_num_locals;
		reactivate_current_locals();
	    }
    break;

  case 211:

/* Line 1806 of yacc.c  */
#line 3127 "grammar.y"
    {
#line 3031 "grammar.y.pre"
#ifdef WOMBLES
	        if(*(outp-2) != ':')
		  yyerror("End of functional not found");
#endif
		(yyval.node) = new_node();
		(yyval.node)->kind = NODE_FUNCTION_CONSTRUCTOR;
		(yyval.node)->type = TYPE_FUNCTION;
		(yyval.node)->r.expr = 0;
		switch ((yyvsp[(1) - (1)].number) & 0xff) {
		case FP_L_VAR:
		    yyerror("Illegal to use local variable in a functional.");
		    CREATE_NUMBER((yyval.node)->l.expr, 0);
		    (yyval.node)->l.expr->r.expr = 0;
		    (yyval.node)->l.expr->l.expr = 0;
		    (yyval.node)->v.number = FP_FUNCTIONAL;
		    break;
		case FP_G_VAR:
		    CREATE_OPCODE_1((yyval.node)->l.expr, F_GLOBAL, 0, (yyvsp[(1) - (1)].number) >> 8);
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
		    (yyval.node)->v.number = (yyvsp[(1) - (1)].number);
		    break;
		}
	    }
    break;

  case 212:

/* Line 1806 of yacc.c  */
#line 3165 "grammar.y"
    {
#line 3068 "grammar.y.pre"
#ifdef WOMBLES
	        if(*(outp-2) != ':')
		  yyerror("End of functional not found");
#endif
		(yyval.node) = new_node();
		(yyval.node)->kind = NODE_FUNCTION_CONSTRUCTOR;
		(yyval.node)->type = TYPE_FUNCTION;
		(yyval.node)->v.number = (yyvsp[(1) - (5)].number);
		(yyval.node)->r.expr = (yyvsp[(3) - (5)].node);
		
		switch ((yyvsp[(1) - (5)].number) & 0xff) {
		case FP_EFUN: {
		    int *argp;
		    int f = (yyvsp[(1) - (5)].number) >>8;
		    int num = (yyvsp[(3) - (5)].node)->kind;
		    int max_arg = predefs[f].max_args;
		    if(f!=-1){
		      if (num > max_arg && max_arg != -1) {
			parse_node_t *pn = (yyvsp[(3) - (5)].node);
			
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
			parse_node_t *enode = (yyvsp[(3) - (5)].node);
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
    break;

  case 213:

/* Line 1806 of yacc.c  */
#line 3260 "grammar.y"
    {
#line 3162 "grammar.y.pre"
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
		 (yyval.node)->l.expr = (yyvsp[(2) - (4)].node);
		 if ((yyvsp[(2) - (4)].node)->kind == NODE_STRING)
		     yywarn("Function pointer returning string constant is NOT a function call");
		 (yyval.node)->r.expr = current_function_context->values_list->r.expr;
		 (yyval.node)->v.number = FP_FUNCTIONAL + current_function_context->bindable
		     + (current_function_context->num_parameters << 8);
		 pop_function_context();
             }
    break;

  case 214:

/* Line 1806 of yacc.c  */
#line 3283 "grammar.y"
    {
#line 3184 "grammar.y.pre"
#ifdef WOMBLES
	        if(*(outp-2) != ']')
		  yyerror("End of mapping not found");
#endif
		CREATE_CALL((yyval.node), F_AGGREGATE_ASSOC, TYPE_MAPPING, (yyvsp[(2) - (4)].node));
	    }
    break;

  case 215:

/* Line 1806 of yacc.c  */
#line 3292 "grammar.y"
    {
#line 3192 "grammar.y.pre"
#ifdef WOMBLES
	        if(*(outp-2) != '}')
		  yyerror("End of array not found");
#endif  
		CREATE_CALL((yyval.node), F_AGGREGATE, TYPE_ANY | TYPE_MOD_ARRAY, (yyvsp[(2) - (4)].node));
	    }
    break;

  case 216:

/* Line 1806 of yacc.c  */
#line 3304 "grammar.y"
    {
#line 3203 "grammar.y.pre"
		(yyval.node) = (yyvsp[(1) - (1)].decl).node;
	    }
    break;

  case 217:

/* Line 1806 of yacc.c  */
#line 3309 "grammar.y"
    {
#line 3207 "grammar.y.pre"
		(yyval.node) = insert_pop_value((yyvsp[(2) - (3)].node));
	    }
    break;

  case 218:

/* Line 1806 of yacc.c  */
#line 3317 "grammar.y"
    {
#line 3214 "grammar.y.pre"
		(yyval.number) = context;
		context = SPECIAL_CONTEXT;
	    }
    break;

  case 219:

/* Line 1806 of yacc.c  */
#line 3323 "grammar.y"
    {
#line 3219 "grammar.y.pre"
		CREATE_CATCH((yyval.node), (yyvsp[(3) - (3)].node));
		context = (yyvsp[(2) - (3)].number);
	    }
    break;

  case 220:

/* Line 1806 of yacc.c  */
#line 3333 "grammar.y"
    {
#line 3243 "grammar.y.pre"
		int p = (yyvsp[(6) - (7)].node)->v.number;
		CREATE_LVALUE_EFUN((yyval.node), TYPE_NUMBER, (yyvsp[(6) - (7)].node));
		CREATE_BINARY_OP_1((yyval.node)->l.expr, F_SSCANF, 0, (yyvsp[(3) - (7)].node), (yyvsp[(5) - (7)].node), p);
	    }
    break;

  case 221:

/* Line 1806 of yacc.c  */
#line 3343 "grammar.y"
    {
#line 3252 "grammar.y.pre"
		int p = (yyvsp[(8) - (9)].node)->v.number;
		CREATE_LVALUE_EFUN((yyval.node), TYPE_NUMBER, (yyvsp[(8) - (9)].node));
		CREATE_TERNARY_OP_1((yyval.node)->l.expr, F_PARSE_COMMAND, 0, 
				    (yyvsp[(3) - (9)].node), (yyvsp[(5) - (9)].node), (yyvsp[(7) - (9)].node), p);
	    }
    break;

  case 222:

/* Line 1806 of yacc.c  */
#line 3354 "grammar.y"
    {
#line 3262 "grammar.y.pre"
		(yyval.number) = context;
		context = SPECIAL_CONTEXT;
	    }
    break;

  case 223:

/* Line 1806 of yacc.c  */
#line 3360 "grammar.y"
    {
#line 3267 "grammar.y.pre"
		CREATE_TIME_EXPRESSION((yyval.node), (yyvsp[(3) - (3)].node));
		context = (yyvsp[(2) - (3)].number);
	    }
    break;

  case 224:

/* Line 1806 of yacc.c  */
#line 3369 "grammar.y"
    {
#line 3275 "grammar.y.pre"
	        (yyval.node) = new_node_no_line();
		(yyval.node)->r.expr = 0;
	        (yyval.node)->v.number = 0;
	    }
    break;

  case 225:

/* Line 1806 of yacc.c  */
#line 3376 "grammar.y"
    {
#line 3281 "grammar.y.pre"
		parse_node_t *insert;
		
		(yyval.node) = (yyvsp[(3) - (3)].node);
		insert = new_node_no_line();
		insert->r.expr = (yyvsp[(3) - (3)].node)->r.expr;
		insert->l.expr = (yyvsp[(2) - (3)].node);
		(yyvsp[(3) - (3)].node)->r.expr = insert;
		(yyval.node)->v.number++;
	    }
    break;

  case 226:

/* Line 1806 of yacc.c  */
#line 3391 "grammar.y"
    {
#line 3295 "grammar.y.pre"
		CREATE_STRING((yyval.node), (yyvsp[(1) - (1)].string));
		scratch_free((yyvsp[(1) - (1)].string));
	    }
    break;

  case 228:

/* Line 1806 of yacc.c  */
#line 3401 "grammar.y"
    {
#line 3304 "grammar.y.pre"
		(yyval.string) = (yyvsp[(2) - (3)].string);
	    }
    break;

  case 229:

/* Line 1806 of yacc.c  */
#line 3406 "grammar.y"
    {
#line 3308 "grammar.y.pre"
		(yyval.string) = scratch_join((yyvsp[(1) - (3)].string), (yyvsp[(3) - (3)].string));
	    }
    break;

  case 231:

/* Line 1806 of yacc.c  */
#line 3415 "grammar.y"
    {
#line 3316 "grammar.y.pre"
		(yyval.string) = scratch_join((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string));
	    }
    break;

  case 232:

/* Line 1806 of yacc.c  */
#line 3422 "grammar.y"
    {
#line 3322 "grammar.y.pre"
	(yyval.node) = new_node();
	(yyval.node)->l.expr = (parse_node_t *)(yyvsp[(1) - (3)].string);
	(yyval.node)->v.expr = (yyvsp[(3) - (3)].node);
	(yyval.node)->r.expr = 0;
    }
    break;

  case 233:

/* Line 1806 of yacc.c  */
#line 3433 "grammar.y"
    {
#line 3332 "grammar.y.pre"
	(yyval.node) = 0;
    }
    break;

  case 234:

/* Line 1806 of yacc.c  */
#line 3438 "grammar.y"
    {
#line 3336 "grammar.y.pre"
	(yyval.node) = (yyvsp[(3) - (3)].node);
	(yyval.node)->r.expr = (yyvsp[(1) - (3)].node);
    }
    break;

  case 235:

/* Line 1806 of yacc.c  */
#line 3448 "grammar.y"
    {
#line 3345 "grammar.y.pre"
		(yyval.number) = context;
		(yyvsp[(2) - (2)].number) = num_refs;
		context |= ARG_LIST; 
	    }
    break;

  case 236:

/* Line 1806 of yacc.c  */
#line 3455 "grammar.y"
    {
#line 3351 "grammar.y.pre"
		context = (yyvsp[(3) - (5)].number);
		(yyval.node) = validate_efun_call((yyvsp[(1) - (5)].number),(yyvsp[(4) - (5)].node));
		(yyval.node) = check_refs(num_refs - (yyvsp[(2) - (5)].number), (yyvsp[(4) - (5)].node), (yyval.node));
		num_refs = (yyvsp[(2) - (5)].number);
	    }
    break;

  case 237:

/* Line 1806 of yacc.c  */
#line 3463 "grammar.y"
    {
#line 3358 "grammar.y.pre"
		(yyval.number) = context;
		(yyvsp[(2) - (2)].number) = num_refs;
		context |= ARG_LIST;
	    }
    break;

  case 238:

/* Line 1806 of yacc.c  */
#line 3470 "grammar.y"
    {
#line 3364 "grammar.y.pre"
		ident_hash_elem_t *ihe;
		int f;

		context = (yyvsp[(3) - (5)].number);
		ihe = lookup_ident("clone_object");

		if ((f = ihe->dn.simul_num) != -1) {
		    (yyval.node) = (yyvsp[(4) - (5)].node);
		    (yyval.node)->kind = NODE_CALL_1;
		    (yyval.node)->v.number = F_SIMUL_EFUN;
		    (yyval.node)->l.number = f;
		    (yyval.node)->type = (SIMUL(f)->type) & ~DECL_MODS;
		} else {
		    (yyval.node) = validate_efun_call(lookup_predef("clone_object"), (yyvsp[(4) - (5)].node));
#ifdef CAST_CALL_OTHERS
		    (yyval.node)->type = TYPE_UNKNOWN;
#else
		    (yyval.node)->type = TYPE_ANY;
#endif		  
		}
		(yyval.node) = check_refs(num_refs - (yyvsp[(2) - (5)].number), (yyvsp[(4) - (5)].node), (yyval.node));
		num_refs = (yyvsp[(2) - (5)].number);
            }
    break;

  case 239:

/* Line 1806 of yacc.c  */
#line 3496 "grammar.y"
    {
#line 3389 "grammar.y.pre"
		parse_node_t *node;
		
		if ((yyvsp[(4) - (6)].ihe)->dn.class_num == -1) {
		    char buf[256];
		    char *end = EndOf(buf);
		    char *p;
		    
		    p = strput(buf, end, "Undefined class '");
		    p = strput(p, end, (yyvsp[(4) - (6)].ihe)->name);
		    p = strput(p, end, "'");
		    yyerror(buf);
		    CREATE_ERROR((yyval.node));
		    node = (yyvsp[(5) - (6)].node);
		    while (node) {
			scratch_free((char *)node->l.expr);
			node = node->r.expr;
		    }
		} else {
		    int type = (yyvsp[(4) - (6)].ihe)->dn.class_num | TYPE_MOD_CLASS;
		    
		    if ((node = (yyvsp[(5) - (6)].node))) {
			CREATE_TWO_VALUES((yyval.node), type, 0, 0);
			(yyval.node)->l.expr = reorder_class_values((yyvsp[(4) - (6)].ihe)->dn.class_num,
							node);
			CREATE_OPCODE_1((yyval.node)->r.expr, F_NEW_CLASS,
					type, (yyvsp[(4) - (6)].ihe)->dn.class_num);
			
		    } else {
			CREATE_OPCODE_1((yyval.node), F_NEW_EMPTY_CLASS,
					type, (yyvsp[(4) - (6)].ihe)->dn.class_num);
		    }
		}
            }
    break;

  case 240:

/* Line 1806 of yacc.c  */
#line 3532 "grammar.y"
    {
#line 3424 "grammar.y.pre"
		parse_node_t *node;
		char buf[256];
		char *end = EndOf(buf);
		char *p;

		p = strput(buf, end, "Undefined class '");
		p = strput(p, end, (yyvsp[(4) - (6)].string));
		p = strput(p, end, "'");
		yyerror(buf);
		CREATE_ERROR((yyval.node));
		node = (yyvsp[(5) - (6)].node);
		while (node) {
		    scratch_free((char *)node->l.expr);
		    node = node->r.expr;
		}
	    }
    break;

  case 241:

/* Line 1806 of yacc.c  */
#line 3551 "grammar.y"
    {
#line 3442 "grammar.y.pre"
		(yyval.number) = context;
		(yyvsp[(2) - (2)].number) = num_refs;
		context |= ARG_LIST;
	    }
    break;

  case 242:

/* Line 1806 of yacc.c  */
#line 3558 "grammar.y"
    {
#line 3448 "grammar.y.pre"
		int f;
		
		context = (yyvsp[(3) - (5)].number);
		(yyval.node) = (yyvsp[(4) - (5)].node);
		if ((f = (yyvsp[(1) - (5)].ihe)->dn.function_num) != -1) {
		    if (current_function_context)
			current_function_context->bindable = FP_NOT_BINDABLE;
		    
		    (yyval.node)->kind = NODE_CALL_1;
		    (yyval.node)->v.number = F_CALL_FUNCTION_BY_ADDRESS;
		    (yyval.node)->l.number = f;
		    (yyval.node)->type = validate_function_call(f, (yyvsp[(4) - (5)].node)->r.expr);
		} else if ((f=(yyvsp[(1) - (5)].ihe)->dn.simul_num) != -1) {
		    (yyval.node)->kind = NODE_CALL_1;
		    (yyval.node)->v.number = F_SIMUL_EFUN;
		    (yyval.node)->l.number = f;
		    (yyval.node)->type = (SIMUL(f)->type) & ~DECL_MODS;
		} else if ((f=(yyvsp[(1) - (5)].ihe)->dn.efun_num) != -1) {
		    (yyval.node) = validate_efun_call(f, (yyvsp[(4) - (5)].node));
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
		    
		    f = define_new_function((yyvsp[(1) - (5)].ihe)->name, 0, 0, 
					    DECL_PUBLIC|FUNC_UNDEFINED, TYPE_ANY);
		    (yyval.node)->kind = NODE_CALL_1;
		    (yyval.node)->v.number = F_CALL_FUNCTION_BY_ADDRESS;
		    (yyval.node)->l.number = f;
		    (yyval.node)->type = TYPE_ANY; /* just a guess */
		    if (exact_types) {
			char buf[256];
			char *end = EndOf(buf);
			char *p;
			const char *n = (yyvsp[(1) - (5)].ihe)->name;
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
		(yyval.node) = check_refs(num_refs - (yyvsp[(2) - (5)].number), (yyvsp[(4) - (5)].node), (yyval.node));
		num_refs = (yyvsp[(2) - (5)].number);
	    }
    break;

  case 243:

/* Line 1806 of yacc.c  */
#line 3620 "grammar.y"
    {
#line 3509 "grammar.y.pre"
		(yyval.number) = context;
		(yyvsp[(2) - (2)].number) = num_refs;
		context |= ARG_LIST;
	    }
    break;

  case 244:

/* Line 1806 of yacc.c  */
#line 3627 "grammar.y"
    {
#line 3515 "grammar.y.pre"
	      char *name = (yyvsp[(1) - (5)].string);

	      context = (yyvsp[(3) - (5)].number);
	      (yyval.node) = (yyvsp[(4) - (5)].node);
	      
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
		      (yyval.node)->type = validate_function_call(f, (yyvsp[(4) - (5)].node)->r.expr);
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
		      char *n = (yyvsp[(1) - (5)].string);
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
	      (yyval.node) = check_refs(num_refs - (yyvsp[(2) - (5)].number), (yyvsp[(4) - (5)].node), (yyval.node));
	      num_refs = (yyvsp[(2) - (5)].number);
	      scratch_free(name);
	  }
    break;

  case 245:

/* Line 1806 of yacc.c  */
#line 3693 "grammar.y"
    {
#line 3580 "grammar.y.pre"
		(yyval.number) = context;
		(yyvsp[(4) - (4)].number) = num_refs;
		context |= ARG_LIST;
	    }
    break;

  case 246:

/* Line 1806 of yacc.c  */
#line 3700 "grammar.y"
    {
#line 3586 "grammar.y.pre"
		ident_hash_elem_t *ihe;
		int f;
		parse_node_t *pn1, *pn2;
		
		(yyvsp[(6) - (7)].node)->v.number += 2;

		pn1 = new_node_no_line();
		pn1->type = 0;
		pn1->v.expr = (yyvsp[(1) - (7)].node);
		pn1->kind = (yyvsp[(6) - (7)].node)->v.number;
		
		pn2 = new_node_no_line();
		pn2->type = 0;
		CREATE_STRING(pn2->v.expr, (yyvsp[(3) - (7)].string));
		scratch_free((yyvsp[(3) - (7)].string));
		
		/* insert the two nodes */
		pn2->r.expr = (yyvsp[(6) - (7)].node)->r.expr;
		pn1->r.expr = pn2;
		(yyvsp[(6) - (7)].node)->r.expr = pn1;
		
		if (!(yyvsp[(6) - (7)].node)->l.expr) (yyvsp[(6) - (7)].node)->l.expr = pn2;
		    
		context = (yyvsp[(5) - (7)].number);
		ihe = lookup_ident("call_other");

		if ((f = ihe->dn.simul_num) != -1) {
		    (yyval.node) = (yyvsp[(6) - (7)].node);
		    (yyval.node)->kind = NODE_CALL_1;
		    (yyval.node)->v.number = F_SIMUL_EFUN;
		    (yyval.node)->l.number = f;
		    (yyval.node)->type = (SIMUL(f)->type) & ~DECL_MODS;
		} else {
		    (yyval.node) = validate_efun_call(arrow_efun, (yyvsp[(6) - (7)].node));
#ifdef CAST_CALL_OTHERS
		    (yyval.node)->type = TYPE_UNKNOWN;
#else
		    (yyval.node)->type = TYPE_ANY;
#endif		  
		}
		(yyval.node) = check_refs(num_refs - (yyvsp[(4) - (7)].number), (yyvsp[(6) - (7)].node), (yyval.node));
		num_refs = (yyvsp[(4) - (7)].number);
	    }
    break;

  case 247:

/* Line 1806 of yacc.c  */
#line 3746 "grammar.y"
    {
#line 3631 "grammar.y.pre"
		(yyval.number) = context;
		(yyvsp[(5) - (5)].number) = num_refs;
		context |= ARG_LIST;
	    }
    break;

  case 248:

/* Line 1806 of yacc.c  */
#line 3753 "grammar.y"
    {
#line 3637 "grammar.y.pre"
	        parse_node_t *expr;

		context = (yyvsp[(6) - (8)].number);
		(yyval.node) = (yyvsp[(7) - (8)].node);
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
		expr->v.expr = (yyvsp[(3) - (8)].node);
		expr->r.expr = (yyval.node)->r.expr;
		(yyval.node)->r.expr = expr;
		(yyval.node) = check_refs(num_refs - (yyvsp[(5) - (8)].number), (yyvsp[(7) - (8)].node), (yyval.node));
		num_refs = (yyvsp[(5) - (8)].number);
	    }
    break;

  case 249:

/* Line 1806 of yacc.c  */
#line 3777 "grammar.y"
    {
#line 3660 "grammar.y.pre"
	svalue_t *res;
	ident_hash_elem_t *ihe;

	(yyval.number) = (ihe = lookup_ident((yyvsp[(3) - (3)].string))) ? ihe->dn.efun_num : -1;
	if ((yyval.number) == -1) {
	    char buf[256];
	    char *end = EndOf(buf);
	    char *p;
	    
	    p = strput(buf, end, "Unknown efun: ");
	    p = strput(p, end, (yyvsp[(3) - (3)].string));
	    yyerror(buf);
	} else {
	    push_malloced_string(the_file_name(current_file));
	    share_and_push_string((yyvsp[(3) - (3)].string));
	    push_malloced_string(add_slash(main_file_name()));
	    res = safe_apply_master_ob(APPLY_VALID_OVERRIDE, 3);
	    if (!MASTER_APPROVED(res)) {
		yyerror("Invalid simulated efunction override");
		(yyval.number) = -1;
	    }
	}
	scratch_free((yyvsp[(3) - (3)].string));
      }
    break;

  case 250:

/* Line 1806 of yacc.c  */
#line 3803 "grammar.y"
    {
#line 3685 "grammar.y.pre"
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
    break;

  case 252:

/* Line 1806 of yacc.c  */
#line 3821 "grammar.y"
    {
#line 3702 "grammar.y.pre"
		int l = strlen((yyvsp[(2) - (2)].string)) + 1;
		char *p;
		/* here we be a bit cute.  we put a : on the front so we
		 * don't have to strchr for it.  Here we do:
		 * "name" -> ":::name"
		 */
		(yyval.string) = scratch_realloc((yyvsp[(2) - (2)].string), l + 3);
		p = (yyval.string) + l;
		while (p--,l--)
		    *(p+3) = *p;
		strncpy((yyval.string), ":::", 3);
	    }
    break;

  case 253:

/* Line 1806 of yacc.c  */
#line 3836 "grammar.y"
    {
#line 3716 "grammar.y.pre"
		int z, l = strlen((yyvsp[(3) - (3)].string)) + 1;
		char *p;
		/* <type> and "name" -> ":type::name" */
		z = strlen(compiler_type_names[(yyvsp[(1) - (3)].number)]) + 3; /* length of :type:: */
		(yyval.string) = scratch_realloc((yyvsp[(3) - (3)].string), l + z);
		p = (yyval.string) + l;
		while (p--,l--)
		    *(p+z) = *p;
		(yyval.string)[0] = ':';
		strncpy((yyval.string) + 1, compiler_type_names[(yyvsp[(1) - (3)].number)], z - 3);
		(yyval.string)[z-2] = ':';
		(yyval.string)[z-1] = ':';
	    }
    break;

  case 254:

/* Line 1806 of yacc.c  */
#line 3852 "grammar.y"
    {
#line 3731 "grammar.y.pre"
		int l = strlen((yyvsp[(1) - (3)].string));
		/* "ob" and "name" -> ":ob::name" */
		(yyval.string) = scratch_alloc(l + strlen((yyvsp[(3) - (3)].string)) + 4);
		*((yyval.string)) = ':';
		strcpy((yyval.string) + 1, (yyvsp[(1) - (3)].string));
		strcpy((yyval.string) + l + 1, "::");
		strcpy((yyval.string) + l + 3, (yyvsp[(3) - (3)].string));
		scratch_free((yyvsp[(1) - (3)].string));
		scratch_free((yyvsp[(3) - (3)].string));
	    }
    break;

  case 255:

/* Line 1806 of yacc.c  */
#line 3868 "grammar.y"
    {
#line 3746 "grammar.y.pre"
		/* x != 0 -> x */
		if (IS_NODE((yyvsp[(3) - (6)].node), NODE_BINARY_OP, F_NE)) {
		    if (IS_NODE((yyvsp[(3) - (6)].node)->r.expr, NODE_NUMBER, 0))
			(yyvsp[(3) - (6)].node) = (yyvsp[(3) - (6)].node)->l.expr;
		    else if (IS_NODE((yyvsp[(3) - (6)].node)->l.expr, NODE_NUMBER, 0))
			     (yyvsp[(3) - (6)].node) = (yyvsp[(3) - (6)].node)->r.expr;
		}

		/* TODO: should optimize if (0), if (1) here.  
		 * Also generalize this.
		 */

		if ((yyvsp[(5) - (6)].node) == 0) {
		    if ((yyvsp[(6) - (6)].node) == 0) {
			/* if (x) ; -> x; */
			(yyval.node) = pop_value((yyvsp[(3) - (6)].node));
			break;
		    } else {
			/* if (x) {} else y; -> if (!x) y; */
			parse_node_t *repl;
			
			CREATE_UNARY_OP(repl, F_NOT, TYPE_NUMBER, (yyvsp[(3) - (6)].node));
			(yyvsp[(3) - (6)].node) = repl;
			(yyvsp[(5) - (6)].node) = (yyvsp[(6) - (6)].node);
			(yyvsp[(6) - (6)].node) = 0;
		    }
		}
		CREATE_IF((yyval.node), (yyvsp[(3) - (6)].node), (yyvsp[(5) - (6)].node), (yyvsp[(6) - (6)].node));
	    }
    break;

  case 256:

/* Line 1806 of yacc.c  */
#line 3903 "grammar.y"
    {
#line 3780 "grammar.y.pre"
		(yyval.node) = 0;
	    }
    break;

  case 257:

/* Line 1806 of yacc.c  */
#line 3908 "grammar.y"
    {
#line 3784 "grammar.y.pre"
		(yyval.node) = (yyvsp[(2) - (2)].node);
            }
    break;



/* Line 1806 of yacc.c  */
#line 6788 "y.tab.c"
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

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
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

  /* Do not reclaim the symbols of the rule which action triggered
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
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

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

  *++yyvsp = yylval;


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

#if !defined(yyoverflow) || YYERROR_VERBOSE
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
  /* Do not reclaim the symbols of the rule which action triggered
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
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 3913 "grammar.y"



#line 3789 "grammar.y.pre"

