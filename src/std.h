#ifndef STD_H
#define STD_H

/* This stuff should be included EVERYWHERE */

#include "arch.h"
#include "options.h"
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
