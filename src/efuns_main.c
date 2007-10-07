/*
        efuns_main.c: this file contains the efunctions called from
        inside eval_instruction() in interpret.c.  Note: if you are adding
    local efunctions that are specific to your driver, you would be better
    off adding them to a separate source file.  Doing so will make it much
    easier for you to upgrade (won't have to patch this file).  Be sure
    to #include "lpc_incl.h" in that separate source file.
*/

#include "std.h"
#include "efuns_main.h"
#include "file_incl.h"
#include "file.h"
#include "comm.h"
#include "parse.h"
#include "sprintf.h"
#include "backend.h"
#include "port.h"
#include "otable.h"
#include "crc32.h"
#include "reclaim.h"
#include "dumpstat.h"
#include "call_out.h"
#include "ed.h"
#include "md.h"
#include "master.h"
#include "efun_protos.h"
#include "add_action.h"
#include "eval.h"

int call_origin = 0;

int data_size (object_t * ob);
void reload_object (object_t * obj);

#ifdef F_ALL_INVENTORY
void
f_all_inventory (void)
{
    array_t *vec = all_inventory(sp->u.ob, 0);
    free_object(&sp->u.ob, "f_all_inventory");
    sp->type = T_ARRAY;
    sp->u.arr = vec;
}
#endif

#ifdef F_ALLOCATE
void
f_allocate (void)
{
    if (st_num_arg == 2) {
        (sp-1)->u.arr = allocate_array2((sp-1)->u.number, sp);
        pop_stack();
    } else {
        sp->u.arr = allocate_array(sp->u.number);
    }
    sp->type = T_ARRAY;
}
#endif

#ifdef F_ALLOCATE_BUFFER
void
f_allocate_buffer (void)
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
f_allocate_mapping (void)
{
    array_t *arr;
    
    if (st_num_arg == 2) {
        if ((sp-1)->type != T_ARRAY)
            error("Bad argument 1 to allocate_mapping()\n");
        if (sp->type == T_ARRAY) {
            arr = (sp-1)->u.arr;
            if (sp->u.arr->size != arr->size)
                error("Arrays passed to allocate_mapping() must be the same size\n");
            (sp-1)->u.map = mkmapping(arr, sp->u.arr);
        } else {
            arr = (sp-1)->u.arr;
            (sp-1)->u.map = allocate_mapping2(arr, sp);
        }
        pop_stack();
        free_array(arr);
    } else if (sp->type == T_NUMBER) {
        sp->u.map = allocate_mapping(sp->u.number);
    } else {
      error("Bad argument 1 to allocate_mapping()\n");
    }
    sp->type = T_MAPPING;
}
#endif

#ifdef F_BIND
void
f_bind (void)
{
    object_t *ob = sp->u.ob;
    funptr_t *old_fp = (sp-1)->u.fp;
    funptr_t *new_fp;
    svalue_t *res;

    if (ob == old_fp->hdr.owner) {
        /* no change */
        free_object(&ob, "bind nop");
        sp--;
        return;
    }
    
    if (old_fp->hdr.type == (FP_LOCAL | FP_NOT_BINDABLE))
        error("Illegal to rebind a pointer to a local function.\n");
    if (old_fp->hdr.type & FP_NOT_BINDABLE)
        error("Illegal to rebind a functional that references globals or local functions.\n");
    
    /* the object doing the binding */
    push_object(current_object);

    /* the old owner */
    push_object(old_fp->hdr.owner);

    /* the new owner */
    push_object(ob);
    
    res = apply_master_ob(APPLY_VALID_BIND, 3);
    if (!MASTER_APPROVED(res))
        error("Master object denied permission to bind() function pointer.\n");
    
    new_fp = ALLOCATE(funptr_t, TAG_FUNP, "f_bind");
    *new_fp = *old_fp;
    new_fp->hdr.ref = 1;
    new_fp->hdr.owner = ob; /* one ref from being on stack */
    if (new_fp->hdr.args)
        new_fp->hdr.args->ref++;
    if ((old_fp->hdr.type & 0x0f) == FP_FUNCTIONAL) {
        new_fp->f.functional.prog->func_ref++;
        debug(d_flag, ("add func ref /%s: now %i\n",
                   new_fp->f.functional.prog->filename,
                   new_fp->f.functional.prog->func_ref));
    }

    free_funp(old_fp);
    sp--;
    sp->u.fp = new_fp;
}
#endif

#ifdef F_CACHE_STATS
static void print_cache_stats (outbuffer_t * ob)
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

void f_cache_stats (void)
{
    outbuffer_t ob;

    outbuf_zero(&ob);
    print_cache_stats(&ob);
    outbuf_push(&ob);
}
#endif

#ifdef F__CALL_OTHER
 /* enhanced call_other written 930314 by Luke Mewburn <zak@rmit.edu.au> */
void
f__call_other (void)
{
    svalue_t *arg;
    const char *funcname;
    int i;
    int num_arg = st_num_arg;
    object_t *ob;

    if (current_object->flags & O_DESTRUCTED) { /* No external calls allowed */
        pop_n_elems(num_arg);
        push_undefined();
        return;
    }
    arg = sp - num_arg + 1;
    if (arg[1].type == T_STRING)
        funcname = arg[1].u.string;
    else {                      /* must be T_ARRAY then */
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
    if (apply_low(funcname, ob, num_arg - 2) == 0) {    /* Function not found */
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
f_call_out (void)
{
    svalue_t *arg = sp - st_num_arg + 1;
    int num = st_num_arg - 2;
#ifdef CALLOUT_HANDLES
    int ret;

    if (!(current_object->flags & O_DESTRUCTED)) {
        ret = new_call_out(current_object, arg, arg[1].u.number, num, arg + 2);
        /* args have been transfered; don't free them;
           also don't need to free the int */
        sp -= num + 1;
    } else {
        ret = 0;
        pop_n_elems(num);
        sp--;
    }
    /* the function */
    free_svalue(sp, "call_out");
    put_number(ret);
#else
    if (!(current_object->flags & O_DESTRUCTED)) {
        new_call_out(current_object, arg, arg[1].u.number, num, arg + 2);
        sp -= num + 1;
    } else {
        pop_n_elems(num);
        sp--;
    }
    free_svalue(sp--, "call_out");
#endif
}
#endif

#ifdef F_CALL_OUT_INFO
void
f_call_out_info (void)
{
    push_refed_array(get_all_call_outs());
}
#endif

#if defined(F_CALL_STACK) || defined(F_ORIGIN)
static const char *origin_name (int orig) {
    /* FIXME: this should use ffs() if available (BSD) */
    int i = 0;
    static const char *origins[] = {
        "driver",
        "local",
        "call_other",
        "simul",
        "internal",
        "efun",
        "function pointer",
        "functional"
    };
    while (orig >>= 1) i++;
    return origins[i];
}
#endif

#ifdef F_CALL_STACK
void
f_call_stack (void)
{
    int i, n = csp - &control_stack[0] + 1;
    array_t *ret;

    if (sp->u.number < 0 || sp->u.number > 3)
        error("First argument of call_stack() must be 0, 1, 2, or 3.\n");

    ret = allocate_empty_array(n);
    
    switch (sp->u.number) {
    case 0:
        ret->item[0].type = T_STRING;
        ret->item[0].subtype = STRING_MALLOC;
        ret->item[0].u.string = add_slash(current_prog->filename);
        for (i = 1; i < n; i++) {
            ret->item[i].type = T_STRING;
            ret->item[i].subtype = STRING_MALLOC;
            ret->item[i].u.string = add_slash((csp - i + 1)->prog->filename);
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
                const program_t *prog = (i ? (csp-i+1)->prog : current_prog);
                int index = (csp-i)->fr.table_index;
                function_t *cfp = &prog->function_table[index];

                ret->item[i].subtype = STRING_SHARED;
                ret->item[i].u.string = cfp->funcname;
                ref_string(cfp->funcname);
            } else {
                ret->item[i].subtype = STRING_CONSTANT;
                ret->item[i].u.string = (((csp - i)->framekind & FRAME_MASK) == FRAME_CATCH) ? "CATCH" : "<function>";
            }
        }
        break;
    case 3:
        ret->item[0].type = T_STRING;
        ret->item[0].subtype = STRING_CONSTANT;
        ret->item[0].u.string = origin_name(caller_type);
        
        for (i = 1; i < n; i++) {
            ret->item[i].type = T_STRING;
            ret->item[i].subtype = STRING_CONSTANT;
            ret->item[i].u.string = origin_name((csp-i+1)->caller_type);
        }
        break;
    }
    put_array(ret);
}
#endif

#ifdef F_CAPITALIZE
void
f_capitalize (void)
{
    if (uislower(sp->u.string[0])) {
        unlink_string_svalue(sp);
        //unlinked, so this is ok
        ((char *)(sp->u.string))[0] = toupper((unsigned char)sp->u.string[0]);
    }
}
#endif

#ifdef F_CHILDREN
void
f_children (void)
{
    array_t *vec;

    vec = children(sp->u.string);
    free_string_svalue(sp);
    put_array(vec);
}
#endif

#ifdef F_CLASSP
void
f_classp (void)
{
    if (sp->type == T_CLASS) {
        free_class(sp->u.arr);
        *sp = const1;
    } else {
        free_svalue(sp, "f_classp");
        *sp = const0;
    }
}
#endif

#ifdef F_CLEAR_BIT
void
f_clear_bit (void)
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
    str = (char *)sp->u.string;

    if (str[ind] > 0x3f + ' ' || str[ind] < ' ')
        error("Illegal bit pattern in clear_bit character %d\n", ind);
    str[ind] = ((str[ind] - ' ') & ~(1 << bit)) + ' ';
}
#endif

#ifdef F_CLONEP
void
f_clonep (void)
{
    if ((sp->type == T_OBJECT) && (sp->u.ob->flags & O_CLONE)) {
        free_object(&sp->u.ob, "f_clonep");
        *sp = const1;
    } else {
        free_svalue(sp, "f_clonep");
        *sp = const0;
    }
}
#endif

#ifdef F__NEW
void
f__new (void)
{
    svalue_t *arg = sp - st_num_arg + 1;
    object_t *ob;

    ob = clone_object(arg->u.string, st_num_arg - 1);
    
    free_string_svalue(sp);
    if (ob) {
        put_unrefed_undested_object(ob, "f_clone_object");
    } else *sp = const0;
}
#endif

#ifdef F_CP
void
f_cp (void)
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
f_crc32 (void)
{
    int len;
    unsigned char *buf;
    UINT32 crc;

    if (sp->type == T_STRING) {
        len = SVALUE_STRLEN(sp);
        buf = (unsigned char *) sp->u.string;
#ifndef NO_BUFFER_TYPE
    } else if (sp->type == T_BUFFER) {
        len = sp->u.buf->size;
        buf = sp->u.buf->item;
#endif
    } else {
#ifdef NO_BUFFER_TYPE
        bad_argument(sp, T_STRING, 1, F_CRC32);
#else
        bad_argument(sp, T_STRING | T_BUFFER, 1, F_CRC32);
#endif
    }
    crc = compute_crc32(buf, len);
    free_svalue(sp, "f_crc32");
    put_number(crc);
}
#endif

#ifdef F_CTIME
void
f_ctime (void)
{
    char *cp, *nl, *p;
    int l;
    if(st_num_arg)
      cp = time_string((time_t)sp->u.number);
    else {
      push_number(0);
      cp = time_string((time_t)current_time);
    }
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
f_deep_inherit_list (void)
{
    array_t *vec;

    vec = deep_inherit_list(sp->u.ob);
    free_object(&sp->u.ob, "f_deep_inherit_list");
    put_array(vec);
}
#endif

#ifdef DEBUG_MACRO

#ifdef F_SET_DEBUG_LEVEL
void
f_set_debug_level (void)
{
    if (sp->type == T_STRING) {
        debug_level_set(sp->u.string);
        free_string_svalue(sp--);
    } else
        debug_level = (sp--)->u.number;
}

void
f_clear_debug_level (void) {
    debug_level_clear(sp->u.string);
}

void     
f_debug_levels (void) {
    /* not in debug.h since debug.h is included in many places that don't
       know about mapping_t */
    mapping_t *debug_levels (void);
    
    push_refed_mapping(debug_levels());
}
#endif

#endif

#ifdef F_DEEP_INVENTORY
void
f_deep_inventory (void)
{
    array_t *vec;

    vec = deep_inventory(sp->u.ob, 0);
    free_object(&sp->u.ob, "f_deep_inventory");
    put_array(vec);
}
#endif

#ifdef F_DESTRUCT
void
f_destruct (void)
{
    destruct_object(sp->u.ob);
    sp--; /* Ok since the object was removed from the stack */
}
#endif

#ifdef F_DUMPALLOBJ
void
f_dumpallobj (void)
{
    if (st_num_arg) {
        dumpstat(sp->u.string);
        free_string_svalue(sp--);
    } else {
        dumpstat("/OBJ_DUMP");
    }
}
#endif

#ifdef F_ED
void
f_ed (void)
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
void f_ed_cmd (void)
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
void f_ed_start (void)
{
    char *res;
    const char *fname;
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
    else {
        STACK_INC;
        sp->type = T_STRING;
    }
    
    if (res) {
        sp->subtype = STRING_MALLOC;
        sp->u.string = res;
    } else {
        sp->subtype = STRING_CONSTANT;
        sp->u.string = "";
    }
}
#endif

#ifdef F_ENABLE_WIZARD
void
f_enable_wizard (void)
{
    if (current_object->interactive)
        current_object->flags |= O_IS_WIZARD;
}
#endif

#ifdef F_ERROR
void
f_error (void)
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
    
    error_handler(err_buf);
}
#endif

#ifdef F_DISABLE_WIZARD
void
f_disable_wizard (void)
{
    if (current_object->interactive)
        current_object->flags &= ~O_IS_WIZARD;
}
#endif

#ifdef F_ENVIRONMENT
void
f_environment (void)
{
    object_t *ob;
    
    if (st_num_arg) {
        if ((ob = sp->u.ob)->flags & O_DESTRUCTED)
            error("environment() of destructed object.\n");
        ob = ob->super;
        free_object(&(sp--)->u.ob, "f_environment");
    } else if (!(current_object->flags & O_DESTRUCTED))
        ob = current_object->super;
    else 
	error("environment() of destructed object.\n");

    if (ob && object_visible(ob)) 
	push_object(ob);
    else 
	push_number(0);
}
#endif

#ifdef F_EXEC
void
f_exec (void)
{
    int i;

    i = replace_interactive((sp - 1)->u.ob, sp->u.ob);

    /* They might have been destructed */
    if (sp->type == T_OBJECT)
        free_object(&sp->u.ob, "f_exec:1");
    if ((--sp)->type == T_OBJECT)
        free_object(&sp->u.ob, "f_exec:2");
    put_number(i);
}
#endif

#ifdef F_EXPLODE
void
f_explode (void)
{
    array_t *vec;

    int len = SVALUE_STRLEN(sp-1);
    
    vec = explode_string((sp - 1)->u.string, len,
                         sp->u.string, SVALUE_STRLEN(sp));
    free_string_svalue(sp--);
    free_string_svalue(sp);
    put_array(vec);
}
#endif

#ifdef F_FILE_NAME
void
f_file_name (void)
{
    char *res;

    /* This function now returns a leading '/' */
    res = (char *) add_slash(sp->u.ob->obname);
    free_object(&sp->u.ob, "f_file_name");
    put_malloced_string(res);
}
#endif

#ifdef F_FILE_SIZE
void
f_file_size (void)
{
    long i = file_size(sp->u.string);
    free_string_svalue(sp);
    put_number(i);
}
#endif

#ifdef F_FILTER
void
f_filter (void)
{
    svalue_t *arg = sp - st_num_arg + 1;

    if (arg->type == T_MAPPING) filter_mapping(arg, st_num_arg);
    else if (arg->type == T_STRING) filter_string(arg, st_num_arg);
    else filter_array(arg, st_num_arg);
}
#endif

#ifdef F_FIND_CALL_OUT
void
f_find_call_out (void)
{
    int i;
#ifdef CALLOUT_HANDLES
    if (sp->type == T_NUMBER) {
        i = find_call_out_by_handle(sp->u.number);
    } else { /* T_STRING */
#endif
        i = find_call_out(current_object, sp->u.string);
        free_string_svalue(sp);
#ifdef CALLOUT_HANDLES
    }
#endif
    put_number(i);
}
#endif

#ifdef F_FIND_OBJECT
void
f_find_object (void)
{
    object_t *ob;
    
    if ((sp--)->u.number)
        ob = find_object(sp->u.string);
    else
        ob = find_object2(sp->u.string);
    free_string_svalue(sp);
    if (ob && object_visible(ob)) { 
        /* find_object only returns undested objects */
        put_unrefed_undested_object(ob, "find_object"); 
    } else
        *sp = const0;
}
#endif

#ifdef F_FUNCTION_PROFILE
/* f_function_profile: John Garnett, 1993/05/31, 0.9.17.3 */
void
f_function_profile (void)
{
    array_t *vec;
    mapping_t *map;
    program_t *prog;
    int nf, j;
    object_t *ob;

    ob = sp->u.ob;

    prog = ob->prog;
    nf = prog->num_functions_defined;
    vec = allocate_empty_array(nf);
    for (j = 0; j < nf; j++) {
        map = allocate_mapping(3);
        add_mapping_pair(map, "calls", prog->function_table[j].calls);
        add_mapping_pair(map, "self", prog->function_table[j].self
                         - prog->function_table[j].children);
        add_mapping_pair(map, "children", prog->function_table[j].children);
        add_mapping_shared_string(map, "name", prog->function_table[j].funcname);
        vec->item[j].type = T_MAPPING;
        vec->item[j].u.map = map;
        prog->function_table[j].calls = 0;
        prog->function_table[j].self = 0;
        prog->function_table[j].children = 0;

    }
    free_object(&ob, "f_function_profile");
    put_array(vec);
}
#endif

#ifdef F_FUNCTION_EXISTS
void
f_function_exists (void)
{
    const char *str;
    char *res;
    int l;
    object_t *ob;
    int flag = 0;
    
    if (st_num_arg > 1) {
        if (st_num_arg > 2)
            flag = (sp--)->u.number;
        ob = sp->u.ob;
        free_object(&(sp--)->u.ob, "f_function_exists");
    } else {
        if (current_object->flags & O_DESTRUCTED) {
            free_string_svalue(sp);
            *sp = const0;
            return;
        }
        ob = current_object;
    }

    str = function_exists(sp->u.string, ob, flag);
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
void f_generate_source (void)
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
f_get_char (void)
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
f_get_config (void)
{
    if (!get_config_item(sp, sp))
        error("Bad argument to get_config()\n");
}
#endif

#ifdef F_GET_DIR
void
f_get_dir (void)
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
f_implode (void)
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
f_in_edit (void)
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
        free_object(&sp->u.ob, "f_in_edit:1");
        put_malloced_string(add_slash(fn));
        return;
    }
    free_object(&sp->u.ob, "f_in_edit:1");
    *sp = const0;
    return;
}
#endif

#ifdef F_IN_INPUT
void
f_in_input (void)
{
    int i;

    i = sp->u.ob->interactive && sp->u.ob->interactive->input_to;
    free_object(&sp->u.ob, "f_in_input");
    put_number(i != 0);
}
#endif

#ifdef F_INHERITS
int
inherits (program_t * prog, program_t * thep)
{
    int j, k = prog->num_inherited;
    program_t *pg;

    for (j = 0; j < k; j++) {
        if ((pg = prog->inherit[j].prog) == thep)
            return 1;
        if (!strcmp(pg->filename, thep->filename))
            return 2;
        if (inherits(pg, thep))
            return 1;
    }
    return 0;
}

void
f_inherits (void)
{
    object_t *ob, *base;
    int i;

    base = (sp--)->u.ob;
    ob = find_object2(sp->u.string);
    if (!ob) {
        free_object(&base, "f_inherits");
        assign_svalue(sp, &const0);
        return;
    }
    i = inherits(base->prog, ob->prog);
    free_object(&base, "f_inherits");
    free_string_svalue(sp);
    put_number(i);
}
#endif

#ifdef F_SHALLOW_INHERIT_LIST
void
f_shallow_inherit_list (void)
{
    array_t *vec;

    vec = inherit_list(sp->u.ob);
    free_object(&sp->u.ob, "f_inherit_list");
    put_array(vec);
}
#endif

#ifdef F_INPUT_TO
void
f_input_to (void)
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
f_interactive (void)
{
    int i;

    i = (sp->u.ob->interactive != 0);
    free_object(&sp->u.ob, "f_interactive");
    put_number(i);
}
#endif

#ifdef F_HAS_MXP
void
f_has_mxp (void)
{
    int i=0;

    if (sp->u.ob->interactive)
    {
       i = sp->u.ob->interactive->iflags & USING_MXP;
       i = !!i; //force 1 or 0
    }
    free_object(&sp->u.ob, "f_has_mxp");
    put_number(i);
}
#endif

#ifdef F_INTP
void
f_intp (void)
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
f_functionp (void)
{
    int i;
    
    if (sp->type == T_FUNCTION) {
        i = sp->u.fp->hdr.type;
        if (sp->u.fp->hdr.args) 
            i |= FP_HAS_ARGUMENTS;
        if (!sp->u.fp->hdr.owner || (sp->u.fp->hdr.owner->flags & O_DESTRUCTED))
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
f_keys (void)
{
    array_t *vec;

    vec = mapping_indices(sp->u.map);
    free_mapping(sp->u.map);
    put_array(vec);
}
#endif

#ifdef F_VALUES
void
f_values (void)
{
    array_t *vec;

    vec = mapping_values(sp->u.map);
    free_mapping(sp->u.map);
    put_array(vec);
}
#endif

#ifdef F_LINK
void
f_link (void)
{
    svalue_t *ret, *arg;
    int i;

    arg = sp;
    push_svalue(arg - 1);
    push_svalue(arg);
    ret = apply_master_ob(APPLY_VALID_LINK, 2);
    if (MASTER_APPROVED(ret))
        i = do_rename((sp - 1)->u.string, sp->u.string, F_LINK);
    else
        i = 0;
    (--sp)->type = T_NUMBER;
    sp->u.number = i;
    sp->subtype = 0;
}
#endif                          /* F_LINK */

#ifdef F_LOWER_CASE
void
f_lower_case (void)
{
    char *str;

    str = (char *)sp->u.string;
    /* find first upper case letter, if any */
    for (; *str; str++) {
        if (uisupper(*str)) {
            int l = str - sp->u.string;
            unlink_string_svalue(sp);
            str = (char *)sp->u.string + l;
            *str = tolower((unsigned char)*str);
            for (str++; *str; str++) {
                if (uisupper(*str))
                    *str = tolower((unsigned char)*str);
            }
            return;
        }
    }
}
#endif

#ifdef F_MALLOC_STATUS
void f_malloc_status (void)
{
    outbuffer_t ob;

    outbuf_zero(&ob);

#ifdef MMALLOC
    outbuf_add(&ob, "Using mmap malloc");
#endif
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
f_map_delete (void)
{
    mapping_delete((sp - 1)->u.map, sp);
    pop_stack();
#ifndef COMPAT_32
    free_mapping((sp--)->u.map);
#endif
}
#endif

#ifdef F_MAPP
void
f_mapp (void)
{
    if (sp->type == T_MAPPING) {
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
f_map (void)
{
    svalue_t *arg = sp - st_num_arg + 1;

    if (arg->type == T_MAPPING) map_mapping(arg, st_num_arg);
    else if (arg->type == T_ARRAY) map_array(arg, st_num_arg);
    else map_string(arg, st_num_arg);
}
#endif

#ifdef F_MASTER
void
f_master (void)
{
    if (!master_ob)
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
f_match_path (void)
{
    svalue_t *value;
    register const char *src;
    register char *dst;
    svalue_t *nvalue;
    mapping_t *map;
    char *tmpstr;
    
    value = &const0u;

    tmpstr = DMALLOC(SVALUE_STRLEN(sp) + 1, TAG_STRING, "match_path");

    src = sp->u.string;
    dst = tmpstr;
    
    while (*src != '\0') {
        while (*src != '/' && *src != '\0')
            *dst++ = *src++;
        if (*src == '/') {
            while (*++src == '/');
            if (*src != '\0' || dst == tmpstr)
                *dst++ = '/';
        }
        *dst = '\0';
        nvalue = find_string_in_mapping((sp - 1)->u.map, tmpstr);
        
        if (nvalue != &const0u)
            value = nvalue;
    }

    FREE(tmpstr);
    /* Don't free mapping first, in case sometimes one uses a ref 1 mapping */
    /* Randor - 5/29/94 */
    free_string_svalue(sp--);
    map = sp->u.map;
    assign_svalue_no_free(sp, value);
    free_mapping(map);
}
#endif                          /* F_MATCH_PATH */

#ifdef F_MEMBER_ARRAY
void
f_member_array (void)
{
    array_t *v;
    int flag = 0;
    int i;
    int size;

    if (st_num_arg > 2) {
        if (st_num_arg > 3) {
            CHECK_TYPES(sp, T_NUMBER, 4, F_MEMBER_ARRAY);
            flag = (sp--)->u.number;
        }
        CHECK_TYPES(sp, T_NUMBER, 3, F_MEMBER_ARRAY);

        i = (sp--)->u.number;
        if (i<0) bad_arg(3, F_MEMBER_ARRAY);
    } else 
	i = 0;

    if (sp->type == T_STRING) {
        char *res;
        if(flag & 2)
          error("member_array: can not search backwards in strings");
        CHECK_TYPES(sp-1, T_NUMBER, 1, F_MEMBER_ARRAY);
        if (i > SVALUE_STRLEN(sp)) error("Index to start search from in member_array() is > string length.\n");
        if ((res = strchr(sp->u.string + i, (sp-1)->u.number)))
            i = res - sp->u.string;
        else
            i = -1;
        free_string_svalue(sp--);
    } else {
        svalue_t *sv;
        svalue_t *find;
        int flen;

        size = (v = sp->u.arr)->size;
        find = (sp - 1);
        /* optimize a bit */
        if (find->type == T_STRING) {
            /* *not* COUNTED_STRLEN() which can do a (costly) strlen() call */
            if (find->subtype & STRING_COUNTED)
                flen = MSTR_SIZE(find->u.string);
            else if (flag & 1)
                flen = strlen(find->u.string);
            else flen = 0;
        }

        for (; i < size; i++) {
            int tmp = i;
            if(flag & 2)
                tmp = size - tmp - 1;
            switch (find->type|(sv= v->item + tmp)->type) {
            case T_STRING:
                if (flag & 1) {
                    if (flen && (sv->subtype & STRING_COUNTED)
                        && flen > MSTR_SIZE(sv->u.string))
                      continue;
                    if (strncmp(find->u.string, sv->u.string, flen)) continue;
                } else {
                  if (flen && (sv->subtype & STRING_COUNTED)
                      && flen != MSTR_SIZE(sv->u.string))
                    continue;
                  if (strcmp(find->u.string, sv->u.string)) continue;
                }
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
                if (sv->u.ob->flags & O_DESTRUCTED) {
                    assign_svalue(sv, &const0u);
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
#ifndef NO_BUFFER_TYPE
            case T_BUFFER:
                if (find->u.buf == sv->u.buf) break;
                continue;
#endif
            default:
                if (sv->type == T_OBJECT && (sv->u.ob->flags & O_DESTRUCTED)) {
                    assign_svalue(sv, &const0u);
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
    if(flag & 2)
        i = size - i - 1;
    put_number(i);
}
#endif

#ifdef F_MESSAGE
void
f_message (void)
{
    array_t *use, *avoid;
    int num_arg = st_num_arg;
    svalue_t *args;

    args = sp - num_arg + 1;
    switch (args[2].type) {
    case T_OBJECT:
    case T_STRING:
        use = allocate_empty_array(1);
        use->item[0] = args[2];
        args[2].type = T_ARRAY;
        args[2].u.arr = use;
        break;
    case T_ARRAY:
        use = args[2].u.arr;
        break;
    case T_NUMBER:
        if (args[2].u.number == 0) {
            int len = SVALUE_STRLEN(args + 1);
            
            /* this is really bad and probably should be rm'ed -Beek;
             * on the other hand, we don't have a debug_message() efun yet.
             * Well, there is one in contrib now ...
             */
            /* for compatibility (write() simul_efuns, etc)  -bobf */
            if (len > LARGEST_PRINTABLE_STRING)
                error("Printable strings limited to length of %d.\n",
                      LARGEST_PRINTABLE_STRING);

            add_message(command_giver, args[1].u.string, len);
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
            avoid = allocate_empty_array(1);
            avoid->item[0] = args[3];
            args[3].type = T_ARRAY;
            args[3].u.arr = avoid;
            break;
        case T_ARRAY:
            avoid = args[3].u.arr;
            break;
        default:
            avoid = &the_null_array;
        }
    } else
        avoid = &the_null_array;
    do_message(&args[0], &args[1], use, avoid, 1);
    pop_n_elems(num_arg);
}
#endif

#ifdef F_MKDIR
void
f_mkdir (void)
{
    const char *path;

    path = check_valid_path(sp->u.string, current_object, "mkdir", 1);
    if (!path || OS_mkdir(path, 0770) == -1) {
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
f_move_object (void)
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
void f_mud_status (void)
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
            outbuf_addv(&ob, "Open file test failed: %s\n", port_strerror(errno));
        }

        outbuf_addv(&ob, "current working directory: %s\n\n",
                    get_current_dir(dir_buf, 1024));
        outbuf_add(&ob, "add_message statistics\n");
        outbuf_add(&ob, "------------------------------\n");
        outbuf_addv(&ob, "Calls to add_message: %d   Packets: %d   Average packet size: %f\n\n",
        add_message_calls, inet_packets, (float) inet_volume / inet_packets);

        stat_living_objects(&ob);

#ifdef F_CACHE_STATS
        print_cache_stats(&ob);
        outbuf_add(&ob, "\n");
#endif
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
#ifndef DEBUG
        outbuf_addv(&ob, "Objects:\t\t\t%8d %8d\n",
                    tot_alloc_object, tot_alloc_object_size);
#else
        outbuf_addv(&ob, "Objects:\t\t\t%8d %8d (%8d dangling)\n",
                    tot_alloc_object, tot_alloc_object_size, tot_dangling_object);
#endif
        outbuf_addv(&ob, "Prog blocks:\t\t\t%8d %8d\n",
                    total_num_prog_blocks, total_prog_block_size);
#ifdef ARRAY_STATS
        outbuf_addv(&ob, "Arrays:\t\t\t\t%8d %8d\n", num_arrays,
                    total_array_size);
#else
        outbuf_add(&ob, "<Array statistics disabled, no information available>\n");
#endif
        outbuf_addv(&ob, "Mappings:\t\t\t%8d %8d\n", num_mappings,
                    total_mapping_size);
        outbuf_addv(&ob, "Mappings(nodes):\t\t%8d\n", total_mapping_nodes);
        outbuf_addv(&ob, "Interactives:\t\t\t%8d %8d\n", num_user,
                    num_user * sizeof(interactive_t));

        tot = show_otable_status(&ob, verbose) +
            heart_beat_status(&ob, verbose) +
            add_string_status(&ob, verbose) +
            print_call_out_usage(&ob, verbose);
    }

    tot += total_prog_block_size +
#ifdef ARRAY_STATS
        total_array_size +
#endif
        total_mapping_size +
        tot_alloc_sentence * sizeof(sentence_t) +
        tot_alloc_object_size +
        num_user * sizeof(interactive_t) +
        res;

    if (!verbose) {
        outbuf_add(&ob, "\t\t\t\t\t --------\n");
        outbuf_addv(&ob, "Total:\t\t\t\t\t %8d\n", tot);
    }
    outbuf_push(&ob);
}
#endif

#ifdef F_OBJECTP
void
f_objectp (void)
{
    if (sp->type == T_OBJECT && !(sp->u.ob->flags & O_DESTRUCTED)) {
        free_object(&sp->u.ob, "f_objectp");
        *sp = const1;
    } else {
        free_svalue(sp, "f_objectp");
        *sp = const0;
    }
}
#endif

#ifdef F_OPCPROF
void
f_opcprof (void)
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
f_origin (void)
{
    push_constant_string(origin_name(caller_type));
}
#endif

#ifdef F_POINTERP
void
f_pointerp (void)
{
    if (sp->type == T_ARRAY) {
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
f_present (void)
{
    int num_arg = st_num_arg;
    svalue_t *arg = sp - num_arg + 1;
    object_t *ob;
    
#if !defined(NO_RESETS) && defined(LAZY_RESETS)
    if (num_arg == 2) {
        try_reset(arg[1].u.ob);
    }
#endif
    ob = object_present(arg, num_arg == 1 ? 0 : arg[1].u.ob);
    pop_n_elems(num_arg);
    if (ob && object_visible(ob)) push_object(ob);
    else push_number(0);
}
#endif

#ifdef F_PREVIOUS_OBJECT
void
f_previous_object (void)
{
    control_stack_t *p;
    int i;
    object_t *ob;
    
    if ((i = sp->u.number) > 0) {
        if (i >= CFG_MAX_CALL_DEPTH) {
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
            if ((p->framekind & FRAME_OB_CHANGE) && p->prev_ob)
		i++;
        } while (--p >= control_stack);
        v = allocate_empty_array(i);
        p = csp;
        if (previous_ob) {
            if (!(previous_ob->flags & O_DESTRUCTED)) {
                v->item[0].type = T_OBJECT;
                v->item[0].u.ob = previous_ob;
                add_ref(previous_ob, "previous_object(-1)");
            } else 
		v->item[0] = const0u;
            i = 1;
        } else 
	    i = 0;
        do {
            if ((p->framekind & FRAME_OB_CHANGE) && (ob = p->prev_ob)) {
                if (!(ob->flags & O_DESTRUCTED)) {
                    v->item[i].type = T_OBJECT;
                    v->item[i].u.ob = ob;
                    add_ref(ob, "previous_object(-1)");
                } else v->item[i] = const0u;
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
f_printf (void)
{
    int num_arg = st_num_arg;
    char *ret;
    
    if (command_giver) {
        ret = string_print_formatted((sp - num_arg + 1)->u.string,
                                     num_arg - 1, sp - num_arg + 2);
        if (ret) {
            tell_object(command_giver, ret, COUNTED_STRLEN(ret));
            FREE_MSTR(ret);
        }
    }
    
    pop_n_elems(num_arg);
}
#endif

#ifdef F_PROCESS_STRING
void
f_process_string (void)
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
f_process_value (void)
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
f_query_ed_mode (void)
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
f_query_host_name (void)
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
f_query_idle (void)
{
    int i;

    i = query_idle(sp->u.ob);
    free_object(&sp->u.ob, "f_query_idle");
    put_number(i);
}
#endif

#ifdef F_QUERY_IP_NAME
void
f_query_ip_name (void)
{
    char *tmp;

    tmp = query_ip_name(st_num_arg ? sp->u.ob : 0);
    if (st_num_arg) free_object(&(sp--)->u.ob, "f_query_ip_name");
    if (!tmp) push_number(0);
    else share_and_push_string(tmp);
}
#endif

#ifdef F_QUERY_IP_NUMBER
void
f_query_ip_number (void)
{
    char *tmp;

    tmp = query_ip_number(st_num_arg ? sp->u.ob : 0);
    if (st_num_arg) free_object(&(sp--)->u.ob, "f_query_ip_number");
    if (!tmp) push_number(0);
    else share_and_push_string(tmp);
}
#endif

#ifdef F_QUERY_LOAD_AVERAGE
void
f_query_load_average (void)
{
    copy_and_push_string(query_load_av());
}
#endif

#ifdef F_QUERY_PRIVS
void
f_query_privs (void)
{
    object_t *ob;
    
    ob = sp->u.ob;
    if (ob->privs != NULL) {
        sp->type = T_STRING;
        sp->u.string = make_shared_string(ob->privs);
        sp->subtype = STRING_SHARED;
	free_object(&ob, "f_query_privs");
    } else {
        free_object(&ob, "f_query_privs");
        *sp = const0;
    }
}
#endif

#ifdef F_QUERY_SNOOPING
void
f_query_snooping (void)
{
    object_t *ob;
    
    ob = query_snooping(sp->u.ob);
    free_object(&sp->u.ob, "f_query_snooping");
    if (ob) { put_unrefed_undested_object(ob, "query_snooping"); }
    else *sp = const0;
}
#endif

#ifdef F_QUERY_SNOOP
void
f_query_snoop (void)
{
    object_t *ob;
    
    ob = query_snoop(sp->u.ob);
    free_object(&sp->u.ob, "f_query_snoop");
    if (ob) { put_unrefed_undested_object(ob, "query_snoop"); }
    else *sp = const0;
}
#endif

#ifdef F_RANDOM
void
f_random (void)
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
f_read_bytes (void)
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
f_read_buffer (void)
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
        STACK_INC;
        sp->type = T_BUFFER;
        sp->u.buf = buf;
        FREE_MSTR(str);
    } else {                    /* T_BUFFER */
        push_malloced_string(str);
    }
}
#endif

#ifdef F_READ_FILE
void
f_read_file (void)
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
f_receive (void)
{
    if (sp->type == T_STRING) {
        if (current_object->interactive) {
            int len = SVALUE_STRLEN(sp);
            
            if (len > LARGEST_PRINTABLE_STRING)
                error("Printable strings limited to length of %d.\n",
                      LARGEST_PRINTABLE_STRING);
                
            add_message(current_object, sp->u.string, len);
        }
        free_string_svalue(sp--);
    }
#ifndef NO_BUFFER_TYPE
    else {
        if (current_object->interactive)
            add_message(current_object, (char *)sp->u.buf->item, sp->u.buf->size);

        free_buffer((sp--)->u.buf);
    }
#endif
}
#endif

#ifdef F_REG_ASSOC
void
f_reg_assoc (void) {
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
f_regexp (void)
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
        sp->u.arr = v;
    }
}
#endif

#ifdef F_REMOVE_CALL_OUT
void
f_remove_call_out (void)
{
    int i;

    if (st_num_arg) {
#ifdef CALLOUT_HANDLES
        if (sp->type == T_STRING) {
#endif
            i = remove_call_out(current_object, sp->u.string);
            free_string_svalue(sp);
#ifdef CALLOUT_HANDLES
        } else {
            i = remove_call_out_by_handle(sp->u.number);
        }
#endif
    } else {
        remove_all_call_out(current_object);
        i = 0;
        STACK_INC;
    }
    put_number(i);
}
#endif

#ifdef F_RENAME
void
f_rename (void)
{
    int i;

    i = do_rename((sp - 1)->u.string, sp->u.string, F_RENAME);
    free_string_svalue(sp--);
    free_string_svalue(sp);
    put_number(i);
}
#endif                          /* F_RENAME */

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
f_replace_string (void)
{
    int plen, rlen, dlen, slen, first, last, cur, j;
    
    const char *pattern;
    const char *replace;
    register const char *src;
    register char *dst1, *dst2;
    svalue_t *arg;
    int skip_table[256];
    const char *slimit;
    const char *flimit;
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
        dst2 = dst1 = (char *)arg->u.string;
        
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
                        memcpy(dst2, src, plen);
                        dst2 += plen;
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
                            *(char *)src = *replace;
                        }
                    }
                    src++;
                }
            } else { /* rlen is zero */
                while (*src) {
                    if (*src++ == *pattern) {
                        cur++;
                        if (cur >= first) {
                            dst2 = (char *)src - 1;
                            while (*src) {
                                if (*src == *pattern) {
                                    cur++;
                                    if (cur <= last) {
                                        src++;
                                        continue;
                                    } else {
                                        while (*src)
                                            *dst2++ = *src++;
                                        break;
                                    }
                                }
                                *dst2++ = *src++;
                            }
                            *dst2 = 0;
                            arg->u.string = extend_string(dst1, dst2 - dst1);
                            break;
                        }
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
                        if (max_string_length - dlen <= rlen) {
                            pop_n_elems(st_num_arg);
                            push_svalue(&const0u);
                            FREE_MSTR(dst1);
                            return;
                        }
                        memcpy(dst2, replace, rlen);
                        dst2 += rlen;
                        dlen += rlen;
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
f_resolve (void)
{
    int i;

    i = query_addr_number((sp - 1)->u.string, sp);
    pop_stack();
    free_string_svalue(sp);
    put_number(i);
}
#endif

#ifdef F_RESTORE_OBJECT
void
f_restore_object (void)
{
    int flag, tmp_eval;

    flag = (st_num_arg > 1) ? (sp--)->u.number : 0;

    flag = restore_object(current_object, sp->u.string, flag);

    free_string_svalue(sp);
    put_number(flag);
}
#endif

#ifdef F_RESTORE_VARIABLE
void
f_restore_variable (void) {
    svalue_t v;

    unlink_string_svalue(sp);
    v.type = T_NUMBER;

    //unlinked string
    restore_variable(&v, (char *)sp->u.string);
    FREE_MSTR(sp->u.string);
    *sp = v;
}
#endif

#ifdef F_RM
void
f_rm (void)
{
    int i;

    i = remove_file(sp->u.string);
    free_string_svalue(sp);
    put_number(i);
}
#endif

#ifdef F_RMDIR
void
f_rmdir (void)
{
    const char *path;

    path = check_valid_path(sp->u.string, current_object, "rmdir", 1);
    if (!path || rmdir(path) == -1) {
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
f_save_object (void)
{
    int flag, tmp_eval;

    if (st_num_arg == 2) {
        flag = (sp--)->u.number;
    } else {
        flag = 0;
    }
    
    flag = save_object(current_object, sp->u.string, flag);

    free_string_svalue(sp);
    put_number(flag);
}
#endif

#ifdef F_SAVE_VARIABLE
void
f_save_variable (void) {
    char *p;

    p = save_variable(sp);
    pop_stack();
    push_malloced_string(p);
}
#endif

#ifdef F_SAY
void
f_say (void)
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
        avoid = &the_null_array;
        say(sp, avoid);
        pop_stack();
    } else {
        if (sp->type == T_OBJECT) {
            vtmp.item[0].type = T_OBJECT;
            vtmp.item[0].u.ob = sp->u.ob;
            avoid = &vtmp;
        } else {                /* must be an array... */
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
f_set_eval_limit (void)
{
    switch (sp->u.number) {
    case 0:
        sp->u.number = max_cost;
	set_eval(max_cost);
        break;
    case -1:
        sp->u.number = get_eval();
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
f_set_bit (void)
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
        str = (char *)sp->u.string;
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
f_set_heart_beat (void)
{
    set_heart_beat(current_object, (sp--)->u.number);
}
#endif

#ifdef F_QUERY_HEART_BEAT
void
f_query_heart_beat (void)
{
    int num = query_heart_beat(sp->u.ob);
    free_object(&sp->u.ob, "f_query_heart_beat");
    put_number(num);
}
#endif

#ifdef F_SET_HIDE
void
f_set_hide (void)
{
    if (!valid_hide(current_object)) {
        sp--;
        return;
    }
    if ((sp--)->u.number) {
        num_hidden++;
        if (!(current_object->flags & O_HIDDEN) && current_object->interactive)
            num_hidden_users++;
        current_object->flags |= O_HIDDEN;
    } else {
        num_hidden--;
        if ((current_object->flags & O_HIDDEN) && current_object->interactive)
            num_hidden_users--;
        current_object->flags &= ~O_HIDDEN;
    }
}
#endif

#ifdef F_SET_LIGHT
void
f_set_light (void)
{
    object_t *o1;

    add_light(current_object, sp->u.number);
    o1 = current_object;
#ifndef NO_ENVIRONMENT
    while (o1->super)
        o1 = o1->super;
#endif
    sp->u.number = o1->total_light;
}
#endif

#ifdef F_SET_PRIVS
void
f_set_privs (void)
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
    free_object(&ob, "f_set_privs");
    sp--;
}
#endif

#ifdef F_SHADOW
void
f_shadow (void)
{
    object_t *ob;

    ob = (sp - 1)->u.ob;
    if (!((sp--)->u.number)) {
        ob = ob->shadowed;
        free_object(&sp->u.ob, "f_shadow:1");
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
            free_object(&ob, "f_shadow:2");
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
        free_object(&sp->u.ob, "f_shadow:3");
        add_ref(ob, "shadow(ob, 1)");
        sp->u.ob = ob;
        return;
    }
    free_object(&sp->u.ob, "f_shadow:4");
    *sp = const0;
}
#endif

#ifdef F_SHOUT
void
f_shout (void)
{
    shout_string(sp->u.string);
    free_string_svalue(sp--);
}
#endif

#ifdef F_SHUTDOWN
void
f_shutdown (void)
{
    if (st_num_arg) {
        shutdownMudOS(sp->u.number);
    } else {
        shutdownMudOS(0);
        push_number(0);
    }
}
#endif

#ifdef F_SIZEOF
void
f_sizeof (void)
{
    int i;

    switch (sp->type) {
    case T_CLASS:
        i = sp->u.arr->size;
        free_class(sp->u.arr);
        break;
    case T_ARRAY:
        i = sp->u.arr->size;
        free_array(sp->u.arr);
        break;
    case T_MAPPING:
        i = sp->u.map->count;
        free_mapping(sp->u.map);
        break;
#ifndef NO_BUFFER_TYPE
    case T_BUFFER:
        i = sp->u.buf->size;
        free_buffer(sp->u.buf);
        break;
#endif
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
f_snoop (void)
{
    /*
     * This one takes a variable number of arguments. It returns 0 or an
     * object.
     */
    if (st_num_arg == 1) {
        if (!new_set_snoop(sp->u.ob, 0) || (sp->u.ob->flags & O_DESTRUCTED)) {
            free_object(&sp->u.ob, "f_snoop:1");
            *sp = const0;
        }
    } else {
        if (!new_set_snoop((sp - 1)->u.ob, sp->u.ob) || 
            (sp->u.ob->flags & O_DESTRUCTED)) {
            free_object(&(sp--)->u.ob, "f_snoop:2");
            free_object(&sp->u.ob, "f_snoop:3");
            *sp = const0;
        } else {
            free_object(&(--sp)->u.ob, "f_snoop:4");
            sp->u.ob = (sp+1)->u.ob;
        }
    }
}
#endif

#ifdef F_SPRINTF
void
f_sprintf (void)
{
    char *s;
    int num_arg = st_num_arg;

    s = string_print_formatted((sp - num_arg + 1)->u.string,
                               num_arg - 1, sp - num_arg + 2);
    pop_n_elems(num_arg);

    STACK_INC;
    sp->type = T_STRING;
    if (!s) {
        sp->subtype = STRING_CONSTANT;
        sp->u.string = "";
    } else {
        sp->subtype = STRING_MALLOC;
        sp->u.string = s;
    }
}
#endif

#ifdef F_STAT
void
f_stat (void)
{
    struct stat buf;
    const char *path;
    array_t *v;
    object_t *ob;
    
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
f_strsrch (void)
{
    register const char *big, *little, *pos;
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
            pos = strchr(big, little[0]);
        else
            pos = (char *)strstr(big, little);
        /* start at right */
    } else {                    /* XXX: maybe test for -1 */
        if (!little[1])         /* 1 char srch pattern */
            pos = strrchr(big, little[0]);
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
        i = pos - big;
    if (sp->type == T_STRING) free_string_svalue(sp);
    free_string_svalue(--sp);
    put_number(i);
}                               /* strsrch */
#endif

#ifdef F_STRCMP
void
f_strcmp (void)
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
f_stringp (void)
{
    if (sp->type == T_STRING) {
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
f_bufferp (void)
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

#ifdef F_TELL_OBJECT
void
f_tell_object (void)
{
    tell_object((sp - 1)->u.ob, sp->u.string, SVALUE_STRLEN(sp));
    free_string_svalue(sp--);
    pop_stack();
}
#endif

#ifdef F_TELL_ROOM
void
f_tell_room (void)
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

    int num_arg = st_num_arg;
    svalue_t *arg = sp - num_arg + 1;
    object_t *ob;
    
    if (arg->type == T_OBJECT) {
        ob = arg[0].u.ob;
    } else {                    /* must be a string... */
        ob = find_object(arg[0].u.string);
        if (!ob || !object_visible(ob))
            error("Bad argument 1 to tell_room()\n");
    }

    if (num_arg == 2) {
        avoid = &the_null_array;
    } else {
        if (arg[2].type == T_OBJECT) {
            vtmp.item[0].type = T_OBJECT;
            vtmp.item[0].u.ob = arg[2].u.ob;
            avoid = &vtmp;
        } else {
            avoid = arg[2].u.arr;
        }
    }

    tell_room(ob, &arg[1], avoid);
    if (num_arg > 2 && arg[2].type != T_OBJECT)
        free_array(avoid);
    free_svalue(arg + 1, "f_tell_room");
    free_svalue(arg, "f_tell_room");
    sp = arg - 1;
}
#endif

#ifdef F_TEST_BIT
void
f_test_bit (void)
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
f_next_bit (void)
{
    int start = (sp--)->u.number;
    int len = SVALUE_STRLEN(sp);
    int which, bit=0, value;
    
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

#ifdef F__THIS_OBJECT
void
f__this_object (void)
{
    push_object(current_object);
}
#endif

#ifdef F_THIS_PLAYER
void
f_this_player (void)
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
f_set_this_player (void)
{
    if (sp->type == T_NUMBER)
        set_command_giver(0);
    else
        set_command_giver(sp->u.ob);
    pop_stack();
}
#endif

#ifdef F_THROW
void
f_throw (void)
{
    free_svalue(&catch_value, "f_throw");
    catch_value = *sp--;
    throw_error();              /* do the longjump, with extra checks... */
}
#endif

#ifdef F_TIME
void
f_time (void)
{
    push_number(current_time);
}
#endif

#ifdef F__TO_FLOAT
void
f__to_float (void)
{
    double temp = 0;

    switch(sp->type) {
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

#ifdef F__TO_INT
void
f__to_int (void)
{
    switch(sp->type) {
        case T_REAL:
            sp->type = T_NUMBER;
            sp->u.number = (long) sp->u.real;
            break;
        case T_STRING:
        {
            int temp;
            char *p;
            
            temp = strtol(sp->u.string, &p, 10);
            if (*p) {
                /* have to be a little careful here.  Checkign if p ==
                 * sp->u.string isn't good enough.
                 * 
                 * Odd cases:
                 * to_int("  foo")  // p == sp->u.string + 2
                 *
                 * POSIX guarantees the strtol() works in terms of isspace(),
                 * though.  If there is something other than whitespace, then
                 * there was a valid character consistent with the base,
                 * so we were successful.
                 *
                 * (note: this means to_int("10x") == 10.  If you want to
                 *  detect trailing garbage, use sscanf(str, "%d%s", ...).
                 */
                while (p > sp->u.string && uisspace(*(p - 1)))
                    p--;

                if (p == sp->u.string) {
                    free_string_svalue(sp);
                    *sp = const0u;
                    break;
                }
            }
            free_string_svalue(sp);
            sp->u.number = temp;
            sp->type = T_NUMBER;
            break;
        }
#ifndef NO_BUFFER_TYPE
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
#endif
    }
}
#endif

#ifdef F_TYPEOF
void
f_typeof (void)
{
    const char *t = type_name(sp->type);

    free_svalue(sp, "f_typeof");
    put_constant_string(t);
}
#endif

#ifdef F_UNDEFINEDP
void
f_undefinedp (void)
{
    if (sp->type == T_NUMBER) {
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
f_uptime (void)
{
    push_number(current_time - boot_time);
}
#endif

#ifdef F_USERP
void
f_userp (void)
{
    int i;

    i = sp->u.ob->flags & O_ONCE_INTERACTIVE;
    free_object(&sp->u.ob, "f_userp");
    put_number(i != 0);
}
#endif

#ifdef F_USERS
void
f_users (void)
{
    push_refed_array(users());
}
#endif

#ifdef F_WIZARDP
void
f_wizardp (void)
{
    int i;

    i = sp->u.ob->flags & O_IS_WIZARD;
    free_object(&sp->u.ob, "f_wizardp");
    put_number(i != 0);
}
#endif

#ifdef F_VIRTUALP
void
f_virtualp (void)
{
    int i;

    i = sp->u.ob->flags & O_VIRTUAL;
    free_object(&sp->u.ob, "f_virtualp");
    put_number(i != 0);
}
#endif

#ifdef F_WRITE
void
f_write (void)
{
    do_write(sp);
    pop_stack();
}
#endif

#ifdef F_WRITE_BYTES
void
f_write_bytes (void)
{
    int i;

    switch(sp->type) {
        case T_NUMBER:
        {
            int netint;
            char *netbuf;

            if (!sp->u.number) 
		bad_arg(3, F_WRITE_BYTES);
            netint = htonl(sp->u.number);       /* convert to network
                                                 * byte-order */
            netbuf = (char *) &netint;
            i = write_bytes((sp - 2)->u.string, (sp - 1)->u.number, netbuf,
                            sizeof(int));
            break;
        }

#ifndef NO_BUFFER_TYPE
        case T_BUFFER:
        {
            i = write_bytes((sp - 2)->u.string, (sp - 1)->u.number,
                            (char *) sp->u.buf->item, sp->u.buf->size);
            break;
        }
#endif

        case T_STRING:
        {
            i = write_bytes((sp - 2)->u.string, (sp - 1)->u.number,
                            sp->u.string, SVALUE_STRLEN(sp));
            break;
        }

        default:
        {
#ifdef NO_BUFFER_TYPE
            bad_argument(sp, T_STRING | T_NUMBER, 3, F_WRITE_BYTES);
#else
            bad_argument(sp, T_BUFFER | T_STRING | T_NUMBER, 3, F_WRITE_BYTES);
#endif
        }
    }
    free_svalue(sp--, "f_write_bytes");
    free_string_svalue(--sp);
    put_number(i);
}
#endif

#ifdef F_WRITE_BUFFER
void
f_write_buffer (void)
{
    int i;

    if ((sp-2)->type == T_STRING) {
        f_write_bytes();
        return;
    }

    switch(sp->type) {
        case T_NUMBER:
        {
            int netint;
            char *netbuf;

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
    free_svalue(--sp, "f_write_buffer");
    put_number(i);
}
#endif

#ifdef F_WRITE_FILE
void
f_write_file (void)
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
f_dump_file_descriptors (void)
{
    outbuffer_t out;

    outbuf_zero(&out);
    dump_file_descriptors(&out);
    outbuf_push(&out);
}
#endif

#ifdef F_RECLAIM_OBJECTS
void f_reclaim_objects (void)
{
    push_number(reclaim_objects());
}
#endif

#ifdef F_MEMORY_INFO
void
f_memory_info (void)
{
    long mem;
    object_t *ob;
    
    if (st_num_arg == 0) {
        int res, tot;

        if (reserved_area)
            res = RESERVED_SIZE;
        else
            res = 0;
        tot = total_prog_block_size +
#ifdef ARRAY_STATS
            total_array_size +
#endif
            total_mapping_size +
            tot_alloc_object_size +
            tot_alloc_sentence * sizeof(sentence_t) +
            num_user * sizeof(interactive_t) +
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
    /* as documented, object memory usage is not additive due to sharing of
       structures, so always include the program's total size even if this
       object is a clone or the program has more than one reference to it.
       There's no reliable way to determine when the program size should be
       included or not to be more accurate -- Marius, 30-Jul-2000 */
    mem = ob->prog->total_size;
    mem += (data_size(ob) + sizeof(object_t));
    free_object(&ob, "f_memory_info");
    put_number(mem);
}
#endif

#ifdef F_RELOAD_OBJECT
void
f_reload_object (void)
{
    reload_object(sp->u.ob);
    free_object(&(sp--)->u.ob, "f_reload_object");
}
#endif

#ifdef F_QUERY_SHADOWING
void
f_query_shadowing (void)
{
    object_t *ob;
    
    if ((sp->type == T_OBJECT) && (ob = sp->u.ob)->shadowing) {
        add_ref(ob->shadowing, "query_shadowing(ob)");
        sp->u.ob = ob->shadowing;
        free_object(&ob, "f_query_shadowing");
    } else {
        free_svalue(sp, "f_query_shadowing");
        *sp = const0;
    }
}
#endif

#ifdef F_SET_RESET
void
f_set_reset (void)
{
    if (st_num_arg == 2) {
        (sp - 1)->u.ob->next_reset = current_time + sp->u.number;
        free_object(&(--sp)->u.ob, "f_set_reset:1");
        sp--;
    } else {
        sp->u.ob->next_reset = current_time + TIME_TO_RESET / 2 +
            random_number(TIME_TO_RESET / 2);
        free_object(&(sp--)->u.ob, "f_set_reset:2");
    }
}
#endif

#ifdef F_FLOATP
void
f_floatp (void)
{
    if (sp->type == T_REAL) {
        sp->type = T_NUMBER;
        sp->u.number = 1;
    }
    else {
        free_svalue(sp, "f_floatp");
        *sp = const0;
    }
}
#endif

#ifdef F_FLUSH_MESSAGES
void
f_flush_messages (void) {
    if (st_num_arg == 1) {
        if (sp->u.ob->interactive)
            flush_message(sp->u.ob->interactive);
        pop_stack();
    } else {
        int i;
        
        for (i = 0; i < max_users; i++) {
            if (all_users[i] && !(all_users[i]->iflags & CLOSING))
                flush_message(all_users[i]);
        }
    }
}
#endif

#ifdef F_FIRST_INVENTORY
void
f_first_inventory (void)
{
    object_t *ob;
    
    ob = first_inventory(sp);
    free_svalue(sp, "f_first_inventory");
    if (ob) { put_unrefed_undested_object(ob, "first_inventory"); }
    else *sp = const0;
}
#endif

#ifdef F_NEXT_INVENTORY
void
f_next_inventory (void)
{
    object_t *ob;
    
    ob = sp->u.ob->next_inv;
    free_object(&sp->u.ob, "f_next_inventory");

#ifdef F_SET_HIDE
    while (ob && (ob->flags & O_HIDDEN) && !object_visible(ob))
        ob = ob->next_inv;
#endif

    if (ob) {
        add_ref(ob, "next_inventory(ob) : 1");
        sp->u.ob = ob;
    } else
        *sp = const0;
}
#endif
