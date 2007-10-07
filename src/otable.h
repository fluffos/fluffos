#ifndef OTABLE_H
#define OTABLE_H

#include "lpc_incl.h"

/*
 * otable.c
 */
void init_otable (void);
void enter_object_hash (object_t *);
void remove_object_hash (object_t *);
void remove_precompiled_hashes (char *);
object_t *lookup_object_hash (const char *);
int show_otable_status (outbuffer_t *, int);
array_t *find_ch_n (const char * s);

#endif
