#ifndef RC_H
#define RC_H
/*
 * rc.c
 */
#include "interpret.h"

void set_defaults PROT((char *));
int get_config_int PROT((int));
char *get_config_str PROT((int));
int get_config_item PROT((struct svalue *, struct svalue *));

#endif
