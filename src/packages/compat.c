#ifdef LATTICE
#include "/lpc_incl.h"
#else
#include "../lpc_incl.h"
#endif

#ifdef F_CAT
void
f_cat(num_arg, instruction)
int num_arg, instruction;
{
  struct svalue *arg;
  int i, start = 0, len = 0;

  arg = sp - num_arg + 1;
  if (num_arg > 1)
    start = arg[1].u.number;
  if (num_arg == 3)
    {	
      if (arg[2].type != T_NUMBER)
	bad_arg(2, instruction);
      len = arg[2].u.number;
    }
  i = print_file(arg[0].u.string, start, len);
  pop_n_elems(num_arg);
  push_number(i);
}
#endif

#ifdef F_LOG_FILE
void
f_log_file(num_arg, instruction)
int num_arg, instruction;
{
	if (IS_ZERO(sp)) {
		bad_arg(2, instruction);
		pop_stack();
	} else {
		log_file((sp-1)->u.string, sp->u.string);
		pop_stack();
	}
}
#endif

#ifdef F_EXTRACT
void
f_extract(num_arg, instruction)
int num_arg, instruction;
{
  int len, from, to;
  struct svalue *arg;
  char *res;

  arg = sp - num_arg + 1;
  len = SVALUE_STRLEN(arg);
  if (num_arg == 1)
    return;
  from = arg[1].u.number;
  if (from < 0)
    from = len + from;
  if (from >= len)
    {
      pop_n_elems(num_arg);
      push_string("", STRING_CONSTANT);
      return;
    }
  if (num_arg == 2)
    {
      res = string_copy(arg->u.string + from);
      pop_2_elems();
      push_malloced_string(res);
      return;
    }
  if (arg[2].type != T_NUMBER)
    error("Bad third argument to extract\n");
  to = arg[2].u.number;
  if (to < 0)
    to = len + to;
  if (to < from)
    {
      pop_3_elems();
      push_string("", STRING_CONSTANT);
      return;
    }
  if (to >= len)
    to = len - 1;
  if (to == len -1)
    {
      res = string_copy(arg->u.string + from);
      pop_3_elems();
      push_malloced_string(res);
      return;
    }
  res = DXALLOC(to - from + 2, 30, "f_extract: res");
  strncpy(res, arg[0].u.string + from, to - from + 1);
  res[to - from + 1] = '\0';
  pop_3_elems();
  push_malloced_string(res);
}
#endif

#ifdef F_NEXT_LIVING
void
f_next_living(num_arg, instruction)
int num_arg, instruction;
{
  ob = sp->u.ob->next_hashed_living;
  pop_stack();
  if (!ob)
    push_number(0);
  else
    push_object(ob);
}
#endif

#ifdef F_FIRST_INVENTORY
void
f_first_inventory(num_arg, instruction)
int num_arg, instruction;
{
  ob = first_inventory(sp);
  pop_stack();
  if (ob)
    push_object(ob);
  else
    push_number(0);
}
#endif

#ifdef F_NEXT_INVENTORY
void
f_next_inventory(num_arg, instruction)
int num_arg, instruction;
{
  ob = sp->u.ob;
  pop_stack();
  ob = ob->next_inv;
  while (ob)
  {
    if (ob->flags & O_HIDDEN)
    {
      if (object_visible(ob))
      {
        push_object(ob);
        return;
      }
    } else
    {
      push_object(ob);
      return;
    }
    ob = ob->next_inv;
  }
    push_number(0);
}
#endif

