#ifndef STD_H
#define STD_H

/* This stuff should be included EVERYWHERE */

#include "arch.h"
#include "options.h"
#include "port.h"
#include "macros.h"
#ifndef _FUNC_SPEC_
#   include "std_incl.h"
#   ifndef NO_OPCODES
#      include "opcodes.h"
#   endif
/* TEMP HACK - v20.25 won't need this */
#include "debugmalloc.h"
#endif

#endif
