#ifndef BACKEND_H
#define BACKEND_H

#define NULL_ERROR_CONTEXT       0
#define NORMAL_ERROR_CONTEXT     1
#define CATCH_ERROR_CONTEXT      2
#define SAFE_APPLY_ERROR_CONTEXT 4

/*
 * backend.c
 */
extern jmp_buf error_recovery_context;
extern int error_recovery_context_exists;
extern int current_time;
extern int heart_beat_flag;
extern struct object *current_heart_beat;
extern int eval_cost;

void backend PROT((void));
void clear_state PROT((void));
void logon PROT((struct object *));
int parse_command PROT((char *, struct object *));
int set_heart_beat PROT((struct object *, int));
int query_heart_beat PROT((struct object *));
int heart_beat_status PROT((int));
void preload_objects PROT((int));
INLINE void remove_destructed_objects PROT((void));
void update_load_av PROT((void));
void update_compile_av PROT((int));
char *query_load_av PROT((void));

#endif
