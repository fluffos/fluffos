#ifdef LATTICE
#include "/lpc_incl.h"
#else
#include "../lpc_incl.h"
#endif

#ifdef F_CAT
void
f_cat PROT((void))
{
    svalue_t *arg;
    int i, start = 0, len = 0;

    arg = sp - st_num_arg + 1;
    if (st_num_arg > 1){
	start = arg[1].u.number;
	if (st_num_arg == 3)
	{	
	    if (sp->type != T_NUMBER)
		bad_arg(2, F_CAT);
	    len = (sp--)->u.number;
	}
	sp--;
    }
    i = print_file(arg[0].u.string, start, len);
    free_string_svalue(sp);
    put_number(i);
}
#endif

#ifdef F_LOG_FILE
void
f_log_file PROT((void))
{
    log_file((sp-1)->u.string, sp->u.string);
    free_string_svalue(sp--);
    free_string_svalue(sp--);
}
#endif

#ifdef F_EXTRACT
void
f_extract PROT((void))
{
  int len, from, to;
  svalue_t *arg;
  char *res;

  arg = sp - st_num_arg + 1;
  len = SVALUE_STRLEN(arg);
  if (st_num_arg == 1)
    return;
  from = arg[1].u.number;
  if (from < 0)
    from = len + from;
  if (from >= len)
    {
      pop_n_elems(st_num_arg);
      push_string("", STRING_CONSTANT);
      return;
    }
  if (st_num_arg == 2)
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
f_next_living PROT((void))
{
    ob = sp->u.ob->next_hashed_living;
    free_object(sp->u.ob, "f_next_living");
    if (!ob) *sp = const0;
    else {
      add_ref(ob, "next_living(ob)");
      sp->u.ob = ob;
    }
}
#endif


