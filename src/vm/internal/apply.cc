#include "base/std.h"

#include "vm/internal/apply.h"

#include <algorithm>  // for std::min
#include <cstdio>     // for sprintf

#include "base/internal/tracing.h"
#include "vm/internal/base/apply_cache.h"
#include "vm/internal/base/machine.h"
#include "compiler/internal/compiler.h"
#include "compiler/internal/lex.h"

// global static result
svalue_t apply_ret_value;

int convert_type(int /*type*/);

int convert_type(int type) {
  switch (type & (~DECL_MODS)) {
    case TYPE_UNKNOWN:
    case TYPE_NOVALUE:
    case TYPE_VOID:
      return T_INVALID;
    case TYPE_ANY:
      return T_ANY;
    case TYPE_NUMBER:
      return T_NUMBER;
    case TYPE_STRING:
      return T_STRING;
    case TYPE_OBJECT:
      return T_OBJECT;
    case TYPE_MAPPING:
      return T_MAPPING;
    case TYPE_FUNCTION:
      return T_FUNCTION;
    case TYPE_REAL:
      return T_REAL;
    case TYPE_BUFFER:
      return T_BUFFER;
    default:
      if (type & TYPE_MOD_ARRAY) {
        return T_ARRAY;
      } else if (type & TYPE_MOD_CLASS) {
        return T_CLASS;
      } else {
        return T_ANY;  // we probably forgot one then, should we get here
      }
  }
}

// TODO: These should be moved somewhere else
void check_co_args2(unsigned short *types, int num_arg, const char *name, const char *ob_name,
                    int sparg) {
  int argc = sparg;
  int exptype, i = 0;
  do {
    argc--;
    if ((types[i] & DECL_MODS) == LOCAL_MOD_REF) {
      exptype = T_REF;
    } else {
      exptype = convert_type(types[i]);
    }
    i++;
    if (exptype == T_ANY) {
      continue;
    }

    if ((sp - argc)->type != exptype) {
      char buf[1024];
      if ((sp - argc)->type == T_NUMBER && !(sp - argc)->u.number) {
        continue;
      }
      sprintf(buf, "Bad argument %d in call to %s() in %s\nExpected: %s Got %s.\n", i, name,
              ob_name, type_name(exptype), type_name((sp - argc)->type));
      if (CONFIG_INT(__RC_CALL_OTHER_WARN__)) {
        if (current_prog) {
          const char *file;
          int line;
          get_line_number_info(&file, &line);
          int prsave = pragmas;
          pragmas &= ~PRAGMA_ERROR_CONTEXT;
          smart_log(file, line, buf, 1);
          pragmas = prsave;
        } else {
          smart_log("driver", 0, buf, 1);
        }
      } else {
        error(buf);
      }
    }
  } while (i < num_arg);
}

// util functions
void check_co_args(int num_arg, const program_t *prog, function_t *fun, int findex) {
  if (CONFIG_INT(__RC_CALL_OTHER_TYPE_CHECK__)) {
    if (num_arg != fun->num_arg) {
      char buf[1024];
      // if(!current_prog) what do i need this for again?
      // current_prog = master_ob->prog;
      sprintf(buf, "Wrong number of arguments to %s in %s.\n", fun->funcname, prog->filename);
      if (CONFIG_INT(__RC_CALL_OTHER_WARN__)) {
        if (current_prog) {
          const char *file;
          int line;
          int prsave = pragmas;
          pragmas &= ~PRAGMA_ERROR_CONTEXT;
          get_line_number_info(&file, &line);
          smart_log(file, line, buf, 1);
          pragmas = prsave;
        } else {
          smart_log("driver", 0, buf, 1);
        }
      } else {
        error(buf);
      }
    }
    int num_arg_check = std::min((unsigned char)num_arg, fun->num_arg);
    if (num_arg_check && prog->type_start && prog->type_start[findex] != INDEX_START_NONE)
      check_co_args2(&prog->argument_types[prog->type_start[findex]], num_arg, fun->funcname,
                     prog->filename, num_arg);
  }
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

int apply_low(const char *fun, object_t *ob, int num_arg) {
  ScopedTracer _tracer(__PRETTY_FUNCTION__);

  int local_call_origin = call_origin;

#ifdef DEBUG
  control_stack_t *save_csp;
#endif

  if (!local_call_origin) {
    local_call_origin = ORIGIN_DRIVER;
  }
  call_origin = 0;
  ob->time_of_ref = g_current_gametick; /* Used by the swapper */
                                        /*
                                         * This object will now be used, and is thus a target for reset later on
                                         * (when time due).
                                         */
  if (!CONFIG_INT(__RC_NO_RESETS__) && CONFIG_INT(__RC_LAZY_RESETS__)) {
    try_reset(ob);
  }
  if (ob->flags & O_DESTRUCTED) {
    pop_n_elems(num_arg);
    return 0;
  }
  ob->flags &= ~O_RESET_STATE;
#ifndef NO_SHADOWS
  /*
   * If there is a chain of objects shadowing, start with the first of
   * these.
   */
  while (ob->shadowed && ob->shadowed != current_object &&
         (!(ob->shadowed->flags & O_DESTRUCTED))) {
    ob = ob->shadowed;
  }
retry_for_shadow:
#endif
  DEBUG_CHECK(ob->flags & O_DESTRUCTED, "apply() on destructed object\n");

  auto entry = apply_cache_lookup(fun, ob->prog);

#ifndef NO_SHADOWS
  if (!entry.progp && ob->shadowing) {
    /*
     * This is an object shadowing another. The function was not
     * found, but can maybe be found in the object we are shadowing.
     */
    ob = ob->shadowing;
    goto retry_for_shadow;
  }
#endif

  /* This function is not found, return failure. */
  if (!entry.progp) {
    pop_n_elems(num_arg);
    return 0;
  }
  /* Ready to call the function now. */
  {
    int need;
    function_t *funp = entry.funp;
    int findex = (funp - entry.progp->function_table);
    int funflags, runtime_index;

    runtime_index = findex + entry.progp->last_inherited + entry.function_index_offset;
    funflags = ob->prog->function_flags[runtime_index];

    need = (local_call_origin == ORIGIN_DRIVER
                ? DECL_HIDDEN
                : ((current_object == ob || local_call_origin == ORIGIN_INTERNAL) ? DECL_PRIVATE
                                                                                  : DECL_PUBLIC));

    // Check whether caller has sufficient permission.
    if ((funflags & DECL_ACCESS) < need) {
      debug_message(
          "apply() with insufficient permission: \n"
          "cob: %s, ob: %s, function: %s, origin: %s, needs: %s, has: %s \n",
          current_object ? current_object->obname : "null", ob ? ob->obname : "null", fun,
          origin_to_name(local_call_origin), access_to_name(need),
          access_to_name(funflags & DECL_ACCESS));
      pop_n_elems(num_arg);
      return 0;
    }
    /* Check arguments */
    if (!(funflags & FUNC_VARARGS)) {
      check_co_args(num_arg, entry.progp, funp, findex);
    }
    /* Setup new call frame */
    push_control_stack(FRAME_FUNCTION | FRAME_OB_CHANGE);
    current_prog = entry.progp;
    caller_type = local_call_origin;
    csp->num_local_variables = num_arg;
    function_index_offset = entry.function_index_offset;
    variable_index_offset = entry.variable_index_offset;
    csp->fr.table_index = findex;
#ifdef PROFILE_FUNCTIONS
    get_cpu_times(&(csp->entry_secs), &(csp->entry_usecs));
    current_prog->function_table[findex].calls++;
#endif
    /* Setup variables */
    if (funflags & FUNC_TRUE_VARARGS) {
      setup_varargs_variables(csp->num_local_variables, funp->num_local, funp->num_arg);
    } else {
      setup_variables(csp->num_local_variables, funp->num_local, funp->num_arg);
    }
    /* Call the program */
    previous_ob = current_object;
    current_object = ob;
#ifdef DEBUG
    save_csp = csp;
#endif
    call_program(current_prog, funp->address);
    DEBUG_CHECK(save_csp - 1 != csp, "Bad csp after execution in apply_low.\n");
    return 1;
  }
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

svalue_t *apply(const char *fun, object_t *ob, int num_arg, int where) {
#ifdef DEBUG
  svalue_t *expected_sp;
#endif

  call_origin = where;

#ifdef DEBUG
  expected_sp = sp - num_arg;
#endif
  if (apply_low(fun, ob, num_arg) == 0) {
    return nullptr;
  }
  free_svalue(&apply_ret_value, "sapply");
  apply_ret_value = *sp--;
  DEBUG_CHECK(expected_sp != sp, "Corrupt stack pointer.\n");
  return &apply_ret_value;
}

/*
 * this is a "safe" version of apply
 * this allows you to have dangerous driver mudlib dependencies
 * and not have to worry about causing serious bugs when errors occur in the
 * applied function and the driver depends on being able to do something
 * after the apply. (such as the ed exit function, and the net_dead function).
 */
svalue_t *safe_apply(const char *fun, object_t *ob, int num_arg, int where) {
  /* Arguments are already pushed on stack */

  if (ob->flags & O_DESTRUCTED) {
    pop_n_elems(num_arg);
    return nullptr;
  }

  error_context_t econ;
  save_context(&econ);

  svalue_t *ret = nullptr;
  try {
    ret = apply(fun, ob, num_arg, where);
  } catch (const char *) {
    restore_context(&econ);
    pop_n_elems(num_arg);
    ret = nullptr;
  }
  pop_context(&econ);
  return ret;
}
