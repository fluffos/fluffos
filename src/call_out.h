#ifndef CALL_OUT_H
#define CALL_OUT_H

#include "lpc_incl.h"

/*
 * call_out.c
 */
void call_out (void);
void reclaim_call_outs (void);
#ifdef CALLOUT_HANDLES
int find_call_out_by_handle (int);
int remove_call_out_by_handle (int);
int new_call_out (object_t *, svalue_t *, int, int, svalue_t *);
#else
void new_call_out (object_t *, svalue_t *, int, int, svalue_t *);
#endif
int remove_call_out (object_t *, const char *);
void remove_all_call_out (object_t *);
int find_call_out (object_t *, const char *);
array_t *get_all_call_outs (void);
int print_call_out_usage (outbuffer_t *, int);
void mark_call_outs (void);
void reclaim_call_outs (void);

#endif
