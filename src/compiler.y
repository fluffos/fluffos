%{
# line 3 "compiler.y"
/* NOTE: this is the 3.1.1 version with 3.0.53-A3.1 fixes patched in */
/* The above line is to give proper line number references. Please mail me
 * if your compiler complains about it.
 */
/*
 * This is the grammar definition of LPC, and its code generator.
 */
#include "config.h"
#include <string.h>
#include <stdio.h>
#ifndef LATTICE
#include <memory.h>
#endif
#ifdef __386BSD__
#include <stdlib.h>
#endif
#if defined(sun)
#include <alloca.h>
#endif
#if defined(NeXT) || defined(SunOS_5)
#include <stdlib.h>
#endif
#ifdef sun
#include <malloc.h>
#endif

#include "mudlib_stats.h"
#include "interpret.h"
#define _YACC_
#include "lint.h"
#include "object.h"
#include "exec.h"
#include "instrs.h"
#include "incralloc.h"
#include "switch.h"
#include "opcodes.h"

static void insert_pop_value();
#ifdef LPC_OPTIMIZE_LOOPS
static int optimize_loop_cond PROT((char *e, int len));
static int optimize_while_dec PROT((char *e, int len));
#endif
static int last_expression = -1;

#define YYMAXDEPTH	600

/* NUMPAREAS areas are saved with the program code after compilation,
 * the rest are only temporary and used for convenience.
 */
#define A_PROGRAM		0	/* executable code */
#define A_FUNCTIONS		1	/* table of functions */
#define A_STRINGS		2	/* table of strings */
#define A_VARIABLES		3	/* table of variables */
#define A_LINENUMBERS		4	/* linenumber information */
#define A_INHERITS		5	/* table of inherited progs */
#define A_ARGUMENT_TYPES	6	/* */
#define A_ARGUMENT_INDEX	7	/* */
#define NUMPAREAS		8
#define A_CASE_NUMBERS		8	/* case labels for numbers */
#define A_CASE_STRINGS		9	/* case labels for strings */
#define A_CASE_LABELS		10	/* used to build switch tables */
#define A_STRING_NEXT		11	/* next prog string in hash chain */
#define A_STRING_REFS		12	/* reference count of prog string */
#define A_INCLUDES		13	/* list of included files */
#define A_PATCH			14	/* for save_binary() */
#define NUMAREAS		15

#define BREAK_ON_STACK		0x40000
#define BREAK_FROM_CASE		0x80000

#define SWITCH_STACK_SIZE  200
#define EXPR_STACK_SIZE  200

#define CURRENT_PROGRAM_SIZE (mem_block[A_PROGRAM].current_size)
#define BREAK_DELIMITER       -0x200000
#define CONTINUE_DELIMITER    -0x40000000

#define SET_CURRENT_PROGRAM_SIZE(x) \
	( CURRENT_PROGRAM_SIZE = (x), last_expression = -1 )

#define LAST_EXPR_CODE (mem_block[A_PROGRAM].block[last_expression])

typedef struct expr_s {
	char *expr;
	int len;
} expr_t;

/* make sure that this struct has a size that is a power of two */
struct case_heap_entry { int key; short addr; short line; };
#define CASE_HEAP_ENTRY_ALIGN(offset) offset &= -sizeof(struct case_heap_entry)

static struct mem_block mem_block[NUMAREAS];

/*
   these three variables used to properly adjust the 'break_sp' stack in
   the event a 'continue' statement is issued from inside a 'switch'.
*/
static short switches = 0;
static int switch_sptr = 0;
static int expr_sptr = 0;
static short switch_stack[SWITCH_STACK_SIZE];
expr_t expr_stack[EXPR_STACK_SIZE];
expr_t *pop_expression();
void push_expression();

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

#if !defined(__alpha) && !defined(cray)
#define align(x) (((x) + 3) & ~3)
#else
#define align(x) (((x) + 7) & ~7)
#endif

/*
 * If the type of the function is given, then strict types are
 * checked and required.
 */
static int exact_types;
extern int pragma_strict_types;	/* Maintained by lex.c */
extern int pragma_save_types;	/* Also maintained by lex.c */
#ifdef SAVE_BINARIES
extern int pragma_save_binaries;/* here too :-) */
#endif
int approved_object;		/* How I hate all these global variables */

extern int total_num_prog_blocks, total_prog_block_size;

extern int num_parse_error;
extern int d_flag;
static int heart_beat;		/* Number of the heart beat function */

static int current_break_address;
static int current_continue_address;
static int current_case_number_heap;
static int current_case_string_heap;
#define SOME_NUMERIC_CASE_LABELS 0x40000
#define NO_STRING_CASE_LABELS    0x80000
static int zero_case_label;
static int current_type;

static int last_push_indexed;
static int last_push_local;
static int last_push_identifier;

/*
 * There is always function starting at address 0, which will execute
 * the initialization code. This code is spread all over the program,
 * with jumps to next initializer. The next variable keeps track of
 * the previous jump. After the last initializer, the jump will be changed
 * into a return(0) statement instead.
 *
 * A function named '__INIT' will be defined, which will contain the
 * initialization code. If there was no initialization code, then the
 * function will not be defined. That is the usage of the
 * first_last_initializer_end variable.
 *
 * When inheriting from another object, a call will automatically be made
 * to call __INIT in that code from the current __INIT.
 */
static int last_initializer_end;
static int first_last_initializer_end;

static struct program NULL_program; /* marion - clean neat empty struct */

void epilog();
static int check_declared PROT((char *str));
static void prolog();
static void clean_parser();
static char *get_two_types PROT((int type1, int type2));
void free_all_local_names(),
    add_local_name PROT((char *, int)), smart_log PROT((char *, int, char *,int));
extern int yylex();
static int verify_declared PROT((char *));
static void copy_variables();
static int copy_functions PROT((struct program *, int type));
void type_error PROT((char *, int));

char *string_copy();

extern int current_line;
/*
 * 'inherit_file' is used as a flag. If it is set to a string
 * after yyparse(), this string should be loaded as an object,
 * and the original object must be loaded again.
 */
extern char *current_file, *inherit_file;

/*
 * The names and types of arguments and auto variables.
 */
char *local_names[MAX_LOCAL];
unsigned short type_of_locals[MAX_LOCAL];
int current_number_of_locals = 0;
int current_break_stack_need = 0  ,max_break_stack_need = 0;

/*
 * The types of arguments when calling functions must be saved,
 * to be used afterwards for checking. And because function calls
 * can be done as an argument to a function calls,
 * a stack of argument types is needed. This stack does not need to
 * be freed between compilations, but will be reused.
 */
static struct mem_block type_of_arguments;

struct program *prog;	/* Is returned to the caller of yyparse */

/*
 * Compare two types, and return true if they are compatible.
 */

static int compatible_types(t1, t2)
    int t1, t2;
{
    if (t1 == TYPE_UNKNOWN || t2 == TYPE_UNKNOWN)
#ifdef CAST_CALL_OTHERS      
	return 0;
#else  
	return 1;
#endif
    if (t1 == t2)
	return 1;
    if ((t1 & (TYPE_NUMBER | TYPE_REAL)) && (t2 & (TYPE_NUMBER | TYPE_REAL)))
	return 1;
    if (t1 == TYPE_ANY || t2 == TYPE_ANY)
	return 1;
    if ((t1 & TYPE_MOD_POINTER) && (t2 & TYPE_MOD_POINTER)) {
	if ((t1 & TYPE_MOD_MASK) == (TYPE_ANY|TYPE_MOD_POINTER) ||
	    (t2 & TYPE_MOD_MASK) == (TYPE_ANY|TYPE_MOD_POINTER))
	    return 1;
    }
    return 0;
}

/*
 * Add another argument type to the argument type stack
 */
static void add_arg_type(type)
    unsigned short type;
{
    struct mem_block *mbp = &type_of_arguments;
    while (mbp->current_size + sizeof type > mbp->max_size) {
	mbp->max_size <<= 1;
	mbp->block = (char *)DREALLOC((char *)mbp->block, mbp->max_size, 48,
		"add_arg_type");
    }
    memcpy(mbp->block + mbp->current_size, (char *)&type, sizeof type);
    mbp->current_size += sizeof type;
}

/*
 * Pop the argument type stack 'n' elements.
 */
INLINE
static void pop_arg_stack(n)
    int n;
{
    type_of_arguments.current_size -= sizeof (unsigned short) * n;
}

/*
 * Get type of argument number 'arg', where there are
 * 'n' arguments in total in this function call. Argument
 * 0 is the first argument.
 */
INLINE
int get_argument_type(arg, n)
    int arg, n;
{
    return
	((unsigned short *)
	 (type_of_arguments.block + type_of_arguments.current_size))[arg - n];
}

INLINE
static void realloc_mem_block(m, size)
    struct mem_block *m;
    int size;
{
    while (size > m->max_size) {
	m->max_size <<= 1;
	m->block = (char *)
	DREALLOC((char *)m->block, m->max_size, 49, "realloc_mem_block");
    }
}

INLINE
static void add_to_mem_block(n, data, size)
    int n, size;
    char *data;
{
    struct mem_block *mbp = &mem_block[n];
    if (mbp->current_size + size > mbp->max_size)
	realloc_mem_block(mbp, mbp->current_size + size);
    if (data)
	memcpy(mbp->block + mbp->current_size, data, size);
    mbp->current_size += size;
}

static void ins_byte(b)
    char b;
{
    struct mem_block *mbp = &mem_block[A_PROGRAM];
    if (mbp->current_size == mbp->max_size)
	realloc_mem_block(mbp, mbp->current_size + 1);
    mbp->block[mbp->current_size++] = b;
}

/*
 * Store a 2 byte number. It is stored in such a way as to be sure
 * that correct byte order is used, regardless of machine architecture.
 * Also beware that some machines can't write a word to odd addresses.
 */
static void ins_short(l)
    short l;
{
    struct mem_block *mbp = &mem_block[A_PROGRAM];
    if (mbp->current_size + 2 > mbp->max_size)
	realloc_mem_block(mbp, mbp->current_size + 2);
    mbp->block[mbp->current_size++] = ((char *)&l)[0];
    mbp->block[mbp->current_size++] = ((char *)&l)[1];
}

static void upd_short(offset, l)
    int offset;
    short l;
{
#ifdef DEBUG
	if (offset > CURRENT_PROGRAM_SIZE) {
		char buff[1024];

		sprintf(buff, "patch offset %x larger than current program size %x.\n",
			offset, CURRENT_PROGRAM_SIZE);
		yyerror(buff);
	}
#endif
    mem_block[A_PROGRAM].block[offset + 0] = ((char *)&l)[0];
    mem_block[A_PROGRAM].block[offset + 1] = ((char *)&l)[1];
}

static short read_short(offset)
    int offset;
{
    short l;

    ((char *)&l)[0] = mem_block[A_PROGRAM].block[offset + 0];
    ((char *)&l)[1] = mem_block[A_PROGRAM].block[offset + 1];
    return l;
}

/*
 * Store a 4 byte number. It is stored in such a way as to be sure
 * that correct byte order is used, regardless of machine architecture.
 */
static void ins_long(l)
    int l;
{
    struct mem_block *mbp = &mem_block[A_PROGRAM];
    if (mbp->current_size + 4 > mbp->max_size)
	realloc_mem_block(mbp, mbp->current_size + 4);
    mbp->block[mbp->current_size++] = ((char *)&l)[0];
    mbp->block[mbp->current_size++] = ((char *)&l)[1];
    mbp->block[mbp->current_size++] = ((char *)&l)[2];
    mbp->block[mbp->current_size++] = ((char *)&l)[3];
}

static void ins_real(l)
    double l;
{
    float f = (float)l;

    struct mem_block *mbp = &mem_block[A_PROGRAM];
    if (mbp->current_size + 4 > mbp->max_size)
	realloc_mem_block(mbp, mbp->current_size + 4);
    mbp->block[mbp->current_size++] = ((char *)&f)[0];
    mbp->block[mbp->current_size++] = ((char *)&f)[1];
    mbp->block[mbp->current_size++] = ((char *)&f)[2];
    mbp->block[mbp->current_size++] = ((char *)&f)[3];
}

INLINE static void
ins_expr_f_byte(b)
unsigned int b;
{
	last_expression = CURRENT_PROGRAM_SIZE;
	if (b >= 0xff) {
		ins_byte((char)F_CALL_EXTRA);
		ins_byte((char)(b - 0xff));
	} else {
		ins_byte((char)b);
	}
}

INLINE static void
ins_f_byte(b)
unsigned int b;
{
	ins_expr_f_byte(b);
	last_expression = -1;
}

/*
 * Return the index of the function found, otherwise -1.
 */
static int defined_function(s)
    char *s;
{
	int offset;
	char *shared_string;
	struct function *funp;

	/* if not in shared string table then it hasn't been defined */
	if ((shared_string = findstring(s))) {
		for (offset = 0; offset < mem_block[A_FUNCTIONS].current_size;
			offset += sizeof (struct function))
		{
			funp = (struct function *)&mem_block[A_FUNCTIONS].block[offset];
			if (funp->flags & NAME_HIDDEN)
				continue;
			/* can do pointer compare instead of strcmp since is shared */
			if (funp->name == shared_string)
				return offset / sizeof (struct function);
		}
	}
	return -1;
}

/*
 * A mechanism to remember addresses on a stack. The size of the stack is
 * defined in config.h.
 */
static int comp_stackp;
static int comp_stack[COMPILER_STACK_SIZE];

static void push_address() {
    if (comp_stackp >= COMPILER_STACK_SIZE) {
	yyerror("Compiler stack overflow");
	comp_stackp++;
	return;
    }
    comp_stack[comp_stackp++] = mem_block[A_PROGRAM].current_size;
}

static void push_explicit(address)
    int address;
{
    if (comp_stackp >= COMPILER_STACK_SIZE) {
	yyerror("Compiler stack overflow");
	comp_stackp++;
	return;
    }
    comp_stack[comp_stackp++] = address;
}

static int pop_address() {
    if (comp_stackp == 0)
	fatal("Compiler stack underflow.\n");
    if (comp_stackp > COMPILER_STACK_SIZE) {
	--comp_stackp;
	return 0;
    }
    return comp_stack[--comp_stackp];
}

static int
find_in_table(funp, cutoff)
    struct function *funp;
    int cutoff;
{
	int i;
	struct function *tfunp;

	for (i = 0; i < cutoff; i += sizeof(struct function)) {
		tfunp = (struct function *)(mem_block[A_FUNCTIONS].block + i);
		/* function names are shared strings */
		if ((tfunp->name == funp->name) && !(tfunp->flags & NAME_UNDEFINED))
		{
			*funp = *tfunp;
			return 1;
		}
	}
	return 0;
}

/*
 * Patch a function definition of an inherited function, to what it really
 * should be.
 * The name of the function can be one of:
 *    object::name
 *    ::name
 *    name
 * Where 'object' is the name of the superclass.
 */
static void find_inherited(funp)
    struct function *funp;
{
    int i;
    struct inherit *ip;
    int num_inherits, super_length = 0;
    char *real_name, *super_name = 0, *p;
	char *shared_string;

    real_name = funp->name;
    if (real_name[0] == ':')
	real_name = real_name + 2;	/* There will be exactly two ':' */
    else if ((p = strchr(real_name, ':'))) {
	real_name = p+2;
	super_name = funp->name;
	super_length = real_name - super_name - 2;
    }
    num_inherits = mem_block[A_INHERITS].current_size /
	sizeof (struct inherit);
	/* no need to look for it unless its in the shared string table */
	if ((shared_string = findstring(real_name))) {
    ip = (struct inherit *)mem_block[A_INHERITS].block;
    for (; num_inherits > 0; ip++, num_inherits--) {
	if (super_name) {
	    int l = strlen(ip->prog->name);	/* Including .c */
	    if (l - 2 < super_length)
		continue;
	    if (strncmp(super_name, ip->prog->name + l - 2 - super_length,
			super_length) != 0)
		continue;
	}
	for (i=0; (unsigned)i < ip->prog->p.i.num_functions; i++) {
	    if (ip->prog->p.i.functions[i].flags & (NAME_UNDEFINED|NAME_HIDDEN))
		continue;
		/* can use pointer compare because both are shared */
	    if (ip->prog->p.i.functions[i].name != shared_string)
		continue;
	    funp->offset = ip - (struct inherit *)mem_block[A_INHERITS].block;
	    funp->flags = ip->prog->p.i.functions[i].flags | NAME_INHERITED;
	    funp->num_local = ip->prog->p.i.functions[i].num_local;
	    funp->num_arg = ip->prog->p.i.functions[i].num_arg;
	    funp->type = ip->prog->p.i.functions[i].type;
	    funp->function_index_offset = i;
	    return;
	}
    }
	} /* if in shared string table */
}

/*
 * Define a new function. Note that this function is called at least twice
 * for all function definitions. First as a prototype, then as the real
 * function. Thus, there are tests to avoid generating error messages more
 * than once by looking at (flags & NAME_PROTOTYPE).
 */
static int define_new_function(name, num_arg, num_local, offset, flags, type)
    char *name;
    int num_arg, num_local;
    int offset, flags, type;
{
    int num;
    struct function fun;
    unsigned short argument_start_index;

    num = defined_function(name);
    if (num >= 0) {
	struct function *funp;

	/*
	 * The function was already defined. It may be one of several reasons:
	 *
	 * 1.	There has been a prototype.
	 * 2.	There was the same function defined by inheritance.
	 * 3.	This function has been called, but not yet defined.
	 * 4.	The function is doubly defined.
	 * 5.	A "late" prototype has been encountered.
	 */
	funp = (struct function *)(mem_block[A_FUNCTIONS].block) + num;
	if (!(funp->flags & NAME_UNDEFINED) &&
	    !(flags & NAME_PROTOTYPE) &&
	    !(funp->flags & NAME_INHERITED))
	{
	    char buff[500];
	    sprintf(buff, "Redeclaration of function %s.", name);
	    yyerror(buff);
	    return num;
	}
	/*
	 * It was either an undefined but used funtion, or an inherited
	 * function. In both cases, we now consider this to be THE new
	 * definition. It might also have been a prototype to an already
	 * defined function.
	 *
	 * Check arguments only when types are supposed to be tested,
	 * and if this function really has been defined already.
	 *
	 * 'nomask' functions may not be redefined.
	 */
	if ((funp->type & TYPE_MOD_NO_MASK) &&
	    !(funp->flags & NAME_PROTOTYPE) &&
	    !(flags & NAME_PROTOTYPE))
	{
	    char p[2048];
	    sprintf(p, "Illegal to redefine 'nomask' function \"%s\"",name);
	    yyerror(p);
	}
	if (exact_types && funp->type != TYPE_UNKNOWN) {
	    int i;
	    if (funp->num_arg != num_arg && !(funp->type & TYPE_MOD_VARARGS))
		yyerror("Incorrect number of arguments.");
	    else if (!(funp->flags & NAME_STRICT_TYPES))
		yyerror("Called function not compiled with type testing.");
	    else {
		/* Now check that argument types wasn't changed. */
		for (i=0; i < num_arg; i++) {
		}
	    }
	}
	/* If it was yet another prototype, then simply return. */
	if (flags & NAME_PROTOTYPE)
	    return num;
	funp->num_arg = num_arg;
	funp->num_local = num_local;
	funp->flags = flags;
	funp->offset = offset;
	funp->function_index_offset = 0;
	funp->type = type;
	if (exact_types)
	    funp->flags |= NAME_STRICT_TYPES;
	return num;
    }
    if (strcmp(name, "heart_beat") == 0)
	heart_beat = mem_block[A_FUNCTIONS].current_size /
	    sizeof (struct function);
    fun.name = make_shared_string(name);
    fun.offset = offset;
    fun.flags = flags;
    fun.num_arg = num_arg;
    fun.num_local = num_local;
    fun.function_index_offset = 0;
    fun.type = type;
#ifdef PROFILE_FUNCTIONS
    fun.calls = 0L;
    fun.self = 0L;
    fun.children = 0L;
#endif
    if (exact_types)
	fun.flags |= NAME_STRICT_TYPES;
    num = mem_block[A_FUNCTIONS].current_size / sizeof fun;
    /* Number of local variables will be updated later */
    add_to_mem_block(A_FUNCTIONS, (char *)&fun, sizeof fun);

    if (exact_types == 0 || num_arg == 0) {
	argument_start_index = INDEX_START_NONE;
    } else {
	int i;

	/*
	 * Save the start of argument types.
	 */
	argument_start_index =
	    mem_block[A_ARGUMENT_TYPES].current_size /
		sizeof (unsigned short);
	for (i=0; i < num_arg; i++) {
	    add_to_mem_block(A_ARGUMENT_TYPES, &type_of_locals[i],
			     sizeof type_of_locals[i]);
	}
    }
    add_to_mem_block(A_ARGUMENT_INDEX, &argument_start_index,
		     sizeof argument_start_index);
    return num;
}

static void define_variable(name, type, flags)
    char *name;
    int type;
    int flags;
{
    struct variable dummy;
    int n;

    n = check_declared(name);
    if (n != -1 && (VARIABLE(n)->type & TYPE_MOD_NO_MASK)) {
	char p[2048];

	sprintf(p, "Illegal to redefine 'nomask' variable \"%s\"", name);
	yyerror(p);
    }
    dummy.name = make_shared_string(name);
    dummy.type = type;
    dummy.flags = flags;
    add_to_mem_block(A_VARIABLES, (char *)&dummy, sizeof dummy);
}

static short string_idx[0x100];
static unsigned char string_tags[0x20];
static short freed_string;

#define STRING_HASH(var,str) \
    var = (long)str ^ (long)str >> 16; \
    var = (var ^ var >> 8) & 0xff;

short store_prog_string(str)
    char *str;
{
    short i, next, *next_tab, *idxp;
    char **p;
    unsigned char hash, mask, *tagp;

    str = make_shared_string(str);
    STRING_HASH(hash,str);
    idxp = &string_idx[hash];

    /* string_tags is a big bit-array, so find correct bit */
    mask = 1 << (hash & 7);
    tagp = &string_tags[hash >> 3];

    p = (char **) mem_block[A_STRINGS].block;
    next_tab = (short *) mem_block[A_STRING_NEXT].block;
    
    if (*tagp & mask) {
	/* search hash chain to see if it's there */
	for (i = *idxp; i >= 0; i = next_tab[i]) {
	    if (p[i] == str) {
		free_string(str); /* needed as string is only free'ed once. */
		((short *)mem_block[A_STRING_REFS].block)[i]++;
		return i;
	    }
	}
	next = *idxp;
    } else {
	*tagp |= mask;
	next = -1;
    }

    /*
     * New string, add to table
     */
    
    if (freed_string >= 0) {
	/* reuse freed string*/
	int top;
	i = freed_string;
	
	top = mem_block[A_STRINGS].current_size / sizeof str;
	for (freed_string++; freed_string < top; freed_string++) {
	    if (p[freed_string] == 0)
		break;
	}
	if (freed_string >= top)
	    freed_string = -1;
    } else {
	/* grow by one element. */
	add_to_mem_block(A_STRINGS, 0, sizeof str);
	add_to_mem_block(A_STRING_NEXT, 0, sizeof(short));
	add_to_mem_block(A_STRING_REFS, 0, sizeof(short));
	/* test if number of strings isn't too large ? */
	i = mem_block[A_STRINGS].current_size / sizeof str - 1;
    }
    ((char **)mem_block[A_STRINGS].block)[i] = str;
    ((short *)mem_block[A_STRING_NEXT].block)[i] = next;
    ((short *)mem_block[A_STRING_REFS].block)[i] = 1;
    *idxp = i;
    return i;
}

void free_prog_string(num)
    short num;
{
    short i, prv, *next_tab, top, *idxp;
    char **p, *str;
    unsigned char hash, mask;

    top = mem_block[A_STRINGS].current_size / sizeof(char*) - 1;
    if (num < 0 || num > top)
    {
	yyerror("free_prog_string: index out of range.\n");
	return;
    }

    if (--((short *)mem_block[A_STRING_REFS].block)[num] >= 1)
	return;

    p = (char **) mem_block[A_STRINGS].block;
    next_tab = (short *) mem_block[A_STRING_NEXT].block;

    str = p[num];
    STRING_HASH(hash,str);
    idxp = &string_idx[hash];

    for (prv = -1, i = *idxp; i != num; prv = i, i = next_tab[i]) {
	if (i == -1) {
	    yyerror("free_prog_string: string not in prog table.\n");
	    return;
	}
    }

    if (prv == -1) {  /* string is head of list */
	*idxp = next_tab[i];
	if (*idxp == -1) {
	    /* clear tag bit since hash chain now empty */
	    mask = 1 << (hash & 7);
	    string_tags[hash >> 3] &= ~mask;
	}
    } else {  /* easy unlink */
	next_tab[prv] = next_tab[i];
    }

    free_string(str);  /* important */
    p[i] = 0;
    if (i != top) {
	if (i < freed_string || freed_string == -1)
	    freed_string = i;
    } else {
	/* shrink table */
	mem_block[A_STRINGS].current_size -= sizeof str;
	mem_block[A_STRING_REFS].current_size -= sizeof(short);
	mem_block[A_STRING_NEXT].current_size -= sizeof(short);
    }
}

void add_to_case_heap(block_index,entry)
    int block_index;
    struct case_heap_entry *entry;
{
    char *heap_start;
    int offset,parent;
    int current_heap;

    if ( block_index == A_CASE_NUMBERS )
        current_heap = current_case_number_heap;
    else
        current_heap = current_case_string_heap;
    offset = mem_block[block_index].current_size - current_heap;
    add_to_mem_block(block_index, (char*)entry, sizeof(*entry) );
    heap_start = mem_block[block_index].block + current_heap;
    for ( ; offset; offset = parent ) {
        parent = ( offset - sizeof(struct case_heap_entry) ) >> 1 ;
        CASE_HEAP_ENTRY_ALIGN(parent);
        if ( ((struct case_heap_entry*)(heap_start+offset))->key <
             ((struct case_heap_entry*)(heap_start+parent))->key )
        {
            *(struct case_heap_entry*)(heap_start+offset) =
            *(struct case_heap_entry*)(heap_start+parent);
            *(struct case_heap_entry*)(heap_start+parent) = *entry;
        }
    }
}

/*
 * Arrange a jump to the current position for the initialization code
 * to continue.
 */
static void transfer_init_control() {
    if (mem_block[A_PROGRAM].current_size - 2 == last_initializer_end)
	mem_block[A_PROGRAM].current_size -= 3;
    else {
	/*
	 * Change the address of the last jump after the last
	 * initializer to this point.
	 */
	upd_short(last_initializer_end,
		  mem_block[A_PROGRAM].current_size);
    }
}

/*
 * Generate the code to push a number on the stack.
 * This varies since there are several opcodes (for
 * optimizing speed and/or size).
 */
static void write_number(val)
	  int val;
{
    if (val == 0) {
	ins_expr_f_byte(F_CONST0);
    } else if (val == 1) {
	ins_expr_f_byte(F_CONST1);
    } else if (val >= 0 && val < 256) {
	ins_f_byte(F_BYTE);
	ins_byte(val);
    } else if (val < 0 && val > -256) {
	ins_f_byte(F_NBYTE);
	ins_byte(-val);
    } else {
	ins_f_byte(F_NUMBER);
	ins_long(val);
    }
}

#ifdef LPC_OPTIMIZE

#define SIZEOF_ZERO_CODE 1   /* size of code to push 0 */

/* go through all the hassle of reading the number back out again */
static int
read_number(address)
	  int address;
{
    int val;
    register char *block;

    block = &mem_block[A_PROGRAM].block[address];
    switch(EXTRACT_UCHAR(block++)) {
    case F_CONST0:
	return 0;
    case F_CONST1:
	return 1;
    case F_BYTE:
	return EXTRACT_UCHAR(block);
    case F_NBYTE:
	return -EXTRACT_UCHAR(block);
    case F_NUMBER:
	((char *)&val)[0] = block[0];
	((char *)&val)[1] = block[1];
	((char *)&val)[2] = block[2];
	((char *)&val)[3] = block[3];
	return val;
    default:
	yyerror("Internal error in read_number.");
    }
	return 0;
}

static float read_real(address)
	  int address;
{
    register char *block;
    block = &mem_block[A_PROGRAM].block[address];
    if (EXTRACT_UCHAR(block++) == F_REAL) {
	float real;
	((char *)&real)[0] = block[0];
	((char *)&real)[1] = block[1];
	((char *)&real)[2] = block[2];
	((char *)&real)[3] = block[3];
	return real;
    } else {
	yyerror("Internal error in read_real.");
    }
	return 0.0;
}

static short read_string(address)
	  int address;
{
    register char *block;
    block = &mem_block[A_PROGRAM].block[address];
    if (EXTRACT_UCHAR(block++) == F_STRING) {
	short num;
	((char *)&num)[0] = block[0];
	((char *)&num)[1] = block[1];
	return num;
    } else {
	yyerror("Internal error in read_string.");
    }
	return -1;
}
#endif

void add_new_init_jump();
%}

/*
 * Token definitions.
 */

%token L_CASE L_DEFAULT L_RANGE
%token L_IF L_IDENTIFIER L_LAND L_LOR L_STATUS
%token L_RETURN L_STRING
%token L_INC L_DEC
%token L_NUMBER L_REAL L_INT L_FLOAT
%token L_EQ L_NE L_GE L_LE
%token L_ADD_EQ L_SUB_EQ L_DIV_EQ L_MULT_EQ
%token L_WHILE L_BREAK
%token L_DO L_FOR L_SWITCH
%token L_SSCANF L_PARSE_COMMAND L_STRING_DECL L_LOCAL_NAME
%token L_ELSE
%token L_CONTINUE
%token L_MOD_EQ L_INHERIT L_COLON_COLON
%token L_STATIC
%token L_ARROW
%token L_AND_EQ L_OR_EQ L_XOR_EQ
%token L_LSH L_LSH_EQ L_RSH L_RSH_EQ
%token L_CATCH L_TIME_EXPRESSION
%token L_MAPPING L_OBJECT L_VOID L_MIXED L_PRIVATE L_NO_MASK L_NOT
%token L_PROTECTED L_PUBLIC
%token L_FUNCTION
%token L_VARARGS

/*
 * Define precedences of operators to make grammar simpler.
 * (this makes %token for them redundant, but...)
 */

%right '='
%right '?'
%left L_LOR
%left L_LAND
%left '|'
%left '^'
%left '&'
%left L_EQ L_NE          /* nonassoc? */
%left '>' L_GE '<' L_LE  /* nonassoc? */
%left L_LSH L_RSH
%left '+' '-'
%left '*' '%' '/'
%right L_NOT '~'
%nonassoc L_INC L_DEC

%union
{
	int number;
	float real; 
	unsigned int address;	/* Address of an instruction */
	char *string;
	short type;
	struct { int key; char block; } case_label;
	struct { short type; char iscon; unsigned int addr; } expr;
	struct function *funp;
}

%type <number> assign L_NUMBER constant L_LOCAL_NAME expr_list
%type <real>   L_REAL
%type <number> lvalue_list argument type basic_type optional_star expr_list2
%type <number> expr_list3 expr_list4 assoc_pair
%type <number> type_modifier type_modifier_list opt_basic_type block_or_semi
%type <number> argument_list
%type <string> L_IDENTIFIER L_STRING string_con1 string_con2
%type <string> string_constant function_name

%type <case_label> case_label

/* The following symbols return type information */

%type <type> function_call cast lvalue for_expr comma_expr

%type <expr> number real string expr4 expr0 comma_expr1
%%

all: program;

program: program def possible_semi_colon
       |	 /* empty */ ;

possible_semi_colon: /* empty */
                   | ';' { yyerror("Extra ';'. Ignored."); };

inheritance: type_modifier_list L_INHERIT string_con1 ';'
		{
		    struct object *ob;
		    struct inherit inherit;
		    int initializer;

		    ob = find_object2($3);
		    if (ob == 0) {
			inherit_file = $3;
			/* Return back to load_object() */
			YYACCEPT;
		    }
		    FREE($3);
		    inherit.prog = ob->prog;
		    inherit.function_index_offset =
			mem_block[A_FUNCTIONS].current_size /
			    sizeof (struct function);
		    inherit.variable_index_offset =
			mem_block[A_VARIABLES].current_size /
			    sizeof (struct variable);
		    add_to_mem_block(A_INHERITS, &inherit, sizeof inherit);
		    copy_variables(ob->prog, $1);
		    initializer = copy_functions(ob->prog, $1);
		    if (initializer > 0) {
			struct function *funp;
			int f;

			f = define_new_function("::__INIT", 0, 0, 0, 0, 0);
			funp = FUNCTION(f);
			funp->offset = mem_block[A_INHERITS].current_size /
			    sizeof (struct inherit) - 1;
			funp->flags = NAME_STRICT_TYPES |
			    NAME_INHERITED | NAME_HIDDEN;
			funp->type = TYPE_VOID;
			funp->function_index_offset = initializer;
			transfer_init_control();
			ins_f_byte(F_CALL_FUNCTION_BY_ADDRESS);
			ins_short(f);
			ins_byte(0);	/* Actual number of arguments */
			ins_f_byte(F_POP_VALUE);
			add_new_init_jump();
		    }
		}

real: L_REAL
    {
	$$.type = TYPE_REAL;
	$$.iscon = 1;
	$$.addr = CURRENT_PROGRAM_SIZE;
	ins_f_byte(F_REAL);
	ins_real($1);
    }
    ;

number: L_NUMBER
	{
	    if ($1 == 0)
		$$.type = TYPE_ANY;
	    else
		$$.type = TYPE_NUMBER;
	    $$.iscon = 1;
	    $$.addr = CURRENT_PROGRAM_SIZE;
	    write_number($1);
	} ;

optional_star: /* empty */ { $$ = 0; } | '*' { $$ = TYPE_MOD_POINTER; } ;

block_or_semi: block { $$ = 0; } | ';' { $$ = ';'; } ;

def: type optional_star L_IDENTIFIER
	{
	    /* Save start of function. */
	    push_explicit(mem_block[A_PROGRAM].current_size);

	    if ($1 & TYPE_MOD_MASK) {
		exact_types = $1 | $2;
	    } else {
		if (pragma_strict_types)
		    yyerror("\"#pragma strict_types\" requires type of function");
		/* force strict types - no more compat mode */
#ifdef STRICT_TYPE_CHECKING
		exact_types = TYPE_ANY; /* default to return type of mixed */
#else
		exact_types = 0;
#endif
	    }
	}
	'(' argument ')'
	{
	    /*
	     * Define a prototype. If it is a real function, then the
	     * prototype will be replaced below.
	     */
	    define_new_function($3, $6, 0, 0,
				NAME_UNDEFINED|NAME_PROTOTYPE, $1 | $2);
	}
        block_or_semi
	{
	    /* Either a prototype or a block */
	    if ($9 == ';') {
		(void)pop_address(); /* Not used here */
	    } else {
		define_new_function($3, $6, current_number_of_locals - $6+
			( max_break_stack_need -1 ) / sizeof(struct svalue) +1,
			pop_address(), 0, $1 | $2);
		ins_f_byte(F_CONST0); ins_f_byte(F_RETURN);
	    }
	    free_all_local_names();
	    FREE($3);		/* Value was copied above */
	}
   | type name_list ';' { if ($1 == 0) yyerror("Missing type"); }
   | inheritance ;

new_arg_name: type optional_star L_IDENTIFIER
	{
	    if (exact_types && $1 == 0) {
		yyerror("Missing type for argument");
		add_local_name($3, TYPE_ANY);	/* Supress more errors */
	    } else {
		add_local_name($3, $1 | $2);
	    }
	}
	  | type L_LOCAL_NAME
		{yyerror("Illegal to redeclare local name"); } ;

argument: /* empty */ { $$ = 0; }
	  | argument_list ;

argument_list: new_arg_name { $$ = 1; }
	     | argument_list ',' new_arg_name { $$ = $1 + 1; } ;

type_modifier: L_NO_MASK { $$ = TYPE_MOD_NO_MASK; }
	     | L_STATIC { $$ = TYPE_MOD_STATIC; }
	     | L_PRIVATE { $$ = TYPE_MOD_PRIVATE; }
	     | L_PUBLIC { $$ = TYPE_MOD_PUBLIC; }
	     | L_VARARGS { $$ = TYPE_MOD_VARARGS; }
	     | L_PROTECTED { $$ = TYPE_MOD_PROTECTED; } ;

type_modifier_list: /* empty */ { $$ = 0; }
		  | type_modifier type_modifier_list { $$ = $1 | $2; } ;

type: type_modifier_list opt_basic_type { $$ = $1 | $2; current_type = $$; } ;

cast: '(' basic_type optional_star ')'
	{
	    $$ = $2 | $3;
	} ;

opt_basic_type: basic_type | /* empty */ { $$ = TYPE_UNKNOWN; } ;

basic_type: L_STATUS { $$ = TYPE_NUMBER; current_type = $$; }
	| L_INT { $$ = TYPE_NUMBER; current_type = $$; }
	| L_FLOAT { $$ = TYPE_REAL; current_type = $$; }
	| L_STRING_DECL { $$ = TYPE_STRING; current_type = $$; }
	| L_OBJECT { $$ = TYPE_OBJECT; current_type = $$; }
	| L_MAPPING { $$ = TYPE_MAPPING; current_type = $$; }
	| L_FUNCTION { $$ = TYPE_FUNCTION; current_type = $$; }
	| L_VOID {$$ = TYPE_VOID; current_type = $$; }
	| L_MIXED { $$ = TYPE_ANY; current_type = $$; } ;

name_list: new_name
	 | new_name ',' name_list;

new_name: optional_star L_IDENTIFIER
	{
	    define_variable($2, current_type | $1, 0);
	    FREE($2);
	}
| optional_star L_IDENTIFIER
	{
	    int var_num;
	    define_variable($2, current_type | $1, 0);
	    var_num = verify_declared($2);
	    transfer_init_control();
	    ins_f_byte(F_PUSH_IDENTIFIER_LVALUE);
	    ins_byte(var_num);
	}
	'=' expr0
	{
	    if (!compatible_types((current_type | $1) & TYPE_MOD_MASK, $5.type)){
		char buff[100];
		sprintf(buff, "Type mismatch %s when initializing %s",
			get_two_types(current_type | $1, $5.type), $2);
		yyerror(buff);
	    }
	    ins_f_byte(F_VOID_ASSIGN);
	    add_new_init_jump();
	    FREE($2);
	} ;
block: '{' local_declarations statements '}'
	{ ; };

local_declarations: /* empty */
		  | local_declarations basic_type local_name_list ';' ;

new_local_name: optional_star L_IDENTIFIER
	{
	    add_local_name($2, current_type | $1);
	} ;

local_name_list: new_local_name
	| new_local_name ',' local_name_list ;

statements: /* empty */
	  | statement statements
	  | error ';' ;

statement: comma_expr ';'
	{
	    insert_pop_value();
	    if (d_flag)
		ins_f_byte(F_BREAK_POINT);
	    /* if (exact_types && !TYPE($1,TYPE_VOID))
		yyerror("Value thrown away"); */
	}
	 | cond | while | do | for | switch | case | default | return ';'
	 | block
  	 | /* empty */ ';'
	 | L_BREAK ';'	/* This code is a jump to a jump */
		{
		    if (current_break_address == 0)
			yyerror("break statement outside loop");
		    if (current_break_address & BREAK_ON_STACK) {
			ins_f_byte(F_BREAK);
		    } else {
				/* form a linked list of the break addresses */
		        ins_f_byte(F_JUMP); ins_short(current_break_address);
				current_break_address =  mem_block[A_PROGRAM].current_size - 2;
		    }
		}
	 | L_CONTINUE ';'	/* This code is a jump */
		{
		    if (current_continue_address == 0)
			yyerror("continue statement outside loop");
			if (switches) {
				ins_f_byte(F_POP_BREAK); ins_byte(switches);
			}
			/* form a linked list of the continue addresses */
		    ins_f_byte(F_JUMP); ins_short(current_continue_address);
			current_continue_address = mem_block[A_PROGRAM].current_size - 2;
		}
         ;

while:  {
	/* be intelligent about the order in which the code is laid out.
	   By inserting the code for the block first followed by the
	   the expression (instead of vice versa), it is possible to save
	   an instruction on each while loop iteration.
	*/
		push_explicit(current_continue_address);
		push_explicit(current_break_address);
		/* remember size of program before the while expression */
		push_address(); /* 1 */
		/* keep track of # of nested switches prior to this while
		   so that we'll know how many to pop from the break stack in
		   the event of a "continue;" in the body of a switch().
		*/
		push_switches();
	} L_WHILE '(' comma_expr ')'
	{
		int addr = pop_address(); /* 1 */
		expr_t e;
		char branch;

		/* optimize if last expression did F_NOT */
		if (last_expression == CURRENT_PROGRAM_SIZE-1 &&
		    LAST_EXPR_CODE == F_NOT)
		{
		    SET_CURRENT_PROGRAM_SIZE(last_expression);
		    branch = F_BBRANCH_WHEN_ZERO;
		} else {
		    branch = F_BBRANCH_WHEN_NON_ZERO;
		}

		/* + 3 to leave room for (byte) jump code and (short) target */
		e.len = CURRENT_PROGRAM_SIZE - addr;
		e.expr = (char *)DMALLOC(e.len + 3, 50, "WHILE");
		/* copy the code for the loop control expression into a temp space */
		memcpy(e.expr, mem_block[A_PROGRAM].block + addr, e.len);
		/* relative offset (backwards) branch */
		e.expr[e.len] = branch;
#ifdef LPC_OPTIMIZE_LOOPS
		if (!optimize_loop_cond(e.expr, e.len)) {
			optimize_while_dec(e.expr, e.len);
		}
#endif
		/* adjust the size of the program so that we will overwrite the
		   expression in the compiled code (remember we've already copied the
		   expression into temp storage).
		*/
		SET_CURRENT_PROGRAM_SIZE(addr);
		last_push_identifier = -1;
		last_push_local = -1;
		last_push_indexed = -1;
		/* jump to code for expression (which hasn't yet been inserted) */
		ins_f_byte(F_JUMP);
		/* remember where the place holder is stored */
		push_address(); /* 2 */
		ins_short(0); /* insert the place holder */
		push_expression(&e); /* 3 */
		/* each 'continue' statement will add to a linked list of slots that
		   need filled in once the continue_address is known.  The delimeters
		   serve as markers for the ends of the lists.
		*/
		current_continue_address = CONTINUE_DELIMITER;
		current_break_address = BREAK_DELIMITER;
	}
       statement
	{
		expr_t *e;
		int addr = pop_address(); /* 2 */
		unsigned short start = addr + 2;
		unsigned short offset;
		int next_addr;

		upd_short(addr, CURRENT_PROGRAM_SIZE);
		e = pop_expression(); /* 3 */
		/* traverse the linked list filling in the current_continue_address
		   required by each "continue" statement.
		*/
		for(; current_continue_address > 0;
			current_continue_address = next_addr)
		{
			next_addr = read_short(current_continue_address);
			upd_short(current_continue_address, CURRENT_PROGRAM_SIZE);
		}
		/* branch already added to end of expression */
		offset = CURRENT_PROGRAM_SIZE + (e->len + 1 - start);
		e->expr[e->len+1] = ((char *)&offset)[0];
		e->expr[e->len+2] = ((char *)&offset)[1];
		add_to_mem_block(A_PROGRAM, e->expr, e->len + 3);
		/* fill in branch address */
		FREE(e->expr);

		/* traverse the linked list filling in the current_break_address
		   required by each "break" statement.
		*/
		for(;current_break_address > 0; current_break_address = next_addr) {
			next_addr = read_short(current_break_address);
			upd_short(current_break_address, CURRENT_PROGRAM_SIZE);
		}
		pop_switches();
		current_break_address = pop_address();
		current_continue_address = pop_address();
	}

do: {
		push_switches();
		push_explicit(current_continue_address);
		push_explicit(current_break_address);
		current_break_address = BREAK_DELIMITER;
		current_continue_address = CONTINUE_DELIMITER;
        push_address(); /* 1 */
    } L_DO statement
    {
        push_address(); /* 2 */
    }
    L_WHILE '(' comma_expr ')' ';'
	{
		int cont_addr = pop_address(); /* 2 */
		int addr = pop_address(); /* 1 */
		int next_addr;

		for (;current_continue_address > 0;
			current_continue_address = next_addr)
		{
			next_addr = read_short(current_continue_address);
			upd_short(current_continue_address, cont_addr);
		}
		/* optimize if last expression did F_NOT */
		if (last_expression == CURRENT_PROGRAM_SIZE-1 &&
		    LAST_EXPR_CODE == F_NOT)
		{
		    SET_CURRENT_PROGRAM_SIZE(last_expression);
		    ins_f_byte(F_BBRANCH_WHEN_ZERO);
		} else {
		    ins_f_byte(F_BBRANCH_WHEN_NON_ZERO);
		}
		ins_short(CURRENT_PROGRAM_SIZE - addr);
		for(;current_break_address > 0; current_break_address = next_addr) {
			next_addr = read_short(current_break_address);
			upd_short(current_break_address, CURRENT_PROGRAM_SIZE);
		}
		current_break_address = pop_address();
		current_continue_address = pop_address();
        pop_switches();
	}

for: L_FOR '('{
		push_explicit(current_continue_address);
		push_explicit(current_break_address);
		push_switches();
	}
    for_expr ';' {
		current_continue_address = CONTINUE_DELIMITER;
		insert_pop_value();
		push_address();  /* 1 */
	}
	for_expr ';' {
		int start = pop_address(); /* 1 */
		expr_t e;
		char branch;
		
		/* optimize if last expression did F_NOT */
		if (last_expression == CURRENT_PROGRAM_SIZE-1 &&
		    LAST_EXPR_CODE == F_NOT)
		{
		    SET_CURRENT_PROGRAM_SIZE(last_expression);
		    branch = F_BBRANCH_WHEN_ZERO;
		} else {
		    branch = F_BBRANCH_WHEN_NON_ZERO;
		}

		e.len = CURRENT_PROGRAM_SIZE - start;
		e.expr = (char *)DMALLOC(e.len + 3, 51, "for_expr");
		memcpy(e.expr, mem_block[A_PROGRAM].block + start, e.len);
		e.expr[e.len] = branch;
#ifdef LPC_OPTIMIZE_LOOPS
		optimize_loop_cond(e.expr, e.len);
#endif
		push_expression(&e);
		SET_CURRENT_PROGRAM_SIZE(start);
		push_address(); /* 2 */
		last_push_identifier = -1;
		last_push_local = -1;
		last_push_indexed = -1;
	}
	for_expr ')' {
		expr_t e;
		int start = pop_address(); /* 3 */

		insert_pop_value();
		e.len = CURRENT_PROGRAM_SIZE - start;
		e.expr = e.len ? (char *)DMALLOC(e.len, 52, "for_expr:") : (char *)0;
		if (e.expr) {
			memcpy(e.expr, mem_block[A_PROGRAM].block + start, e.len);
#ifdef LPC_OPTIMIZE_LOOPS
            if ((e.len == 3) && (e.expr[0] == F_PUSH_LOCAL_VARIABLE_LVALUE)
                && (e.expr[2] == F_INC))
            { /* optimizaton for the i++ of a for loop ;) - jwg */
                e.expr[0] = F_LOOP_INCR;
                e.len = 2;
            }
#endif
		}
		push_expression(&e);
		SET_CURRENT_PROGRAM_SIZE(start);
		push_address(); /* 4 */
		ins_f_byte(F_JUMP); /* to expression */
		ins_short(0); /* 5 */
		current_break_address = BREAK_DELIMITER;
		last_push_identifier = -1;
		last_push_local = -1;
		last_push_indexed = -1;
	}
	statement
	{
		int next_addr;
		unsigned short int jump = (unsigned short)pop_address(); /* 4 */
		unsigned short int start = jump + 3;
		unsigned short offset;
		expr_t *e;

		for(; current_continue_address > 0;
			current_continue_address = next_addr)
		{
			next_addr = read_short(current_continue_address);
			upd_short(current_continue_address, CURRENT_PROGRAM_SIZE);
		}

		e = pop_expression();
		if (e->len) {
			add_to_mem_block(A_PROGRAM, e->expr, e->len);
			FREE(e->expr);
		}

		/* fix F_JUMP target inserted above */
		upd_short(jump + 1, CURRENT_PROGRAM_SIZE); /* 5 */

		e = pop_expression();
		offset = CURRENT_PROGRAM_SIZE + (e->len + 1 - start);
		e->expr[e->len + 1] = ((char *)&offset)[0];
		e->expr[e->len + 2] = ((char *)&offset)[1];
		add_to_mem_block(A_PROGRAM, e->expr, e->len + 3);
		FREE(e->expr);

		for(;current_break_address > 0;current_break_address = next_addr) {
			next_addr = read_short(current_break_address);
			upd_short(current_break_address, CURRENT_PROGRAM_SIZE);
		}
		current_break_address = pop_address();
		current_continue_address = pop_address();
		pop_switches();
	}

for_expr: /* EMPTY */
	{
		ins_expr_f_byte(F_CONST1);
	}
	| comma_expr;

switch: L_SWITCH '(' comma_expr ')'
    {
	switches++;
        current_break_stack_need += sizeof(short);
        if ( current_break_stack_need > max_break_stack_need )
            max_break_stack_need = current_break_stack_need;
	push_explicit(current_case_number_heap);
	push_explicit(current_case_string_heap);
	push_explicit(zero_case_label);
	push_explicit(current_break_address);
	ins_f_byte(F_SWITCH);
	ins_byte(0xff); /* kind of table */
	current_case_number_heap = mem_block[A_CASE_NUMBERS].current_size;
	current_case_string_heap = mem_block[A_CASE_STRINGS].current_size;
	zero_case_label = NO_STRING_CASE_LABELS;
	ins_short(0); /* address of table */
	current_break_address = mem_block[A_PROGRAM].current_size |
				BREAK_ON_STACK | BREAK_FROM_CASE ;
	ins_short(0); /* break address to push, table is entered before */
	ins_short(0); /* default address */
    }
      statement
    {
	char *heap_start;
	int heap_end_offs;
	int i,o;
	int current_key, last_key = 0;
	/* int size_without_table; */
	int block_index;
	int current_case_heap;
	int first_key;
        int num_keys;
        
	current_break_address &= ~(BREAK_ON_STACK|BREAK_FROM_CASE);

	if ( !read_short(current_break_address+2 ) )
	    upd_short(current_break_address+2,     /* no default given ->  */
	      mem_block[A_PROGRAM].current_size);  /* create one           */

	/* it isn't unusual that the last case/default has no break */
	ins_f_byte(F_BREAK);
	if(zero_case_label & (NO_STRING_CASE_LABELS|SOME_NUMERIC_CASE_LABELS)){
	    block_index = A_CASE_NUMBERS;
	    current_case_heap = current_case_number_heap;
	} else {
	    block_index = A_CASE_STRINGS;
	    current_case_heap = current_case_string_heap;
	    if (zero_case_label&0xffff) {
		struct case_heap_entry temp;

		temp.key = (int)ZERO_AS_STR_CASE_LABEL;
		temp.addr = zero_case_label & 0xffff;
		temp.line = 0; /* if this is accessed later, something is
				* really wrong				  */
		add_to_case_heap(A_CASE_STRINGS,&temp);
	    }
#ifdef SAVE_BINARIES
	    {
		short sw;
		sw = current_break_address - 4;  /* the F_SWITCH */
		add_to_mem_block(A_PATCH, (char *)&sw, sizeof sw);
	    }
#endif
	}
	heap_start = mem_block[block_index].block + current_case_heap ;
	heap_end_offs = mem_block[block_index].current_size -current_case_heap;
	if (!heap_end_offs) yyerror("switch without case not supported");

        /* add a dummy entry so that we can always
        * assume we have no or two childs
        */
        add_to_mem_block(block_index, "\0\0\0\0\0\0\0\0",
            sizeof(struct case_heap_entry) );

        /* read out the heap and build a sorted table */
	/* the table could be optimized better, but let's first see
	* how much switch is used at all when it is full-featured...
	*/
	mem_block[A_CASE_LABELS].current_size = 0;
	first_key = ((struct case_heap_entry*)heap_start)->key;
        num_keys = 0;
        for( ; ((struct case_heap_entry*)heap_start)->addr; )
        {
            int offset;
	    int curr_line, last_line = 0;
	    unsigned short current_addr,last_addr = 0xffff;
	    int range_start = 0;

            current_key = ((struct case_heap_entry*)heap_start)->key ;
            curr_line = ((struct case_heap_entry*)heap_start)->line ;
            current_addr = ((struct case_heap_entry*)heap_start)->addr ;
            if ( current_key == last_key &&
              mem_block[A_CASE_LABELS].current_size )
            {
                char buf[90];

                sprintf(buf,"Duplicate case in line %d and %d",
		    last_line, curr_line);
                yyerror(buf);
            }
	    if (curr_line) {
		if (last_addr == 1) {
                    char buf[120];
    
		    sprintf(buf,
"Discontinued case label list range, line %d by line %d",
		      last_line, curr_line);
                    yyerror(buf);
		}
		else if (current_key == (last_key + 1) && current_addr == last_addr) {
		    if (mem_block[A_CASE_LABELS].current_size != (range_start + 6)) {
		     /* todo: this next line is probably non-portable (byte-ordering) */
		      *(short*)(mem_block[A_CASE_LABELS].block+range_start+4) = 1;
		      mem_block[A_CASE_LABELS].current_size = range_start + 6;
		    }
		} else {
		    range_start = mem_block[A_CASE_LABELS].current_size;
		}
	    }
            last_key = current_key;
	    last_line = curr_line;
	    last_addr = current_addr;
            num_keys++;
	    add_to_mem_block(A_CASE_LABELS,
                (char *)&current_key, sizeof(int) );
	    add_to_mem_block(A_CASE_LABELS,
		(char *)&current_addr, sizeof(short) );
            for ( offset = 0; ; )
            {

                int child1,child2;

                child1 = ( offset << 1 ) + sizeof(struct case_heap_entry);
                child2 = child1 + sizeof(struct case_heap_entry);
                if ( child1 >= heap_end_offs ) break;
                if ( ((struct case_heap_entry*)(heap_start+child1))->addr &&
                  ( !((struct case_heap_entry*)(heap_start+child2))->addr ||
                   ((struct case_heap_entry*)(heap_start+child1))->key <=
                   ((struct case_heap_entry*)(heap_start+child2))->key  ) )
                {
                    *(struct case_heap_entry*)(heap_start+offset) =
                    *(struct case_heap_entry*)(heap_start+child1);
                    offset = child1;
                } else
                    if (((struct case_heap_entry*)(heap_start+child2))->addr ) {
                        *(struct case_heap_entry*)(heap_start+offset) =
                        *(struct case_heap_entry*)(heap_start+child2);
                        offset = child2;
                    } else break;
            }
            ((struct case_heap_entry*)(heap_start+offset))->addr = 0;
        }

	/* write start of table */
        upd_short(current_break_address-2,
            mem_block[A_PROGRAM].current_size);

        /* check to see if quicker direct lookup can be used */
	if (block_index == A_CASE_NUMBERS &&
	    num_keys == last_key - first_key + 1)
	{
	    short offset;
	    for (i = 0; i<num_keys; i++) {
		/* only using offset part of table here */
		((char *)&offset)[0] = mem_block[A_CASE_LABELS].block[i*6 + 4];
		((char *)&offset)[1] = mem_block[A_CASE_LABELS].block[i*6 + 5];
              if (offset <= 1) {
                  ((char *)&offset)[0] = mem_block[A_CASE_LABELS].block[i*6 + 4 + 6];
                  ((char *)&offset)[1] = mem_block[A_CASE_LABELS].block[i*6 + 5 + 6];
              }
		ins_short(offset);
	    }
	    ins_long(first_key);
	    mem_block[A_PROGRAM].block[current_break_address-3] = 0xfe;
	} else { /* use slower binary search */
	    /* copy over table */
	    add_to_mem_block(A_PROGRAM, mem_block[A_CASE_LABELS].block,
			     mem_block[A_CASE_LABELS].current_size );
	    /* calculate starting index for search at execution time */
	    for(i=0xf0,o=6; o<<1 <= mem_block[A_CASE_LABELS].current_size; )
		i++,o<<=1;
	    if (block_index == A_CASE_STRINGS) i = ( i << 4 ) | 0xf;
	    /* and store it */
	    mem_block[A_PROGRAM].block[current_break_address-3] &= (i & 0xff);
	}
	upd_short(current_break_address, mem_block[A_PROGRAM].current_size);
	
	mem_block[A_CASE_NUMBERS].current_size = current_case_number_heap;
	mem_block[A_CASE_STRINGS].current_size = current_case_string_heap;
    	current_break_address = pop_address();
	zero_case_label = pop_address();
    	current_case_string_heap = pop_address();
    	current_case_number_heap = pop_address();
        current_break_stack_need -= sizeof(short);
        switches--;
    } ;

case: L_CASE case_label ':'
    {
	struct case_heap_entry temp;

	if ( !( current_break_address & BREAK_FROM_CASE ) ) {
	    yyerror("Case outside switch");
	    break;
	}
	temp.key = $2.key;
	temp.addr = mem_block[A_PROGRAM].current_size;
	temp.line = current_line;
	add_to_case_heap($2.block,&temp);
    }
    | L_CASE case_label L_RANGE case_label ':'
    {
	struct case_heap_entry temp;

	if ( $2.block != A_CASE_NUMBERS || $4.block != A_CASE_NUMBERS )
	    yyerror("String case labels not allowed as range bounds");
	if ($2.key > $4.key) break;
	temp.key = $2.key;
	temp.addr = 1;
	temp.line = current_line;
	add_to_case_heap(A_CASE_NUMBERS,&temp);
	temp.key = $4.key;
	temp.addr = mem_block[A_PROGRAM].current_size;
	temp.line = 0;
	add_to_case_heap(A_CASE_NUMBERS,&temp);
    } ;
	
case_label: constant
        {
	    if ( !(zero_case_label & NO_STRING_CASE_LABELS) && $1 )
		yyerror("Mixed case label list not allowed");
	    if ( ($$.key = $1) )
	        zero_case_label |= SOME_NUMERIC_CASE_LABELS;
	    else
		zero_case_label |= mem_block[A_PROGRAM].current_size;
	    $$.block = A_CASE_NUMBERS;
	}
	  | string_constant
        {
	    if ( zero_case_label & SOME_NUMERIC_CASE_LABELS )
		yyerror("Mixed case label list not allowed");
	    zero_case_label &= ~NO_STRING_CASE_LABELS;
	    store_prog_string($1);
	    $$.key = (int)$1;
	    $$.block = A_CASE_STRINGS;
        }
	  ;

constant:
        constant '|' constant { $$ = $1 | $3; }
      | constant '^' constant { $$ = $1 ^ $3; }
      | constant '&' constant { $$ = $1 & $3; }
      | constant L_EQ constant { $$ = $1 == $3; }
      | constant L_NE constant { $$ = $1 != $3; }
      | constant '>'  constant { $$ = $1 >  $3; }
      | constant L_GE constant { $$ = $1 >= $3; }
      | constant '<'  constant { $$ = $1 <  $3; }
      | constant L_LE constant { $$ = $1 <= $3; } 
      | constant L_LSH constant { $$ = $1 << $3; }
      | constant L_RSH constant { $$ = $1 >> $3; }
      | constant '+' constant { $$ = $1 + $3; }
      | constant '-' constant { $$ = $1 - $3; }
      | constant '*' constant { $$ = $1 * $3; }
      | constant '%' constant
            { if ($3) $$ = $1 % $3; else yyerror("Modulo by zero"); }
      | constant '/' constant
            { if ($3) $$ = $1 / $3; else yyerror("Division by zero"); }
      | '(' constant ')' { $$ = $2; }
      | L_NUMBER
      | '-'   L_NUMBER { $$ = -$2; }
      | L_NOT L_NUMBER { $$ = !$2; }
      | '~'   L_NUMBER { $$ = ~$2; }
      ;

default: L_DEFAULT ':'
    {
	if ( !( current_break_address & BREAK_FROM_CASE ) ) {
	    yyerror("Default outside switch");
	    break;
	}
	current_break_address &= ~(BREAK_ON_STACK|BREAK_FROM_CASE);
	if ( read_short(current_break_address+2 ) )
	    yyerror("Duplicate default");
	upd_short(current_break_address+2, mem_block[A_PROGRAM].current_size);
	current_break_address |= (BREAK_ON_STACK|BREAK_FROM_CASE);
    } ;


comma_expr: expr0 { $$ = $1.type; }
          | comma_expr { insert_pop_value(); }
	',' expr0
	{ $$ = $4.type; } ;

comma_expr1: expr0 { $$ = $1; }
          | comma_expr1 { insert_pop_value(); }
	',' expr0
	{ $$ = $4; $$.iscon = 0; } ;

expr0:
       lvalue assign expr0  %prec '='
	{
	    if (exact_types && !compatible_types($1, $3.type) &&
		!($1 == TYPE_STRING && $3.type == TYPE_NUMBER &&
		  $2 == F_ADD_EQ))
	    {
		type_error("Bad assignment. Rhs", $3.type);
	    }
	    if (($1 == TYPE_REAL) && ($3.type == TYPE_NUMBER)) {
		ins_expr_f_byte(F_TO_FLOAT);
	    } else if (($1 == TYPE_NUMBER) && ($3.type == TYPE_REAL)) {
		ins_expr_f_byte(F_TO_INT);
	    }
	    ins_expr_f_byte($2);
	    $$.type = $3.type;
	    $$.addr = 0; $$.iscon = 0;
	}
     | error assign expr0  %prec '='
        { yyerror("Illegal LHS");  $$.type = TYPE_ANY; }

     | expr0 '?'
	{
	    /* optimize if last expression did F_NOT */
	    if (last_expression == CURRENT_PROGRAM_SIZE-1 &&
		LAST_EXPR_CODE == F_NOT)
	    {
		SET_CURRENT_PROGRAM_SIZE(last_expression);
		ins_f_byte(F_BRANCH_WHEN_NON_ZERO); /* relative0 */
	    } else {
		ins_f_byte(F_BRANCH_WHEN_ZERO); /* relative0 */
	    }
	    push_address();
	    ins_short(0);
	}
      expr0
	{
	    int i;
	    i = pop_address();
	    ins_f_byte(F_BRANCH); /* relative1 */
	    push_address(); ins_short(0);
	    upd_short(i, CURRENT_PROGRAM_SIZE - i); /* relative0 */
	}
      ':' expr0  %prec '?'
	{
	    int i = pop_address();

	    last_expression = -1;
	    upd_short(i, CURRENT_PROGRAM_SIZE - i); /* relative1 */
	    if (exact_types && !compatible_types($4.type, $7.type)) {
		type_error("Different types in ?: expr", $4.type);
		type_error("                      and ", $7.type);
	    }
	    if ($4.type == TYPE_ANY)
		$$.type = $7.type;
	    else if (TYPE($4.type, TYPE_MOD_POINTER|TYPE_ANY))
		$$.type = $7.type;
	    else
		$$.type = $4.type;
	    $$.addr = $1.addr; $$.iscon = 0;
	}

     | expr0 L_LOR
	{
#if 0
	    ins_f_byte(F_DUP);
	    ins_f_byte(F_BRANCH_WHEN_NON_ZERO); /* relative2 */
	    push_address();
	    ins_short(0);
	    ins_f_byte(F_POP_VALUE);
#else
	    ins_f_byte(F_LOR);
	    push_address();
	    ins_short(0);
#endif
	}
       expr0
	{
		int i = pop_address();

	    last_expression = -1;
	    upd_short(i, CURRENT_PROGRAM_SIZE - i); /* relative2 */
	    if ($1.type == $4.type)
		$$.type = $1.type;
	    else
		$$.type = TYPE_ANY;	/* Return type can't be known */
	    $$.addr = $1.addr; $$.iscon = 0;
	}
     | expr0 L_LAND
	{
#if 0
	    ins_f_byte(F_DUP);
	    ins_f_byte(F_BRANCH_WHEN_ZERO); /* relative3 */
	    push_address();
	    ins_short(0);
	    ins_f_byte(F_POP_VALUE);
#else
	    ins_f_byte(F_LAND);
	    push_address();
	    ins_short(0);
#endif
	}
       expr0
	{
		int i = pop_address();

	    last_expression = -1;
	    upd_short(i, CURRENT_PROGRAM_SIZE - i); /* relative3 */
	    if ($1.type == $4.type)
		$$.type = $1.type;
	    else
		$$.type = TYPE_ANY;	/* Return type can't be known */
	    $$.addr = $1.addr; $$.iscon = 0;
	}

     | expr0 '|' expr0
          {
	      if (exact_types && !TYPE($1.type,TYPE_NUMBER))
		  type_error("Bad argument 1 to |", $1.type);
	      if (exact_types && !TYPE($3.type,TYPE_NUMBER))
		  type_error("Bad argument 2 to |", $3.type);
	      $$.type = TYPE_NUMBER;
	      $$.addr = $1.addr; $$.iscon = 0;
#ifdef LPC_OPTIMIZE
	      /* constant expressions */
	      if ($1.iscon && BASIC_TYPE($1.type, TYPE_NUMBER) &&
		  $3.iscon && BASIC_TYPE($3.type, TYPE_NUMBER))
	      {
		  SET_CURRENT_PROGRAM_SIZE($1.addr);
		  write_number(read_number($1.addr) | read_number($3.addr));
		  $$.iscon = 1;
		  break;
	      }
#endif
	      ins_f_byte(F_OR);
	  }
       | expr0 '^' expr0
	  {
	      if (exact_types && !TYPE($1.type,TYPE_NUMBER))
		  type_error("Bad argument 1 to ^", $1.type);
	      if (exact_types && !TYPE($3.type,TYPE_NUMBER))
		  type_error("Bad argument 2 to ^", $3.type);
	      $$.type = TYPE_NUMBER;
	      $$.addr = $1.addr; $$.iscon = 0;
#ifdef LPC_OPTIMIZE
	      /* constant expressions */
	      if ($1.iscon && BASIC_TYPE($1.type, TYPE_NUMBER) &&
		  $3.iscon && BASIC_TYPE($3.type, TYPE_NUMBER))
	      {
		  SET_CURRENT_PROGRAM_SIZE($1.addr);
		  write_number(read_number($1.addr) ^ read_number($3.addr));
		  $$.iscon = 1;
		  break;
	      }
#endif
	      ins_f_byte(F_XOR);
	  }
       | expr0 '&' expr0
	  {
	      if ( !($1.type & TYPE_MOD_POINTER) || !($3.type & TYPE_MOD_POINTER) ) {
	          if (exact_types && !TYPE($1.type,TYPE_NUMBER))
		      type_error("Bad argument 1 to &", $1.type);
	          if (exact_types && !TYPE($3.type,TYPE_NUMBER))
		      type_error("Bad argument 2 to &", $3.type);
	      }
	      $$.type = TYPE_NUMBER;
	      $$.addr = $1.addr; $$.iscon = 0;
#ifdef LPC_OPTIMIZE
	      /* constant expressions */
	      if ($1.iscon && BASIC_TYPE($1.type, TYPE_NUMBER) &&
		  $3.iscon && BASIC_TYPE($3.type, TYPE_NUMBER))
	      {
		  SET_CURRENT_PROGRAM_SIZE($1.addr);
		  write_number(read_number($1.addr) & read_number($3.addr));
		  $$.iscon = 1;
		  break;
	      }
#endif
	      ins_f_byte(F_AND);
	  }

      | expr0 L_EQ expr0
	{
	    int t1 = $1.type & TYPE_MOD_MASK, t2 = $3.type & TYPE_MOD_MASK;
	    if (exact_types &&
		(t1 != t2) &&
!((t1 & (TYPE_NUMBER | TYPE_REAL)) && (t2 & (TYPE_NUMBER | TYPE_REAL))) &&
		(t1 != TYPE_ANY && t2 != TYPE_ANY)) {
		type_error("== always false because of incompatible types",$1.type);
		type_error("                               compared to", $3.type);
	    }
	    ins_f_byte(F_EQ);
	    $$.type = TYPE_NUMBER;
	    $$.addr = $1.addr; $$.iscon = 0;
	}
      | expr0 L_NE expr0
	{
	    int t1 = $1.type & TYPE_MOD_MASK, t2 = $3.type & TYPE_MOD_MASK;
	    if (exact_types &&
		(t1 != t2) &&
!((t1 & (TYPE_NUMBER | TYPE_REAL)) && (t2 & (TYPE_NUMBER | TYPE_REAL))) &&
		(t1 != TYPE_ANY && t2 != TYPE_ANY)) {
		type_error("!= always true because of incompatible types", $1.type);
		type_error("                               compared to", $3.type);
	    }
	    ins_f_byte(F_NE);
	    $$.type = TYPE_NUMBER;
	    $$.addr = $1.addr; $$.iscon = 0;
	}

      | expr0 '>' expr0
	{
	    $$.addr = $1.addr; $$.iscon = 0;
	    $$.type = TYPE_NUMBER;
	    ins_f_byte(F_GT);
	};
      | expr0 L_GE expr0
	{
	    $$.addr = $1.addr; $$.iscon = 0;
	    $$.type = TYPE_NUMBER;
	    ins_f_byte(F_GE);
	};
      | expr0 '<' expr0
	{
	    $$.addr = $1.addr; $$.iscon = 0;
	    $$.type = TYPE_NUMBER;
	    ins_f_byte(F_LT);
	};
      | expr0 L_LE expr0
	{
	    $$.addr = $1.addr; $$.iscon = 0;
	    $$.type = TYPE_NUMBER;
	    ins_f_byte(F_LE);
	};

      | expr0 L_LSH expr0
	{
	    ins_f_byte(F_LSH);
	    $$.type = TYPE_NUMBER;
	    $$.addr = $1.addr; $$.iscon = 0;
	    if (exact_types && !TYPE($1.type, TYPE_NUMBER))
		type_error("Bad argument number 1 to '<<'", $1.type);
	    if (exact_types && !TYPE($3.type, TYPE_NUMBER))
		type_error("Bad argument number 2 to '<<'", $3.type);
	}
      | expr0 L_RSH expr0
	{
	    ins_f_byte(F_RSH);
	    $$.type = TYPE_NUMBER;
	    $$.addr = $1.addr; $$.iscon = 0;
	    if (exact_types && !TYPE($1.type, TYPE_NUMBER))
		type_error("Bad argument number 1 to '>>'", $1.type);
	    if (exact_types && !TYPE($3.type, TYPE_NUMBER))
		type_error("Bad argument number 2 to '>>'", $3.type);
	}

      | expr0 '+' expr0	/* Type checks of this case are incomplete */
	{
	  if($1.type == $3.type)
	    $$.type = $1.type;
	  else if(($1.type & TYPE_ANY) || ($3.type & TYPE_ANY))
	    $$.type = TYPE_ANY;
	  else if((TYPE($1.type, TYPE_NUMBER) && TYPE($3.type, TYPE_REAL)) ||
		  (TYPE($1.type, TYPE_REAL) && TYPE($3.type, TYPE_NUMBER)))
	    $$.type = TYPE_REAL;
	  else
	    $$.type = TYPE_ANY;

	  $$.addr = $1.addr;
	  $$.iscon = 0;

#ifdef LPC_OPTIMIZE
	  /* optimize 0's */
	  if ($1.iscon && $1.type == TYPE_ANY &&
	      ($3.type == TYPE_NUMBER || $3.type == TYPE_REAL))
	  {
	      /* 0 + X */
	      /* Currently, $3.addr isn't set a lot of times */
	      int len;
	      len = CURRENT_PROGRAM_SIZE - ($1.addr + SIZEOF_ZERO_CODE);
	      memcpy(mem_block[A_PROGRAM].block + $1.addr,
		     mem_block[A_PROGRAM].block + $1.addr + SIZEOF_ZERO_CODE,
		     len);
	      SET_CURRENT_PROGRAM_SIZE($1.addr + len);
	      $$ = $3;
	      $$.addr = $1.addr;
	      last_expression = -1;  /* could be smarter */
	      break;
	  } else if ($3.iscon && $3.type == TYPE_ANY &&
		     ($1.type == TYPE_NUMBER || $1.type == TYPE_REAL))
	  {
	      /* X + 0 */
	      SET_CURRENT_PROGRAM_SIZE($3.addr);
	      $$ = $1;
	      last_expression = -1; /* ? */
	      break;
	  }
	  
	  /* constant expressions */
	  if ($1.iscon && $3.iscon) {
	      if (BASIC_TYPE($1.type, TYPE_NUMBER) &&
		  BASIC_TYPE($3.type, TYPE_NUMBER))
	      {
		  SET_CURRENT_PROGRAM_SIZE($1.addr);
		  write_number(read_number($1.addr) +
			       read_number($3.addr));
		  $$.iscon = 1;
		  break;
	      } else if ($$.type == TYPE_REAL) {
		  double val;
		  if ($1.type == TYPE_NUMBER)
		      val = read_number($1.addr) + read_real($3.addr);
		  else if ($3.type == TYPE_NUMBER)
		      val = read_real($1.addr) + read_number($3.addr);
		  else
		      val = read_real($1.addr) + read_real($3.addr);
		  SET_CURRENT_PROGRAM_SIZE($1.addr);
		  ins_f_byte(F_REAL);
		  ins_real(val);
		  $$.iscon = 1;
		  break;
	      } else if ($$.type == TYPE_STRING) {
		  /* Combine strings */
		  short n1, n2;
		  char *new, *s1, *s2;
		  n1 = read_string($1.addr);
		  s1 = ((char **)mem_block[A_STRINGS].block)[n1];
		  n2 = read_string($3.addr);
		  s2 = ((char **)mem_block[A_STRINGS].block)[n2];
		  new = DXALLOC( strlen(s1)+strlen(s2)+1, 53, "string add" );
		  strcpy(new, s1);
		  strcat(new, s2);
		  /* free old strings (ordering may help shrink table) */
		  if (n1 > n2) {
		      free_prog_string(n1); free_prog_string(n2);
		  } else {
		      free_prog_string(n2); free_prog_string(n1);
		  }
		  /* store new string */
		  SET_CURRENT_PROGRAM_SIZE($1.addr);
		  ins_f_byte(F_STRING);
		  ins_short(store_prog_string(new));
		  FREE(new);
		  $$.iscon = 1;
		  break;
	      }
	  }
#endif /* OPTIMIZE */
	  /* wasn't optimized away */
	  ins_f_byte(F_ADD);
	};
      | expr0 '-' expr0
	{
	  int bad_arg = 0;

	  if(exact_types){
	    if(!TYPE($1.type, TYPE_NUMBER) &&
		!TYPE($1.type, TYPE_REAL) &&
		!($1.type & TYPE_MOD_POINTER)){
	      type_error("Bad argument number 1 to '-'", $1.type);
	      bad_arg++;
	    }
	    if(!TYPE($3.type, TYPE_NUMBER) &&
	       !TYPE($3.type, TYPE_REAL) &&
	       !($3.type & TYPE_MOD_POINTER)){
	      type_error("Bad argument number 2 to '-'", $3.type);
	      bad_arg++;
	    }
	  }
	  $$.type = TYPE_ANY;
	  $$.addr = $1.addr;
	  $$.iscon = 0;
	  if (($1.type & TYPE_MOD_POINTER) || ($3.type & TYPE_MOD_POINTER))
	    $$.type = TYPE_MOD_POINTER | TYPE_ANY;
	  if (!($1.type & TYPE_MOD_POINTER) || !($3.type & TYPE_MOD_POINTER)) {
	    if (exact_types && $$.type != TYPE_ANY && !bad_arg)
	      yyerror("Arguments to '-' don't match");
	    if(($1.type & TYPE_ANY) || ($3.type & TYPE_ANY))
	      $$.type = TYPE_ANY;
	    else if((TYPE($1.type, TYPE_NUMBER) && TYPE($3.type, TYPE_NUMBER)))
	      $$.type = TYPE_NUMBER;
	    else
	      $$.type = TYPE_REAL;
	  }
	  
#ifdef LPC_OPTIMIZE
	  if ($3.iscon && $3.type == TYPE_ANY) {  /* optimize X-0 */
	      SET_CURRENT_PROGRAM_SIZE($3.addr);
	      $$ = $1;
	      last_expression = -1; /* ? */
	      break;
	  }

	  /* constant expressions */
	  if ($1.iscon && $3.iscon) {
	      if (BASIC_TYPE($1.type, TYPE_NUMBER) &&
		  BASIC_TYPE($3.type, TYPE_NUMBER))
	      {
		  SET_CURRENT_PROGRAM_SIZE($1.addr);
		  write_number(read_number($1.addr) - read_number($3.addr));
		  $$.iscon = 1;
		  break;
	      } else if ($$.type == TYPE_REAL) {
		  double val;
		  if ($1.type == TYPE_NUMBER)
		      val = read_number($1.addr) - read_real($3.addr);
		  else if ($3.type == TYPE_NUMBER)
		      val = read_real($1.addr) - read_number($3.addr);
		  else
		      val = read_real($1.addr) - read_real($3.addr);
		  SET_CURRENT_PROGRAM_SIZE($1.addr);
		  ins_f_byte(F_REAL);
		  ins_real(val);
		  $$.iscon = 1;
		  break;
	      }
	  }
#endif
	  ins_f_byte(F_SUBTRACT);
	}
      | expr0 '*' expr0
	{
	  if (($1.type != TYPE_MAPPING) || ($3.type != TYPE_MAPPING)) {
	    if (exact_types && !TYPE($1.type, TYPE_NUMBER) && !TYPE($1.type, TYPE_REAL))
	      type_error("Bad argument number 1 to '*'", $1.type);
	    if (exact_types && !TYPE($3.type, TYPE_NUMBER) && !TYPE($3.type, TYPE_REAL))
	      type_error("Bad argument number 2 to '*'", $3.type);
	    if (TYPE($1.type, TYPE_NUMBER) && TYPE($3.type, TYPE_NUMBER))
	      $$.type = TYPE_NUMBER;
	    else
              $$.type = TYPE_REAL;
	  }
	  else
	    $$.type = TYPE_MAPPING;
	  $$.addr = $1.addr; $$.iscon = 0;
	  
#ifdef LPC_OPTIMIZE
	  /* constant expressions */
	  if ($1.iscon && $3.iscon) {
	      if (BASIC_TYPE($1.type, TYPE_NUMBER) &&
		  BASIC_TYPE($3.type, TYPE_NUMBER))
	      {
		  SET_CURRENT_PROGRAM_SIZE($1.addr);
		  write_number(read_number($1.addr) * read_number($3.addr));
		  $$.iscon = 1;
		  break;
	      } else if ($$.type == TYPE_REAL) {
		  double val;
		  if ($1.type == TYPE_NUMBER)
		      val = read_number($1.addr) * read_real($3.addr);
		  else if ($3.type == TYPE_NUMBER)
		      val = read_real($1.addr) * read_number($3.addr);
		  else
		      val = read_real($1.addr) * read_real($3.addr);
		  SET_CURRENT_PROGRAM_SIZE($1.addr);
		  ins_f_byte(F_REAL);
		  ins_real(val);
		  $$.iscon = 1;
		  break;
	      }
	  }
#endif
	  ins_f_byte(F_MULTIPLY);
	};
      | expr0 '%' expr0
	{
	    if (exact_types && !TYPE($1.type, TYPE_NUMBER))
		type_error("Bad argument number 1 to '%'", $1.type);
	    if (exact_types && !TYPE($3.type, TYPE_NUMBER))
		type_error("Bad argument number 2 to '%'", $3.type);
	    $$.type = TYPE_NUMBER;
	    $$.addr = $1.addr; $$.iscon = 0;
#ifdef LPC_OPTIMIZE
	      /* constant expressions */
	      if ($1.iscon && BASIC_TYPE($1.type, TYPE_NUMBER) &&
		  $3.iscon && BASIC_TYPE($3.type, TYPE_NUMBER))
	      {
		  SET_CURRENT_PROGRAM_SIZE($1.addr);
		  write_number(read_number($1.addr) % read_number($3.addr));
		  $$.iscon = 1;
		  break;
	      }
#endif
	    ins_f_byte(F_MOD);
	};
      | expr0 '/' expr0
	{
	    if (exact_types && !TYPE($1.type, TYPE_NUMBER) && !TYPE($1.type, TYPE_REAL))
		type_error("Bad argument number 1 to '/'", $1.type);
	    if (exact_types && !TYPE($3.type, TYPE_NUMBER) && !TYPE($3.type, TYPE_REAL))
		type_error("Bad argument number 2 to '/'", $3.type);
	    if (TYPE($1.type, TYPE_NUMBER) && TYPE($3.type, TYPE_NUMBER))
		$$.type = TYPE_NUMBER;
	    else
		$$.type = TYPE_REAL;
	    $$.addr = $1.addr; $$.iscon = 0;
#ifdef LPC_OPTIMIZE
	    /* constant expressions */
	    if ($1.iscon && $3.iscon) {
		if (BASIC_TYPE($1.type, TYPE_NUMBER) &&
		    BASIC_TYPE($3.type, TYPE_NUMBER))
		{
		    int v;
		    if ((v = read_number($3.addr)) == 0) {
			yyerror("Divide by zero in constant");
			break;
		    }
		    SET_CURRENT_PROGRAM_SIZE($1.addr);
		    write_number(read_number($1.addr) / v);
		    $$.iscon = 1;
		    break;
		} else if ($$.type == TYPE_REAL) {
		    double val;
		    /* a bit complicated with divide-by-zero checking */
		    if ($3.type == TYPE_REAL) {
			if ((val = read_real($3.addr)) == 0.0) {
			    yyerror("Divide by zero in constant");
			    break;
			}
			if ($1.type == TYPE_NUMBER)
			    val = read_number($1.addr) / val;
			else
			    val = read_real($1.addr) / val;
		    } else {
			int iv;
			if ((iv = read_number($3.addr)) == 0) {
			    yyerror("Divide by zero in constant");
			    break;
			}
			val = read_real($1.addr) / iv;
		    }
		    SET_CURRENT_PROGRAM_SIZE($1.addr);
		    ins_f_byte(F_REAL);
		    ins_real(val);
		    $$.iscon = 1;
		    break;
		}
	    }
#endif
	    ins_f_byte(F_DIVIDE);
	}
      | cast expr0  %prec L_NOT
        {
	  $$.type = $1;
	  $$.addr = $2.addr; $$.iscon = 0;
	  if (exact_types && $2.type != TYPE_ANY && $2.type != TYPE_UNKNOWN &&
	      $1 != TYPE_VOID)
	type_error("Casts are only legal for type mixed, or when unknown", $2.type);
	}

      | L_INC lvalue  %prec L_NOT  /* note lower precedence here */
        {
	    ins_expr_f_byte(F_PRE_INC);
	    if (exact_types && !TYPE($2, TYPE_NUMBER) && !TYPE($2, TYPE_REAL))
		type_error("Bad argument to ++", $2);
            if (TYPE($2, TYPE_NUMBER))
		$$.type = TYPE_NUMBER;
            else
		$$.type = TYPE_REAL;
	    $$.addr = $$.iscon = 0;
	};
      | L_DEC lvalue  %prec L_NOT  /* note lower precedence here */
        {
	    ins_expr_f_byte(F_PRE_DEC);
	    if (exact_types && !TYPE($2, TYPE_NUMBER) && !TYPE($2, TYPE_REAL))
		type_error("Bad argument to --", $2);
            if (TYPE($2, TYPE_NUMBER))
		$$.type = TYPE_NUMBER;
            else
		$$.type = TYPE_REAL;
	    $$.addr = $$.iscon = 0;
	};
      | L_NOT expr0
	{
	    $$.type = TYPE_NUMBER;
	    $$.addr = $2.addr; $$.iscon = 0;
#ifdef LPC_OPTIMIZE
	    /* constant expressions */
	    if ($2.iscon && BASIC_TYPE($2.type, TYPE_NUMBER))
	    {
		SET_CURRENT_PROGRAM_SIZE($2.addr);
		write_number(!read_number($2.addr));
		$$.iscon = 1;
		break;
	    }
#endif
	    ins_expr_f_byte(F_NOT);	/* Any type is valid here. */
	};
      | '~' expr0
	{
	    if (exact_types && !TYPE($2.type, TYPE_NUMBER))
		type_error("Bad argument to ~", $2.type);
	    $$.type = TYPE_NUMBER;
	    $$.addr = $2.addr; $$.iscon = 0;
#ifdef LPC_OPTIMIZE
	    /* constant expressions */
	    if ($2.iscon && BASIC_TYPE($2.type, TYPE_NUMBER))
	    {
		SET_CURRENT_PROGRAM_SIZE($2.addr);
		write_number(~read_number($2.addr));
		$$.iscon = 1;
		break;
	    }
#endif
	    ins_f_byte(F_COMPL);
	};
      | '-' expr0  %prec L_NOT
	{
	    if (exact_types && !TYPE($2.type, TYPE_NUMBER) && !TYPE($2.type, TYPE_REAL))
		type_error("Bad argument to unary '-'", $2.type);
            if (TYPE($2.type, TYPE_NUMBER))
	      $$.type = TYPE_NUMBER;
	    else
	      $$.type = TYPE_REAL;
	    $$.addr = $2.addr; $$.iscon = 0;
#ifdef LPC_OPTIMIZE
	    /* constant expressions */
	    if ($2.iscon) {
		if ($$.type == TYPE_NUMBER) {
		    SET_CURRENT_PROGRAM_SIZE($2.addr);
		    write_number(-read_number($2.addr));
		    $$.iscon = 1;
		    break;
		} else if ($$.type == TYPE_REAL) {
		    SET_CURRENT_PROGRAM_SIZE($2.addr);
		    ins_f_byte(F_REAL);
		    ins_real(-read_real($2.addr));
		    $$.iscon = 1;
		    break;
		}
	    }
#endif
	    ins_f_byte(F_NEGATE);
	}

      | lvalue L_INC   /* normal precedence here */
         {
	     ins_expr_f_byte(F_POST_INC);
	     if (exact_types && !TYPE($1, TYPE_NUMBER) && !TYPE($1, TYPE_REAL))
		 type_error("Bad argument to ++", $1);
            if (TYPE($1, TYPE_NUMBER))
		$$.type = TYPE_NUMBER;
            else
		$$.type = TYPE_REAL;
	    $$.addr = $$.iscon = 0;
	 };
      | lvalue L_DEC
         {
	     ins_expr_f_byte(F_POST_DEC);
	     if (exact_types && !TYPE($1, TYPE_NUMBER) && !TYPE($1, TYPE_REAL))
		 type_error("Bad argument to --", $1);
            if (TYPE($1, TYPE_NUMBER))
		$$.type = TYPE_NUMBER;
            else
		$$.type = TYPE_REAL;
	    $$.addr = $$.iscon = 0;
	 }

      | expr4
      ;

assign: '=' { $$ = F_ASSIGN; }
      | L_AND_EQ { $$ = F_AND_EQ; }
      | L_OR_EQ { $$ = F_OR_EQ; }
      | L_XOR_EQ { $$ = F_XOR_EQ; }
      | L_LSH_EQ { $$ = F_LSH_EQ; }
      | L_RSH_EQ { $$ = F_RSH_EQ; }
      | L_ADD_EQ { $$ = F_ADD_EQ; }
      | L_SUB_EQ { $$ = F_SUB_EQ; }
      | L_MULT_EQ { $$ = F_MULT_EQ; }
      | L_MOD_EQ { $$ = F_MOD_EQ; }
      | L_DIV_EQ { $$ = F_DIV_EQ; };

return: L_RETURN
	{
	    if (exact_types && !TYPE(exact_types, TYPE_VOID))
		type_error("Must return a value for a function declared",
			   exact_types);
	    ins_f_byte(F_CONST0);
	    ins_f_byte(F_RETURN);
	}
      | L_RETURN comma_expr
	{
	    if (exact_types && !TYPE($2, exact_types & TYPE_MOD_MASK))
		type_error("Return type not matching", exact_types);
	    last_expression = -1;
	    ins_f_byte(F_RETURN);
	};

expr_list: /* empty */		{ $$ = 0; }
	 | expr_list2		{ $$ = $1; }
	 | expr_list2 ','	{ $$ = $1; } ; /* Allow a terminating comma */

expr_list2: expr0		{ $$ = 1; add_arg_type($1.type); }
         | expr_list2 ',' expr0	{ $$ = $1 + 1; add_arg_type($3.type); } ;

expr_list3: /* empty */         { $$ = 0; }
           | expr_list4           { $$ = $1; }
           | expr_list4 ','       { $$ = $1; } ; /* Allow terminating comma */

expr_list4: assoc_pair          { $$ = $1; }
           | expr_list4 ',' assoc_pair  { $$ = $1 + 2; } ;

assoc_pair: expr0 ':' expr0    { $$ = 2; } ;

expr4: function_call { $$.type = $1; $$.addr = $$.iscon = 0; }
     | lvalue
	{
	    int pos = mem_block[A_PROGRAM].current_size;
	    /* Some optimization. Replace the push-lvalue with push-value */
	    if (last_push_identifier == pos-2)
		mem_block[A_PROGRAM].block[last_push_identifier] = F_IDENTIFIER;
	    else if (last_push_local == pos-2)
		mem_block[A_PROGRAM].block[last_push_local] = F_LOCAL_NAME;
	    else if (last_push_indexed == pos-1)
		mem_block[A_PROGRAM].block[last_push_indexed] = F_INDEX;
	    else if (last_push_indexed != 0)
		fatal("Should be a push at this point !\n");
	    $$.type = $1;
	    $$.addr = $$.iscon = 0;
	}
     | string | number | real
     | '(' comma_expr1 ')' { $$ = $2; }
     | catch { $$.type = TYPE_ANY; $$.addr = $$.iscon = 0; }
     | sscanf { $$.type = TYPE_NUMBER; $$.addr = $$.iscon = 0; }
     | parse_command { $$.type = TYPE_NUMBER; $$.addr = $$.iscon = 0; }
     | time_expression { $$.type = TYPE_NUMBER; $$.addr = $$.iscon = 0; }
     | '(' ':' expr0 ',' expr0 ':' ')'
         {
             ins_f_byte(F_FUNCTION_CONSTRUCTOR);
             $$.type = TYPE_FUNCTION;
	     $$.addr = $3.addr; $$.iscon = 0;
         }
     | '(' '[' expr_list3 ']' ')' 
         { 
             ins_f_byte(F_AGGREGATE_ASSOC);
             ins_short($3);
             $$.type = TYPE_MAPPING;
	     $$.addr = $$.iscon = 0;
         }
     | '(' '{' expr_list '}' ')'
       {
	   pop_arg_stack($3);		/* We don't care about these types */
	   ins_f_byte(F_AGGREGATE);
	   ins_short($3);
	   $$.type = TYPE_MOD_POINTER | TYPE_ANY;
	   $$.addr = $$.iscon = 0;
       };

catch: L_CATCH { ins_f_byte(F_CATCH); push_address(); ins_short(0);}
       '(' comma_expr ')'
	       {
	       ins_f_byte(F_END_CATCH);
		   upd_short(pop_address(), mem_block[A_PROGRAM].current_size);
	       };


sscanf: L_SSCANF '(' expr0 ',' expr0 lvalue_list ')'
	{
	    ins_f_byte(F_SSCANF); ins_byte($6 + 2);
	}

parse_command: L_PARSE_COMMAND '(' expr0 ',' expr0 ',' expr0 lvalue_list ')'
	{
	    ins_f_byte(F_PARSE_COMMAND); ins_byte($8 + 3);
	}

time_expression: L_TIME_EXPRESSION { ins_f_byte(F_TIME_EXPRESSION); }
	'(' comma_expr ')' { ins_f_byte(F_END_TIME_EXPRESSION); }
	;

lvalue_list: /* empty */ { $$ = 0; }
	   | ',' lvalue lvalue_list { $$ = 1 + $3; } ;

lvalue: L_IDENTIFIER
	{
	    int i = verify_declared($1);

	    last_push_identifier = CURRENT_PROGRAM_SIZE;
	    ins_f_byte(F_PUSH_IDENTIFIER_LVALUE);
	    ins_byte(i);
	    FREE($1);
	    if (i == -1)
		$$ = TYPE_ANY;
	    else
		$$ = VARIABLE(i)->type & TYPE_MOD_MASK;
	}
        | L_LOCAL_NAME
	{
	    last_push_local = CURRENT_PROGRAM_SIZE;
	    ins_f_byte(F_PUSH_LOCAL_VARIABLE_LVALUE);
	    ins_byte($1);
	    $$ = type_of_locals[$1];
	}
	| expr4 '[' comma_expr L_RANGE comma_expr ']'
       {
         ins_f_byte(F_RANGE);
         last_push_indexed = 0;
         if (exact_types) {
             if (($1.type & TYPE_MOD_POINTER) == 0
               && !TYPE($1.type, TYPE_STRING))
                 type_error("Bad type to indexed value", $1.type);
             if (!TYPE($3, TYPE_NUMBER))
                 type_error("Bad type of index", $3);
             if (!TYPE($5, TYPE_NUMBER))
                 type_error("Bad type of index", $5);
         }
            if ($1.type == TYPE_ANY)
                $$ = TYPE_ANY;
            else if (TYPE($1.type, TYPE_STRING))
                $$ = TYPE_STRING;
            else if ($1.type & TYPE_MOD_POINTER)
                $$ = $1.type;
            else if (exact_types)
                type_error("Bad type of argument used for range", $1.type);
       };
	| expr4 '[' comma_expr ']'
	{ 
               last_push_indexed = CURRENT_PROGRAM_SIZE;
               if (TYPE($1.type, TYPE_MAPPING) || TYPE($1.type, TYPE_FUNCTION)) {
                  ins_f_byte(F_PUSH_INDEXED_LVALUE);
                  $$ = TYPE_ANY;
               } else {
                ins_f_byte(F_PUSH_INDEXED_LVALUE);
                if (exact_types) {
                      if (!($1.type & TYPE_MOD_POINTER) && !TYPE($1.type, TYPE_STRING))
                        type_error("Bad type to indexed value", $1.type);
                      if (!TYPE($3, TYPE_NUMBER))
                        type_error("Bad type of index", $3);
                }
                if ($1.type == TYPE_ANY)
                    $$ = TYPE_ANY;
                else if (TYPE($1.type, TYPE_STRING))
                    $$ = TYPE_NUMBER;
                else
                    $$ = $1.type & TYPE_MOD_MASK & ~TYPE_MOD_POINTER;
              }
           };

string: string_con2
	{
	    $$.type = TYPE_STRING;
	    $$.iscon = 1;
	    $$.addr = CURRENT_PROGRAM_SIZE;
	    ins_f_byte(F_STRING);
	    ins_short(store_prog_string($1));
	    FREE($1);
	};

string_constant: string_con1
        {
            char *p = make_shared_string($1);
            FREE($1);
            $$ = p;
        };

string_con1: L_STRING
	   | string_con1 '+' L_STRING
      {
          $$ = DXALLOC( strlen($1) + strlen($3) + 1, 53, "string_con1" );
          strcpy($$, $1);
          strcat($$, $3);
          FREE($1);
          FREE($3);
      };

string_con2: L_STRING
         | string_con2 L_STRING
        {
          $$ = DXALLOC( strlen($1) + strlen($2) + 1, 53, "string_con2" );
	  strcpy($$, $1);
          strcat($$, $2);
	  FREE($1);
          FREE($2);
        };

function_call: function_name
    {
	/* This seems to be an ordinary function call. But, if the function
	 * is not defined, then it might be a call to a simul_efun.
	 * If it is, then we make it a call_other(), which requires the
	 * function name as argument.
	 * We have to remember until after parsing the arguments if it was
	 * a simulated efun or not, which means that the pointer has to be
	 * pushed on a stack. Use the internal yacc stack for this purpose.
	 */
	$<funp>$ = 0;
	if (defined_function($1) == -1) {
	    char *p = make_shared_string($1);
	    $<funp>$ = find_simul_efun(p);
	    if ($<funp>$ && !($<funp>$->type & TYPE_MOD_STATIC)) {
		ins_f_byte(F_STRING);
		ins_short(store_prog_string(
	              query_simul_efun_file_name()));
		ins_f_byte(F_STRING);
		ins_short(store_prog_string(p));
	    } else {
		$<funp>$ = 0;
	    }
	    free_string(p);
	}
    }
	'(' expr_list ')'
    { 
	int f;
	int efun_override = strncmp($1, "efun::", 6) == 0;

	if ($<funp>2) {
	    ins_f_byte(F_CALL_OTHER);
	    ins_byte($4 + 2);
	    $$ = $<funp>2->type;
	} else if (!efun_override && (f = defined_function($1)) >= 0) {
	    struct function *funp;
	    ins_f_byte(F_CALL_FUNCTION_BY_ADDRESS); ins_short(f);
	    ins_byte($4);	/* Actual number of arguments */
	    funp = FUNCTION(f);
	    if (funp->flags & NAME_UNDEFINED)
		find_inherited(funp);
	    /*
	     * Verify that the function has been defined already.
	     */
	    if ((funp->flags & NAME_UNDEFINED) &&
		!(funp->flags & NAME_PROTOTYPE) && exact_types)
	    {
		char buff[100];
		sprintf(buff, "Function %.50s undefined", funp->name);
		yyerror(buff);
	    }
	    $$ = funp->type & TYPE_MOD_MASK;
	    /*
	     * Check number of arguments.
	     */
	    if (funp->num_arg != $4 && !(funp->type & TYPE_MOD_VARARGS) &&
		(funp->flags & NAME_STRICT_TYPES) && exact_types)
	    {
		char buff[100];
		sprintf(buff, "Wrong number of arguments to %.60s", $1);
		yyerror(buff);
	    }
	    /*
	     * Check the argument types.
	     */
	    if (exact_types && *(unsigned short *)&mem_block[A_ARGUMENT_INDEX].block[f * sizeof (unsigned short)] != INDEX_START_NONE)
	    {
		int i, first;
		unsigned short *arg_types;
		
		arg_types = (unsigned short *)
		    mem_block[A_ARGUMENT_TYPES].block;
		first = *(unsigned short *)&mem_block[A_ARGUMENT_INDEX].block[f * sizeof (unsigned short)];
		for (i=0; (unsigned)i < funp->num_arg && i < $4; i++) {
		    int tmp = get_argument_type(i, $4);
		    if (!TYPE(tmp, arg_types[first + i])) {
			char buff[100];
			sprintf(buff, "Bad type for argument %d %s", i+1,
				get_two_types(arg_types[first+i], tmp));
			yyerror(buff);
		    }
		}
	    }
	} else if (efun_override || (f = lookup_predef($1)) != -1) {
	    int min, max, def, *argp;
	    extern int efun_arg_types[];

	    if (efun_override) {
		f = lookup_predef($1+6);
	    }
	    if (f == -1) {	/* Only possible for efun_override */
		char buff[100];
		sprintf(buff, "Unknown efun: %s", $1+6);
		yyerror(buff);
	    } else {
		min = instrs[f].min_arg;
		max = instrs[f].max_arg;
		def = instrs[f].Default;
		$$ = instrs[f].ret_type;
		argp = &efun_arg_types[instrs[f].arg_index];
		if (def && $4 == min-1) {
		    ins_f_byte(def);
		    max--;
		    min--;
		} else if ($4 < min) {
		    char bff[100];
		    sprintf(bff, "Too few arguments to %s", instrs[f].name);
		    yyerror(bff);
		} else if ($4 > max && max != -1) {
		    char bff[100];
		    sprintf(bff, "Too many arguments to %s", instrs[f].name);
		    yyerror(bff);
		} else if (max != -1 && exact_types) {
		    /*
		     * Now check all types of the arguments to efuns.
		     */
		    int i, argn;
		    char buff[100];
		    for (argn=0; argn < $4; argn++) {
			int tmp = get_argument_type(argn, $4);
			for(i=0; !TYPE(argp[i], tmp) && argp[i] != 0; i++)
			    ;
			if (argp[i] == 0) {
			    sprintf(buff, "Bad argument %d type to efun %s()",
				    argn+1, instrs[f].name);
			    yyerror(buff);
			}
			while(argp[i] != 0)
			    i++;
			argp += i + 1;
		    }
		}
		ins_f_byte(f);
		/* Only store number of arguments for instructions
		 * that allowed a variable number.
		 */
		if (max != min)
		    ins_byte($4);/* Number of actual arguments */
	    }
	} else {
	    struct function *funp;

	    f = define_new_function($1, 0, 0, 0, NAME_UNDEFINED, 0);
	    ins_f_byte(F_CALL_FUNCTION_BY_ADDRESS);
	    ins_short(f);
	    ins_byte($4);	/* Number of actual arguments */
	    funp = FUNCTION(f);
	    if (strchr($1, ':')) {
		/*
		 * A function defined by inheritance. Find
		 * real definition immediately.
		 */
		find_inherited(funp);
	    }
	    /*
	     * Check if this function has been defined.
	     * But, don't complain yet about functions defined
	     * by inheritance.
	     */
	    if (exact_types && (funp->flags & NAME_UNDEFINED)) {
		char buff[100];
		sprintf(buff, "Undefined function %.50s", $1);
		yyerror(buff);
	    }
	    if (!(funp->flags & NAME_UNDEFINED))
		$$ = funp->type;
	    else
		$$ = TYPE_ANY;	/* Just a guess */
	}
	FREE($1);
	pop_arg_stack($4);	/* Argument types not needed more */
    }
| expr4 L_ARROW function_name
    {
	ins_f_byte(F_STRING);
	ins_short(store_prog_string($3));
	FREE($3);
    }
'(' expr_list ')'
    {
	ins_f_byte(F_CALL_OTHER);
	ins_byte($6 + 2);
	$$ = TYPE_UNKNOWN;
	pop_arg_stack($6);	/* No good need of these arguments */
    }
| '(' '*' comma_expr ')'
    {
		ins_f_byte(F_FUNCTION_SPLIT);
    }
     '(' expr_list ')'
    {
		ins_f_byte(F_CALL_OTHER);
		ins_byte($7 + 2);
		$$ = TYPE_UNKNOWN;
		pop_arg_stack($7);	/* No good need of these arguments */
    };

function_name: L_IDENTIFIER
	     | L_COLON_COLON L_IDENTIFIER
		{
		    char *p = DXALLOC(strlen($2) + 3, 54, "function_name: 1");
		    strcpy(p, "::"); strcat(p, $2); FREE($2);
		    $$ = p;
		}
	      | L_OBJECT L_COLON_COLON L_IDENTIFIER
		{
		    char *p;

			p = DXALLOC(strlen("object") + strlen($3) + 3, 55,
				"function_name: 2");
			strcpy(p, "object"); strcat(p, "::"); strcat(p, $3);
			FREE($3);
			$$ = p;
		}
	      | L_IDENTIFIER L_COLON_COLON L_IDENTIFIER
		{
		    char *p;
			struct svalue *res;
			extern struct object *master_ob;
			int invalid = 0;

			if (master_ob && (strcmp($1, "efun") == 0)) {
				push_malloced_string(the_file_name(current_file));
				push_constant_string($3);
				res = safe_apply_master_ob("valid_override", 2);
				if (IS_ZERO(res)) {
					yyerror("Invalid simulated efunction override");
					invalid = 1;
				}
			}
			if (invalid) {
				FREE($1);
				$$ = $3;
			} else {
				p = DXALLOC(strlen($1) + strlen($3) + 3, 55,
					"function_name: 2");
				strcpy(p, $1); strcat(p, "::"); strcat(p, $3);
				FREE($1); FREE($3);
				$$ = p;
			}
		};

cond: condStart
      statement
      optional_else_part
	{
		int i = pop_address();
#ifdef DEBUG
		/* check on 'i' handled by upd_short() */
		upd_short(i, CURRENT_PROGRAM_SIZE - i);
#else
		/* check on 'i' not handled by upd_short() */
		if (i <= CURRENT_PROGRAM_SIZE) {
			upd_short(i, CURRENT_PROGRAM_SIZE - i);
		} else {
			fprintf(stderr,
		"compiler.y: patch offset %x larger than current program size %x.\n",
		i, CURRENT_PROGRAM_SIZE);
		}
#endif
	};

condStart: L_IF '(' comma_expr ')'
	{
	    /* optimize if last expression did F_NOT */
	    if (last_expression == CURRENT_PROGRAM_SIZE-1 &&
		LAST_EXPR_CODE == F_NOT)
	    {
		SET_CURRENT_PROGRAM_SIZE(last_expression);
		ins_f_byte(F_BRANCH_WHEN_NON_ZERO);
	    } else {
		ins_f_byte(F_BRANCH_WHEN_ZERO);
	    }
	    push_address();
	    ins_short(0);
	} ;

optional_else_part:
         /* empty */
       | L_ELSE
        {
	    int i;
	    i = pop_address();
	    ins_f_byte(F_BRANCH); push_address(); ins_short(0);
	    upd_short(i, CURRENT_PROGRAM_SIZE - i);
        }
         statement
       ;
%%

void yyerror(str)
char *str;
{
    extern int num_parse_error;

    if (num_parse_error > 5)
	return;
    (void)fprintf(stderr, "%s: %s line %d\n", current_file, str,
		  current_line);
    fflush(stderr);
    smart_log(current_file, current_line, str,0);
    add_errors_for_file (current_file, 1);
    num_parse_error++;
}

static int check_declared(str)
    char *str;
{
	struct variable *vp;
	char *shared_string;
	int offset;

	/* if not in shared string table, then its not declared */
	if ((shared_string = findstring(str))) {
		/* find where its declared */
		for (offset=0; offset < mem_block[A_VARIABLES].current_size;
			offset += sizeof (struct variable))
		{
			vp = (struct variable *)&mem_block[A_VARIABLES].block[offset];
			if (vp->flags & NAME_HIDDEN)
				continue;
			/* Pointer comparison is possible since we compare against shared
			   string.
			*/
			if (vp->name == shared_string)
				return offset / sizeof (struct variable);
		}
	}
	return -1;
}

static int verify_declared(str)
    char *str;
{
    int r;

    r = check_declared(str);
    if (r < 0) {
	char buff[100];
        (void)sprintf(buff, "Variable %s not declared !", str);
        yyerror(buff);
	return -1;
    }
    return r;
}

void free_all_local_names()
{
    int i;

    for (i=0; i<current_number_of_locals; i++) {
	FREE(local_names[i]);
	local_names[i] = 0;
    }
    current_number_of_locals = 0;
    current_break_stack_need = 0;
    max_break_stack_need = 0;
}

void add_local_name(str, type)
    char *str;
    int type;
{
    if (current_number_of_locals == MAX_LOCAL)
	yyerror("Too many local variables");
    else {
	type_of_locals[current_number_of_locals] = type;
	local_names[current_number_of_locals++] = str;
    }
}

/*
 * Copy all function definitions from an inherited object. They are added
 * as undefined, so that they can be redefined by a local definition.
 * If they are not redefined, then they will be updated, so that they
 * point to the inherited definition. See epilog(). Types will be copied
 * at that moment (if available).
 *
 * A call to an inherited function will not be
 * done through this entry (because this entry can be replaced by a new
 * definition). If an function defined by inheritance is called, then one
 * special definition will be made at first call.
 */
static int copy_functions(from, type)
    struct program *from;
    int type;
{
    int i, initializer = -1;
    unsigned short tmp_short;

    for (i=0; (unsigned)i < from->p.i.num_functions; i++) {
	/* Do not call define_new_function() from here, as duplicates would
	 * be removed.
	 */
	struct function fun;
	int new_type;

	fun = from->p.i.functions[i];	/* Make a copy */
	/* Prepare some data to be used if this function will not be
	 * redefined.
	 */
	if (strchr(fun.name, ':'))
	    fun.flags |= NAME_HIDDEN;	/* Not to be used again ! */
	fun.name = make_shared_string(fun.name);	/* Incr ref count */
#ifdef PROFILE_FUNCTIONS
	fun.calls = 0L;
	fun.self = 0L;
	fun.children = 0L;
#endif
	fun.offset = mem_block[A_INHERITS].current_size /
	    sizeof (struct inherit) - 1;
	fun.function_index_offset = i;
	if (fun.type & TYPE_MOD_NO_MASK) {
	    int n;
	    if ((n = defined_function(fun.name)) != -1 &&
		!(((struct function *)mem_block[A_FUNCTIONS].block)[n].flags &
		  NAME_UNDEFINED))
	    {
		char p[2048];

		sprintf(p, "Illegal to redefine 'nomask' function \"%s\"",
			fun.name);
		yyerror(p);
	    }
	    fun.flags |= NAME_INHERITED;
	} else if (!(fun.flags & NAME_HIDDEN)) {
	    fun.flags |= NAME_UNDEFINED;
	}
	/*
	 * public functions should not become private when inherited
	 * 'private'
	 */
	new_type = type;
	if (fun.type & TYPE_MOD_PUBLIC)
	    new_type &= ~TYPE_MOD_PRIVATE;
	fun.type |= new_type;
	/* marion
	 * this should make possible to inherit a heart beat function, and
	 * thus to mask it if wanted.
	 */
	if (heart_beat == -1 && fun.name[0] == 'h' &&
	    strcmp(fun.name, "heart_beat") == 0)
	{
	    heart_beat = mem_block[A_FUNCTIONS].current_size /
		sizeof (struct function);
	} else if (fun.name[0] == '_' && strcmp(fun.name, "__INIT") == 0) {
	    initializer = i;
	    fun.flags |= NAME_INHERITED;
	}
	add_to_mem_block(A_FUNCTIONS, (char *)&fun, sizeof fun);
	/*
	 * Copy information about the types of the arguments, if it is
	 * available.
	 */
	tmp_short = INDEX_START_NONE;	/* Presume not available. */
	if (from->p.i.type_start != 0 && from->p.i.type_start[i] != INDEX_START_NONE)
	{
	    int arg;
	    /*
	     * They are available for function number 'i'. Copy types of
	     * all arguments, and remember where they started.
	     */
	    tmp_short = mem_block[A_ARGUMENT_TYPES].current_size /
		sizeof from->p.i.argument_types[0];
	    for (arg = 0; (unsigned)arg < fun.num_arg; arg++) {
		add_to_mem_block(A_ARGUMENT_TYPES,
				 &from->p.i.argument_types[from->p.i.type_start[i]],
				 sizeof (unsigned short));
	    }
	}
	/*
	 * Save the index where they started. Every function will have an
	 * index where the type info of arguments starts.
	 */
	add_to_mem_block(A_ARGUMENT_INDEX, &tmp_short, sizeof tmp_short);
    }
    return initializer;
}

/*
 * Copy all variabel names from the object that is inherited from.
 * It is very important that they are stored in the same order with the
 * same index.
 */
static void copy_variables(from, type)
    struct program *from;
    int type;
{
    int i;

    for (i=0; (unsigned)i<from->p.i.num_variables; i++) {
	int new_type = type;
	int n = check_declared(from->p.i.variable_names[i].name);

	if (n != -1 && (VARIABLE(n)->type & TYPE_MOD_NO_MASK)) {
	    char p[2048];

	    sprintf(p, "Illegal to redefine 'nomask' variable \"%s\"",
		    VARIABLE(n)->name);
	    yyerror(p);
	}
	/*
	 * 'public' variables should not become private when inherited
	 * 'private'.
	 */
	if (from->p.i.variable_names[i].type & TYPE_MOD_PUBLIC)
	    new_type &= ~TYPE_MOD_PRIVATE;
	define_variable(from->p.i.variable_names[i].name,
			from->p.i.variable_names[i].type | new_type,
			from->p.i.variable_names[i].type & TYPE_MOD_PRIVATE ?
			    NAME_HIDDEN : 0);
    }
}

/*
 * This function is called from lex.c for every new line read from the
 * "top" file (means not included files). Some new lines are missed,
 * as with #include statements, so it is compensated for.
 */
void store_line_number_info()
{
    unsigned short offset = mem_block[A_PROGRAM].current_size;

    while(mem_block[A_LINENUMBERS].current_size / sizeof (short) <
	  current_line)
    {
	add_to_mem_block(A_LINENUMBERS, (char *)&offset, sizeof offset);
    }
}

static char *get_type_name(type)
    int type;
{
    static char buff[100];
    static char *type_name[] = { "unknown", "void", "int", "string",
				   "object", "mapping", "function",
				   "float", "mixed"};
    int pointer = 0;

    buff[0] = 0;
    if (type & TYPE_MOD_STATIC)
	strcat(buff, "static ");
    if (type & TYPE_MOD_NO_MASK)
	strcat(buff, "nomask ");
    if (type & TYPE_MOD_PRIVATE)
	strcat(buff, "private ");
    if (type & TYPE_MOD_PROTECTED)
	strcat(buff, "protected ");
    if (type & TYPE_MOD_PUBLIC)
	strcat(buff, "public ");
    if (type & TYPE_MOD_VARARGS)
	strcat(buff, "varargs ");
    type &= TYPE_MOD_MASK;
    if (type & TYPE_MOD_POINTER) {
	pointer = 1;
	type &= ~TYPE_MOD_POINTER;
    }
    if (type >= sizeof type_name / sizeof type_name[0])
	fatal("Bad type\n");
    strcat(buff, type_name[type]);
    strcat(buff," ");
    if (pointer)
	strcat(buff, "* ");
    return buff;
}

void type_error(str, type)
    char *str;
    int type;
{
    static char buff[100];
    char *p;
    p = get_type_name(type);
    if (strlen(str) + strlen(p) + 5 >= sizeof buff) {
	yyerror(str);
    } else {
	strcpy(buff, str);
	strcat(buff, ": \"");
	strcat(buff, p);
	strcat(buff, "\"");
	yyerror(buff);
    }
}

/*
 * Compile an LPC file.
 */
void compile_file() {
    int yyparse();

    prolog();
    yyparse();
    epilog();
}

static char *get_two_types(type1, type2)
    int type1, type2;
{
    static char buff[100];

    strcpy(buff, "( ");
    strcat(buff, get_type_name(type1));
    strcat(buff, "vs ");
    strcat(buff, get_type_name(type2));
    strcat(buff, ")");
    return buff;
}

int get_id_number() {
    static int current_id_number = 1;
    return current_id_number++;
}

/*
 * The program has been compiled. Prepare a 'struct program' to be returned.
 */
void epilog() {
    int size, i;
    char *p;
    struct function *funp;
	void save_binary PROT((struct program *prog, struct mem_block *includes,
		struct mem_block *patches));

    if (num_parse_error > 0 || inherit_file) {
        clean_parser();
        return;
    }
    
#ifdef DEBUG
    if (type_of_arguments.current_size != 0)
	fatal("Failed to deallocate argument type stack\n");
#endif
    /*
     * Define the __INIT function, but only if there was any code
     * to initialize.
     */
    if (first_last_initializer_end != last_initializer_end) {
	define_new_function("__INIT", 0, 0, 0, 0, 0);
	/*
	 * Change the last jump after the last initializer into a
	 * return(1) statement.
	 */
	mem_block[A_PROGRAM].block[last_initializer_end-1] = F_CONST1;
	mem_block[A_PROGRAM].block[last_initializer_end-0] = F_RETURN;
    }

    /*
     * If functions are undefined, replace them by definitions done
     * by inheritance. All explicit "name::func" are already resolved.
     */
    for (i = 0; i < mem_block[A_FUNCTIONS].current_size; i += sizeof *funp) {
	funp = (struct function *)(mem_block[A_FUNCTIONS].block + i);
	   if (!(funp->flags & NAME_UNDEFINED))
	      continue;
       if (!find_in_table(funp, i)) { /* enforce proper M.I. semantics */
	       find_inherited(funp);
       }
    }

    size = align(sizeof (struct program));
    for (i=0; i<NUMPAREAS; i++)
	if (i != A_LINENUMBERS && i != A_ARGUMENT_TYPES && i != A_ARGUMENT_INDEX)
	    size += align(mem_block[i].current_size);
    p = (char *)DXALLOC(size, 56, "epilog: 1");
    prog = (struct program *)p;
    *prog = NULL_program;
    prog->p.i.total_size = size;
    prog->p.i.ref = 0;
    prog->p.i.heart_beat = heart_beat;
    prog->name = string_copy(current_file);
    prog->p.i.id_number = get_id_number();
    total_prog_block_size += prog->p.i.total_size;
    total_num_prog_blocks += 1;

    prog->p.i.line_swap_index = -1;
    prog->p.i.line_numbers = (unsigned short *)
	XALLOC(mem_block[A_LINENUMBERS].current_size + sizeof(short));
    memcpy(((char*)&prog->p.i.line_numbers[1]),
	   mem_block[A_LINENUMBERS].block,
	   mem_block[A_LINENUMBERS].current_size);
    prog->p.i.line_numbers[0] =
	(mem_block[A_LINENUMBERS].current_size / sizeof(short)) + 1;
    
    p += align(sizeof (struct program));
    prog->p.i.program = p;
    if (mem_block[A_PROGRAM].current_size)
	memcpy(p, mem_block[A_PROGRAM].block,
	       mem_block[A_PROGRAM].current_size);
    prog->p.i.program_size = mem_block[A_PROGRAM].current_size;

    p += align(mem_block[A_PROGRAM].current_size);
    prog->p.i.functions = (struct function *)p;
    prog->p.i.num_functions = mem_block[A_FUNCTIONS].current_size /
	sizeof (struct function);
    if (mem_block[A_FUNCTIONS].current_size)
	memcpy(p, mem_block[A_FUNCTIONS].block,
	       mem_block[A_FUNCTIONS].current_size);

    p += align(mem_block[A_FUNCTIONS].current_size);
    prog->p.i.strings = (char **)p;
    prog->p.i.num_strings = mem_block[A_STRINGS].current_size /
	sizeof (char *);
    if (mem_block[A_STRINGS].current_size)
	memcpy(p, mem_block[A_STRINGS].block,
	       mem_block[A_STRINGS].current_size);

    p += align(mem_block[A_STRINGS].current_size);
    prog->p.i.variable_names = (struct variable *)p;
    prog->p.i.num_variables = mem_block[A_VARIABLES].current_size /
	sizeof (struct variable);
    if (mem_block[A_VARIABLES].current_size)
	memcpy(p, mem_block[A_VARIABLES].block,
	       mem_block[A_VARIABLES].current_size);

    p += align(mem_block[A_VARIABLES].current_size);
    prog->p.i.num_inherited = mem_block[A_INHERITS].current_size /
	sizeof (struct inherit);
    if (prog->p.i.num_inherited) {
	memcpy(p, mem_block[A_INHERITS].block,
	       mem_block[A_INHERITS].current_size);
	prog->p.i.inherit = (struct inherit *)p;
    } else
	prog->p.i.inherit = 0;
    
    prog->p.i.argument_types = 0;	/* For now. Will be fixed someday */
    prog->p.i.type_start = 0;

#ifdef SAVE_BINARIES
#ifdef ALWAYS_SAVE_BINARIES
	save_binary(prog, &mem_block[A_INCLUDES], &mem_block[A_PATCH]);
#else
    if (pragma_save_binaries) {
	save_binary(prog, &mem_block[A_INCLUDES], &mem_block[A_PATCH]);
    }
#endif
#endif

    swap_line_numbers(prog); /* do this after saving binary */
    
    for (i=0; i<NUMAREAS; i++)
        FREE((char *)mem_block[i].block);

    /*  marion
	Do referencing here - avoid multiple referencing when an object
	inherits more than one object and one of the inherited is already
	loaded and not the last inherited
    */
    reference_prog (prog, "epilog");
    for (i = 0; (unsigned)i < prog->p.i.num_inherited; i++) {
	reference_prog (prog->p.i.inherit[i].prog, "inheritance");
    }
}

/*
 * Initialize the environment that the compiler needs.
 */
static void prolog() {
    int i;
	void free_expressions();

    switches = 0;
    switch_sptr = 0;
	last_expression = -1;
	free_expressions();
    if (type_of_arguments.block == 0) {
	type_of_arguments.max_size = 100;
	type_of_arguments.block =
		DXALLOC(type_of_arguments.max_size, 57, "prolog: 1");
    }
    type_of_arguments.current_size = 0;
    approved_object = 0;
    last_push_indexed = -1;
    last_push_local = -1;
    last_push_identifier = -1;
    prog = 0;		/* 0 means fail to load. */
    heart_beat = -1;
    comp_stackp = 0;	/* Local temp stack used by compiler */
    current_continue_address = 0;
    current_break_address = 0;
    num_parse_error = 0;
    free_all_local_names();	/* In case of earlier error */
    /* Initialize memory blocks where the result of the compilation
     * will be stored.
     */
    for (i=0; i < NUMAREAS; i++) {
	mem_block[i].block = DXALLOC(START_BLOCK_SIZE, 58, "prolog: 2");
	mem_block[i].current_size = 0;
	mem_block[i].max_size = START_BLOCK_SIZE;
    }
    memset(string_tags, 0, sizeof(string_tags));
    freed_string = -1;
    add_new_init_jump();
    first_last_initializer_end = last_initializer_end;
}

/*
 * The program has errors, clean things up.
 */
static void clean_parser() {
    int i;
    struct function *funp;
    struct variable dummy;
    char *s;
    
    /*
     * Free function stuff.
     */
    for (i = 0; i < mem_block[A_FUNCTIONS].current_size; i += sizeof *funp) {
	funp = (struct function *)(mem_block[A_FUNCTIONS].block + i);
	if (funp->name)
            free_string(funp->name);
    }
    for (i = 0; i < mem_block[A_STRINGS].current_size; i += sizeof(char *)) {
        ((char *)&s)[0] = mem_block[A_STRINGS].block[i + 0];
        ((char *)&s)[1] = mem_block[A_STRINGS].block[i + 1];
        ((char *)&s)[2] = mem_block[A_STRINGS].block[i + 2];
        ((char *)&s)[3] = mem_block[A_STRINGS].block[i + 3];
        free_string(s);
    }
    for (i = 0; i < mem_block[A_VARIABLES].current_size; i += sizeof dummy) {
        memcpy(&dummy, mem_block[A_VARIABLES].block + i, sizeof dummy);
        free_string(dummy.name);
    }
    
    prog = 0;
    for (i=0; i<NUMAREAS; i++)
        FREE(mem_block[i].block);
    if (num_parse_error)
	smart_log (NULL,0,NULL,1);
}

/*
 * Add a trailing jump after the last initialization code.
 */
void add_new_init_jump() {
    /*
     * Add a new jump.
     */
    ins_f_byte(F_JUMP);
    last_initializer_end = mem_block[A_PROGRAM].current_size;
    ins_short(0);
}

/*
   push_switches should be called at the beginning of the parsing of
   any loop construct.
*/

void
push_switches()
{
#ifdef DEBUG
    if (switch_sptr == SWITCH_STACK_SIZE) {
        fatal("switch_stack overflow\n");
    }
#endif
    switch_stack[switch_sptr++] = switches;
    switches = 0;
}

/*
   pop_switches should be called at the end of the parsing of any loop
   construct.
*/

void
pop_switches()
{
#ifdef DEBUG
    if (switch_sptr == 0) {
       fatal("switch_stack underflow\n");
    }
#endif
    switches = switch_stack[--switch_sptr];
}

void
push_expression(e)
expr_t *e;
{
#ifdef DEBUG
    if (expr_sptr == EXPR_STACK_SIZE) {
        fatal("expr_stack overflow\n");
    }
#endif
	memcpy(&expr_stack[expr_sptr++], e, sizeof(expr_t));
}

expr_t *
pop_expression()
{
#ifdef DEBUG
    if (expr_sptr == 0) {
       fatal("expr_stack underflow\n");
    }
#endif
	return &expr_stack[--expr_sptr];
}

/* this only necessary in case of errors in compiling LPC code */
void
free_expressions()
{
	int j;

	for (j = 0; j < expr_sptr; j++) {
		if (expr_stack[j].expr) {
			FREE(expr_stack[j].expr);
		}
	}
	expr_sptr = 0;
}

char *
the_file_name(name)
char *name;
{
	char *tmp;
	int len;

	len = strlen(name);
	if (len < 3) {
		return string_copy(name);
	}
	tmp = (char *)DXALLOC(len, 59, "the_file_name");
	if (!tmp) {
		return string_copy(name);
	}
	strcpy(tmp, "/");
	strncpy(tmp + 1, name, len - 2);
	tmp[len - 1] = '\0';
	return tmp;
}

static void
insert_pop_value()
{
	if (last_expression == CURRENT_PROGRAM_SIZE-1) {
		switch ( LAST_EXPR_CODE ) {
		case F_ASSIGN:
			mem_block[A_PROGRAM].block[last_expression] =
				F_VOID_ASSIGN;
			break;
		case F_ADD_EQ:
			mem_block[A_PROGRAM].block[last_expression] =
				F_VOID_ADD_EQ;
			break;
		case F_PRE_INC:
		case F_POST_INC:
			mem_block[A_PROGRAM].block[last_expression] =
				F_INC;
			break;
		case F_PRE_DEC:
		case F_POST_DEC:
			mem_block[A_PROGRAM].block[last_expression] =
				F_DEC;
			break;
		case F_CONST0:
		case F_CONST1:
			SET_CURRENT_PROGRAM_SIZE(last_expression);
			break;
		default: ins_f_byte(F_POP_VALUE);
			break;
		}
		last_expression = -1;
	} else {
		ins_f_byte(F_POP_VALUE);
	}
}

#ifdef LPC_OPTIMIZE_LOOPS
/* peephole optimization on the middle for_expr and while loop conditions. */

static int
optimize_loop_cond(e, len)
char *e;
int len;
{
	if ((len == 5) && (e[0] == F_LOCAL_NAME) && (e[2] == F_LOCAL_NAME)
		&& (e[4] == F_LT) && (e[5] == F_BBRANCH_WHEN_NON_ZERO))
	{
		e[0] = F_LOOP_COND;
		return 1;
	}
	if ((len == 8) && (e[0] == F_LOCAL_NAME) && (e[2] == F_NUMBER)
		&& (e[7] == F_LT) && (e[8] == F_BBRANCH_WHEN_NON_ZERO))
	{
		e[0] = F_LOOP_COND;
		return 1;
	}
	return 0;
}

/* peephole optimization on the middle for_expr and while loop conditions. */

static int
optimize_while_dec(e, len)
char *e;
int len;
{
	if ((len == 3) && (e[0] == F_PUSH_LOCAL_VARIABLE_LVALUE)
		&& (e[2] == F_POST_DEC) && (e[3] == F_BBRANCH_WHEN_NON_ZERO))
	{
		e[0] = F_WHILE_DEC;
		return 1;
	}
	return 0;
}
#endif

/*
 * Save a list of all the include files.  This is so we can
 * tell if a saved binary is out of date or not.  In the future,
 * may be used to give better line numbers for errors.
 */
void
save_include(name)
    char *name;
{
    /* at the moment, only used for binaries, so ifdef out */
#ifdef SAVE_BINARIES
    extern int pragma_save_binaries;

    if (pragma_save_binaries) {
	add_to_mem_block(A_INCLUDES, name, strlen(name)+1);
    }
#endif
}
