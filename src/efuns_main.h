#ifndef EFUNS_MAIN_H
#define EFUNS_MAIN_H

#include "lpc_incl.h"

/* These really should be moved somewhere else */
/*
 * efuns_main.c
 */
extern int call_origin;

int inherits (program_t *, program_t *);
void add_mapping_pair (mapping_t *, const char *, long);
void add_mapping_string (mapping_t *, const char *, const char *);
void add_mapping_object (mapping_t *, const char *, object_t *);
void add_mapping_array (mapping_t *, const char *, array_t *);
void add_mapping_shared_string (mapping_t *, const char *, char *);

#endif
