#include "base/package_api.h"

#include <unicode/ucnv.h>

#include <memory>

namespace {
const char* default_encoding = "utf-8";
}  // namespace

#ifdef F_SET_ENCODING
vm::Value f_set_encoding(vm::EfunArgs&, std::optional<vm::StringArg> encoding) {
  if (!command_giver || !command_giver->interactive) {
    return vm::Value::str_malloced(string_copy(default_encoding, "f_set_encoding: 1"));
  }

  auto* ip = command_giver->interactive;

  // Reset to no-transcoding
  if (!encoding) {
    if (ip && ip->trans) {
      ucnv_close(ip->trans);
      ip->trans = nullptr;
    }
    return vm::Value::str_malloced(string_copy(default_encoding, "f_set_encoding: 1"));
  }

  // Set to specific encoding

  // ignore if user want utf8
  UConverter* new_trans = nullptr;
  if (ucnv_compareNames(default_encoding, encoding->c_str()) != 0) {
    UErrorCode error_code = U_ZERO_ERROR;
    new_trans = ucnv_open(encoding->c_str(), &error_code);
    if (U_FAILURE(error_code)) {
      error("Fail to set encoding to '%s', error: %s.", encoding->c_str(), u_errorName(error_code));
    }
  }

  if (ip->trans) {
    ucnv_close(ip->trans);
    ip->trans = nullptr;
  }
  ip->trans = new_trans;

  // Now let's return an canonical name
  if (!ip->trans) {
    return vm::Value::str_malloced(string_copy(default_encoding, "f_set_encoding: 2"));
  }

  UErrorCode error_code = U_ZERO_ERROR;
  const auto* name = ucnv_getName(ip->trans, &error_code);
  if (U_FAILURE(error_code)) {
    error("Fail to set encoding, ucnv_getName error: %s.", u_errorName(error_code));
  }
  return vm::Value::str_malloced(string_copy(name, "f_set_encoding: 3"));
}
#endif

#ifdef F_QUERY_ENCODING
vm::Value f_query_encoding(vm::EfunArgs&) {
  if (!command_giver || !command_giver->interactive) {
    return vm::Value::str_malloced(string_copy(default_encoding, "f_set_encoding: 1"));
  }

  const auto* res = default_encoding;

  auto* ip = command_giver->interactive;
  if (ip) {
    auto* trans = ip->trans;
    if (trans) {
      UErrorCode error_code = U_ZERO_ERROR;
      res = ucnv_getName(trans, &error_code);
      if (U_FAILURE(error_code)) {
        error("Fail to query encoding: %s.", u_errorName(error_code));
      }
    }
  }

  return vm::Value::str_malloced(string_copy(res, "f_query_encoding"));
}
#endif

#if defined(F_STRING_ENCODE) || defined(F_STRING_DECODE)
namespace {
/* error() is a C++ throw, so these owners also release on an error path:
 * an oversized result makes allocate_buffer()/new_string() error(). */
using ConverterOwner = std::unique_ptr<UConverter, decltype(&ucnv_close)>;
struct FreeMstr {
  void operator()(char* p) const { FREE_MSTR(p); }
};
}  // namespace
#endif

#ifdef F_STRING_ENCODE
vm::Value f_string_encode(vm::EfunArgs&, vm::StringArg str, vm::StringArg encoding_arg) {
  const auto* data = str.c_str();
  auto len = str.size();
  const auto* encoding = encoding_arg.c_str();

  UErrorCode error_code = U_ZERO_ERROR;
  ConverterOwner trans(ucnv_open(encoding, &error_code), &ucnv_close);
  if (U_FAILURE(error_code)) {
    error("string_encode: Invalid encoding '%s', error: %s.", encoding, u_errorName(error_code));
  }

  error_code = U_ZERO_ERROR;
  auto required = ucnv_fromAlgorithmic(trans.get(), UConverterType::UCNV_UTF8, nullptr, 0, data,
                                       len, &error_code);
  if (U_FAILURE(error_code) && error_code != U_BUFFER_OVERFLOW_ERROR) {
    error("string_encode: error: %s.", u_errorName(error_code));
  }

  size_t const translen = required;
  error_code = U_ZERO_ERROR;
  std::unique_ptr<buffer_t, decltype(&free_buffer)> buffer(allocate_buffer(translen),
                                                           &free_buffer);
  auto* transdata = (char*)buffer->item;

  auto written = ucnv_fromAlgorithmic(trans.get(), UConverterType::UCNV_UTF8, transdata, translen,
                                      data, len, &error_code);
  DEBUG_CHECK(written != translen, "Bug: translation buffer size calculation error.");
  if (U_FAILURE(error_code)) {
    error("string_encode: error: %s.", u_errorName(error_code));
  }
  return vm::Value::refed(buffer.release());
}
#endif

#ifdef F_STRING_DECODE
vm::Value f_string_decode(vm::EfunArgs&, buffer_t* buf, vm::StringArg encoding_arg) {
  auto* data = (char*)(buf->item);
  auto len = buf->size;
  const auto* encoding = encoding_arg.c_str();

  // get rid of all ending '\0's from buffer.
  while (len > 1 && data[len - 1] == '\0') len--;

  UErrorCode error_code = U_ZERO_ERROR;
  ConverterOwner trans(ucnv_open(encoding, &error_code), &ucnv_close);
  if (U_FAILURE(error_code)) {
    error("string_decode: Invalid encoding '%s', error: %s.", encoding, u_errorName(error_code));
  }

  error_code = U_ZERO_ERROR;
  auto required = ucnv_toAlgorithmic(UConverterType::UCNV_UTF8, trans.get(), nullptr, 0, data,
                                     len, &error_code);
  if (U_FAILURE(error_code) && error_code != U_BUFFER_OVERFLOW_ERROR) {
    error("string_decode: error: %s.", u_errorName(error_code));
  }

  std::unique_ptr<char, FreeMstr> res(new_string(required, "f_string_decode"));

  error_code = U_ZERO_ERROR;
  auto written = ucnv_toAlgorithmic(UConverterType::UCNV_UTF8, trans.get(), res.get(), required,
                                    data, len, &error_code);
  res.get()[required] = '\0';
  if (U_FAILURE(error_code)) {
    error("string_decode: error: %s.", u_errorName(error_code));
  }
  DEBUG_CHECK(written != required, "Bug: translation buffer size calculation error.");
  return vm::Value::str_malloced(res.release());
}
#endif

#ifdef F_BUFFER_TRANSCODE
vm::Value f_buffer_transcode(vm::EfunArgs&, buffer_t* buf, vm::StringArg from_encoding_arg,
                             vm::StringArg to_encoding_arg) {
  auto* data = (char*)(buf->item);
  auto len = buf->size;
  const auto* from_encoding = from_encoding_arg.c_str();
  const auto* to_encoding = to_encoding_arg.c_str();

  UErrorCode error_code = U_ZERO_ERROR;

  error_code = U_ZERO_ERROR;
  auto required = ucnv_convert(to_encoding, from_encoding, nullptr, 0, data, len, &error_code);
  if (error_code != U_BUFFER_OVERFLOW_ERROR) {
    error("buffer_transcode: error: %s.", u_errorName(error_code));
  }

  auto* res = allocate_buffer(required);

  error_code = U_ZERO_ERROR;
  auto written =
      ucnv_convert(to_encoding, from_encoding, (char*)res->item, required, data, len, &error_code);
  DEBUG_CHECK(written != required, "Bug: translation buffer size calculation error.");
  if (U_FAILURE(error_code)) {
    free_buffer(res);
    error("buffer_transcode: error: %s.", u_errorName(error_code));
  }
  return vm::Value::refed(res);
}
#endif
