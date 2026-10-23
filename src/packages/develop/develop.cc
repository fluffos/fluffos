#include "base/package_api.h"

#include "packages/core/file.h"
#include "packages/core/sprintf.h"
#include "packages/core/outbuf.h"

#ifdef F_DEBUG_INFO
vm::Value f_debug_info(vm::EfunArgs&, LPC_INT flag, object_t* target) {
  outbuffer_t out;

  outbuf_zero(&out);
  switch (flag) {
    case 0: {
      int i, flags;
      object_t* obj2;

      flags = target->flags;
      outbuf_addv(&out, "O_HEART_BEAT      : %s\n", flags & O_HEART_BEAT ? "TRUE" : "FALSE");
#ifndef NO_WIZARDS
      outbuf_addv(&out, "O_IS_WIZARD       : %s\n", flags & O_IS_WIZARD ? "TRUE" : "FALSE");
#endif
#ifdef NO_ADD_ACTION
      outbuf_addv(&out, "O_LISTENER        : %s\n", flags & O_LISTENER ? "TRUE" : "FALSE");
#else
      outbuf_addv(&out, "O_ENABLE_COMMANDS : %s\n", flags & O_ENABLE_COMMANDS ? "TRUE" : "FALSE");
#endif
      outbuf_addv(&out, "O_CLONE           : %s\n", flags & O_CLONE ? "TRUE" : "FALSE");
      outbuf_addv(&out, "O_VIRTUAL         : %s\n", flags & O_VIRTUAL ? "TRUE" : "FALSE");
      outbuf_addv(&out, "O_DESTRUCTED      : %s\n", flags & O_DESTRUCTED ? "TRUE" : "FALSE");
      outbuf_addv(&out, "O_ONCE_INTERACTIVE: %s\n", flags & O_ONCE_INTERACTIVE ? "TRUE" : "FALSE");
      outbuf_addv(&out, "O_RESET_STATE     : %s\n", flags & O_RESET_STATE ? "TRUE" : "FALSE");
      outbuf_addv(&out, "O_WILL_CLEAN_UP   : %s\n", flags & O_WILL_CLEAN_UP ? "TRUE" : "FALSE");
      outbuf_addv(&out, "O_WILL_RESET      : %s\n", flags & O_WILL_RESET ? "TRUE" : "FALSE");
      outbuf_addv(&out, "O_NOTIFY_DESTRUCT : %s\n", flags & O_NOTIFY_DESTRUCT ? "TRUE" : "FALSE");

      if (target->interactive) {
        outbuf_addv(&out, "O_ZMP             : %s\n",
                    target->interactive->iflags & USING_ZMP ? "TRUE" : "FALSE");
        outbuf_addv(&out, "O_GMCP            : %s\n",
                    target->interactive->iflags & USING_GMCP ? "TRUE" : "FALSE");
        outbuf_addv(&out, "O_MXP             : %s\n",
                    target->interactive->iflags & USING_MXP ? "TRUE" : "FALSE");
        outbuf_addv(&out, "O_COMPRESS        : %s\n",
                    target->interactive->iflags & USING_COMPRESS ? "TRUE" : "FALSE");
        outbuf_addv(&out, "O_MSP             : %s\n",
                    target->interactive->iflags & USING_MSP ? "TRUE" : "FALSE");
        outbuf_addv(&out, "O_MSDP            : %s\n",
                    target->interactive->iflags & USING_MSDP ? "TRUE" : "FALSE");
      }

      if (!CONFIG_INT(__RC_NO_RESETS__)) {
        outbuf_addv(&out, "next_reset  : %" PRId64 "\n", target->next_reset);
      }
#ifndef NO_LIGHT
      outbuf_addv(&out, "total light : %d\n", target->total_light);
#endif
      outbuf_addv(&out, "time_of_ref : %" PRIu64 "\n", target->time_of_ref);
      outbuf_addv(&out, "ref         : %d\n", target->ref);
#ifdef DEBUGMALLOC_EXTENSIONS
      outbuf_addv(&out, "extra_ref   : %d\n", target->extra_ref);
#endif
      outbuf_addv(&out, "name        : '/%s'\n", target->obname);
      outbuf_addv(&out, "next_all    : OBJ(/%s)\n",
                  target->next_all ? target->next_all->obname : "NULL");
      if (obj_list == target) {
        outbuf_add(&out, "This object is the head of the object list.\n");
      }
      for (obj2 = obj_list, i = 1; obj2; obj2 = obj2->next_all, i++) {
        if (obj2->next_all == target) {
          outbuf_addv(&out, "Previous object in object list: OBJ(/%s)\n", obj2->obname);
          outbuf_addv(&out, "position in object list:%d\n", i);
        }
      }
      break;
    }
    case 1:
      outbuf_addv(&out, "program ref's %d\n", target->prog->ref);
      outbuf_addv(&out, "Name /%s\n", target->prog->filename);
      outbuf_addv(&out, "program size %d\n", target->prog->program_size);
      outbuf_addv(
          &out, "function flags table %d (%lu) \n",
          target->prog->last_inherited + target->prog->num_functions_defined,
          (target->prog->last_inherited + target->prog->num_functions_defined) *
              sizeof(unsigned short));
      outbuf_addv(&out, "compiler function table %u (%" PRIu64 ") \n",
                  target->prog->num_functions_defined,
                  target->prog->num_functions_defined * sizeof(function_t));
      outbuf_addv(&out, "num strings %d\n", target->prog->num_strings);
      outbuf_addv(&out, "num vars %u (%" PRIu64 ")\n", target->prog->num_variables_defined,
                  target->prog->num_variables_defined * (sizeof(char*) + sizeof(lpc_type_t)));
      outbuf_addv(&out, "num inherits %d (%lu)\n", target->prog->num_inherited,
                  target->prog->num_inherited * sizeof(inherit_t));
      outbuf_addv(&out, "total size %d\n", target->prog->total_size);
      break;
    case 2: {
      int i;
      for (i = 0; i < target->prog->num_variables_total; i++) {
        /* inefficient, but: */
        outbuf_addv(&out, "%s: ", variable_name(target->prog, i));
        svalue_to_string(&target->variables[i], &out, 2, 0, 0);
        outbuf_add(&out, "\n");
      }
      break;
    }
    default:
      bad_arg(1, F_DEBUG_INFO);
  }
  return outbuf_value(&out);
}
#endif

#ifdef F_REFS
vm::Value f_refs(vm::EfunArgs&, vm::Arg value) {
  int r;

  switch (value.type()) {
    case T_MAPPING:
      r = value.mapping()->ref;
      break;
    case T_CLASS:
      r = value.class_rep()->ref;
      break;
    case T_ARRAY:
      r = value.array()->ref;
      break;
    case T_OBJECT:
      r = value.object()->ref;
      break;
    case T_FUNCTION:
      r = value.funptr()->hdr.ref;
      break;
    case T_BUFFER:
      r = value.buffer()->ref;
      break;
    case T_PROMISE:
      r = value.promise()->ref;
      break;
    case T_STRING:
      if (value.subtype() & STRING_COUNTED) {
        r = MSTR_REF(value.string().c_str());
      } else {
        r = 0;
      }
      break;
    default:
      r = 0;
      break;
  }
  return r - 1; /* minus 1 to compensate for being arg of refs() */
}
#endif

#ifdef F_DESTRUCTED_OBJECTS
vm::Value f_destructed_objects(vm::EfunArgs&) {
  int i;
  array_t* ret;
  object_t* ob;

  ret = allocate_empty_array(tot_dangling_object);
  ob = obj_list_dangling;

  for (i = 0; i < tot_dangling_object; i++) {
    ret->item[i].type = T_ARRAY;
    ret->item[i].u.arr = allocate_empty_array(2);
    ret->item[i].u.arr->item[0].type = T_STRING;
    ret->item[i].u.arr->item[0].subtype = STRING_SHARED;
    ret->item[i].u.arr->item[0].u.string = make_shared_string(ob->obname);
    ret->item[i].u.arr->item[1].type = T_NUMBER;
    ret->item[i].u.arr->item[1].u.number = ob->ref;

    ob = ob->next_all;
  }

  return vm::Value::refed(ret);
}
#endif

#ifdef F_DUMP_STRALLOC
void dump_stralloc(outbuffer_t*);

vm::Value f_dump_stralloc(vm::EfunArgs&, vm::StringArg file) {
  const auto* target_file = file.c_str();
  const char* fn;
  FILE* fp;
  vm::Value fnv = check_valid_path(target_file, current_object, "debugmalloc", 1);
  if (!fnv.is(T_STRING)) {
    error("Invalid path '%s' for writing.\n", target_file);
  }
  fn = fnv->u.string;
  fp = fopen(fn, "w");
  if (!fp) {
    error("Unable to open %s for writing.\n", fn);
  }

  outbuffer_t out;
  outbuf_zero(&out);

  dump_stralloc(&out);

  outbuf_fix(&out);

  fputs(out.buffer, fp);
  fclose(fp);

  return outbuf_value(&out);
}
#endif

#ifdef F_DUMP_JEMALLOC
#ifdef HAVE_JEMALLOC
#define JEMALLOC_MANGLE
#include <jemalloc/jemalloc.h>  // for mallctl
#endif

void f_dump_jemalloc(vm::EfunArgs&) {
#ifdef HAVE_JEMALLOC
  mallctl("prof.dump", nullptr, nullptr, nullptr, 0);
  malloc_stats_print(nullptr, nullptr, "");
#else
  debug_message("Jemalloc is disabled, dump_jemalloc() has no effect.\n");
#endif
}
#endif

#if (defined(DEBUGMALLOC) && defined(DEBUGMALLOC_EXTENSIONS))
#ifdef F_DEBUGMALLOC
vm::Value f_debugmalloc(vm::EfunArgs&, vm::StringArg file, LPC_INT mask) {
  char* res;

  res = dump_debugmalloc(file.c_str(), mask);
  return vm::Value::str_malloced(res);
}
#endif

#ifdef F_SET_MALLOC_MASK
void f_set_malloc_mask(vm::EfunArgs&, LPC_INT mask) { set_malloc_mask(mask); }
#endif

#ifdef F_CHECK_MEMORY
vm::Value check_all_blocks(int); /* checkmemory.cc */

vm::Value f_check_memory(vm::EfunArgs&, LPC_INT flag) {
  return check_all_blocks(static_cast<int>(flag));
}
#endif

#ifdef F_FIND_ORPHANED_CYCLES
vm::Value f_find_orphaned_cycles(vm::EfunArgs&, LPC_INT collect_arg) {
  // any nonzero LPC int means collect (don't truncate the 64-bit argument)
  int const collect = collect_arg != 0;
  return md_scan_orphaned_cycles(collect, nullptr);
}
#endif
#endif /* (defined(DEBUGMALLOC) && \
        * defined(DEBUGMALLOC_EXTENSIONS)) */

#ifdef F_TRACE
vm::Value f_trace(vm::EfunArgs&, LPC_INT level) {
  int ot = -1;

  if (command_giver && command_giver->interactive) {
    ot = command_giver->interactive->trace_level;
    command_giver->interactive->trace_level = level;
  }
  return ot;
}
#endif

#ifdef F_TRACEPREFIX
vm::Value f_traceprefix(vm::EfunArgs&, vm::Arg prefix) {
  const char* old = nullptr;

  if (command_giver && command_giver->interactive) {
    old = command_giver->interactive->trace_prefix;
    if (prefix.is(T_STRING)) {
      const char* p = prefix.string().c_str();
      if (*p == '/') {
        p++;
      }

      command_giver->interactive->trace_prefix = make_shared_string(p);
    } else {
      command_giver->interactive->trace_prefix = nullptr;
    }
  }
  if (old) {
    vm::Value result = vm::Value::str_malloced(add_slash(old));
    free_string(old);
    return result;
  }
  return 0;
}
#endif
