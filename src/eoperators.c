/*
	eoperators.c: this file contains all of the operators called from
	inside eval_instruction() in interpret.c.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <memory.h>
#include <setjmp.h>
#include "config.h"
#include "lint.h"
#include "interpret.h"
#include "mapping.h"
#include "object.h"
#include "exec.h"
#include "efun_protos.h"
#include "comm.h"
#include "lang.tab.h"
#include "switch.h"
#include "stralloc.h"
#include "debug.h"

/* Needed these from interpret.c. But what for? -SH */
#define TRACE_CALL_OTHER 2
#define TRACE_RETURN 4
#define TRACE_ARGS 8
#define TRACE_HEART_BEAT 32
#define TRACETST(b) (command_giver->interactive->trace_level & (b))
#define TRACEP(b) \
    (command_giver && command_giver->interactive && TRACETST(b) && \
     (command_giver->interactive->trace_prefix == 0 || \
      (current_object && strpref(command_giver->interactive->trace_prefix, \
	      current_object->name))) )
#define TRACEHB (current_heart_beat == 0 \
	 || (command_giver->interactive->trace_level & TRACE_HEART_BEAT))

extern int d_flag;
extern char *pc;
extern int tracedepth;
extern int current_time;
extern char *last_verb;
extern struct svalue *fp;	  /* Pointer to first argument. */
extern int function_index_offset; /* Needed for inheritance */
extern int variable_index_offset; /* Needed for inheritance */
extern struct object *previous_ob;
extern struct object *master_ob;
extern struct svalue *expected_stack;
extern userid_t *backbone_uid;
extern struct svalue const0, const1, const0u, const0n;
extern struct object *current_heart_beat, *current_interactive;
extern struct svalue catch_value;	/* Used to throw an error to a catch */
extern short *break_sp;		/* Points to address to branch to
				 * at next F_BREAK			*/
extern struct control_stack *csp;	/* Points to last element pushed */

extern struct svalue *sp;
extern int eval_cost;

static int i;
static struct svalue *argp;

INLINE void
f_add(num_arg, instruction)
int num_arg, instruction;
{
  struct svalue ret;
  switch((sp-1)->type)
    {
    case T_STRING:
      {
	switch(sp->type)
	  {
	  case T_STRING:
	    {
	      char *res;
          int r = SVALUE_STRLEN(sp-1);
          int len = r + SVALUE_STRLEN(sp) + 1;

	      res = DXALLOC(len, 64, "f_add: 1");
          eval_cost += (len >> 3);
	      (void)strcpy(res, (sp-1)->u.string);
	      (void)strcpy(res + r, sp->u.string);
	      ret.type = T_STRING;
	      ret.subtype = STRING_SHARED;
	      ret.u.string = make_shared_string(res);
	      FREE(res);
	      break;
	    } 
	  case T_NUMBER:
	    {
	      char buff[20];
	      char *res;
          int len;

	      sprintf(buff, "%d", sp->u.number);
          len = SVALUE_STRLEN(sp-1) + strlen(buff) + 1;
	      res = DXALLOC(len, 64, "f_add: 2");
          eval_cost += (len >> 3);
	      strcpy(res, (sp-1)->u.string);
	      strcat(res, buff);
	      ret.type = T_STRING;
	      ret.subtype = STRING_SHARED;
	      ret.u.string = make_shared_string(res);
	      FREE(res);
	      break;
	    }
	  case T_POINTER:
	    ret.type = T_POINTER;
	    ret.u.vec = (struct vector *)prepend_vector(sp->u.vec, sp-1);
        eval_cost += (ret.u.vec->size << 3);
	    break;
	  default: 
	    error("Bad type argument to +. %d %d\n", 
		  (sp-1)->type, sp->type);
	  }
	break;
      } 
    case T_NUMBER:
      {
	ret.type = sp->type;
	switch(sp->type)
	  {
	  case T_NUMBER:
	    ret.u.number = sp->u.number + (sp-1)->u.number;
	    break;
	  case T_STRING:
	    {
	      char buff[20], *res;
          int len;

	      sprintf(buff, "%d", (sp-1)->u.number);
          len = SVALUE_STRLEN(sp) + strlen(buff) + 1;
	      res = DXALLOC(len, 64, "f_add: 3");
          eval_cost += (len >> 3);
	      strcpy(res, buff);
	      strcat(res, sp->u.string);
	      ret.subtype = STRING_SHARED;
	      ret.u.string = make_shared_string(res);
	      FREE(res);
	      break;
	    }
	  case T_POINTER:
	    ret.u.vec = (struct vector *)prepend_vector(sp->u.vec, sp-1);
        eval_cost += (ret.u.vec->size << 3);
	    break;
	  default: 
	    error("Bad type argument to +. %d %d\n", 
		  (sp-1)->type, sp->type);
	  }
	break;
      }
    case T_POINTER:
      {
	ret.type = T_POINTER;
	switch(sp->type)
	  {
	  case T_POINTER:
	    ret.u.vec = add_array((sp-1)->u.vec, sp->u.vec);
        eval_cost += (ret.u.vec->size << 3);
	    break;
	  case T_NUMBER:
	  case T_STRING:
	    ret.u.vec = (struct vector *)append_vector((sp-1)->u.vec, sp);
        eval_cost += (ret.u.vec->size << 2);
	    break;
	  default: 
	    error("Bad type argument to +. %d %d\n", 
		  (sp-1)->type, sp->type);
	  }
	break;
      } 
    case T_MAPPING:
      if (sp->type == T_MAPPING)
	{
	  ret.type = T_MAPPING;
	  ret.u.map = add_mapping((sp-1)->u.map, sp->u.map);
      eval_cost += (ret.u.map->count << 2);
	}
      else
	{
	  error("Bad type argument to +. %d %d\n", (sp-1)->type,
		sp->type);
	}
      break;
    default: 
      error("Bad type argument to +. %d %d\n", 
	    (sp-1)->type, sp->type);
    }
  pop_n_elems(2);
  push_svalue(&ret);
  free_svalue(&ret);
}

INLINE void
f_add_eq(num_arg, instruction)
int num_arg, instruction;
{
	struct svalue *argp;

	if (sp[-1].type != T_LVALUE)
		bad_arg(1, F_ADD_EQ);
	  argp = sp[-1].u.lvalue;
	  switch(argp->type) {
	  case T_STRING:
		{
		  char *new_str;
		  if (sp->type == T_STRING)
		{
		  int l = SVALUE_STRLEN(argp);
		  int len = l + strlen(sp->u.string) + 1;

		  new_str = DXALLOC(len, 64, "f_add_eq: 1");
		  eval_cost += (len >> 3);
		  strcpy(new_str, argp->u.string);
		  strcpy(new_str + l, sp->u.string);
		  pop_n_elems(2);
		  push_malloced_string(new_str);
		}
		  else if (sp->type == T_NUMBER)
		{
		  char buff[20];
		  int len;
		  sprintf(buff, "%d", sp->u.number);
		  len = SVALUE_STRLEN(argp) + strlen(buff) + 1;
		  eval_cost += (len >> 3);
		  new_str = DXALLOC(len, 64, "f_add_eq: 2");
		  strcpy(new_str, argp->u.string);
		  strcat(new_str, buff);
		  pop_n_elems(2);
		  push_malloced_string(new_str);
		}
		  else
		bad_arg(2, F_ADD_EQ);
		  break;
		}
	  case T_NUMBER:
		if (sp->type == T_NUMBER)
		  {
		i = argp->u.number + sp->u.number;
		pop_n_elems(2);
		push_number(i);
		  }
		else
		  error("Bad type number to rhs +=.\n");
		break;
	  case T_POINTER:
		if (sp->type != T_POINTER)
		  bad_arg(2, F_ADD_EQ);
		else
		  {
		struct vector *v;
		v = add_array(argp->u.vec,sp->u.vec);
		eval_cost += (v->size << 3);
		pop_n_elems(2);
		push_vector(v);		/* This will make ref count == 2 */
		v->ref--;
		  }
		break;	      
	  case T_MAPPING:
		if (sp->type != T_MAPPING)
		  bad_arg(2, F_ADD_EQ);
		else
		  {
		struct mapping *m;

		m = add_mapping(argp->u.map, sp->u.map);
		eval_cost += (m->count << 2);
		pop_n_elems(2);
		push_mapping(m);
		m->ref--;
		  }
		break;
	  default:
		bad_arg(1, F_ADD_EQ);
	  }
	  assign_svalue(argp, sp);
}

INLINE void
f_and(num_arg, instruction)
int num_arg, instruction;
{
  if (sp->type == T_POINTER && (sp-1)->type == T_POINTER)
    {
      extern struct vector *intersect_array
	PROT((struct vector *, struct vector *));
      (sp-1)->u.vec = intersect_array(sp->u.vec, (sp-1)->u.vec);
      sp--;
      return;
    }
  if ((sp-1)->type != T_NUMBER)
    bad_arg(1, F_AND);
  if (sp->type != T_NUMBER)
    bad_arg(2, F_AND);
  i = (sp-1)->u.number & sp->u.number;
  sp--;
  sp->u.number = i;
}

INLINE void
f_and_eq(num_arg, instruction)
int num_arg, instruction;
{
  if (sp[-1].type != T_LVALUE)
    bad_arg(1, F_AND_EQ);
  argp = sp[-1].u.lvalue;
  if (argp->type != T_NUMBER)
    error("Bad left type to &=.\n");
  if (sp->type != T_NUMBER)
    error("Bad right type to &=");
  i = argp->u.number & sp->u.number;
  pop_n_elems(2);
  push_number(i);
  assign_svalue(argp, sp);
}

INLINE void
f_parse_command(num_arg, instruction)
int num_arg, instruction;
{
    struct svalue *arg;

    num_arg = EXTRACT_UCHAR(pc);
    pc++;
    arg = sp - num_arg + 1;
    if (arg[0].type != T_STRING)
        bad_arg(1, F_PARSE_COMMAND);
    if (arg[1].type != T_OBJECT && arg[1].type != T_POINTER)
        bad_arg(2, F_PARSE_COMMAND);
    if (arg[2].type != T_STRING)
        bad_arg(3, F_PARSE_COMMAND);
    if (arg[1].type == T_POINTER)
        check_for_destr(arg[1].u.vec);

    i = parse(arg[0].u.string, &arg[1], arg[2].u.string, &arg[3],
          num_arg-3);
    pop_n_elems(num_arg);   /* Get rid of all arguments */
    push_number(i);     /* Push the result value */
}

INLINE void
f_call_function_by_address(num_arg, instruction)
int num_arg, instruction;
{
  unsigned short func_index;
  struct function *funp;

  ((char *)&func_index)[0] = pc[0];
  ((char *)&func_index)[1] = pc[1];
  pc += 2;
  func_index += function_index_offset;
  /*
   * Find the function in the function table. As the function may have
   * been redefined by inheritance, we must look in the last table,
   * which is pointed to by current_object.
   */
#ifdef DEBUG
  if (func_index >= current_object->prog->p.i.num_functions)
    fatal("Illegal function index\n");
#endif

  /* NOT current_prog, which can be an inherited object. */
  funp = &current_object->prog->p.i.functions[func_index];

  if (funp->flags & NAME_UNDEFINED)
    error("Undefined function: %s\n", funp->name);
  /* Save all important global stack machine registers */
  push_control_stack(funp);	/* return pc is adjusted later */

  /* This assigment must be done after push_control_stack() */
  current_prog = current_object->prog;
  /*
   * If it is an inherited function, search for the real
   * definition.
   */
  csp->num_local_variables = EXTRACT_UCHAR(pc);
  pc++;
  funp = setup_new_frame(funp);
  csp->pc = pc;			/* The corrected return address */
  pc = current_prog->p.i.program + funp->offset;
  csp->extern_call = 0;
}

INLINE void
f_divide(num_arg, instruction)
int num_arg, instruction;
{
  if ((sp-1)->type != T_NUMBER)
    bad_arg(1, instruction);
  if (sp->type != T_NUMBER)
    bad_arg(2, instruction);
  if (sp->u.number == 0)
    error("Division by zero\n");
  i = (sp-1)->u.number / sp->u.number;
  sp--;
  sp->u.number = i;
}

INLINE void
f_div_eq(num_arg, instruction)
int num_arg, instruction;
{
  if (sp[-1].type != T_LVALUE)
    bad_arg(1, F_DIV_EQ);
  argp = sp[-1].u.lvalue;
  if (argp->type != T_NUMBER)
    error("Bad left type to /=.\n");
  if (sp->type != T_NUMBER)
    error("Bad right type to /=");
  if (sp->u.number == 0)
    error("Division by 0\n");
  i = argp->u.number / sp->u.number;
  pop_n_elems(2);
  push_number(i);
  assign_svalue(argp, sp);
}

INLINE void
f_end_catch(num_arg, instruction)
int num_arg, instruction;
{
	/* this code from Amylaar's driver version 3.1.2i */
	pop_stack();
	free_svalue(&catch_value);
	catch_value.type = T_NUMBER;
	catch_value.u.number = 0;
	/* We come here when no longjmp() was executed. */
	pop_control_stack();
	push_pop_error_context(0);
	push_number(0);
}

INLINE void
f_eq(num_arg, instruction)
int num_arg, instruction;
{
  if ((sp-1)->type != sp->type)
    {
      pop_stack();
      assign_svalue(sp, &const0);
      return;
    }
  switch(sp->type)
    {
    case T_NUMBER:
      i = (sp-1)->u.number == sp->u.number;
      break;
    case T_POINTER:
      i = (sp-1)->u.vec == sp->u.vec;
      break;
    case T_STRING:
      i = strcmp((sp-1)->u.string, sp->u.string) == 0;
      break;
    case T_OBJECT:
      i = (sp-1)->u.ob == sp->u.ob;
      break;
    default:
      i = 0;
      break;
    }
  pop_n_elems(2);
  push_number(i);
}

INLINE void
f_lsh(num_arg, instruction)
int num_arg, instruction;
{
  if ((sp-1)->type != T_NUMBER)
    bad_arg(1, instruction);
  if (sp->type != T_NUMBER)
    bad_arg(2, instruction);
  i = (sp-1)->u.number << sp->u.number;
  sp--;
  sp->u.number = i;
}

INLINE void
f_lsh_eq(num_arg, instruction)
int num_arg, instruction;
{
  if (sp[-1].type != T_LVALUE)
    bad_arg(1, F_LSH_EQ);
  argp = sp[-1].u.lvalue;
  if (argp->type != T_NUMBER)
    error("Bad left type to <<=.\n");
  if (sp->type != T_NUMBER)
    error("Bad right type to <<=");
  i = argp->u.number << sp->u.number;
  pop_n_elems(2);
  push_number(i);
  assign_svalue(argp, sp);
}

INLINE void
f_mod(num_arg, instruction)
int num_arg, instruction;
{
  if ((sp-1)->type != T_NUMBER)
    bad_arg(1, instruction);
  if (sp->type != T_NUMBER)
    bad_arg(2, instruction);
  if (sp->u.number == 0)
    error("Modulus by zero.\n");
  i = (sp-1)->u.number % sp->u.number;
  sp--;
  sp->u.number = i;
}

INLINE void
f_mod_eq(num_arg, instruction)
int num_arg, instruction;
{
  if (sp[-1].type != T_LVALUE)
    bad_arg(1, F_MOD_EQ);
  argp = sp[-1].u.lvalue;
  if (argp->type != T_NUMBER)
    error("Bad left type to %=.\n");
  if (sp->type != T_NUMBER)
    error("Bad right type to %=");
  if (sp->u.number == 0)
    error("Division by 0\n");
  i = argp->u.number % sp->u.number;
  pop_n_elems(2);
  push_number(i);
  assign_svalue(argp, sp);
}

INLINE void
f_multiply(num_arg, instruction)
int num_arg, instruction;
{
  if ((sp-1)->type != sp->type)
    bad_arg(1, instruction);
  if (sp->type == T_NUMBER)
  {
    i = (sp-1)->u.number * sp->u.number;
    sp--;
    sp->u.number = i;
    return;
  }
  else if (sp->type == T_MAPPING)
  {
    struct mapping *m;
    m = compose_mapping((sp-1)->u.map, sp->u.map);
    pop_n_elems(2);
    push_mapping(m);
    return;
  }
  bad_arg(2, instruction);
}

INLINE void
f_mult_eq(num_arg, instruction)
int num_arg, instruction;
{
  if (sp[-1].type != T_LVALUE)
    bad_arg(1, F_MULT_EQ);
  argp = sp[-1].u.lvalue;
  if (argp->type != sp->type)
    error("Mismatched types on *=.\n");
  if (sp->type == T_NUMBER)
  {
    i = argp->u.number * sp->u.number;
    pop_n_elems(2);
    push_number(i);
  }
  else if (sp->type == T_MAPPING)
  {
    struct mapping *m;
    m = compose_mapping(argp->u.map, sp->u.map);
    pop_n_elems(2);
    push_mapping(m);
  }
  else
    bad_arg(2, F_MULT_EQ);
  assign_svalue(argp, sp);
}

INLINE void
f_ne(num_arg, instruction)
int num_arg, instruction;
{
  if ((sp-1)->type != sp->type)
  {
    pop_stack();
    assign_svalue(sp, &const1);
    return;
  }
  switch(sp->type)
    {
    case T_NUMBER:
      i = (sp-1)->u.number != sp->u.number;
      break;
    case T_STRING:
      i = strcmp((sp-1)->u.string, sp->u.string);
      break;
    case T_POINTER:
      i = (sp-1)->u.vec != sp->u.vec;
      break;
    case T_OBJECT:
      i = (sp-1)->u.ob != sp->u.ob;
      break;
	case T_MAPPING:
      i = (sp-1)->u.map != sp->u.map;
      break;
    default:
      fatal("Illegal type to !=\n");
    }
  pop_n_elems(2);
  push_number(i);
}

INLINE void
f_or(num_arg, instruction)
int num_arg, instruction;
{
  if ((sp-1)->type != T_NUMBER)
    bad_arg(1, F_OR);
  if (sp->type != T_NUMBER)
    bad_arg(2, F_OR);
  i = (sp-1)->u.number | sp->u.number;
  sp--;
  sp->u.number = i;
}

INLINE void
f_or_eq(num_arg, instruction)
int num_arg, instruction;
{
  if (sp[-1].type != T_LVALUE)
    bad_arg(1, F_OR_EQ);
  argp = sp[-1].u.lvalue;
  if (sp[-1].type != T_LVALUE)
    bad_arg(1, F_OR_EQ);
  argp = sp[-1].u.lvalue;
  if (argp->type != T_NUMBER)
    error("Bad left type to |=.\n");
  if (sp->type != T_NUMBER)
    error("Bad right type to |=");
  i = argp->u.number | sp->u.number;
  pop_n_elems(2);
  push_number(i);
  assign_svalue(argp, sp);
}

INLINE void
f_range(num_arg, instruction)
int num_arg, instruction;
{
  if (sp[-1].type != T_NUMBER)
    error("Bad type of start interval to [ .. ] range.\n");
  if (sp[0].type != T_NUMBER)
    error("Bad type of end interval to [ .. ] range.\n");
  if (sp[-2].type == T_POINTER)
    {
      struct vector *v;

      v = slice_array(sp[-2].u.vec, sp[-1].u.number, sp[0].u.number);
      pop_n_elems(3);
      if (v)
	{
	  push_vector(v);
	  v->ref--;		/* Will make ref count == 1 */
	}
      else
	push_number(0);
    }
  else if (sp[-2].type == T_STRING)
    {
      int len, from, to;
      char *res;
	    
      len = strlen(sp[-2].u.string);
      from = sp[-1].u.number;
      if (from < 0)
	from = len + from;
      if (from >= len)
	{
	  pop_n_elems(3);
	  push_string("", STRING_CONSTANT);
	  return;
	}
      to = sp[0].u.number;
      if (to < 0)
	to = len + to;
      if (to < from)
	{
	  pop_n_elems(3);
	  push_string("", STRING_CONSTANT);
	  return;
	}
      if (to >= len)
	to = len-1;
      if (to == len-1)
	{
	  res = string_copy(sp[-2].u.string + from);
	  pop_n_elems(3);
	  push_malloced_string(res);
	  return;
	}
      res = DXALLOC(to - from + 2, 64, "f_range");
      strncpy(res, sp[-2].u.string + from, to - from + 1);
      res[to - from + 1] = '\0';
      pop_n_elems(3);
      push_malloced_string(res);
    }
  else
    error("Bad argument to [ .. ] range operand.\n");
}

INLINE void
f_rsh(num_arg, instruction)
int num_arg, instruction;
{
  if ((sp-1)->type != T_NUMBER)
    bad_arg(1, instruction);
  if (sp->type != T_NUMBER)
    bad_arg(2, instruction);
  i = (sp-1)->u.number >> sp->u.number;
  sp--;
  sp->u.number = i;
}

INLINE void
f_rsh_eq(num_arg, instruction)
int num_arg, instruction;
{
  if (sp[-1].type != T_LVALUE)
    bad_arg(1, F_RSH_EQ);
  argp = sp[-1].u.lvalue;
  if (argp->type != T_NUMBER)
    error("Bad left type to >>=.\n");
  if (sp->type != T_NUMBER)
    error("Bad right type to >>=");
  i = argp->u.number >> sp->u.number;
  pop_n_elems(2);
  push_number(i);
  assign_svalue(argp, sp);
}

INLINE void
f_subtract(num_arg, instruction)
int num_arg, instruction;
{
	extern struct vector *subtract_array PROT((struct vector *,struct vector*));
	if ((sp-1)->type == T_POINTER && sp->type == T_POINTER) {
		extern struct vector *subtract_array
			PROT((struct vector *,struct vector*));
		struct vector *v, *w;

		v = sp->u.vec;
		if (v->ref > 1) {
			v = slice_array(v, 0, v->size-1 );
			v->ref--;
		}
		sp--;
		/* subtract_array already takes care of destructed objects */
		w = subtract_array(sp->u.vec, v);
		free_vector(v);
		free_vector(sp->u.vec);
		sp->u.vec = w;
		return;
	}
	if ((sp-1)->type != T_NUMBER)
		bad_arg(1, F_SUBTRACT);
	if (sp->type != T_NUMBER)
		bad_arg(2, F_SUBTRACT);
	i = (sp-1)->u.number - sp->u.number;
	sp--;
	sp->u.number = i;
}

INLINE void
f_sub_eq(num_arg, instruction)
     int num_arg, instruction;
{
  if (sp[-1].type != T_LVALUE)
    bad_arg(1, F_SUB_EQ);
  argp = sp[-1].u.lvalue;
  switch (argp->type) {
  case T_NUMBER:
    if (sp->type != T_NUMBER)
      error("Bad right type to -=");
    argp->u.number -= sp->u.number;
    sp--;
    break;
  case T_POINTER:
    {
      struct vector *subtract_array PROT((struct vector*,struct vector*));
      struct vector *v, *w;

      if (sp->type != T_POINTER)
	error("Bad right type to -=");
      v = sp->u.vec;
      if (v->ref > 1) {
	v = slice_array(v, 0, v->size-1 );
	v->ref--;
      }
      sp--;
      w = subtract_array(argp->u.vec, v);
      free_vector(argp->u.vec);
      free_vector(v);  /* no longer freed in subtract_array() */
      argp->u.vec = w;
      break;
    }
  default:
    error("Bad left type to -=.\n");
  }
  assign_svalue_no_free(sp, argp);
}

INLINE void
f_switch(num_arg, instruction)
int num_arg, instruction;
{
  extern char* findstring PROT((char*));
  unsigned short offset, break_adr;
  int d,s,r;
  char *l,*end_tab;
  static unsigned short off_tab[] = {
    0*6,1*6,3*6,7*6,15*6,31*6,63*6,127*6,255*6,
    511*6,1023*6,2047*6,4095*6,8191*6
  };

  ((char *)&offset)[0] = pc[1];
  ((char *)&offset)[1] = pc[2];
  ((char *)&break_adr)[0] = pc[3];
  ((char *)&break_adr)[1] = pc[4];
  *--break_sp = break_adr;
  if ( ( i = EXTRACT_UCHAR(pc) >> 4 ) != 0xf )
  {
    if ( sp->type == T_NUMBER && !sp->u.number )
    {
      /* special case: uninitalized string */
      s = (int)ZERO_AS_STR_CASE_LABEL;
    }
    else if ( sp->type == T_STRING )
    {
      switch(sp->subtype)
      {
      case STRING_SHARED:
	s = (int)sp->u.string;
	break;
      default:
	s = (int)findstring(sp->u.string);
	break;
      }
    }
    else
    {
      bad_arg(1, F_SWITCH);
    }
  }
  else
  {
    if (sp->type != T_NUMBER) bad_arg(1, F_SWITCH);
    s = sp->u.number;
    i = (int)pc[0] &0xf ;
  }
  pop_stack();
  end_tab = current_prog->p.i.program + break_adr;
  if ( i >= 14 )
    if ( i == 14 )
    {
      /* fastest switch format : lookup table */
      l = current_prog->p.i.program + offset;
      ((char *)&d)[0] = end_tab[-4];
      ((char *)&d)[1] = end_tab[-3];
      ((char *)&d)[2] = end_tab[-2];
      ((char *)&d)[3] = end_tab[-1];
      if ( s >= d && l + (s=(s-d)*sizeof(short)) < end_tab - 4 )
      {
	((char *)&offset)[0] = l[s];
	((char *)&offset)[1] = l[s+1];
	if (offset) {
	  pc = current_prog->p.i.program + offset;
	}
      }
      /* default */
      ((char *)&offset)[0] = pc[5];
      ((char *)&offset)[1] = pc[6];
      pc = current_prog->p.i.program + offset;
    }
    else
      fatal("unsupported switch table format.\n");
  l = current_prog->p.i.program + offset + off_tab[i];
  d = (off_tab[i]+(unsigned)6) >> 1;
  if (d == 3) d=0;
  for(;;)
  {
    ((char *)&r)[0] = l[0];
    ((char *)&r)[1] = l[1];
    ((char *)&r)[2] = l[2];
    ((char *)&r)[3] = l[3];
    if (s < r)
      if (d < 6)
      {
	if (!d)
	{		/* test for range */
	  ((char *)&offset)[0] = l[-2];
	  ((char *)&offset)[1] = l[-1];

	  /* F_BREAK is required to be > 1 */
	  if (offset <= 1)
	  {
	    ((char *)&r)[0] = l[-6];
	    ((char *)&r)[1] = l[-5];
	    ((char *)&r)[2] = l[-4];
	    ((char *)&r)[3] = l[-3];
	    if (s >= r)
	    {
	      /* s is in the range */
	      if (!offset)
	      {
		/* range with lookup table */
		((char *)&offset)[0] = l[4];
		((char *)&offset)[1] = l[5];
		l = current_prog->p.i.program + offset +
		  (s-r) * sizeof(short);
		((char *)&offset)[0] = l[0];
		((char *)&offset)[1] = l[1];
		break;
	      }
	      ((char *)&offset)[0] = l[4];
	      ((char *)&offset)[1] = l[5];
	      break;
	    }
	  }
	  /* use default address */
	  ((char *)&offset)[0] = pc[5];
	  ((char *)&offset)[1] = pc[6];
	  break;
	}			/* !d */
	d = 0;
      }
      else
      {
	/* d >= 6 */
	l -= d;
	d >>= 1;
      }
    else if (s > r)
    {
      if (d < 6) 
     {
	if (!d)
	{		/* test for range */
	  ((char *)&offset)[0] = l[4];
	  ((char *)&offset)[1] = l[5];
	  if (offset <= 1)
	  {
	    ((char *)&r)[0] = l[6];
	    ((char *)&r)[1] = l[7];
	    ((char *)&r)[2] = l[8];
	    ((char *)&r)[3] = l[9];
	    if (s <= r)
	    {
	      /* s is in the range */
	      if (!offset)
	      {
		/* range with lookup table */
		((char *)&offset)[0] = l[10];
		((char *)&offset)[1] = l[11];
		l = current_prog->p.i.program + offset +
		  (s-r) * sizeof(short);
		((char *)&offset)[0] = l[0];
		((char *)&offset)[1] = l[1];
		break;
	      }
	      ((char *)&offset)[0] = l[10];
	      ((char *)&offset)[1] = l[11];
	      break;
	    }
	  }
	  /* use default address */
	  ((char *)&offset)[0] = pc[5];
	  ((char *)&offset)[1] = pc[6];
	  break;
	}			/* !d */
	d = 0;
      }
      else
      {
	/* d >= 6 */
	l += d;
	while (l >= end_tab)
	{
	  d >>= 1;
	  if (d <= 3)
	  {
	    if (!d) break;
	    d = 0;
	  }
	  l -= d;
	}
	d >>= 1;
      }
    }
    else
    {
      /* s == r */
      ((char *)&offset)[0] = l[4];
      ((char *)&offset)[1] = l[5];
      if ( !l[-2] && !l[-1] )     
     {
	/* end of range with lookup table */
	((char *)&r)[0] = l[-6];
	((char *)&r)[1] = l[-5];
	((char *)&r)[2] = l[-4];
	((char *)&r)[3] = l[-3];
	l = current_prog->p.i.program + offset + (s-r)*sizeof(short);
	((char *)&offset)[0] = l[0];
	((char *)&offset)[1] = l[1];
      }
      if (offset <= 1)
      {
	if (!offset)
	{
	  /* start of range with lookup table */
	  ((char *)&offset)[0] = l[10];
	  ((char *)&offset)[1] = l[11];
	  l = current_prog->p.i.program + offset;
	  ((char *)&offset)[0] = l[0];
	  ((char *)&offset)[1] = l[1];
	}
	else
	{
	  ((char *)&offset)[0] = l[10];
	  ((char *)&offset)[1] = l[11];
	}
      }
      break;
    }
  }
  pc = current_prog->p.i.program + offset;
}

INLINE void
f_xor(num_arg, instruction)
int num_arg, instruction;
{
  if ((sp-1)->type != T_NUMBER)
    bad_arg(1, instruction);
  if (sp->type != T_NUMBER)
    bad_arg(2, instruction);
  i = (sp-1)->u.number ^ sp->u.number;
  sp--;
  sp->u.number = i;
}

INLINE void
f_xor_eq(num_arg, instruction)
int num_arg, instruction;
{
  if (sp[-1].type != T_LVALUE)
    bad_arg(1, F_XOR_EQ);
  argp = sp[-1].u.lvalue;
  if (argp->type != T_NUMBER)
    error("Bad left type to ^=.\n");
  if (sp->type != T_NUMBER)
    error("Bad right type to ^=");
  i = argp->u.number ^ sp->u.number;
  pop_n_elems(2);
  push_number(i);
  assign_svalue(argp, sp);
}

INLINE void
f_function_constructor(num_arg, instruction)
int num_arg, instruction;
{
	struct funp *fp;

	fp = make_funp(sp - 1, sp);
	pop_n_elems(2);
	push_funp(fp);
	fp->ref--;
}

INLINE void
f_function_split(num_arg, instruction)
int num_arg, instruction;
{
	struct svalue *obj, *fun;

	if (sp->type != T_FUNCTION) {
		bad_arg(1, F_FUNCTION_SPLIT);
	}
	obj = &sp->u.fp->obj;
	fun = &sp->u.fp->fun;
	/* the POP is inserted from postlang.y (don't do it here) */
	push_svalue(obj);
	push_svalue(fun);
}

INLINE void
f_aggregate_assoc(num_arg, instruction)
int num_arg, instruction;
{
  struct mapping *m;
  unsigned short num;

  ((char *)&num)[0] = pc[0];
  ((char *)&num)[1] = pc[1];
  pc += 2;
  m = load_mapping_from_aggregate(sp - num + 1, num);
  pop_n_elems((int)num);
  push_mapping(m);
  m->ref--;
}
   
INLINE void
f_aggregate(num_arg, instruction)
int num_arg, instruction;
{
  struct vector *v;
  unsigned short num;

  ((char *)&num)[0] = pc[0];
  ((char *)&num)[1] = pc[1];
  pc += 2;
  v = allocate_array((int)num);
  for (i=0; (unsigned)i < num; i++)
    assign_svalue_no_free(&v->item[i], sp + i - num + 1);
  pop_n_elems((int)num);
  push_vector(v);
  v->ref--;
}

INLINE void
f_sscanf(num_arg, instruction)
int num_arg, instruction;
{
  num_arg = EXTRACT_UCHAR(pc);
  pc++;
  i = inter_sscanf(num_arg);
  pop_n_elems(num_arg);
  push_number(i);
}
