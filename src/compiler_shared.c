#include "compiler_shared.h"
#include "opcodes.h"
#include "trees.h"

#ifdef DEBUG
int dump_function_table() {
    int i;

    printf("FUNCTIONS:\n");
    printf("      name                          offset    fio  flags  # locals  # args\n");
    printf("      -----------                   ------    ---  -----  --------  ------\n");
    for (i = 0; i < mem_block[A_FUNCTIONS].current_size/sizeof(struct function); i++) {
	char sflags[7];
	int flags;
	struct function *funp;

	funp = (struct function *)(mem_block[A_FUNCTIONS].block + i * sizeof(struct function));
	flags = funp->flags;
	sflags[5] = '\0';
	sflags[0] = (flags & NAME_INHERITED) ? 'i' : '-';
	sflags[1] = (flags & NAME_UNDEFINED) ? 'u' : '-';
	sflags[2] = (flags & NAME_STRICT_TYPES) ? 's' : '-';
	sflags[3] = (flags & NAME_PROTOTYPE) ? 'p' : '-';
	sflags[4] = (flags & NAME_DEF_BY_INHERIT) ? 'd' : '-';
	printf("%4d: %-30s %5d  %5d  %5s  %8d  %6d\n",
	       i,
	       funp->name,
	       (int)funp->offset,
	       funp->function_index_offset,
	       sflags,
	       funp->num_local,
	       funp->num_arg
	       );
    }
    return 0;
}
#endif

struct mem_block mem_block[NUMAREAS];

#ifdef NEW_FUNCTIONS
struct function_context_t function_context;
#endif

int exact_types;
int approved_object;

int current_type;

int current_block;

struct program NULL_program;

struct program *prog;

static short string_idx[0x100];
unsigned char string_tags[0x20];
short freed_string;

unsigned short type_of_locals[MAX_LOCAL];
struct ident_hash_elem *locals[MAX_LOCAL];

int current_number_of_locals = 0;

int current_break_stack_need = 0, max_break_stack_need = 0;

#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
unsigned short a_functions_root = (unsigned short) 0xffff;
#endif

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

    for (i=0; i < current_number_of_locals; i++) {
	if (locals[i]->dn.local_num != -1)
	    locals[i]->sem_value--;
	locals[i]->dn.local_num = -1;
    }
    current_number_of_locals = 0;
    current_break_stack_need = 0;
    max_break_stack_need = 0;
}

/*
 * Copy all variable names from the object that is inherited from.
 * It is very important that they are stored in the same order with the
 * same index.
 */
void copy_variables P2(struct program *, from, int, type)
{
    int i;
    char tmp[2048];

    for (i = 0; (unsigned) i < from->p.i.num_variables; i++) {
	int new_type = type;
	struct ident_hash_elem *ihe;
	int n;

	/*
	 * 'public' variables should not become private when inherited
	 * 'private'.
	 */
	if (from->p.i.variable_names[i].type & TYPE_MOD_PUBLIC)
	    new_type &= ~TYPE_MOD_PRIVATE;

	n = (ihe = lookup_ident(from->p.i.variable_names[i].name))
	     ? ihe->dn.global_num : -1;

	if (n != -1) {
	    if (VARIABLE(n)->type & TYPE_MOD_NO_MASK) {
		sprintf(tmp, "Illegal to redefine 'nomask' variable \"%s\"",
			VARIABLE(n)->name);
		yyerror(tmp);
	    }
	}
	define_variable(from->p.i.variable_names[i].name,
			from->p.i.variable_names[i].type | new_type,
			from->p.i.variable_names[i].type & TYPE_MOD_PRIVATE);
    }
}

static void copy_function_details P2(struct function *, to, struct function *, from) {
    to->offset = from->offset;
    to->function_index_offset = from->function_index_offset;
    to->type = from->type;
    to->num_local = from->num_local;
    to->num_arg = from->num_arg;
    to->flags = (from->flags & NAME_MASK) | NAME_DEF_BY_INHERIT | NAME_UNDEFINED;
}

/* copy a function verbatim into this object, and possibly add it to the
   list of functions in this object, as well
 */
static struct function *copy_function P2(struct function *, new, int, add) {
    struct function *ret;
    int num = mem_block[A_FUNCTIONS].current_size/sizeof(struct function);
    struct ident_hash_elem *ihe;

    ret = (struct function *)allocate_in_mem_block(A_FUNCTIONS, sizeof(struct function));
    *ret = *new;
    ref_string(new->name); /* increment ref count */
    /* don't propagate certain flags forward */
    ret->flags &= NAME_MASK;
    if (!(new->flags & NAME_UNDEFINED))
	ret->flags |= NAME_DEF_BY_INHERIT;

    if (add) {
#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
	add_function((struct function *) mem_block[A_FUNCTIONS].block,
		     &a_functions_root, num);
#endif
	/* add the identifier */
	ihe = find_or_add_ident(new->name, 0);
	if (ihe->dn.function_num == -1)
	    ihe->sem_value++;
	ihe->dn.function_num = num;
    }
    return ret;
}

/* Overload the function index with the new definition */
static struct function *overload_function P2(int, index, struct function *, new) {
    struct function *funp, *alias;

    funp = (struct function *)mem_block[A_FUNCTIONS].block + index;
    /* Be careful with nomask; if both functions exists and either is nomask,
       we error.  */
    if (
	((funp->type & TYPE_MOD_NO_MASK) && (funp->flags & NAME_DEF_BY_INHERIT)
	&& (!(new->flags & NAME_NO_CODE)))
	||
	((new->type & TYPE_MOD_NO_MASK) && (!(funp->flags & NAME_NO_CODE ||
				    funp->flags & NAME_DEF_BY_INHERIT)))
	 ) {
	char buf[2048];
	sprintf(buf, "Illegal to redefine 'nomask' function \"%s\"",
		funp->name);
	yyerror(buf);
    }
    /* A new function also has to be inserted, since this spot will be
       used when this function is called in an object beneath us.  Point
       it at the overloaded function. */
    alias = copy_function(new, 0);
    /* Ick! copy_functions calls allocate_in_mem_block(), so funp might
       be dangling now.  Be safe and find it again. */
    funp = (struct function *)mem_block[A_FUNCTIONS].block + index;
    alias->flags = NAME_ALIAS;
    /* offset to the real def */
    alias->offset = alias - funp;

    /* The rule here is that the latest function wins, so if it's not
       defined at this level and defined in the new object, we copy it in */
    if ((funp->flags & NAME_UNDEFINED) && (!(new->flags & NAME_NO_CODE)))
	copy_function_details(funp, new);
    else
	funp = 0; /* we aren't overloading */

    return funp;
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
	struct function *funp;
	int new_type;
	struct ident_hash_elem *ihe;

	if (from->p.i.functions[i].flags & NAME_COLON_COLON) { 
	    /* we need to propagate this entry up the inherit tree
	       b/c of the way overloading in implemented */
	    funp = copy_function(&from->p.i.functions[i], 0);
	    funp->flags |= NAME_INHERITED;
	} else {
	    int num;
	    
	    ihe = lookup_ident(from->p.i.functions[i].name);
	    if (ihe && ((num = ihe->dn.function_num)!=-1)) {
		/* The function has already been defined in this object */
		funp = overload_function(num, &from->p.i.functions[i]);
	    } else {
		funp = copy_function(&from->p.i.functions[i], 1);
		/* the function hasn't been defined at this level yet */
		funp->flags |= NAME_UNDEFINED;
	    }
	}

	if (funp) {
	    /* point the new function entry at the one in the inherited file,
	       in case it's not overloaded and this becomes a real function */
	    funp->offset = mem_block[A_INHERITS].current_size / sizeof(struct inherit) - 1;
	    funp->function_index_offset = i;

#ifdef PROFILE_FUNCTIONS
	    funp->calls = 0L;
	    funp->self = 0L;
	    funp->children = 0L;
#endif
	    
	    /*
	     * public functions should not become private when inherited
	     * 'private'
	     */
	    new_type = type;
	    if (funp->type & TYPE_MOD_PUBLIC)
		new_type &= ~TYPE_MOD_PRIVATE;
	    funp->type |= new_type;

	    if (strcmp(funp->name, APPLY___INIT) == 0)
		initializer = i;
	}

	/* Beek - some of this stuff below belongs above where we decide
	 * whether to copy the function or not.  But this code doesn't
	 * do anything yet anyway.
	 * shouldn't this depend on save_types anyway? Anyway, future
	 * work.
	 */
	    
	/*
	 * Copy information about the types of the arguments, if it is
	 * available.
	 */
	tmp_short = INDEX_START_NONE;	/* Presume not available. */
	if (from->p.i.type_start != 0 && from->p.i.type_start[i] != INDEX_START_NONE) {
	    int arg;
	    
	    /*
	     * They are available for function number 'i'. Copy types of
	     * all arguments, and remember where they started.
	     */
	    tmp_short = mem_block[A_ARGUMENT_TYPES].current_size /
		sizeof from->p.i.argument_types[0];
	    for (arg = 0; (unsigned) arg < from->p.i.functions[i].num_arg; arg++) {
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

/* The only version of the if effectively always was true */
#ifndef OLD_TYPE_BEHAVIOR
    if ((t1 == TYPE_NUMBER || t1 == TYPE_REAL) && (t2 == TYPE_NUMBER || t2 == TYPE_REAL))
#endif
	return 1;
    if (t1 == TYPE_ANY || t2 == TYPE_ANY)
	return 1;
    if ((t1 & TYPE_MOD_POINTER) && (t2 & TYPE_MOD_POINTER)) {
	return ((t1 & TYPE_MOD_MASK) == (TYPE_ANY | TYPE_MOD_POINTER) ||
	    (t2 & TYPE_MOD_MASK) == (TYPE_ANY | TYPE_MOD_POINTER));
    }
    return 0;
}

/*
 * Patch a function definition of an inherited function, to what it really
 * should be.
 * The name of the function can be one of:
 *    object::name
 *    ::name
 * Where 'object' is the name of the superclass.
 *
 * Note: this function is now only used for resolving :: references
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
			     & NAME_UNDEFINED)) {
#else
	    for (i = 0; (unsigned) i < ip->prog->p.i.num_functions; i++) {
		if (ip->prog->p.i.functions[i].flags & NAME_UNDEFINED)
		    continue;
		/* can use pointer compare because both are shared */
		if (ip->prog->p.i.functions[i].name != shared_string)
		    continue;
#endif
		funp->offset = ip - (struct inherit *) mem_block[A_INHERITS].block;
		funp->flags = NAME_INHERITED | NAME_COLON_COLON |
		    (ip->prog->p.i.functions[i].flags & NAME_MASK);
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
    struct ident_hash_elem *ihe;

    num = (ihe = lookup_ident(name)) ? ihe->dn.function_num : -1;
    if (num >= 0) {
	struct function *funp;

	/*
	 * The function was already defined. It may be one of several
	 * reasons:
	 * 
	 * 1.	There has been a prototype.
	 * 2.	There was the same function defined by inheritance.
	 * 3.	This function has been called, but not yet defined.
	 * 4.	The function is doubly defined.
	 * 5.	A "late" prototype has been encountered.
	 */
	funp = (struct function *) (mem_block[A_FUNCTIONS].block) + num;
	if (!(funp->flags & NAME_UNDEFINED) &&
	    !(flags & NAME_PROTOTYPE)) {
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
	    else if (!(funp->flags & NAME_STRICT_TYPES) && !(flags & NAME_PROTOTYPE))
		yyerror("Called function not compiled with type testing.");
	    else {
		/* Now check that argument types wasn't changed. */
		for (i = 0; i < num_arg; i++) {
		    ;		/* FIXME? */
		}
	    }
	}
	/* If it was yet another prototype, then simply return. */
	if (flags & NAME_PROTOTYPE) {
	    return num;
	}
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
    ihe = find_or_add_ident(fun.name, 0);
    if (ihe->dn.function_num == -1)
	ihe->sem_value++;
    ihe->dn.function_num = num;
    return num;
}

int define_variable P3(char *, name, int, type, int, hide)
{
    struct variable *dummy;
    int n;
    char *str;
    struct ident_hash_elem *ihe;

    str = make_shared_string(name);

    n = (mem_block[A_VARIABLES].current_size / sizeof(struct variable));

    ihe = find_or_add_ident(str, 0);
    if (ihe->dn.global_num == -1)
	ihe->sem_value++;
    else {
	if (VARIABLE(ihe->dn.global_num)->type & TYPE_MOD_NO_MASK) {
	    char p[2048];
	    
	    sprintf(p, "Illegal to redefine 'nomask' variable \"%s\"", name);
	    yyerror(p);
	}
	/* *sigh* make the old version static */
	VARIABLE(ihe->dn.global_num)->type |= TYPE_MOD_STATIC;
    }
    ihe->dn.global_num = n;

    dummy = (struct variable *)allocate_in_mem_block(A_VARIABLES,sizeof(struct variable));
    dummy->name = str;
    dummy->type = type;

    if (hide) dummy->type |= TYPE_MOD_HIDDEN;

    return n;
}

char *get_type_name P1(int, type)
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

int validate_function_call P3(struct function *, funp, int, f, struct parse_node *, args)
{
    int num_arg = ( args ? args->kind : 0 );

    /*
     * Verify that the function has been defined already.
     */
    if ((funp->flags & NAME_UNDEFINED) &&
	!(funp->flags & (NAME_PROTOTYPE | NAME_DEF_BY_INHERIT)) && exact_types) {
	char buff[256];

	sprintf(buff, "Function %.50s undefined", funp->name);
	yyerror(buff);
    }
    /*
     * Check number of arguments.
     */
    if (funp->num_arg != num_arg && !(funp->type & TYPE_MOD_VARARGS) &&
	(funp->flags & NAME_STRICT_TYPES) && exact_types) {
	char buff[100];

	sprintf(buff, "Wrong number of arguments to %.60s\n    Expected: %d  Got: %d", funp->name, funp->num_arg, num_arg);
	yyerror(buff);
    }
    /*
     * Check the argument types.
     */
    if (exact_types && *(unsigned short *)
	&mem_block[A_ARGUMENT_INDEX].block[f * sizeof(unsigned short)]
	!= INDEX_START_NONE) {
	int i, first, tmp;
	unsigned short *arg_types;
	struct parse_node *enode = args;

	arg_types = (unsigned short *) mem_block[A_ARGUMENT_TYPES].block;
	first = *(unsigned short *) &mem_block[A_ARGUMENT_INDEX].block[f * sizeof(unsigned short)];
	for (i = 0; (unsigned) i < funp->num_arg && i < num_arg; i++) {
	    tmp = enode->v.expr->type;

	    if (!TYPE(tmp, arg_types[first + i])) {
		char buff[100];

		sprintf(buff, "Bad type for argument %d %s", i + 1,
			get_two_types(arg_types[first + i], tmp));
		yyerror(buff);
	    }
	    enode = enode->right;
	}
    }
    return funp->type & TYPE_MOD_MASK;
}

struct parse_node *
validate_efun_call P2(int, f, struct parse_node *, args) {
    int num = args->v.number;
    int min_arg, max_arg, def, *argp;
    
    if (f != 0) {
	/* should this move out of here? */
	switch (predefs[f].token) {
	case F_SIZEOF:
	    if (num == 1) {
		if (args->right->v.expr->kind == F_AGGREGATE) {
		    num = args->right->v.expr->v.number;
		    args = make_node(F_NUMBER, E_CONST, (num ? TYPE_NUMBER : TYPE_ANY));
		    args->v.number = num;
		    return args;
		}
	    }
	}

	args->flags = 0;
	args->type = predefs[f].ret_type;
	min_arg = predefs[f].min_args;
	max_arg = predefs[f].max_args;

	def = predefs[f].Default;
	if (def && num == min_arg -1) {
	    struct parse_node *tmp;
	    tmp = make_branched_node(0, 0, 0, 0);
	    args->left->right = tmp;
	    if (def > 0) {
		tmp->v.expr = make_branched_node(def, 0, 0, 0);
		tmp->v.expr->v.number = -1;
	    } else {
		tmp->v.expr = make_branched_node((-def) >> 8, 0, 0, 0);
		tmp->v.expr->v.number = (-def) & 0xff;
	    }
	    max_arg--;
	    min_arg--;
	} else if (num < min_arg) {
	    char bff[100];
	    sprintf(bff, "Too few arguments to %s", predefs[f].word);
	    yyerror(bff);
	} else if (num > max_arg && max_arg != -1) {
	    char bff[100];
	    sprintf(bff, "Too many arguments to %s", predefs[f].word);
	    yyerror(bff);
	} else if (max_arg != -1 && exact_types) {
	    /*
	     * Now check all types of arguments to efuns.
	     */
	    int i, argn, tmp;
	    char buff[100];
	    struct parse_node *enode = args;
	    argp = &efun_arg_types[predefs[f].arg_index];
	    
	    for (argn = 0; argn < num; argn++) {
		enode = enode->right;
		tmp = enode->v.expr->type;
		for (i=0; !compatible_types(argp[i], tmp) && argp[i] != 0; i++)
		    ;
		if (argp[i] == 0) {
		    sprintf(buff, "Bad argument %d to efun %s()",
			    argn+1, predefs[f].word);
		    yyerror(buff);
		}
		while (argp[i] != 0)
		    i++;
		argp += i + 1;
	    }
	}
	args->kind = predefs[f].token;
	/* Only store number of arguments for instructions that allow
	 * a variable number.
	 */
	if (max_arg == min_arg)
	    args->v.number = -1;
    } else {
	args = make_node(F_CONST0, 0, TYPE_ANY);
    }
    return args;
}
	 
