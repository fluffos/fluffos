/*
 * reclaim.c
 * loops through all variables in all objects looking for the possibility
 * of freeing up destructed objects (that are still hanging around because
 * of references) -- coded by Blackthorn@Genocide Feb. 1993
 */

#include "std.h"
#include "lpc_incl.h"
#include "reclaim.h"

#define MAX_RECURSION 25

static void gc_indices PROT((struct mapping *));
static void gc_values PROT((struct mapping *));
static int gcOne PROT((struct mapping *, struct node *, vinfo_t *));
static void check_svalue PROT((struct svalue *));

static int cleaned, nested;

static void
check_svalue P1(struct svalue *, v)
{
    register int idx;

    nested++;
    if (nested > MAX_RECURSION) {
	return;
    }
    switch (v->type) {
    case T_OBJECT:
	if (v->u.ob->flags & O_DESTRUCTED) {
	    free_svalue(v, "reclaim_objects");
	    *v = const0n;
	    cleaned++;
	}
	break;
    case T_MAPPING:
	gc_values(v->u.map);
	gc_indices(v->u.map);
	break;
    case T_POINTER:
	for (idx = 0; idx < v->u.vec->size; idx++)
	    check_svalue(&v->u.vec->item[idx]);
	break;
    case T_FUNCTION:
#ifdef NEW_FUNCTIONS
	if (v->u.fp->type == FP_CALL_OTHER)
	    check_svalue(&v->u.fp->f.obj);
	check_svalue(&v->u.fp->args);
#else
	check_svalue(&v->u.fp->obj);
	check_svalue(&v->u.fp->fun);
#endif
	break;
    }
    nested--;
    return;
}

static int gcOne P3(struct mapping *, m, struct node *, elt, vinfo_t *, info)
{
    check_svalue(&elt->values[info->w]);
    return 0;
}

static void
gc_indices P1(struct mapping *, m)
{
    vinfo_t info;

    info.w = 0;
    mapTraverse(m, (int (*) ()) gcOne, &info);
}

static void
gc_values P1(struct mapping *, m)
{
    vinfo_t info;

    info.w = 1;
    mapTraverse(m, (int (*) ()) gcOne, &info);
}

int reclaim_objects()
{
    int i;
    struct object *ob;

    cleaned = nested = 0;
    for (ob = obj_list; ob; ob = ob->next_all)
	if (ob->prog)
	    for (i = 0; i < (int) ob->prog->p.i.num_variables; i++)
		check_svalue(&ob->variables[i]);
    return cleaned;
}
