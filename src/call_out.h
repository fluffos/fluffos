#ifndef CALL_OUT_H
#define CALL_OUT_H

/*
 * call_out.c
 */
void call_out PROT((void));
void new_call_out PROT((object_t *, svalue_t *, int, int, svalue_t *));
int remove_call_out PROT((object_t *, char *));
void remove_all_call_out PROT((object_t *));
int find_call_out PROT((object_t *, char *));
array_t *get_all_call_outs PROT((void));
int print_call_out_usage PROT((outbuffer_t *, int));
void mark_call_outs PROT((void));

#endif
