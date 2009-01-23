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

#define BASE_CONFIG_STR        0
#define CFG_STR(x) ((x) + BASE_CONFIG_STR)
/*
 * These config settings return a string
 */

#define __MUD_NAME__                CFG_STR(0)

#define __ADDR_SERVER_IP__          CFG_STR(1)

#define __MUD_LIB_DIR__             CFG_STR(2)
#define __BIN_DIR__                 CFG_STR(3)

#define __LOG_DIR__                 CFG_STR(4)
#define __INCLUDE_DIRS__            CFG_STR(5)

#define __MASTER_FILE__             CFG_STR(7)
#define __SIMUL_EFUN_FILE__         CFG_STR(8)
#define __SWAP_FILE__               CFG_STR(9)
#define __DEBUG_LOG_FILE__          CFG_STR(10)

#define __DEFAULT_ERROR_MESSAGE__   CFG_STR(11)
#define __DEFAULT_FAIL_MESSAGE__    CFG_STR(12)
#define __GLOBAL_INCLUDE_FILE__     CFG_STR(13)

#define __MUD_IP__                  CFG_STR(14)

/*
 * These config settings return an int (ie number)
 */

#define BASE_CONFIG_INT (BASE_CONFIG_STR + 15)
#define CFG_INT(x)  ((x) + BASE_CONFIG_INT)

#define __MUD_PORT__                CFG_INT(0)
#define __ADDR_SERVER_PORT__        CFG_INT(1)

#define __TIME_TO_CLEAN_UP__        CFG_INT(2)
#define __TIME_TO_RESET__           CFG_INT(3)
#define __TIME_TO_SWAP__            CFG_INT(4)

#define __COMPILER_STACK_SIZE__     CFG_INT(5)
#define __EVALUATOR_STACK_SIZE__    CFG_INT(6)
#define __INHERIT_CHAIN_SIZE__      CFG_INT(7)
#define __MAX_EVAL_COST__           CFG_INT(8)
#define __MAX_LOCAL_VARIABLES__     CFG_INT(9)
#define __MAX_CALL_DEPTH__          CFG_INT(10)
#define __MAX_ARRAY_SIZE__          CFG_INT(11)
#define __MAX_BUFFER_SIZE__         CFG_INT(12)
#define __MAX_MAPPING_SIZE__        CFG_INT(13)
#define __MAX_STRING_LENGTH__       CFG_INT(14)
#define __MAX_BITFIELD_BITS__       CFG_INT(15)

#define __MAX_BYTE_TRANSFER__       CFG_INT(16)
#define __MAX_READ_FILE_SIZE__      CFG_INT(17)

#define __RESERVED_MEM_SIZE__       CFG_INT(18)

#define __SHARED_STRING_HASH_TABLE_SIZE__ CFG_INT(19)
#define __OBJECT_HASH_TABLE_SIZE__        CFG_INT(20)
#define __LIVING_HASH_TABLE_SIZE__        CFG_INT(21)

#define __FD6_PORT__		    CFG_INT(22)
#define __FD6_KIND__		    CFG_INT(23)

#define RUNTIME_CONFIG_NEXT CFG_INT(24)

/*
 * The following is for internal use (ie driver) only
 */

#define NUM_CONFIG_STRS (BASE_CONFIG_INT - BASE_CONFIG_STR)
#define NUM_CONFIG_INTS (RUNTIME_CONFIG_NEXT - BASE_CONFIG_INT)

#endif				/* RUNTIME_CONFIG_H */
