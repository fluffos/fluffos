#include "base/package_api.h"

#include "vm/internal/hooks.h"

#ifdef F_HOOK_ATTACH
void f_hook_attach(int /*num_arg*/) {
  /* args: string point, mixed action, mapping spec, int flags */
  int flags = sp->u.number;
  mapping_t* spec = (sp - 1)->u.map;
  svalue_t* action = sp - 2;
  const char* point = (sp - 3)->u.string;
  LPC_INT id = hook_efun_attach(point, action, spec, flags);
  pop_n_elems(4);
  push_number(id);
}
#endif

#ifdef F_HOOK_DETACH
void f_hook_detach(int /*num_arg*/) {
  LPC_INT id = sp->u.number;
  hook_efun_detach(id);
  pop_stack();
}
#endif

#ifdef F_HOOK_QUERY
void f_hook_query(int /*num_arg*/) {
  LPC_INT id = sp->u.number;
  mapping_t* m = hook_efun_query(id);
  free_svalue(sp, "hook_query");
  sp->type = T_MAPPING;
  sp->u.map = m;
}
#endif

#ifdef F_HOOK_LIST
void f_hook_list(int num_arg) {
  object_t* ob = nullptr;
  if (num_arg == 1) {
    if (sp->type == T_OBJECT) {
      ob = sp->u.ob;
    }
    pop_stack();
  }
  array_t* arr = hook_efun_list(ob);
  push_refed_array(arr);
}
#endif
/* f_hook_proceed is defined in vm/internal/hooks.cc */
