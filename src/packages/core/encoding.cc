#include "base/package_api.h"

#include <unicode/ucnv.h>

namespace {
const char *DEFAULT_ENCODING = "utf-8";
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
  UConverter *new_trans = nullptr;
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

#ifdef F_STRING_ENCODE
void f_string_encode() {
  auto encoding = sp->u.string;
  auto data = (sp - 1)->u.string;
  auto len = SVALUE_STRLEN(sp - 1);

  UErrorCode error_code = U_ZERO_ERROR;
  auto trans = ucnv_open(encoding, &error_code);
  if (U_FAILURE(error_code)) {
    error("string_encode: Invalid encoding '%s', error: %s.", encoding, u_errorName(error_code));
  }

  error_code = U_ZERO_ERROR;
  auto required =
      ucnv_fromAlgorithmic(trans, UConverterType::UCNV_UTF8, nullptr, 0, data, len, &error_code);
  if (U_FAILURE(error_code) && error_code != U_BUFFER_OVERFLOW_ERROR) {
    ucnv_close(trans);
    error("string_encode: error: %s.", u_errorName(error_code));
  }

  size_t translen = required;
  error_code = U_ZERO_ERROR;
  auto buffer = allocate_buffer(translen);
  auto transdata = (char *)buffer->item;

  auto written = ucnv_fromAlgorithmic(trans, UConverterType::UCNV_UTF8, transdata, translen, data,
                                      len, &error_code);
  DEBUG_CHECK(written != translen, "Bug: translation buffer size calculation error.");
  if (U_FAILURE(error_code)) {
    ucnv_close(trans);
    free_buffer(buffer);
    error("string_encode: error: %s.", u_errorName(error_code));
  }
  pop_2_elems();
  push_refed_buffer(buffer);

  ucnv_close(trans);
}
#endif

#ifdef F_STRING_DECODE
void f_string_decode() {
  auto encoding = sp->u.string;
  auto data = (char *)((sp - 1)->u.buf->item);
  auto len = (sp - 1)->u.buf->size;

  UErrorCode error_code = U_ZERO_ERROR;
  auto trans = ucnv_open(encoding, &error_code);
  if (U_FAILURE(error_code)) {
    error("string_decode: Invalid encoding '%s', error: %s.", encoding, u_errorName(error_code));
  }

  error_code = U_ZERO_ERROR;
  auto required =
      ucnv_toAlgorithmic(UConverterType::UCNV_UTF8, trans, nullptr, 0, data, len, &error_code);
  if (U_FAILURE(error_code) && error_code != U_BUFFER_OVERFLOW_ERROR) {
    ucnv_close(trans);
    error("string_decode: error: %s.", u_errorName(error_code));
  }

  auto res = new_string(required, "f_string_decode");

  error_code = U_ZERO_ERROR;
  auto written =
      ucnv_toAlgorithmic(UConverterType::UCNV_UTF8, trans, res, required, data, len, &error_code);
  res[required] = '\0';
  if (U_FAILURE(error_code)) {
    ucnv_close(trans);
    FREE_MSTR(res);
    error("string_decode: error: %s.", u_errorName(error_code));
  }
  DEBUG_CHECK(written != required, "Bug: translation buffer size calculation error.");
  pop_2_elems();
  push_malloced_string(res);

  ucnv_close(trans);
}
#endif

#ifdef F_BUFFER_TRANSCODE
void f_buffer_transcode() {
  auto to_encoding = sp->u.string;
  auto from_encoding = (sp - 1)->u.string;
  auto data = (char *)((sp - 2)->u.buf->item);
  auto len = (sp - 2)->u.buf->size;

  UErrorCode error_code = U_ZERO_ERROR;

  error_code = U_ZERO_ERROR;
  auto required = ucnv_convert(to_encoding, from_encoding, nullptr, 0, data, len, &error_code);
  if (error_code != U_BUFFER_OVERFLOW_ERROR) {
    error("buffer_transcode: error: %s.", u_errorName(error_code));
  }

  auto res = allocate_buffer(required);

  error_code = U_ZERO_ERROR;
  auto written =
      ucnv_convert(to_encoding, from_encoding, (char *)res->item, required, data, len, &error_code);
  DEBUG_CHECK(written != required, "Bug: translation buffer size calculation error.");
  if (U_FAILURE(error_code)) {
    free_buffer(res);
    error("buffer_transcode: error: %s.", u_errorName(error_code));
  }
  pop_3_elems();
  push_refed_buffer(res);
}
#endif
