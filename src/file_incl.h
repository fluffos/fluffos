#ifndef FILE_INCL_H
#define FILE_INCL_H

#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif
#ifdef HAVE_SYS_DIR_H
#  include <sys/dir.h>
#endif

#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

#ifdef HAVE_SYS_FILIO_H
#  include <sys/filio.h>
#endif
#ifdef HAVE_SYS_SOCKIO_H
#  include <sys/sockio.h>
#endif
#ifdef HAVE_SYS_MKDEV_H
#  include <sys/mkdev.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
#  include <sys/resource.h>
#endif
#ifdef HAVE_SYS_RUSAGE_H
#  include <sys/rusage.h>
#endif
#ifdef HAVE_CRYPT_H
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
