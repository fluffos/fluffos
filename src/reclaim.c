/*
 * reclaim.c
 * loops through all variables in all objects looking for the possibility
 * of freeing up destructed objects (that are still hanging around because
 * of references) -- coded by Blackthorn@Genocide Feb. 1993
 */
 
#include <stdio.h>
#include "config.h"
#include "lint.h"
#include "interpret.h"
#include "mapping.h"
#include "object.h"
#include "exec.h"
 
#define MAX_RECURSION 25

void gc_indices PROT((struct mapping *));
void gc_values PROT((struct mapping *));

extern struct svalue const0n;
 
static int cleaned, nested;
 
void
check_svalue(v)
struct svalue *v;
{
  register int idx;

  nested++;
  if (nested > MAX_RECURSION)
  {
    return;
  }
  switch (v->type)
  {
    case T_OBJECT:
      if (v->u.ob->flags & O_DESTRUCTED)
      {
        free_svalue(v);
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
      check_svalue(&v->u.fp->obj);
      check_svalue(&v->u.fp->fun);
      break;
  }
  nested--;
  return;
}
 
int gcOne(m, elt, info)
struct mapping *m;
struct node *elt;
vinfo_t *info;
{
  check_svalue(&elt->values[info->w]);
  return 0;
}
 
void
gc_indices(m)
struct mapping *m;
{
  vinfo_t info;
  info.w = 0;
  mapTraverse(m, (int (*)())gcOne, &info);
}
 
void
gc_values(m)
struct mapping *m;
{
  vinfo_t info;
  info.w = 1;
  mapTraverse(m, (int (*)())gcOne, &info);
}
 
int
reclaim_objects()
{
  int i;
  struct object *ob;

  cleaned = nested = 0;
  for (ob = obj_list; ob; ob = ob->next_all)
    if (ob->prog)
      for (i = 0; i < (int)ob->prog->p.i.num_variables; i++)
        check_svalue(&ob->variables[i]);
  return cleaned;
}
