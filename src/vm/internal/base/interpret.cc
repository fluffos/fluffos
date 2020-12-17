#include "base/std.h"

#include <algorithm>
#include <functional>

#include "applies_table.autogen.h"
#include "base/internal/lru_cache.h"
#include "base/internal/tracing.h"
#include "comm.h"  // add_vmessage FIXME: reverse API
#include "thirdparty/scope_guard/scope_guard.hpp"
#include "vm/internal/apply.h"
#include "vm/internal/base/machine.h"
#include "vm/internal/eval_limit.h"
#include "vm/internal/master.h"
#include "vm/internal/simulate.h"
#include "vm/internal/simul_efun.h"
#include "compiler/internal/icode.h"  // for PUSH_WHAT
#include "compiler/internal/lex.h"    // for insstr, FIXME
#include "packages/core/sprintf.h"    // FIXME
#include "packages/core/regexp.h"     // FIXME
#include "packages/ops/ops.h"         // FIXME

int call_origin = 0;
error_context_t *current_error_context = nullptr;

static const char *type_names[] = {"int",      "string", "array",  "object", "mapping",
                                   "function", "float",  "buffer", "class"};
#define TYPE_CODES_END 0x400
#define TYPE_CODES_START 0x2

#ifdef PACKAGE_UIDS
extern struct userid_t *backbone_uid;
#endif
extern int call_origin;
static int find_line(char * /*p*/, const program_t * /*progp*/, const char ** /*ret_file*/,
                     int * /*ret_line*/);
void push_indexed_lvalue(int /*code*/);

void break_point(void);
static void do_loop_cond_number(void);
static void do_loop_cond_local(void);
static void do_catch(char * /*pc*/, unsigned short /*new_pc_offset*/);
int last_instructions(void);
static const char *get_arg(int, int);
extern inline const char *access_to_name(int /*mode*/);
extern inline const char *origin_to_name(int /*origin*/);

#ifdef DEBUG
int stack_in_use_as_temporary = 0;
#endif

int inter_sscanf(svalue_t * /*arg*/, svalue_t * /*s0*/, svalue_t * /*s1*/, int /*num_arg*/);
program_t *current_prog;
short int caller_type;
int tracedepth;
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
svalue_t *fp; /* Pointer to first argument. */

svalue_t *sp;

int function_index_offset; /* Needed for inheritance */
int variable_index_offset; /* Needed for inheritance */
int st_num_arg;

// For safety, we leave some buffer in before and after the space.
static svalue_t _stack[10 + CFG_EVALUATOR_STACK_SIZE + 10];
static svalue_t *const start_of_stack = &_stack[10];
svalue_t *const end_of_stack = start_of_stack + CFG_EVALUATOR_STACK_SIZE;

/* Used to throw an error to a catch */
svalue_t catch_value = {T_NUMBER};

// For safety, we leave some buffer in before and after the space.
control_stack_t _control_stack[5 + CFG_MAX_CALL_DEPTH + 5];
control_stack_t *const control_stack = &_control_stack[5];
control_stack_t *csp; /* Points to last element pushed */

int too_deep_error = 0, max_eval_error = 0;

ref_t *global_ref_list = nullptr;

void kill_ref(ref_t *ref) {
  if (ref->sv.type == T_MAPPING && (ref->sv.u.map->count & MAP_LOCKED)) {
    ref_t *r = global_ref_list;

    /* if some other ref references this mapping, it needs to remain
       locked */
    while (r) {
      if (r->sv.u.map == ref->sv.u.map) {
        break;
      }
      r = r->next;
    }
    if (!r) {
      unlock_mapping(ref->sv.u.map);
    }
  }
  if (ref->lvalue) {
    free_svalue(&ref->sv, "kill_ref");
  }
  if (ref->next) {
    ref->next->prev = ref->prev;
  }
  if (ref->prev) {
    ref->prev->next = ref->next;
  } else {
    global_ref_list = ref->next;
    if (global_ref_list) {
      global_ref_list->prev = nullptr;
    }
  }
  if (ref->ref > 0) {
    /* still referenced */
    ref->lvalue = nullptr;
    ref->prev = ref;  // so it doesn't get set to the global list above
    ref->next = ref;
  } else {
    FREE(ref);
  }
}

ref_t *make_ref(void) {
  auto *ref = reinterpret_cast<ref_t *>(DMALLOC(sizeof(ref_t), TAG_TEMPORARY, "make_ref"));
  ref->next = global_ref_list;
  ref->prev = nullptr;
  if (ref->next) {
    ref->next->prev = ref;
  }
  global_ref_list = ref;
  ref->csp = csp;
  ref->ref = 1;
  return ref;
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

void push_object(object_t *ob) {
  STACK_INC;

  if (!ob || (ob->flags & O_DESTRUCTED) || ob->flags & O_BEING_DESTRUCTED) {
    *sp = const0u;
    return;
  }

  sp->type = T_OBJECT;
  sp->u.ob = ob;
  add_ref(ob, "push_object");
}

const char *type_name(int c) {
  int j = 0;
  int limit = TYPE_CODES_START;

  do {
    if (c & limit) {
      return type_names[j];
    }
    j++;
  } while (!((limit <<= 1) & TYPE_CODES_END));
  /* Oh crap.  Take some time and figure out what we have. */
  switch (c) {
    case T_INVALID:
      return "*invalid*";
    case T_LVALUE:
      return "*lvalue*";
    case T_REF:
      return "*ref*";
    case T_LVALUE_BYTE:
      return "*lvalue_byte*";
    case T_LVALUE_RANGE:
      return "*lvalue_range*";
    case T_LVALUE_CODEPOINT:
      return "*lvalue_codepoint*";
    case T_ERROR_HANDLER:
      return "*error_handler*";
#ifdef DEBUG
    case T_FREED:
      return "*freed*";
#endif
  }
  return "*unknown*";
}

/*
 * May current_object shadow object 'ob' ? We rely heavily on the fact that
 * function names are pointers to shared strings, which means that equality
 * can be tested simply through pointer comparison.
 */
static program_t *ffbn_recurse(program_t * /*prog*/, char * /*name*/, int * /*indexp*/,
                               int * /*runtime_index*/);

#ifndef NO_SHADOWS

static char *check_shadow_functions(program_t *shadow, program_t *victim) {
  ScopedTracer _tracer(__PRETTY_FUNCTION__);

  int i;
  int pindex, runtime_index;
  program_t *prog;
  char *fun;

  for (i = 0; i < shadow->num_functions_defined; i++) {
    prog = ffbn_recurse(victim, shadow->function_table[i].funcname, &pindex, &runtime_index);
    if (prog && (victim->function_flags[runtime_index] & DECL_NOMASK)) {
      return prog->function_table[pindex].funcname;
    }
  }

  /* Loop through all the inherits of the program also */
  for (i = 0; i < shadow->num_inherited; i++) {
    fun = check_shadow_functions(shadow->inherit[i].prog, victim);
    if (fun) {
      return fun;
    }
  }
  return nullptr;
}

int validate_shadowing(object_t *ob) {
  program_t *shadow = current_object->prog, *victim = ob->prog;
  svalue_t *ret;
  char *fun;

  if (current_object->shadowing) {
    error("shadow: Already shadowing.\n");
  }
  if (current_object->shadowed) {
    error("shadow: Can't shadow when shadowed.\n");
  }
#ifndef NO_ENVIRONMENT
  if (current_object->super) {
    error("shadow: The shadow must not reside inside another object.\n");
  }
#endif
  if (ob == master_ob) {
    error("shadow: cannot shadow the master object.\n");
  }
  if (ob->shadowing) {
    error("shadow: Can't shadow a shadow.\n");
  }

  if ((fun = check_shadow_functions(shadow, victim))) {
    error("Illegal to shadow 'nomask' function \"%s\".\n", fun);
  }

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
void push_number(LPC_INT n) {
  STACK_INC;
  sp->type = T_NUMBER;
  sp->subtype = 0;
  sp->u.number = n;
}

void push_real(LPC_FLOAT n) {
  STACK_INC;
  sp->type = T_REAL;
  sp->u.real = n;
}

/*
 * Push undefined (const0u) onto the value stack.
 */

void push_undefined() {
  STACK_INC;
  *sp = const0u;
}

static void push_undefineds(int num) {
  CHECK_STACK_OVERFLOW(num);
  while (num--) {
    *++sp = const0u;
  }
}

void copy_and_push_string(const char *p) {
  STACK_INC;
  sp->type = T_STRING;
  sp->subtype = STRING_MALLOC;
  sp->u.string = string_copy(p, "copy_and_push_string");
}

void share_and_push_string(const char *p) {
  STACK_INC;
  sp->type = T_STRING;
  sp->subtype = STRING_SHARED;
  sp->u.string = make_shared_string(p);
}

/*
 * Get address to a valid global variable.
 */
#ifdef DEBUG
static svalue_t *find_value(int num) {
  DEBUG_CHECK2(num >= current_object->prog->num_variables_total,
               "Illegal variable access %d(%d).\n", num, current_object->prog->num_variables_total);
  return &current_object->variables[num];
}
#else
#define find_value(num) (&current_object->variables[num])
#endif

void free_string_svalue(svalue_t *v) {
  const char *str = v->u.string;

  if (v->subtype & STRING_COUNTED) {
    int size = MSTR_SIZE(str);
    if (DEC_COUNTED_REF(str)) {
      SUB_STRING(size);
      NDBG(BLOCK(str));
      if (v->subtype & STRING_HASHED) {
        SUB_NEW_STRING(size, sizeof(block_t));
        deallocate_string(const_cast<char *>(str));
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

void unlink_string_svalue(svalue_t *s) {
  char *str;

  switch (s->subtype) {
    case STRING_MALLOC:
      if (MSTR_REF(s->u.string) > 1) {
        s->u.string = string_unlink(s->u.string, "unlink_string_svalue");
      }
      break;
    case STRING_SHARED: {
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

void process_efun_callback(int narg, function_to_call_t *ftc, int f) {
  int argc = st_num_arg;
  svalue_t *arg = sp - argc + 1 + narg;

  if (arg->type == T_FUNCTION) {
    ftc->f.fp = arg->u.fp;
    ftc->ob = nullptr;
    ftc->narg = argc - narg - 1;
    ftc->args = arg + 1;
  } else {
    ftc->f.str = arg->u.string;
    if (argc < narg + 2) {
      ftc->ob = current_object;
      ftc->narg = 0;
    } else {
      if ((arg + 1)->type == T_OBJECT) {
        ftc->ob = (arg + 1)->u.ob;
      } else if ((arg + 1)->type == T_STRING) {
        if (!(ftc->ob = find_object((arg + 1)->u.string)) || !object_visible(ftc->ob)) {
          bad_argument(arg + 1, T_STRING | T_OBJECT, 3, f);
        }
      } else {
        bad_argument(arg + 1, T_STRING | T_OBJECT, 3, f);
      }

      ftc->narg = argc - narg - 2;
      ftc->args = arg + 2;

      if (ftc->ob->flags & O_DESTRUCTED) {
        bad_argument(arg + 1, T_STRING | T_OBJECT, 3, f);
      }
    }
  }
}

svalue_t *call_efun_callback(function_to_call_t *ftc, int n) {
  svalue_t *v;

  if (ftc->narg) {
    push_some_svalues(ftc->args, ftc->narg);
  }
  if (ftc->ob) {
    if (ftc->ob->flags & O_DESTRUCTED) {
      error("Object destructed during efun callback.\n");
    }
    v = apply(ftc->f.str, ftc->ob, n + ftc->narg, ORIGIN_EFUN);
  } else {
    v = call_function_pointer(ftc->f.fp, n + ftc->narg);
  }
  return v;
}

svalue_t *safe_call_efun_callback(function_to_call_t *ftc, int n) {
  svalue_t *v;

  if (ftc->narg) {
    push_some_svalues(ftc->args, ftc->narg);
  }
  if (ftc->ob) {
    if (ftc->ob->flags & O_DESTRUCTED) {
      error("Object destructed during efun callback.\n");
    }
    v = safe_apply(ftc->f.str, ftc->ob, n + ftc->narg, ORIGIN_EFUN);
  } else {
    v = safe_call_function_pointer(ftc->f.fp, n + ftc->narg);
  }
  return v;
}

/*
 * Free several svalues, and free up the space used by the svalues.
 * The svalues must be sequentially located.
 */
void free_some_svalues(svalue_t *v, int num) {
  while (num--) {
    free_svalue(v + num, "free_some_svalues");
  }
  FREE(v);
}

/*
 * Prepend a slash in front of a string.
 */
char *add_slash(const char *const str) {
  char *tmp;

  if (str[0] == '<' && strcmp(str + 1, "function>") == 0) {
    return string_copy(str, "add_slash");
  }
  tmp = new_string(strlen(str) + 1, "add_slash");
  *tmp = '/';
  strcpy(tmp + 1, str);
  return tmp;
}

void push_some_svalues(svalue_t *v, int num) {
  while (num--) {
    push_svalue(v++);
  }
}

void transfer_push_some_svalues(svalue_t *v, int num) {
  CHECK_STACK_OVERFLOW(num);
  memcpy(sp + 1, v, num * sizeof(svalue_t));
  sp += num;
}

/*
 * Pop the top-most value of the stack.
 * Don't do this if it is a value that will be used afterwards, as the
 * data may be sent to FREE(), and destroyed.
 */
void pop_stack() {
  DEBUG_CHECK(sp < start_of_stack, "Stack underflow.\n");
  free_svalue(sp--, "pop_stack");
}

svalue_t global_lvalue_byte = {T_LVALUE_BYTE};

int lv_owner_type;
refed_t *lv_owner;

// LVALUE points to an character(codepoint) in string
static struct {
  int32_t index;
  svalue_t *owner;
} global_lvalue_codepoint;
static svalue_t global_lvalue_codepoint_sv = {T_LVALUE_CODEPOINT};

/*
 * Compute the address of an array element.
 */
void push_indexed_lvalue(int reverse) {
  int ind;
  svalue_t *lv;

  if (sp->type == T_LVALUE) {
    lv = sp->u.lvalue;
    if (!reverse && lv->type == T_MAPPING) {
      sp--;
      if (!(lv = find_for_insert(lv->u.map, sp, 0))) {
        mapping_too_large();
      }
      free_svalue(sp, "push_indexed_lvalue: 1");
      sp->type = T_LVALUE;
      sp->u.lvalue = lv;
#ifdef REF_RESERVED_WORD
      lv_owner_type = T_MAPPING;
      lv_owner = reinterpret_cast<refed_t *>(lv->u.map);
#endif
      return;
    }

    if (!((--sp)->type == T_NUMBER)) {
      error("Illegal type of index\n");
    }

    ind = sp->u.number;

    switch (lv->type) {
      case T_STRING: {
        size_t count;
        auto success = u8_egc_count(lv->u.string, &count);
        DEBUG_CHECK(!success, "Bad UTF-8 String: push_indexed_lvalue");

        if (reverse) {
          ind = count - ind;
        }
        if (ind >= count || ind < 0) {
          error("Index out of bounds in string index lvalue.\n");
        }
        UChar32 c = u8_egc_index_as_single_codepoint(lv->u.string, ind);
        if (c < 0) {
          error("Indexed character is multi-codepoint.\n");
        }
        unlink_string_svalue(lv);
        sp->type = T_LVALUE;
        sp->u.lvalue = &global_lvalue_codepoint_sv;
        global_lvalue_codepoint.index = ind;
        global_lvalue_codepoint.owner = lv;
#ifdef REF_RESERVED_WORD
        lv_owner_type = T_STRING;
        lv_owner = (refed_t *)lv->u.string;
#endif
        break;
      }

      case T_BUFFER: {
        if (reverse) {
          ind = lv->u.buf->size - ind;
        }
        if (ind >= lv->u.buf->size || ind < 0) {
          error("Buffer index out of bounds.\n");
        }
        sp->type = T_LVALUE;
        sp->u.lvalue = &global_lvalue_byte;
        global_lvalue_byte.subtype = 1;
        global_lvalue_byte.u.lvalue_byte = &lv->u.buf->item[ind];
#ifdef REF_RESERVED_WORD
        lv_owner_type = T_BUFFER;
        lv_owner = reinterpret_cast<refed_t *>(lv->u.buf);
#endif
        break;
      }

      case T_ARRAY: {
        if (reverse) {
          ind = lv->u.arr->size - ind;
        }
        if (ind >= lv->u.arr->size || ind < 0) {
          error("Array index out of bounds\n");
        }
        sp->type = T_LVALUE;
        sp->u.lvalue = lv->u.arr->item + ind;
#ifdef REF_RESERVED_WORD
        lv_owner_type = T_ARRAY;
        lv_owner = reinterpret_cast<refed_t *>(lv->u.arr);
#endif
        break;
      }

      default:
        if (lv->type == T_NUMBER && !lv->u.number) {
          error("Value being indexed is zero.\n");
        }
        error("Cannot index value of type '%s'.\n", type_name(lv->type));
    }
  } else {
    /* It is now coming from (x <assign_type> y)[index]... = rhs */
    /* Where x is a _valid_ lvalue */
    /* Hence the reference to sp is at least 2 :) */

    if (!reverse && (sp->type == T_MAPPING)) {
      if (!(lv = find_for_insert(sp->u.map, sp - 1, 0))) {
        mapping_too_large();
      }
      sp->u.map->ref--;
#ifdef REF_RESERVED_WORD
      lv_owner_type = T_MAPPING;
      lv_owner = reinterpret_cast<refed_t *>(sp->u.map);
#endif
      free_svalue(--sp, "push_indexed_lvalue: 2");
      sp->type = T_LVALUE;
      sp->u.lvalue = lv;
      return;
    }

    if (!((sp - 1)->type == T_NUMBER)) {
      error("Illegal type of index\n");
    }

    ind = (sp - 1)->u.number;

    switch (sp->type) {
      case T_STRING: {
        error("Illegal to make char lvalue from assigned string\n");
        break;
      }

      case T_BUFFER: {
        if (reverse) {
          ind = sp->u.buf->size - ind;
        }
        if (ind >= sp->u.buf->size || ind < 0) {
          error("Buffer index out of bounds.\n");
        }
        sp->u.buf->ref--;
#ifdef REF_RESERVED_WORD
        lv_owner_type = T_BUFFER;
        lv_owner = reinterpret_cast<refed_t *>(sp->u.buf);
#endif
        (--sp)->type = T_LVALUE;
        sp->u.lvalue = &global_lvalue_byte;
        global_lvalue_byte.subtype = 1;
        global_lvalue_byte.u.lvalue_byte = (sp + 1)->u.buf->item + ind;
        break;
      }

      case T_ARRAY: {
        if (reverse) {
          ind = sp->u.arr->size - ind;
        }
        if (ind >= sp->u.arr->size || ind < 0) {
          error("Array index out of bounds.\n");
        }
        sp->u.arr->ref--;
#ifdef REF_RESERVED_WORD
        lv_owner_type = T_ARRAY;
        lv_owner = reinterpret_cast<refed_t *>(sp->u.arr);
#endif
        (--sp)->type = T_LVALUE;
        sp->u.lvalue = (sp + 1)->u.arr->item + ind;
        break;
      }

      default:
        if (sp->type == T_NUMBER && !sp->u.number) {
          error("Value being indexed is zero.\n");
        }
        error("Cannot index value of type '%s'.\n", type_name(sp->type));
    }
  }
}

static struct lvalue_range {
  size_t ind1, ind2, size;
  svalue_t *owner;
} global_lvalue_range;

static svalue_t global_lvalue_range_sv = {T_LVALUE_RANGE};

static void push_lvalue_range(int code) {
  int32_t ind1, ind2;
  size_t size = 0, u8len = 0;
  svalue_t *lv;

  {
    switch ((lv = global_lvalue_range.owner = sp->u.lvalue)->type) {
      case T_ARRAY:
        size = lv->u.arr->size;
        break;
      case T_STRING: {
        size = SVALUE_STRLEN(lv);
        auto success = u8_egc_count(lv->u.string, &u8len);
        if (!success) {
          error("Invalid UTF-8 String: push_lvalue_range");
        }
        unlink_string_svalue(lv);
        break;
      }
      case T_BUFFER:
        size = lv->u.buf->size;
        break;
      default:
        error("Range lvalue on illegal type\n");
        return;
    }
  }

  if (!((--sp)->type == T_NUMBER)) {
    error("Illegal 2nd index type to range lvalue\n");
  }

  if (lv->type == T_STRING) {
    ind2 = sp->u.number;
    ind2 = (code & 0x01) ? (u8len - ind2) : ind2;
    ind2 = ind2 + 1;
    if (ind2 < 0 || ind2 > u8len) {
      error(
          "The 2nd index to range lvalue must be >= -1 and < sizeof(indexed "
          "value)\n");
    }
    ind2 = u8_egc_index_to_offset(lv->u.string, ind2);
    if (ind2 < 0) {
      error("push_lvalue_range: invalid ind2");
    }
  } else {
    ind2 = (code & 0x01) ? (size - sp->u.number) : sp->u.number;
    ind2 = ind2 + 1;
    if (ind2 <= 0 || (ind2 > size)) {
      error(
          "The 2nd index to range lvalue must be >= -1 and < sizeof(indexed "
          "value)\n");
    }
  }

  if (!((--sp)->type == T_NUMBER)) {
    error("Illegal 1st index type to range lvalue\n");
  }

  if (lv->type == T_STRING) {
    ind1 = sp->u.number;
    ind1 = (code & 0x10) ? (u8len - ind1) : ind1;
    if (ind1 < 0 || ind1 >= u8len) {
      error(
          "The 1st index to range lvalue must be >= 0 and < sizeof(indexed "
          "value)\n");
    }
    ind1 = u8_egc_index_to_offset(lv->u.string, ind1);
    if (ind1 < 0) {
      error("push_lvalue_range: invalid ind1");
    }
  } else {
    ind1 = (code & 0x10) ? (size - sp->u.number) : sp->u.number;
    if (ind1 < 0 || ind1 > size) {
      error(
          "The 1st index to range lvalue must be >= 0 and <= sizeof(indexed "
          "value)\n");
    }
  }

  global_lvalue_range.ind1 = ind1;
  global_lvalue_range.ind2 = ind2;
  global_lvalue_range.size = size;
  sp->type = T_LVALUE;
  sp->u.lvalue = &global_lvalue_range_sv;
}

void copy_lvalue_range(svalue_t *from) {
  int ind1, ind2, size, fsize;
  svalue_t *owner;

  ind1 = global_lvalue_range.ind1;
  ind2 = global_lvalue_range.ind2;
  size = global_lvalue_range.size;
  owner = global_lvalue_range.owner;

  switch (owner->type) {
    case T_ARRAY: {
      array_t *fv, *dv;
      svalue_t *fptr, *dptr;
      if (from->type != T_ARRAY) {
        error("Illegal rhs to array range lvalue\n");
      }

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
          while (fsize--) {
            assign_svalue(dptr++, fptr++);
          }
          fv->ref--;
        }
      } else {
        array_t *old_dv = owner->u.arr;
        svalue_t *old_dptr = old_dv->item;

        /* Need to reallocate the array */
        dv = allocate_empty_array(size - ind2 + ind1 + fsize);
        dptr = dv->item;

        /* ind1 can range from 0 to sizeof(old_dv) */
        while (ind1--) {
          assign_svalue_no_free(dptr++, old_dptr++);
        }

        if (fv->ref == 1) {
          while (fsize--) {
            *dptr++ = *fptr++;
          }
          free_empty_array(fv);
        } else {
          while (fsize--) {
            assign_svalue_no_free(dptr++, fptr++);
          }
          fv->ref--;
        }

        /* ind2 can range from 0 to sizeof(old_dv) */
        old_dptr = old_dv->item + ind2;
        size -= ind2;

        while (size--) {
          assign_svalue_no_free(dptr++, old_dptr++);
        }
        free_array(old_dv);

        owner->u.arr = dv;
      }
      break;
    }

    case T_STRING: {
      if (from->type != T_STRING) {
        error("Illegal rhs to string range lvalue.\n");
      }

      if ((fsize = SVALUE_STRLEN(from)) == ind2 - ind1) {
        /* since fsize >= 0, ind2 - ind1 <= strlen(orig string) */
        /* because both of them can only range from 0 to len */

        strncpy((const_cast<char *>(owner->u.string)) + ind1, from->u.string, fsize);
      } else {
        char *tmp, *dstr = const_cast<char *>(owner->u.string);

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

    case T_BUFFER: {
      if (from->type != T_BUFFER) {
        error("Illegal rhs to buffer range lvalue.\n");
      }

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

        if ((size -= ind2) >= 1) {
          memcpy(new_item, old_item + ind2, size);
        }
        free_buffer(owner->u.buf);
        owner->u.buf = b;
      }
      free_buffer(from->u.buf);
      break;
    }
  }
}

template <typename F>
void assign_lvalue_codepoint(F &&func) {
  {
    UChar32 c = u8_egc_index_as_single_codepoint(global_lvalue_codepoint.owner->u.string,
                                                 global_lvalue_codepoint.index);
    if (c < 0) {
      error("Invalid string index, multi-codepoint character.\n");
    }
    auto old_len = U8_LENGTH(c);
    DEBUG_CHECK(old_len == 0, "Invalid UTF-8 Codepoint: assign_lvalue_codepoint");

    auto newc = func(c);
    if (newc == 0) {
      error("Strings cannot contain 0 byte.\n");
    }
    c = newc;
    auto new_len = U8_LENGTH(c);
    if (!new_len) {
      error("Strings cannot contain invalid utf8 codepoint.\n");
    }
    auto res = new_string(SVALUE_STRLEN(global_lvalue_codepoint.owner) - old_len + new_len,
                          "assign_lvalue_codepoint");
    u8_copy_and_replace_codepoint_at(global_lvalue_codepoint.owner->u.string, res,
                                     global_lvalue_codepoint.index, c);

    free_string_svalue(global_lvalue_codepoint.owner);

    global_lvalue_codepoint.owner->u.string = res;
    global_lvalue_codepoint.owner->subtype = STRING_MALLOC;
  }
}

void assign_lvalue_range(svalue_t *from) {
  int ind1, ind2, size, fsize;
  svalue_t *owner;

  ind1 = global_lvalue_range.ind1;
  ind2 = global_lvalue_range.ind2;
  size = global_lvalue_range.size;
  owner = global_lvalue_range.owner;

  switch (owner->type) {
    case T_ARRAY: {
      array_t *fv, *dv;
      svalue_t *fptr, *dptr;
      if (from->type != T_ARRAY) {
        error("Illegal rhs to array range lvalue\n");
      }

      fv = from->u.arr;
      fptr = fv->item;

      if ((fsize = fv->size) == ind2 - ind1) {
        dptr = (owner->u.arr)->item + ind1;
        while (fsize--) {
          assign_svalue(dptr++, fptr++);
        }
      } else {
        array_t *old_dv = owner->u.arr;
        svalue_t *old_dptr = old_dv->item;

        /* Need to reallocate the array */
        dv = allocate_empty_array(size - ind2 + ind1 + fsize);
        dptr = dv->item;

        /* ind1 can range from 0 to sizeof(old_dv) */
        while (ind1--) {
          assign_svalue_no_free(dptr++, old_dptr++);
        }

        while (fsize--) {
          assign_svalue_no_free(dptr++, fptr++);
        }

        /* ind2 can range from 0 to sizeof(old_dv) */
        old_dptr = old_dv->item + ind2;
        size -= ind2;

        while (size--) {
          assign_svalue_no_free(dptr++, old_dptr++);
        }
        free_array(old_dv);

        owner->u.arr = dv;
      }
      break;
    }

    case T_STRING: {
      if (from->type != T_STRING) {
        error("Illegal rhs to string range lvalue.\n");
      }

      if ((fsize = SVALUE_STRLEN(from)) == ind2 - ind1) {
        /* since fsize >= 0, ind2 - ind1 <= strlen(orig string) */
        /* because both of them can only range from 0 to len */

        strncpy((const_cast<char *>(owner->u.string)) + ind1, from->u.string, fsize);
      } else {
        char *tmp;
        const char *dstr = const_cast<char *>(owner->u.string);

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

    case T_BUFFER: {
      if (from->type != T_BUFFER) {
        error("Illegal rhs to buffer range lvalue.\n");
      }

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

        if ((size -= ind2) >= 1) {
          memcpy(new_item, old_item + ind2, size);
        }
        free_buffer(owner->u.buf);
        owner->u.buf = b;
      }
      break;
    }
  }
}

/*
 * Deallocate 'n' values from the stack.
 */
void pop_n_elems(int n) {
  DEBUG_CHECK1(n < 0, "pop_n_elems: %d elements.\n", n);
  while (n--) {
    pop_stack();
  }
}

/*
 * Deallocate 2 values from the stack.
 */
void pop_2_elems() {
  free_svalue(sp--, "pop_2_elems");
  DEBUG_CHECK(sp < start_of_stack, "Stack underflow.\n");
  free_svalue(sp--, "pop_2_elems");
}

/*
 * Deallocate 3 values from the stack.
 */
void pop_3_elems() {
  free_svalue(sp--, "pop_3_elems");
  free_svalue(sp--, "pop_3_elems");
  DEBUG_CHECK(sp < start_of_stack, "Stack underflow.\n");
  free_svalue(sp--, "pop_3_elems");
}

[[noreturn]] void bad_arg(int arg, int instr) {
  error("Bad Argument %d to %s()\n", arg, query_instr_name(instr));
}

[[noreturn]] void bad_argument(svalue_t *val, int type, int arg, int instr) {
  outbuffer_t outbuf;
  int flag = 0;
  int j = TYPE_CODES_START;
  int k = 0;

  outbuf_zero(&outbuf);
  outbuf_addv(&outbuf, "Bad argument %d to %s%s\nExpected: ", arg, query_instr_name(instr),
              (instr < EFUN_BASE ? "" : "()"));

  do {
    if (type & j) {
      if (flag) {
        outbuf_add(&outbuf, " or ");
      } else {
        flag = 1;
      }
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

void push_control_stack(int frkind) {
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
  csp->defers = nullptr;
  csp->trace_id.reset();
}

/*
 * Pop the control stack one element, and restore registers.
 * extern_call must not be modified here, as it is used imediately after pop.
 */

extern int playerchanged;

void pop_control_stack() {
  DEBUG_CHECK(csp == (control_stack - 1), "Popped out of the control stack\n");
#ifdef PROFILE_FUNCTIONS
  if ((csp->framekind & FRAME_MASK) == FRAME_FUNCTION) {
    long secs, usecs, dsecs;
    function_t *cfp = &current_prog->function_table[csp->fr.table_index];
    int stof = 0;

    get_cpu_times((unsigned long *)&secs, (unsigned long *)&usecs);
    dsecs = (((secs - csp->entry_secs) * 1000000) + (usecs - csp->entry_usecs));
    cfp->self += dsecs;

    while ((csp - stof) != control_stack) {
      if (((csp - stof - 1)->framekind & FRAME_MASK) == FRAME_FUNCTION) {
        function_t *parent =
            &((csp - stof)->prog->function_table[(csp - stof - 1)->fr.table_index]);
        if (parent != cfp) {  // if it's recursion it's not really a child
          parent->children += dsecs;
        }
        break;
      }
      stof++;
    }
  }
#endif
  struct defer_list *stuff = csp->defers;
  csp->defers = nullptr;
  while (stuff) {
    function_to_call_t ftc = {};
    ftc.f.fp = stuff->func.u.fp;
    int s = outoftime;
    if (outoftime) {
      set_eval(max_eval_cost);
    }
    save_command_giver(stuff->tp.u.ob);
    playerchanged = 0;
    safe_call_efun_callback(&ftc, 0);
    object_t *cgo = command_giver;
    restore_command_giver();
    if (playerchanged) {
      set_command_giver(cgo);
    }
    outoftime = s;
    free_svalue(&(stuff->func), "pop_stack");
    free_svalue(&(stuff->tp), "pop_stack");

    struct defer_list *old = stuff;
    stuff = stuff->next;
    FREE(old);
  }
  current_object = csp->ob;
  current_prog = csp->prog;
  previous_ob = csp->prev_ob;
  caller_type = csp->caller_type;
  pc = csp->pc;
  fp = csp->fp;
  function_index_offset = csp->function_index_offset;
  variable_index_offset = csp->variable_index_offset;
  if (Tracer::enabled()) {
    if (csp->trace_id && !csp->trace_id->empty()) {
      Tracer::end(*csp->trace_id, EventCategory::LPC_FUNCTION);
      csp->trace_id.reset();
    }
  }
  csp--;
}

/*
 * Push a pointer to a array on the stack. Note that the reference count
 * is incremented. Newly created arrays normally have a reference count
 * initialized to 1.
 */
void push_array(array_t *v) {
  STACK_INC;
  v->ref++;
  sp->type = T_ARRAY;
  sp->u.arr = v;
}

void push_refed_array(array_t *v) {
  STACK_INC;
  sp->type = T_ARRAY;
  sp->u.arr = v;
}

void push_buffer(buffer_t *b) {
  STACK_INC;
  b->ref++;
  sp->type = T_BUFFER;
  sp->u.buf = b;
}

void push_refed_buffer(buffer_t *b) {
  STACK_INC;
  sp->type = T_BUFFER;
  sp->u.buf = b;
}

/*
 * Push a mapping on the stack.  See push_array(), above.
 */
void push_mapping(mapping_t *m) {
  STACK_INC;
  m->ref++;
  sp->type = T_MAPPING;
  sp->u.map = m;
}

void push_refed_mapping(mapping_t *m) {
  STACK_INC;
  sp->type = T_MAPPING;
  sp->u.map = m;
}

/*
 * Push a class on the stack.  See push_array(), above.
 */
void push_class(array_t *v) {
  STACK_INC;
  v->ref++;
  sp->type = T_CLASS;
  sp->u.arr = v;
}

void push_refed_class(array_t *v) {
  STACK_INC;
  sp->type = T_CLASS;
  sp->u.arr = v;
}

/*
 * Push a string on the stack that is already malloced.
 */
void push_malloced_string(const char *p) {
  STACK_INC;
  sp->type = T_STRING;
  sp->u.string = p;
  sp->subtype = STRING_MALLOC;
}

/*
 * Pushes a known shared string.  Note that this references, while
 * push_malloced_string doesn't.
 */
void push_shared_string(const char *p) {
  STACK_INC;
  sp->type = T_STRING;
  sp->u.string = p;
  sp->subtype = STRING_SHARED;
  ref_string(p);
}

/*
 * Push a string on the stack that is already constant.
 */

void push_constant_string(const char *p) {
  STACK_INC;
  sp->type = T_STRING;
  sp->subtype = STRING_CONSTANT;
  sp->u.string = p;
}

/*
 * Argument is the function to execute. If it is defined by inheritance,
 * then search for the real definition, and return it.
 * There is a number of arguments on the stack. Normalize them and initialize
 * local variables, so that the called function is pleased.
 */
void setup_variables(int actual, int local, int num_arg) {
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

void setup_varargs_variables(int actual, int local, int num_arg) {
  array_t *arr;
  if (actual >= num_arg) {
    int n = actual - num_arg + 1;
    /* Aggregate excessive arguments */
    arr = allocate_empty_array(n);
    while (n--) {
      arr->item[n] = *sp--;
    }
  } else {
    /* Correct number of arguments and local variables */
    push_undefineds(num_arg - 1 - actual);
    arr = &the_null_array;
  }
  push_refed_array(arr);
  push_undefineds(local);
  fp = sp - (csp->num_local_variables = local + num_arg) + 1;
}

function_t *setup_new_frame(int findex) {
  function_t *func_entry;
  int low, high, mid;
  int flags;

  function_index_offset = variable_index_offset = 0;

  /* Walk up the inheritance tree to the real definition */
  if (current_prog->function_flags[findex] & FUNC_ALIAS) {
    findex = current_prog->function_flags[findex] & ~FUNC_ALIAS;
  }

  while (current_prog->function_flags[findex] & FUNC_INHERITED) {
    low = 0;
    high = current_prog->num_inherited - 1;

    while (high > low) {
      mid = (low + high + 1) >> 1;
      if (current_prog->inherit[mid].function_index_offset > findex) {
        high = mid - 1;
      } else {
        low = mid;
      }
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
    setup_varargs_variables(csp->num_local_variables, func_entry->num_local, func_entry->num_arg);
  } else {
    setup_variables(csp->num_local_variables, func_entry->num_local, func_entry->num_arg);
  }
  return &current_prog->function_table[findex];
}

function_t *setup_inherited_frame(int findex) {
  function_t *func_entry;
  int low, high, mid;
  int flags;

  /* Walk up the inheritance tree to the real definition */
  if (current_prog->function_flags[findex] & FUNC_ALIAS) {
    findex = current_prog->function_flags[findex] & ~FUNC_ALIAS;
  }

  while (current_prog->function_flags[findex] & FUNC_INHERITED) {
    low = 0;
    high = current_prog->num_inherited - 1;

    while (high > low) {
      mid = (low + high + 1) >> 1;
      if (current_prog->inherit[mid].function_index_offset > findex) {
        high = mid - 1;
      } else {
        low = mid;
      }
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
  /* Remove excessive arguments */
  if (flags & FUNC_TRUE_VARARGS) {
    setup_varargs_variables(csp->num_local_variables, func_entry->num_local, func_entry->num_arg);
  } else {
    setup_variables(csp->num_local_variables, func_entry->num_local, func_entry->num_arg);
  }
  return &current_prog->function_table[findex];
}

#ifdef DEBUG
/* This function is called at the end of every complete LPC statement, so
 * it is a good place to insert debugging code to find out where during
 * LPC code certain assertions fail, etc
 */
void break_point() {
  /* The current implementation of foreach leaves some stuff lying on the
     stack */
  if (!stack_in_use_as_temporary && sp - fp - csp->num_local_variables + 1 != 0) {
    fatal("Bad stack pointer.\n");
  }
}
#endif

program_t fake_prog = {"<driver>"};
unsigned char fake_program = F_RETURN;

/*
 * Very similar to push_control_stack() [which see].  The purpose of this is
 * to insert an frame containing the object which defined a function pointer
 * in cases where it would otherwise not be on the call stack.  This
 * preserves the idea that function pointers calls happen 'through' the
 * object that define the function pointer.
 * These frames are the ones that show up as <function> in error traces.
 */
void setup_fake_frame(funptr_t *fun) {
  if (csp == &control_stack[CFG_MAX_CALL_DEPTH - 1]) {
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
  csp->function_index_offset = function_index_offset;
  csp->variable_index_offset = variable_index_offset;
  csp->num_local_variables = 0;

  pc = reinterpret_cast<char *>(&fake_program);
  caller_type = ORIGIN_FUNCTION_POINTER;
  current_prog = &fake_prog;
  previous_ob = current_object;
  current_object = fun->hdr.owner;
}

/* Remove a fake frame added by setup_fake_frame().  Basically just a
 * specialized version of pop_control_stack().
 */
void remove_fake_frame() {
  DEBUG_CHECK(csp == (control_stack - 1), "Popped out of the control stack\n");
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
void check_for_destr(array_t *v) {
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

static void do_loop_cond_local() {
  svalue_t *s1, *s2;
  int i;

  s1 = fp + EXTRACT_UCHAR(pc++); /* a from (a < b) */
  s2 = fp + EXTRACT_UCHAR(pc++);
  switch (s1->type | s2->type) {
    case T_NUMBER:
      i = s1->u.number < s2->u.number;
      break;
    case T_REAL:
      i = s1->u.real < s2->u.real;
      break;
    case T_STRING:
      i = (strcmp(s1->u.string, s2->u.string) < 0);
      break;
    case T_NUMBER | T_REAL:
      if (s1->type == T_NUMBER) {
        i = s1->u.number < s2->u.real;
      } else {
        i = s1->u.real < s2->u.number;
      }
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
      switch (s1->type) {
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
  } else {
    pc += 2;
  }
}

static void do_loop_cond_number() {
  svalue_t *s1;
  LPC_INT i;

  s1 = fp + EXTRACT_UCHAR(pc++); /* a from (a < b) */
  LOAD_INT(i, pc);
  if (s1->type == T_NUMBER) {
    if (s1->u.number < i) {
      unsigned short offset;

      COPY_SHORT(&offset, pc);
      pc -= offset;
    } else {
      pc += 2;
    }
  } else if (s1->type == T_REAL) {
    if (s1->u.real < i) {
      unsigned short offset;

      COPY_SHORT(&offset, pc);
      pc -= offset;
    } else {
      pc += 2;
    }
  } else {
    error("Right side of < is a number, left side is not.\n");
  }
}

static void show_lpc_line(char *f, int l) {
  static FILE *fp = nullptr;
  static char *fn = nullptr;
  static int lastline, offset;
  static char buf[32768], *p;
  static int n;
  int dir;
  char *q;

  if (fn == f && l == lastline) {
    return;
  }
  printf("LPC: %s:%i\n", f, l);
  if (!(debug_level & DBG_LPC_line)) {
    fn = f;
    lastline = l;
    return;
  }

  if (fn != f) {
    if (fp) {
      fclose(fp);
    }
    fp = fopen(f, "r");
    if (!fp) {
      goto bail_hard;
    }
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
        if (offset == 0) {
          goto bail_hard;
        }
        n = 32767;
        if (n > offset) {
          n = offset;
        }
      } else {
        n = 32767;
      }
      offset += dir * n;
      if (fseek(fp, offset, SEEK_SET) == -1) {
        goto bail_hard;
      }
      n = fread(buf, 1, n, fp);
      if (n <= 0) {
        goto bail_hard;
      }
      buf[n] = 0;
      p = (dir == 1 ? &buf[n - 1] : buf);
    } else {
      p += dir;
      lastline += dir;
    }
  }
  if (dir == -1) {
    while (*p != '\n') {
      p--;
      if (p < buf) {
        if (offset == 0) {
          p++;
          break;
        }
        n = 32767;
        if (n > offset) {
          n = offset;
        }
        offset -= n;
        if (fseek(fp, offset, SEEK_SET) == -1) {
          goto bail_hard;
        }
        n = fread(buf, 1, 32767, fp);
        if (n == -1) {
          goto bail_hard;
        }
        buf[n] = 0;
        p = &buf[n - 1];
      }
    }
  }
  q = p;
  while (true) {
    while (*q) {
      putchar(*q);
      if (*q++ == '\n') {
        return;
      }
    }
    offset += 32767;
    if (fseek(fp, offset, SEEK_SET) == -1) {
      goto bail_hard;
    }
    n = fread(buf, 1, 32767, fp);
    if (n == -1) {
      goto bail_hard;
    }
    buf[n] = 0;
    p = buf;
  }
  return;

bail_hard:
  fn = nullptr;
  return;
}

namespace {

std::shared_ptr<std::string> get_trace_id(control_stack_t *csp) {
  static lru_cache<char *, std::shared_ptr<std::string>> trace_id_cache(4096);
  auto result = trace_id_cache.get(pc);
  if (!result) {
    auto _current_line = get_line_number_if_any();
    std::string trace_id((csp->framekind & FRAME_MASK) == FRAME_FUNCTION
                             ? current_prog->function_table[csp->fr.table_index].funcname
                             : "");
    trace_id += " at ";
    trace_id += _current_line;
    result = std::make_shared<std::string>(trace_id);
    trace_id_cache.insert(pc, result.value());
  }
  return result.value();
}

json get_trace_args(svalue_t *sp, int num_args) {
  json args = json::array();
  for (int i = num_args; i > 0; i--) {
    args.push_back(svalue_to_json_summary(sp - i + 1));
  }
  return args;
}

json get_trace_context(control_stack_t *csp, svalue_t *sp) {
  json context = json::object();
  if (CONFIG_INT(__RC_TRACE_CONTEXT__)) {
    if ((csp->framekind & FRAME_MASK) == FRAME_FUNCTION) {
      auto num_args = current_prog->function_table[csp->fr.table_index].num_arg;
      context["args"] = get_trace_args(sp, num_args);
    }
    context["previous_object"] = csp->prev_ob ? csp->prev_ob->obname : "null";
    context["current_object"] = csp->ob ? csp->ob->obname : "null";
  }
  return context;
}

}  // namespace

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
static int previous_instruction[60];
static int stack_size[60];
static char *previous_pc[60];
static int last;

void eval_instruction(char *p) {
  ScopedTracer _tracer(__PRETTY_FUNCTION__);

#ifdef DEBUG
  int num_arg;
#endif
  LPC_INT i, n;
  LPC_FLOAT real;
  svalue_t *lval;
  int instruction;
  int real_instruction;
  unsigned short offset;

#ifdef DEBUG
  svalue_t *expected_stack;
#endif

  /* Next F_RETURN at this level will return out of eval_instruction() */
  csp->framekind |= FRAME_EXTERNAL;
  pc = p;

  json trace_context = {};
  if (Tracer::enabled()) {
    if (!csp->trace_id) {
      csp->trace_id = ::get_trace_id(csp);
    }
    trace_context = ::get_trace_context(csp, sp);
    Tracer::begin(*csp->trace_id, EventCategory::LPC_FUNCTION, std::move(trace_context));
  }

  while (true) {
    if (debug_level & DBG_LPC) {
      char *f;
      int l;
      /* this could be much more efficient ... */
      get_line_number_info((const char **)&f, &l);
      show_lpc_line(f, l);
    }
    instruction = EXTRACT_UCHAR(pc++);
    if (CONFIG_INT(__RC_TRACE_CODE__)) {
      real_instruction = instruction;
      /* real EFUN is stored as an short after F_EFUN0 - F_EFUNV instructions */
      if (instruction >= F_EFUN0 && instruction <= F_EFUNV) {
        COPY_SHORT(&real_instruction, pc);
        if (real_instruction < EFUN_BASE || real_instruction > NUM_OPCODES) {
          fatal("Error in icode.");
        }
      }
      previous_instruction[last] = real_instruction;
      previous_pc[last] = pc - 1;
      stack_size[last] = sp - fp - csp->num_local_variables;
      last = (last + 1) % (sizeof previous_instruction / sizeof(int));
    }

    if (outoftime) {
      debug_message("Eval interrupted: object %s cost limit reached, limit: %ld usec.\n",
                    current_object->obname, max_eval_cost);
      set_eval(max_eval_cost);
      max_eval_error = 1;
      error("Too long evaluation. Execution aborted.\n");
    }
    /*
     * Execute current instruction. Note that all functions callable from
     * LPC must return a value. This does not apply to control
     * instructions, like F_JUMP.
     */
    if (CONFIG_INT(__RC_TRACE_INSTR__)) {
      ScopedTracer _tracer_ins(instrs[instruction].name ? instrs[instruction].name : "unknown");
    }

    switch (instruction) {
      case F_PUSH: /* Push a number of things onto the stack */
        n = EXTRACT_UCHAR(pc++);
        while (n--) {
          i = EXTRACT_UCHAR(pc++);
          switch (i & PUSH_WHAT) {
            case PUSH_STRING:
              DEBUG_CHECK1((i & PUSH_MASK) >= current_prog->num_strings,
                           "string %d out of range in F_STRING!\n", i & PUSH_MASK);
              push_shared_string(current_prog->strings[i & PUSH_MASK]);
              break;
            case PUSH_LOCAL:
              lval = fp + (i & PUSH_MASK);
              DEBUG_CHECK((fp - lval) >= csp->num_local_variables,
                          "Tried to push non-existent local\n");
              if ((lval->type == T_OBJECT) && (lval->u.ob->flags & O_DESTRUCTED)) {
                assign_svalue(lval, &const0u);
              }
              push_svalue(lval);
              break;
            case PUSH_GLOBAL:
              lval = find_value(((i & PUSH_MASK) + variable_index_offset));
              if ((lval->type == T_OBJECT) && (lval->u.ob->flags & O_DESTRUCTED)) {
                assign_svalue(lval, &const0u);
              }
              push_svalue(lval);
              break;
            case PUSH_NUMBER:
              push_number(i & PUSH_MASK);
              break;
          }
        }
        break;
      case F_INC:
        DEBUG_CHECK(sp->type != T_LVALUE, "non-lvalue argument to ++\n");
        lval = (sp--)->u.lvalue;
        switch (lval->type) {
          case T_NUMBER:
            lval->u.number++;
            break;
          case T_REAL:
            lval->u.real++;
            break;
          case T_LVALUE_BYTE:
            ++*global_lvalue_byte.u.lvalue_byte;
            break;
          case T_LVALUE_CODEPOINT: {
            assign_lvalue_codepoint([](UChar32 c) { return c + 1; });
            break;
          }
          default:
            error("++ of non-numeric argument\n");
        }
        break;
      case F_WHILE_DEC: {
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
      } break;
      case F_LOCAL_LVALUE:
        STACK_INC;
        sp->type = T_LVALUE;
        sp->u.lvalue = fp + EXTRACT_UCHAR(pc++);
        break;
#ifdef REF_RESERVED_WORD
      case F_MAKE_REF: {
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
            ref->sv.u.string = reinterpret_cast<char *>(lv_owner);
            INC_COUNTED_REF(lv_owner);
            ADD_STRING(MSTR_SIZE(lv_owner));
            NDBG(BLOCK(lv_owner));
          } else {
            ref->sv.u.refed = lv_owner;
            lv_owner->ref++;
            if (lv_owner_type == T_MAPPING) {
              (reinterpret_cast<mapping_t *>(lv_owner))->count |= MAP_LOCKED;
            }
          }
        } else {
          ref->sv.type = T_NUMBER;
        }
        sp->type = T_REF;
        sp->u.ref = ref;
        break;
      }
      case F_KILL_REFS: {
        int num = EXTRACT_UCHAR(pc++);
        while (num--) {
          kill_ref(global_ref_list);
        }
        break;
      }
      case F_REF: {
        svalue_t *s = fp + EXTRACT_UCHAR(pc++);
        svalue_t *reflval = nullptr;

        {
          reflval = s->u.ref->lvalue;
          if (!reflval) {
            error("Reference is invalid.\n");
          }

          if (reflval->type == T_LVALUE_BYTE) {
            push_number(*global_lvalue_byte.u.lvalue_byte);
            break;
          } else if (reflval->type == T_LVALUE_CODEPOINT) {
            push_number(u8_egc_index_as_single_codepoint(global_lvalue_codepoint.owner->u.string,
                                                         global_lvalue_codepoint.index));
            break;
          }
        }

        if (reflval->type == T_OBJECT && (reflval->u.ob->flags & O_DESTRUCTED)) {
          assign_svalue(reflval, &const0u);
        }
        push_svalue(reflval);

        break;
      }
      case F_REF_LVALUE: {
        svalue_t *s = fp + EXTRACT_UCHAR(pc++);

        if (s->type == T_REF) {
          if (s->u.ref->lvalue) {
            STACK_INC;
            sp->type = T_LVALUE;
            sp->u.lvalue = s->u.ref->lvalue;
          } else {
            error("Reference is invalid.\n");
          }
        } else {
          error("Non-reference value passed as reference argument.\n");
        }
        break;
      }
#endif
      case F_SHORT_INT: {
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
        if ((i = (sp->type == T_NUMBER)) && (sp->u.number == 0)) {
          pc += 2;
        } else {
          COPY_SHORT(&offset, pc);
          pc = current_prog->program + offset;
        }
        if (i) {
          sp--; /* when sp is an integer svalue, its cheaper
                 * to do this */
        } else {
          pop_stack();
        }
        break;
#endif
      case F_BRANCH: /* relative offset */
        COPY_SHORT(&offset, pc);
        pc += offset;
        break;
      case F_BBRANCH: /* relative offset */
        COPY_SHORT(&offset, pc);
        pc -= offset;
        break;
      case F_BRANCH_NE:
        f_ne();
        if ((sp--)->u.number) {
          COPY_SHORT(&offset, pc);
          pc += offset;
        } else {
          pc += 2;
        }
        break;
      case F_BRANCH_GE:
        f_ge();
        if ((sp--)->u.number) {
          COPY_SHORT(&offset, pc);
          pc += offset;
        } else {
          pc += 2;
        }
        break;
      case F_BRANCH_LE:
        f_le();
        if ((sp--)->u.number) {
          COPY_SHORT(&offset, pc);
          pc += offset;
        } else {
          pc += 2;
        }
        break;
      case F_BRANCH_EQ:
        f_eq();
        if ((sp--)->u.number) {
          COPY_SHORT(&offset, pc);
          pc += offset;
        } else {
          pc += 2;
        }
        break;
      case F_BBRANCH_LT:
        f_lt();
        if ((sp--)->u.number) {
          COPY_SHORT(&offset, pc);
          pc -= offset;
        } else {
          pc += 2;
        }
        break;
      case F_BRANCH_WHEN_ZERO: /* relative offset */
        if ((sp->type == T_NUMBER && !sp->u.number) || (sp->type == T_REAL && !sp->u.real)) {
          sp--;
          COPY_SHORT(&offset, pc);
          pc += offset;
          break;
        } else {
          pop_stack();
        }
        pc += 2; /* skip over the offset */
        break;
      case F_BRANCH_WHEN_NON_ZERO: /* relative offset */
        if ((sp->type == T_NUMBER && !sp->u.number) || (sp->type == T_REAL && !sp->u.real)) {
          sp--;
          pc += 2;
          break;
        } else {
          pop_stack();
        }
        COPY_SHORT(&offset, pc);
        pc += offset;
        break;
      case F_BBRANCH_WHEN_ZERO: /* relative backwards offset */
        if ((sp->type == T_NUMBER && !sp->u.number) || (sp->type == T_REAL && !sp->u.real)) {
          sp--;
          COPY_SHORT(&offset, pc);
          pc -= offset;
          break;
        } else {
          pop_stack();
        }
        pc += 2;
        break;
      case F_BBRANCH_WHEN_NON_ZERO: /* relative backwards offset */
        if ((sp->type == T_NUMBER && !sp->u.number) || (sp->type == T_REAL && !sp->u.real)) {
          sp--;
          pc += 2;
          break;
        } else {
          pop_stack();
        }
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
        } else {
          pop_stack();
        }
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
      case F_TRANSFER_LOCAL: {
        svalue_t *s;

        s = fp + EXTRACT_UCHAR(pc++);
        DEBUG_CHECK((fp - s) >= csp->num_local_variables, "Tried to push non-existent local\n");
        if ((s->type == T_OBJECT) && (s->u.ob->flags & O_DESTRUCTED)) {
          assign_svalue(s, &const0u);
        }

        STACK_INC;
        assign_svalue_no_free(sp, s);
        break;
      }
      case F_LOCAL: {
        svalue_t *s;

        s = fp + EXTRACT_UCHAR(pc++);
        DEBUG_CHECK((fp - s) >= csp->num_local_variables, "Tried to push non-existent local\n");

        /*
         * If variable points to a destructed object, replace it
         * with 0, otherwise, fetch value of variable.
         */
        if ((s->type == T_OBJECT) && (s->u.ob->flags & O_DESTRUCTED)) {
          assign_svalue(s, &const0u);
        }
        push_svalue(s);
        break;
      }
      case F_LT:
        f_lt();
        break;
      case F_ADD: {
        switch (sp->type) {
          case T_BUFFER: {
            if (!((sp - 1)->type == T_BUFFER)) {
              error("Bad type argument to +. Had %s and %s.\n", type_name((sp - 1)->type),
                    type_name(sp->type));
            } else {
              buffer_t *b;

              b = allocate_buffer(sp->u.buf->size + (sp - 1)->u.buf->size);
              memcpy(b->item, (sp - 1)->u.buf->item, (sp - 1)->u.buf->size);
              memcpy(b->item + (sp - 1)->u.buf->size, sp->u.buf->item, sp->u.buf->size);
              free_buffer((sp--)->u.buf);
              free_buffer(sp->u.buf);
              sp->u.buf = b;
            }
            break;
          } /* end of x + T_BUFFER */
          case T_NUMBER: {
            switch ((--sp)->type) {
              case T_NUMBER:
                sp->u.number += (sp + 1)->u.number;
                sp->subtype = 0;
                break;
              case T_REAL:
                sp->u.real += (sp + 1)->u.number;
                break;
              case T_STRING: {
                char buff[100];
                sprintf(buff, "%" LPC_INT_FMTSTR_P, (sp + 1)->u.number);
                EXTEND_SVALUE_STRING(sp, buff, "f_add: 2");
                break;
              }
              default:
                error("Bad type argument to +.  Had %s and %s.\n", type_name(sp->type),
                      type_name((sp + 1)->type));
            }
            break;
          } /* end of x + NUMBER */
          case T_REAL: {
            switch ((--sp)->type) {
              case T_NUMBER:
                sp->type = T_REAL;
                sp->u.real = sp->u.number + (sp + 1)->u.real;
                break;
              case T_REAL:
                sp->u.real += (sp + 1)->u.real;
                break;
              case T_STRING: {
                char buff[400];
                sprintf(buff, "%" LPC_FLOAT_FMTSTR_P, (sp + 1)->u.real);
                EXTEND_SVALUE_STRING(sp, buff, "f_add: 2");
                break;
              }
              default:
                error("Bad type argument to +. Had %s and %s\n", type_name(sp->type),
                      type_name((sp + 1)->type));
            }
            break;
          } /* end of x + T_REAL */
          case T_ARRAY: {
            if (!((sp - 1)->type == T_ARRAY)) {
              error("Bad type argument to +. Had %s and %s\n", type_name((sp - 1)->type),
                    type_name(sp->type));
            } else {
              /* add_array now free's the arrays */
              (sp - 1)->u.arr = add_array((sp - 1)->u.arr, sp->u.arr);
              sp--;
              break;
            }
          } /* end of x + T_ARRAY */
          case T_MAPPING: {
            if ((sp - 1)->type == T_MAPPING) {
              mapping_t *map;

              map = add_mapping((sp - 1)->u.map, sp->u.map);
              free_mapping((sp--)->u.map);
              free_mapping(sp->u.map);
              sp->u.map = map;
              break;
            } else {
              error("Bad type argument to +. Had %s and %s\n", type_name((sp - 1)->type),
                    type_name(sp->type));
            }
          } /* end of x + T_MAPPING */
          case T_STRING: {
            switch ((sp - 1)->type) {
              case T_OBJECT: {
                char buff[1024];
                object_t *ob = (sp - 1)->u.ob;
                sprintf(buff, "/%s", ob->obname);
                SVALUE_STRING_ADD_LEFT(buff, "f_add: 3");
                free_object(&ob, "f_add: 3");
                break;
              }
              case T_NUMBER: {
                char buff[100];
                sprintf(buff, "%" LPC_INT_FMTSTR_P, (sp - 1)->u.number);
                SVALUE_STRING_ADD_LEFT(buff, "f_add: 3");
                break;
              } /* end of T_NUMBER + T_STRING */
              case T_REAL: {
                char buff[400];
                sprintf(buff, "%" LPC_FLOAT_FMTSTR_P, (sp - 1)->u.real);
                SVALUE_STRING_ADD_LEFT(buff, "f_add: 3");
                break;
              } /* end of T_REAL + T_STRING */
              case T_STRING: {
                SVALUE_STRING_JOIN(sp - 1, sp, "f_add: 1");
                sp--;
                break;
              } /* end of T_STRING + T_STRING */
              default:
                error("Bad type argument to +. Had %s and %s\n", type_name((sp - 1)->type),
                      type_name(sp->type));
            }
            break;
          } /* end of x + T_STRING */
          case T_OBJECT:
            switch ((sp - 1)->type) {
              case T_STRING: {
                const char *fname = sp->u.ob->obname;
                free_object(&(sp--)->u.ob, "f_add: str+ob");
                EXTEND_SVALUE_STRING(sp, "/", "f_add: str ob");
                EXTEND_SVALUE_STRING(sp, fname, "f_add: str ob");
                break;
              }
              default:
                error("Bad type argument to +.  Had %s and %s.\n", type_name(sp->type),
                      type_name((sp + 1)->type));
            }
            break;
          default:
            error("Bad type argument to +.  Had %s and %s.\n", type_name((sp - 1)->type),
                  type_name(sp->type));
        }
        break;
      }
      case F_VOID_ADD_EQ:
      case F_ADD_EQ:
        DEBUG_CHECK(sp->type != T_LVALUE, "non-lvalue argument to +=\n");
        lval = sp->u.lvalue;
        sp--; /* points to the RHS */
        switch (lval->type) {
          case T_STRING:
            if (sp->type == T_STRING) {
              SVALUE_STRING_JOIN(lval, sp, "f_add_eq: 1");
            } else if (sp->type == T_NUMBER) {
              char buff[100];
              sprintf(buff, "%" LPC_INT_FMTSTR_P, sp->u.number);
              EXTEND_SVALUE_STRING(lval, buff, "f_add_eq: 2");
            } else if (sp->type == T_REAL) {
              char buff[400];
              sprintf(buff, "%" LPC_FLOAT_FMTSTR_P, sp->u.real);
              EXTEND_SVALUE_STRING(lval, buff, "f_add_eq: 2");
            } else if (sp->type == T_OBJECT) {
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
              error(
                  "Left hand side of += is a number (or zero); right side is "
                  "not a number.\n");
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
              error(
                  "Left hand side of += is a number (or zero); right side is "
                  "not a number.\n");
            }
            break;
          case T_BUFFER:
            if (sp->type != T_BUFFER) {
              bad_argument(sp, T_BUFFER, 2, instruction);
            } else {
              buffer_t *b;

              b = allocate_buffer(lval->u.buf->size + sp->u.buf->size);
              memcpy(b->item, lval->u.buf->item, lval->u.buf->size);
              memcpy(b->item + lval->u.buf->size, sp->u.buf->item, sp->u.buf->size);
              free_buffer(sp->u.buf);
              free_buffer(lval->u.buf);
              lval->u.buf = b;
            }
            break;
          case T_ARRAY:
            if (sp->type != T_ARRAY) {
              bad_argument(sp, T_ARRAY, 2, instruction);
            } else {
              /* add_array now frees the arrays */
              lval->u.arr = add_array(lval->u.arr, sp->u.arr);
            }
            break;
          case T_MAPPING:
            if (sp->type != T_MAPPING) {
              bad_argument(sp, T_MAPPING, 2, instruction);
            } else {
              absorb_mapping(lval->u.map, sp->u.map);
              free_mapping(sp->u.map); /* free RHS */
              /* LHS not freed because its being reused */
            }
            break;
          case T_LVALUE_BYTE: {
            char c;

            if (sp->type != T_NUMBER) {
              error("Bad right type to += of char lvalue.\n");
            }

            c = *global_lvalue_byte.u.lvalue_byte + sp->u.number;

            if (global_lvalue_byte.subtype == 0 && c == '\0') {
              error("Strings cannot contain 0 bytes.\n");
            }
            *global_lvalue_byte.u.lvalue_byte = c;
          } break;
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

      case F_FOREACH: {
        int flags = EXTRACT_UCHAR(pc++);

#ifdef DEBUG
        stack_in_use_as_temporary++;
#endif
        if (flags & FOREACH_MAPPING) {
          CHECK_TYPES(sp, T_MAPPING, 2, F_FOREACH);

          push_refed_array(mapping_indices(sp->u.map));

          STACK_INC;
          sp->type = T_NUMBER;
          sp->u.lvalue = (sp - 1)->u.arr->item;
          sp->subtype = (sp - 1)->u.arr->size;

          STACK_INC;
          sp->type = T_LVALUE;
          if (flags & FOREACH_LEFT_GLOBAL) {
            unsigned short idx = 0;
            LOAD2(idx, pc);
            sp->u.lvalue = find_value(idx + variable_index_offset);
          } else {
            sp->u.lvalue = fp + EXTRACT_UCHAR(pc++);
          }
        } else if (sp->type == T_STRING) {
          STACK_INC;
          sp->type = T_NUMBER;
          global_lvalue_codepoint.index = -1;
          global_lvalue_codepoint.owner = sp - 1;
          size_t count = 0;
          auto success = u8_egc_count((sp - 1)->u.string, &count);
          if (!success) {
            error("foreach: Invalid utf-8 string.");
          }
          sp->subtype = count;
        } else {
          CHECK_TYPES(sp, T_ARRAY, 2, F_FOREACH);

          STACK_INC;
          sp->type = T_NUMBER;
          sp->u.lvalue = (sp - 1)->u.arr->item;
          sp->subtype = (sp - 1)->u.arr->size;
        }

        if (flags & FOREACH_RIGHT_GLOBAL) {
          short idx = 0;
          LOAD2(idx, pc);

          STACK_INC;
          sp->type = T_LVALUE;
          sp->u.lvalue = find_value(idx + variable_index_offset);
        } else if (flags & FOREACH_REF) {
          ref_t *ref = make_ref();
          svalue_t *loc = fp + EXTRACT_UCHAR(pc++);

          /* foreach guarantees our target remains valid */
          ref->lvalue = nullptr;
          ref->sv.type = T_NUMBER;
          STACK_INC;
          sp->type = T_REF;
          sp->u.ref = ref;
          DEBUG_CHECK(loc->type != T_NUMBER && loc->type != T_REF,
                      "Somehow a reference in foreach acquired a value before "
                      "coming into scope");
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
        if ((sp - 1)->type == T_LVALUE) {
          /* mapping */
          if ((sp - 2)->subtype--) {
            svalue_t *key = (sp - 2)->u.lvalue++;
            svalue_t *value = find_in_mapping((sp - 4)->u.map, key);

            assign_svalue((sp - 1)->u.lvalue, key);
            if (sp->type == T_REF) {
              if (value == &const0u) {
                sp->u.ref->lvalue = nullptr;
              } else {
                sp->u.ref->lvalue = value;
              }
            } else {
              assign_svalue(sp->u.lvalue, value);
            }
            COPY_SHORT(&offset, pc);
            pc -= offset;
            break;
          }
        } else {
          /* array or string */
          if ((sp - 1)->subtype--) {
            if ((sp - 2)->type == T_STRING) {
              if (sp->type == T_REF) {
                sp->u.ref->lvalue = &global_lvalue_codepoint_sv;
                global_lvalue_codepoint.index++;
              } else {
                free_svalue(sp->u.lvalue, "foreach-string");
                sp->u.lvalue->type = T_NUMBER;
                sp->u.lvalue->subtype = 0;
                sp->u.lvalue->u.number = u8_egc_index_as_single_codepoint(
                    global_lvalue_codepoint.owner->u.string, global_lvalue_codepoint.index++);
              }
            } else {
              if (sp->type == T_REF) {
                sp->u.ref->lvalue = (sp - 1)->u.lvalue++;
              } else {
                assign_svalue(sp->u.lvalue, (sp - 1)->u.lvalue++);
              }
            }
            COPY_SHORT(&offset, pc);
            pc -= offset;
            break;
          }
        }
        pc += 2;
        /* fallthrough */
      case F_EXIT_FOREACH:
#ifdef DEBUG
        stack_in_use_as_temporary--;
#endif
        if (sp->type == T_REF) {
          if (sp->u.ref->lvalue == &global_lvalue_codepoint_sv) {
            sp->u.ref->lvalue = nullptr;

            global_lvalue_codepoint.index = 0;
            global_lvalue_codepoint.owner = nullptr;
          } else {
            sp->u.ref->lvalue = nullptr;
          }

          if (!(--sp->u.ref->ref) && sp->u.ref->lvalue == nullptr) {
            FREE(sp->u.ref);
          }
        }
        if ((sp - 1)->type == T_LVALUE) {
          /* mapping */
          sp -= 3;
          free_array((sp--)->u.arr);
          free_mapping((sp--)->u.map);
        } else {
          /* array or string */
          sp -= 2;
          if (sp->type == T_STRING) {
            free_string_svalue(sp--);
          } else {
            free_array((sp--)->u.arr);
          }
        }
        break;

      case F_EXPAND_VARARGS: {
        svalue_t *s, *t;
        array_t *arr;

        i = EXTRACT_UCHAR(pc++);
        s = sp - i;

        if (s->type != T_ARRAY) {
          error("Item being expanded with ... is not an array\n");
        }

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
            while (n--) {
              assign_svalue_no_free(t--, &arr->item[n]);
            }
          }
        }
        free_array(arr);
        break;
      }

      case F_NEW_CLASS: {
        array_t *cl;

        cl = allocate_class(&current_prog->classes[EXTRACT_UCHAR(pc++)], 1);
        push_refed_class(cl);
      } break;
      case F_NEW_EMPTY_CLASS: {
        array_t *cl;

        cl = allocate_class(&current_prog->classes[EXTRACT_UCHAR(pc++)], 0);
        push_refed_class(cl);
      } break;
      case F_AGGREGATE: {
        array_t *v;

        LOAD_SHORT(offset, pc);
        offset += num_varargs;
        num_varargs = 0;
        v = allocate_empty_array(offset);
        /*
         * transfer svalues in reverse...popping stack as we go
         */
        while (offset--) {
          v->item[offset] = *sp--;
        }
        push_refed_array(v);
      } break;
      case F_AGGREGATE_ASSOC: {
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
        if (sp->type != T_LVALUE) {
          fatal("Bad argument to F_ASSIGN\n");
        }
#endif
        switch (sp->u.lvalue->type) {
          case T_LVALUE_BYTE: {
            if ((sp - 1)->type != T_NUMBER) {
              error("Illegal rhs to byte lvalue\n");
            }
            *global_lvalue_byte.u.lvalue_byte = (sp - 1)->u.number;
          } break;
          case T_LVALUE_RANGE:
            assign_lvalue_range(sp - 1);
            break;
          case T_LVALUE_CODEPOINT: {
            if ((sp - 1)->type != T_NUMBER) {
              error("Illegal rhs to char lvalue\n");
            }
            UChar32 newc = (sp - 1)->u.number;
            assign_lvalue_codepoint([=](UChar32 c) { return newc; });
            break;
          }
          default:
            assign_svalue(sp->u.lvalue, sp - 1);
            break;
        }
        sp--; /* ignore lvalue */
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
        if (sp->type != T_LVALUE) {
          fatal("Bad argument to F_VOID_ASSIGN\n");
        }
#endif
        lval = (sp--)->u.lvalue;
        if (sp->type != T_INVALID) {
          switch (lval->type) {
            case T_LVALUE_BYTE: {
              if (sp->type != T_NUMBER) {
                error("Illegal rhs to byte lvalue\n");
              } else {
                char c = (sp--)->u.number & 0xff;
                *global_lvalue_byte.u.lvalue_byte = c;
              }
              break;
            }
            case T_LVALUE_RANGE: {
              copy_lvalue_range(sp--);
              break;
            }
            case T_LVALUE_CODEPOINT: {
              if (sp->type != T_NUMBER) {
                error("Illegal rhs to byte lvalue\n");
              }
              UChar32 newc = sp->u.number;
              assign_lvalue_codepoint([=](UChar32 c) { return newc; });
              pop_stack();
              break;
            }
            default: {
              free_svalue(lval, "F_VOID_ASSIGN : 3");
              *lval = *sp--;
            }
          }
        } else {
          sp--;
        }
        break;
#ifdef DEBUG
      case F_BREAK_POINT:
        break_point();
        break;
#endif
      case F_CALL_FUNCTION_BY_ADDRESS: {
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

        if (current_object->prog->function_flags[offset] & (FUNC_PROTOTYPE | FUNC_UNDEFINED)) {
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
        // if(offset > USHRT_MAX)
        // error("Broken function table"); offset is a USHRT, so this just can't
        // happen!
        funp = setup_new_frame(offset);
        csp->pc = pc; /* The corrected return address */
        pc = current_prog->program + funp->address;
        if (Tracer::enabled()) {
          csp->trace_id = ::get_trace_id(csp);
          trace_context = ::get_trace_context(csp, sp);
          Tracer::begin(*csp->trace_id, EventCategory::LPC_FUNCTION, std::move(trace_context));
        }
      } break;
      case F_CALL_INHERITED: {
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

        if (Tracer::enabled()) {
          csp->trace_id = ::get_trace_id(csp);
          trace_context = ::get_trace_context(csp, sp);
          Tracer::begin(*csp->trace_id, EventCategory::LPC_FUNCTION, std::move(trace_context));
        }
      } break;
      case F_COMPL:
        if (sp->type != T_NUMBER) {
          error("Bad argument to ~\n");
        }
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
        DEBUG_CHECK(sp->type != T_LVALUE, "non-lvalue argument to --\n");
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
            if (global_lvalue_byte.subtype == 0 && *global_lvalue_byte.u.lvalue_byte == '\x1') {
              error("Strings cannot contain 0 bytes.\n");
            }
            sp->type = T_NUMBER;
            sp->subtype = 0;
            sp->u.number = --(*global_lvalue_byte.u.lvalue_byte);
            break;
          default:
            error("-- of non-numeric argument\n");
        }
        break;
      case F_DEC:
        DEBUG_CHECK(sp->type != T_LVALUE, "non-lvalue argument to --\n");
        lval = (sp--)->u.lvalue;
        switch (lval->type) {
          case T_NUMBER:
            lval->u.number--;
            break;
          case T_REAL:
            lval->u.real--;
            break;
          case T_LVALUE_BYTE:
            --(*global_lvalue_byte.u.lvalue_byte);
            break;
          case T_LVALUE_CODEPOINT:
            assign_lvalue_codepoint([](UChar32 c) { return c - 1; });
            break;
          default:
            error("-- of non-numeric argument\n");
        }
        break;
      case F_DIVIDE: {
        switch ((sp - 1)->type | sp->type) {
          case T_NUMBER: {
            if (!(sp--)->u.number) {
              error("Division by zero\n");
            }
            sp->u.number /= (sp + 1)->u.number;
            break;
          }

          case T_REAL: {
            if ((sp--)->u.real == 0.0) {
              error("Division by zero\n");
            }
            sp->u.real /= (sp + 1)->u.real;
            break;
          }

          case T_NUMBER | T_REAL: {
            if ((sp--)->type == T_NUMBER) {
              if (!((sp + 1)->u.number)) {
                error("Division by zero\n");
              }
              sp->u.real /= (sp + 1)->u.number;
            } else {
              if ((sp + 1)->u.real == 0.0) {
                error("Division by 0.0\n");
              }
              sp->type = T_REAL;
              sp->u.real = sp->u.number / (sp + 1)->u.real;
            }
            break;
          }

          default: {
            if (!((sp - 1)->type & (T_NUMBER | T_REAL))) {
              bad_argument(sp - 1, T_NUMBER | T_REAL, 1, instruction);
            }
            if (!(sp->type & (T_NUMBER | T_REAL))) {
              bad_argument(sp, T_NUMBER | T_REAL, 2, instruction);
            }
          }
        }
      } break;
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
      case F_GLOBAL: {
        svalue_t *s;

        unsigned short idx = 0;
        LOAD2(idx, pc);
        s = find_value(idx + variable_index_offset);

        /*
         * If variable points to a destructed object, replace it
         * with 0, otherwise, fetch value of variable.
         */
        if ((s->type == T_OBJECT) && (s->u.ob->flags & O_DESTRUCTED)) {
          assign_svalue(s, &const0u);
        }
        push_svalue(s);
        break;
      }
      case F_PRE_INC:
        DEBUG_CHECK(sp->type != T_LVALUE, "non-lvalue argument to ++\n");
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
            sp->type = T_NUMBER;
            sp->subtype = 0;
            sp->u.number = ++*global_lvalue_byte.u.lvalue_byte;
            break;
          case T_LVALUE_CODEPOINT:
            assign_lvalue_codepoint([](UChar32 c) { return ++c; });
            break;
          default:
            error("++ of non-numeric argument\n");
        }
        break;
      case F_MEMBER: {
        array_t *arr;

        if (sp->type != T_CLASS) {
          error("Tried to take a member of something that isn't a class.\n");
        }
        i = EXTRACT_UCHAR(pc++);
        arr = sp->u.arr;
        if (i >= arr->size) {
          error("Class has no corresponding member.\n");
        }
        if (arr->item[i].type == T_OBJECT && (arr->item[i].u.ob->flags & O_DESTRUCTED)) {
          assign_svalue(&arr->item[i], &const0u);
        }
        assign_svalue_no_free(sp, &arr->item[i]);
        free_class(arr);

        break;
      }
      case F_MEMBER_LVALUE: {
        array_t *arr;

        if (sp->type != T_CLASS) {
          error("Tried to take a member of something that isn't a class.\n");
        }
        i = EXTRACT_UCHAR(pc++);
        arr = sp->u.arr;
        if (i >= arr->size) {
          error("Class has no corresponding member.\n");
        }
        sp->type = T_LVALUE;
        sp->u.lvalue = arr->item + i;
#ifdef REF_RESERVED_WORD
        lv_owner_type = T_CLASS;
        lv_owner = reinterpret_cast<refed_t *>(arr);
#endif
        free_class(arr);
        break;
      }
      case F_INDEX:
        switch (sp->type) {
          case T_MAPPING: {
            ScopedTracer _tracer_index("F_INDEX: mapping");

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
          case T_BUFFER: {
            ScopedTracer _tracer_index("F_INDEX: buffer");

            if ((sp - 1)->type != T_NUMBER) {
              error("Buffer indexes must be integers.\n");
            }

            i = (sp - 1)->u.number;
            if ((i >= sp->u.buf->size) || (i < 0)) {
              error("Buffer index out of bounds.\n");
            }
            i = sp->u.buf->item[i];
            free_buffer(sp->u.buf);
            (--sp)->u.number = i;
            sp->subtype = 0;
            break;
          }
          case T_STRING: {
            ScopedTracer _tracer_index("F_INDEX: string");

            if ((sp - 1)->type != T_NUMBER) {
              error("String indexes must be integers.\n");
            }
            i = (sp - 1)->u.number;
            if (i < 0) {
              error("String index out of bounds.\n");
            }

            UChar32 res = u8_egc_index_as_single_codepoint(sp->u.string, i);
            if (res == -2) {
              error("String index out of bounds.\n");
            } else if (res < 0) {
              error("String index only work for single codepoint character.\n");
            }
            free_string_svalue(sp);
            (--sp)->u.number = res;
            break;
          }
          case T_ARRAY: {
            ScopedTracer _tracer_index("F_INDEX: array");

            array_t *arr;

            if ((sp - 1)->type != T_NUMBER) {
              error("Array indexes must be integers.\n");
            }
            i = (sp - 1)->u.number;
            if (i < 0) {
              error("Array index must be positive or zero.\n");
            }
            arr = sp->u.arr;
            if (i >= arr->size) {
              error("Array index out of bounds.\n");
            }
            if (arr->item[i].type == T_OBJECT && (arr->item[i].u.ob->flags & O_DESTRUCTED)) {
              assign_svalue(&arr->item[i], &const0u);
            }
            assign_svalue_no_free(--sp, &arr->item[i]);
            free_array(arr);
            break;
          }
          default:
            if (sp->type == T_NUMBER && !sp->u.number) {
              error("Value being indexed is zero.\n");
            }
            error("Cannot index value of type '%s'.\n", type_name(sp->type));
        }
        break;
      case F_RINDEX:
        switch (sp->type) {
          case T_BUFFER: {
            if ((sp - 1)->type != T_NUMBER) {
              error("Indexing a buffer with an illegal type.\n");
            }

            i = sp->u.buf->size - (sp - 1)->u.number;
            if ((i > sp->u.buf->size) || (i < 0)) {
              error("Buffer index out of bounds.\n");
            }

            i = sp->u.buf->item[i];
            free_buffer(sp->u.buf);
            (--sp)->u.number = i;
            sp->subtype = 0;
            break;
          }
          case T_STRING: {
            if ((sp - 1)->type != T_NUMBER) {
              error("Indexing a string with an illegal type.\n");
            }
            size_t count;
            auto success = u8_egc_count(sp->u.string, &count);
            if (!success) {
              error("Invalid UTF8 string: f_rindex.\n");
            }
            i = count - (sp - 1)->u.number;
            // COMPAT: allow str[<0] == 0
            if ((i > count) || (i < 0)) {
              error("String rindex out of bounds.\n");
            }
            UChar32 c = u8_egc_index_as_single_codepoint(sp->u.string, i);
            if (c < 0) {
              error("String rindex only work for single codepoint character.\n");
            }
            free_string_svalue(sp);
            (--sp)->u.number = c;
            break;
          }
          case T_ARRAY: {
            array_t *arr = sp->u.arr;

            if ((sp - 1)->type != T_NUMBER) {
              error("Indexing an array with an illegal type.\n");
            }
            i = arr->size - (sp - 1)->u.number;
            if (i < 0 || i >= arr->size) {
              error("Array index out of bounds.\n");
            }
            if (arr->item[i].type == T_OBJECT && (arr->item[i].u.ob->flags & O_DESTRUCTED)) {
              assign_svalue(&arr->item[i], &const0u);
            }
            assign_svalue_no_free(--sp, &arr->item[i]);
            free_array(arr);
            break;
          }
          default:
            if (sp->type == T_NUMBER && !sp->u.number) {
              error("Value being indexed is zero.\n");
            }
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
          sp--; /* cheaper to do this when sp is an integer
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
      case F_MOD: {
        CHECK_TYPES(sp - 1, T_NUMBER, 1, instruction);
        CHECK_TYPES(sp, T_NUMBER, 2, instruction);
        if ((sp--)->u.number == 0) {
          error("Modulus by zero.\n");
        }
        sp->u.number %= (sp + 1)->u.number;
      } break;
      case F_MOD_EQ:
        f_mod_eq();
        break;
      case F_MULTIPLY: {
        switch ((sp - 1)->type | sp->type) {
          case T_NUMBER: {
            sp--;
            sp->u.number *= (sp + 1)->u.number;
            break;
          }

          case T_REAL: {
            sp--;
            sp->u.real *= (sp + 1)->u.real;
            break;
          }

          case T_NUMBER | T_REAL: {
            if ((--sp)->type == T_NUMBER) {
              sp->type = T_REAL;
              sp->u.real = sp->u.number * (sp + 1)->u.real;
            } else {
              sp->u.real *= (sp + 1)->u.number;
            }
            break;
          }

          case T_MAPPING: {
            mapping_t *m;
            m = compose_mapping((sp - 1)->u.map, sp->u.map, 1);
            pop_2_elems();
            push_refed_mapping(m);
            break;
          }

          default: {
            if (!((sp - 1)->type & (T_NUMBER | T_REAL | T_MAPPING))) {
              bad_argument(sp - 1, T_NUMBER | T_REAL | T_MAPPING, 1, instruction);
            }
            if (!(sp->type & (T_NUMBER | T_REAL | T_MAPPING))) {
              bad_argument(sp, T_NUMBER | T_REAL | T_MAPPING, 2, instruction);
            }
            error("Args to * are not compatible.\n");
          }
        }
      } break;
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
        } else if (sp->type == T_REAL) {
          sp->u.real = -sp->u.real;
        } else {
          error("Bad argument to unary minus\n");
        }
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
        DEBUG_CHECK(sp->type != T_LVALUE, "non-lvalue argument to --\n");
        lval = sp->u.lvalue;
        switch (lval->type) {
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
            sp->subtype = 0;
            sp->u.number = (*global_lvalue_byte.u.lvalue_byte)--;
            break;
          case T_LVALUE_CODEPOINT:
            assign_lvalue_codepoint([](UChar32 c) { return --c; });
            break;
          default:
            error("-- of non-numeric argument\n");
        }
        break;
      case F_POST_INC:
        DEBUG_CHECK(sp->type != T_LVALUE, "non-lvalue argument to ++\n");
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
            sp->type = T_NUMBER;
            sp->u.number = (*global_lvalue_byte.u.lvalue_byte)++;
            sp->subtype = 0;
            break;
          case T_LVALUE_CODEPOINT:
            assign_lvalue_codepoint([](UChar32 c) { return c++; });
            break;
          default:
            error("++ of non-numeric argument\n");
        }
        break;
      case F_GLOBAL_LVALUE: {
        unsigned short idx = 0;
        LOAD2(idx, pc);
        STACK_INC;
        sp->type = T_LVALUE;
        sp->u.lvalue = find_value(idx + variable_index_offset);
        break;
      }
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
      case F_RETURN_ZERO: {
        if (csp->framekind & FRAME_CATCH) {
          free_svalue(&catch_value, "F_RETURN_ZERO");
          catch_value = const0;
          while (csp->framekind & FRAME_CATCH) {
            pop_control_stack();
          }
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
        /* The control stack was popped just before */
        if (csp[1].framekind & (FRAME_EXTERNAL | FRAME_RETURNED_FROM_CATCH)) {
          return;
        }
      } break;
      case F_RETURN: {
        svalue_t sv;

        if (csp->framekind & FRAME_CATCH) {
          free_svalue(&catch_value, "F_RETURN");
          catch_value = const0;
          while (csp->framekind & FRAME_CATCH) {
            pop_control_stack();
          }
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
        /* The control stack was popped just before */
        if (csp[1].framekind & (FRAME_EXTERNAL | FRAME_RETURNED_FROM_CATCH)) {
          return;
        }
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
        DEBUG_CHECK1(offset >= current_prog->num_strings, "string %d out of range in F_STRING!\n",
                     offset);
        push_shared_string(current_prog->strings[offset]);
        break;
      case F_SHORT_STRING:
        DEBUG_CHECK1(EXTRACT_UCHAR(pc) >= current_prog->num_strings,
                     "string %d out of range in F_STRING!\n", EXTRACT_UCHAR(pc));
        push_shared_string(current_prog->strings[EXTRACT_UCHAR(pc++)]);
        break;
      case F_SUBTRACT: {
        i = (sp--)->type;
        switch (i | sp->type) {
          case T_NUMBER:
            sp->u.number -= (sp + 1)->u.number;
            break;

          case T_REAL:
            sp->u.real -= (sp + 1)->u.real;
            break;

          case T_NUMBER | T_REAL:
            if (sp->type == T_REAL) {
              sp->u.real -= (sp + 1)->u.number;
            } else {
              sp->type = T_REAL;
              sp->u.real = sp->u.number - (sp + 1)->u.real;
            }
            break;

          case T_ARRAY: {
            /*
             * subtract_array already takes care of
             * destructed objects
             */
            sp->u.arr = subtract_array(sp->u.arr, (sp + 1)->u.arr);
            break;
          }

          default:
            if (!((sp++)->type & (T_NUMBER | T_REAL | T_ARRAY))) {
              error("Bad left type to -.\n");
            } else if (!(sp->type & (T_NUMBER | T_REAL | T_ARRAY))) {
              error("Bad right type to -.\n");
            } else {
              error("Arguments to - do not have compatible types.\n");
            }
        }
        break;
      }
      case F_SUB_EQ:
        f_sub_eq();
        break;
      case F_SIMUL_EFUN: {
        unsigned short sindex;
        int num_args;

        LOAD_SHORT(sindex, pc);
        num_args = EXTRACT_UCHAR(pc++) + num_varargs;
        num_varargs = 0;
        call_simul_efun(sindex, num_args);
      } break;
      case F_SWITCH:
        f_switch();
        break;
      case F_XOR:
        f_xor();
        break;
      case F_XOR_EQ:
        f_xor_eq();
        break;
      case F_CATCH: {
        /*
         * Compute address of next instruction after the CATCH
         * statement.
         */
        LOAD_SHORT(offset, pc);
        offset = (pc - 2) + offset - current_prog->program;

        do_catch(pc, offset);
        if ((csp[1].framekind & (FRAME_EXTERNAL | FRAME_RETURNED_FROM_CATCH)) ==
            (FRAME_EXTERNAL | FRAME_RETURNED_FROM_CATCH)) {
          return;
        }

        break;
      }
      case F_END_CATCH: {
        free_svalue(&catch_value, "F_END_CATCH");
        catch_value = const0;
        /* We come here when no longjmp() was executed */
        pop_control_stack();
        push_number(0);
        return; /* return to do_catch */
      }
      case F_TIME_EXPRESSION: {
        long sec, usec;
#ifdef DEBUG
        stack_in_use_as_temporary++;
#endif
        get_usec_clock(&sec, &usec);
        push_number(sec);
        push_number(usec);
        break;
      }
      case F_END_TIME_EXPRESSION: {
        long sec, usec;

        get_usec_clock(&sec, &usec);
        usec = (sec - (sp - 1)->u.number) * 1000000 + (usec - sp->u.number);
        sp -= 2;
#ifdef DEBUG
        stack_in_use_as_temporary--;
#endif
        push_number(usec);
        break;
      }
      case F_TYPE_CHECK: {
        int type = sp->u.number;
        pop_stack();
        if (sp->type != type && !(sp->type == T_NUMBER && sp->u.number == 0) &&
            !(sp->type == T_LVALUE)) {
          error("Trying to put %s in %s\n", type_name(sp->type), type_name(type));
        }
        break;
      }
#define CALL_THE_EFUN goto call_the_efun
      case F_EFUN0:
        st_num_arg = 0;
        LOAD_SHORT(instruction, pc);
        CALL_THE_EFUN;
        break;
      case F_EFUN1:
        st_num_arg = 1;
        LOAD_SHORT(instruction, pc);
        CHECK_TYPES(sp, instrs[instruction].type[0], 1, instruction);
        CALL_THE_EFUN;
        break;
      case F_EFUN2:
        st_num_arg = 2;
        LOAD_SHORT(instruction, pc);
        CHECK_TYPES(sp - 1, instrs[instruction].type[0], 1, instruction);
        CHECK_TYPES(sp, instrs[instruction].type[1], 2, instruction);
        CALL_THE_EFUN;
        break;
      case F_EFUN3:
        st_num_arg = 3;
        LOAD_SHORT(instruction, pc);
        CHECK_TYPES(sp - 2, instrs[instruction].type[0], 1, instruction);
        CHECK_TYPES(sp - 1, instrs[instruction].type[1], 2, instruction);
        CHECK_TYPES(sp, instrs[instruction].type[2], 3, instruction);
        CALL_THE_EFUN;
        break;
      case F_EFUNV: {
        int num;
        LOAD_SHORT(instruction, pc);
        st_num_arg = EXTRACT_UCHAR(pc++) + num_varargs;
        num_varargs = 0;
        num = instrs[instruction].min_arg;
        for (i = 1; i <= num; i++) {
          CHECK_TYPES(sp - st_num_arg + i, instrs[instruction].type[i - 1], i, instruction);
        }
        CALL_THE_EFUN;
        break;
      }
      default:
        /* un-recognized instruction */
        if (instruction < EFUN_BASE) {
          fatal("No case for eoperator %s (%d)\n", query_instr_name(instruction), instruction);
        } else {
          fatal("Undefined instruction %s (%d)\n", query_instr_name(instruction), instruction);
        }
        break;
      call_the_efun:
#ifdef DEBUG
        /* We have an efun.  Execute it.*/
        if (instruction < EFUN_BASE || instruction > NUM_OPCODES) {
          fatal("wrong!");
        }
        if (instrs[instruction].ret_type == TYPE_NOVALUE) {
          expected_stack = sp - st_num_arg;
        } else {
          expected_stack = sp - st_num_arg + 1;
        }
        num_arg = st_num_arg;
#endif
        {
          if (Tracer::enabled() && CONFIG_INT(__RC_TRACE_CONTEXT__)) {
            trace_context["args"] = get_trace_args(sp, st_num_arg);
          }

          ScopedTracer _efun_tracer(instrs[instruction].name, EventCategory::LPC_EFUN,
                                    std::move(trace_context));
          (*efun_table[instruction - EFUN_BASE])();
        }

#ifdef DEBUG
        if (expected_stack != sp) {
          fatal("Bad sp %p (should be %p) after calling efun '%s', num arg %d.\n", sp,
                expected_stack, instrs[instruction].name, num_arg);
        }
#endif
    } /* switch (instruction) */
    DEBUG_CHECK2(sp < fp + csp->num_local_variables - 1,
                 "Bad stack after evaluation. Instruction '%s' (%d) \n", instrs[instruction].name,
                 instruction);
#if defined(DEBUG) && 0  // super slow
    {
      svalue_t *current_stack = sp;
      while (current_stack >= fp) {
        auto v = *current_stack--;
        if (v.type == T_STRING) {
          DEBUG_CHECK2(!u8_validate((const uint8_t *)v.u.string),
                       "Corrupted UTF8 string after evaluation. Instruction '%s' (%d)\n",
                       instrs[instruction].name, instruction);
        }
      }
    }
#endif
  } /* while (1) */
}

static void do_catch(char *pc, unsigned short new_pc_offset) {
  error_context_t econ;

  /*
   * Save some global variables that must be restored separately after a
   * longjmp. The stack will have to be manually popped all the way.
   */
  save_context(&econ);
  push_control_stack(FRAME_CATCH);
  csp->pc = current_prog->program + new_pc_offset;
  if (CONFIG_INT(__RC_TRACE_CODE__)) {
    csp->num_local_variables = (csp - 1)->num_local_variables; /* marion */
  } else {
#if defined(DEBUG)
    csp->num_local_variables = (csp - 1)->num_local_variables; /* marion */
#endif
  }

  assign_svalue(&catch_value, &const1);
  try {
    ScopedTracer _tracer("Catch", EventCategory::LPC_CATCH);
    /* note, this will work, since csp->extern_call won't be used */
    eval_instruction(pc);
  } catch (const char *) {
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
    if (too_deep_error) {
      too_deep_error = 0;
    }
  }
  pop_context(&econ);
}

static program_t *ffbn_recurse(program_t *prog, char *name, int *indexp, int *runtime_index) {
  int high = prog->num_functions_defined - 1;
  int low = 0, mid;
  int ri;
  char *p;

  /* Search our function table */
  while (high >= low) {
    mid = (high + low) >> 1;
    p = prog->function_table[mid].funcname;
    if (name < p) {
      high = mid - 1;
    } else if (name > p) {
      low = mid + 1;
    } else {
      ri = mid + prog->last_inherited;

      if (prog->function_flags[ri] & (FUNC_UNDEFINED | FUNC_PROTOTYPE)) {
        return nullptr;
      }

      *indexp = mid;
      *runtime_index = ri;
      return prog;
    }
  }

  /* Search inherited function tables */
  mid = prog->num_inherited;
  while (mid--) {
    program_t *ret = ffbn_recurse(prog->inherit[mid].prog, name, indexp, runtime_index);
    if (ret) {
      *runtime_index += prog->inherit[mid].function_index_offset;
      return ret;
    }
  }
  return nullptr;
}

program_t *find_function_by_name(object_t *ob, const char *name, int *indexp, int *runtime_index) {
  char *funname = findstring(name);

  if (!funname) {
    return nullptr;
  }
  return ffbn_recurse(ob->prog, funname, indexp, runtime_index);
}

/* Reason for the following 1. save cache space 2. speed :) */
/* The following is to be called only from reset_object for */
/* otherwise extra checks are needed - Sym                  */

void call___INIT(object_t *ob) {
  program_t *progp;
  function_t *cfp;
  int num_functions;
#ifdef DEBUG
  svalue_t *expected_sp;
  control_stack_t *save_csp;
#endif

  tracedepth = 0;

#ifdef DEBUG
  expected_sp = sp;
#endif

  /* No try_reset here for obvious reasons :) */

  ob->flags &= ~O_RESET_STATE;

  progp = ob->prog;
  num_functions = progp->num_functions_defined;
  if (!num_functions) {
    return;
  }

  /* ___INIT turns out to be always the last function */
  cfp = &progp->function_table[num_functions - 1];
  if (cfp->funcname[0] != APPLY___INIT_SPECIAL_CHAR) {
    return;
  }
  push_control_stack(FRAME_FUNCTION | FRAME_OB_CHANGE);
  current_prog = progp;
  csp->fr.table_index = num_functions - 1;
#ifdef PROFILE_FUNCTIONS
  get_cpu_times(&(csp->entry_secs), &(csp->entry_usecs));
  current_prog->function_table[num_functions - 1].calls++;
#endif
  caller_type = ORIGIN_DRIVER;
  csp->num_local_variables = 0;

  previous_ob = current_object;

  current_object = ob;
  setup_new_frame(num_functions - 1 + progp->last_inherited);
#ifdef DEBUG
  save_csp = csp;
#endif
  call_program(current_prog, cfp->address);

  DEBUG_CHECK(save_csp - 1 != csp, "Bad csp after execution in apply_low\n");
  sp--;
  DEBUG_CHECK(expected_sp != sp, "Corrupt stack pointer.\n");
}

/*
 * Call a function in all objects in a array.
 */
array_t *call_all_other(array_t *v, const char *func, int numargs) {
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
      if (!ob || !object_visible(ob)) {
        continue;
      }
    } else {
      continue;
    }
    if (ob->flags & O_DESTRUCTED) {
      continue;
    }
    i = numargs;
    while (i--) {
      push_svalue(tmp - i);
    }
    call_origin = ORIGIN_CALL_OTHER;
    if (apply_low(func, ob, numargs)) {
      *rptr = *sp--;
    }
  }
  sp--;
  pop_n_elems(numargs);
  return ret;
}

char *function_name(program_t *prog, int findex) {
  int low, high, mid;

  /* Walk up the inheritance tree to the real definition */
  if (prog->function_flags[findex] & FUNC_ALIAS) {
    findex = prog->function_flags[findex] & ~FUNC_ALIAS;
  }

  while (prog->function_flags[findex] & FUNC_INHERITED) {
    low = 0;
    high = prog->num_inherited - 1;

    while (high > low) {
      mid = (low + high + 1) >> 1;
      if (prog->inherit[mid].function_index_offset > findex) {
        high = mid - 1;
      } else {
        low = mid;
      }
    }
    findex -= prog->inherit[low].function_index_offset;
    prog = prog->inherit[low].prog;
  }

  findex -= prog->last_inherited;

  return prog->function_table[findex].funcname;
}

/*
 * This function is similar to apply(), except that it will not
 * call the function, only return object name if the function exists,
 * or 0 otherwise.  If flag is nonzero, then we admit static and private
 * functions exist.  Note that if you actually intend to call the function,
 * it's faster to just try to call it and check if apply() returns zero.
 */
const char *function_exists(const char *fun, object_t *ob, int flag) {
  int findex, runtime_index;
  program_t *prog;
  int flags;

  DEBUG_CHECK(ob->flags & O_DESTRUCTED, "function_exists() on destructed object\n");

  if (fun[0] == APPLY___INIT_SPECIAL_CHAR) {
    return nullptr;
  }

  prog = find_function_by_name(ob, fun, &findex, &runtime_index);
  if (!prog) {
    return nullptr;
  }

  flags = ob->prog->function_flags[runtime_index];

  if ((flags & FUNC_UNDEFINED) ||
      (!flag && (flags & (DECL_PROTECTED | DECL_PRIVATE | DECL_HIDDEN)))) {
    return nullptr;
  }

  return prog->filename;
}

#ifndef NO_SHADOWS
/*
  is_static: returns 1 if a function named 'fun' is declared 'static' in 'ob';
  0 otherwise.
*/
int is_static(const char *fun, object_t *ob) {
  int findex;
  int runtime_index;
  program_t *prog;
  int flags;

  DEBUG_CHECK(ob->flags & O_DESTRUCTED, "is_static() on destructed object\n");

  prog = find_function_by_name(ob, fun, &findex, &runtime_index);
  if (!prog) {
    return 0;
  }

  flags = ob->prog->function_flags[runtime_index];
  if (flags & (FUNC_UNDEFINED | FUNC_PROTOTYPE)) {
    return 0;
  }
  if (flags & (DECL_PROTECTED | DECL_PRIVATE | DECL_HIDDEN)) {
    return 1;
  }

  return 0;
}
#endif

/*
 * Call a function by object and index number.  Used by parts of the
 * driver which cache function numbers to optimize away function lookup.
 * The return value is left on the stack.
 * Currently: heart_beats, simul_efuns, master applies.
 */
void call_direct(object_t *ob, int offset, int origin, int num_arg) {
  function_t *funp;
  program_t *prog = ob->prog;

  ob->time_of_ref = g_current_gametick;
  push_control_stack(FRAME_FUNCTION | FRAME_OB_CHANGE);
  caller_type = origin;
  csp->num_local_variables = num_arg;
  current_prog = prog;
  previous_ob = current_object;
  current_object = ob;
  funp = setup_new_frame(offset);
  call_program(current_prog, funp->address);
}

void translate_absolute_line(int abs_line, unsigned short *file_info, int *ret_file,
                             int *ret_line) {
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
    if (p2[1] == file) {
      line_tmp += *p2;
    }
    p2 += 2;
  }
  *ret_line = line_tmp;
  *ret_file = file;
}

static int find_line(char *p, const program_t *progp, const char **ret_file, int *ret_line) {
  ScopedTracer _tracer(__PRETTY_FUNCTION__);

  int offset;
  unsigned char *lns;
  ADDRESS_TYPE abs_line;
  int file_idx;

  *ret_file = nullptr;
  *ret_line = 0;

  if (!progp) {
    return 1;
  }
  if (progp == &fake_prog) {
    return 2;
  }

  if (!progp->line_info) {
    return 4;
  }

  offset = p - progp->program;
  DEBUG_CHECK2(offset > progp->program_size, "Illegal offset %d in object /%s\n", offset,
               progp->filename);

  lns = progp->line_info;
  while (offset > *lns) {
    offset -= *lns;
    lns += (sizeof(ADDRESS_TYPE) + 1);
    if (offset > progp->program_size || offset < 0) {
      debug_message("Something is wrong when looking for line number, bail out.");
      return 0;
    }
  }

#if !defined(USE_32BIT_ADDRESSES)
  COPY_SHORT(&abs_line, lns + 1);
#else
  COPY4(&abs_line, lns + 1);
#endif

  translate_absolute_line(abs_line, &progp->file_info[2], &file_idx, ret_line);

  *ret_file = progp->strings[file_idx - 1];
  return 0;
}

void get_explicit_line_number_info(char *p, const program_t *progp, const char **ret_file,
                                   int *ret_line) {
  int i = find_line(p, progp, ret_file, ret_line);

  switch (i) {
    case 1:
      *ret_file = "(no program)";
      break;
    case 2:
      *ret_file = "(fake)";
      break;
    case 3:
      *ret_file = "(compiled program)";
      break;
    case 4:
      *ret_file = "(no line numbers)";
      break;
    case 5:
      *ret_file = "(includes too deep)";
  }
  if (!(*ret_file)) {
    *ret_file = progp->filename;
  }
}

void get_line_number_info(const char **ret_file, int *ret_line) {
  get_explicit_line_number_info(pc, current_prog, ret_file, ret_line);
}

char *get_line_number(char *p, const program_t *progp) {
  static char buf[256];
  const char *file;
  int line;

  get_explicit_line_number_info(p, progp, &file, &line);

  sprintf(buf, "/%s:%d", file, line);
  return buf;
}

char *get_line_number_if_any() {
  if (current_prog) {
    return get_line_number(pc, current_prog);
  }
  return nullptr;
}

#define SSCANF_ASSIGN_SVALUE_STRING(S) \
  arg->type = T_STRING;                \
  arg->u.string = S;                   \
  arg->subtype = STRING_MALLOC;        \
  arg--;                               \
  num_arg--

#define SSCANF_ASSIGN_SVALUE_NUMBER(N) \
  arg->type = T_NUMBER;                \
  arg->subtype = 0;                    \
  arg->u.number = N;                   \
  arg--;                               \
  num_arg--

#define SSCANF_ASSIGN_SVALUE(T, U, V) \
  arg->type = T;                      \
  arg->U = V;                         \
  arg--;                              \
  num_arg--

/* arg points to the same place it used to */
int inter_sscanf(svalue_t *arg, svalue_t *s0, svalue_t *s1, int num_arg) {
  const char *fmt;       /* Format description */
  const char *in_string; /* The string to be parsed. */
  int number_of_matches;
  int skipme; /* Encountered a '*' ? */
  int base = 10;
  LPC_INT num;
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
          if (*in_string++ != '%') {
            return number_of_matches;
          }
          fmt++;
          continue;
        }
        if (!*fmt) {
          error("Format string cannot end in '%%' in sscanf()\n");
        }
        break;
      }
      if (*fmt++ != *in_string++) {
        return number_of_matches;
      }
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

    if ((skipme = (*fmt == '*'))) {
      fmt++;
    } else if (num_arg < 1 && *fmt != '%') {
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
      case 'd': {
        LPC_INT tmp_num;

        tmp = in_string;
        tmp_num = strtoll(const_cast<char *>(in_string), const_cast<char **>(&in_string), base);
        if (tmp == in_string) {
          return number_of_matches;
        }
        if (!skipme) {
          SSCANF_ASSIGN_SVALUE_NUMBER(tmp_num);
        }
        base = 10;
        continue;
      }
      case 'f': {
        LPC_FLOAT tmp_num;

        tmp = in_string;
        tmp_num = strtod(const_cast<char *>(in_string), const_cast<char **>(&in_string));
        if (tmp == in_string) {
          return number_of_matches;
        }
        if (!skipme) {
          SSCANF_ASSIGN_SVALUE(T_REAL, u.real, tmp_num);
        }
        continue;
      }
      case '(': {
        struct regexp *reg;

        tmp = fmt; /* 1 after the ( */
        num = 1;
        while (true) {
          switch (*tmp) {
            case '\\':
              if (*++tmp) {
                tmp++;
                continue;
              }
              // fall through
            case '\0':
              error("Bad regexp format: '%%%s' in sscanf format string\n", fmt);
            case '(':
              num++;
            /* FALLTHROUGH */
            default:
              tmp++;
              continue;
            case ')':
              if (!--num) {
                break;
              }
              tmp++;
              continue;
          }
          {
            int n = tmp - fmt;
            char *buf = reinterpret_cast<char *>(DMALLOC(n + 1, TAG_TEMPORARY, "sscanf regexp"));
            memcpy(buf, fmt, n);
            buf[n] = 0;
            regexp_user = EFUN_REGEXP;
            reg = regcomp(reinterpret_cast<unsigned char *>(buf), 0);
            FREE(buf);
            if (!reg) {
              error(regexp_error);
            }
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
      if ((skipme2 = (*fmt == '*'))) {
        fmt++;
      }
      if (num_arg < (!skipme + !skipme2) && *fmt != '%') {
        error("Too few arguments to sscanf().\n");
      }

      number_of_matches++;

      switch (*fmt++) {
        case 's':
          error(
              "Illegal to have 2 adjacent %%s's in format string in "
              "sscanf()\n");
        case 'x':
          do {
            while (*tmp && (*tmp != '0')) {
              tmp++;
            }
            if (*tmp == '0') {
              if ((tmp[1] == 'x' || tmp[1] == 'X') && uisxdigit(tmp[2])) {
                break;
              }
              tmp += 2;
            }
          } while (*tmp);
          break;
        case 'd':
          while (*tmp && !uisdigit(*tmp)) {
            tmp++;
          }
          break;
        case 'f':
          while (*tmp && !uisdigit(*tmp) && (*tmp != '.' || !uisdigit(tmp[1]))) {
            tmp++;
          }
          break;
        case '%':
          while (*tmp && (*tmp != '%')) {
            tmp++;
          }
          break;
        case '(': {
          struct regexp *reg;

          tmp = fmt;
          num = 1;
          while (true) {
            switch (*tmp) {
              case '\\':
                if (*++tmp) {
                  tmp++;
                  continue;
                }
                // fall through
              case '\0':
                error("Bad regexp format : '%%%s' in sscanf format string\n", fmt);
              case '(':
                num++;
              /* FALLTHROUGH */
              default:
                tmp++;
                continue;

              case ')':
                if (!--num) {
                  break;
                }
                tmp++;
                continue;
            }
            {
              int n = tmp - fmt;
              char *buf = reinterpret_cast<char *>(DMALLOC(n + 1, TAG_TEMPORARY, "sscanf regexp"));
              memcpy(buf, fmt, n);
              buf[n] = 0;
              regexp_user = EFUN_REGEXP;
              reg = regcomp(reinterpret_cast<unsigned char *>(buf), 0);
              FREE(buf);
              if (!reg) {
                error(regexp_error);
              }
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
      if (!*(in_string = tmp)) {
        return number_of_matches;
      }
      switch (fmt[-1]) {
        case 'x':
          base = 16;
          // fall through
        case 'd': {
          num = strtoll(const_cast<char *>(in_string), const_cast<char **>(&in_string), base);
          /* We already knew it would be matched - Sym */
          if (!skipme2) {
            SSCANF_ASSIGN_SVALUE_NUMBER(num);
          }
          base = 10;
          continue;
        }
        case 'f': {
          LPC_FLOAT tmp_num =
              strtod(const_cast<char *>(in_string), const_cast<char **>(&in_string));
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
    if ((tmp = strchr(fmt, '%')) != nullptr) {
      num = tmp - fmt + 1;
    } else {
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
    if (fmt == tmp) { /* If match, then do continue. */
      continue;
    }

    /*
     * No match was found. Then we stop here, and return the result so
     * far !
     */
    break;
  }
  return number_of_matches;
}

/*
 * Reset the virtual stack machine.
 */
void reset_machine(int first) {
  csp = control_stack - 1;
  if (first) {
    sp = start_of_stack - 1;
  } else {
    pop_n_elems(sp - start_of_stack + 1);
#ifdef DEBUG
    stack_in_use_as_temporary = 0;
#endif
  }
}

static const char *get_arg(int a, int b) {
  static char buff[50];
  char *from, *to;

  from = previous_pc[a];
  to = previous_pc[b];
  if (to - from < 2) {
    return "";
  }
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
    LPC_INT arg;

    COPY_INT(&arg, from + 1);
    sprintf(buff, "%" LPC_INT_FMTSTR_P, arg);
    return buff;
  }
  return "";
}

int last_instructions() {
  int i;

  debug_message("Recent instruction trace:\n");
  i = last;
  do {
    if (previous_instruction[i] != 0)
      debug_message("%p: %3d %8s %-25s (%d)\n", previous_pc[i], previous_instruction[i],
                    get_arg(i, (i + 1) % (sizeof previous_instruction / sizeof(int))),
                    query_instr_name(previous_instruction[i]), stack_size[i] + 1);
    i = (i + 1) % (sizeof previous_instruction / sizeof(int));
  } while (i != last);
  return last;
}

/*
 * When an object is destructed, all references to it must be removed
 * from the stack.
 */
void remove_object_from_stack(object_t *ob) {
  svalue_t *svp;

  for (svp = start_of_stack; svp <= sp; svp++) {
    if (svp->type != T_OBJECT) {
      continue;
    }
    if (svp->u.ob != ob) {
      continue;
    }
    free_object(&svp->u.ob, "remove_object_from_stack");
    svp->type = T_NUMBER;
    svp->u.number = 0;
  }
}

int strpref(const char *p, const char *s) {
  while (*p) {
    if (*p++ != *s++) {
      return 0;
    }
  }
  return 1;
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_stack() {
  svalue_t *sv;

  for (sv = start_of_stack; sv <= sp; sv++) {
    mark_svalue(sv);
  }
}
#endif

void save_context(error_context_t *econ) {
  econ->save_sp = sp;
  econ->save_csp = csp;
  econ->save_cgsp = cgsp;
  econ->save_context = current_error_context;

  current_error_context = econ;
}

void pop_context(error_context_t *econ) { current_error_context = econ->save_context; }

/* can the error handler do this ? */
void restore_context(error_context_t *econ) {
  ref_t *refp;
#ifdef PACKAGE_DWLIB
  extern int _in_reference_allowed;
  _in_reference_allowed = 0;
#endif
  /* unwind the command_giver stack to the saved position */

  while (csp > econ->save_csp) {
    pop_control_stack();
  }

  while (cgsp != econ->save_cgsp) {
    restore_command_giver();
  }
  DEBUG_CHECK(csp < econ->save_csp, "csp is below econ->csp before unwinding.\n");

  pop_n_elems(sp - econ->save_sp);
  refp = global_ref_list;
  while (refp) {
    if (refp->csp >= csp) {
      ref_t *ref = refp;
      refp = refp->next;
      kill_ref(ref);
    } else {
      refp = refp->next;
    }
  }
}
