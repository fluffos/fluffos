/*
	efuns_main.c: this file contains the efunctions called from
	inside eval_instruction() in interpret.c.  Note: if you are adding
    local efunctions that are specific to your driver, you would be better
    off adding them to a separate source file.  Doing so will make it much
    easier for you to upgrade (won't have to patch this file).  Be sure
    to #include "efuns.h" in that separate source file.
*/

#include "config.h"

#include "efuns.h"
#include "stralloc.h"
#include "include/origin.h"
#include "ed.h"

#if defined(__386BSD__) || defined(SunOS_5)
#include <unistd.h>
#endif
#if defined(LATTICE)
#include <stdlib.h>
#include <dos.h>
#include <amiga.h>
#endif

#include "lint.h"
#include "applies.h"

static struct object *ob;

int using_bsd_malloc = 0;
int using_smalloc = 0;
int call_origin = 0;

int data_size PROT((struct object * ob));
void reload_object PROT((struct object * obj));

#ifdef F_ADD_ACTION
void
f_add_action P2(int, num_arg, int, instruction)
{
    struct svalue *arg;

    arg = sp - num_arg + 1;
    if (num_arg == 3) {
	CHECK_TYPES(&arg[2], T_NUMBER, 3, instruction);
    }
    if (num_arg > 1 && arg[1].type == T_POINTER) {
	int i,n;
	n = arg[1].u.vec->size;
	for (i=0; i<n; i++) {
	    if (arg[1].u.vec->item[i].type == T_STRING) {
		add_action(arg[0].u.string,
			   arg[1].u.vec->item[i].u.string,
			   num_arg > 2 ? arg[2].u.number : 0);
	    }
	}
    } else {
	add_action(arg[0].u.string,
		   num_arg > 1 ? arg[1].u.string : 0,
		   num_arg > 2 ? arg[2].u.number : 0);
    }
    pop_n_elems(num_arg - 1);
}
#endif

#ifdef F_ALL_INVENTORY
void
f_all_inventory P2(int, num_arg, int, instruction)
{
    struct vector *vec;

    vec = all_inventory(sp->u.ob, 0);
    if (!vec) {
	assign_svalue(sp, &const0);
	return;
    }
    pop_stack();
    push_refed_vector(vec);
}
#endif

#ifdef F_ALLOCATE
void
f_allocate P2(int, num_arg, int, instruction)
{
    struct vector *vec;

    vec = allocate_array(sp->u.number);
    pop_stack();
    push_refed_vector(vec);
}
#endif

#ifdef F_ALLOCATE_BUFFER
void
f_allocate_buffer P2(int, num_arg, int, instruction)
{
    struct buffer *buf;

    buf = allocate_buffer(sp->u.number);
    if (buf) {
	pop_stack();
	push_refed_buffer(buf);
    } else {
	assign_svalue(sp, &const0);
    }
}
#endif

#ifdef F_ALLOCATE_MAPPING
void
f_allocate_mapping P2(int, num_arg, int, instruction)
{
    struct mapping *map;

    map = allocate_mapping(sp->u.number);
    pop_stack();
    push_refed_mapping(map);
}
#endif

#ifdef F_BREAK_STRING
void
f_break_string P2(int, num_arg, int, instruction)
{
    struct svalue *arg = sp - num_arg + 1;
    char *str;
    extern char *break_string PROT((char *, int, struct svalue *));

    if (arg[0].type == T_STRING) {
	str = break_string(arg[0].u.string, arg[1].u.number,
			   (num_arg > 2 ? &arg[2] : (struct svalue *) 0));
	pop_n_elems(num_arg);
	push_malloced_string(str);
    } else {
	pop_n_elems(num_arg);
	push_number(0);
    }
}
#endif

#ifdef F_CACHE_STATS
void print_cache_stats()
{
    add_message("Function cache information\n");
    add_message("-------------------------------\n");
    add_message("%% cache hits:    %10.2f\n",
	     100 * ((double) apply_low_cache_hits / apply_low_call_others));
    add_message("call_others:     %10lu\n", apply_low_call_others);
    add_message("cache hits:      %10lu\n", apply_low_cache_hits);
    add_message("cache size:      %10lu\n", APPLY_CACHE_SIZE);
    add_message("slots used:      %10lu\n", apply_low_slots_used);
    add_message("%% slots used:    %10.2f\n",
		100 * ((double) apply_low_slots_used / APPLY_CACHE_SIZE));
    add_message("collisions:      %10lu\n", apply_low_collisions);
    add_message("%% collisions:    %10.2f\n",
	     100 * ((double) apply_low_collisions / apply_low_call_others));
}

void f_cache_stats P2(int, num_arg, int, instruction)
{
    print_cache_stats();
    push_number(0);
}
#endif

#ifdef F_CALL_OTHER
 /* enhanced call_other written 930314 by Luke Mewburn <zak@rmit.edu.au> */
void
f_call_other P2(int, num_arg, int, instruction)
{
    struct svalue *arg;
    char *funcname;
#ifndef NEW_FUNCTIONS
    int i;
#endif

    if (current_object->flags & O_DESTRUCTED) {	/* No external calls allowed */
	pop_n_elems(num_arg);
	push_undefined();
	return;
    }
    arg = sp - num_arg + 1;
    if (arg[1].type == T_STRING)
	funcname = arg[1].u.string;
    else {			/* must be T_POINTER then */
	check_for_destr(arg[1].u.vec);
	if ((arg[1].u.vec->size < 1)
	    || (arg[1].u.vec->item[0].type != T_STRING))
	    error("call_other: 1st elem of array for arg 2 must be a string\n");
	funcname = arg[1].u.vec->item[0].u.string;	/* complicated huh? */
#ifdef NEW_FUNCTIONS
	num_arg = 2 + merge_arg_lists(num_arg - 2, arg[1].u.vec, 1);
#else
	for (i = 1; i < arg[1].u.vec->size; i++)
	    push_svalue(&arg[1].u.vec->item[i]);
	num_arg += i - 1;
#endif
    }
    if (arg[0].type == T_OBJECT)
	ob = arg[0].u.ob;
    else if (arg[0].type == T_POINTER) {
	struct vector *ret;

	ret = call_all_other(arg[0].u.vec, funcname, num_arg - 2);
	pop_2_elems();
	push_refed_vector(ret);
	return;
    } else {
	ob = find_object(arg[0].u.string);
	if (!ob || !object_visible(ob))
	    error("call_other() couldn't find object\n");
    }
    /* Send the remaining arguments to the function. */
#ifdef TRACE
    if (TRACEP(TRACE_CALL_OTHER)) {
	do_trace("Call other ", funcname, "\n");
    }
#endif
    call_origin = ORIGIN_CALL_OTHER;
    if (apply_low(funcname, ob, num_arg - 2) == 0) {	/* Function not found */
	pop_2_elems();
	push_undefined();
	return;
    }
    /*
     * The result of the function call is on the stack.  So is the function
     * name and object that was called, though. These have to be removed.
     */
    free_svalue(sp - 2, "f_call_other");
    *(sp - 2) = *sp;
    sp--;
    pop_stack();
    return;
}
#endif

#ifdef F_CALL_OUT
void
f_call_out P2(int, num_arg, int, instruction)
{
    struct svalue *arg;

    arg = sp - num_arg + 1;
    if (!(current_object->flags & O_DESTRUCTED))
	new_call_out(current_object, arg[0].u.string, arg[1].u.number,
		     num_arg - 3, (num_arg >= 3) ? &arg[2] : 0);
    pop_n_elems(num_arg);
    push_number(0);
}
#endif

#ifdef F_CALL_OUT_INFO
void
f_call_out_info P2(int, num_arg, int, instruction)
{
    push_refed_vector(get_all_call_outs());
}
#endif

#ifdef F_CAPITALIZE
void
f_capitalize P2(int, num_arg, int, instruction)
{
    char *str;

    str = sp->u.string;
    if (islower(str[0])) {
	if (sp->subtype == STRING_MALLOC) {
	    str[0] += 'A' - 'a';
	} else {
	    str = string_copy(str);
	    str[0] += 'A' - 'a';
	    pop_stack();
	    push_malloced_string(str);
	}
    }
    return;
}
#endif

#ifdef F_CHILDREN
void
f_children P2(int, num_arg, int, instruction)
{
    struct vector *vec;

    vec = children(sp->u.string);
    pop_stack();
    if (!vec)
	push_number(0);
    else {
	push_refed_vector(vec);
    }
}
#endif

#ifdef F_CLEAR_BIT
void
f_clear_bit P2(int, num_arg, int, instruction)
{
    char *str;
    int len, ind;

    if (sp->u.number > MAX_BITS)
	error("clear_bit: %d > MAX_BITS\n", sp->u.number);
    len = SVALUE_STRLEN(sp - 1);
    ind = sp->u.number / 6;
    if (ind >= len) {		/* return first arg unmodified */
	pop_stack();
	return;
    }
    str = DXALLOC(len + 1, 29, "f_clear_bit: str");
    memcpy(str, (sp - 1)->u.string, len + 1);	/* including null byte */
    if (str[ind] > 0x3f + ' ' || str[ind] < ' ')
	error("Illegal bit pattern in clear_bit character %d\n", ind);
    str[ind] = ((str[ind] - ' ') & ~(1 << (sp->u.number % 6))) + ' ';
    pop_2_elems();
    push_malloced_string(str);
}
#endif

#ifdef F_CLONEP
void
f_clonep P2(int, num_arg, int, instruction)
{
    if ((sp->type == T_OBJECT) && (sp->u.ob->flags & O_CLONE))
	assign_svalue(sp, &const1);
    else
	assign_svalue(sp, &const0);
}
#endif

#ifdef F_COMMAND
void
f_command P2(int, num_arg, int, instruction)
{
    struct svalue *arg;
    int i;

    arg = sp - num_arg + 1;
#ifdef OLD_COMMAND
    i = command_for_object(arg[0].u.string, (num_arg == 2) ? arg[1].u.ob : 0);
#else
    if ((num_arg == 2) && (arg[1].u.ob != current_object)) {
	error("Use command(cmd) or command(cmd, this_object()).\n");
    }
    i = command_for_object(arg[0].u.string, 0);
#endif
    pop_n_elems(num_arg);
    push_number(i);
}
#endif

#ifdef F_COMMANDS
void
f_commands P2(int, num_arg, int, instruction)
{
    struct vector *vec;

    vec = commands(current_object);
    push_refed_vector(vec);
}
#endif

#ifdef F_CP
void
f_cp P2(int, num_arg, int, instruction)
{
    int i;

    i = copy_file(sp[-1].u.string, sp[0].u.string);
    pop_2_elems();
    push_number(i);
}
#endif

#ifdef F_CRC32
void
f_crc32 P2(int, num_arg, int, instruction)
{
    int len;
    unsigned char *buf;
    UINT32 crc;

    if (sp->type == T_STRING) {
	len = strlen(sp->u.string);
	buf = (unsigned char *) sp->u.string;
    } else if (sp->type == T_BUFFER) {
	len = sp->u.buf->size;
	buf = sp->u.buf->item;
    } else {
	bad_argument(sp, T_STRING | T_BUFFER, 1, instruction);
    }
    crc = compute_crc32(buf, len);
    pop_stack();
    push_number(crc);
}
#endif

#ifdef F_CREATOR
void
f_creator P2(int, num_arg, int, instruction)
{
    ob = sp->u.ob;
    if (ob->uid == 0) {
	assign_svalue(sp, &const0);
    } else {
	pop_stack();
	push_string(ob->uid->name, STRING_CONSTANT);
    }
}
#endif				/* CREATOR */

#ifdef F_CTIME
void
f_ctime P2(int, num_arg, int, instruction)
{
    char *cp;

    cp = string_copy(time_string(sp->u.number));
    pop_stack();
    push_malloced_string(cp);
    /* Now strip the newline. */
    cp = strchr(cp, '\n');
    if (cp)
	*cp = '\0';
}
#endif

#ifdef F_DEEP_INHERIT_LIST
void
f_deep_inherit_list P2(int, num_arg, int, instruction)
{
    struct vector *vec;
    extern struct vector *deep_inherit_list PROT((struct object *));

    if (!(sp->u.ob->flags & O_SWAPPED)) {
	vec = deep_inherit_list(sp->u.ob);
    } else {
	vec = null_array();
    }
    pop_stack();
    push_refed_vector(vec);
}
#endif

#ifdef DEBUG_MACRO

#ifdef F_SET_DEBUG_LEVEL
void
f_set_debug_level P2(int, num_arg, int, instruction)
{
    debug_level = sp->u.number;
    pop_stack();
    push_number(0);
}
#endif

#endif

#ifdef F_DEEP_INVENTORY
void
f_deep_inventory P2(int, num_arg, int, instruction)
{
    struct vector *vec;

    vec = deep_inventory(sp->u.ob, 0);
    pop_stack();
    push_refed_vector(vec);
}
#endif

#ifdef F_DESTRUCT
void
f_destruct P2(int, num_arg, int, instruction)
{
    destruct_object(sp);
    pop_stack();
    push_number(1);
}
#endif

#ifdef F_DISABLE_COMMANDS
void
f_disable_commands P2(int, num_arg, int, instruction)
{
    enable_commands(0);
    push_number(0);
}
#endif

#ifdef F_DUMPALLOBJ
void
f_dumpallobj P2(int, num_arg, int, instruction)
{
    char *fn = "/OBJ_DUMP";

    if (num_arg == 1) {
	fn = sp->u.string;
    }
    dumpstat(fn);
    if (num_arg == 1) {
	pop_stack();
    }
    push_number(0);
}
#endif

/* f_each */

#ifdef F_EACH
void
f_each P2(int, num_arg, int, instruction)
{
    struct mapping *m;
    struct vector *v;
    int flag;

    m = (sp - 1)->u.map;
    flag = sp->u.number;
    if (flag) {
	m->eachObj = current_object;
	m->bucket = 0;
	m->elt = (struct node *) 0;
	pop_stack();
	assign_svalue(sp, &const0);
	return;
    }
    v = mapping_each(m);
    pop_2_elems();
    push_refed_vector(v);
}
#endif

#ifdef F_ED
void
f_ed P2(int, num_arg, int, instruction)
{
    if (command_giver == 0 || command_giver->interactive == 0) {
        if (num_arg)
            pop_n_elems(num_arg - 1);
        else
            push_number(0);
        return;
    }

    if (num_arg == 0) {
        /* ed() */
	ed_start(0, 0, 0, 0, 0);
	push_number(1);
    } else if (num_arg == 1) {
        /* ed(fname) */
	ed_start(sp->u.string, 0, 0, 0, 0);
    } else if (num_arg == 2) {
        /* ed(fname,exitfn) */
	ed_start((sp - 1)->u.string, 0, sp->u.string, 0, current_object);
	pop_stack();
    } else if (num_arg == 3) {
        /* ed(fname,exitfn,restricted) / ed(fname,writefn,exitfn) */
        if (sp->type == T_NUMBER) {
	    ed_start((sp - 2)->u.string, 0, (sp - 1)->u.string, sp->u.number,
		     current_object);
	} else if (sp->type == T_STRING) {
	    ed_start((sp - 2)->u.string, (sp - 1)->u.string, sp->u.string, 0,
	             current_object);
	} else {
            bad_argument(sp, T_NUMBER | T_STRING, 3, instruction);
	}
	pop_2_elems();
    } else {                    /* num_arg == 4 */
        /* ed(fname,writefn,exitfn,restricted) */
        if ((sp - 1)->type != T_STRING)
            bad_argument(sp - 1, T_STRING, 3, instruction);
        if (sp->type != T_NUMBER)
            bad_argument(sp, T_NUMBER, 4, instruction);
        ed_start((sp - 3)->u.string, (sp - 2)->u.string, (sp - 1)->u.string,
                 sp->u.number, current_object);
    }
}
#endif

#ifdef F_ENABLE_COMMANDS
void
f_enable_commands P2(int, num_arg, int, instruction)
{
    enable_commands(1);
    push_number(1);
}
#endif

#ifdef F_ENABLE_WIZARD
void
f_enable_wizard P2(int, num_arg, int, instruction)
{
    if (current_object->interactive) {
	current_object->flags |= O_IS_WIZARD;
	push_number(1);
    } else
	push_number(0);
}
#endif

#ifdef F_ERROR
void
f_error P2(int, num_arg, int, instruction)
{
    error(sp->u.string);
}
#endif

#ifdef F_ERRORP
void
f_errorp P2(int, num_arg, int, instruction)
{
    int i;

    if ((sp->type == T_NUMBER) && (sp->subtype == T_ERROR)) {
	i = 1;
    } else {
	i = 0;
    }
    pop_stack();
    push_number(i);
}
#endif

#ifdef F_DISABLE_WIZARD
void
f_disable_wizard P2(int, num_arg, int, instruction)
{
    if (current_object->interactive) {
	current_object->flags &= ~O_IS_WIZARD;
	push_number(1);
    } else
	push_number(0);
}
#endif

#ifdef F_ENVIRONMENT
void
f_environment P2(int, num_arg, int, instruction)
{
    if (num_arg) {
	ob = environment(sp);
	pop_stack();
    } else if (!(current_object->flags & O_DESTRUCTED))
	ob = current_object->super;
    else
	ob = 0;
    if (ob)
	push_object(ob);
    else
	push_number(0);
}
#endif

#ifdef F_EXEC
void
f_exec P2(int, num_arg, int, instruction)
{
    int i;

    i = replace_interactive((sp - 1)->u.ob, sp->u.ob);
    pop_2_elems();
    push_number(i);
}
#endif

#ifdef F_EXPLODE
void
f_explode P2(int, num_arg, int, instruction)
{
    struct vector *vec;

    vec = explode_string((sp - 1)->u.string, sp->u.string);
    pop_2_elems();
    if (vec) {
	push_refed_vector(vec);
    } else
	push_number(0);
}
#endif

#ifdef F_FILE_NAME
void
f_file_name P2(int, num_arg, int, instruction)
{
    char *name, *res;

    /* This function now returns a leading '/' */
    name = sp->u.ob->name;
    res = (char *) add_slash(name);
    pop_stack();
    push_malloced_string(res);
}
#endif

#ifdef F_FILE_SIZE
void
f_file_size P2(int, num_arg, int, instruction)
{
    int i;

    i = file_size(sp->u.string);
    pop_stack();
    push_number(i);
}
#endif

#ifdef F_FIND_CALL_OUT
void
f_find_call_out P2(int, num_arg, int, instruction)
{
    int i;

    i = find_call_out(current_object, sp->u.string);
    pop_stack();
    push_number(i);
}
#endif

#ifdef F_FIND_LIVING
void
f_find_living P2(int, num_arg, int, instruction)
{
    ob = find_living_object(sp->u.string, 0);
    pop_stack();
    if (!ob)
	push_number(0);
    else
	push_object(ob);
}
#endif

#ifdef F_FIND_OBJECT
void
f_find_object P2(int, num_arg, int, instruction)
{
    if ((sp--)->u.number)
	ob = find_object(sp->u.string);
    else
	ob = find_object2(sp->u.string);
    pop_stack();
    if (ob) {
	if (object_visible(ob))
	    push_object(ob);
	else
	    push_number(0);
    } else
	push_number(0);
}
#endif

#ifdef F_FIND_PLAYER
void
f_find_player P2(int, num_arg, int, instruction)
{
    ob = find_living_object(sp->u.string, 1);
    pop_stack();
    if (!ob)
	push_number(0);
    else
	push_object(ob);
}
#endif

#ifdef F_FUNCTION_PROFILE
/* f_function_profile: John Garnett, 1993/05/31, 0.9.17.3 */
void
f_function_profile P2(int, num_arg, int, instruction)
{
    struct vector *vec;
    struct mapping *map;
    struct program *prog;
    int nf, j;

    ob = sp->u.ob;
    if (ob->flags & O_SWAPPED) {
	load_ob_from_swap(ob);
    }
    prog = ob->prog;
    nf = prog->p.i.num_functions;
    vec = allocate_array(nf);
    for (j = 0; j < nf; j++) {
	map = allocate_mapping(3);
	add_mapping_pair(map, "calls", prog->p.i.functions[j].calls);
	add_mapping_pair(map, "self", prog->p.i.functions[j].self
			 - prog->p.i.functions[j].children);
	add_mapping_pair(map, "children", prog->p.i.functions[j].children);
	add_mapping_shared_string(map, "name", prog->p.i.functions[j].name);
	vec->item[j].type = T_MAPPING;
	vec->item[j].u.map = map;
    }
    pop_stack();
    push_refed_vector(vec);
}
#endif

#ifdef F_FUNCTION_EXISTS
void
f_function_exists P2(int, num_arg, int, instruction)
{
    char *str, *res;

    str = function_exists((sp - 1)->u.string, sp->u.ob);
    pop_2_elems();
    if (str) {
	res = (char *) add_slash(str);
	if ((str = strrchr(res, '.')))
	    *str = 0;
	push_malloced_string(res);
    } else {
	push_number(0);
    }
}
#endif

#ifdef F_GENERATE_SOURCE
void f_generate_source P2(int, num_arg, int, instruction)
{
    int i;

    if (num_arg == 2) {
	i = generate_source((sp - 1)->u.string, sp->u.string);
	pop_stack();
    } else
	i = generate_source(sp->u.string, 0);
    free_svalue(sp, "f_generate_source");
    sp->type = T_NUMBER;
    sp->subtype = 0;
    sp->u.number = i;
}
#endif

#ifdef F_GET_CHAR
void
f_get_char P2(int, num_arg, int, instruction)
{
    struct svalue *arg;
    int i, tmp;
    int flag;

    arg = sp - num_arg + 1;	/* Points arg at first argument. */
    if (num_arg == 1 || (arg[1].type != T_NUMBER)) {
	tmp = 0;
	flag = 0;
    } else {
	tmp = 1;
	num_arg--;		/* Don't count the flag as an arg */
	flag = arg[1].u.number;
    }
    num_arg--;
    i = get_char(arg[0].u.string, flag, num_arg, &arg[1 + tmp]);
    free_svalue(arg, "f_get_char");
    sp = arg;
    sp->type = T_NUMBER;
    sp->u.number = i;
}
#endif

#ifdef F_GET_CONFIG
void
f_get_config P2(int, num_arg, int, instruction)
{
    if (!get_config_item(sp, sp))
	error("Bad argument to get_config()\n");
}
#endif

#ifdef F_GET_DIR
void
f_get_dir P2(int, num_arg, int, instruction)
{
    struct vector *vec;

    vec = get_dir((sp - 1)->u.string, sp->u.number);
    pop_2_elems();
    if (vec) {
	push_refed_vector(vec);
    } else
	push_number(0);
}
#endif

#ifdef F_IMPLODE
void
f_implode P2(int, num_arg, int, instruction)
{
    char *str;

    check_for_destr((sp - 1)->u.vec);
    str = implode_string((sp - 1)->u.vec, sp->u.string);
    pop_2_elems();
    if (str)
	push_malloced_string(str);
    else
	push_number(0);
}
#endif

#ifdef F_IN_EDIT
void
f_in_edit P2(int, num_arg, int, instruction)
{
    char *fn;

    if (sp->u.ob->interactive && sp->u.ob->interactive->ed_buffer
	&& (fn = sp->u.ob->interactive->ed_buffer->fname)) {
	pop_stack();
	push_string(fn, STRING_CONSTANT);
    } else {
	pop_stack();
	push_number(0);
    }
}
#endif

#ifdef F_IN_INPUT
void
f_in_input P2(int, num_arg, int, instruction)
{
    int i;

    i = sp->u.ob->interactive && sp->u.ob->interactive->input_to;
    pop_stack();
    if (i)
	push_number(1);
    else
	push_number(0);
}
#endif

#ifdef F_INHERITS
int
inherits P2(struct program *, prog, struct program *, thep)
{
    int j;

    for (j = 0; j < (int) prog->p.i.num_inherited; j++) {
	if (prog->p.i.inherit[j].prog == thep)
	    return 1;
	if (!strcmp(prog->p.i.inherit[j].prog->name, thep->name))
	    return 2;
	if (inherits(prog->p.i.inherit[j].prog, thep))
	    return 1;
    }
    return 0;
}

void
f_inherits P2(int, num_arg, int, instruction)
{
    struct object *ob, *base;
    int i;

    ob = find_object2((sp - 1)->u.string);
    base = sp->u.ob;
    if (IS_ZERO(sp) || !base || !ob || (ob->flags & O_SWAPPED)) {
	pop_stack();
	assign_svalue(sp, &const0);
	return;
    }
    if (base->flags & O_SWAPPED)
	load_ob_from_swap(base);
    i = inherits(base->prog, ob->prog);
    pop_stack();
    pop_stack();
    push_number(i);
}
#endif

#ifdef F_INHERIT_LIST
void
f_inherit_list P2(int, num_arg, int, instruction)
{
    struct vector *vec;
    extern struct vector *inherit_list PROT((struct object *));


    if (!(sp->u.ob->flags & O_SWAPPED)) {
	vec = inherit_list(sp->u.ob);
    } else {
	vec = null_array();
    }
    pop_stack();
    push_refed_vector(vec);
}
#endif

#ifdef F_INPUT_TO
void
f_input_to P2(int, num_arg, int, instruction)
{
    struct svalue *arg;
    int i, tmp;
    int flag;

    arg = sp - num_arg + 1;	/* Points arg at first argument. */
    if ((num_arg == 1) || (arg[1].type != T_NUMBER)) {
	tmp = 0;
	flag = 0;
    } else {
	tmp = 1;
	num_arg--;		/* Don't count the flag as an arg */
	flag = arg[1].u.number;
    }
    num_arg--;			/* Don't count the name of the func either. */
    i = input_to(arg[0].u.string, flag, num_arg, &arg[1 + tmp]);
    free_svalue(arg, "f_input_to");
    sp = arg;
    sp->type = T_NUMBER;
    sp->u.number = i;
}
#endif

#ifdef F_INTERACTIVE
void
f_interactive P2(int, num_arg, int, instruction)
{
    int i;

    i = ((int) sp->u.ob->interactive != 0);
    pop_stack();
    push_number(i);
}
#endif

#ifdef F_INTP
void
f_intp P2(int, num_arg, int, instruction)
{
    if (sp->type == T_NUMBER)
	assign_svalue(sp, &const1);
    else
	assign_svalue(sp, &const0);
}
#endif

#ifdef F_FUNCTIONP
void
f_functionp P2(int, num_arg, int, instruction)
{
#ifdef NEW_FUNCTIONS
    int i;
#endif
    
    if (sp->type == T_FUNCTION) {
#ifdef NEW_FUNCTIONS
	i = sp->u.fp->type;
	if (sp->u.fp->args.type == T_POINTER) i |= 1;
	pop_stack();
	push_number(i);
	return;
#else
	if (((sp->u.fp->obj.type == T_OBJECT) &&
	     !(sp->u.fp->obj.u.ob->flags & O_DESTRUCTED)) ||
	    (sp->u.fp->obj.type == T_STRING))
	    {
		if (sp->u.fp->fun.type == T_STRING) {
		    assign_svalue(sp, &const1);
		    return;
		} else if (sp->u.fp->fun.type == T_POINTER) {
		    pop_stack();
		    push_number(2);
		    return;
		}
	    }
#endif
    }
    assign_svalue(sp, &const0);
}
#endif

#ifdef F_KEYS
void
f_keys P2(int, num_arg, int, instruction)
{
    struct vector *vec;
    
    vec = mapping_indices(sp->u.map);
    pop_stack();
    push_refed_vector(vec);
}
#endif

#ifdef F_VALUES
void
f_values P2(int, num_arg, int, instruction)
{
    struct vector *vec;

    vec = mapping_values(sp->u.map);
    pop_stack();
    push_refed_vector(vec);
}
#endif

#ifdef F_LINK
void
f_link P2(int, num_arg, int, instruction)
{
    struct svalue *ret;
    int i;

    push_string((sp - 1)->u.string, STRING_CONSTANT);
    push_string(sp->u.string, STRING_CONSTANT);
    ret = apply_master_ob(APPLY_VALID_LINK, 2);
    if (MASTER_APPROVED(ret))
	i = do_rename((sp - 1)->u.string, sp->u.string, F_LINK);
    pop_2_elems();
    push_number(i);
}
#endif				/* F_LINK */

#ifdef F_LIVING
void
f_living P2(int, num_arg, int, instruction)
{
    if (sp->u.ob->flags & O_ENABLE_COMMANDS)
	assign_svalue(sp, &const1);
    else
	assign_svalue(sp, &const0);
}
#endif

#ifdef F_LIVINGS
void
f_livings P2(int, num_arg, int, instruction)
{
    push_refed_vector(livings());
}
#endif

#ifdef F_LOWER_CASE
void
f_lower_case P2(int, num_arg, int, instruction)
{
    char *str;

    if (sp->subtype == STRING_MALLOC) {
	str = sp->u.string;

	for (; *str; str++)
	    if (isalpha(*str))
		*str |= 'a' - 'A';
    } else {
	char *result;

	result = str = string_copy(sp->u.string);
	for (; *str; str++)
	    if (isalpha(*str))
		*str |= 'a' - 'A';
	pop_stack();
	push_malloced_string(result);
    }
}
#endif

#ifdef F_MALLOC_STATUS
void f_malloc_status P2(int, num_arg, int, instruction)
{
#ifdef DO_MSTATS
    void show_mstats PROT((char *));

#endif
#if (defined(WRAPPEDMALLOC) || defined(DEBUGMALLOC))
    void dump_malloc_data PROT((void));

#endif

    if (using_bsd_malloc) {
	add_message("Using BSD malloc.\n");
    }
    if (using_smalloc) {
	add_message("Using Smalloc.\n");
    }
#ifdef DO_MSTATS
    show_mstats("malloc_status()");
#endif
#if (defined(WRAPPEDMALLOC) || defined(DEBUGMALLOC))
    dump_malloc_data();
#endif
#ifdef SYSMALLOC
    if (!using_bsd_malloc && !using_smalloc) {
	add_message("Using system malloc.\n");
    }
#endif
    push_number(0);
}
#endif

#ifdef F_MAP_DELETE
void
f_map_delete P2(int, num_arg, int, instruction)
{
    mapping_delete((sp - 1)->u.map, sp);
    pop_stack();		/* all functions must leave exactly 1 element
				 * on stack */
}
#endif

#ifdef F_MAPP
void
f_mapp P2(int, num_arg, int, instruction)
{
    assign_svalue(sp, (sp->type == T_MAPPING) ? &const1 : &const0);
}
#endif

#ifdef F_MAP
void
f_map P2(int, num_arg, int, instruction)
{
    struct svalue *arg = sp - num_arg + 1;

    if (arg->type == T_MAPPING) map_mapping(arg, num_arg);
    else map_array(arg, num_arg);
}
#endif

#ifdef F_MASTER
void
f_master P2(int, num_arg, int, instruction)
{
    int err;
    
    err = assert_master_ob_loaded("master", "");
    if (err != 1)
	push_number(0);
    else
	push_object(master_ob);
}
#endif

/*
This efun searches a mapping for a path.  Each key is assumed to be a
string.  The value is completely arbitrary.  The efun finds the largest
matching path in the mapping.  Keys ended in '/' are assumed to match
paths with character that follow the '/', i.e. / is a wildcard for anything
below this directory.  DO NOT CHANGE THIS EFUN TIL YOU UNDERSTAND IT.  It
catches folks by suprise at first, but it is coded the way it is for a reason.
It effectively implements the search loop in TMI's access object as a single
efun.

        Cygnus
*/
#ifdef F_MATCH_PATH
void
f_match_path P2(int, num_arg, int, instruction)
{
    struct svalue *value;
    struct svalue string;
    char *src;
    char *dst;
    struct svalue *nvalue;
    struct mapping *map;

    value = &const0u;

    string.type = T_STRING;
    string.subtype = STRING_MALLOC;
    string.u.string = (char *) DMALLOC(strlen(sp->u.string) + 1, 0, "match_path");

    src = sp->u.string;
    dst = string.u.string;

    while (*src != '\0') {
	while (*src != '/' && *src != '\0')
	    *dst++ = *src++;
	if (*src == '/') {
	    while (*++src == '/');
	    if (*src != '\0' || dst == string.u.string)
		*dst++ = '/';
	}
	*dst = '\0';
	nvalue = find_in_mapping((sp - 1)->u.map, &string);
	if (nvalue != &const0u)
	    value = nvalue;
    }

    FREE(string.u.string);

    /* Don't free mapping first, in case sometimes one uses a ref 1 mapping */
    /* Randor - 5/29/94 */
    pop_stack();
    map = sp->u.map;
    assign_svalue_no_free(sp, value);
    free_mapping(map);
}
#endif				/* F_MATCH_PATH */

#ifdef F_MEMBER_ARRAY
void
f_member_array P2(int, num_arg, int, instruction)
{
    struct vector *v;
    struct svalue *find;
    int i = 0;

    if (num_arg > 2) {
	CHECK_TYPES(sp, T_NUMBER, 3, instruction);
	i = (sp--)->u.number;
	if (i<0) bad_arg(3, instruction);
    }
    if (sp->type == T_STRING) {
	char *res;
	CHECK_TYPES(sp-1, T_NUMBER, 1, instruction);
	if (res = strchr(sp->u.string + i, (sp-1)->u.number))
	    i = res - sp->u.string;
	else
	    i = -1;
    } else {
	v = sp->u.vec;
	find = (sp - 1);
	
	for (; i < v->size; i++) {
	    if (v->item[i].type == T_OBJECT &&
		v->item[i].u.ob->flags & O_DESTRUCTED)
		assign_svalue(&v->item[i], &const0);
	    if (v->item[i].type != find->type)
		continue;
	    switch (find->type) {
	    case T_STRING:
		if (strcmp(find->u.string, v->item[i].u.string) == 0)
		    break;
		continue;
	    case T_NUMBER:
		if (find->u.number == v->item[i].u.number)
		    break;
		continue;
	    case T_REAL:
		if (find->u.real == v->item[i].u.real)
		    break;
		continue;
	    case T_POINTER:
		if (find->u.vec == v->item[i].u.vec)
		    break;
		continue;
	    case T_OBJECT:
		if (find->u.ob == v->item[i].u.ob)
		    break;
		continue;
	    case T_MAPPING:
		if (find->u.map == v->item[i].u.map)
		    break;
		continue;
	    case T_FUNCTION:
		if (find->u.fp == v->item[i].u.fp)
		    break;
		continue;
	    case T_BUFFER:
		if (find->u.buf == v->item[i].u.buf)
		    break;
		continue;
	    default:
		fatal("Bad type to member_array(): %d\n", (sp - 1)->type);
	    }
	    break;
	}
	if (i == v->size)
	    i = -1;			/* Return -1 for failure */
    }
    pop_2_elems();
    push_number(i);
}
#endif

#ifdef F_MESSAGE
void
f_message P2(int, num_arg, int, instruction)
{
    struct vector *use, *avoid;
    struct svalue *args;

    static struct vector vtmp1 =
    {1,
#ifdef DEBUG
     1,
#endif
     1,
#ifndef NO_MUDLIB_STATS
     {(mudlib_stats_t *) NULL, (mudlib_stats_t *) NULL}
#endif
    };
    static struct vector vtmp2 =
    {1,
#ifdef DEBUG
     1,
#endif
     1,
#ifndef NO_MUDLIB_STATS
     {(mudlib_stats_t *) NULL, (mudlib_stats_t *) NULL}
#endif
    };

    args = sp - num_arg + 1;
    switch (args[2].type) {
    case T_OBJECT:
	vtmp1.item[0].type = T_OBJECT;
	vtmp1.item[0].u.ob = args[2].u.ob;
	use = &vtmp1;
	break;
    case T_POINTER:
	use = args[2].u.vec;
	break;
    case T_STRING:
	vtmp1.item[0].type = T_STRING;
	vtmp1.item[0].u.string = args[2].u.string;
	use = &vtmp1;
	break;
    case T_NUMBER:
	if (args[2].u.number == 0) {
	    /* for compatibility (write() simul_efuns, etc)  -bobf */
	    check_legal_string(args[1].u.string);
	    add_message(args[1].u.string);
	    pop_n_elems(num_arg);
	    push_number(0);
	    return;
	}
    default:
	bad_argument(&args[2], T_OBJECT | T_STRING | T_POINTER | T_NUMBER,
		     3, F_MESSAGE);
    }
    if (num_arg == 4) {
	switch (args[3].type) {
	case T_OBJECT:
	    vtmp2.item[0].type = T_OBJECT;
	    vtmp2.item[0].u.ob = args[3].u.ob;
	    avoid = &vtmp2;
	    break;
	case T_POINTER:
	    avoid = args[3].u.vec;
	    break;
	default:
	    avoid = null_array();
	}
    } else
	avoid = null_array();
    do_message(&args[0], args[1].u.string, use, avoid, 1);
    pop_n_elems(num_arg);
    push_number(0);
    return;
}
#endif

#ifdef F_MKDIR
void
f_mkdir P2(int, num_arg, int, instruction)
{
    char *path;

    path = check_valid_path(sp->u.string, current_object, "mkdir", 1);
    if (path == 0 || mkdir(path, 0770) == -1)
	assign_svalue(sp, &const0);
    else
	assign_svalue(sp, &const1);
}
#endif

#ifdef F_MOVE_OBJECT
void
f_move_object P2(int, num_arg, int, instruction)
{
    struct object *o1, *o2;

    /* get destination */
    if (sp->type == T_OBJECT)
	o2 = sp->u.ob;
    else {
	o2 = find_object(sp->u.string);
	if (!o2 || !object_visible(o2))
	    error("move_object failed: could not find destination\n");
    }

    /* get source */
    if (num_arg == 1) {
	o1 = current_object;
	if (o1->flags & O_DESTRUCTED)
	    error("move_object(): can't move a destructed object\n");

    } else {
	if ((sp - 1)->type == T_OBJECT)
	    o1 = (sp - 1)->u.ob;
	else {
	    o1 = find_object((sp - 1)->u.string);
	    if (!o1)
		error("move_object(): can't move non-existent object\n");
	}
	pop_stack();
    }

    move_object(o1, o2);
}
#endif

#ifdef F_MUD_STATUS
void f_mud_status P2(int, num_arg, int, instruction)
{
    int tot, res, verbose = 0;
    extern char *reserved_area;
    extern int tot_alloc_object, tot_alloc_sentence, tot_alloc_object_size,
        num_mappings, num_arrays, total_array_size, total_mapping_size,
        total_users, total_mapping_nodes;
    extern int total_num_prog_blocks;
    extern int total_prog_block_size;
    extern int add_message_calls, inet_packets, inet_volume;

    verbose = sp->u.number;
    pop_stack();

    if (reserved_area)
	res = RESERVED_SIZE;
    else
	res = 0;

    if (verbose) {
	char dir_buf[1024];
	FILE *testfp;

	if (testfp = fopen(".mudos_test_file", "w")) {
	    fclose(testfp);
	    add_message("Open-file-test succeeded.\n");
	    unlink(".mudos_test_file");
	} else {
	    /* if strerror() is missing, edit the #ifdef for it in port.c */
	    add_message("Open file test failed: %s\n", strerror(errno));
	}

	add_message("current working directory: %s\n\n",
		    get_current_dir(dir_buf, 1024));
	add_message("add_message statistics\n");
	add_message("------------------------------\n");
	add_message("Calls to add_message: %d   Packets: %d   Average packet size: %f\n\n",
	add_message_calls, inet_packets, (float) inet_volume / inet_packets);

	stat_living_objects();
	add_message("\n");
#ifdef F_CACHE_STATS
	print_cache_stats();
	add_message("\n");
#endif
	print_swap_stats();
	add_message("\n");

        tot = show_otable_status(verbose);
        add_message("\n");
        tot += heart_beat_status(verbose);
        add_message("\n");
        tot += add_string_status(verbose);
        add_message("\n");
        tot += print_call_out_usage(verbose);
    } else {
	/* !verbose */
	add_message("Sentences:\t\t\t%8d %8d\n", tot_alloc_sentence,
		    tot_alloc_sentence * sizeof(struct sentence));
	add_message("Objects:\t\t\t%8d %8d\n",
		    tot_alloc_object, tot_alloc_object_size);
	add_message("Prog blocks:\t\t\t%8d %8d\n",
		    total_num_prog_blocks, total_prog_block_size);
	add_message("Arrays:\t\t\t\t%8d %8d\n", num_arrays,
		    total_array_size);
	add_message("Mappings:\t\t\t%8d %8d\n", num_mappings,
		    total_mapping_size);
	add_message("Mappings(nodes):\t\t%8d\n", total_mapping_nodes);
	add_message("Interactives:\t\t\t%8d %8d\n", total_users,
		    total_users * sizeof(struct interactive));

	tot = show_otable_status(verbose) +
	    heart_beat_status(verbose) +
	    add_string_status(verbose) +
	    print_call_out_usage(verbose);
    }

    tot += total_prog_block_size +
	total_array_size +
	total_mapping_size +
	tot_alloc_sentence * sizeof(struct sentence) +
	tot_alloc_object_size +
	total_users * sizeof(struct interactive) +
	res;

    if (!verbose) {
	add_message("\t\t\t\t\t --------\n");
	add_message("Total:\t\t\t\t\t %8d\n", tot);
    }

    push_number(0);
}
#endif

#if defined(F_NEW)
void
f_new P2(int, num_arg, int, instruction)
{
    struct object *ob;

    ob = clone_object(sp->u.string);
    pop_stack();
    if (ob) {
	sp++;
	sp->type = T_OBJECT;
	sp->u.ob = ob;
	add_ref(ob, "F_NEW");
    } else
	push_number(0);
}
#endif

#ifdef F_NOTIFY_FAIL
void
f_notify_fail P2(int, num_arg, int, instruction)
{
    if (sp->type == T_STRING)
	set_notify_fail_message(sp->u.string);
    else if (sp->type == T_FUNCTION)
	set_notify_fail_function(sp->u.fp);
    pop_stack();
    push_number(0);
}
#endif

#ifdef F_NULLP
void
f_nullp P2(int, num_arg, int, instruction)
{
    if (IS_NULL(sp))
	assign_svalue(sp, &const1);
    else
	assign_svalue(sp, &const0);
}
#endif

#ifdef F_OBJECTP
void
f_objectp P2(int, num_arg, int, instruction)
{
    if (sp->type == T_OBJECT)
	assign_svalue(sp, &const1);
    else
	assign_svalue(sp, &const0);
}
#endif

#ifdef F_OPCPROF

void
f_opcprof P2(int, num_arg, int, instruction)
{
    char *fn = "/OPCPROF";

    if (num_arg == 1) {
	fn = sp->u.string;
    }
    opcdump(fn);
    if (num_arg == 1) {
	pop_stack();
    }
    push_number(0);
}
#endif

#ifdef F_ORIGIN
void
f_origin P2(int, num_arg, int, instruction)
{
    push_number((int) caller_type);
}
#endif

#ifdef F_POINTERP
void
f_pointerp P2(int, num_arg, int, instruction)
{
    if (sp->type == T_POINTER)
	assign_svalue(sp, &const1);
    else
	assign_svalue(sp, &const0);
}
#endif

#ifdef F_PRESENT
void
f_present P2(int, num_arg, int, instruction)
{
    struct svalue *arg = sp - num_arg + 1;

#ifdef LAZY_RESETS
    if (num_arg == 2) {
	try_reset(arg[1].u.ob);
    }
#endif
    ob = object_present(arg, num_arg == 1 ? 0 : arg[1].u.ob);
    pop_n_elems(num_arg);
    if (ob)
	push_object(ob);
    else
	push_number(0);
}
#endif

#ifdef F_PREVIOUS_OBJECT
void
f_previous_object P2(int, num_arg, int, instruction)
{
    extern struct control_stack control_stack[MAX_TRACE];
    struct control_stack *p;
    int i;

    if ((i = sp->u.number) > 0) {
	if (i >= MAX_TRACE) {
	    sp->u.number = 0;
	    return;
	}
	ob = 0;
	p = csp + 1;
	while ((p--) >= control_stack) {
	    if (p->extern_call) {
		i--;
		if (!i) {
		    ob = p->prev_ob;
		    break;
		}
	    }
	}
    } else if (i == -1) {
	struct vector *v;
	
	i = previous_ob ? 1 : 0;
	p = csp + 1;
	while ((p--) > control_stack) {
	    if (p->extern_call && p->prev_ob)
		i++;
	}
	v = allocate_array(i);
	p = csp + 1;
	if (previous_ob) {
	    if (!(previous_ob->flags & O_DESTRUCTED)) {
		v->item[0].type = T_OBJECT;
		v->item[0].u.ob = previous_ob;
		add_ref(previous_ob, "previous_object(-1)");
	    }
	    i = 1;
	} else i = 0;
	while ((p--) > control_stack) {
	    if (p->extern_call && p->prev_ob) {
		if (!(p->prev_ob->flags & O_DESTRUCTED)) {
		    v->item[i].type = T_OBJECT;
		    v->item[i].u.ob = p->prev_ob;
		    add_ref(p->prev_ob, "previous_object(-1)");
		}
		i++;
	    }
	}
	sp--;
	push_refed_vector(v);
	return;
    } else if (i < -1) {
	error("Illegal negative argument to previous_object()\n");
    } else
	ob = previous_ob;
    sp--;

    if (ob == 0 || (ob->flags & O_DESTRUCTED))
	push_number(0);
    else
	push_object(ob);
}
#endif

#ifdef F_PRINTF
void
f_printf P2(int, num_arg, int, instruction)
{
    if (command_giver)
	tell_object(command_giver, string_print_formatted((sp - num_arg + 1)->u.string,
					    num_arg - 1, sp - num_arg + 2));
    pop_n_elems(num_arg - 1);
}
#endif

#ifdef F_PROCESS_STRING
void
f_process_string P2(int, num_arg, int, instruction)
{
    extern char *process_string PROT((char *));
    char *str;

    str = process_string(sp->u.string);
    if (str != sp->u.string) {
	pop_stack();
	push_malloced_string(str);
    }
}
#endif

#ifdef F_PROCESS_VALUE
void
f_process_value P2(int, num_arg, int, instruction)
{
    extern struct svalue *process_value PROT((char *));
    struct svalue *ret;

    ret = process_value(sp->u.string);
    pop_stack();
    push_number(0);
    if (ret)
	assign_svalue(sp, ret);
}
#endif

#ifdef F_QUERY_HOST_NAME
void
f_query_host_name P2(int, num_arg, int, instruction)
{
    extern char *query_host_name();
    char *tmp;

    tmp = query_host_name();
    if (tmp)
	push_string(tmp, STRING_CONSTANT);
    else
	push_number(0);
}
#endif

#ifdef F_QUERY_IDLE
void
f_query_idle P2(int, num_arg, int, instruction)
{
    int i;

    i = query_idle(sp->u.ob);
    pop_stack();
    push_number(i);
}
#endif

#ifdef F_QUERY_IP_NAME
void
f_query_ip_name P2(int, num_arg, int, instruction)
{
    extern char *query_ip_name PROT((struct object *));
    char *tmp;

    tmp = query_ip_name(num_arg ? sp->u.ob : 0);
    if (num_arg)
	pop_stack();
    if (tmp == 0)
	push_number(0);
    else
	push_string(tmp, STRING_MALLOC);
}
#endif

#ifdef F_QUERY_IP_NUMBER
void
f_query_ip_number P2(int, num_arg, int, instruction)
{
    extern char *query_ip_number PROT((struct object *));
    char *tmp;

    tmp = query_ip_number(num_arg ? sp->u.ob : 0);
    if (num_arg)
	pop_stack();
    if (tmp == 0)
	push_number(0);
    else
	push_string(tmp, STRING_MALLOC);
}
#endif

#ifdef F_QUERY_LOAD_AVERAGE
void
f_query_load_average P2(int, num_arg, int, instruction)
{
    push_string(query_load_av(), STRING_MALLOC);
}
#endif

#ifdef F_QUERY_PRIVS
void
f_query_privs P2(int, num_arg, int, instruction)
{
    ob = sp->u.ob;
    if (ob->privs != NULL) {
	pop_stack();
	push_string(ob->privs, STRING_SHARED);
    } else {
	pop_stack();
	push_number(0);
    }
}
#endif

#ifdef F_QUERY_SNOOPING
void
f_query_snooping P2(int, num_arg, int, instruction)
{
    struct object *ob;

    ob = query_snooping(sp->u.ob);
    pop_stack();
    if (ob)
	push_object(ob);
    else
	push_number(0);
}
#endif

#ifdef F_QUERY_SNOOP
void
f_query_snoop P2(int, num_arg, int, instruction)
{
    struct object *ob;

    ob = query_snoop(sp->u.ob);
    pop_stack();
    if (ob)
	push_object(ob);
    else
	push_number(0);
}
#endif

#ifdef F_QUERY_VERB
void
f_query_verb P2(int, num_arg, int, instruction)
{
    if (last_verb == 0) {
	push_number(0);
	return;
    }
    push_string(last_verb, STRING_SHARED);
}
#endif

#ifdef F_RANDOM
void
f_random P2(int, num_arg, int, instruction)
{
    if (sp->u.number <= 0) {
	sp->u.number = 0;
	return;
    }
    sp->u.number = random_number(sp->u.number);
}
#endif

#ifdef F_READ_BYTES
void
f_read_bytes P2(int, num_arg, int, instruction)
{
    char *str;
    struct svalue *arg = sp - num_arg + 1;
    int start = 0, len = 0, rlen = 0;

    if (num_arg > 1)
	start = arg[1].u.number;
    if (num_arg == 3) {
	CHECK_TYPES(&arg[2], T_NUMBER, 2, instruction);
	len = arg[2].u.number;
    }
    str = read_bytes(arg[0].u.string, start, len, &rlen);
    pop_n_elems(num_arg);
    if (str == 0)
	push_number(0);
    else {
	push_malloced_string(str);
    }
}
#endif

#ifdef F_READ_BUFFER
void
f_read_buffer P2(int, num_arg, int, instruction)
{
    char *str;
    struct svalue *arg = sp - num_arg + 1;
    int start = 0, len = 0, rlen = 0;
    int from_file = 0;		/* new line */

    if (num_arg > 1) {
	start = arg[1].u.number;
    }
    if (num_arg == 3) {
	CHECK_TYPES(&arg[2], T_NUMBER, 2, instruction);
	len = arg[2].u.number;
    }
    if (arg[0].type == T_STRING) {
	from_file = 1;		/* new line */
	str = read_bytes(arg[0].u.string, start, len, &rlen);
    } else {			/* T_BUFFER */
	str = read_buffer(arg[0].u.buf, start, len, &rlen);
    }
    pop_n_elems(num_arg);
    if (str == 0) {
	push_number(0);
    } else if (from_file) {	/* changed */
	struct buffer *buf;

	buf = allocate_buffer(rlen);
	memcpy(buf->item, str, rlen);
	push_refed_buffer(buf);
	FREE(str);
    } else {			/* T_BUFFER */
	push_malloced_string(str);
    }
}
#endif

#ifdef F_READ_FILE
void
f_read_file P2(int, num_arg, int, instruction)
{
    char *str;
    struct svalue *arg = sp - num_arg + 1;
    int start = 0, len = 0;

    if (num_arg > 1)
	start = arg[1].u.number;
    if (num_arg == 3) {
	CHECK_TYPES(&arg[2], T_NUMBER, 2, instruction);
	len = arg[2].u.number;
    }
    str = read_file(arg[0].u.string, start, len);
    pop_n_elems(num_arg);
    if (str == 0)
	push_number(0);
    else {
	push_string(str, STRING_MALLOC);
	FREE(str);
    }
}
#endif

#ifdef F_RECEIVE
void
f_receive P2(int, num_arg, int, instruction)
{
    if (current_object->interactive) {
	struct object *save_command_giver = command_giver;

	check_legal_string(sp->u.string);
	command_giver = current_object;
	add_message("%s", sp->u.string);
	command_giver = save_command_giver;
	assign_svalue(sp, &const1);
    } else {
	assign_svalue(sp, &const0);
    }
}
#endif

#ifdef F_REGEXP
void
f_regexp P2(int, num_arg, int, instruction)
{
    struct vector *v;

    v = match_regexp((sp - 1)->u.vec, sp->u.string);
    pop_2_elems();
    if (v == 0)
	push_number(0);
    else {
	push_refed_vector(v);
    }
}
#endif

#ifdef F_REMOVE_ACTION
void
f_remove_action P2(int, num_arg, int, instruction)
{
    int success;

    success = remove_action((sp - 1)->u.string, sp->u.string);
    pop_2_elems();
    push_number(success);
}
#endif

#ifdef F_REMOVE_CALL_OUT
void
f_remove_call_out P2(int, num_arg, int, instruction)
{
    int i;

    i = remove_call_out(current_object, sp->u.string);
    pop_stack();
    push_number(i);
}
#endif

#ifdef F_RENAME
void
f_rename P2(int, num_arg, int, instruction)
{
    int i;

    i = do_rename((sp - 1)->u.string, sp->u.string, F_RENAME);
    pop_2_elems();
    push_number(i);
}
#endif				/* F_RENAME */

#ifdef F_REPLACE_STRING

/*
syntax for replace_string is now:
    string replace_string(src, pat, rep);   // or
    string replace_string(src, pat, rep, max);  // or
    string replace_string(src, pat, rep, first, last);

The 4th/5th args are optional (to retain backward compatibility).
- src, pat, and rep are all strings.
- max is an integer. It will replace all occurances up to max
  matches (starting as 1 as the first), with a value of 0 meaning
  'replace all')
- first and last are just a range to replace between, with
  the following constraints
    first < 1: change all from start
    last == 0 || last > max matches:    change all to end
    first > last: return unmodified array.
(i.e, with 4 args, it's like calling it with:
    replace_string(src, pat, rep, 0, max);
)
*/

void
f_replace_string P2(int, num_arg, int, instruction)
{
    int plen, rlen, dlen, first, last, cur;
    char *src, *pattern, *replace, *dst1, *dst2;

    if (num_arg > 5) {
	error("Too many args to replace_string.\n");
	pop_n_elems(num_arg);
	return;
    }
    CHECK_TYPES((sp - num_arg + 3), T_STRING, 3, instruction);
    src = (sp - num_arg + 1)->u.string;
    pattern = (sp - num_arg + 2)->u.string;
    replace = (sp - num_arg + 3)->u.string;
    first = 0;
    last = 0;

    if (num_arg >= 4) {
	CHECK_TYPES((sp - num_arg + 4), T_NUMBER, 4, instruction);
	first = (sp - num_arg + 4)->u.number;

	if (num_arg == 4) {
	    last = first;
	    first = 0;
	} else if (num_arg == 5) {
	    CHECK_TYPES((sp - num_arg + 5), T_NUMBER, 5, instruction);
	    /* first set above. */
	    last = (sp - num_arg + 5)->u.number;
	}
    }
    if (!last)
	last = max_string_length;

    if (first > last) {		/* just return it */
	pop_n_elems(num_arg - 1);
	return;
    }
    plen = strlen(pattern);
    if (plen == 0) {
	pop_n_elems(num_arg - 1);	/* just return it */
	return;
    }
    rlen = strlen(replace);
    dlen = 0;
    cur = 0;

    if ((sp - num_arg + 1)->subtype == STRING_MALLOC && rlen <= plen) {
	/* in string replacement */
	dst2 = dst1 = src;

	/* assume source string is a string < maximum string length */
	while (*src != '\0') {
	    if (strncmp(src, pattern, plen) == 0) {
		cur++;
		if (cur >= first && cur <= last) {
		    if (rlen != 0) {
			strncpy(dst2, replace, rlen);
			dst2 += rlen;
		    }
		    src += plen;
		    continue;
		}
	    }
	    *dst2++ = *src++;
	}

	/*
	 * shrink block (if necessary)
	 */
	if (rlen < plen) {
	    *dst2 = '\0';
	    (sp - num_arg + 1)->u.string = (char *) DREALLOC(dst1, dst2 - dst1 + 2, 200, "f_replace_string: 1");
	}
	pop_n_elems(num_arg - 1);
    } else {
	dst2 = dst1 = (char *) DMALLOC(max_string_length, 31, "f_replace_string: 2");

	while (*src != '\0') {
	    if (strncmp(src, pattern, plen) == 0) {
		cur++;
		if (cur >= first && cur <= last) {
		    if (rlen != 0) {
			if (max_string_length - dlen <= rlen) {
			    pop_n_elems(num_arg);
			    push_svalue(&const0u);
			    FREE(dst1);
			    return;
			}
			strncpy(dst2, replace, rlen);
			dst2 += rlen;
			dlen += rlen;
		    }
		    src += plen;
		    continue;
		}
	    }
	    if (max_string_length - dlen <= 1) {
		pop_n_elems(num_arg);
		push_svalue(&const0u);
		FREE(dst1);
		return;
	    }
	    *dst2++ = *src++;
	    dlen++;
	}
	*dst2 = '\0';
	pop_n_elems(num_arg);
	/*
	 * shrink block or make a copy of exact size
	 */
	dst1 = (char *) DREALLOC(dst1, dst2 - dst1 + 2, 201, "f_replace_string: 3");
	push_malloced_string(dst1);
    }
}
#endif

#ifdef F_RESOLVE
void
f_resolve P2(int, num_arg, int, instruction)
{
    int i, query_addr_number PROT((char *, char *));

    i = query_addr_number((sp - 1)->u.string, sp->u.string);
    pop_2_elems();
    push_number(i);
}				/* f_resolve() */
#endif

#ifdef F_RESTORE_OBJECT
void
f_restore_object P2(int, num_arg, int, instruction)
{
    int i, flag;
    struct svalue *arg = sp - num_arg + 1;

    flag = (num_arg == 1) ? 0 : arg[1].u.number;
    i = restore_object(current_object, arg[0].u.string, flag);
    pop_n_elems(num_arg);
    push_number(i);
}
#endif

#ifdef F_RESTORE_VARIABLE
void
f_restore_variable P2(int, num_arg, int, instruction) {
    struct svalue v = { T_NUMBER };
    char *s = string_copy(sp->u.string);
    
    if (!s) error("Out of memory\n");
    restore_variable(&v, s);
    free_string_svalue(sp);
    FREE(s);
    *sp = v;
}
#endif

#ifdef F_RM
void
f_rm P2(int, num_arg, int, instruction)
{
    int i;

    i = remove_file(sp->u.string);
    pop_stack();
    push_number(i);
}
#endif

#ifdef F_RMDIR
void
f_rmdir P2(int, num_arg, int, instruction)
{
    char *path;

    path = check_valid_path(sp->u.string, current_object, "rmdir", 1);
    if (path == 0 || rmdir(path) == -1)
	assign_svalue(sp, &const0);
    else
	assign_svalue(sp, &const1);
}
#endif

#ifdef F_SAVE_OBJECT
void
f_save_object P2(int, num_arg, int, instruction)
{
    int flag, i;

    if (num_arg == 2) {
	CHECK_TYPES(sp-1, T_STRING, 1, instruction);
	CHECK_TYPES(sp, T_NUMBER, 2, instruction);
	flag = (sp--)->u.number;
    } else {
	CHECK_TYPES(sp, T_STRING, 1, instruction);
	flag = 0;
    }
    i = save_object(current_object, sp->u.string, flag);
    free_string_svalue(sp--);
    push_number(i);
}
#endif

#ifdef F_SAVE_VARIABLE
void
f_save_variable P2(int, num_arg, int, instruction) {
    char *p;

    p = save_variable(sp);
    pop_stack();
    push_malloced_string(p);
}
#endif

#ifdef F_SAY
void
f_say P2(int, num_arg, int, instruction)
{
    struct vector *avoid;
    static struct vector vtmp =
    {1,
#ifdef DEBUG
     1,
#endif
     1,
#ifndef NO_MUDLIB_STATS
     {(mudlib_stats_t *) NULL, (mudlib_stats_t *) NULL}
#endif
    };

    if (num_arg == 1) {
	avoid = null_array();
	say(sp, avoid);
    } else {
	if (sp->type == T_OBJECT) {
	    vtmp.item[0].type = T_OBJECT;
	    vtmp.item[0].u.ob = sp->u.ob;
	    avoid = &vtmp;
	} else {		/* must be a vector... */
	    avoid = sp->u.vec;
	}
	say(sp - 1, avoid);
	pop_stack();
    }
}
#endif

#ifdef F_SET_EVAL_LIMIT
/* warning: do not enable this without using valid_override() in the master
   object and a set_eval_limit() simul_efun to restrict access.
*/
void
f_set_eval_limit P2(int, num_arg, int, instruction)
{
    extern int max_cost;

    switch (sp->u.number) {
    case 0:
	sp->u.number = eval_cost = max_cost;
	break;
    case -1:
	sp->u.number = eval_cost;
	break;
    case 1:
	sp->u.number = max_cost;
	break;
    default:
	max_cost = sp->u.number;
	break;
    }
}
#endif

#ifdef F_SET_BIT
void
f_set_bit P2(int, num_arg, int, instruction)
{
    char *str;
    int len, old_len, ind;

    if (sp->u.number > MAX_BITS)
	error("set_bit: too big bit number: %d\n", sp->u.number);
    if (sp->u.number < 0)
	error("set_bit: illegal (negative) arg 2\n");
    len = SVALUE_STRLEN(sp - 1);
    old_len = len;
    ind = sp->u.number / 6;
    if (ind >= len)
	len = ind + 1;
    str = DXALLOC(len + 1, 32, "f_set_bit");
    str[len] = '\0';
    if (old_len)
	memcpy(str, (sp - 1)->u.string, old_len);
    if (len > old_len)
	memset(str + old_len, ' ', len - old_len);
    if (str[ind] > 0x3f + ' ' || str[ind] < ' ')
	error("Illegal bit pattern in set_bit character %d\n", ind);
    str[ind] = ((str[ind] - ' ') | (1 << (sp->u.number % 6))) + ' ';
    pop_stack();
    free_svalue(sp, "f_set_bit");
    sp->u.string = str;
    sp->subtype = STRING_MALLOC;
    sp->type = T_STRING;
}
#endif

#ifdef F_SET_HEART_BEAT
void
f_set_heart_beat P2(int, num_arg, int, instruction)
{
    int i;

    i = set_heart_beat(current_object, sp->u.number);
    sp->u.number = i;
}
#endif

#ifdef F_QUERY_HEART_BEAT
void
f_query_heart_beat P2(int, num_arg, int, instruction)
{
    struct object *foo;

    foo = sp->u.ob;
    pop_stack();
    push_number(query_heart_beat(foo));
}
#endif

#ifdef F_SET_HIDE
void
f_set_hide P2(int, num_arg, int, instruction)
{
    if (!valid_hide(current_object))
	return;
    if (sp->u.number) {
	if (!(current_object->flags & O_HIDDEN) && current_object->interactive)
	    num_hidden++;
	current_object->flags |= O_HIDDEN;
    } else {
	if ((current_object->flags & O_HIDDEN) && current_object->interactive)
	    num_hidden--;
	current_object->flags &= ~O_HIDDEN;
    }
}
#endif

#ifdef F_SET_LIGHT
void
f_set_light P2(int, num_arg, int, instruction)
{
    struct object *o1;

#ifndef NO_LIGHT
    add_light(current_object, sp->u.number);
#endif
    o1 = current_object;
    while (o1->super)
	o1 = o1->super;
    sp->u.number = o1->total_light;
}
#endif

#ifdef F_SET_LIVING_NAME
void
f_set_living_name P2(int, num_arg, int, instruction)
{
    set_living_name(current_object, sp->u.string);
}
#endif

#ifdef F_SET_PRIVS
void
f_set_privs P2(int, num_arg, int, instruction)
{
    struct object *ob;

    ob = (sp - 1)->u.ob;
    if (ob->privs != NULL)
	free_string(ob->privs);
    if (sp->type != T_STRING)
	ob->privs = NULL;
    else
	ob->privs = make_shared_string(sp->u.string);
    pop_stack();
    pop_stack();
    push_number(0);
}
#endif

#ifdef F_SHADOW
void
f_shadow P2(int, num_arg, int, instruction)
{
    struct object *ob;

    ob = (sp - 1)->u.ob;
    if (sp->u.number == 0) {
	ob = ob->shadowed;
	pop_2_elems();
	if (ob)
	    push_object(ob);
	else
	    push_number(0);
	return;
    }
    if (ob == current_object) {
	error("shadow: Can't shadow self\n");
    }
    if (validate_shadowing(ob)) {
	if (current_object->flags & O_DESTRUCTED) {
	    pop_stack();
	    assign_svalue(sp, &const0);
	    return;
	}
	/*
	 * The shadow is entered first in the chain.
	 */
	while (ob->shadowed)
	    ob = ob->shadowed;
	current_object->shadowing = ob;
	ob->shadowed = current_object;
	pop_stack();
	pop_stack();
	push_object(ob);
	return;
    }
    pop_stack();
    assign_svalue(sp, &const0);
}
#endif

#ifdef F_SHOUT
void
f_shout P2(int, num_arg, int, instruction)
{
    shout_string(sp->u.string);
}
#endif

#ifdef F_SHUTDOWN
void
f_shutdown P2(int, num_arg, int, instruction)
{
    int exit_code = 0;

    if (num_arg) {
	exit_code = sp->u.number;
    }
    shutdownMudOS(exit_code);
    if (!num_arg) {
	push_number(0);
    }
}
#endif

#ifdef F_SIZEOF
void
f_sizeof P2(int, num_arg, int, instruction)
{
    int i;

    switch (sp->type) {
    case T_POINTER:
	i = sp->u.vec->size;
	free_vector(sp->u.vec);
	break;
    case T_MAPPING:
	i = sp->u.map->count;
	free_mapping(sp->u.map);
	break;
    case T_BUFFER:
	i = sp->u.buf->size;
	free_buffer(sp->u.buf);
	break;
    case T_STRING:
	i = SVALUE_STRLEN(sp);
	free_string_svalue(sp);
	break;
    default:
	i = 0;
	free_svalue(sp, "f_sizeof");
    }
    sp->type = T_NUMBER;
    sp->u.number = i;
}
#endif

#ifdef F_SNOOP
void
f_snoop P2(int, num_arg, int, instruction)
{
    /*
     * This one takes a variable number of arguments. It returns 0 or an
     * object.
     */
    ob = 0;
    if (num_arg == 1) {
	if (new_set_snoop(sp->u.ob, 0))
	    ob = sp->u.ob;
    } else {
	if (new_set_snoop((sp - 1)->u.ob, sp->u.ob))
	    ob = sp->u.ob;
    }
    pop_n_elems(num_arg);
    if (ob && !(ob->flags & O_DESTRUCTED))
	push_object(ob);
    else
	push_number(0);
}
#endif

#ifdef F_SPRINTF
void
f_sprintf P2(int, num_arg, int, instruction)
{
    char *s;

    /*
     * string_print_formatted() returns a pointer to it's internal buffer, or
     * to an internal constant...  Either way, it must be copied before it's
     * returned as a string.
     */

    s = string_print_formatted((sp - num_arg + 1)->u.string,
			       num_arg - 1, sp - num_arg + 2);
    pop_n_elems(num_arg);
    if (!s)
	push_number(0);
    else
	push_malloced_string(string_copy(s));
}
#endif

#ifdef F_STAT
void
f_stat P2(int, num_arg, int, instruction)
{
    struct stat buf;
    char *path;
    struct vector *v;

    path = (sp - 1)->u.string;
    path = check_valid_path(path, current_object, "stat", 0);
    if (!path) {
	pop_stack();
	assign_svalue(sp, &const0);
	return;
    }
    if (stat(path, &buf) != -1) {
	if (buf.st_mode & S_IFREG) {	/* if a regular file */
	    v = allocate_array(3);
	    v->item[0].type = T_NUMBER;
	    v->item[0].u.number = buf.st_size;
	    v->item[1].type = T_NUMBER;
	    v->item[1].u.number = buf.st_mtime;
	    v->item[2].type = T_NUMBER;
	    ob = find_object2(path);
	    if (ob && !object_visible(ob))
		ob = 0;
	    if (ob)
		v->item[2].u.number = ob->load_time;
	    else
		v->item[2].u.number = 0;
	    pop_2_elems();
	    push_refed_vector(v);
	    return;
	}
    }
    v = get_dir((sp - 1)->u.string, sp->u.number);
    pop_2_elems();
    if (v) {
	push_refed_vector(v);
    } else {
	push_number(0);
    }
}
#endif

#ifdef F_STRSRCH
/*
 * int strsrch(string big, string little, [ int flag ])
 * - search for little in big, starting at right if flag is set
 *   return int offset of little, -1 if not found
 *
 * Written 930706 by Luke Mewburn <zak@rmit.edu.au>
 */

void
f_strsrch P2(int, num_arg, int, instruction)
{
    register char *big, *little, *pos;
    static char buf[2];		/* should be initialized to 0 */
    int i;

    CHECK_TYPES(sp, T_NUMBER, 3, instruction);

    big = (sp - 2)->u.string;
    if ((sp - 1)->type == T_NUMBER) {
	buf[0] = (char) (sp - 1)->u.number;
	little = buf;
    } else
	little = (sp - 1)->u.string;

    /* little == ""  or  len(big) < len(little) */
    if (!little[0] || strlen(big) < strlen(little)) {
	pos = NULL;

	/* start at left */
    } else if (!sp->u.number) {
	if (!little[1])		/* 1 char srch pattern */
	    pos = strchr(big, (int) little[0]);
	else
	    pos = _strstr(big, little);

	/* start at right */
    } else {			/* XXX: maybe test for -1 */
	if (!little[1])		/* 1 char srch pattern */
	    pos = strrchr(big, (int) little[0]);
	else {
	    pos = strrchr(big, '\0');	/* find end */
	    pos -= strlen(little) - 1;	/* find rightmost pos it _can_ be */
	    while (1) {
		while (pos-- > big)
		    if (pos[0] == little[0])
			break;
		if (pos[0] != little[0]) {
		    pos = NULL;
		    break;
		}
		for (i = 1; little[i] && (pos[i] == little[i]); i++);	/* scan all chars */
		if (!little[i])
		    break;
	    }
	}
    }

    if (!pos)
	i = -1;
    else
	i = (int) (pos - big);
    pop_3_elems();
    push_number(i);
}				/* strsrch */
#endif

#ifdef F_STRCMP
void
f_strcmp P2(int, num_arg, int, instruction)
{
    int i;

    i = strcmp((sp - 1)->u.string, sp->u.string);
    pop_2_elems();
    push_number(i);
}
#endif

#ifdef F_STRINGP
void
f_stringp P2(int, num_arg, int, instruction)
{
    if (sp->type == T_STRING)
	assign_svalue(sp, &const1);
    else
	assign_svalue(sp, &const0);
}
#endif

#ifdef F_BUFFERP
void
f_bufferp P2(int, num_arg, int, instruction)
{
    if (sp->type == T_BUFFER) {
	assign_svalue(sp, &const1);
    } else {
	assign_svalue(sp, &const0);
    }
}
#endif

#ifdef F_SWAP
void
f_swap P2(int, num_arg, int, instruction)
{
    (void) swap(sp->u.ob);
}
#endif

#ifdef F_TAIL
void
f_tail P2(int, num_arg, int, instruction)
{
    if (tail(sp->u.string))
	assign_svalue(sp, &const1);
    else
	assign_svalue(sp, &const0);
}
#endif

#ifdef F_TELL_OBJECT
void
f_tell_object P2(int, num_arg, int, instruction)
{
    tell_object((sp - 1)->u.ob, sp->u.string);
    pop_stack();		/* Return first argument */
}
#endif

#ifdef F_TELL_ROOM
void
f_tell_room P2(int, num_arg, int, instruction)
{
    struct svalue *arg = sp - num_arg + 1;
    struct vector *avoid;

    if (arg[0].type == T_OBJECT) {
	ob = arg[0].u.ob;
    } else {			/* must be a string... */
	ob = find_object(arg[0].u.string);
	if (!ob || !object_visible(ob))
	    bad_argument(&arg[0], T_OBJECT | T_STRING, 1, F_TELL_ROOM);
    }

    if (num_arg == 2) {
	avoid = null_array();
    } else {
	CHECK_TYPES(&arg[2], T_POINTER, 3, instruction);
	avoid = arg[2].u.vec;
    }

    tell_room(ob, &arg[1], avoid);
    pop_n_elems(num_arg);
    push_number(0);
}
#endif

#ifdef F_TEST_BIT
void
f_test_bit P2(int, num_arg, int, instruction)
{
    int len;

    len = SVALUE_STRLEN(sp - 1);
    if (sp->u.number / 6 >= len) {
	pop_stack();
	assign_svalue(sp, &const0);
	return;
    }
    if (((sp - 1)->u.string[sp->u.number / 6] - ' ') & (1 << (sp->u.number % 6))) {
	pop_stack();
	assign_svalue(sp, &const1);
    } else {
	pop_stack();
	assign_svalue(sp, &const0);
    }
}
#endif

#ifdef F_THIS_OBJECT
void
f_this_object P2(int, num_arg, int, instruction)
{
    if (current_object->flags & O_DESTRUCTED)	/* Fixed from 3.1.1 */
	push_number(0);
    else
	push_object(current_object);
}
#endif

#ifdef F_THIS_PLAYER
void
f_this_player P2(int, num_arg, int, instruction)
{
    if ((sp--)->u.number && current_interactive &&
	!(current_interactive->flags & O_DESTRUCTED))
	push_object(current_interactive);
    else if (command_giver && !(command_giver->flags & O_DESTRUCTED))
	push_object(command_giver);
    else  
	(++sp)->u.number = 0;
}
#endif

#ifdef F_THROW
void
f_throw P2(int, num_arg, int, instruction)
{
    assign_svalue(&catch_value, sp--);
    throw_error();		/* do the longjump, with extra checks... */
}
#endif

#ifdef F_TIME
void
f_time P2(int, num_arg, int, instruction)
{
    push_number(current_time);
}
#endif

#ifdef F_TO_FLOAT
void
f_to_float P2(int, num_arg, int, instruction)
{
    double temp;

    if (sp->type == T_REAL) {
	return;			/* leave the pushed float on the stack for
				 * returning */
    } else if (sp->type == T_NUMBER) {
	temp = (double) sp->u.number;
	pop_stack();
	push_real(temp);
    } else if (sp->type == T_STRING) {
	temp = 0.0;
	sscanf(sp->u.string, "%lf", &temp);
	pop_stack();
	push_real(temp);
    }
}
#endif

#ifdef F_TO_INT
void
f_to_int P2(int, num_arg, int, instruction)
{
    int temp;

    if (sp->type == T_NUMBER) {
	return;			/* leave the pushed int on the stack for
				 * returning */
    } else if (sp->type == T_REAL) {
	temp = (int) sp->u.real;
	pop_stack();
	push_number(temp);
    } else if (sp->type == T_STRING) {
	temp = atoi(sp->u.string);
	pop_stack();
	push_number(temp);
    } else if (sp->type == T_BUFFER) {
	if (sp->u.buf->size < sizeof(int)) {
	    pop_stack();
	    push_number(0);
	} else {
	    int hostint, netint;

	    memcpy((char *) &netint, sp->u.buf->item, sizeof(int));
	    hostint = ntohl(netint);
	    pop_stack();
	    push_number(hostint);
	}
    }
}
#endif

#ifdef F_TYPEOF
void
f_typeof P2(int, num_arg, int, instruction)
{
    int k;

    k = sp->type;
    pop_stack();
    push_number(k);
}
#endif

#ifdef F_UNDEFINEDP
void
f_undefinedp P2(int, num_arg, int, instruction)
{
    if (IS_UNDEFINED(sp))
	assign_svalue(sp, &const1);
    else
	assign_svalue(sp, &const0);
}
#endif

#ifdef F_UNIQUE_ARRAY
void
f_unique_array P2(int, num_arg, int, instruction)
{
    struct vector *res;
    struct svalue *arg;
    
    arg = sp - num_arg + 1;
    check_for_destr(arg[0].u.vec);
    if (arg[1].type == T_STRING) {
	res = make_unique(arg[0].u.vec, arg[1].u.string, NULL,
			  (num_arg == 3) ? sp : &const0);
    } else {
	res = make_unique(arg[0].u.vec, NULL, arg[1].u.fp,
			  (num_arg == 3) ? sp : &const0);
    }
    pop_n_elems(num_arg);
    if (res) {
	push_refed_vector(res);
    } else
	push_number(0);
}
#endif

#ifdef F_UPTIME
void
f_uptime P2(int, num_arg, int, instruction)
{
    push_number(current_time - boot_time);
}
#endif

#ifdef F_USERP
void
f_userp P2(int, num_arg, int, instruction)
{
    int i;

    i = (int) sp->u.ob->flags & O_ONCE_INTERACTIVE;
    pop_stack();
    push_number(i != 0);
}
#endif

#ifdef F_USERS
void
f_users P2(int, num_arg, int, instruction)
{
    push_refed_vector(users());
}
#endif

#ifdef F_WIZARDP
void
f_wizardp P2(int, num_arg, int, instruction)
{
    int i;

    i = (int) sp->u.ob->flags & O_IS_WIZARD;
    pop_stack();
    push_number(i != 0);
}
#endif

#ifdef F_VIRTUALP
void
f_virtualp P2(int, num_arg, int, instruction)
{
    int i;

    i = (int) sp->u.ob->flags & O_VIRTUAL;
    pop_stack();
    push_number(i != 0);
}
#endif

#ifdef F_WRITE
void
f_write P2(int, num_arg, int, instruction)
{
    do_write(sp);
    pop_stack();
    push_number(0);
}
#endif

#ifdef F_WRITE_BYTES
void
f_write_bytes P2(int, num_arg, int, instruction)
{
    int i;

    if (IS_ZERO(sp)) {
	bad_arg(3, instruction);
	pop_3_elems();
	push_number(0);
    } else {
	if (sp->type == T_NUMBER) {
	    int netint;
	    char *netbuf;

	    netint = htonl(sp->u.number);	/* convert to network
						 * byte-order */
	    netbuf = (char *) &netint;
	    i = write_bytes((sp - 2)->u.string, (sp - 1)->u.number, netbuf,
			    sizeof(int));
	} else if (sp->type == T_BUFFER) {
	    i = write_bytes((sp - 2)->u.string, (sp - 1)->u.number,
			    (char *) sp->u.buf->item, sp->u.buf->size);
	} else if (sp->type == T_STRING) {
	    i = write_bytes((sp - 2)->u.string, (sp - 1)->u.number,
			    sp->u.string, strlen(sp->u.string));
	} else {
	    bad_argument(sp, T_BUFFER | T_STRING | T_NUMBER, 3, instruction);
	}
	pop_3_elems();
	push_number(i);
    }
}
#endif

#ifdef F_WRITE_BUFFER
void
f_write_buffer P2(int, num_arg, int, instruction)
{
    int i;

    if (IS_ZERO(sp)) {
	bad_arg(3, instruction);
	pop_3_elems();
	push_number(0);
    } else {
	if ((sp - 2)->type == T_STRING) {
	    f_write_bytes(num_arg, instruction);
	    return;
	}
	if (sp->type == T_NUMBER) {
	    int netint;
	    char *netbuf;

	    netint = htonl(sp->u.number);	/* convert to network
						 * byte-order */
	    netbuf = (char *) &netint;
	    i = write_buffer((sp - 2)->u.buf, (sp - 1)->u.number, netbuf,
			     sizeof(int));
	} else if (sp->type == T_BUFFER) {
	    i = write_buffer((sp - 2)->u.buf, (sp - 1)->u.number,
			     (char *) sp->u.buf->item, sp->u.buf->size);
	} else if (sp->type == T_STRING) {
	    i = write_buffer((sp - 2)->u.buf, (sp - 1)->u.number,
			     sp->u.string, strlen(sp->u.string));
	} else {
	    bad_arg(3, instruction);
	}
	pop_3_elems();
	push_number(i);
    }
}
#endif

#ifdef F_WRITE_FILE
void
f_write_file P2(int, num_arg, int, instruction)
{
    int i;
    int flags = 0;

    if (num_arg == 3) {
	CHECK_TYPES(sp, T_NUMBER, 3, instruction);
	flags = sp->u.number;
	sp--;
    }
    i = write_file((sp - 1)->u.string, sp->u.string, flags);
    free_string_svalue(sp--);
    free_string_svalue(sp);
    sp->type = T_NUMBER;
    sp->subtype = 0;
    sp->u.number = i;
}
#endif

#ifdef F_DUMP_FILE_DESCRIPTORS
void
f_dump_file_descriptors P2(int, num_arg, int, instruction)
{
    dump_file_descriptors();
    push_number(0);
}
#endif

#ifdef F_RECLAIM_OBJECTS
extern int reclaim_objects PROT((void));

void f_reclaim_objects P2(int, num_arg, int, instruction)
{
    push_number(reclaim_objects());
}
#endif

#ifdef F_MEMORY_INFO
void
f_memory_info P2(int, num_arg, int, instruction)
{
    struct object *ob;
    int mem;
    extern int total_prog_block_size;
    extern int total_array_size;
    extern int total_mapping_size;
    extern int tot_alloc_sentence;
    extern int tot_alloc_object_size;
    extern char *reserved_area;

    if (num_arg == 0) {
	int res, tot;

	if (reserved_area)
	    res = RESERVED_SIZE;
	else
	    res = 0;
	tot = total_prog_block_size +
	    total_array_size +
	    total_mapping_size +
	    tot_alloc_object_size +
	    tot_alloc_sentence * sizeof(struct sentence) +
	    total_users * sizeof(struct interactive) +
	    show_otable_status(-1) +
	    heart_beat_status(-1) +
	    add_string_status(-1) +
	    print_call_out_usage(-1) + res;
	push_number(tot);
	return;
    }
    if (sp->type != T_OBJECT)
	bad_argument(sp, T_OBJECT, 1, F_MEMORY_INFO);
    ob = sp->u.ob;
    if (ob->prog && (ob->prog->p.i.ref == 1 || !(ob->flags & O_CLONE)))
	mem = ob->prog->p.i.total_size;
    else
	mem = 0;
    mem += (data_size(ob) + sizeof(struct object));
    pop_stack();
    push_number(mem);
}
#endif

#ifdef F_RELOAD_OBJECT
void
f_reload_object P2(int, num_arg, int, instruction)
{
    if (sp->type != T_OBJECT)
	bad_argument(sp, T_OBJECT, 1, F_RELOAD_OBJECT);
    reload_object(sp->u.ob);
    pop_stack();
    push_number(0);
}
#endif

#ifdef F_QUERY_SHADOWING
void
f_query_shadowing P2(int, num_arg, int, instruction)
{
    if ((sp->type == T_OBJECT) && sp->u.ob->shadowing) {
	struct object *ob;

	ob = sp->u.ob;
	pop_stack();
	push_object(ob->shadowing);
    } else
	assign_svalue(sp, &const0);
}
#endif

#ifdef F_SET_RESET
void
f_set_reset P2(int, num_arg, int, instruction)
{
    if (num_arg == 2) {
	(sp - 1)->u.ob->next_reset = current_time + sp->u.number;
	pop_stack();
	assign_svalue(sp, &const0);
    } else {
	sp->u.ob->next_reset = current_time + TIME_TO_RESET / 2 +
	    random_number(TIME_TO_RESET / 2);
	assign_svalue(sp, &const0);
    }
}
#endif

#ifdef F_FLOATP
void
f_floatp P2(int, num_arg, int, instruction)
{
    if (sp->type == T_REAL)
	assign_svalue(sp, &const1);
    else
	assign_svalue(sp, &const0);
}
#endif

#ifdef F_FIRST_INVENTORY
void
f_first_inventory P2(int, num_arg, int, instruction)
{
    extern struct object *first_inventory PROT((struct svalue *));

    ob = first_inventory(sp);
    pop_stack();
    if (ob)
	push_object(ob);
    else
	push_number(0);
}
#endif

#ifdef F_NEXT_INVENTORY
void
f_next_inventory P2(int, num_arg, int, instruction)
{
    ob = sp->u.ob;
    pop_stack();
    ob = ob->next_inv;
    while (ob) {
	if (ob->flags & O_HIDDEN) {
	    if (object_visible(ob)) {
		push_object(ob);
		return;
	    }
	} else {
	    push_object(ob);
	    return;
	}
	ob = ob->next_inv;
    }
    push_number(0);
}
#endif
