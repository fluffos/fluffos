/*
	eoperators.c: this file contains all of the operators called from
	inside eval_instruction() in interpret.c.
*/

#include <sys/types.h>
#include <sys/stat.h>
#ifndef LATTICE
#include <sys/time.h>
#else
#include <time.h>
#endif
#ifndef LATTICE
#include <sys/ioctl.h>
#include <netdb.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#ifndef LATTICE
#include <memory.h>
#else
#include "amiga.h"
#endif
#include <setjmp.h>
#include "config.h"
#include "lint.h"
#include "interpret.h"
#include "buffer.h"
#include "mapping.h"
#include "object.h"
#include "exec.h"
#include "efun_protos.h"
#include "comm.h"
#include "opcodes.h"
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

/* Points to value of last push. */
extern struct svalue *sp;

extern int eval_cost;

static int i;
static float r;
static struct svalue *argp;

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
    bad_arg(1, instruction);
  if (sp->type != T_NUMBER)
    bad_arg(2, instruction);
  i = (sp-1)->u.number & sp->u.number;
  sp--;
  sp->u.number = i;
}

INLINE void
f_and_eq(num_arg, instruction)
int num_arg, instruction;
{
  if (sp[-1].type != T_LVALUE)
    bad_arg(1, instruction);
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
	bad_arg(1, instruction);
    if (arg[1].type != T_OBJECT && arg[1].type != T_POINTER)
	bad_arg(2, instruction);
    if (arg[2].type != T_STRING)
	bad_arg(3, instruction);
    if (arg[1].type == T_POINTER)
	check_for_destr(arg[1].u.vec);

    i = parse(arg[0].u.string, &arg[1], arg[2].u.string, &arg[3], num_arg-3); 
    pop_n_elems(num_arg);	/* Get rid of all arguments */
    push_number(i);		/* Push the result value */
}

INLINE void
f_div_eq(num_arg, instruction)
int num_arg, instruction;
{
  if((sp - 1)->type != T_LVALUE)
    bad_arg(1, instruction);
  argp = (sp - 1)->u.lvalue;

  if(argp->type == T_NUMBER){
    if(sp->type == T_NUMBER){
      if(sp->u.number == 0)
	error("Division by 0nn\n");
      i = argp->u.number / sp->u.number;
      pop_n_elems(2);
      push_number(i);
      assign_svalue(argp, sp);
    }
    else if(sp->type == T_REAL){
      if(sp->u.real == 0.)
        error("Division by 0.nr\n");
      r = argp->u.number / sp->u.real;
      pop_n_elems(2);
      push_real(r);
      assign_svalue(argp, sp);
    }
    else {
      error("Bad right type to /=");
    }
  }
  else if(argp->type == T_REAL){
    if(sp->type == T_NUMBER){
      if(sp->u.number == 0)
        error("Division by 0rn\n");
      r = argp->u.real / sp->u.number;
      pop_n_elems(2);
      push_real(r);
      assign_svalue(argp, sp);
    }
    else if(sp->type == T_REAL){
      if(sp->u.real == 0.)
        error("Division by 0.rr\n");
      r = argp->u.real / sp->u.real;
      pop_n_elems(2);
      push_real(r);
      assign_svalue(argp, sp);
    }
    else {
      error("Bad right type to /=");
    }
  }
  else {
    error("Bad left type to /=\n");
  }
}

INLINE void
f_eq(num_arg, instruction)
int num_arg, instruction;
{
  switch(sp->type){
  case T_NUMBER:
    if((sp-1)->type == T_NUMBER){
      i = ((sp-1)->u.number == sp->u.number);
    }
    else if((sp-1)->type == T_REAL){
      i = ((sp-1)->u.real == sp->u.number);
    }
    else {
      pop_stack();
      assign_svalue(sp, &const0);
      return;
    }
    break;
  case T_REAL:
    if((sp-1)->type == T_NUMBER){
      i = ((sp-1)->u.number == sp->u.real);
    }
    else if((sp-1)->type == T_REAL){
      i = ((sp-1)->u.real == sp->u.real);
    }
    else {
      pop_stack();
      assign_svalue(sp, &const0);
      return;
    }
    break;
  case T_POINTER:
    if((sp-1)->type == T_POINTER){
      i = ((sp-1)->u.vec == sp->u.vec);
    }
    else {
      pop_stack();
      assign_svalue(sp, &const0);
      return;
    }
    break;
  case T_STRING:
    if((sp-1)->type == T_STRING){
      i = (strcmp((sp-1)->u.string, sp->u.string) == 0);
    }
    else {
      pop_stack();
      assign_svalue(sp, &const0);
      return;
    }
    break;
  case T_OBJECT:
    if((sp-1)->type == T_OBJECT){
      i = ((sp-1)->u.ob == sp->u.ob);
    }
    else {
      pop_stack();
      assign_svalue(sp, &const0);
      return;
    }
    break;
  case T_MAPPING:
    if((sp-1)->type == T_MAPPING){
      i = ((sp-1)->u.map == sp->u.map);
    }
    else {
      pop_stack();
      assign_svalue(sp, &const0);
      return;
    }
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
    bad_arg(1, instruction);
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
f_mod_eq(num_arg, instruction)
int num_arg, instruction;
{
  if (sp[-1].type != T_LVALUE)
    bad_arg(1, instruction);
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
f_mult_eq(num_arg, instruction)
int num_arg, instruction;
{
  if((sp - 1)->type != T_LVALUE)
    bad_arg(1, instruction);
  argp = (sp - 1)->u.lvalue;

  if(argp->type == T_NUMBER){
    if(sp->type == T_NUMBER){
      i = argp->u.number * sp->u.number;
      pop_n_elems(2);
      push_number(i);
      assign_svalue(argp, sp);
    }
    else if(sp->type == T_REAL){
      r = argp->u.number * sp->u.real;
      pop_n_elems(2);
      push_real(r);
      assign_svalue(argp, sp);
    }
    else {
      error("Bad right type to *=");
    }
  }
  else if(argp->type == T_REAL){
    if(sp->type == T_NUMBER){
      r = argp->u.real * sp->u.number;
      pop_n_elems(2);
      push_real(r);
      assign_svalue(argp, sp);
    }
    else if(sp->type == T_REAL){
      r = argp->u.real * sp->u.real;
      pop_n_elems(2);
      push_real(r);
      assign_svalue(argp, sp);
    }
    else {
      error("Bad right type to *=");
    }
  }
  else if(argp->type == T_MAPPING){
    if(sp->type == T_MAPPING){
      struct mapping *m;

      m = compose_mapping(argp->u.map, sp->u.map);
      pop_n_elems(2);
      push_mapping(m);
      assign_svalue(argp, sp);
    }
    else {
      error("Bad right type to *=");
    }
  }
  else {
    error("Bad left type to *=\n");
  }
}

INLINE void
f_ne(num_arg, instruction)
int num_arg, instruction;
{
  switch(sp->type){
  case T_NUMBER:
    if((sp-1)->type == T_NUMBER){
      i = ((sp-1)->u.number != sp->u.number);
    }
    else if((sp-1)->type == T_REAL){
      i = ((sp-1)->u.real != sp->u.number);
    }
    else {
      pop_stack();
      assign_svalue(sp, &const1);
      return;
    }
    break;
  case T_REAL:
    if((sp-1)->type == T_NUMBER){
      i = ((sp-1)->u.number != sp->u.real);
    }
    else if((sp-1)->type == T_REAL){
      i = ((sp-1)->u.real != sp->u.real);
    }
    else {
      pop_stack();
      assign_svalue(sp, &const1);
      return;
    }
    break;
  case T_POINTER:
    if((sp-1)->type == T_POINTER){
      i = ((sp-1)->u.vec != sp->u.vec);
    }
    else {
      pop_stack();
      assign_svalue(sp, &const1);
      return;
    }
    break;
  case T_STRING:
    if((sp-1)->type == T_STRING){
      i = strcmp((sp-1)->u.string, sp->u.string);
    }
    else {
      pop_stack();
      assign_svalue(sp, &const1);
      return;
    }
    break;
  case T_OBJECT:
    if((sp-1)->type == T_OBJECT){
      i = ((sp-1)->u.ob != sp->u.ob);
    }
    else {
      pop_stack();
      assign_svalue(sp, &const1);
      return;
    }
    break;
  case T_MAPPING:
    if((sp-1)->type == T_MAPPING){
      i = ((sp-1)->u.map != sp->u.map);
    }
    else {
      pop_stack();
      assign_svalue(sp, &const1);
      return;
    }
    break;
  default:
    i = 1;
    break;
  }
  pop_n_elems(2);
  push_number(i);
}

INLINE void
f_or(num_arg, instruction)
int num_arg, instruction;
{
  if ((sp-1)->type != T_NUMBER)
    bad_arg(1, instruction);
  if (sp->type != T_NUMBER)
    bad_arg(2, instruction);
  i = (sp-1)->u.number | sp->u.number;
  sp--;
  sp->u.number = i;
}

INLINE void
f_or_eq(num_arg, instruction)
int num_arg, instruction;
{
  if (sp[-1].type != T_LVALUE)
    bad_arg(1, instruction);
  argp = sp[-1].u.lvalue;
  if (sp[-1].type != T_LVALUE)
    bad_arg(1, instruction);
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
      res = DXALLOC(to - from + 2, 37, "f_range");
      strncpy(res, sp[-2].u.string + from, to - from + 1);
      res[to - from + 1] = '\0';
      pop_n_elems(3);
      push_malloced_string(res);
    }
  else if (sp[-2].type == T_BUFFER)
    {
      int len, from, to;
      struct buffer *rbuf;
	    
      len = sp[-2].u.buf->size;
      from = sp[-1].u.number;
      if (from < 0)
	from = len + from;
      if (from >= len)
	{
	  pop_n_elems(3);
	  push_buffer(null_buffer());
	  return;
	}
      to = sp[0].u.number;
      if (to < 0)
	to = len + to;
      if (to < from)
	{
	  pop_n_elems(3);
	  push_buffer(null_buffer());
	  return;
	}
      if (to >= len)
	to = len-1;
      rbuf = allocate_buffer(to - from + 1);
      rbuf->ref--;
      memcpy(rbuf->item, sp[-2].u.buf->item + from, to - from + 1);
      pop_n_elems(3);
      push_buffer(rbuf);
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
    bad_arg(1, instruction);
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
f_sub_eq(num_arg, instruction)
     int num_arg, instruction;
{
  if((sp - 1)->type != T_LVALUE)
    bad_arg(1, instruction);
  argp = (sp - 1)->u.lvalue;

  switch(argp->type){
  case T_NUMBER:
    if(sp->type == T_NUMBER){
      argp->u.number -= sp->u.number;
      sp--;
    }
    else if(sp->type == T_REAL){
      argp->u.number -= sp->u.real;
      sp--;
    }
    else {
      error("Bad type number to rhs -=");
    }
    break;
  case T_REAL:
    if(sp->type == T_NUMBER){
      argp->u.real -= sp->u.number;
      sp--;
    }
    else if(sp->type == T_REAL){
      argp->u.real -= sp->u.real;
      sp--;
    }
    else {
      error("Bad type number to rhs -=");
    }
    break;
  case T_POINTER:
    if(sp->type == T_POINTER){
      struct vector *subtract_array PROT((struct vector*,struct vector*));
      struct vector *v, *w;

      v = sp->u.vec;
      if(v->ref > 1){
        v = slice_array(v, 0, v->size-1 );
        v->ref--;
      }
      w = subtract_array(argp->u.vec, v);
      free_vector(argp->u.vec);
      free_vector(v);  /* no longer freed in subtract_array() */
      eval_cost += (w->size << 3);
      sp--;
      argp->u.vec = w;
    }
    else {
      error("Bad type number to rhs -=");
    }
    break;
  default:
    error("Bad left type to -=\n");
  }
  assign_svalue_no_free(sp, argp);
}

/*
 * Structure of F_SWITCH:
 *   table type (1 byte)
 *   address of table (1 short)
 *   address of break (1 short)
 *   address of default (1 short)
 *     then all the switch code
 *   switch table (varies)
 *
 * Table type is either
 *   0xfe  - integer labels, direct lookup.
 *           Table is followed by 1 long that is minimum key value.
 *           Each table entry is a short address to jump to.
 *   0xfN  - integer labels.  N is size as a power of 2.
 *           Each table entry is 1 long (key) followed by 1 short (address).
 *   0xNf  - string labels.  Otherwise same as for integer labels.
 *
 * For normal string or integer tables, if the address is 0 or 1,
 * the key is the lower end of a range, and the upper end is in
 * the next entry.  If it's a 0, the second address indicates a
 * direct lookup table (currently this case is never generated by
 * the compiler).  If it's a 1, the second address is used for
 * all keys in the range (corresponds to 'case x..y:' labels).
 *
 * Binary search is used on the normal tables.
 */

/* offsets from 'pc' */
#define SW_TYPE		0
#define SW_TABLE	1
#define SW_BREAK	3
#define SW_DEFAULT	5

#define ENTRY_SIZE	6

/* offsets used for range (L_ for lower member, U_ for upper member) */
#define L_LOWER	0
#define L_TYPE	4
#define L_UPPER	6
#define L_ADDR	10
#define U_LOWER	-6
#define U_TYPE	-2
#define U_UPPER	0
#define U_ADDR	4

#define COPY_SHORT(dst, src) \
    { ((char *)(dst))[0] = ((char *)(src))[0]; \
      ((char *)(dst))[1] = ((char *)(src))[1]; }
#define COPY_LONG(dst, src) \
    { ((char *)(dst))[0] = ((char *)(src))[0]; \
      ((char *)(dst))[1] = ((char *)(src))[1]; \
      ((char *)(dst))[2] = ((char *)(src))[2]; \
      ((char *)(dst))[3] = ((char *)(src))[3]; }

INLINE void
f_switch(num_arg, instruction)
int num_arg, instruction;
{
  extern char* findstring PROT((char*));
  unsigned short offset, break_adr;
  int d, s = 0, r;
  char *l,*end_tab;
  static unsigned short off_tab[] = {
    0*ENTRY_SIZE, 1*ENTRY_SIZE, 3*ENTRY_SIZE, 7*ENTRY_SIZE, 15*ENTRY_SIZE,
    31*ENTRY_SIZE, 63*ENTRY_SIZE, 127*ENTRY_SIZE, 255*ENTRY_SIZE,
    511*ENTRY_SIZE, 1023*ENTRY_SIZE, 2047*ENTRY_SIZE, 4095*ENTRY_SIZE,
    8191*ENTRY_SIZE,
  };

  COPY_SHORT(&offset, pc+SW_TABLE);
  COPY_SHORT(&break_adr, pc+SW_BREAK);
  *--break_sp = break_adr;
  if ( ( i = EXTRACT_UCHAR(pc) >> 4 ) != 0xf )
  { /* String table, find correct key */
    if ( sp->type == T_NUMBER && !sp->u.number )
    {
      /* special case: 0 as a string */
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
      if (s == 0)
      {
	/* Take default case now - else we could be get confused
	 * with ZERO_AS_STR_CASE_LABEL.
	 */
	COPY_SHORT(&offset, pc+SW_DEFAULT);
	pop_stack();
	pc = current_prog->p.i.program + offset;
	return;
      }
    }
    else
    {
      bad_arg(1, instruction);
    }
  }
  else
  { /* Integer table, check type */
    if (sp->type != T_NUMBER)
      bad_arg(1, instruction);
    s = sp->u.number;
    i = (int)pc[0] &0xf ;
  }
  pop_stack();
  end_tab = current_prog->p.i.program + break_adr;
  /*
   * i is the table size as a power of 2.  Tells us where to
   * start searching.  i==14 is a special case.
   */
  if ( i >= 14 )
    if ( i == 14 )
    {
      /* fastest switch format : lookup table */
      l = current_prog->p.i.program + offset;
      COPY_LONG(&d, end_tab-4);
      /* d is minimum value - see if in range or not */
      if ( s >= d && l + (s=(s-d)*sizeof(short)) < (end_tab - 4) )
      {
	COPY_SHORT(&offset, &l[s]);
	if (offset) {
	  pc = current_prog->p.i.program + offset;
	  return;
	}
      }
      /* default */
      COPY_SHORT(&offset, pc+SW_DEFAULT);
      pc = current_prog->p.i.program + offset;
      return;
    }
    else
      fatal("unsupported switch table format.\n");

  /*
   * l - current entry we are looking at.
   * d - size to add/subtract from l each iteration.
   * s - key we're looking for
   * r - key l is pointing at
   */
  l = current_prog->p.i.program + offset + off_tab[i];
  d = (int)(off_tab[i]+ENTRY_SIZE) >> 1;
  if (d < ENTRY_SIZE) d=0;
  for(;;)
  {
    COPY_LONG(&r, l);
    if (s < r)
    {
      if (d < ENTRY_SIZE)
      {
	/* test if entry is part of a range */
	/* Don't worry about reading from F_BREAK (byte before table) */
	COPY_SHORT(&offset, l+U_TYPE);
	if (offset <= 1)
	{
	  COPY_LONG(&r, l+U_LOWER);
	  if (s >= r)
	  {
	    /* s is in the range */
	    COPY_SHORT(&offset, l+U_ADDR);
	    if (!offset)
	    {
	      /* range with lookup table */
	      l = current_prog->p.i.program + offset +
		(s-r) * sizeof(short);
	      COPY_SHORT(&offset, l);
	    }  /* else normal range and offset is correct */
	    break;
	  }
	}
	/* key not found, use default address */
	COPY_SHORT(&offset, pc+SW_DEFAULT);
	break;
      }
      else
      {
	/* d >= ENTRY_SIZE */
	l -= d;
	d >>= 1;
      }
    }
    else if (s > r)
    {
      if (d < ENTRY_SIZE) 
      {
	/* test if entry is part of a range */
	COPY_SHORT(&offset, l+L_TYPE);
	if (offset <= 1)
	{
	  COPY_LONG(&r, l+L_UPPER);
	  if (s <= r)
	  {
	    /* s is in the range */
	    COPY_SHORT(&offset, l+L_ADDR);
	    if (!offset)
	    {
	      /* range with lookup table */
	      l = current_prog->p.i.program + offset +
		(s-r) * sizeof(short);
	      COPY_SHORT(&offset, l);
	    }  /* else normal range and offset is correct */
	    break;
	  }
	}
	/* use default address */
	COPY_SHORT(&offset, pc+SW_DEFAULT);
	break;
      }
      else
      { /* d >= ENTRY_SIZE */
	l += d;
	/* if table isn't a power of 2 in size, fix us up */
	while (l >= end_tab)
	{
	  d >>= 1;
	  if (d < ENTRY_SIZE)
	  {
	    d = 0;
	    break;
	  }
	  l -= d;
	}
	if (l == end_tab) {
	  /* use default address */
	  COPY_SHORT(&offset, pc+SW_DEFAULT);
	  break;
	}
	d >>= 1;
      }
    }
    else
    {
      /* s == r */
      COPY_SHORT(&offset, l+U_ADDR);
      /* found the key - but could be part of a range... */
      if ( !l[U_TYPE] && !l[U_TYPE+1] )     
      {
	/* end of range with lookup table */
	COPY_LONG(&r, l+U_LOWER);
	l = current_prog->p.i.program + offset + (s-r)*sizeof(short);
	COPY_SHORT(&offset, l);
      }
      if (offset <= 1)
      {
	COPY_SHORT(&offset, l+L_ADDR);
	if (!offset)
	{
	  /* start of range with lookup table */
	  l = current_prog->p.i.program + offset;
	  COPY_SHORT(&offset, l);
	}  /* else normal range, offset is correct */
      }
      break;
    }
  }
  /* now do jump */
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
    bad_arg(1, instruction);
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

INLINE struct funp *
make_funp(sob, sfun)
struct svalue *sob, *sfun;
{
	struct funp *fp;

	fp = (struct funp *)DMALLOC(sizeof(struct funp), 38, "make_funp");
	if (current_object) {
		if (current_object->euid) {
			fp->euid = current_object->euid;
		} else {
			fp->euid = current_object->uid;
		}
	} else {
		fp->euid = NULL;
	}
	assign_svalue_no_free(&fp->obj, sob);
	assign_svalue_no_free(&fp->fun, sfun);
	fp->ref = 1;
	return fp;
}

INLINE void
push_funp(fp)
struct funp *fp;
{
	fp->ref++;
	sp++;
	sp->type = T_FUNCTION;
	sp->u.fp = fp;
}

INLINE void
free_funp(fp)
struct funp *fp;
{
	fp->ref--;
	if (fp->ref > 0) {
		return;
	}
	free_svalue(&fp->obj);
	free_svalue(&fp->fun);
	FREE(fp);
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
f_this_function_constructor(num_arg, instruction)
int num_arg, instruction;
{
        struct funp *fp;

	if (current_object->flags & O_DESTRUCTED)
	  push_number(0);
	else
	  push_object(current_object);
/* Note: The stack order is reversed here compared to f_function_constructor */
	fp = make_funp(sp , sp - 1);
	pop_n_elems(2);
	push_funp(fp);
	fp->ref--;
}

INLINE void
f_function_split(num_arg, instruction)
int num_arg, instruction;
{
	struct svalue *obj, *fun;
	struct funp *tmp;

	if (sp->type != T_FUNCTION) {
		bad_arg(1, instruction);
	}
	obj = &sp->u.fp->obj;
	fun = &sp->u.fp->fun;
	tmp = sp->u.fp;
	sp--; /* don't free the funp here since that would also free obj and fun */
	if ((obj->type == T_OBJECT) && (obj->u.ob->flags & O_DESTRUCTED)) {
		assign_svalue(obj, &const0n);
		push_null();
	} else {
		push_svalue(obj);
	}
	push_svalue(fun);
	free_funp(tmp); /* go ahead and free it here since the pushes make it ok */
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
  for (i=0; i < (int)num; i++)
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
