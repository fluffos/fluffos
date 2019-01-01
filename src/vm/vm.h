/*
 * vm_incl.h
 *
 *  Created on: Nov 16, 2014
 *      Author: sunyc
 */

#ifndef SRC_VM_INCL_H_
#define SRC_VM_INCL_H_

// This file is the main API bundle for interacting with vm layer.

// for apply()
#include "applies_table.autogen.h"
#include "vm/internal/apply.h"

// for all EFUN defines
#include "efuns.autogen.h"

// for calling into master.
#include "vm/internal/master.h"

// TODO: remove this.
#include "vm/internal/simul_efun.h"

// for everything?
#include "vm/internal/base/machine.h"

// FIXME: merge this?
#include "vm/internal/eval_limit.h"
#include "vm/internal/simulate.h"

// init vm layer.
void vm_init();

#include <ctime>  // for time_t
// VM boot time, inited in vm_init().
extern time_t boot_time;

// Start running VM, this include load master/simul_efun objects and doing preload.
void vm_start();

// Reset vm
void clear_state(void);

// Remove destructed objects
void remove_destructed_objects(void);

#endif /* SRC_VM_INCL_H_ */
