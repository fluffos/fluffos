/*
 * runtime_config.h
 *
 * Copy this file to your mudlib include dir for use with the
 * get_config() efun.
 *
 * See Config.Example describing most of these settings.  Some are actually
 * in options.h, but the mudlib doesn't have to know...
 *
 * Note for backwards compatibility:
 *
 *    get_config(__MUD_NAME__) == MUD_NAME
 *    get_config(__MUD_PORT__) == __PORT__
 */

#ifndef RUNTIME_CONFIG_H
#define RUNTIME_CONFIG_H

/*
 * These config settings return a string
 */

#define __MUD_NAME__                0

#define __ADDR_SERVER_IP__          1

#define __MUD_LIB_DIR__             2
#define __BIN_DIR__                 3

#define __LOG_DIR__                 4
#define __INCLUDE_DIRS__            5
#define __SAVE_BINARIES_DIR__       6

#define __MASTER_FILE__             7
#define __SIMUL_EFUN_FILE__         8
#define __SWAP_FILE__               9
#define __DEBUG_LOG_FILE__          10

#define __DEFAULT_ERROR_MESSAGE__   11
#define __DEFAULT_FAIL_MESSAGE__    12

/*
 * These config settings return an int (ie number)
 */

#define __MUD_PORT__                13
#define __ADDR_SERVER_PORT__        14

#define __TIME_TO_CLEAN_UP__        15
#define __TIME_TO_RESET__           16
#define __TIME_TO_SWAP__            17

#define __MAX_USERS__               18
#define __MAX_EFUN_SOCKS__          19

#define __COMPILER_STACK_SIZE__     20
#define __EVALUATOR_STACK_SIZE__    21
#define __INHERIT_CHAIN_SIZE__      22
#define __MAX_EVAL_COST__           23
#define __MAX_LOCAL_VARIABLES__     24
#define __MAX_CALL_DEPTH__          25

#define __MAX_ARRAY_SIZE__          26
#define __MAX_BUFFER_SIZE__         27
#define __MAX_MAPPING_SIZE__        28
#define __MAX_STRING_LENGTH__       29
#define __MAX_BITFIELD_BITS__       30

#define __MAX_BYTE_TRANSFER__       31
#define __MAX_READ_FILE_SIZE__      32

#define __RESERVED_MEM_SIZE__       33

#define __SHARED_STRING_HASH_TABLE_SIZE__ 34
#define __OBJECT_HASH_TABLE_SIZE__        35
#define __LIVING_HASH_TABLE_SIZE__        36

/*
 * The following are obsolete.
 */

#if 0

#undef __CONFIG_DIR__
#undef __DOMAIN_DIR__
#undef __ACCESS_ALLOW__
#undef __ACCESS_LOG__

#undef __ALLOWED_ED_COMMANDS__
#undef __MAX_COMMANDS_PER_HEARTBEAT__

#endif


/*
 * The following is for internal use (ie driver) only
 */

#define BASE_CONFIG_STR 0
#define BASE_CONFIG_INT 13

#define RUNTIME_CONFIG_BASE BASE_CONFIG_STR
#define RUNTIME_CONFIG_NEXT 37

#define NUM_CONFIG_STRS (BASE_CONFIG_INT - BASE_CONFIG_STR)
#define NUM_CONFIG_INTS (RUNTIME_CONFIG_NEXT - BASE_CONFIG_INT)

#define GET_CONFIG_STR(x) get_config_str(x - BASE_CONFIG_STR)
#define GET_CONFIG_INT(x) get_config_int(x - BASE_CONFIG_INT)

#endif				/* RUNTIME_CONFIG_H */
