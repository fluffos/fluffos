/*
 * A compiled program consists of several data blocks, all allocated
 * contiguos in memory to enhance the working set. At the compilation,
 * the blocks will be allocated separately, as the final size is
 * unknow. When compilation is done, the blocks will be copied into
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
#define NAME_INHERITED		0x1	/* Defined by inheritance */
#define NAME_UNDEFINED		0x2	/* Not defined yet */
#define NAME_STRICT_TYPES	0x4	/* Compiled with type testing */
#define NAME_HIDDEN		0x8	/* Not visible for inheritance */
#define NAME_PROTOTYPE		0x10	/* Defined by a prototype only */

#define T_INTERNAL_PROGRAM 0x0
#define T_EXTERNAL_PROGRAM 0x1

struct function {
	char *name;
	unsigned short offset;	/* Address of function,
				 * or inherit table index when inherited. */
    /* Used so that it is possible to quickly find this function
     * in the inherited program.
     */
	unsigned short function_index_offset;
	unsigned short type;	/* Return type of function. See below. */
	unsigned char num_local;	/* Number of local variables */
	unsigned char num_arg;	/* Number of arguments needed.
				   -1 arguments means function not defined
				   in this object. Probably inherited */
#ifdef PROFILE_FUNCTIONS
	unsigned long calls, self, children;
#endif
	unsigned char flags;	/* NAME_ . See above. */
};

struct variable {
    char *name;
    unsigned short type;	/* Type of variable. See below. TYPE_ */
    unsigned short flags;	/* Facts found by the compiler. NAME_ */
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
    int ref;				/* Reference count */
#ifdef DEBUG
    int extra_ref;			/* Used to verify ref count */
#endif
    char *program;			/* The binary instructions */
    int  id_number;			/* used to associate information with
					   this prog block without needing to
					   increase the reference count     */
    unsigned short *line_numbers;	/* Line number information */
    int line_swap_index;                /* Where line number info is swapped */
    struct function *functions;
    char **strings;			/* All strings uses by the program */
    struct variable *variable_names;	/* All variables defined */
    struct inherit *inherit;		/* List of inherited prgms */
    int total_size;			/* Sum of all data in this struct */
    int heart_beat;			/* Index of the heart beat function.
					 * -1 means no heart beat
					 */
    /*
     * The types of function arguments are saved where 'argument_types'
     * points. It can be a variable number of arguments, so allocation
     * is done dynamically. To know where first argument is found for
     * function 'n' (number of function), use 'type_start[n]'.
     * These two arrays will only be allocated if '#pragma save_types' has
     * been specified. This #pragma should be specified in files that are
     * commonly used for inheritance. There are several lines of code
     * that depends on the type length (16 bits) of 'type_start' (sorry !).
     */
    unsigned short *argument_types;
#define INDEX_START_NONE		65535
    unsigned short *type_start;
    /*
     * And now some general size information.
     */
    unsigned short program_size;	/* size of this instruction code */
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
	char *name;				/* Name of file that defined prog */
	union pu p;
};

extern struct program *current_prog;

/*
 * Types available. The number '0' is valid as any type. These types
 * are only used by the compiler, when type checks are enabled. Compare with
 * the run-time types, named T_ interpret.h.
 */

#define TYPE_UNKNOWN	0   /* This type must be casted */
#define TYPE_VOID       1
#define TYPE_NUMBER     2
#define TYPE_STRING     3
#define TYPE_OBJECT     4
#define TYPE_MAPPING    5
#define TYPE_FUNCTION   6
#define TYPE_REAL       7
#define TYPE_BUFFER     8
#define TYPE_ANY        9  /* Will match any type */

/*
 * These are or'ed in on top of the basic type.
 */
#define TYPE_MOD_STATIC		0x0100	/* Static function or variable */
#define TYPE_MOD_NO_MASK	0x0200	/* The nomask => not redefineable */
#define TYPE_MOD_POINTER	0x0400	/* Pointer to a basic type */
#define TYPE_MOD_PRIVATE	0x0800	/* Can't be inherited */
#define TYPE_MOD_PROTECTED	0x1000
#define TYPE_MOD_PUBLIC		0x2000  /* Force inherit through private */
#define TYPE_MOD_VARARGS	0x4000	/* Used for type checking */

#define TYPE_MOD_MASK		(~(TYPE_MOD_STATIC | TYPE_MOD_NO_MASK |\
				   TYPE_MOD_PRIVATE | TYPE_MOD_PROTECTED |\
				   TYPE_MOD_PUBLIC | TYPE_MOD_VARARGS))
