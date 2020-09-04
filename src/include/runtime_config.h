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
 *   Neither the name or the number can be changed, or reused, avoid breaking mudlib code.
 */

#ifndef RUNTIME_CONFIG_H
#define RUNTIME_CONFIG_H

#define RC_BASE_CONFIG_STR 0
#define CFG_STR(x) ((x) + RC_BASE_CONFIG_STR)
/*
 * These config settings return a string
 */

#define __MUD_NAME__ CFG_STR(0)
#define __RC_STR_1__ CFG_STR(1)
#define __MUD_LIB_DIR__ CFG_STR(2)
#define __BIN_DIR__ CFG_STR(3)
#define __LOG_DIR__ CFG_STR(4)
#define __INCLUDE_DIRS__ CFG_STR(5)
#define __RC_STR_2__ CFG_STR(6)
#define __MASTER_FILE__ CFG_STR(7)
#define __SIMUL_EFUN_FILE__ CFG_STR(8)
#define __SWAP_FILE__ CFG_STR(9)
#define __DEBUG_LOG_FILE__ CFG_STR(10)
#define __DEFAULT_ERROR_MESSAGE__ CFG_STR(11)
#define __DEFAULT_FAIL_MESSAGE__ CFG_STR(12)
#define __GLOBAL_INCLUDE_FILE__ CFG_STR(13)
#define __MUD_IP__ CFG_STR(14)
#define __RC_WEBSOCKET_HTTP_DIR__ CFG_STR(15)

#define RC_LAST_CONFIG_STR CFG_STR(255)
/*
 * These config settings return an int (ie number)
 */
#define RC_BASE_CONFIG_INT (RC_LAST_CONFIG_STR + 1)
#define CFG_INT(x) ((x) + RC_BASE_CONFIG_INT)

#define __MUD_PORT__ CFG_INT(0)
#define __RC_INT_1__ CFG_INT(1)
#define __TIME_TO_CLEAN_UP__ CFG_INT(2)
#define __TIME_TO_RESET__ CFG_INT(3)
#define __TIME_TO_SWAP__ CFG_INT(4)
#define __COMPILER_STACK_SIZE__ CFG_INT(5)
#define __EVALUATOR_STACK_SIZE__ CFG_INT(6)
#define __INHERIT_CHAIN_SIZE__ CFG_INT(7)
#define __MAX_EVAL_COST__ CFG_INT(8)
#define __MAX_LOCAL_VARIABLES__ CFG_INT(9)
#define __MAX_CALL_DEPTH__ CFG_INT(10)
#define __MAX_ARRAY_SIZE__ CFG_INT(11)
#define __MAX_BUFFER_SIZE__ CFG_INT(12)
#define __MAX_MAPPING_SIZE__ CFG_INT(13)
#define __MAX_STRING_LENGTH__ CFG_INT(14)
#define __MAX_BITFIELD_BITS__ CFG_INT(15)
#define __MAX_BYTE_TRANSFER__ CFG_INT(16)
#define __MAX_READ_FILE_SIZE__ CFG_INT(17)
#define __RC_INT_18__ CFG_INT(18)
#define __SHARED_STRING_HASH_TABLE_SIZE__ CFG_INT(19)
#define __OBJECT_HASH_TABLE_SIZE__ CFG_INT(20)
#define __LIVING_HASH_TABLE_SIZE__ CFG_INT(21)
#define __RC_INT_22__ CFG_INT(22)
#define __RC_INT_23__ CFG_INT(23)
#define __RC_GAMETICK_MSEC__ CFG_INT(24)
#define __RC_HEARTBEAT_INTERVAL_MSEC__ CFG_INT(25)
#define __RC_SANE_EXPLODE_STRING__ CFG_INT(26)
#define __RC_REVERSIBLE_EXPLODE_STRING__ CFG_INT(27)
#define __RC_SANE_SORTING__ CFG_INT(28)
#define __RC_WARN_TAB__ CFG_INT(29)
#define __RC_WOMBLES__ CFG_INT(30)
#define __RC_CALL_OTHER_TYPE_CHECK__ CFG_INT(31)
#define __RC_CALL_OTHER_WARN__ CFG_INT(32)
#define __RC_MUDLIB_ERROR_HANDLER__ CFG_INT(33)
#define __RC_NO_RESETS__ CFG_INT(34)
#define __RC_LAZY_RESETS__ CFG_INT(35)
#define __RC_RANDOMIZED_RESETS__ CFG_INT(36)
#define __RC_NO_ANSI__ CFG_INT(37)
#define __RC_STRIP_BEFORE_PROCESS_INPUT__ CFG_INT(38)
#define __RC_THIS_PLAYER_IN_CALL_OUT__ CFG_INT(39)
#define __RC_TRACE__ CFG_INT(40)
#define __RC_TRACE_CODE__ CFG_INT(41)
#define __RC_INTERACTIVE_CATCH_TELL__ CFG_INT(42)
#define __RC_RECEIVE_SNOOP__ CFG_INT(43)
#define __RC_SNOOP_SHADOWED__ CFG_INT(44)
#define __RC_REVERSE_DEFER__ CFG_INT(45)
#define __RC_HAS_CONSOLE__ CFG_INT(46)
#define __RC_NONINTERACTIVE_STDERR_WRITE__ CFG_INT(47)
#define __RC_TRAP_CRASHES__ CFG_INT(48)
#define __RC_OLD_TYPE_BEHAVIOR__ CFG_INT(49)
#define __RC_OLD_RANGE_BEHAVIOR__ CFG_INT(50)
#define __RC_WARN_OLD_RANGE_BEHAVIOR__ CFG_INT(51)
#define __RC_SUPPRESS_ARGUMENT_WARNINGS__ CFG_INT(52)
#define __RC_ENABLE_COMMANDS_CALL_INIT__ CFG_INT(53)
#define __RC_SPRINTF_ADD_JUSTFIED_IGNORE_ANSI_COLORS__ CFG_INT(54)
#define __RC_APPLY_CACHE_BITS__ CFG_INT(55)
#define __RC_CALL_OUT_ZERO_NEST_LEVEL__ CFG_INT(56)
#define __RC_TRACE_CONTEXT__ CFG_INT(57)
#define __RC_TRACE_INSTR__ CFG_INT(58)
#define __RC_ENABLE_MXP__ CFG_INT(59)
#define __RC_ENABLE_GMCP__ CFG_INT(60)
#define __RC_ENABLE_ZMP__ CFG_INT(61)
#define __RC_ENABLE_MSSP__ CFG_INT(62)

#define RC_LAST_CONFIG_INT CFG_INT(255)
#endif /* RUNTIME_CONFIG_H */
