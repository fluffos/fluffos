#include <stdio.h>
#include <string.h>

#include "config.h"
#include "lint.h"
#include "interpret.h"
#include "object.h"
#include "exec.h"
/*
 * Write statistics about objects on file.
 */

extern struct object *obj_list;

static int svalue_size(v)
    struct svalue *v;
{
    int i, total;

    switch(v->type) {
    case T_OBJECT:
    case T_NUMBER:
      return 0;
    case T_STRING:
      return strlen(v->u.string) + 4; /* Includes some malloc overhead. */
    case T_POINTER:
      for (i=0, total = 0; i < v->u.vec->size; i++) {
	total += svalue_size(&v->u.vec->item[i]) + sizeof (struct svalue);
      }
      return total;
	/* code needs to be written to handle these three cases - 92/02/20 */
	case T_MAPPING:
	case T_MARK:
	case T_ANY:
	break;
    default:
      fatal("Illegal type: %d\n", v->type);
    }
    /*NOTREACHED*/
    return 0;
}

static int data_size(ob)
    struct object *ob;
{
    int total = 0, i;
    if (ob->prog) {
        for (i = 0; i < ob->prog->num_variables; i++)
    	    total += svalue_size(&ob->variables[i]) + sizeof (struct svalue);
    }
    return total;
}

void dumpstat() 
{
    FILE *f;
    struct object *ob;

    f = fopen("OBJ_DUMP", "w");
    if (f == 0)
	return;
    add_message("Dumping to OBJ_DUMP ...");
    for (ob = obj_list; ob; ob = ob->next_all) {
	int tmp;
	if (ob->prog && (ob->prog->ref == 1 || !(ob->flags & O_CLONE)))
	    tmp = ob->prog->total_size;
	else
	    tmp = 0;
	fprintf(f, "%-20s %5d ref %2d %s %s (%ld) %s\n", ob->name,
		tmp + data_size(ob) + sizeof (struct object), ob->ref,
		ob->flags & O_HEART_BEAT ? "HB" : "  ",
		ob->super ? ob->super->name : "--",/*ob->cpu*/ 0,
		ob->swap_num >=0 ? "SWAPPED" : "");
    }
    add_message("done.\n");
    fclose(f);
}
