#include "base/package_api.h"

#include "packages/core/telnet_ext.h"
#include "packages/core/mssp.h"

#include "thirdparty/libtelnet/libtelnet.h"  // FIXME?

static const unsigned char telnet_mssp_value[] = {TELNET_MSSP_VAR, '%', 's', TELNET_MSSP_VAL,
                                                  '%',             's', 0};

static int send_mssp_val(mapping_t *map, mapping_node_t *el, void *data) {
  auto ip = (interactive_t *)data;

  if (el->values[0].type == T_STRING && el->values[1].type == T_STRING) {
    char buf[1024];
    int len =
        sprintf(buf, (char *)telnet_mssp_value, el->values[0].u.string, el->values[1].u.string);

    telnet_send(ip->telnet, buf, len);
  } else if (el->values[0].type == T_STRING && el->values[1].type == T_ARRAY &&
             el->values[1].u.arr->size > 0 && el->values[1].u.arr->item[0].type == T_STRING) {
    char buf[10240];
    int len = sprintf(buf, (char *)telnet_mssp_value, el->values[0].u.string,
                      el->values[1].u.arr->item[0].u.string);

    telnet_send(ip->telnet, buf, len);

    array_t *ar = el->values[1].u.arr;
    int i;
    unsigned char val = TELNET_MSSP_VAL;
    for (i = 1; i < ar->size; i++) {
      if (ar->item[i].type == T_STRING) {
        telnet_send(ip->telnet, (const char *)&val, sizeof(val));
        telnet_send(ip->telnet, (const char *)ar->item[i].u.string, SVALUE_STRLEN(&ar->item[i]));
      }
    }
    flush_message(ip);
  }
  return 0;
}

void on_telnet_do_mssp(interactive_t *ip) {
  svalue_t *res = safe_apply_master_ob(APPLY_GET_MUD_STATS, 0);
  mapping_t *map;
  if (res <= (svalue_t *)0 || res->type != T_MAPPING) {
    map = allocate_mapping(0);
    free_svalue(&apply_ret_value, "telnet neg");
    apply_ret_value.type = T_MAPPING;
    apply_ret_value.u.map = map;
  } else {
    map = res->u.map;
  }

  telnet_begin_sb(ip->telnet, TELNET_TELOPT_MSSP);

  // ok, so we have a mapping, first make sure we send the required
  // values
  char *tmp = findstring("NAME");
  if (tmp) {
    svalue_t *name = find_string_in_mapping(map, tmp);
    if (!name || name->type != T_STRING) {
      tmp = 0;
    }
  }
  if (!tmp) {
    telnet_printf(ip->telnet, (char *)telnet_mssp_value, "NAME", CONFIG_STR(__MUD_NAME__));
  }
  tmp = findstring("PLAYERS");
  if (tmp) {
    svalue_t *players = find_string_in_mapping(map, tmp);
    if (!players || players->type != T_STRING) {
      tmp = 0;
    }
  }
  if (!tmp) {
    char num[5] = {};
    snprintf(num, sizeof(num), "%d", users_num(true));
    telnet_printf(ip->telnet, (char *)telnet_mssp_value, "PLAYERS", num);
  }
  tmp = findstring("UPTIME");
  if (tmp) {
    svalue_t *upt = find_string_in_mapping(map, tmp);
    if (!upt || upt->type != T_STRING) {
      tmp = 0;
    }
  }
  if (!tmp) {
    char num[20] = {};
    snprintf(num, sizeof(num), "%ld", boot_time);
    telnet_printf(ip->telnet, (char *)telnet_mssp_value, "UPTIME", num);
  }
  // now send the rest
  mapTraverse(map, send_mssp_val, ip);
  telnet_finish_sb(ip->telnet);
}
