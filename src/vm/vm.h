/*
 * vm_incl.h
 *
 *  Created on: Nov 16, 2014
 *      Author: sunyc
 */

#ifndef SRC_VM_INCL_H_
#define SRC_VM_INCL_H_

// This file is the main API bundle for interacting with vm layer.

#include "vm/internal/applies.h"
#include "vm/internal/apply.h"

// for calling into master.
#include "vm/internal/master.h"

// TODO: remove this.
#include "vm/internal/simul_efun.h"

// init vm layer.
void vm_init();

#endif /* SRC_VM_INCL_H_ */
