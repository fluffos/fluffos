// This is the file to use for getting access to options within base module.

// IWYU pragma: begin_exports
#include "base/internal/options.h"
#include "base/internal/options_internal.h"
#include "local_options"

#ifdef HEARTBEAT_INTERVAL
#error \
    "local_options: HEARTBEAT_INTERVAL has been moved to runtime config, you must remove it from local_options to compile.'"
#endif

#ifdef NUM_EXTERNAL_CMDS 
#error \
    "local_options: NUM_EXTERNAL_CMDS is no longer a compile time options, please remove it."
#endif

// IWYU pragma: end_exports
