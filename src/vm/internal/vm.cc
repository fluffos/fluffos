/*
 * vm.cc
 *
 *  Created on: Nov 16, 2014
 *      Author: sunyc
 */

#include "base/std.h"

#include <cstdlib>

#include "applies_table.autogen.h"
#include "vm/internal/base/machine.h"  // for reset_machine
#include "vm/internal/eval_limit.h"
#include "vm/internal/master.h"
#include "vm/internal/simul_efun.h"
#include "vm/internal/simulate.h"
#include "compiler/internal/lex.h"       // for add_predefines, fixme!
#include "compiler/internal/compiler.h"  // for init_locals, fixme!

#include "packages/core/replace_program.h"
#ifdef PACKAGE_MUDLIB_STATS
#include "packages/mudlib_stats/mudlib_stats.h"
#endif

time_t boot_time;

namespace {
/* The epilog() in master.c is supposed to return an array of files to load.
 * The preload() in master object called to do the actual loading.
 */
void preload_objects() {
  // Legacy: epilog() has a int param to make it to avoid load anything.
  // I'm not sure who would use that.
  push_number(0);
  auto ret = safe_apply_master_ob(APPLY_EPILOG, 1);

  if ((ret == nullptr) || (ret == (svalue_t *)-1) || (ret->type != T_ARRAY)) {
    return;
  }

  auto prefiles = ret->u.arr;
  if ((prefiles == nullptr) || (prefiles->size < 1)) {
    return;
  }

  // prefiles (the global apply return value) would have been freed on next apply call.
  // so we have to increase ref here to make sure it is around.
  prefiles->ref++;

  debug_message("\nLoading preload files ...\n");

  for (int i = 0; i < prefiles->size; i++) {
    if (prefiles->item[i].type != T_STRING) {
      continue;
    }
    debug_message("%s...\n", prefiles->item[i].u.string);

    push_svalue(&prefiles->item[i]);
    set_eval(max_eval_cost);
    safe_apply_master_ob(APPLY_PRELOAD, 1);
  }
  free_array(prefiles);
} /* preload_objects() */

}  // namespace

void vm_init() {
  boot_time = get_current_time();

  init_eval(); /* in eval.cc */

  init_strings();     /* in stralloc.c */
  init_identifiers(); /* in lex.c */
  init_locals();      /* in compiler.c */

  max_eval_cost = CONFIG_INT(__MAX_EVAL_COST__);
  set_inc_list(CONFIG_STR(__INCLUDE_DIRS__));

  add_predefines();
  reset_machine(1);

  set_eval(max_eval_cost);
}

void vm_start() {
  error_context_t econ;
  save_context(&econ);
  try {
    debug_message("Loading simul_efun file : %s\n", CONFIG_STR(__SIMUL_EFUN_FILE__));
    init_simul_efun(CONFIG_STR(__SIMUL_EFUN_FILE__));
    debug_message("Loading master file: %s\n", CONFIG_STR(__MASTER_FILE__));
    init_master(CONFIG_STR(__MASTER_FILE__));
  } catch (const char *) {
    debug_message("The simul_efun (%s) and master (%s) objects must be loadable.\n",
                  CONFIG_STR(__SIMUL_EFUN_FILE__), CONFIG_STR(__MASTER_FILE__));
    debug_message("Please check log files for exact error. \n");
    restore_context(&econ);
    pop_context(&econ);
    exit(-1);
  }
  pop_context(&econ);

  // TODO: move this to correct location.
#ifdef PACKAGE_MUDLIB_STATS
  restore_stat_files();
#endif

  preload_objects();
}

/*
 * There are global variables that must be zeroed before any execution.
 *
 * This routine must only be called from top level, not from inside
 * stack machine execution (as stack will be cleared).
 */
void clear_state() {
  current_object = nullptr;
  set_command_giver(nullptr);
  current_interactive = nullptr;
  previous_ob = nullptr;
  current_prog = nullptr;
  caller_type = 0;
  reset_machine(0); /* Pop down the stack. */
} /* clear_state() */

/* All destructed objects are moved into a sperate linked list,
 * and deallocated after program execution.  */
// TODO: find where they are
extern object_t *obj_list_destruct;
void remove_destructed_objects() {
  if (obj_list_replace) {
    replace_programs();
  }

  if (obj_list_destruct) {
    object_t *ob, *next;
    for (ob = obj_list_destruct; ob; ob = next) {
      next = ob->next_all;
      destruct2(ob);
    }
    obj_list_destruct = nullptr;
  }
} /* remove_destructed_objects() */
