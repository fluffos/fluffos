#include "compiler_shared.h"
#include "opcodes.h"

struct mem_block mem_block[NUMAREAS];

int exact_types;
int approved_object;

int current_type;
int heart_beat;

int current_block;

struct program NULL_program;

struct program *prog;

static short string_idx[0x100];
unsigned char string_tags[0x20];
short freed_string;

char *local_names[MAX_LOCAL];
unsigned short type_of_locals[MAX_LOCAL];
int current_number_of_locals = 0;

int current_break_stack_need = 0, max_break_stack_need = 0;

#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
unsigned short a_functions_root = (unsigned short) 0xffff;

#endif

int comp_stackp;
static int comp_stack[COMPILER_STACK_SIZE];

/*
 * The types of arguments when calling functions must be saved,
 * to be used afterwards for checking. And because function calls
 * can be done as an argument to a function calls,
 * a stack of argument types is needed. This stack does not need to
 * be freed between compilations, but will be reused.
 */
struct mem_block type_of_arguments;

/*
 * Prototypes for local functions
 */
static int check_declared PROT((char *));
static char *get_type_name PROT((int));

static int check_declared P1(char *, str)
{
    struct variable *vp;
    char *shared_string;
    int offset;

    /* if not in shared string table, then its not declared */
    if ((shared_string = findstring(str))) {
	/* find where its declared */
	for (offset = 0; offset < mem_block[A_VARIABLES].current_size;
	     offset += sizeof(struct variable)) {
	    vp = (struct variable *) & mem_block[A_VARIABLES].block[offset];
	    if (vp->flags & NAME_HIDDEN)
		continue;
	    /*
	     * Pointer comparison is possible since we compare against shared
	     * string.
	     */
	    if (vp->name == shared_string)
		return (int) (offset / sizeof(struct variable));
	}
    }
    return -1;
}

char *get_two_types P2(int, type1, int, type2)
{
    static char buff[100];

    strcpy(buff, "( ");
    strcat(buff, get_type_name(type1));
    strcat(buff, "vs ");
    strcat(buff, get_type_name(type2));
    strcat(buff, ")");
    return buff;
}

void free_all_local_names()
{
    int i;

    for (i = 0; i < current_number_of_locals; i++) {
	FREE(local_names[i]);
	local_names[i] = 0;
    }
    current_number_of_locals = 0;
    current_break_stack_need = 0;
    max_break_stack_need = 0;
}

int verify_declared P1(char *, str)
{
    int r;

    r = check_declared(str);
    if (r < 0) {
	char buff[100];

	(void) sprintf(buff, "Variable %s not declared !", str);
	yyerror(buff);
	return -1;
    }
    return r;
}

/*
 * Copy all variabel names from the object that is inherited from.
 * It is very important that they are stored in the same order with the
 * same index.
 */
void copy_variables P2(struct program *, from, int, type)
{
    int i;

    for (i = 0; (unsigned) i < from->p.i.num_variables; i++) {
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
int copy_functions P2(struct program *, from, int, type)
{
    int i, initializer = -1;
    unsigned short tmp_short;

    for (i = 0; (unsigned) i < from->p.i.num_functions; i++) {
	/*
	 * Do not call define_new_function() from here, as duplicates would
	 * be removed.
	 */
	struct function fun;
	int new_type;

	fun = from->p.i.functions[i];	/* Make a copy */
	/*
	 * Prepare some data to be used if this function will not be
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
	    sizeof(struct inherit) - 1;
	fun.function_index_offset = i;
	if (fun.type & TYPE_MOD_NO_MASK) {
	    int n;

	    if ((n = defined_function(fun.name)) != -1 &&
	     !(((struct function *) mem_block[A_FUNCTIONS].block)[n].flags &
	       NAME_UNDEFINED)) {
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
	/*
	 * marion this should make possible to inherit a heart beat function,
	 * and thus to mask it if wanted.
	 */
	if (heart_beat == -1 && fun.name[0] == 'h' &&
	    strcmp(fun.name, "heart_beat") == 0) {
	    heart_beat = mem_block[A_FUNCTIONS].current_size /
		sizeof(struct function);
	} else if (fun.name[0] == '_' && strcmp(fun.name, "__INIT") == 0) {
	    initializer = i;
	    fun.flags |= NAME_INHERITED;
	}
	add_to_mem_block(A_FUNCTIONS, (char *) &fun, sizeof fun);
#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
	add_function((struct function *) mem_block[A_FUNCTIONS].block,
		   &a_functions_root, (mem_block[A_FUNCTIONS].current_size /
				       sizeof(struct function)) - 1);
#endif

	/*
	 * Copy information about the types of the arguments, if it is
	 * available.
	 */
	tmp_short = INDEX_START_NONE;	/* Presume not available. */
	if (from->p.i.type_start != 0 && from->p.i.type_start[i] != INDEX_START_NONE) {
	    int arg;

	    /*
	     * They are available for function number 'i'. Copy types of all
	     * arguments, and remember where they started.
	     */
	    tmp_short = mem_block[A_ARGUMENT_TYPES].current_size /
		sizeof from->p.i.argument_types[0];
	    for (arg = 0; (unsigned) arg < fun.num_arg; arg++) {
		add_to_mem_block(A_ARGUMENT_TYPES,
		(char *) &from->p.i.argument_types[from->p.i.type_start[i]],
				 sizeof(unsigned short));
	    }
	}
	/*
	 * Save the index where they started. Every function will have an
	 * index where the type info of arguments starts.
	 */
	add_to_mem_block(A_ARGUMENT_INDEX, (char *) &tmp_short, sizeof tmp_short);
    }
    return initializer;
}

void type_error P2(char *, str, int, type)
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
 * Compare two types, and return true if they are compatible.
 */

int compatible_types P2(int, t1, int, t2)
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
	if ((t1 & TYPE_MOD_MASK) == (TYPE_ANY | TYPE_MOD_POINTER) ||
	    (t2 & TYPE_MOD_MASK) == (TYPE_ANY | TYPE_MOD_POINTER))
	    return 1;
    }
    return 0;
}

/*
 * Add another argument type to the argument type stack
 */
void add_arg_type P1(unsigned short, type)
{
    struct mem_block *mbp = &type_of_arguments;

    while (mbp->current_size + sizeof type > mbp->max_size) {
	mbp->max_size <<= 1;
	mbp->block = (char *) DREALLOC((char *) mbp->block, mbp->max_size, 48,
				       "add_arg_type");
    }
    memcpy(mbp->block + mbp->current_size, (char *) &type, sizeof type);
    mbp->current_size += sizeof type;
}

/*
 * Return the index of the function found, otherwise -1.
 */
int defined_function P1(char *, s)
{
    int offset;
    char *shared_string;
    struct function *funp;

    /* if not in shared string table then it hasn't been defined */
    if ((shared_string = findstring(s))) {
#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
	offset = lookup_function((struct function *)
	     mem_block[A_FUNCTIONS].block, a_functions_root, shared_string);
	if (offset != -1) {
	    funp = (struct function *) & mem_block[A_FUNCTIONS].block[offset * sizeof(struct function)];
	    if (!(funp->flags & NAME_HIDDEN))
		return offset;
	}
#else
	for (offset = 0; offset < mem_block[A_FUNCTIONS].current_size;
	     offset += sizeof(struct function)) {
	    funp = (struct function *) & mem_block[A_FUNCTIONS].block[offset];
	    if (funp->flags & NAME_HIDDEN)
		continue;
	    /* can do pointer compare instead of strcmp since is shared */
	    if (funp->name == shared_string)
		return (int) (offset / sizeof(struct function));
	}
#endif
    }
    return -1;
}

void push_address()
{
    if (comp_stackp >= COMPILER_STACK_SIZE) {
	yyerror("Compiler stack overflow");
	comp_stackp++;
	return;
    }
    comp_stack[comp_stackp++] = mem_block[current_block].current_size;
}

void push_explicit P1(int, address)
{
    if (comp_stackp >= COMPILER_STACK_SIZE) {
	yyerror("Compiler stack overflow");
	comp_stackp++;
	return;
    }
    comp_stack[comp_stackp++] = address;
}

int pop_address()
{
    if (comp_stackp == 0)
	fatal("Compiler stack underflow.\n");
    if (comp_stackp > COMPILER_STACK_SIZE) {
	--comp_stackp;
	return 0;
    }
    return comp_stack[--comp_stackp];
}

int
find_in_table P2(struct function *, funp, int, cutoff)
{
    int i;
    struct function *tfunp;

#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
    i = lookup_function((struct function *) mem_block[A_FUNCTIONS].block,
			a_functions_root, funp->name);
    if (i != -1) {
	tfunp = (struct function *) (mem_block[A_FUNCTIONS].block + i * sizeof(struct function));
	if (!(tfunp->flags & NAME_UNDEFINED)) {
	    *funp = *tfunp;
	    return 1;
	}
    }
#else
    for (i = 0; i < cutoff; i += sizeof(struct function)) {
	tfunp = (struct function *) (mem_block[A_FUNCTIONS].block + i);
	/* function names are shared strings */
	if ((tfunp->name == funp->name) && !(tfunp->flags & NAME_UNDEFINED)) {
	    *funp = *tfunp;
	    return 1;
	}
    }
#endif
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
void find_inherited P1(struct function *, funp)
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
	real_name = p + 2;
	super_name = funp->name;
	super_length = real_name - super_name - 2;
    }
    num_inherits = mem_block[A_INHERITS].current_size /
	sizeof(struct inherit);
    /* no need to look for it unless its in the shared string table */
    if ((shared_string = findstring(real_name))) {
	ip = (struct inherit *) mem_block[A_INHERITS].block;
	for (; num_inherits > 0; ip++, num_inherits--) {
	    if (super_name) {
		int l = strlen(ip->prog->name);	/* Including .c */

		if (l - 2 < super_length)
		    continue;
		if (strncmp(super_name, ip->prog->name + l - 2 - super_length,
			    super_length) != 0)
		    continue;
	    }
#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
	    i = lookup_function(ip->prog->p.i.functions,
				ip->prog->p.i.tree_r, shared_string);
	    if (i != -1 && !(ip->prog->p.i.functions[i].flags
			     & (NAME_UNDEFINED | NAME_HIDDEN))) {
#else
	    for (i = 0; (unsigned) i < ip->prog->p.i.num_functions; i++) {
		if (ip->prog->p.i.functions[i].flags & (NAME_UNDEFINED | NAME_HIDDEN))
		    continue;
		/* can use pointer compare because both are shared */
		if (ip->prog->p.i.functions[i].name != shared_string)
		    continue;
#endif
		funp->offset = ip - (struct inherit *) mem_block[A_INHERITS].block;
		funp->flags = ip->prog->p.i.functions[i].flags | NAME_INHERITED;
		funp->num_local = ip->prog->p.i.functions[i].num_local;
		funp->num_arg = ip->prog->p.i.functions[i].num_arg;
		funp->type = ip->prog->p.i.functions[i].type;
		funp->function_index_offset = i;
		return;
	    }
	}
    }				/* if in shared string table */
}

/*
 * Define a new function. Note that this function is called at least twice
 * for all function definitions. First as a prototype, then as the real
 * function. Thus, there are tests to avoid generating error messages more
 * than once by looking at (flags & NAME_PROTOTYPE).
 */
int define_new_function P6(char *, name, int, num_arg, int, num_local,
                           int, offset, int, flags, int, type)
{
    int num;
    struct function fun;
    unsigned short argument_start_index;

    num = defined_function(name);
    if (num >= 0) {
	struct function *funp;

	/*
	 * The function was already defined. It may be one of several
	 * reasons:
	 * 
	 * 1.	There has been a prototype. 2.	There was the same function
	 * defined by inheritance. 3.	This function has been called, but
	 * not yet defined. 4.	The function is doubly defined. 5.	A
	 * "late" prototype has been encountered.
	 */
	funp = (struct function *) (mem_block[A_FUNCTIONS].block) + num;
	if (!(funp->flags & NAME_UNDEFINED) &&
	    !(flags & NAME_PROTOTYPE) &&
	    !(funp->flags & NAME_INHERITED)) {
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
	 * Check arguments only when types are supposed to be tested, and if
	 * this function really has been defined already.
	 * 
	 * 'nomask' functions may not be redefined.
	 */
	if ((funp->type & TYPE_MOD_NO_MASK) &&
	    !(funp->flags & NAME_PROTOTYPE) &&
	    !(flags & NAME_PROTOTYPE)) {
	    char p[2048];

	    sprintf(p, "Illegal to redefine 'nomask' function \"%s\"", name);
	    yyerror(p);
	}
	if (exact_types && funp->type != TYPE_UNKNOWN) {
	    int i;

	    if (funp->num_arg != num_arg && !(funp->type & TYPE_MOD_VARARGS)
		&& !(flags & NAME_PROTOTYPE))
		yyerror("Incorrect number of arguments.");
	    else if (!(funp->flags & NAME_STRICT_TYPES))
		yyerror("Called function not compiled with type testing.");
	    else {
		/* Now check that argument types wasn't changed. */
		for (i = 0; i < num_arg; i++) {
		    ;		/* FIXME? */
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
	    sizeof(struct function);
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
    add_to_mem_block(A_FUNCTIONS, (char *) &fun, sizeof fun);
#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
    add_function((struct function *) mem_block[A_FUNCTIONS].block,
		 &a_functions_root, num);
#endif

    if (exact_types == 0 || num_arg == 0) {
	argument_start_index = INDEX_START_NONE;
    } else {
	int i;

	/*
	 * Save the start of argument types.
	 */
	argument_start_index =
	    mem_block[A_ARGUMENT_TYPES].current_size /
	    sizeof(unsigned short);
	for (i = 0; i < num_arg; i++) {
	    add_to_mem_block(A_ARGUMENT_TYPES, (char *) &type_of_locals[i],
			     sizeof type_of_locals[i]);
	}
    }
    add_to_mem_block(A_ARGUMENT_INDEX, (char *) &argument_start_index,
		     sizeof argument_start_index);
    return num;
}

void define_variable P3(char *, name, int, type, int, flags)
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
    add_to_mem_block(A_VARIABLES, (char *) &dummy, sizeof dummy);
}

static char *get_type_name P1(int, type)
{
    static char buff[100];
    static char *type_name[] =
    {"unknown", "void", "int", "string",
     "object", "mapping", "function",
     "float", "buffer", "mixed"};
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
    strcat(buff, " ");
    if (pointer)
	strcat(buff, "* ");
    return buff;
}

#define STRING_HASH(var,str) \
    var = (long)str ^ (long)str >> 16; \
    var = (var ^ var >> 8) & 0xff;

short store_prog_string P1(char *, str)
{
    short i, next, *next_tab, *idxp;
    char **p;
    unsigned char hash, mask, *tagp;

    str = make_shared_string(str);
    STRING_HASH(hash, str);
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
		free_string(str);	/* needed as string is only free'ed
					 * once. */
		((short *) mem_block[A_STRING_REFS].block)[i]++;
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
	/* reuse freed string */
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
    ((char **) mem_block[A_STRINGS].block)[i] = str;
    ((short *) mem_block[A_STRING_NEXT].block)[i] = next;
    ((short *) mem_block[A_STRING_REFS].block)[i] = 1;
    *idxp = i;
    return i;
}

void free_prog_string P1(short, num)
{
    short i, prv, *next_tab, top, *idxp;
    char **p, *str;
    unsigned char hash, mask;

    top = mem_block[A_STRINGS].current_size / sizeof(char *) - 1;
    if (num < 0 || num > top) {
	yyerror("free_prog_string: index out of range.\n");
	return;
    }
    if (--((short *) mem_block[A_STRING_REFS].block)[num] >= 1)
	return;

    p = (char **) mem_block[A_STRINGS].block;
    next_tab = (short *) mem_block[A_STRING_NEXT].block;

    str = p[num];
    STRING_HASH(hash, str);
    idxp = &string_idx[hash];

    for (prv = -1, i = *idxp; i != num; prv = i, i = next_tab[i]) {
	if (i == -1) {
	    yyerror("free_prog_string: string not in prog table.\n");
	    return;
	}
    }

    if (prv == -1) {		/* string is head of list */
	*idxp = next_tab[i];
	if (*idxp == -1) {
	    /* clear tag bit since hash chain now empty */
	    mask = 1 << (hash & 7);
	    string_tags[hash >> 3] &= ~mask;
	}
    } else {			/* easy unlink */
	next_tab[prv] = next_tab[i];
    }

    free_string(str);		/* important */
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

int validate_function_call P3(struct function *, funp, int, f, int, num_arg)
{
    if (funp->flags & NAME_UNDEFINED)
	find_inherited(funp);
    /*
     * Verify that the function has been defined already.
     */
    if ((funp->flags & NAME_UNDEFINED) &&
	!(funp->flags & NAME_PROTOTYPE) && exact_types) {
	char buff[100];

	sprintf(buff, "Function %.50s undefined", funp->name);
	yyerror(buff);
    }
    /*
     * Check number of arguments.
     */
    if (funp->num_arg != num_arg && !(funp->type & TYPE_MOD_VARARGS) &&
	(funp->flags & NAME_STRICT_TYPES) && exact_types) {
	char buff[100];

	sprintf(buff, "Wrong number of arguments to %.60s", funp->name);
	yyerror(buff);
    }
    /*
     * Check the argument types.
     */
    if (exact_types && *(unsigned short *)
	&mem_block[A_ARGUMENT_INDEX].block[f * sizeof(unsigned short)]
	!= INDEX_START_NONE) {
	int i, first;
	unsigned short *arg_types;

	arg_types = (unsigned short *) mem_block[A_ARGUMENT_TYPES].block;
	first = *(unsigned short *) &mem_block[A_ARGUMENT_INDEX].block[f * sizeof(unsigned short)];
	for (i = 0; (unsigned) i < funp->num_arg && i < num_arg; i++) {
	    int tmp = get_argument_type(i, num_arg);

	    if (!TYPE(tmp, arg_types[first + i])) {
		char buff[100];

		sprintf(buff, "Bad type for argument %d %s", i + 1,
			get_two_types(arg_types[first + i], tmp));
		yyerror(buff);
	    }
	}
    }
    return funp->type & TYPE_MOD_MASK;
}
