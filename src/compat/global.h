/*
 * global.h: this file contains definitions that are used throughout the
 *           supplied compat code
 */

#ifndef _MUDLIB_GLOBAL_H_
#define _MUDLIB_GLOBAL_H_

#include "options.h"

#ifndef __SENSIBLE_MODIFIERS__
#define protected static
#define nosave static
#endif

// Supporting daemons
#define CALL_OUT_D      "/misc/call_out"
#define HEART_BEAT_D    "/misc/heart_beat"
#define STATE_D         "/misc/state"

// Modules
#define M_CONTAINABLE   "/modules/m_containable"
#define M_CONTAINER     "/modules/m_container"
#define M_INTERACTIVE   "/modules/m_interactive"
#define M_STATE         "/modules/m_state"

#define IS_LISTENER(ob) (interactive(ob) || \
                         function_exists("catch_tell", (ob)) || \
                         function_exists("receive_message", (ob)))

#endif
