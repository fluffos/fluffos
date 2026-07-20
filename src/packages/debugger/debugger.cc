#include "base/package_api.h"

#include "debugger/debug_hook.h"

#ifdef F_DEBUGGER_ATTACHED
void f_debugger_attached() { push_number(lpc_debugger_attached()); }
#endif

#ifdef F_DEBUG_BREAK
void f_debug_break() { lpc_debugger_break(); }
#endif
