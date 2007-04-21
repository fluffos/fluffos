#ifndef RC_H
#define RC_H

#include "lpc_incl.h"

/*
 * rc.c
 */

void set_defaults (char *);
int get_config_int (int);
char *get_config_str (int);
int get_config_item (svalue_t *, svalue_t *);

#endif
