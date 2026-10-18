#include "base/package_api.h"

#include "vm/internal/hooks.h"

#ifdef F_HOOK_ATTACH
Value f_hook_attach(EfunArgs&, StringArg point, Arg action, mapping_t* spec, LPC_INT flags) {
  LPC_INT id = hook_efun_attach(point.c_str(), action.sv(), spec, static_cast<int>(flags));
  return id;
}
#endif

#ifdef F_HOOK_DETACH
void f_hook_detach(EfunArgs&, LPC_INT id) { hook_efun_detach(id); }
#endif

#ifdef F_HOOK_QUERY
Value f_hook_query(EfunArgs&, LPC_INT id) { return Value::refed(hook_efun_query(id)); }
#endif

#ifdef F_HOOK_LIST
Value f_hook_list(EfunArgs&, std::optional<object_t*> ob) {
  /* The argument slot keeps ob alive while the list is built. */
  return Value::refed(hook_efun_list(ob.value_or(nullptr)));
}
#endif
/* f_hook_proceed is defined in vm/internal/hooks.cc */
