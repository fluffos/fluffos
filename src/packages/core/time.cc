#include "base/package_api.h"

#include <chrono>
#include <iomanip>
#include <string>
#include <sstream>

#ifdef F_PERF_COUNTER_NS
void f_perf_counter_ns() {
  auto now = std::chrono::high_resolution_clock::now();
  push_number(std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count());
}
#endif

#ifdef F_TIME
void f_time(void) { push_number(get_current_time()); }
#endif

#ifdef F_CTIME
void f_ctime(void) {
  char buf[255] = {};
  const char *cp = buf, *nl;
  char *p;
  int l;
  time_t timestamp;

  if (st_num_arg) {
    timestamp = sp->u.number;
  } else {
    push_number(0);
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
  put_malloced_string(p);
}
#endif

#ifdef F_LOCALTIME
/* FIXME: most of the #ifdefs here should be based on configure checks
   instead.  Same for rusage() */
void f_localtime(void) {
  struct tm res = {};
  time_t lt;
  lt = sp->u.number;
  auto tm = localtime_r(&lt, &res);

  pop_stack();

  if (!tm) {
    push_svalue(&const0u);
    return;
  }

  auto vec = allocate_empty_array(11);

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
  push_refed_array(vec);
}
#endif

#ifdef F_STRFTIME
void f_strftime() {
  auto arg_time = sp->u.number;
  auto arg_fmt = (sp-1)->u.string;

  time_t lt = arg_time;
  if (lt <= 0) lt = get_current_time();
  struct tm res = {};
  auto tm = localtime_r(&lt, &res);
  if (!tm) {
    error("Invalid time passed to strftime");
  }

  std::ostringstream ss;
  ss << std::put_time(tm, arg_fmt);
  if (ss.fail()) {
    error("strftime() failed.");
  }

  pop_2_elems();
  auto result = ss.str();
  copy_and_push_string(result.c_str());
}
#endif

#ifdef F_STRPTIME
void f_strptime() {
  auto arg_timestr = (sp)->u.string;
  auto arg_fmt = (sp-1)->u.string;

  struct tm res = {};
  std::istringstream ss(arg_timestr);
  ss >> std::get_time(&res, arg_fmt);

  if (ss.fail()) {
    error("strptime() parse failed.");
  }
  pop_2_elems();
  push_number(mktime(&res));
}
#endif
