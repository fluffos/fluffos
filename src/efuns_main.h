#ifndef EFUNS_MAIN_H
#define EFUNS_MAIN_H

#include "lpc_incl.h"

/* These really should be moved somewhere else */
/*
 * efuns_main.c
 */
extern int call_origin;

int inherits PROT((program_t *, program_t *));
void add_mapping_pair PROT((mapping_t *, const char *, int));
void add_mapping_string PROT((mapping_t *, const char *, const char *));
void add_mapping_object PROT((mapping_t *, const char *, object_t *));
void add_mapping_array PROT((mapping_t *, const char *, array_t *));
void add_mapping_shared_string PROT((mapping_t *, char *, char *));

#endif
