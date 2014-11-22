#include "base/package_api.h"

int get_config_item(svalue_t *res, svalue_t *arg) {
  int num;

  num = arg->u.number;

  if (num < 0 || num >= RUNTIME_CONFIG_NEXT) {
    return 0;
  }
  if (num >= BASE_CONFIG_INT) {
    res->type = T_NUMBER;
    res->u.number = config_int[num - BASE_CONFIG_INT];
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
