#ifdef LATTICE
#include "/lpc_incl.h"
#include "/eoperators.h"
#else
#include "../lpc_incl.h"
#include "../eoperators.h"
#endif

static struct object *ob;
static struct svalue *argp;

#ifdef F_EXPORT_UID
void
f_export_uid PROT((void))
{
    if (current_object->euid == NULL)
        error("Illegal to export uid 0\n");
    ob = sp->u.ob;
    if (ob->euid){
        free_object(ob, "f_export_uid:1");
        *sp = const0;
    }
    else {
        ob->uid = current_object->euid;
        free_object(ob, "f_export_uid:2");
        *sp = const1;
    }
}
#endif

#ifdef F_GETEUID
void
f_geteuid PROT((void))
{
    if (sp->type & T_OBJECT) {
        ob = sp->u.ob;
        if (ob->euid) {
            put_constant_string(ob->euid->name);
            free_object(ob, "f_geteuid:1");
            return;
	} else {
            free_object(ob, "f_geteuid:2");
            *sp = const0;
            return;
	}
    } else if (sp->type & T_FUNCTION) {
        struct funp *fp;
#ifdef NEW_FUNCTIONS
        if ((fp = sp->u.fp)->owner && fp->owner->euid) {
            put_constant_string(fp->owner->euid->name);
            free_funp(fp);
            return;
	} 
#else
        if ((fp = sp->u.fp)->euid) {
            put_constant_string(fp->euid->name);
            free_funp(fp);
            return;
	}
#endif
	free_funp(fp);
	*sp = const0;
    }
}
#endif

#ifdef F_GETUID
void
f_getuid PROT((void))
{
    ob = sp->u.ob;
#ifdef DEBUG
    if (ob->uid == NULL)
        fatal("UID is a null pointer\n");
#endif
    put_constant_string(ob->uid->name);
    free_object(ob, "f_getuid");
}
#endif

#ifdef F_SETEUID
void
f_seteuid PROT((void))
{
    struct svalue *ret;
    char *tmp;

    if (sp->type & T_NUMBER) {
        if (sp->u.number)
            bad_arg(1, F_SETEUID);
        current_object->euid = NULL;
	sp->u.number = 1;
        return;
    }
    tmp = sp->u.string;
    push_object(current_object);
    push_constant_string(tmp);
    ret = apply_master_ob(APPLY_VALID_SETEUID, 2);
    if (!MASTER_APPROVED(ret)) {
        free_string_svalue(sp);
        *sp = const0;
        return;
    }
    current_object->euid = add_uid(tmp);
    free_string_svalue(sp);
    *sp = const1;
}
#endif

