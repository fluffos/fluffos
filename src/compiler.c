#include "std.h"
#include "lpc_incl.h"
#include "compiler.h"
#include "trees.h"
#include "lex.h"
#include "functab_tree.h"
#include "generate.h"
#include "swap.h"
#include "scratchpad.h"
#include "qsort.h"
#include "file.h"
#include "binaries.h"
/* This should be moved with initializers move out of here */
#include "icode.h"

static void clean_parser PROT((void));
static void prolog PROT((int, char *));
static void epilog PROT((void));
static void show_overload_warnings PROT((void));

#define CT(x) (1 << (x))
#define CT_SIMPLE(x) (CT(TYPE_ANY) | CT(x))

short compatible[11] = { 
    /* UNKNOWN */  0,
    /* ANY */      0xfff,
    /* NOVALUE */  CT_SIMPLE(TYPE_NOVALUE) | CT(TYPE_VOID) | CT(TYPE_NUMBER),
    /* VOID */     CT_SIMPLE(TYPE_VOID) | CT(TYPE_NOVALUE),
    /* NUMBER */   CT_SIMPLE(TYPE_NUMBER) | CT(TYPE_NOVALUE) | CT(TYPE_REAL),
    /* STRING */   CT_SIMPLE(TYPE_STRING),
    /* OBJECT */   CT_SIMPLE(TYPE_OBJECT),
    /* MAPPING */  CT_SIMPLE(TYPE_MAPPING),
    /* FUNCTION */ CT_SIMPLE(TYPE_FUNCTION),
    /* REAL */     CT_SIMPLE(TYPE_REAL) | CT(TYPE_NUMBER),
    /* BUFFER */   CT_SIMPLE(TYPE_BUFFER),
	       };

short is_type[11] = { 
    /* UNKNOWN */ 0,
    /* ANY */     0xfff,
    /* NOVALUE */ CT_SIMPLE(TYPE_NOVALUE) | CT(TYPE_VOID),
    /* VOID */    CT_SIMPLE(TYPE_VOID) | CT(TYPE_NOVALUE),
    /* NUMBER */  CT_SIMPLE(TYPE_NUMBER),
    /* STRING */  CT_SIMPLE(TYPE_STRING),
    /* OBJECT */  CT_SIMPLE(TYPE_OBJECT),
    /* MAPPING */ CT_SIMPLE(TYPE_MAPPING),
    /* FUNCTION */CT_SIMPLE(TYPE_FUNCTION),
    /* REAL */    CT_SIMPLE(TYPE_REAL),
    /* BUFFER */  CT_SIMPLE(TYPE_BUFFER),
	      };

mem_block_t mem_block[NUMAREAS];

function_context_t function_context;

int exact_types;

int current_type;

int current_block;
char *prog_code;
char *prog_code_max;

program_t NULL_program;

program_t *prog;

static short string_idx[0x100];
unsigned char string_tags[0x20];
short freed_string;

unsigned short *type_of_locals;
ident_hash_elem_t **locals;
char *runtime_locals;

unsigned short *type_of_locals_ptr;
ident_hash_elem_t **locals_ptr;
char *runtime_locals_ptr;

int locals_size = 0;
int type_of_locals_size = 0; 
int current_number_of_locals = 0;
int max_num_locals = 0;

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

void init_locals()
{
    type_of_locals = CALLOCATE(MAX_LOCAL,unsigned short, 
			       TAG_LOCALS, "init_locals:1");
    locals = CALLOCATE(MAX_LOCAL, ident_hash_elem_t *, 
		       TAG_LOCALS, "init_locals:2");
    runtime_locals = CALLOCATE(MAX_LOCAL, char, 
			       TAG_LOCALS, "init_locals:3");
    type_of_locals_ptr = type_of_locals;
    locals_ptr = locals;
    runtime_locals_ptr = runtime_locals;
    locals_size = type_of_locals_size = MAX_LOCAL;
    current_number_of_locals = max_num_locals = 0;
}

void free_all_local_names()
{
    int i;

    for (i=0; i < current_number_of_locals; i++) {
	locals_ptr[i]->sem_value--;
	locals_ptr[i]->dn.local_num = -1;
    }
    current_number_of_locals = 0;
    max_num_locals = 0;
}

void deactivate_current_locals(){
    int i;

    for (i = 0; i < current_number_of_locals; i++){
	runtime_locals_ptr[i] = locals_ptr[i]->dn.local_num;
	locals_ptr[i]->dn.local_num = -1;
    }
}

void reactivate_current_locals(){
    int i;

    for (i = 0; i < current_number_of_locals; i++){
	locals_ptr[i]->dn.local_num = runtime_locals_ptr[i];
	locals_ptr[i]->sem_value++;
    }
}

void clean_up_locals()
{
    int offset;

    offset = locals_ptr + current_number_of_locals - locals;
    while (offset--){
	locals[offset]->sem_value--;
	locals[offset]->dn.local_num = -1;
    }
    current_number_of_locals = 0;
    max_num_locals = 0;
    locals_ptr = locals;
    type_of_locals_ptr = type_of_locals;
    runtime_locals_ptr = runtime_locals;
}

void pop_n_locals P1(int, num) {
    while (num--) {
	locals_ptr[--current_number_of_locals]->sem_value--;
	locals_ptr[current_number_of_locals]->dn.local_num = -1;
    }
}

int add_local_name P2(char *, str, int, type)
{
    if (max_num_locals == MAX_LOCAL) {
	yyerror("Too many local variables");
	return 0;
    } else {
	ident_hash_elem_t *ihe;

	ihe = find_or_add_ident(str,FOA_NEEDS_MALLOC);
	type_of_locals_ptr[max_num_locals] = type;
	locals_ptr[current_number_of_locals++] = ihe;
	if (ihe->dn.local_num == -1)
	    ihe->sem_value++;
	return ihe->dn.local_num = max_num_locals++;
    }
}

void reallocate_locals(){
    int offset;
    offset = type_of_locals_ptr - type_of_locals;
    type_of_locals = RESIZE(type_of_locals, type_of_locals_size += MAX_LOCAL,
			    unsigned short, TAG_LOCALS, "reallocate_locals:1");
    type_of_locals_ptr = type_of_locals + offset;
    offset = locals_ptr - locals;
    locals = RESIZE(locals, locals_size, 
		    ident_hash_elem_t *, TAG_LOCALS, "reallocate_locals:2");
    locals_ptr = locals + offset;
    runtime_locals = RESIZE(runtime_locals, locals_size, char,
			    TAG_LOCALS, "reallocate_locals:3");
    runtime_locals_ptr = runtime_locals + offset;
}

/*
 * Copy all variable names from the object that is inherited from.
 * It is very important that they are stored in the same order with the
 * same index.
 */
void copy_variables P2(program_t *, from, int, type)
{
    int i, numvars = from->num_variables, n;
    variable_t *from_vars = from->variable_names;
    ident_hash_elem_t *ihe;
    int new_type;

    for (i = 0; (unsigned) i < numvars; i++) {
        new_type = type;

        /*
         * 'public' variables should not become private when inherited
         * 'private'.
         */
        if (from_vars[i].type & TYPE_MOD_PUBLIC)
            new_type &= ~TYPE_MOD_PRIVATE;

        if ((ihe = lookup_ident(from->variable_names[i].name)) &&
	    ((n = ihe->dn.global_num) != -1)) {
            char tmp[2048];

            if (VARIABLE(n)->type & TYPE_MOD_NO_MASK) {
                sprintf(tmp, "Illegal to redefine 'nomask' variable \"%s\"",
                        VARIABLE(n)->name);
                yyerror(tmp);
	    }
	}
        define_variable(from->variable_names[i].name,
                        from->variable_names[i].type | new_type,
                        from->variable_names[i].type & TYPE_MOD_PRIVATE);
     } 
}



static void copy_function_details P2(function_t *, to, function_t *, from) {
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
static function_t *copy_function P2(function_t *, new, int, add) {
    function_t *ret;
    int num = mem_block[A_FUNCTIONS].current_size/sizeof(function_t);
    ident_hash_elem_t *ihe;

    ret = (function_t *)allocate_in_mem_block(A_FUNCTIONS, sizeof(function_t));
    *ret = *new;
    ref_string(new->name); /* increment ref count */
    /* don't propagate certain flags forward */
    ret->flags &= NAME_MASK;
    if (!(new->flags & NAME_UNDEFINED))
	ret->flags |= NAME_DEF_BY_INHERIT;

    if (add) {
#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
	add_function((function_t *) mem_block[A_FUNCTIONS].block,
		     &a_functions_root, num);
#endif
	/* add the identifier */
	ihe = find_or_add_ident(new->name, FOA_GLOBAL_SCOPE);
	if (ihe->dn.function_num == -1)
	    ihe->sem_value++;
	ihe->dn.function_num = num;
    }
    return ret;
}

void copy_structures P1(program_t *, prog) {
    class_def_t *sd;
    class_member_entry_t *sme;
    ident_hash_elem_t *ihe;
    char *str;
    int sm_off = mem_block[A_CLASS_MEMBER].current_size / sizeof(class_member_entry_t);
    int sd_off = mem_block[A_CLASS_DEF].current_size / sizeof(class_def_t);
    int i, num = 0;

    sd = (class_def_t *)allocate_in_mem_block(A_CLASS_DEF, prog->num_classes * sizeof(class_def_t));
    for (i = 0; i < prog->num_classes; i++) {
	sd[i].size = prog->classes[i].size;
	num += sd[i].size;
	sd[i].index = prog->classes[i].index + sm_off;
	str = prog->strings[prog->classes[i].name];
	sd[i].name = store_prog_string(str);
	ihe = find_or_add_ident(str, FOA_GLOBAL_SCOPE);
	if (ihe->dn.class_num == -1)
	    ihe->sem_value++;
	ihe->dn.class_num = i + sd_off;
    }

    sme = (class_member_entry_t *)allocate_in_mem_block(A_CLASS_MEMBER, sizeof(class_member_entry_t) * num);
    while (num--) {
	sme[num].type = prog->class_members[num].type;
	sme[num].name = store_prog_string(prog->strings[prog->class_members[num].name]);
    }
}

typedef struct ovlwarn_s {
    struct ovlwarn_s *next;
    char *func;
    char *warn;
} ovlwarn_t;

ovlwarn_t *overload_warnings = 0;

static void remove_overload_warnings P1(char *, func) {
    ovlwarn_t **p;
    ovlwarn_t *tmp;

    p = &overload_warnings;
    while (*p) {
	if (!func || (*p)->func == func) {
	    FREE((*p)->warn);
	    tmp = *p;
	    *p = (*p)->next;
	    FREE(tmp);
	} else
	    p = &(*p)->next;
    }
}

static void show_overload_warnings() {
    ovlwarn_t *p, *next;
    p = overload_warnings;
    while (p) {
	yywarn(p->warn);
	FREE(p->warn);
	next = p->next;
	FREE(p);
	p = next;
    }
    overload_warnings = 0;
}

/* Overload the function index with the new definition */
static function_t *overload_function P3(int, index, program_t *, prog, int, newindex) {
    function_t *funp, *alias;
    function_t *new;

    new = &prog->functions[newindex];
    funp = (function_t *)mem_block[A_FUNCTIONS].block + index;
    /* Be careful with nomask; if both functions exists and either is nomask,
       we error.  */
    if (!(new->flags & NAME_NO_CODE) && REAL_FUNCTION(funp)
        && ((new->type & TYPE_MOD_NO_MASK) || (funp->type & TYPE_MOD_NO_MASK))
       ) {
	char buf[2048];
	sprintf(buf, "Illegal to redefine 'nomask' function \"%s\"",
		funp->name);
	yyerror(buf);
    }
    /* Try to prevent some confusion re: overloading.
     * Warn them about the behavior of inheriting the same function
     * from two branches.
     * 
     * Note that we don't want to scream now, b/c if the function is 
     * overloaded later this becomes irrelevant.
     *
     * Note also that this is real spammy if you inherit the same object
     * twice.  Something should be done about that.
     */
    if ((pragmas & PRAGMA_WARNINGS) 
	&& REAL_FUNCTION(funp) && !(new->flags & NAME_NO_CODE)) {
	/* don't scream if one is private.  Why not?  Because I said so.
	 * private is pretty screwed up anyway.  In the future there
	 * won't be such a clash b/c private won't come up the tree.
	 * This also give the coder a way to shut the compiler up when
	 * you do inherit the same object twice in different branches :)
	 */
	if (!(funp->type & TYPE_MOD_PRIVATE) && !(new->type & TYPE_MOD_PRIVATE)) {
	    char buf[1024];
	    char *from1 ;
	    ovlwarn_t *ow;
	    
	    from1 = ((inherit_t *) mem_block[A_INHERITS].block + funp->offset)->prog->name;
	    sprintf(buf, "%s() inherited from both %s and %s; using the definition in %s.", funp->name, from1, prog->name, prog->name);
	
	    ow = ALLOCATE(ovlwarn_t, TAG_COMPILER, "overload warning");
	    ow->next = overload_warnings;
	    ow->func = funp->name;
	    ow->warn = alloc_cstring(buf, "overload warning");
	    overload_warnings = ow;
	}
    }

    /* A new function also has to be inserted, since this spot will be
       used when this function is called in an object beneath us.  Point
       it at the overloaded function. */
    alias = copy_function(new, 0);
    /* Ick! copy_functions calls allocate_in_mem_block(), so funp might
       be dangling now.  Be safe and find it again. */
    funp = (function_t *)mem_block[A_FUNCTIONS].block + index;
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
int copy_functions P2(program_t *, from, int, type)
{
    int i, initializer = -1, num_functions = from->num_functions;
    unsigned short tmp_short;
    function_t *from_funcs = from->functions;
    function_t *funp;
    int new_type;
    ident_hash_elem_t *ihe;
    int num;


    if (num_functions && (*from_funcs[num_functions-1].name == APPLY___INIT_SPECIAL_CHAR)){
        initializer = --num_functions;
    }
	
    for (i = 0; i < num_functions; i++) {
	    
	ihe = lookup_ident(from_funcs[i].name);
	if (ihe && ((num = ihe->dn.function_num)!=-1)) {
	  /* The function has already been defined in this object */
	  funp = overload_function(num, from, i);
	} else {
	  funp = copy_function(&from_funcs[i], 1);
	  /* the function hasn't been defined at this level yet */
	  funp->flags |= NAME_UNDEFINED;
	}

	if (funp) {
	    /* point the new function entry at the one in the inherited file,
	       in case it's not overloaded and this becomes a real function */
	    funp->offset = mem_block[A_INHERITS].current_size / sizeof(inherit_t) - 1;
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
	if (exact_types && from->type_start != 0 && from->type_start[i] != INDEX_START_NONE) {
	    /*
	     * They are available for function number 'i'. Copy types of
	     * all arguments, and remember where they started.
	     */
	    tmp_short = mem_block[A_ARGUMENT_TYPES].current_size /
		sizeof from->argument_types[0];
	    add_to_mem_block(A_ARGUMENT_TYPES, 
			     (char *) &from->argument_types[from->type_start[i]],
			     sizeof(unsigned short) * from->functions[i].num_arg);
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
#ifdef OLD_TYPE_BEHAVIOR
    /* The old version effectively was almost always was true */
    return 1;
#else
    t1 &= TYPE_MOD_MASK;
    t2 &= TYPE_MOD_MASK;
    if (t1 == TYPE_ANY || t2 == TYPE_ANY) return 1;
    if ((t1 == (TYPE_ANY | TYPE_MOD_ARRAY) && (t2 & TYPE_MOD_ARRAY)))
	return 1;
    if ((t2 == (TYPE_ANY | TYPE_MOD_ARRAY) && (t1 & TYPE_MOD_ARRAY)))
	return 1;
    if (t1 & TYPE_MOD_CLASS)
	return t1 == t2;
    if (t1 & TYPE_MOD_ARRAY) {
	if (!(t2 & TYPE_MOD_ARRAY)) return 0;
	return t1 == (TYPE_MOD_ARRAY | TYPE_ANY) ||
	       t2 == (TYPE_MOD_ARRAY | TYPE_ANY) || (t1 == t2);
    } else if (t2 & TYPE_MOD_ARRAY)
	return 0;
    return compatible[t1] & (1 << t2);
#endif
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
void arrange_call_inherited P2(char *, name, parse_node_t *, node)
{
    int i;
    inherit_t *ip;
    int num_inherits, super_length = 0;
    char *super_name = 0, *p, *real_name = name;
    char *shared_string;

    if (real_name[0] == ':')
	real_name += 2; 	/* There will be exactly two ':' */
    else if ((p = strchr(real_name, ':'))) {
        super_name = name;
        real_name = p+2;
        super_length = real_name - super_name - 2;
    }
    num_inherits = mem_block[A_INHERITS].current_size /
	sizeof(inherit_t);
    /* no need to look for it unless its in the shared string table */
    if ((shared_string = findstring(real_name))) {
	ip = (inherit_t *) mem_block[A_INHERITS].block;
	for (; num_inherits > 0; ip++, num_inherits--) {
	    if (super_name) {
		int l = strlen(ip->prog->name);	/* Including .c */

		if (l - 2 < super_length)
		    continue;
		if (strncmp(super_name, ip->prog->name + l - 2 -super_length,
			    super_length) != 0 ||
                    !((l - 2 == super_length) ||
                     ((ip->prog->name + l - 3 - super_length)[0] == '/')))
                    continue;
	    }
#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
	    i = lookup_function(ip->prog->functions,
				ip->prog->tree_r, shared_string);
	    if (i != -1 && !(ip->prog->functions[i].flags
			     & NAME_UNDEFINED)) {
#else
	    for (i = 0; (unsigned) i < ip->prog->num_functions; i++) {
		if (ip->prog->functions[i].flags & NAME_UNDEFINED)
		    continue;
		    /* can use pointer compare because both are shared */
		if (ip->prog->functions[i].name != shared_string)
		    continue;
#endif
		node->kind = NODE_CALL_2;
		node->v.number = F_CALL_INHERITED;
		node->l.number = i + ((ip - (inherit_t *) mem_block[A_INHERITS].block) << 16);
		node->type = ip->prog->functions[i].type;
		return;
	    }
	}
    }				/* if in shared string table */
    {
	char buff[MAXLINE + 30];
	sprintf(buff, "No such inherited function %.50s", name);
	yyerror(buff);
	node->kind = NODE_CALL_2;
	node->v.number = F_CALL_INHERITED;
	node->l.number = 0;
	node->type = TYPE_ANY;
    }
}

/*
 * Define a new function. Note that this function is called at least twice
 * for all function definitions. First as a prototype, then as the real
 * function. Thus, there are tests to avoid generating error messages more
 * than once by looking at (flags & NAME_PROTOTYPE).
 */
int define_new_function P5(char *, name, int, num_arg, int, num_local,
                           int, flags, int, type)
{
    int num;
    function_t fun;
    unsigned short argument_start_index;
    ident_hash_elem_t *ihe;

    num = (ihe = lookup_ident(name)) ? ihe->dn.function_num : -1;
    if (num >= 0) {
	function_t *funp;

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
	funp = (function_t *) (mem_block[A_FUNCTIONS].block) + num;
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
	/* only check prototypes for matching.  It shouldn't be required that
	   overloading a function must have the same signature */
	if (exact_types && (funp->flags & NAME_PROTOTYPE) && funp->type != TYPE_UNKNOWN) {
	    int i;

	    /* This should be changed to catch two prototypes which disagree */
	    if (funp->num_arg != num_arg && !(funp->type & TYPE_MOD_VARARGS)
		&& !(flags & NAME_PROTOTYPE))
		yyerror("Number of arguments disagrees with previous definition.");
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

	if (pragmas & PRAGMA_WARNINGS)
	    remove_overload_warnings(funp->name);
	
	funp->num_arg = num_arg;
	funp->num_local = num_local;
	funp->flags = flags;
	funp->offset = 0;
	funp->function_index_offset = 0;
	funp->type = type;
	if (exact_types)
	    funp->flags |= NAME_STRICT_TYPES;
	return num;
    }
    fun.name = make_shared_string(name);
    fun.offset = 0;
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
    add_function((function_t *) mem_block[A_FUNCTIONS].block,
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
	    add_to_mem_block(A_ARGUMENT_TYPES, 
			     (char *) &type_of_locals_ptr[i],
			     sizeof type_of_locals_ptr[i]);
	}
    }
    add_to_mem_block(A_ARGUMENT_INDEX, (char *) &argument_start_index,
		     sizeof argument_start_index);
    ihe = find_or_add_ident(fun.name, FOA_GLOBAL_SCOPE);
    if (ihe->dn.function_num == -1)
	ihe->sem_value++;
    ihe->dn.function_num = num;
    return num;
}

int define_variable P3(char *, name, int, type, int, hide)
{
    variable_t *dummy;
    int n;
    char *str;
    ident_hash_elem_t *ihe;

    str = make_shared_string(name);

    n = (mem_block[A_VARIABLES].current_size / sizeof(variable_t));

    ihe = find_or_add_ident(str, FOA_GLOBAL_SCOPE);
    if (ihe->dn.global_num == -1) {
	ihe->sem_value++;
	ihe->dn.global_num = n;
    } else {
	if (VARIABLE(ihe->dn.global_num)->type & TYPE_MOD_NO_MASK) {
	    char p[2048];
	    
	    sprintf(p, "Illegal to redefine 'nomask' variable \"%s\"", name);
	    yyerror(p);
	}
	/* Okay, the nasty idiots have two variables of the same name in
	   the same object.  This causes headaches for save_object().
	   To keep save_object sane, we need to make one static;
	   Also, be careful not to hide the current variable with a
	   hidden one. */
	if (!(type & TYPE_MOD_STATIC)) {
	    /* this one isn't static, make the other one static */
	    VARIABLE(ihe->dn.global_num)->type |= TYPE_MOD_STATIC;
	}
	if (!hide)
	    ihe->dn.global_num = n;
    }
    ihe->dn.global_num = n;

    dummy = (variable_t *)allocate_in_mem_block(A_VARIABLES,sizeof(variable_t));
    dummy->name = str;
    dummy->type = type;

    if (hide) dummy->type |= TYPE_MOD_HIDDEN;

    return n;
}

char *compiler_type_names[] = {"unknown", "mixed", "void", "void", 
		               "int", "string", "object", "mapping",
		               "function", "float", "buffer" };

char *get_type_name P1(int, type)
{
    static char buff[100];
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
    if (type & TYPE_MOD_ARRAY) {
	pointer = 1;
	type &= ~TYPE_MOD_ARRAY;
    }
    if (type & TYPE_MOD_CLASS) {
	strcat(buff, "class ");
	strcat(buff, PROG_STRING(CLASS(type & ~TYPE_MOD_CLASS)->name));
    } else {
	DEBUG_CHECK(type >= sizeof compiler_type_names / sizeof compiler_type_names[0], "Bad type\n");
	strcat(buff, compiler_type_names[type]);
    }
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

    p = (char **)&PROG_STRING(0);
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
    PROG_STRING(i) = str;
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

int validate_function_call P3(function_t *, funp, int, f, parse_node_t *, args)
{
    int num_arg = ( args ? args->kind : 0 );
    int num_var = 0;
    parse_node_t *pn = args;
    
    while (pn) {
	if (pn->type & 1) num_var++;
	pn = pn->r.expr;
    }
	
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
    if (!(funp->type & TYPE_MOD_VARARGS) &&
	(funp->flags & NAME_STRICT_TYPES) &&
	exact_types) {
	char buff[100];

	if (num_var) {
	    sprintf(buff, "Illegal to pass a variable number of arguments to non-varargs function %.60s\n", funp->name);
	    yyerror(buff);
	} else
	if (funp->num_arg != num_arg) {
	    sprintf(buff, "Wrong number of arguments to %.60s\n    Expected: %d  Got: %d", funp->name, funp->num_arg, num_arg);
	    yyerror(buff);
	}
    }
    /*
     * Check the argument types.
     */
    if (exact_types && *(unsigned short *)
	&mem_block[A_ARGUMENT_INDEX].block[f * sizeof(unsigned short)]
	!= INDEX_START_NONE) {
	int i, first, tmp;
	unsigned short *arg_types;
	parse_node_t *enode = args;

	arg_types = (unsigned short *) mem_block[A_ARGUMENT_TYPES].block;
	first = *(unsigned short *) &mem_block[A_ARGUMENT_INDEX].block[f * sizeof(unsigned short)];
	for (i = 0; (unsigned) i < funp->num_arg && i < num_arg; i++) {
	    if (enode->type & 1) break;
	    tmp = enode->v.expr->type;

	    if (!compatible_types(tmp, arg_types[first + i])) {
		char buff[100];

		sprintf(buff, "Bad type for argument %d %s", i + 1,
			get_two_types(arg_types[first + i], tmp));
		yyerror(buff);
	    }
	    enode = enode->r.expr;
	}
    }
    return funp->type & TYPE_MOD_MASK;
}

parse_node_t *
promote_to_float P1(parse_node_t *, node) {
    parse_node_t *expr;
    if (node->kind == NODE_NUMBER) {
	node->kind = NODE_REAL;
	node->v.real = node->v.number;
	return node;
    }
    expr = new_node();
    expr->kind = NODE_EFUN;
    expr->v.number = F_TO_FLOAT;
    expr->type = TYPE_REAL;
    expr->l.number = 1;
    expr->r.expr = new_node_no_line();
    expr->r.expr->kind = 1;
    expr->r.expr->l.expr = expr->r.expr;
    expr->r.expr->type = 0;
    expr->r.expr->v.expr = node;
    expr->r.expr->r.expr = 0;
    return expr;
}

parse_node_t *
promote_to_int P1(parse_node_t *, node) {
    parse_node_t *expr;
    if (node->kind == NODE_REAL) {
	node->kind = NODE_NUMBER;
	node->v.number = node->v.real;
	return node;
    }
    expr = new_node();
    expr->kind = NODE_EFUN;
    expr->v.number = F_TO_INT;
    expr->type = TYPE_NUMBER;
    expr->l.number = 1;
    expr->r.expr = new_node_no_line();
    expr->r.expr->kind = 1;
    expr->r.expr->l.expr = expr->r.expr;
    expr->r.expr->type = 0;
    expr->r.expr->v.expr = node;
    expr->r.expr->r.expr = 0;
    return expr;
}

parse_node_t *do_promotions P2(parse_node_t *, node, int, type) {
    if (type == TYPE_REAL) {
	if (node->type == TYPE_NUMBER || node->kind == NODE_NUMBER)
	    return promote_to_float(node);
    }
    if (type == TYPE_NUMBER && node->type == TYPE_REAL)
	return promote_to_int(node);
    return node;
}

parse_node_t *
validate_efun_call P2(int, f, parse_node_t *, args) {
    int num = args->v.number;
    int min_arg, max_arg, def, *argp;
    int num_var = 0;
    parse_node_t *pn = args->r.expr;
    
    while (pn) {
	if (pn->type & 1) num_var++;
	pn = pn->r.expr;
    }
    
    if (f != -1) {
	/* should this move out of here? */
	switch (predefs[f].token) {
	case F_SIZEOF:
	    if (!pn && num == 1 && 
		IS_NODE(args->r.expr->v.expr, NODE_CALL, F_AGGREGATE)) {
		num = args->r.expr->v.expr->l.number;
		CREATE_NUMBER(args, num);
		return args;
	    }
	}

	args->type = predefs[f].ret_type;
	min_arg = predefs[f].min_args;
	max_arg = predefs[f].max_args;

	def = predefs[f].Default;
	if (!num_var && def != DEFAULT_NONE && num == min_arg -1) {
	    parse_node_t *tmp;
	    tmp = new_node_no_line();
	    tmp->r.expr = 0;
	    tmp->type = 0;
	    args->l.expr->r.expr = tmp;
	    if (def == DEFAULT_THIS_OBJECT) {
		tmp->v.expr = new_node_no_line();
		tmp->v.expr->kind = NODE_EFUN;
		tmp->v.expr->v.number = F_THIS_OBJECT;
		tmp->v.expr->l.number = 0;
		tmp->v.expr->type = TYPE_ANY;
		tmp->v.expr->r.expr = 0;
	    } else {
		CREATE_NUMBER(tmp->v.expr, def);
	    }
	    args->v.number++;
	    num++;
	} else if (num_var && max_arg != -1) {
	    char bff[100];
	    sprintf(bff, "Illegal to pass variable number of arguments to non-varargs efun %s", predefs[f].word);
	    yyerror(bff);
	    return args;
	} else if ((num - num_var) < min_arg) {
	    char bff[100];
	    sprintf(bff, "Too few arguments to %s", predefs[f].word);
	    yyerror(bff);
	    return args;
	} else if (num > max_arg && max_arg != -1) {
	    char bff[100];
	    sprintf(bff, "Too many arguments to %s", predefs[f].word);
	    yyerror(bff);
	    return args;
	}
	if (max_arg != -1 && exact_types) {
	    /*
	     * Now check all types of arguments to efuns.
	     */
	    int i, argn, tmp;
	    char buff[100];
	    parse_node_t *enode = args;
	    argp = &efun_arg_types[predefs[f].arg_index];
	    
	    for (argn = 0; argn < num; argn++) {
		enode = enode->r.expr;
		if (enode->type & 1) break;
		/* this can happen for default args */
		if (!enode->v.expr) break;
		tmp = enode->v.expr->type;
		for (i=0; !compatible_types(argp[i], tmp) && argp[i] != 0; i++)
		    ;

		if (argp[i] == 0) {
		    sprintf(buff, "Bad argument %d to efun %s()",
			    argn+1, predefs[f].word);
		    yyerror(buff);
		} else {
		    /* check for (int) -> (float) promotion */
		    if (tmp == TYPE_NUMBER && argp[i] == TYPE_REAL) {
			for (i++; argp[i] && argp[i] != TYPE_NUMBER; i++)
			    ;
			if (!argp[i])
			    enode->v.expr = promote_to_float(enode->v.expr);
		    }
		    else if (tmp == TYPE_REAL && argp[i] == TYPE_NUMBER) {
			for (i++; argp[i] && argp[i] != TYPE_REAL; i++)
			    ;
			if (!argp[i])
			    enode->v.expr = promote_to_int(enode->v.expr);
		    }
		}
		while (argp[i] != 0)
		    i++;
		argp += i + 1;
	    }
	}
	args->l.number = num;
	args->v.number = predefs[f].token;
	args->kind = NODE_EFUN;
    } else {
	CREATE_ERROR(args);
    }
    return args;
}

/*
 * Initialization code is now more compact.  It's collected in
 * A_INITIALIZER and put at the end of the program.  For compatibility,
 * there is a jump to it at address 0.
 */
void switch_to_block P1(int, block) {
    UPDATE_PROGRAM_SIZE;

    prog_code = mem_block[block].block + mem_block[block].current_size;
    prog_code_max = mem_block[block].block + mem_block[block].max_size;
    current_block = block;
}

void yyerror P1(char *, str)
{
    extern int num_parse_error;

    function_context.num_parameters = -1;
    if (num_parse_error > 5)
	return;
    smart_log(current_file, current_line, str, 0);
#ifdef PACKAGE_MUDLIB_STATS
    add_errors_for_file (current_file, 1);
#endif
    num_parse_error++;
}

void yywarn P1(char *, str) {
    if (!(pragmas & PRAGMA_WARNINGS)) return;
    
    smart_log(current_file, current_line, str, 1);
}

/*
 * Compile an LPC file.
 */
void compile_file P2(int, f, char *, name) {
    int yyparse PROT((void));

    prolog(f, name);
    yyparse();
    epilog();
}

int get_id_number() {
    static int current_id_number = 1;
    return current_id_number++;
}

INLINE void copy_in P2(int, which, char **, start) {
    char *block;
    int size;

    size = mem_block[which].current_size;
    if (!size) return;

    block = mem_block[which].block;
    memcpy(*start, block, size);

    *start += align(size);
}

/*
 * The program has been compiled. Prepare a 'program_t' to be returned.
 */
static void epilog() {
    int size, i, lnsz, lnoff;
    char *p;
    function_t *funp;
    ident_hash_elem_t *ihe;

    /* don't need the parse trees any more */
    release_tree();
    
    if (num_parse_error > 0 || inherit_file) {
	/* don't print these; they can be wrong, since we didn't parse the
	   entire file */
	if (pragmas & PRAGMA_WARNINGS)
	    remove_overload_warnings(0);
	clean_parser();
	end_new_file();
	free_string(current_file);
	current_file = 0;
	return;
    }

    if (pragmas & PRAGMA_WARNINGS)
	show_overload_warnings();
    
    /*
     * Define the __INIT function, but only if there was any code
     * to initialize.
     */
    UPDATE_PROGRAM_SIZE;

    if (mem_block[A_INITIALIZER].current_size) {
	parse_node_t *pn;
	int fun;
	/* end the __INIT function */
	switch_to_block(A_INITIALIZER);
	CREATE_RETURN(pn, 0);
	generate(pn);
	switch_to_block(A_PROGRAM);
 	fun = define_new_function(APPLY___INIT, 0, 0, 
			    NAME_STRICT_TYPES, TYPE_VOID | TYPE_MOD_PRIVATE);
	FUNCTION(fun)->offset = CURRENT_PROGRAM_SIZE;
	generate___INIT();
    }

    generate_final_program(0);
    UPDATE_PROGRAM_SIZE;

    /*
     * If functions are undefined, replace them by definitions done
     * by inheritance. All explicit "name::func" are already resolved.
     * Also replace aliases with the updated info.  We use aliases
     * so that only one function definition has to be fooled with for
     * overloading during compile time, but here we want to make sure
     * they're all the same again.
     */
    for (i = 0; i < mem_block[A_FUNCTIONS].current_size; i += sizeof *funp) {
	funp = (function_t *)(mem_block[A_FUNCTIONS].block + i);
	if ((funp->flags & NAME_UNDEFINED) && (funp->flags & NAME_DEF_BY_INHERIT))
	    funp->flags = (funp->flags & ~NAME_UNDEFINED) | NAME_INHERITED;
	if (funp->flags & NAME_ALIAS) {
	    *funp = *(funp - funp->offset);
	    funp->flags |= NAME_ALIAS;
	}
    }
    generate_final_program(1);

    size = align(sizeof (program_t));

    /* delete argument information if we're not saving it */
    if (!(pragmas & PRAGMA_SAVE_TYPES))
	mem_block[A_ARGUMENT_TYPES].current_size = 0;
    if (!(mem_block[A_ARGUMENT_TYPES].current_size))
	mem_block[A_ARGUMENT_INDEX].current_size = 0;

    for (i=0; i<NUMPAREAS; i++)
	if (i != A_LINENUMBERS)
	    size += align(mem_block[i].current_size);

    p = (char *)DXALLOC(size, TAG_PROGRAM, "epilog: 1");
    prog = (program_t *)p;
    *prog = NULL_program;
    prog->total_size = size;
    prog->ref = 0;
    prog->func_ref = 0;
    ihe = lookup_ident("heart_beat");
    prog->heart_beat = (ihe ? ihe->dn.function_num : -1);
    prog->name = current_file;
    current_file = 0;

    prog->id_number = get_id_number();
    total_prog_block_size += prog->total_size;
    total_num_prog_blocks += 1;

#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
    prog->tree_r = a_functions_root;
#endif

    prog->line_swap_index = -1;
    /* Format is now:
     * <short total size> <short line_info_offset> <file info> <line info>
     */
    lnoff = 2 + (mem_block[A_FILE_INFO].current_size / sizeof(short));
    lnsz = lnoff * sizeof(short) + mem_block[A_LINENUMBERS].current_size;

    prog->file_info = (unsigned short *)DXALLOC(lnsz, TAG_LINENUMBERS
						    , "epilog");
    prog->file_info[0] = (unsigned short)lnsz;
    prog->file_info[1] = (unsigned short)lnoff;

    memcpy(((char*)&prog->file_info[2]),
	  mem_block[A_FILE_INFO].block,
	  mem_block[A_FILE_INFO].current_size);

    prog->line_info = (unsigned char *)(&prog->file_info[lnoff]);
    memcpy(((char*)&prog->file_info[lnoff]),
	   mem_block[A_LINENUMBERS].block,
	   mem_block[A_LINENUMBERS].current_size);

    p += align(sizeof (program_t));

    prog->program = p;
    prog->program_size = mem_block[A_PROGRAM].current_size;
    copy_in(A_PROGRAM, &p);

    prog->functions = (function_t *)p;
    prog->num_functions = mem_block[A_FUNCTIONS].current_size /
	sizeof (function_t);
    copy_in(A_FUNCTIONS, &p);

    prog->classes = (class_def_t *)p;
    prog->num_classes = mem_block[A_CLASS_DEF].current_size /
	  sizeof (class_def_t);
    copy_in(A_CLASS_DEF, &p);

    prog->class_members = (class_member_entry_t *)p;
    copy_in(A_CLASS_MEMBER, &p);

    prog->strings = (char **)p;
    prog->num_strings = mem_block[A_STRINGS].current_size /
	  sizeof (char *);
    copy_in(A_STRINGS, &p);

    prog->variable_names = (variable_t *)p;
    prog->num_variables = mem_block[A_VARIABLES].current_size /
	  sizeof (variable_t);
    copy_in(A_VARIABLES, &p);

    prog->num_inherited = mem_block[A_INHERITS].current_size /
	  sizeof (inherit_t);
    if (prog->num_inherited) {
	prog->inherit = (inherit_t *)p;
	copy_in(A_INHERITS, &p);
    } else
	prog->inherit = 0;

    if (mem_block[A_ARGUMENT_TYPES].current_size) {
	prog->argument_types = (unsigned short *) p;
	copy_in(A_ARGUMENT_TYPES, &p);

	prog->type_start = (unsigned short *) p;
	copy_in(A_ARGUMENT_INDEX, &p);
    } else {
	prog->argument_types = 0;
	prog->type_start = 0;
    }
#ifdef BINARIES
    if ((pragmas & PRAGMA_SAVE_BINARY)
#ifdef LPC_TO_C
    || compile_to_c
#endif
    ) {
	save_binary(prog, &mem_block[A_INCLUDES], &mem_block[A_PATCH]);
    }
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
    for (i = 0; (unsigned)i < prog->num_inherited; i++) {
	reference_prog (prog->inherit[i].prog, "inheritance");
    }
    scratch_destroy();
    clean_up_locals();
    free_unused_identifiers();
    end_new_file();
}

/*
 * Initialize the environment that the compiler needs.
 */
static void prolog P2(int, f, char *, name) {
    int i;

    function_context.num_parameters = -1;
    prog = 0;   /* 0 means fail to load. */
    num_parse_error = 0;
#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
    a_functions_root = (unsigned short)0xffff;
#endif
    /* Initialize memory blocks where the result of the compilation
     * will be stored.
     */
    for (i=0; i < NUMAREAS; i++) {
	mem_block[i].block = DXALLOC(START_BLOCK_SIZE, TAG_COMPILER, "prolog: 2");
	mem_block[i].current_size = 0;
	mem_block[i].max_size = START_BLOCK_SIZE;
    }
    memset(string_tags, 0, sizeof(string_tags));
    freed_string = -1;
    initialize_parser();

    current_file = make_shared_string(name);
    current_file_id = add_program_file(name, 1);
    start_new_file(f);
}

/*
 * The program has errors, clean things up.
 */
static void clean_parser() {
    int i;
    function_t *funp;
    variable_t dummy;
    char *s;

    /*
     * Free function stuff.
     */
    for (i = 0; i < mem_block[A_FUNCTIONS].current_size; i += sizeof *funp) {
	funp = (function_t *)(mem_block[A_FUNCTIONS].block + i);
	if (funp->name)
	    free_string(funp->name);
    }
    for (i = 0; i < mem_block[A_STRINGS].current_size; i += sizeof(char *)) {
	COPY_PTR(&s, mem_block[A_STRINGS].block + i);
	free_string(s);
    }
    for (i = 0; i < mem_block[A_VARIABLES].current_size; i += sizeof dummy) {
	memcpy(&dummy, mem_block[A_VARIABLES].block + i, sizeof dummy);
	free_string(dummy.name);
    }

    prog = 0;
    for (i=0; i<NUMAREAS; i++)
	FREE(mem_block[i].block);
    clean_up_locals();
    scratch_destroy();
    free_unused_identifiers();
}

char *
the_file_name P1(char *, name)
{
    char *tmp;
    int len;

    len = strlen(name);
    if (len < 3) {
	return string_copy(name, "the_file_name");
    }
    tmp = new_string(len - 1, "the_file_name");
    if (!tmp) {
	return string_copy(name, "the_file_name");
    }
    tmp[0] = '/';
    strncpy(tmp + 1, name, len - 2);
    tmp[len - 1] = '\0';
    return tmp;
}

int case_compare P2(parse_node_t **, c1, parse_node_t **, c2) {
    if ((*c1)->kind == NODE_DEFAULT)
	return -1;
    if ((*c2)->kind == NODE_DEFAULT)
	return 1;

    return ((*c1)->r.number - (*c2)->r.number);
}

int string_case_compare P2(parse_node_t **, c1, parse_node_t **, c2) {
    if ((*c1)->kind == NODE_DEFAULT)
	return -1;
    if ((*c2)->kind == NODE_DEFAULT)
	return 1;

    return (PROG_STRING((*c1)->r.number) - PROG_STRING((*c2)->r.number));
}

void prepare_cases P2(parse_node_t *, pn, int, start) {
    parse_node_t **ce_start, **ce_end, **ce;
    int end, last_key, this_key;
    int direct = 1;
    
    ce_start = (parse_node_t **)&mem_block[A_CASES].block[start];
    end = mem_block[A_CASES].current_size;
    ce_end = (parse_node_t **)&mem_block[A_CASES].block[end];

    if (ce_start == ce_end) {
	/* no cases */
	pn->v.expr = 0;
	mem_block[A_CASES].current_size = start;
	return;
    }

    if (pn->kind == NODE_SWITCH_STRINGS)
	quickSort((char *)ce_start, ce_end - ce_start, sizeof(parse_node_t *),
		  string_case_compare);
    else
	quickSort((char *)ce_start, ce_end - ce_start, sizeof(parse_node_t *),
		  case_compare);

    ce = ce_start;
    if ((*ce)->kind == NODE_DEFAULT) {
	if (ce + 1 == ce_end) {
	    /* only a default */
	    pn->v.expr = *ce;
	    (*ce)->l.expr = 0;
	    mem_block[A_CASES].current_size = start;
	    return;
	}
	ce++;
	(*(ce-1))->l.expr = *ce;
    }
    if ((*ce)->v.expr) {
	last_key = (*ce)->v.expr->r.number;
	direct = 0;
    } else
	last_key = (*ce)->r.number;
    ce++;
    while (ce < ce_end) {
	this_key = (*ce)->r.number;
	if (pn->kind == NODE_SWITCH_RANGES && this_key <= last_key) {
	    char buf[1024];
	    char *f1, *f2;
	    int fi1, fi2;
	    int l1, l2;

	    /* make sure line numbers exist for the cases */
	    save_file_info(current_file_id, current_line - current_line_saved);
	    current_line_saved = current_line;

	    translate_absolute_line((*ce)->line, 
				    (unsigned short *)mem_block[A_FILE_INFO].block,
				    &fi1, &l1);
	    translate_absolute_line((*(ce-1))->line, 
				    (unsigned short *)mem_block[A_FILE_INFO].block,
				    &fi2, &l2);
	    f1 = PROG_STRING(fi1);
	    f2 = PROG_STRING(fi2);

	    sprintf(buf, "Overlapping cases: %s%s%d and %s%s%d.",
		    f1 ? f1 : "", f1 ? ":" : "line ", l1,
		    f2 ? f2 : "", f2 ? ":" : "line ", l2);
	    yyerror(buf);
	}
	(*(ce-1))->l.expr = *ce;
	if ((*ce)->v.expr) {
	    last_key = (*ce)->v.expr->r.number;
	    direct = 0;
	} else {
	    if (last_key + 1 != this_key) direct = 0;
	    last_key = this_key;
	}
	ce++;
    }
    (*(ce_end-1))->l.expr = 0;
    if (direct && pn->kind == NODE_SWITCH_NUMBERS)
	pn->kind = NODE_SWITCH_DIRECT;
    pn->v.expr = *(ce_start);
    mem_block[A_CASES].current_size = start;
}

void
save_file_info P2(int, file_id, int, lines) {
    short fi[2];

    fi[0] = lines;
    fi[1] = file_id;
    add_to_mem_block(A_FILE_INFO, (char *)&fi[0], sizeof(fi));
}

int
add_program_file P2(char *, name, int, top) {
    if (!top)
	add_to_mem_block(A_INCLUDES, name, strlen(name)+1);
    return store_prog_string(name) + 1;
}
