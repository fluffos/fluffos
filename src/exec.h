#ifndef _EXEC_H_
#define _EXEC_H_

/*
 * A compiled program consists of several data blocks, all allocated
 * contiguously in memory to enhance the working set. During compilation,
 * the blocks will be allocated separately, as the final size is
 * unknown. When compilation is done, the blocks will be copied into
 * the one big area.
 *
 * There are 5 different blocks of information for each program:
 * 1. The program itself. Consists of machine code instructions for a virtual
 *    stack machine. The size of the program must not be bigger than
 *    65535 bytes, as 16 bit pointers are used. Who would ever need a bigger
 *    program :-)
 * 2. Function names. All local functions that has been defined or called,
 *    with the address of the function in the program. Inherited functions
 *    will be found here too, with information of how far up the inherit
 *    chain that the function was defined.
 * 3. String table. All strings used in the program. They are all pointers
 *    into the shared string area. Thus, they are easily found and deallocated
 *    when the object is destructed.
 * 4. Table of variable names. They all point into the shared string table.
 * 5. Line number information. A table which tells at what address every
 *    line belongs to. The table has the same number of entries as the
 *    programs has source lines. This is used at errors, to find out the
 *    line number of the error.  This is usually swapped out to save space.
 *    First entry is the length of the table.
 * 6. List of inherited objects.
 */

/*
 * When a new object inherits from another, all function definitions
 * are copied, and all variable definitions.
 * Flags below can't explicitly declared. Flags that can be declared,
 * are found with TYPE_ below.
 *
 * When an object is compiled with type testing NAME_STRICT_TYPES, all
 * types are saved of the arguments for that function during compilation.
 * If the #pragma save_types is specified, then the types are saved even
 * after compilation, to be used when the object is inherited.
 */

/* NAME_INHERITED - The function entry that exists in this object actually
                    is a function in an object we inherited
 * NAME_UNDEFINED - the function hasn't been defined yet at this level
 * NAME_STRICT_TYPES - compiled with strict type testing
 * NAME_PROTOTYPE - only a prototype has been found so far
 * NAME_DEF_BY_INHERIT - this function actually exists in an object we've
                         inherited; if we don't find a function at this level
			 we'll use that one
 * NAME_ALIAS     - This entry refers us to another entry, usually because
                    this function was overloaded by that function
 * NAME_COLON_COLON - has a :: in it.  Don't allow call_others to it, etc
                      This function is propagated up the tree for technical
		      reasons.
 */
#define NAME_INHERITED		0x1
#define NAME_UNDEFINED		0x2
#define NAME_STRICT_TYPES	0x4
#define NAME_PROTOTYPE		0x10
#define NAME_DEF_BY_INHERIT     0x20
#define NAME_ALIAS              0x40
#define NAME_COLON_COLON        0x80

/* only the flags that should be copied up through inheritance levels */
#define NAME_MASK (NAME_UNDEFINED | NAME_STRICT_TYPES | NAME_PROTOTYPE | NAME_COLON_COLON)
/* a function that isn't 'real' */
#define NAME_NO_CODE  (NAME_UNDEFINED | NAME_ALIAS)

struct function {
    char *name;
#ifndef LPC_TO_C
    unsigned short offset;	/* Address of function, or inherit table
				 * index when inherited. */
#else
    unsigned long offset;
#endif
    /*
     * Used so that it is possible to quickly find this function in the
     * inherited program.
     */
    unsigned short function_index_offset;
    unsigned short type;	/* Return type of function. See below. */
    unsigned char num_local;	/* Number of local variables */
    unsigned char num_arg;	/* Number of arguments needed. -1 arguments
				 * means function not defined in this object.
				 * Probably inherited */
#ifdef PROFILE_FUNCTIONS
    unsigned long calls, self, children;
#endif
#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
    unsigned short tree_l, tree_r;	/* Left and right branches. */
    SIGNED short tree_b;	/* Balance of subtrees. */
#endif
    unsigned char flags;	/* NAME_ . See above. */
};

struct variable {
    char *name;
    unsigned short type;	/* Type of variable. See below. TYPE_ */
};

struct inherit {
    struct program *prog;
    unsigned short function_index_offset;
    unsigned short variable_index_offset;
};

struct external_program {
    int (*interface) PROT((char *, int));
    void *data;
};

struct internal_program {
    int ref;			/* Reference count */
#ifdef DEBUG
    int extra_ref;		/* Used to verify ref count */
#endif
    char *program;		/* The binary instructions */
    int id_number;		/* used to associate information with this
				 * prog block without needing to increase the
				 * reference count     */
    unsigned short *line_numbers;	/* Line number information */
    int line_swap_index;	/* Where line number info is swapped */
    struct function *functions;
#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
    unsigned short tree_r;	/* function table tree's 'root' */
#endif
    char **strings;		/* All strings uses by the program */
    struct variable *variable_names;	/* All variables defined */
    struct inherit *inherit;	/* List of inherited prgms */
    int total_size;		/* Sum of all data in this struct */
    int heart_beat;		/* Index of the heart beat function. -1 means
				 * no heart beat */
    /*
     * The types of function arguments are saved where 'argument_types'
     * points. It can be a variable number of arguments, so allocation is
     * done dynamically. To know where first argument is found for function
     * 'n' (number of function), use 'type_start[n]'. These two arrays will
     * only be allocated if '#pragma save_types' has been specified. This
     * #pragma should be specified in files that are commonly used for
     * inheritance. There are several lines of code that depends on the type
     * length (16 bits) of 'type_start' (sorry !).
     */
    unsigned short *argument_types;
#define INDEX_START_NONE		65535
    unsigned short *type_start;
    /*
     * And now some general size information.
     */
    unsigned short program_size;/* size of this instruction code */
    unsigned short num_functions;
    unsigned short num_strings;
    unsigned short num_variables;
    unsigned short num_inherited;
};

union pu {
    struct internal_program i;
    struct external_program e;
};

struct program {
    char *name;			/* Name of file that defined prog */
    union pu p;
};

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

/*
 * These are or'ed in on top of the basic type.
 */
#define TYPE_MOD_POINTER	0x0040	/* Pointer to a basic type */
#define TYPE_MOD_HIDDEN         0x0080  /* used by private vars */
#define TYPE_MOD_STATIC		0x0100	/* Static function or variable */
#define TYPE_MOD_NO_MASK	0x0200	/* The nomask => not redefineable */
#define TYPE_MOD_PRIVATE	0x0800	/* Can't be inherited */
#define TYPE_MOD_PROTECTED	0x1000
#define TYPE_MOD_PUBLIC		0x2000	/* Force inherit through private */
#define TYPE_MOD_VARARGS	0x4000	/* Used for type checking */

#define TYPE_MOD_MASK		(~(TYPE_MOD_STATIC | TYPE_MOD_NO_MASK |\
				   TYPE_MOD_PRIVATE | TYPE_MOD_PROTECTED |\
				   TYPE_MOD_PUBLIC | TYPE_MOD_VARARGS))

typedef struct {
  SIGNED short local_num, global_num, efun_num;
  SIGNED short function_num, simul_num;
} defined_name;

/* to speed up cleaning the hash table, and identify the union */
#define IHE_RESWORD    0x8000
#define IHE_EFUN       0x4000
#define IHE_SIMUL      0x2000
#define IHE_PERMANENT  (IHE_RESWORD | IHE_EFUN | IHE_SIMUL)
#define TOKEN_MASK     0x0fff

#define INDENT_HASH_SIZE 1024 /* must be a power of 2 */

struct ident_hash_elem {
    char *name;
    short token; /* only flags */
    short sem_value; /* for these, a count of the ambiguity */
    struct ident_hash_elem *next;
/* the fields above must correspond to struct keyword */
    defined_name dn;
};
#endif


