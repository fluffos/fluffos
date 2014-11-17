/*
 * vm.cc
 *
 *  Created on: Nov 16, 2014
 *      Author: sunyc
 */

#include "base/std.h"

#include <cstdlib>

#include "vm/internal/eval.h"
#include "vm/internal/master.h"
#include "vm/internal/simul_efun.h"
#include "vm/internal/posix_timers.h"
#include "vm/internal/otable.h"             // for init_otable
#include "vm/internal/base/machine.h"       // for reset_machine
#include "vm/internal/compiler/lex.h"       // for add_predefines, fixme!
#include "vm/internal/compiler/compiler.h"  // for init_locals, fixme!

void vm_init() {
  init_identifiers(); /* in lex.c */
  init_locals();      /* in compiler.c */
  init_otable();      /* in otable.c */

  max_cost = CONFIG_INT(__MAX_EVAL_COST__);
  set_inc_list(CONFIG_STR(__INCLUDE_DIRS__));

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

  // init posix timers
  init_posix_timers();
}
