#ifndef BACKEND_H
#define BACKEND_H

#include "interpret.h"
#include "object.h"

#define NULL_ERROR_CONTEXT       0
#define NORMAL_ERROR_CONTEXT     1
#define CATCH_ERROR_CONTEXT      2
#define SAFE_APPLY_ERROR_CONTEXT 4

/*
 * backend.c
 */
extern int current_time;
extern int eval_cost;
extern error_context_t *current_error_context;

void backend PROT((void));
void clear_state PROT((void));
int parse_command PROT((char *, object_t *));
void preload_objects PROT((int));
INLINE void remove_destructed_objects PROT((void));
void update_load_av PROT((void));
void update_compile_av PROT((int));
char *query_load_av PROT((void));
void set_pulse_interval PROT((int));

#endif
