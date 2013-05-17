/*
        efunctions.c: this file contains the efunctions called from
        inside eval_instruction() in interpret.c.  Note: if you are adding
    local efunctions that are specific to your driver, you would be better
    off adding them to a separate source file.  Doing so will make it much
    easier for you to upgrade (won't have to patch this file).  Be sure
    to #include "lpc_incl.h" in that separate source file.
*/

#include "std.h"
#include "lpc_incl.h"
#include "file_incl.h"
#include "include/localtime.h"
#include "port.h"
#include "crypt.h"
#include "efun_protos.h"
#include <stdio.h>

#ifdef F_CRYPT
#define SALT_LEN        8
#ifdef CUSTOM_CRYPT
#define CRYPT(x, y) custom_crypt(x, y, 0)
#endif

void
f_crypt(void)
{
  const char *res, *p;
  char salt[SALT_LEN + 1];
  const char *choice =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ./";

  if (sp->type == T_STRING && SVALUE_STRLEN(sp) >= 2) {
    p = sp->u.string;
  } else {
    int i;

    for (i = 0; i < SALT_LEN; i++) {
      salt[i] = choice[random_number(strlen(choice))];
    }

    salt[SALT_LEN] = 0;
    p = salt;
  }

  res = string_copy(CRYPT((sp - 1)->u.string, p), "f_crypt");
  pop_stack();
  free_string_svalue(sp);
  sp->subtype = STRING_MALLOC;
  sp->u.string = res;
}
#endif

#ifdef F_OLDCRYPT
void
f_oldcrypt(void)
{
#ifndef WIN32

  char *res, salt[3];
  const char *choice =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ./";

  if (sp->type == T_STRING && SVALUE_STRLEN(sp) >= 2) {
    salt[0] = sp->u.string[0];
    salt[1] = sp->u.string[1];
    free_string_svalue(sp--);
  } else {
    salt[0] = choice[random_number(strlen(choice))];
    salt[1] = choice[random_number(strlen(choice))];
    pop_stack();
  }
  salt[2] = 0;
  res = string_copy(OLDCRYPT(sp->u.string, salt), "f_crypt");
  free_string_svalue(sp);
  sp->subtype = STRING_MALLOC;
  sp->u.string = res;
#endif
}
#endif

#ifdef F_LOCALTIME
/* FIXME: most of the #ifdefs here should be based on configure checks
   instead.  Same for rusage() */
void
f_localtime(void)
{
  struct tm *tm;
  array_t *vec;
  time_t lt;

  lt = sp->u.number;
  tm = localtime(&lt);

  pop_stack();

  if (!tm) {
    push_svalue(&const0u);
    return;
  }

  vec = allocate_empty_array(11);

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
  vec->item[LT_GMTOFF].u.number = timezone;
  if (!tm->tm_isdst) {
    vec->item[LT_ZONE].u.string = string_copy(tzname[0], "f_localtime");
  } else {
    vec->item[LT_ZONE].u.string = string_copy(tzname[1], "f_localtime");
  }
  push_refed_array(vec);
}
#endif

#ifdef F_RUSAGE
void
f_rusage(void)
{
  struct rusage rus;
  mapping_t *m;
  long usertime, stime, maxrss;

  if (getrusage(RUSAGE_SELF, &rus) < 0) {
    m = allocate_mapping(0);
  } else {
    char buf[256];
    int fd;
    usertime = rus.ru_utime.tv_sec * 1000 + rus.ru_utime.tv_usec / 1000;
    stime = rus.ru_stime.tv_sec * 1000 + rus.ru_stime.tv_usec / 1000;
    maxrss = rus.ru_maxrss;
    m = allocate_mapping(16);
    add_mapping_pair(m, "utime", usertime);
    add_mapping_pair(m, "stime", stime);
    add_mapping_pair(m, "maxrss", maxrss);
    add_mapping_pair(m, "ixrss", rus.ru_ixrss);
    add_mapping_pair(m, "idrss", rus.ru_idrss);
    add_mapping_pair(m, "isrss", rus.ru_isrss);
    add_mapping_pair(m, "minflt", rus.ru_minflt);
    add_mapping_pair(m, "majflt", rus.ru_majflt);
    add_mapping_pair(m, "nswap", rus.ru_nswap);
    add_mapping_pair(m, "inblock", rus.ru_inblock);
    add_mapping_pair(m, "oublock", rus.ru_oublock);
    add_mapping_pair(m, "msgsnd", rus.ru_msgsnd);
    add_mapping_pair(m, "msgrcv", rus.ru_msgrcv);
    add_mapping_pair(m, "nsignals", rus.ru_nsignals);
    add_mapping_pair(m, "nvcsw", rus.ru_nvcsw);
    add_mapping_pair(m, "nivcsw", rus.ru_nivcsw);
  }
  push_refed_mapping(m);
}
#endif
