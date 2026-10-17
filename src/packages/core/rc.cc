#include "base/package_api.h"

int get_config_item(svalue_t* res, svalue_t* arg) {
  /* an LPC int: as an int, 0x100000000 would read config item 0 */
  LPC_INT const num = arg->u.number;

  if (num < 0 || num > RC_LAST_CONFIG_INT) {
    return 0;
  }
  if (num >= RC_BASE_CONFIG_INT) {
    res->type = T_NUMBER;
    res->u.number = config_int[num - RC_BASE_CONFIG_INT];
  } else {
    if (!config_str[num]) {  // obsolete value, less muds break if we don't
                             // renumber!
      return 0;
    }
    res->type = T_STRING;
    res->subtype = STRING_CONSTANT;
    res->u.string = config_str[num];
  }
  return 1;
}

#ifdef F_GET_CONFIG
void f_get_config(LPCFrame& f) {
  if (!get_config_item(f.argp(0), f.argp(0))) {
    error("Bad argument to get_config()\n");
  }
  f.done();
}
#endif

#ifdef F_SET_CONFIG
void f_set_config(LPCFrame& f) {
  auto num = f.number(0);
  auto* value = f.argp(1);

  if (num < 0 || num > RC_LAST_CONFIG_INT) {
    error("Bad 1st argument to set_config()\n");
  }
  if (num < RC_BASE_CONFIG_INT) {
    error("Can not set string config through set_config() yet.\n");
  }

  if (value->type != T_NUMBER) {
    error("Bad 2nd argument to set_config()\n");
  }

  CONFIG_INT(num) = value->u.number;
  f.ret_void();
}
#endif
