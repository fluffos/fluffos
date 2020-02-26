#ifndef BASE_INTERNAL_RUSAGE_H

#include "config.h"

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

/* Specification.  */
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#ifdef HAVE_SYS_RUSAGE_H
#include <sys/rusage.h>
#endif

#ifndef RUSAGE_SELF
#define _FLUFFOS_RUSAGE_REPLACEMENT

#define RUSAGE_SELF 0
#define RUSAGE_CHILDREN -1

struct rusage {
  struct timeval ru_utime; /* CPU time used in user mode */
  struct timeval ru_stime; /* CPU time used in system mode (kernel) */
  long ru_maxrss;
  long ru_ixrss;
  long ru_idrss;
  long ru_isrss;
  long ru_minflt;
  long ru_majflt;
  long ru_nswap;
  long ru_inblock;
  long ru_oublock;
  long ru_msgsnd;
  long ru_msgrcv;
  long ru_nsignals;
  long ru_nvcsw;
  long ru_nivcsw;
};

int getrusage(int who, struct rusage *usage_p);
#endif

#endif  // BASE_INTERNAL_RUSAGE_H
