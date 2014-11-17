#ifndef RC_H
#define RC_H

/*
 * rc.c
 */

#include "include/runtime_config.h"

void read_config(char *);
int get_config_item(struct svalue_t *, struct svalue_t *);

extern int config_int[NUM_CONFIG_INTS];
extern char *config_str[NUM_CONFIG_STRS];

#define CONFIG_STR(x) config_str[(x)-BASE_CONFIG_STR]
#define CONFIG_INT(x) config_int[(x)-BASE_CONFIG_INT]

#endif
