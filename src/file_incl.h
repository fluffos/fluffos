#ifndef FILE_INCL_H
#define FILE_INCL_H

#include <sys/stat.h>
#ifndef SunOS_5
#  include <sys/dir.h>
#endif

#if defined(OSF) || defined(M_UNIX) || defined(_SEQUENT_) || defined(cray) \
 || defined(SunOS_5)
#  include <dirent.h>
#endif

#ifdef SVR4
#  include <dirent.h>
#  include <sys/filio.h>
#  include <sys/sockio.h>
#  include <sys/mkdev.h>
#endif

#if !defined(LATTICE) && !defined(_M_UNIX)
#  include <sys/resource.h>
#endif
#ifdef SunOS_5
#  include <sys/rusage.h>
#  include <crypt.h>
#endif

#if defined(OS2) || defined(MSDOS)
#  define lstat stat
#endif

#ifndef LATTICE
extern char *sys_errlist[];
#endif

#if defined(LATTICE) || defined(__SASC)
#undef close
#undef ioctl
#undef write
#undef read
#endif

#endif
