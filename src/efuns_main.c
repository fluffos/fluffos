/*
	efunctions.c: this file contains the efunctions called from
	inside eval_instruction() in interpret.c.  Note: if you are adding
    local efunctions that are specific to your driver, you would be better
    off adding them to a separate source file.  Doing so will make it much
    easier for you to upgrade (won't have to patch this file).  Be sure
    to #include "efuns.h" in that separate source file.
*/

#include "efuns.h"
#include "stralloc.h"
#if defined(__386BSD__) || defined(SunOS_5)
#include <unistd.h>
#endif

static struct svalue *argp;
static struct object *ob;

int using_bsd_malloc = 0;
int using_smalloc = 0;
int was_call_other = 0;

int data_size PROT((struct object *ob));
void reload_object PROT((struct object *obj));

#ifdef F_ADD_ACTION
void
f_add_action(num_arg, instruction)
int num_arg, instruction;
{
  struct svalue *arg;
  arg = sp - num_arg + 1;
  if (num_arg == 3)
    {	if (arg[2].type != T_NUMBER)
	  bad_arg(3, instruction);
	}
  add_action(arg[0].u.string,
	     num_arg > 1 ? arg[1].u.string : 0,
	     num_arg > 2 ? arg[2].u.number : 0);
  pop_n_elems(num_arg-1);
}
#endif

/*
  OBSOLETE
	I'm not sure this function should exist anymore, outside of
	being a simul_efun that tells you to use add_action...
	--Sulam
*/

#ifdef F_ADD_VERB
void
f_add_verb(num_arg, instruction)
int num_arg, instruction;
{
  add_verb(sp->u.string, 0);
  return;
}
#endif

#ifdef F_ADD_WORTH
void
f_add_worth(num_arg, instruction)
int num_arg, instruction;
{
  struct object *ob;
  int worth;

  if (num_arg == 2)
    {
      ob = sp->u.ob;
      worth = (sp-1)->u.number;
      pop_stack();
    }
  else
    {
      if (!previous_ob)
	return;
      ob = previous_ob;
      worth = sp->u.number;
    }
  add_worth (&ob->stats, worth);
}
#endif

/*
  OBSOLETE
	If we get rid of add_verb, we should also get rid of this...
	--Sulam
*/

#ifdef F_ADD_XVERB
void
f_add_xverb(num_arg, instruction)
int num_arg, instruction;
{
  add_verb(sp->u.string, 1);
}
#endif

#ifdef F_ALL_INVENTORY
void
f_all_inventory(num_arg, instruction)
int num_arg, instruction;
{
  struct vector *vec;

  vec = all_inventory(sp->u.ob, 0);
  pop_stack();
  if (!vec)
    {
      push_number(0);
      return;
    }
  push_vector(vec);		/* This will make ref count == 2 */
  vec->ref--;
}
#endif

#ifdef F_ALLOCATE
void
f_allocate(num_arg, instruction)
int num_arg, instruction;
{
  struct vector *vec;

  vec = allocate_array(sp->u.number);
  pop_stack();
  push_vector(vec);
  vec->ref--;
}
#endif

#ifdef F_ALLOCATE_MAPPING
void
f_allocate_mapping(num_arg, instruction)
int num_arg, instruction;
{
  struct mapping *map;

  map = allocate_mapping(sp->u.number);
  pop_stack();
  push_mapping(map);
  map->ref--;
}
#endif

#ifdef F_CACHE_STATS

void print_cache_stats ()
{
   add_message ("Function cache information\n");
   add_message ("-------------------------------\n");
   add_message("%% cache hits:    %10.2f\n",
	       100 * ((double)apply_low_cache_hits / apply_low_call_others));
   add_message("call_others:     %10lu\n", apply_low_call_others);
   add_message("cache hits:      %10lu\n", apply_low_cache_hits);
   add_message("cache size:      %10lu\n", APPLY_CACHE_SIZE);
   add_message("slots used:      %10lu\n", apply_low_slots_used);
   add_message("%% slots used:    %10.2f\n",
	       100 * ((double)apply_low_slots_used / APPLY_CACHE_SIZE));
   add_message("collisions:      %10lu\n", apply_low_collisions);
   add_message("%% collisions:    %10.2f\n",
	       100 * ((double)apply_low_collisions / apply_low_call_others));
}

void f_cache_stats(num_arg, instruction)
int num_arg, instruction;
{
   print_cache_stats();
   push_number(0);
}
#endif

#ifdef F_CALL_OTHER
    /* enhanced call_other written 930314 by Luke Mewburn <zak@rmit.edu.au> */
void
f_call_other(num_arg, instruction)
int num_arg, instruction;
{
    struct svalue *arg, tmp;
    char *funcname;
	int i;

    if (current_object->flags & O_DESTRUCTED)
    {				/* No external calls allowed */
	pop_n_elems(num_arg);
	push_undefined();
	return;
    }
    arg = sp - num_arg + 1;
    if (arg[1].type == T_STRING)
	funcname = arg[1].u.string;
    else		/* must be T_POINTER then */
    {
	check_for_destr(arg[1].u.vec);
	if ( (arg[1].u.vec->size < 1)
	  || (arg[1].u.vec->item[0].type != T_STRING) )
	    error("call_other: 1st elem of array for arg 2 must be a string\n");
	funcname = arg[1].u.vec->item[0].u.string;	/* complicated huh? */
	for (i = 1; i < arg[1].u.vec->size; i++)
	    push_svalue(&arg[1].u.vec->item[i]);
	num_arg += i-1;	/* hopefully that will work */
    }
    if (funcname[0] == ':')
	error("Illegal function name in call_other: %s\n", arg[1].u.string);
    if (arg[0].type == T_OBJECT)
	ob = arg[0].u.ob;
    else if (arg[0].type == T_POINTER)
    {
        struct vector *ret;
        extern struct vector *call_all_other PROT((struct vector *, char *,int));


	ret = call_all_other(arg[0].u.vec, funcname, num_arg-2);
	pop_n_elems(2);
        push_vector(ret);
        sp->u.vec->ref--;
	return;
    }
    else
    {
	ob = find_object(arg[0].u.string);
	if (!ob || !object_visible(ob))
	    error("call_other() couldn't find object\n");
    }
	    /* Send the remaining arguments to the function. */
#ifdef TRACE
    if (TRACEP(TRACE_CALL_OTHER))
    {
	do_trace("Call other ", funcname, "\n");
    }
#endif
	was_call_other = 1;
    if (apply_low(funcname, ob, num_arg-2) == 0)
    {				/* Function not found */
	pop_n_elems(2);
	push_undefined();
	return;
    }
    /*
    The result of the function call is on the stack.  So is the
    function name and object that was called, though.
    These have to be removed.
    */
    tmp = *sp--;		/* Copy the function call result */
    pop_n_elems(2);		/* Remove old arguments to call_other */
    *++sp = tmp;		/* Re-insert function result */
    return;
}
#endif

#ifdef F_CALL_OUT
void
f_call_out(num_arg, instruction)
int num_arg, instruction;
{
  struct svalue *arg;

  arg = sp - num_arg + 1;
  if (!(current_object->flags & O_DESTRUCTED))
    new_call_out(current_object, arg[0].u.string, arg[1].u.number,
		 num_arg - 3, (num_arg >= 3) ? &arg[2] : 0);
  pop_n_elems(num_arg);
  push_number(0);
}
#endif

#ifdef F_CALL_OUT_INFO
void
f_call_out_info(num_arg, instruction)
int num_arg, instruction;
{
  push_vector(get_all_call_outs());
  sp->u.vec->ref--;		/* set ref count to 1 */
}
#endif

#ifdef F_CAPITALIZE
void
f_capitalize(num_arg, instruction)
int num_arg, instruction;
{
  if (islower(sp->u.string[0]))
    {
      char *str;
		
      str = string_copy(sp->u.string);
      str[0] += 'A' - 'a';
      pop_stack();
      push_malloced_string(str);
    }
  return;
}
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

#ifdef F_CHILDREN
void
f_children(num_arg, instruction)
int num_arg, instruction;
{
  struct vector *vec;

  vec = children(sp->u.string);
  pop_stack();
  if (!vec)
    push_number(0);
  else
    {
      push_vector(vec);
      vec->ref--;		/* reset ref count */
    }
}
#endif

#ifdef F_CLEAR_BIT
void
f_clear_bit(num_arg, instruction)
int num_arg, instruction;
{
  char *str;
  int len, ind;

  if (sp->u.number > MAX_BITS)
    error("clear_bit: %d > MAX_BITS\n", sp->u.number);
  len = SVALUE_STRLEN(sp-1);
  ind = sp->u.number / 6;
  if (ind >= len)
    {				/* return first arg unmodified */
      pop_stack();
      return;
    }
  str = DXALLOC(len+1, 29, "f_clear_bit: str");
  memcpy(str, (sp-1)->u.string, len+1);	/* including null byte */
  if (str[ind] > 0x3f + ' ' || str[ind] < ' ')
    error("Illegal bit pattern in clear_bit character %d\n", ind);
  str[ind] = ((str[ind] - ' ') & ~(1 << (sp->u.number % 6))) + ' ';
  pop_n_elems(2);
  push_malloced_string(str);
}
#endif

#ifdef F_CLONEP
void
f_clonep(num_arg, instruction)
int num_arg, instruction;
{
  if ((sp->type == T_OBJECT) && (sp->u.ob->flags & O_CLONE))
    assign_svalue(sp, &const1);
  else
    assign_svalue(sp, &const0);
}
#endif

#ifdef F_COMMAND
void
f_command(num_arg, instruction)
int num_arg, instruction;
{
  struct svalue *arg;
  int i;

  arg = sp - num_arg + 1;
#ifdef OLD_COMMAND
  i = command_for_object(arg[0].u.string, (num_arg == 2) ? arg[1].u.ob : 0);
#else
  if ((num_arg == 2) && (arg[1].u.ob != current_object)) {
     error("Use command(cmd) or command(cmd, this_object()).\n");
  }
  i = command_for_object(arg[0].u.string, 0);
#endif
  pop_n_elems(num_arg);
  push_number(i);
}
#endif

#ifdef F_COMMANDS
void
f_commands(num_arg, instruction)
int num_arg, instruction;
{
  struct vector *vec;

  vec = commands(current_object);
  push_vector(vec);
  vec->ref--;			/* reset ref count to 1 */
}
#endif

/*
  OBSOLETE
	Couldn't this be a simul_efun??  Do we even need it as a
	function at all??
	--Sulam

   problem with making it a simul_efun is that we are limited to copying
   files of size less than read_file is limited to
*/

#ifdef F_CP
void
f_cp(num_arg, instruction)
int num_arg, instruction;
{
  int i;

  i = copy_file(sp[-1].u.string, sp[0].u.string);
  pop_n_elems(2);
  push_number(i);
}
#endif

#ifdef F_CREATOR
void
f_creator(num_arg, instruction)
int num_arg, instruction;
{
  ob = sp->u.ob;
  if (ob->uid == 0) {
    assign_svalue(sp, &const0);
  } else {
    pop_stack();
    push_string(ob->uid->name, STRING_CONSTANT);
  }
}
#endif /* CREATOR */

#ifdef F_CTIME
void
f_ctime(num_arg, instruction)
int num_arg, instruction;
{
  char *cp;

  cp = string_copy(time_string(sp->u.number));
  pop_stack();
  push_malloced_string(cp);
  /* Now strip the newline. */
  cp = strchr(cp, '\n');
  if (cp)
    *cp = '\0';
}
#endif

#ifdef F_DEBUG_INFO
void
f_debug_info(num_arg, instruction)
int num_arg, instruction;
{
  struct svalue *arg, res;

  arg = sp - 1;
  switch (arg[0].u.number)
    {
    case 0:
      {
	int i, flags;
	struct object *obj2;

	ob = arg[1].u.ob;
	flags = ob->flags;
	add_message("O_HEART_BEAT      : %s\n",
		    flags & O_HEART_BEAT      ?"TRUE":"FALSE");
	add_message("O_IS_WIZARD       : %s\n",
		    flags & O_IS_WIZARD       ?"TRUE":"FALSE");
	add_message("O_ENABLE_COMMANDS : %s\n",
		    flags & O_ENABLE_COMMANDS ?"TRUE":"FALSE");
	add_message("O_CLONE           : %s\n",
		    flags & O_CLONE           ?"TRUE":"FALSE");
	add_message("O_DESTRUCTED      : %s\n",
		    flags & O_DESTRUCTED      ?"TRUE":"FALSE");
	add_message("O_SWAPPED         : %s\n",
		    flags & O_SWAPPED          ?"TRUE":"FALSE");
	add_message("O_ONCE_INTERACTIVE: %s\n",
		    flags & O_ONCE_INTERACTIVE?"TRUE":"FALSE");
	add_message("O_RESET_STATE     : %s\n",
		    flags & O_RESET_STATE     ?"TRUE":"FALSE");
	add_message("O_WILL_CLEAN_UP   : %s\n",
		    flags & O_WILL_CLEAN_UP   ?"TRUE":"FALSE");
	add_message("O_WILL_RESET: %s\n",
		    flags & O_WILL_RESET ?"TRUE":"FALSE");
	add_message("total light : %d\n", ob->total_light);
	add_message("next_reset  : %d\n", ob->next_reset);
	add_message("time_of_ref : %d\n", ob->time_of_ref);
	add_message("ref         : %d\n", ob->ref);
#ifdef DEBUG
	add_message("extra_ref   : %d\n", ob->extra_ref);
#endif
	add_message("swap_num    : %d\n", ob->swap_num);
	add_message("name        : '%s'\n", ob->name);
	add_message("next_all    : OBJ(%s)\n",
		    ob->next_all ? ob->next_all->name : "NULL");
	if (obj_list == ob)
	  add_message("This object is the head of the object list.\n");
	for (obj2 = obj_list, i=1; obj2; obj2 = obj2->next_all, i++)
	  if (obj2->next_all == ob)
	    {
	      add_message("Previous object in object list: OBJ(%s)\n",
			  obj2->name);
	      add_message("position in object list:%d\n",i);
	    }
	assign_svalue_no_free(&res,&const0);
	break;
      }
    case 1:
      ob = arg[1].u.ob;
      if (ob->flags & O_SWAPPED) {
         add_message("Swapped\n");
         break;
      }
      add_message("program ref's %d\n", ob->prog->p.i.ref);
      add_message("Name %s\n", ob->prog->name);
      add_message("program size %d\n",
		  ob->prog->p.i.program_size);
      add_message("num func's %d (%d) \n", ob->prog->p.i.num_functions,
		  ob->prog->p.i.num_functions * sizeof(struct function));
      add_message("num strings %d\n", ob->prog->p.i.num_strings);
      add_message("num vars %d (%d)\n", ob->prog->p.i.num_variables,
		  ob->prog->p.i.num_variables * sizeof(struct variable));
      add_message("num inherits %d (%d)\n", ob->prog->p.i.num_inherited,
		  ob->prog->p.i.num_inherited * sizeof(struct inherit));
      add_message("total size %d\n", ob->prog->p.i.total_size);
      assign_svalue_no_free(&res,&const0);
      break;
    default:
      bad_arg(1,instruction);
    }
  pop_n_elems(2);
  sp++;
  *sp = res;
}
#endif

#if (defined(DEBUGMALLOC) && defined(DEBUGMALLOC_EXTENSIONS))
#ifdef F_DEBUGMALLOC
void
f_debugmalloc(num_arg, instruction)
int num_arg, instruction;
{
	dump_debugmalloc((sp - 1)->u.string, sp->u.number);
	pop_n_elems(2);
	push_number(0);
}
#endif

#ifdef F_SET_MALLOC_MASK
void
f_set_malloc_mask(num_arg, instruction)
int num_arg, instruction;
{
	set_malloc_mask(sp->u.number);
	pop_stack();
	push_number(0);
}
#endif
#endif /* (defined(DEBUGMALLOC) && defined(DEBUGMALLOC_EXTENSIONS)) */

#ifdef F_DEEP_INHERIT_LIST
void
f_deep_inherit_list(num_arg, instruction)
int num_arg, instruction;
{
  struct vector *vec;
  extern struct vector *deep_inherit_list PROT((struct object *));

  if (!(sp->u.ob->flags & O_SWAPPED)) {
     vec = deep_inherit_list(sp->u.ob);
  } else {
     vec = null_array();
  }
  pop_stack();
  push_vector(vec);
  vec->ref--;			/* reset ref count */
}
#endif

#ifdef DEBUG_MACRO

#ifdef F_SET_DEBUG_LEVEL
void
f_set_debug_level(num_arg, instruction)
int num_arg, instruction;
{
	debug_level = sp->u.number;
	pop_stack();
	push_number(0);
}
#endif

#endif

#ifdef F_DEEP_INVENTORY
void
f_deep_inventory(num_arg, instruction)
int num_arg, instruction;
{
  struct vector *vec;

  vec = deep_inventory(sp->u.ob, 0);
  pop_stack();
  push_vector(vec);
  vec->ref--;			/* reset ref count */
}
#endif

#ifdef F_DESTRUCT
void
f_destruct(num_arg, instruction)
int num_arg, instruction;
{
	struct object *ob;

	if(sp->type == T_OBJECT) {
		ob = sp->u.ob;
	} else {
		error("Invalid argument to destruct()\n");
	}
	destruct_object(sp);
	pop_stack();
	push_number(1);
}
#endif

#ifdef F_DISABLE_COMMANDS
void
f_disable_commands(num_arg, instruction)
int num_arg, instruction;
{
  enable_commands(0);
  push_number(0);
}
#endif

#ifdef F_DUMPALLOBJ
void
f_dumpallobj(num_arg, instruction)
int num_arg, instruction;
{
	char *fn = "/OBJ_DUMP";

	if (num_arg == 1) {
		fn = sp->u.string;
	}
	dumpstat(fn);
	if (num_arg == 1) {
		pop_stack();
	}
	push_number(0);
}
#endif

/* f_each */

#ifdef F_EACH
void
f_each(num_arg, instruction)
int num_arg, instruction;
{
	struct mapping *m;
	struct vector *v;
	int flag;

	m = (sp - 1)->u.map;
	flag = sp->u.number;
	if (flag) {
		m->eachObj = current_object;
		m->bucket = 0;
		m->elt = (struct node *)0;
		pop_n_elems(2);
		push_number(0);
		return;
	}
	v = mapping_each(m);
	pop_n_elems(2);
	push_vector(v);
	v->ref--;
}
#endif

#ifdef F_ED
void
f_ed(num_arg, instruction)
int num_arg, instruction;
{
	if (num_arg == 0) {
		if (command_giver == 0 || command_giver->interactive == 0)
			push_number(0);
		else {
			ed_start(0, 0, 0, 0);
			push_number(1);
		}
	} else if (num_arg == 1)
		ed_start(sp->u.string, 0, 0, 0);
	else if (num_arg == 2) {
		if (sp->type == T_STRING)
			ed_start((sp-1)->u.string, sp->u.string, 0, current_object);
		else
			ed_start((sp-1)->u.string, 0, 0, 0);
		pop_stack();
	} else { /* num_arg == 3 */
		if (((sp-1)->type == T_STRING) && (sp->type == T_NUMBER)) {
			ed_start((sp-2)->u.string, (sp-1)->u.string, sp->u.number,
				current_object);
		} else {
			ed_start((sp-1)->u.string, 0, 0, 0);
		}
		pop_n_elems(2);
	}
}
#endif

#ifdef F_ENABLE_COMMANDS
void
f_enable_commands(num_arg, instruction)
int num_arg, instruction;
{
  enable_commands(1);
  push_number(1);
}
#endif

#ifdef F_ENABLE_WIZARD
void
f_enable_wizard(num_arg, instruction)
int num_arg, instruction;
{
  if (current_object->interactive)
    {
      current_object->flags |= O_IS_WIZARD;
      push_number(1);
    }
  else
    push_number(0);
}
#endif

#ifdef F_ERROR
void
f_error(num_arg, instruction)
int num_arg, instruction;
{
	error(sp->u.string);
}
#endif

#ifdef F_ERRORP
void
f_errorp(num_arg, instruction)
int num_arg, instruction;
{
	int i;

	if ((sp->type == T_NUMBER) && (sp->subtype == T_ERROR)) {
		i = 1;
	} else {
		i = 0;
	}
	pop_stack();
	push_number(i);
}
#endif

#ifdef F_DISABLE_WIZARD
void
f_disable_wizard(num_arg, instruction)
int num_arg, instruction;
{
  if (current_object->interactive)
    {
      current_object->flags &= ~O_IS_WIZARD;
      push_number(1);
    }
  else
    push_number(0);
}
#endif

#ifdef F_ENVIRONMENT
void
f_environment(num_arg, instruction)
int num_arg, instruction;
{
  if (num_arg)
    {
      ob = environment(sp);
      pop_stack();
    }
  else if (!(current_object->flags & O_DESTRUCTED))
    ob = current_object->super;
  else
    ob = 0;
  if (ob)
    push_object(ob);
  else
    push_number(0);
}
#endif

#ifdef F_EXEC
void
f_exec(num_arg, instruction)
int num_arg, instruction;
{
  int i;

  i = replace_interactive((sp-1)->u.ob, sp->u.ob);
  pop_stack();
  pop_stack();
  push_number(i);
}
#endif

#ifdef F_EXPLODE
void
f_explode(num_arg, instruction)
int num_arg, instruction;
{
  struct vector *vec;
	
  vec = explode_string((sp-1)->u.string, sp->u.string);
  pop_n_elems(2);
  if (vec)
    {
      push_vector(vec);		/* This will make ref count == 2 */
      vec->ref--;
    }
  else
    push_number(0);
}
#endif

#ifdef F_EXPORT_UID
void
f_export_uid(num_arg, instruction)
int num_arg, instruction;
{
  if (current_object->euid == NULL)
    error("Illegal to export uid 0\n");
  ob = sp->u.ob;
  pop_stack();
  if (ob->euid)
    push_number(0);
  else
    {
      ob->uid = current_object->euid;
      push_number(1);
    }
}
#endif

/*
  OBSOLETE
	Isn't this a simul_efun??  Why do we have it here??
	--Sulam
*/

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
      pop_n_elems(2);
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
      pop_n_elems(3);
      push_string("", STRING_CONSTANT);
      return;
    }
  if (to >= len)
    to = len - 1;
  if (to == len -1)
    {
      res = string_copy(arg->u.string + from);
      pop_n_elems(3);
      push_malloced_string(res);
      return;
    }
  res = DXALLOC(to - from + 2, 30, "f_extract: res");
  strncpy(res, arg[0].u.string + from, to - from + 1);
  res[to - from + 1] = '\0';
  pop_n_elems(3);
  push_malloced_string(res);
}
#endif

#ifdef F_FILE_NAME
void
f_file_name(num_arg, instruction)
int num_arg, instruction;
{
  char *name, *res;

  /* This function now returns a leading '/' */
  name = sp->u.ob->name;
  res = (char *)add_slash(name);
  pop_stack();
  push_malloced_string(res);
}
#endif

#ifdef F_FILE_SIZE
void
f_file_size(num_arg, instruction)
int num_arg, instruction;
{
  int i;

  i = file_size(sp->u.string);
  pop_stack();
  push_number(i);
}
#endif

#ifdef F_FILTER_ARRAY
void
f_filter_array(num_arg, instruction)
int num_arg, instruction;
{
  struct vector *vec;
  struct svalue *arg;

  arg = sp - num_arg + 1; ob = 0;
  if (arg[2].type == T_OBJECT)
    ob = arg[2].u.ob;
  else if (arg[2].type == T_STRING)
   {
     ob = find_object(arg[2].u.string);
     if (ob && !object_visible(ob)) ob = 0;
   }
  if (!ob)
    error("Bad third argument to filter_array()\n");
  if (arg[0].type == T_POINTER)
    {
      check_for_destr(arg[0].u.vec);
      vec = filter(arg[0].u.vec, arg[1].u.string, ob,
		   num_arg > 3 ? sp : (struct svalue *)0);
    }
  else
    {
      vec = 0;
    }
  pop_n_elems(num_arg);
  if (vec)
    {
      push_vector(vec);	/* This will make ref count == 2 */
      vec->ref--;
    }
  else
    {
      push_number(0);
    }
}
#endif

#ifdef F_FIND_CALL_OUT
void
f_find_call_out(num_arg, instruction)
int num_arg, instruction;
{
  int i;

  i = find_call_out(current_object, sp->u.string);
  pop_stack();
  push_number(i);
}
#endif

#ifdef F_FIND_LIVING
void
f_find_living(num_arg, instruction)
int num_arg, instruction;
{
  ob = find_living_object(sp->u.string, 0);
  pop_stack();
  if (!ob)
    push_number(0);
  else
    push_object(ob);
}
#endif

#ifdef F_FIND_OBJECT
void
f_find_object(num_arg, instruction)
int num_arg, instruction;
{
    ob = find_object2(sp->u.string);
    pop_stack();
    if (ob)
    {
        if (object_visible(ob))
            push_object(ob);
          else
            push_number(0);
    }
  else
    push_number(0);
}
#endif

#ifdef F_FIND_PLAYER
void
f_find_player(num_arg, instruction)
int num_arg, instruction;
{
  ob = find_living_object(sp->u.string, 1);
  pop_stack();
  if (!ob)
    push_number(0);
  else
    push_object(ob);
}
#endif

/*
  OBSOLETE
	How much do we need this?  It would be easy to make into
	a simul_efun...
	--Sulam
*/

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

void add_mapping_shared_string(m, key, value)
struct mapping *m;
char *key, *value;
{
	struct svalue *s, lv;

	lv.type = T_STRING;
	lv.subtype = STRING_CONSTANT;
	lv.u.string = key;
	s = find_for_insert(m, &lv, 1);
	s->type = T_STRING;
	s->subtype = STRING_SHARED;
	s->u.string = ref_string(value);
}

#ifdef F_FUNCTION_PROFILE
/* f_function_profile: John Garnett, 1993/05/31, 0.9.17.3 */
void
f_function_profile(num_arg, instruction)
	int num_arg, instruction;
{
	struct vector *vec;
	struct mapping *map;
	struct program *prog;
	int nf, j;

	ob = sp->u.ob;
	if (ob->flags & O_SWAPPED) {
		load_ob_from_swap(ob);
	}
	prog = ob->prog;
	nf = prog->p.i.num_functions;
	vec = allocate_array(nf);
	for (j = 0; j < nf; j++) {
		map = allocate_mapping(3);
		add_mapping_pair(map, "calls", prog->p.i.functions[j].calls);
		add_mapping_pair(map, "self", prog->p.i.functions[j].self
			- prog->p.i.functions[j].children);
		add_mapping_pair(map, "children", prog->p.i.functions[j].children);
		add_mapping_shared_string(map, "name", prog->p.i.functions[j].name);
		vec->item[j].type = T_MAPPING;
		vec->item[j].u.map = map;
	}
	pop_stack();
	push_vector(vec);
	vec->ref--;
}
#endif

#ifdef F_FUNCTION_EXISTS
void
f_function_exists(num_arg, instruction)
int num_arg, instruction;
{
  char *str, *res;

  str = function_exists((sp-1)->u.string, sp->u.ob);
  pop_n_elems(2);
  if (str)
    {
      res = (char *)add_slash(str);
      if ((str = strrchr (res, '.')))
	*str = 0;
      push_malloced_string(res);
    }
  else
    {
      push_number(0);
    }
}
#endif

#ifdef F_GET_CHAR
void
f_get_char(num_arg, instruction)
int num_arg, instruction;
{
  struct svalue *arg;
  int i, flag = 1;

  arg = sp - num_arg + 1;
  if (num_arg == 1 || (sp->type == T_NUMBER && sp->u.number == 0))
    flag = 0;
  i = get_char(arg[0].u.string, flag);
  pop_n_elems(num_arg);
  push_number(i);
}
#endif

#ifdef F_GET_DIR
void
f_get_dir(num_arg, instruction)
int num_arg, instruction;
{
  struct vector *vec;

  vec = get_dir((sp-1)->u.string, sp->u.number);
  pop_n_elems(2);
  if (vec)
    {
      push_vector(vec);
      vec->ref--;		/* resets ref count */
    }
  else
      push_number(0);
}
#endif

#ifdef F_GETEUID
void
f_geteuid(num_arg, instruction)
int num_arg, instruction;
{
  ob = sp->u.ob;
  if (ob->euid)
    {
      char *tmp;
      
      tmp = ob->euid->name;
      pop_stack();
      push_string(tmp, STRING_CONSTANT);
    }
  else
    {
      pop_stack();
      push_number(0);
    }
}
#endif

#ifdef F_GETUID
void
f_getuid(num_arg, instruction)
int num_arg, instruction;
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

#ifdef F_IMPLODE
void
f_implode(num_arg, instruction)
int num_arg, instruction;
{
  char *str;

  check_for_destr((sp-1)->u.vec);
  str = implode_string((sp-1)->u.vec, sp->u.string);
  pop_n_elems(2);
  if (str)
      push_malloced_string(str);
   else
      push_number(0);
}
#endif

#ifdef F_IN_EDIT
void
f_in_edit(num_arg, instruction)
int num_arg, instruction;
{
  int i;

  i = sp->u.ob->interactive && sp->u.ob->interactive->ed_buffer;
  pop_stack();
  if (i)
    push_number(1);
  else
    push_number(0);
}
#endif

#ifdef F_IN_INPUT
void
f_in_input(num_arg, instruction)
int num_arg, instruction;
{
  int i;

  i = sp->u.ob->interactive && sp->u.ob->interactive->input_to;
  pop_stack();
  if (i)
    push_number(1);
  else
    push_number(0);
}
#endif

#ifdef F_INHERITS
int
inherits(prog, thep)
struct program *prog, *thep;
{
	int j;

	for (j = 0; j < (int)prog->p.i.num_inherited; j++) {
		if (prog->p.i.inherit[j].prog == thep)
			return 1;
		if (!strcmp(prog->p.i.inherit[j].prog->name, thep->name))
			return 2;
		if (inherits(prog->p.i.inherit[j].prog, thep))
			return 1;
	}
	return 0;
}

void
f_inherits(num_arg, instruction)
int num_arg, instruction;
{
	struct object *ob, *base;
	int i;

	ob = find_object2((sp - 1)->u.string);
	base = sp->u.ob;
	if (IS_ZERO(sp) || !base || !ob || (ob->flags & O_SWAPPED)) {
		pop_n_elems(2);
		push_number(0);
		return;
	}
	if (base->flags & O_SWAPPED)
		load_ob_from_swap(base);
	i = inherits(base->prog, ob->prog);
	pop_n_elems(2);
	push_number(i);
}
#endif

#ifdef F_INHERIT_LIST
void
f_inherit_list(num_arg, instruction)
int num_arg, instruction;
{
  struct vector *vec;
  extern struct vector *inherit_list PROT((struct object *));


  if (!(sp->u.ob->flags & O_SWAPPED)) {
     vec = inherit_list(sp->u.ob);
  } else {
     vec = null_array();
  }
  pop_stack();
  push_vector(vec);
  vec->ref--;			/* reset ref count */
}
#endif

#ifdef F_INPUT_TO
void
f_input_to(num_arg, instruction)
int num_arg, instruction;
{
  struct svalue *arg;
  int i, flag, tmp;

  arg = sp - num_arg + 1;	/* Points arg at first argument. */
  if ((num_arg < 2) || (arg[1].type != T_NUMBER))
    {
      tmp = 0;
      flag = 0;
    }
  else
    {
      tmp = 1;	 
      num_arg--;		/* Don't count the flag as an arg */
      flag = arg[1].u.number;
    }
  num_arg--;			/* Don't count the name of the func either. */
  i = input_to(arg[0].u.string, flag, num_arg, &arg[1 + tmp]);
  pop_n_elems(num_arg + 1 + tmp);
  push_number(i);
}
#endif

#ifdef F_INTERACTIVE
void
f_interactive(num_arg, instruction)
int num_arg, instruction;
{
  int i;

  i = (int)sp->u.ob->interactive;
  pop_stack();
  push_number(i);
}
#endif

#ifdef F_INTP
void
f_intp(num_arg, instruction)
int num_arg, instruction;
{
  if (sp->type == T_NUMBER)
    assign_svalue(sp, &const1);
  else
    assign_svalue(sp, &const0);
}
#endif

#ifdef F_FUNCTIONP
void
f_functionp(num_arg, instruction)
int num_arg, instruction;
{
	if (sp->type == T_FUNCTION &&
		((sp->u.fp->obj.type == T_OBJECT &&
		!(sp->u.fp->obj.u.ob->flags & O_DESTRUCTED)) ||
		sp->u.fp->obj.type == T_STRING) &&
		sp->u.fp->fun.type == T_STRING) {
			assign_svalue(sp, &const1);
	} else {
		assign_svalue(sp, &const0);
	}
}
#endif

#ifdef F_KEYS
void
f_keys(num_arg, instruction)
int num_arg, instruction;
{
  struct vector *vec;

  vec = mapping_indices(sp->u.map);
  eval_cost += vec->size;
  pop_stack();
  push_vector(vec);
  vec->ref--;			/* resets ref count */
}
#endif

#ifdef F_VALUES
void
f_values(num_arg, instruction)
int num_arg, instruction;
{
  struct vector *vec;

  vec = mapping_values(sp->u.map);
  eval_cost += vec->size;
  pop_stack();
  push_vector(vec);
  vec->ref--;			/* resets ref count */
}
#endif

#ifdef F_LINK
void
f_link(num_arg, instruction)
int num_arg, instruction;
{
  struct svalue *ret;
  int i;

  push_string((sp-1)->u.string, STRING_CONSTANT);
  push_string(sp->u.string, STRING_CONSTANT);
  ret = apply_master_ob("valid_link", 2);
  if (!IS_ZERO(ret))
    i = do_rename((sp-1)->u.string, sp->u.string, F_LINK);
  pop_n_elems(2);
  push_number(i);
}
#endif /* F_LINK */

#ifdef F_LIVING
void
f_living(num_arg, instruction)
int num_arg, instruction;
{
  if (sp->u.ob->flags & O_ENABLE_COMMANDS)
    assign_svalue(sp, &const1);
  else
    assign_svalue(sp, &const0);
}
#endif

#ifdef F_LIVINGS
void
f_livings(num_arg, instruction)
int num_arg, instruction;
{
  push_vector(livings());	/* livings() has already set ref count to 1 */
  sp->u.vec->ref--;
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

#ifdef F_LOWER_CASE
void
f_lower_case(num_arg, instruction)
int num_arg, instruction;
{
  int i;

  char *str = string_copy(sp->u.string);
  for (i = strlen(str)-1; i>=0; i--)
    if (isalpha(str[i]))
      str[i] |= 'a' - 'A';
  pop_stack();
  push_malloced_string(str);
}
#endif

#ifdef F_MALLOC_STATUS
void f_malloc_status(num_arg, instruction)
int num_arg, instruction;
{
#ifdef DO_MSTATS
	void show_mstats PROT((char *));
#endif
#if (defined(WRAPPEDMALLOC) || defined(DEBUGMALLOC))
    void dump_malloc_data();
#endif

	if (using_bsd_malloc) {
		add_message("Using BSD malloc.\n");
	}
	if (using_smalloc) {
		add_message("Using Smalloc.\n");
	}
#ifdef DO_MSTATS
	show_mstats("malloc_status()");
#endif
#if (defined(WRAPPEDMALLOC) || defined(DEBUGMALLOC))
    dump_malloc_data();
#endif
#ifdef SYSMALLOC
	if (!using_bsd_malloc && !using_smalloc) {
		add_message("Using system malloc.\n");
	}
#endif
#ifdef GMALLOC
    add_message("Using Gnu malloc.\n");
#endif
    push_number(0);
}
#endif

#ifdef F_MAP_DELETE
void
f_map_delete(num_arg, instruction)
int num_arg, instruction;
{
  mapping_delete((sp-1)->u.map,sp);
  pop_stack(); /* all functions must leave exactly 1 element on stack */
}
#endif

#ifdef F_MAPP
void
f_mapp(num_arg, instruction)
int num_arg, instruction;
{
	assign_svalue(sp, (sp->type == T_MAPPING) ? &const1 : &const0);
}
#endif

#ifdef F_MAP_ARRAY
void
f_map_array(num_arg, instruction)
int num_arg, instruction;
{
  struct vector *res = 0;
  struct mapping *map = (struct mapping *) 0;
  struct svalue *arg;

  arg = sp - num_arg + 1; ob = 0;

  if (arg[2].type == T_OBJECT)
    ob = arg[2].u.ob;
  else if (arg[2].type == T_STRING) 
   {
     ob = find_object(arg[2].u.string);
     if (ob && !object_visible(ob)) ob = 0;
   }

  if (!ob)
    bad_arg (3, instruction);

  if (arg[0].type == T_POINTER) {
    check_for_destr(arg[0].u.vec);
    res = map_array (arg[0].u.vec, arg[1].u.string, ob,
		     num_arg > 3 ? sp : (struct svalue *)0);
  } else if (arg[0].type == T_MAPPING) {
    map = map_mapping (arg[0].u.map, arg[1].u.string, ob,
		       num_arg > 3 ? sp : (struct svalue *) 0);
  } else {
    res = 0;
  }
  pop_n_elems (num_arg);
  if (map)
  {
    push_mapping(map);
    map->ref--;
  }
  else if (res)
  {
    push_vector (res);		/* This will make ref count == 2 */
    res->ref--;
  }
  else
    push_number (0);
}
#endif

#ifdef F_MASTER
void
f_master(num_arg, instruction)
int num_arg, instruction;
{
  assert_master_ob_loaded("master()");
  push_object(master_ob);
}
#endif

/*
This efun searches a mapping for a path.  Each key is assumed to be a
string.  The value is completely arbitrary.  The efun finds the largest
matching path in the mapping.  Keys ended in '/' are assumed to match
paths with character that follow the '/', i.e. / is a wildcard for anything
below this directory.  DO NOT CHANGE THIS EFUN TIL YOU UNDERSTAND IT.  It
catches folks by suprise at first, but it is coded the way it is for a reason.
It effectively implements the search loop in TMI's access object as a single
efun.
 
        Cygnus
*/
#ifdef F_MATCH_PATH
void
f_match_path(num_arg, instruction)
int num_arg, instruction;
{
  struct svalue *value;
  struct svalue string;
  char *src;
  char *dst;
  struct svalue *nvalue;
 
  value = &const0u;
 
  string.type = T_STRING;
  string.subtype = STRING_MALLOC;
  string.u.string = (char *)MALLOC(strlen(sp->u.string) + 1);
 
  src = sp->u.string;
  dst = string.u.string;
 
  while (*src != '\0') {
    while (*src != '/' && *src != '\0')
      *dst++ = *src++;
    if (*src == '/') {
      while (*++src == '/') ;
        if (*src != '\0' || dst == string.u.string)
          *dst++ = '/';
    }
    *dst = '\0';
    nvalue = find_in_mapping((sp-1)->u.map, &string);
    if (nvalue != &const0u)
      value = nvalue;
  }
 
  FREE(string.u.string);
 
  pop_stack();
  pop_stack();
 
  push_svalue(value);
}
#endif /* F_MATCH_PATH */

#ifdef F_MEMBER_ARRAY
void
f_member_array(num_arg, instruction)
int num_arg, instruction;
{
  struct vector *v;
  struct svalue *find;
  int i, ncmp = 0;

  if ((num_arg > 2) && (sp->type == T_NUMBER) &&
      sp->u.number && ((sp-2)->type == T_STRING))
  {
    ncmp = strlen((sp-2)->u.string);
    v = (sp-1)->u.vec;
    find = (sp-2);
  } else {
    v = sp->u.vec;
    find = (sp-1);
  }
  check_for_destr(v);
  for (i=0; i < v->size; i++)
  {
    if (v->item[i].type != find->type)
      continue;
    switch(find->type)
      {
      case T_STRING:
        if (ncmp) {
          if (strncmp(find->u.string, v->item[i].u.string, ncmp) == 0)
            break;
        } else if (strcmp(find->u.string, v->item[i].u.string) == 0)
	  break;
	continue;
      case T_POINTER:
	if (find->u.vec == v->item[i].u.vec)
	  break;
	continue;
      case T_OBJECT:
	if (find->u.ob == v->item[i].u.ob)
	  break;
	continue;
      case T_NUMBER:
	if (find->u.number == v->item[i].u.number)
	  break;
	continue;
      case T_MAPPING:
	if (find->u.map == v->item[i].u.map)
	  break;
	continue;
      default:
	fatal("Bad type to member_array(): %d\n", (sp-1)->type);
      }
    break;
  }
  if (i == v->size)
    i = -1;			/* Return -1 for failure */
  pop_n_elems(num_arg);
  push_number(i);
}
#endif

#ifdef F_MESSAGE
void
f_message(num_arg, instruction)
int num_arg, instruction;
{
  struct vector *use, *avoid;
  struct svalue *args;
 
  static struct vector vtmp1 = { 1,1,
#ifdef DEBUG
 
                                   1,
#endif
        {(mudlib_stats_t *)NULL, (mudlib_stats_t *)NULL}};
  static struct vector vtmp2 = { 1,1,
#ifdef DEBUG
                                   1,
#endif
       {(mudlib_stats_t *)NULL, (mudlib_stats_t *)NULL}};
 
    args = sp - num_arg + 1;
    switch (args[2].type)
    {
        case T_OBJECT:
                vtmp1.item[0].type = T_OBJECT;
                vtmp1.item[0].u.ob = args[2].u.ob;
                use = &vtmp1;
                break;
        case T_POINTER:
                use = args[2].u.vec;
                break;
        case T_STRING:
                vtmp1.item[0].type = T_STRING;
                vtmp1.item[0].u.string = args[2].u.string;
                use = &vtmp1;
                break;
        case T_NUMBER:
                if (args[2].u.number == 0)
                {
                  /* for compatibility (write() simul_efuns, etc)  -bobf */
                        check_legal_string(args[1].u.string);
                        add_message(args[1].u.string);
                        pop_n_elems(num_arg);
                        push_number(0);
                        return;
                }
        default:
                error("Bad argument 3 to message()\n");
    }
    if (num_arg == 4)
    {
        switch (args[3].type)
        {
                case T_OBJECT:
                       vtmp2.item[0].type = T_OBJECT;
                       vtmp2.item[0].u.ob = args[3].u.ob;
                       avoid = &vtmp2;
                       break;
                case T_POINTER:
                       avoid = args[3].u.vec;
                       break;
                default:
			avoid = null_array();
        }
    }
    else
        avoid = null_array();
    do_message(args[0].u.string, args[1].u.string, use, avoid, 1);
    pop_n_elems(num_arg);
    push_number(0);
    return;
}
#endif

#ifdef F_MKDIR
void
f_mkdir(num_arg, instruction)
int num_arg, instruction;
{
  char *path;

  path = check_valid_path(sp->u.string, current_object, "mkdir", 1);
  if (path == 0 || mkdir(path, 0770) == -1)
    assign_svalue(sp, &const0);
  else
    assign_svalue(sp, &const1);
}
#endif

#ifdef F_MOVE_OBJECT
void
f_move_object(num_arg, instruction)
int num_arg, instruction;
{
  struct object *o1, *o2;

  if (sp->type == T_OBJECT)
    o2 = sp->u.ob;
  else
    {
      o2 = find_object(sp->u.string);
        if (!o2 || !object_visible(o2))
	error("move_object failed\n");
    }
  if (num_arg == 1)
    o1 = current_object;
  else
  {
    o1 = (sp-1)->u.ob;
    pop_stack();
  }
  move_object(o1, o2);
}
#endif

#ifdef F_MUD_STATUS
void f_mud_status(num_arg, instruction)
int num_arg, instruction;
{
  int tot, res, verbose = 0;
  extern char *reserved_area;
  extern int tot_alloc_object, tot_alloc_sentence,
  tot_alloc_object_size, num_mappings, num_arrays, total_array_size,
  total_mapping_size, total_users, total_mapping_nodes;
  extern int total_num_prog_blocks;
  extern int total_prog_block_size;
#ifdef COMM_STAT
  extern int add_message_calls, inet_packets, inet_volume;
#endif

  verbose = sp->u.number;
  pop_stack();
  if (reserved_area)
    res = RESERVED_SIZE;
  else
    res = 0;
#ifdef COMM_STAT
  if (verbose) {
     add_message ("add_message statistics\n");
     add_message ("------------------------------\n");
     add_message("Calls to add_message: %d   Packets: %d   Average packet size: %f\n\n",add_message_calls,inet_packets,(float)inet_volume/inet_packets);
  }
#endif
  if (!verbose) {
    add_message("Sentences:\t\t\t%8d %8d\n", tot_alloc_sentence,
		tot_alloc_sentence * sizeof (struct sentence));
    add_message("Objects:\t\t\t%8d %8d\n",
		tot_alloc_object, tot_alloc_object_size);
    add_message("Prog blocks:\t\t\t%8d %8d\n",
                total_num_prog_blocks, total_prog_block_size);
    add_message("Arrays:\t\t\t\t%8d %8d\n", num_arrays,
		total_array_size);
    add_message("Mappings:\t\t\t%8d %8d\n", num_mappings,
		total_mapping_size);
    add_message("Mappings(nodes):\t\t%8d\n", total_mapping_nodes);
    add_message("Interactives:\t\t\t%8d %8d\n", total_users,
        total_users * sizeof(struct interactive));
  }
  if (verbose) {
     stat_living_objects();
#ifdef F_CACHE_STATS
     print_cache_stats();
#endif
     print_swap_stats();
  }
  tot = total_prog_block_size +
    total_array_size +
      total_mapping_size +
	tot_alloc_object_size +
	  show_otable_status(verbose) +
	    heart_beat_status(verbose) +
	      add_string_status(verbose) +
		print_call_out_usage(verbose) +
		  res;

  if (!verbose) {
    add_message("\t\t\t\t\t --------\n");
    add_message("Total:\t\t\t\t\t %8d\n", tot);
  } else {
    ;
  }
     
  push_number(0);
}
#endif

#if defined(F_NEW)
void
f_new(num_arg, instruction)
int num_arg, instruction;
{
  struct object *ob;

  ob = clone_object(sp->u.string);
  pop_stack();
  if (ob)
  {
    sp++;
    sp->type = T_OBJECT;
    sp->u.ob = ob;
    add_ref(ob, "F_NEW");
  }
  else
    push_number(0);
}
#endif

/*
   OBSOLETE
   	Is this neccessary either? It could be a simul_efun. -SH
*/
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

#ifdef F_NOTIFY_FAIL
void
f_notify_fail(num_arg, instruction)
int num_arg, instruction;
{
  set_notify_fail_message(sp->u.string);
  pop_stack();
  push_number(0);
  /* Return the argument */
}
#endif

#ifdef F_NULLP
void
f_nullp(num_arg, instruction)
int num_arg, instruction;
{
  if (IS_NULL(sp))
    assign_svalue(sp, &const1);
  else
    assign_svalue(sp, &const0);
}
#endif

#ifdef F_OBJECTP
void
f_objectp(num_arg, instruction)
int num_arg, instruction;
{
  if (sp->type == T_OBJECT)
    assign_svalue(sp, &const1);
  else
    assign_svalue(sp, &const0);
}
#endif

#ifdef F_OPCPROF

void
f_opcprof(num_arg, instruction)
int num_arg, instruction;
{
	char *fn = "/OPCPROF";

	if (num_arg == 1) {
		fn = sp->u.string;
	}
	opcdump(fn);
	if (num_arg == 1) {
		pop_stack();
	}
	push_number(0);
}
#endif

#ifdef F_ORIGIN
void
f_origin(num_arg, instruction)
int num_arg, instruction;
{
	push_number((int)caller_type);
}
#endif

#ifdef F_POINTERP
void
f_pointerp(num_arg, instruction)
int num_arg, instruction;
{
  if (sp->type == T_POINTER)
    assign_svalue(sp, &const1);
  else
    assign_svalue(sp, &const0);
}
#endif

#ifdef F_PRESENT
void
f_present(num_arg, instruction)
int num_arg, instruction;
{
  struct svalue *arg = sp - num_arg + 1;

#ifdef LAZY_RESETS
  if (num_arg == 2) {
     try_reset(arg[1].u.ob);
  }
#endif
  ob = object_present(arg, num_arg == 1 ? 0 : arg[1].u.ob);
  pop_n_elems(num_arg);
  if (ob)
    push_object(ob);
  else
    push_number(0);
}
#endif

#ifdef F_PREVIOUS_OBJECT
void
f_previous_object(num_arg, instruction)
int num_arg, instruction;
{
  if (previous_ob == 0 || (previous_ob->flags & O_DESTRUCTED))
    push_number(0);
  else
    push_object(previous_ob);
}
#endif

#ifdef F_PRINTF
void
f_printf(num_arg, instruction)
int num_arg, instruction;
{
	if (command_giver)
		tell_object(command_giver, string_print_formatted((sp-num_arg+1)->u.string,
		num_arg-1, sp-num_arg+2));
	pop_n_elems(num_arg-1);
}
#endif

#ifdef F_PROCESS_STRING
void
f_process_string(num_arg, instruction)
int num_arg, instruction;
{
  extern char
    *process_string PROT((char *));

  char *str;

  str = process_string(sp->u.string);
  if (str != sp->u.string)
    {
      pop_stack();
      push_malloced_string(str);
    }
}
#endif

#ifdef F_QUERY_HOST_NAME
void
f_query_host_name(num_arg, instruction)
int num_arg, instruction;
{
  extern char *query_host_name();
  char *tmp;

  tmp = query_host_name();
  if (tmp)
    push_string(tmp, STRING_CONSTANT);
  else
    push_number(0);
}
#endif

#ifdef F_QUERY_IDLE
void
f_query_idle(num_arg, instruction)
int num_arg, instruction;
{
  int i;

  i = query_idle(sp->u.ob);
  pop_stack();
  push_number(i);
}
#endif

#ifdef F_QUERY_IP_NAME
void
f_query_ip_name(num_arg, instruction)
int num_arg, instruction;
{
  extern char *query_ip_name PROT((struct object *));
  char *tmp;

  if (num_arg == 1 && sp->type != T_OBJECT)
    error("Bad optional argument to query_ip_name()\n");
  tmp = query_ip_name(num_arg ? sp->u.ob : 0);
  if (num_arg)
    pop_stack();
  if (tmp == 0)
    push_number(0);
  else
    push_string(tmp, STRING_MALLOC);
}
#endif

#ifdef F_QUERY_IP_NUMBER
void
f_query_ip_number(num_arg, instruction)
int num_arg, instruction;
{
  extern char *query_ip_number PROT((struct object *));
  char *tmp;

  if (num_arg == 1 && sp->type != T_OBJECT)
    error("Bad optional argument to query_ip_number()\n");
  tmp = query_ip_number(num_arg ? sp->u.ob : 0);
  if (num_arg)
    pop_stack();
  if (tmp == 0)
    push_number(0);
  else
    push_string(tmp, STRING_MALLOC);
}
#endif

#ifdef F_QUERY_LOAD_AVERAGE
void
f_query_load_average(num_arg, instruction)
int num_arg, instruction;
{
  push_string(query_load_av(), STRING_MALLOC);
}
#endif

#ifdef F_QUERY_PRIVS
void
f_query_privs(num_arg, instruction)
    int num_arg, instruction;
{
    ob = sp->u.ob;
    if (ob->privs != NULL) {
	pop_stack();
	push_string(ob->privs, STRING_SHARED);
    } else {
	pop_stack();
	push_number(0);
    }
}
#endif

#ifdef F_QUERY_SNOOPING
void
f_query_snooping(num_arg, instruction)
int num_arg, instruction;
{
  struct object *ob;
  ob = query_snooping(sp->u.ob);
  pop_stack();
  if (ob)
    push_object(ob);
  else
    push_number(0);
}
#endif

#ifdef F_QUERY_SNOOP
void
f_query_snoop(num_arg, instruction)
int num_arg, instruction;
{
  struct object *ob;
  ob = query_snoop(sp->u.ob);
  pop_stack();
  if (ob)
    push_object(ob);
  else
    push_number(0);
}
#endif

#ifdef F_QUERY_VERB
void
f_query_verb(num_arg, instruction)
int num_arg, instruction;
{
  if (last_verb == 0)
    {
      push_number(0);
      return;
    }
  push_string(last_verb, STRING_SHARED);
}
#endif

#ifdef F_RANDOM
void
f_random(num_arg, instruction)
int num_arg, instruction;
{
  if (sp->u.number <= 0)
    {
      sp->u.number = 0;
      return;
    }
  sp->u.number = random_number(sp->u.number);
}
#endif

#ifdef F_READ_BYTES
void
f_read_bytes(num_arg, instruction)
int num_arg, instruction;
{
  char *str;
  struct svalue *arg = sp- num_arg + 1;
  int start = 0, len = 0;

  if (num_arg > 1)
    start = arg[1].u.number;
  if (num_arg == 3)
    {
      if (arg[2].type != T_NUMBER)
	bad_arg(2, instruction);
      len = arg[2].u.number;
    }
  str = read_bytes(arg[0].u.string, start, len);
  pop_n_elems(num_arg);
  if (str == 0)
    push_number(0);
  else
    {
      push_string(str, STRING_MALLOC);
      FREE(str);
    }
}
#endif

#ifdef F_READ_FILE
void
f_read_file(num_arg, instruction)
int num_arg, instruction;
{
  char *str;
  struct svalue *arg = sp- num_arg + 1;
  int start = 0, len = 0;

  if (num_arg > 1)
    start = arg[1].u.number;
  if (num_arg == 3)
    {
      if (arg[2].type != T_NUMBER)
	bad_arg(2, instruction);
      len = arg[2].u.number;
    }

  str = read_file(arg[0].u.string, start, len);
  pop_n_elems(num_arg);
  if (str == 0)
    push_number(0);
  else
    {
      push_string(str, STRING_MALLOC);
      FREE(str);
    }
}
#endif

#ifdef F_RECEIVE
void
f_receive(num_arg, instruction)
int num_arg, instruction;
{
	if (current_object->interactive) {
		struct object *save_command_giver = command_giver;

		check_legal_string(sp->u.string);
		command_giver = current_object;
		add_message("%s", sp->u.string);
		command_giver = save_command_giver;
		assign_svalue(sp, &const1);
	} else {
		assign_svalue(sp, &const0);
	}
}
#endif

#ifdef F_REFS
void
f_refs(num_arg, instruction)
int num_arg, instruction;
{
	int r;

	switch (sp->type) {
	case T_MAPPING :
		r = sp->u.map->ref;
		break;
	case T_POINTER :
		r = sp->u.vec->ref;
		break;
	case T_OBJECT :
		r = sp->u.ob->ref;
		break;
	case T_FUNCTION :
		r = sp->u.fp->ref;
	default :
		r = 0;
		break;
	}
	pop_stack();
	push_number(r - 1);/* minus 1 to compensate for being arg of refs() */
}
#endif

#ifdef F_REGEXP
void
f_regexp(num_arg, instruction)
int num_arg, instruction;
{
  struct vector *v;

  v = match_regexp((sp-1)->u.vec, sp->u.string);
  pop_n_elems(2);
  if (v == 0)
    push_number(0);
  else
    {
      push_vector(v);
      v->ref--;			/* Will make ref count == 1 */
    }
}
#endif

#ifdef F_REMOVE_ACTION
void
f_remove_action(num_arg, instruction)
int num_arg, instruction;
{
  int success;
  success = remove_action((sp-1)->u.string, sp->u.string);
  pop_n_elems(2);
  push_number(success);
}
#endif

#ifdef F_REMOVE_CALL_OUT
void
f_remove_call_out(num_arg, instruction)
int num_arg, instruction;
{
  int i;

  i = remove_call_out(current_object, sp->u.string);
  pop_stack();
  push_number(i);
}
#endif

#ifdef F_RENAME
void
f_rename(num_arg, instruction)
int num_arg, instruction;
{
  int i;

  i = do_rename((sp-1)->u.string, sp->u.string, F_RENAME);
  pop_n_elems(2);
  push_number(i);
}
#endif /* F_RENAME */

#ifdef F_REPLACE_STRING

/*
syntax for replace_string is now:
    string replace_string(src, pat, rep);   // or
    string replace_string(src, pat, rep, max);  // or
    string replace_string(src, pat, rep, first, last);

The 4th/5th args are optional (to retain backward compatibility).
- src, pat, and rep are all strings.
- max is an integer. It will replace all occurances up to max
  matches (starting as 1 as the first), with a value of 0 meaning
  'replace all')
- first and last are just a range to replace between, with
  the following constraints
    first < 1: change all from start
    last == 0 || last > max matches:    change all to end
    first > last: return unmodified array.
(i.e, with 4 args, it's like calling it with:
    replace_string(src, pat, rep, 0, max);
)
*/

void
f_replace_string(num_arg, instruction)
    int num_arg, instruction;
{
    int plen, rlen, dlen, first, last, cur;
    char *src, *pattern, *replace, *dst1, *dst2;

    if (num_arg > 5) {
	error("Too many args to replace_string.\n");
	pop_n_elems(num_arg);
	return;
    }
    if ((sp-num_arg+3)->type != T_STRING) {
    	bad_arg(3, instruction);  /* first and second args checked elsewhere */
    	pop_n_elems(num_arg);
    	return;
    }
    src = (sp-num_arg+1)->u.string;
    pattern = (sp-num_arg+2)->u.string;
    replace = (sp-num_arg+3)->u.string;
    first = 0;
    last = 0;

    if (num_arg >=4) {
	if ((sp-num_arg+4)->type != T_NUMBER) {
	    bad_arg(4, instruction); 
	    pop_n_elems(num_arg);
	    return;
	}
	first = (sp-num_arg+4)->u.number;
	if (num_arg == 4) {
	    last = first;
	    first = 0;
	}
    }
    if (num_arg == 5) {
	if ((sp-num_arg+5)->type != T_NUMBER) {
	    bad_arg(5, instruction); 
	    pop_n_elems(num_arg);
	    return;
	}
	    /* first set above. */
	last = (sp-num_arg+5)->u.number;
    }
    if (!last)
	last = max_string_length;
    if (first > last) {		/* just return it */
	push_string(src, STRING_CONSTANT);
	pop_n_elems(num_arg);
	return;
    }
    dst2 = dst1 = (char *)DMALLOC(max_string_length, 31, "f_replace_string");

    plen = strlen(pattern);
    rlen = strlen(replace);
    dlen = 0;
    cur = 0;

    if (plen == 0) {
	strcpy(dst2, src);
    } else {
	while (*src != '\0') {
	    if (strncmp(src, pattern, plen) == 0) {
		cur++;
		if (cur >= first && cur <= last) {
		    if (rlen != 0) {
			if (max_string_length - dlen <= rlen) {
			    pop_n_elems(num_arg);
			    push_svalue(&const0u);
			    FREE(dst1);
			    return;
			}
			strncpy(dst2, replace, rlen);
			dst2 += rlen;
			dlen += rlen;
		    }
		    src += plen;
		    continue;
		}
	    }
	    if (max_string_length - dlen <= 1) {
		pop_n_elems(num_arg);
		push_svalue(&const0u);
		FREE(dst1);
		return;
	    }
	    *dst2++ = *src++;
	    dlen++;
	}
	*dst2 = '\0';
    }
    pop_n_elems(num_arg);
    push_string(dst1, STRING_MALLOC);
    FREE(dst1);
}
#endif

#ifdef F_RESOLVE
void
f_resolve(num_arg, instruction)
int num_arg, instruction;
{
  int i, query_addr_number PROT((char *, char *));

  i = query_addr_number((sp-1)->u.string, sp->u.string);
  pop_n_elems(2);
  push_number(i);
} /* f_resolve() */
#endif

#ifdef F_RESTORE_OBJECT
void
f_restore_object(num_arg, instruction)
int num_arg, instruction;
{
	int i, flag;
	struct svalue *arg = sp - num_arg + 1;

      flag = (num_arg == 1) ? 0 : arg[1].u.number;
      i = restore_object(current_object, arg[0].u.string, flag);
	pop_n_elems(num_arg);
	push_number(i);
}
#endif

#ifdef F_RM
void
f_rm(num_arg, instruction)
int num_arg, instruction;
{
  int i;

  i = remove_file(sp->u.string);
  pop_stack();
  push_number(i);
}
#endif

#ifdef F_RMDIR
void
f_rmdir(num_arg, instruction)
int num_arg, instruction;
{
  char *path;

  path = check_valid_path(sp->u.string, current_object, "rmdir", 1);
  if (path == 0 || rmdir(path) == -1)
    assign_svalue(sp, &const0);
  else
    assign_svalue(sp, &const1);
}
#endif

void add_mapping_pair(m, key, value)
struct mapping *m;
char *key;
int value;
{
	struct svalue *s, lv;

	lv.type = T_STRING;
	lv.subtype = STRING_CONSTANT;
	lv.u.string = key;
	s = find_for_insert(m, &lv, 1);
	s->type = T_NUMBER;
	s->subtype = 0;
	s->u.number = value;
}

#ifdef F_SAVE_OBJECT
void
f_save_object(num_arg, instruction)
int num_arg, instruction;
{
	int flag, i;
	struct svalue *arg = sp - num_arg + 1;

	flag = (num_arg == 1) ? 0 : arg[1].u.number;
	i = save_object(current_object, arg[0].u.string, flag);
	pop_n_elems(num_arg);
	push_number(i);
}
#endif

#ifdef F_SAY
void
f_say(num_arg, instruction)
int num_arg, instruction;
{
  struct vector *avoid;
  static struct vector vtmp = { 1,1,
#ifdef DEBUG
                                   1,
#endif
        {(mudlib_stats_t *)NULL, (mudlib_stats_t *)NULL}};
 
  if (num_arg == 1)
  {
        avoid = null_array();
        say(sp, avoid);
  } else {
        if (sp->type == T_OBJECT)
        {
                vtmp.item[0].type = T_OBJECT;
                vtmp.item[0].u.ob = sp->u.ob;
                avoid = &vtmp;
        } else    /* must be a vector... */
        {
                avoid = sp->u.vec;
        }
	say(sp - 1, avoid);
	pop_stack();
  }
}
#endif

#ifdef F_SET_EVAL_LIMIT
/* warning: do not enable this without using valid_override() in the master
   object and a set_eval_limit() simul_efun to restrict access.
*/
void
f_set_eval_limit(num_arg, instruction)
int num_arg, instruction;
{
	extern int max_cost;

	max_cost = sp->u.number;
}
#endif

#ifdef F_SET_BIT
void
f_set_bit(num_arg, instruction)
int num_arg, instruction;
{
  char *str;
  int len, old_len, ind;

  if (sp->u.number > MAX_BITS)
    error("set_bit: too big bit number: %d\n", sp->u.number);
  if (sp->u.number < 0)
    error("set_bit: illegal (negative) arg 2\n");
  len = SVALUE_STRLEN(sp-1);
  old_len = len;
  ind = sp->u.number/6;
  if (ind >= len)
    len = ind + 1;
  str = DXALLOC(len+1, 32, "f_set_bit");
  str[len] = '\0';
  if (old_len)
    memcpy(str, (sp-1)->u.string, old_len);
  if (len > old_len)
    memset(str + old_len, ' ', len - old_len);
  if (str[ind] > 0x3f + ' ' || str[ind] < ' ')
    error("Illegal bit pattern in set_bit character %d\n", ind);
  str[ind] = ((str[ind] - ' ') | (1 << (sp->u.number % 6))) + ' ';
  pop_n_elems(2);
  sp++;
  sp->u.string = str;
  sp->subtype = STRING_MALLOC;
  sp->type = T_STRING;
}
#endif

#ifdef F_SET_HEART_BEAT
void
f_set_heart_beat(num_arg, instruction)
int num_arg, instruction;
{
  int i;

  i = set_heart_beat(current_object, sp->u.number);
  sp->u.number = i;
}
#endif

#ifdef F_QUERY_HEART_BEAT
void
f_query_heart_beat(num_arg, instruction)
int num_arg, instruction;
{
	push_number(query_heart_beat(current_object));
}
#endif

#ifdef F_SET_HIDE
void
f_set_hide(num_arg, instruction)
int num_arg, instruction;
{
    if (!valid_hide(current_object))
	    return;
	if (sp->u.number) {
		if (!(current_object->flags & O_HIDDEN) && current_object->interactive)
			num_hidden++;
		current_object->flags |= O_HIDDEN;
	}
	else {
		if ((current_object->flags & O_HIDDEN) && current_object->interactive)
			num_hidden--;
		current_object->flags &= ~O_HIDDEN;
	}
}
#endif

#ifdef F_SET_LIGHT
void
f_set_light(num_arg, instruction)
int num_arg, instruction;
{
  struct object *o1;

  add_light(current_object, sp->u.number);
  o1 = current_object;
  while(o1->super)
    o1 = o1->super;
  sp->u.number = o1->total_light;
}
#endif

#ifdef F_SET_LIVING_NAME
void
f_set_living_name(num_arg, instruction)
int num_arg, instruction;
{
  set_living_name(current_object, sp->u.string);
}
#endif

#ifdef F_SET_PRIVS
void
f_set_privs(num_arg, instruction)
    int num_arg, instruction;
{
    struct object *ob;

    ob = (sp-1)->u.ob;
    if (ob->privs != NULL)
	free_string(ob->privs);
    if (sp->type != T_STRING)
	ob->privs = NULL;
    else
	ob->privs = make_shared_string(sp->u.string);
    pop_stack();
    pop_stack();
    push_number(0);
}
#endif

#ifdef F_SETEUID
void
f_seteuid(num_arg, instruction)
int num_arg, instruction;
{
  struct svalue *ret;

  if (sp->type == T_NUMBER) {
    if (sp->u.number != 0)
      bad_arg(1, instruction);
    current_object->euid = NULL;
    pop_stack();
    push_number(1);
    return;
  }
  argp = sp;
  if (argp->type != T_STRING)
    bad_arg(1, instruction);
  push_object(current_object);
  push_string(argp->u.string, STRING_CONSTANT);
  ret = apply_master_ob("valid_seteuid", 2);
  if (ret == 0 || ret->type != T_NUMBER || ret->u.number != 1) {
    pop_stack();
    push_number(0);
    return;
  }
  current_object->euid = add_uid(argp->u.string);
  pop_stack();
  push_number(1);
}
#endif

#ifdef F_SETUID
void
f_setuid(num_arg, instruction)
int num_arg, instruction;
     struct svalue *sp;
     int num_arg;
     int instruction;
{
  setuid();
  push_number(0);
}
#endif /* F_SETUID */

#ifdef F_SHADOW
void
f_shadow(num_arg, instruction)
int num_arg, instruction;
{
  struct object *ob;

  ob = (sp-1)->u.ob;
  if (sp->u.number == 0) {
    ob = ob->shadowed;
    pop_n_elems(2);
    if (ob)
      push_object(ob);
    else
      push_number(0);
    return;
  }
  if (validate_shadowing(ob))
  {
     if (current_object->flags & O_DESTRUCTED) {
         pop_n_elems(2);
         push_number(0);
         return;
     }
    /*
     * The shadow is entered first in the chain.
     */
    while (ob->shadowed)
      ob = ob->shadowed;
    current_object->shadowing = ob;
    ob->shadowed = current_object;
    pop_n_elems(2);
    push_object(ob);
    return;
  }
  pop_n_elems(2);
  push_number(0);
}
#endif

#ifdef F_SHOUT
void
f_shout(num_arg, instruction)
int num_arg, instruction;
{
  shout_string(sp->u.string);
}
#endif

#ifdef F_SHUTDOWN
void
f_shutdown(num_arg, instruction)
int num_arg, instruction;
{
	int exit_code = 0;

	if (num_arg) {
		exit_code = sp->u.number;
	}
	shutdownMudOS(exit_code);
	if (!num_arg) {
		push_number(0);
	}
}
#endif

#ifdef F_SIZEOF
void
f_sizeof(num_arg, instruction)
int num_arg, instruction;
{
  int i;

  if (sp->type == T_MAPPING)
    i = sp->u.map->count;
  else if (sp->type == T_POINTER)
    i = sp->u.vec->size;
  else
    i = 0;
  free_svalue(sp);
  sp->type = T_NUMBER;
  sp->u.number = i;
}
#endif

#ifdef F_SNOOP
void
f_snoop(num_arg, instruction)
int num_arg, instruction;
{
  /* This one takes a variable number of arguments. It returns
   * 0 or an object.
   */
  if (!command_giver)
    {
      pop_n_elems(num_arg);
      push_number(0);
    }
  else
    {
      ob = 0;	       /* Do not remove this, it is not 0 by default */
      switch (num_arg)
	{
	case 1:
	  if (new_set_snoop(sp->u.ob, 0))
	    ob = sp->u.ob;
	  break;
	case 2:
	  if (new_set_snoop((sp-1)->u.ob, sp->u.ob))
	    ob = sp->u.ob;
	  break;
	default:
	  ob = 0;
	  break;
	}
      pop_n_elems(num_arg);
      if (ob)
	push_object(ob);
      else
	push_number(0);
    }
}
#endif

#ifdef F_SORT_ARRAY
void
f_sort_array(num_arg, instruction)
int num_arg, instruction;
{
  extern struct vector *sort_array
    PROT((struct vector*,char *,struct object *));
  struct vector *res;
  struct svalue *arg;

  arg = sp - 2; ob = 0;

  if (arg[2].type == T_OBJECT)
    ob = arg[2].u.ob;
  else if (arg[2].type == T_STRING) 
  {
    ob = find_object(arg[2].u.string);
    if (ob && !object_visible(ob)) ob = 0;
  }

  if (!ob)
    bad_arg (3, instruction);

  if (arg[0].type == T_POINTER)
    {
      struct vector *tmp;

      tmp = slice_array(arg[0].u.vec, 0, arg[0].u.vec->size-1);
      check_for_destr(tmp);
      res = sort_array(tmp, arg[1].u.string, ob);
    }
    else
      res = 0;
  pop_n_elems (3);
  sp++;
  if (res)
    {
      sp->type = T_POINTER;
      sp->u.vec = res;
    }
  else
    *sp = const0;
}
#endif

#ifdef F_SPRINTF
void
f_sprintf(num_arg, instruction)
int num_arg, instruction;
{
  char *s;

  /*
   * string_print_formatted() returns a pointer to it's internal
   * buffer, or to an internal constant...  Either way, it must
   * be copied before it's returned as a string.
   */

  s = string_print_formatted((sp-num_arg+1)->u.string,
			     num_arg-1, sp-num_arg+2);
  pop_n_elems(num_arg);
  if (!s)
    push_number(0);
  else
    push_malloced_string(string_copy(s));
}
#endif

#ifdef F_STAT
void
f_stat(num_arg, instruction)
int num_arg, instruction;
{
	struct stat buf;
	char *path;
	struct vector *v;
      
	path = (sp-1)->u.string;
	if (*path == '/')
		path++;
	if (stat(path,&buf) != -1) {
		if (buf.st_mode & S_IFREG) { /* if a regular file */
			v = allocate_array(3);
			v->item[0].type = T_NUMBER;
			v->item[0].u.number = buf.st_size;
			v->item[1].type = T_NUMBER;
			v->item[1].u.number = buf.st_mtime;
			v->item[2].type = T_NUMBER;
			ob = find_object2(path);
                if (ob && !object_visible(ob)) ob = 0;
			if (ob)
				v->item[2].u.number = ob->load_time;
			else
				v->item[2].u.number = 0;
			pop_n_elems(2);
			push_vector(v);
			v->ref--;      /* Will now be 1. */
			return;
		}
	}
	v = get_dir((sp-1)->u.string, sp->u.number);
	pop_n_elems(2);
	if (v) {
		push_vector(v);
		v->ref--;			/* Will now be 1. */
	} else {
		push_number(0);
	}
}
#endif

#ifdef F_STRSRCH
/*
 * int strsrch(string big, string little, [ int flag ])
 * - search for little in big, starting at right if flag is set
 *   return int offset of little, -1 if not found
 *
 * Written 930706 by Luke Mewburn <zak@rmit.edu.au>
 */

void
f_strsrch(num_arg, instruction)
    int num_arg, instruction;
{
    register char	*big, *little, *pos;
    static char		buf[2];		/* should be initialized to 0 */
    int i;

    if (sp->type != T_NUMBER)
	bad_arg(3, instruction);

    big = (sp-2)->u.string;
    if ((sp-1)->type == T_NUMBER) {
	buf[0] = (char) (sp-1)->u.number;
	little = buf;
    } else
	little = (sp-1)->u.string;

		/* little == ""  or  len(big) < len(little) */
    if (!little[0] || strlen(big) < strlen(little)) {
	pos = NULL;

		/* start at left */
    } else if (!sp->u.number) {
	if (!little[1])			/* 1 char srch pattern */
	    pos = strchr(big, (int) little[0]);
	else
	    pos = _strstr(big, little);

		/* start at right */
    } else {				/* XXX: maybe test for -1 */
	if (!little[1])			/* 1 char srch pattern */
	    pos = strrchr(big, (int) little[0]);
	else {
	    pos = strrchr(big, '\0');	/* find end */
	    pos -= strlen(little) - 1;	/* find rightmost pos it _can_ be */
	    while (1) {
		while (pos-- > big)
		    if (pos[0] == little[0])
			break;
		if (pos[0] != little[0]) {
		    pos = NULL;
		    break;
		}
		for (i = 1; little[i] && (pos[i] == little[i]); i++)
		    ;		/* scan all chars */
		if (!little[i])
		    break;
	    }
	}
    }

    if (!pos)
	i = -1;
    else
	i = (int) (pos - big);
    pop_n_elems(3);
    push_number(i);
} /* strsrch */
#endif

#ifdef F_STRCMP
void
f_strcmp(num_arg, instruction)
int num_arg, instruction;
{
	int i;

	i = strcmp((sp - 1)->u.string, sp->u.string);
	pop_n_elems(2);
	push_number(i);
}
#endif

#ifdef F_STRINGP
void
f_stringp(num_arg, instruction)
int num_arg, instruction;
{
  if (sp->type == T_STRING)
    assign_svalue(sp, &const1);
  else
    assign_svalue(sp, &const0);
}
#endif

#ifdef F_STRLEN
void
f_strlen(num_arg, instruction)
int num_arg, instruction;
{
	int i;

	i = SVALUE_STRLEN(sp);
	pop_stack();
	push_number(i);
}
#endif

#ifdef F_SWAP
void
f_swap(num_arg, instruction)
int num_arg, instruction;
{
  (void)swap(sp->u.ob);
}
#endif

#ifdef F_TAIL
void
f_tail(num_arg, instruction)
int num_arg, instruction;
{
  if (tail(sp->u.string))
    assign_svalue(sp, &const1);
  else
    assign_svalue(sp, &const0);
}
#endif

#ifdef F_TELL_OBJECT
void
f_tell_object(num_arg, instruction)
int num_arg, instruction;
{
  tell_object((sp-1)->u.ob, sp->u.string);
  pop_stack();			/* Return first argument */
}
#endif

#ifdef F_TELL_ROOM
void
f_tell_room(num_arg, instruction)
int num_arg, instruction;
{
  struct svalue *arg = sp - num_arg + 1;
  struct vector *avoid;
 
  if (arg[0].type == T_OBJECT)
  {
    ob = arg[0].u.ob;
  }
    else   /* must be a string... */
  {
    ob = find_object(arg[0].u.string);
    if (!ob || !object_visible(ob))
      error("Bad argument 1 to tell_room()\n");
  }
 
  if (num_arg == 2)
  {
    avoid = null_array();
  }
  else
  {
    if (arg[2].type != T_POINTER)
      bad_arg(3, instruction);
      else
      avoid = arg[2].u.vec;
  }
 
  tell_room(ob, &arg[1], avoid);
  pop_n_elems(num_arg);
  push_number(0);
}
#endif

#ifdef F_TEST_BIT
void
f_test_bit(num_arg, instruction)
int num_arg, instruction;
{
  int len;

  len = SVALUE_STRLEN(sp-1);
  if (sp->u.number/6 >= len)
  {
    pop_n_elems(2);
    push_number(0);
    return;
  }
  if (((sp-1)->u.string[sp->u.number/6] - ' ') & (1 << (sp->u.number % 6)))
  {
    pop_n_elems(2);
    push_number(1);
  }
  else
  {
    pop_n_elems(2);
    push_number(0);
  }
}
#endif

#ifdef F_THIS_OBJECT
void
f_this_object(num_arg, instruction)
int num_arg, instruction;
{
  if (current_object->flags & O_DESTRUCTED) /* Fixed from 3.1.1 */
    push_number(0);
  else
    push_object(current_object);
}
#endif

#ifdef F_THIS_PLAYER
void
f_this_player(num_arg, instruction)
int num_arg, instruction;
{
  pop_n_elems(num_arg);
  if (num_arg && current_interactive &&
      !(current_interactive->flags & O_DESTRUCTED))
    push_object(current_interactive);
  else if (command_giver && !(command_giver->flags & O_DESTRUCTED))
    push_object(command_giver);
  else
    push_number(0);
}
#endif

#ifdef F_THROW
void
f_throw(num_arg, instruction)
int num_arg, instruction;
{
	assign_svalue(&catch_value, sp--);
	throw_error();		/* do the longjump, with extra checks... */
}
#endif

#ifdef F_TIME
void
f_time(num_arg, instruction)
int num_arg, instruction;
{
  push_number(current_time);
}
#endif

#ifdef F_TO_FLOAT
void
f_to_float(num_arg, instruction)
int num_arg, instruction;
{
	double temp;

	if (sp->type == T_REAL) {
		return; /* leave the pushed float on the stack for returning */
	} else if (sp->type == T_NUMBER) {
		temp = (double)sp->u.number;
		pop_stack();
		push_real(temp);
	} else if (sp->type == F_STRING) {
		sscanf(sp->u.string, "%lf", &temp);
		pop_stack();
		push_real(temp);
	}
}
#endif

#ifdef F_TO_INT
void
f_to_int(num_arg, instruction)
int num_arg, instruction;
{
	int temp;

	if (sp->type == T_NUMBER) {
		return; /* leave the pushed int on the stack for returning */
	} else if (sp->type == T_REAL) {
		temp = (int)sp->u.real;
		pop_stack();
		push_number(temp);
	} else if (sp->type == T_STRING) {
		sscanf(sp->u.string, "%d", &temp);
		pop_stack();
		push_number(temp);
	}
}
#endif

#ifdef F_TRACE
void
f_trace(num_arg, instruction)
int num_arg, instruction;
{
  int ot = -1;
  if (command_giver && command_giver->interactive &&
      command_giver->flags & O_IS_WIZARD) {
    ot = command_giver->interactive->trace_level;
    command_giver->interactive->trace_level = sp->u.number;
  }
  pop_stack();
  push_number(ot);
}
#endif

#ifdef F_TRACEPREFIX
void
f_traceprefix(num_arg, instruction)
int num_arg, instruction;
{
  char *old = 0;

  if (command_giver && command_giver->interactive &&
      command_giver->flags & O_IS_WIZARD)
    {
      old = command_giver->interactive->trace_prefix;
      if (sp->type == T_STRING)
      {
	command_giver->interactive->trace_prefix = 
	  make_shared_string(sp->u.string);
      }
      else
	command_giver->interactive->trace_prefix = 0;
    }
  pop_stack();
  if (old)
  {
    push_string(old, STRING_SHARED); /* Will incr ref count */
    free_string(old);
  }
  else
  {
    push_number(0);
  }
}
#endif

#ifdef F_TYPE
void
f_type(num_arg, instruction)
int num_arg, instruction;
{
  int k;

  k = sp->type;
  pop_stack();
  push_number(k);
}
#endif

#ifdef F_UNDEFINEDP
void
f_undefinedp(num_arg, instruction)
int num_arg, instruction;
{
  if (IS_UNDEFINED(sp))
    assign_svalue(sp, &const1);
  else
    assign_svalue(sp, &const0);
}
#endif

#ifdef F_UNIQUE_ARRAY
void
f_unique_array(num_arg, instruction)
int num_arg, instruction;
{
  extern struct vector
    *make_unique PROT((struct vector *arr,char *func,
		       struct svalue *skipnum));
  struct vector *res;

  if (num_arg < 3)
  {
    check_for_destr((sp-1)->u.vec);
    res = make_unique((sp-1)->u.vec, sp->u.string, &const0);
  }
  else
  {
    check_for_destr((sp-2)->u.vec);
    res = make_unique((sp-2)->u.vec, (sp-1)->u.string, sp);
    pop_stack ();
  }
  pop_n_elems(2);
  if (res)
  {
    push_vector (res);		/* This will make ref count == 2 */
    res->ref--;
  }
  else
    push_number (0);
}
#endif

#ifdef F_UPTIME
void
f_uptime(num_arg, instruction)
int num_arg, instruction;
{
	push_number(current_time - boot_time);
}
#endif

#ifdef F_USERP
void
f_userp(num_arg, instruction)
int num_arg, instruction;
{
  int i;

  i = (int)sp->u.ob->flags & O_ONCE_INTERACTIVE;
  pop_stack();
  push_number(i != 0);
}
#endif

#ifdef F_USERS
void
f_users(num_arg, instruction)
int num_arg, instruction;
{
  push_vector(users());		/* users() has already set ref count to 1 */
  sp->u.vec->ref--;
}
#endif

#ifdef F_WIZARDP
void
f_wizardp(num_arg, instruction)
int num_arg, instruction;
{
  int i;

  i = (int)sp->u.ob->flags & O_IS_WIZARD;
  pop_stack();
  push_number(i != 0);
}
#endif

#ifdef F_VIRTUALP
void
f_virtualp(num_arg, instruction)
int num_arg, instruction;
{
	int i;

	i = (int)sp->u.ob->flags & O_VIRTUAL;
	pop_stack();
	push_number(i != 0);
}
#endif

#ifdef F_DOMAIN_STATS
void
f_domain_stats(num_arg, instruction)
int num_arg, instruction;
{
	struct mapping *m;

	if (num_arg) {
		m = get_domain_stats(sp->u.string);
		pop_stack();
	} else {
		m = get_domain_stats(0);
	}
	if (!m) {
		push_number(0);
	} else {
		/* ref count is properly decremented by get_domain_stats */
		push_mapping(m);
	}
}
#endif

#ifdef F_AUTHOR_STATS
void
f_author_stats(num_arg, instruction)
int num_arg, instruction;
{
	struct mapping *m;

	if (num_arg) {
		m = get_author_stats(sp->u.string);
		pop_stack();
	} else {
		m = get_author_stats(0);
	}
	if (!m) {
		push_number(0);
	} else {
		/* ref count is properly decremented by get_author_stats */
		push_mapping(m);
	}
}
#endif

#ifdef F_SET_AUTHOR
void
f_set_author (num_arg, instruction)
     int num_arg, instruction;
{
  set_author (sp->u.string);
}
#endif

#ifdef F_WRITE
void
f_write(num_arg, instruction)
int num_arg, instruction;
{
  do_write(sp);
}
#endif

#ifdef F_WRITE_BYTES
void
f_write_bytes(num_arg, instruction)
int num_arg, instruction;
{
	int i;

	if (IS_ZERO(sp)) {
		bad_arg(3, instruction);
		pop_n_elems(3);
		push_number(0);	
	} else {
		i = write_bytes((sp-2)->u.string, (sp-1)->u.number, sp->u.string);
		pop_n_elems(3);
		push_number(i);
	}
}
#endif

#ifdef F_WRITE_FILE
void
f_write_file(num_arg, instruction)
int num_arg, instruction;
{
	int i;

	if (IS_ZERO(sp)) {
		bad_arg(2, instruction);
		pop_n_elems(2);
		push_number(0);	
	} else {
		i = write_file((sp-1)->u.string, sp->u.string);
		pop_n_elems(2);
		push_number(i);	
	}
}
#endif

#ifdef F_DUMP_FILE_DESCRIPTORS
void
f_dump_file_descriptors(num_arg, instruction)
int num_arg, instruction;
{
  dump_file_descriptors();
  push_number(0);	
}
#endif

#ifdef F_RECLAIM_OBJECTS
extern int reclaim_objects();

void
f_reclaim_objects(num_arg, instruction)
int num_arg, instruction;
{
  push_number(reclaim_objects());
}
#endif

#ifdef F_OBJECTS
extern struct vector *objects();

void
f_objects(num_arg, instruction)
int num_arg, instruction;
{
    struct svalue *arg;
    char *func;
    struct object *ob;
    struct vector *vec;

    arg = sp - num_arg + 1;
    if (num_arg == 0)
	func = NULL;
    else {
	   func = arg[0].u.string;
	if (num_arg == 1)
	    ob = current_object;
	else
	    ob = arg[1].u.ob;
    }
    vec = objects(func, ob);
	pop_n_elems(num_arg);
    if (!vec) {
	push_number(0);
    } else {
	push_vector(vec);
	vec->ref--;
    }
}
#endif

#ifdef F_MEMORY_INFO
void
f_memory_info(num_arg, instruction)
int num_arg, instruction;
{
  struct object *ob;
  int mem;
  extern int total_prog_block_size;
  extern int total_array_size;
  extern int total_mapping_size;
  extern int tot_alloc_object_size;
  extern char *reserved_area;
 
  if (num_arg == 0)
  {
    int res, tot;
    if (reserved_area)
      res = RESERVED_SIZE;
    else
      res = 0;
    tot = total_prog_block_size + total_array_size + total_mapping_size +
          tot_alloc_object_size + show_otable_status(-1) +
          heart_beat_status(-1) + add_string_status(-1) +
          print_call_out_usage(-1) + res;
    push_number(tot);
    return;
  }
  if (sp->type != T_OBJECT)
    error("Bad argument 1 to memory_info()\n");    
  ob = sp->u.ob;
  if (ob->prog && (ob->prog->p.i.ref == 1 || !(ob->flags & O_CLONE)))
    mem = ob->prog->p.i.total_size;
    else
    mem = 0;
  mem += (data_size(ob) + sizeof(struct object));
  pop_stack();
  push_number(mem);
}
#endif

#ifdef F_RELOAD_OBJECT
void 
f_reload_object(num_arg, instruction)
int num_arg, instruction;
{
  if (sp->type != T_OBJECT)
    error("Bad arg 1 to reload_object()\n");
  reload_object(sp->u.ob);
  pop_stack();
  push_number(0);
}
#endif

#ifdef F_QUERY_SHADOWING
void
f_query_shadowing(num_arg, instruction)
int num_arg, instruction;
{
  if ((sp->type == T_OBJECT) && sp->u.ob->shadowing)
  {
    struct object *ob;
    ob = sp->u.ob;
    pop_stack();
    push_object(ob->shadowing);
  }
    else
    assign_svalue(sp, &const0);
}
#endif

#ifdef F_SET_RESET
void
f_set_reset(num_arg, instruction)
int num_arg, instruction;
{
  if (num_arg == 2)
  {
    (sp-1)->u.ob->next_reset = current_time + sp->u.number;
    pop_n_elems(2);
  }
    else
  {
    sp->u.ob->next_reset = current_time + TIME_TO_RESET/2 +
        random_number(TIME_TO_RESET/2);
    pop_stack();
  }
  push_number(0);
}                 
#endif

#ifdef F_FLOATP
void
f_floatp(num_arg, instruction)
int num_arg, instruction;
{
  if (sp->type == T_REAL)
    assign_svalue(sp, &const1);
    else
    assign_svalue(sp, &const0);
}
#endif
