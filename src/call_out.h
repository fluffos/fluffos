#ifndef CALL_OUT_H
#define CALL_OUT_H

/*
 * call_out.c
 */
void call_out PROT((void));
void new_call_out PROT((struct object *, struct svalue *, int, int, struct svalue *));
int remove_call_out PROT((struct object *, char *));
void remove_all_call_out PROT((struct object *));
int find_call_out PROT((struct object *, char *));
struct vector *get_all_call_outs PROT((void));
int print_call_out_usage PROT((int));
void mark_call_outs PROT((void));

#endif
