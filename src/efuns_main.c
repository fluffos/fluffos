/*
	efuns_main.c: this file contains the efunctions called from
	inside eval_instruction() in interpret.c.  Note: if you are adding
    local efunctions that are specific to your driver, you would be better
    off adding them to a separate source file.  Doing so will make it much
    easier for you to upgrade (won't have to patch this file).  Be sure
    to #include "lpc_incl.h" in that separate source file.
*/

#include "std.h"
#include "lpc_incl.h"
#include "file_incl.h"
#include "file.h"
#include "comm.h"
#include "parse.h"
#include "sprintf.h"
#include "backend.h"
#include "port.h"
#include "swap.h"
#include "strstr.h"
#include "otable.h"
#include "eoperators.h"
#include "crc32.h"
#include "reclaim.h"
#include "dumpstat.h"
#include "efuns_main.h"
#include "call_out.h"
#include "debug.h"
#include "ed.h"
#ifdef LPC_TO_C
#include "interface.h"
#include "compile_file.h"
#endif

int call_origin = 0;

static object_t *ob;

int data_size PROT((object_t * ob));
void reload_object PROT((object_t * obj));

#ifdef F_ADD_ACTION
void
f_add_action PROT((void))
{
    int flag;

    if (st_num_arg == 3) {
	flag = (sp--)->u.number;
    } else flag = 0;

    if (sp->type == T_ARRAY){
	int i, n = sp->u.arr->size;
	svalue_t *sv = sp->u.arr->item;

	for (i = 0; i < n; i++){
	    if (sv[i].type == T_STRING){
		add_action(sp-1, sv[i].u.string, flag);
	    }
	}
	free_array((sp--)->u.arr);
    } else {
	add_action((sp-1), sp->u.string, flag);
	free_string_svalue(sp--);
    }
    pop_stack();
}
#endif

#ifdef F_ALL_INVENTORY
void
f_all_inventory PROT((void))
{
    array_t *vec = all_inventory(sp->u.ob, 0);
    free_object(sp->u.ob, "f_all_inventory");
    sp->type = T_ARRAY;
    sp->u.arr = vec;
}
#endif

#ifdef F_ALLOCATE
void
f_allocate PROT((void))
{
    sp->u.arr = allocate_array(sp->u.number);
    sp->type = T_ARRAY;
}
#endif

#ifdef F_ALLOCATE_BUFFER
void
f_allocate_buffer PROT((void))
{
    buffer_t *buf;

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
f_allocate_mapping PROT((void))
{
    sp->type = T_MAPPING;
    sp->u.map = allocate_mapping(sp->u.number);
}
#endif

#ifdef F_BIND
void
f_bind PROT((void))
{
    object_t *ob = sp->u.ob;
    funptr_t *old_fp = (sp-1)->u.fp;
    funptr_t *new_fp;
    svalue_t *res;

    if (old_fp->hdr.type == (FP_LOCAL | FP_NOT_BINDABLE))
	error("Illegal to rebind a pointer to a local function.\n");
    if (old_fp->hdr.type & FP_NOT_BINDABLE)
	error("Illegal to rebind a functional that references globals or local functions.\n");
    
    /* the object doing the binding */
    if (current_object->flags & O_DESTRUCTED)
	push_number(0);
    else
	push_object(current_object);

    /* the old owner */
    if (old_fp->hdr.owner->flags & O_DESTRUCTED)
	push_number(0);
    else
	push_object(old_fp->hdr.owner);

    /* the new owner */
    push_object(ob);
    
    res = apply_master_ob(APPLY_VALID_BIND, 3);
    if (!MASTER_APPROVED(res))
	error("Master object denied permission to bind() function pointer.\n");
    
    new_fp = ALLOCATE(funptr_t, TAG_FUNP, "f_bind");
    *new_fp = *old_fp;
    new_fp->hdr.owner = ob; /* one ref from being on stack */
    if (new_fp->hdr.args)
	new_fp->hdr.args->ref++;
    if ((old_fp->hdr.type & 0x0f) == FP_FUNCTIONAL) {
	new_fp->f.functional.prog->func_ref++;
#ifdef DEBUG
	if (d_flag)
	    printf("add func ref %s: now %i\n",
		   new_fp->f.functional.prog->name,
		   new_fp->f.functional.prog->func_ref);
#endif	
    }

    free_funp(old_fp);
    sp--;
    sp->u.fp = new_fp;
}
#endif

#ifdef F_BREAK_STRING
void
f_break_string PROT((void))
{
    char *str;
    svalue_t *arg = sp - st_num_arg + 1;

    if (arg->type == T_STRING) {
	str = break_string(arg[0].u.string, arg[1].u.number,
			   (st_num_arg > 2 ? &arg[2] : (svalue_t *) 0));
	if (st_num_arg > 2) free_svalue(sp--, "f_break_string");
	free_string_svalue(--sp);
	sp->subtype = STRING_MALLOC;
	sp->u.string = str;
    } else {
	pop_n_elems(st_num_arg);
	*++sp = const0;
    }
}
#endif

#ifdef F_CACHE_STATS
static void print_cache_stats P1(outbuffer_t *, ob)
{
    outbuf_add(ob, "Function cache information\n");
    outbuf_add(ob, "-------------------------------\n");
    outbuf_addv(ob, "%% cache hits:    %10.2f\n",
	     100 * ((double) apply_low_cache_hits / apply_low_call_others));
    outbuf_addv(ob, "call_others:     %10lu\n", apply_low_call_others);
    outbuf_addv(ob, "cache hits:      %10lu\n", apply_low_cache_hits);
    outbuf_addv(ob, "cache size:      %10lu\n", APPLY_CACHE_SIZE);
    outbuf_addv(ob, "slots used:      %10lu\n", apply_low_slots_used);
    outbuf_addv(ob, "%% slots used:    %10.2f\n",
		100 * ((double) apply_low_slots_used / APPLY_CACHE_SIZE));
    outbuf_addv(ob, "collisions:      %10lu\n", apply_low_collisions);
    outbuf_addv(ob, "%% collisions:    %10.2f\n",
	     100 * ((double) apply_low_collisions / apply_low_call_others));
}

void f_cache_stats PROT((void))
{
    outbuffer_t ob;

    outbuf_zero(&ob);
    print_cache_stats(&ob);
    outbuf_push(&ob);
}
#endif

#ifdef F_CALL_OTHER
 /* enhanced call_other written 930314 by Luke Mewburn <zak@rmit.edu.au> */
void
f_call_other PROT((void))
{
    svalue_t *arg;
    char *funcname;
    int i;
    int num_arg = st_num_arg;


    if (current_object->flags & O_DESTRUCTED) {	/* No external calls allowed */
	pop_n_elems(num_arg);
	push_undefined();
	return;
    }
    arg = sp - num_arg + 1;
    if (arg[1].type == T_STRING)
	funcname = arg[1].u.string;
    else {			/* must be T_ARRAY then */
        array_t *v = arg[1].u.arr;
        svalue_t *sv;

        check_for_destr(v);
        if (((i = v->size) < 1) || !((sv = v->item)->type == T_STRING))
            error("call_other: 1st elem of array for arg 2 must be a string\n");
        funcname = sv->u.string;
        num_arg = 2 + merge_arg_lists(num_arg - 2, v, 1);
    }

    if (arg[0].type == T_OBJECT)
	ob = arg[0].u.ob;
    else if (arg[0].type == T_ARRAY) {
	array_t *ret;

	ret = call_all_other(arg[0].u.arr, funcname, num_arg - 2);
        pop_stack();
        free_array(arg->u.arr);
        sp->u.arr = ret;
	return;
    } else {
        object_t *old_ob;
	ob = find_object(arg[0].u.string);
	if (!(old_ob = ob) || !object_visible(ob))
	    error("call_other() couldn't find object\n");
	ob = old_ob;
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
    free_svalue(--sp, "f_call_other:1");
    free_svalue(--sp, "f_call_other:2");
    *sp = *(sp+2);
    return;
}
#endif

#ifdef F_CALL_OUT
void
f_call_out PROT((void))
{
    svalue_t *arg;

    arg = sp - st_num_arg + 1;
    if (!(current_object->flags & O_DESTRUCTED))
	new_call_out(current_object, arg, arg[1].u.number,
		     st_num_arg - 3, (st_num_arg >= 3) ? &arg[2] : 0);
    pop_n_elems(st_num_arg);
}
#endif

#ifdef F_CALL_OUT_INFO
void
f_call_out_info PROT((void))
{
    push_refed_array(get_all_call_outs());
}
#endif

#ifdef F_CALL_STACK
void
f_call_stack PROT((void))
{
    int i, n = csp - &control_stack[0] + 1;
    array_t *ret = allocate_empty_array(n);

    if (sp->u.number < 0 || sp->u.number > 3)
	error("First argument of call_stack() must be 0, 1, 2, or 3.\n");
    
    switch (sp->u.number) {
    case 0:
	ret->item[0].type = T_STRING;
	ret->item[0].subtype = STRING_SHARED;
	ret->item[0].u.string = current_prog->name;
	ref_string(current_prog->name);
	for (i = 1; i < n; i++) {
	    ret->item[i].type = T_STRING;
	    ret->item[i].subtype = STRING_SHARED;
	    ret->item[i].u.string = make_shared_string((csp - i + 1)->prog->name);
	}
	break;
    case 1:
	ret->item[0].type = T_OBJECT;
	ret->item[0].u.ob = current_object;
	add_ref(current_object, "f_call_stack: curr");
	for (i = 1; i < n; i++) {
	    ret->item[i].type = T_OBJECT;
	    ret->item[i].u.ob = (csp - i + 1)->ob;
	    add_ref((csp - i + 1)->ob, "f_call_stack");
	}
	break;
    case 2:
	for (i = 0; i < n; i++) {
	    ret->item[i].type = T_STRING;
	    if (((csp - i)->framekind & FRAME_MASK) == FRAME_FUNCTION) {
		ret->item[i].subtype = STRING_SHARED;
		ret->item[i].u.string = (csp - i)->fr.func->name;
		ref_string((csp - i)->fr.func->name);
	    } else {
		ret->item[i].subtype = STRING_CONSTANT;
		ret->item[i].u.string = (((csp - i)->framekind & FRAME_MASK) == FRAME_CATCH) ? "CATCH" : "<function>";
	    }
	}
	break;
    case 3:
	for (i = 0; i < n; i++) {
	    ret->item[i].type = T_NUMBER;
	    ret->item[i].u.number = (csp - i)->caller_type;
	}
	break;
    }
    put_array(ret);
}
#endif

#ifdef F_CAPITALIZE
void
f_capitalize PROT((void))
{
    if (islower(sp->u.string[0])) {
	unlink_string_svalue(sp);
	sp->u.string[0] += 'A' - 'a';
    }
}
#endif

#ifdef F_CHILDREN
void
f_children PROT((void))
{
    array_t *vec;

    vec = children(sp->u.string);
    free_string_svalue(sp);
    put_array(vec);
}
#endif

#ifdef F_CLEAR_BIT
void
f_clear_bit PROT((void))
{
    char *str;
    int len, ind, bit;

    if (sp->u.number > MAX_BITS)
        error("clear_bit() bit requested : %d > maximum bits: %d\n", sp->u.number, MAX_BITS);
    bit = (sp--)->u.number;
    if (bit < 0)
	error("Bad argument 2 (negative) to clear_bit().\n");
    ind = bit / 6;
    bit %= 6;
    len = SVALUE_STRLEN(sp);
    if (ind >= len) 
	return;         /* return first arg unmodified */
    unlink_string_svalue(sp);
    str = sp->u.string;

    if (str[ind] > 0x3f + ' ' || str[ind] < ' ')
        error("Illegal bit pattern in clear_bit character %d\n", ind);
    str[ind] = ((str[ind] - ' ') & ~(1 << bit)) + ' ';
}
#endif

#ifdef F_CLONEP
void
f_clonep PROT((void))
{
    if ((sp->type == T_OBJECT) && (sp->u.ob->flags & O_CLONE)){
        free_object(sp->u.ob, "f_clonep");
        *sp = const1;
    } else {
        free_svalue(sp, "f_clonep");
        *sp = const0;
    }
}
#endif

#ifdef F_CLONE_OBJECT
void
f_clone_object PROT((void))
{
    svalue_t *arg = sp - st_num_arg + 1;

    ob = clone_object(arg->u.string, st_num_arg - 1);
    free_string_svalue(sp);
    if (ob) {
	put_unrefed_undested_object(ob, "f_clone_object");
    } else *sp = const0;
}
#endif

#ifdef F_COMMAND
void
f_command PROT((void))
{
    int i;

    i = command_for_object(sp->u.string);
    free_string_svalue(sp);
    put_number(i);
}
#endif

#ifdef F_COMMANDS
void
f_commands PROT((void))
{
    push_refed_array(commands(current_object));
}
#endif

#ifdef F_CP
void
f_cp PROT((void))
{
    int i;

    i = copy_file(sp[-1].u.string, sp[0].u.string);
    free_string_svalue(sp--);
    free_string_svalue(sp);
    put_number(i);
}
#endif

#ifdef F_CRC32
void
f_crc32 PROT((void))
{
    int len;
    unsigned char *buf;
    UINT32 crc;

    if (sp->type == T_STRING) {
        len = SVALUE_STRLEN(sp);
        buf = (unsigned char *) sp->u.string;
    } else if (sp->type == T_BUFFER) {
        len = sp->u.buf->size;
        buf = sp->u.buf->item;
    } else {
        bad_argument(sp, T_STRING | T_BUFFER, 1, F_CRC32);
    }
    crc = compute_crc32(buf, len);
    free_svalue(sp, "f_crc32");
    put_number(crc);
}
#endif

#ifdef F_CREATOR
void
f_creator PROT((void))
{
    ob = sp->u.ob;
    if (!ob->uid){
        free_object(ob, "f_creator");
        *sp = const0;
    } else {
        char *str = ob->uid->name;
        free_object(sp->u.ob, "f_creator");
        put_constant_string(str);
    }
}
#endif				/* CREATOR */

#ifdef F_CTIME
void
f_ctime PROT((void))
{
    char *cp, *nl, *p;
    int l;
    
    cp = time_string(sp->u.number);

    if ((nl = strchr(cp, '\n')))
	l = nl - cp;
    else
	l = strlen(cp);

    p = new_string(l, "f_ctime");
    strncpy(p, cp, l);
    p[l] = '\0';
    put_malloced_string(p);
}
#endif

#ifdef F_DEEP_INHERIT_LIST
void
f_deep_inherit_list PROT((void))
{
    array_t *vec;

    if (!(sp->u.ob->flags & O_SWAPPED)) {
        vec = deep_inherit_list(sp->u.ob);
    } else {
        vec = null_array();
    }
    free_object(sp->u.ob, "f_deep_inherit_list");
    put_array(vec);
}
#endif

#ifdef DEBUG_MACRO

#ifdef F_SET_DEBUG_LEVEL
void
f_set_debug_level PROT((void))
{
    debug_level = (sp--)->u.number;
}
#endif

#endif

#ifdef F_DEEP_INVENTORY
void
f_deep_inventory PROT((void))
{
    array_t *vec;

    vec = deep_inventory(sp->u.ob, 0);
    free_object(sp->u.ob, "f_deep_inventory");
    put_array(vec);
}
#endif

#ifdef F_DESTRUCT
void
f_destruct PROT((void))
{
    destruct_object(sp);
    sp--; /* Ok since the object was removed from the stack */
}
#endif

#ifdef F_DISABLE_COMMANDS
void
f_disable_commands PROT((void))
{
    enable_commands(0);
}
#endif

#ifdef F_DUMPALLOBJ
void
f_dumpallobj PROT((void))
{
    if (st_num_arg) {
        dumpstat(sp->u.string);
        free_string_svalue(sp--);
    } else {
        dumpstat("/OBJ_DUMP");
    }
}
#endif

/* f_each */

#ifdef F_EACH
void
f_each PROT((void))
{
    mapping_t *m;
    array_t *v;
    int flag = (sp--)->u.number;

    m = sp->u.map;
    if (flag) {
        m->eachObj = current_object;
        m->bucket = 0;
        m->elt = (mapping_node_t *) 0;
        free_mapping(m);
        *sp = const0;
        return;
    }
    v = mapping_each(m);
    free_mapping(m);
    put_array(v);
}
#endif

#ifdef F_ED
void
f_ed PROT((void))
{
    if (!command_giver || !command_giver->interactive) {
	pop_n_elems(st_num_arg);
        return;
    }

    if (!st_num_arg) {
        /* ed() */
        ed_start(0, 0, 0, 0, 0);
    } else if (st_num_arg == 1) {
        /* ed(fname) */
        ed_start(sp->u.string, 0, 0, 0, 0);
	pop_stack();
    } else if (st_num_arg == 2) {
        /* ed(fname,exitfn) */
        ed_start((sp - 1)->u.string, 0, sp->u.string, 0, current_object);
        pop_2_elems();
    } else if (st_num_arg == 3) {
        /* ed(fname,exitfn,restricted) / ed(fname,writefn,exitfn) */
        if (sp->type == T_NUMBER) {
            ed_start((sp - 2)->u.string, 0, (sp - 1)->u.string, sp->u.number,
                     current_object);
	} else if (sp->type == T_STRING) {
            ed_start((sp - 2)->u.string, (sp - 1)->u.string, sp->u.string, 0,
                     current_object);
	} else {
            bad_argument(sp, T_NUMBER | T_STRING, 3, F_ED);
	}
        pop_3_elems();
    } else {                    /* st_num_arg == 4 */
        /* ed(fname,writefn,exitfn,restricted) */
        if (!((sp - 1)->type == T_STRING))
            bad_argument(sp - 1, T_STRING, 3, F_ED);
        if (!(sp->type == T_NUMBER))
            bad_argument(sp, T_NUMBER, 4, F_ED);
        ed_start((sp - 3)->u.string, (sp - 2)->u.string, (sp - 1)->u.string,
                 sp->u.number, current_object);
	pop_n_elems(4);
    }
}
#endif

#ifdef F_ED_CMD
void f_ed_cmd PROT((void))
{
    char *res;
    
    if (current_object->flags & O_DESTRUCTED)
	error("destructed objects can't use ed.\n");

    if (!(current_object->flags & O_IN_EDIT))
	error("ed_cmd() called with no ed session active.\n");

    res = object_ed_cmd(current_object, sp->u.string);

    free_string_svalue(sp);
    if (res) {
	sp->subtype = STRING_MALLOC;
	sp->u.string = res;
    } else {
	sp->subtype = STRING_CONSTANT;
	sp->u.string = "";
    }
}
#endif

#ifdef F_ED_START
void f_ed_start PROT((void))
{
    char *res;
    char *fname;
    int restr = 0;

    if (st_num_arg == 2)
	restr = (sp--)->u.number;

    if (st_num_arg)
	fname = sp->u.string;
    else
	fname = 0;

    if (current_object->flags & O_DESTRUCTED)
	error("destructed objects can't use ed.\n");

    if (current_object->flags & O_IN_EDIT)
	error("ed_start() called while an ed session is already started.\n");

    res = object_ed_start(current_object, fname, restr);

    if (fname) free_string_svalue(sp);
    
    if (res) {
	sp->subtype = STRING_MALLOC;
	sp->u.string = res;
    } else {
	sp->subtype = STRING_CONSTANT;
	sp->u.string = "";
    }
}
#endif

#ifdef F_ENABLE_COMMANDS
void
f_enable_commands PROT((void))
{
    enable_commands(1);
}
#endif

#ifdef F_ENABLE_WIZARD
void
f_enable_wizard PROT((void))
{
    if (current_object->interactive)
	current_object->flags |= O_IS_WIZARD;
}
#endif

#ifdef F_ERROR
void
f_error PROT((void))
{
    int l = SVALUE_STRLEN(sp);
    char err_buf[2048];

    if (sp->u.string[l - 1] == '\n')
	l--;
    if (l > 2045) l = 2045;
    
    err_buf[0] = '*';
    strncpy(err_buf + 1, sp->u.string, l);
    err_buf[l + 1] = '\n';
    err_buf[l + 2] = 0;
    
    error_handler(sp->u.string);
}
#endif

#ifdef F_DISABLE_WIZARD
void
f_disable_wizard PROT((void))
{
    if (current_object->interactive)
	current_object->flags &= ~O_IS_WIZARD;
}
#endif

#ifdef F_ENVIRONMENT
void
f_environment PROT((void))
{
    if (st_num_arg) {
        if ((ob = sp->u.ob)->flags & O_DESTRUCTED)
            error("environment() of destructed object.\n");
        ob = ob->super;
        free_object((sp--)->u.ob, "f_environment");
    } else if (!(current_object->flags & O_DESTRUCTED))
        ob = current_object->super;
    else error("environment() of destructed object.\n");

    if (ob) push_object(ob);
    else push_number(0);
}
#endif

#ifdef F_EXEC
void
f_exec PROT((void))
{
    int i;

    i = replace_interactive((sp - 1)->u.ob, sp->u.ob);

    /* They might have been destructed */
    if (sp->type == T_OBJECT)
	free_object(sp->u.ob, "f_exec:1");
    if ((--sp)->type == T_OBJECT)
	free_object(sp->u.ob, "f_exec:2");
    put_number(i);
}
#endif

#ifdef F_EXPLODE
void
f_explode PROT((void))
{
    array_t *vec;

    vec = explode_string((sp - 1)->u.string, SVALUE_STRLEN(sp-1),
			 sp->u.string, SVALUE_STRLEN(sp));
    free_string_svalue(sp--);
    free_string_svalue(sp);
    put_array(vec);
}
#endif

#ifdef F_FILE_NAME
void
f_file_name PROT((void))
{
    char *res;

    /* This function now returns a leading '/' */
    res = (char *) add_slash(sp->u.ob->name);
    free_object(sp->u.ob, "f_file_name");
    put_malloced_string(res);
}
#endif

#ifdef F_FILE_SIZE
void
f_file_size PROT((void))
{
    int i = file_size(sp->u.string);
    free_string_svalue(sp);
    put_number(i);
}
#endif

#ifdef F_FILTER
void
f_filter PROT((void))
{
    svalue_t *arg = sp - st_num_arg + 1;

    if (arg->type == T_MAPPING) filter_mapping(arg, st_num_arg);
    else filter_array(arg, st_num_arg);
}
#endif

#ifdef F_FIND_CALL_OUT
void
f_find_call_out PROT((void))
{
    int i = find_call_out(current_object, sp->u.string);
    free_string_svalue(sp);
    put_number(i);
}
#endif

#ifdef F_FIND_LIVING
void
f_find_living PROT((void))
{
    ob = find_living_object(sp->u.string, 0);
    free_string_svalue(sp);
    /* safe b/c destructed objects have had their living names removed */
    if (ob) { put_unrefed_undested_object(ob, "find_living"); }
    else *sp = const0;
}
#endif

#ifdef F_FIND_OBJECT
void
f_find_object PROT((void))
{
    if ((sp--)->u.number)
        ob = find_object(sp->u.string);
    else
        ob = find_object2(sp->u.string);
    free_string_svalue(sp);
    if (ob) {
	object_t *old_ob = ob;
	/* object_visible might change ob, a global - Sym */
        if (object_visible(ob)) { 
	  /* find_object only returns undested objects */
	  put_unrefed_undested_object(old_ob, "find_object"); 
	}
        else *sp = const0;
    } else *sp = const0;
}
#endif

#ifdef F_FIND_PLAYER
void
f_find_player PROT((void))
{
    ob = find_living_object(sp->u.string, 1);
    free_string_svalue(sp);
    if (ob) { 
      put_unrefed_undested_object(ob, "find_player"); 
    }
    else *sp = const0;
}
#endif

#ifdef F_FUNCTION_PROFILE
/* f_function_profile: John Garnett, 1993/05/31, 0.9.17.3 */
void
f_function_profile PROT((void))
{
    array_t *vec;
    mapping_t *map;
    program_t *prog;
    int nf, j;

    ob = sp->u.ob;
    if (ob->flags & O_SWAPPED) {
	load_ob_from_swap(ob);
    }
    prog = ob->prog;
    nf = prog->num_functions;
    vec = allocate_empty_array(nf);
    for (j = 0; j < nf; j++) {
	map = allocate_mapping(3);
	add_mapping_pair(map, "calls", prog->functions[j].calls);
	add_mapping_pair(map, "self", prog->functions[j].self
			 - prog->functions[j].children);
	add_mapping_pair(map, "children", prog->functions[j].children);
	add_mapping_shared_string(map, "name", prog->functions[j].name);
	vec->item[j].type = T_MAPPING;
	vec->item[j].u.map = map;
    }
    free_object(ob, "f_function_profile");
    put_array(vec);
}
#endif

#ifdef F_FUNCTION_EXISTS
void
f_function_exists PROT((void))
{
    char *str, *res;
    int l;

    str = function_exists((sp - 1)->u.string, sp->u.ob);
    free_object((sp--)->u.ob, "f_function_exists");
    free_string_svalue(sp);
    if (str) {
	l = SHARED_STRLEN(str) - 2; /* no .c */
	res = new_string(l + 1, "function_exists");
	res[0] = '/';
	strncpy(res + 1, str, l);
	res[l + 1] = 0;

	sp->subtype = STRING_MALLOC;
        sp->u.string = res;
    } else *sp = const0;
}
#endif

#ifdef F_GENERATE_SOURCE
void f_generate_source PROT((void))
{
    int i;

    if (st_num_arg == 2) {
	i = generate_source(sp - 1, sp->u.string);
	pop_stack();
    } else
	i = generate_source(sp, 0);
    free_svalue(sp, "f_generate_source");
    put_number(i);
}
#endif

#ifdef F_GET_CHAR
void
f_get_char PROT((void))
{
    svalue_t *arg;
    int i, tmp;
    int flag;

    arg = sp - st_num_arg + 1;  /* Points arg at first argument. */
    if (st_num_arg == 1 || !(arg[1].type == T_NUMBER)) {
        tmp = 0;
        flag = 0;
    } else {
        tmp = 1;
        st_num_arg--;           /* Don't count the flag as an arg */
        flag = arg[1].u.number;
    }
    st_num_arg--;
    i = get_char(arg, flag, st_num_arg, &arg[1 + tmp]);
    free_svalue(arg, "f_get_char");
    (sp = arg)->type = T_NUMBER;
    sp->u.number = i;
}
#endif

#ifdef F_GET_CONFIG
void
f_get_config PROT((void))
{
    if (!get_config_item(sp, sp))
	error("Bad argument to get_config()\n");
}
#endif

#ifdef F_GET_DIR
void
f_get_dir PROT((void))
{
    array_t *vec;

    vec = get_dir((sp - 1)->u.string, sp->u.number);
    free_string_svalue(--sp);
    if (vec) { put_array(vec); }
    else *sp = const0;
}
#endif

#ifdef F_IMPLODE
void
f_implode PROT((void))
{
    array_t *arr;
    int flag;
    svalue_t *args;
    
    if (st_num_arg == 3) {
	args = (sp - 2);
	if (args[1].type == T_STRING)
	    error("Third argument to implode() is illegal with implode(array, string)\n");
	flag = 1;
    } else {
	args = (sp - 1);
	flag = 0;
    }
    arr = args->u.arr;
    check_for_destr(arr);

    if (args[1].type == T_STRING) {
	/* st_num_arg == 2 here */
	char *str;
	    
	str = implode_string(arr, sp->u.string,
			     SVALUE_STRLEN(sp));
	free_string_svalue(sp--);
	free_array(arr);
	put_malloced_string(str);
    } else { /* function */
	funptr_t *funp = args[1].u.fp;
	    
	/* this pulls the extra arg off the stack if it exists */
	implode_array(funp, arr, args, flag);
	sp--;
	free_funp(funp);
	free_array(arr);
    }
}
#endif

#ifdef F_IN_EDIT
void
f_in_edit PROT((void))
{
    char *fn;
    ed_buffer_t *eb = 0;

#ifdef OLD_ED
    if (sp->u.ob->interactive)
	eb = sp->u.ob->interactive->ed_buffer;
#else
    if (sp->u.ob->flags & O_IN_EDIT)
	eb = find_ed_buffer(sp->u.ob);
#endif
    if (eb && (fn = eb->fname)) {
	free_object(sp->u.ob, "f_in_edit:1");
	put_constant_string(fn); /* is this safe?  - Beek */
	return;
    }
    free_object(sp->u.ob, "f_in_edit:1");
    *sp = const0;
    return;
}
#endif

#ifdef F_IN_INPUT
void
f_in_input PROT((void))
{
    int i;

    i = sp->u.ob->interactive && sp->u.ob->interactive->input_to;
    free_object(sp->u.ob, "f_in_input");
    put_number(i != 0);
}
#endif

#ifdef F_INHERITS
int
inherits P2(program_t *, prog, program_t *, thep)
{
    int j, k = prog->num_inherited;
    program_t *pg;

    for (j = 0; j < k; j++) {
        if ((pg = prog->inherit[j].prog) == thep)
            return 1;
        if (!strcmp(pg->name, thep->name))
            return 2;
        if (inherits(pg, thep))
            return 1;
    }
    return 0;
}

void
f_inherits PROT((void))
{
    object_t *ob, *base;
    int i;

    base = (sp--)->u.ob;
    ob = find_object2(sp->u.string);
    if (!ob || (ob->flags & O_SWAPPED)) {
	free_object(base, "f_inherits");
        assign_svalue(sp, &const0);
        return;
    }
    if (base->flags & O_SWAPPED)
        load_ob_from_swap(base);
    i = inherits(base->prog, ob->prog);
    free_object(base, "f_inherits");
    free_string_svalue(sp);
    put_number(i);
}
#endif

#ifdef F_INHERIT_LIST
void
f_inherit_list PROT((void))
{
    array_t *vec;

    if (!(sp->u.ob->flags & O_SWAPPED)) {
        vec = inherit_list(sp->u.ob);
    } else {
        vec = null_array();
    }
    free_object(sp->u.ob, "f_inherit_list");
    put_array(vec);
}
#endif

#ifdef F_INPUT_TO
void
f_input_to PROT((void))
{
    svalue_t *arg;
    int i, tmp;
    int flag;

    arg = sp - st_num_arg + 1;  /* Points arg at first argument. */
    if ((st_num_arg == 1) || !(arg[1].type == T_NUMBER)) {
        tmp = flag = 0;
    } else {
        tmp = 1;
        st_num_arg--;           /* Don't count the flag as an arg */
        flag = arg[1].u.number;
    }
    st_num_arg--;               /* Don't count the name of the func either. */
    i = input_to(arg, flag, st_num_arg, &arg[1 + tmp]);
    free_svalue(arg, "f_input_to");
    (sp = arg)->type = T_NUMBER;
    sp->u.number = i;
}
#endif

#ifdef F_INTERACTIVE
void
f_interactive PROT((void))
{
    int i;

    i = (sp->u.ob->interactive != 0);
    free_object(sp->u.ob, "f_interactive");
    put_number(i);
}
#endif

#ifdef F_INTP
void
f_intp PROT((void))
{
    if (sp->type == T_NUMBER) sp->u.number = 1;
    else {
	free_svalue(sp, "f_intp");
        put_number(0);
    }
}
#endif

#ifdef F_FUNCTIONP
void
f_functionp PROT((void))
{
    int i;
    
    if (sp->type == T_FUNCTION) {
        i = sp->u.fp->hdr.type;
        if (sp->u.fp->hdr.args) 
	    i |= FP_HAS_ARGUMENTS;
	if (sp->u.fp->hdr.owner->flags & O_DESTRUCTED)
	    i |= FP_OWNER_DESTED;
        free_funp(sp->u.fp);
        put_number(i);
        return;
    }
    assign_svalue(sp, &const0);
}
#endif

#ifdef F_KEYS
void
f_keys PROT((void))
{
    array_t *vec;

    vec = mapping_indices(sp->u.map);
    free_mapping(sp->u.map);
    put_array(vec);
}
#endif

#ifdef F_VALUES
void
f_values PROT((void))
{
    array_t *vec;

    vec = mapping_values(sp->u.map);
    free_mapping(sp->u.map);
    put_array(vec);
}
#endif

#ifdef F_LINK
void
f_link PROT((void))
{
    svalue_t *ret;
    int i;

    push_svalue(sp - 1);
    push_svalue(sp);
    ret = apply_master_ob(APPLY_VALID_LINK, 2);
    if (MASTER_APPROVED(ret))
        i = do_rename((sp - 1)->u.string, sp->u.string, F_LINK);
    else
	i = 0;
    (--sp)->type = T_NUMBER;
    sp->u.number = i;
    sp->subtype = 0;
}
#endif				/* F_LINK */

#ifdef F_LIVING
void
f_living PROT((void))
{
    if (sp->u.ob->flags & O_ENABLE_COMMANDS){
        free_object(sp->u.ob, "f_living:1");
        *sp = const1;
    }
    else {
        free_object(sp->u.ob, "f_living:2");
        *sp = const0;
    }
}
#endif

#ifdef F_LIVINGS
void
f_livings PROT((void))
{
    push_refed_array(livings());
}
#endif

#ifdef F_LOWER_CASE
void
f_lower_case PROT((void))
{
    register char *str;

    str = sp->u.string;
    /* find first upper case letter, if any */
    for (; *str; str++) {
	if (isupper(*str)) {
	    int l = str - sp->u.string;
	    unlink_string_svalue(sp);
	    str = sp->u.string + l;
	    *str += 'a' - 'A';
	    for (str++; *str; str++) {
		if (isupper(*str))
		    *str += 'a' - 'A';
	    }
	    return;
	}
    }
}
#endif

#ifdef F_LPC_INFO
void f_lpc_info PROT((void))
{
    outbuffer_t out;

    interface_t **p = interface;
    object_t *ob;

    outbuf_zero(&out);
    outbuf_addv(&out, "%30s  Loaded  Using compiled program\n", "Program");
    while (*p) {
	outbuf_addv(&out, "%30s: ", (*p)->fname);
	ob = lookup_object_hash((*p)->fname);
	if (ob) {
	    if (ob->flags & O_COMPILED_PROGRAM) {
		outbuf_add(&out, " No\n");
	    } else if (ob->prog->program_size == 0) {
		outbuf_add(&out, " Yes      Yes\n");
	    } else {
		outbuf_add(&out, " Yes      No\n");
	    }
	} else {
	    outbuf_add(&out, "Something REALLY wierd happened; no record of the object.\n");
	}
	p++;
    }
    outbuf_push(&out);
}
#endif

#ifdef F_MALLOC_STATUS
void f_malloc_status PROT((void))
{
    outbuffer_t ob;

    outbuf_zero(&ob);

#ifdef BSDMALLOC
    outbuf_add(&ob, "Using BSD malloc");
#endif
#ifdef SMALLOC
    outbuf_add(&ob, "Using Smalloc");
#endif
#ifdef SYSMALLOC
    outbuf_add(&ob, "Using system malloc");
#endif
#ifdef DEBUGMALLOC
    outbuf_add(&ob, ", wrapped with debugmalloc");
#endif
#ifdef WRAPPEDMALLOC
    outbuf_add(&ob, ", wrapped with wrappedmalloc");
#endif
    outbuf_add(&ob, ".\n");
#ifdef DO_MSTATS
    show_mstats(&ob, "malloc_status()");
#endif
#if (defined(WRAPPEDMALLOC) || defined(DEBUGMALLOC))
    dump_malloc_data(&ob);
#endif
    outbuf_push(&ob);
}
#endif

#ifdef F_MAP_DELETE
void
f_map_delete PROT((void))
{
    mapping_delete((sp - 1)->u.map, sp);
    pop_stack();
    free_mapping((sp--)->u.map);
}
#endif

#ifdef F_MAPP
void
f_mapp PROT((void))
{
    if (sp->type == T_MAPPING){
        free_mapping(sp->u.map);
        *sp = const1;
    } else {
        free_svalue(sp, "f_mapp");
        *sp = const0;
    }
}
#endif

#ifdef F_MAP
void
f_map PROT((void))
{
    svalue_t *arg = sp - st_num_arg + 1;

    if (arg->type == T_MAPPING) map_mapping(arg, st_num_arg);
    else if (arg->type == T_ARRAY) map_array(arg, st_num_arg);
    else map_string(arg, st_num_arg);
}
#endif

#ifdef F_MASTER
void
f_master PROT((void))
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
f_match_path PROT((void))
{
    svalue_t *value;
    svalue_t string;
    register char *src, *dst;
    svalue_t *nvalue;
    mapping_t *map;

    value = &const0u;

    string.type = T_STRING;
    string.subtype = STRING_CONSTANT;
    string.u.string = DMALLOC(SVALUE_STRLEN(sp) + 1, TAG_STRING, "match_path");

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
    free_string_svalue(sp--);
    map = sp->u.map;
    assign_svalue_no_free(sp, value);
    free_mapping(map);
}
#endif				/* F_MATCH_PATH */

#ifdef F_MEMBER_ARRAY
void
f_member_array PROT((void))
{
    array_t *v;
    int i;

    if (st_num_arg > 2) {
        i = (sp--)->u.number;
        if (i<0) bad_arg(3, F_MEMBER_ARRAY);
    } else i = 0;

    if (sp->type == T_STRING) {
        char *res;
        CHECK_TYPES(sp-1, T_NUMBER, 1, F_MEMBER_ARRAY);
        if (i > SVALUE_STRLEN(sp)) error("Index to start search from in member_array() is > string length.\n");
        if ((res = strchr(sp->u.string + i, (sp-1)->u.number)))
            i = res - sp->u.string;
        else
            i = -1;
        free_string_svalue(sp--);
    } else {
        int size = (v = sp->u.arr)->size;
        svalue_t *sv;
	svalue_t *find;
	int flen;

        find = (sp - 1);
	/* optimize a bit */
	if (find->type == T_STRING) {
	    /* *not* COUNTED_STRLEN() which can do a (costly) strlen() call */
	    if (find->subtype & STRING_COUNTED)
		flen = MSTR_SIZE(find->u.string);
	    else flen = 0;
	}

        for (; i < size; i++) {
            switch (find->type|(sv= v->item + i)->type) {
            case T_STRING:
		if (flen && (sv->subtype & STRING_COUNTED)
		    && flen != MSTR_SIZE(sv->u.string))
		    continue;
                if (strcmp(find->u.string, sv->u.string)) continue;
		break;
            case T_NUMBER:
                if (find->u.number == sv->u.number) break;
                continue;
            case T_REAL:
                if (find->u.real == sv->u.real) break;
                continue;
            case T_ARRAY:
                if (find->u.arr == sv->u.arr) break;
                continue;
            case T_OBJECT:
	    {
		if (sv->u.ob->flags & O_DESTRUCTED){
		    assign_svalue(sv, &const0);
		    continue;
		}
                if (find->u.ob == sv->u.ob) break;
                continue;
	    }
            case T_MAPPING:
                if (find->u.map == sv->u.map) break;
                continue;
            case T_FUNCTION:
                if (find->u.fp == sv->u.fp) break;
                continue;
            case T_BUFFER:
                if (find->u.buf == sv->u.buf) break;
                continue;
            default:
                if (sv->type == T_OBJECT && sv->u.ob->flags & O_DESTRUCTED){
                    assign_svalue(sv, &const0);
                    if (find->type == T_NUMBER && !find->u.number) break;
		}
                continue;
	    }
            break;
	}
        if (i == size)
            i = -1;                     /* Return -1 for failure */
        free_array(v);
	free_svalue(find, "f_member_array");
	sp--;
    }
    put_number(i);
}
#endif

#ifdef F_MESSAGE
void
f_message PROT((void))
{
    array_t *use, *avoid;
    int num_arg = st_num_arg;
    svalue_t *args;

    static array_t vtmp1 =
    {1,
#ifdef DEBUG
     1,
#endif
     1,
#ifdef PACKAGE_MUDLIB_STATS
     {(mudlib_stats_t *) NULL, (mudlib_stats_t *) NULL}
#endif
    };
    static array_t vtmp2 =
    {1,
#ifdef DEBUG
     1,
#endif
     1,
#ifdef PACKAGE_MUDLIB_STATS
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
    case T_ARRAY:
	use = args[2].u.arr;
	break;
    case T_STRING:
	vtmp1.item[0].type = T_STRING;
	vtmp1.item[0].u.string = args[2].u.string;
	use = &vtmp1;
	break;
    case T_NUMBER:
	if (args[2].u.number == 0) {
	    /* this is really bad and probably should be rm'ed -Beek */
	    /* for compatibility (write() simul_efuns, etc)  -bobf */
	    check_legal_string(args[1].u.string);
	    add_message(command_giver, args[1].u.string);
	    pop_n_elems(num_arg);
	    return;
	}
    default:
	bad_argument(&args[2], T_OBJECT | T_STRING | T_ARRAY | T_NUMBER,
		     3, F_MESSAGE);
    }
    if (num_arg == 4) {
	switch (args[3].type) {
	case T_OBJECT:
	    vtmp2.item[0].type = T_OBJECT;
	    vtmp2.item[0].u.ob = args[3].u.ob;
	    avoid = &vtmp2;
	    break;
	case T_ARRAY:
	    avoid = args[3].u.arr;
	    break;
	default:
	    avoid = null_array();
	}
    } else
	avoid = null_array();
    do_message(&args[0], args[1].u.string, use, avoid, 1);
    pop_n_elems(num_arg);
    return;
}
#endif

#ifdef F_MKDIR
void
f_mkdir PROT((void))
{
    char *path;

    path = check_valid_path(sp->u.string, current_object, "mkdir", 1);
    if (!path || mkdir(path, 0770) == -1){
        free_string_svalue(sp);
        *sp = const0;
    }
    else{
        free_string_svalue(sp);
        *sp = const1;
    }
}
#endif

#ifdef F_MOVE_OBJECT
void
f_move_object PROT((void))
{
    object_t *o1, *o2;

    /* get destination */
    if (sp->type == T_OBJECT)
        o2 = sp->u.ob;
    else {
        if (!(o2 = find_object(sp->u.string)) || !object_visible(o2))
            error("move_object failed: could not find destination\n");
    }
    
    if ((o1 = current_object)->flags & O_DESTRUCTED)
	error("move_object(): can't move a destructed object\n");
    
    move_object(o1, o2);
    pop_stack();
}
#endif

#ifdef F_MUD_STATUS
void f_mud_status PROT((void))
{
    int tot, res, verbose = 0;
    outbuffer_t ob;

    outbuf_zero(&ob);
    verbose = (sp--)->u.number;

    if (reserved_area)
	res = RESERVED_SIZE;
    else
	res = 0;

    if (verbose) {
	char dir_buf[1024];
	FILE *testfp;

	if ((testfp = fopen(".mudos_test_file", "w"))) {
	    fclose(testfp);
	    outbuf_add(&ob, "Open-file-test succeeded.\n");
	    unlink(".mudos_test_file");
	} else {
	    /* if strerror() is missing, edit the #ifdef for it in port.c */
	    outbuf_addv(&ob, "Open file test failed: %s\n", strerror(errno));
	}

	outbuf_addv(&ob, "current working directory: %s\n\n",
		    get_current_dir(dir_buf, 1024));
	outbuf_add(&ob, "add_message statistics\n");
	outbuf_add(&ob, "------------------------------\n");
	outbuf_addv(&ob, "Calls to add_message: %d   Packets: %d   Average packet size: %f\n\n",
	add_message_calls, inet_packets, (float) inet_volume / inet_packets);

#ifndef NO_ADD_ACTION
	stat_living_objects(&ob);
	outbuf_add(&ob, "\n");
#endif
#ifdef F_CACHE_STATS
	print_cache_stats(&ob);
	outbuf_add(&ob, "\n");
#endif
	print_swap_stats(&ob);
	outbuf_add(&ob, "\n");

        tot = show_otable_status(&ob, verbose);
        outbuf_add(&ob, "\n");
        tot += heart_beat_status(&ob, verbose);
        outbuf_add(&ob, "\n");
        tot += add_string_status(&ob, verbose);
        outbuf_add(&ob, "\n");
        tot += print_call_out_usage(&ob, verbose);
    } else {
	/* !verbose */
	outbuf_addv(&ob, "Sentences:\t\t\t%8d %8d\n", tot_alloc_sentence,
		    tot_alloc_sentence * sizeof(sentence_t));
	outbuf_addv(&ob, "Objects:\t\t\t%8d %8d\n",
		    tot_alloc_object, tot_alloc_object_size);
	outbuf_addv(&ob, "Prog blocks:\t\t\t%8d %8d\n",
		    total_num_prog_blocks, total_prog_block_size);
	outbuf_addv(&ob, "Arrays:\t\t\t\t%8d %8d\n", num_arrays,
		    total_array_size);
	outbuf_addv(&ob, "Mappings:\t\t\t%8d %8d\n", num_mappings,
		    total_mapping_size);
	outbuf_addv(&ob, "Mappings(nodes):\t\t%8d\n", total_mapping_nodes);
	outbuf_addv(&ob, "Interactives:\t\t\t%8d %8d\n", total_users,
		    total_users * sizeof(interactive_t));

	tot = show_otable_status(&ob, verbose) +
	    heart_beat_status(&ob, verbose) +
	    add_string_status(&ob, verbose) +
	    print_call_out_usage(&ob, verbose);
    }

    tot += total_prog_block_size +
	total_array_size +
	total_mapping_size +
	tot_alloc_sentence * sizeof(sentence_t) +
	tot_alloc_object_size +
	total_users * sizeof(interactive_t) +
	res;

    if (!verbose) {
	outbuf_add(&ob, "\t\t\t\t\t --------\n");
	outbuf_addv(&ob, "Total:\t\t\t\t\t %8d\n", tot);
    }
    outbuf_push(&ob);
}
#endif

#ifdef F_NOTIFY_FAIL
void
f_notify_fail PROT((void))
{
    if (sp->type == T_STRING) {
	set_notify_fail_message(sp->u.string);
	free_string_svalue(sp--);
    }
    else {
	set_notify_fail_function(sp->u.fp);
	free_funp((sp--)->u.fp);
    }
}
#endif

#ifdef F_NULLP
void
f_nullp PROT((void))
{
    if (sp->type == T_NUMBER){
        if (!sp->u.number && (sp->subtype == T_NULLVALUE)) {
	    sp->subtype = 0;
            sp->u.number = 1;
	} else sp->u.number = 0;
    } else {
        free_svalue(sp, "f_nullp");
        *sp = const0;
    }
}
#endif

#ifdef F_OBJECTP
void
f_objectp PROT((void))
{
    if (sp->type == T_OBJECT){
        free_object(sp->u.ob, "f_objectp");
        *sp = const1;
    } else {
        free_svalue(sp, "f_objectp");
        *sp = const0;
    }
}
#endif

#ifdef F_OPCPROF
void
f_opcprof PROT((void))
{
    if (st_num_arg == 1) {
        opcdump(sp->u.string);
        free_string_svalue(sp--);
    } else
	opcdump("/OPCPROF");
}
#endif

#ifdef F_ORIGIN
void
f_origin PROT((void))
{
    int i = 0, j;
    static char *origins[] = {
	"driver",
	"local",
	"call_other",
	"simul",
	"call_out",
	"efun",
	"function pointer",
	"functional"
    };
    j = caller_type;
    while (j >>= 1) i++;
    push_constant_string(origins[i]);
}
#endif

#ifdef F_POINTERP
void
f_pointerp PROT((void))
{
    if (sp->type == T_ARRAY){
        free_array(sp->u.arr);
        *sp = const1;
    } else {
        free_svalue(sp, "f_pointerp");
        *sp = const0;
    }
}
#endif

#ifdef F_PRESENT
void
f_present PROT((void))
{
    int num_arg = st_num_arg;
    svalue_t *arg = sp - num_arg + 1;

#ifdef LAZY_RESETS
    if (num_arg == 2) {
	try_reset(arg[1].u.ob);
    }
#endif
    ob = object_present(arg, num_arg == 1 ? 0 : arg[1].u.ob);
    pop_n_elems(num_arg);
    if (ob) push_object(ob);
    else *++sp = const0;
}
#endif

#ifdef F_PREVIOUS_OBJECT
void
f_previous_object PROT((void))
{
    control_stack_t *p;
    int i;

    if ((i = sp->u.number) > 0) {
        if (i >= MAX_TRACE) {
            sp->u.number = 0;
            return;
	}
        ob = 0;
        p = csp;
        do {
            if ((p->framekind & FRAME_OB_CHANGE) && !(--i)) {
                ob = p->prev_ob;
                break;
	    }
	} while (--p >= control_stack);
    } else if (i == -1) {
        array_t *v;

        i = previous_ob ? 1 : 0;
        p = csp;
        do {
            if ((p->framekind & FRAME_OB_CHANGE) && p->prev_ob) i++;
	} while (--p >= control_stack);
        v = allocate_empty_array(i);
        p = csp;
        if (previous_ob){
	    if (!(previous_ob->flags & O_DESTRUCTED)) {
		v->item[0].type = T_OBJECT;
		v->item[0].u.ob = previous_ob;
		add_ref(previous_ob, "previous_object(-1)");
	    } else v->item[0] = const0;
            i = 1;
	} else i = 0;
        do {
            if ((p->framekind & FRAME_OB_CHANGE) && (ob = p->prev_ob)) {
		if (!(ob->flags & O_DESTRUCTED)){
		    v->item[i].type = T_OBJECT;
		    v->item[i].u.ob = ob;
		    add_ref(ob, "previous_object(-1)");
		} else v->item[i] = const0;
		i++;
	    }
	} while (--p >= control_stack);
        put_array(v);
        return;
    } else if (i < 0) {
        error("Illegal negative argument to previous_object()\n");
    } else
        ob = previous_ob;
    if (!ob || (ob->flags & O_DESTRUCTED))
        sp->u.number = 0;
    else {
        put_unrefed_undested_object(ob, "previous_object()");
    }
}
#endif

#ifdef F_PRINTF
void
f_printf PROT((void))
{
    int num_arg = st_num_arg;
    if (command_giver)
        tell_object(command_giver, string_print_formatted((sp - num_arg + 1)->u.string,
                                            num_arg - 1, sp - num_arg + 2));
    pop_n_elems(num_arg);
}
#endif

#ifdef F_PROCESS_STRING
void
f_process_string PROT((void))
{
    char *str;

    str = process_string(sp->u.string);
    if (str != sp->u.string) {
        free_string_svalue(sp);
        put_malloced_string(str);
    }
}
#endif

#ifdef F_PROCESS_VALUE
void
f_process_value PROT((void))
{
    svalue_t *ret;

    ret = process_value(sp->u.string);
    free_string_svalue(sp);
    if (ret) assign_svalue_no_free(sp, ret);
    else *sp = const0;
}
#endif

#ifdef F_QUERY_ED_MODE
void
f_query_ed_mode PROT((void))
{
    /* n = prompt for line 'n'
       0 = normal ed prompt
       -1 = not in ed
       -2 = more prompt */
    if (current_object->flags & O_IN_EDIT) {
	push_number(object_ed_mode(current_object));
    } else
	push_number(-1);
}
#endif

#ifdef F_QUERY_HOST_NAME
void
f_query_host_name PROT((void))
{
    char *tmp;

    if ((tmp = query_host_name()))
        push_constant_string(tmp);
    else
        push_number(0);
}
#endif

#ifdef F_QUERY_IDLE
void
f_query_idle PROT((void))
{
    int i;

    i = query_idle(sp->u.ob);
    free_object(sp->u.ob, "f_query_idle");
    put_number(i);
}
#endif

#ifdef F_QUERY_IP_NAME
void
f_query_ip_name PROT((void))
{
    char *tmp;

    tmp = query_ip_name(st_num_arg ? sp->u.ob : 0);
    if (st_num_arg) free_object((sp--)->u.ob, "f_query_ip_name");
    if (!tmp) *++sp = const0;
    else push_string(tmp, STRING_MALLOC);
}
#endif

#ifdef F_QUERY_IP_NUMBER
void
f_query_ip_number PROT((void))
{
    char *tmp;

    tmp = query_ip_number(st_num_arg ? sp->u.ob : 0);
    if (st_num_arg) free_object((sp--)->u.ob, "f_query_ip_number");
    if (!tmp) *++sp = const0;
    else push_string(tmp, STRING_MALLOC);
}
#endif

#ifdef F_QUERY_LOAD_AVERAGE
void
f_query_load_average PROT((void))
{
    push_string(query_load_av(), STRING_MALLOC);
}
#endif

#ifdef F_QUERY_PRIVS
void
f_query_privs PROT((void))
{
    ob = sp->u.ob;
    if (ob->privs != NULL) {
        free_object(ob, "f_query_privs");
        sp->type = T_STRING;
        sp->u.string = make_shared_string(ob->privs);
        sp->subtype = STRING_SHARED;
    } else {
        free_object(ob, "f_query_privs");
        *sp = const0;
    }
}
#endif

#ifdef F_QUERY_SNOOPING
void
f_query_snooping PROT((void))
{
    ob = query_snooping(sp->u.ob);
    free_object(sp->u.ob, "f_query_snooping");
    if (ob) { put_unrefed_undested_object(ob, "query_snooping"); }
    else *sp = const0;
}
#endif

#ifdef F_QUERY_SNOOP
void
f_query_snoop PROT((void))
{
    ob = query_snoop(sp->u.ob);
    free_object(sp->u.ob, "f_query_snoop");
    if (ob) { put_unrefed_undested_object(ob, "query_snoop"); }
    else *sp = const0;
}
#endif

#ifdef F_QUERY_VERB
void
f_query_verb PROT((void))
{
    if (!last_verb) {
        push_number(0);
        return;
    }
    push_string(last_verb, STRING_SHARED);
}
#endif

#ifdef F_RANDOM
void
f_random PROT((void))
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
f_read_bytes PROT((void))
{
    char *str;
    int start = 0, len = 0, rlen = 0, num_arg = st_num_arg;
    svalue_t *arg;

    arg = sp - num_arg + 1;
    if (num_arg > 1)
        start = arg[1].u.number;
    if (num_arg == 3) {
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
f_read_buffer PROT((void))
{
    char *str;
    int start = 0, len = 0, rlen = 0, num_arg = st_num_arg;
    int from_file = 0;          /* new line */
    svalue_t *arg = sp - num_arg + 1;

    if (num_arg > 1) {
       start = arg[1].u.number;
       if (num_arg == 3) {
	   len = arg[2].u.number;
       }
    }
    if (arg[0].type == T_STRING) {
        from_file = 1;          /* new line */
        str = read_bytes(arg[0].u.string, start, len, &rlen);
    } else {                    /* T_BUFFER */
        str = read_buffer(arg[0].u.buf, start, len, &rlen);
    }
    pop_n_elems(num_arg);
    if (str == 0) {
        push_number(0);
    } else if (from_file) {     /* changed */
        buffer_t *buf;

        buf = allocate_buffer(rlen);
        memcpy(buf->item, str, rlen);
        (++sp)->u.buf = buf;
        sp->type = T_BUFFER;
        FREE_MSTR(str);
    } else {                    /* T_BUFFER */
        push_malloced_string(str);
    }
}
#endif

#ifdef F_READ_FILE
void
f_read_file PROT((void))
{
    char *str;
    int start,len;

    if (st_num_arg == 3) {
        len = (sp--)->u.number;
    } else len = 0;
    if (st_num_arg > 1)
        start = (sp--)->u.number;
    else start = 0;

    str = read_file(sp->u.string, start, len);
    free_string_svalue(sp);
    if (!str) *sp = const0;
    else { sp->subtype = STRING_MALLOC; sp->u.string = str; }
}
#endif

#ifdef F_RECEIVE
void
f_receive PROT((void))
{
    if (current_object->interactive) {
	check_legal_string(sp->u.string);
	add_message(current_object, sp->u.string);
    }
    free_string_svalue(sp--);
}
#endif

#ifdef F_REG_ASSOC
void
f_reg_assoc PROT((void)) {
    svalue_t *arg;
    array_t *vec;

    arg = sp - st_num_arg + 1;
    
    if (!(arg[2].type == T_ARRAY))
	error("Bad argument 3 to reg_assoc()\n");
    
    vec = reg_assoc(arg[0].u.string, arg[1].u.arr, arg[2].u.arr, st_num_arg > 3 ? &arg[3] : &const0);

    if (st_num_arg == 4)
	pop_3_elems();
    else
	pop_2_elems();
    free_string_svalue(sp);
    sp->type = T_ARRAY;
    sp->u.arr = vec;
}
#endif

#ifdef F_REGEXP
void
f_regexp PROT((void))
{
    array_t *v;
    int flag;

    if (st_num_arg > 2) {
        if (!(sp->type == T_NUMBER)) error("Bad argument 3 to regexp()\n");
	if (sp[-2].type == T_STRING) error("3rd argument illegal for regexp(string, string)\n"); 
	flag = (sp--)->u.number;
    } else flag = 0;
    if (sp[-1].type == T_STRING) {
	flag = match_single_regexp((sp - 1)->u.string, sp->u.string);
	free_string_svalue(sp--);
	free_string_svalue(sp);
	put_number(flag);
    } else {
	v = match_regexp((sp - 1)->u.arr, sp->u.string, flag);

	free_string_svalue(sp--);
	free_array(sp->u.arr);
	if (!v) *sp = const0;
	else sp->u.arr = v;
    }
}
#endif

#ifdef F_REMOVE_ACTION
void
f_remove_action PROT((void))
{
    int success;

    success = remove_action((sp - 1)->u.string, sp->u.string);
    free_string_svalue(sp--);
    free_string_svalue(sp);
    put_number(success);
}
#endif

#ifdef F_REMOVE_CALL_OUT
void
f_remove_call_out PROT((void))
{
    int i;

    if (st_num_arg) {
	i = remove_call_out(current_object, sp->u.string);
	free_string_svalue(sp);
    } else {
	remove_all_call_out(current_object);
	i = -1;
	sp++;
    }
    put_number(i);
}
#endif

#ifdef F_RENAME
void
f_rename PROT((void))
{
    int i;

    i = do_rename((sp - 1)->u.string, sp->u.string, F_RENAME);
    free_string_svalue(sp--);
    free_string_svalue(sp);
    put_number(i);
}
#endif				/* F_RENAME */

/* This is an enhancement to the f_replace_string() in efuns_main.c of
   MudOS v21.  When the search pattern has more than one character,
   this version of f_replace_string() uses a skip table to more efficiently
   search the file for the search pattern (the basic idea is to avoid
   strings comparisons where possible).  This version is anywhere from
   15% to 40% faster than the old version depending on the size of the
   string to be searched and the length of the search string (and depending
   on the relative frequency with which the letters in the search string
   appear in the string to be searched).

   Note: this version should behave identically to the old version (except
   for runtime).  When the search pattern is only one character long, the
   old algorithm is used.  The new algorithm is actually about 10% slower
   than the old one when the search string is only one character long.

   This enhancement to f_replace_string() was written by John Garnett
   (aka Truilkan) on 1995/04/29.  I believe the original replace_string()
   was written by Dave Richards (Cygnus).

   I didn't come up with the idea of this algorithm (learned it in
   a university programming course way back when).  For those interested
   in the workings of the algorithm, you can probably find it in a book on
   string processing algorithms.  Its also fairly easy to figure out the
   algorithm by tracing through it for a small example.
*/
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
f_replace_string PROT((void))
{
    int plen, rlen, dlen, slen, first, last, cur, j;
    
    char *pattern;
    char *replace;
    register char *src, *dst1, *dst2;
    svalue_t *arg;
    int skip_table[256];
    char *slimit;
    char *flimit;
    char *climit;
    int probe;
    int skip;
    
    if (st_num_arg > 5) {
        error("Too many args to replace_string.\n");
        pop_n_elems(st_num_arg);
        return;
    }
    arg = sp - st_num_arg + 1;
    src = arg->u.string;
    first = 0;
    last = 0;
    
    if (st_num_arg >= 4) {
        CHECK_TYPES((arg+3), T_NUMBER, 4, F_REPLACE_STRING);
        first = (arg+3)->u.number;
	
        if (st_num_arg == 4) {
            last = first;
            first = 0;
        } else if (st_num_arg == 5) {
            CHECK_TYPES(sp, T_NUMBER, 5, F_REPLACE_STRING);
            /* first set above. */
            last = sp->u.number;
        }
    }
        
    if (!last)
        last = max_string_length;
    
    if (first > last) {         /* just return it */
        pop_n_elems(st_num_arg - 1);
        return;
    }
    pattern = (arg+1)->u.string;
    plen = SVALUE_STRLEN(arg+1);
    if (!plen) {
        pop_n_elems(st_num_arg - 1);    /* just return it */
	
        return;
    }
    replace = (arg+2)->u.string;
    rlen = SVALUE_STRLEN(arg+2);
    dlen = 0;
    cur = 0;

    if (rlen <= plen) {
	/* we're going to do in string replacement */
	unlink_string_svalue(arg);
	src = arg->u.string;
    }
    
    if (plen > 1) {
        /* build skip table */
        for (j = 0; j < 256; j++) {
            skip_table[j] = plen;
        }
        for (j = 0; j < plen; j++) {
            skip_table[(unsigned char)pattern[j]] = plen - j - 1;
        }
        slen = SVALUE_STRLEN(arg);
        slimit = src + slen;
        flimit = slimit - plen + 1;
        probe = plen - 1;
    }
    
    if (rlen <= plen) {
        /* in string replacement */
        dst2 = dst1 = arg->u.string;
	
        if (plen > 1) { /* pattern length > 1, jump table most efficient */
            while (src < flimit) {
                if ((skip = skip_table[(unsigned char)src[probe]])) {
                    for (climit = dst2 + skip; dst2 < climit; *dst2++ = *src++)
                        ;
                } else if (memcmp(src, pattern, plen) == 0) {
                    cur++;
                    if ((cur >= first) &&  (cur <= last)) {
                        if (rlen) {
                            memcpy(dst2, replace, rlen);
                            dst2 += rlen;
                        }
                        src += plen;
                        if (cur == last) break;
                    } else {
                        src += plen;
                    }
                } else {
                    *dst2++ = *src++;
                }
            }
            memcpy(dst2, src, slimit - src);
            dst2 += (slimit - src);
	    *dst2 = 0;
	    arg->u.string = extend_string(dst1, dst2 - dst1);
        } else { /* pattern length <= 1, brute force most efficient */
	    /* Beek - if it was zero, we already returned, so plen == 1 */
	    /* assume source string is a string < maximum string length */
	    if (rlen) {
		while (*src) {
		    if (*src == *pattern) {
			cur++;
		    
			if (cur >= first && cur <= last) {
			    *src = *replace;
			}
		    }
		    src++;
		}
	    } else { /* rlen is zero */
		while (*src) {
		    if (*src++ == *pattern) {
			dst2 = src - 1;
			while (*src) {
			    if (*src == *pattern) src++;
			    else *dst2++ = *src++;
			}
			*dst2 = 0;
			arg->u.string = extend_string(dst1, dst2 - dst1);
			break;
		    }
		}
	    }
	}
        pop_n_elems(st_num_arg - 1);
    } else {
        dst2 = dst1 = new_string(max_string_length, "f_replace_string: 2");
	
        if (plen > 1) {
            while (src < flimit) {
                if ((skip = skip_table[(unsigned char)src[probe]])) {
                    for (climit = dst2 + skip; dst2 < climit; *dst2++ = *src++)
                        ;
		    
                } else if (memcmp(src, pattern, plen) == 0) {
                    cur++;
                    if ((cur >= first) && (cur <= last)) {
                        if (rlen) {
                            if (max_string_length - dlen <= rlen) {
                                pop_n_elems(st_num_arg);
                                push_svalue(&const0u);
                                FREE_MSTR(dst1);
                                return;
                            }
                            memcpy(dst2, replace, rlen);
                            dst2 += rlen;
                            dlen += rlen;
                        }
                        src += plen;
                        if (cur == last) break;
                    } else {
			dlen += plen;
			if (max_string_length - dlen <= 0) {
			    pop_n_elems(st_num_arg);
			    push_svalue(&const0u);
			    
			    FREE_MSTR(dst1);
			    return;
			}			    
			memcpy(dst2, src, plen);
			dst2 += plen;
                        src += plen;
                    }
                } else {
                    if (max_string_length - dlen <= 1) {
                        pop_n_elems(st_num_arg);
                        push_svalue(&const0u);
			
                        FREE_MSTR(dst1);
                        return;
                    }
                    *dst2++ = *src++;
                    dlen++;
                }
            }
            if (max_string_length - dlen <= (slimit - src)) {
                pop_n_elems(st_num_arg);
                push_svalue(&const0u);
                FREE_MSTR(dst1);
                return;
            }
            memcpy(dst2, src, slimit - src);
            dst2 += (slimit - src);
        } else { /* plen <= 1 */
	    /* Beek: plen == 1 */
            while (*src != '\0') {
                if (*src == *pattern) {
                    cur++;
                    if (cur >= first && cur <= last) {
                        if (rlen != 0) {
                            if (max_string_length - dlen <= rlen) {
                                pop_n_elems(st_num_arg);
                                push_svalue(&const0u);
                                FREE_MSTR(dst1);
                                return;
                            }
                            strncpy(dst2, replace, rlen);
                            dst2 += rlen;
                            dlen += rlen;
                        }
                        src++;
                        continue;
                    }
                }
                if (max_string_length - dlen <= 1) {
                    pop_n_elems(st_num_arg);
                    push_svalue(&const0u);
                    FREE_MSTR(dst1);
                    return;
                }
                *dst2++ = *src++;
                dlen++;
            }
        }
        *dst2 = '\0';

        pop_n_elems(st_num_arg);
        /*
         * shrink block or make a copy of exact size
         */
        push_malloced_string(extend_string(dst1, dst2 - dst1));
    }
}
#endif

#ifdef F_RESOLVE
void
f_resolve PROT((void))
{
    int i, query_addr_number PROT((char *, char *));

    i = query_addr_number((sp - 1)->u.string, sp->u.string);
    free_string_svalue(sp--);
    free_string_svalue(sp);
    put_number(i);
}				/* f_resolve() */
#endif

#ifdef F_RESTORE_OBJECT
void
f_restore_object PROT((void))
{
    int flag;

    flag = (st_num_arg > 1) ? (sp--)->u.number : 0;
    flag = restore_object(current_object, sp->u.string, flag);
    free_string_svalue(sp);
    put_number(flag);
}
#endif

#ifdef F_RESTORE_VARIABLE
void
f_restore_variable PROT((void)) {
    svalue_t v;

    unlink_string_svalue(sp);
    v.type = T_NUMBER;

    restore_variable(&v, sp->u.string);
    FREE_MSTR(sp->u.string);
    *sp = v;
}
#endif

#ifdef F_RM
void
f_rm PROT((void))
{
    int i;

    i = remove_file(sp->u.string);
    free_string_svalue(sp);
    put_number(i);
}
#endif

#ifdef F_RMDIR
void
f_rmdir PROT((void))
{
    char *path;

    path = check_valid_path(sp->u.string, current_object, "rmdir", 1);
    if (!path || rmdir(path) == -1){
        free_string_svalue(sp);
        *sp = const0;
    }
    else {
        free_string_svalue(sp);
        *sp = const1;
    }
}
#endif

#ifdef F_SAVE_OBJECT
void
f_save_object PROT((void))
{
    int flag;

    flag = (st_num_arg == 2) && (sp--)->u.number;
    flag = save_object(current_object, sp->u.string, flag);
    free_string_svalue(sp);
    put_number(flag);
}
#endif

#ifdef F_SAVE_VARIABLE
void
f_save_variable PROT((void)) {
    char *p;

    p = save_variable(sp);
    pop_stack();
    push_malloced_string(p);
}
#endif

#ifdef F_SAY
void
f_say PROT((void))
{
    array_t *avoid;
    static array_t vtmp =
    {1,
#ifdef DEBUG
     1,
#endif
     1,
#ifdef PACKAGE_MUDLIB_STATS
     {(mudlib_stats_t *) NULL, (mudlib_stats_t *) NULL}
#endif
    };

    if (st_num_arg == 1) {
	avoid = null_array();
	say(sp, avoid);
	pop_stack();
    } else {
	if (sp->type == T_OBJECT) {
	    vtmp.item[0].type = T_OBJECT;
	    vtmp.item[0].u.ob = sp->u.ob;
	    avoid = &vtmp;
	} else {		/* must be a array... */
	    avoid = sp->u.arr;
	}
	say(sp - 1, avoid);
	pop_2_elems();
    }
}
#endif

#ifdef F_SET_EVAL_LIMIT
/* warning: do not enable this without using valid_override() in the master
   object and a set_eval_limit() simul_efun to restrict access.
*/
void
f_set_eval_limit PROT((void))
{
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
f_set_bit PROT((void))
{
    char *str;
    int len, old_len, ind, bit;

    if (sp->u.number > MAX_BITS)
        error("set_bit() bit requested: %d > maximum bits: %d\n", sp->u.number, MAX_BITS);
    bit = (sp--)->u.number;
    if (bit < 0)
	error("Bad argument 2 (negative) to set_bit().\n");
    ind = bit/6;
    bit %= 6;
    old_len = len = SVALUE_STRLEN(sp);
    if (ind >= len)
        len = ind + 1;
    if (ind < old_len) {
	unlink_string_svalue(sp);
	str = sp->u.string;
    } else {
	str = new_string(len, "f_set_bit: str");
	str[len] = '\0';
	if (old_len)
	    memcpy(str, sp->u.string, old_len);
	if (len > old_len)
	    memset(str + old_len, ' ', len - old_len);
	free_string_svalue(sp);
	sp->subtype = STRING_MALLOC;
	sp->u.string = str;
    }

    if (str[ind] > 0x3f + ' ' || str[ind] < ' ')
        error("Illegal bit pattern in set_bit character %d\n", ind);
    str[ind] = ((str[ind] - ' ') | (1 << bit)) + ' ';
}
#endif

#ifdef F_SET_HEART_BEAT
void
f_set_heart_beat PROT((void))
{
    set_heart_beat(current_object, (sp--)->u.number);
}
#endif

#ifdef F_QUERY_HEART_BEAT
void
f_query_heart_beat PROT((void))
{
    free_object(ob = sp->u.ob, "f_query_heart_beat");
    put_number(query_heart_beat(ob));
}
#endif

#ifdef F_SET_HIDE
void
f_set_hide PROT((void))
{
    if (!valid_hide(current_object)) {
	sp--;
	return;
    }
    if ((sp--)->u.number) {
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
f_set_light PROT((void))
{
    object_t *o1;

#ifdef NO_LIGHT
    sp->u.number = 1;
#else
    add_light(current_object, sp->u.number);
    o1 = current_object;
    while (o1->super)
	o1 = o1->super;
    sp->u.number = o1->total_light;
#endif
}
#endif

#ifdef F_SET_LIVING_NAME
void
f_set_living_name PROT((void))
{
    set_living_name(current_object, sp->u.string);
    free_string_svalue(sp--);
}
#endif

#ifdef F_SET_PRIVS
void
f_set_privs PROT((void))
{
    object_t *ob;

    ob = (sp - 1)->u.ob;
    if (ob->privs != NULL)
        free_string(ob->privs);
    if (!(sp->type == T_STRING)) {
        ob->privs = NULL;
	sp--; /* It's a number */
    } else {
        ob->privs = make_shared_string(sp->u.string);
	free_string_svalue(sp--);
    }	    
    free_object(ob, "f_set_privs");
    sp--;
}
#endif

#ifdef F_SHADOW
void
f_shadow PROT((void))
{
    object_t *ob;

    ob = (sp - 1)->u.ob;
    if (!((sp--)->u.number)) {
        ob = ob->shadowed;
        free_object(sp->u.ob, "f_shadow:1");
        if (ob) {
	  add_ref(ob, "shadow(ob, 0)");
	  sp->u.ob = ob;
	}
        else *sp = const0;
        return;
    }
    if (ob == current_object) {
        error("shadow: Can't shadow self\n");
    }
    if (validate_shadowing(ob)) {
        if (current_object->flags & O_DESTRUCTED) {
            free_object(ob, "f_shadow:2");
            *sp = const0;
            return;
	}
        /*
         * The shadow is entered first in the chain.
         */
        while (ob->shadowed)
            ob = ob->shadowed;
        current_object->shadowing = ob;
        ob->shadowed = current_object;
        free_object(sp->u.ob, "f_shadow:3");
	add_ref(ob, "shadow(ob, 1)");
        sp->u.ob = ob;
        return;
    }
    free_object(sp->u.ob, "f_shadow:4");
    *sp = const0;
}
#endif

#ifdef F_SHOUT
void
f_shout PROT((void))
{
    shout_string(sp->u.string);
    free_string_svalue(sp--);
}
#endif

#ifdef F_SHUTDOWN
void
f_shutdown PROT((void))
{
    shutdownMudOS(st_num_arg ? sp->u.number : (*++sp = const0, 0));
}
#endif

#ifdef F_SIZEOF
void
f_sizeof PROT((void))
{
    int i;

    switch (sp->type) {
    case T_CLASS:
    case T_ARRAY:
	i = sp->u.arr->size;
	free_array(sp->u.arr);
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
f_snoop PROT((void))
{
    /*
     * This one takes a variable number of arguments. It returns 0 or an
     * object.
     */
    if (st_num_arg == 1) {
        if (!new_set_snoop(sp->u.ob, 0) || (sp->u.ob->flags & O_DESTRUCTED)) {
	    free_object(sp->u.ob, "f_snoop:1");
	    *sp = const0;
	}
    } else {
        if (!new_set_snoop((sp - 1)->u.ob, sp->u.ob) || 
	    (sp->u.ob->flags & O_DESTRUCTED)) {
	    free_object((sp--)->u.ob, "f_snoop:2");
	    free_object(sp->u.ob, "f_snoop:3");
	    *sp = const0;
	} else {
	    free_object((--sp)->u.ob, "f_snoop:4");
	    sp->u.ob = (sp+1)->u.ob;
	}
    }
}
#endif

#ifdef F_SPRINTF
void
f_sprintf PROT((void))
{
    char *s;
    int num_arg = st_num_arg;

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
        push_malloced_string(string_copy(s, "f_sprintf"));
}
#endif

#ifdef F_STAT
void
f_stat PROT((void))
{
    struct stat buf;
    char *path;
    array_t *v;

    path = check_valid_path((--sp)->u.string, current_object, "stat", 0);
    if (!path) {
        free_string_svalue(sp);
        *sp = const0;
        return;
    }
    if (stat(path, &buf) != -1) {
        if (buf.st_mode & S_IFREG) {    /* if a regular file */
            v = allocate_empty_array(3);
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
            free_string_svalue(sp);
            put_array(v);
            return;
	}
    }
    v = get_dir(sp->u.string, (sp+1)->u.number);
    free_string_svalue(sp);
    if (v) { put_array(v); }
    else *sp = const0;
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
f_strsrch PROT((void))
{
    register char *big, *little, *pos;
    static char buf[2];         /* should be initialized to 0 */
    int i, blen, llen;

    sp--;
    big = (sp - 1)->u.string;
    blen = SVALUE_STRLEN(sp - 1);
    if (sp->type == T_NUMBER) {
        little = buf;
        if ((buf[0] = (char) sp->u.number))
	    llen = 1;
	else
	    llen = 0;
    } else {
        little = sp->u.string;
	llen = SVALUE_STRLEN(sp);
    }

    if (!llen || blen < llen) {
        pos = NULL;

        /* start at left */
    } else if (!((sp+1)->u.number)) {
        if (!little[1])         /* 1 char srch pattern */
            pos = strchr(big, (int) little[0]);
        else
            pos = (char *)_strstr(big, little);
        /* start at right */
    } else {                    /* XXX: maybe test for -1 */
        if (!little[1])         /* 1 char srch pattern */
            pos = strrchr(big, (int) little[0]);
        else {
            char c = *little;

            pos = big + blen;   /* find end */
            pos -= llen;        /* find rightmost pos it _can_ be */
            do {
                do {
                    if (*pos == c) break;
		} while (--pos >= big);
                if (*pos != c) {
                    pos = NULL;
                    break;
		}
                for (i = 1; little[i] && (pos[i] == little[i]); i++);   /* scan all chars */
                if (!little[i])
                    break;
	    } while (--pos >= big);
	}
    }

    if (!pos)
        i = -1;
    else
        i = (int) (pos - big);
    if (sp->type == T_STRING) free_string_svalue(sp);
    free_string_svalue(--sp);
    put_number(i);
}				/* strsrch */
#endif

#ifdef F_STRCMP
void
f_strcmp PROT((void))
{
    int i;

    i = strcmp((sp - 1)->u.string, sp->u.string);
    free_string_svalue(sp--);
    free_string_svalue(sp);
    put_number(i);
}
#endif

#ifdef F_STRINGP
void
f_stringp PROT((void))
{
    if (sp->type == T_STRING){
        free_string_svalue(sp);
        *sp = const1;
    }
    else {
        free_svalue(sp, "f_stringp");
        *sp = const0;
    }
}
#endif

#ifdef F_BUFFERP
void
f_bufferp PROT((void))
{
    if (sp->type == T_BUFFER) {
        free_buffer(sp->u.buf);
        *sp = const1;
    } else {
        free_svalue(sp, "f_bufferp");
        *sp = const0;
    }
}
#endif

#ifdef F_SWAP
void
f_swap PROT((void))
{
    object_t *ob = sp->u.ob;
    control_stack_t *p;

    /* a few sanity checks */
    if (!(ob->flags & O_SWAPPED) && (ob != current_object)) {
	for (p = csp; p >= control_stack; p--)
	    if (ob == csp->ob) {
		pop_stack();
		return;
	    }
	(void) swap(sp->u.ob);
    }

    pop_stack();
}
#endif

#ifdef F_TAIL
void
f_tail PROT((void))
{
    if (tail(sp->u.string)){
        free_string_svalue(sp);
        *sp = const1;
    }
    else {
	free_string_svalue(sp);
	*sp = const0;
    }
}
#endif

#ifdef F_TELL_OBJECT
void
f_tell_object PROT((void))
{
    tell_object((sp - 1)->u.ob, sp->u.string);
    free_string_svalue(sp--);
    pop_stack();
}
#endif

#ifdef F_TELL_ROOM
void
f_tell_room PROT((void))
{
    array_t *avoid;
    int num_arg = st_num_arg;
    svalue_t *arg = sp - num_arg + 1;

    if (arg->type == T_OBJECT) {
        ob = arg[0].u.ob;
    } else {                    /* must be a string... */
        ob = find_object(arg[0].u.string);
        if (!ob || !object_visible(ob))
            error("Bad argument 1 to tell_room()\n");
    }

    if (num_arg == 2) {
        avoid = null_array();
    } else {
        avoid = arg[2].u.arr;
    }

    tell_room(ob, &arg[1], avoid);
    free_array(avoid);
    free_svalue(arg + 1, "f_tell_room");
    free_svalue(arg, "f_tell_room");
    sp = arg - 1;
}
#endif

#ifdef F_TEST_BIT
void
f_test_bit PROT((void))
{
    int ind = (sp--)->u.number;

    if (ind / 6 >= SVALUE_STRLEN(sp)) {
        free_string_svalue(sp);
        *sp = const0;
        return;
    }
    if (ind < 0) error("Bad argument 2 (negative) to test_bit().\n");
    if ((sp->u.string[ind / 6] - ' ') & (1 << (ind % 6))) {
        free_string_svalue(sp);
        *sp = const1;
    } else {
        free_string_svalue(sp);
        *sp = const0;
    }
}
#endif

#ifdef F_NEXT_BIT
void
f_next_bit PROT((void))
{
    int start = (sp--)->u.number;
    int len = SVALUE_STRLEN(sp);
    int which, bit, value;
    
    if (!len || start / 6 >= len) {
        free_string_svalue(sp);
	put_number(-1);
        return;
    }
    /* Find the next bit AFTER start */
    if (start > 0) {
	if (start % 6 == 5) {
	    which = (start / 6) + 1;
	    value = sp->u.string[which] - ' ';
	} else {
	    /* we have a partial byte to check */
	    which = start / 6;
	    bit = 0x3f - ((1 << ((start % 6) + 1)) - 1);
	    value = (sp->u.string[which] - ' ') & bit;
	}
    } else {
	which = 0;
	value = *sp->u.string - ' ';
    }

    while (1) {
	if (value)  {
	    if (value & 0x07) {
		if (value & 0x01)
		    bit = which * 6;
		else if (value & 0x02)
		    bit = which * 6 + 1;
		else if (value & 0x04)
		    bit = which * 6 + 2;
		break;
	    } else if (value & 0x38) {
		if (value & 0x08)
		    bit = which * 6 + 3;
		else if (value & 0x10)
		    bit = which * 6 + 4;
		else if (value & 0x20)
		    bit = which * 6 + 5;
		break;
	    }
	}
	which++;
	if (which == len) {
	    bit = -1;
	    break;
	}
	value = sp->u.string[which] - ' ';
    }

    free_string_svalue(sp);
    put_number(bit);
}
#endif

#ifdef F_THIS_OBJECT
void
f_this_object PROT((void))
{
    if (current_object->flags & O_DESTRUCTED)   /* Fixed from 3.1.1 */
        *++sp = const0;
    else
        push_object(current_object);
}
#endif

#ifdef F_THIS_PLAYER
void
f_this_player PROT((void))
{
    if (sp->u.number) {
	if (current_interactive)
	    put_unrefed_object(current_interactive, "this_player(1)");
	else sp->u.number = 0;
    } else {
	if (command_giver)
	    put_unrefed_object(command_giver, "this_player(0)");
	/* else zero is on stack already */
    }
}
#endif

#ifdef F_SET_THIS_PLAYER
void
f_set_this_player PROT((void))
{
    if (sp->type == T_NUMBER)
	command_giver = 0;
    else
	command_giver = sp->u.ob;
    pop_stack();
}
#endif

#ifdef F_THROW
void
f_throw PROT((void))
{
    free_svalue(&catch_value, "f_throw");
    catch_value = *sp--;
    throw_error();		/* do the longjump, with extra checks... */
}
#endif

#ifdef F_TIME
void
f_time PROT((void))
{
    push_number(current_time);
}
#endif

#ifdef F_TO_FLOAT
void
f_to_float PROT((void))
{
    double temp = 0;

    switch(sp->type){
        case T_NUMBER:
            sp->type = T_REAL;
            sp->u.real = (double) sp->u.number;
            break;
        case T_STRING:
            sscanf(sp->u.string, "%lf", &temp);
            free_string_svalue(sp);
            sp->type = T_REAL;
            sp->u.real = temp;
    }
}
#endif

#ifdef F_TO_INT
void
f_to_int PROT((void))
{
    switch(sp->type){
        case T_REAL:
            sp->type = T_NUMBER;
            sp->u.number = (int) sp->u.real;
            break;
        case T_STRING:
        {
            int temp;

            temp = atoi(sp->u.string);
            free_string_svalue(sp);
            sp->u.number = temp;
            sp->type = T_NUMBER;
            break;
	}
        case T_BUFFER:
            if (sp->u.buf->size < sizeof(int)) {
                free_buffer(sp->u.buf);
                *sp = const0;
	    } else {
                int hostint, netint;

                memcpy((char *) &netint, sp->u.buf->item, sizeof(int));
                hostint = ntohl(netint);
                free_buffer(sp->u.buf);
                put_number(hostint);
	    }
    }
}
#endif

#ifdef F_TYPEOF
void
f_typeof PROT((void))
{
    char *t = type_name(sp->type);

    free_svalue(sp, "f_typeof");
    put_constant_string(t);
}
#endif

#ifdef F_UNDEFINEDP
void
f_undefinedp PROT((void))
{
    if (sp->type == T_NUMBER){
        if (!sp->u.number && (sp->subtype == T_UNDEFINED)) {
	    *sp = const1;
        } else *sp = const0;
    } else {
        free_svalue(sp, "f_undefinedp");
        *sp = const0;
    }
}
#endif

#ifdef F_UPTIME
void
f_uptime PROT((void))
{
    push_number(current_time - boot_time);
}
#endif

#ifdef F_USERP
void
f_userp PROT((void))
{
    int i;

    i = (int) sp->u.ob->flags & O_ONCE_INTERACTIVE;
    free_object(sp->u.ob, "f_userp");
    put_number(i != 0);
}
#endif

#ifdef F_USERS
void
f_users PROT((void))
{
    push_refed_array(users());
}
#endif

#ifdef F_WIZARDP
void
f_wizardp PROT((void))
{
    int i;

    i = (int) sp->u.ob->flags & O_IS_WIZARD;
    free_object(sp->u.ob, "f_wizardp");
    put_number(i != 0);
}
#endif

#ifdef F_VIRTUALP
void
f_virtualp PROT((void))
{
    int i;

    i = (int) sp->u.ob->flags & O_VIRTUAL;
    free_object(sp->u.ob, "f_virtualp");
    put_number(i != 0);
}
#endif

#ifdef F_WRITE
void
f_write PROT((void))
{
    do_write(sp);
    pop_stack();
}
#endif

#ifdef F_WRITE_BYTES
void
f_write_bytes PROT((void))
{
    int i;

    switch(sp->type){
        case T_NUMBER:
        {
            int netint;
            char *netbuf;

            if (!sp->u.number) bad_arg(3, F_WRITE_BYTES);
            netint = htonl(sp->u.number);       /* convert to network
                                                 * byte-order */
            netbuf = (char *) &netint;
            i = write_bytes((sp - 2)->u.string, (sp - 1)->u.number, netbuf,
                            sizeof(int));
            break;
	}

        case T_BUFFER:
        {
            i = write_bytes((sp - 2)->u.string, (sp - 1)->u.number,
                            (char *) sp->u.buf->item, sp->u.buf->size);
            break;
	}

        case T_STRING:
        {
            i = write_bytes((sp - 2)->u.string, (sp - 1)->u.number,
                            sp->u.string, SVALUE_STRLEN(sp));
            break;
	}

        default:
        {
            bad_argument(sp, T_BUFFER | T_STRING | T_NUMBER, 3, F_WRITE_BYTES);
	}
    }
    free_svalue(sp--, "f_write_bytes");
    free_string_svalue(--sp);
    put_number(i);
}
#endif

#ifdef F_WRITE_BUFFER
void
f_write_buffer PROT((void))
{
    int i;

    if ((sp-2)->type == T_STRING){
        f_write_bytes();
        return;
    }

    switch(sp->type){
        case T_NUMBER:
        {
            int netint;
            char *netbuf;

            if (!sp->u.number) bad_arg(3, F_WRITE_BUFFER);
            netint = htonl(sp->u.number);       /* convert to network
                                                 * byte-order */
            netbuf = (char *) &netint;
            i = write_buffer((sp - 2)->u.buf, (sp - 1)->u.number, netbuf,
                            sizeof(int));
            break;
	}

        case T_BUFFER:
        {
            i = write_buffer((sp - 2)->u.buf, (sp - 1)->u.number,
                            (char *) sp->u.buf->item, sp->u.buf->size);
            break;
	}

        case T_STRING:
        {
            i = write_buffer((sp - 2)->u.buf, (sp - 1)->u.number,
                            sp->u.string, SVALUE_STRLEN(sp));
            break;
	}

        default:
        {
            bad_argument(sp, T_BUFFER | T_STRING | T_NUMBER, 3, F_WRITE_BUFFER);
	}
    }
    free_svalue(sp--, "f_write_buffer");
    free_string_svalue(--sp);
    put_number(i);
}
#endif

#ifdef F_WRITE_FILE
void
f_write_file PROT((void))
{
    int flags = 0;

    if (st_num_arg == 3) {
        flags = (sp--)->u.number;
    }
    flags = write_file((sp - 1)->u.string, sp->u.string, flags);
    free_string_svalue(sp--);
    free_string_svalue(sp);
    put_number(flags);
}
#endif

#ifdef F_DUMP_FILE_DESCRIPTORS
void
f_dump_file_descriptors PROT((void))
{
    outbuffer_t out;

    outbuf_zero(&out);
    dump_file_descriptors(&out);
    outbuf_push(&out);
}
#endif

#ifdef F_RECLAIM_OBJECTS
void f_reclaim_objects PROT((void))
{
    push_number(reclaim_objects());
}
#endif

#ifdef F_MEMORY_INFO
void
f_memory_info PROT((void))
{
    int mem;

    if (st_num_arg == 0) {
	int res, tot;

	if (reserved_area)
	    res = RESERVED_SIZE;
	else
	    res = 0;
	tot = total_prog_block_size +
	    total_array_size +
	    total_mapping_size +
	    tot_alloc_object_size +
	    tot_alloc_sentence * sizeof(sentence_t) +
	    total_users * sizeof(interactive_t) +
	    show_otable_status(0, -1) +
	    heart_beat_status(0, -1) +
	    add_string_status(0, -1) +
	    print_call_out_usage(0, -1) + res;
	push_number(tot);
	return;
    }
    if (sp->type != T_OBJECT)
	bad_argument(sp, T_OBJECT, 1, F_MEMORY_INFO);
    ob = sp->u.ob;
    if (ob->prog && (ob->prog->ref == 1 || !(ob->flags & O_CLONE)))
	mem = ob->prog->total_size;
    else
	mem = 0;
    mem += (data_size(ob) + sizeof(object_t));
    free_object(ob, "f_memory_info");
    put_number(mem);
}
#endif

#ifdef F_RELOAD_OBJECT
void
f_reload_object PROT((void))
{
    reload_object(sp->u.ob);
    free_object((sp--)->u.ob, "f_reload_object");
}
#endif

#ifdef F_QUERY_SHADOWING
void
f_query_shadowing PROT((void))
{
    if ((sp->type == T_OBJECT) && (ob = sp->u.ob)->shadowing) {
        add_ref(ob->shadowing, "query_shadowing(ob)");
        sp->u.ob = ob->shadowing;
        free_object(ob, "f_query_shadowing");
    } else {
        free_svalue(sp, "f_query_shadowing");
        *sp = const0;
    }
}
#endif

#ifdef F_SET_RESET
void
f_set_reset PROT((void))
{
    if (st_num_arg == 2) {
        (sp - 1)->u.ob->next_reset = current_time + sp->u.number;
        free_object((--sp)->u.ob, "f_set_reset:1");
        sp--;
    } else {
        sp->u.ob->next_reset = current_time + TIME_TO_RESET / 2 +
            random_number(TIME_TO_RESET / 2);
        free_object((sp--)->u.ob, "f_set_reset:2");
    }
}
#endif

#ifdef F_FLOATP
void
f_floatp PROT((void))
{
    if (sp->type == T_REAL){
        sp->type = T_NUMBER;
        sp->u.number = 1;
    }
    else {
        free_svalue(sp, "f_floatp");
        *sp = const0;
    }
}
#endif

#ifdef F_FIRST_INVENTORY
void
f_first_inventory PROT((void))
{
    ob = first_inventory(sp);
    free_svalue(sp, "f_first_inventory");
    if (ob) { put_unrefed_undested_object(ob, "first_inventory"); }
    else *sp = const0;
}
#endif

#ifdef F_NEXT_INVENTORY
void
f_next_inventory PROT((void))
{
    ob = sp->u.ob->next_inv;
    free_object(sp->u.ob, "f_next_inventory");
    while (ob) {
        if (ob->flags & O_HIDDEN) {
	    object_t *old_ob = ob;
            if (object_visible(ob)) {
	        add_ref(old_ob, "next_inventory(ob) : 1");
                sp->u.ob = old_ob;
                return;
	    }
	} else {
	    add_ref(ob, "next_inventory(ob) : 2");
            sp->u.ob = ob;
            return;
	}
        ob = ob->next_inv;
    }
    *sp = const0;
}
#endif
