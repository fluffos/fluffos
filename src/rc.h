#ifndef RC_H
#define RC_H

#include "lpc_incl.h"

/*
 * rc.c
 */

void set_defaults PROT((char *));
int get_config_int PROT((int));
char *get_config_str PROT((int));
int get_config_item PROT((svalue_t *, svalue_t *));

#endif
