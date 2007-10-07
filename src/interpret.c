#define SUPPRESS_COMPILER_INLINES
#include "std.h"
#include "lpc_incl.h"
#include "efuns_incl.h"
#include "file.h"
#include "file_incl.h"
#include "patchlevel.h"
#include "backend.h"
#include "simul_efun.h"
#include "eoperators.h"
#include "efunctions.h"
#include "sprintf.h"
#include "comm.h"
#include "port.h"
#include "qsort.h"
#include "compiler.h"
#include "regexp.h"
#include "master.h" 
#include "eval.h"

#ifdef OPCPROF
#include "opc.h"

static int opc_eoper[BASE];
#endif

#ifdef OPCPROF_2D
/* warning, this is typically 4 * 100 * 100 = 40k */
static int opc_eoper_2d[BASE+1][BASE+1];
static int last_eop = 0;
#endif

static const char *type_names[] = {
  "int",
  "string",
  "array",
  "object",
  "mapping",
  "function",
  "float",
  "buffer",
  "class"
};
#define TYPE_CODES_END 0x400
#define TYPE_CODES_START 0x2

#ifdef PACKAGE_UIDS
extern userid_t *backbone_uid;
#endif
extern int max_cost;
extern int call_origin;
static int find_line (char *, const program_t *, const char **, int *);
INLINE void push_indexed_lvalue (int);
#ifdef TRACE
static void do_trace_call (int);
#endif
void break_point (void);
INLINE_STATIC void do_loop_cond_number (void);
INLINE_STATIC void do_loop_cond_local (void);
static void do_catch (char *, unsigned short);
int last_instructions (void);
static float _strtof (char *, char **);
#ifdef TRACE_CODE
static char *get_arg (int, int);
#endif

#ifdef DEBUG
int stack_in_use_as_temporary = 0;
#endif

int inter_sscanf (svalue_t *, svalue_t *, svalue_t *, int);
program_t *current_prog;
short int caller_type;
static int tracedepth;
int num_varargs;

/*
 * Inheritance:
 * An object X can inherit from another object Y. This is done with
 * the statement 'inherit "file";'
 * The inherit statement will clone a copy of that file, call reset
 * in it, and set a pointer to Y from X.
 * Y has to be removed from the linked list of all objects.
 * All variables declared by Y will be copied to X, so that X has access
 * to them.
 *
 * If Y isn't loaded when it is needed, X will be discarded, and Y will be
 * loaded separately. X will then be reloaded again.
 */

/*
 * These are the registers used at runtime.
 * The control stack saves registers to be restored when a function
 * will return. That means that control_stack[0] will have almost no
 * interesting values, as it will terminate execution.
 */
char *pc;     /* Program pointer. */
svalue_t *fp;   /* Pointer to first argument. */

svalue_t *sp;
svalue_t const0, const1, const0u;

int function_index_offset;  /* Needed for inheritance */
int variable_index_offset;  /* Needed for inheritance */
int st_num_arg;

static svalue_t start_of_stack[CFG_EVALUATOR_STACK_SIZE+10];
svalue_t *end_of_stack = start_of_stack + CFG_EVALUATOR_STACK_SIZE;

/* Used to throw an error to a catch */
svalue_t catch_value = {T_NUMBER};

/* used by routines that want to return a pointer to an svalue */
svalue_t apply_ret_value = {T_NUMBER};

control_stack_t control_stack[CFG_MAX_CALL_DEPTH+5];
control_stack_t *csp; /* Points to last element pushed */

int too_deep_error = 0, max_eval_error = 0;

ref_t *global_ref_list = 0;

void kill_ref (ref_t * ref) {
  if (ref->sv.type == T_MAPPING && (ref->sv.u.map->count & MAP_LOCKED)) {
    ref_t *r = global_ref_list;
    
    /* if some other ref references this mapping, it needs to remain
       locked */
    while (r) {
      if (r->sv.u.map == ref->sv.u.map)
        break;
      r = r->next;
    }
    if (!r)
      unlock_mapping(ref->sv.u.map);
  }
  if(ref->lvalue)
    free_svalue(&ref->sv, "kill_ref");
  if(ref->next)
    ref->next->prev = ref->prev;
  if(ref->prev)
    ref->prev->next = ref->next;
  else {
    global_ref_list = ref->next;
    if(global_ref_list)
      global_ref_list->prev = 0;
  }
  if (ref->ref > 0) {
    /* still referenced */
    ref->lvalue = 0;
    ref->prev = ref; //so it doesn't get set to the global list above
    ref->next = ref;
  } else {
    FREE(ref); 
  }
}

ref_t *make_ref (void) {
  ref_t *ref = ALLOCATE(ref_t, TAG_TEMPORARY, "make_ref");
  ref->next = global_ref_list;
  ref->prev = NULL;
  if(ref->next)
    ref->next->prev = ref;
  global_ref_list = ref;
  ref->csp = csp;
  ref->ref = 1;
  return ref;
}

void get_version (char * buff)
{
  sprintf(buff, "FluffOS %s", PATCH_LEVEL);
}

/*
 * Information about assignments of values:
 *
 * There are three types of l-values: Local variables, global variables
 * and array elements.
 *
 * The local variables are allocated on the stack together with the arguments.
 * the register 'frame_pointer' points to the first argument.
 *
 * The global variables must keep their values between executions, and
 * have space allocated at the creation of the object.
 *
 * Elements in arrays are similar to global variables. There is a reference
 * count to the whole array, that states when to deallocate the array.
 * The elements consists of 'svalue_t's, and will thus have to be freed
 * immediately when over written.
 */

/*
 * Push an object pointer on the stack. Note that the reference count is
 * incremented.
 * A destructed object must never be pushed onto the stack.
 */
INLINE
void push_object (object_t * ob)
{
  STACK_INC;

  if (!ob || (ob->flags & O_DESTRUCTED)) {
    *sp = const0u;
    return;
  }

  sp->type = T_OBJECT;
  sp->u.ob = ob;
  add_ref(ob, "push_object");
}

const char * type_name (int c) { 
  int j = 0; 
  int limit = TYPE_CODES_START;

  do {
    if (c & limit) return type_names[j];
    j++;
  } while (!((limit <<= 1) & TYPE_CODES_END));
  /* Oh crap.  Take some time and figure out what we have. */
  switch (c) {
  case T_INVALID: return "*invalid*";
  case T_LVALUE: return "*lvalue*";
  case T_REF: return "*ref*";
  case T_LVALUE_BYTE: return "*lvalue_byte*";
  case T_LVALUE_RANGE: return "*lvalue_range*";
  case T_ERROR_HANDLER: return "*error_handler*";
    IF_DEBUG(case T_FREED: return "*freed*");
  }
  return "*unknown*";
}

/*
 * May current_object shadow object 'ob' ? We rely heavily on the fact that
 * function names are pointers to shared strings, which means that equality
 * can be tested simply through pointer comparison.
 */
static program_t *ffbn_recurse (program_t *, char *, int *, int *);
static program_t *ffbn_recurse2 (program_t *, const char *, int *, int *, int *, int *);

#ifndef NO_SHADOWS

static char *check_shadow_functions (program_t * shadow, program_t * victim) {
  int i;
  int pindex, runtime_index;
  program_t *prog;
  char *fun;
    
  for (i = 0; i < shadow->num_functions_defined; i++) {
    prog = ffbn_recurse(victim, shadow->function_table[i].funcname, &pindex, &runtime_index);
    if (prog && (victim->function_flags[runtime_index] & DECL_NOMASK))
      return prog->function_table[pindex].funcname;
  }

  /* Loop through all the inherits of the program also */
  for (i = 0; i < shadow->num_inherited; i++) {
      fun = check_shadow_functions(shadow->inherit[i].prog, victim);
      if (fun)
          return fun;
  }
  return 0;
}

int validate_shadowing (object_t * ob)
{
  program_t *shadow = current_object->prog, *victim = ob->prog;
  svalue_t *ret;
  char *fun;
    
  if (current_object->shadowing)
    error("shadow: Already shadowing.\n");
  if (current_object->shadowed)
    error("shadow: Can't shadow when shadowed.\n");
#ifndef NO_ENVIRONMENT
  if (current_object->super)
    error("shadow: The shadow must not reside inside another object.\n");
#endif
  if (ob == master_ob)
    error("shadow: cannot shadow the master object.\n");
  if (ob->shadowing)
    error("shadow: Can't shadow a shadow.\n");

  if ((fun = check_shadow_functions(shadow, victim)))
    error("Illegal to shadow 'nomask' function \"%s\".\n", fun);
    
  push_object(ob);
  ret = apply_master_ob(APPLY_VALID_SHADOW, 1);
  if (!(ob->flags & O_DESTRUCTED) && MASTER_APPROVED(ret)) {
    return 1;
  }
  return 0;
}
#endif

/*
 * Push a number on the value stack.
 */
INLINE void
push_number (long n)
{
  STACK_INC;
  sp->type = T_NUMBER;
  sp->subtype = 0;
  sp->u.number = n;
}

INLINE void
push_real (float n)
{
  STACK_INC;
  sp->type = T_REAL;
  sp->u.real = n;
}

/*
 * Push undefined (const0u) onto the value stack.
 */
INLINE
void push_undefined()
{
  STACK_INC;
  *sp = const0u;
}

INLINE_STATIC void push_undefineds (int num)
{
  CHECK_STACK_OVERFLOW(num);
  while (num--) *++sp = const0u;
}

INLINE
void copy_and_push_string (const char * p) {
  STACK_INC;
  sp->type = T_STRING;
  sp->subtype = STRING_MALLOC;
  sp->u.string = string_copy(p, "copy_and_push_string");
}

INLINE
void share_and_push_string (const char * p) {
  STACK_INC;
  sp->type = T_STRING;
  sp->subtype = STRING_SHARED;
  sp->u.string = make_shared_string(p);
}

/*
 * Get address to a valid global variable.
 */
#ifdef DEBUG
INLINE_STATIC svalue_t *find_value (int num)
{
  DEBUG_CHECK2(num >= current_object->prog->num_variables_total,
               "Illegal variable access %d(%d).\n",
               num, current_object->prog->num_variables_total);
  return &current_object->variables[num];
}
#else
#define find_value(num) (&current_object->variables[num])
#endif

INLINE void
free_string_svalue (svalue_t * v)
{
  const char *str = v->u.string;

  if (v->subtype & STRING_COUNTED) {
#ifdef STRING_STATS
    int size = MSTR_SIZE(str);
#endif
    if (DEC_COUNTED_REF(str)) {
      SUB_STRING(size);
      NDBG(BLOCK(str));
      if (v->subtype & STRING_HASHED) {
        SUB_NEW_STRING(size, sizeof(block_t));
        deallocate_string((char *)str);
        CHECK_STRING_STATS;
      } else {
        SUB_NEW_STRING(size, sizeof(malloc_block_t));
        FREE(MSTR_BLOCK(str));
        CHECK_STRING_STATS;
      }
    } else {
      SUB_STRING(size);
      NDBG(BLOCK(str));
    }
  }
}

void unlink_string_svalue (svalue_t * s) {
  char *str;

  switch (s->subtype) {
  case STRING_MALLOC:
    if (MSTR_REF(s->u.string) > 1)
      s->u.string = string_unlink(s->u.string, "unlink_string_svalue");
    break;
  case STRING_SHARED:
    {
      int l = SHARED_STRLEN(s->u.string);
        
      str = new_string(l, "unlink_string_svalue");
      strncpy(str, s->u.string, l + 1);
      free_string(s->u.string);
      s->subtype = STRING_MALLOC;
      s->u.string = str;
      break;
    }
  case STRING_CONSTANT:
    s->u.string = string_copy(s->u.string, "unlink_string_svalue");
    s->subtype = STRING_MALLOC;
    break;
  }
}

/*
 * Free the data that an svalue is pointing to. Not the svalue
 * itself.
 * Use the free_svalue() define to call this
 */
#ifdef DEBUG
INLINE void int_free_svalue (svalue_t * v, char * tag)
#else
     INLINE void int_free_svalue (svalue_t * v)
#endif
{
  /* Marius, 30-Mar-2001: T_FREED could be OR'd in with the type now if the
   * svalue has been 'freed' as an optimization by the F_TRANSFER_LOCAL op.
   * This will allow us to keep the type of the variable known for error
   * handler purposes but not duplicate the free.
   */
  if (v->type == T_STRING) {
    const char *str = v->u.string;
      
    if (v->subtype & STRING_COUNTED) {
#ifdef STRING_STATS
      int size = MSTR_SIZE(str);
#endif
      if (DEC_COUNTED_REF(str)) {
        SUB_STRING(size);
        NDBG(BLOCK(str));
        if (v->subtype & STRING_HASHED) {
          SUB_NEW_STRING(size, sizeof(block_t));
          deallocate_string((char *)str);
          CHECK_STRING_STATS;
        } else {
          SUB_NEW_STRING(size, sizeof(malloc_block_t));
          FREE(MSTR_BLOCK(str));
          CHECK_STRING_STATS;
        }
      } else {
        SUB_STRING(size);
        NDBG(BLOCK(str));
      }
    }
  } else if ((v->type & T_REFED) && !(v->type & T_FREED)) {
#ifdef DEBUG_MACRO
    if (v->type == T_OBJECT)
      debug(d_flag, ("Free_svalue %s (%d) from %s\n", v->u.ob->obname, v->u.ob->ref - 1, tag));
#endif
    if (!(--v->u.refed->ref)) {
      switch (v->type) {
      case T_OBJECT:
        dealloc_object(v->u.ob, "free_svalue");
        break;
      case T_CLASS:
        dealloc_class(v->u.arr);
        break;
      case T_ARRAY:
        if (v->u.arr != &the_null_array)
          dealloc_array(v->u.arr);
        break;
#ifndef NO_BUFFER_TYPE
      case T_BUFFER:
        if (v->u.buf != &null_buf)
          FREE((char *)v->u.buf);
        break;
#endif
      case T_MAPPING:
        dealloc_mapping(v->u.map);
        break;
      case T_FUNCTION:
        dealloc_funp(v->u.fp);
        break;
      case T_REF:
        if (!v->u.ref->lvalue){
          kill_ref(v->u.ref);
        }
        break;
      }
    }
  } else if (v->type == T_ERROR_HANDLER) {
    (*v->u.error_handler)();
  }
#ifdef DEBUG
  else if (v->type == T_FREED) {
    fatal("T_FREED svalue freed.  Previously freed by %s.\n", v->u.string);
  }
  v->type = T_FREED;
  v->u.string = tag;
#endif
}

void process_efun_callback (int narg, function_to_call_t * ftc, int f) {
  int argc = st_num_arg;
  svalue_t *arg = sp - argc + 1 + narg;
  
  if (arg->type == T_FUNCTION) {
    ftc->f.fp = arg->u.fp;
    ftc->ob = 0;
    ftc->narg = argc - narg - 1;
    ftc->args = arg + 1;
  } else {
    ftc->f.str = arg->u.string;
    if (argc < narg + 2) {
      ftc->ob = current_object;
      ftc->narg = 0;
    } else {
      if ((arg+1)->type == T_OBJECT) {
        ftc->ob = (arg+1)->u.ob;
      } else
        if ((arg+1)->type == T_STRING) {
          if (!(ftc->ob = find_object((arg+1)->u.string)) ||
              !object_visible(ftc->ob))
            bad_argument(arg+1, T_STRING | T_OBJECT, 3, f);
        } else
          bad_argument(arg+1, T_STRING | T_OBJECT, 3, f);
      
      ftc->narg = argc - narg - 2;
      ftc->args = arg + 2;
      
      if (ftc->ob->flags & O_DESTRUCTED)
        bad_argument(arg+1, T_STRING | T_OBJECT, 3, f);
    }
  }
}

svalue_t *call_efun_callback (function_to_call_t * ftc, int n) {
  svalue_t *v;
    
  if (ftc->narg)
    push_some_svalues(ftc->args, ftc->narg);
  if (ftc->ob) {
    if (ftc->ob->flags & O_DESTRUCTED)
      error("Object destructed during efun callback.\n");
    v = apply(ftc->f.str, ftc->ob, n + ftc->narg, ORIGIN_EFUN);
  } else
    v = call_function_pointer(ftc->f.fp, n + ftc->narg);
  return v;
}

/*
 * Free several svalues, and free up the space used by the svalues.
 * The svalues must be sequentially located.
 */
INLINE void free_some_svalues (svalue_t * v, int num)
{
  while (num--)
    free_svalue(v + num, "free_some_svalues");
  FREE(v);
}

/*
 * Prepend a slash in front of a string.
 */
char *add_slash (const char * const str)
{
  char *tmp;

  if (str[0] == '<' && strcmp(str + 1, "function>") == 0)
    return string_copy(str, "add_slash");
  tmp = new_string(strlen(str) + 1, "add_slash");
  *tmp = '/';
  strcpy(tmp + 1, str);
  return tmp;
}

/*
 * Assign to a svalue.
 * This is done either when element in array, or when to an identifier
 * (as all identifiers are kept in a array pointed to by the object).
 */

INLINE void assign_svalue_no_free (svalue_t * to, svalue_t * from)
{
  DEBUG_CHECK(from == 0, "Attempt to assign_svalue() from a null ptr.\n");
  DEBUG_CHECK(to == 0, "Attempt to assign_svalue() to a null ptr.\n");
  DEBUG_CHECK((from->type & (from->type - 1)) & ~T_FREED, "from->type is corrupt; >1 bit set.\n");
    
  if (from->type == T_OBJECT && (!from->u.ob || (from->u.ob->flags & O_DESTRUCTED))) {
    *to = const0u;
    return;
  }

  *to = *from;

  if ((to->type & T_FREED) && to->type != T_FREED)
    to->type &= ~T_FREED;

  if (from->type == T_STRING) {
    if (from->subtype & STRING_COUNTED) {
      INC_COUNTED_REF(to->u.string);
      ADD_STRING(MSTR_SIZE(to->u.string));
      NDBG(BLOCK(to->u.string));
    }
  } else if (from->type & T_REFED) {
#ifdef DEBUG_MACRO
    if (from->type == T_OBJECT)
      add_ref(from->u.ob, "assign_svalue_no_free");
    else
#endif
      from->u.refed->ref++;
  }
}

INLINE void assign_svalue (svalue_t * dest, svalue_t * v)
{
  /* First deallocate the previous value. */
  free_svalue(dest, "assign_svalue");
  assign_svalue_no_free(dest, v);
}

INLINE void push_some_svalues (svalue_t * v, int num)
{
  while (num--) push_svalue(v++);
}

/*
 * Copies an array of svalues to another location, which should be
 * free space.
 */
INLINE void copy_some_svalues (svalue_t * dest, svalue_t * v, int num)
{
  while (num--)
    assign_svalue_no_free(dest+num, v+num);
}

INLINE void transfer_push_some_svalues (svalue_t * v, int num)
{
  CHECK_STACK_OVERFLOW(num);
  memcpy(sp + 1, v, num * sizeof(svalue_t));
  sp += num;
}

/*
 * Pop the top-most value of the stack.
 * Don't do this if it is a value that will be used afterwards, as the
 * data may be sent to FREE(), and destroyed.
 */
INLINE void pop_stack()
{
  DEBUG_CHECK(sp < start_of_stack, "Stack underflow.\n");
  free_svalue(sp--, "pop_stack");
}

svalue_t global_lvalue_byte = { T_LVALUE_BYTE };

int lv_owner_type;
refed_t *lv_owner;

/*
 * Compute the address of an array element.
 */
INLINE void push_indexed_lvalue (int code)
{
  int ind;
  svalue_t *lv;
    
  if (sp->type == T_LVALUE) {
    lv = sp->u.lvalue;
    if (!code && lv->type == T_MAPPING) {
      sp--;
      if (!(lv = find_for_insert(lv->u.map, sp, 0)))
        mapping_too_large();
      free_svalue(sp, "push_indexed_lvalue: 1");
      sp->type = T_LVALUE;
      sp->u.lvalue = lv;
#ifdef REF_RESERVED_WORD
      lv_owner_type = T_MAPPING;
      lv_owner = (refed_t *)lv->u.map;
#endif
      return;
    }
  
    if (!((--sp)->type == T_NUMBER))
      error("Illegal type of index\n");

    ind = sp->u.number;

    switch(lv->type) {
    case T_STRING:
      {
        int len = SVALUE_STRLEN(lv);

        if (code) ind = len - ind;
        if (ind >= len || ind < 0)
          error("Index out of bounds in string index lvalue.\n");
        unlink_string_svalue(lv);
        sp->type = T_LVALUE;
        sp->u.lvalue = &global_lvalue_byte;
        global_lvalue_byte.subtype = 0;
        global_lvalue_byte.u.lvalue_byte = (unsigned char *)&lv->u.string[ind];
#ifdef REF_RESERVED_WORD
        lv_owner_type = T_STRING;
        lv_owner = (refed_t *)lv->u.string;
#endif
        break;
      }

#ifndef NO_BUFFER_TYPE       
    case T_BUFFER:
      {
        if (code) ind = lv->u.buf->size - ind;
        if (ind >= lv->u.buf->size || ind < 0)
          error("Buffer index out of bounds.\n");
        sp->type = T_LVALUE;
        sp->u.lvalue = &global_lvalue_byte;
        global_lvalue_byte.subtype = 1;
        global_lvalue_byte.u.lvalue_byte = &lv->u.buf->item[ind];
#ifdef REF_RESERVED_WORD
        lv_owner_type = T_BUFFER;
        lv_owner = (refed_t *)lv->u.buf;
#endif
        break;
      }
#endif
       
    case T_ARRAY:
      {
        if (code) ind = lv->u.arr->size - ind;
        if (ind >= lv->u.arr->size || ind < 0)
          error("Array index out of bounds\n");
        sp->type = T_LVALUE;
        sp->u.lvalue = lv->u.arr->item + ind;
#ifdef REF_RESERVED_WORD
        lv_owner_type = T_ARRAY;
        lv_owner = (refed_t *)lv->u.arr;
#endif
        break;
      }
       
    default:
      if (lv->type == T_NUMBER && !lv->u.number) 
        error("Value being indexed is zero.\n");
      error("Cannot index value of type '%s'.\n", type_name(lv->type));
    }
  } else {
    /* It is now coming from (x <assign_type> y)[index]... = rhs */
    /* Where x is a _valid_ lvalue */
    /* Hence the reference to sp is at least 2 :) */
  
    if (!code && (sp->type == T_MAPPING)) {
      if (!(lv = find_for_insert(sp->u.map, sp-1, 0)))
        mapping_too_large();
      sp->u.map->ref--;
#ifdef REF_RESERVED_WORD
      lv_owner_type = T_MAPPING;
      lv_owner = (refed_t *)sp->u.map;
#endif
      free_svalue(--sp, "push_indexed_lvalue: 2");
      sp->type = T_LVALUE;
      sp->u.lvalue = lv;
      return;
    }
  
    if (!((sp-1)->type == T_NUMBER))
      error("Illegal type of index\n");
  
    ind = (sp-1)->u.number;
  
    switch (sp->type) {
    case T_STRING:
      {
        error("Illegal to make char lvalue from assigned string\n");
        break;
      }
      
#ifndef NO_BUFFER_TYPE
    case T_BUFFER:
      {
        if (code) ind = sp->u.buf->size - ind;
        if (ind >= sp->u.buf->size || ind < 0)
          error("Buffer index out of bounds.\n");
        sp->u.buf->ref--;
#ifdef REF_RESERVED_WORD
        lv_owner_type = T_BUFFER;
        lv_owner = (refed_t *)sp->u.buf;
#endif
        (--sp)->type = T_LVALUE;
        sp->u.lvalue = &global_lvalue_byte;
        global_lvalue_byte.subtype = 1;
        global_lvalue_byte.u.lvalue_byte = (sp+1)->u.buf->item + ind;
        break;
      }
#endif
      
    case T_ARRAY:
      {
        if (code) ind = sp->u.arr->size - ind;
        if (ind >= sp->u.arr->size || ind < 0)
          error("Array index out of bounds.\n");
        sp->u.arr->ref--;
#ifdef REF_RESERVED_WORD
        lv_owner_type = T_ARRAY;
        lv_owner = (refed_t *)sp->u.arr;
#endif
        (--sp)->type = T_LVALUE;
        sp->u.lvalue = (sp+1)->u.arr->item + ind;
        break;
      }
      
    default:
      if (sp->type == T_NUMBER && !sp->u.number) 
        error("Value being indexed is zero.\n");
      error("Cannot index value of type '%s'.\n", type_name(sp->type));
    }
  }
}

static struct lvalue_range {
  int ind1, ind2, size;
  svalue_t *owner;
} global_lvalue_range;

static svalue_t global_lvalue_range_sv = { T_LVALUE_RANGE };

INLINE_STATIC void push_lvalue_range (int code)
{
  int ind1, ind2, size;
  svalue_t *lv;
    
  if (sp->type == T_LVALUE) {
    switch((lv = global_lvalue_range.owner = sp->u.lvalue)->type) {
    case T_ARRAY:
      size = lv->u.arr->size;
      break;
    case T_STRING: {
      size = SVALUE_STRLEN(lv);
      unlink_string_svalue(lv);
      break;
    }
#ifndef NO_BUFFER_TYPE
    case T_BUFFER:
      size = lv->u.buf->size;
      break;
#endif
    default:
      error("Range lvalue on illegal type\n");
      IF_DEBUG(size = 0);
    }
  } else
    error("Range lvalue on illegal type\n");
    
  if (!((--sp)->type == T_NUMBER)) error("Illegal 2nd index type to range lvalue\n");
    
  ind2 = (code & 0x01) ? (size - sp->u.number) : sp->u.number;
  if (++ind2 < 0 || (ind2 > size))
    error("The 2nd index to range lvalue must be >= -1 and < sizeof(indexed value)\n");
    
  if (!((--sp)->type == T_NUMBER)) error("Illegal 1st index type to range lvalue\n");
  ind1 = (code & 0x10) ? (size - sp->u.number) : sp->u.number;
    
  if (ind1 < 0 || ind1 > size)
    error("The 1st index to range lvalue must be >= 0 and <= sizeof(indexed value)\n");
    
  global_lvalue_range.ind1 = ind1;
  global_lvalue_range.ind2 = ind2;
  global_lvalue_range.size = size;
  sp->type = T_LVALUE;
  sp->u.lvalue = &global_lvalue_range_sv;
}

INLINE void copy_lvalue_range (svalue_t * from)
{
  int ind1, ind2, size, fsize;
  svalue_t *owner;
    
  ind1 = global_lvalue_range.ind1;
  ind2 = global_lvalue_range.ind2;
  size = global_lvalue_range.size;
  owner = global_lvalue_range.owner;
    
  switch(owner->type) {
  case T_ARRAY:
    {
      array_t *fv, *dv;
      svalue_t *fptr, *dptr;
      if (from->type != T_ARRAY) error("Illegal rhs to array range lvalue\n");
      
      fv = from->u.arr;
      fptr = fv->item;
      
      if ((fsize = fv->size) == ind2 - ind1) {
        dptr = (owner->u.arr)->item + ind1;
    
        if (fv->ref == 1) {
          /* Transfer the svalues */
          while (fsize--) {
            free_svalue(dptr, "copy_lvalue_range : 1");
            *dptr++ = *fptr++;
          }
          free_empty_array(fv);
        } else {
          while (fsize--) assign_svalue(dptr++, fptr++);
          fv->ref--;
        }
      } else {
        array_t *old_dv = owner->u.arr;
        svalue_t *old_dptr = old_dv->item;
    
        /* Need to reallocate the array */
        dv = allocate_empty_array(size - ind2 + ind1 + fsize);
        dptr = dv->item;
    
        /* ind1 can range from 0 to sizeof(old_dv) */
        while (ind1--) assign_svalue_no_free(dptr++, old_dptr++);
    
        if (fv->ref == 1) {
          while (fsize--) *dptr++ = *fptr++;
          free_empty_array(fv);
        } else {
          while (fsize--) assign_svalue_no_free(dptr++, fptr++);
          fv->ref--;
        }
    
        /* ind2 can range from 0 to sizeof(old_dv) */
        old_dptr = old_dv->item + ind2;
        size -= ind2;
    
        while (size--) assign_svalue_no_free(dptr++, old_dptr++);
        free_array(old_dv);
    
        owner->u.arr = dv;
      }
      break;
    }
  
  case T_STRING:
    {
      if (from->type != T_STRING) error("Illegal rhs to string range lvalue.\n");
      
      if ((fsize = SVALUE_STRLEN(from)) == ind2 - ind1) {
        /* since fsize >= 0, ind2 - ind1 <= strlen(orig string) */
        /* because both of them can only range from 0 to len */
    
        strncpy(((char *)(owner->u.string)) + ind1, from->u.string, fsize);
      } else {
        char *tmp, *dstr = (char *)(owner->u.string);
    
        owner->u.string = tmp = new_string(size - ind2 + ind1 + fsize, "copy_lvalue_range");
        if (ind1 >= 1) {
          strncpy(tmp, dstr, ind1);
          tmp += ind1;
        }
        strcpy(tmp, from->u.string);
        tmp += fsize;
    
        size -= ind2;
        if (size >= 1) {
          strncpy(tmp, dstr + ind2, size);
          *(tmp + size) = 0;
        }
        FREE_MSTR(dstr);
      }
      free_string_svalue(from);
      break;
    }
  
#ifndef NO_BUFFER_TYPE
  case T_BUFFER:
    {
      if (from->type != T_BUFFER) error("Illegal rhs to buffer range lvalue.\n");
      
      if ((fsize = from->u.buf->size) == ind2 - ind1) {
        memcpy((owner->u.buf)->item + ind1, from->u.buf->item, fsize);
      } else {
        buffer_t *b;
        unsigned char *old_item = (owner->u.buf)->item;
        unsigned char *new_item;
    
        b = allocate_buffer(size - ind2 + ind1 + fsize);
        new_item = b->item;
        if (ind1 >= 1) {
          memcpy(b->item, old_item, ind1);
          new_item += ind1;
        }
        memcpy(new_item, from->u.buf, fsize);
        new_item += fsize;
    
        if ((size -= ind2) >= 1)
          memcpy(new_item, old_item + ind2, size);
        free_buffer(owner->u.buf);
        owner->u.buf = b;
      }
      free_buffer(from->u.buf);
      break;
    }
#endif
  }
}

INLINE void assign_lvalue_range (svalue_t * from)
{
  int ind1, ind2, size, fsize;
  svalue_t *owner;
    
  ind1 = global_lvalue_range.ind1;
  ind2 = global_lvalue_range.ind2;
  size = global_lvalue_range.size;
  owner = global_lvalue_range.owner;
    
  switch(owner->type) {
  case T_ARRAY:
    {
      array_t *fv, *dv;
      svalue_t *fptr, *dptr;
      if (from->type != T_ARRAY) error("Illegal rhs to array range lvalue\n");
      
      fv = from->u.arr;
      fptr = fv->item;
      
      if ((fsize = fv->size) == ind2 - ind1) {
        dptr = (owner->u.arr)->item + ind1;
        while (fsize--) assign_svalue(dptr++, fptr++);
      } else {
        array_t *old_dv = owner->u.arr;
        svalue_t *old_dptr = old_dv->item;
    
        /* Need to reallocate the array */
        dv = allocate_empty_array(size - ind2 + ind1 + fsize);
        dptr = dv->item;
    
        /* ind1 can range from 0 to sizeof(old_dv) */
        while (ind1--) assign_svalue_no_free(dptr++, old_dptr++);
    
        while (fsize--) assign_svalue_no_free(dptr++, fptr++);
    
        /* ind2 can range from 0 to sizeof(old_dv) */
        old_dptr = old_dv->item + ind2;
        size -= ind2;
    
        while (size--) assign_svalue_no_free(dptr++, old_dptr++);
        free_array(old_dv);
    
        owner->u.arr = dv;
      }
      break;
    }
  
  case T_STRING:
    {
      if (from->type != T_STRING) error("Illegal rhs to string range lvalue.\n");
      
      if ((fsize = SVALUE_STRLEN(from)) == ind2 - ind1) {
        /* since fsize >= 0, ind2 - ind1 <= strlen(orig string) */
        /* because both of them can only range from 0 to len */
    
        strncpy(((char *)(owner->u.string)) + ind1, from->u.string, fsize);
      } else {
        char *tmp;
        const char *dstr = (char *)(owner->u.string);
    
        owner->u.string = tmp = new_string(size - ind2 + ind1 + fsize, "assign_lvalue_range");
        if (ind1 >= 1) {
          strncpy(tmp, dstr, ind1);
          tmp += ind1;
        }
        strcpy(tmp, from->u.string);
        tmp += fsize;
    
        size -= ind2;
        if (size >= 1) {
          strncpy(tmp, dstr + ind2, size);
          *(tmp + size) = 0;
        }
        FREE_MSTR(dstr);
      }
      break;
    }
  
#ifndef NO_BUFFER_TYPE
  case T_BUFFER:
    {
      if (from->type != T_BUFFER) error("Illegal rhs to buffer range lvalue.\n");
      
      if ((fsize = from->u.buf->size) == ind2 - ind1) {
        memcpy((owner->u.buf)->item + ind1, from->u.buf->item, fsize);
      } else {
        buffer_t *b;
        unsigned char *old_item = (owner->u.buf)->item;
        unsigned char *new_item;
    
        b = allocate_buffer(size - ind2 + ind1 + fsize);
        new_item = b->item;
        if (ind1 >= 1) {
          memcpy(b->item, old_item, ind1);
          new_item += ind1;
        }
        memcpy(new_item, from->u.buf, fsize);
        new_item += fsize;
    
        if ((size -= ind2) >= 1)
          memcpy(new_item, old_item + ind2, size);
        free_buffer(owner->u.buf);
        owner->u.buf = b;
      }
      break;
    }
#endif
  }
}

/*
 * Deallocate 'n' values from the stack.
 */
INLINE void
pop_n_elems (int n)
{
  DEBUG_CHECK1(n < 0, "pop_n_elems: %d elements.\n", n);
  while (n--) {
    pop_stack();
  }
}

/*
 * Deallocate 2 values from the stack.
 */
INLINE void
pop_2_elems()
{
  free_svalue(sp--, "pop_2_elems");
  DEBUG_CHECK(sp < start_of_stack, "Stack underflow.\n");
  free_svalue(sp--, "pop_2_elems");
}

/*
 * Deallocate 3 values from the stack.
 */
INLINE void
pop_3_elems()
{
  free_svalue(sp--, "pop_3_elems");
  free_svalue(sp--, "pop_3_elems");
  DEBUG_CHECK(sp < start_of_stack, "Stack underflow.\n");
  free_svalue(sp--, "pop_3_elems");
}

void bad_arg (int arg, int instr)
{
  error("Bad Argument %d to %s()\n", arg, query_instr_name(instr));
}

void bad_argument (svalue_t * val, int type, int arg, int instr)
{
  outbuffer_t outbuf;
  int flag = 0;
  int j = TYPE_CODES_START;
  int k = 0;

  outbuf_zero(&outbuf);
  outbuf_addv(&outbuf, "Bad argument %d to %s%s\nExpected: ", arg, 
              query_instr_name(instr), (instr < BASE ? "" : "()"));

  do {
    if (type & j) {
      if (flag) outbuf_add(&outbuf, " or ");
      else flag = 1;
      outbuf_add(&outbuf, type_names[k]);
    }
    k++;
  } while (!((j <<= 1) & TYPE_CODES_END));

  outbuf_add(&outbuf, " Got: ");
  svalue_to_string(val, &outbuf, 0, 0, 0);
  outbuf_add(&outbuf, ".\n");
  outbuf_fix(&outbuf);
  error_needs_free(outbuf.buffer);
}

INLINE void
push_control_stack (int frkind)
{
  if (csp == &control_stack[CFG_MAX_CALL_DEPTH - 1]) {
    too_deep_error = 1;
    error("Too deep recursion.\n");
  }
  csp++;
  csp->caller_type = caller_type;
  csp->ob = current_object;
  csp->framekind = frkind;
  csp->prev_ob = previous_ob;
  csp->fp = fp;
  csp->prog = current_prog;
  csp->pc = pc;
  csp->function_index_offset = function_index_offset;
  csp->variable_index_offset = variable_index_offset;
}

/*
 * Pop the control stack one element, and restore registers.
 * extern_call must not be modified here, as it is used imediately after pop.
 */
void pop_control_stack()
{
  DEBUG_CHECK(csp == (control_stack - 1),
              "Popped out of the control stack\n");
#ifdef PROFILE_FUNCTIONS
  if ((csp->framekind & FRAME_MASK) == FRAME_FUNCTION) {
    long secs, usecs, dsecs;
    function_t *cfp = &current_prog->function_table[csp->fr.table_index];
    int stof = 0;

    get_cpu_times((unsigned long *) &secs, (unsigned long *) &usecs);
    dsecs = (((secs - csp->entry_secs) * 1000000)
             + (usecs - csp->entry_usecs));
    cfp->self += dsecs;
    
    while((csp-stof) != control_stack){
      if (((csp-stof-1)->framekind & FRAME_MASK) == FRAME_FUNCTION) {
	(csp-stof)->prog->function_table[(csp-stof-1)->fr.table_index].children += dsecs;
	break;
      }
      stof++;
    }
  }
#endif
  current_object = csp->ob;
  current_prog = csp->prog;
  previous_ob = csp->prev_ob;
  caller_type = csp->caller_type;
  pc = csp->pc;
  fp = csp->fp;
  function_index_offset = csp->function_index_offset;
  variable_index_offset = csp->variable_index_offset;
  csp--;
}

/*
 * Push a pointer to a array on the stack. Note that the reference count
 * is incremented. Newly created arrays normally have a reference count
 * initialized to 1.
 */
INLINE void push_array (array_t * v)
{
  STACK_INC;
  v->ref++;
  sp->type = T_ARRAY;
  sp->u.arr = v;
}

INLINE void push_refed_array (array_t * v)
{
  STACK_INC;
  sp->type = T_ARRAY;
  sp->u.arr = v;
}

#ifndef NO_BUFFER_TYPE
INLINE void
push_buffer (buffer_t * b)
{
  STACK_INC;
  b->ref++;
  sp->type = T_BUFFER;
  sp->u.buf = b;
}

INLINE void
push_refed_buffer (buffer_t * b)
{
  STACK_INC;
  sp->type = T_BUFFER;
  sp->u.buf = b;
}
#endif

/*
 * Push a mapping on the stack.  See push_array(), above.
 */
INLINE void
push_mapping (mapping_t * m)
{
  STACK_INC;
  m->ref++;
  sp->type = T_MAPPING;
  sp->u.map = m;
}

INLINE void
push_refed_mapping (mapping_t * m)
{
  STACK_INC;
  sp->type = T_MAPPING;
  sp->u.map = m;
}

/*
 * Push a class on the stack.  See push_array(), above.
 */
INLINE void
push_class (array_t * v)
{
  STACK_INC;
  v->ref++;
  sp->type = T_CLASS;
  sp->u.arr = v;
}

INLINE void
push_refed_class (array_t * v)
{
  STACK_INC;
  sp->type = T_CLASS;
  sp->u.arr = v;
}

/*
 * Push a string on the stack that is already malloced.
 */
INLINE void push_malloced_string (char * p)
{
  STACK_INC;
  sp->type = T_STRING;
  sp->u.string = p;
  sp->subtype = STRING_MALLOC;
}

/*
 * Pushes a known shared string.  Note that this references, while 
 * push_malloced_string doesn't.
 */
INLINE void push_shared_string (char * p) {
  STACK_INC;
  sp->type = T_STRING;
  sp->u.string = p;
  sp->subtype = STRING_SHARED;
  ref_string(p);
}

/*
 * Push a string on the stack that is already constant.
 */
INLINE
void push_constant_string (const char * p)
{
  STACK_INC;
  sp->type = T_STRING;
  sp->subtype = STRING_CONSTANT;
  sp->u.string = p;
}

#ifdef TRACE
static void do_trace_call (int offset)
{
  do_trace("Call direct ", current_prog->function_table[offset].funcname, " ");
  if (TRACEHB) {
    if (TRACETST(TRACE_ARGS)) {
      int i, n;
      
      n = current_prog->function_table[offset].num_arg;

      add_vmessage(command_giver, " with %d arguments: ", n);
      for (i = n - 1; i >= 0; i--) {
        print_svalue(&sp[-i]);
        add_message(command_giver, " ", 1);
      }
    }
    add_message(command_giver, "\n", 1);
  }
}
#endif

/*
 * Argument is the function to execute. If it is defined by inheritance,
 * then search for the real definition, and return it.
 * There is a number of arguments on the stack. Normalize them and initialize
 * local variables, so that the called function is pleased.
 */
INLINE void setup_variables (int actual, int local, int num_arg) {
  int tmp;
    
  if ((tmp = actual - num_arg) > 0) {
    /* Remove excessive arguments */
    pop_n_elems(tmp);
    push_undefineds(local);
  } else {
    /* Correct number of arguments and local variables */
    push_undefineds(local - tmp);
  }
  fp = sp - (csp->num_local_variables = local + num_arg) + 1;
}

INLINE_STATIC void setup_varargs_variables (int actual, int local, int num_arg) {
  array_t *arr;
  if (actual >= num_arg) {
    int n = actual - num_arg + 1;
    /* Aggregate excessive arguments */
    arr = allocate_empty_array(n);
    while (n--)
      arr->item[n] = *sp--;
  } else {
    /* Correct number of arguments and local variables */
    push_undefineds(num_arg - 1 - actual);
    arr = &the_null_array;
  }
  push_refed_array(arr);
  push_undefineds(local);
  fp = sp - (csp->num_local_variables = local + num_arg) + 1;
}

INLINE function_t *
setup_new_frame (int findex)
{
  function_t *func_entry;
  register int low, high, mid;
  int flags;

  function_index_offset = variable_index_offset = 0;

  /* Walk up the inheritance tree to the real definition */ 
  if (current_prog->function_flags[findex] & FUNC_ALIAS) {
    findex = current_prog->function_flags[findex] & ~FUNC_ALIAS;
  }
    
  while (current_prog->function_flags[findex] & FUNC_INHERITED) {
    low = 0;
    high = current_prog->num_inherited -1;
      
    while (high > low) {
      mid = (low + high + 1) >> 1;
      if (current_prog->inherit[mid].function_index_offset > findex)
        high = mid -1;
      else low = mid;
    }
    findex -= current_prog->inherit[low].function_index_offset;
    function_index_offset += current_prog->inherit[low].function_index_offset;
    variable_index_offset += current_prog->inherit[low].variable_index_offset;
    current_prog = current_prog->inherit[low].prog;
  }
    
  flags = current_prog->function_flags[findex];
    
  findex -= current_prog->last_inherited;
    
  func_entry = current_prog->function_table + findex;
  csp->fr.table_index = findex;
#ifdef PROFILE_FUNCTIONS
  get_cpu_times(&(csp->entry_secs), &(csp->entry_usecs));
  current_prog->function_table[findex].calls++;
#endif

  /* Remove excessive arguments */
  if (flags & FUNC_TRUE_VARARGS) {
    setup_varargs_variables(csp->num_local_variables, 
                            func_entry->num_local, 
                            func_entry->num_arg);
  }
  else
    setup_variables(csp->num_local_variables,
                    func_entry->num_local, 
                    func_entry->num_arg);
#ifdef TRACE
  tracedepth++;
  if (TRACEP(TRACE_CALL)) {
    do_trace_call(findex);
  }
#endif
  return &current_prog->function_table[findex];
}

INLINE function_t *setup_inherited_frame (int findex)
{
  function_t *func_entry;
  register int low, high, mid;
  int flags;

  /* Walk up the inheritance tree to the real definition */ 
  if (current_prog->function_flags[findex] & FUNC_ALIAS) {
    findex = current_prog->function_flags[findex] & ~FUNC_ALIAS;
  }
    
  while (current_prog->function_flags[findex] & FUNC_INHERITED) {
    low = 0;
    high = current_prog->num_inherited -1;
  
    while (high > low) {
      mid = (low + high + 1) >> 1;
      if (current_prog->inherit[mid].function_index_offset > findex)
        high = mid -1;
      else low = mid;
    }
    findex -= current_prog->inherit[low].function_index_offset;
    function_index_offset += current_prog->inherit[low].function_index_offset;
    variable_index_offset += current_prog->inherit[low].variable_index_offset;
    current_prog = current_prog->inherit[low].prog;
  }
    
  flags = current_prog->function_flags[findex];
  findex -= current_prog->last_inherited;

  func_entry = current_prog->function_table + findex;
  csp->fr.table_index = findex;
#ifdef PROFILE_FUNCTIONS
  get_cpu_times(&(csp->entry_secs), &(csp->entry_usecs));
  current_prog->function_table[findex].calls++;
#endif

  /* Remove excessive arguments */
  if (flags & FUNC_TRUE_VARARGS)
    setup_varargs_variables(csp->num_local_variables, 
                            func_entry->num_local, 
                            func_entry->num_arg);
  else
    setup_variables(csp->num_local_variables,
                    func_entry->num_local, 
                    func_entry->num_arg);
#ifdef TRACE
  tracedepth++;
  if (TRACEP(TRACE_CALL)) {
    do_trace_call(findex);
  }
#endif
  return &current_prog->function_table[findex];
}

#ifdef DEBUG
/* This function is called at the end of every complete LPC statement, so
 * it is a good place to insert debugging code to find out where during
 * LPC code certain assertions fail, etc
 */
void break_point()
{
  /* The current implementation of foreach leaves some stuff lying on the
     stack */
  if (!stack_in_use_as_temporary && sp - fp - csp->num_local_variables + 1 != 0)
    fatal("Bad stack pointer.\n");
}
#endif

program_t fake_prog = { "<driver>" };
unsigned char fake_program = F_RETURN;

/*
 * Very similar to push_control_stack() [which see].  The purpose of this is
 * to insert an frame containing the object which defined a function pointer
 * in cases where it would otherwise not be on the call stack.  This 
 * preserves the idea that function pointers calls happen 'through' the
 * object that define the function pointer. 
 * These frames are the ones that show up as <function> in error traces.
 */
void setup_fake_frame (funptr_t * fun) {
  if (csp == &control_stack[CFG_MAX_CALL_DEPTH-1]) {
    too_deep_error = 1;
    error("Too deep recursion.\n");
  }
  csp++;
  csp->caller_type = caller_type;
  csp->framekind = FRAME_FAKE | FRAME_OB_CHANGE;
  csp->fr.funp = fun;
  csp->ob = current_object;
  csp->prev_ob = previous_ob;
  csp->fp = fp;
  csp->prog = current_prog;
  csp->pc = pc;
  pc = (char *)&fake_program;
  csp->function_index_offset = function_index_offset;
  csp->variable_index_offset = variable_index_offset;
  caller_type = ORIGIN_FUNCTION_POINTER;
  csp->num_local_variables = 0;
  current_prog = &fake_prog;
  previous_ob = current_object;
  current_object = fun->hdr.owner;
}

/* Remove a fake frame added by setup_fake_frame().  Basically just a
 * specialized version of pop_control_stack().
 */
void remove_fake_frame() {
  DEBUG_CHECK(csp == (control_stack - 1),
              "Popped out of the control stack\n");
  current_object = csp->ob;
  current_prog = csp->prog;
  previous_ob = csp->prev_ob;
  caller_type = csp->caller_type;
  pc = csp->pc;
  fp = csp->fp;
  function_index_offset = csp->function_index_offset;
  variable_index_offset = csp->variable_index_offset;
  csp--;
}

/*
 * When a array is given as argument to an efun, all items have to be
 * checked if there would be a destructed object.
 * A bad problem currently is that a array can contain another array, so this
 * should be tested too. But, there is currently no prevention against
 * recursive arrays, which means that this can not be tested. Thus, MudOS
 * may crash if a array contains a array that contains a destructed object
 * and this top-most array is used as an argument to an efun.
 */
/* MudOS won't crash when doing simple operations like assign_svalue
 * on a destructed object. You have to watch out, of course, that you don't
 * apply a function to it.
 * to save space it is preferable that destructed objects are freed soon.
 *   amylaar
 */
void check_for_destr (array_t * v)
{
  int i = v->size;
    
  while (i--) {
    if ((v->item[i].type == T_OBJECT) && (v->item[i].u.ob->flags & O_DESTRUCTED)) {
      free_svalue(&v->item[i], "check_for_destr");
      v->item[i] = const0u;
    }
  }
}

/* do_loop_cond() coded by John Garnett, 1993/06/01
   
Optimizes these four cases (with 'int i'):
   
1) for (expr0; i < integer_variable; expr2) statement;
2) for (expr0; i < integer_constant; expr2) statement;
3) while (i < integer_variable) statement;
4) while (i < integer_constant) statement;
*/

INLINE_STATIC void do_loop_cond_local()
{
  svalue_t *s1, *s2;
  int i;
    
  s1 = fp + EXTRACT_UCHAR(pc++); /* a from (a < b) */
  s2 = fp + EXTRACT_UCHAR(pc++);
  switch(s1->type | s2->type) {
  case T_NUMBER: 
    i = s1->u.number < s2->u.number;
    break;
  case T_REAL:
    i = s1->u.real < s2->u.real;
    break;
  case T_STRING:
    i = (strcmp(s1->u.string, s2->u.string) < 0);
    break;
  case T_NUMBER|T_REAL:
    if (s1->type == T_NUMBER) i = s1->u.number < s2->u.real;
    else i = s1->u.real < s2->u.number;
    break;
  default:
    if (s1->type == T_OBJECT && (s1->u.ob->flags & O_DESTRUCTED)) {
      free_object(&s1->u.ob, "do_loop_cond:1");
      *s1 = const0u;
    }
    if (s2->type == T_OBJECT && (s2->u.ob->flags & O_DESTRUCTED)) {
      free_object(&s2->u.ob, "do_loop_cond:2");
      *s2 = const0u;
    }
    if (s1->type == T_NUMBER && s2->type == T_NUMBER) {
      i = s1->u.number < s2->u.number;
      break;
    }
    switch(s1->type) {
    case T_NUMBER:
    case T_REAL:
      error("2nd argument to < is not numeric when the 1st is.\n");
    case T_STRING:
      error("2nd argument to < is not string when the 1st is.\n");
    default:
      error("Bad 1st argument to <.\n");
    }
    i = 0;
  }
  if (i) {
    unsigned short offset;
  
    COPY_SHORT(&offset, pc);
    pc -= offset;
  } else pc += 2;
}

INLINE_STATIC void do_loop_cond_number()
{
  svalue_t *s1;
  long i;
    
  s1 = fp + EXTRACT_UCHAR(pc++); /* a from (a < b) */
  LOAD_INT(i, pc);
  if (s1->type == T_NUMBER) {
    if (s1->u.number < i) {
      unsigned short offset;
      
      COPY_SHORT(&offset, pc);
      pc -= offset;
    } else pc += 2;
  } else if (s1->type == T_REAL) {
    if (s1->u.real < i) {
      unsigned short offset;
      
      COPY_SHORT(&offset, pc);
      pc -= offset;
    } else pc += 2;
  } else error("Right side of < is a number, left side is not.\n");
}

#ifdef DEBUG_MACRO
static void show_lpc_line (char * f, int l) {
  static FILE *fp = 0;
  static char *fn = 0;
  static int lastline, offset;
  static char buf[32768], *p;
  static int n;
  int dir;
  char *q;

  if (fn == f && l == lastline) return;
  printf("LPC: %s:%i\n", f, l);
  if (!(debug_level & DBG_LPC_line)) {
    fn = f;
    lastline = l;
    return;
  }
    
  if (fn != f) {
    if (fp) fclose(fp);
    fp = fopen(f, "r");
    if (!fp) goto bail_hard;
    fn = f;
    lastline = 1;
    offset = 0;
    n = fread(buf, 1, 32767, fp);
    p = buf;
    buf[n] = 0;
  }

  dir = (lastline < l ? 1 : -1);
  while (lastline - l != 0) {
    while (p >= buf && *p && *p != '\n') {
      p += dir;
    }

    if (p < buf || !*p) {
      if (dir == -1) {
        if (offset == 0) goto bail_hard;
        n = 32767;
        if (n > offset) n = offset;
      } else {
        n = 32767;
      }
      offset += dir * n;
      if (fseek(fp, offset, SEEK_SET) == -1) goto bail_hard;
      n = fread(buf, 1, n, fp);
      if (n <= 0) goto bail_hard;
      buf[n] = 0;
      p = (dir == 1 ? &buf[n-1] : buf);
    } else {
      p += dir;
      lastline += dir;
    }
  }
  if (dir == -1) {
    while (*p != '\n') {
      p--;
      if (p < buf) {
        if (offset == 0) { p++; break; }
        n = 32767;
        if (n > offset) n = offset;
        offset -= n;
        if (fseek(fp, offset, SEEK_SET) == -1) goto bail_hard;
        n = fread(buf, 1, 32767, fp);
        if (n == -1) goto bail_hard;
        buf[n] = 0;
        p = &buf[n-1];
      }
    }
  }
  q = p;
  while (1) {
    while (*q) {
      putchar(*q);
      if (*q++ == '\n') return;
    }
    offset += 32767;
    if (fseek(fp, offset, SEEK_SET) == -1) goto bail_hard;
    n = fread(buf, 1, 32767, fp);
    if (n == -1) goto bail_hard;
    buf[n] = 0;
    p = buf;
  }
  return;

 bail_hard:
  fn = 0;
  return;
}
#endif

/*
 * Evaluate instructions at address 'p'. All program offsets are
 * to current_prog->program. 'current_prog' must be setup before
 * call of this function.
 *
 * There must not be destructed objects on the stack. The destruct_object()
 * function will automatically remove all occurences. The effect is that
 * all called efuns knows that they won't have destructed objects as
 * arguments.
 */
#ifdef TRACE_CODE
static int previous_instruction[60];
static int stack_size[60];
static char *previous_pc[60];
static int last;
#endif

void
eval_instruction (char * p)
{
#ifdef DEBUG
  int num_arg;
#endif
  long i, n;
  float real;
  svalue_t *lval;
  int instruction;
#if defined(TRACE_CODE) || defined(TRACE) || defined(OPCPROF) || defined(OPCPROF_2D)
  int real_instruction;
#endif
  unsigned short offset;
  static func_t *oefun_table = efun_table - BASE + ONEARG_MAX;
#ifndef DEBUG
  static func_t *ooefun_table = efun_table - BASE;
#endif
  static instr_t *instrs2 = instrs + ONEARG_MAX;
    
  IF_DEBUG(svalue_t *expected_stack);

  /* Next F_RETURN at this level will return out of eval_instruction() */
  csp->framekind |= FRAME_EXTERNAL;
  pc = p;
  while (1) {
#  ifdef DEBUG_MACRO
    if (debug_level & DBG_LPC) { 
      char *f;
      int l;
      /* this could be much more efficient ... */
      get_line_number_info(&f, &l);
      show_lpc_line(f, l);
    }
#  endif
    instruction = EXTRACT_UCHAR(pc++);
#if defined(TRACE_CODE) || defined(TRACE) || defined(OPCPROF) || defined(OPCPROF_2D)
    if (instruction >= F_EFUN0 && instruction <= F_EFUNV)
      real_instruction = EXTRACT_UCHAR(pc) + ONEARG_MAX;
    else
      real_instruction = instruction;
#  ifdef TRACE_CODE
    previous_instruction[last] = real_instruction;
    previous_pc[last] = pc - 1;
    stack_size[last] = sp - fp - csp->num_local_variables;
    last = (last + 1) % (sizeof previous_instruction / sizeof(int));
#  endif
#  ifdef TRACE
    if (TRACEP(TRACE_EXEC)) {
      do_trace("Exec ", query_instr_name(real_instruction), "\n");
    }
#  endif
#  ifdef OPCPROF
    if (real_instruction < BASE)
      opc_eoper[real_instruction]++;
    else
      opc_efun[real_instruction-BASE].count++;
#  endif
#  ifdef OPCPROF_2D
    if (real_instruction < BASE) {
      if (last_eop) opc_eoper_2d[last_eop][real_instruction]++;
      last_eop = real_instruction;
    } else {
      if (last_eop) opc_eoper_2d[last_eop][BASE]++;
      last_eop = BASE;
    }
#  endif
#endif
    if (outoftime) {
      debug_message("object /%s: eval_cost too big %d\n", 
                    current_object->obname, max_cost);
      set_eval(max_cost);
      max_eval_error = 1;
      error("Too long evaluation. Execution aborted.\n");
    }
    /*
     * Execute current instruction. Note that all functions callable from
     * LPC must return a value. This does not apply to control
     * instructions, like F_JUMP.
     */

    switch (instruction) {
    case F_PUSH:    /* Push a number of things onto the stack */
      n = EXTRACT_UCHAR(pc++);
      while (n--) {
        i = EXTRACT_UCHAR(pc++);
        switch (i & PUSH_WHAT) {
        case PUSH_STRING:
          DEBUG_CHECK1((i & PUSH_MASK) >= current_prog->num_strings,
                       "string %d out of range in F_STRING!\n",
                       i & PUSH_MASK);
          push_shared_string(current_prog->strings[i & PUSH_MASK]);
          break;
        case PUSH_LOCAL:
          lval = fp + (i & PUSH_MASK);
          DEBUG_CHECK((fp - lval) >= csp->num_local_variables,
                      "Tried to push non-existent local\n");
          if ((lval->type == T_OBJECT) && (lval->u.ob->flags & O_DESTRUCTED))
            assign_svalue(lval, &const0u);
          push_svalue(lval);
          break;
        case PUSH_GLOBAL:
          lval = find_value(((i & PUSH_MASK) + variable_index_offset));
          if ((lval->type == T_OBJECT) && (lval->u.ob->flags & O_DESTRUCTED))
            assign_svalue(lval, &const0u);
          push_svalue(lval);
          break;
        case PUSH_NUMBER:
          push_number(i & PUSH_MASK);
          break;
        }
      }
      break;
    case F_INC:
      DEBUG_CHECK(sp->type != T_LVALUE,
                  "non-lvalue argument to ++\n");
      lval = (sp--)->u.lvalue;
      switch (lval->type) {
      case T_NUMBER:
        lval->u.number++;
        break;
      case T_REAL:
        lval->u.real++;
        break;
      case T_LVALUE_BYTE:
        if (global_lvalue_byte.subtype == 0 &&
            *global_lvalue_byte.u.lvalue_byte == (unsigned char)255)
          error("Strings cannot contain 0 bytes.\n");
        ++*global_lvalue_byte.u.lvalue_byte;
        break;
      default:
        error("++ of non-numeric argument\n");
      }
      break;
    case F_WHILE_DEC:
      {
        svalue_t *s;

        s = fp + EXTRACT_UCHAR(pc++);
        if (s->type == T_NUMBER) {
          i = s->u.number--;
        } else if (s->type == T_REAL) {
          i = s->u.real--;
        } else {
          error("-- of non-numeric argument\n");
        }
        if (i) {
          COPY_SHORT(&offset, pc);
          pc -= offset;
        } else {
          pc += 2;
        }
      }
      break;
    case F_LOCAL_LVALUE:
      STACK_INC;
      sp->type = T_LVALUE;
      sp->u.lvalue = fp + EXTRACT_UCHAR(pc++);
      break;
#ifdef REF_RESERVED_WORD
    case F_MAKE_REF:
      {
        ref_t *ref;
        int op = EXTRACT_UCHAR(pc++);
        /* global and local refs need no protection since they are
         * guaranteed to outlive the current scope.  Lvalues
         * inside structures may not, however ...  
         */
        ref = make_ref();
        ref->lvalue = sp->u.lvalue;
        if (op != F_GLOBAL_LVALUE && op != F_LOCAL_LVALUE && op != F_REF_LVALUE) {
          ref->sv.type = lv_owner_type;
          ref->sv.subtype = STRING_MALLOC; /* ignored if non-string */
          if (lv_owner_type == T_STRING) {
            ref->sv.u.string = (char *)lv_owner;
            INC_COUNTED_REF(lv_owner);
            ADD_STRING(MSTR_SIZE(lv_owner));
            NDBG(BLOCK(lv_owner));
          } else {
            ref->sv.u.refed = lv_owner;
            lv_owner->ref++;
            if (lv_owner_type == T_MAPPING)
              ((mapping_t *)lv_owner)->count |= MAP_LOCKED;
          }
        } else
          ref->sv.type = T_NUMBER;
        sp->type = T_REF;
        sp->u.ref = ref;
        break;
      }
    case F_KILL_REFS:
      {
        int num = EXTRACT_UCHAR(pc++);
        while (num--) 
          kill_ref(global_ref_list);
        break;
      }
    case F_REF:
      {
        svalue_t *s = fp + EXTRACT_UCHAR(pc++);
        svalue_t *reflval;
      
        if (s->type == T_REF) {
          reflval = s->u.ref->lvalue;
          if (!reflval)
            error("Reference is invalid.\n");
        
          if (reflval->type == T_LVALUE_BYTE) {
            push_number(*global_lvalue_byte.u.lvalue_byte);
            break;
          }
        } else {
          error("Non-reference value passed as reference argument.\n");
        }
    
        if (reflval->type == T_OBJECT && (reflval->u.ob->flags & O_DESTRUCTED))
          assign_svalue(reflval, &const0u);
        push_svalue(reflval);
      
        break;
      }
    case F_REF_LVALUE:
      {
        svalue_t *s = fp + EXTRACT_UCHAR(pc++);
      
        if (s->type == T_REF) {
          if (s->u.ref->lvalue) {
            STACK_INC;
            sp->type = T_LVALUE;
            sp->u.lvalue = s->u.ref->lvalue;
          } else
            error("Reference is invalid.\n");
        } else
          error("Non-reference value passed as reference argument.\n");
        break;
      }
#endif  
    case F_SHORT_INT:
      {
        short s;
      
        LOAD_SHORT(s, pc);
        push_number(s);
        break;
      }
    case F_NUMBER:
      LOAD_INT(i, pc);
      push_number(i);
      break;
    case F_REAL:
      LOAD_FLOAT(real, pc);
      push_real(real);
      break;
    case F_BYTE:
      push_number(EXTRACT_UCHAR(pc++));
      break;
    case F_NBYTE:
      push_number(-(EXTRACT_UCHAR(pc++)));
      break;
#ifdef F_JUMP_WHEN_NON_ZERO
    case F_JUMP_WHEN_NON_ZERO:
      if ((i = (sp->type == T_NUMBER)) && (sp->u.number == 0))
        pc += 2;
      else {
        COPY_SHORT(&offset, pc);
        pc = current_prog->program + offset;
      }
      if (i) {
        sp--;   /* when sp is an integer svalue, its cheaper
                 * to do this */
      } else {
        pop_stack();
      }
      break;
#endif
    case F_BRANCH:    /* relative offset */
      COPY_SHORT(&offset, pc);
      pc += offset;
      break;
    case F_BBRANCH:   /* relative offset */
      COPY_SHORT(&offset, pc);
      pc -= offset;
      break;
    case F_BRANCH_NE:
      f_ne();
      if ((sp--)->u.number) {
        COPY_SHORT(&offset, pc);
        pc += offset;
      } else
        pc += 2;
      break;
    case F_BRANCH_GE:
      f_ge();
      if ((sp--)->u.number) {
        COPY_SHORT(&offset, pc);
        pc += offset;
      } else
        pc += 2;
      break;
    case F_BRANCH_LE:
      f_le();
      if ((sp--)->u.number) {
        COPY_SHORT(&offset, pc);
        pc += offset;
      } else
        pc += 2;
      break;
    case F_BRANCH_EQ:
      f_eq();
      if ((sp--)->u.number) {
        COPY_SHORT(&offset, pc);
        pc += offset;
      } else
        pc += 2;
      break;
    case F_BBRANCH_LT:
      f_lt();
      if ((sp--)->u.number) {
        COPY_SHORT(&offset, pc);
        pc -= offset;
      } else
        pc += 2;
      break;
    case F_BRANCH_WHEN_ZERO: /* relative offset */
      if (sp->type == T_NUMBER) {
        if (!((sp--)->u.number)) {
          COPY_SHORT(&offset, pc);
          pc += offset;
          break;
        }
      } else pop_stack();
      pc += 2;    /* skip over the offset */
      break;
    case F_BRANCH_WHEN_NON_ZERO: /* relative offset */
      if (sp->type == T_NUMBER) {
        if (!((sp--)->u.number)) {
          pc += 2;
          break;
        }
      } else pop_stack();
      COPY_SHORT(&offset, pc);
      pc += offset;
      break;
    case F_BBRANCH_WHEN_ZERO: /* relative backwards offset */
      if (sp->type == T_NUMBER) {
        if (!((sp--)->u.number)) {
          COPY_SHORT(&offset, pc);
          pc -= offset;
          break;
        }
      } else pop_stack();
      pc += 2;
      break;
    case F_BBRANCH_WHEN_NON_ZERO: /* relative backwards offset */
      if (sp->type == T_NUMBER) {
        if (!((sp--)->u.number)) {
          pc += 2;
          break;
        }
      } else pop_stack();
      COPY_SHORT(&offset, pc);
      pc -= offset;
      break;
    case F_LOR:
      /* replaces F_DUP; F_BRANCH_WHEN_NON_ZERO; F_POP */
      if (sp->type == T_NUMBER) {
        if (!sp->u.number) {
          pc += 2;
          sp--;
          break;
        }
      }
      COPY_SHORT(&offset, pc);
      pc += offset;
      break;
    case F_LAND:
      /* replaces F_DUP; F_BRANCH_WHEN_ZERO; F_POP */
      if (sp->type == T_NUMBER) {
        if (!sp->u.number) {
          COPY_SHORT(&offset, pc);
          pc += offset;
          break;
        }
        sp--;
      } else pop_stack();
      pc += 2;
      break;
    case F_LOOP_INCR: /* this case must be just prior to
                       * F_LOOP_COND */
      {
        svalue_t *s;
    
        s = fp + EXTRACT_UCHAR(pc++);
        if (s->type == T_NUMBER) {
          s->u.number++;
        } else if (s->type == T_REAL) {
          s->u.real++;
        } else {
          error("++ of non-numeric argument\n");
        }
      }
      if (*pc == F_LOOP_COND_LOCAL) {
        pc++;
        do_loop_cond_local();
      } else if (*pc == F_LOOP_COND_NUMBER) {
        pc++;
        do_loop_cond_number();
      }
      break;
    case F_LOOP_COND_LOCAL:
      do_loop_cond_local();
      break;
    case F_LOOP_COND_NUMBER:
      do_loop_cond_number();
      break;
    case F_TRANSFER_LOCAL:
      {
        svalue_t *s;
    
        s = fp + EXTRACT_UCHAR(pc++);
        DEBUG_CHECK((fp-s) >= csp->num_local_variables,
                    "Tried to push non-existent local\n");
        if ((s->type == T_OBJECT) && (s->u.ob->flags & O_DESTRUCTED))
          assign_svalue(s, &const0u);

        STACK_INC;
        *sp = *s;

        /* The optimizer has asserted this won't be used again.  Make
         * it look like a number to avoid double frees. */
        s->type |= T_FREED;
        break;
      }
    case F_LOCAL:
      {
        svalue_t *s;
    
        s = fp + EXTRACT_UCHAR(pc++);
        DEBUG_CHECK((fp-s) >= csp->num_local_variables,
                    "Tried to push non-existent local\n");
    
        /*
         * If variable points to a destructed object, replace it
         * with 0, otherwise, fetch value of variable.
         */
        if ((s->type == T_OBJECT) && (s->u.ob->flags & O_DESTRUCTED))
          assign_svalue(s, &const0u);
        push_svalue(s);
        break;
      }
    case F_LT:
      f_lt();
      break;
    case F_ADD:
      {
        switch (sp->type) {
#ifndef NO_BUFFER_TYPE
        case T_BUFFER:
          {
            if (!((sp-1)->type == T_BUFFER)) {
              error("Bad type argument to +. Had %s and %s.\n",
                    type_name((sp - 1)->type), type_name(sp->type));
            } else {
              buffer_t *b;
           
              b = allocate_buffer(sp->u.buf->size + (sp - 1)->u.buf->size);
              memcpy(b->item, (sp - 1)->u.buf->item, (sp - 1)->u.buf->size);
              memcpy(b->item + (sp - 1)->u.buf->size, sp->u.buf->item,
                     sp->u.buf->size);
              free_buffer((sp--)->u.buf);
              free_buffer(sp->u.buf);
              sp->u.buf = b;
            }
            break;
          } /* end of x + T_BUFFER */
#endif
        case T_NUMBER:
          {
            switch ((--sp)->type) {
            case T_NUMBER:
              sp->u.number += (sp+1)->u.number;
              sp->subtype = 0;
              break;
            case T_REAL:
              sp->u.real += (sp+1)->u.number;
              break;
            case T_STRING:
              {
                char buff[30];

                sprintf(buff, "%ld", (sp+1)->u.number);
                EXTEND_SVALUE_STRING(sp, buff, "f_add: 2");
                break;
              }
            default:
              error("Bad type argument to +.  Had %s and %s.\n",
                    type_name(sp->type), type_name((sp+1)->type));
            }
            break;
          } /* end of x + NUMBER */
        case T_REAL:
          {
            switch ((--sp)->type) {
            case T_NUMBER:
              sp->type = T_REAL;
              sp->u.real = sp->u.number + (sp+1)->u.real;
              break;
            case T_REAL:
              sp->u.real += (sp+1)->u.real;
              break;
            case T_STRING:
              {
                char buff[40];

                sprintf(buff, "%f", (sp+1)->u.real);
                EXTEND_SVALUE_STRING(sp, buff, "f_add: 2");
                break;
              }
            default:
              error("Bad type argument to +. Had %s and %s\n",
                    type_name(sp->type), type_name((sp+1)->type));
            }
            break;
          } /* end of x + T_REAL */
        case T_ARRAY:
          {
            if (!((sp-1)->type == T_ARRAY)) {
              error("Bad type argument to +. Had %s and %s\n",
                    type_name((sp - 1)->type), type_name(sp->type));
            } else {
              /* add_array now free's the arrays */
              (sp-1)->u.arr = add_array((sp - 1)->u.arr, sp->u.arr);
              sp--;
              break;
            }
          } /* end of x + T_ARRAY */
        case T_MAPPING:
          {
            if ((sp-1)->type == T_MAPPING) {
              mapping_t *map;
      
              map = add_mapping((sp - 1)->u.map, sp->u.map);
              free_mapping((sp--)->u.map);
              free_mapping(sp->u.map);
              sp->u.map = map;
              break;
            } else
              error("Bad type argument to +. Had %s and %s\n",
                    type_name((sp - 1)->type), type_name(sp->type));
          } /* end of x + T_MAPPING */
        case T_STRING:
          {
            switch ((sp-1)->type) {
	    case T_OBJECT:
	      {
		char buff[1024];
		object_t *ob = (sp-1)->u.ob;
		sprintf(buff, "/%s", ob->obname);
                SVALUE_STRING_ADD_LEFT(buff, "f_add: 3");
		free_object(&ob, "f_add: 3");
                break;
	      }
            case T_NUMBER:
              {
                char buff[30];
            
                sprintf(buff, "%ld", (sp-1)->u.number);
                SVALUE_STRING_ADD_LEFT(buff, "f_add: 3");
                break;
              } /* end of T_NUMBER + T_STRING */
            case T_REAL:
              {
                char buff[40];
          
                sprintf(buff, "%f", (sp - 1)->u.real);
                SVALUE_STRING_ADD_LEFT(buff, "f_add: 3");
                break;
              } /* end of T_REAL + T_STRING */
            case T_STRING:
              {
                SVALUE_STRING_JOIN(sp-1, sp, "f_add: 1");
                sp--;
                break;
              } /* end of T_STRING + T_STRING */
            default:
              error("Bad type argument to +. Had %s and %s\n",
                    type_name((sp - 1)->type), type_name(sp->type));
            }
            break;
          } /* end of x + T_STRING */
	case T_OBJECT:
	  switch ((sp-1)->type) {
	    case T_STRING:
	      {
		const char *fname = sp->u.ob->obname;
		free_object(&(sp--)->u.ob, "f_add: str+ob");
		EXTEND_SVALUE_STRING(sp, "/", "f_add: str ob");
		EXTEND_SVALUE_STRING(sp, fname, "f_add: str ob");
		break;
	      }
	     default:
	       error("Bad type argument to +.  Had %s and %s.\n",
		     type_name(sp->type), type_name((sp+1)->type));
	  }
	  break;
        default:
          error("Bad type argument to +.  Had %s and %s.\n",
                type_name((sp-1)->type), type_name(sp->type));
        }
        break;
      }
    case F_VOID_ADD_EQ:
    case F_ADD_EQ:
      DEBUG_CHECK(sp->type != T_LVALUE,
                  "non-lvalue argument to +=\n");
      lval = sp->u.lvalue;
      sp--;   /* points to the RHS */
      switch (lval->type) {
      case T_STRING:
        if (sp->type == T_STRING) {
          SVALUE_STRING_JOIN(lval, sp, "f_add_eq: 1");
        } else if (sp->type == T_NUMBER) {
          char buff[30];
         
          sprintf(buff, "%ld", sp->u.number);
          EXTEND_SVALUE_STRING(lval, buff, "f_add_eq: 2");
        } else if (sp->type == T_REAL) {
          char buff[40];
         
          sprintf(buff, "%f", sp->u.real);
          EXTEND_SVALUE_STRING(lval, buff, "f_add_eq: 2");
	} else if(sp->type == T_OBJECT) {
	  const char *fname = sp->u.ob->obname;
	  free_object(&(sp--)->u.ob, "f_add_eq: 2");
	  EXTEND_SVALUE_STRING(lval, "/", "f_add: str ob");
	  EXTEND_SVALUE_STRING(lval, fname, "f_add_eq: 2");
        } else {
          bad_argument(sp, T_OBJECT | T_STRING | T_NUMBER | T_REAL, 2, instruction);
        }
        break;
      case T_NUMBER:
        if (sp->type == T_NUMBER) {
          lval->u.number += sp->u.number;
          lval->subtype = 0;
          /* both sides are numbers, no freeing required */
        } else if (sp->type == T_REAL) {
          lval->u.number += sp->u.real;
          lval->subtype = 0;
          /* both sides are numbers, no freeing required */
        } else {
          error("Left hand side of += is a number (or zero); right side is not a number.\n");
        }
        break;
      case T_REAL:
        if (sp->type == T_NUMBER) {
          lval->u.real += sp->u.number;
          /* both sides are numerics, no freeing required */
        } else if (sp->type == T_REAL) {
          lval->u.real += sp->u.real;
          /* both sides are numerics, no freeing required */
        } else {
          error("Left hand side of += is a number (or zero); right side is not a number.\n");
        }
        break;
#ifndef NO_BUFFER_TYPE
      case T_BUFFER:
        if (sp->type != T_BUFFER) {
          bad_argument(sp, T_BUFFER, 2, instruction);
        } else {
          buffer_t *b;
         
          b = allocate_buffer(lval->u.buf->size + sp->u.buf->size);
          memcpy(b->item, lval->u.buf->item, lval->u.buf->size);
          memcpy(b->item + lval->u.buf->size, sp->u.buf->item,
                 sp->u.buf->size);
          free_buffer(sp->u.buf);
          free_buffer(lval->u.buf);
          lval->u.buf = b;
        }
        break;
#endif
      case T_ARRAY:
        if (sp->type != T_ARRAY)
          bad_argument(sp, T_ARRAY, 2, instruction);
        else {
          /* add_array now frees the arrays */
          lval->u.arr = add_array(lval->u.arr, sp->u.arr);
        }
        break;
      case T_MAPPING:
        if (sp->type != T_MAPPING)
          bad_argument(sp, T_MAPPING, 2, instruction);
        else {
          absorb_mapping(lval->u.map, sp->u.map);
          free_mapping(sp->u.map); /* free RHS */
          /* LHS not freed because its being reused */
        }
        break;
      case T_LVALUE_BYTE:
        {
          char c;

          if (sp->type != T_NUMBER)
            error("Bad right type to += of char lvalue.\n");
        
          c = *global_lvalue_byte.u.lvalue_byte + sp->u.number;
        
          if (global_lvalue_byte.subtype == 0 && c == '\0')
            error("Strings cannot contain 0 bytes.\n");
          *global_lvalue_byte.u.lvalue_byte = c;
        }
        break;
      default:
        bad_arg(1, instruction);
      }
       
      if (instruction == F_ADD_EQ) { /* not void add_eq */
        assign_svalue_no_free(sp, lval);
      } else {
        /*
         * but if (void)add_eq then no need to produce an
         * rvalue
         */
        sp--;
      }
      break;
    case F_AND:
      f_and();
      break;
    case F_AND_EQ:
      f_and_eq();
      break;
    case F_FUNCTION_CONSTRUCTOR:
      f_function_constructor();
      break;

    case F_FOREACH:
      {
        int flags = EXTRACT_UCHAR(pc++);

        IF_DEBUG(stack_in_use_as_temporary++);
        if (flags & FOREACH_MAPPING) {
          CHECK_TYPES(sp, T_MAPPING, 2, F_FOREACH);
        
          push_refed_array(mapping_indices(sp->u.map));
        
          STACK_INC;
          sp->type = T_NUMBER;
          sp->u.lvalue = (sp-1)->u.arr->item;
          sp->subtype = (sp-1)->u.arr->size;
        
          STACK_INC;
          sp->type = T_LVALUE;
          if (flags & FOREACH_LEFT_GLOBAL) {
            sp->u.lvalue = find_value(EXTRACT_UCHAR(pc++) + variable_index_offset);
          } else {
            sp->u.lvalue = fp + EXTRACT_UCHAR(pc++);
          }
        } else
          if (sp->type == T_STRING) {
            STACK_INC;
            sp->type = T_NUMBER;
            sp->u.lvalue_byte = (unsigned char *)((sp-1)->u.string);
            sp->subtype = SVALUE_STRLEN(sp - 1);
          } else {
            CHECK_TYPES(sp, T_ARRAY, 2, F_FOREACH);

            STACK_INC;
            sp->type = T_NUMBER;
            sp->u.lvalue = (sp-1)->u.arr->item;
            sp->subtype = (sp-1)->u.arr->size;
          }

        if (flags & FOREACH_RIGHT_GLOBAL) {
          STACK_INC;
          sp->type = T_LVALUE;
          sp->u.lvalue = find_value((EXTRACT_UCHAR(pc++) + variable_index_offset));
        } else if (flags & FOREACH_REF) {
          ref_t *ref = make_ref();
          svalue_t *loc = fp + EXTRACT_UCHAR(pc++);
        
          /* foreach guarantees our target remains valid */
          ref->lvalue = 0;
          ref->sv.type = T_NUMBER;
          STACK_INC;
          sp->type = T_REF;
          sp->u.ref = ref;
          DEBUG_CHECK(loc->type != T_NUMBER && loc->type != T_REF, "Somehow a reference in foreach acquired a value before coming into scope");
          loc->type = T_REF;
          loc->u.ref = ref;
          ref->ref++;
        } else {
          STACK_INC;
          sp->type = T_LVALUE;
          sp->u.lvalue = fp + EXTRACT_UCHAR(pc++);
        }
        break;
      }
    case F_NEXT_FOREACH:
      if ((sp-1)->type == T_LVALUE) {
        /* mapping */
        if ((sp-2)->subtype--) {
          svalue_t *key = (sp-2)->u.lvalue++;
          svalue_t *value = find_in_mapping((sp-4)->u.map, key);
        
          assign_svalue((sp-1)->u.lvalue, key);
          if (sp->type == T_REF) {
            if (value == &const0u)
              sp->u.ref->lvalue = 0;
            else
              sp->u.ref->lvalue = value;
          } else
            assign_svalue(sp->u.lvalue, value);
          COPY_SHORT(&offset, pc);
          pc -= offset;
          break;
        }
      } else {
        /* array or string */
        if ((sp-1)->subtype--) {
          if ((sp-2)->type == T_STRING) {
            if (sp->type == T_REF) {
              sp->u.ref->lvalue = &global_lvalue_byte;
              global_lvalue_byte.u.lvalue_byte = (unsigned char *)((sp-1)->u.lvalue_byte++);
            } else {
              free_svalue(sp->u.lvalue, "foreach-string");
              sp->u.lvalue->type = T_NUMBER;
              sp->u.lvalue->subtype = 0;
              sp->u.lvalue->u.number = *((sp-1)->u.lvalue_byte)++;
            }
          } else {
            if (sp->type == T_REF)
              sp->u.ref->lvalue = (sp-1)->u.lvalue++;
            else
              assign_svalue(sp->u.lvalue, (sp-1)->u.lvalue++);
          }
          COPY_SHORT(&offset, pc);
          pc -= offset;
          break;
        }
      }
      pc += 2;
      /* fallthrough */
    case F_EXIT_FOREACH:
      IF_DEBUG(stack_in_use_as_temporary--);
      if (sp->type == T_REF) {
        if (!(--sp->u.ref->ref) && sp->u.ref->lvalue == 0)
          FREE(sp->u.ref);
      }
      if ((sp-1)->type == T_LVALUE) {
        /* mapping */
        sp -= 3;
        free_array((sp--)->u.arr);
        free_mapping((sp--)->u.map);
      } else {
        /* array or string */
        sp -= 2;
        if (sp->type == T_STRING)
          free_string_svalue(sp--);
        else
          free_array((sp--)->u.arr);
      }
      break;

    case F_EXPAND_VARARGS:
      {
        svalue_t *s, *t;
        array_t *arr;
    
        i = EXTRACT_UCHAR(pc++);
        s = sp - i;
    
        if (s->type != T_ARRAY)
          error("Item being expanded with ... is not an array\n");
    
        arr = s->u.arr;
        n = arr->size;
        CHECK_STACK_OVERFLOW(n - 1);
        num_varargs += n - 1;
        if (!n) {
          t = s;
          while (t < sp) {
            *t = *(t + 1);
            t++;
          }
          sp--;
        } else if (n == 1) {
          assign_svalue_no_free(s, &arr->item[0]);
        } else {
          t = sp;
          CHECK_STACK_OVERFLOW(n - 1);
          sp += n - 1;
          while (t > s) {
            *(t + n - 1) = *t;
            t--;
          }
          t = s + n - 1;
          if (arr->ref == 1) {
            memcpy(s, arr->item, n * sizeof(svalue_t));
            free_empty_array(arr);
            break;
          } else {
            while (n--)
              assign_svalue_no_free(t--, &arr->item[n]);
          }
        }
        free_array(arr);
        break;
      }
      
    case F_NEW_CLASS:
      {
        array_t *cl;

        cl = allocate_class(&current_prog->classes[EXTRACT_UCHAR(pc++)], 1);
        push_refed_class(cl);
      }
      break;
    case F_NEW_EMPTY_CLASS:
      {
        array_t *cl;

        cl = allocate_class(&current_prog->classes[EXTRACT_UCHAR(pc++)], 0);
        push_refed_class(cl);
      }
      break;
    case F_AGGREGATE:
      {
        array_t *v;
    
        LOAD_SHORT(offset, pc);
        offset += num_varargs;
        num_varargs = 0;
        v = allocate_empty_array(offset);
        /*
         * transfer svalues in reverse...popping stack as we go
         */
        while (offset--)
          v->item[offset] = *sp--;
        push_refed_array(v);
      }
      break;
    case F_AGGREGATE_ASSOC:
      {
        mapping_t *m;
    
        LOAD_SHORT(offset, pc);

        offset += num_varargs;
        num_varargs = 0;
        m = load_mapping_from_aggregate(sp -= offset, offset);
        push_refed_mapping(m);
        break;
      }
    case F_ASSIGN:
#ifdef DEBUG
      if (sp->type != T_LVALUE) fatal("Bad argument to F_ASSIGN\n");
#endif
      switch(sp->u.lvalue->type) {
      case T_LVALUE_BYTE:
        {
          unsigned char c;
    
          if ((sp - 1)->type != T_NUMBER) {
            error("Illegal rhs to char lvalue\n");
          } else {
            c = ((sp - 1)->u.number & 0xff);
            if (global_lvalue_byte.subtype == 0 && c == '\0')
              error("Strings cannot contain 0 bytes.\n");
            *global_lvalue_byte.u.lvalue_byte = c;
          }
          break;
        }
      default:
        assign_svalue(sp->u.lvalue, sp - 1);
        break;
      case T_LVALUE_RANGE:
        assign_lvalue_range(sp - 1);
        break;
      }
      sp--;   /* ignore lvalue */
      /* rvalue is already in the correct place */
      break;
    case F_VOID_ASSIGN_LOCAL:
      if (sp->type != T_INVALID) {
        lval = fp + EXTRACT_UCHAR(pc++);
        free_svalue(lval, "F_VOID_ASSIGN_LOCAL");
        *lval = *sp--;
      } else {
        sp--;
        pc++;
      }
      break;
    case F_VOID_ASSIGN:
#ifdef DEBUG
      if (sp->type != T_LVALUE) fatal("Bad argument to F_VOID_ASSIGN\n");
#endif
      lval = (sp--)->u.lvalue;
      if (sp->type != T_INVALID) {
        switch(lval->type) {
        case T_LVALUE_BYTE:
          {
            if (sp->type != T_NUMBER) {
              error("Illegal rhs to char lvalue\n");
            } else {
              char c = (sp--)->u.number & 0xff;
              if (global_lvalue_byte.subtype == 0 && c == '\0')
                error("Strings cannot contain 0 bytes.\n");
              *global_lvalue_byte.u.lvalue_byte = c;
            }
            break;
          }

        case T_LVALUE_RANGE:
          {
            copy_lvalue_range(sp--);
            break;
          }

        default:
          {
            free_svalue(lval, "F_VOID_ASSIGN : 3");
            *lval = *sp--;
          }
        }
      } else sp--;
      break;
#ifdef DEBUG
    case F_BREAK_POINT:
      break_point();
      break;
#endif
    case F_CALL_FUNCTION_BY_ADDRESS:
      {
        function_t *funp;
    
        LOAD_SHORT(offset, pc);

        offset += function_index_offset;
        /*
         * Find the function in the function table. As the
         * function may have been redefined by inheritance, we
         * must look in the last table, which is pointed to by
         * current_object.
         */
        DEBUG_CHECK(offset >= current_object->prog->last_inherited +
                    current_object->prog->num_functions_defined,
                    "Illegal function index\n");
    
        if (current_object->prog->function_flags[offset] & FUNC_ALIAS) {
          offset = current_object->prog->function_flags[offset] & ~FUNC_ALIAS;
        }
    
        if (current_object->prog->function_flags[offset] 
            & (FUNC_PROTOTYPE|FUNC_UNDEFINED)) {
          error("Undefined function called: %s\n", function_name(current_object->prog, offset));
        }

        /* Save all important global stack machine registers */
        push_control_stack(FRAME_FUNCTION);
        current_prog = current_object->prog;
    
        caller_type = ORIGIN_LOCAL;
        /*
         * If it is an inherited function, search for the real
         * definition.
         */
        csp->num_local_variables = EXTRACT_UCHAR(pc++) + num_varargs;
        num_varargs = 0;
	//if(offset > USHRT_MAX)
	//error("Broken function table"); offset is a USHRT, so this just can't happen!
        funp = setup_new_frame(offset);
        csp->pc = pc; /* The corrected return address */

          pc = current_prog->program + funp->address;
      }
      break;
    case F_CALL_INHERITED:
      {
        inherit_t *ip = current_prog->inherit + EXTRACT_UCHAR(pc++);
        program_t *temp_prog = ip->prog;
        function_t *funp;
    
        LOAD_SHORT(offset, pc);

        push_control_stack(FRAME_FUNCTION);
        current_prog = temp_prog;

        caller_type = ORIGIN_LOCAL;
    
        csp->num_local_variables = EXTRACT_UCHAR(pc++) + num_varargs;
        num_varargs = 0;
    
        function_index_offset += ip->function_index_offset;
        variable_index_offset += ip->variable_index_offset;
    
        funp = setup_inherited_frame(offset);
        csp->pc = pc;
	pc = current_prog->program + funp->address;
      }
      break;
    case F_COMPL:
      if (sp->type != T_NUMBER)
        error("Bad argument to ~\n");
      sp->u.number = ~sp->u.number;
      sp->subtype = 0;
      break;
    case F_CONST0:
      push_number(0);
      break;
    case F_CONST1:
      push_number(1);
      break;
    case F_PRE_DEC:
      DEBUG_CHECK(sp->type != T_LVALUE, 
                  "non-lvalue argument to --\n");
      lval = sp->u.lvalue;
      switch (lval->type) {
      case T_NUMBER:
        sp->type = T_NUMBER;
        sp->subtype = 0;
        sp->u.number = --(lval->u.number);
        break;
      case T_REAL:
        sp->type = T_REAL;
        sp->u.real = --(lval->u.real);
        break;
      case T_LVALUE_BYTE:
        if (global_lvalue_byte.subtype == 0 &&
            *global_lvalue_byte.u.lvalue_byte == '\x1')
          error("Strings cannot contain 0 bytes.\n");
        sp->type = T_NUMBER;
        sp->subtype = 0;
        sp->u.number = --(*global_lvalue_byte.u.lvalue_byte);
        break;
      default:
        error("-- of non-numeric argument\n");
      }
      break;
    case F_DEC:
      DEBUG_CHECK(sp->type != T_LVALUE,
                  "non-lvalue argument to --\n");
      lval = (sp--)->u.lvalue;
      switch (lval->type) {
      case T_NUMBER:
        lval->u.number--;
        break;
      case T_REAL:
        lval->u.real--;
        break;
      case T_LVALUE_BYTE:
        if (global_lvalue_byte.subtype == 0 && 
            *global_lvalue_byte.u.lvalue_byte == '\x1')
          error("Strings cannot contain 0 bytes.\n");
        --(*global_lvalue_byte.u.lvalue_byte);
        break;
      default:
        error("-- of non-numeric argument\n");
      }
      break;
    case F_DIVIDE:
      { 
        switch((sp-1)->type|sp->type) {
        
        case T_NUMBER:
          {
            if (!(sp--)->u.number) error("Division by zero\n");
            sp->u.number /= (sp+1)->u.number;
            break;
          }
        
        case T_REAL:
          {
            if ((sp--)->u.real == 0.0) error("Division by zero\n");
            sp->u.real /= (sp+1)->u.real;
            break;
          }
        
        case T_NUMBER|T_REAL:
          {
            if ((sp--)->type == T_NUMBER) {
              if (!((sp+1)->u.number)) error("Division by zero\n");
              sp->u.real /= (sp+1)->u.number;
            } else {
              if ((sp+1)->u.real == 0.0) error("Division by 0.0\n");
              sp->type = T_REAL;
              sp->u.real = sp->u.number / (sp+1)->u.real;
            }
            break;
          }
        
        default:
          {
            if (!((sp-1)->type & (T_NUMBER|T_REAL)))
              bad_argument(sp-1,T_NUMBER|T_REAL,1, instruction);
            if (!(sp->type & (T_NUMBER|T_REAL)))
              bad_argument(sp, T_NUMBER|T_REAL,2, instruction);
          }
        }
      }
      break;
    case F_DIV_EQ:
      f_div_eq();
      break;
    case F_EQ:
      f_eq();
      break;
    case F_GE:
      f_ge();
      break;
    case F_GT:
      f_gt();
      break;
    case F_GLOBAL:
      {
        svalue_t *s;
    
        s = find_value((EXTRACT_UCHAR(pc++) + variable_index_offset));
    
        /*
         * If variable points to a destructed object, replace it
         * with 0, otherwise, fetch value of variable.
         */
        if ((s->type == T_OBJECT) && (s->u.ob->flags & O_DESTRUCTED))
          assign_svalue(s, &const0u);
        push_svalue(s);
        break;
      }
    case F_PRE_INC:
      DEBUG_CHECK(sp->type != T_LVALUE,
                  "non-lvalue argument to ++\n");
      lval = sp->u.lvalue;
      switch (lval->type) {
      case T_NUMBER:
        sp->type = T_NUMBER;
        sp->subtype = 0;
        sp->u.number = ++lval->u.number;
        break;
      case T_REAL:
        sp->type = T_REAL;
        sp->u.real = ++lval->u.real;
        break;
      case T_LVALUE_BYTE:
        if (global_lvalue_byte.subtype == 0 &&
            *global_lvalue_byte.u.lvalue_byte == (unsigned char)255)
          error("Strings cannot contain 0 bytes.\n");
        sp->type = T_NUMBER;
        sp->subtype = 0;
        sp->u.number = ++*global_lvalue_byte.u.lvalue_byte;
        break;
      default:
        error("++ of non-numeric argument\n");
      }
      break;
    case F_MEMBER:
      { 
        array_t *arr;
     
        if (sp->type != T_CLASS)
          error("Tried to take a member of something that isn't a class.\n");
        i = EXTRACT_UCHAR(pc++);
        arr = sp->u.arr;
        if (i >= arr->size) error("Class has no corresponding member.\n");
        if (arr->item[i].type == T_OBJECT &&
            (arr->item[i].u.ob->flags & O_DESTRUCTED)) {
          assign_svalue(&arr->item[i], &const0u);
        }
        assign_svalue_no_free(sp, &arr->item[i]);
        free_class(arr);

        break;
      }
    case F_MEMBER_LVALUE:
      { 
        array_t *arr;
     
        if (sp->type != T_CLASS)
          error("Tried to take a member of something that isn't a class.\n");
        i = EXTRACT_UCHAR(pc++);
        arr = sp->u.arr;
        if (i >= arr->size) error("Class has no corresponding member.\n");
        sp->type = T_LVALUE;
        sp->u.lvalue = arr->item + i;
#ifdef REF_RESERVED_WORD
        lv_owner_type = T_CLASS;
        lv_owner = (refed_t *)arr;
#endif
        free_class(arr);
        break;
      }
    case F_INDEX:
      switch (sp->type) {
      case T_MAPPING:
        {
          svalue_t *v;
          mapping_t *m;

          v = find_in_mapping(m = sp->u.map, sp - 1);
          if (v->type == T_OBJECT && (v->u.ob->flags & O_DESTRUCTED)) {
            assign_svalue(v, &const0u);
          }
          assign_svalue(--sp, v); /* v will always have a value */
          free_mapping(m);
          break;
        }
#ifndef NO_BUFFER_TYPE
      case T_BUFFER:
        {
          if ((sp-1)->type != T_NUMBER)
            error("Buffer indexes must be integers.\n");

          i = (sp - 1)->u.number;
          if ((i > sp->u.buf->size) || (i < 0))
            error("Buffer index out of bounds.\n");
          i = sp->u.buf->item[i];
          free_buffer(sp->u.buf);
          (--sp)->u.number = i;
          sp->subtype = 0;
          break;
        }
#endif
      case T_STRING:
        {
          if ((sp-1)->type != T_NUMBER) {
            error("String indexes must be integers.\n");
          }
          i = (sp - 1)->u.number;
          if ((i > SVALUE_STRLEN(sp)) || (i < 0))
            error("String index out of bounds.\n");
          i = (unsigned char) sp->u.string[i];
          free_string_svalue(sp);
          (--sp)->u.number = i;
          break;
        }
      case T_ARRAY:
        {
          array_t *arr;

          if ((sp-1)->type != T_NUMBER)
            error("Array indexes must be integers.\n");
          i = (sp - 1)->u.number;
          if (i<0) error("Array index must be positive or zero.\n");
          arr = sp->u.arr;
          if (i >= arr->size) error("Array index out of bounds.\n");
          if (arr->item[i].type == T_OBJECT &&
              (arr->item[i].u.ob->flags & O_DESTRUCTED)) {
            assign_svalue(&arr->item[i], &const0u);
          }
          assign_svalue_no_free(--sp, &arr->item[i]);
          free_array(arr);
          break;
        }
      default:
        if (sp->type == T_NUMBER && !sp->u.number) 
          error("Value being indexed is zero.\n");
        error("Cannot index value of type '%s'.\n", type_name(sp->type));
      }
      break;
    case F_RINDEX:
      switch (sp->type) {
#ifndef NO_BUFFER_TYPE
      case T_BUFFER:
        {
          if ((sp-1)->type != T_NUMBER)
            error("Indexing a buffer with an illegal type.\n");

          i = sp->u.buf->size - (sp - 1)->u.number;
          if ((i > sp->u.buf->size) || (i < 0))
            error("Buffer index out of bounds.\n");

          i = sp->u.buf->item[i];
          free_buffer(sp->u.buf);
          (--sp)->u.number = i;
          sp->subtype = 0;
          break;
        }
#endif
      case T_STRING:
        {
          int len = SVALUE_STRLEN(sp);
          if ((sp-1)->type != T_NUMBER) {
            error("Indexing a string with an illegal type.\n");
          }
          i = len - (sp - 1)->u.number;
          if ((i > len) || (i < 0))
            error("String index out of bounds.\n");
          i = (unsigned char) sp->u.string[i];
          free_string_svalue(sp);
          (--sp)->u.number = i;
          break;
        }
      case T_ARRAY:
        {
          array_t *arr = sp->u.arr;

          if ((sp-1)->type != T_NUMBER)
            error("Indexing an array with an illegal type\n");
          i = arr->size - (sp - 1)->u.number;
          if (i < 0 || i >= arr->size) error("Array index out of bounds.\n");
          if (arr->item[i].type == T_OBJECT &&
              (arr->item[i].u.ob->flags & O_DESTRUCTED)) {
            assign_svalue(&arr->item[i], &const0u);
          }
          assign_svalue_no_free(--sp, &arr->item[i]);
          free_array(arr);
          break;
        }
      default:
        if (sp->type == T_NUMBER && !sp->u.number) 
          error("Value being indexed is zero.\n");
        error("Cannot index value of type '%s'.\n", type_name(sp->type));
      }
      break;
#ifdef F_JUMP_WHEN_ZERO
    case F_JUMP_WHEN_ZERO:
      if ((i = (sp->type == T_NUMBER)) && sp->u.number == 0) {
        COPY_SHORT(&offset, pc);
        pc = current_prog->program + offset;
      } else {
        pc += 2;
      }
      if (i) {
        sp--;   /* cheaper to do this when sp is an integer
                 * svalue */
      } else {
        pop_stack();
      }
      break;
#endif
#ifdef F_JUMP
    case F_JUMP:
      COPY_SHORT(&offset, pc);
      pc = current_prog->program + offset;
      break;
#endif
    case F_LE:
      f_le();
      break;
    case F_LSH:
      f_lsh();
      break;
    case F_LSH_EQ:
      f_lsh_eq();
      break;
    case F_MOD:
      {
        CHECK_TYPES(sp - 1, T_NUMBER, 1, instruction);
        CHECK_TYPES(sp, T_NUMBER, 2, instruction);
        if ((sp--)->u.number == 0)
          error("Modulus by zero.\n");
        sp->u.number %= (sp+1)->u.number;
      }
      break;
    case F_MOD_EQ:
      f_mod_eq();
      break;
    case F_MULTIPLY:
      {
        switch((sp-1)->type|sp->type) {
        case T_NUMBER:
          {
            sp--;
            sp->u.number *= (sp+1)->u.number;
            break;
          }
         
        case T_REAL:
          {
            sp--;
            sp->u.real *= (sp+1)->u.real;
            break;
          }
         
        case T_NUMBER|T_REAL:
          {
            if ((--sp)->type == T_NUMBER) {
              sp->type = T_REAL;
              sp->u.real = sp->u.number * (sp+1)->u.real;
            }
            else sp->u.real *= (sp+1)->u.number;
            break;
          }
         
        case T_MAPPING:
          {
            mapping_t *m;
            m = compose_mapping((sp-1)->u.map, sp->u.map, 1);
            pop_2_elems();
            push_refed_mapping(m);
            break;
          }
         
        default:
          {
            if (!((sp-1)->type & (T_NUMBER|T_REAL|T_MAPPING)))
              bad_argument(sp-1, T_NUMBER|T_REAL|T_MAPPING,1, instruction);
            if (!(sp->type & (T_NUMBER|T_REAL|T_MAPPING)))
              bad_argument(sp, T_NUMBER|T_REAL|T_MAPPING,2, instruction);
            error("Args to * are not compatible.\n");
          }
        }
      }
      break;
    case F_MULT_EQ:
      f_mult_eq();
      break;
    case F_NE:
      f_ne();
      break;
    case F_NEGATE:
      if (sp->type == T_NUMBER) {
        sp->u.number = -sp->u.number;
        sp->subtype = 0;
      } else if (sp->type == T_REAL)
        sp->u.real = -sp->u.real;
      else
        error("Bad argument to unary minus\n");
      break;
    case F_NOT:
      if (sp->type == T_NUMBER) {
        sp->u.number = !sp->u.number;
        sp->subtype = 0;
      } else {
        free_svalue(sp, "f_not");
        *sp = const0;
      }
      break;
    case F_OR:
      f_or();
      break;
    case F_OR_EQ:
      f_or_eq();
      break;
    case F_PARSE_COMMAND:
      f_parse_command();
      break;
    case F_POP_VALUE:
      pop_stack();
      break;
    case F_POST_DEC:
      DEBUG_CHECK(sp->type != T_LVALUE,
                  "non-lvalue argument to --\n");
      lval = sp->u.lvalue;
      switch(lval->type) {
      case T_NUMBER:
        sp->type = T_NUMBER;
        sp->u.number = lval->u.number--;
        sp->subtype = 0;
        break;
      case T_REAL:
        sp->type = T_REAL;
        sp->u.real = lval->u.real--;
        break;
      case T_LVALUE_BYTE:
        sp->type = T_NUMBER;
        if (global_lvalue_byte.subtype == 0 && 
            *global_lvalue_byte.u.lvalue_byte == '\x1')
          error("Strings cannot contain 0 bytes.\n");
        sp->u.number = (*global_lvalue_byte.u.lvalue_byte)--;
        sp->subtype = 0;
        break;
      default:
        error("-- of non-numeric argument\n");
      }
      break;
    case F_POST_INC:
      DEBUG_CHECK(sp->type != T_LVALUE,
                  "non-lvalue argument to ++\n");
      lval = sp->u.lvalue;
      switch (lval->type) {
      case T_NUMBER:
        sp->type = T_NUMBER;
        sp->u.number = lval->u.number++;
        sp->subtype = 0;
        break;
      case T_REAL:
        sp->type = T_REAL;
        sp->u.real = lval->u.real++;
        break;
      case T_LVALUE_BYTE:
        if (global_lvalue_byte.subtype == 0 &&
            *global_lvalue_byte.u.lvalue_byte == (unsigned char)255)
          error("Strings cannot contain 0 bytes.\n");
        sp->type = T_NUMBER;
        sp->u.number = (*global_lvalue_byte.u.lvalue_byte)++;
        sp->subtype = 0;
        break;
      default:
        error("++ of non-numeric argument\n");
      }
      break;
    case F_GLOBAL_LVALUE:
      STACK_INC;
      sp->type = T_LVALUE;
      sp->u.lvalue = find_value((EXTRACT_UCHAR(pc++) +
                                       variable_index_offset));
      break;
    case F_INDEX_LVALUE:
      push_indexed_lvalue(0);
      break;
    case F_RINDEX_LVALUE:
      push_indexed_lvalue(1);
      break;
    case F_NN_RANGE_LVALUE:
      push_lvalue_range(0x00);
      break;
    case F_RN_RANGE_LVALUE:
      push_lvalue_range(0x10);
      break;
    case F_RR_RANGE_LVALUE:
      push_lvalue_range(0x11);
      break;
    case F_NR_RANGE_LVALUE:
      push_lvalue_range(0x01);
      break;
    case F_NN_RANGE:
      f_range(0x00);
      break;
    case F_RN_RANGE:
      f_range(0x10);
      break;
    case F_NR_RANGE:
      f_range(0x01);
      break;
    case F_RR_RANGE:
      f_range(0x11);
      break;
    case F_NE_RANGE:
      f_extract_range(0);
      break;
    case F_RE_RANGE:
      f_extract_range(1);
      break;
    case F_RETURN_ZERO:
      {
        if (csp->framekind & FRAME_CATCH) {
          free_svalue(&catch_value, "F_RETURN_ZERO");
          catch_value = const0;
          while (csp->framekind & FRAME_CATCH)
            pop_control_stack();
          csp->framekind |= FRAME_RETURNED_FROM_CATCH;
        }

        /*
         * Deallocate frame and return.
         */
        pop_n_elems(sp - fp + 1);
        STACK_INC;

        DEBUG_CHECK(sp != fp, "Bad stack at F_RETURN_ZERO\n");
        *sp = const0;
        pop_control_stack();
#ifdef TRACE
        tracedepth--;
        if (TRACEP(TRACE_RETURN)) {
          do_trace("Return", "", "");
          if (TRACEHB) {
            if (TRACETST(TRACE_ARGS)) {
              static char msg[] = "with value: 0";
          
              add_message(command_giver, msg, sizeof(msg)-1);
            }
            add_message(command_giver, "\n", 1);
          }
        }
#endif
        /* The control stack was popped just before */
        if (csp[1].framekind & (FRAME_EXTERNAL | FRAME_RETURNED_FROM_CATCH))
          return;
      }
      break;
    case F_RETURN:
      {
        svalue_t sv;

        if (csp->framekind & FRAME_CATCH) {
          free_svalue(&catch_value, "F_RETURN");
          catch_value = const0;
          while (csp->framekind & FRAME_CATCH)
            pop_control_stack();
          csp->framekind |= FRAME_RETURNED_FROM_CATCH;
        }
    
        if (sp - fp + 1) {
          sv = *sp--;
          /*
           * Deallocate frame and return.
           */
          pop_n_elems(sp - fp + 1);
          STACK_INC;
          DEBUG_CHECK(sp != fp, "Bad stack at F_RETURN\n");
          *sp = sv; /* This way, the same ref counts are
                     * maintained */
        }
        pop_control_stack();
#ifdef TRACE
        tracedepth--;
        if (TRACEP(TRACE_RETURN)) {
          do_trace("Return", "", "");
          if (TRACEHB) {
            if (TRACETST(TRACE_ARGS)) {
              char msg[] = " with value: ";
          
              add_message(command_giver, msg, sizeof(msg)-1);
              print_svalue(sp);
            }
            add_message(command_giver, "\n", 1);
          }
        }
#endif
        /* The control stack was popped just before */
        if (csp[1].framekind & (FRAME_EXTERNAL | FRAME_RETURNED_FROM_CATCH))
          return;
        break;
      }
    case F_RSH:
      f_rsh();
      break;
    case F_RSH_EQ:
      f_rsh_eq();
      break;
    case F_SSCANF:
      f_sscanf();
      break;
    case F_STRING:
      LOAD_SHORT(offset, pc);
      DEBUG_CHECK1(offset >= current_prog->num_strings,
                   "string %d out of range in F_STRING!\n",
                   offset);
      push_shared_string(current_prog->strings[offset]);
      break;
    case F_SHORT_STRING:
      DEBUG_CHECK1(EXTRACT_UCHAR(pc) >= current_prog->num_strings,
                   "string %d out of range in F_STRING!\n",
                   EXTRACT_UCHAR(pc));
      push_shared_string(current_prog->strings[EXTRACT_UCHAR(pc++)]);
      break;
    case F_SUBTRACT:
      {
        i = (sp--)->type;
        switch (i | sp->type) {
        case T_NUMBER:
          sp->u.number -= (sp+1)->u.number;
          break;

        case T_REAL:
          sp->u.real -= (sp+1)->u.real;
          break;

        case T_NUMBER | T_REAL:
          if (sp->type == T_REAL) sp->u.real -= (sp+1)->u.number;
          else {
            sp->type = T_REAL;
            sp->u.real = sp->u.number - (sp+1)->u.real;
          }
          break;

        case T_ARRAY:
          {
            /*
             * subtract_array already takes care of
             * destructed objects
             */
            sp->u.arr = subtract_array(sp->u.arr, (sp+1)->u.arr);
            break;
          }
        
        default:
          if (!((sp++)->type & (T_NUMBER|T_REAL|T_ARRAY)))
            error("Bad left type to -.\n");
          else if (!(sp->type & (T_NUMBER|T_REAL|T_ARRAY)))
            error("Bad right type to -.\n");
          else error("Arguments to - do not have compatible types.\n");
        }
        break;
      }
    case F_SUB_EQ:
      f_sub_eq();
      break;
    case F_SIMUL_EFUN:
      {
        unsigned short sindex;
        int num_args;
    
        LOAD_SHORT(sindex, pc);
        num_args = EXTRACT_UCHAR(pc++) + num_varargs;
        num_varargs = 0;
        call_simul_efun(sindex, num_args);
      }
      break;
    case F_SWITCH:
      f_switch();
      break;
    case F_XOR:
      f_xor();
      break;
    case F_XOR_EQ:
      f_xor_eq();
      break;
    case F_CATCH:
      {
        /*
         * Compute address of next instruction after the CATCH
         * statement.
         */
        ((char *) &offset)[0] = pc[0];
        ((char *) &offset)[1] = pc[1];
        offset = pc + offset - current_prog->program;
        pc += 2;
    
        do_catch(pc, offset);
        if ((csp[1].framekind & (FRAME_EXTERNAL | FRAME_RETURNED_FROM_CATCH)) ==
            (FRAME_EXTERNAL | FRAME_RETURNED_FROM_CATCH)) {
          return;
        }
    
        break;
      }
    case F_END_CATCH:
      {
        free_svalue(&catch_value, "F_END_CATCH");
        catch_value = const0;
        /* We come here when no longjmp() was executed */
        pop_control_stack();
        push_number(0);
        return;   /* return to do_catch */
      }
    case F_TIME_EXPRESSION:
      {
        long sec, usec;

        IF_DEBUG(stack_in_use_as_temporary++);
        get_usec_clock(&sec, &usec);
        push_number(sec);
        push_number(usec);
        break;
      }
    case F_END_TIME_EXPRESSION:
      {
        long sec, usec;
    
        get_usec_clock(&sec, &usec);
        usec = (sec - (sp - 1)->u.number) * 1000000 + (usec - sp->u.number);
        sp -= 2;
        IF_DEBUG(stack_in_use_as_temporary--);
        push_number(usec);
        break;
      }
    case F_TYPE_CHECK:
      {
        int type = sp->u.number;
        pop_stack();
        if(sp->type != type && !(sp->type == T_NUMBER && sp->u.number == 0) &&
	   !(sp->type == T_LVALUE))
          error("Trying to put %s in %s\n", type_name(sp->type), type_name(type));
        break;
      }
#define Instruction (instruction + ONEARG_MAX)
#ifdef DEBUG
#define CALL_THE_EFUN goto call_the_efun
#else
#define CALL_THE_EFUN (*oefun_table[instruction])(); continue
#endif
    case F_EFUN0:
      st_num_arg = 0;
      instruction = EXTRACT_UCHAR(pc++);
      CALL_THE_EFUN;
    case F_EFUN1:
      st_num_arg = 1;
      instruction = EXTRACT_UCHAR(pc++);
      CHECK_TYPES(sp, instrs2[instruction].type[0], 1, Instruction);
      CALL_THE_EFUN;
    case F_EFUN2:
      st_num_arg = 2;
      instruction = EXTRACT_UCHAR(pc++);
      CHECK_TYPES(sp - 1, instrs2[instruction].type[0], 1, Instruction);
      CHECK_TYPES(sp, instrs2[instruction].type[1], 2, Instruction);
      CALL_THE_EFUN;
    case F_EFUN3:
      st_num_arg = 3;
      instruction = EXTRACT_UCHAR(pc++);
      CHECK_TYPES(sp - 2, instrs2[instruction].type[0], 1, Instruction);
      CHECK_TYPES(sp - 1, instrs2[instruction].type[1], 2, Instruction);
      CHECK_TYPES(sp, instrs2[instruction].type[2], 3, Instruction);
      CALL_THE_EFUN;
    case F_EFUNV:
      {
        int num;
        st_num_arg = EXTRACT_UCHAR(pc++) + num_varargs;
        num_varargs = 0;
        instruction = EXTRACT_UCHAR(pc++);
        num = instrs2[instruction].min_arg;
        for (i = 1; i <= num; i++) {
          CHECK_TYPES(sp - st_num_arg + i, instrs2[instruction].type[i-1], i, Instruction);
        }
        CALL_THE_EFUN;
      }
    default:
      /* optimized 1 arg efun */
      st_num_arg = 1;
      CHECK_TYPES(sp, instrs[instruction].type[0], 1, instruction);
#ifndef DEBUG
      (*ooefun_table[instruction])();
      continue;
#else
      instruction -= ONEARG_MAX;
    call_the_efun:
      /* We have an efun.  Execute it
       */
      if (Instruction > NUM_OPCODES) {
        fatal("Undefined instruction %s (%d)\n",
              query_instr_name(Instruction), Instruction);
      }
      if (Instruction < BASE) {
        fatal("No case for eoperator %s (%d)\n",
              query_instr_name(Instruction), Instruction);
      }
      if (instrs2[instruction].ret_type == TYPE_NOVALUE)
        expected_stack = sp - st_num_arg;
      else
        expected_stack = sp - st_num_arg + 1;
      num_arg = st_num_arg;
      
      (*oefun_table[instruction]) ();

      if (expected_stack != sp)
        fatal("Bad stack after efun. Instruction %d, num arg %d\n",
              instruction, num_arg);
      instruction += ONEARG_MAX;
#endif
    } /* switch (instruction) */
    DEBUG_CHECK1(sp < fp + csp->num_local_variables - 1,
                 "Bad stack after evaluation. Instruction %d\n",
                 instruction);
  } /* while (1) */
}

static void
do_catch (char * pc, unsigned short new_pc_offset)
{
  error_context_t econ;
    
  /*
   * Save some global variables that must be restored separately after a
   * longjmp. The stack will have to be manually popped all the way.
   */
  if (!save_context(&econ))
    error("Can't catch too deep recursion error.\n");
  push_control_stack(FRAME_CATCH);
  csp->pc = current_prog->program + new_pc_offset;
#if defined(DEBUG) || defined(TRACE_CODE)
  csp->num_local_variables = (csp - 1)->num_local_variables; /* marion */
#endif

  if (SETJMP(econ.context)) {
    /*
     * They did a throw() or error. That means that the control stack
     * must be restored manually here.
     */
    restore_context(&econ);
    STACK_INC;
    *sp = catch_value;
    catch_value = const1;

    /* if it's too deep or max eval, we can't let them catch it */
    if (max_eval_error) {
      pop_context(&econ);
      error("Can't catch eval cost too big error.\n");
    }
    if (0 && too_deep_error) {//can't we??
      pop_context(&econ);
      error("Can't catch too deep recursion error.\n");
    }
  } else {
    assign_svalue(&catch_value, &const1);
    /* note, this will work, since csp->extern_call won't be used */
    eval_instruction(pc);
  }
  pop_context(&econ);
}

static program_t *ffbn_recurse (program_t * prog, char * name,
                                  int * indexp, int * runtime_index) {
  register int high = prog->num_functions_defined - 1;
  register int low = 0, mid;
  int ri;
  char *p;
    
  /* Search our function table */
  while (high >= low) {
    mid = (high + low) >> 1;
    p = prog->function_table[mid].funcname;
    if (name < p) high = mid - 1;
    else if (name > p) low = mid + 1;
    else {
      ri = mid + prog->last_inherited;
      
      if (prog->function_flags[ri] & 
          (FUNC_UNDEFINED | FUNC_PROTOTYPE)) {
        return 0;
      }
        
      *indexp = mid;
      *runtime_index = ri;
      return prog;
    }
  }

  /* Search inherited function tables */
  mid = prog->num_inherited;
  while (mid--) {
    program_t *ret = ffbn_recurse(prog->inherit[mid].prog, name, indexp, 
                                  runtime_index);
    if (ret) {
      *runtime_index += prog->inherit[mid].function_index_offset;
      return ret;
    }
  }
  return 0;
}

static program_t *ffbn_recurse2 (program_t * prog, const char * name,
                                   int * indexp, int * runtime_index,
                                   int * fio, int * vio) {
  register int high = prog->num_functions_defined - 1;
  register int low = 0, mid;
  int ri;
  char *p;

  /* Search our function table */
  while (high >= low) {
    mid = (high + low) >> 1;
    p = prog->function_table[mid].funcname;
    if (name < p) high = mid - 1;
    else if (name > p) low = mid + 1;
    else {
      ri = mid + prog->last_inherited;
        
      if (prog->function_flags[ri] &
          (FUNC_UNDEFINED | FUNC_PROTOTYPE)) {
        return 0;
      }

      *indexp = mid;
      *runtime_index = ri;
      *fio = *vio = 0;
      return prog;
    }
  }

  /* Search inherited function tables */
  mid = prog->num_inherited;
  while (mid--) {
    program_t *ret = ffbn_recurse2(prog->inherit[mid].prog, name, indexp, 
                                   runtime_index, fio, vio);
    if (ret) {
      *runtime_index += prog->inherit[mid].function_index_offset;
      *fio += prog->inherit[mid].function_index_offset;
      *vio += prog->inherit[mid].variable_index_offset;
      return ret;
    }
  }
  return 0;
}

INLINE program_t *
find_function_by_name (object_t * ob, const char * name, 
                         int * indexp, int * runtime_index) {
  char *funname = findstring(name);
    
  if (!funname) return 0;
  return ffbn_recurse(ob->prog, funname, indexp, runtime_index);
}

INLINE_STATIC program_t *
find_function_by_name2 (object_t * ob, const char ** name, 
                          int * indexp, int * runtime_index, 
                          int * fio, int * vio) {
  if (!(*name = findstring(*name))) return 0;
  return ffbn_recurse2(ob->prog, *name, indexp, runtime_index, fio, vio);
}



/*
 * Apply a fun 'fun' to the program in object 'ob', with
 * 'num_arg' arguments (already pushed on the stack).
 * If the function is not found, search in the object pointed to by the
 * inherit pointer.
 * If the function name starts with '::', search in the object pointed out
 * through the inherit pointer by the current object. The 'current_object'
 * stores the base object, not the object that has the current function being
 * evaluated. Thus, the variable current_prog will normally be the same as
 * current_object->prog, but not when executing inherited code. Then,
 * it will point to the code of the inherited object. As more than one
 * object can be inherited, the call of function by index number has to
 * be adjusted. The function number 0 in a superclass object must not remain
 * number 0 when it is inherited from a subclass object. The same problem
 * exists for variables. The global variables function_index_offset and
 * variable_index_offset keep track of how much to adjust the index when
 * executing code in the superclass objects.
 *
 * There is a special case when called from the heart beat, as
 * current_prog will be 0. When it is 0, set current_prog
 * to the 'ob->prog' sent as argument.
 *
 * Arguments are always removed from the stack.
 * If the function is not found, return 0 and nothing on the stack.
 * Otherwise, return 1, and a pushed return value on the stack.
 *
 * Note that the object 'ob' can be destructed. This must be handled by
 * the caller of apply().
 *
 * If the function failed to be called, then arguments must be deallocated
 * manually !  (Look towards end of this function.)
 */

#ifdef DEBUG
static char debug_apply_fun[30];/* For debugging */
#endif

#ifdef CACHE_STATS
unsigned int apply_low_call_others = 0;
unsigned int apply_low_cache_hits = 0;
unsigned int apply_low_slots_used = 0;
unsigned int apply_low_collisions = 0;
#endif

typedef struct cache_entry_s {
  program_t *oprogp;
  program_t *progp;
  function_t *funp;
  unsigned short function_index_offset;
  unsigned short variable_index_offset;
} cache_entry_t;

static cache_entry_t cache[APPLY_CACHE_SIZE];

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_apply_low_cache() {
  int i;
  for (i = 0; i < APPLY_CACHE_SIZE; i++) {
    if (cache[i].funp && !cache[i].progp)
      EXTRA_REF(BLOCK((char *)cache[i].funp))++;
    if (cache[i].oprogp)
      cache[i].oprogp->extra_ref++;
    if (cache[i].progp)
      cache[i].progp->extra_ref++;
  }
}
#endif

void check_co_args2 (unsigned short * types, int num_arg, char * name, char * ob_name){
  int argc = num_arg;
  int exptype, i = 0;
  do{
    argc--;
    exptype = convert_type(types[i++]);
    if(exptype == T_ANY)
      continue;
    
    if((sp-argc)->type != exptype){
      char buf[1024];
      if((sp-argc)->type == T_NUMBER && !(sp-argc)->u.number)
        continue;
      sprintf(buf, "Bad argument %d in call to %s() in %s\nExpected: %s Got %s.\n",
              num_arg - argc, name, ob_name, 
              type_name(exptype), type_name((sp-argc)->type));
#ifdef CALL_OTHER_WARN
      if(current_prog){
        const char *file;
        int line;
        find_line(pc, current_prog, &file, &line);
        smart_log(file, line, buf, 1);
      } else 
        smart_log("driver", 0, buf, 1);
#else
      error(buf);
#endif
    }
  } while (argc);
}

void check_co_args (int num_arg, const program_t * prog, function_t * fun, int findex) {
#ifdef CALL_OTHER_TYPE_CHECK
  if(num_arg != fun->num_arg){
    char buf[1024];
    //if(!current_prog) what do i need this for again?
    // current_prog = master_ob->prog;
    sprintf(buf, "Wrong number of arguments to %s in %s.\n", fun->funcname, prog->filename);
#ifdef CALL_OTHER_WARN
    if(current_prog){
      const char *file;
      int line;
      find_line(pc, current_prog, &file, &line);
      smart_log(file, line, buf, 1);
    } else
      smart_log("driver", 0, buf, 1);
#else
    error(buf);
#endif
  }
          
  if(num_arg && prog->type_start &&
     prog->type_start[findex] != INDEX_START_NONE)
    check_co_args2(&prog->argument_types[prog->type_start[findex]], num_arg,
                   fun->funcname, prog->filename);
#endif
}


int apply_low (const char * fun, object_t * ob, int num_arg)
{
  /*
   * static memory is initialized to zero by the system or so Jacques says
   * :)
   */
  const char *sfun;
  cache_entry_t *entry;
  program_t *progp, *prog;
  int ix;
  POINTER_INT pfun, pprog;
  static int cache_mask = APPLY_CACHE_SIZE - 1;
  int local_call_origin = call_origin;
  IF_DEBUG(control_stack_t *save_csp);
  
  if (!local_call_origin)
    local_call_origin = ORIGIN_DRIVER;
  call_origin = 0;
  ob->time_of_ref = current_time; /* Used by the swapper */
  /*
   * This object will now be used, and is thus a target for reset later on
   * (when time due).
   */
#if !defined(NO_RESETS) && defined(LAZY_RESETS)
  try_reset(ob);
#endif
  if (ob->flags & O_DESTRUCTED) {
    pop_n_elems(num_arg);
    return 0;
  }
  
  ob->flags &= ~O_RESET_STATE;
#ifdef DEBUG
  strncpy(debug_apply_fun, fun, sizeof(debug_apply_fun));
  debug_apply_fun[sizeof debug_apply_fun - 1] = '\0';
#endif
  /*
   * If there is a chain of objects shadowing, start with the first of
   * these.
   */
#ifndef NO_SHADOWS
  while (ob->shadowed && ob->shadowed != current_object && 
         (!(ob->shadowed->flags & O_DESTRUCTED)))
    ob = ob->shadowed;
 retry_for_shadow:
#endif
  
  progp = ob->prog;
  DEBUG_CHECK(ob->flags & O_DESTRUCTED,"apply() on destructed object\n");
#ifdef CACHE_STATS
  apply_low_call_others++;
#endif
  pfun = (POINTER_INT)fun;
  pprog = (POINTER_INT)progp;
  ix = (pfun >> 2)^(pfun >> (2 + APPLY_CACHE_BITS))^(pprog >> 2)^(pprog >> (2 + APPLY_CACHE_BITS));
  entry = &cache[ix & cache_mask];
  if (entry->oprogp == progp && 
      (entry->progp ? (strcmp(entry->funp->funcname, fun) == 0) :
       strcmp((char *)entry->funp, fun) == 0)) {
#ifdef CACHE_STATS
    apply_low_cache_hits++;
#endif
    
    /* if progp is zero, the cache is telling us the function isn't here*/
    if (entry->progp) {
      int need;
      function_t *funp = entry->funp;
      int findex = (funp - entry->progp->function_table);
      int funflags, runtime_index;
      
      runtime_index = findex + entry->progp->last_inherited + entry->function_index_offset;
      funflags = entry->oprogp->function_flags[runtime_index];
      
      need = (local_call_origin == ORIGIN_DRIVER ? DECL_HIDDEN : ((current_object == ob || local_call_origin == ORIGIN_INTERNAL) ? DECL_PROTECTED : DECL_PUBLIC));

      if ((funflags & DECL_ACCESS) >= need) {
        /*
         * the cache will tell us in which program the function is,
         * and where
         */
        if(!(funflags & FUNC_VARARGS))
          check_co_args(num_arg, entry->progp, funp, findex);
        
        push_control_stack(FRAME_FUNCTION | FRAME_OB_CHANGE);
        current_prog = entry->progp;
        caller_type = local_call_origin;
        csp->num_local_variables = num_arg;
        function_index_offset = entry->function_index_offset;
        variable_index_offset = entry->variable_index_offset;
        
        csp->fr.table_index = findex;
#ifdef PROFILE_FUNCTIONS
        get_cpu_times(&(csp->entry_secs), &(csp->entry_usecs));
        current_prog->function_table[findex].calls++;
#endif

        if (funflags & FUNC_TRUE_VARARGS)
          setup_varargs_variables(csp->num_local_variables,
                                  funp->num_local, funp->num_arg);
        else
          setup_variables(csp->num_local_variables,
                          funp->num_local, funp->num_arg);
#ifdef TRACE
        tracedepth++;
        if (TRACEP(TRACE_CALL)) {
          do_trace_call(findex);
        }
#endif

        previous_ob = current_object;
        current_object = ob;
        IF_DEBUG(save_csp = csp);
        call_program(current_prog, funp->address);
        
        DEBUG_CHECK(save_csp - 1 != csp, 
                    "Bad csp after execution in apply_low.\n");
        return 1;
      }
    } /* when we come here, the cache has told us
       * that the function isn't defined in the
       * object */
  } else {
    int findex, runtime_index, fio, vio;
    /* we have to search the function */

    if (entry->oprogp) {
      free_prog(&entry->oprogp);
      entry->oprogp = 0;
    }
    if (entry->progp) {
      free_prog(&entry->progp);
      entry->progp = 0;
    } else {
      if (entry->funp){
        free_string((char *)entry->funp);
	entry->funp = 0;
      }
    }
  
#ifdef CACHE_STATS
    if (!entry->funp) {
      apply_low_slots_used++;
    } else {
      apply_low_collisions++;
    }
#endif
    sfun = fun;
    prog = find_function_by_name2(ob, &sfun, &findex, &runtime_index,
                                  &fio, &vio);
    
    if (prog) {
      int need;
      function_t *funp = &prog->function_table[findex];
      int funflags = ob->prog->function_flags[runtime_index];
      
      need = (local_call_origin == ORIGIN_DRIVER ? DECL_HIDDEN : ((current_object == ob || local_call_origin == ORIGIN_INTERNAL) ? DECL_PROTECTED : DECL_PUBLIC));
      
      if ((funflags & DECL_ACCESS) >= need) {
        
        if(!(funflags & FUNC_VARARGS))
          check_co_args(num_arg, prog, funp, findex);
        
        push_control_stack(FRAME_FUNCTION | FRAME_OB_CHANGE);
        current_prog = prog;
        caller_type = local_call_origin;
        /* The searched function is found */
        entry->oprogp = ob->prog;
        entry->funp = funp;
        csp->fr.table_index = findex;
#ifdef PROFILE_FUNCTIONS
        get_cpu_times(&(csp->entry_secs), &(csp->entry_usecs));
        current_prog->function_table[findex].calls++;
#endif
        csp->num_local_variables = num_arg;
        entry->variable_index_offset = variable_index_offset = vio;
        entry->function_index_offset = function_index_offset = fio;
        if (funflags & FUNC_TRUE_VARARGS)
          setup_varargs_variables(csp->num_local_variables,
                                  funp->num_local, 
                                  funp->num_arg);
        else
          setup_variables(csp->num_local_variables,
                          funp->num_local, 
                          funp->num_arg);
        entry->progp = current_prog;
        /* previously, programs had an id_number so they
         * didn't have be refed while in the cache.  This is
         * phenomenally stupid, since it wastes 4
         * bytes/program and 4 bytes/cache entry just to save
         * an instruction or two.  Actually, less, since
         * updating the ref count is as quick, or quicker,
         * than checking the id.
         *
         * The other solution is to clear the cache like the
         * stack is cleared when objects destruct.  However, that
         * can be expensive, since the cache can be quite large.
         * [the stack is typically quite small]
         *
         * This does have the side effect that checking refs no
         * longer tells you if a program is inherited by any other
         * program, but most uses can cope (see appropriate comments).
         */
        reference_prog(entry->oprogp, "apply_low() cache [oprogp]");
        reference_prog(entry->progp, "apply_low() cache [progp]");
        previous_ob = current_object;
        current_object = ob;
        IF_DEBUG(save_csp = csp);
        call_program(current_prog, funp->address);
        
        DEBUG_CHECK(save_csp - 1 != csp,
                    "Bad csp after execution in apply_low\n");
        /*
         * Arguments and local variables are now removed. One
         * resulting value is always returned on the stack.
         */
        return 1;
      } 
    }

    /* We have to mark a function not to be in the object */
    entry->oprogp = progp;
    reference_prog(entry->oprogp, "apply_low() cache [oprogp miss]");
    if (sfun) {
      ref_string(sfun);
      entry->funp = (function_t *)sfun;
    } else
      entry->funp = (function_t *)make_shared_string(fun);
    entry->progp = 0;
  }
#ifndef NO_SHADOWS
  if (ob->shadowing) {
    /*
     * This is an object shadowing another. The function was not
     * found, but can maybe be found in the object we are shadowing.
     */
    ob = ob->shadowing;
    goto retry_for_shadow;
  }
#endif
  /* Failure. Deallocate stack. */
  pop_n_elems(num_arg);
  return 0;
}

/*
 * Arguments are supposed to be
 * pushed (using push_string() etc) before the call. A pointer to a
 * 'svalue_t' will be returned. It will be a null pointer if the called
 * function was not found. Otherwise, it will be a pointer to a static
 * area in apply(), which will be overwritten by the next call to apply.
 * Reference counts will be updated for this value, to ensure that no pointers
 * are deallocated.
 */
    
svalue_t *apply (const char * fun, object_t * ob, int num_arg,
                   int where)
{
  IF_DEBUG(svalue_t *expected_sp);
    
  tracedepth = 0;
  call_origin = where;
    
#ifdef TRACE
  if (TRACEP(TRACE_APPLY)) {
    static int inapply = 0;
    if(!inapply){
      inapply = 1;
      do_trace("Apply", "", "\n");
      inapply = 0;
    }
  }
#endif
    
  IF_DEBUG(expected_sp = sp - num_arg);
  if (apply_low(fun, ob, num_arg) == 0)
    return 0;
  free_svalue(&apply_ret_value, "sapply");
  apply_ret_value = *sp--;
  DEBUG_CHECK(expected_sp != sp,
              "Corrupt stack pointer.\n");
  return &apply_ret_value;
}

/* Reason for the following 1. save cache space 2. speed :) */
/* The following is to be called only from reset_object for */
/* otherwise extra checks are needed - Sym                  */

void call___INIT (object_t * ob)
{
  program_t *progp;
  function_t *cfp;
  int num_functions;
  IF_DEBUG(svalue_t *expected_sp);
  IF_DEBUG(control_stack_t *save_csp);
    
  tracedepth = 0;
    
#ifdef TRACE
  if (TRACEP(TRACE_APPLY)) {
    do_trace("Apply", "", "\n");
  }
#endif
    
  IF_DEBUG(expected_sp = sp);
    
  /* No try_reset here for obvious reasons :) */
    
  ob->flags &= ~O_RESET_STATE;
    
  progp = ob->prog;
  num_functions = progp->num_functions_defined;
  if (!num_functions) return;
    
  /* ___INIT turns out to be always the last function */
  cfp = &progp->function_table[num_functions - 1];
  if (cfp->funcname[0] != APPLY___INIT_SPECIAL_CHAR) return;
  push_control_stack(FRAME_FUNCTION | FRAME_OB_CHANGE);
  current_prog = progp;
  csp->fr.table_index = num_functions - 1;
#ifdef PROFILE_FUNCTIONS
  get_cpu_times(&(csp->entry_secs), &(csp->entry_usecs));
  current_prog->function_table[num_functions - 1].calls++;
#endif
  caller_type = ORIGIN_DRIVER;
  csp->num_local_variables = 0;
    
  setup_new_frame(num_functions - 1 + progp->last_inherited);
  previous_ob = current_object;
    
  current_object = ob;
  IF_DEBUG(save_csp = csp);
  call_program(current_prog, cfp->address);
    
  DEBUG_CHECK(save_csp - 1 != csp,
              "Bad csp after execution in apply_low\n");
  sp--;
  DEBUG_CHECK(expected_sp != sp,
              "Corrupt stack pointer.\n");
}

/*
 * this is a "safe" version of apply
 * this allows you to have dangerous driver mudlib dependencies
 * and not have to worry about causing serious bugs when errors occur in the
 * applied function and the driver depends on being able to do something
 * after the apply. (such as the ed exit function, and the net_dead function).
 * note: this function uses setjmp() and thus is fairly expensive when
 * compared to a normal apply().  Use sparingly.
 */

svalue_t *
safe_apply (const char * fun, object_t * ob, int num_arg, int where)
{
  svalue_t *ret;
  error_context_t econ;

  if (!save_context(&econ))
    return 0;
  if (!SETJMP(econ.context)) {
    if (!(ob->flags & O_DESTRUCTED)) {
      ret = apply(fun, ob, num_arg, where);
    } else ret = 0;
  } else {
    restore_context(&econ);
    pop_n_elems(num_arg); /* saved state had args on stack already */
    ret = 0;
  }
  pop_context(&econ);
  return ret;
}

/*
 * Call a function in all objects in a array.
 */
array_t *call_all_other (array_t * v, const char * func, int numargs)
{
  int size;
  svalue_t *tmp, *vptr, *rptr;
  array_t *ret;
  object_t *ob;
  int i;

  tmp = sp;
  STACK_INC;
  sp->type = T_ARRAY;
  sp->u.arr = ret = allocate_array(size = v->size);
  CHECK_STACK_OVERFLOW(numargs);
  for (vptr = v->item, rptr = ret->item; size--; vptr++, rptr++) {
    if (vptr->type == T_OBJECT) {
      ob = vptr->u.ob;
    } else if (vptr->type == T_STRING) {
      ob = find_object(vptr->u.string);
      if (!ob || !object_visible(ob))
        continue;
    } else continue;
    if (ob->flags & O_DESTRUCTED) 
      continue;
    i = numargs;
    while (i--) push_svalue(tmp - i);
    call_origin = ORIGIN_CALL_OTHER;
    if (apply_low(func, ob, numargs)) *rptr = *sp--;
  }
  sp--;
  pop_n_elems(numargs);
  return ret;
}

char *function_name (program_t * prog, int findex) {
  register int low, high, mid;

  /* Walk up the inheritance tree to the real definition */ 
  if (prog->function_flags[findex] & FUNC_ALIAS) {
    findex = prog->function_flags[findex] & ~FUNC_ALIAS;
  }
    
  while (prog->function_flags[findex] & FUNC_INHERITED) {
    low = 0;
    high = prog->num_inherited -1;
  
    while (high > low) {
      mid = (low + high + 1) >> 1;
      if (prog->inherit[mid].function_index_offset > findex)
        high = mid -1;
      else low = mid;
    }
    findex -= prog->inherit[low].function_index_offset;
    prog = prog->inherit[low].prog;
  }
    
  findex -= prog->last_inherited;

  return prog->function_table[findex].funcname;
}
 
static void get_trace_details (const program_t * prog, int findex,
                                 char ** fname, int * na, int * nl) {
  function_t *cfp = &prog->function_table[findex];

  *fname = cfp->funcname;
  *na = cfp->num_arg;
  *nl = cfp->num_local;
}

/*
 * This function is similar to apply(), except that it will not
 * call the function, only return object name if the function exists,
 * or 0 otherwise.  If flag is nonzero, then we admit static and private
 * functions exist.  Note that if you actually intend to call the function,
 * it's faster to just try to call it and check if apply() returns zero.
 */
const char *function_exists (const char * fun, object_t * ob, int flag) {
  int findex, runtime_index;
  program_t *prog;
  int flags;
    
  DEBUG_CHECK(ob->flags & O_DESTRUCTED,
              "function_exists() on destructed object\n");
    
  if (fun[0] == APPLY___INIT_SPECIAL_CHAR)
    return 0;

  prog = find_function_by_name(ob, fun, &findex, &runtime_index);
  if (!prog) return 0;

  flags = ob->prog->function_flags[runtime_index];
    
  if ((flags & FUNC_UNDEFINED) || (!flag && (flags & (DECL_PROTECTED|DECL_PRIVATE|DECL_HIDDEN))))
    return 0;
    
  return prog->filename;
}
    
#ifndef NO_SHADOWS
/*
  is_static: returns 1 if a function named 'fun' is declared 'static' in 'ob';
  0 otherwise.
*/
int is_static (const char *fun, object_t * ob)
{
  int findex;
  int runtime_index;
  program_t *prog;
  int flags;
    
  DEBUG_CHECK(ob->flags & O_DESTRUCTED,
              "is_static() on destructed object\n");

  prog = find_function_by_name(ob, fun, &findex, &runtime_index);
  if (!prog) return 0;
    
  flags = ob->prog->function_flags[runtime_index];
  if (flags & (FUNC_UNDEFINED|FUNC_PROTOTYPE))
    return 0;
  if (flags & (DECL_PROTECTED|DECL_PRIVATE|DECL_HIDDEN))
    return 1;

  return 0;
}
#endif
  
/*
 * Call a function by object and index number.  Used by parts of the
 * driver which cache function numbers to optimize away function lookup.
 * The return value is left on the stack.
 * Currently: heart_beats, simul_efuns, master applies.
 */
void call_direct (object_t * ob, int offset, int origin, int num_arg) {
  function_t *funp;
  program_t *prog = ob->prog;

  ob->time_of_ref = current_time;
  push_control_stack(FRAME_FUNCTION | FRAME_OB_CHANGE);
  caller_type = origin;
  csp->num_local_variables = num_arg;
  current_prog = prog;
  funp = setup_new_frame(offset);
  previous_ob = current_object;
  current_object = ob;
  call_program(current_prog, funp->address);
}

void translate_absolute_line (int abs_line, unsigned short * file_info,
                                int * ret_file, int * ret_line) {
  unsigned short *p1, *p2;
  int file;
  int line_tmp = abs_line;
    
  /* two passes: first, find out what file we're interested in */
  p1 = file_info;
  while (line_tmp > *p1) {
    line_tmp -= *p1;
    p1 += 2;
  }
  file = p1[1];
    
  /* now correct the line number for that file */
  p2 = file_info;
  while (p2 < p1) {
    if (p2[1] == file)
      line_tmp += *p2;
    p2 += 2;
  }
  *ret_line = line_tmp;
  *ret_file = file;
}

static int find_line (char * p, const program_t * progp,
                        const char ** ret_file, int * ret_line )
{
  int offset;
  unsigned char *lns;
  ADDRESS_TYPE abs_line;
  int file_idx;
    
  *ret_file = 0;
  *ret_line = 0;
    
  if (!progp) return 1;
  if (progp == &fake_prog) return 2;
    
  if (!progp->line_info) 
      return 4;

  offset = p - progp->program;
  DEBUG_CHECK2(offset > progp->program_size,
               "Illegal offset %d in object /%s\n", offset, progp->filename);
    
  lns = progp->line_info;
  while (offset > *lns) {
    offset -= *lns;
    lns += (sizeof(ADDRESS_TYPE) + 1);
  }
    
#if !defined(USE_32BIT_ADDRESSES) 
  COPY_SHORT(&abs_line, lns + 1);
#else
  COPY4(&abs_line, lns + 1);
#endif
    
  translate_absolute_line(abs_line, &progp->file_info[2], 
                          &file_idx, ret_line);
    
  *ret_file = progp->strings[file_idx - 1];
  return 0;
}

static void get_explicit_line_number_info (char * p, const program_t * prog,
                                             const char ** ret_file, int * ret_line) {
  find_line(p, prog, ret_file, ret_line);
  if (!(*ret_file))
    *ret_file = prog->filename;
}

void get_line_number_info (const char ** ret_file, int * ret_line)
{
  find_line(pc, current_prog, ret_file, ret_line);
  if (!(*ret_file))
    *ret_file = current_prog->filename;
}

char* get_line_number (char * p, const program_t * progp)
{
  static char buf[256];
  int i;
  const char *file;
  int line;

  i = find_line(p, progp, &file, &line);
    
  switch (i) {
  case 1:
    strcpy(buf, "(no program)");
    return buf;
  case 2: 
    *buf = 0;
    return buf;
  case 3:
    strcpy(buf, "(compiled program)");
    return buf;
  case 4:
    strcpy(buf, "(no line numbers)");
    return buf;
  case 5:
    strcpy(buf, "(includes too deep)");
    return buf;
  }
  if (!file)
    file = progp->filename;
  sprintf(buf, "/%s:%d", file, line);
  return buf;
}

static void dump_trace_line (const char * fname, const char * pname,
                               const char * const obname, char * where) {
  char line[256];
  char *end = EndOf(line);
  char *p;

  p = strput(line, end, "Object: ");
  if (obname[0] != '<' && p < end)
    *p++ = '/';
  p = strput(p, end, obname);
  p = strput(p, end, ", Program: ");
  if (pname[0] != '<' && p < end)
    *p++ = '/';
  p = strput(p, end, pname);
  p = strput(p, end, "\n   in ");
  p = strput(p, end, fname);
  p = strput(p, end, "() at ");
  p = strput(p, end, where);
  p = strput(p, end, "\n");
  debug_message(line);
}

/*
 * Write out a trace. If there is a heart_beat(), then return the
 * object that had that heart beat.
 */
const char *dump_trace (int how)
{
  control_stack_t *p;
  const char * ret = 0;
  char *fname;
  int num_arg = -1, num_local = -1;
    
#if defined(ARGUMENTS_IN_TRACEBACK) || defined(LOCALS_IN_TRACEBACK)
  svalue_t *ptr;
  int i, context_saved = 0;
  error_context_t econ;
#endif

  if (current_prog == 0)
    return 0;
  if (csp < &control_stack[0]) {
    return 0;
  }

#if defined(ARGUMENTS_IN_TRACEBACK) || defined(LOCALS_IN_TRACEBACK)
  /*
   * save context here because svalue_to_string could generate an error
   * which would throw us into a bad state in the error handler.  this
   * will allow us to recover cleanly.  Don't bother if we're in a
   * eval cost exceeded or too deep recursion state because (s)printf
   * won't make the object_name() apply and save_context() might fail
   * here (too deep recursion)
   */
  if (!too_deep_error) {
    if (!save_context(&econ))
      return 0;
    context_saved = 1;
    if (SETJMP(econ.context)) {
      restore_context(&econ);
      pop_context(&econ);
      return 0;
    }
  }
#endif

#ifdef TRACE_CODE
  if (how)
    last_instructions();
#endif
  debug_message("--- trace ---\n");
  for (p = &control_stack[0]; p < csp; p++) {
    switch (p[0].framekind & FRAME_MASK) {
    case FRAME_FUNCTION:
      get_trace_details(p[1].prog, p[0].fr.table_index,
                        &fname, &num_arg, &num_local);
      dump_trace_line(fname, p[1].prog->filename, p[1].ob->obname,
                      get_line_number(p[1].pc, p[1].prog));
      if (strcmp(fname, "heart_beat") == 0)
        ret = p->ob ? p->ob->obname : 0;
      break;
    case FRAME_FUNP:
      {
        outbuffer_t tmpbuf;
        svalue_t tmpval;
      
        tmpbuf.real_size = 0;
        tmpbuf.buffer = 0;
      
        tmpval.type = T_FUNCTION;
        tmpval.u.fp = p[0].fr.funp;

        svalue_to_string(&tmpval, &tmpbuf, 0, 0, 0);

        dump_trace_line(tmpbuf.buffer, p[1].prog->filename, p[1].ob->obname,
                        get_line_number(p[1].pc, p[1].prog));

        FREE_MSTR(tmpbuf.buffer);
        num_arg = p[0].fr.funp->f.functional.num_arg;
        num_local = p[0].fr.funp->f.functional.num_local;
      }
      break;
    case FRAME_FAKE:
      dump_trace_line("<fake>", p[1].prog->filename, p[1].ob->obname,
                      get_line_number(p[1].pc, p[1].prog));
      num_arg = -1;
      break;
    case FRAME_CATCH:
      dump_trace_line("<catch>", p[1].prog->filename, p[1].ob->obname,
                      get_line_number(p[1].pc, p[1].prog));
      num_arg = -1;
      break;
#ifdef DEBUG
    default:
      fatal("unknown type of frame\n");
#endif
    }
#ifdef ARGUMENTS_IN_TRACEBACK
    if (num_arg != -1) {
      ptr = p[1].fp;
      debug_message("arguments were (");
      for (i = 0; i < num_arg; i++) {
        outbuffer_t outbuf;

        if (i) {
          debug_message(",");
        }
        outbuf_zero(&outbuf);
        svalue_to_string(&ptr[i], &outbuf, 0, 0, 0);
        /* don't need to fix length here */
        debug_message("%s", outbuf.buffer);
        FREE_MSTR(outbuf.buffer);
      }
      debug_message(")\n");
    }
#endif
#ifdef LOCALS_IN_TRACEBACK
    if (num_local > 0 && num_arg != -1) {
      ptr = p[1].fp + num_arg;
      debug_message("locals were: ");
      for (i = 0; i < num_local; i++) {
        outbuffer_t outbuf;
        
        if (i) {
          debug_message(",");
        }
        outbuf_zero(&outbuf);
        svalue_to_string(&ptr[i], &outbuf, 0, 0, 0);
        /* no need to fix length */
        debug_message("%s", outbuf.buffer);
        FREE_MSTR(outbuf.buffer);
      } 
      debug_message("\n");
    }
#endif
  }
  switch (p[0].framekind & FRAME_MASK) {
  case FRAME_FUNCTION:
    get_trace_details(current_prog, p[0].fr.table_index,
                      &fname, &num_arg, &num_local);
    debug_message("'%15s' in '/%20s' ('/%20s') %s\n",
                  fname, current_prog->filename, current_object->obname,
                  get_line_number(pc, current_prog));
    break;
  case FRAME_FUNP:
    {
      outbuffer_t tmpbuf;
      svalue_t tmpval;
      
      tmpbuf.real_size = 0;
      tmpbuf.buffer = 0;
      
      tmpval.type = T_FUNCTION;
      tmpval.u.fp = p[0].fr.funp;

      svalue_to_string(&tmpval, &tmpbuf, 0, 0, 0);

      debug_message("'%s' in '/%20s' ('/%20s') %s\n",
                    tmpbuf.buffer,
                    current_prog->filename, current_object->obname,
                    get_line_number(pc, current_prog));
      FREE_MSTR(tmpbuf.buffer);
      num_arg = p[0].fr.funp->f.functional.num_arg;
      num_local = p[0].fr.funp->f.functional.num_local;
    }
    break;
  case FRAME_FAKE:
    debug_message("'     <fake>' in '/%20s' ('/%20s') %s\n",
                  current_prog->filename, current_object->obname,
                  get_line_number(pc, current_prog));
    num_arg = -1;
    break;
  case FRAME_CATCH:
    debug_message("'          CATCH' in '/%20s' ('/%20s') %s\n",
                  current_prog->filename, current_object->obname,
                  get_line_number(pc, current_prog));
    num_arg = -1;
    break;
  }
#ifdef ARGUMENTS_IN_TRACEBACK
  if (num_arg != -1) {
    debug_message("arguments were (");
    for (i = 0; i < num_arg; i++) {
      outbuffer_t outbuf;

      if (i) {
        debug_message(",");
      }
      outbuf_zero(&outbuf);
      svalue_to_string(&fp[i], &outbuf, 0, 0, 0);
      /* no need to fix length */
      debug_message("%s", outbuf.buffer);
      FREE_MSTR(outbuf.buffer);
    }
    debug_message(")\n");
  }
#endif
#ifdef LOCALS_IN_TRACEBACK
  if (num_local > 0 && num_arg != -1) {
    ptr = fp + num_arg;
    debug_message("locals were: ");
    for (i = 0; i < num_local; i++) {
      outbuffer_t outbuf;

      if (i) {
        debug_message(",");
      }
      outbuf_zero(&outbuf);
      svalue_to_string(&ptr[i], &outbuf, 0, 0, 0);
      /* no need to fix length */
      debug_message("%s", outbuf.buffer);
      FREE_MSTR(outbuf.buffer);
    }
    debug_message("\n");
  }
#endif
  debug_message("--- end trace ---\n");
#if defined(ARGUMENTS_IN_TRACEBACK) || defined(LOCALS_IN_TRACEBACK)
  if (context_saved)
    pop_context(&econ);
#endif
  return ret;
}

array_t *get_svalue_trace()
{
  control_stack_t *p;
  array_t *v;
  mapping_t *m;
  const char *file;
  int line;
  char *fname;
  int num_arg, num_local = -1;
    
#if defined(ARGUMENTS_IN_TRACEBACK) || defined(LOCALS_IN_TRACEBACK)
  svalue_t *ptr;
  int i;
#endif

  if (current_prog == 0)
    return &the_null_array;
  if (csp < &control_stack[0]) {
    return &the_null_array;
  }
  v = allocate_empty_array((csp - &control_stack[0]) + 1);
  for (p = &control_stack[0]; p < csp; p++) {
    m = allocate_mapping(6);
    switch (p[0].framekind & FRAME_MASK) {
    case FRAME_FUNCTION:
      get_trace_details(p[1].prog, p[0].fr.table_index,
                        &fname, &num_arg, &num_local);
      add_mapping_string(m, "function", fname);
      break;
    case FRAME_CATCH:
      add_mapping_string(m, "function", "CATCH");
      num_arg = -1;
      break;
    case FRAME_FAKE:
      add_mapping_string(m, "function", "<fake>");
      num_arg = -1;
      break;
    case FRAME_FUNP:
      {
        outbuffer_t tmpbuf;
        svalue_t tmpval;
      
        tmpbuf.real_size = 0;
        tmpbuf.buffer = 0;
      
        tmpval.type = T_FUNCTION;
        tmpval.u.fp = p[0].fr.funp;

        svalue_to_string(&tmpval, &tmpbuf, 0, 0, 0);

        add_mapping_string(m, "function", tmpbuf.buffer);
        FREE_MSTR(tmpbuf.buffer);
        num_arg = p[0].fr.funp->f.functional.num_arg;
        num_local = p[0].fr.funp->f.functional.num_local;
      }
      break;
#ifdef DEBUG
    default:
      fatal("unknown type of frame\n");
#endif
    }
    add_mapping_malloced_string(m, "program", add_slash(p[1].prog->filename));
    add_mapping_object(m, "object", p[1].ob);
    get_explicit_line_number_info(p[1].pc, p[1].prog, &file, &line);
    add_mapping_malloced_string(m, "file", add_slash(file));
    add_mapping_pair(m, "line", line);
#ifdef ARGUMENTS_IN_TRACEBACK
    if (num_arg != -1) {
      array_t *v2;

      ptr = p[1].fp;
      v2 = allocate_empty_array(num_arg);
      for (i = 0; i < num_arg; i++) {
        assign_svalue_no_free(&v2->item[i], &ptr[i]);
      }
      add_mapping_array(m, "arguments", v2);
      v2->ref--;
    }
#endif
#ifdef LOCALS_IN_TRACEBACK
    if (num_local > 0 && num_arg != -1) {
      array_t *v2;

      ptr = p[1].fp + num_arg;
      v2 = allocate_empty_array(num_local);
      for (i = 0; i < num_local; i++) {
        assign_svalue_no_free(&v2->item[i], &ptr[i]);
      }
      add_mapping_array(m, "locals", v2);
      v2->ref--;
    }
#endif
    v->item[(p - &control_stack[0])].type = T_MAPPING;
    v->item[(p - &control_stack[0])].u.map = m;
  }
  m = allocate_mapping(6);
  switch (p[0].framekind & FRAME_MASK) {
  case FRAME_FUNCTION:
    get_trace_details(current_prog, p[0].fr.table_index,
                      &fname, &num_arg, &num_local);
    add_mapping_string(m, "function", fname);
    break;
  case FRAME_CATCH:
    add_mapping_string(m, "function", "CATCH");
    num_arg = -1;
    break;
  case FRAME_FAKE:
    add_mapping_string(m, "function", "<fake>");
    num_arg = -1;
    break;
  case FRAME_FUNP:
    {
      outbuffer_t tmpbuf;
      svalue_t tmpval;
  
      tmpbuf.real_size = 0;
      tmpbuf.buffer = 0;
  
      tmpval.type = T_FUNCTION;
      tmpval.u.fp = p[0].fr.funp;

      svalue_to_string(&tmpval, &tmpbuf, 0, 0, 0);
      add_mapping_string(m, "function", tmpbuf.buffer);
      FREE_MSTR(tmpbuf.buffer);
      num_arg = p[0].fr.funp->f.functional.num_arg;
      num_local = p[0].fr.funp->f.functional.num_local;
    }
    break;
  }
  add_mapping_malloced_string(m, "program", add_slash(current_prog->filename));
  add_mapping_object(m, "object", current_object);
  get_line_number_info(&file, &line);
  add_mapping_malloced_string(m, "file", add_slash(file));
  add_mapping_pair(m, "line", line);
#ifdef ARGUMENTS_IN_TRACEBACK
  if (num_arg != -1) {
    array_t *v2;

    v2 = allocate_empty_array(num_arg);
    for (i = 0; i < num_arg; i++) {
      assign_svalue_no_free(&v2->item[i], &fp[i]);
    }
    add_mapping_array(m, "arguments", v2);
    v2->ref--;
  }
#endif
#ifdef LOCALS_IN_TRACEBACK
  if (num_local > 0 && num_arg != -1) {
    array_t *v2;

    v2 = allocate_empty_array(num_local);
    for (i = 0; i < num_local; i++) {
      assign_svalue_no_free(&v2->item[i], &fp[i + num_arg]);
    }
    add_mapping_array(m, "locals", v2);
    v2->ref--;
  }
#endif
  v->item[(csp - &control_stack[0])].type = T_MAPPING;
  v->item[(csp - &control_stack[0])].u.map = m;
  /* return a reference zero array */
  v->ref--;
  return v;
}

char * get_line_number_if_any()
{
  if (current_prog)
    return get_line_number(pc, current_prog);
  return 0;
}

#define SSCANF_ASSIGN_SVALUE_STRING(S) \
arg->type = T_STRING; \
arg->u.string = S; \
arg->subtype = STRING_MALLOC; \
arg--; \
num_arg--

#define SSCANF_ASSIGN_SVALUE_NUMBER(N) \
arg->type = T_NUMBER; \
arg->subtype = 0; \
arg->u.number = N; \
arg--; \
num_arg--

#define SSCANF_ASSIGN_SVALUE(T,U,V) \
arg->type = T; \
arg->U = V; \
arg--; \
num_arg--

/* arg points to the same place it used to */
int inter_sscanf (svalue_t * arg, svalue_t * s0, svalue_t * s1, int num_arg)
{
  const char *fmt;      /* Format description */
  const char *in_string;    /* The string to be parsed. */
  int number_of_matches;
  int skipme;     /* Encountered a '*' ? */
  int base = 10;
  int num;
  const char *match;
  char old_char;
  const char *tmp;
    
  /*
   * First get the string to be parsed.
   */
  CHECK_TYPES(s0, T_STRING, 1, F_SSCANF);
  in_string = s0->u.string;

  /*
   * Now get the format description.
   */
  CHECK_TYPES(s1, T_STRING, 2, F_SSCANF);
  fmt = s1->u.string;

  /*
   * Loop for every % or substring in the format.
   */
  for (number_of_matches = 0; num_arg >= 0; number_of_matches++) {
    while (*fmt) {
      if (*fmt == '%') {
        if (*++fmt == '%') {
          if (*in_string++ != '%') return number_of_matches;
          fmt++;
          continue;
        }
        if (!*fmt)
          error("Format string cannot end in '%%' in sscanf()\n");
        break;
      }
      if (*fmt++ != *in_string++) return number_of_matches;
    }
  
    if (!*fmt) {
      /*
       * We have reached the end of the format string.  If there are
       * any chars left in the in_string, then we put them in the
       * last variable (if any).
       */
      if (*in_string && num_arg) {
        number_of_matches++;
        SSCANF_ASSIGN_SVALUE_STRING(string_copy(in_string, "sscanf"));
      }
      break;
    }
    DEBUG_CHECK(fmt[-1] != '%', "In sscanf, should be a %% now!\n");
  
    if ((skipme = (*fmt == '*'))) fmt++;
    else if (num_arg < 1 && *fmt != '%') {
      /*
       * Hmm ... maybe we should return number_of_matches here instead
       * of an error
       */
      error("Too few arguments to sscanf()\n");
    }
  
    switch (*fmt++) {
    case 'x':
      base = 16;
      /* fallthrough */
    case 'd':
      {
        tmp = in_string;
        num = strtol((char *)in_string, (char **)&in_string, base);
        if (tmp == in_string) return number_of_matches;
        if (!skipme) {
          SSCANF_ASSIGN_SVALUE_NUMBER(num);
        }
        base = 10;
        continue;
      }
    case 'f':
      {
        float tmp_num;
    
        tmp = in_string;
        tmp_num = _strtof((char *)in_string, (char **)&in_string);
        if (tmp == in_string)return number_of_matches;
        if (!skipme) {
          SSCANF_ASSIGN_SVALUE(T_REAL, u.real, tmp_num);
        }
        continue;
      }
    case '(':
      {
        struct regexp *reg;
    
        tmp = fmt; /* 1 after the ( */
        num = 1;
        while (1) {
          switch (*tmp) {
          case '\\':
            if (*++tmp) {
              tmp++;
              continue;
            }
          case '\0':
            error("Bad regexp format: '%%%s' in sscanf format string\n", fmt);
          case '(':
            num++;
            /* FALLTHROUGH */
          default:
            tmp++;
            continue;
          case ')':
            if (!--num) break;
            tmp++;
            continue;
          }
          {
            int n = tmp - fmt;
            char *buf = (char *)DXALLOC(n + 1, TAG_TEMPORARY,
                                        "sscanf regexp");
            memcpy(buf, fmt, n);
            buf[n] = 0;
            regexp_user = EFUN_REGEXP;
            reg = regcomp((unsigned char *)buf, 0);
            FREE(buf);
            if (!reg) error(regexp_error);
            if (!regexec(reg, in_string) || (in_string != reg->startp[0])) {
              FREE(reg);
              return number_of_matches;
            }
            if (!skipme) {
              n = *reg->endp - in_string;
              buf = new_string(n, "sscanf regexp return");
              memcpy(buf, in_string, n);
              buf[n] = 0;
              SSCANF_ASSIGN_SVALUE_STRING(buf);
            }       
            in_string = *reg->endp;
            FREE((char *)reg);
            fmt = ++tmp;
            break;
          }
        }
        continue;
      }
    case 's':
      break;
    default:
      error("Bad type : '%%%c' in sscanf() format string\n", fmt[-1]);
    }
  
    /*
     * Now we have the string case.
     */

    /*
     * First case: There were no extra characters to match. Then this is
     * the last match.
     */
    if (!*fmt) {
      number_of_matches++;
      if (!skipme) {
        SSCANF_ASSIGN_SVALUE_STRING(string_copy(in_string, "sscanf"));
      }
      break;
    }
    /*
     * If the next char in the format string is a '%' then we have to do
     * some special checks. Only %d, %f, %x, %(regexp) and %% are allowed
     * after a %s
     */
    if (*fmt++ == '%') {
      int skipme2;
      
      tmp = in_string;
      if ((skipme2 = (*fmt == '*'))) fmt++;
      if (num_arg < (!skipme + !skipme2) && *fmt != '%')
        error("Too few arguments to sscanf().\n");
      
      number_of_matches++;
      
      switch (*fmt++) {
      case 's':
        error("Illegal to have 2 adjacent %%s's in format string in sscanf()\n");
      case 'x':
        do {
          while (*tmp && (*tmp != '0')) tmp++;
          if (*tmp == '0') {
            if ((tmp[1] == 'x' || tmp[1] == 'X') &&
                uisxdigit(tmp[2])) break;
            tmp += 2;
          }
        } while (*tmp);
        break;
      case 'd':
        while (*tmp && !uisdigit(*tmp)) tmp++;
        break;
      case 'f':
        while (*tmp && !uisdigit(*tmp) && 
               (*tmp != '.' || !uisdigit(tmp[1]))) tmp++;
        break;
      case '%':
        while (*tmp && (*tmp != '%')) tmp++;
        break;
      case '(':
        {
          struct regexp *reg;
        
          tmp = fmt;
          num = 1;
          while (1) {
            switch (*tmp) {
            case '\\':
              if (*++tmp) {
                tmp++;
                continue;
              }
            case '\0':
              error("Bad regexp format : '%%%s' in sscanf format string\n", fmt);
            case '(':
              num++;
              /* FALLTHROUGH */
            default:
              tmp++;
              continue;
          
            case ')':
              if (!--num) break;
              tmp++;
              continue;
            }
            {
              int n = tmp - fmt;
              char *buf = (char *)DXALLOC(n + 1, TAG_TEMPORARY,
                                          "sscanf regexp");
              memcpy(buf, fmt, n);
              buf[n] = 0;
              regexp_user = EFUN_REGEXP;
              reg = regcomp((unsigned char *)buf, 0);
              FREE(buf);
              if (!reg) error(regexp_error);
              if (!regexec(reg, in_string)) {
                if (!skipme) {
                  SSCANF_ASSIGN_SVALUE_STRING(string_copy(in_string, "sscanf"));
                }
                FREE((char *)reg);
                return number_of_matches;
              } else {
                if (!skipme) {
                  char *tmp2 = new_string(num = (*reg->startp - in_string), "inter_sscanf");
                  memcpy(tmp2, in_string, num);
                  tmp2[num] = 0;
                  match = tmp2;
                  SSCANF_ASSIGN_SVALUE_STRING(match);
                }
                in_string = *reg->endp;
                if (!skipme2) {
                  char *tmp2 = new_string(num = (*reg->endp - *reg->startp), "inter_sscanf");
                  memcpy(tmp2, *reg->startp, num);
                  tmp2[num] = 0;
                  match = tmp2;
                  SSCANF_ASSIGN_SVALUE_STRING(match);
                }
                FREE((char *)reg);
              }
              fmt = ++tmp;
              break;
            }
          }
          continue;
        }

      case 0:
        error("Format string can't end in '%%'.\n");
      default:
        error("Bad type : '%%%c' in sscanf() format string\n", fmt[-1]);
      }
      
      if (!skipme) {
        char *tmp2 = new_string(num = (tmp - in_string), "inter_sscanf");
        memcpy(tmp2, in_string, num);
        tmp2[num] = 0;
        match = tmp2;
        SSCANF_ASSIGN_SVALUE_STRING(match);
      }
      if (!*(in_string = tmp)) return number_of_matches;
      switch (fmt[-1]) {
      case 'x':
        base = 16;
      case 'd':
        {
          num = strtol((char *)in_string, (char **)&in_string, base);
          /* We already knew it would be matched - Sym */
          if (!skipme2) {
            SSCANF_ASSIGN_SVALUE_NUMBER(num);
          }
          base = 10;
          continue;
        }
      case 'f':
        {
          float tmp_num = _strtof((char *)in_string, (char **)&in_string);
          if (!skipme2) {
            SSCANF_ASSIGN_SVALUE(T_REAL, u.real, tmp_num);
          }
          continue;
        }
      case '%':
        in_string++;
        continue; /* on the big for loop */
      }
    }
    if ((tmp = strchr(fmt, '%')) != NULL) num = tmp - fmt + 1;
    else {
      tmp = fmt + (num = strlen(fmt));
      num++;
    }
  
    old_char = *--fmt;
    match = in_string;
    
    /* This loop would be even faster if it used replace_string's skiptable
       algorithm.  Maybe that algorithm should be lifted so it can be
       used in strsrch as well has here, etc? */
    while (*in_string) {
      if ((*in_string == old_char) && !strncmp(in_string, fmt, num)) {
        /*
         * Found a match !
         */
        if (!skipme) {
          char *newmatch;
        
          newmatch = new_string(skipme = (in_string - match), "inter_sscanf");
          memcpy(newmatch, match, skipme);
          newmatch[skipme] = 0;
          SSCANF_ASSIGN_SVALUE_STRING(newmatch);
        }
        in_string += num;
        fmt = tmp; /* advance fmt to next % */
        break;
      }
      in_string++;
    }
    if (fmt == tmp)    /* If match, then do continue. */
      continue;
  
    /*
     * No match was found. Then we stop here, and return the result so
     * far !
     */
    break;
  }
  return number_of_matches;
}

/* dump # of times each efun has been used */
#ifdef OPCPROF
void opcdump (const char *tfn)
{
  int i, len, limit;
  char tbuf[SMALL_STRING_SIZE];
  const char *fn;
  FILE *fp;

  if ((len = strlen(tfn)) >= (SMALL_STRING_SIZE - 7)) {
    error("Path '%s' too long.\n", tfn);
    return;
  }
  strcpy(tbuf, tfn);
  strcpy(tbuf + len, ".efun");
  fn = check_valid_path(tbuf, current_object, "opcprof", 1);
  if (!fn) {
    error("Invalid path '%s' for writing.\n", tbuf);
    return;
  }
  fp = fopen(fn, "w");
  if (!fp) {
    error("Unable to open %s.\n", fn);
    return;
  }
  limit = sizeof(opc_efun) / sizeof(opc_t);
  for (i = 0; i < limit; i++) {
    fprintf(fp, "%-30s: %10d\n", opc_efun[i].name, opc_efun[i].count);
  }
  fclose(fp);

  strcpy(tbuf, tfn);
  strcpy(tbuf + len, ".eoper");
  fn = check_valid_path(tbuf, current_object, "opcprof", 1);
  if (!fn) {
    error("Invalid path '%s' for writing.\n", tbuf);
    return;
  }
  fp = fopen(fn, "w");
  if (!fp) {
    error("Unable to open %s for writing.\n", fn);
    return;
  }
  for (i = 0; i < BASE; i++) {
    fprintf(fp, "%-30s: %10d\n",
            query_instr_name(i), opc_eoper[i]);
  }
  fclose(fp);
}
#endif

/* dump # of times each efun has been used */
#ifdef OPCPROF_2D
typedef struct {
  int op1, op2;
  int num_calls;
} sort_elem_t;

int sort_elem_cmp (sort_elem_t * se1, sort_elem_t * se2) {
  return se2->num_calls - se1->num_calls;
}

void opcdump (char * tfn)
{
  int ind, i, j, len;
  char tbuf[SMALL_STRING_SIZE], *fn;
  FILE *fp;
  sort_elem_t ops[(BASE + 1) * (BASE + 1)];

  if ((len = strlen(tfn)) >= (SMALL_STRING_SIZE - 10)) {
    error("Path '%s' too long.\n", tfn);
    return;
  }
  strcpy(tbuf, tfn);
  strcpy(tbuf + len, ".eop-2d");
  fn = check_valid_path(tbuf, current_object, "opcprof", 1);
  if (!fn) {
    error("Invalid path '%s' for writing.\n", tbuf);
    return;
  }
  fp = fopen(fn, "w");
  if (!fp) {
    error("Unable to open %s for writing.\n", fn);
    return;
  }
  for (i = 0; i <= BASE; i++) {
    for (j = 0; j <= BASE; j++) {
      ind = i * (BASE + 1) + j;
      ops[ind].num_calls = opc_eoper_2d[i][j];
      ops[ind].op1 = i;
      ops[ind].op2 = j;
    }
  }
  quickSort((char *) ops, (BASE + 1) * (BASE + 1), sizeof(sort_elem_t),
            sort_elem_cmp);
  for (i = 0; i < (BASE + 1) * (BASE + 1); i++) {
    if (ops[i].num_calls)
      fprintf(fp, "%-30s %-30s: %10d\n", query_instr_name(ops[i].op1),
              query_instr_name(ops[i].op2), ops[i].num_calls);
  }
  fclose(fp);
}
#endif

/*
 * Reset the virtual stack machine.
 */
void reset_machine (int first)
{
  csp = control_stack - 1;
  if (first)
    sp = &start_of_stack[-1];
  else {
    pop_n_elems(sp - start_of_stack + 1);
    IF_DEBUG(stack_in_use_as_temporary = 0);
  }
}

#ifdef TRACE_CODE
static char *get_arg (int a, int b)
{
  static char buff[10];
  char *from, *to;

  from = previous_pc[a];
  to = previous_pc[b];
  if (to - from < 2)
    return "";
  if (to - from == 2) {
    sprintf(buff, "%d", from[1]);
    return buff;
  }
  if (to - from == 3) {
    short arg;

    COPY_SHORT(&arg, from + 1);
    sprintf(buff, "%d", arg);
    return buff;
  }
  if (to - from == 5) {
    int arg;

    COPY_INT(&arg, from + 1);
    sprintf(buff, "%d", arg);
    return buff;
  }
  return "";
}

int last_instructions()
{
  int i;

  debug_message("Recent instruction trace:\n");
  i = last;
  do {
    if (previous_instruction[i] != 0)
      debug_message("%6x: %3d %8s %-25s (%d)\n", previous_pc[i],
                    previous_instruction[i],
                    get_arg(i, (i + 1) %
                            (sizeof previous_instruction / sizeof(int))),
                    query_instr_name(previous_instruction[i]),
                    stack_size[i] + 1);
    i = (i + 1) % (sizeof previous_instruction / sizeof(int));
  } while (i != last);
  return last;
}

#endif        /* TRACE_CODE */


#ifdef TRACE
/* Generate a debug message to the user */
void do_trace (const char * msg, const char * fname, const char * post)
{
  const char *objname;

  if (!TRACEHB)
    return;
  objname = TRACETST(TRACE_OBJNAME) ? (current_object && current_object->obname ? current_object->obname : "??") : "";
  add_vmessage(command_giver, "*** %d %*s %s %s %s%s", tracedepth, tracedepth, "", msg, objname, fname, post);
}
#endif

/*
 * When an object is destructed, all references to it must be removed
 * from the stack.
 */
void remove_object_from_stack (object_t * ob)
{
  svalue_t *svp;

  for (svp = start_of_stack; svp <= sp; svp++) {
    if (svp->type != T_OBJECT)
      continue;
    if (svp->u.ob != ob)
      continue;
    free_object(&svp->u.ob, "remove_object_from_stack");
    svp->type = T_NUMBER;
    svp->u.number = 0;
  }
}

int strpref (const char * p, const char * s)
{
  while (*p)
    if (*p++ != *s++)
      return 0;
  return 1;
}

static float _strtof (char * nptr, char ** endptr)
{
  register char *s = nptr;
  register float acc;
  register int neg, c, any, divv;

  divv = 1;
  neg = 0;
  /*
   * Skip white space and pick up leading +/- sign if any.
   */
  do {
    c = *s++;
  } while (isspace(c));
  if (c == '-') {
    neg = 1;
    c = *s++;
  } else if (c == '+')
    c = *s++;

  for (acc = 0, any = 0;; c = *s++) {
    if (isdigit(c))
      c -= '0';
    else if ((divv == 1) && (c == '.')) {
      divv = 10;
      continue;
    } else
      break;
    if (divv == 1) {
      acc *= (float) 10;
      acc += (float) c;
    } else {
      acc += (float) c / (float) divv;
      divv *= 10;
    }
    any = 1;
  }

  if (neg)
    acc = -acc;

  if (endptr != 0)
    *endptr = any ? s - 1 : (char *) nptr;

  return acc;
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_stack() {
  svalue_t *sv;

  for (sv = start_of_stack; sv <= sp; sv++) mark_svalue(sv);
}
#endif

/* Be careful.  This assumes there will be a frame pushed right after this,
   as we use econ->save_csp + 1 to restore */
int save_context (error_context_t * econ) {
  if (csp == &control_stack[CFG_MAX_CALL_DEPTH - 1]) {
    /* Attempting to push the frame will give Too deep recursion.
       fail now. */
    return 0;
  }
  econ->save_sp = sp;
  econ->save_csp = csp;
  econ->save_cgsp = cgsp;
  econ->save_context = current_error_context;

  current_error_context = econ;
  return 1;
}

void pop_context (error_context_t * econ) {
  current_error_context = econ->save_context;
}

/* can the error handler do this ? */
void restore_context (error_context_t * econ) {
  ref_t *refp;
#ifdef PACKAGE_DWLIB
  extern int _in_reference_allowed;
  _in_reference_allowed = 0;
#endif
  /* unwind the command_giver stack to the saved position */
  while (cgsp != econ->save_cgsp)
    restore_command_giver();
  DEBUG_CHECK(csp < econ->save_csp, "csp is below econ->csp before unwinding.\n");
  if (csp > econ->save_csp) {
    /* Unwind the control stack to the saved position */
#ifdef PROFILE_FUNCTIONS
    /* PROFILE_FUNCTIONS needs current_prog to be correct in 
       pop_control_stack() */
    if (csp > econ->save_csp + 1) {
      csp = econ->save_csp + 1;
      current_prog = (csp+1)->prog;
    } else
#endif
      csp = econ->save_csp + 1;
    pop_control_stack();
  }
  pop_n_elems(sp - econ->save_sp);
  refp = global_ref_list;
  while (refp) {
    if (refp->csp >= csp) {
      ref_t *ref = refp;
      refp = refp->next;
      kill_ref(ref);
    } else
      refp = refp->next;
  }
}

