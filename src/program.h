#ifndef PROGRAM_H
#define PROGRAM_H
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

/* The semantics of the flags are currently as follows:

   (1) A function is either a definition, a prototype or an undefined
       function

   (2) A definition is an address that executes code when
       it is jumped to and finishes with a return.  
       The address is labeled by the following info:
       function name, type, flags,number of arguments, number of locals, 
       argument types.

   (3) A prototype does not execute code, and is basically a label with
       the following info:
       argument types (optionally also argument names),function name,
       type, flags, number of arguments

   (4) An undefined function is an unknown function that has been called or
       a label that is as yet not known to be a prototype or definition

   Note that the above rules apply to comments in compiler.c as well.
   In particular, a known prototype is not undefined. When 'function'
   by itself is used, (1) is always meant.

   FUNC_INHERITED - The function entry that exists in this object actually
                    is a function in an object we inherited
   FUNC_UNDEFINED - see (4)
   FUNC_STRICT_TYPES - compiled with strict type testing
   FUNC_PROTOTYPE - see (3)
   FUNC_ALIAS     - This entry refers us to another entry, usually because
                    this function was overloaded by that function
                    
   Sym
 */
#define FUNC_INHERITED          0x0001
#define FUNC_UNDEFINED          0x0002
#define FUNC_STRICT_TYPES       0x0004
#define FUNC_PROTOTYPE          0x0008
#define FUNC_TRUE_VARARGS       0x0010
#define FUNC_VARARGS            0x0020
#define FUNC_ALIAS              0x8000        /* This shouldn't be changed */

#define DECL_HIDDEN             0x0100        /* used by private vars */
#define DECL_PRIVATE            0x0200        /* Can't be inherited */
#define DECL_PROTECTED          0x0400       /* Static function or variable */
#define DECL_PUBLIC             0x0800  
#define DECL_NOMASK             0x1000    /* The nomask => not redefineable */
#define DECL_NOSAVE             0x2000
#ifndef SENSIBLE_MODIFIERS
#define DECL_VISIBLE            0x4000     /* Force inherit through private */

#define DECL_ACCESS             (DECL_HIDDEN | DECL_PRIVATE | DECL_PROTECTED | DECL_PUBLIC | DECL_VISIBLE)

#define DECL_MODIFY(x,y) ((((x)|(y))&DECL_VISIBLE) ? ((((x)|(y))&~DECL_ACCESS)|DECL_VISIBLE) : DECL_MODIFY2(x,y))
#else
#define DECL_ACCESS             (DECL_HIDDEN | DECL_PRIVATE | DECL_PROTECTED | DECL_PUBLIC)

#define DECL_MODIFY(x,y) DECL_MODIFY2(x,y)
#endif
#define DECL_MODS               (DECL_ACCESS | DECL_NOMASK | DECL_NOSAVE)

#define DECL_MODIFY2(t, mod) ((((t) & DECL_ACCESS) > ((mod) & DECL_ACCESS)) ? ((t) & ~DECL_ACCESS) | (mod) : (t) | ((mod) & ~DECL_ACCESS))

/* only the flags that should be copied up through inheritance levels */
#define FUNC_MASK (FUNC_VARARGS | FUNC_UNDEFINED | FUNC_STRICT_TYPES | FUNC_PROTOTYPE | FUNC_TRUE_VARARGS | FUNC_ALIAS | DECL_MODS)

/* a function that isn't 'real' */
#define FUNC_NO_CODE  (FUNC_ALIAS | FUNC_PROTOTYPE | FUNC_UNDEFINED)
#define REAL_FUNCTION(x) (!((x) & (FUNC_ALIAS | FUNC_PROTOTYPE)))

/*
 * These are or'ed in on top of the basic type.
 */

/* Note, the following restricts class_num to < 0x40 or 64   */
/* The reason for this is that vars still have a ushort type */
/* This restriction is not unreasonable, since LPC is still  */
/* catered for mini-applications (compared to say, C++ or    */
/* java)..for now - Sym                                      */ 
  
#define TYPE_MOD_ARRAY          0x0040        /* Pointer to a basic type */
#define TYPE_MOD_CLASS          0x0080        /* a class */
#define CLASS_NUM_MASK          0x003f
  
#define LOCAL_MOD_REF           0x0100
#define LOCAL_MOD_UNUSED        0x0200
  
#define LOCAL_MODS (LOCAL_MOD_UNUSED|LOCAL_MOD_REF)
  
typedef struct {
    unsigned char num_arg;
    unsigned char num_local;
    unsigned short f_index; /* Index in sorted function table */
} runtime_defined_t;

typedef struct {
    unsigned short offset;
    unsigned short function_index_offset;
} runtime_inherited_t;

typedef union {
    runtime_defined_t def;
    runtime_inherited_t inh;
} runtime_function_u;

typedef struct {
    unsigned short first_defined;
    unsigned short first_overload; 
    unsigned short num_compressed;
    unsigned short num_deleted;
    unsigned char index[1];
} compressed_offset_table_t;

#ifdef USE_32BIT_ADDRESSES
#define ADDRESS_TYPE    unsigned int
#define ADDRESS_MAX     UINT_MAX
#else
#define ADDRESS_TYPE    unsigned short
#define ADDRESS_MAX     USHRT_MAX
#endif

typedef struct {
    char *funcname;
    unsigned short type;
    unsigned char num_arg;
    unsigned char num_local;
    ADDRESS_TYPE address;
#ifdef PROFILE_FUNCTIONS
    unsigned long calls, self, children;
#endif
} function_t;

typedef struct {
    unsigned short classname;
    unsigned short type;
    unsigned short size;
    unsigned short index;
} class_def_t;

typedef struct {
    unsigned short membername;
    unsigned short type;
} class_member_entry_t;

typedef struct {
    char *name;
    unsigned short type;        /* Type of variable. See above. TYPE_ */
} variable_t;

typedef struct {
    struct program_s *prog;
    unsigned short function_index_offset;
    unsigned short variable_index_offset;
    unsigned short type_mod;
} inherit_t;

typedef struct program_s {
    const char *filename;                 /* Name of file that defined prog */
    unsigned short flags;
    unsigned short last_inherited;
    unsigned short ref;                 /* Reference count */
    unsigned short func_ref;
#ifdef DEBUG
    int extra_ref;              /* Used to verify ref count */
    int extra_func_ref;
#endif
    char *program;              /* The binary instructions */
    unsigned char *line_info;   /* Line number information */
    unsigned short *file_info;
    int line_swap_index;        /* Where line number info is swapped */
    function_t *function_table;
    unsigned short *function_flags; /* separate for alignment reasons */
    class_def_t *classes;
    class_member_entry_t *class_members;
    char **strings;             /* All strings uses by the program */
    char **variable_table;      /* variables defined by this program */
    unsigned short *variable_types;     /* variables defined by this program */
    inherit_t *inherit; /* List of inherited prgms */
    int total_size;             /* Sum of all data in this struct */
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
#define INDEX_START_NONE                65535
    unsigned short *type_start;
    /*
     * And now some general size information.
     */
    unsigned short heart_beat;  /* Index of the heart beat function. 0 means
                                 * no heart beat */
    unsigned short program_size;/* size of this instruction code */
    unsigned short num_classes;
    unsigned short num_functions_defined;
    unsigned short num_strings;
    unsigned short num_variables_total;
    unsigned short num_variables_defined;
    unsigned short num_inherited;
} program_t;

extern int total_num_prog_blocks;
extern int total_prog_block_size;
void reference_prog (program_t *, const char *);
void free_prog (program_t **);
void deallocate_program (program_t *);
char *variable_name (program_t *, int);
function_t *find_func_entry (program_t *, int);

#endif


