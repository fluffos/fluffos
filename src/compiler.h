#ifndef COMPILER_H
#define COMPILER_H

#include "std.h"
#include "incralloc.h"
#include "trees.h"
#include "instrs.h"
#include "program.h"

#define _YACC_
#ifdef sun
#include <malloc.h>
#endif

#define YYMAXDEPTH    600

/* NUMPAREAS ares are saved with the program code after compilation,
 * the rest are only temporary.
 */
#define A_PROGRAM		0	/* executable code */
#define A_FUNCTIONS		1	/* table of functions */
#define A_STRINGS		2	/* table of strings */
#define A_VARIABLES		3	/* table of variables */
#define A_LINENUMBERS		4	/* linenumber information */
#define A_FILE_INFO             5       /* start of file line nos */
#define A_INHERITS		6	/* table of inherited progs */
#define A_ARGUMENT_TYPES	7	/* */
#define A_ARGUMENT_INDEX	8	/* */
#define NUMPAREAS		8
#define A_CASES                 9       /* keep track of cases */
#define A_STRING_NEXT		10	/* next prog string in hash chain */
#define A_STRING_REFS		11	/* reference count of prog string */
#define A_INCLUDES		12	/* list of included files */
#define A_PATCH			13	/* for save_binary() */
#define A_INITIALIZER           14
#define NUMAREAS		15

/*
 * Types available. The number '0' is valid as any type. These types
 * are only used by the compiler, when type checks are enabled. Compare with
 * the run-time types, named T_ interpret.h.
 */

#define TYPE_UNKNOWN	0	/* This type must be casted */
#define TYPE_VOID       1
#define TYPE_NUMBER     2
#define TYPE_STRING     3
#define TYPE_OBJECT     4
#define TYPE_MAPPING    5
#define TYPE_FUNCTION   6
#define TYPE_REAL       7
#define TYPE_BUFFER     8
#define TYPE_ANY        9	/* Will match any type */

extern struct mem_block mem_block[NUMAREAS];

#define CURRENT_PROGRAM_SIZE prog_code - mem_block[current_block].block
#define UPDATE_PROGRAM_SIZE mem_block[current_block].current_size = CURRENT_PROGRAM_SIZE

#define LOOP_CONTEXT            0x1
#define SWITCH_CONTEXT          0x2
#define SWITCH_STRINGS          0x4
#define SWITCH_NUMBERS          0x8
#define SWITCH_DEFAULT          0x10

#ifdef NEW_FUNCTIONS
struct function_context_t {
    short bindable;
    short num_parameters;
    short num_locals;
    struct parse_node *values_list;
};

extern struct function_context_t function_context;
#endif

/*
 * Some good macros to have.
 */

#define BASIC_TYPE(e,t) ((e) == TYPE_ANY ||\
                         (e) == (t) ||\
                         (t) == TYPE_ANY)

#define TYPE(e,t) (BASIC_TYPE((e) & TYPE_MOD_MASK, (t) & TYPE_MOD_MASK) ||\
                (((e) & TYPE_MOD_POINTER) && ((t) & TYPE_MOD_POINTER) &&\
                BASIC_TYPE((e) & (TYPE_MOD_MASK & ~TYPE_MOD_POINTER),\
                (t) & (TYPE_MOD_MASK & ~TYPE_MOD_POINTER))))

#define FUNCTION(n) ((struct function *)mem_block[A_FUNCTIONS].block + (n))
#define VARIABLE(n) ((struct variable *)mem_block[A_VARIABLES].block + (n))
#define SIMUL(n)    (simuls[n])
#define PROG_STRING(n)   (((char **)mem_block[A_STRINGS].block)[n])

#if !defined(__alpha) && !defined(cray)
#define align(x) (((x) + 3) & ~3)
#else
#define align(x) (((x) + 7) & ~7)
#endif

#define SOME_NUMERIC_CASE_LABELS 0x40000
#define NO_STRING_CASE_LABELS    0x80000

int validate_function_call PROT((struct function *, int, struct parse_node *));
struct parse_node *validate_efun_call PROT((int, struct parse_node *));
extern struct mem_block mem_block[];
extern int exact_types;
extern int approved_object;
extern int current_type;
extern int current_block;
extern char *prog_code;
extern char *prog_code_max;
extern struct program NULL_program;
extern struct program *prog;
extern unsigned char string_tags[0x20];
extern short freed_string;
extern struct ident_hash_elem **locals;
extern unsigned short *type_of_locals;
extern char *runtime_locals;
extern int current_number_of_locals;
extern int max_num_locals;
extern unsigned short *type_of_locals_ptr;
extern struct ident_hash_elem **locals_ptr;
extern char *runtime_locals_ptr;

extern int type_of_locals_size;
extern int locals_size;
extern int current_number_of_locals;
extern int max_num_locals;
extern unsigned short a_functions_root;
extern struct mem_block type_of_arguments;
extern struct function_block_info_t *func_block;

char *get_two_types PROT((int, int));
char *get_type_name PROT((int));
void init_locals PROT((void));

void save_file_info PROT((int, int));
int add_program_file PROT((char *, int));
void yyerror PROT((char *));
void yywarn PROT((char *));
void start_initializer PROT((void));
void end_initializer PROT((void));
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
void compile_file PROT((int, char *));
void reset_function_blocks PROT((void));
void copy_variables PROT((struct program *, int));
int copy_functions PROT((struct program *, int));
void type_error PROT((char *, int));
int compatible_types PROT((int, int));
void arrange_call_inherited PROT((char *, struct parse_node *));
void add_arg_type PROT((unsigned short));
int find_in_table PROT((struct function *, int));
void find_inherited PROT((struct function *));
int define_new_function PROT((char *, int, int, int, int, int));
int define_variable PROT((char *, int, int));
short store_prog_string PROT((char *));
void free_prog_string PROT((short));
#ifdef DEBUG
int dump_function_table PROT((void));
#endif
void prepare_cases PROT((struct parse_node *, int));
void push_func_block PROT((void));
void pop_func_block PROT((void));

/* inlines - if we're lucky, they'll get honored. */
INLINE static void realloc_mem_block PROT((struct mem_block *, int));
INLINE static void add_to_mem_block PROT((int, char *, int));
INLINE static void insert_in_mem_block PROT((int, int, int));
INLINE static char *allocate_in_mem_block PROT((int, int));

INLINE static
void realloc_mem_block P2(struct mem_block *, m, int, size)
{
    while (size > m->max_size) {
	m->max_size <<= 1;
	m->block = (char *)
	    DREALLOC((char *) m->block, m->max_size, TAG_COMPILER, "realloc_mem_block");
    }
}

INLINE static
void add_to_mem_block P3(int, n, char *, data, int, size)
{
    struct mem_block *mbp = &mem_block[n];

    if (mbp->current_size + size > mbp->max_size)
	realloc_mem_block(mbp, mbp->current_size + size);
    if (data)
	memcpy(mbp->block + mbp->current_size, data, size);
    mbp->current_size += size;
}

INLINE static
char *allocate_in_mem_block P2(int, n, int, size)
{
    struct mem_block *mbp = &mem_block[n];
    char *ret;

    if (mbp->current_size + size > mbp->max_size)
	realloc_mem_block(mbp, mbp->current_size + size);
    ret = mbp->block + mbp->current_size;
    mbp->current_size += size;
    return ret;
}

INLINE static
void insert_in_mem_block P3(int, n, int, where, int, size)
{
    struct mem_block *mbp = &mem_block[n];
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

