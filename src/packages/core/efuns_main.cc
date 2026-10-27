/*
 efuns_main.c: this file contains the efunctions called from
 inside eval_instruction() in interpret.c.  Note: if you are adding
 local efunctions that are specific to your driver, you would be better
 off adding them to a separate source file.  Doing so will make it much
 easier for you to upgrade (won't have to patch this file).  Be sure
 to #include "lpc_incl.h" in that separate source file.
 */

#include "base/package_api.h"

#include <algorithm>
#include <set>
#include <utility>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifndef _WIN32
#include <arpa/inet.h>  // for htonl/ntohl
#endif

#include <unistd.h>  // for rmdir(), FIXME
#include "thirdparty/crypt/crypt.h"

#include "packages/core/call_out.h"
#include "packages/core/dns.h"
#include "packages/core/add_action.h"
#include "base/internal/scratchpad.h"
#include "packages/core/file.h"
#include "packages/core/regexp.h"
#include "packages/core/sprintf.h"  // string_print_formatted
#include "packages/core/outbuf.h"
#include "packages/core/reclaim.h"
#include "packages/core/replace_string.h"
#include "packages/core/custom_crypt.h"
#include "packages/core/ed.h"
#include "packages/core/heartbeat.h"

int data_size(object_t* ob);
void reload_object(object_t* obj);

#ifdef F_ALL_INVENTORY
vm::Value f_all_inventory(vm::EfunArgs&, object_t* ob) {
  /* Fresh array (ref 1); transfer without ref++. */
  return vm::Value::refed(all_inventory(ob, 0));
}
#endif

#ifdef F_ALLOCATE
vm::Value f_allocate(vm::EfunArgs&, LPC_INT size, std::optional<vm::Arg> value) {
  if (value) {
    return vm::Value::refed(allocate_array2(size, value->sv()));
  }
  return vm::Value::refed(allocate_array(size));
}
#endif

#ifdef F_ALLOCATE_BUFFER
vm::Value f_allocate_buffer(vm::EfunArgs&, LPC_INT size) {
  buffer_t* buf = allocate_buffer(size);
  if (buf) {
    return vm::Value::refed(buf);
  }
  return 0;
}
#endif

#ifdef F_ALLOCATE_MAPPING
vm::Value f_allocate_mapping(vm::EfunArgs&, vm::Arg size_or_keys, std::optional<vm::Arg> values) {
  if (values) {
    if (!size_or_keys.is(T_ARRAY)) {
      error("Bad argument 1 to allocate_mapping()\n");
    }
    array_t* arr = size_or_keys.array();
    if (values->is(T_ARRAY)) {
      if (values->array()->size != arr->size) {
        error("Arrays passed to allocate_mapping() must be the same size\n");
      }
      return vm::Value::refed(mkmapping(arr, values->array()));
    }
    return vm::Value::refed(allocate_mapping2(arr, values->sv()));
  }
  if (size_or_keys.is(T_NUMBER)) {
    return vm::Value::refed(allocate_mapping(size_or_keys.number()));
  }
  error("Bad argument 1 to allocate_mapping()\n");
}
#endif

#ifdef F_BIND
vm::Value f_bind(vm::EfunArgs& f, funptr_t* old_fp, object_t* ob) {
  funptr_t* new_fp;

  if (ob == old_fp->hdr.owner) {
    /* no change */
    return vm::Value::addref(old_fp);
  }

  if (old_fp->hdr.type == (FP_LOCAL | FP_NOT_BINDABLE)) {
    error("Illegal to rebind a pointer to a local function.\n");
  }
  if (old_fp->hdr.type & FP_NOT_BINDABLE) {
    error(
        "Illegal to rebind a functional that references globals or local "
        "functions.\n");
  }

  {
    vm::Call b;
    b.push(current_object);       /* the object doing the binding */
    b.push(old_fp->hdr.owner);    /* the old owner */
    b.push(ob);                   /* the new owner */
    vm::CallResult res = b.apply_master(APPLY_VALID_BIND);
    if (!master_approved(res, "valid_bind")) {
      error("Master object denied permission to bind() function pointer.\n");
    }
  }

  /* valid_bind() ran arbitrary LPC: if it destructed the new owner, the
   * destruct sweep (remove_object_from_stack) already released the stack's
   * ref and zeroed our argument slot -- storing `ob` into hdr.owner below
   * would create an uncounted reference whose eventual free_object() in
   * dealloc_funp() over-decrements a ref we no longer hold. Check the slot,
   * not ob->flags: ob may already be deallocated memory. */
  if (!f[1].is(T_OBJECT)) {
    error("New owner was destructed during valid_bind().\n");
  }

  new_fp = reinterpret_cast<funptr_t*>(DMALLOC(sizeof(funptr_t), TAG_FUNP, "f_bind"));
  *new_fp = *old_fp;
  new_fp->hdr.ref = 1;
  /* The new pointer takes over the argument slot's reference to its owner. */
  new_fp->hdr.owner = ob;
  f.forget(1);
  new_fp->hdr.owner_gen = ob->prog_generation;
  if (new_fp->hdr.args) {
    new_fp->hdr.args->ref++;
  }
  if ((old_fp->hdr.type & 0x0f) == FP_FUNCTIONAL) {
    new_fp->f.functional.prog->func_ref++;
    debug(d_flag, "add func ref /%s: now %i\n", new_fp->f.functional.prog->filename,
          new_fp->f.functional.prog->func_ref);
  }

  /* old_fp is released along with the (now empty) owner slot. */
  return vm::Value::refed(new_fp);
}
#endif

static int print_cache_stats(outbuffer_t* ob, int verbose) {
  auto size = apply_cache_items * sizeof(lookup_entry_s);
  if (verbose == 1) {
    outbuf_add(ob, "Apply lookup cache information\n");
    outbuf_add(ob, "-------------------------------\n");
    outbuf_addv(ob, "%% cache hits:    %10.2f\n",
                100 * (static_cast<LPC_FLOAT>(apply_cache_hits) / apply_cache_lookups));
    outbuf_addv(ob, "total lookup:     %10lu\n", apply_cache_lookups);
    outbuf_addv(ob, "cache hits:      %10lu\n", apply_cache_hits);
    outbuf_addv(ob, "cache size (bytes w/o overhead):  %10lu\n", size);
  } else if (verbose != -1) {
    outbuf_addv(ob, "%-20s %8lu %8lu\n", "Apply cache", apply_cache_items, size);
  }
  return size;
}

#ifdef F_CACHE_STATS
vm::Value f_cache_stats(vm::EfunArgs&) {
  outbuffer_t ob;

  outbuf_zero(&ob);
  print_cache_stats(&ob, 1);
  return outbuf_value(&ob);
}

#endif

#ifdef F__CALL_OTHER
/* enhanced call_other written 930314 by Luke Mewburn <zak@rmit.edu.au> */
vm::Value f__call_other(vm::EfunArgs& f, vm::Arg target, vm::Arg function) {
  int num_arg = f.argc();
  const char* funcname;
  object_t* ob;

  if (current_object->flags & O_DESTRUCTED) { /* No external calls allowed */
    return vm::Value::undefined();
  }
  if (function.is(T_STRING)) {
    funcname = function.string().c_str();
  } else { /* must be T_ARRAY then */
    array_t* v = function.array();
    svalue_t* sv = nullptr;

    if (!((sv = v->item)->type == T_STRING)) {
      error("call_other: 1st elem of array for arg 2 must be a string\n");
    }
    check_for_destr(v);
    funcname = sv->u.string;
    /* merge_arg_lists() pushes the array's remaining elements as extra
     * arguments, above the window. */
    num_arg = 2 + merge_arg_lists(num_arg - 2, v, 1);
    f.resync();
  }

  if (target.is(T_OBJECT)) {
    ob = target.object();
  } else if (target.is(T_ARRAY)) {
    array_t* ret = call_all_other(target.array(), funcname, num_arg - 2);
    /* call_all_other consumed the trailing args; left [obj_array, name]. */
    f.resync();
    return vm::Value::refed(ret);
  } else {
    ob = find_object(target.string().c_str());
    if (!ob || !object_visible(ob)) {
      error("call_other() couldn't find object '%s'.\n", target.string().c_str());
    }
  }
  /* apply() consumes the num_arg-2 trailing args; left [obj, name]. */
  svalue_t* applied = ::apply(funcname, ob, num_arg - 2, ORIGIN_CALL_OTHER);
  f.resync();
  if (applied == nullptr) {
    return vm::Value::undefined();
  }
  /* applied is the driver's shared return slot: move the result out. */
  return vm::Value::take(applied);
}

#endif

#if defined(F_CALL_STACK) || defined(F_ORIGIN)
static const char* origin_name(int orig) {
  /* FIXME: this should use ffs() if available (BSD) */
  int i = 0;
  static const char* origins[] = {"driver",   "local", "call_other",       "simul",
                                  "internal", "efun",  "function pointer", "functional"};
  while (orig >>= 1) {
    i++;
  }
  return origins[i];
}
#endif

#ifdef F_CALL_STACK
vm::Value f_call_stack(vm::EfunArgs&, LPC_INT which) {
  int i, n = csp - &control_stack[0] + 1;
  array_t* ret;

  if (which < 0 || which > 4) {
    error("First argument of call_stack() must be 0, 1, 2, 3, or 4.\n");
  }

  ret = allocate_array(n);

  /* A frame entered from DRIVER CONTEXT (a bare tick callback, a heart_beat,
   * a promise-reaction delivery) records a null program/object. Historically
   * only control_stack[0] could be such a frame and the loops below start at
   * 1, so the nulls were never dereferenced; a construct that pushes its own
   * frame before entering LPC (the promise drain's error-capture frame) puts
   * one at index >= 1 too. Report those honestly instead of crashing:
   * "<driver>" for a file, 0 for an object -- the array is zero-initialized,
   * so a skipped slot is already 0. */
  switch (which) {
    case 0:
      ret->item[0].type = T_STRING;
      ret->item[0].subtype = STRING_MALLOC;
      ret->item[0].u.string = add_slash(current_prog ? current_prog->filename : "<driver>");
      for (i = 1; i < n; i++) {
        const program_t* prog = (csp - i + 1)->prog;
        ret->item[i].type = T_STRING;
        ret->item[i].subtype = STRING_MALLOC;
        ret->item[i].u.string = add_slash(prog ? prog->filename : "<driver>");
      }
      break;
    case 1:
      if (current_object) {
        ret->item[0].type = T_OBJECT;
        ret->item[0].u.ob = current_object;
        add_ref(current_object, "f_call_stack: curr");
      }
      for (i = 1; i < n; i++) {
        object_t* ob = (csp - i + 1)->ob;
        if (!ob) {
          continue; /* driver context: leave the slot 0 */
        }
        ret->item[i].type = T_OBJECT;
        ret->item[i].u.ob = ob;
        add_ref(ob, "f_call_stack");
      }
      break;
    case 2:
      for (i = 0; i < n; i++) {
        ret->item[i].type = T_STRING;
        const program_t* fprog = (i ? (csp - i + 1)->prog : current_prog);
        if (((csp - i)->framekind & FRAME_MASK) == FRAME_FUNCTION && fprog != nullptr) {
          int const index = (csp - i)->fr.table_index;
          function_t* cfp = &fprog->function_table[index];

          ret->item[i].subtype = STRING_SHARED;
          ret->item[i].u.string = cfp->funcname;
          ref_string(cfp->funcname);
        } else {
          ret->item[i].subtype = STRING_CONSTANT;
          ret->item[i].u.string =
              (((csp - i)->framekind & FRAME_MASK) == FRAME_CATCH) ? "CATCH" : "<function>";
        }
      }
      break;
    case 3:
      ret->item[0].type = T_STRING;
      ret->item[0].subtype = STRING_CONSTANT;
      ret->item[0].u.string = origin_name(caller_type);

      for (i = 1; i < n; i++) {
        ret->item[i].type = T_STRING;
        ret->item[i].subtype = STRING_CONSTANT;
        ret->item[i].u.string = origin_name((csp - i + 1)->caller_type);
      }
      break;
    case 4:
      /* Every frame reports "file:line", matching the documented behavior. */
      for (i = 0; i < n; i++) {
        const program_t* prog = (i ? (csp - i + 1)->prog : current_prog);
        char* progc = (i ? (csp - i + 1)->pc : pc);
        ret->item[i].type = T_STRING;
        ret->item[i].subtype = STRING_MALLOC;
        /* get_line_number() needs a real program; a driver-context frame
         * has none (see the case-0 comment) */
        ret->item[i].u.string =
            prog ? string_copy(get_line_number(progc, prog), "call_stack") : string_copy("", "call_stack");
      }
      break;
  }

  return vm::Value::refed(ret);
}

#endif

#ifdef F_CAPITALIZE
vm::Value f_capitalize(vm::EfunArgs& f, vm::StringArg str) {
  if (uislower(str.c_str()[0])) {
    vm::Value s = f.take(0);
    char* p = s.unlink_string();
    p[0] = toupper(static_cast<unsigned char>(p[0]));
    return s;
  }
  return f.take(0);
}
#endif

#ifdef F_CHILDREN
vm::Value f_children(vm::EfunArgs&, vm::StringArg name) {
  auto max_array_size = CONFIG_INT(__MAX_ARRAY_SIZE__);
  auto v = ObjectTable::instance().children(name.c_str());
  auto len = v.size() < max_array_size ? v.size() : max_array_size;
  auto* res = allocate_empty_array(len);

  for (auto i = 0; i < v.size() && i < len; ++i) {
    res->item[i].u.ob = v[i];
    res->item[i].type = T_OBJECT;
    add_ref(v[i], "children");
  }
  return vm::Value::refed(res);
}

#endif

#ifdef F_CLASSP
vm::Value f_classp(vm::EfunArgs&, vm::Arg x) { return x.is(T_CLASS); }

#endif

#ifdef F_CLEAR_BIT
vm::Value f_clear_bit(vm::EfunArgs& f, vm::StringArg bits, LPC_INT requested) {
  char* str;
  int len, ind, bit;

  auto max_bitfield_bits = CONFIG_INT(__MAX_BITFIELD_BITS__);
  if (requested > max_bitfield_bits) {
    error("clear_bit() bit requested : %" LPC_INT_FMTSTR_P " > maximum bits: %d\n", requested,
          max_bitfield_bits);
  }
  /* check the sign on the LPC int, before narrowing it (as set_bit does) */
  if (requested < 0) {
    error("Bad argument 2 (negative) to clear_bit().\n");
  }
  bit = requested;
  ind = bit / 6;
  bit %= 6;
  len = bits.size();
  if (ind >= len) {
    /* return first arg unmodified */
    return f.take(0);
  }
  /* Validate before unlink: a shared/constant string would otherwise be
   * copied only to throw on a bad bit pattern. */
  if (bits.c_str()[ind] > 0x3f + ' ' || bits.c_str()[ind] < ' ') {
    error("Illegal bit pattern in clear_bit character %d\n", ind);
  }
  vm::Value result = f.take(0);
  str = result.unlink_string();
  str[ind] = ((str[ind] - ' ') & ~(1 << bit)) + ' ';
  return result;
}

#endif

#ifdef F_CLONEP
vm::Value f_clonep(vm::EfunArgs&, vm::Arg x) {
  return x.is(T_OBJECT) && (x.object()->flags & O_CLONE);
}

#endif

#ifdef F__NEW
vm::Value f__new(vm::EfunArgs& f, vm::StringArg name) {
  object_t* ob = clone_object(name.c_str(), f.argc() - 1);
  /* clone_object() / call_create() consumed the trailing create args;
   * only the filename remains. */
  f.resync();
  if (ob) {
    return vm::Value::object(ob); /* adds a ref; the clone starts at ref 1 */
  }
  return 0;
}

#endif

#ifdef F_DEEP_INHERIT_LIST
vm::Value f_deep_inherit_list(vm::EfunArgs&, object_t* ob) {
  return vm::Value::refed(deep_inherit_list(ob));
}

#endif

#ifdef F_SET_DEBUG_LEVEL
void f_set_debug_level(vm::EfunArgs&, vm::Arg level) {
  if (level.is(T_STRING)) {
    debug_level_set(level.string().c_str());
  } else {
    debug_level = level.number();
  }
}

void f_clear_debug_level(vm::EfunArgs&, vm::StringArg level) { debug_level_clear(level.c_str()); }

/* not in debug.h since debug.h is included in many places that don't
 know about mapping_t */
mapping_t* debug_levels();

vm::Value f_debug_levels(vm::EfunArgs&) { return vm::Value::refed(debug_levels()); }

#endif

#ifdef F_DEEP_INVENTORY
vm::Value f_deep_inventory(vm::EfunArgs&, vm::Arg target, std::optional<funptr_t*> filter) {
  array_t* vec;
  if (filter && (target.is(T_ARRAY) || target.is(T_OBJECT))) {
    if (target.is(T_ARRAY)) {
      vec = deep_inventory_array(target.array(), 1, *filter);
    } else {
      vec = deep_inventory(target.object(), 0, *filter);
    }
  } else if (!filter) {
    if (target.is(T_FUNCTION)) {
      vec = deep_inventory(current_object, 0, target.funptr());
    } else if (target.is(T_ARRAY)) {
      vec = deep_inventory_array(target.array(), 1, nullptr);
    } else {
      vec = deep_inventory(target.object(), 0, nullptr);
    }
  } else {
    vec = &the_null_array;
  }

  return vm::Value::refed(vec);
}

#endif

#ifdef F_DESTRUCT
void f_destruct(vm::EfunArgs&, object_t* ob) {
  /* remove_object_from_stack() frees the object and leaves a number 0 in the
   * arg slot, which the void efun's return then pops. */
  destruct_object(ob);
}

#endif

#ifdef F_ENABLE_WIZARD
void f_enable_wizard(vm::EfunArgs&) {
  if (current_object->interactive) {
    current_object->flags |= O_IS_WIZARD;
  }
}

#endif

#ifdef F_ERROR
void f_error(vm::EfunArgs&, vm::StringArg msg) {
  int l = msg.size();
  if (l && msg.c_str()[l - 1] == '\n') {
    l--;
  }
  const auto max_string_length = LARGEST_PRINTABLE_STRING;
  l = std::min(l, max_string_length - 3);

  // error_handler() copies the message before it throws, and the unwind
  // frees this buffer.
  std::string err_buf(l + 3, '\0');

  err_buf[0] = '*';
  u8_strncpy(reinterpret_cast<uint8_t*>(&err_buf[1]), reinterpret_cast<const uint8_t*>(msg.c_str()),
             l);
  err_buf[l + 1] = '\n';
  err_buf[l + 2] = 0;

  error_handler(&err_buf[0]);
}

#endif

#ifdef F_DISABLE_WIZARD
void f_disable_wizard(vm::EfunArgs&) {
  if (current_object->interactive) {
    current_object->flags &= ~O_IS_WIZARD;
  }
}

#endif

#ifdef F_ENVIRONMENT
vm::Value f_environment(vm::EfunArgs&, std::optional<object_t*> target) {
  object_t* ob = nullptr;

  if (target) {
    if ((ob = *target)->flags & O_DESTRUCTED) {
      error("environment() of destructed object.\n");
    }
    ob = ob->super;
  } else {
    ob = current_object->super;
  }

  if (ob && object_visible(ob)) {
    return vm::Value::object(ob);
  }
  return 0;
}

#endif

#ifdef F_FILE_NAME
vm::Value f_file_name(vm::EfunArgs&, object_t* ob) {
  /* This function now returns a leading '/' */
  return vm::Value::str_malloced(add_slash(ob->obname));
}

#endif

#ifdef F_FILTER
vm::Value f_filter(vm::EfunArgs& f, vm::Arg source, vm::Arg) {
  /* The helpers leave the result in the first argument's slot and pop the
   * rest. */
  if (source.is(T_MAPPING)) {
    filter_mapping(source.sv(), f.argc());
  } else if (source.is(T_STRING)) {
    filter_string(source.sv(), f.argc());
  } else {
    filter_array(source.sv(), f.argc());
  }
  f.resync();
  return f.take(0);
}

#endif

#ifdef F_FIND_CALL_OUT
vm::Value f_find_call_out(vm::EfunArgs&, vm::Arg which) {
  int i;
  if (which.is(T_NUMBER)) {
    i = find_call_out_by_handle(current_object, which.number());
  } else { /* T_STRING */
    i = find_call_out(current_object, which.string().c_str());
  }
  return i;
}

#endif

#ifdef F_FIND_OBJECT
vm::Value f_find_object(vm::EfunArgs&, vm::StringArg name, LPC_INT load) {
  object_t* ob;

  if (load) {
    ob = find_object(name.c_str());
  } else {
    ob = find_object2(name.c_str());
  }
  if (ob && object_visible(ob)) {
    return vm::Value::object(ob);
  }
  return 0;
}

#endif

#ifdef F_FUNCTION_PROFILE
/* f_function_profile: John Garnett, 1993/05/31, 0.9.17.3 */
vm::Value f_function_profile(vm::EfunArgs&, object_t* ob) {
  array_t* vec;
  mapping_t* map;
  program_t* prog;
  int nf, j;

  prog = ob->prog;
  nf = prog->num_functions_defined;
  vec = allocate_empty_array(nf);
  for (j = 0; j < nf; j++) {
    map = allocate_mapping(3);
    add_mapping_pair(map, "calls", prog->function_table[j].calls);
    add_mapping_pair(map, "self", prog->function_table[j].self - prog->function_table[j].children);
    add_mapping_pair(map, "children", prog->function_table[j].children);
    add_mapping_shared_string(map, "name", prog->function_table[j].funcname);
    vec->item[j].type = T_MAPPING;
    vec->item[j].u.map = map;
    prog->function_table[j].calls = 0;
    prog->function_table[j].self = 0;
    prog->function_table[j].children = 0;
  }
  return vm::Value::refed(vec);
}

#endif

#ifdef F_FUNCTION_EXISTS
vm::Value f_function_exists(vm::EfunArgs&, vm::StringArg name, std::optional<object_t*> target,
                            std::optional<LPC_INT> flag_arg) {
  const char* str;
  char* res;
  int l;
  object_t* ob;
  int flag = 0;

  if (target) {
    if (flag_arg) {
      flag = *flag_arg;
    }
    ob = *target;
  } else {
    if (current_object->flags & O_DESTRUCTED) {
      return 0;
    }
    ob = current_object;
  }

  str = function_exists(name.c_str(), ob, flag);
  if (str) {
    l = SHARED_STRLEN(str); /* strip the source extension (.lpc or .c) */
    if (l > 4 && strcmp(str + l - 4, ".lpc") == 0) {
      l -= 4;
    } else if (l > 2 && strcmp(str + l - 2, ".c") == 0) {
      l -= 2;
    }
    res = new_string(l + 1, "function_exists");
    res[0] = '/';
    strncpy(res + 1, str, l);
    res[l + 1] = 0;
    return vm::Value::str_malloced(res);
  }
  return 0;
}

#endif

#ifdef F_GET_CHAR
vm::Value f_get_char(vm::EfunArgs& f, vm::Arg fun) {
  int num_arg = f.argc();
  int i, tmp;
  int flag;

  if (num_arg == 1 || !f[1].is(T_NUMBER)) {
    tmp = 0;
    flag = 0;
  } else {
    tmp = 1;
    num_arg--; /* Don't count the flag as an arg */
    flag = f[1].number();
  }
  num_arg--;
  i = get_char(fun.sv(), flag, num_arg, num_arg ? f[1 + tmp].sv() : nullptr);
  if (i) {
    /* get_char() moved the extra arguments into the pending input */
    for (int k = 1 + tmp; k < f.argc(); k++) {
      f.forget(k);
    }
  }
  return i;
}

#endif

#ifdef F_IN_INPUT
vm::Value f_in_input(vm::EfunArgs&, object_t* ob) {
  int i = ob->interactive && ob->interactive->input_to;
  return i != 0;
}

#endif

#ifdef F_INHERITS
int inherits(program_t* prog, program_t* thep) {
  int j, k = prog->num_inherited, l;
  program_t* pg;

  for (j = 0; j < k; j++) {
    if ((pg = prog->inherit[j].prog) == thep) {
      return 1;
    }
    if (!strcmp(pg->filename, thep->filename)) {
      return 2;
    }
    if ((l = inherits(pg, thep))) {
      return l;
    }
  }
  return 0;
}

vm::Value f_inherits(vm::EfunArgs&, vm::StringArg name, object_t* base) {
  object_t* ob = find_object2(name.c_str());
  if (!ob) {
    return 0;
  }
  return inherits(base->prog, ob->prog);
}

#endif

#ifdef F_SHALLOW_INHERIT_LIST
vm::Value f_shallow_inherit_list(vm::EfunArgs&, object_t* ob) {
  return vm::Value::refed(inherit_list(ob));
}

#endif

#ifdef F_INCLUDE_LIST
vm::Value f_include_list(vm::EfunArgs&, object_t* ob) { return vm::Value::refed(include_list(ob)); }

#endif

#ifdef F_INPUT_TO
vm::Value f_input_to(vm::EfunArgs& f, vm::Arg fun) {
  int num_arg = f.argc();
  int i, tmp;
  int flag;

  if ((num_arg == 1) || !f[1].is(T_NUMBER)) {
    tmp = flag = 0;
  } else {
    tmp = 1;
    num_arg--; /* Don't count the flag as an arg */
    flag = f[1].number();
  }
  num_arg--; /* Don't count the name of the func either. */
  i = input_to(fun.sv(), flag, num_arg, num_arg ? f[1 + tmp].sv() : nullptr);
  if (i) {
    /* input_to() moved the extra arguments into the pending input */
    for (int k = 1 + tmp; k < f.argc(); k++) {
      f.forget(k);
    }
  }
  return i;
}

#endif

#ifdef F_INTERACTIVE
vm::Value f_interactive(vm::EfunArgs&, object_t* ob) {
  int i = (ob->interactive != nullptr);
  return i;
}

#endif

#ifdef F_INTP
vm::Value f_intp(vm::EfunArgs&, vm::Arg x) { return x.is(T_NUMBER); }

#endif

#ifdef F_FUNCTIONP
vm::Value f_functionp(vm::EfunArgs&, vm::Arg x) {
  int i;

  if (x.is(T_FUNCTION)) {
    funptr_t* fp = x.funptr();
    i = fp->hdr.type;
    if (fp->hdr.args) {
      i |= FP_HAS_ARGUMENTS;
    }
    if (!fp->hdr.owner || (fp->hdr.owner->flags & O_DESTRUCTED)) {
      i |= FP_OWNER_DESTED;
    }
    return i;
  }
  return 0;
}

#endif

#ifdef F_KEYS
vm::Value f_keys(vm::EfunArgs&, mapping_t* m) { return vm::Value::refed(mapping_indices(m)); }

#endif

#ifdef F_VALUES
vm::Value f_values(vm::EfunArgs&, mapping_t* m) { return vm::Value::refed(mapping_values(m)); }

#endif

#ifdef F_LOWER_CASE
vm::Value f_lower_case(vm::EfunArgs& f, vm::StringArg arg) {
  const char* scan = arg.c_str();

  /* find first upper case letter, if any */
  for (; *scan; scan++) {
    if (uisupper(*scan)) {
      int const l = scan - arg.c_str();
      vm::Value s = f.take(0);
      char* str = s.unlink_string() + l;
      *str = tolower(static_cast<unsigned char>(*str));
      for (str++; *str; str++) {
        if (uisupper(*str)) {
          *str = tolower(static_cast<unsigned char>(*str));
        }
      }
      return s;
    }
  }
  return f.take(0);
}

#endif

#ifdef F_MALLOC_STATUS
vm::Value f_malloc_status(vm::EfunArgs&) {
  outbuffer_t ob;

  outbuf_zero(&ob);
  outbuf_add(&ob, "Using system malloc");
#ifdef DEBUGMALLOC
  outbuf_add(&ob, ", wrapped with debugmalloc");
#endif
  outbuf_add(&ob, ".\n");
#if defined(DEBUGMALLOC)
  dump_malloc_data(&ob);
#endif
  return outbuf_value(&ob);
}

#endif

#ifdef F_MAP_DELETE
void f_map_delete(vm::EfunArgs&, mapping_t* m, vm::Arg key) { mapping_delete(m, key.sv()); }

#endif

#ifdef F_MAPP
vm::Value f_mapp(vm::EfunArgs&, vm::Arg x) { return x.is(T_MAPPING); }

#endif

#ifdef F_MAP
vm::Value f_map(vm::EfunArgs& f, vm::Arg source, vm::Arg) {
  /* The helpers leave the result in the first argument's slot and pop the
   * rest. */
  if (source.is(T_MAPPING)) {
    map_mapping(source.sv(), f.argc());
  } else if (source.is(T_ARRAY)) {
    map_array(source.sv(), f.argc());
  } else {
    map_string(source.sv(), f.argc());
  }
  f.resync();
  return f.take(0);
}

#endif

#ifdef F_MASTER
vm::Value f_master(vm::EfunArgs&) {
  if (!master_ob) {
    return 0;
  }
  return vm::Value::object(master_ob);
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
vm::Value f_match_path(vm::EfunArgs&, mapping_t* map, vm::StringArg path) {
  svalue_t* value;
  const char* src;
  char* dst;
  svalue_t* nvalue;
  char* tmpstr;

  value = &const0u;

  tmpstr = reinterpret_cast<char*>(DMALLOC(path.size() + 1, TAG_STRING, "match_path"));

  src = path.c_str();
  dst = tmpstr;

  while (*src != '\0') {
    while (*src != '/' && *src != '\0') {
      *dst++ = *src++;
    }
    if (*src == '/') {
      while (*++src == '/') {
        ;
      }
      if (*src != '\0' || dst == tmpstr) {
        *dst++ = '/';
      }
    }
    *dst = '\0';
    nvalue = find_string_in_mapping(map, tmpstr);

    if (nvalue != &const0u) {
      value = nvalue;
    }
  }

  FREE(tmpstr);
  /* Don't free mapping first, in case sometimes one uses a ref 1 mapping */
  /* Randor - 5/29/94 */
  return vm::Value::copy(value);
}

#endif /* F_MATCH_PATH */

#ifdef F_MEMBER_ARRAY
/* member_array(int, string) is a BYTE search (strchr): the start offset and
 * the result are byte offsets and only one byte can match. The answer is a
 * character answer exactly when:
 *   - the needle is ASCII and not found: an ASCII byte never occurs inside a
 *     multi-byte character, and starting at byte k never covers less text
 *     than starting at character k, so -1 is right;
 *   - the needle is ASCII, found at byte p, and character p is exactly that
 *     one byte -- no non-ASCII text or CR LF (one character) before it, and
 *     not the start of a combining sequence nor the LF of a CR LF;
 *   - the needle is above 0x7F and the text has no byte >= 0x80 (so no such
 *     character can be there, and -1 is right).
 * Otherwise the caller gets one warning per call site. The decision only runs
 * for a site that has not warned yet, so a warned site costs one set lookup. */
static bool has_high_byte(const char* s, size_t n) {
  for (size_t k = 0; k < n; k++) {
    if (static_cast<unsigned char>(s[k]) >= 0x80) {
      return true;
    }
  }
  return false;
}

static bool member_array_byte_answer_misleads(const char* str, size_t len, LPC_INT c,
                                              LPC_INT result) {
  if (c > 0x7f || c < 0) {
    return has_high_byte(str, len);
  }
  if (result < 0) {
    return false;
  }
  /* The byte answer is the character answer iff character `result` is
   * exactly the one byte at offset `result`: it starts there, and the next
   * character starts one byte later. That catches non-ASCII text or a CR LF
   * before the match, a combining sequence starting at it, and the LF of a
   * CR LF. */
  EGCSmartIterator iter(str, len);
  if (!iter.ok()) {
    return true;
  }
  int32_t const at = iter.index_to_offset(result);
  int32_t next = iter.index_to_offset(result + 1);
  if (next == icu::BreakIterator::DONE) {
    next = static_cast<int32_t>(len);
  }
  return at != result || next != result + 1;
}

/* Once per call site. A call through an efun pointer runs in a fake frame
 * (current_prog == &fake_prog), so the site is the caller's, saved in csp. */
static std::set<std::pair<const program_t*, ptrdiff_t>> member_array_warned_sites;

static void member_array_call_site(const program_t** prog, const char** at) {
  *prog = current_prog;
  *at = pc;
  if (current_prog == &fake_prog && csp && csp->prog) {
    *prog = csp->prog;
    *at = csp->pc;
  }
}

static bool member_array_site_warned(const program_t* prog, const char* at) {
  return member_array_warned_sites.count({prog, prog ? at - prog->program : 0}) != 0;
}

static void warn_member_array_byte_search(const program_t* prog, const char* at) {
  static constexpr size_t kMaxWarnedSites = 1024;
  if (member_array_warned_sites.size() >= kMaxWarnedSites) {
    /* forget rather than go silent: reloaded programs keep adding sites */
    member_array_warned_sites.clear();
  }
  member_array_warned_sites.insert({prog, prog ? at - prog->program : 0});
  const char* file = nullptr;
  int line = 0;
  if (prog && prog != &fake_prog) {
    get_explicit_line_number_info(const_cast<char*>(at), prog, &file, &line);
  }
  debug_message(
      "/%s:%d: Warning: member_array(int, string) searches BYTES and returns a byte offset; on "
      "this non-ASCII string that is not a character index. Use strsrch(str, ch) to find a "
      "character (any Unicode code point) by character index.\n",
      file ? file : "<unknown>", line);
}

/* member_array()'s predicate search (issue #900): the index of the first
 * element, from the i'th searched, that makes fp return truthy, or -1.
 * Out of line, and the whole loop rather than one call per element: the
 * vm::Call and its result live in memory, which costs this frame a
 * stack-protector canary and spilled registers -- once per search here,
 * instead of once per element or on every member_array() call. */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
static LPC_INT
member_array_predicate_search(funptr_t* fp, array_t* v, LPC_INT i, bool backwards) {
  int const size = v->size;
  for (; i < size; i++) {
    LPC_INT const index = backwards ? size - i - 1 : i;
    vm::CallResult ret = vm::Call(vm::Value::copy(&v->item[index])).call(fp);
    if (ret && !(ret->type == T_NUMBER && ret->u.number == 0)) {
      return index;
    }
  }
  return -1;
}

vm::Value f_member_array(vm::EfunArgs&, vm::Arg item, vm::Arg haystack,
                         std::optional<LPC_INT> start, std::optional<LPC_INT> flags) {
  array_t* v;
  int flag = 0;
  LPC_INT i;
  int size;

  if (start) {
    if (flags) {
      flag = *flags;
    }

    i = *start;
    if (i < 0) {
      bad_arg(3, F_MEMBER_ARRAY);
    }
  } else {
    i = 0;
  }

  if (haystack.is(T_STRING)) {
    vm::StringArg str = haystack.string();
    const char* res;
    if (flag & 2) {
      error("member_array: can not search backwards in strings");
    }
    CHECK_TYPES(item.sv(), T_NUMBER, 1, F_MEMBER_ARRAY);
    if (i > str.size()) {
      error("Index to start search from in member_array() is > string length.\n");
    }
    /* strchr() searches for a char: a value outside 0..255 would be
     * truncated onto some other byte (0x161 -> 'a', 0x100 -> the NUL
     * terminator), so it can never be in the string. */
    LPC_INT const c = item.number();
    if (c >= 0 && c <= 255 && (res = strchr(str.c_str() + i, static_cast<int>(c)))) {
      i = res - str.c_str();
    } else {
      i = -1;
    }
    if (!SVALUE_STR_ASCII(str.sv())) { /* cached; ASCII and CR-free never warns */
      const program_t* site_prog;
      const char* site_pc;
      member_array_call_site(&site_prog, &site_pc);
      if (!member_array_site_warned(site_prog, site_pc) &&
          member_array_byte_answer_misleads(str.c_str(), str.size(), c, i)) {
        warn_member_array_byte_search(site_prog, site_pc);
      }
    }
    return i;
  } else {
    svalue_t* sv;
    svalue_t* find;
    int flen = 0;

    size = (v = haystack.array())->size;
    find = item.sv();
    /* optimize a bit */
    if (find->type == T_STRING) {
      /* *not* COUNTED_STRLEN() which can do a (costly) strlen() call */
      if (find->subtype & STRING_COUNTED) {
        flen = MSTR_SIZE(find->u.string);
      } else if (flag & 1) {
        flen = strlen(find->u.string);
      } else {
        flen = 0;
      }
    }

    if ((flag & 4) && find->type == T_FUNCTION) {
      // Predicate search (issue #900): return the first index whose
      // element makes the function return truthy.
      return member_array_predicate_search(find->u.fp, v, i, flag & 2);
    }
    for (; i < size; i++) {
      int tmp = i;
      if (flag & 2) {
        tmp = size - tmp - 1;
      }
      sv = v->item + tmp;
      switch (find->type | sv->type) {
        case T_STRING:
          if (flag & 1) {
            if (flen && (sv->subtype & STRING_COUNTED) && flen > MSTR_SIZE(sv->u.string)) {
              continue;
            }
            if (strncmp(find->u.string, sv->u.string, flen) != 0) {
              continue;
            }
          } else {
            if (flen && (sv->subtype & STRING_COUNTED) && flen != MSTR_SIZE(sv->u.string)) {
              continue;
            }
            if (strcmp(find->u.string, sv->u.string) != 0) {
              continue;
            }
          }
          break;
        case T_NUMBER:
          if (find->u.number == sv->u.number) {
            break;
          }
          continue;
        case T_REAL:
          if (find->u.real == sv->u.real) {
            break;
          }
          continue;
        case T_ARRAY:
          if (find->u.arr == sv->u.arr) {
            break;
          }
          continue;
        case T_CLASS:
          if (find->u.arr == sv->u.arr) {
            break;
          }
          continue;
        case T_OBJECT: {
          if (sv->u.ob->flags & O_DESTRUCTED) {
            assign_svalue(sv, &const0u);
            continue;
          }
          if (find->u.ob == sv->u.ob) {
            break;
          }
          continue;
        }
        case T_MAPPING:
          if (find->u.map == sv->u.map) {
            break;
          }
          continue;
        case T_FUNCTION:
          if (find->u.fp == sv->u.fp) {
            break;
          }
          continue;
        case T_BUFFER:
          if (find->u.buf == sv->u.buf) {
            break;
          }
          continue;
        case T_PROMISE:
          if (find->u.prom == sv->u.prom) {
            break;
          }
          continue;
        default:
          if (sv->type == T_OBJECT && (sv->u.ob->flags & O_DESTRUCTED)) {
            assign_svalue(sv, &const0u);
            if (find->type == T_NUMBER && !find->u.number) {
              break;
            }
          }
          continue;
      }
      break;
    }
    if (i >= size) {
      i = -1; /* Return -1 for failure */
    }
  }
  if (i != -1 && (flag & 2)) {
    /* not-found used to be "reversed" too, returning size instead of -1 */
    i = size - i - 1;
  }
  return i;
}

#endif

#ifdef F_MESSAGE
void f_message(vm::EfunArgs&, vm::Arg msg_class, vm::Arg message, vm::Arg target,
               std::optional<vm::Arg> exclude) {
  array_t *use = nullptr, *avoid;

  /* A single target or exclude is wrapped in an array that replaces it in its
   * own argument slot (taking over the slot's reference), so the window's
   * release frees the array. */
  switch (target.type()) {
    case T_OBJECT:
    case T_STRING:
      use = allocate_empty_array(1);
      use->item[0] = *target.sv();
      target.sv()->type = T_ARRAY;
      target.sv()->u.arr = use;
      break;
    case T_ARRAY:
      use = target.array();
      break;
    case T_NUMBER: {
      int const len = SVALUE_STRLEN(message.sv());
      add_message(command_giver, message.sv()->u.string, len);
      return;
    }
    default:
      bad_argument(target.sv(), T_OBJECT | T_STRING | T_ARRAY | T_NUMBER, 3, F_MESSAGE);
  }
  if (exclude) {
    switch (exclude->type()) {
      case T_OBJECT:
        avoid = allocate_empty_array(1);
        avoid->item[0] = *exclude->sv();
        exclude->sv()->type = T_ARRAY;
        exclude->sv()->u.arr = avoid;
        break;
      case T_ARRAY:
        avoid = exclude->array();
        break;
      default:
        avoid = &the_null_array;
    }
  } else {
    avoid = &the_null_array;
  }
  do_message(msg_class.sv(), message.sv(), use, avoid, 1);
}

#endif

#ifdef F_MOVE_OBJECT
void f_move_object(vm::EfunArgs&, vm::Arg dest) {
  object_t *o1, *o2;

  /* get destination */
  if (dest.is(T_OBJECT)) {
    o2 = dest.object();
  } else {
    if (!(o2 = find_object(dest.string().c_str())) || !object_visible(o2)) {
      error("move_object failed: could not find destination\n");
    }
  }

  if ((o1 = current_object)->flags & O_DESTRUCTED) {
    error("move_object(): can't move a destructed object\n");
  }

  move_object(o1, o2);
}

#endif

namespace {
int calculate_and_maybe_print_memory_info(outbuffer_t* ob, int verbose) {
  uint64_t tot = 0;

  tot += print_cache_stats(ob, verbose);
  if (verbose && verbose != -1) outbuf_add(ob, "\n");

  tot += ObjectTable::instance().showStatus(ob, verbose);
  if (verbose && verbose != -1) outbuf_add(ob, "\n");

  tot += heart_beat_status(ob, verbose);
  if (verbose && verbose != -1) outbuf_add(ob, "\n");

  tot += print_call_out_usage(ob, verbose);
  if (verbose && verbose != -1) outbuf_add(ob, "\n");

  if (verbose && verbose != -1) {
    outbuf_add(ob, "Memory statistics\n");
    outbuf_add(ob, "------------------------------\n");
  }
  auto total_sentence_size = tot_alloc_sentence * sizeof(sentence_t);
  if (verbose != -1) {
    outbuf_addv(ob, "%-20s %8" PRIu64 " %8" PRIu64 "\n", "Sentences", tot_alloc_sentence,
                total_sentence_size);
  }
  tot += total_sentence_size;

  if (verbose != -1) {
#ifndef DEBUG
    outbuf_addv(ob, "%-20s %8" PRIu64 " %8" PRIu64 "\n", "Objects", tot_alloc_object,
                tot_alloc_object_size);
#else
    outbuf_addv(ob, "%-20s %8" PRIu64 " %8" PRIu64 " (%" PRIu64 " dangling)\n",
                "Objects:", tot_alloc_object, tot_alloc_object_size, tot_dangling_object);
#endif
  }
  tot += tot_alloc_object_size;

  auto total_users = (uint64_t)users_num(true);
  auto total_users_size = total_users * sizeof(interactive_t);
  if (verbose != -1) {
    outbuf_addv(ob, "%-20s %8" PRIu64 " %8" PRIu64 "\n", "Interactives", total_users,
                total_users_size);
  }
  tot += total_users_size;

  if (verbose != -1) {
    outbuf_addv(ob, "%-20s %8" PRIu64 " %8" PRIu64 "\n", "Prog blocks", total_num_prog_blocks,
                total_prog_block_size);
  }
  tot += total_prog_block_size;

  if (verbose != -1) {
    outbuf_addv(ob, "%-20s %8" PRIu64 " %8" PRIu64 "\n", "Arrays", num_arrays, total_array_size);
  }
  tot += total_array_size;

  if (verbose != -1) {
    outbuf_addv(ob, "%-20s %8" PRIu64 " %8" PRIu64 "\n", "Classes", num_classes, total_class_size);
  }
  tot += total_class_size;

  auto total_mapping_free_nodes = free_node_count();
  auto total_mapping_free_nodes_size = total_mapping_free_nodes * sizeof(mapping_node_t);
  auto total_mapping_nodes_size = total_mapping_nodes * sizeof(mapping_node_t);
  if (verbose != -1) {
    outbuf_addv(ob,
                "%-20s %8" PRIu64 " %8" PRIu64 " (nodes %" PRIu64 ", size %" PRIu64
                ", free %" PRIu64 ", size %" PRIu64 ")\n",
                "Mappings", num_mappings, total_mapping_size + total_mapping_free_nodes_size,
                total_mapping_nodes, total_mapping_nodes_size, total_mapping_free_nodes,
                total_mapping_free_nodes_size);
  }
  tot += total_mapping_size + total_mapping_free_nodes_size;
  if (verbose && verbose != -1) outbuf_add(ob, "\n");

  tot += heart_beat_status(ob, verbose);
  if (verbose && verbose != -1) outbuf_add(ob, "\n");

  tot += add_string_status(ob, verbose);
  if (verbose && verbose != -1) outbuf_add(ob, "\n");

  tot += scratchpad_status(ob, verbose);
  if (verbose && verbose != -1) outbuf_add(ob, "\n");

  return tot;
}
}  // namespace

#ifdef F_MUD_STATUS
vm::Value f_mud_status(vm::EfunArgs&, LPC_INT verbose_arg) {
  uint64_t tot = 0;
  int verbose = 0;
  outbuffer_t ob;

  outbuf_zero(&ob);
  verbose = verbose_arg;

  if (verbose) {
    char dir_buf[1024];
    outbuf_addv(&ob, "current working directory: %s\n", get_current_dir(dir_buf, 1024));
    outbuf_add(&ob, "\n");

    outbuf_add(&ob, "add_message statistics\n");
    outbuf_add(&ob, "------------------------------\n");
    outbuf_addv(&ob,
                "Calls to add_message: %8" PRIu64 "   Packets: %8" PRIu64
                "   Average packet size: %.2lf bytes\n\n",
                add_message_calls, inet_packets,
                inet_volume && inet_packets ? static_cast<double>(inet_volume) / inet_packets : 0);

    outbuf_add(&ob, "\n");

    stat_living_objects(&ob);
    outbuf_add(&ob, "\n");
  }

  tot = calculate_and_maybe_print_memory_info(&ob, verbose);
  if (verbose) outbuf_add(&ob, "\n");

  if (verbose) {
    outbuf_add(&ob, "Summary\n");
    outbuf_add(&ob, "------------------------------\n");
  }
  outbuf_addv(&ob, "Total accounted: %8" PRIu64 "\n", tot);

  struct rusage current;
  if (!getrusage(RUSAGE_SELF, &current)) {
    outbuf_addv(&ob, "Max RSS: %8" PRIu64 "\n", current.ru_maxrss * 1024);
  }

  return outbuf_value(&ob);
}

#endif

#ifdef F_OBJECTP
vm::Value f_objectp(vm::EfunArgs&, vm::Arg x) {
  return x.is(T_OBJECT) && !(x.object()->flags & O_DESTRUCTED);
}

#endif

#ifdef F_ORIGIN
vm::Value f_origin(vm::EfunArgs&) { return vm::Value::str_constant(origin_name(caller_type)); }

#endif

#ifdef F_POINTERP
vm::Value f_pointerp(vm::EfunArgs&, vm::Arg x) { return x.is(T_ARRAY); }

#endif

#ifdef F_PRESENT
vm::Value f_present(vm::EfunArgs& f, vm::Arg what, std::optional<object_t*> env) {
  object_t* ob;

  if (!CONFIG_INT(__RC_NO_RESETS__) && CONFIG_INT(__RC_LAZY_RESETS__)) {
    if (env) {
      try_reset(*env);
    }
  }
  /* try_reset() runs LPC. A destructed search target is a 0 in its slot;
   * a destructed *environment* must return 0, not fall through to the
   * 1-arg form that searches current_object. */
  if (!what.is(T_OBJECT) && !what.is(T_STRING)) {
    return 0;
  }
  if (env && !f[1].is(T_OBJECT)) {
    return 0;
  }
  ob = object_present(what.sv(), env ? f[1].object() : nullptr);
  if (ob && object_visible(ob)) {
    return vm::Value::object(ob);
  }
  return 0;
}

#endif

#ifdef F_PREVIOUS_OBJECT
vm::Value f_previous_object(vm::EfunArgs&, LPC_INT which) {
  control_stack_t* p;
  int i;
  object_t* ob = nullptr;

  if ((i = which) > 0) {
    if (i >= CONFIG_INT(__MAX_CALL_DEPTH__)) {
      return 0;
    }
    ob = nullptr;
    p = csp;
    do {
      if ((p->framekind & FRAME_OB_CHANGE) && !(--i)) {
        ob = p->prev_ob;
        break;
      }
    } while (--p >= control_stack);
  } else if (i == -1) {
    array_t* v;

    i = previous_ob ? 1 : 0;
    p = csp;
    do {
      if ((p->framekind & FRAME_OB_CHANGE) && p->prev_ob) {
        i++;
      }
    } while (--p >= control_stack);
    v = allocate_empty_array(i);
    p = csp;
    if (previous_ob) {
      if (!(previous_ob->flags & O_DESTRUCTED)) {
        v->item[0].type = T_OBJECT;
        v->item[0].u.ob = previous_ob;
        add_ref(previous_ob, "previous_object(-1)");
      } else {
        v->item[0] = const0u;
      }
      i = 1;
    } else {
      i = 0;
    }
    do {
      if ((p->framekind & FRAME_OB_CHANGE) && (ob = p->prev_ob)) {
        if (!(ob->flags & O_DESTRUCTED)) {
          v->item[i].type = T_OBJECT;
          v->item[i].u.ob = ob;
          add_ref(ob, "previous_object(-1)");
        } else {
          v->item[i] = const0u;
        }
        i++;
      }
    } while (--p >= control_stack);
    return vm::Value::refed(v);
  } else {
    ob = previous_ob;
  }
  if (!ob || (ob->flags & O_DESTRUCTED)) {
    return 0;
  }
  return vm::Value::object(ob);
}

#endif

#ifdef F_PRINTF
void f_printf(vm::EfunArgs& f, vm::StringArg format) {
  int num_arg = f.argc();
  char* ret;

  if (command_giver) {
    /* argc==1 is format-only; argv pointer is unused when argc-1 == 0 */
    ret = string_print_formatted(format.c_str(), num_arg - 1, num_arg > 1 ? f[1].sv() : nullptr);
    if (ret) {
      tell_object(command_giver, ret, COUNTED_STRLEN(ret));
      FREE_MSTR(ret);
    }
  }
}

#endif

#ifdef F_QUERY_ED_MODE
vm::Value f_query_ed_mode(vm::EfunArgs&) {
  /* n = prompt for line 'n'
   0 = normal ed prompt
   -1 = not in ed
   -2 = more prompt */
  if (current_object->flags & O_IN_EDIT) {
    return static_cast<LPC_INT>(object_ed_mode(current_object));
  }
  return -1;
}

#endif

#ifdef F_QUERY_HOST_NAME
vm::Value f_query_host_name(vm::EfunArgs&) {
  char* tmp;

  if ((tmp = query_host_name())) {
    return vm::Value::str_constant(tmp);
  }
  return 0;
}

#endif

#ifdef F_QUERY_IDLE
vm::Value f_query_idle(vm::EfunArgs&, object_t* ob) {
  int i = query_idle(ob);
  return i;
}

#endif

#ifdef F_QUERY_IP_NAME
vm::Value f_query_ip_name(vm::EfunArgs&, std::optional<object_t*> ob) {
  const char* tmp;

  tmp = query_ip_name(ob.value_or(nullptr));
  if (!tmp) {
    return 0;
  }
  /* query_ip_name() hands us a reference: the result takes it over */
  svalue_t v{};
  v.type = T_STRING;
  v.subtype = STRING_SHARED;
  v.u.string = tmp;
  return vm::Value::owned(v);
}

#endif

#ifdef F_QUERY_IP_NUMBER
vm::Value f_query_ip_number(vm::EfunArgs&, std::optional<object_t*> ob) {
  const char* tmp;

  tmp = query_ip_number(ob.value_or(nullptr));
  if (!tmp) {
    return 0;
  }
  vm::Value result = vm::Value::str_copy(tmp);
  free_string(tmp);
  return result;
}

#endif

#ifdef F_QUERY_LOAD_AVERAGE
vm::Value f_query_load_average(vm::EfunArgs&) { return vm::Value::str_copy(query_load_av()); }

#endif

#ifdef F_QUERY_PRIVS
vm::Value f_query_privs(vm::EfunArgs&, object_t* ob) {
  if (ob->privs != nullptr) {
    return vm::Value::str_shared(ob->privs);
  }
  return 0;
}

#endif

#ifdef F_QUERY_SNOOPING
vm::Value f_query_snooping(vm::EfunArgs&, object_t* snooper) {
  object_t* ob = query_snooping(snooper);
  if (ob) {
    return vm::Value::object(ob);
  }
  return 0;
}

#endif

#ifdef F_QUERY_SNOOP
vm::Value f_query_snoop(vm::EfunArgs&, object_t* snoopee) {
  object_t* ob = query_snoop(snoopee);
  if (ob) {
    return vm::Value::object(ob);
  }
  return 0;
}

#endif

#ifdef F_RANDOM
vm::Value f_random(vm::EfunArgs&, LPC_INT n) {
  if (n <= 0) {
    return 0;
  }
  return random_number(n);
}

#endif

#ifdef F_SECURE_RANDOM
vm::Value f_secure_random(vm::EfunArgs&, LPC_INT n) {
  if (n <= 0) {
    return 0;
  }
  return secure_random_number(n);
}

#endif

#if defined(F_READ_BYTES) || defined(F_READ_FILE)
/* A file's text as an LPC string: the malloced buffer itself when it is
 * already valid UTF-8 (no copy), a sanitized copy otherwise. Either way it
 * stops at the first NUL, as the sanitized copy always did. */
static vm::Value file_text_value(char* str) {
  /* MSTR_SIZE is the allocation; text may be shorter if a NUL was embedded.
   * Bound the length first, then validate that slice once (not strlen+scan). */
  size_t const cap = MSTR_SIZE(str);
  size_t const len = strnlen(str, cap);
  if (u8_validate(reinterpret_cast<const uint8_t*>(str), len)) {
    if (len != cap) {
      str = extend_string(str, len); /* the counted size must be the text's */
    }
    return vm::Value::str_malloced(str);
  }
  auto res = u8_sanitize(std::string_view(str, len));
  FREE_MSTR(str);
  return vm::Value::str_copy(res);
}
#endif

#ifdef F_READ_BYTES
vm::Value f_read_bytes(vm::EfunArgs&, vm::StringArg file, std::optional<LPC_INT> start_arg,
                       std::optional<LPC_INT> len_arg) {
  char* str;
  LPC_INT start = start_arg.value_or(0), len = len_arg.value_or(0);
  int rlen = 0;

  str = read_bytes(file.c_str(), start, len, &rlen);
  if (str == nullptr) {
    return 0;
  }
  /* Bound by read_bytes into a counted malloced string; file_text_value
   * only uses MSTR_SIZE / strnlen, not an attacker-controlled length. */
  return file_text_value(str);  // coverity[tainted_data]
}

#endif

#ifdef F_READ_BUFFER
vm::Value f_read_buffer(vm::EfunArgs&, vm::Arg source, std::optional<LPC_INT> start_arg,
                        std::optional<LPC_INT> len_arg) {
  char* str;
  LPC_INT start = start_arg.value_or(0), len = len_arg.value_or(0);
  int rlen = 0;
  int from_file = 0; /* new line */

  if (source.is(T_STRING)) {
    from_file = 1; /* new line */
    str = read_bytes(source.string().c_str(), start, len, &rlen);
  } else { /* T_BUFFER */
    str = read_buffer(source.buffer(), start, len, &rlen);
  }
  if (str == nullptr) {
    return 0;
  }
  if (from_file) { /* changed */
    vm::Value bytes = vm::Value::str_malloced(str); /* freed even if allocate_buffer() errors */
    buffer_t* buf = allocate_buffer(rlen);
    memcpy(buf->item, str, rlen);
    return vm::Value::refed(buf);
  }
  /* T_BUFFER */
  return vm::Value::str_malloced(str);
}

#endif

#ifdef F_READ_FILE
vm::Value f_read_file(vm::EfunArgs&, vm::StringArg file, std::optional<LPC_INT> start_arg,
                      std::optional<LPC_INT> len_arg) {
  char* str;
  LPC_INT start = start_arg.value_or(0), len = len_arg.value_or(0);

  str = read_file(file.c_str(), start, len);
  if (!str) {
    return 0;
  }
  return file_text_value(str);
}

#endif

#ifdef F_RECEIVE
void f_receive(vm::EfunArgs&, vm::Arg data) {
  if (data.is(T_STRING)) {
    if (current_object->interactive) {
      vm::StringArg str = data.string();
      int const len = str.size();
      add_message(current_object, str.c_str(), len);
    }
  } else {
    if (current_object->interactive) {
      buffer_t* buf = data.buffer();
      add_message(current_object, reinterpret_cast<char*>(buf->item), buf->size);
    }
  }
}

#endif

#ifdef F_REG_ASSOC
vm::Value f_reg_assoc(vm::EfunArgs& f, vm::StringArg, array_t* patterns, array_t* tokens,
                      std::optional<vm::Arg> no_match) {
  return vm::Value::refed(
      reg_assoc(f[0].sv(), patterns, tokens, no_match ? no_match->sv() : &const0));
}

#endif

#ifdef F_REGEXP
vm::Value f_regexp(vm::EfunArgs&, vm::Arg subject, vm::StringArg pattern,
                   std::optional<LPC_INT> flag_arg) {
  int flag;

  if (flag_arg) {
    if (subject.is(T_STRING)) {
      error("3rd argument illegal for regexp(string, string)\n");
    }
    flag = *flag_arg;
  } else {
    flag = 0;
  }
  if (subject.is(T_STRING)) {
    flag = match_single_regexp(subject.string().c_str(), pattern.c_str());
    return flag;
  }
  return vm::Value::refed(match_regexp(subject.array(), pattern.c_str(), flag));
}

#endif

#ifdef F_REMOVE_CALL_OUT
vm::Value f_remove_call_out(vm::EfunArgs&, std::optional<vm::Arg> which) {
  int i;
  if (which) {
    if (which->is(T_STRING)) {
      i = remove_call_out(current_object, which->string().c_str());
    } else {
      i = remove_call_out_by_handle(current_object, which->number());
    }
    return i;
  }
  remove_all_call_out(current_object);
  return 0;
}

#endif

#ifdef F_RENAME
vm::Value f_rename(vm::EfunArgs&, vm::StringArg from, vm::StringArg to) {
  int i = do_rename(from.c_str(), to.c_str(), F_RENAME);
  return i;
}

#endif /* F_RENAME */

/* This is an enhancement to the f_replace_string() in efuns_main.c of
 MudOS v21.  When the search pattern has more than one character,
 this version of f_replace_string() uses a skip table to more efficiently
 search the file for the search pattern (the basic idea is to avoid
 strings comparisons where possible).  This version is anywhere from
 15% to 40% faster than the old version depending on the size of the
 string to be searched and the length of the search string (and depending
 on the relative frequency with which the letters in the search string
 appear in the string to be searched).

 Note: this version should behave identically to the old version (except
 for runtime).  When the search pattern is only one character long, the
 old algorithm is used.  The new algorithm is actually about 10% slower
 than the old one when the search string is only one character long.

 This enhancement to f_replace_string() was written by John Garnett
 (aka Truilkan) on 1995/04/29.  I believe the original replace_string()
 was written by Dave Richards (Cygnus).

 I didn't come up with the idea of this algorithm (learned it in
 a university programming course way back when).  For those interested
 in the workings of the algorithm, you can probably find it in a book on
 string processing algorithms.  Its also fairly easy to figure out the
 algorithm by tracing through it for a small example.
 */
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

/* replace_string()'s body, shared with dwlib's replace(). `source` is owned:
 * when the replacement is not longer than the pattern and nothing else
 * references the string, it is rewritten in place with no copy (a chained
 * replace_string(replace_string(s, ...), ...) never copies). */
vm::Value replace_string_value(vm::Value source, vm::StringArg pattern_arg,
                               vm::StringArg replacement, LPC_INT first, LPC_INT last) {
  auto max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);

  int plen, rlen, dlen, cur;

  const char* pattern;
  const char* replace;
  const char* src;
  char *dst1, *dst2;
  char* mutable_src = nullptr;
  int skip_table[256];
  const char* slimit;
  const char* flimit;
  char* climit;
  int probe;
  int skip;

  src = source->u.string;

  if (!last) {
    last = max_string_length;
  }

  if (first > last) { /* just return it */
    return source;
  }
  pattern = pattern_arg.c_str();
  plen = pattern_arg.size();
  if (!plen) {
    return source; /* just return it */
  }
  replace = replacement.c_str();
  rlen = replacement.size();
  dlen = 0;
  cur = 0;

  if (rlen <= plen) {
    /* in-string replacement: unlink copies only if something else still
     * references the string */
    mutable_src = source.unlink_string();
    src = mutable_src;
  }

  if (plen > 1) {
    /* build skip table */
    for (int j = 0; j < 256; j++) {
      skip_table[j] = plen;
    }
    for (int j = 0; j < plen; j++) {
      skip_table[static_cast<unsigned char>(pattern[j])] = plen - j - 1;
    }
    int const slen = SVALUE_STRLEN(source.get());
    slimit = src + slen;
    flimit = slimit - plen + 1;
    probe = plen - 1;
  }

  if (rlen <= plen) {
    /* in string replacement */
    dst2 = dst1 = mutable_src;

    if (plen > 1) { /* pattern length > 1, jump table most efficient */
      while (src < flimit) {
        if ((skip = skip_table[static_cast<unsigned char>(src[probe])])) {
          for (climit = dst2 + skip; dst2 < climit; *dst2++ = *src++) {
            ;
          }
        } else if (memcmp(src, pattern, plen) == 0) {
          cur++;
          if ((cur >= first) && (cur <= last)) {
            if (rlen) {
              memmove(dst2, replace, rlen);
              dst2 += rlen;
            }
            src += plen;
            if (cur == last) {
              break;
            }
          } else {
            memmove(dst2, src, plen);
            dst2 += plen;
            src += plen;
          }
        } else {
          *dst2++ = *src++;
        }
      }
      memmove(dst2, src, slimit - src);
      dst2 += (slimit - src);
      *dst2 = 0;
      source->u.string = extend_string(dst1, dst2 - dst1);
    } else { /* pattern length <= 1, brute force most efficient */
      /* Beek - if it was zero, we already returned, so plen == 1 */
      /* assume source string is a string < maximum string length */
      if (rlen) {
        while (*src) {
          if (*src == *pattern) {
            cur++;

            if (cur >= first && cur <= last) {
              *const_cast<char*>(src) = *replace;
            }
          }
          src++;
        }
        // Unlike every other in-place branch of this efun, this one never
        // reaches extend_string() (which resets the cached ASCII tag), yet it
        // just spliced *replace bytes into the unlinked STRING_MALLOC buffer.
        // A valid 1-byte replacement is always ASCII, but "\r" is too -- and
        // splicing CR into a CR-free string tagged MSTR_ASCII_YES breaks the
        // byte==cluster identity ("\r\n" is one grapheme cluster), leaving
        // sizeof() to answer from the stale tag. Reset so it re-derives.
        MSTR_ASCII(source->u.string) = MSTR_ASCII_UNKNOWN;
      } else { /* rlen is zero */
        while (*src) {
          if (*src++ == *pattern) {
            cur++;
            if (cur >= first) {
              dst2 = const_cast<char*>(src) - 1;
              while (*src) {
                if (*src == *pattern) {
                  cur++;
                  if (cur <= last) {
                    src++;
                    continue;
                  }
                  while (*src) {
                    *dst2++ = *src++;
                  }
                  break;
                }
                *dst2++ = *src++;
              }
              *dst2 = 0;
              source->u.string = extend_string(dst1, dst2 - dst1);
              break;
            }
          }
        }
      }
    }
    return source;
  } else {
    dst2 = dst1 = new_string(max_string_length, "f_replace_string: 2");

    if (plen > 1) {
      while (src < flimit) {
        if ((skip = skip_table[static_cast<unsigned char>(src[probe])])) {
          // This fast-path copy is not covered by the dlen-based guards below;
          // bound it against the output buffer and count it in dlen so a later
          // match's expansion guard sees the true written length.
          if (dst2 + skip > dst1 + max_string_length) {
            FREE_MSTR(dst1);
            return vm::Value::undefined();
          }
          for (climit = dst2 + skip; dst2 < climit; *dst2++ = *src++) {
            ;
          }
          dlen += skip;

        } else if (memcmp(src, pattern, plen) == 0) {
          cur++;
          if ((cur >= first) && (cur <= last)) {
            if (max_string_length - dlen <= rlen) {
              FREE_MSTR(dst1);
              return vm::Value::undefined();
            }
            memmove(dst2, replace, rlen);
            dst2 += rlen;
            dlen += rlen;
            src += plen;
            if (cur == last) {
              break;
            }
          } else {
            dlen += plen;
            if (max_string_length - dlen <= 0) {
              FREE_MSTR(dst1);
              return vm::Value::undefined();
            }
            memmove(dst2, src, plen);
            dst2 += plen;
            src += plen;
          }
        } else {
          if (max_string_length - dlen <= 1) {
            FREE_MSTR(dst1);
            return vm::Value::undefined();
          }
          *dst2++ = *src++;
          dlen++;
        }
      }
      if (max_string_length - dlen <= (slimit - src)) {
        FREE_MSTR(dst1);
        return vm::Value::undefined();
      }
      memmove(dst2, src, slimit - src);
      dst2 += (slimit - src);
    } else { /* plen <= 1 */
      /* Beek: plen == 1 */
      while (*src != '\0') {
        if (*src == *pattern) {
          cur++;
          if (cur >= first && cur <= last) {
            if (rlen != 0) {
              if (max_string_length - dlen <= rlen) {
                FREE_MSTR(dst1);
                return vm::Value::undefined();
              }
              strncpy(dst2, replace, rlen);
              dst2 += rlen;
              dlen += rlen;
            }
            src++;
            continue;
          }
        }
        if (max_string_length - dlen <= 1) {
          FREE_MSTR(dst1);
          return vm::Value::undefined();
        }
        *dst2++ = *src++;
        dlen++;
      }
    }
    *dst2 = '\0';

    /*
     * shrink block or make a copy of exact size
     */
    return vm::Value::str_malloced(extend_string(dst1, dst2 - dst1));
  }
}

/* 4-arg form: last = first (max replacements), first = 0. */
vm::Value f_replace_string(vm::EfunArgs& f, vm::StringArg, vm::StringArg pattern,
                           vm::StringArg replacement, std::optional<LPC_INT> first_arg,
                           std::optional<LPC_INT> last_arg) {
  LPC_INT first = 0, last = 0; /* LPC ints: an int would wrap 0x100000001 to 1 */

  if (first_arg) {
    first = *first_arg;
    if (last_arg) {
      last = *last_arg;
    } else {
      last = first;
      first = 0;
    }
  }
  return replace_string_value(f.take(0), pattern, replacement, first, last);
}
#endif

#ifdef F_RESOLVE
vm::Value f_resolve(vm::EfunArgs&, vm::StringArg host, vm::Arg callback) {
  LPC_INT i = query_addr_by_name(host.c_str(), callback.sv());
  return i;
}

#endif

#ifdef F_RM
vm::Value f_rm(vm::EfunArgs&, vm::StringArg file) {
  int i = remove_file(file.c_str());
  return i;
}

#endif

#ifdef F_RMDIR
vm::Value f_rmdir(vm::EfunArgs&, vm::StringArg dir) {
  vm::Value pathv = check_valid_path(dir.c_str(), current_object, "rmdir", 1);
  if (!pathv.is(T_STRING) || rmdir(pathv->u.string) == -1) {
    return 0;
  }
  return 1;
}

#endif

#ifdef F_SAY
void f_say(vm::EfunArgs& f, vm::StringArg, std::optional<vm::Arg> exclude) {
  array_t* avoid;
  /* Stack-local: nested say() from catch_tell must not clobber a static
   * exclude slot (same hazard as f_tell_room). */
  array_t vtmp = {1,
#ifdef DEBUGMALLOC_EXTENSIONS
                  1,
#endif
                  1,
#ifdef PACKAGE_MUDLIB_STATS
                  {(mudlib_stats_t*)nullptr, (mudlib_stats_t*)nullptr}
#endif
  };

  if (!exclude) {
    avoid = &the_null_array;
  } else if (exclude->is(T_OBJECT)) {
    vtmp.item[0].type = T_OBJECT;
    vtmp.item[0].u.ob = exclude->object();
    avoid = &vtmp;
  } else if (exclude->is(T_ARRAY)) {
    avoid = exclude->array();
  } else {
    avoid = &the_null_array;
  }
  say(f[0].sv(), avoid);
}
#endif

#ifdef F_SET_EVAL_LIMIT
/* warning: do not enable this without using valid_override() in the master
 object and a set_eval_limit() simul_efun to restrict access.
 */
vm::Value f_set_eval_limit(vm::EfunArgs&, LPC_INT limit) {
  switch (limit) {
    case 0: {
      vm::Value old_limit = max_eval_cost;
      set_eval(max_eval_cost);
      return old_limit;
    }
    case -1:
      return get_eval();
    case 1:
      return max_eval_cost;
    default:
      if (limit < -1) {
        error("Invalid number.");
      }
      max_eval_cost = limit;
      return limit;
  }
}
#endif

#ifdef F_SET_BIT
vm::Value f_set_bit(vm::EfunArgs& f, vm::StringArg bits, LPC_INT requested) {
  char* str;
  int len, old_len, ind, bit;
  vm::Value result;

  auto max_bitfield_bits = CONFIG_INT(__MAX_BITFIELD_BITS__);
  if (requested > max_bitfield_bits) {
    error("set_bit() bit requested: %" LPC_INT_FMTSTR_P " > maximum bits: %d\n", requested,
          max_bitfield_bits);
  }
  // Check the sign on the untruncated 64-bit value: a large-magnitude
  // negative LPC_INT can truncate into a positive `int` that would slip
  // past a post-truncation check, bypassing max_bitfield_bits entirely.
  if (requested < 0) {
    error("Bad argument 2 (negative) to set_bit().\n");
  }
  bit = requested;
  ind = bit / 6;
  bit %= 6;
  old_len = len = bits.size();
  if (ind >= len) {
    len = ind + 1;
  }
  /* Validate in-range characters before unlink/copy (same as clear_bit). */
  if (ind < old_len && (bits.c_str()[ind] > 0x3f + ' ' || bits.c_str()[ind] < ' ')) {
    error("Illegal bit pattern in set_bit character %d\n", ind);
  }
  if (ind < old_len) {
    result = f.take(0);
    str = result.unlink_string();
  } else {
    str = new_string(len, "f_set_bit: str");
    str[len] = '\0';
    if (old_len) {
      memcpy(str, bits.c_str(), old_len);
    }
    if (len > old_len) {
      memset(str + old_len, ' ', len - old_len);
    }
    result = vm::Value::str_malloced(str);
  }

  str[ind] = ((str[ind] - ' ') | (1 << bit)) + ' ';
  return result;
}
#endif

#ifdef F_SET_NOTIFY_DESTRUCT
void f_set_notify_destruct(vm::EfunArgs&, LPC_INT flag) {
  int const num = flag;
  if (num == 1) {
    current_object->flags |= O_NOTIFY_DESTRUCT;
  } else if (num == 0) {
    current_object->flags &= ~O_NOTIFY_DESTRUCT;
  } else {
    error("Bad argument 1 to set_notify_destructing()\n");
  }
}

#endif

#ifdef F_QUERY_NOTIFY_DESTRUCT
vm::Value f_query_notify_destruct(vm::EfunArgs&, object_t* ob) {
  int const num = ob->flags & O_NOTIFY_DESTRUCT;
  return num ? 1 : 0;
}

#endif

#ifdef F_SET_HEART_BEAT
void f_set_heart_beat(vm::EfunArgs&, LPC_INT interval) { set_heart_beat(current_object, interval); }

#endif

#ifdef F_QUERY_HEART_BEAT
vm::Value f_query_heart_beat(vm::EfunArgs&, object_t* ob) { return query_heart_beat(ob); }

#endif

#ifdef F_SET_HIDE
void f_set_hide(vm::EfunArgs&, LPC_INT hide) {
  if (!valid_hide(current_object)) {
    return;
  }
  if (hide) {
    num_hidden++;
    current_object->flags |= O_HIDDEN;
  } else {
    num_hidden--;
    current_object->flags &= ~O_HIDDEN;
  }
}
#endif

#ifdef F_SET_LIGHT
vm::Value f_set_light(vm::EfunArgs&, LPC_INT delta) {
  object_t* o1;

  add_light(current_object, delta);
  o1 = current_object;
#ifndef NO_ENVIRONMENT
  while (o1->super) {
    o1 = o1->super;
  }
#endif
  return o1->total_light;
}
#endif

#ifdef F_SET_PRIVS
void f_set_privs(vm::EfunArgs&, object_t* ob, vm::Arg privs) {
  if (ob->privs != nullptr) {
    free_string(ob->privs);
  }
  if (!privs.is(T_STRING)) {
    ob->privs = nullptr;
  } else {
    ob->privs = make_shared_string(privs.string().c_str());
  }
}
#endif

#ifdef F_SHADOW
vm::Value f_shadow(vm::EfunArgs&, object_t* ob, LPC_INT flag) {
  if (!flag) {
    ob = ob->shadowed;
    if (ob) {
      return vm::Value::object(ob);
    }
    return 0;
  }
  if (ob == current_object) {
    error("shadow: Can't shadow self\n");
  }
  if (validate_shadowing(ob)) {
    if (current_object->flags & O_DESTRUCTED) {
      return 0;
    }
    /*
     * The shadow is entered first in the chain.
     */
    while (ob->shadowed) {
      ob = ob->shadowed;
    }
    current_object->shadowing = ob;
    ob->shadowed = current_object;
    return vm::Value::object(ob);
  }
  return 0;
}
#endif

#ifdef F_SHOUT
void f_shout(vm::EfunArgs&, vm::StringArg str) { shout_string(str.c_str()); }

#endif

#ifdef F_SHUTDOWN
void f_shutdown(vm::EfunArgs&, std::optional<LPC_INT> exit_code) {
  /* does not return */
  shutdownMudOS(exit_code.value_or(0));
}
#endif

#ifdef F_SIZEOF
vm::Value f_sizeof(vm::EfunArgs&, vm::Arg x) {
  size_t i;

  switch (x.type()) {
    case T_CLASS:
      i = x.class_rep()->size;
      break;
    case T_ARRAY:
      i = x.array()->size;
      break;
    case T_MAPPING:
      i = MAP_COUNT(x.mapping());
      break;
    case T_BUFFER:
      i = x.buffer()->size;
      break;
    case T_STRING: {
      vm::StringArg str = x.string();
      // Pure ASCII (the overwhelming majority of mudlib text) has one
      // grapheme cluster per byte, and counted strings remember that in
      // their block header -- so this is O(1) with no iterator at all.
      if (u8_string_is_ascii_cached(str.c_str(), str.size(), (x.subtype() & STRING_COUNTED) != 0)) {
        i = str.size();
        return static_cast<LPC_INT>(i);
      }
      EGCSmartIterator iter(str.c_str(), str.size());
      if (!iter.ok()) {
        error("f_sizeof: Invalid UTF8 string!");
      }
      i = iter.count();
      break;
    }
    default:
      i = 0;
  }
  return static_cast<LPC_INT>(i);
}
#endif

#ifdef F_SNOOP
vm::Value f_snoop(vm::EfunArgs& f, object_t* snooper, std::optional<object_t*> snoopee) {
  /*
   * This one takes a variable number of arguments. It returns 0 or an
   * object.
   *
   * new_set_snoop() runs the valid_snoop apply: check the argument's slot
   * afterwards, since a destruct there leaves a 0 in it.
   */
  if (!snoopee) {
    if (!new_set_snoop(snooper, nullptr) || !f[0].is(T_OBJECT) ||
        (f[0].object()->flags & O_DESTRUCTED)) {
      return 0;
    }
    return f.take(0);
  }
  if (!new_set_snoop(snooper, *snoopee) || !f[1].is(T_OBJECT) ||
      (f[1].object()->flags & O_DESTRUCTED)) {
    return 0;
  }
  return f.take(1);
}
#endif

#ifdef F_SPRINTF
vm::Value f_sprintf(vm::EfunArgs& f, vm::StringArg format) {
  int num_arg = f.argc();
  char* s = string_print_formatted(format.c_str(), num_arg - 1, num_arg > 1 ? f[1].sv() : nullptr);
  if (!s) {
    return vm::Value::str_constant("");
  }
  return vm::Value::str_malloced(s);
}
#endif

#ifdef F_STAT
vm::Value f_stat(vm::EfunArgs&, vm::StringArg file, LPC_INT flag) {
  struct stat buf;
  const char* path;
  array_t* v;
  object_t* ob;

  // The flag arrives by value, so the valid_read apply check_valid_path()
  // runs cannot clobber it (issue #1271).
  vm::Value pathv = check_valid_path(file.c_str(), current_object, "stat", 0);
  if (!pathv.is(T_STRING)) {
    return 0;
  }
  path = pathv->u.string;
  if (stat(path, &buf) != -1) {
    if (buf.st_mode & S_IFREG) { /* if a regular file */
      /* object_visible() can run valid_hide() (LPC): ask before the result
       * array exists, so an error() there has nothing to leak. */
      LPC_INT load_time = 0;
      ob = find_object2(path);
      if (ob && object_visible(ob)) {
        load_time = ob->load_time;
      }

      v = allocate_empty_array(4);
      v->item[0].type = T_NUMBER;
      v->item[0].subtype = 0;
      v->item[0].u.number = buf.st_size;
      v->item[1].type = T_NUMBER;
      v->item[1].subtype = 0;
      v->item[1].u.number = buf.st_mtime;
      v->item[2].type = T_NUMBER;
      v->item[2].subtype = 0;
      v->item[2].u.number = load_time;

      v->item[3].type = T_NUMBER;
      v->item[3].subtype = 0;
      v->item[3].u.number = buf.st_ctime;

      return vm::Value::refed(v);
    }
  }
  v = get_dir(file.c_str(), flag);
  if (v) {
    return vm::Value::refed(v);
  }
  return 0;
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

vm::Value f_strsrch(vm::EfunArgs&, vm::StringArg big, vm::Arg little, LPC_INT from_right) {
  const svalue_t* arg1 = big.sv();
  const svalue_t* arg2 = little.sv();

  // fast track empty string search
  if (arg1->u.string[0] == '\0') {
    auto ret = -1;
    if ((arg2->type == T_STRING && arg2->u.string[0] == '\0') ||
        (arg2->type == T_NUMBER && arg2->u.number == '\0')) {
      ret = 0;
    }

    return ret;
  }

  // fast track single ascii character search
  {
    bool single_char_search = false;
    char single = -1;
    if (arg2->type == T_STRING && (arg2->u.string[0] != '\0' && arg2->u.string[1] == '\0') &&
        U8_IS_SINGLE(arg2->u.string[0])) {
      single = arg2->u.string[0];
      single_char_search = true;
    } else if (arg2->type == T_NUMBER && arg2->u.number >= 0 && arg2->u.number <= 0x7f) {
      single = arg2->u.number;
      single_char_search = true;
    }
    if (single_char_search && single >= 0) {
      const auto* res =
          from_right == 0 ? strchr(arg1->u.string, single) : strrchr(arg1->u.string, single);
      auto pos = res == nullptr ? -1 : (const char*)res - arg1->u.string;

      EGCIterator iter(arg1->u.string, SVALUE_STRLEN(arg1));
      auto ret = pos == -1 || !iter.ok() ? -1 : u8_offset_to_egc_index(iter, pos);

      return ret;
    }
  }

  uint8_t buf[U8_MAX_LENGTH + 1] = {0};
  const char* find = nullptr;
  size_t find_len = 0;

  if (arg2->type == T_NUMBER) {
    UBool is_error = false;
    int offset = 0;
    /* U8_APPEND takes a 32-bit UChar32: check the LPC int first, or
     * 0x1000000E9 would be searched for as U+00E9. */
    if (arg2->u.number < 0 || arg2->u.number > UCHAR_MAX_VALUE) {
      error("Invalid codepoint to search.");
    }
    U8_APPEND(buf, offset, sizeof(buf), arg2->u.number, is_error);
    if (is_error) {
      error("Invalid codepoint to search.");
    }
    buf[offset] = '\0';
    find = (const char*)buf;
    find_len = offset;
  } else {
    find = arg2->u.string;
    find_len = SVALUE_STRLEN(arg2);
  }
  size_t const src_len = SVALUE_STRLEN(arg1);

  LPC_INT ret = -1;
  // only search if there is a chance.
  if (find_len <= src_len) {
    EGCIterator iter(arg1->u.string, src_len);

    auto pos = u8_egc_find_as_offset(iter, find, find_len, from_right != 0);
    ret = pos == -1 ? -1 : u8_offset_to_egc_index(iter, pos);
  }

  return ret;
} /* strsrch */
#endif

#ifdef F_STRCMP
vm::Value f_strcmp(vm::EfunArgs&, vm::StringArg a, vm::StringArg b) {
  return strcmp(a.c_str(), b.c_str());
}

#endif

#ifdef F_STRINGP
vm::Value f_stringp(vm::EfunArgs&, vm::Arg x) { return x.is(T_STRING); }

#endif

#ifdef F_BUFFERP
vm::Value f_bufferp(vm::EfunArgs&, vm::Arg x) { return x.is(T_BUFFER); }

#endif

#ifdef F_TELL_OBJECT
void f_tell_object(vm::EfunArgs&, object_t* ob, vm::StringArg msg) {
  tell_object(ob, msg.c_str(), msg.size());
}

#endif

#ifdef F_TELL_ROOM
void f_tell_room(vm::EfunArgs&, vm::Arg room, vm::Arg msg, std::optional<vm::Arg> exclude) {
  array_t* avoid;
  /* Stack-local: a nested tell_room() (e.g. from a room create() during
   * find_object below) must not clobber a static exclude slot. */
  array_t vtmp = {1,
#ifdef DEBUGMALLOC_EXTENSIONS
                  1,
#endif
                  1,
#ifdef PACKAGE_MUDLIB_STATS
                  {(mudlib_stats_t*)nullptr, (mudlib_stats_t*)nullptr}
#endif
  };
  object_t* ob;

  /* Resolve the room first. find_object()/object_visible() run LPC that can
   * destruct the excluded object (zeroing its argument slot) or call
   * tell_room() recursively; both are why exclude is read only afterward. */
  if (room.is(T_OBJECT)) {
    ob = room.object();
  } else { /* must be a string... */
    ob = find_object(room.string().c_str());
    if (!ob || !object_visible(ob)) {
      error("Bad argument 1 to tell_room()\n");
    }
  }

  if (!exclude) {
    avoid = &the_null_array;
  } else if (exclude->is(T_OBJECT)) {
    vtmp.item[0].type = T_OBJECT;
    vtmp.item[0].u.ob = exclude->object();
    avoid = &vtmp;
  } else if (exclude->is(T_ARRAY)) {
    avoid = exclude->array();
  } else {
    /* Excluded object was destructed during find_object()/object_visible(). */
    avoid = &the_null_array;
  }

  tell_room(ob, msg.sv(), avoid);
}
#endif

#ifdef F_TEST_BIT
vm::Value f_test_bit(vm::EfunArgs&, vm::StringArg bits, LPC_INT ind) {
  if (ind / 6 >= bits.size()) {
    return 0;
  }
  if (ind < 0) {
    error("Bad argument 2 (negative) to test_bit().\n");
  }
  if ((bits.c_str()[ind / 6] - ' ') & (1 << (ind % 6))) {
    return 1;
  }
  return 0;
}
#endif

#ifdef F_NEXT_BIT
vm::Value f_next_bit(vm::EfunArgs&, vm::StringArg bits, LPC_INT start) {
  const char* str = bits.c_str();
  int const len = bits.size();
  int which, bit = 0, value;

  if (!len || start / 6 >= len) {
    return -1;
  }
  /* Find the next bit AFTER start */
  if (start > 0) {
    if (start % 6 == 5) {
      which = (start / 6) + 1;
      value = str[which] - ' ';
    } else {
      /* we have a partial byte to check */
      which = start / 6;
      bit = 0x3f - ((1 << ((start % 6) + 1)) - 1);
      value = (str[which] - ' ') & bit;
    }
  } else {
    which = 0;
    value = *str - ' ';
  }

  while (true) {
    if (value) {
      if (value & 0x07) {
        if (value & 0x01) {
          bit = which * 6;
        } else if (value & 0x02) {
          bit = which * 6 + 1;
        } else if (value & 0x04) {
          bit = which * 6 + 2;
        }
        break;
      }
      if (value & 0x38) {
        if (value & 0x08) {
          bit = which * 6 + 3;
        } else if (value & 0x10) {
          bit = which * 6 + 4;
        } else if (value & 0x20) {
          bit = which * 6 + 5;
        }
        break;
      }
    }
    which++;
    if (which == len) {
      bit = -1;
      break;
    }
    value = str[which] - ' ';
  }

  return bit;
}
#endif

#ifdef F__THIS_OBJECT
vm::Value f__this_object(vm::EfunArgs&) { return vm::Value::object(current_object); }

#endif

#ifdef F_THIS_PLAYER
vm::Value f_this_player(vm::EfunArgs&, LPC_INT interactive) {
  if (interactive) {
    if (current_interactive) {
      return vm::Value::object(current_interactive);
    }
    return 0;
  }
  if (command_giver) {
    return vm::Value::object(command_giver);
  }
  return 0;
}
#endif

int playerchanged;

#ifdef F_SET_THIS_PLAYER
void f_set_this_player(vm::EfunArgs&, vm::Arg who) {
  if (who.is(T_NUMBER)) {
    set_command_giver(0);
  } else {
    set_command_giver(who.object());
  }
  playerchanged = 1;
}
#endif

#ifdef F_THROW
void f_throw(vm::EfunArgs& f, vm::Arg) {
  free_svalue(&catch_value, "f_throw");
  catch_value = f.take(0).release();
  f.pop_back(1);
  throw_error(); /* do the longjump, with extra checks... */
}
#endif

#ifdef F__TO_FLOAT
vm::Value f__to_float(vm::EfunArgs&, vm::Arg x) {
  switch (x.type()) {
    case T_NUMBER:
      return static_cast<LPC_FLOAT>(x.number());
    case T_STRING:
      return strtod(x.string().c_str(), nullptr);
    default: /* T_REAL: already a float */
      return x.real();
  }
}
#endif

#ifdef F__TO_BUFFER
vm::Value f__to_buffer(vm::EfunArgs& f, vm::Arg x) {
  switch (x.type()) {
    case T_BUFFER:
      /* already a buffer */
      return f.take(0);
    case T_STRING:
    case T_ARRAY: {
      /* string -> its raw UTF-8 bytes; array -> ints 0..255, one byte
       * each (errors on any other item). Also the compile-time promotion
       * behind 'buffer b = str' and 'b += str'. */
      return vm::Value::refed(svalue_to_buffer_bytes(x.sv()));
    }
    default:
      bad_argument(x.sv(), T_STRING | T_BUFFER | T_ARRAY, 1, F__TO_BUFFER);
  }
}
#endif

#ifdef F__TO_INT
vm::Value f__to_int(vm::EfunArgs& f, vm::Arg x) {
  LPC_INT temp = 0;
  switch (x.type()) {
    case T_REAL:
      temp = x.real();
      return temp;
    case T_STRING: {
      const char* str = x.string().c_str();
      char* p;

      temp = strtoll(str, &p, 10);
      if (*p) {
        /* have to be a little careful here.  Checkign if p ==
         * str isn't good enough.
         *
         * Odd cases:
         * to_int("  foo")  // p == str + 2
         *
         * POSIX guarantees the strtoll() works in terms of isspace(),
         * though.  If there is something other than whitespace, then
         * there was a valid character consistent with the base,
         * so we were successful.
         *
         * (note: this means to_int("10x") == 10.  If you want to
         *  detect trailing garbage, use sscanf(str, "%d%s", ...).
         */
        while (p > str && uisspace(*(p - 1))) {
          p--;
        }

        if (p == str) {
          return vm::Value::undefined();
        }
      }
      return temp;
    }
    case T_BUFFER: {
      buffer_t* buf = x.buffer();
      if (buf->size < sizeof(int)) {
        return 0;
      }
      int hostint, netint;

      memcpy(reinterpret_cast<char*>(&netint), buf->item, sizeof(int));
      hostint = ntohl(netint);
      return hostint;
    }
    default: /* T_NUMBER: already an int (owning nothing, so copied as is) */
      return vm::Value::owned(*x.sv());
  }
}
#endif

#ifdef F_TYPEOF
vm::Value f_typeof(vm::EfunArgs&, vm::Arg x) {
  const char* t = type_name(x.type());
  return vm::Value::str_make_shared(t);
}

#endif

#ifdef F_UNDEFINEDP
vm::Value f_undefinedp(vm::EfunArgs&, vm::Arg x) {
  if (x.is(T_NUMBER)) {
    return !x.number() && (x.subtype() == T_UNDEFINED);
  }
  return 0;
}
#endif

#ifdef F_UPTIME
vm::Value f_uptime(vm::EfunArgs&) { return static_cast<LPC_INT>(get_current_time() - boot_time); }

#endif

#ifdef F_USERP
vm::Value f_userp(vm::EfunArgs&, object_t* ob) {
  int i = ob->flags & O_ONCE_INTERACTIVE;
  return i != 0;
}

#endif

#ifdef F_USERS
vm::Value f_users(vm::EfunArgs&) {
  bool include_hidden = true;
#ifdef F_SET_HIDE
  include_hidden = (current_object->flags & O_HIDDEN) || (valid_hide(current_object));
#endif

  auto total = users_num(include_hidden);
  array_t* ret = allocate_empty_array(total);

  int i = 0;
  for (auto* user : users()) {
    if (!include_hidden) {
      if ((user->ob->flags & O_HIDDEN) != 0) {
        continue;
      }
    }

    DEBUG_CHECK(i >= total, "Driver BUG: f_users()");

    ret->item[i].type = T_OBJECT;
    ret->item[i].u.ob = user->ob;
    add_ref(user->ob, "f_users");
    i++;
  }

  return vm::Value::refed(ret);
}
#endif

#ifdef F_WIZARDP
vm::Value f_wizardp(vm::EfunArgs&, object_t* ob) {
  int i = ob->flags & O_IS_WIZARD;
  return i != 0;
}

#endif

#ifdef F_VIRTUALP
vm::Value f_virtualp(vm::EfunArgs&, object_t* ob) {
  int i = ob->flags & O_VIRTUAL;
  return i != 0;
}

#endif

#ifdef F_WRITE
void f_write(vm::EfunArgs&, vm::Arg msg) { do_write(msg.sv()); }

#endif

#if defined(F_WRITE_BYTES) || defined(F_WRITE_BUFFER)
/* write_bytes() proper, and write_buffer() to a file, which also accepts a
 * buffer or an int (written as 4 bytes in network order) as the data. Its
 * errors name write_bytes(), as they always have. */
static int write_bytes_to_file(const char* file, LPC_INT start, vm::Arg data) {
  int i = 0;

  switch (data.type()) {
    case T_NUMBER: {
      int netint;
      char* netbuf;

      if (!data.number()) {
        bad_arg(3, F_WRITE_BYTES);
      }
      /* convert to network byte-order */
      netint = htonl(data.number());
      netbuf = reinterpret_cast<char*>(&netint);
      i = write_bytes(file, start, netbuf, sizeof(int));
      break;
    }

    case T_BUFFER: {
      buffer_t* buf = data.buffer();
      i = write_bytes(file, start, reinterpret_cast<char*>(buf->item), buf->size);
      break;
    }

    case T_STRING: {
      vm::StringArg str = data.string();
      i = write_bytes(file, start, str.c_str(), str.size());
      break;
    }

    default: {
      bad_argument(data.sv(), T_BUFFER | T_STRING | T_NUMBER, 3, F_WRITE_BYTES);
    }
  }
  return i;
}
#endif

#ifdef F_WRITE_BYTES
vm::Value f_write_bytes(vm::EfunArgs& f, vm::StringArg file, LPC_INT start, vm::StringArg) {
  return write_bytes_to_file(file.c_str(), start, f[2]);
}
#endif

#ifdef F_WRITE_BUFFER
vm::Value f_write_buffer(vm::EfunArgs&, vm::Arg dest, LPC_INT start, vm::Arg data) {
  int i = 0;

  if (dest.is(T_STRING)) {
    return write_bytes_to_file(dest.string().c_str(), start, data);
  }

  buffer_t* buf = dest.buffer();
  switch (data.type()) {
    case T_NUMBER: {
      int netint;
      char* netbuf;

      /* convert to network byte-order */
      netint = htonl(data.number());
      netbuf = reinterpret_cast<char*>(&netint);
      i = write_buffer(buf, start, netbuf, sizeof(int));
      break;
    }

    case T_BUFFER: {
      buffer_t* src = data.buffer();
      i = write_buffer(buf, start, reinterpret_cast<char*>(src->item), src->size);
      break;
    }

    case T_STRING: {
      vm::StringArg str = data.string();
      i = write_buffer(buf, start, str.c_str(), str.size());
      break;
    }

    default: {
      bad_argument(data.sv(), T_BUFFER | T_STRING | T_NUMBER, 3, F_WRITE_BUFFER);
    }
  }
  return i;
}
#endif

#ifdef F_WRITE_FILE
vm::Value f_write_file(vm::EfunArgs&, vm::StringArg file, vm::StringArg str, LPC_INT flags) {
  return write_file(file.c_str(), str.c_str(), flags);
}
#endif

#ifdef F_DUMP_FILE_DESCRIPTORS
vm::Value f_dump_file_descriptors(vm::EfunArgs&) {
  outbuffer_t out;

  outbuf_zero(&out);
  outbuf_add(&out, "ERROR: dump_file_descriptors is no longer supported.");
  return outbuf_value(&out);
}
#endif

#ifdef F_RECLAIM_OBJECTS
vm::Value f_reclaim_objects(vm::EfunArgs&) {
  auto res = reclaim_objects(false);
  add_gametick_event(0, TickEvent::callback_type([] { remove_destructed_objects(); }));
  return static_cast<LPC_INT>(res);
}

#endif

#ifdef F_MEMORY_INFO
vm::Value f_memory_info(vm::EfunArgs&, std::optional<object_t*> target) {
  LPC_INT mem;
  object_t* ob;

  if (!target) {
    LPC_INT const tot = calculate_and_maybe_print_memory_info(nullptr, -1);
    return tot;
  }
  ob = *target;
  /* as documented, object memory usage is not additive due to sharing of
   structures, so always include the program's total size even if this
   object is a clone or the program has more than one reference to it.
   There's no reliable way to determine when the program size should be
   included or not to be more accurate -- Marius, 30-Jul-2000 */
  mem = ob->prog->total_size;
  mem += (data_size(ob) + sizeof(object_t));
  return mem;
}
#endif

#ifdef F_RELOAD_OBJECT
void f_reload_object(vm::EfunArgs&, object_t* ob) {
  // The target may destruct itself during reload (call___INIT()/create() can
  // call destruct(this_object())): destruct sweeps the VM stack, so the arg
  // slot may hold a plain 0 instead of the object reference afterwards; the
  // window's release copes with either.
  reload_object(ob);
}
#endif

#ifdef F_RECOMPILE_OBJECT
vm::Value f_recompile_object(vm::EfunArgs&, object_t* ob) {
  // The target (or one of its clones) may destruct itself in its new
  // program's __INIT: destruct sweeps the VM stack, so the arg slot may hold
  // a plain 0 instead of the object reference afterwards; the window's
  // release copes with either.
  return recompile_object(ob);
}
#endif

#ifdef F_QUERY_SHADOWING
vm::Value f_query_shadowing(vm::EfunArgs&, object_t* ob) {
  if (ob->shadowing) {
    return vm::Value::object(ob->shadowing);
  }
  return 0;
}
#endif

#ifdef F_REQUEST_CLEAN_UP
vm::Value f_request_clean_up(vm::EfunArgs&, object_t* ob) {
  int success = 0;

  // Same condition as at load/clone time: only objects that actually
  // define clean_up() are put back on the sweep's query list (issue #917).
  if (!(ob->flags & O_DESTRUCTED) && function_exists(APPLY_CLEAN_UP, ob, 1)) {
    ob->flags |= O_WILL_CLEAN_UP;
    success = 1;
  }
  return success;
}

#endif

#ifdef F_SET_CLEAN_UP
void f_set_clean_up(vm::EfunArgs&, object_t* ob, std::optional<LPC_INT> seconds) {
  if (seconds) {
    ob->next_cleanup = g_current_gametick + time_to_next_gametick(std::chrono::seconds(*seconds));
  } else {
    ob->next_cleanup = 0; /* back to the idle-time rule */
  }

  /* Same gate as at load/clone time and request_clean_up(): only objects
     that actually define clean_up() are queried by the sweep. */
  if (!(ob->flags & O_DESTRUCTED) && function_exists(APPLY_CLEAN_UP, ob, 1)) {
    ob->flags |= O_WILL_CLEAN_UP;
  }
}
#endif

#ifdef F_SET_RESET
void f_set_reset(vm::EfunArgs&, object_t* ob, std::optional<LPC_INT> seconds) {
  const auto time_to_reset = CONFIG_INT(__TIME_TO_RESET__);

  if (seconds) {
    ob->next_reset = g_current_gametick + time_to_next_gametick(std::chrono::seconds(*seconds));
  } else {
    ob->next_reset =
        g_current_gametick + time_to_next_gametick(std::chrono::seconds(
                                 time_to_reset / 2 + random_number(time_to_reset / 2)));
  }
}
#endif

#ifdef F_FLOATP
vm::Value f_floatp(vm::EfunArgs&, vm::Arg x) { return x.is(T_REAL); }

#endif

#ifdef F_FLUSH_MESSAGES
void f_flush_messages(vm::EfunArgs&, std::optional<object_t*> ob) {
  if (ob) {
    if ((*ob)->interactive) {
      flush_message((*ob)->interactive);
    }
  } else {
    flush_message_all();
  }
}
#endif

#ifdef F_FIRST_INVENTORY
vm::Value f_first_inventory(vm::EfunArgs&, vm::Arg container) {
  object_t* ob = first_inventory(container.sv());
  if (ob) {
    return vm::Value::object(ob);
  }
  return 0;
}
#endif

#ifdef F_NEXT_INVENTORY
vm::Value f_next_inventory(vm::EfunArgs&, object_t* item) {
  object_t* ob = item->next_inv;

#ifdef F_SET_HIDE
  while (ob && (ob->flags & O_HIDDEN) && !object_visible(ob)) {
    ob = ob->next_inv;
  }
#endif

  if (ob) {
    return vm::Value::object(ob);
  }
  return 0;
}
#endif

#ifdef F_DEFER
void f_defer(vm::EfunArgs& f, funptr_t*) {
  auto* newlist = reinterpret_cast<struct defer_list*>(
      DMALLOC(sizeof(struct defer_list), TAG_DEFERS, "defer: new item"));

  if (CONFIG_INT(__RC_REVERSE_DEFER__)) {
    // In reverse mode, newlist always will be the last data.
    newlist->next = nullptr;
  } else {
    // In normal mode, newlist always will be the first data.
    newlist->next = csp->defers;
  }
  // Configure the new item: it takes over the argument's reference.
  newlist->func = f.take(0).release();
  /* push_object()'s rule: a destructed command_giver reads as undefined. */
  newlist->tp = command_giver ? const0u : const0;
  if (command_giver && !(command_giver->flags & (O_DESTRUCTED | O_BEING_DESTRUCTED))) {
    newlist->tp.type = T_OBJECT;
    newlist->tp.u.ob = command_giver;
    add_ref(command_giver, "defer");
  }

  // In reverse mode, if list is not null, then add new item to the end.
  if (CONFIG_INT(__RC_REVERSE_DEFER__)) {
    // If list is null, then init it with new item.
    if (csp->defers == nullptr) {
      csp->defers = newlist;
    } else {
      // Search last defer.
      struct defer_list* last_defer = csp->defers;
      while (last_defer->next) last_defer = last_defer->next;

      last_defer->next = newlist;
    }
  } else {
    // In normal mode, actual newlist will be init of defers list.
    csp->defers = newlist;
  }
}
#endif

#ifdef F_CRYPT
vm::Value f_crypt(vm::EfunArgs&, vm::StringArg password, vm::Arg salt_arg) {
  const int sh_a512_prefix_len = 3;
  const int sh_a512_salt_len = 16;
  char salt[sh_a512_prefix_len + sh_a512_salt_len + 1];
  const char* saltp = nullptr;
  const char* choice = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

  if (salt_arg.is(T_STRING)) {
    const char* given = salt_arg.string().c_str();
    size_t const given_len = salt_arg.string().size();
    // Support $1$, $2a$ (a,x,y), $5$, $6$
    if (given[0] == '$') {
      // $2?$
      if (given_len >= 4 && given[1] == '2' &&
          (given[2] == 'a' || given[2] == 'x' || given[2] == 'y') && given[3] == '$') {
        saltp = given;
      } else {
        if (given_len >= 3 && (given[1] == '1' || given[1] == '5' || given[1] == '6') &&
            given[2] == '$') {
          saltp = given;
        }
      }
    } else if (given_len >= 2) {
      // Compat: Old f_crypt only use first two character as key.
      debug_message(
          "old crypt() password detected, It is only using first 2 character as key and ignore "
          "password beyond 8 characters, please upgrade to SHA512 using crypt(password) "
          "immediately.\n");
      salt[0] = given[0];
      salt[1] = given[1];
      salt[2] = '\0';
      saltp = salt;
    }
  }

  if (saltp == nullptr) {
    salt[0] = '$';
    salt[1] = '6';
    salt[2] = '$';
    // Salts must come from the CSPRNG, not the game-visible mt19937_64 that
    // random() draws from: its state is recoverable from observed output, so
    // salts drawn from it are predictable ahead of account creation.
    for (auto i = 0; i < sh_a512_salt_len; i++) {
      salt[3 + i] = choice[secure_random_number(strlen(choice))];
    }
    salt[sizeof(salt) - 1] = '\0';
    saltp = salt;
  }
  const auto* key = password.c_str();
  auto* result = __crypt(key, saltp);
  if (result == nullptr || (result && result[0] == '*')) {
    error("Error in crypt(), check your salt");
  }
  result = string_copy(result, "f_crypt");
  return vm::Value::str_malloced(result);
}
#endif

#ifdef F_OLDCRYPT
vm::Value f_oldcrypt(vm::EfunArgs&, vm::StringArg password, vm::Arg salt_arg) {
  const int salt_len = 8;

  const char *res, *p;
  char salt[salt_len + 1];
  const char* choice = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ./";

  if (salt_arg.is(T_STRING) && salt_arg.string().size() >= 2) {
    p = salt_arg.string().c_str();
  } else {
    int i;

    for (i = 0; i < salt_len; i++) {
      salt[i] = choice[secure_random_number(strlen(choice))];
    }

    salt[salt_len] = 0;
    p = salt;
  }
  debug_message(
      "oldcrypt() is deprecated! it is using MD5 and unsafe, please upgrade your code to use "
      "crypt(password).\n");
  res = string_copy(custom_crypt(password.c_str(), p, nullptr), "f_oldcrypt");
  return vm::Value::str_malloced(res);
}
#endif

#ifdef F_RUSAGE
vm::Value f_rusage(vm::EfunArgs&) {
  mapping_t* m;
  struct rusage rus = {};

  if (getrusage(RUSAGE_SELF, &rus) < 0) {
    m = allocate_mapping(0);
  } else {
    uint64_t usertime, stime, maxrss;

    usertime = rus.ru_utime.tv_sec * 1000 + rus.ru_utime.tv_usec / 1000;
    stime = rus.ru_stime.tv_sec * 1000 + rus.ru_stime.tv_usec / 1000;
    maxrss = rus.ru_maxrss;

    m = allocate_mapping(16);
    add_mapping_pair(m, "utime", usertime);
    add_mapping_pair(m, "stime", stime);
    add_mapping_pair(m, "maxrss", maxrss);
    add_mapping_pair(m, "ixrss", rus.ru_ixrss);
    add_mapping_pair(m, "idrss", rus.ru_idrss);
    add_mapping_pair(m, "isrss", rus.ru_isrss);
    add_mapping_pair(m, "minflt", rus.ru_minflt);
    add_mapping_pair(m, "majflt", rus.ru_majflt);
    add_mapping_pair(m, "nswap", rus.ru_nswap);
    add_mapping_pair(m, "inblock", rus.ru_inblock);
    add_mapping_pair(m, "oublock", rus.ru_oublock);
    add_mapping_pair(m, "msgsnd", rus.ru_msgsnd);
    add_mapping_pair(m, "msgrcv", rus.ru_msgrcv);
    add_mapping_pair(m, "nsignals", rus.ru_nsignals);
    add_mapping_pair(m, "nvcsw", rus.ru_nvcsw);
    add_mapping_pair(m, "nivcsw", rus.ru_nivcsw);
  }
  return vm::Value::refed(m);
}

#endif

#ifdef F_STRWIDTH
vm::Value f_strwidth(vm::EfunArgs&, vm::StringArg str) {
  auto width = u8_width(str.c_str(), -1);
  return static_cast<LPC_INT>(width);
}

#endif
