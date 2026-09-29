#include "base/std.h"

#include "base/internal/tracing.h"
#include "vm/vm.h"
#include "vm/internal/base/machine.h"
#include "compiler/internal/lexer.h"  // for instrs, FIXME

#include "packages/core/replace_program.h"

#include <cstring>
#include <unordered_map>
#include <vector>

/* A named-function reference with no bound arguments -- `cb`, `(: cb :)`,
 * `write`, a simul_efun name -- evaluates to ONE function pointer per
 * (owner, function) for as long as that pointer is alive, so `cb == cb` holds
 * and `arr -= ({ cb })` removes what `arr += ({ cb })` added. Every equality
 * site in the driver (==, array set operations, member_array, mapping keys)
 * compares function values by pointer, so interning at construction is the
 * one place this has to be decided.
 *
 * The table is WEAK: it holds no reference. An entry lives exactly as long as
 * the pointer it names, and dealloc_funp() removes it. That is all the
 * semantics need -- two evaluations can only be compared while something
 * holds the first one, which keeps its entry -- and it means the table
 * creates no owner <-> pointer cycle and needs no mark in check_memory().
 *
 * Entries are grouped by owner so recompile_object() can find an object's
 * pointers without scanning everyone's. The key is safe to trust for the
 * entry's lifetime: the pointer holds a ref on its owner (and an FP_LOCAL on
 * its program), so neither address can be reused while the entry exists. The
 * one path that drops an owner ref early, reclaim_objects(), un-interns first.
 *
 * FP_LOCAL keys include the program and prog_generation the index is relative
 * to. recompile_object() re-resolves the owner's local-function pointers by
 * name against the new program (refresh_named_funps()) and re-keys them
 * before any of the new program's code runs, so a reference evaluated after
 * the recompile finds the same pointer. Efun and simul_efun pointers don't
 * depend on the owner's layout; their keys never change.
 *
 * Addresses are stored XOR-masked (as md.h's chain pointers are) so the
 * table does not make a leaked pointer or its owner look reachable to
 * LeakSanitizer. */
namespace {

const uintptr_t kNamedFunpMask = static_cast<uintptr_t>(0xA5A5A5A5A5A5A5A5ull);

uintptr_t mask_ptr(const void* p) { return reinterpret_cast<uintptr_t>(p) ^ kNamedFunpMask; }

struct NamedFunpKey {
  uintptr_t prog;
  uint32_t gen;
  short kind;
  short index;

  bool operator==(const NamedFunpKey& o) const {
    return prog == o.prog && gen == o.gen && kind == o.kind && index == o.index;
  }
};

struct NamedFunpKeyHash {
  size_t operator()(const NamedFunpKey& k) const {
    size_t h = std::hash<uintptr_t>{}(k.prog);
    h ^= std::hash<uint32_t>{}(k.gen) + 0x9e3779b97f4a7c15ull + (h << 6) + (h >> 2);
    h ^= (static_cast<size_t>(k.kind) << 16 | static_cast<uint16_t>(k.index)) +
         0x9e3779b97f4a7c15ull + (h << 6) + (h >> 2);
    return h;
  }
};

using OwnerFunps = std::unordered_map<NamedFunpKey, uintptr_t, NamedFunpKeyHash>;

/* Never destroyed: pointers can still be freed while the driver tears down
 * after static destructors have started. */
std::unordered_map<uintptr_t, OwnerFunps>& named_funps =
    *new std::unordered_map<uintptr_t, OwnerFunps>();

NamedFunpKey named_funp_key(short kind, int index, program_t* prog, uint32_t gen) {
  return NamedFunpKey{prog ? mask_ptr(prog) : 0, gen, kind, static_cast<short>(index)};
}

/* The key fp was interned under, if it is the kind of pointer that is. */
bool named_funp_key_of(const funptr_t* fp, NamedFunpKey* key) {
  if (fp->hdr.args || !fp->hdr.owner) {
    return false;
  }
  switch (fp->hdr.type) {
    case FP_LOCAL | FP_NOT_BINDABLE:
      *key = named_funp_key(FP_LOCAL, fp->f.local.index, fp->f.local.prog, fp->hdr.owner_gen);
      return true;
    case FP_EFUN:
      *key = named_funp_key(FP_EFUN, fp->f.efun.index, nullptr, 0);
      return true;
    case FP_SIMUL:
      *key = named_funp_key(FP_SIMUL, fp->f.simul.index, nullptr, 0);
      return true;
    default:
      return false;
  }
}

/* The interned pointer for `key` (no reference taken), or nullptr. */
funptr_t* peek_named_funp(object_t* owner, const NamedFunpKey& key) {
  auto bucket = named_funps.find(mask_ptr(owner));
  if (bucket == named_funps.end()) {
    return nullptr;
  }
  auto it = bucket->second.find(key);
  if (it == bucket->second.end()) {
    return nullptr;
  }
  return reinterpret_cast<funptr_t*>(it->second ^ kNamedFunpMask);
}

/* Returns the live interned pointer for `key` with a new reference taken, or
 * nullptr. */
funptr_t* find_named_funp(object_t* owner, const NamedFunpKey& key) {
  funptr_t* fp = peek_named_funp(owner, key);
  if (fp) {
    fp->hdr.ref++;
  }
  return fp;
}

void intern_named_funp(object_t* owner, const NamedFunpKey& key, funptr_t* fp) {
  named_funps[mask_ptr(owner)][key] = mask_ptr(fp);
}

/* Remove fp's entry if fp is what the entry names. Returns whether it was. */
bool unintern_named_funp(funptr_t* fp) {
  NamedFunpKey key;
  if (named_funps.empty() || !named_funp_key_of(fp, &key)) {
    return false;
  }
  auto bucket = named_funps.find(mask_ptr(fp->hdr.owner));
  if (bucket == named_funps.end()) {
    return false;
  }
  /* Only the pointer the entry names may remove it: bind() copies, and a
   * pointer whose bound args were detached by the cycle breakers, match a
   * key without being the interned value. */
  auto it = bucket->second.find(key);
  if (it == bucket->second.end() || it->second != mask_ptr(fp)) {
    return false;
  }
  bucket->second.erase(it);
  if (bucket->second.empty()) {
    named_funps.erase(bucket);
  }
  return true;
}

/* The program that defines runtime function slot `slot` of `prog`, and the
 * slot's index in that program's function_table. The walk is by POSITION --
 * inherited slots occupy [0, last_inherited), partitioned by each inherit's
 * function_index_offset -- rather than by FUNC_INHERITED, because an
 * override's inherited slot is a FUNC_ALIAS word whose low bits hold the
 * alias target, not flags. So unlike get_function_at_index() this never
 * follows an alias: the slot still names the inherited definition. */
std::pair<program_t*, int> defining_function(program_t* prog, int slot) {
  while (slot < prog->last_inherited) {
    int low = 0;
    int high = prog->num_inherited - 1;
    while (high > low) {
      int mid = (low + high + 1) >> 1;
      if (prog->inherit[mid].function_index_offset > slot) {
        high = mid - 1;
      } else {
        low = mid;
      }
    }
    slot -= prog->inherit[low].function_index_offset;
    prog = prog->inherit[low].prog;
  }
  return {prog, slot - prog->last_inherited};
}

bool same_function(program_t* prog, int slot, const char* file, const char* name) {
  auto def = defining_function(prog, slot);
  const char* funcname = def.first->function_table[def.second].funcname;
  return funcname && !strcmp(funcname, name) && !strcmp(def.first->filename, file);
}

/* The slot in new_prog that names the same function old_slot named in
 * old_prog: same defining program file, same function name, and the same
 * occurrence among slots matching both -- a program inherited twice has one
 * copy of each function per inherit, each with its own variables. -1 when
 * the function is gone. */
int find_same_function(program_t* old_prog, int old_slot, program_t* new_prog) {
  auto def = defining_function(old_prog, old_slot);
  const char* file = def.first->filename;
  const char* name = def.first->function_table[def.second].funcname;
  if (!name) {
    return -1;
  }

  int occurrence = 0;
  for (int s = 0; s < old_slot; s++) {
    if (same_function(old_prog, s, file, name)) {
      occurrence++;
    }
  }
  int total = new_prog->last_inherited + new_prog->num_functions_defined;
  for (int s = 0; s < total; s++) {
    if (same_function(new_prog, s, file, name) && occurrence-- == 0) {
      return s;
    }
  }
  return -1;
}

}  // namespace

void unintern_funp(funptr_t* fp) { unintern_named_funp(fp); }

bool refresh_local_funp(funptr_t* fp) {
  object_t* owner = fp->hdr.owner;
  if (!owner || (owner->flags & O_DESTRUCTED)) {
    return false;
  }
  if (fp->hdr.owner_gen == owner->prog_generation) {
    return true;
  }
  program_t* old_prog = fp->f.local.prog;
  program_t* new_prog = owner->prog;
  int slot = find_same_function(old_prog, fp->f.local.ref_index, new_prog);
  if (slot < 0) {
    return false;
  }
  int index = slot;
  if (new_prog->function_flags[index] & FUNC_ALIAS) {
    index = new_prog->function_flags[index] & ~FUNC_ALIAS;
  }

  bool const interned = unintern_named_funp(fp);

  new_prog->func_ref++;
  fp->f.local.prog = new_prog;
  fp->f.local.index = index;
  fp->f.local.ref_index = slot;
  fp->hdr.owner_gen = owner->prog_generation;

  old_prog->func_ref--;
  if (!old_prog->func_ref && !old_prog->ref) {
    deallocate_program(old_prog);
  }

  if (interned) {
    NamedFunpKey key;
    named_funp_key_of(fp, &key);
    /* recompile_object() re-keys before any new code runs, so nothing can
     * already hold this key; if something ever did, it stays canonical and
     * this pointer simply stops being the interned one. */
    if (!peek_named_funp(owner, key)) {
      intern_named_funp(owner, key, fp);
    }
  }
  return true;
}

void refresh_named_funps(object_t* ob) {
  auto bucket = named_funps.find(mask_ptr(ob));
  if (bucket == named_funps.end()) {
    return;
  }
  // Collected first: refreshing re-keys entries in this very bucket.
  std::vector<funptr_t*> locals;
  for (const auto& entry : bucket->second) {
    auto* fp = reinterpret_cast<funptr_t*>(entry.second ^ kNamedFunpMask);
    if (fp->hdr.type == (FP_LOCAL | FP_NOT_BINDABLE)) {
      locals.push_back(fp);
    }
  }
  for (funptr_t* fp : locals) {
    refresh_local_funp(fp);
  }
}

void dealloc_funp(funptr_t* fp) {
  program_t* prog = nullptr;

  switch (fp->hdr.type) {
    case FP_LOCAL | FP_NOT_BINDABLE:
      prog = fp->f.local.prog;
      break;
    case FP_FUNCTIONAL:
    case FP_FUNCTIONAL | FP_NOT_BINDABLE:
      prog = fp->f.functional.prog;
      break;
  }

  unintern_funp(fp);

  if (fp->hdr.owner) {
    free_object(&fp->hdr.owner, "free_funp");
  }
  if (fp->hdr.args) {
    free_array(fp->hdr.args);
  }

  if (prog) {
    prog->func_ref--;
    debug(d_flag, "subtr func ref /%s: now %i\n", prog->filename, prog->func_ref);
    if (!prog->func_ref && !prog->ref) {
      deallocate_program(prog);
    }
  }

  FREE(fp);
}

void free_funp(funptr_t* fp) {
  fp->hdr.ref--;
  if (fp->hdr.ref > 0) {
    return;
  }
  dealloc_funp(fp);
}

void push_refed_funp(funptr_t* fp) {
  STACK_INC;
  sp->type = T_FUNCTION;
  sp->u.fp = fp;
}

void push_funp(funptr_t* fp) {
  STACK_INC;
  sp->type = T_FUNCTION;
  sp->u.fp = fp;
  fp->hdr.ref++;
}

/* num_arg args are on the stack, and the args from the array vec should be
 * put in front of them.  This is so that the order of arguments is logical.
 *
 * evaluate( (: f, a :), b) -> f(a,b) and not f(b, a) which would happen
 * if we simply pushed the args from vec at this point.  (Note that the
 * old function pointers are broken in this regard)
 */
int merge_arg_lists(int num_arg, array_t* arr, int start) {
  int num_arr_arg = arr->size - start;
  svalue_t* sptr;

  if (num_arr_arg) {
    CHECK_STACK_OVERFLOW(num_arr_arg);
    sptr = (sp += num_arr_arg);
    if (num_arg) {
      /* We need to do some stack movement so that the order
         of arguments is logical */
      while (num_arg--) {
        *sptr = *(sptr - num_arr_arg);
        sptr--;
      }
    }
    num_arg = arr->size;
    while (--num_arg >= start) {
      assign_svalue_no_free(sptr--, &arr->item[num_arg]);
    }
    /* could just return num_arr_arg if num_arg is 0 but .... -Sym */
    return (sp - sptr);
  }
  return num_arg;
}

funptr_t* make_efun_funp(int opcode, svalue_t* args) {
  funptr_t* fp;
  bool const named = args->type != T_ARRAY;
  NamedFunpKey key{};

  if (named) {
    key = named_funp_key(FP_EFUN, opcode, nullptr, 0);
    if ((fp = find_named_funp(current_object, key))) {
      return fp;
    }
  }

  fp = reinterpret_cast<funptr_t*>(DMALLOC(sizeof(funptr_t), TAG_FUNP, "make_efun_funp"));
  fp->hdr.owner = current_object;
  add_ref(current_object, "make_efun_funp");
  fp->hdr.owner_gen = current_object->prog_generation;
  fp->hdr.type = FP_EFUN;

  fp->f.efun.index = opcode;

  if (!named) {
    fp->hdr.args = args->u.arr;
    args->u.arr->ref++;
  } else {
    fp->hdr.args = nullptr;
  }

  fp->hdr.ref = 1;
  if (named) {
    intern_named_funp(current_object, key, fp);
  }
  return fp;
}

funptr_t* make_lfun_funp(int index, svalue_t* args) {
  funptr_t* fp;
  int newindex;

  if (replace_program_pending(current_object)) {
    error(
        "cannot bind an lfun fp to an object with a pending "
        "replace_program()\n");
  }

  int const ref_index = index + function_index_offset;
  newindex = ref_index;
  if (current_object->prog->function_flags[newindex] & FUNC_ALIAS) {
    newindex = current_object->prog->function_flags[newindex] & ~FUNC_ALIAS;
  }

  bool const named = args->type != T_ARRAY;
  NamedFunpKey key{};

  if (named) {
    key = named_funp_key(FP_LOCAL, newindex, current_object->prog,
                         current_object->prog_generation);
    if ((fp = find_named_funp(current_object, key))) {
      return fp;
    }
  }

  fp = reinterpret_cast<funptr_t*>(DMALLOC(sizeof(funptr_t), TAG_FUNP, "make_lfun_funp"));
  fp->hdr.owner = current_object;
  add_ref(current_object, "make_lfun_funp");
  fp->hdr.owner_gen = current_object->prog_generation;
  fp->hdr.type = FP_LOCAL | FP_NOT_BINDABLE;

  fp->f.local.prog = current_object->prog;
  fp->f.local.prog->func_ref++;
  debug(d_flag, "add func ref /%s: now %i\n", fp->f.local.prog->filename,
        fp->f.local.prog->func_ref);

  fp->f.local.index = newindex;
  fp->f.local.ref_index = ref_index;

  if (!named) {
    fp->hdr.args = args->u.arr;
    args->u.arr->ref++;
  } else {
    fp->hdr.args = nullptr;
  }

  fp->hdr.ref = 1;
  if (named) {
    intern_named_funp(current_object, key, fp);
  }
  return fp;
}

funptr_t* make_simul_funp(int index, svalue_t* args) {
  funptr_t* fp;
  bool const named = args->type != T_ARRAY;
  NamedFunpKey key{};

  if (named) {
    key = named_funp_key(FP_SIMUL, index, nullptr, 0);
    if ((fp = find_named_funp(current_object, key))) {
      return fp;
    }
  }

  fp = reinterpret_cast<funptr_t*>(DMALLOC(sizeof(funptr_t), TAG_FUNP, "make_simul_funp"));
  fp->hdr.owner = current_object;
  add_ref(current_object, "make_simul_funp");
  fp->hdr.owner_gen = current_object->prog_generation;
  fp->hdr.type = FP_SIMUL;

  fp->f.simul.index = index;

  if (!named) {
    fp->hdr.args = args->u.arr;
    args->u.arr->ref++;
  } else {
    fp->hdr.args = nullptr;
  }

  fp->hdr.ref = 1;
  if (named) {
    intern_named_funp(current_object, key, fp);
  }
  return fp;
}

funptr_t* make_functional_funp(short num_arg, short num_local, short len, svalue_t* args,
                               int flag) {
  funptr_t* fp;

  if (replace_program_pending(current_object)) {
    error(
        "cannot bind a functional to an object with a pending "
        "replace_program()\n");
  }

  fp = reinterpret_cast<funptr_t*>(DMALLOC(sizeof(funptr_t), TAG_FUNP, "make_functional_funp"));
  fp->hdr.owner = current_object;
  add_ref(current_object, "make_functional_funp");
  fp->hdr.owner_gen = current_object->prog_generation;
  fp->hdr.type = FP_FUNCTIONAL + flag;

  current_prog->func_ref++;
  debug(d_flag, "add func ref /%s: now %i\n", current_prog->filename, current_prog->func_ref);

  fp->f.functional.prog = current_prog;
  fp->f.functional.offset = pc - current_prog->program;
  fp->f.functional.num_arg = num_arg;
  fp->f.functional.num_local = num_local;
  fp->f.functional.fio = function_index_offset;
  fp->f.functional.vio = variable_index_offset;
  pc += len;

  if (args && args->type == T_ARRAY) {
    fp->hdr.args = args->u.arr;
    args->u.arr->ref++;
    fp->f.functional.num_arg += args->u.arr->size;
  } else {
    fp->hdr.args = nullptr;
  }

  fp->hdr.ref = 1;
  md_record_ref_journal(PTR_TO_NODET(fp), true, 1, "make_functional_funp");
  return fp;
}

typedef void (*func_t)(void);
extern func_t efun_table[];

svalue_t* call_function_pointer(funptr_t* funp, int num_arg) {
  array_t* v;

  if (!funp->hdr.owner || (funp->hdr.owner->flags & O_DESTRUCTED)) {
    error("Owner (/%s) of function pointer is destructed.\n",
          (funp->hdr.owner ? funp->hdr.owner->obname : "(null)"));
  }
  /* FP_LOCAL indices and FP_FUNCTIONAL variable offsets are relative to
     the owner's program layout when the pointer was made; after a
     recompile_object() they would run the wrong function or scribble over the
     re-laid-out variables. Fail cleanly instead. */
  switch (funp->hdr.type & FP_MASK) {
    case FP_LOCAL:
      /* A named function survives the recompile if the new program still has
       * it: re-resolve by name, as recompile_object() does for variables. The
       * owner's argument-less pointers were already refreshed during the
       * recompile; this catches the ones carrying bound arguments. */
      if (funp->hdr.owner_gen != funp->hdr.owner->prog_generation && !refresh_local_funp(funp)) {
        error("Stale function pointer: owner /%s was recompiled since it was created.\n",
              funp->hdr.owner->obname);
      }
      break;
    case FP_FUNCTIONAL:
      if (funp->hdr.owner_gen != funp->hdr.owner->prog_generation) {
        error("Stale function pointer: owner /%s was recompiled since it was created.\n",
              funp->hdr.owner->obname);
      }
      break;
    default:
      break;
  }
  setup_fake_frame(funp);
  if ((v = funp->hdr.args)) {
    check_for_destr(v);
    num_arg = merge_arg_lists(num_arg, v, 0);
  }

  switch (funp->hdr.type) {
    case FP_SIMUL:
      call_simul_efun(funp->f.simul.index, num_arg);
      break;
    case FP_EFUN: {
      int i, def;
      fp = sp - num_arg + 1;

      i = funp->f.efun.index;
      if (num_arg == instrs[i].min_arg - 1 && ((def = instrs[i].Default) != DEFAULT_NONE)) {
        if (def == DEFAULT_THIS_OBJECT) {
          push_object(current_object);
        } else {
          push_number(def);
        }
        num_arg++;
      } else if (num_arg < instrs[i].min_arg) {
        error("Too few arguments to efun %s in efun pointer.\n", query_instr_name(i));
      } else if (num_arg > instrs[i].max_arg && instrs[i].max_arg != -1) {
        error("Too many arguments to efun %s in efun pointer.\n", query_instr_name(i));
      }
      /* possibly we should add TRACE, OPC, etc here;
         also on eval_cost here, which is ok for just 1 efun */
      {
        int j, n = num_arg;
        st_num_arg = num_arg;

        if (n >= 4 || instrs[i].max_arg == -1) {
          n = instrs[i].min_arg;
        }

        for (j = 0; j < n; j++) {
          CHECK_TYPES(sp - num_arg + j + 1, instrs[i].type[j], j + 1, i);
        }

        {
          ScopedTracer _efun_tracer(instrs[i].name, EventCategory::LPC_EFUN, [&] {
            json trace_context = {};

            json args = json::array();
            for (int i = st_num_arg; i > 0; i--) {
              args.push_back(svalue_to_json_summary(sp - i + 1));
            }
            trace_context["args"] = args;
            return trace_context;
          });

          (*efun_table[i - EFUN_BASE])();
        }

        free_svalue(&apply_ret_value, "call_function_pointer");
        if (instrs[i].ret_type == TYPE_NOVALUE) {
          apply_ret_value = const0;
        } else {
          apply_ret_value = *sp--;
        }
        remove_fake_frame();
        return &apply_ret_value;
      }
    }
    case FP_LOCAL | FP_NOT_BINDABLE: {
      function_t* func;

      if (current_object->prog->function_flags[funp->f.local.index] &
          (FUNC_PROTOTYPE | FUNC_UNDEFINED)) {
        error("Undefined lfun pointer called: %s\n",
              function_name(current_object->prog, funp->f.local.index));
      }
      /* Function-pointer calls must fill default arguments like a direct
       * call -- `(: foo :)()` used to run foo with zeros instead of its
       * declared defaults. Fill BEFORE fp is set: the helpers push the
       * missing values as ordinary arguments. */
      bool local_is_async = false;
      {
        auto* cprog = current_object->prog;
        int roff = funp->f.local.index;
        if (cprog->function_flags[roff] & FUNC_ALIAS) {
          roff = cprog->function_flags[roff] & ~FUNC_ALIAS;
        }
        local_is_async = (cprog->function_flags[roff] & FUNC_ASYNC) != 0;
        auto result = get_function_at_index(cprog, roff);
        if (result.first != nullptr) {
          num_arg = fill_default_args(result.first, &result.first->function_table[result.second],
                                      cprog->function_flags[roff], num_arg);
        }
      }

      fp = sp - num_arg + 1;

      push_control_stack(FRAME_FUNCTION);
      current_prog = funp->hdr.owner->prog;

      caller_type = ORIGIN_LOCAL;

      csp->num_local_variables = num_arg;
      func = setup_new_frame(funp->f.local.index);

      if (local_is_async) {
        csp->framekind |= FRAME_ASYNC;
        run_async_function(current_prog->program + func->address, func);
      } else {
        call_program(current_prog, func->address);
      }
      break;
    }
    case FP_FUNCTIONAL:
    case FP_FUNCTIONAL | FP_NOT_BINDABLE: {
      fp = sp - num_arg + 1;
      push_control_stack(FRAME_FUNP);
      current_prog = funp->f.functional.prog;
      csp->fr.funp = funp;

      caller_type = ORIGIN_FUNCTIONAL;

      setup_variables(num_arg, funp->f.functional.num_local, funp->f.functional.num_arg);

      function_index_offset = funp->f.functional.fio;
      variable_index_offset = funp->f.functional.vio;
      call_program(funp->f.functional.prog, funp->f.functional.offset);
      break;
    }
    default:
      error("Unsupported function pointer type.\n");
  }
  free_svalue(&apply_ret_value, "call_function_pointer");
  apply_ret_value = *sp--;
  remove_fake_frame();
  return &apply_ret_value;
}

svalue_t* safe_call_function_pointer(funptr_t* funp, int num_arg) {
  svalue_t* ret;

  error_context_t econ;
  save_context(&econ);
  /* Same as safe_apply(): the callee owns the arguments once the call
     starts (excess args are popped before the body runs), so the unwind
     mark sits below them and the unwind reclaims them (issue #1014). */
  econ.save_sp -= num_arg;
  try {
    ret = call_function_pointer(funp, num_arg);
  } catch (const char*) {
    restore_context(&econ);
    ret = nullptr;
  }
  pop_context(&econ);
  return ret;
}
