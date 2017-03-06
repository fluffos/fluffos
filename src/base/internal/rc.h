#ifndef RC_H
#define RC_H

/*
 * rc.c
 */

#include "include/runtime_config.h"

/*
 * The following is for internal use (ie driver) only
 */
#define NUM_CONFIG_STRS (BASE_CONFIG_INT - BASE_CONFIG_STR)
#define NUM_CONFIG_INTS (RUNTIME_CONFIG_NEXT - BASE_CONFIG_INT)

// How many external cmds are supported.
constexpr int g_num_external_cmds = 100;
extern char *external_cmd[g_num_external_cmds];

void read_config(const char *);

extern int config_int[NUM_CONFIG_INTS];
extern char *config_str[NUM_CONFIG_STRS];

#define CONFIG_STR(x) config_str[(x)-BASE_CONFIG_STR]
#define CONFIG_INT(x) config_int[(x)-BASE_CONFIG_INT]

#endif
