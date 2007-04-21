#include "std.h"
#include "comm.h"
#include "master.h"
#include "efuns_main.h"
#include "otable.h"

object_t *master_ob = 0;
function_lookup_info_t *master_applies = 0;

/* Note that now, once the master object loads once, there is ALWAYS a
 * master object, so the only way this can fail is if the master object
 * hasn't loaded yet.  In that case, we return (svalue_t *)-1, and the
 * calling routine should let the check succeed.
 */
svalue_t *apply_master_ob (int fun, int num_arg)
{
    if (!master_ob) {
        pop_n_elems(num_arg);
        return (svalue_t *)-1;
    }

    if (master_applies[fun].func) {
#ifdef TRACE
        if (TRACEP(TRACE_APPLY)) {
            do_trace("master apply", master_applies[fun].func->funcname, "\n");
        }
#endif

        call_direct(master_ob, master_applies[fun].index,
                    ORIGIN_DRIVER, num_arg);
        free_svalue(&apply_ret_value, "apply_master_ob");
        apply_ret_value = *sp--;
        return &apply_ret_value;
    } else {
        pop_n_elems(num_arg);
        return 0;
    }
}

/* Hmm, need something like a safe_call_direct() to do this one */
svalue_t *safe_apply_master_ob (int fun, int num_arg)
{
    if (!master_ob) {
        pop_n_elems(num_arg);
        return (svalue_t *)-1;
    }
    return safe_apply(applies_table[fun], master_ob, num_arg, ORIGIN_DRIVER);
}

void init_master() {
    char buf[512];
    object_t *new_ob;

    if (!strip_name(MASTER_FILE, buf, sizeof buf))
        error("Illegal master file name '%s'\n", MASTER_FILE);
    
    new_ob = load_object(buf, compiled_version);
    if (new_ob == 0) {
        fprintf(stderr, "The master file %s was not loaded.\n",
                MASTER_FILE);
        exit(-1);
    }
    set_master(new_ob);
}

static void get_master_applies (object_t * ob) {
    int i;
    
    /* master_applies will be allocated if we're recompiling master_ob */
    if (master_applies)
        FREE(master_applies);
    master_applies = CALLOCATE(NUM_MASTER_APPLIES, function_lookup_info_t,
                               TAG_SIMULS, "get_master_applies");

    for (i = 0; i < NUM_MASTER_APPLIES; i++) {
        char *name = applies_table[i];
        int ind, ri;
        
        if (find_function_by_name(ob, name, &ind, &ri)) {
            master_applies[i].func = find_func_entry(ob->prog, ri);
            master_applies[i].index = ri;
        } else {
            master_applies[i].func = 0;
        }
    }
}

void set_master (object_t * ob) {
#if defined(PACKAGE_UIDS) || defined(PACKAGE_MUDLIB_STATS)
    int first_load = (!master_ob);
#endif
#ifdef PACKAGE_UIDS
    svalue_t *ret;
#endif

    get_master_applies(ob);
    master_ob = ob;
    /* Make sure master_ob is never made a dangling pointer. */
    add_ref(master_ob, "set_master");
#ifndef PACKAGE_UIDS
#  ifdef PACKAGE_MUDLIB_STATS
    if (first_load) {
        set_backbone_domain("BACKBONE");
        set_master_author("NONAME");
    }
#  endif
#else
    ret = apply_master_ob(APPLY_GET_ROOT_UID, 0);
    /* can't be -1 or we wouldn't be here */
    if (!ret) {
        debug_message("No function %s() in master object; possibly the mudlib doesn't want PACKAGE_UIDS to be defined.\n",
                      applies_table[APPLY_GET_ROOT_UID]);
        exit(-1);
    }
    if (ret->type != T_STRING) {
        debug_message("%s() in master object does not work.\n",
                      applies_table[APPLY_GET_ROOT_UID]);
        exit(-1);
    }
    if (first_load) {
        master_ob->uid = set_root_uid(ret->u.string);
        master_ob->euid = master_ob->uid;
#  ifdef PACKAGE_MUDLIB_STATS
        set_master_author(ret->u.string);
#  endif
        ret = apply_master_ob(APPLY_GET_BACKBONE_UID, 0);
        if (ret == 0 || ret->type != T_STRING) {
            debug_message("%s() in the master file does not work\n",
                          applies_table[APPLY_GET_BACKBONE_UID]);
            exit(-1);
        }
        set_backbone_uid(ret->u.string);
#  ifdef PACKAGE_MUDLIB_STATS
        set_backbone_domain(ret->u.string);
#  endif
    } else {
        master_ob->uid = add_uid(ret->u.string);
        master_ob->euid = master_ob->uid;
    }
#endif
}
