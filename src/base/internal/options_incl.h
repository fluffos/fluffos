// This is the file to use for getting access to options within base module.
// We process this file to generate list of defines, so we can't use include guard.

// #ifndef __OPTIONS_INCL_H__
// #define __OPTIONS_INCL_H__

// IWYU pragma: begin_exports
#include "base/internal/options_internal.h"
#include "local_options"
// Must be under local_options to be effective
#include "config.h"
#include "packages/packages.autogen.h"
// IWYU pragma: end_exports

// #endif /* __OPTIONS_INCL_H__ */
