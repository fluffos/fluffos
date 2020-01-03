#include "base/package_api.h"

#include <unicode/ucnv.h>

namespace {
const char* DEFAULT_ENCODING = "utf-8";
}  // namespace

#ifdef F_SET_ENCODING
void f_set_encoding() {
  if (!command_giver || !command_giver->interactive) {
    if (st_num_arg) {
      pop_stack();
    }
    push_malloced_string(string_copy(DEFAULT_ENCODING, "f_set_encoding: 1"));
    return;
  }

  auto ip = command_giver->interactive;

  // Reset to no-transcoding
  if (!st_num_arg) {
    if (ip && ip->trans) {
      ucnv_close(ip->trans);
      ip->trans = nullptr;
    }
    push_malloced_string(string_copy(DEFAULT_ENCODING, "f_set_encoding: 1"));
    return;
  }

  // Set to specific encoding

  // ignore if user want utf8
  UConverter* new_trans = nullptr;
  if (ucnv_compareNames(DEFAULT_ENCODING, sp->u.string) != 0) {
    UErrorCode error_code = U_ZERO_ERROR;
    new_trans = ucnv_open(sp->u.string, &error_code);
    if (U_FAILURE(error_code)) {
      error("Fail to set encoding to '%s', error: %s.", sp->u.string, u_errorName(error_code));
    }
  }

  if (ip->trans) {
    ucnv_close(ip->trans);
    ip->trans = nullptr;
  }
  ip->trans = new_trans;

  pop_stack();

  // Now let's return an canonical name
  if (!ip->trans) {
    push_malloced_string(string_copy(DEFAULT_ENCODING, "f_set_encoding: 2"));
    return;
  }

  UErrorCode error_code = U_ZERO_ERROR;
  auto name = ucnv_getName(ip->trans, &error_code);
  if (U_FAILURE(error_code)) {
    error("Fail to set encoding, ucnv_getName error: %s.", u_errorName(error_code));
  }
  push_malloced_string(string_copy(name, "f_set_encoding: 3"));
}
#endif

#ifdef F_QUERY_ENCODING
void f_query_encoding() {
  if (!command_giver || !command_giver->interactive) {
    if (st_num_arg) {
      pop_stack();
    }
    push_malloced_string(string_copy(DEFAULT_ENCODING, "f_set_encoding: 1"));
    return;
  }

  auto res = DEFAULT_ENCODING;

  auto ip = command_giver->interactive;
  if (ip) {
    auto trans = ip->trans;
    if (trans) {
      UErrorCode error_code = U_ZERO_ERROR;
      res = ucnv_getName(trans, &error_code);
      if (U_FAILURE(error_code)) {
        error("Fail to query encoding: %s.", u_errorName(error_code));
      }
    }
  }

  push_malloced_string(string_copy(res, "f_query_encoding"));
}
#endif
