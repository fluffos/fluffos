#include "std.h"
#include "lpc_incl.h"
#include "compiler.h"
#include "trees.h"
#include "lex.h"
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
static program_t *epilog PROT((void));
static void show_overload_warnings PROT((void));

#define CT(x) (1 << (x))
#define CT_SIMPLE(x) (CT(TYPE_ANY) | CT(x))

short compatible[11] = { 
    /* UNKNOWN */  0,
    /* ANY */      0xfff,
    /* NOVALUE to*/CT_SIMPLE(TYPE_NOVALUE) | CT(TYPE_VOID) | CT(TYPE_NUMBER),
    /* VOID to*/   CT_SIMPLE(TYPE_VOID) | CT(TYPE_NUMBER),
    /* NUMBER to*/ CT_SIMPLE(TYPE_NUMBER) | CT(TYPE_REAL),
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

int exact_types, global_modifiers;

int current_type;
int var_defined;

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

/* This function has strput() semantics; see comments in simulate.c */
char *get_two_types P4(char *, where, char *, end, int, type1, int, type2)
{
    where = strput(where, end, "( ");
    where = get_type_name(where, end, type1);
    where = strput(where, end, "vs ");
    where = get_type_name(where, end, type2);
    where = strput(where, end, ")");

    return where;
}

void init_locals()
{
    type_of_locals = CALLOCATE(CFG_MAX_LOCAL_VARIABLES,unsigned short, 
			       TAG_LOCALS, "init_locals:1");
    locals = CALLOCATE(CFG_MAX_LOCAL_VARIABLES, ident_hash_elem_t *, 
		       TAG_LOCALS, "init_locals:2");
    runtime_locals = CALLOCATE(CFG_MAX_LOCAL_VARIABLES, char, 
			       TAG_LOCALS, "init_locals:3");
    type_of_locals_ptr = type_of_locals;
    locals_ptr = locals;
    runtime_locals_ptr = runtime_locals;
    locals_size = type_of_locals_size = CFG_MAX_LOCAL_VARIABLES;
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

void deactivate_current_locals() {
    int i;

    for (i = 0; i < current_number_of_locals; i++){
	runtime_locals_ptr[i] = locals_ptr[i]->dn.local_num;
	locals_ptr[i]->dn.local_num = -1;
    }
}

void reactivate_current_locals() {
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
    while (offset--) {
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
    if (max_num_locals == CFG_MAX_LOCAL_VARIABLES) {
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

void reallocate_locals() {
    int offset;
    offset = type_of_locals_ptr - type_of_locals;
    type_of_locals = RESIZE(type_of_locals, type_of_locals_size += CFG_MAX_LOCAL_VARIABLES,
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
void copy_variables P2(program_t *, from, int, type) {
    int i;
    
    for (i = 0; i < from->num_inherited; i++)
	copy_variables(from->inherit[i].prog, 
		       type | from->inherit[i].type_mod);
    for (i = 0; i < from->num_variables_defined; i++) {
	int t = from->variable_types[i] | type;
	if (t & NAME_PUBLIC)
	    t &= ~NAME_PRIVATE;
	
	define_variable(from->variable_table[i], t, t & NAME_PRIVATE);
    }
}

int add_new_function_entry() {
    int index = mem_block[A_FUNCTION_FLAGS].current_size / sizeof(unsigned short);

    allocate_in_mem_block(A_RUNTIME_FUNCTIONS, sizeof(runtime_function_u));
    allocate_in_mem_block(A_FUNCTION_FLAGS, sizeof(unsigned short));
    allocate_in_mem_block(A_FUNCTION_DEFS, sizeof(compiler_temp_t));
    
    return index;
}

/* copy a function verbatim into this object, and possibly add it to the
   list of functions in this object, as well
 */
void copy_function P5(program_t *, prog, int, index,
		      program_t *, defprog, int, defindex,
		      int, typemod) {
    ident_hash_elem_t *ihe;
    
    int where = add_new_function_entry();
    int flags = prog->function_flags[index];
    int f = (flags & NAME_MASK) | NAME_DEF_BY_INHERIT | NAME_UNDEFINED;
    
    /* 'private' functions become 'hidden' */
    if (f & NAME_PRIVATE)
	f |= NAME_HIDDEN;
	
    f |= typemod;
    /* remember 'public' turns off 'private' */
    if (f & NAME_PUBLIC)
	f &= ~NAME_PRIVATE;

    FUNCTION_FLAGS(where) = f;

    FUNCTION_TEMP(where)->prog = defprog;
    FUNCTION_TEMP(where)->u.func = defprog->function_table + defindex;

    FUNCTION_RENTRY(where)->inh.offset = NUM_INHERITS - 1;
    FUNCTION_RENTRY(where)->inh.function_index_offset = index;
    FUNCTION_ALIAS(where) = 1;
    
    /* add the identifier */
    ihe = find_or_add_ident(defprog->function_table[defindex].name,
			    FOA_GLOBAL_SCOPE);
    if (ihe->dn.function_num == -1)
	ihe->sem_value++;
    ihe->dn.function_num = where;
}

int lookup_class_member P3(int, which, char *, name, char *, type) {
    int i;
    class_def_t *cd;
    class_member_entry_t *cme;

    cd = ((class_def_t *)mem_block[A_CLASS_DEF].block) + which;
    cme = ((class_member_entry_t *)mem_block[A_CLASS_MEMBER].block) + cd->index;
    for (i = 0; i < cd->size; i++) {
	if (strcmp(PROG_STRING(cme[i].name), name) == 0)
	    break;
    }
    if (i == cd->size) {
	char buf[256];
	char *end = EndOf(buf);
	char *p;
	
	p = strput(buf, end, "Class '");
	p = strput(p, end, PROG_STRING(cd->name));
	p = strput(p, end, "' has no member '");
	p = strput(p, end, name);
	p = strput(p, end, "'");
	yyerror(buf);
	if (type) *type = TYPE_ANY;
	return -1;
    } else {
	if (type) *type = cme[i].type;
	return i;
    }
}	

parse_node_t *reorder_class_values P2(int, which, parse_node_t *, node) {
    class_def_t *cd;
    parse_node_t **tmp;
    int i;
    
    cd = ((class_def_t *)mem_block[A_CLASS_DEF].block) + which;
    tmp = CALLOCATE(cd->size, parse_node_t *, TAG_COMPILER,
		    "reorder_class_values");

    for (i = 0; i < cd->size; i++) 
	tmp[i] = 0;

    while (node) {
	i = lookup_class_member(which, (char *)node->l.expr, 0);
	if (i != -1)
	    tmp[i] = node->v.expr;
	node = node->r.expr;
    }
    i = cd->size;
    node = 0;
    while (i--) {
	parse_node_t *newnode;
	if (tmp[i]) {
	    CREATE_STATEMENTS(newnode, tmp[i], node);
	} else {
	    CREATE_STATEMENTS(newnode, 0, node);
	    CREATE_NUMBER(newnode->l.expr, 0);
	}
	node = newnode;
    }

    FREE(tmp);
    return node;
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
	else {
	    /* Possibly, this should check if the definitions are
	       consistent */
	    char buf[256];
	    char *end = EndOf(buf);
	    char *p;
	    
	    p = strput(buf, end, "Illegal to redefine class ");
	    p = strput(p, end, str);
	    yyerror(buf);
	}
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
void overload_function P6(program_t *, prog, int, index,
			  program_t *, defprog, int, defindex,
			  int, oldindex, int, typemod) {
    int alias;
    
    int oldflags = FUNCTION_FLAGS(oldindex);
    compiler_function_t *definition = &defprog->function_table[defindex];
    int newflags = prog->function_flags[index];

    /* check that we aren't overloading a nomask function */
    if (!(newflags & NAME_NO_CODE) &&
	REAL_FUNCTION(oldflags) && (oldflags & NAME_NO_MASK)) {
	char buf[256];
	char *end = EndOf(buf);
	char *p;
	
	p = strput(buf, end, "Illegal to redefine 'nomask' function \"");
	p = strput(p, end, definition->name);
	p = strput(p, end, "\"");
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
	&& REAL_FUNCTION(oldflags) && !(newflags & NAME_NO_CODE)
	&& (oldflags & NAME_UNDEFINED) /* not defined at top level yet */
	) {
	/* don't scream if one is private.  Why not?  Because I said so.
	 * private is pretty screwed up anyway.  In the future there
	 * won't be such a clash b/c private won't come up the tree.
	 * This also give the coder a way to shut the compiler up when
	 * you do inherit the same object twice in different branches :)
	 */
	if (!(oldflags & NAME_PRIVATE) && !(newflags & NAME_PRIVATE)) {
	    char buf[1024];
	    char *end = EndOf(buf);
	    char *p;
	    ovlwarn_t *ow;
	    program_t *defprog2;
	    int defindex2;
	    runtime_function_u *func_entry = FUNCTION_RENTRY(oldindex);
	    program_t *prog2 = INHERIT(func_entry->inh.offset)->prog;

	    defprog2 = prog2;
	    defindex2 = func_entry->inh.function_index_offset;
	    func_entry = FIND_FUNC_ENTRY(defprog2, defindex2);
	    
	    while (defprog2->function_flags[defindex2] & NAME_INHERITED) {
		defprog2 = defprog2->inherit[func_entry->inh.offset].prog;
		defindex2 = func_entry->inh.function_index_offset;
		func_entry = FIND_FUNC_ENTRY(defprog2, defindex2);
	    }
	    
	    p = strput(buf, end, definition->name);
	    p = strput(p, end, "() inherited from both /");
	    p = strput(p, end, defprog->name);
	    if (prog != defprog) {
		p = strput(p, end, " (via /");
		p = strput(p, end, prog->name);
		p = strput(p, end, ")");
	    }
	    p = strput(p, end, " and /");
	    p = strput(p, end, defprog2->name);
	    if (prog2 != defprog2) {
		p = strput(p, end, " (via /");
		p = strput(p, end, prog2->name);
		p = strput(p, end, ")");
	    }
	    p = strput(p, end, "; using the definition in /");
	    p = strput(p, end, prog->name);
	    p = strput(p, end, ".");
	    
	    ow = ALLOCATE(ovlwarn_t, TAG_COMPILER, "overload warning");
	    ow->next = overload_warnings;
	    ow->func = definition->name;
	    ow->warn = alloc_cstring(buf, "overload warning");
	    overload_warnings = ow;
	}
    }

    /* A new function also has to be inserted, since this spot will be
     * used when this function is called in an object beneath us.  Point
     * it at the overloaded function. 
     * No need to set much here because epilog() will fixup this entry
     * later.
     */
    alias = add_new_function_entry();
    FUNCTION_FLAGS(alias) = NAME_INHERITED | NAME_ALIAS;
    FUNCTION_RENTRY(alias)->inh.offset = NUM_INHERITS - 1;
    FUNCTION_RENTRY(alias)->inh.function_index_offset = index;
    FUNCTION_ALIAS(alias) = oldindex;
    FUNCTION_TEMP(alias)->prog = defprog;
    FUNCTION_TEMP(alias)->u.func = defprog->function_table + defindex;

    /* The rule here is that the latest function wins, so if it's not
       defined at this level and defined in the new object, we copy it in */
    if ((oldflags & NAME_UNDEFINED) && (!(newflags & NAME_NO_CODE))) {
	int f = (prog->function_flags[index] & NAME_MASK)
	    | NAME_DEF_BY_INHERIT | NAME_UNDEFINED;

        /* 'private' functions become 'hidden' */
	if (f & NAME_PRIVATE)
	    f |= NAME_HIDDEN;
	
	f |= typemod;
	/* remember 'public' turns off 'private' */
	if (f & NAME_PUBLIC)
	    f &= ~NAME_PRIVATE;

	FUNCTION_FLAGS(oldindex) = f;

	if (FUNCTION_PROG(oldindex) == 0) {
	    /* Woops; there was a prototype at this level already.  Mark
	       the old function table entry for removal */
	    COMPILER_FUNC(FUNCTION_TEMP(oldindex)->u.index)->address = USHRT_MAX;
	}
	FUNCTION_TEMP(oldindex)->prog = defprog;
	FUNCTION_TEMP(oldindex)->u.func = defprog->function_table + defindex;

	FUNCTION_RENTRY(oldindex)->inh.offset = NUM_INHERITS - 1;
	FUNCTION_RENTRY(oldindex)->inh.function_index_offset = index;
    }

    if (!(newflags & NAME_ALIAS))
	FUNCTION_ALIAS(oldindex)++;
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
int copy_functions P2(program_t *, from, int, typemod)
{
    int i, initializer = -1, num_functions = from->num_functions_total;
    ident_hash_elem_t *ihe;
    int num;
    
    if (from->num_functions_defined && 
	(from->function_table[from->num_functions_defined - 1].name[0] == APPLY___INIT_SPECIAL_CHAR))
        initializer = --num_functions;

    for (i = 0; i < num_functions; i++) {
	program_t *prog = from;
	int index = i;
	runtime_function_u *func_entry = FIND_FUNC_ENTRY(prog, index);
	compiler_function_t *funp;
	
	/* Walk up the inheritance tree to the real definition */
	while (prog->function_flags[index] & NAME_INHERITED) {
	    prog = prog->inherit[func_entry->inh.offset].prog;
	    index = func_entry->inh.function_index_offset;
	    func_entry = FIND_FUNC_ENTRY(prog, index);
	}
	DEBUG_CHECK(func_entry->def.f_index >= prog->num_functions_defined,
		    "Function index out of bounds!");
	funp = prog->function_table + func_entry->def.f_index;
	
	ihe = lookup_ident(funp->name);
	if (ihe && ((num = ihe->dn.function_num)!=-1)) {
	  /* The function has already been defined in this object */
	    overload_function(from, i, prog, func_entry->def.f_index,
			      num, typemod);
	} else {
	    copy_function(from, i, prog, func_entry->def.f_index, typemod);
	}
    }
    return initializer;
}

void type_error P2(char *, str, int, type)
{
    static char buff[256];
    char *end = EndOf(buff);
    char *p;

    p = strput(buff, end, str);
    p = strput(p, end, ": \"");
    p = get_type_name(p, end, type);
    p = strput(p, end, "\"");
    yyerror(buff);
}

/*
 * Compare two types, and return true if they are compatible.
 */

/* This one really is t1->t2; it isn't symmetric, since int->void isn't allowed. */
int compatible_types P2(int, t1, int, t2)
{
#ifdef OLD_TYPE_BEHAVIOR
    /* The old version effectively was almost always was true */
    return 1;
#else
    t1 &= ~NAME_TYPE_MOD;
    t2 &= ~NAME_TYPE_MOD;
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

/* This one is symmetric.  Used for comparison operators, etc */
int compatible_types2 P2(int, t1, int, t2)
{
#ifdef OLD_TYPE_BEHAVIOR
    /* The old version effectively was almost always was true */
    return 1;
#else
    t1 &= ~NAME_TYPE_MOD;
    t2 &= ~NAME_TYPE_MOD;
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
    if (compatible[t1] & (1 << t2))
	return 1;
    return compatible[t2] & (1 << t1);
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
static int find_matching_function P3(program_t *, prog, char *, name,
				     parse_node_t *, node) {
    int high = prog->num_functions_defined - 1;
    int low = 0;
    int i;
    
    /* Search our function table */
    while (high >= low) {
	int mid = (high + low)/2;
	char *p = prog->function_table[mid].name;
	
	if (name < p)
	    high = mid-1;
	else if (name > p)
	    low = mid+1;
	else {
	    int ind = prog->function_table[mid].runtime_index;
	    int flags = prog->function_flags[ind];
	    if (flags & (NAME_UNDEFINED | NAME_PROTOTYPE | NAME_INHERITED)) {
		if (flags & NAME_INHERITED)
		    break;
		return 0;
	    }
	    if (flags & NAME_PRIVATE)
		return -1;
	    
	    node->kind = NODE_CALL_2;
	    node->v.number = F_CALL_INHERITED;
	    node->l.number = ind;
	    node->type = prog->function_table[mid].type;
	    return 1;
	}
    }
    
    /* Search inherited function tables */
    i = prog->num_inherited;
    while (i--) {
	if (find_matching_function(prog->inherit[i].prog, name, node)) {
	    if (prog->inherit[i].type_mod & NAME_PRIVATE)
		return -1;
	    
	    node->l.number += prog->inherit[i].function_index_offset;
	    return 1;
	}
    }
    return 0;
}

int arrange_call_inherited P2(char *, name, parse_node_t *, node)
{
    inherit_t *ip;
    int num_inherits, super_length;
    char *super_name, *p, *real_name = name;
    char *shared_string;
    int ret;
    
    if (real_name[0] == ':') {
	super_name = 0;
	real_name += 2; 	/* There will be exactly two ':' */
	super_length = 0;
    } else if ((p = strchr(real_name, ':'))) {
        super_name = name;
        real_name = p+2;
        super_length = real_name - super_name - 2;
    }
    num_inherits = NUM_INHERITS;
    /* no need to look for it unless its in the shared string table */
    if ((shared_string = findstring(real_name))) {
	ip = (inherit_t *) mem_block[A_INHERITS].block;
	for (; num_inherits > 0; ip++, num_inherits--) {
	    int tmp;
	    
	    if (super_name) {
		int l = SHARED_STRLEN(ip->prog->name);	/* Including .c */

		if (l - 2 < super_length)
		    continue;
		if (strncmp(super_name, ip->prog->name + l - 2 -super_length,
			    super_length) != 0 ||
                    !((l - 2 == super_length) ||
                     ((ip->prog->name + l - 3 - super_length)[0] == '/')))
                    continue;
	    }
	    if ((tmp = find_matching_function(ip->prog, shared_string, node))) {
		if (tmp == -1 || (ip->type_mod & NAME_PRIVATE)) {
		    yyerror("Called function is private.");

		    goto invalid;
		}
		
		ret = node->l.number + ip->function_index_offset;
		node->l.number += ((ip - (inherit_t *) mem_block[A_INHERITS].block) << 16);
		return ret;
	    }
	}
    }				/* if in shared string table */
    {
	char buff[256];
	char *end = EndOf(buff);
	char *p;
	
	p = strput(buff, end, "No such inherited function ");
	p = strput(p, end, name);
	yyerror(buff);
    }

  invalid:
    node->kind = NODE_CALL_2;
    node->v.number = F_CALL_INHERITED;
    node->l.number = 0;
    node->type = TYPE_ANY;
    
    return -1;
}

/*
 * Define a new function. Note that this function is called at least twice
 * for all function definitions. First as a prototype, then as the real
 * function. Thus, there are tests to avoid generating error messages more
 * than once by looking at (flags & NAME_PROTOTYPE).
 */
/* Warning: returns an index into A_COMPILER_FUNCTIONS, not the full
 * function list
 */
int define_new_function P5(char *, name, int, num_arg, int, num_local,
                           int, flags, int, type)
{
    int runtime_num, num;
    unsigned short argument_start_index;
    ident_hash_elem_t *ihe;
    compiler_function_t *funp = 0;
    
    runtime_num = (ihe = lookup_ident(name)) ? ihe->dn.function_num : -1;
    if (runtime_num >= 0) {
	runtime_defined_t *fundefp = &FUNCTION_DEF_RENTRY(runtime_num)->def;
	int funflags = FUNCTION_FLAGS(runtime_num);
	funp = FUNCTION_DEF(runtime_num);
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
	if (!(funflags & NAME_UNDEFINED) && !(flags & NAME_PROTOTYPE)) {
	    char buff[256];
	    char *end = EndOf(buff);
	    char *p;

	    p = strput(buff, end, "Redeclaration of function ");
	    p = strput(p, end, name);
	    p = strput(p, end, ".");
	    yyerror(buff);
	    return -1;
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
	if ((funflags & NAME_NO_MASK) &&
	    !(funflags & NAME_PROTOTYPE) &&
	    !(flags & NAME_PROTOTYPE)) {
	    char buf[256];
	    char *end = EndOf(buf);
	    char *p;
	    
	    p = strput(buf, end, "Illegal to redefine 'nomask' function \"");
	    p = strput(p, end, name);
	    p = strput(p, end, "\"");
	    yyerror(buf);
	}
	/* only check prototypes for matching.  It shouldn't be required that
	   overloading a function must have the same signature */
	if (exact_types && (funflags & NAME_PROTOTYPE) && 
	    funp->type != TYPE_UNKNOWN) {
	    int i;

	    /* This should be changed to catch two prototypes which disagree */
	    if (!(flags & NAME_PROTOTYPE)) {
		if (fundefp->num_arg != num_arg 
		    && !(funflags & NAME_VARARGS))
		    yyerror("Number of arguments disagrees with previous definition.");
		if (!(funflags & NAME_STRICT_TYPES))
		    yyerror("Called function not compiled with type testing.");

		/* Now check that argument types wasn't changed. */
		if ((type & (~NAME_TYPE_MOD)) != funp->type) {
		    char buff[256];
		    char *end = EndOf(buff);
		    char *p;
		    
		    p = strput(buff, end, "Return type doesn't match prototype ");
		    get_two_types(p, end, type & (~NAME_TYPE_MOD), funp->type);
		    yywarn(buff);
		}
		
		for (i = 0; i < num_arg; i++) {
		    /* FIXME: check arg types here */
		}
	    }
	}
	
	/* If it was yet another prototype, then simply return. */
	if (flags & NAME_PROTOTYPE)
	    return -1; /* unused for prototypes */

	if (pragmas & PRAGMA_WARNINGS)
	    remove_overload_warnings(funp->name);

	/* If there was already a definition at this level (due to a 
	 * prototype), clear it out.  Don't free the function name, though;
	 * the name is the same, and the ident hash table might be counting
	 * on it to stay allocated.
	 */
	if (FUNCTION_PROG(runtime_num) == 0) {
	    num = FUNCTION_TEMP(runtime_num)->u.index;
	    funp = COMPILER_FUNC(num);
	} else
	    funp = 0;
    }
    if (!funp) {
	num = mem_block[A_COMPILER_FUNCTIONS].current_size / sizeof(compiler_function_t);
	funp = (compiler_function_t *)allocate_in_mem_block(A_COMPILER_FUNCTIONS, sizeof(compiler_function_t));
	funp->name = make_shared_string(name);
	argument_start_index = INDEX_START_NONE;
	add_to_mem_block(A_ARGUMENT_INDEX, (char *) &argument_start_index,
			 sizeof argument_start_index);
	
	if (runtime_num == -1) {
	    runtime_num = add_new_function_entry();
	    ihe = find_or_add_ident(funp->name, FOA_GLOBAL_SCOPE);
	    if (ihe->dn.function_num == -1)
		ihe->sem_value++;
	    ihe->dn.function_num = runtime_num;
	    FUNCTION_ALIAS(runtime_num) = 0;
	}
    }
    FUNCTION_TEMP(runtime_num)->prog = 0;
    FUNCTION_TEMP(runtime_num)->u.index = num;
    
    if (exact_types)
	flags |= NAME_STRICT_TYPES;
    FUNCTION_FLAGS(runtime_num) = (type & NAME_TYPE_MOD) | flags;

    FUNCTION_RENTRY(runtime_num)->def.num_local = num_local;
    FUNCTION_RENTRY(runtime_num)->def.num_arg = num_arg;
    FUNCTION_RENTRY(runtime_num)->def.f_index = num;
    FUNCTION_ALIAS(runtime_num)++;
    
    funp->type = type & ~NAME_TYPE_MOD;
    funp->runtime_index = runtime_num;
    funp->address = 0;
#ifdef PROFILE_FUNCTIONS
    funp->calls = 0L;
    funp->self = 0L;
    funp->children = 0L;
#endif

    if (exact_types && num_arg) {
	*((unsigned short *)mem_block[A_ARGUMENT_INDEX].block + num) = 
	    mem_block[A_ARGUMENT_TYPES].current_size / sizeof(unsigned short);
	add_to_mem_block(A_ARGUMENT_TYPES, (char *)type_of_locals_ptr,
			 num_arg * sizeof(*type_of_locals_ptr));
    }
    return num;
}

int define_variable P3(char *, name, int, type, int, hide)
{
    variable_t *dummy;
    int n;
    ident_hash_elem_t *ihe;

    n = (mem_block[A_VAR_TEMP].current_size / sizeof(variable_t));

    ihe = find_or_add_ident(name, FOA_GLOBAL_SCOPE);
    if (ihe->dn.global_num == -1) {
	ihe->sem_value++;
	ihe->dn.global_num = n;
    } else {
	if (VAR_TEMP(ihe->dn.global_num)->type & NAME_NO_MASK) {
	    char buf[256];
	    char *end = EndOf(buf);
	    char *p;
	    
	    p = strput(buf, end, "Illegal to redefine 'nomask' variable \"");
	    p = strput(p, end, name);
	    p = strput(p, end, "\"");
	    yyerror(buf);
	}
	/* Okay, the nasty idiots have two variables of the same name in
	   the same object.  This causes headaches for save_object().
	   To keep save_object sane, we need to make one static */
	if (!(VAR_TEMP(ihe->dn.global_num)->type & NAME_STATIC))
	    type |= NAME_STATIC;

	/* hidden variables don't cause variables that are visible to become
	   invisible; we only add them above (in the !hide case) for better
	   error messages */
	if (!hide)
	    ihe->dn.global_num = n;
    }

    dummy = (variable_t *)allocate_in_mem_block(A_VAR_TEMP, sizeof(variable_t));
    dummy->name = name;
    dummy->type = type;

    if (hide) dummy->type |= NAME_HIDDEN;

    return n;
}

int define_new_variable P2(char *, name, int, type) {
    int n;
    unsigned short *tp;
    char **np;
    
    var_defined = 1;
    name = make_shared_string(name);
    n = define_variable(name, type, 0);
    np = (char **)allocate_in_mem_block(A_VAR_NAME, sizeof(char*));
    *np = name;
    tp = (unsigned short *)allocate_in_mem_block(A_VAR_TYPE, sizeof(unsigned short));
    *tp = type;
    
    return n;
}

char *compiler_type_names[] = {"unknown", "mixed", "void", "void", 
		               "int", "string", "object", "mapping",
		               "function", "float", "buffer" };

/* This routine has the semantics of strput(); see comments in simulate.c */
char *get_type_name P3(char *, where, char *, end, int, type)
{
    int pointer = 0;

    if (type & NAME_STATIC)
	where = strput(where, end, "static ");
    if (type & NAME_NO_MASK)
	where = strput(where, end, "nomask ");
    if (type & NAME_PRIVATE)
	where = strput(where, end, "private ");
    if (type & NAME_PROTECTED)
	where = strput(where, end, "protected ");
    if (type & NAME_PUBLIC)
	where = strput(where, end, "public ");
    if (type & NAME_VARARGS)
	where = strput(where, end, "varargs ");
    type &= ~NAME_TYPE_MOD;
    if (type & TYPE_MOD_ARRAY) {
	pointer = 1;
	type &= ~TYPE_MOD_ARRAY;
    }
    if (type & TYPE_MOD_CLASS) {
	where = strput(where, end, "class ");
	/* we're sometimes called from outside the compiler */
	if (current_file)
	    where = strput(where, end, PROG_STRING(CLASS(type & ~TYPE_MOD_CLASS)->name));
    } else {
	DEBUG_CHECK(type >= sizeof compiler_type_names / sizeof compiler_type_names[0], "Bad type\n");
	where = strput(where, end, compiler_type_names[type]);
    }
    where = strput(where, end, " ");
    if (pointer)
	where = strput(where, end, "* ");
    return where;
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

int validate_function_call P2(int, f, parse_node_t *, args)
{
    compiler_function_t *funp = FUNCTION_DEF(f);
    runtime_defined_t *fundefp = &(FUNCTION_DEF_RENTRY(f)->def);
    int funflags = FUNCTION_FLAGS(f);
    int num_arg = ( args ? args->kind : 0 );
    int num_var = 0;
    parse_node_t *pn = args;
    unsigned short *arg_types = 0;
    program_t *prog;
    
    while (pn) {
	if (pn->type & 1) num_var++;
	pn = pn->r.expr;
    }

    /* Make sure it isn't private */
    if (funflags & NAME_HIDDEN) {
	char buf[256];
	char *end = EndOf(buf);
	char *p;
	
	p = strput(buf, end, "Illegal to call private function '");
	p = strput(p, end, funp->name);
	p = strput(p, end, "'");
	yyerror(buf);
    }
    
    /*
     * Verify that the function has been defined already.
     */
    if ((funflags & NAME_UNDEFINED) &&
	!(funflags & (NAME_PROTOTYPE | NAME_DEF_BY_INHERIT)) && exact_types) {
	char buf[256];
	char *end = EndOf(buf);
	char *p;

	p = strput(buf, end, "Function ");
	p = strput(p, end, funp->name);
	p = strput(p, end, " undefined");
	yyerror(buf);
    }
    /*
     * Check number of arguments.
     */
    if (!(funflags & NAME_VARARGS) &&
	(funflags & NAME_STRICT_TYPES) &&
	exact_types) {
	char buff[256];
	char *end = EndOf(buff);
	char *p;
	
	if (num_var) {
	    p = strput(buff, end, "Illegal to pass a variable number of arguments to non-varargs function ");
	    p = strput(p, end, funp->name);
	    p = strput(p, end, "\n");
	    yyerror(buff);
	} else
	if (fundefp->num_arg != num_arg) {
	    p = strput(buff, end, "Wrong number of arguments to ");
	    p = strput(p, end, funp->name);
	    p = strput(p, end, "\n    Expected: ");
	    p = strput_int(p, end, fundefp->num_arg);
	    p = strput(p, end, "  Got: ");
	    p = strput_int(p, end, num_arg);
	    yyerror(buff);
	}
    }
    /*
     * Check the argument types.
     */
    if ((prog = FUNCTION_PROG(f))) {
	int which = (FUNCTION_TEMP(f)->u.func - prog->function_table);
	if (prog->type_start) {
	    int start = prog->type_start[which];
	    if (start != INDEX_START_NONE)
		arg_types = prog->argument_types + start;
	}
    } else {
	int which = FUNCTION_TEMP(f)->u.index;
	int start = *((unsigned short *)mem_block[A_ARGUMENT_INDEX].block + which);
	if (start != INDEX_START_NONE)
	    arg_types = (unsigned short *)mem_block[A_ARGUMENT_TYPES].block + start;
    }
    
    if (exact_types && arg_types) {
	int i, tmp;
	parse_node_t *enode = args;
	int fnarg = fundefp->num_arg;

	if (funflags & NAME_TRUE_VARARGS) 
	    fnarg--;

	for (i = 0; (unsigned) i < fnarg && i < num_arg; i++) {
	    if (enode->type & 1) break;
	    tmp = enode->v.expr->type;

	    if (!compatible_types(tmp, arg_types[i])) {
		char buff[256];
		char *end = EndOf(buff);
		char *p;
		
		p = strput(buff, end, "Bad type for argument ");
		p = strput_int(p, end, i+1);
		p = strput(p, end, " of ");
		p = strput(p, end, funp->name);
		p = strput(p, end, " ");
		p = get_two_types(p, end, arg_types[i], tmp);
		yyerror(buff);
	    }
	    enode = enode->r.expr;
	}
    }
    return funp->type;
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

/* Take a NODE_CALL, and discard the call, preserving only the args with
   side effects */
parse_node_t *
throw_away_call P1(parse_node_t *, pn) {
    parse_node_t *enode;
    parse_node_t *ret = 0;
    parse_node_t *arg;
    
    enode = pn->r.expr;
    while (enode) {
	arg = insert_pop_value(enode->v.expr);
	if (arg) {
	    /* woops.  Don't lose the side effect. */
	    if (ret) {
		parse_node_t *tmp;
		CREATE_STATEMENTS(tmp, ret, arg);
		ret = tmp;
	    } else {
		ret = arg;
	    }
	}
	enode = enode->r.expr;
    }
    return ret;
}

parse_node_t *
throw_away_mapping P1(parse_node_t *, pn) {
    parse_node_t *enode;
    parse_node_t *ret = 0;
    parse_node_t *arg;
    
    enode = pn->r.expr;
    while (enode) {
	arg = insert_pop_value(enode->v.expr->l.expr);
	if (arg) {
	    /* woops.  Don't lose the side effect. */
	    if (ret) {
		parse_node_t *tmp;
		CREATE_STATEMENTS(tmp, ret, arg);
		ret = tmp;
	    } else {
		ret = arg;
	    }
	}
	arg = insert_pop_value(enode->v.expr->r.expr);
	if (arg) {
	    /* woops.  Don't lose the side effect. */
	    if (ret) {
		parse_node_t *tmp;
		CREATE_STATEMENTS(tmp, ret, arg);
		ret = tmp;
	    } else {
		ret = arg;
	    }
	}
	enode = enode->r.expr;
    }
    return ret;
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
#ifdef F_SIZEOF
	case F_SIZEOF:
	    /* Obscene crap like: sizeof( ({ 1, i++, x + 1, foo() }) )
	     *                    -> i++, foo(), 4
	     */
	    if (!pn && num == 1 && 
		IS_NODE(args->r.expr->v.expr, NODE_CALL, F_AGGREGATE)) {
		parse_node_t *repl, *ret, *node;

		CREATE_NUMBER(node, args->r.expr->v.expr->l.number);
		ret = throw_away_call(args->r.expr->v.expr);
		if (ret) {
		    CREATE_TWO_VALUES(repl, TYPE_NUMBER, ret, node);
		    return repl;
		} else
		    return node;
	    }
#endif
	}

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
	    char buff[256];
	    char *end = EndOf(buff);
	    char *p;
	    
	    p = strput(buff, end, "Illegal to pass variable number of arguments to non-varargs efun ");
	    p = strput(p, end, predefs[f].word);
	    yyerror(buff);
	    CREATE_ERROR(args);
	    return args;
	} else if ((num - num_var) < min_arg) {
	    char buff[256];
	    char *end = EndOf(buff);
	    char *p;
	    
	    p = strput(buff, end, "Too few arguments to ");
	    p = strput(p, end, predefs[f].word);
	    CREATE_ERROR(args);
	    yyerror(buff);
	    return args;
	} else if (num > max_arg && max_arg != -1) {
	    char buff[256];
	    char *end = EndOf(buff);
	    char *p;
	    
	    p = strput(buff, end, "Too many arguments to ");
	    p = strput(p, end, predefs[f].word);
	    yyerror(buff);
	    CREATE_ERROR(args);
	    return args;
	}
	if (max_arg != -1 && exact_types) {
	    /*
	     * Now check all types of arguments to efuns.
	     */
	    int i, argn, tmp;
	    char buff[256];
	    char *end = EndOf(buff);
	    char *p;
	    parse_node_t *enode = args;
	    argp = &efun_arg_types[predefs[f].arg_index];
	    
	    for (argn = 0; argn < num; argn++) {
		enode = enode->r.expr;
		if (enode->type & 1) break;
		/* this can happen for default args */
		if (!enode->v.expr) break;
		tmp = enode->v.expr->type;
		for (i=0; !compatible_types(tmp, argp[i]) && argp[i] != 0; i++)
		    ;

		if (argp[i] == 0) {
		    p = strput(buff, end, "Bad argument ");
		    p = strput_int(p, end, argn + 1);
		    p = strput(p, end, " to efun ");
		    p = strput(p, end, predefs[f].word);
		    p = strput(p, end, "()");
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
	args->type = predefs[f].ret_type;
	if (args->type == TYPE_NOVALUE) {
	    args->v.number += NOVALUE_USED_FLAG;
	    args->type = TYPE_VOID;
	}
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
program_t *
compile_file P2(int, f, char *, name) {
    int yyparse PROT((void));
    static int guard = 0;
    program_t *prog;
    
    /* The parser isn't reentrant.  On a few occasions (compile
     * errors, valid_override) LPC code is called during compilation,
     * causing the possibility of arriving here again.
     */
    if (guard) {
	error("Object cannot be loaded during compilation.\n");
    }
    guard = 1;
    
    prolog(f, name);
    yyparse();
    prog = epilog();

    guard = 0;
    return prog;
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

static int compare_compiler_funcs P2(int *, x, int *, y) {
    char *n1 = COMPILER_FUNC(*x)->name;
    char *n2 = COMPILER_FUNC(*y)->name;
    int sp1, sp2;
    
    /* make sure #global_init# stays last; also shuffle empty entries to
     * the end so we can delete them easily.
     */
    if (n1[0] == '#')
	sp1 = 1;
    else if (COMPILER_FUNC(*x)->address == USHRT_MAX)
	sp1 = 2;
    else
	sp1 = 0;
    
    if (n2[0] == '#')
	sp2 = 1;
    else if (COMPILER_FUNC(*y)->address == USHRT_MAX)
	sp2 = 2;
    else
	sp2 = 0;
    
    if (sp1 < sp2)
	return -1;
    if (sp1 > sp2)
	return 1;
    
    if (n1 < n2)
	return -1;
    if (n1 > n2)
	return 1;

    return 0;
}

static void copy_and_sort_function_table P2(program_t *,prog, char **, p) {
    int *temp, *inverse;
    int i, num_runtime;
    compiler_function_t *dest = (compiler_function_t *)*p;
    int num, new_num;
    
    num = mem_block[A_COMPILER_FUNCTIONS].current_size / sizeof (compiler_function_t);
    if (!num) {
	prog->num_functions_defined = 0;
	return;
    }
    
    temp = CALLOCATE(num, int, TAG_TEMPORARY, "copy_and_sort_function_table");
    for (i = 0; i < num; i++)
	temp[i] = i;

    quickSort(temp, num, sizeof(int), compare_compiler_funcs);

    new_num = num;
    while (new_num && COMPILER_FUNC(temp[new_num-1])->address == USHRT_MAX)
	new_num--;
    prog->num_functions_defined = new_num;

    inverse = CALLOCATE(num, int, TAG_TEMPORARY, "copy_and_sort_function_table");
    for (i = 0; i < num; i++) 
	inverse[temp[i]] = i;

    for (i = 0; i < new_num; i++)
	dest[i] = *COMPILER_FUNC(temp[i]);

    num_runtime = mem_block[A_RUNTIME_FUNCTIONS].current_size / sizeof(runtime_function_u);
#ifdef COMPRESS_FUNCTION_TABLES
    {
	compressed_offset_table_t *cftp = (compressed_offset_table_t *)mem_block[A_RUNTIME_COMPRESSED].block;
	int f_ov = cftp->first_overload;
	int f_def = cftp->first_defined;
	int n_ov = f_def - cftp->num_compressed;
	int n_def = prog->num_functions_total - f_def;
	int n_real = f_def - cftp->num_deleted;
	
	for (i = 0; i < n_ov; i++) {
	    int j = cftp->index[i];
	    int ri = f_ov + i;
	    if (j == 255)
		continue;
	    if (!(FUNCTION_FLAGS(ri) & NAME_INHERITED)) {
		/* remember the function entries have 'moved' */
		int oldix = FUNCTION_RENTRY(j)->def.f_index;
		DEBUG_CHECK(oldix >= num, "Function index out of range");
		FUNCTION_RENTRY(j)->def.f_index = inverse[oldix];
	    }
	}
	for (i = 0; i < n_def; i++) {
	    int ri = f_def + i;
	    if (!(FUNCTION_FLAGS(ri) & NAME_INHERITED)) {
		/* remember the function entries have 'moved' */
		int oldix = FUNCTION_RENTRY(n_real + i)->def.f_index;
		DEBUG_CHECK(oldix >= num, "Function index out of range");
		FUNCTION_RENTRY(n_real + i)->def.f_index = inverse[oldix];
	    }
	}
    }
#else
    for (i = 0; i < num_runtime; i++) {
	if (!(FUNCTION_FLAGS(i) & NAME_INHERITED)) {
	    int oldix = FUNCTION_RENTRY(i)->def.f_index;
	    DEBUG_CHECK(oldix >= num, "Function index out of range");
	    FUNCTION_RENTRY(i)->def.f_index = inverse[oldix];
	}
    }
#endif

    *p += align(new_num * sizeof(compiler_function_t));

    if (mem_block[A_ARGUMENT_TYPES].current_size) {
	unsigned short *dest;

	prog->argument_types = (unsigned short *) *p;
	copy_in(A_ARGUMENT_TYPES, p);

	dest = prog->type_start = (unsigned short *) *p;
	for (i = 0; i < new_num; i++)
	    dest[i] = *((unsigned short *)mem_block[A_ARGUMENT_INDEX].block + temp[i]);
	
	*p += align(new_num * sizeof(unsigned short));
    } else {
	prog->argument_types = 0;
	prog->type_start = 0;
    }

    /* Free function entries that have been discarded */
    for (i = new_num; i < num; i++) {
	compiler_function_t *funp = COMPILER_FUNC(temp[i]);
	if (funp->name)
	    free_string(funp->name);
    }

    FREE(temp);
    FREE(inverse);
}

#ifdef COMPRESS_FUNCTION_TABLES
#define EXPECTED_INDEX(rfu) (INHERIT(rfu->inh.offset)->function_index_offset + rfu->inh.function_index_offset)

void compress_function_tables() {
    compressed_offset_table_t *cftp;
    runtime_function_u *p;
    int f_ov, l_ov, f_def, n_def, n_tot, n_ov, i, j;

    n_tot = (mem_block[A_RUNTIME_FUNCTIONS].current_size / sizeof(runtime_function_u));
    f_def = n_tot - 1;
    while (f_def >= 0) {
	runtime_function_u *rfu = FUNCTION_RENTRY(f_def);
	if ((FUNCTION_FLAGS(f_def) & NAME_INHERITED) &&
	    f_def == EXPECTED_INDEX(rfu))
	    break;
	f_def--;
    }
    f_def++;
    n_def = n_tot - f_def;
    
    f_ov = 0;
    while (f_ov < f_def && (FUNCTION_FLAGS(f_ov) & NAME_INHERITED)) {
	runtime_function_u *rfu = FUNCTION_RENTRY(f_ov);
	if (f_ov != EXPECTED_INDEX(rfu))
	    break;
	f_ov++;
    }
    
    l_ov = f_def - 1;
    while (l_ov > f_ov && (FUNCTION_FLAGS(l_ov) & NAME_INHERITED)) {
	runtime_function_u *rfu = FUNCTION_RENTRY(l_ov);
	if (l_ov != EXPECTED_INDEX(rfu))
	    break;
	l_ov--;
    }

    n_ov = l_ov - f_ov + 1;
    cftp = (compressed_offset_table_t*)allocate_in_mem_block(A_RUNTIME_COMPRESSED, sizeof(compressed_offset_table_t) + (n_ov - 1));
    
    cftp->first_defined = f_def;
    cftp->first_overload = f_ov;
    cftp->num_compressed = f_def - n_ov;

    for (i = 0, j = 0; i < n_ov; i++) {
	int ri = f_ov + i;
	runtime_function_u *rfu = FUNCTION_RENTRY(ri);
	if ((FUNCTION_FLAGS(ri) & NAME_INHERITED) && ri == EXPECTED_INDEX(rfu))
	    cftp->index[i] = 255;
	else {
	    cftp->index[i] = j++;
	    if (j == 256) {
		/* Woops.  Fix things up a bit */
		cftp->first_defined = f_def = f_ov + i;
		cftp->num_compressed = i;
		for (j = i; j < n_ov; j++)
		    cftp->index[j] = 255;
		j = 255;
		break;
	    }
	}
    }
    cftp->num_deleted = cftp->first_defined - j;

    /* Now modify A_RUNTIME_FUNCTIONS */
    if (j + n_def == 0) {
	mem_block[A_RUNTIME_FUNCTIONS].current_size = 0;
    } else if (f_def) {
	p = (runtime_function_u *)DXALLOC((j + n_def) * sizeof(runtime_function_u), TAG_COMPILER, "compress_function_table");
	for (i = 0; i < n_ov; i++) {
	    if (cftp->index[i] == 255)
		continue;
	    p[cftp->index[i]] = *FUNCTION_RENTRY(f_ov + i);
	}
	for (i = 0; i < n_def; i++) {
	    p[i + j] = *FUNCTION_RENTRY(f_def + i);
	}
	FREE(mem_block[A_RUNTIME_FUNCTIONS].block);
	mem_block[A_RUNTIME_FUNCTIONS].block = (char *)p;
	mem_block[A_RUNTIME_FUNCTIONS].current_size =
	    (j + n_def) * sizeof(runtime_function_u);
    }
}
#endif

/*
 * The program has been compiled. Prepare a 'program_t' to be returned.
 */
static program_t *epilog() {
    int size, i, lnsz, lnoff;
    char *p;
    int num_fun;
    ident_hash_elem_t *ihe;
    program_t *prog;
    
    if (num_parse_error > 0 || inherit_file) {
	/* don't print these; they can be wrong, since we didn't parse the
	   entire file */
	if (pragmas & PRAGMA_WARNINGS)
	    remove_overload_warnings(0);
	clean_parser();
	end_new_file();
	free_string(current_file);
	current_file = 0;
	return 0;
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
				  NAME_STRICT_TYPES | NAME_PRIVATE, TYPE_VOID);
	COMPILER_FUNC(fun)->address = CURRENT_PROGRAM_SIZE;
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
    num_fun = mem_block[A_RUNTIME_FUNCTIONS].current_size / sizeof(runtime_function_u);
    for (i = 0; i < num_fun; i++) {
	int funflags = FUNCTION_FLAGS(i);
	runtime_function_u *func = FUNCTION_RENTRY(i);

	/* Look for functions not defined at this level, but defined below */
	if ((funflags & NAME_UNDEFINED) && (funflags & NAME_DEF_BY_INHERIT)) {
	    /*If it's a real function, make it a real function at this level*/
	    if (!(funflags & (NAME_PROTOTYPE|NAME_ALIAS)))
		funflags &= ~NAME_UNDEFINED;
	    /* Mark it as inherited */
	    FUNCTION_FLAGS(i) = funflags | NAME_INHERITED;
	}
	if (funflags & NAME_ALIAS) {
	    int which = FUNCTION_ALIAS(i);
	    if (!(FUNCTION_FLAGS(which) & NAME_INHERITED)
		|| FUNCTION_ALIAS(which) >= 2) {
		*func = *FUNCTION_RENTRY(which);
		FUNCTION_FLAGS(i) = FUNCTION_FLAGS(which) | NAME_ALIAS;
	    }
	}
    }
    generate_final_program(1);

    size = align(sizeof (program_t));

    /* delete argument information if we're not saving it */
    if (!(pragmas & PRAGMA_SAVE_TYPES))
	mem_block[A_ARGUMENT_TYPES].current_size = 0;
    if (!(mem_block[A_ARGUMENT_TYPES].current_size))
	mem_block[A_ARGUMENT_INDEX].current_size = 0;

#ifdef COMPRESS_FUNCTION_TABLES
    compress_function_tables();
#endif

    for (i=0; i<NUMPAREAS; i++)
	if (i != A_LINENUMBERS && i != A_FILE_INFO)
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

    prog->num_functions_total = num_fun;

    prog->function_table = (compiler_function_t *)p;
    copy_and_sort_function_table(prog, &p);

    /* Warning: copy_and_sort_function_table modifies A_RUNTIME_FUNCTIONS,
       so this must be copied in AFTER the call. */
    prog->function_offsets = (runtime_function_u *)p;
    copy_in(A_RUNTIME_FUNCTIONS, &p);

#ifdef COMPRESS_FUNCTION_TABLES
    prog->function_compressed = (compressed_offset_table_t *)p;
    copy_in(A_RUNTIME_COMPRESSED, &p);
#endif

    prog->function_flags = (unsigned short *)p;
    copy_in(A_FUNCTION_FLAGS, &p);
    
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

    prog->num_variables_defined = mem_block[A_VAR_NAME].current_size /
	sizeof (char *);
    prog->num_variables_total = mem_block[A_VAR_TEMP].current_size /
	  sizeof (variable_t);

    prog->variable_table = (char **)p;
    copy_in(A_VAR_NAME, &p);
    prog->variable_types = (unsigned short *)p;
    copy_in(A_VAR_TYPE, &p);

    prog->num_inherited = mem_block[A_INHERITS].current_size /
	  sizeof (inherit_t);
    if (prog->num_inherited) {
	prog->inherit = (inherit_t *)p;
	copy_in(A_INHERITS, &p);
    } else
	prog->inherit = 0;

#ifdef DEBUG
    if (p - (char *)prog != size) {
	fprintf(stderr, "Program size miscalculated for /%s.\n", prog->name);
	fprintf(stderr, "is: %i, expected: %i\n", p-(char *)prog, size);
    }
#endif
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
    release_tree();
    scratch_destroy();
    clean_up_locals();
    free_unused_identifiers();
    end_new_file();

    return prog;
}

/*
 * Initialize the environment that the compiler needs.
 */
static void prolog P2(int, f, char *, name) {
    int i;

    function_context.num_parameters = -1;
    num_parse_error = 0;
    global_modifiers = 0;
    var_defined = 0;
    
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
    int i, n;
    compiler_function_t *funp;
    
    /* don't need the parse trees any more */
    release_tree();
    
    /*
     * Free function stuff.
     */
    for (i = 0; i < mem_block[A_COMPILER_FUNCTIONS].current_size / sizeof(*funp); i++) {
	funp = COMPILER_FUNC(i);
	if (funp->name)
	    free_string(funp->name);
    }
    n = mem_block[A_STRINGS].current_size / sizeof(char *);
    for (i = 0; i < n; i++) {
	free_string(*((char **)mem_block[A_STRINGS].block + i));
    }
    n = mem_block[A_VAR_NAME].current_size / sizeof(char *);
    for (i = 0; i < n; i++) {
	free_string(*((char **)mem_block[A_VAR_NAME].block + i));
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
    int i1, i2;
    char *p1, *p2;
    
    if ((*c1)->kind == NODE_DEFAULT)
	return -1;
    if ((*c2)->kind == NODE_DEFAULT)
	return 1;

    i1 = (*c1)->r.number;
    i2 = (*c2)->r.number;
    p1 = (i1 ? PROG_STRING(i1) : 0);
    p2 = (i2 ? PROG_STRING(i2) : 0);
    
    return (p1 - p2);
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
	    char *end = EndOf(buf);
	    char *p;
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

	    p = strput(buf, end, "Overlapping cases: ");
	    if (f1) {
		p = strput(p, end, f1);
		p = strput(p, end, ":");
	    } else
		p = strput(p, end, "line ");
	    p = strput(p, end, " and ");
	    if (f2) {
		p = strput(p, end, f2);
		p = strput(p, end, ":");
	    } else
		p = strput(p, end, "line ");
	    p = strput(p, end, ".");
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
