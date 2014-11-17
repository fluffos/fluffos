/*
 * vm.cc
 *
 *  Created on: Nov 16, 2014
 *      Author: sunyc
 */

#include "base/std.h"

#include "vm/internal/master.h"
#include "vm/internal/simul_efun.h"

void vm_init() {
  add_predefines();
  reset_machine(1);

  try {
    init_simul_efun(CONFIG_STR(__SIMUL_EFUN_FILE__));
    init_master(CONFIG_STR(__MASTER_FILE__));
  } catch (const char *) {
    debug_message("The simul_efun (%s) and master (%s) objects must be loadable.\n",
                  CONFIG_STR(__SIMUL_EFUN_FILE__), CONFIG_STR(__MASTER_FILE__));
    exit(-1);
  }

  init_identifiers(); /* in lex.c */
  init_locals();      /* in compiler.c */

  max_cost = CONFIG_INT(__MAX_EVAL_COST__);
  set_inc_list(CONFIG_STR(__INCLUDE_DIRS__));
}
