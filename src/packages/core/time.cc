#include "base/package_api.h"

#include <chrono>
#include <string>
#include <iomanip>
#include <sstream>
#include <vector>

#ifdef F_PERF_COUNTER_NS
void f_perf_counter_ns(LPCFrame& f) {
#ifdef _WIN32
  static LARGE_INTEGER Frequency{};
  if (Frequency.QuadPart == 0) {
    QueryPerformanceFrequency(&Frequency);
  }

  LARGE_INTEGER t;
  QueryPerformanceCounter(&t);

  f.ret(static_cast<LPC_INT>(t.QuadPart * 1000000000 / Frequency.QuadPart));
#else
  auto now = std::chrono::high_resolution_clock::now();
  f.ret(static_cast<LPC_INT>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count()));
#endif
}
#endif

#ifdef F_TIME
void f_time(LPCFrame& f) {
  f.ret(static_cast<LPC_INT>(get_current_time()));
}
#endif

#ifdef F_TIME_NS
void f_time_ns(LPCFrame& f) {
  auto now = std::chrono::system_clock::now();
  f.ret(static_cast<LPC_INT>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count()));
}
#endif

#ifdef F_CTIME
void f_ctime(LPCFrame& f) {
  int num_arg = f.argc();
  char buf[255] = {};
  const char *cp = buf, *nl;
  char* p;
  int l;
  time_t timestamp;

  if (num_arg) {
    timestamp = f.number(0);
  } else {
    timestamp = get_current_time();
  }

  cp = ctime_r(&timestamp, buf);
  if (!cp) {
    cp = "ctime failed!";
  }

  if ((nl = strchr(cp, '\n'))) {
    l = nl - cp;
  } else {
    l = strlen(cp);
  }

  p = new_string(l, "f_ctime");
  strncpy(p, cp, l);
  p[l] = '\0';
  f.ret_str_malloced(p);
}
#endif

#ifdef F_LOCALTIME
/* FIXME: most of the #ifdefs here should be based on configure checks
   instead.  Same for rusage() */
void f_localtime(LPCFrame& f) {
  struct tm res = {};
  time_t lt;
  lt = f.number(0);
  auto* tm = localtime_r(&lt, &res);

  if (!tm) {
    f.ret_undefined();
    return;
  }

  auto* vec = allocate_empty_array(11);

  vec->item[LT_SEC].type = T_NUMBER;
  vec->item[LT_SEC].u.number = tm->tm_sec;
  vec->item[LT_MIN].type = T_NUMBER;
  vec->item[LT_MIN].u.number = tm->tm_min;
  vec->item[LT_HOUR].type = T_NUMBER;
  vec->item[LT_HOUR].u.number = tm->tm_hour;
  vec->item[LT_MDAY].type = T_NUMBER;
  vec->item[LT_MDAY].u.number = tm->tm_mday;
  vec->item[LT_MON].type = T_NUMBER;
  vec->item[LT_MON].u.number = tm->tm_mon;
  vec->item[LT_YEAR].type = T_NUMBER;
  vec->item[LT_YEAR].u.number = tm->tm_year + 1900;
  vec->item[LT_WDAY].type = T_NUMBER;
  vec->item[LT_WDAY].u.number = tm->tm_wday;
  vec->item[LT_YDAY].type = T_NUMBER;
  vec->item[LT_YDAY].u.number = tm->tm_yday;
  vec->item[LT_GMTOFF].type = T_NUMBER;
  vec->item[LT_ZONE].type = T_STRING;
  vec->item[LT_ZONE].subtype = STRING_MALLOC;
  vec->item[LT_ISDST].type = T_NUMBER;
  vec->item[LT_ISDST].u.number = tm->tm_isdst;
#ifdef __FreeBSD__
  vec->item[LT_GMTOFF].u.number = tm->tm_gmtoff;
#else
  vec->item[LT_GMTOFF].u.number = timezone;
#endif
  if (!tm->tm_isdst) {
    vec->item[LT_ZONE].u.string = string_copy(tzname[0], "f_localtime");
  } else {
    vec->item[LT_ZONE].u.string = string_copy(tzname[1], "f_localtime");
  }
  f.ret_refed(vec);
}
#endif

#ifdef F_STRFTIME
void f_strftime(LPCFrame& f) {
  auto arg_time = f.number(1);
  const auto* arg_fmt = f.string(0);

  time_t lt = arg_time;
  if (lt <= 0) lt = get_current_time();
  struct tm res = {};
  const struct tm* res_tm = localtime_r(&lt, &res);
  if (!res_tm) {
    error("Invalid time passed to strftime");
  }

  const auto max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);
  // Heap, not a stack VLA: __MAX_STRING_LENGTH__ defaults to 1MB and is
  // admin-configurable up to INT_MAX, so `char buf[max_string_length]` risked
  // a stack overflow on every call.
  std::vector<char> buf(max_string_length);
  size_t const size = strftime(buf.data(), buf.size(), arg_fmt, res_tm);
  buf[size] = '\0';

  f.ret_str_copy(buf.data());
}
#endif

#ifdef F_STRPTIME
void f_strptime(LPCFrame& f) {
  const auto* arg_timestr = f.string(1);
  const auto* arg_fmt = f.string(0);

  struct tm res = {};
  res.tm_isdst = -1;  // make sure mktime check local dst time
#ifndef _WIN32
  auto* p = strptime(arg_timestr, arg_fmt, &res);
  if (p == nullptr) {
    error("strptime() parse failed.");
  }
#else
  std::istringstream ss(arg_timestr);
  ss >> std::get_time(&res, arg_fmt);
  if (ss.fail()) {
    error("strptime() parse failed.");
  }
#endif
  f.ret(static_cast<LPC_INT>(mktime(&res)));
}
#endif
