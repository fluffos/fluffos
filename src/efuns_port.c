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

/* get a value for CLK_TCK for use by times() */
#if (defined(TIMES) && !defined(RUSAGE))
/* this may need #ifdef'd to handle different types of machines */
#include <limits.h>
#endif

#ifdef F_CRYPT
#define SALT_LEN        8
#ifdef CUSTOM_CRYPT
#define CRYPT(x, y) custom_crypt(x, y, 0)
#endif

void
f_crypt (void)
{
    const char *res, *p;
    char salt[SALT_LEN + 1];
    const char *choice =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ./";

    if (sp->type == T_STRING && SVALUE_STRLEN(sp) >= 2) {
        p = sp->u.string;
    } else {
        int i;

        for (i = 0; i < SALT_LEN; i++)
            salt[i] = choice[random_number(strlen(choice))];

        salt[SALT_LEN] = 0;
        p = salt;
    }

    res = string_copy(CRYPT((sp-1)->u.string, p), "f_crypt");
    pop_stack();
    free_string_svalue(sp);
    sp->subtype = STRING_MALLOC;
    sp->u.string = res;
}
#endif

#ifdef F_OLDCRYPT
void
f_oldcrypt (void) {
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
f_localtime (void)
{
    struct tm *tm;
    array_t *vec;
    time_t lt;

#ifdef sequent
    struct timezone tz;
#endif

    lt = sp->u.number;
    tm = localtime(&lt);

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
#if defined(BSD42) || defined(apollo) || defined(_AUX_SOURCE) \
        || defined(OLD_ULTRIX)
    /* 4.2 BSD doesn't seem to provide any way to get these last three values */
    vec->item[LT_GMTOFF].u.number = 0;
    vec->item[LT_ZONE].type = T_NUMBER;
    vec->item[LT_ZONE].u.number = 0;
    vec->item[LT_ISDST].u.number = -1;
#else                           /* BSD42 */
    vec->item[LT_ISDST].u.number = tm->tm_isdst;
#if defined(sequent)
    vec->item[LT_GMTOFF].u.number = 0;
    gettimeofday(NULL, &tz);
    vec->item[LT_GMTOFF].u.number = tz.tz_minuteswest;
    vec->item[LT_ZONE].u.string =
        string_copy(timezone(tz.tz_minuteswest, tm->tm_isdst), "f_localtime");
#else                           /* sequent */
#if (defined(hpux) || defined(_SEQUENT_) || defined(_AIX) || defined(SunOS_5) \
        || defined(SVR4) || defined(sgi) || defined(linux) || defined(cray) \
        || defined(__CYGWIN__)\
    )
    if (!tm->tm_isdst) {
        vec->item[LT_GMTOFF].u.number = timezone;
        vec->item[LT_ZONE].u.string = string_copy(tzname[0], "f_localtime");
    } else {
#if (defined(_AIX) || defined(hpux) || defined(linux) || defined(cray) \
	|| defined(__CYGWIN__)\
	)
        vec->item[LT_GMTOFF].u.number = timezone;
#else
        vec->item[LT_GMTOFF].u.number = altzone;
#endif
        vec->item[LT_ZONE].u.string = string_copy(tzname[1], "f_localtime");
    }
#else
#ifndef WIN32
    vec->item[LT_GMTOFF].u.number = tm->tm_gmtoff;
    vec->item[LT_ZONE].u.string = string_copy(tm->tm_zone, "f_localtime");
#else
    vec->item[LT_GMTOFF].u.number = _timezone;
    vec->item[LT_ZONE].u.string = string_copy(_tzname[_daylight?1:0],"f_localtime");
#endif
#endif
#endif                          /* sequent */
#endif                          /* BSD42 */
    put_array(vec);
}
#endif

#ifdef F_RUSAGE
#ifdef WIN32
void f_rusage (void)
{
    error("rusage() not supported under Windows.\n");
}
#else

#ifdef RUSAGE
void
f_rusage (void)
{
    struct rusage rus;
    mapping_t *m;
    long usertime, stime;
    int maxrss;

    if (getrusage(RUSAGE_SELF, &rus) < 0) {
        m = allocate_mapping(0);
    } else {
        char buf[256];
	int fd;
        usertime = rus.ru_utime.tv_sec * 1000 + rus.ru_utime.tv_usec / 1000;
        stime = rus.ru_stime.tv_sec * 1000 + rus.ru_stime.tv_usec / 1000;
        maxrss = rus.ru_maxrss;
#ifdef sun
        maxrss *= getpagesize() / 1024;
#else
#ifdef __linux__
        fd = open("/proc/self/statm", O_RDONLY);
        buf[read(fd, buf, 256)] = 0;
        close(fd);
        sscanf(buf, "%*d %d %*s", &maxrss);
        maxrss *= getpagesize() / 1024;
#endif
#endif
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
#else

#ifdef GET_PROCESS_STATS
void
f_rusage (void)
{
    struct process_stats ps;
    mapping_t *m;
    int utime, stime, maxrss;

    if (get_process_stats(NULL, PS_SELF, &ps, NULL) == -1)
        m = allocate_mapping(0);
    else {
        utime = ps.ps_utime.tv_sec * 1000 + ps.ps_utime.tv_usec / 1000;
        stime = ps.ps_stime.tv_sec * 1000 + ps.ps_stime.tv_usec / 1000;
        maxrss = ps.ps_maxrss * getpagesize() / 1024;

        m = allocate_mapping(19);
        add_mapping_pair(m, "utime", utime);
        add_mapping_pair(m, "stime", stime);
        add_mapping_pair(m, "maxrss", maxrss);
        add_mapping_pair(m, "pagein", ps.ps_pagein);
        add_mapping_pair(m, "reclaim", ps.ps_reclaim);
        add_mapping_pair(m, "zerofill", ps.ps_zerofill);
        add_mapping_pair(m, "pffincr", ps.ps_pffincr);
        add_mapping_pair(m, "pffdecr", ps.ps_pffdecr);
        add_mapping_pair(m, "swap", ps.ps_swap);
        add_mapping_pair(m, "syscall", ps.ps_syscall);
        add_mapping_pair(m, "volcsw", ps.ps_volcsw);
        add_mapping_pair(m, "involcsw", ps.ps_involcsw);
        add_mapping_pair(m, "signal", ps.ps_signal);
        add_mapping_pair(m, "lread", ps.ps_lread);
        add_mapping_pair(m, "lwrite", ps.ps_lwrite);
        add_mapping_pair(m, "bread", ps.ps_bread);
        add_mapping_pair(m, "bwrite", ps.ps_bwrite);
        add_mapping_pair(m, "phread", ps.ps_phread);
        add_mapping_pair(m, "phwrite", ps.ps_phwrite);
    }
    push_refed_mapping(m);
}
#else

#ifdef TIMES                    /* has times() but not getrusage() */

/*
  warning times are reported in processor dependent units of time.
  see man pages for 'times' to figure out how long a tick is on your system.
*/

void
f_rusage (void)
{
    mapping_t *m;
    struct tms t;

    times(&t);
    m = allocate_mapping(2);
    add_mapping_pair(m, "utime", t.tms_utime * 1000 / CLK_TCK);
    add_mapping_pair(m, "stime", t.tms_stime * 1000 / CLK_TCK);
    push_refed_mapping(m);
}

#else

#endif                          /* TIMES */

#endif                          /* GET_PROCESS_STATS */

#endif                          /* RUSAGE */

#endif                          /* WIN32 */

#endif

#ifdef F_MALLOC_CHECK
/* this efun only useful on the NeXT (func_spec.c has #ifdef NeXT).  A
   non-zero return value indicates that some memory corruption has occurred
   at some time prior to this calling of this efun.
*/
void
f_malloc_check (void)
{
    push_number(NXMallocCheck());
}
#endif

#ifdef F_MALLOC_DEBUG
/* NeXT specific efun for setting the debugging level of NeXT's built-in
   malloc.
*/
void
f_malloc_debug (void)
{
    int level;

    level = sp->u.number;
    if (level < 0) {
        int rc;

        rc = malloc_debug(0);
        malloc_singlethreaded();
        sp->u.number = rc;
    } else {
        sp->u.number = malloc_debug(level);
    }
}
#endif
