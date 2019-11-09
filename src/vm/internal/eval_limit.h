#ifndef EVAL_LIMIT_H
#define EVAL_LIMIT_H

#include <cstdint>

// Use to indicate VM execution limits are met. this variable is accessed through signal handler, so
// it must be volatile.
extern volatile int outoftime;

// TODO: we should simply remove this variable and directly read from RC.
// Stores the current maximum eval cost, this is only changed through set_eval_limit efun and
// through runtime config.
extern uint64_t max_eval_cost;

// Initialization for eval limit
void init_eval();

// Set evaluation deadline to given microseconds.
void set_eval(uint64_t time);

// Get remaining microseconds, if no, 0.
int64_t get_eval();

#endif
