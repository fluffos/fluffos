#ifdef LATTICE
#include "/config.h"
#include "/efuns.h"
#include "/lint.h"
#include "/applies.h"
#else
#include "../config.h"
#include "../efuns.h"
#include "../lint.h"
#include "../applies.h"
#endif

static struct object *ob;
static struct svalue *argp;

#ifdef F_EXPORT_UID
void
f_export_uid P2(int, num_arg, int, instruction)
{
    if (current_object->euid == NULL)
	error("Illegal to export uid 0\n");
    ob = sp->u.ob;
    pop_stack();
    if (ob->euid)
	push_number(0);
    else {
	ob->uid = current_object->euid;
	push_number(1);
    }
}
#endif

#ifdef F_GETEUID
void
f_geteuid P2(int, num_arg, int, instruction)
{
    if (sp->type == T_OBJECT) {
	ob = sp->u.ob;
	if (ob->euid) {
	    char *tmp;
	    
	    tmp = ob->euid->name;
	    pop_stack();
	    push_string(tmp, STRING_CONSTANT);
	    return;
	} else {
	    pop_stack();
	    push_number(0);
	    return;
	}
    } else if (sp->type == T_FUNCTION) {
#ifdef NEW_FUNCTIONS
	if (sp->u.fp->owner && sp->u.fp->owner->euid) {
	    char *tmp;
	    
	    tmp = sp->u.fp->owner->euid->name;
	    pop_stack();
	    push_string(tmp, STRING_CONSTANT);
	    return;
	} else {
	    pop_stack();
	    push_number(0);
	    return;
	}
#else
	if (sp->u.fp->euid) {
	    pop_stack();
	    push_string(sp->u.fp->euid->name, STRING_CONSTANT);
	    return;
	}
#endif
    }
    pop_stack();
    push_number(0);
}
#endif

#ifdef F_GETUID
void
f_getuid P2(int, num_arg, int, instruction)
{
    char *tmp;

    ob = sp->u.ob;
#ifdef DEBUG
    if (ob->uid == NULL)
	fatal("UID is a null pointer\n");
#endif
    tmp = ob->uid->name;
    pop_stack();
    push_string(tmp, STRING_CONSTANT);
}
#endif

#ifdef F_SETEUID
void
f_seteuid P2(int, num_arg, int, instruction)
{
    struct svalue *ret;

    if (sp->type == T_NUMBER) {
	if (sp->u.number != 0)
	    bad_arg(1, instruction);
	current_object->euid = NULL;
	*sp = const1;
	return;
    }
    argp = sp;
    CHECK_TYPES(argp, T_STRING, 1, instruction);
    push_object(current_object);
    push_string(argp->u.string, STRING_CONSTANT);
    ret = apply_master_ob(APPLY_VALID_SETEUID, 2);
    if (!MASTER_APPROVED(ret)) {
	pop_stack();
	push_number(0);
	return;
    }
    current_object->euid = add_uid(argp->u.string);
    free_string_svalue(sp);
    *sp = const1;
}
#endif
