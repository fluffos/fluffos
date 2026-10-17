#include "base/package_api.h"

#include "vm/internal/hooks.h"

#ifdef F_HOOK_ATTACH
void f_hook_attach(int num_arg) {
  /* args: string point, mixed action, mapping spec, int flags */
  LPCFrame f = LPCFrame::args(num_arg);
  const char* point = f.string(0);
  svalue_t* action = f.argp(1);
  mapping_t* spec = f.mapping(2);
  int flags = static_cast<int>(f.number(3));
  LPC_INT id = hook_efun_attach(point, action, spec, flags);
  f.ret(id);
}
#endif

#ifdef F_HOOK_DETACH
void f_hook_detach(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  LPC_INT id = f.number(0);
  hook_efun_detach(id);
  f.ret_void();
}
#endif

#ifdef F_HOOK_QUERY
void f_hook_query(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  LPC_INT id = f.number(0);
  mapping_t* m = hook_efun_query(id);
  f.ret(m);
}
#endif

#ifdef F_HOOK_LIST
void f_hook_list(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  object_t* ob = nullptr;
  if (num_arg == 1) {
    if (f.type(0) == T_OBJECT) {
      ob = f.object(0);
      /* Keep alive across the stack pop: stack may hold the only ref. */
      add_ref(ob, "hook_list");
    }
    f.ret_void();
  } else {
    f.done();
  }
  array_t* arr = hook_efun_list(ob);
  if (ob) {
    free_object(&ob, "hook_list");
  }
  LPCFrame out = LPCFrame::builder();
  out.push_refed(arr);
  out.done();
}
#endif
/* f_hook_proceed is defined in vm/internal/hooks.cc */
