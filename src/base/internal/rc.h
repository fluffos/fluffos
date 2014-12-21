#ifndef RC_H
#define RC_H

/*
 * rc.c
 */

#include "include/runtime_config.h"

/*
 * The following is for internal use (ie driver) only
 */
#define RUNTIME_CONFIG_NEXT CFG_INT(26)
#define NUM_CONFIG_STRS (BASE_CONFIG_INT - BASE_CONFIG_STR)
#define NUM_CONFIG_INTS (RUNTIME_CONFIG_NEXT - BASE_CONFIG_INT)

// How many external cmds are supported.
constexpr int NUM_EXTERNAL_CMDS = 100;
extern char *external_cmd[NUM_EXTERNAL_CMDS];

void read_config(char *);

extern int config_int[NUM_CONFIG_INTS];
extern char *config_str[NUM_CONFIG_STRS];

#define CONFIG_STR(x) config_str[(x)-BASE_CONFIG_STR]
#define CONFIG_INT(x) config_int[(x)-BASE_CONFIG_INT]

#endif
