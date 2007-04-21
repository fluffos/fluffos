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
extern long current_time;
extern object_t *current_heart_beat;
extern error_context_t *current_error_context;

void backend (void);
void clear_state (void);
void logon (object_t *);
int parse_command (char *, object_t *);
int set_heart_beat (object_t *, int);
int query_heart_beat (object_t *);
int heart_beat_status (outbuffer_t *, int);
void preload_objects (int);
INLINE void remove_destructed_objects (void);
void update_load_av (void);
void update_compile_av (int);
char *query_load_av (void);
array_t *get_heart_beats (void);
int query_time_used (void);
void call_heart_beat (void);

#endif
