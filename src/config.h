/*
   config.h: do not change anything in this file.  The user definable
   options have been moved into the options.h file.
*/

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "include/runtime_config.h"
#include "rc.h"

/*
 * runtime config strings.  change these values in the runtime configuration
 * file (config.example)
 */

#define MUD_NAME                GET_CONFIG_STR(__MUD_NAME__)
#define ADDR_SERVER_IP          GET_CONFIG_STR(__ADDR_SERVER_IP__)
#define MUD_LIB                 GET_CONFIG_STR(__MUD_LIB_DIR__)
#define BIN_DIR                 GET_CONFIG_STR(__BIN_DIR__)
#define LOG_DIR                 GET_CONFIG_STR(__LOG_DIR__)
#define INCLUDE_DIRS            GET_CONFIG_STR(__INCLUDE_DIRS__)
#define SAVE_BINARIES           GET_CONFIG_STR(__SAVE_BINARIES_DIR__)
#define MASTER_FILE             GET_CONFIG_STR(__MASTER_FILE__)
#define SIMUL_EFUN              GET_CONFIG_STR(__SIMUL_EFUN_FILE__)
#define SWAP_FILE               GET_CONFIG_STR(__SWAP_FILE__)
#define DEBUG_LOG_FILE          GET_CONFIG_STR(__DEBUG_LOG_FILE__)
#define DEFAULT_ERROR_MESSAGE   GET_CONFIG_STR(__DEFAULT_ERROR_MESSAGE__)
#define DEFAULT_FAIL_MESSAGE    GET_CONFIG_STR(__DEFAULT_FAIL_MESSAGE__)
#define GLOBAL_INCLUDE_FILE     GET_CONFIG_STR(__GLOBAL_INCLUDE_FILE__)

/*
 * runtime config ints
 */

#define PORTNO                  GET_CONFIG_INT(__MUD_PORT__)
#define ADDR_SERVER_PORT        GET_CONFIG_INT(__ADDR_SERVER_PORT__)
#define TIME_TO_CLEAN_UP        GET_CONFIG_INT(__TIME_TO_CLEAN_UP__)
#define TIME_TO_RESET           GET_CONFIG_INT(__TIME_TO_RESET__)
#define TIME_TO_SWAP            GET_CONFIG_INT(__TIME_TO_SWAP__)
#define MAX_COST                GET_CONFIG_INT(__MAX_EVAL_COST__)
#define MAX_BITS                GET_CONFIG_INT(__MAX_BITFIELD_BITS__)
#define MAX_ARRAY_SIZE          GET_CONFIG_INT(__MAX_ARRAY_SIZE__)
#define MAX_BUFFER_SIZE         GET_CONFIG_INT(__MAX_BUFFER_SIZE__)
#define MAX_MAPPING_SIZE        GET_CONFIG_INT(__MAX_MAPPING_SIZE__)
#define MAX_STRING_LENGTH       GET_CONFIG_INT(__MAX_STRING_LENGTH__)
#define READ_FILE_MAX_SIZE      GET_CONFIG_INT(__MAX_READ_FILE_SIZE__)
#define MAX_BYTE_TRANSFER       GET_CONFIG_INT(__MAX_BYTE_TRANSFER__)
#define RESERVED_SIZE           GET_CONFIG_INT(__RESERVED_MEM_SIZE__)
#define HTABLE_SIZE             GET_CONFIG_INT(__SHARED_STRING_HASH_TABLE_SIZE__)
#define OTABLE_SIZE             GET_CONFIG_INT(__OBJECT_HASH_TABLE_SIZE__)
#define INHERIT_CHAIN_SIZE      GET_CONFIG_INT(__INHERIT_CHAIN_SIZE__)

#ifdef USE_POSIX_SIGNALS
#define sigblock(m) port_sigblock(m)
#define sigmask(s)  port_sigmask(s)
#define signal(s,f) port_signal(s,f)
#define sigsetmask(m)   port_sigsetmask(m)
#endif

#define SETJMP(x) setjmp(x)
#define LONGJMP(x,y) longjmp(x,y)

#ifndef HAS_UALARM
#define SYSV_HEARTBEAT_INTERVAL  ((HEARTBEAT_INTERVAL+999999)/1000000)
#endif

#define APPLY_CACHE_SIZE (1 << APPLY_CACHE_BITS)

#define NUM_CONSTS 5

#define NULL_MSG "0"
#endif
