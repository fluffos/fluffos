#ifndef BACKEND_H
#define BACKEND_H

#include "interpret.h"

#define NULL_ERROR_CONTEXT       0
#define NORMAL_ERROR_CONTEXT     1
#define CATCH_ERROR_CONTEXT      2
#define SAFE_APPLY_ERROR_CONTEXT 4

/*
 * backend.c
 */
extern int current_time;
extern int heart_beat_flag;
extern object_t *current_heart_beat;
extern int eval_cost;
extern error_context_t *current_error_context;

void backend PROT((void));
void clear_state PROT((void));
void logon PROT((object_t *));
int parse_command PROT((char *, object_t *));
int set_heart_beat PROT((object_t *, int));
int query_heart_beat PROT((object_t *));
int heart_beat_status PROT((outbuffer_t *, int));
void preload_objects PROT((int));
INLINE void remove_destructed_objects PROT((void));
void update_load_av PROT((void));
void update_compile_av PROT((int));
char *query_load_av PROT((void));
array_t *get_heart_beats PROT((void));

#endif
