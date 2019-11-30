#include "base/package_api.h"

#ifdef F_DUMP_TRACE
void f_dump_trace() { push_array(get_svalue_trace()); }
#endif
