// This is the file to use for getting access to options within base module.

// IWYU pragma: begin_exports
#include "base/internal/options.h"
#include "base/internal/options_internal.h"
#include "local_options"

#ifdef HEARTBEAT_INTERVAL
#error \
    "HEARTBEAT_INTERVAL has been moved to runtime config, you must remove it from local_options to compile.'"
#endif

// IWYU pragma: end_exports
