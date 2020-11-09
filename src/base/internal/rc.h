#ifndef RC_H
#define RC_H

/*
 * rc.c
 */

#include "include/runtime_config.h"

/*
 * The following is for internal use (ie driver) only
 */
#define NUM_CONFIG_STRS (RC_LAST_CONFIG_STR - RC_BASE_CONFIG_STR)
#define NUM_CONFIG_INTS (RC_LAST_CONFIG_INT - RC_BASE_CONFIG_INT)

// How many external cmds are supported.
constexpr int g_num_external_cmds = 100;
extern char *external_cmd[g_num_external_cmds];

void read_config(char *);
void print_rc_table();

extern int config_int[NUM_CONFIG_INTS];
extern char *config_str[NUM_CONFIG_STRS];

#define CONFIG_STR(x) config_str[(x)-RC_BASE_CONFIG_STR]
#define CONFIG_INT(x) config_int[(x)-RC_BASE_CONFIG_INT]

#endif
