#include "base/package_api.h"

#include "vm/internal/hooks.h"

#ifdef F_HOOK_ATTACH
vm::Value f_hook_attach(vm::EfunArgs&, vm::StringArg point, vm::Arg action, mapping_t* spec,
                        LPC_INT flags) {
  LPC_INT id = hook_efun_attach(point.c_str(), action.sv(), spec, static_cast<int>(flags));
  return id;
}
#endif

#ifdef F_HOOK_DETACH
void f_hook_detach(vm::EfunArgs&, LPC_INT id) { hook_efun_detach(id); }
#endif

#ifdef F_HOOK_QUERY
vm::Value f_hook_query(vm::EfunArgs&, LPC_INT id) { return vm::Value::refed(hook_efun_query(id)); }
#endif

#ifdef F_HOOK_LIST
vm::Value f_hook_list(vm::EfunArgs&, std::optional<object_t*> ob) {
  /* The argument slot keeps ob alive while the list is built. */
  return vm::Value::refed(hook_efun_list(ob.value_or(nullptr)));
}
#endif
/* f_hook_proceed is defined in vm/internal/hooks.cc */
