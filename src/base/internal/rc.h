#ifndef RC_H
#define RC_H

/*
 * rc.c
 */

#include "include/runtime_config.h"

// How many external cmds are supported.
constexpr int NUM_EXTERNAL_CMDS = 100;
extern char *external_cmd[NUM_EXTERNAL_CMDS];

void read_config(char *);

extern int config_int[NUM_CONFIG_INTS];
extern char *config_str[NUM_CONFIG_STRS];

#define CONFIG_STR(x) config_str[(x)-BASE_CONFIG_STR]
#define CONFIG_INT(x) config_int[(x)-BASE_CONFIG_INT]

#endif
