/*
   config.h: do not change anything in this file.  The user definable
   options have been moved into the options.h file.
*/

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "include/runtime_config.h"

extern int config_int[NUM_CONFIG_INTS];
extern char *config_str[NUM_CONFIG_STRS];

#define CONFIG_STR(x) config_str[(x)-BASE_CONFIG_STR]
#define CONFIG_INT(x) config_int[(x)-BASE_CONFIG_INT]

#endif
