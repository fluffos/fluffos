#include "base/package_api.h"

int get_config_item(svalue_t *res, svalue_t *arg) {
  int num;

  num = arg->u.number;

  if (num < 0 || num >= RC_LAST_CONFIG_INT) {
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
void f_get_config(void) {
  if (!get_config_item(sp, sp)) {
    error("Bad argument to get_config()\n");
  }
}
#endif

#ifdef F_SET_CONFIG
void f_set_config() {
  auto num = (sp - 1)->u.number;
  auto value = sp;

  if (num < 0 || num >= RC_LAST_CONFIG_INT) {
    pop_2_elems();
    error("Bad 1st argument to set_config()\n");
  }
  if (num < RC_BASE_CONFIG_INT) {
    pop_2_elems();
    error("Can not set string config through set_config() yet.\n");
  }

  if (value->type != T_NUMBER) {
    pop_2_elems();
    error("Bad 2nd argument to set_config()\n");
  }

  CONFIG_INT(num) = value->u.number;
  pop_2_elems();
}
#endif
