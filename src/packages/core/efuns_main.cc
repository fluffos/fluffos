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
#include "packages/core/custom_crypt.h"
#include "packages/core/ed.h"
#include "packages/core/heartbeat.h"

int data_size(object_t* ob);
void reload_object(object_t* obj);

#ifdef F_ALL_INVENTORY
void f_all_inventory(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  array_t* vec = all_inventory(f.object(0), 0);
  /* Fresh array (ref 1); transfer without ref++. */
  f.ret_refed(vec);
}
#endif

#ifdef F_ALLOCATE
void f_allocate(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  if (num_arg == 2) {
    array_t* arr = allocate_array2(f.number(0), f.argp(1));
    f.ret_refed(arr);
  } else {
    array_t* arr = allocate_array(f.number(0));
    f.ret_refed(arr);
  }
}
#endif

#ifdef F_ALLOCATE_BUFFER
void f_allocate_buffer(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  buffer_t* buf = allocate_buffer(f.number(0));
  if (buf) {
    f.ret_refed(buf);
  } else {
    f.ret(0);
  }
}
#endif

#ifdef F_ALLOCATE_MAPPING
void f_allocate_mapping(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);

  if (num_arg == 2) {
    if (f.type(0) != T_ARRAY) {
      error("Bad argument 1 to allocate_mapping()\n");
    }
    if (f.type(1) == T_ARRAY) {
      array_t* arr = f.array(0);
      if (f.array(1)->size != arr->size) {
        error("Arrays passed to allocate_mapping() must be the same size\n");
      }
      mapping_t* m = mkmapping(arr, f.array(1));
      f.ret_refed(m);
    } else {
      array_t* arr = f.array(0);
      mapping_t* m = allocate_mapping2(arr, f.argp(1));
      f.ret_refed(m);
    }
  } else if (f.type(0) == T_NUMBER) {
    mapping_t* m = allocate_mapping(f.number(0));
    f.ret_refed(m);
  } else {
    error("Bad argument 1 to allocate_mapping()\n");
  }
}
#endif

#ifdef F_BIND
void f_bind(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  object_t* ob = f.object(1);
  funptr_t* old_fp = f.funptr(0);
  funptr_t* new_fp;
  svalue_t* res;

  if (ob == old_fp->hdr.owner) {
    /* no change */
    f.pop_back(1);
    f.done();
    return;
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
    LPCFrame b = LPCFrame::builder();
    b.push(current_object);
    b.push(old_fp->hdr.owner);
    b.push(ob);
    res = apply_master_ob(APPLY_VALID_BIND, 3);
    b.done();
  }
  if (!MASTER_APPROVED(res, "valid_bind")) {
    error("Master object denied permission to bind() function pointer.\n");
  }

  /* valid_bind() ran arbitrary LPC: if it destructed the new owner, the
   * destruct sweep (remove_object_from_stack) already released the stack's
   * ref and zeroed our argument slot -- storing `ob` into hdr.owner below
   * would create an uncounted reference whose eventual free_object() in
   * dealloc_funp() over-decrements a ref we no longer hold. Check the slot,
   * not ob->flags: ob may already be deallocated memory. */
  if (f.type(1) != T_OBJECT) {
    error("New owner was destructed during valid_bind().\n");
  }
  ob = f.object(1);

  new_fp = reinterpret_cast<funptr_t*>(DMALLOC(sizeof(funptr_t), TAG_FUNP, "f_bind"));
  *new_fp = *old_fp;
  new_fp->hdr.ref = 1;
  new_fp->hdr.owner = ob; /* one ref from being on stack */
  new_fp->hdr.owner_gen = ob->prog_generation;
  if (new_fp->hdr.args) {
    new_fp->hdr.args->ref++;
  }
  if ((old_fp->hdr.type & 0x0f) == FP_FUNCTIONAL) {
    new_fp->f.functional.prog->func_ref++;
    debug(d_flag, "add func ref /%s: now %i\n", new_fp->f.functional.prog->filename,
          new_fp->f.functional.prog->func_ref);
  }

  free_funp(old_fp);
  f.pop_back(1);
  f.top().u.fp = new_fp;
  f.done();
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
void f_cache_stats(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  outbuffer_t ob;

  outbuf_zero(&ob);
  print_cache_stats(&ob, 1);
  outbuf_push(&ob);
  f.done();
}

#endif

#ifdef F__CALL_OTHER
/* enhanced call_other written 930314 by Luke Mewburn <zak@rmit.edu.au> */
void f__call_other(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  svalue_t* arg;
  const char* funcname;
  object_t* ob;

  if (current_object->flags & O_DESTRUCTED) { /* No external calls allowed */
    f.ret_undefined();
    return;
  }
  arg = f.argp(0);
  if (arg[1].type == T_STRING) {
    funcname = arg[1].u.string;
  } else { /* must be T_ARRAY then */
    array_t* v = arg[1].u.arr;
    svalue_t* sv = nullptr;

    if (!((sv = v->item)->type == T_STRING)) {
      error("call_other: 1st elem of array for arg 2 must be a string\n");
    }
    check_for_destr(v);
    funcname = sv->u.string;
    num_arg = 2 + merge_arg_lists(num_arg - 2, v, 1);
    f.rebase(num_arg);
    arg = f.argp(0);
  }

  if (arg[0].type == T_OBJECT) {
    ob = arg[0].u.ob;
  } else if (arg[0].type == T_ARRAY) {
    array_t* ret = call_all_other(arg[0].u.arr, funcname, num_arg - 2);
    /* call_all_other consumed trailing args; left [obj_array, name]. */
    f.rebase(2);
    f.pop_back(1); /* name */
    f.set_top_array(ret);
    f.done();
    return;
  } else {
    ob = find_object(arg[0].u.string);
    if (!ob || !object_visible(ob)) {
      error("call_other() couldn't find object '%s'.\n", arg[0].u.string);
    }
  }
  /* apply() consumes num_arg-2 trailing args; left [obj, name]. */
  svalue_t* applied = ::apply(funcname, ob, num_arg - 2, ORIGIN_CALL_OTHER);
  f.rebase(2);
  if (applied == nullptr) {
    f.ret_undefined();
    return;
  }
  f.ret_copy(&apply_ret_value);
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
void f_call_stack(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i, n = csp - &control_stack[0] + 1;
  array_t* ret;

  if (f.number(0) < 0 || f.number(0) > 4) {
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
  switch (f.number(0)) {
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

  f.ret_refed(ret);
}

#endif

#ifdef F_CAPITALIZE
void f_capitalize(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  if (uislower(f.top().u.string[0])) {
    unlink_string_svalue(&f.top());
    // unlinked, so this is ok
    (const_cast<char*>(f.top().u.string))[0] =
        toupper(static_cast<unsigned char>(f.top().u.string[0]));
  }
  f.done();
}
#endif

#ifdef F_CHILDREN
void f_children(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  auto max_array_size = CONFIG_INT(__MAX_ARRAY_SIZE__);
  auto v = ObjectTable::instance().children(f.string(0));
  auto len = v.size() < max_array_size ? v.size() : max_array_size;
  auto* res = allocate_empty_array(len);

  for (auto i = 0; i < v.size() && i < len; ++i) {
    res->item[i].u.ob = v[i];
    res->item[i].type = T_OBJECT;
    add_ref(v[i], "children");
  }
  f.ret_refed(res);
}

#endif

#ifdef F_CLASSP
void f_classp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  f.ret(f.type(0) == T_CLASS);
}

#endif

#ifdef F_CLEAR_BIT
void f_clear_bit(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  char* str;
  int len, ind, bit;

  auto max_bitfield_bits = CONFIG_INT(__MAX_BITFIELD_BITS__);
  if (f.number(1) > max_bitfield_bits) {
    error("clear_bit() bit requested : %" LPC_INT_FMTSTR_P " > maximum bits: %d\n", f.number(1),
          max_bitfield_bits);
  }
  /* check the sign on the LPC int, before narrowing it (as set_bit does) */
  if (f.number(1) < 0) {
    error("Bad argument 2 (negative) to clear_bit().\n");
  }
  bit = f.number(1);
  f.pop_back(1);
  ind = bit / 6;
  bit %= 6;
  len = f.string_len(0);
  if (ind >= len) {
    f.done(); /* return first arg unmodified */
    return;
  }
  unlink_string_svalue(&f.top());
  str = const_cast<char*>(f.top().u.string);

  if (str[ind] > 0x3f + ' ' || str[ind] < ' ') {
    error("Illegal bit pattern in clear_bit character %d\n", ind);
  }
  str[ind] = ((str[ind] - ' ') & ~(1 << bit)) + ' ';
  f.done();
}

#endif

#ifdef F_CLONEP
void f_clonep(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  f.ret(f.type(0) == T_OBJECT && (f.object(0)->flags & O_CLONE));
}

#endif

#ifdef F__NEW
void f__new(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  object_t* ob = clone_object(f.string(0), num_arg - 1);

  if (ob) {
    f.ret(ob);
  } else {
    f.ret(0);
  }
}

#endif

#ifdef F_DEEP_INHERIT_LIST
void f_deep_inherit_list(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  array_t* vec = deep_inherit_list(f.object(0));
  f.ret_refed(vec);
}

#endif

#ifdef F_SET_DEBUG_LEVEL
void f_set_debug_level(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  if (f.type(0) == T_STRING) {
    debug_level_set(f.string(0));
  } else {
    debug_level = f.number(0);
  }
  f.ret_void();
}


void f_clear_debug_level(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  debug_level_clear(f.string(0));
  f.ret_void();
}


void f_debug_levels(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  /* not in debug.h since debug.h is included in many places that don't
   know about mapping_t */
  mapping_t* debug_levels();

  f.ret_refed(debug_levels());
}

#endif

#ifdef F_DEEP_INVENTORY
void f_deep_inventory(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  array_t* vec;
  if (num_arg == 2 && f.type(1) == T_FUNCTION &&
      (f.type(0) == T_ARRAY || f.type(0) == T_OBJECT)) {
    if (f.type(0) == T_ARRAY) {
      vec = deep_inventory_array(f.array(0), 1, f.funptr(1));
    } else {
      vec = deep_inventory(f.object(0), 0, f.funptr(1));
    }
  } else if (num_arg == 1 &&
             (f.type(0) == T_FUNCTION || f.type(0) == T_ARRAY || f.type(0) == T_OBJECT)) {
    if (f.type(0) == T_FUNCTION) {
      vec = deep_inventory(current_object, 0, f.funptr(0));
    } else if (f.type(0) == T_ARRAY) {
      vec = deep_inventory_array(f.array(0), 1, nullptr);
    } else {
      vec = deep_inventory(f.object(0), 0, nullptr);
    }
  } else {
    vec = &the_null_array;
  }

  f.ret_refed(vec);
}

#endif

#ifdef F_DESTRUCT
void f_destruct(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  destruct_object(f.object(0));
  /* Object was already removed from the stack by destruct_object(). */
  f.abandon();
}

#endif

#ifdef F_ENABLE_WIZARD
void f_enable_wizard(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  if (current_object->interactive) {
    current_object->flags |= O_IS_WIZARD;
  }
  f.ret_void();
}

#endif

#ifdef F_ERROR
void f_error(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int l = f.string_len(0);
  if (l && f.string(0)[l - 1] == '\n') {
    l--;
  }
  const auto max_string_length = LARGEST_PRINTABLE_STRING;
  l = std::min(l, max_string_length - 3);

  // VLA
  char err_buf[l + 3];

  err_buf[0] = '*';
  u8_strncpy(reinterpret_cast<uint8_t*>(err_buf + 1),
             reinterpret_cast<const uint8_t*>(f.string(0)), l);
  err_buf[l + 1] = '\n';
  err_buf[l + 2] = 0;

  error_handler(err_buf);
}

#endif

#ifdef F_DISABLE_WIZARD
void f_disable_wizard(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  if (current_object->interactive) {
    current_object->flags &= ~O_IS_WIZARD;
  }
  f.ret_void();
}

#endif

#ifdef F_ENVIRONMENT
void f_environment(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  object_t* ob = nullptr;

  if (num_arg) {
    if ((ob = f.object(0))->flags & O_DESTRUCTED) {
      error("environment() of destructed object.\n");
    }
    ob = ob->super;
  } else {
    ob = current_object->super;
  }

  if (ob && object_visible(ob)) {
    f.ret(ob);
  } else {
    f.ret(0);
  }
}

#endif

#ifdef F_FILE_NAME
void f_file_name(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  /* This function now returns a leading '/' */
  char* res = add_slash(f.object(0)->obname);
  f.ret_str_malloced(res);
}

#endif

#ifdef F_FILTER
void f_filter(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  svalue_t* arg = f.argp(0);

  if (arg->type == T_MAPPING) {
    filter_mapping(arg, num_arg);
  } else if (arg->type == T_STRING) {
    filter_string(arg, num_arg);
  } else {
    filter_array(arg, num_arg);
  }
  f.done();
}

#endif

#ifdef F_FIND_CALL_OUT
void f_find_call_out(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i;
  if (f.type(0) == T_NUMBER) {
    i = find_call_out_by_handle(current_object, f.number(0));
  } else { /* T_STRING */
    i = find_call_out(current_object, f.string(0));
  }
  f.ret(i);
}

#endif

#ifdef F_FIND_OBJECT
void f_find_object(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  object_t* ob;

  if (f.number(1)) {
    ob = find_object(f.string(0));
  } else {
    ob = find_object2(f.string(0));
  }
  if (ob && object_visible(ob)) {
    f.ret(ob);
  } else {
    f.ret(0);
  }
}

#endif

#ifdef F_FUNCTION_PROFILE
/* f_function_profile: John Garnett, 1993/05/31, 0.9.17.3 */
void f_function_profile(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  array_t* vec;
  mapping_t* map;
  program_t* prog;
  int nf, j;
  object_t* ob;

  ob = f.object(0);

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
  f.ret_refed(vec);
}

#endif

#ifdef F_FUNCTION_EXISTS
void f_function_exists(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  const char* str;
  char* res;
  int l;
  object_t* ob;
  int flag = 0;

  if (num_arg > 1) {
    if (num_arg > 2) {
      flag = f.number(2);
      f.pop_back(1);
    }
    ob = f.object(1);
    f.pop_back(1);
  } else {
    if (current_object->flags & O_DESTRUCTED) {
      f.ret(0);
      return;
    }
    ob = current_object;
  }

  str = function_exists(f.string(0), ob, flag);
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
    f.ret_str_malloced(res);
  } else {
    f.ret(0);
  }
}

#endif

#ifdef F_GET_CHAR
void f_get_char(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  svalue_t* arg;
  int i, tmp;
  int flag;

  arg = f.argp(0); /* Points arg at first argument. */
  if (num_arg == 1 || !(arg[1].type == T_NUMBER)) {
    tmp = 0;
    flag = 0;
  } else {
    tmp = 1;
    num_arg--; /* Don't count the flag as an arg */
    flag = arg[1].u.number;
  }
  num_arg--;
  i = get_char(arg, flag, num_arg, &arg[1 + tmp]);
  f.ret_front_number(i);
}

#endif

#ifdef F_IN_INPUT
void f_in_input(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i = f.object(0)->interactive && f.object(0)->interactive->input_to;
  f.ret(i != 0);
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

void f_inherits(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  object_t *ob, *base;
  int i;

  base = f.object(1);
  ob = find_object2(f.string(0));
  if (!ob) {
    f.ret(0);
    return;
  }
  i = inherits(base->prog, ob->prog);
  f.ret(i);
}

#endif

#ifdef F_SHALLOW_INHERIT_LIST
void f_shallow_inherit_list(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  array_t* vec = inherit_list(f.object(0));
  f.ret_refed(vec);
}

#endif

#ifdef F_INCLUDE_LIST
void f_include_list(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  array_t* vec = include_list(f.object(0));
  f.ret_refed(vec);
}

#endif

#ifdef F_INPUT_TO
void f_input_to(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  svalue_t* arg;
  int i, tmp;
  int flag;

  arg = f.argp(0); /* Points arg at first argument. */
  if ((num_arg == 1) || !(arg[1].type == T_NUMBER)) {
    tmp = flag = 0;
  } else {
    tmp = 1;
    num_arg--; /* Don't count the flag as an arg */
    flag = arg[1].u.number;
  }
  num_arg--; /* Don't count the name of the func either. */
  i = input_to(arg, flag, num_arg, &arg[1 + tmp]);
  f.ret_front_number(i);
}

#endif

#ifdef F_INTERACTIVE
void f_interactive(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i = (f.object(0)->interactive != nullptr);
  f.ret(i);
}

#endif

#ifdef F_INTP
void f_intp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  f.ret(f.type(0) == T_NUMBER);
}

#endif

#ifdef F_FUNCTIONP
void f_functionp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i;

  if (f.type(0) == T_FUNCTION) {
    i = f.funptr(0)->hdr.type;
    if (f.funptr(0)->hdr.args) {
      i |= FP_HAS_ARGUMENTS;
    }
    if (!f.funptr(0)->hdr.owner || (f.funptr(0)->hdr.owner->flags & O_DESTRUCTED)) {
      i |= FP_OWNER_DESTED;
    }
    f.ret(i);
    return;
  }
  f.ret(0);
}

#endif

#ifdef F_KEYS
void f_keys(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  array_t* vec = mapping_indices(f.mapping(0));
  f.ret_refed(vec);
}

#endif

#ifdef F_VALUES
void f_values(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  array_t* vec = mapping_values(f.mapping(0));
  f.ret_refed(vec);
}

#endif

#ifdef F_LOWER_CASE
void f_lower_case(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  char* str;

  str = const_cast<char*>(f.top().u.string);
  /* find first upper case letter, if any */
  for (; *str; str++) {
    if (uisupper(*str)) {
      int const l = str - f.top().u.string;
      unlink_string_svalue(&f.top());
      str = const_cast<char*>(f.top().u.string) + l;
      *str = tolower(static_cast<unsigned char>(*str));
      for (str++; *str; str++) {
        if (uisupper(*str)) {
          *str = tolower(static_cast<unsigned char>(*str));
        }
      }
      f.done();
      return;
    }
  }
  f.done();
}

#endif

#ifdef F_MALLOC_STATUS
void f_malloc_status(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
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
  outbuf_push(&ob);
  f.done();
}

#endif

#ifdef F_MAP_DELETE
void f_map_delete(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  mapping_delete(f.mapping(0), f.argp(1));
  f.ret_void();
}

#endif

#ifdef F_MAPP
void f_mapp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  f.ret(f.type(0) == T_MAPPING);
}

#endif

#ifdef F_MAP
void f_map(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  svalue_t* arg = f.argp(0);

  if (arg->type == T_MAPPING) {
    map_mapping(arg, num_arg);
  } else if (arg->type == T_ARRAY) {
    map_array(arg, num_arg);
  } else {
    map_string(arg, num_arg);
  }
  f.done();
}

#endif

#ifdef F_MASTER
void f_master(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  if (!master_ob) {
    f.ret(0);
  } else {
    f.ret(master_ob);
  }
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
void f_match_path(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  svalue_t* value;
  const char* src;
  char* dst;
  svalue_t* nvalue;
  mapping_t* map;
  char* tmpstr;

  value = &const0u;

  tmpstr = reinterpret_cast<char*>(DMALLOC(f.string_len(1) + 1, TAG_STRING, "match_path"));

  src = f.string(1);
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
    nvalue = find_string_in_mapping(f.mapping(0), tmpstr);

    if (nvalue != &const0u) {
      value = nvalue;
    }
  }

  FREE(tmpstr);
  /* Don't free mapping first, in case sometimes one uses a ref 1 mapping */
  /* Randor - 5/29/94 */
  svalue_t result;
  assign_svalue_no_free(&result, value);
  f.ret(result);
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

void f_member_array(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  array_t* v;
  int flag = 0;
  LPC_INT i;
  int size;

  if (num_arg > 2) {
    if (num_arg > 3) {
      CHECK_TYPES(f.argp(3), T_NUMBER, 4, F_MEMBER_ARRAY);
      flag = f.number(3);
      f.pop_back(1);
      num_arg--;
    }
    CHECK_TYPES(f.argp(2), T_NUMBER, 3, F_MEMBER_ARRAY);

    i = f.number(2);
    f.pop_back(1);
    num_arg--;
    if (i < 0) {
      bad_arg(3, F_MEMBER_ARRAY);
    }
  } else {
    i = 0;
  }

  if (f.type(1) == T_STRING) {
    const char* res;
    if (flag & 2) {
      error("member_array: can not search backwards in strings");
    }
    CHECK_TYPES(f.argp(0), T_NUMBER, 1, F_MEMBER_ARRAY);
    if (i > f.string_len(1)) {
      error("Index to start search from in member_array() is > string length.\n");
    }
    /* strchr() searches for a char: a value outside 0..255 would be
     * truncated onto some other byte (0x161 -> 'a', 0x100 -> the NUL
     * terminator), so it can never be in the string. */
    LPC_INT const c = f.number(0);
    if (c >= 0 && c <= 255 && (res = strchr(f.string(1) + i, static_cast<int>(c)))) {
      i = res - f.string(1);
    } else {
      i = -1;
    }
    if (!SVALUE_STR_ASCII(f.argp(1))) { /* cached; ASCII and CR-free never warns */
      const program_t* site_prog;
      const char* site_pc;
      member_array_call_site(&site_prog, &site_pc);
      if (!member_array_site_warned(site_prog, site_pc) &&
          member_array_byte_answer_misleads(f.string(1), f.string_len(1), c, i)) {
        warn_member_array_byte_search(site_prog, site_pc);
      }
    }
    f.ret(i);
    return;
  } else {
    svalue_t* sv;
    svalue_t* find;
    int flen = 0;

    size = (v = f.array(1))->size;
    find = f.argp(0);
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

    for (; i < size; i++) {
      int tmp = i;
      if (flag & 2) {
        tmp = size - tmp - 1;
      }
      sv = v->item + tmp;
      if ((flag & 4) && find->type == T_FUNCTION) {
        // Predicate search (issue #900): return the first index whose
        // element makes the function return truthy.
        {
          LPCFrame b = LPCFrame::builder();
          b.push(sv);
          svalue_t* ret = call_function_pointer(find->u.fp, 1);
          b.done();
          if (ret && !(ret->type == T_NUMBER && ret->u.number == 0)) {
            break;
          }
        }
        continue;
      }
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
  f.ret(i);
}

#endif

#ifdef F_MESSAGE
void f_message(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  array_t *use = nullptr, *avoid;
  svalue_t* args;

  args = f.argp(0);
  switch (args[2].type) {
    case T_OBJECT:
    case T_STRING:
      use = allocate_empty_array(1);
      use->item[0] = args[2];
      args[2].type = T_ARRAY;
      args[2].u.arr = use;
      break;
    case T_ARRAY:
      use = args[2].u.arr;
      break;
    case T_NUMBER: {
      int const len = SVALUE_STRLEN(args + 1);
      add_message(command_giver, args[1].u.string, len);
      f.ret_void();
      return;
    }
    default:
      bad_argument(&args[2], T_OBJECT | T_STRING | T_ARRAY | T_NUMBER, 3, F_MESSAGE);
  }
  if (num_arg == 4) {
    switch (args[3].type) {
      case T_OBJECT:
        avoid = allocate_empty_array(1);
        avoid->item[0] = args[3];
        args[3].type = T_ARRAY;
        args[3].u.arr = avoid;
        break;
      case T_ARRAY:
        avoid = args[3].u.arr;
        break;
      default:
        avoid = &the_null_array;
    }
  } else {
    avoid = &the_null_array;
  }
  do_message(&args[0], &args[1], use, avoid, 1);
  f.ret_void();
}

#endif

#ifdef F_MOVE_OBJECT
void f_move_object(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  object_t *o1, *o2;

  /* get destination */
  if (f.type(0) == T_OBJECT) {
    o2 = f.object(0);
  } else {
    if (!(o2 = find_object(f.string(0))) || !object_visible(o2)) {
      error("move_object failed: could not find destination\n");
    }
  }

  if ((o1 = current_object)->flags & O_DESTRUCTED) {
    error("move_object(): can't move a destructed object\n");
  }

  move_object(o1, o2);
  f.ret_void();
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
void f_mud_status(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  uint64_t tot = 0;
  int verbose = 0;
  outbuffer_t ob;

  outbuf_zero(&ob);
  verbose = f.number(0);
  f.pop_back(1);

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

  outbuf_push(&ob);
  f.done();
}

#endif

#ifdef F_OBJECTP
void f_objectp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  f.ret(f.type(0) == T_OBJECT && !(f.object(0)->flags & O_DESTRUCTED));
}

#endif

#ifdef F_ORIGIN
void f_origin(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  f.ret_str_constant(origin_name(caller_type));
}

#endif

#ifdef F_POINTERP
void f_pointerp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  f.ret(f.type(0) == T_ARRAY);
}

#endif

#ifdef F_PRESENT
void f_present(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  svalue_t* arg = f.argp(0);
  object_t* ob;

  if (!CONFIG_INT(__RC_NO_RESETS__) && CONFIG_INT(__RC_LAZY_RESETS__)) {
    if (num_arg == 2) {
      try_reset(arg[1].u.ob);
    }
  }
  ob = object_present(arg, num_arg == 1 ? nullptr : arg[1].u.ob);
  if (ob && object_visible(ob)) {
    f.ret(ob);
  } else {
    f.ret(0);
  }
}

#endif

#ifdef F_PREVIOUS_OBJECT
void f_previous_object(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  control_stack_t* p;
  int i;
  object_t* ob = nullptr;

  if ((i = f.number(0)) > 0) {
    if (i >= CONFIG_INT(__MAX_CALL_DEPTH__)) {
      f.set_top_number(0);
      f.done();
      return;
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
    f.ret_refed(v);
    return;
  } else {
    ob = previous_ob;
  }
  if (!ob || (ob->flags & O_DESTRUCTED)) {
    f.set_top_number(0);
    f.done();
  } else {
    f.ret(ob);
  }
}

#endif

#ifdef F_PRINTF
void f_printf(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  char* ret;

  if (command_giver) {
    ret = string_print_formatted(f.string(0), num_arg - 1, f.argp(1));
    if (ret) {
      tell_object(command_giver, ret, COUNTED_STRLEN(ret));
      FREE_MSTR(ret);
    }
  }

  f.ret_void();
}

#endif

#ifdef F_PROCESS_STRING
void f_process_string(int /*num_arg*/) {
  LPCFrame f = LPCFrame::args(1);
  char* str;

  str = process_string(f.string(0));
  if (str != f.string(0)) {
    f.ret_str_malloced(str);
  } else {
    f.done();
  }
}

#endif

#ifdef F_PROCESS_VALUE
void f_process_value(int /*num_arg*/) {
  LPCFrame f = LPCFrame::args(1);
  svalue_t* ret;

  ret = process_value(f.string(0));
  if (ret) {
    svalue_t v;
    assign_svalue_no_free(&v, ret);
    f.ret(v);
  } else {
    f.ret(0);
  }
}

#endif

#ifdef F_QUERY_ED_MODE
void f_query_ed_mode(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  /* n = prompt for line 'n'
   0 = normal ed prompt
   -1 = not in ed
   -2 = more prompt */
  if (current_object->flags & O_IN_EDIT) {
    f.ret(static_cast<LPC_INT>(object_ed_mode(current_object)));
  } else {
    f.ret(static_cast<LPC_INT>(-1));
  }
}

#endif

#ifdef F_QUERY_HOST_NAME
void f_query_host_name(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  char* tmp;

  if ((tmp = query_host_name())) {
    f.ret_str_constant(tmp);
  } else {
    f.ret(0);
  }
}

#endif

#ifdef F_QUERY_IDLE
void f_query_idle(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i = query_idle(f.object(0));
  f.ret(i);
}

#endif

#ifdef F_QUERY_IP_NAME
void f_query_ip_name(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  const char* tmp;

  tmp = query_ip_name(num_arg ? f.object(0) : nullptr);
  if (!tmp) {
    f.ret(0);
  } else {
    f.ret_str_shared(tmp);
  }
  // no need to free
}

#endif

#ifdef F_QUERY_IP_NUMBER
void f_query_ip_number(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  const char* tmp;

  tmp = query_ip_number(num_arg ? f.object(0) : nullptr);
  if (!tmp) {
    f.ret(0);
  } else {
    f.ret_str_copy(tmp);
  }

  if (tmp) free_string(tmp);
}

#endif

#ifdef F_QUERY_LOAD_AVERAGE
void f_query_load_average(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  f.ret_str_copy(query_load_av());
}

#endif

#ifdef F_QUERY_PRIVS
void f_query_privs(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  object_t* ob = f.object(0);
  if (ob->privs != nullptr) {
    f.ret_str_shared(ob->privs);
  } else {
    f.ret(0);
  }
}

#endif

#ifdef F_QUERY_SNOOPING
void f_query_snooping(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  object_t* ob = query_snooping(f.object(0));
  if (ob) {
    f.ret(ob);
  } else {
    f.ret(0);
  }
}

#endif

#ifdef F_QUERY_SNOOP
void f_query_snoop(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  object_t* ob = query_snoop(f.object(0));
  if (ob) {
    f.ret(ob);
  } else {
    f.ret(0);
  }
}

#endif

#ifdef F_RANDOM
void f_random(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  if (f.number(0) <= 0) {
    f.set_top_number(0);
    f.done();
    return;
  }
  f.set_top_number(random_number(f.number(0)));
  f.done();
}

#endif

#ifdef F_SECURE_RANDOM
void f_secure_random(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  if (f.number(0) <= 0) {
    f.set_top_number(0);
    f.done();
    return;
  }
  f.set_top_number(secure_random_number(f.number(0)));
  f.done();
}

#endif

#ifdef F_READ_BYTES
void f_read_bytes(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  char* str;
  LPC_INT start = 0, len = 0;
  int rlen = 0;

  if (num_arg > 1) {
    start = f.number(1);
  }
  if (num_arg == 3) {
    len = f.number(2);
  }
  str = read_bytes(f.string(0), start, len, &rlen);
  if (str == nullptr) {
    f.ret(0);
  } else {
    auto res = u8_sanitize(str);
    FREE_MSTR(str);
    f.ret_str_copy(res.c_str());
  }
}

#endif

#ifdef F_READ_BUFFER
void f_read_buffer(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  char* str;
  LPC_INT start = 0, len = 0;
  int rlen = 0;
  int from_file = 0; /* new line */

  if (num_arg > 1) {
    start = f.number(1);
    if (num_arg == 3) {
      len = f.number(2);
    }
  }
  if (f.type(0) == T_STRING) {
    from_file = 1; /* new line */
    str = read_bytes(f.string(0), start, len, &rlen);
  } else { /* T_BUFFER */
    str = read_buffer(f.buffer(0), start, len, &rlen);
  }
  if (str == nullptr) {
    f.ret(0);
  } else if (from_file) { /* changed */
    buffer_t* buf = allocate_buffer(rlen);
    memcpy(buf->item, str, rlen);
    FREE_MSTR(str);
    f.ret_refed(buf);
  } else { /* T_BUFFER */
    f.ret_str_malloced(str);
  }
}

#endif

#ifdef F_READ_FILE
void f_read_file(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  char* str;
  LPC_INT start = 0, len = 0;

  switch (num_arg) {
    case 3:
      len = f.number(2);
      f.pop_back(1);
    /* fall through */
    case 2:
      start = f.number(1);
      f.pop_back(1);
    /* fall through */
    case 1:
      break;
    default:
      error("Wrong number of arguments!");
  }
  str = read_file(f.string(0), start, len);
  if (!str) {
    f.ret(0);
  } else {
    auto res = u8_sanitize(str);
    FREE_MSTR(str);
    f.ret_str_copy(res.c_str());
  }
}

#endif

#ifdef F_RECEIVE
void f_receive(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  if (f.type(0) == T_STRING) {
    if (current_object->interactive) {
      int const len = f.string_len(0);
      add_message(current_object, f.string(0), len);
    }
  } else {
    if (current_object->interactive) {
      add_message(current_object, reinterpret_cast<char*>(f.buffer(0)->item), f.buffer(0)->size);
    }
  }
  f.ret_void();
}

#endif

#ifdef F_REG_ASSOC
void f_reg_assoc(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  svalue_t* arg;
  array_t* vec;

  arg = f.argp(0);

  if (!(arg[2].type == T_ARRAY)) {
    error("Bad argument 3 to reg_assoc()\n");
  }

  vec = reg_assoc(arg, arg[1].u.arr, arg[2].u.arr, num_arg > 3 ? &arg[3] : &const0);
  f.ret_refed(vec);
}

#endif

#ifdef F_REGEXP
void f_regexp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  array_t* v;
  int flag;

  if (num_arg > 2) {
    if (!(f.type(2) == T_NUMBER)) {
      error("Bad argument 3 to regexp()\n");
    }
    if (f.type(0) == T_STRING) {
      error("3rd argument illegal for regexp(string, string)\n");
    }
    flag = f.number(2);
    f.pop_back(1);
  } else {
    flag = 0;
  }
  if (f.type(0) == T_STRING) {
    flag = match_single_regexp(f.string(0), f.string(1));
    f.ret(flag);
  } else {
    v = match_regexp(f.array(0), f.string(1), flag);
    f.ret_refed(v);
  }
}

#endif

#ifdef F_REMOVE_CALL_OUT
void f_remove_call_out(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i;
  if (num_arg) {
    if (f.type(0) == T_STRING) {
      i = remove_call_out(current_object, f.string(0));
    } else {
      i = remove_call_out_by_handle(current_object, f.number(0));
    }
    f.ret(i);
  } else {
    remove_all_call_out(current_object);
    f.ret(0);
  }
}

#endif

#ifdef F_RENAME
void f_rename(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i = do_rename(f.string(0), f.string(1), F_RENAME);
  f.ret(i);
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

void f_replace_string(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  auto max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);

  int plen, rlen, dlen, cur;
  LPC_INT first, last; /* LPC ints: an int would wrap 0x100000001 to 1 */

  const char* pattern;
  const char* replace;
  const char* src;
  char *dst1, *dst2;
  svalue_t* arg;
  int skip_table[256];
  const char* slimit;
  const char* flimit;
  char* climit;
  int probe;
  int skip;

  if (num_arg > 5) {
    error("Too many args to replace_string.\n");
  }
  arg = f.argp(0);
  src = arg->u.string;
  first = 0;
  last = 0;

  if (num_arg >= 4) {
    CHECK_TYPES((arg + 3), T_NUMBER, 4, F_REPLACE_STRING);
    first = (arg + 3)->u.number;

    if (num_arg == 4) {
      last = first;
      first = 0;
    } else if (num_arg == 5) {
      CHECK_TYPES(f.argp(4), T_NUMBER, 5, F_REPLACE_STRING);
      /* first set above. */
      last = f.number(4);
    }
  }

  if (!last) {
    last = max_string_length;
  }

  if (first > last) { /* just return it */
    f.pop_back(num_arg - 1);
    f.done();
    return;
  }
  pattern = (arg + 1)->u.string;
  plen = SVALUE_STRLEN(arg + 1);
  if (!plen) {
    f.pop_back(num_arg - 1); /* just return it */
    f.done();
    return;
  }
  replace = (arg + 2)->u.string;
  rlen = SVALUE_STRLEN(arg + 2);
  dlen = 0;
  cur = 0;

  if (rlen <= plen) {
    /* we're going to do in string replacement */
    unlink_string_svalue(arg);
    src = arg->u.string;
  }

  if (plen > 1) {
    /* build skip table */
    for (int j = 0; j < 256; j++) {
      skip_table[j] = plen;
    }
    for (int j = 0; j < plen; j++) {
      skip_table[static_cast<unsigned char>(pattern[j])] = plen - j - 1;
    }
    int const slen = SVALUE_STRLEN(arg);
    slimit = src + slen;
    flimit = slimit - plen + 1;
    probe = plen - 1;
  }

  if (rlen <= plen) {
    /* in string replacement */
    dst2 = dst1 = const_cast<char*>(arg->u.string);

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
      arg->u.string = extend_string(dst1, dst2 - dst1);
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
        MSTR_ASCII(arg->u.string) = MSTR_ASCII_UNKNOWN;
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
              arg->u.string = extend_string(dst1, dst2 - dst1);
              break;
            }
          }
        }
      }
    }
    f.pop_back(num_arg - 1);
    f.done();
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
            f.ret_undefined();
            return;
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
              f.ret_undefined();
              return;
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
              f.ret_undefined();
              return;
            }
            memmove(dst2, src, plen);
            dst2 += plen;
            src += plen;
          }
        } else {
          if (max_string_length - dlen <= 1) {
            FREE_MSTR(dst1);
              f.ret_undefined();
              return;
          }
          *dst2++ = *src++;
          dlen++;
        }
      }
      if (max_string_length - dlen <= (slimit - src)) {
        FREE_MSTR(dst1);
        f.ret_undefined();
        return;
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
                f.ret_undefined();
                return;
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
          f.ret_undefined();
          return;
        }
        *dst2++ = *src++;
        dlen++;
      }
    }
    *dst2 = '\0';

    /*
     * shrink block or make a copy of exact size
     */
    f.ret_str_malloced(extend_string(dst1, dst2 - dst1));
  }
}
#endif

#ifdef F_RESOLVE
void f_resolve(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  LPC_INT i = query_addr_by_name(f.string(0), f.argp(1));
  f.ret(i);
}

#endif

#ifdef F_RM
void f_rm(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i = remove_file(f.string(0));
  f.ret(i);
}

#endif

#ifdef F_RMDIR
void f_rmdir(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  const char* path = check_valid_path(f.string(0), current_object, "rmdir", 1);
  if (!path || rmdir(path) == -1) {
    f.ret(0);
  } else {
    f.ret(1);
  }
}

#endif

#ifdef F_SAY
void f_say(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  array_t* avoid;
  static array_t vtmp = {1,
#ifdef DEBUGMALLOC_EXTENSIONS
                         1,
#endif
                         1,
#ifdef PACKAGE_MUDLIB_STATS
                         {(mudlib_stats_t*)nullptr, (mudlib_stats_t*)nullptr}
#endif
  };

  if (num_arg == 1) {
    avoid = &the_null_array;
    say(f.argp(0), avoid);
  } else {
    if (f.type(1) == T_OBJECT) {
      vtmp.item[0].type = T_OBJECT;
      vtmp.item[0].u.ob = f.object(1);
      avoid = &vtmp;
    } else { /* must be an array... */
      avoid = f.array(1);
    }
    say(f.argp(0), avoid);
  }
  f.ret_void();
}
#endif

#ifdef F_SET_EVAL_LIMIT
/* warning: do not enable this without using valid_override() in the master
 object and a set_eval_limit() simul_efun to restrict access.
 */
void f_set_eval_limit(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  switch (f.number(0)) {
    case 0:
      f.set_top_number(max_eval_cost);
      set_eval(max_eval_cost);
      break;
    case -1:
      f.set_top_number(get_eval());
      break;
    case 1:
      f.set_top_number(max_eval_cost);
      break;
    default:
      if (f.number(0) < -1) {
        error("Invalid number.");
      }
      max_eval_cost = f.number(0);
      break;
  }
  f.done();
}
#endif

#ifdef F_SET_BIT
void f_set_bit(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  char* str;
  int len, old_len, ind, bit;

  auto max_bitfield_bits = CONFIG_INT(__MAX_BITFIELD_BITS__);
  if (f.number(1) > max_bitfield_bits) {
    error("set_bit() bit requested: %" LPC_INT_FMTSTR_P " > maximum bits: %d\n", f.number(1),
          max_bitfield_bits);
  }
  // Check the sign on the untruncated 64-bit value: a large-magnitude
  // negative LPC_INT can truncate into a positive `int` that would slip
  // past a post-truncation check, bypassing max_bitfield_bits entirely.
  if (f.number(1) < 0) {
    error("Bad argument 2 (negative) to set_bit().\n");
  }
  bit = f.number(1);
  f.pop_back(1);
  ind = bit / 6;
  bit %= 6;
  old_len = len = f.string_len(0);
  if (ind >= len) {
    len = ind + 1;
  }
  if (ind < old_len) {
    unlink_string_svalue(&f.top());
    str = const_cast<char*>(f.top().u.string);
  } else {
    str = new_string(len, "f_set_bit: str");
    str[len] = '\0';
    if (old_len) {
      memcpy(str, f.string(0), old_len);
    }
    if (len > old_len) {
      memset(str + old_len, ' ', len - old_len);
    }
    f.set_top_str_malloced(str);
  }

  if (str[ind] > 0x3f + ' ' || str[ind] < ' ') {
    error("Illegal bit pattern in set_bit character %d\n", ind);
  }
  str[ind] = ((str[ind] - ' ') | (1 << bit)) + ' ';
  f.done();
}
#endif

#ifdef F_SET_NOTIFY_DESTRUCT
void f_set_notify_destruct(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int const num = f.number(0);
  if (num == 1) {
    current_object->flags |= O_NOTIFY_DESTRUCT;
  } else if (num == 0) {
    current_object->flags &= ~O_NOTIFY_DESTRUCT;
  } else {
    error("Bad argument 1 to set_notify_destructing()\n");
  }
  f.ret_void();
}

#endif

#ifdef F_QUERY_NOTIFY_DESTRUCT
void f_query_notify_destruct(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int const num = f.object(0)->flags & O_NOTIFY_DESTRUCT;
  f.ret(num ? 1 : 0);
}

#endif

#ifdef F_SET_HEART_BEAT
void f_set_heart_beat(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  set_heart_beat(current_object, f.number(0));
  f.ret_void();
}

#endif

#ifdef F_QUERY_HEART_BEAT
void f_query_heart_beat(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  f.ret(query_heart_beat(f.object(0)));
}

#endif

#ifdef F_SET_HIDE
void f_set_hide(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  if (!valid_hide(current_object)) {
    f.ret_void();
    return;
  }
  if (f.number(0)) {
    num_hidden++;
    current_object->flags |= O_HIDDEN;
  } else {
    num_hidden--;
    current_object->flags &= ~O_HIDDEN;
  }
  f.ret_void();
}
#endif

#ifdef F_SET_LIGHT
void f_set_light(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  object_t* o1;

  add_light(current_object, f.number(0));
  o1 = current_object;
#ifndef NO_ENVIRONMENT
  while (o1->super) {
    o1 = o1->super;
  }
#endif
  f.set_top_number(o1->total_light);
  f.done();
}
#endif

#ifdef F_SET_PRIVS
void f_set_privs(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  object_t* ob = f.object(0);

  if (ob->privs != nullptr) {
    free_string(ob->privs);
  }
  if (!(f.type(1) == T_STRING)) {
    ob->privs = nullptr;
  } else {
    ob->privs = make_shared_string(f.string(1));
  }
  f.ret_void();
}
#endif

#ifdef F_SHADOW
void f_shadow(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  object_t* ob = f.object(0);

  if (!f.number(1)) {
    ob = ob->shadowed;
    if (ob) {
      f.ret(ob);
    } else {
      f.ret(0);
    }
    return;
  }
  if (ob == current_object) {
    error("shadow: Can't shadow self\n");
  }
  if (validate_shadowing(ob)) {
    if (current_object->flags & O_DESTRUCTED) {
      f.ret(0);
      return;
    }
    /*
     * The shadow is entered first in the chain.
     */
    while (ob->shadowed) {
      ob = ob->shadowed;
    }
    current_object->shadowing = ob;
    ob->shadowed = current_object;
    f.ret(ob);
    return;
  }
  f.ret(0);
}
#endif

#ifdef F_SHOUT
void f_shout(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  shout_string(f.string(0));
  f.ret_void();
}

#endif

#ifdef F_SHUTDOWN
void f_shutdown(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  if (num_arg) {
    shutdownMudOS(f.number(0));
    f.done();
  } else {
    shutdownMudOS(0);
    f.ret(0);
  }
}
#endif

#ifdef F_SIZEOF
void f_sizeof(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  size_t i;

  switch (f.type(0)) {
    case T_CLASS:
      i = f.array(0)->size;
      break;
    case T_ARRAY:
      i = f.array(0)->size;
      break;
    case T_MAPPING:
      i = MAP_COUNT(f.mapping(0));
      break;
    case T_BUFFER:
      i = f.buffer(0)->size;
      break;
    case T_STRING: {
      // Pure ASCII (the overwhelming majority of mudlib text) has one
      // grapheme cluster per byte, and counted strings remember that in
      // their block header -- so this is O(1) with no iterator at all.
      if (u8_string_is_ascii_cached(f.string(0), f.string_len(0),
                                    (f.subtype(0) & STRING_COUNTED) != 0)) {
        i = f.string_len(0);
        f.ret(static_cast<LPC_INT>(i));
        return;
      }
      EGCSmartIterator iter(f.string(0), f.string_len(0));
      if (!iter.ok()) {
        error("f_sizeof: Invalid UTF8 string!");
      }
      i = iter.count();
      break;
    }
    default:
      i = 0;
  }
  f.ret(static_cast<LPC_INT>(i));
}
#endif

#ifdef F_SNOOP
void f_snoop(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  /*
   * This one takes a variable number of arguments. It returns 0 or an
   * object.
   */
  if (num_arg == 1) {
    if (!new_set_snoop(f.object(0), nullptr) || (f.object(0)->flags & O_DESTRUCTED)) {
      f.ret(0);
    } else {
      f.done();
    }
  } else {
    if (!new_set_snoop(f.object(0), f.object(1)) || (f.object(1)->flags & O_DESTRUCTED)) {
      f.ret(0);
    } else {
      svalue_t v = f.steal(1);
      f.ret(v);
    }
  }
}
#endif

#ifdef F_SPRINTF
void f_sprintf(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  char* s = string_print_formatted(f.string(0), num_arg - 1, f.argp(1));
  if (!s) {
    f.ret_str_constant("");
  } else {
    f.ret_str_malloced(s);
  }
}
#endif

#ifdef F_STAT
void f_stat(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  struct stat buf;
  const char* path;
  array_t* v;
  object_t* ob;

  // Capture the flag before check_valid_path(): the valid_read apply it
  // runs pushes its arguments right where the flag sits (issue #1271).
  LPC_INT const flag = f.number(1);
  f.pop_back(1);

  path = check_valid_path(f.string(0), current_object, "stat", 0);
  if (!path) {
    f.ret(0);
    return;
  }
  if (stat(path, &buf) != -1) {
    if (buf.st_mode & S_IFREG) { /* if a regular file */
      v = allocate_empty_array(4);
      v->item[0].type = T_NUMBER;
      v->item[0].subtype = 0;
      v->item[0].u.number = buf.st_size;
      v->item[1].type = T_NUMBER;
      v->item[1].subtype = 0;
      v->item[1].u.number = buf.st_mtime;
      v->item[2].type = T_NUMBER;
      v->item[2].subtype = 0;
      ob = find_object2(path);
      if (ob && !object_visible(ob)) {
        ob = nullptr;
      }
      if (ob) {
        v->item[2].u.number = ob->load_time;
      } else {
        v->item[2].u.number = 0;
      }

      v->item[3].type = T_NUMBER;
      v->item[3].subtype = 0;
      v->item[3].u.number = buf.st_ctime;

      f.ret_refed(v);
      return;
    }
  }
  v = get_dir(f.string(0), flag);
  if (v) {
    f.ret_refed(v);
  } else {
    f.ret(0);
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

void f_strsrch(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  auto* arg1 = f.argp(0);
  auto* arg2 = f.argp(1);
  auto* arg3 = f.argp(2);

  // fast track empty string search
  if (arg1->u.string[0] == '\0') {
    auto ret = -1;
    if ((arg2->type == T_STRING && arg2->u.string[0] == '\0') ||
        (arg2->type == T_NUMBER && arg2->u.number == '\0')) {
      ret = 0;
    }

    f.ret(ret);
    return;
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
          arg3->u.number == 0 ? strchr(arg1->u.string, single) : strrchr(arg1->u.string, single);
      auto pos = res == nullptr ? -1 : (const char*)res - arg1->u.string;

      EGCIterator iter(arg1->u.string, SVALUE_STRLEN(arg1));
      auto ret = pos == -1 || !iter.ok() ? -1 : u8_offset_to_egc_index(iter, pos);

      f.ret(ret);
      return;
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

    auto pos = u8_egc_find_as_offset(iter, find, find_len, arg3->u.number != 0);
    ret = pos == -1 ? -1 : u8_offset_to_egc_index(iter, pos);
  }

  f.ret(ret);
} /* strsrch */
#endif

#ifdef F_STRCMP
void f_strcmp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  f.ret(strcmp(f.string(0), f.string(1)));
}

#endif

#ifdef F_STRINGP
void f_stringp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  f.ret(f.type(0) == T_STRING);
}

#endif

#ifdef F_BUFFERP
void f_bufferp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  f.ret(f.type(0) == T_BUFFER);
}

#endif

#ifdef F_TELL_OBJECT
void f_tell_object(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  tell_object(f.object(0), f.string(1), f.string_len(1));
  f.ret_void();
}

#endif

#ifdef F_TELL_ROOM
void f_tell_room(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  array_t* avoid;
  static array_t vtmp = {1,
#ifdef DEBUGMALLOC_EXTENSIONS
                         1,
#endif
                         1,
#ifdef PACKAGE_MUDLIB_STATS
                         {(mudlib_stats_t*)nullptr, (mudlib_stats_t*)nullptr}
#endif
  };
  svalue_t* arg = f.argp(0);
  object_t* ob;

  if (arg->type == T_OBJECT) {
    ob = arg[0].u.ob;
  } else { /* must be a string... */
    ob = find_object(arg[0].u.string);
    if (!ob || !object_visible(ob)) {
      error("Bad argument 1 to tell_room()\n");
    }
  }

  if (num_arg == 2) {
    avoid = &the_null_array;
  } else {
    if (arg[2].type == T_OBJECT) {
      vtmp.item[0].type = T_OBJECT;
      vtmp.item[0].u.ob = arg[2].u.ob;
      avoid = &vtmp;
    } else {
      avoid = arg[2].u.arr;
    }
  }

  tell_room(ob, &arg[1], avoid);
  f.ret_void();
}
#endif

#ifdef F_TEST_BIT
void f_test_bit(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  LPC_INT const ind = f.number(1);

  if (ind / 6 >= f.string_len(0)) {
    f.ret(0);
    return;
  }
  if (ind < 0) {
    error("Bad argument 2 (negative) to test_bit().\n");
  }
  if ((f.string(0)[ind / 6] - ' ') & (1 << (ind % 6))) {
    f.ret(1);
  } else {
    f.ret(0);
  }
}
#endif

#ifdef F_NEXT_BIT
void f_next_bit(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  LPC_INT const start = f.number(1);
  int const len = f.string_len(0);
  int which, bit = 0, value;

  if (!len || start / 6 >= len) {
    f.ret(-1);
    return;
  }
  /* Find the next bit AFTER start */
  if (start > 0) {
    if (start % 6 == 5) {
      which = (start / 6) + 1;
      value = f.string(0)[which] - ' ';
    } else {
      /* we have a partial byte to check */
      which = start / 6;
      bit = 0x3f - ((1 << ((start % 6) + 1)) - 1);
      value = (f.string(0)[which] - ' ') & bit;
    }
  } else {
    which = 0;
    value = *f.string(0) - ' ';
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
    value = f.string(0)[which] - ' ';
  }

  f.ret(bit);
}
#endif

#ifdef F__THIS_OBJECT
void f__this_object(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  f.ret(current_object);
}

#endif

#ifdef F_THIS_PLAYER
void f_this_player(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  if (f.number(0)) {
    if (current_interactive) {
      f.ret(current_interactive);
    } else {
      f.set_top_number(0);
      f.done();
    }
  } else {
    if (command_giver) {
      f.ret(command_giver);
    } else {
      f.done();
    }
  }
}
#endif

int playerchanged;

#ifdef F_SET_THIS_PLAYER
void f_set_this_player(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  if (f.type(0) == T_NUMBER) {
    set_command_giver(0);
  } else {
    set_command_giver(f.object(0));
  }
  f.ret_void();
  playerchanged = 1;
}
#endif

#ifdef F_THROW
void f_throw(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  free_svalue(&catch_value, "f_throw");
  catch_value = f.steal(0);
  f.pop_back(1);
  f.done();
  throw_error(); /* do the longjump, with extra checks... */
}
#endif

#ifdef F__TO_FLOAT
void f__to_float(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  LPC_FLOAT temp = 0;

  switch (f.type(0)) {
    case T_NUMBER:
      temp = f.number(0);
      f.set_top_real(temp);
      break;
    case T_STRING:
      temp = strtod(f.string(0), nullptr);
      f.set_top_real(temp);
  }
  f.done();
}
#endif

#ifdef F__TO_BUFFER
void f__to_buffer(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  switch (f.type(0)) {
    case T_BUFFER:
      /* already a buffer */
      f.done();
      break;
    case T_STRING:
    case T_ARRAY: {
      /* string -> its raw UTF-8 bytes; array -> ints 0..255, one byte
       * each (errors on any other item). Also the compile-time promotion
       * behind 'buffer b = str' and 'b += str'. */
      buffer_t* b = svalue_to_buffer_bytes(f.argp(0));
      f.ret_refed(b);
      break;
    }
    default:
      bad_argument(f.argp(0), T_STRING | T_BUFFER | T_ARRAY, 1, F__TO_BUFFER);
  }
}
#endif

#ifdef F__TO_INT
void f__to_int(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  LPC_INT temp = 0;
  switch (f.type(0)) {
    case T_REAL:
      temp = f.real(0);
      f.set_top_number(temp);
      f.done();
      break;
    case T_STRING: {
      char* p;

      temp = strtoll(f.string(0), &p, 10);
      if (*p) {
        /* have to be a little careful here.  Checkign if p ==
         * f.string(0) isn't good enough.
         *
         * Odd cases:
         * to_int("  foo")  // p == f.string(0) + 2
         *
         * POSIX guarantees the strtoll() works in terms of isspace(),
         * though.  If there is something other than whitespace, then
         * there was a valid character consistent with the base,
         * so we were successful.
         *
         * (note: this means to_int("10x") == 10.  If you want to
         *  detect trailing garbage, use sscanf(str, "%d%s", ...).
         */
        while (p > f.string(0) && uisspace(*(p - 1))) {
          p--;
        }

        if (p == f.string(0)) {
          f.ret_undefined();
          break;
        }
      }
      f.set_top_number(temp);
      f.done();
      break;
    }
    case T_BUFFER:
      if (f.buffer(0)->size < sizeof(int)) {
        f.ret(0);
      } else {
        int hostint, netint;

        memcpy(reinterpret_cast<char*>(&netint), f.buffer(0)->item, sizeof(int));
        hostint = ntohl(netint);
        f.ret(hostint);
      }
  }
}
#endif

#ifdef F_TYPEOF
void f_typeof(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  const char* t = type_name(f.type(0));
  f.ret_str_constant(t);
}

#endif

#ifdef F_UNDEFINEDP
void f_undefinedp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  if (f.type(0) == T_NUMBER) {
    f.ret(!f.number(0) && (f.subtype(0) == T_UNDEFINED));
  } else {
    f.ret(0);
  }
}
#endif

#ifdef F_UPTIME
void f_uptime(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  f.ret(static_cast<LPC_INT>(get_current_time() - boot_time));
}

#endif

#ifdef F_USERP
void f_userp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i = f.object(0)->flags & O_ONCE_INTERACTIVE;
  f.ret(i != 0);
}

#endif

#ifdef F_USERS
void f_users(int num_arg) {
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

    DEBUG_CHECK(i >= total, "Driver BUG: f_users(num_arg)");

    ret->item[i].type = T_OBJECT;
    ret->item[i].u.ob = user->ob;
    add_ref(user->ob, "f_users");
    i++;
  }

  push_refed_array(ret);
}
#endif

#ifdef F_WIZARDP
void f_wizardp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i = f.object(0)->flags & O_IS_WIZARD;
  f.ret(i != 0);
}

#endif

#ifdef F_VIRTUALP
void f_virtualp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i = f.object(0)->flags & O_VIRTUAL;
  f.ret(i != 0);
}

#endif

#ifdef F_WRITE
void f_write(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  do_write(f.argp(0));
  f.ret_void();
}

#endif

#ifdef F_WRITE_BYTES
void f_write_bytes(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i = 0;

  switch (f.type(2)) {
    case T_NUMBER: {
      int netint;
      char* netbuf;

      if (!f.number(2)) {
        bad_arg(3, F_WRITE_BYTES);
      }
      /* convert to network byte-order */
      netint = htonl(f.number(2));
      netbuf = reinterpret_cast<char*>(&netint);
      i = write_bytes(f.string(0), f.number(1), netbuf, sizeof(int));
      break;
    }

    case T_BUFFER: {
      i = write_bytes(f.string(0), f.number(1),
                      reinterpret_cast<char*>(f.buffer(2)->item), f.buffer(2)->size);
      break;
    }

    case T_STRING: {
      i = write_bytes(f.string(0), f.number(1), f.string(2), f.string_len(2));
      break;
    }

    default: {
      bad_argument(f.argp(2), T_BUFFER | T_STRING | T_NUMBER, 3, F_WRITE_BYTES);
    }
  }
  f.ret(i);
}
#endif

#ifdef F_WRITE_BUFFER
void f_write_buffer(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i = 0;

  if (f.type(0) == T_STRING) {
    f.done(); /* hand the same args window to f_write_bytes */
    f_write_bytes(num_arg);
    return;
  }

  switch (f.type(2)) {
    case T_NUMBER: {
      int netint;
      char* netbuf;

      /* convert to network byte-order */
      netint = htonl(f.number(2));
      netbuf = reinterpret_cast<char*>(&netint);
      i = write_buffer(f.buffer(0), f.number(1), netbuf, sizeof(int));
      break;
    }

    case T_BUFFER: {
      i = write_buffer(f.buffer(0), f.number(1),
                       reinterpret_cast<char*>(f.buffer(2)->item), f.buffer(2)->size);
      break;
    }

    case T_STRING: {
      i = write_buffer(f.buffer(0), f.number(1), f.string(2), f.string_len(2));
      break;
    }

    default: {
      bad_argument(f.argp(2), T_BUFFER | T_STRING | T_NUMBER, 3, F_WRITE_BUFFER);
    }
  }
  f.ret(i);
}
#endif

#ifdef F_WRITE_FILE
void f_write_file(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int flags = f.number(2);
  flags = write_file(f.string(0), f.string(1), flags);
  f.ret(flags);
}
#endif

#ifdef F_DUMP_FILE_DESCRIPTORS
void f_dump_file_descriptors(int num_arg) {
  outbuffer_t out;

  outbuf_zero(&out);
  outbuf_add(&out, "ERROR: dump_file_descriptors is no longer supported.");
  outbuf_push(&out);
}
#endif

#ifdef F_RECLAIM_OBJECTS
void f_reclaim_objects(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  auto res = reclaim_objects(false);
  add_gametick_event(0, TickEvent::callback_type([] { remove_destructed_objects(); }));
  f.ret(static_cast<LPC_INT>(res));
}

#endif

#ifdef F_MEMORY_INFO
void f_memory_info(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  LPC_INT mem;
  object_t* ob;

  if (num_arg == 0) {
    LPC_INT const tot = calculate_and_maybe_print_memory_info(nullptr, -1);
    f.ret(tot);
    return;
  }
  if (f.type(0) != T_OBJECT) {
    bad_argument(f.argp(0), T_OBJECT, 1, F_MEMORY_INFO);
  }
  ob = f.object(0);
  /* as documented, object memory usage is not additive due to sharing of
   structures, so always include the program's total size even if this
   object is a clone or the program has more than one reference to it.
   There's no reliable way to determine when the program size should be
   included or not to be more accurate -- Marius, 30-Jul-2000 */
  mem = ob->prog->total_size;
  mem += (data_size(ob) + sizeof(object_t));
  f.ret(mem);
}
#endif

#ifdef F_RELOAD_OBJECT
void f_reload_object(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  reload_object(f.object(0));
  // The target may have destructed itself during reload (call___INIT()/
  // create() can call destruct(this_object())): destruct sweeps the VM
  // stack, so the arg slot may hold a plain 0 instead of the object reference by now.
  f.ret_void();
}
#endif

#ifdef F_RECOMPILE_OBJECT
void f_recompile_object(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int count = recompile_object(f.object(0));
  // The target (or one of its clones) may have destructed itself in its
  // new program's __INIT: destruct sweeps the VM stack, so the arg slot may hold
  // a plain 0 instead of the object reference by now.
  f.ret(count);
}
#endif

#ifdef F_QUERY_SHADOWING
void f_query_shadowing(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  object_t* ob;

  if ((f.type(0) == T_OBJECT) && (ob = f.object(0))->shadowing) {
    f.ret(ob->shadowing);
  } else {
    f.ret(0);
  }
}
#endif

#ifdef F_REQUEST_CLEAN_UP
void f_request_clean_up(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  object_t* ob = f.object(0);
  int success = 0;
  if (!(ob->flags & O_DESTRUCTED) && function_exists(APPLY_CLEAN_UP, ob, 1)) {
    ob->flags |= O_WILL_CLEAN_UP;
    success = 1;
  }
  f.ret(success);
}

#endif

#ifdef F_SET_CLEAN_UP
void f_set_clean_up(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  object_t* ob;

  if (num_arg == 2) {
    ob = f.object(0);
    ob->next_cleanup =
        g_current_gametick + time_to_next_gametick(std::chrono::seconds(f.number(1)));
  } else {
    ob = f.object(0);
    ob->next_cleanup = 0; /* back to the idle-time rule */
  }

  /* Same gate as at load/clone time and request_clean_up(): only objects
     that actually define clean_up() are queried by the sweep. */
  if (!(ob->flags & O_DESTRUCTED) && function_exists(APPLY_CLEAN_UP, ob, 1)) {
    ob->flags |= O_WILL_CLEAN_UP;
  }

  f.ret_void();
}
#endif

#ifdef F_SET_RESET
void f_set_reset(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  const auto time_to_reset = CONFIG_INT(__TIME_TO_RESET__);

  if (num_arg == 2) {
    f.object(0)->next_reset =
        g_current_gametick + time_to_next_gametick(std::chrono::seconds(f.number(1)));
  } else {
    f.object(0)->next_reset =
        g_current_gametick + time_to_next_gametick(std::chrono::seconds(
                                 time_to_reset / 2 + random_number(time_to_reset / 2)));
  }
  f.ret_void();
}
#endif

#ifdef F_FLOATP
void f_floatp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  f.ret(f.type(0) == T_REAL);
}

#endif

#ifdef F_FLUSH_MESSAGES
void f_flush_messages(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  if (num_arg == 1) {
    if (f.object(0)->interactive) {
      flush_message(f.object(0)->interactive);
    }
    f.ret_void();
  } else {
    flush_message_all();
    f.done();
  }
}
#endif

#ifdef F_FIRST_INVENTORY
void f_first_inventory(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  object_t* ob = first_inventory(f.argp(0));
  if (ob) {
    f.ret(ob);
  } else {
    f.ret(0);
  }
}
#endif

#ifdef F_NEXT_INVENTORY
void f_next_inventory(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  object_t* ob = f.object(0)->next_inv;

#ifdef F_SET_HIDE
  while (ob && (ob->flags & O_HIDDEN) && !object_visible(ob)) {
    ob = ob->next_inv;
  }
#endif

  if (ob) {
    f.ret(ob);
  } else {
    f.ret(0);
  }
}
#endif

#ifdef F_DEFER
void f_defer(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  auto* newlist = reinterpret_cast<struct defer_list*>(
      DMALLOC(sizeof(struct defer_list), TAG_DEFERS, "defer: new item"));

  if (CONFIG_INT(__RC_REVERSE_DEFER__)) {
    // In reverse mode, newlist always will be the last data.
    newlist->next = nullptr;
  } else {
    // In normal mode, newlist always will be the first data.
    newlist->next = csp->defers;
  }
  // Configure the new item.
  newlist->func = f.steal(0);
  f.ret_void();
  if (command_giver) {
    LPCFrame b = LPCFrame::builder();
    b.push(command_giver);
    newlist->tp = b.steal(0);
    b.pop_back(1);
    b.done();
  } else {
    newlist->tp = const0;
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
void f_crypt(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  const int sh_a512_prefix_len = 3;
  const int sh_a512_salt_len = 16;
  char salt[sh_a512_prefix_len + sh_a512_salt_len + 1];
  const char* saltp = nullptr;
  const char* choice = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

  if (f.type(1) == T_STRING) {
    // Support $1$, $2a$ (a,x,y), $5$, $6$
    if (f.string(1)[0] == '$') {
      // $2?$
      if (f.string_len(1) >= 4 && f.string(1)[1] == '2' &&
          (f.string(1)[2] == 'a' || f.string(1)[2] == 'x' || f.string(1)[2] == 'y') &&
          f.string(1)[3] == '$') {
        saltp = f.string(1);
      } else {
        if (f.string_len(1) >= 3 &&
            (f.string(1)[1] == '1' || f.string(1)[1] == '5' || f.string(1)[1] == '6') &&
            f.string(1)[2] == '$') {
          saltp = f.string(1);
        }
      }
    } else if (f.string_len(1) >= 2) {
      // Compat: Old f_crypt only use first two character as key.
      debug_message(
          "old crypt() password detected, It is only using first 2 character as key and ignore "
          "password beyond 8 characters, please upgrade to SHA512 using crypt(password) "
          "immediately.\n");
      salt[0] = f.string(1)[0];
      salt[1] = f.string(1)[1];
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
  const auto* key = f.string(0);
  auto* result = __crypt(key, saltp);
  if (result == nullptr || (result && result[0] == '*')) {
    error("Error in crypt(), check your salt");
    return;
  }
  result = string_copy(result, "f_crypt");
  f.ret_str_malloced(result);
}
#endif

#ifdef F_OLDCRYPT
void f_oldcrypt(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  const int salt_len = 8;

  const char *res, *p;
  char salt[salt_len + 1];
  const char* choice = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ./";

  if (f.type(1) == T_STRING && f.string_len(1) >= 2) {
    p = f.string(1);
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
  res = string_copy(custom_crypt(f.string(0), p, nullptr), "f_oldcrypt");
  f.ret_str_malloced(res);
}
#endif

#ifdef F_RUSAGE
void f_rusage(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
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
  f.ret_refed(m);
}

#endif

#ifdef F_STRWIDTH
void f_strwidth(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  auto width = u8_width(f.string(0), -1);
  f.ret(static_cast<LPC_INT>(width));
}

#endif
