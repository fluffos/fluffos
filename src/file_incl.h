#ifndef FILE_INCL_H
#define FILE_INCL_H

#ifdef INCL_SYS_STAT_H
#  include <sys/stat.h>
#endif
#ifdef INCL_SYS_DIR_H
#  include <sys/dir.h>
#endif

#ifndef linux
#ifdef INCL_SYS_DIRENT_H
#  include <sys/dirent.h>
#endif
#endif

#ifdef INCL_DIRENT_H
#  include <dirent.h>
#endif
#ifdef INCL_SYS_FILIO_H
#  include <sys/filio.h>
#endif
#ifdef INCL_SYS_SOCKIO_H
#  include <sys/sockio.h>
#endif
#ifdef INCL_SYS_MKDEV_H
#  include <sys/mkdev.h>
#endif

#ifdef INCL_SYS_RESOURCE_H
#  include <sys/resource.h>
#endif
#ifdef INCL_SYS_RUSAGE_H
#  include <sys/rusage.h>
#endif
#ifdef INCL_CRYPT_H
#  include <crypt.h>
#endif

#ifdef WIN32
#  include <direct.h>
#  include <io.h>

#  define OS_mkdir(x, y) mkdir(x)
#  define lstat(x, y) stat(x, y)
#else
#  define OS_mkdir(x, y) mkdir(x, y)
#endif

#endif
