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

static void gc_indices PROT((mapping_t *));
static void gc_values PROT((mapping_t *));
static int gcOne PROT((mapping_t *, mapping_node_t *, vinfo_t *));
static void check_svalue PROT((svalue_t *));

static int cleaned, nested;

static void
check_svalue P1(svalue_t *, v)
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
    case T_ARRAY:
	for (idx = 0; idx < v->u.arr->size; idx++)
	    check_svalue(&v->u.arr->item[idx]);
	break;
    case T_FUNCTION:
	{
	    svalue_t tmp;
	    tmp.type = T_ARRAY;
	    tmp.u.arr = v->u.fp->hdr.args;
	    check_svalue(&tmp);
	    break;
	}
    }
    nested--;
    return;
}

static int gcOne P3(mapping_t *, m, mapping_node_t *, elt, vinfo_t *, info)
{
    check_svalue(&elt->values[info->w]);
    return 0;
}

static void
gc_indices P1(mapping_t *, m)
{
    vinfo_t info;

    info.w = 0;
    mapTraverse(m, (int (*) ()) gcOne, &info);
}

static void
gc_values P1(mapping_t *, m)
{
    vinfo_t info;

    info.w = 1;
    mapTraverse(m, (int (*) ()) gcOne, &info);
}

int reclaim_objects()
{
    int i;
    object_t *ob;

    cleaned = nested = 0;
    for (ob = obj_list; ob; ob = ob->next_all)
	if (ob->prog)
	    for (i = 0; i < (int) ob->prog->num_variables; i++)
		check_svalue(&ob->variables[i]);
    return cleaned;
}
