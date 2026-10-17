/* vm_stack.h -- evaluation-stack registers for VM-internal code only.
 *
 * Packages must NOT include this header. Use LPCFrame or vm_sp() /
 * push_*, pop_*, put_* from machine.h / interpret.h instead.
 *
 * Allowed consumers: interpret.cc, ops (bytecode operators), apply,
 * promise, hooks, simulate, and other vm/internal translation units.
 */

#ifndef VM_STACK_H
#define VM_STACK_H

#include "vm/internal/base/svalue.h"

struct control_stack_t;

/* Canonical stack-top pointer (also exposed as vm_sp() via interpret.h). */
extern svalue_t* lpc_eval_sp;
#ifndef sp
#define sp lpc_eval_sp
#endif

extern svalue_t* fp;
extern char* pc;
extern svalue_t* start_of_stack;
extern svalue_t* end_of_stack;
extern control_stack_t* control_stack;
extern control_stack_t* control_stack_last;
extern control_stack_t* csp;

#endif /* VM_STACK_H */
