#ifndef EVAL_H
#define EVAL_H

#include <cstdint>

// This variable is used in the signal handler, so it must be marked as volatile.
extern volatile int outoftime;

extern uint64_t max_cost;

void set_eval(uint64_t time);
uint64_t get_eval();
#endif
