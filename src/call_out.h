#ifndef CALL_OUT_H
#define CALL_OUT_H

#include "lpc_incl.h"

/*
 * call_out.c
 */
void call_out PROT((void));
void reclaim_call_outs PROT((void));
#ifdef CALLOUT_HANDLES
int find_call_out_by_handle PROT((int));
int remove_call_out_by_handle PROT((int));
int new_call_out PROT((object_t *, svalue_t *, int, int, svalue_t *));
#else
void new_call_out PROT((object_t *, svalue_t *, int, int, svalue_t *));
#endif
int remove_call_out PROT((object_t *, char *));
void remove_all_call_out PROT((object_t *));
int find_call_out PROT((object_t *, char *));
array_t *get_all_call_outs PROT((void));
int print_call_out_usage PROT((outbuffer_t *, int));
void mark_call_outs PROT((void));
void reclaim_call_outs PROT((void));

#endif
