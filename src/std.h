#ifndef STD_H
#define STD_H

/* This stuff should be included EVERYWHERE */

/* the definition of ARCH */
#include "arch.h"

#ifndef EDIT_SOURCE
/* all options and configuration */
#include "options_incl.h"
#include "configure.h"
#endif

#include "portability.h"
#include "macros.h"
#ifndef _FUNC_SPEC_
#   include "std_incl.h"
#   include "malloc_incl.h"
#   ifndef NO_OPCODES
#      include "opcodes.h"
#   endif
#endif

#endif






