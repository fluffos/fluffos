#include "std.h"
#include "comm.h"
#include "master.h"
#include "efuns_main.h"
#include "otable.h"

object_t *master_ob = 0;

/* Note that now, once the master object loads once, there is ALWAYS a
 * master object, so the only way this can fail is if the master object
 * hasn't loaded yet.  In that case, we return (svalue_t *)-1, and the
 * calling routine should let the check succeed.
 */
svalue_t *apply_master_ob P2(char *, fun, int, num_arg)
{
    IF_DEBUG(svalue_t *expected_sp);

    if (!master_ob) {
	pop_n_elems(num_arg);
	return (svalue_t *)-1;
    }
    call_origin = ORIGIN_DRIVER;

#ifdef TRACE
    if (TRACEP(TRACE_APPLY)) {
	do_trace("Apply", "", "\n");
    }
#endif

    IF_DEBUG(expected_sp = sp - num_arg);
    if (apply_low(fun, master_ob, num_arg) == 0)
	return 0;
    free_svalue(&apply_ret_value, "sapply");
    apply_ret_value = *sp--;
    DEBUG_CHECK(expected_sp != sp,
		"Corrupt stack pointer.\n");
    return &apply_ret_value;
}

svalue_t *safe_apply_master_ob P2(char *, fun, int, num_arg)
{
    if (!master_ob) {
	pop_n_elems(num_arg);
	return (svalue_t *)-1;
    }
    return safe_apply(fun, master_ob, num_arg, ORIGIN_DRIVER);
}

void init_master() {
    char buf[512];
#ifdef LPC_TO_C
    lpc_object_t *compiled_version;
#endif
    object_t *new_ob;
    
    if (!strip_name(MASTER_FILE, buf, sizeof buf))
	error("Illegal master file name '%s'\n", MASTER_FILE);
    
#ifdef LPC_TO_C
    compiled_version = (lpc_object_t *)lookup_object_hash(buf);
#endif

    new_ob = load_object(buf, compiled_version);
    if (new_ob == 0) {
	fprintf(stderr, "The master file %s was not loaded.\n",
		MASTER_FILE);
	exit(-1);
    }
    set_master(new_ob);
}

void set_master P1(object_t *, ob) {
#if defined(PACKAGE_UIDS) || defined(PACKAGE_MUDLIB_STATS)
    int first_load = (!master_ob);
#endif
#ifdef PACKAGE_UIDS
    svalue_t *ret;
#endif

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
		      APPLY_GET_ROOT_UID);
	exit(-1);
    }
    if (ret->type != T_STRING) {
        debug_message("%s() in master object does not work.\n",
		      APPLY_GET_ROOT_UID);
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
			  APPLY_GET_BACKBONE_UID);
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
