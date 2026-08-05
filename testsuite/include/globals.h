// file: globals.h

#ifdef __SENSIBLE_MODIFIERS__
#define staticf protected
#define staticv nosave
#else
#define staticf static
#define staticv static
#endif

#include "tests.h"

// Runtime config slots for get_config()/set_config().  The driver predefines
// the compile-time __CFG_*__ constants but not these slot numbers, so mirror
// the ones tests need from src/include/runtime_config.h (CFG_INT(n) is
// n + RC_BASE_CONFIG_INT, and RC_BASE_CONFIG_INT is 256).
#define __MAX_ARRAY_SIZE__   267  // CFG_INT(11)
#define __MAX_MAPPING_SIZE__ 269  // CFG_INT(13)

#define SINGLE_DIR "/single"
#define CONFIG_DIR "/etc"
#define LOG_DIR    "/log"

#define VIRTUAL_D  "/single/virtual"

#define LOGIN_OB   "/clone/login"
#define USER_OB    "/clone/user"
#define VOID_OB    "/single/void"

#define ROOT_UID     "Root"
#define BACKBONE_UID "Backbone"

#define BASE            "/inherit/base"
#define OVERRIDES_FILE  "/single/simul_efun"

#define COMMAND_PREFIX "/command/"

// classes for message() efun.
#define M_STATUS "status"
#define M_SAY    "say"

#define CLEAN_UP "/inherit/clean_up"
