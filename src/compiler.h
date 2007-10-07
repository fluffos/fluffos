#ifndef COMPILER_H
#define COMPILER_H

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

#define START_BLOCK_SIZE        4096

/* NUMPAREAS ares are saved with the program code after compilation,
 * the rest are only temporary.
 */
#define A_PROGRAM               0       /* executable code */
#define A_FUNCTIONS             1
#define A_STRINGS               2       /* table of strings */
#define A_VAR_NAME              3
#define A_VAR_TYPE              4
#define A_LINENUMBERS           5       /* linenumber information */
#define A_FILE_INFO             6       /* start of file line nos */
#define A_INHERITS              7       /* table of inherited progs */
#define A_CLASS_DEF             8
#define A_CLASS_MEMBER          9
#define A_ARGUMENT_TYPES        10      /* */
#define A_ARGUMENT_INDEX        11      /* */
#define NUMPAREAS               12
#define A_CASES                 13      /* keep track of cases */
#define A_STRING_NEXT           14      /* next prog string in hash chain */
#define A_STRING_REFS           15      /* reference count of prog string */
#define A_INCLUDES              16      /* list of included files */
#define A_PATCH                 17      /* for save_binary() */
#define A_FUNCTIONALS           18
#define A_FUNCTION_DEFS         19
#define A_VAR_TEMP              20      /* table of variables */
#define NUMAREAS                22

#define TREE_MAIN               0
#define TREE_INIT               1
#define NUMTREES                2

#define CURRENT_PROGRAM_SIZE (prog_code - mem_block[A_PROGRAM].block)
#define UPDATE_PROGRAM_SIZE mem_block[A_PROGRAM].current_size = CURRENT_PROGRAM_SIZE

/*
 * Types available. The number '0' is valid as any type. These types
 * are only used by the compiler, when type checks are enabled. Compare with
 * the run-time types, named T_ interpret.h.
 */

#define TYPE_UNKNOWN    0       /* This type must be casted */
#define TYPE_ANY        1       /* Will match any type */
#define TYPE_NOVALUE    2
#define TYPE_VOID       3
#define TYPE_NUMBER     4
#define TYPE_STRING     5
#define TYPE_OBJECT     6
#define TYPE_MAPPING    7
#define TYPE_FUNCTION   8
#define TYPE_REAL       9
#define TYPE_BUFFER     10
#define TYPE_MASK       0xf

typedef struct {
    int runtime_index;
    ident_hash_elem_t *ihe;
} local_info_t;

extern mem_block_t mem_block[NUMAREAS];
extern const char *compiler_type_names[];

#define LOOP_CONTEXT            0x1
#define SWITCH_CONTEXT          0x2
#define SWITCH_STRINGS          0x4
#define SWITCH_NUMBERS          0x8
#define SWITCH_DEFAULT          0x10
#define SWITCH_RANGES           0x20
#define SWITCH_NOT_EMPTY        0x40
#define LOOP_FOREACH            0x80
#define SPECIAL_CONTEXT         0x100
#define ARG_LIST                0x200


typedef struct function_context_s {
    parse_node_t *values_list;
    short bindable;
    short num_parameters;
    short num_locals;
    struct function_context_s *parent;
} function_context_t;

extern function_context_t *current_function_context;
extern int var_defined;
extern parse_node_t *comp_trees[NUMTREES];
extern unsigned short *comp_def_index_map;
extern unsigned short *func_index_map;

typedef struct compiler_temp_t {
    unsigned short flags;
    unsigned short offset;
    unsigned short function_index_offset;
    struct program_s *prog; /* inherited if nonzero */
    union {
        function_t *func;
        long index;
    } u;
    struct compiler_temp_t *next;
} compiler_temp_t;

/*
 * Some good macros to have.
 */

#define IS_CLASS(t) ((t & (TYPE_MOD_ARRAY | TYPE_MOD_CLASS)) == TYPE_MOD_CLASS)
#define CLASS_IDX(t) (t & ~(DECL_MODS | TYPE_MOD_CLASS))

#define COMP_TYPE(e, t) (!(e & (TYPE_MOD_ARRAY | TYPE_MOD_CLASS)) \
                         && (compatible[(e & ~DECL_MODS)] & (1 << (t))))
#define IS_TYPE(e, t) (!(e & (TYPE_MOD_ARRAY | TYPE_MOD_CLASS)) \
                       && (is_type[(e & ~DECL_MODS)] & (1 << (t))))

#define FUNCTION_TEMP(n) ((compiler_temp_t *)mem_block[A_FUNCTION_DEFS].block + (n))
#define FUNCTION_NEXT(n) (FUNCTION_TEMP(n)->next)
/* function_t from A_FUNCTIONS index */
#define FUNC(n) ((function_t *)mem_block[A_FUNCTIONS].block + (n))
/* program for inherited entry from full function index */
#define FUNCTION_PROG(n) (FUNCTION_TEMP(n)->prog)
#define FUNCTION_ALIAS(n) (FUNCTION_TEMP(n)->alias_for)
/* function_t from full function index */
#define FUNCTION_DEF(n) (FUNCTION_PROG(n) ? FUNCTION_TEMP(n)->u.func : FUNC(FUNCTION_TEMP(n)->u.index))
/* flags from full function index */
#define FUNCTION_FLAGS(n) (FUNCTION_TEMP(n)->flags)

#define NUM_INHERITS (mem_block[A_INHERITS].current_size / sizeof(inherit_t))

#define INHERIT(n)  ((inherit_t *)mem_block[A_INHERITS].block + (n))
#define VAR_TEMP(n) ((variable_t *)mem_block[A_VAR_TEMP].block + (n))
#define SIMUL(n)    (simuls[n].func)
#define PROG_STRING(n)   (((const char **)mem_block[A_STRINGS].block)[n])
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

int validate_function_call (int, parse_node_t *);
parse_node_t *validate_efun_call (int, parse_node_t *);
extern mem_block_t mem_block[];
extern int exact_types, global_modifiers;
extern int current_type;
extern char *prog_code;
extern char *prog_code_max;
extern program_t NULL_program;
extern unsigned char string_tags[0x20];
extern short freed_string;
extern local_info_t *locals, *locals_ptr;
extern unsigned short *type_of_locals, *type_of_locals_ptr;
extern int current_number_of_locals;
extern int max_num_locals;
extern int current_tree;

extern int type_of_locals_size;
extern int locals_size;
extern int current_number_of_locals;
extern int max_num_locals;
extern short compatible[11];
extern short is_type[11];
extern int comp_last_inherited;

char *get_type_modifiers (char *, char *, int);
char *get_two_types (char *, char *, int, int);
char *get_type_name (char *, char *, int);
void init_locals (void);

void save_file_info (int, int);
int add_program_file (char *, int);
void yyerror (const char *);
void yywarn (const char *);
char *the_file_name (char *);
void free_all_local_names (int);
void pop_n_locals (int);
void reactivate_current_locals (void);
void clean_up_locals (void);
void deactivate_current_locals (void);
int add_local_name (const char *, int);
void reallocate_locals (void);
void initialize_locals (void);
int get_id_number (void);
program_t *compile_file (int, char *);
void reset_function_blocks (void);
void copy_variables (program_t *, int);
void copy_structures (program_t *);
int copy_functions (program_t *, int);
void type_error (const char *, int);
int compatible_types (int, int);
int compatible_types2 (int, int);
int arrange_call_inherited (char *, parse_node_t *);
void add_arg_type (unsigned short);
int define_new_function (const char *, int, int, int, int);
int define_variable (char *, int);
int define_new_variable (char *, int);
short store_prog_string (const char *);
void free_prog_string (short);
#ifdef DEBUG
int dump_function_table (void);
#endif
void prepare_cases (parse_node_t *, int);
void push_func_block (void);
void pop_func_block (void);
int decl_fix (int);
parse_node_t *check_refs (int, parse_node_t *, parse_node_t *);

int lookup_any_class_member (char *, unsigned char *);
int lookup_class_member (int, char *, unsigned char *);
parse_node_t *reorder_class_values (int, parse_node_t *);

parse_node_t *promote_to_float (parse_node_t *);
parse_node_t *promote_to_int (parse_node_t *);
int convert_type (int);
parse_node_t *add_type_check (parse_node_t *, int);
parse_node_t *do_promotions (parse_node_t *, int);
parse_node_t *throw_away_call (parse_node_t *);
parse_node_t *throw_away_mapping (parse_node_t *);

#define realloc_mem_block(m) do { \
    mem_block_t *M = m; \
    M->max_size <<= 1; \
    M->block = DREALLOC(M->block, M->max_size, TAG_COMPILER, "realloc_mem_block"); \
} while (0)

#define add_to_mem_block(n, data, size) do { \
    mem_block_t *mbp = &mem_block[n]; \
    int Size = size; \
    \
    if (mbp->current_size + Size > mbp->max_size) { \
        do { \
            mbp->max_size <<= 1; \
        } while (mbp->current_size + Size > mbp->max_size); \
        \
        mbp->block = DREALLOC(mbp->block, mbp->max_size, TAG_COMPILER, "insert_in_mem_block"); \
    } \
    memcpy(mbp->block + mbp->current_size, data, Size); \
    mbp->current_size += Size; \
} while (0)

#ifndef SUPPRESS_COMPILER_INLINES
INLINE_STATIC
char *allocate_in_mem_block (int n, int size)
{
    mem_block_t *mbp = &mem_block[n];
    char *ret;

    if (mbp->current_size + size > mbp->max_size) {
        do {
            mbp->max_size <<= 1;
        } while (mbp->current_size + size > mbp->max_size);
        
        mbp->block = DREALLOC(mbp->block, mbp->max_size, TAG_COMPILER, "insert_in_mem_block");
    }
    ret = mbp->block + mbp->current_size;
    mbp->current_size += size;
    return ret;
}

#endif
#endif






