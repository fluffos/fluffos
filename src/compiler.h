#ifndef COMPILER_H
#define COMPILER_H

#include "std.h"
#include "trees.h"
#include "lex.h"
#include "program.h"

#define _YACC_

#define YYMAXDEPTH    600

/*
 * Information for allocating a block that can grow dynamically
 * using realloc. That means that no pointers should be kept into such
 * an area, as it might be moved.
 */

typedef struct {
    char *block;
    int current_size;
    int max_size;
} mem_block_t;

#define START_BLOCK_SIZE	4096

/* NUMPAREAS ares are saved with the program code after compilation,
 * the rest are only temporary.
 */
#define A_PROGRAM		0	/* executable code */
#define A_RUNTIME_FUNCTIONS	1	/* table of functions */
#define A_COMPILER_FUNCTIONS    2
#define A_RUNTIME_COMPRESSED	3
#define A_FUNCTION_FLAGS	4
#define A_STRINGS		5	/* table of strings */
#define A_VAR_NAME		6
#define A_VAR_TYPE		7
#define A_LINENUMBERS		8	/* linenumber information */
#define A_FILE_INFO             9       /* start of file line nos */
#define A_INHERITS		10	/* table of inherited progs */
#define A_CLASS_DEF             11
#define A_CLASS_MEMBER          12
#define A_ARGUMENT_TYPES	13	/* */
#define A_ARGUMENT_INDEX	14	/* */
#define NUMPAREAS		15
#define A_CASES                 15      /* keep track of cases */
#define A_STRING_NEXT		16	/* next prog string in hash chain */
#define A_STRING_REFS		17	/* reference count of prog string */
#define A_INCLUDES		18	/* list of included files */
#define A_PATCH			19	/* for save_binary() */
#define A_INITIALIZER           20
#define A_FUNCTIONALS           21
#define A_FUNCTION_DEFS		22
#define A_VAR_TEMP	        23	/* table of variables */
#define NUMAREAS		24

#define CURRENT_PROGRAM_SIZE (prog_code - mem_block[current_block].block)
#define UPDATE_PROGRAM_SIZE mem_block[current_block].current_size = CURRENT_PROGRAM_SIZE

/*
 * Types available. The number '0' is valid as any type. These types
 * are only used by the compiler, when type checks are enabled. Compare with
 * the run-time types, named T_ interpret.h.
 */

#define TYPE_UNKNOWN	0	/* This type must be casted */
#define TYPE_ANY        1	/* Will match any type */
#define TYPE_NOVALUE    2
#define TYPE_VOID       3
#define TYPE_NUMBER     4
#define TYPE_STRING     5
#define TYPE_OBJECT     6
#define TYPE_MAPPING    7
#define TYPE_FUNCTION   8
#define TYPE_REAL       9
#define TYPE_BUFFER     10

extern mem_block_t mem_block[NUMAREAS];
extern char *compiler_type_names[];

#define LOOP_CONTEXT            0x1
#define SWITCH_CONTEXT          0x2
#define SWITCH_STRINGS          0x4
#define SWITCH_NUMBERS          0x8
#define SWITCH_DEFAULT          0x10
#define SWITCH_RANGES           0x20
#define LOOP_FOREACH            0x40
#define SPECIAL_CONTEXT		0x80

typedef struct function_context_s {
    parse_node_t *values_list;
    short bindable;
    short num_parameters;
    short num_locals;
    struct function_context_s *parent;
} function_context_t;

extern function_context_t *current_function_context;
extern int var_defined;

/*
 * Some good macros to have.
 */

#define IS_CLASS(t) ((t & (TYPE_MOD_ARRAY | TYPE_MOD_CLASS)) == TYPE_MOD_CLASS)
#define CLASS_IDX(t) (t & ~(NAME_TYPE_MOD | TYPE_MOD_CLASS))

#define COMP_TYPE(e, t) (!(e & (TYPE_MOD_ARRAY | TYPE_MOD_CLASS)) \
			 && (compatible[(unsigned char)e] & (1 << (t))))
#define IS_TYPE(e, t) (!(e & (TYPE_MOD_ARRAY | TYPE_MOD_CLASS)) \
		       && (is_type[(unsigned char)e] & (1 << (t))))

#define FUNCTION_TEMP(n) ((compiler_temp_t *)mem_block[A_FUNCTION_DEFS].block + (n))
/* compiler_function_t from A_COMPILER_FUNCTIONS index */
#define COMPILER_FUNC(n) ((compiler_function_t *)mem_block[A_COMPILER_FUNCTIONS].block + (n))
/* program for inherited entry from full function index */
#define FUNCTION_PROG(n) (FUNCTION_TEMP(n)->prog)
#define FUNCTION_ALIAS(n) (FUNCTION_TEMP(n)->alias_for)
/* compiler_function_t from full function index */
#define FUNCTION_DEF(n) (FUNCTION_PROG(n) ? FUNCTION_TEMP(n)->u.func : COMPILER_FUNC(FUNCTION_TEMP(n)->u.index))
/* runtime_function_u from full function index */
#define FUNCTION_RENTRY(n) ((runtime_function_u *)mem_block[A_RUNTIME_FUNCTIONS].block + (n))
/* runtime_function_u from full function index, but digs down to the definition.  This is rather complex; maybe it should be stored in FUNCTION_TEMP too */
#define FUNCTION_DEF_RENTRY(n) (FUNCTION_PROG(n) ? FIND_FUNC_ENTRY(FUNCTION_PROG(n), FUNCTION_TEMP(n)->u.func->runtime_index) : FUNCTION_RENTRY(n))
/* flags from full function index */
#define FUNCTION_FLAGS(n) *((unsigned short *)mem_block[A_FUNCTION_FLAGS].block + (n))

#define NUM_INHERITS (mem_block[A_INHERITS].current_size / sizeof(inherit_t))

#define INHERIT(n)  ((inherit_t *)mem_block[A_INHERITS].block + (n))
#define VAR_TEMP(n) ((variable_t *)mem_block[A_VAR_TEMP].block + (n))
#define SIMUL(n)    (simuls[n].func)
#define PROG_STRING(n)   (((char **)mem_block[A_STRINGS].block)[n])
#define CLASS(n)    ((class_def_t *)mem_block[A_CLASS_DEF].block + (n))

#if !defined(__alpha) && !defined(cray)
#define align(x) (((x) + 3) & ~3)
#else
#define align(x) (((x) + 7) & ~7)
#endif

#define SOME_NUMERIC_CASE_LABELS 0x40000
#define NO_STRING_CASE_LABELS    0x80000

#define ARG_IS_PROTO             1
#define ARG_IS_VARARGS           2

#define NOVALUE_USED_FLAG        1024

int validate_function_call PROT((int, parse_node_t *));
parse_node_t *validate_efun_call PROT((int, parse_node_t *));
extern mem_block_t mem_block[];
extern int exact_types, global_modifiers;
extern int current_type;
extern int current_block;
extern char *prog_code;
extern char *prog_code_max;
extern program_t NULL_program;
extern unsigned char string_tags[0x20];
extern short freed_string;
extern ident_hash_elem_t **locals;
extern unsigned short *type_of_locals;
extern char *runtime_locals;
extern int current_number_of_locals;
extern int max_num_locals;
extern unsigned short *type_of_locals_ptr;
extern ident_hash_elem_t **locals_ptr;
extern char *runtime_locals_ptr;

extern int type_of_locals_size;
extern int locals_size;
extern int current_number_of_locals;
extern int max_num_locals;
extern unsigned short a_functions_root;
extern mem_block_t type_of_arguments;
extern short compatible[11];
extern short is_type[11];

char *get_two_types PROT((char *, char *, int, int));
char *get_type_name PROT((char *, char *, int));
void init_locals PROT((void));

void save_file_info PROT((int, int));
int add_program_file PROT((char *, int));
void yyerror PROT((char *));
void yywarn PROT((char *));
void switch_to_block PROT((int));
char *the_file_name PROT((char *));
void free_all_local_names PROT((void));
void pop_n_locals PROT((int));
void reactivate_current_locals PROT((void));
void clean_up_locals PROT((void));
void deactivate_current_locals PROT((void));
int add_local_name PROT((char *, int));
void reallocate_locals PROT((void));
void initialize_locals PROT((void));
int get_id_number PROT((void));
program_t *compile_file PROT((int, char *));
void reset_function_blocks PROT((void));
void copy_variables PROT((program_t *, int));
void copy_structures PROT((program_t *));
int copy_functions PROT((program_t *, int));
void type_error PROT((char *, int));
int compatible_types PROT((int, int));
int compatible_types2 PROT((int, int));
int arrange_call_inherited PROT((char *, parse_node_t *));
void add_arg_type PROT((unsigned short));
int define_new_function PROT((char *, int, int, int, int));
int define_variable PROT((char *, int, int));
int define_new_variable PROT((char *, int));
short store_prog_string PROT((char *));
void free_prog_string PROT((short));
#ifdef DEBUG
int dump_function_table PROT((void));
#endif
void prepare_cases PROT((parse_node_t *, int));
void push_func_block PROT((void));
void pop_func_block PROT((void));

int lookup_class_member PROT((int, char *, char *));
parse_node_t *reorder_class_values PROT((int, parse_node_t *));

parse_node_t *promote_to_float PROT((parse_node_t *));
parse_node_t *promote_to_int PROT((parse_node_t *));
parse_node_t *do_promotions PROT((parse_node_t *, int));
parse_node_t *throw_away_call PROT((parse_node_t *));
parse_node_t *throw_away_mapping PROT((parse_node_t *));

#ifndef SUPPRESS_COMPILER_INLINES
/* inlines - if we're lucky, they'll get honored. */
INLINE_STATIC void realloc_mem_block PROT((mem_block_t *, int));
INLINE_STATIC void add_to_mem_block PROT((int, char *, int));
INLINE_STATIC void insert_in_mem_block PROT((int, int, int));
INLINE_STATIC char *allocate_in_mem_block PROT((int, int));

INLINE_STATIC
void realloc_mem_block P2(mem_block_t *, m, int, size)
{
    while (size > m->max_size) {
	m->max_size <<= 1;
	m->block = (char *)
	    DREALLOC((char *) m->block, m->max_size, TAG_COMPILER, "realloc_mem_block");
    }
}

INLINE_STATIC
void add_to_mem_block P3(int, n, char *, data, int, size)
{
    mem_block_t *mbp = &mem_block[n];

    if (mbp->current_size + size > mbp->max_size)
	realloc_mem_block(mbp, mbp->current_size + size);
    if (data)
	memcpy(mbp->block + mbp->current_size, data, size);
    mbp->current_size += size;
}

INLINE_STATIC
char *allocate_in_mem_block P2(int, n, int, size)
{
    mem_block_t *mbp = &mem_block[n];
    char *ret;

    if (mbp->current_size + size > mbp->max_size)
	realloc_mem_block(mbp, mbp->current_size + size);
    ret = mbp->block + mbp->current_size;
    mbp->current_size += size;
    return ret;
}

INLINE_STATIC
void insert_in_mem_block P3(int, n, int, where, int, size)
{
    mem_block_t *mbp = &mem_block[n];
    char *p;

    if (mbp->current_size + size > mbp->max_size)
	realloc_mem_block(mbp, mbp->current_size + size);
    /* memcpy can't handle overlapping blocks on some systems */
    p = mbp->block + mbp->current_size;
    while (p-- > mbp->block + where)
	*(p + size) = *p;
    mbp->current_size += size;
}
#endif
#endif

