#include "base/package_api.h"

#include "packages/core/replace_program.h"

#include "base/internal/stralloc.h"      // for findstring()
#include "vm/internal/base/apply_cache.h"  // for apply_cache_lookup()

/*
 * replace_program.c
 * replaces the program in a running object with one of the programs
 * it inherits, in order to save memory.
 * Ported from Amylaars LP 3.2 driver
 *
 * For a worked example of the intended usage pattern (a generic "shell"
 * object deciding its real class at runtime and replace_program()-ing
 * into it), see testsuite/clone/replace_program/demo_poly_shell.lpc.
 *
 * For the accepted ob->prog-identity tradeoffs the no-op fast path below
 * introduces (clone_object() of a no-op'd blueprint, reload_object(),
 * compile-time inherit of a no-op'd blueprint, and others), see the
 * comment block immediately above f_replace_program().
 */

replace_ob_t* obj_list_replace = nullptr;

static program_t* search_inherited(char* /*str*/, program_t* /*prg*/, int* /*offpnt*/);
static replace_ob_t* retrieve_replace_program_entry();

int replace_program_pending(object_t* ob) {
  replace_ob_t* r_ob;

  for (r_ob = obj_list_replace; r_ob; r_ob = r_ob->next) {
    if (r_ob->ob == ob) {
      return 1;
    }
  }

  return 0;
}

/*
 * Void any pending replace_program() for ob. recompile_object() calls
 * this at the moment it swaps ob's program: a pending entry's target
 * program pointer and variable offset were computed against the
 * program being replaced (old code may have registered it mid-update,
 * e.g. from another target's __INIT), and letting the backend sweep
 * apply it to the fresh program would corrupt the variable block. An
 * entry registered AFTER the swap is computed against the new program
 * and is left alone.
 */
void cancel_pending_replace_program(object_t* ob) {
  replace_ob_t **prev = &obj_list_replace, *r;

  while ((r = *prev)) {
    if (r->ob == ob) {
      *prev = r->next;
      FREE((char*)r);
    } else {
      prev = &r->next;
    }
  }
}

void replace_programs() {
  replace_ob_t *r_ob, *r_next;
  int i, num_fewer, offset;
  svalue_t* svp;

  debug(d_flag, ("start of replace_programs"));

  for (r_ob = obj_list_replace; r_ob; r_ob = r_next) {
    program_t* old_prog;

    num_fewer = r_ob->ob->prog->num_variables_total - r_ob->new_prog->num_variables_total;

    debug(d_flag, "%d less variables\n", num_fewer);

    tot_alloc_object_size -= num_fewer * sizeof(svalue_t[1]);
    if ((offset = r_ob->var_offset)) {
      svp = r_ob->ob->variables;
      /* move our variables up to the top */
      for (i = 0; i < r_ob->new_prog->num_variables_total; i++) {
        free_svalue(svp, "replace_programs");
        *svp = *(svp + offset);
        *(svp + offset) = const0u;
        svp++;
      }
      /* free the rest */
      for (i = 0; i < num_fewer; i++) {
        free_svalue(svp, "replace_programs");
        *svp++ = const0u;
      }
    } else {
      /* We just need to remove the last num_fewer variables */
      svp = &r_ob->ob->variables[r_ob->new_prog->num_variables_total];
      for (i = 0; i < num_fewer; i++) {
        free_svalue(svp, "replace_programs");
        *svp++ = const0u;
      }
    }

    if (r_ob->ob->replaced_program) {
      FREE_MSTR(r_ob->ob->replaced_program);
      r_ob->ob->replaced_program = nullptr;
    }
    r_ob->ob->replaced_program = string_copy(r_ob->new_prog->filename, "replace_programs");

    reference_prog(r_ob->new_prog, "replace_programs");
    old_prog = r_ob->ob->prog;
    r_ob->ob->prog = r_ob->new_prog;
    r_next = r_ob->next;
    free_prog(&old_prog);

    debug(d_flag, ("program freed."));
#ifndef NO_SHADOWS
    if (r_ob->ob->shadowing) {
      /*
       * The master couldn't decide if it's a legal shadowing before
       * the program was actually replaced. It is possible that the
       * blueprint to the replacing program is already destructed, and
       * it's source changed. On the other hand, if we called the
       * master now, all kind of volatile data structures could result,
       * even new entries for obj_list_replace. This would eventually
       * require to reference it, and all the lrpp's , in
       * check_a_lot_ref_counts() and garbage_collection() . Being able
       * to use replace_program() in shadows is hardly worth this
       * effort. Thus, we simply stop the shadowing.
       */
      r_ob->ob->shadowing->shadowed = r_ob->ob->shadowed;
      if (r_ob->ob->shadowed) {
        r_ob->ob->shadowed->shadowing = r_ob->ob->shadowing;
        r_ob->ob->shadowed = nullptr;
      }
      r_ob->ob->shadowing = nullptr;
    }
#endif
    FREE((char*)r_ob);
  }
  obj_list_replace = (replace_ob_t*)nullptr;
  debug(d_flag, ("end of replace_programs"));
}

#ifdef F_REPLACE_PROGRAM
static program_t* search_inherited(char* str, program_t* prg, int* offpnt) {
  program_t* tmp;
  int i;

  debug(d_flag, "search_inherited started");
  debug(d_flag, "searching for PRG(/%s) in PRG(/%s)", str, prg->filename);
  debug(d_flag, "num_inherited=%d\n", prg->num_inherited);

  for (i = 0; i < prg->num_inherited; i++) {
    debug(d_flag, "index %d:", i);
    debug(d_flag, "checking PRG(/%s)", prg->inherit[i].prog->filename);

    if (strcmp(str, prg->inherit[i].prog->filename) == 0) {
      debug(d_flag, "match found");

      *offpnt = prg->inherit[i].variable_index_offset;
      return prg->inherit[i].prog;
    }
    if ((tmp = search_inherited(str, prg->inherit[i].prog, offpnt))) {
      debug(d_flag, "deferred match found");

      *offpnt += prg->inherit[i].variable_index_offset;
      return tmp;
    }
  }
  debug(d_flag, "search_inherited failed");

  return (program_t*)nullptr;
}

static replace_ob_t* retrieve_replace_program_entry() {
  replace_ob_t* r_ob;

  for (r_ob = obj_list_replace; r_ob; r_ob = r_ob->next) {
    if (r_ob->ob == current_object) {
      return r_ob;
    }
  }
  return nullptr;
}

/*
 * True iff swapping cur_prog for new_prog (the classic
 * `inherit X; void create() { replace_program(X); }` boilerplate, where X
 * is already cur_prog's entire effective content) would not change what a
 * single one of cur_prog's OTHER callable functions resolves to. Two names
 * are excluded from the comparison on purpose: "create" and the
 * compiler-synthesized "#global_init#" (see vm/internal/applies -- it's
 * what __INIT resolves to, emitted whenever a program has any initialized
 * global variable of its own; compiler.cc's epilog() defines it, guarded
 * by "if there was any code to initialize").
 * Both are one-shot constructor applies that the driver recognizes and
 * never re-invokes once run, and both are the only functions that can
 * ever legitimately differ here: "create" necessarily contains the very
 * replace_program() call being evaluated, and "#global_init#" necessarily
 * differs in identity between cur_prog and new_prog even when EVERY other
 * function and variable is identical, simply because each program gets
 * its own synthesized init routine. Neither one's presence, absence, or
 * pointer identity says anything about whether the actual observable
 * functions/variables differ -- comparing them would make is_noop() return
 * false for nearly every real object whose inherited base class has state
 * to initialize, which is the common case, not the exception.
 *
 * This reuses apply_low()/call_other()'s own name-resolution machinery
 * (vm/internal/base/apply_cache.cc's fill_lookup_table_recurse(), which
 * walks a program's full inherit tree exactly once per program and caches
 * the result on program_t::apply_lookup_table) rather than comparing raw
 * function-table slot counts: a local function definition -- override or
 * brand new -- always adds a fresh slot to num_functions_defined (aliasing
 * the old slot, see handle_functions() in compiler.cc), so slot counts
 * inflate by one per local definition regardless of whether it changes
 * anything observable. Comparing the RESOLVED (name -> function_t*)
 * mapping instead sidesteps that entirely: two programs expose identical
 * behavior for a name iff the name resolves to the exact same function_t
 * (by pointer -- function_t entries live inside their defining program's
 * function_table and are never copied), no matter how many alias slots
 * either program's compiler happened to emit along the way.
 *
 * The comparison loop below only walks cur_prog's resolved name table,
 * never new_prog's -- sound only in the cur_prog-contains-new_prog
 * direction: f_replace_program() already errors out before reaching here
 * ("program to replace the current with has to be inherited\n") unless
 * new_prog is somewhere in cur_prog's own inherit tree, which guarantees
 * every name new_prog resolves, cur_prog already resolves too (to some
 * function -- possibly a different, overriding one, which the loop below
 * still catches). So a one-directional walk over cur_prog's table alone
 * is enough to catch every way new_prog could expose something cur_prog
 * doesn't.
 */
// Access/behavior-relevant function_flags bits: DECL_ACCESS (visibility --
// see apply.cc's funflags permission check), FUNC_VARARGS/FUNC_TRUE_VARARGS
// (argument-count handling -- see apply()'s default-argument setup right
// after the permission check) and DECL_NOMASK (redefinability). NOMASK is
// NOT a purely compile-time property, despite nomask violations between
// two files normally being caught by the compiler: check_shadow_functions()
// (vm/internal/base/interpret.cc, called from validate_shadowing(), the
// backing function for the shadow() efun) reads
// `victim->prog->function_flags[...] & DECL_NOMASK` at RUNTIME, against an
// already-instantiated object, to decide whether shadow() may legally
// override a given function on it. So a `nomask inherit X` wrapper --
// DECL_NOMASK set in the wrapper's own function_flags entry, absent from
// X's own -- is shadow-proof, while X accessed directly is not: a genuine
// runtime behavioral difference between the two programs, and exactly why
// DECL_NOMASK belongs in this comparison mask. This deliberately excludes
// implementation-internal bits like FUNC_ALIAS/FUNC_INHERITED/
// FUNC_PROTOTYPE/FUNC_UNDEFINED/FUNC_STRICT_TYPES, which are structural
// bookkeeping about how a slot got there, not what calling it does.
constexpr unsigned short kFuncBehaviorMask =
    DECL_ACCESS | FUNC_VARARGS | FUNC_TRUE_VARARGS | DECL_NOMASK;

// Whether the variable at flattened index `target_index` (in prog's own
// num_variables_total numbering) is subject to DECL_NOSAVE -- either its
// own declaration, or an ancestor "nosave inherit ...;" edge marking the
// whole subtree nosave.
//
// This can NOT be answered with a flat prog->variable_types[target_index]
// read: unlike function_flags (sized to a program's FULL flattened
// function count, see fill_lookup_table_recurse()/apply.cc's runtime_index
// usage), variable_table/variable_types are sized to prog->
// num_variables_defined only -- the variables DECLARED directly in this
// program (compiler.cc: the A_VAR_NAME/A_VAR_TYPE mem blocks are only
// appended to by the "declare a new global" path, not by
// copy_variables()'s define_variable() call, which only records into
// A_VAR_TEMP -- the *_total accounting -- for the whole flattened tree).
// An index at or past num_variables_defined belongs to an INHERITED
// variable, which lives in the defining program's own arrays instead;
// reading past the end of variable_types[] here previously read
// uninitialized/adjacent struct memory (confirmed via manual repro: two
// consecutive runs against the same fixture returned two different
// garbage flag words for the exact same slot).
//
// Mirrors save_object_recurse()'s (object.cc) real traversal instead:
// depth-first over prog->inherit[] in order (recursing fully into each
// before moving to the next), then this program's own num_variables_defined
// slots last -- the same order copy_variables() lays them out in, and the
// same order num_variables_total's accounting assumes.
static bool variable_is_nosave(program_t* prog, int target_index, unsigned short type_mod) {
  int base = 0;
  for (int i = 0; i < prog->num_inherited; i++) {
    int count = prog->inherit[i].prog->num_variables_total;
    if (target_index < base + count) {
      return variable_is_nosave(prog->inherit[i].prog, target_index - base,
                                 type_mod | prog->inherit[i].type_mod);
    }
    base += count;
  }
  return ((type_mod & DECL_NOSAVE) != 0) ||
         ((prog->variable_types[target_index - base] & DECL_NOSAVE) != 0);
}

static bool replace_program_is_noop(program_t* cur_prog, program_t* new_prog) {
  // Trigger (and cache) both programs' flattened, override-resolved
  // name tables.
  apply_cache_lookup("create", cur_prog);
  apply_cache_lookup("create", new_prog);

  // apply_cache_lookup() only fills apply_lookup_table as a side effect
  // when the funcname it was asked to look up is already an interned
  // shared string: it returns early -- BEFORE fill_lookup_table() -- when
  // findstring(funcname) is null (see apply_cache.cc). "create" is
  // effectively always interned by the time any replace_program() call
  // can be reached (call_create() looks up "create" for every object the
  // driver ever loads, including whatever object is executing the
  // replace_program() call itself and whatever loaded the master object
  // before it), so this is not known to be reachable in practice -- but
  // it isn't provably impossible either, and the guard is cheap. Fail
  // closed (not a no-op) rather than dereference a still-null table
  // below.
  if (cur_prog->apply_lookup_table == nullptr || new_prog->apply_lookup_table == nullptr) {
    return false;
  }

  const char* create_key = findstring("create");
  const char* global_init_key = findstring("#global_init#");

  for (auto& kv : *cur_prog->apply_lookup_table) {
    if (create_key != nullptr && kv.first == reinterpret_cast<intptr_t>(create_key)) {
      continue;
    }
    if (global_init_key != nullptr && kv.first == reinterpret_cast<intptr_t>(global_init_key)) {
      continue;
    }
    auto it = new_prog->apply_lookup_table->find(kv.first);
    if (it == new_prog->apply_lookup_table->end()) {
      return false;  // a name cur_prog exposes that new_prog does not
    }
    if (it->second.funp != kv.second.funp) {
      return false;  // same name, different (overridden) implementation
    }
    // Same underlying function_t*, but identity alone doesn't guarantee
    // identical calling behavior: copy_new_function() (compiler.cc) bakes
    // per-program access-flag demotions into EACH program's own
    // function_flags[] independently when a function gets copied down an
    // inherit chain -- most notably "private functions become hidden"
    // (`f = (f & ~DECL_PRIVATE) | DECL_HIDDEN;`), plus whatever inherit
    // modifiers (varargs/nosave/nomask inherit) that particular inherit
    // edge applied via DECL_MODIFY(). So the SAME function_t can be
    // DECL_PRIVATE as seen through one program and DECL_HIDDEN (or some
    // other combination) as seen through another, changing what
    // apply()/call_other() permission-check (apply.cc's `funflags =
    // ob->prog->function_flags[entry.runtime_index]`) allows. Compare
    // each program's OWN function_flags entry for this resolved slot,
    // exactly the way apply() itself derives funflags: index into
    // {cur,new}_prog->function_flags with that program's own
    // runtime_index from its own (separately filled) apply_lookup_table.
    unsigned short cur_fflags = cur_prog->function_flags[kv.second.runtime_index];
    unsigned short new_fflags = new_prog->function_flags[it->second.runtime_index];
    if ((cur_fflags & kFuncBehaviorMask) != (new_fflags & kFuncBehaviorMask)) {
      return false;  // same implementation, different observable access/arity contract
    }
  }

  // Variable side: var_offset == 0 plus num_variables_total equality (the
  // caller's existing check) already proves cur_prog's and new_prog's
  // variable blocks are the same size and start at the same index -- i.e.
  // index-aligned 1:1 across their entire range (variables are only ever
  // ADDED by descending an inherit chain, never removed/renumbered/
  // reordered -- see f_replace_program()'s comment above its call to this
  // function). Unlike the function side, DECL_ACCESS (private/protected/
  // public) on a VARIABLE has no runtime enforcement point at all --
  // "private variable" is a purely compile-time restriction on which
  // *other, separately-compiled* files may reference it by name (checked
  // when THAT code is compiled, against the target class's declared
  // flags, entirely unrelated to what replace_program() does to an
  // already-running, already-compiled object later); there is no
  // "apply-a-variable" runtime path the way apply()/call_other() is for
  // functions. So access flags are structural here, not behavioral, and
  // are correctly left out of this comparison. DECL_NOSAVE is the one
  // variable flag with a genuine runtime effect: save_object_recurse()/
  // restore_object() (object.cc) skip a variable's persistence entirely
  // when it (or an ancestor "nosave inherit") marks it nosave -- directly
  // observable via save file contents post-swap. See variable_is_nosave()
  // above for why this needs a real tree walk instead of a flat array
  // index.
  for (int i = 0; i < new_prog->num_variables_total; i++) {
    if (variable_is_nosave(cur_prog, i, 0) != variable_is_nosave(new_prog, i, 0)) {
      return false;  // same variable slot, different observable save contract
    }
  }

  return true;
}

// ---------------------------------------------------------------------
// Accepted semantic change: ob->prog identity under the no-op fast path
// ---------------------------------------------------------------------
// The old driver eventually converged an object's ob->prog pointer to the
// replace_program() target: replace_programs() (top of this file) sets
// `ob->prog = new_prog` whenever the periodic sweep (backend.cc's
// 5-minute tick, or an explicit reclaim_objects()) next ran a pending
// entry. Taking the no-op fast path below means that convergence never
// happens for a no-op target: ob->prog stays the WRAPPER's own program
// forever, never becoming new_prog. replace_program_is_noop() above
// proves this is invisible to apply()/call_other() dispatch -- every
// name resolves identically either way -- but it is NOT invisible to the
// smaller set of consumers that read ob->prog directly instead of
// dispatching through a call:
//
// - clone_object() (vm/internal/base/simulate.cc, `new_ob->prog =
//   ob->prog;`) of a no-op'd BLUEPRINT (not an already-cloned instance)
//   hands every future clone the WRAPPER's own program, not the
//   target's. If the wrapper's create() is just the replace_program()
//   call itself (the classic minimal stub) and never explicitly chains
//   to `::create()`, every clone forever runs the wrapper's trivial
//   create() instead of the target class's own -- permanently, not just
//   until the next sweep. This is the sharpest instance of the
//   divergence: it's the one case where "the sweep would eventually
//   have fixed it" does not apply, because a blueprint that is never
//   itself swapped never gets its own ob->prog updated, no matter how
//   many clones are made from it or how much later.
// - reload_object() (vm/internal/base/object.cc) calls call_create(),
//   which applies APPLY_CREATE and #global_init# against ob->prog --
//   the wrapper, forever, not new_prog.
// - A file that `inherit`s a no-op'd blueprint at compile time
//   (rule_inheritence(), src/compiler/internal/grammar_rules.cc, via
//   `find_object2(path)->prog`) inherits the wrapper's shape, one extra
//   indirection level, instead of the target's -- permanently.
// - The shadow-severing side effect that replace_programs() applies a
//   few dozen lines up (the `#ifndef NO_SHADOWS` block) never runs for
//   a no-op'd object, so an active shadow on it now survives where it
//   previously would have been forcibly dropped -- almost certainly an
//   improvement (shadowing is a legitimate, orthogonal feature), but a
//   behavioral difference worth naming.
// - inherits(X, ob) stays true forever instead of eventually reflecting
//   the "ob became X" transition the old driver's sweep produced --
//   also arguably an improvement, since code checking against the
//   target class keeps working instead of silently stopping.
//
// Risk, grounded in the corpus scan that motivated this fix (see the
// commit message: 350,433 files / 193 codebases): roughly 92.6% of the
// matched occurrences (337,206 of ~364K candidates) sit under a `d/`
// path -- the room/area-content convention dominant across this corpus
// family, where objects are singleton "blueprint-is-the-instance"
// content loaded once via movement or find_object(), never
// clone_object()'d, so the clone_object() divergence above cannot even
// arise for them. The remainder plausibly subject to repeated cloning
// (`clone/`- and `obj/`-style paths) is on the order of 1,700 files --
// under 0.5% of the total -- and a spot-check of that subset found
// boards and singleton utility objects, not mass-cloned NPC/monster
// content that would depend on a fresh target-class create() per clone.
// So the sharpest divergence appears to affect a small minority of the
// actual target corpus -- though for any individual mudlib that DOES use
// this pattern on a genuinely stub-shaped, repeatedly-cloned blueprint,
// the risk to that mudlib specifically is not zero.
//
// A more thorough fix was considered and deliberately deferred, not
// overlooked: applying the swap SYNCHRONOUSLY (`ob->prog = new_prog`
// immediately, instead of skipping bookkeeping entirely) would restore
// old-driver identity semantics for everything above, while also
// closing the closure-binding pending-window completely rather than
// just skipping it for no-ops. It wasn't pursued here because it
// reopens the exact hazard the driver's existing DEFERRED-swap design
// exists to avoid: replace_program() is called from inside the object's
// own still-executing create() call frame (excluded from the no-op
// comparison above precisely because it's expected to differ from the
// target's, or not exist there at all) -- freeing/changing the
// underlying compiled program out from under the interpreter while it
// is still executing bytecode from that very program is a real hazard,
// not a cosmetic one. Making a synchronous swap safe would need
// additional engineering -- e.g. deferring the actual pointer swap
// until the CURRENT call frame specifically returns, rather than either
// "immediately" or "next periodic sweep, up to 5 minutes" -- which is
// out of scope for this fix.

void f_replace_program() {
  replace_ob_t* tmp;
  int name_len;
  char *name, *xname;
  program_t* new_prog;
  int var_offset;

  if (sp->type != T_STRING) {
    bad_arg(1, F_REPLACE_PROGRAM);
  }
  debug(d_flag, ("replace_program called"));

  if (!current_object) {
    error("replace_program called with no current object\n");
  }
  if (current_object == simul_efun_ob) {
    error("replace_program on simul_efun object\n");
  }

  if (current_object->prog->func_ref) {
    debug_message("%s: cannot replace a program with function references, ignored.\n",
                  current_object->prog->filename);
  }

  name_len = SVALUE_STRLEN(sp);
  name = reinterpret_cast<char*>(DMALLOC(name_len + 5, TAG_TEMPORARY, "replace_program"));
  xname = name;
  strcpy(name, sp->u.string);
  bool has_ext = (name_len >= 2 && strcmp(name + name_len - 2, ".c") == 0) ||
                 (name_len >= 4 && strcmp(name + name_len - 4, ".lpc") == 0);
  if (*name == '/') {
    name++;
  }
  if (has_ext) {
    new_prog = search_inherited(name, current_object->prog, &var_offset);
  } else {
    // Both source spellings are first-class: try ".lpc", then ".c".
    size_t blen = strlen(name);
    strcat(name, ".lpc");
    new_prog = search_inherited(name, current_object->prog, &var_offset);
    if (new_prog == nullptr) {
      name[blen] = '\0';
      strcat(name, ".c");
      new_prog = search_inherited(name, current_object->prog, &var_offset);
    }
  }
  FREE(xname);
  if (!new_prog) {
    error("program to replace the current with has to be inherited\n");
  }

  // Fetch (but do not yet register) any already-pending entry for this
  // object ONCE, up front. retrieve_replace_program_entry() implements
  // "last call wins" for repeated replace_program() calls within the same
  // execution (e.g. two calls from create()): a second call reuses and
  // OVERWRITES the first call's pending entry rather than adding a second
  // one. The no-op fast path below must respect that contract too -- see
  // its gate immediately below for why.
  tmp = retrieve_replace_program_entry();

  /*
   * Common mudlib boilerplate: `inherit X; void create() { replace_program(X); }`
   * where X is already the object's entire effective program. Applying the
   * swap machinery for that case is pure overhead: it registers a pending
   * entry that blocks make_lfun_funp()/make_functional_funp() (vm/internal/
   * base/function.cc) from binding any closure to this object until
   * remove_destructed_objects() (vm/internal/vm.cc) next runs
   * replace_programs() -- see backend.cc's 5-minute recurring tick and
   * efuns_main.cc's f_reclaim_objects() for when that happens.
   *
   * Two-part gate: variable side is var_offset == 0 plus num_variables_total
   * equality (current_object->prog contains exactly new_prog's variables,
   * nothing more); function side is replace_program_is_noop() above -- see
   * its own header comment for the full rationale. A genuinely different
   * target fails one of these and falls through to the normal pending path
   * unchanged.
   *
   * Only take this fast path when NO pending entry already exists (tmp ==
   * nullptr): a second replace_program() call this execution must win
   * regardless of whether ITS target happens to be a no-op relative to
   * current_object->prog's still-unswapped state ("last call wins" --
   * see retrieve_replace_program_entry()'s call site above) -- silently
   * returning early here would leave the stale first call's entry pending
   * instead.
   */
  if (tmp == nullptr && var_offset == 0 &&
      current_object->prog->num_variables_total == new_prog->num_variables_total &&
      replace_program_is_noop(current_object->prog, new_prog)) {
    debug(d_flag,
          ("replace_program: target already fully present via inherit (zero new funcs/vars); "
           "no-op, skipping pending-swap bookkeeping"));
    free_string_svalue(sp--);
    return;
  }

  if (!tmp) {
    tmp = reinterpret_cast<replace_ob_t*>(
        DMALLOC(sizeof(replace_ob_t), TAG_REPLACE_OB, "replace_program"));
    tmp->ob = current_object;
    tmp->next = obj_list_replace;
    obj_list_replace = tmp;
  }
  tmp->new_prog = new_prog;
  tmp->var_offset = var_offset;

  debug(d_flag, ("replace_program finished"));

  free_string_svalue(sp--);
}

#endif
