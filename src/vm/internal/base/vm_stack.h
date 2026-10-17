/* vm_stack.h -- evaluation-stack registers for VM-internal code only.
 *
 * Packages must NOT include this header. Package efuns use LPCFrame
 * (lpc_frame.h) exclusively.
 *
 * Allowed consumers: interpret.cc, ops (bytecode operators), apply,
 * promise, hooks, simulate, lpc_frame.cc, and other vm/internal units.
 */

#ifndef VM_STACK_H
#define VM_STACK_H

#include "vm/internal/base/svalue.h"

struct control_stack_t;

/* Canonical stack-top pointer. */
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
