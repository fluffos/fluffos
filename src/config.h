/*
   config.h: do not change anything in this file.  The user definable
   options have been moved into the options.h file.
*/

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "include/runtime_config.h"

/*
 * runtime config strings.  change these values in the runtime configuration
 * file (config.example)
 */

#define CONFIG_STR(x)           config_str[(x) - BASE_CONFIG_STR]
#define CONFIG_INT(x)           config_int[(x) - BASE_CONFIG_INT]

#define MUD_NAME                CONFIG_STR(__MUD_NAME__)
#define ADDR_SERVER_IP          CONFIG_STR(__ADDR_SERVER_IP__)
#define MUD_LIB                 CONFIG_STR(__MUD_LIB_DIR__)
#define BIN_DIR                 CONFIG_STR(__BIN_DIR__)
#define LOG_DIR                 CONFIG_STR(__LOG_DIR__)
#define INCLUDE_DIRS            CONFIG_STR(__INCLUDE_DIRS__)
#define SAVE_BINARIES           CONFIG_STR(__SAVE_BINARIES_DIR__)
#define MASTER_FILE             CONFIG_STR(__MASTER_FILE__)
#define SIMUL_EFUN              CONFIG_STR(__SIMUL_EFUN_FILE__)
#define SWAP_FILE               CONFIG_STR(__SWAP_FILE__)
#define DEBUG_LOG_FILE          CONFIG_STR(__DEBUG_LOG_FILE__)
#define DEFAULT_ERROR_MESSAGE   CONFIG_STR(__DEFAULT_ERROR_MESSAGE__)
#define DEFAULT_FAIL_MESSAGE    CONFIG_STR(__DEFAULT_FAIL_MESSAGE__)
#define GLOBAL_INCLUDE_FILE     CONFIG_STR(__GLOBAL_INCLUDE_FILE__)
#define MUD_IP                  CONFIG_STR(__MUD_IP__)

/*
 * runtime config ints
 */

#define PORTNO                  CONFIG_INT(__MUD_PORT__)
#define ADDR_SERVER_PORT        CONFIG_INT(__ADDR_SERVER_PORT__)
#define TIME_TO_CLEAN_UP        CONFIG_INT(__TIME_TO_CLEAN_UP__)
#define TIME_TO_RESET           CONFIG_INT(__TIME_TO_RESET__)
#define TIME_TO_SWAP            CONFIG_INT(__TIME_TO_SWAP__)
#define MAX_COST                CONFIG_INT(__MAX_EVAL_COST__)
#define MAX_BITS                CONFIG_INT(__MAX_BITFIELD_BITS__)
#define MAX_ARRAY_SIZE          CONFIG_INT(__MAX_ARRAY_SIZE__)
#define MAX_BUFFER_SIZE         CONFIG_INT(__MAX_BUFFER_SIZE__)
#define MAX_MAPPING_SIZE        CONFIG_INT(__MAX_MAPPING_SIZE__)
#define MAX_STRING_LENGTH       CONFIG_INT(__MAX_STRING_LENGTH__)
#define READ_FILE_MAX_SIZE      CONFIG_INT(__MAX_READ_FILE_SIZE__)
#define MAX_BYTE_TRANSFER       CONFIG_INT(__MAX_BYTE_TRANSFER__)
#define RESERVED_SIZE           CONFIG_INT(__RESERVED_MEM_SIZE__)
#define HTABLE_SIZE             CONFIG_INT(__SHARED_STRING_HASH_TABLE_SIZE__)
#define OTABLE_SIZE             CONFIG_INT(__OBJECT_HASH_TABLE_SIZE__)
#define INHERIT_CHAIN_SIZE      CONFIG_INT(__INHERIT_CHAIN_SIZE__)
#define FD6_PORT		CONFIG_INT(__FD6_PORT__)
#define FD6_KIND		CONFIG_INT(__FD6_KIND__)

#ifdef USE_POSIX_SIGNALS
#define sigblock(m) port_sigblock(m)
#define sigmask(s)  port_sigmask(s)
#define signal(s,f) port_signal(s,f)
#define sigsetmask(m)   port_sigsetmask(m)
#endif

#define SETJMP(x) setjmp(x)
#define LONGJMP(x,y) longjmp(x,y)

#define APPLY_CACHE_SIZE (1 << APPLY_CACHE_BITS)

#define NUM_CONSTS 5

#define NULL_MSG "0"

extern int config_int[NUM_CONFIG_INTS];
extern char *config_str[NUM_CONFIG_STRS];
#endif
