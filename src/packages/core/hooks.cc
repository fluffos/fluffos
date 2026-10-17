#include "base/package_api.h"

#include "vm/internal/hooks.h"

#ifdef F_HOOK_ATTACH
void f_hook_attach(LPCFrame& f) {
  /* args: string point, mixed action, mapping spec, int flags */
  const char* point = f.string(0);
  svalue_t* action = f.argp(1);
  mapping_t* spec = f.mapping(2);
  int flags = static_cast<int>(f.number(3));
  LPC_INT id = hook_efun_attach(point, action, spec, flags);
  f.ret(id);
}
#endif

#ifdef F_HOOK_DETACH
void f_hook_detach(LPCFrame& f) {
  LPC_INT id = f.number(0);
  hook_efun_detach(id);
  f.ret_void();
}
#endif

#ifdef F_HOOK_QUERY
void f_hook_query(LPCFrame& f) {
  LPC_INT id = f.number(0);
  f.ret_refed(hook_efun_query(id));
}
#endif

#ifdef F_HOOK_LIST
void f_hook_list(LPCFrame& f) {
  /* The argument slot keeps ob alive while the list is built. */
  object_t* ob = (f.argc() == 1 && f.type(0) == T_OBJECT) ? f.object(0) : nullptr;
  f.ret_refed(hook_efun_list(ob));
}
#endif
/* f_hook_proceed is defined in vm/internal/hooks.cc */
