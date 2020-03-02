/*
 * machine.h
 *
 *  Created on: Nov 17, 2014
 *      Author: sunyc
 */

#ifndef SRC_VM_INTERNAL_BASE_MACHINE_H_
#define SRC_VM_INTERNAL_BASE_MACHINE_H_

#include <chrono>
#include <sys/types.h>

// FIXME: make this more obvious
#include "efuns.autogen.h"

// lpc stuff
#include "vm/internal/base/array.h"
#include "vm/internal/base/buffer.h"
#include "vm/internal/base/object.h"
#include "vm/internal/base/class.h"
#include "vm/internal/base/mapping.h"
#include "vm/internal/base/number.h"
#include "vm/internal/base/program.h"
#include "vm/internal/base/svalue.h"

// functions
#include "include/function.h"
#include "vm/internal/base/function.h"

#include "vm/internal/base/interpret.h"

// TODO: in interpret.cc
extern int call_origin;
// TODO: defined in simulate.cc
[[noreturn]] void error(const char *const, ...);
extern object_t *current_object;
extern object_t *command_giver;
extern object_t *current_interactive;

// TODO: defined in backend.cc
extern uint64_t g_current_gametick;
// Util to help translate gameticks with time.
int time_to_gametick(std::chrono::milliseconds msec);
std::chrono::milliseconds gametick_to_time(int ticks);

// TODO: defined in interpret.cc
extern struct error_context_t *current_error_context;

void transfer_push_some_svalues(svalue_t *, int);
void push_some_svalues(svalue_t *, int);
#ifdef DEBUG
void int_free_svalue(svalue_t *, const char *);
#else
void int_free_svalue(svalue_t *);
#endif
void free_string_svalue(svalue_t *);
void free_some_svalues(svalue_t *, int);
void push_object(object_t *);
void push_number(LPC_INT);
void push_real(LPC_FLOAT);
void push_undefined(void);
void copy_and_push_string(const char *);
void share_and_push_string(const char *);
void push_array(array_t *);
void push_refed_array(array_t *);
void push_buffer(buffer_t *);
void push_refed_buffer(buffer_t *);
void push_mapping(mapping_t *);
void push_refed_mapping(mapping_t *);
void push_class(array_t *);
void push_refed_class(array_t *);
void push_malloced_string(const char *);
void push_shared_string(const char *);
void push_constant_string(const char *);
void pop_stack(void);
void pop_n_elems(int);
void pop_2_elems(void);
void pop_3_elems(void);

#endif /* SRC_VM_INTERNAL_BASE_MACHINE_H_ */
