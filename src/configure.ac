#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([FluffOS],[v3.0-alpha4])

AC_CANONICAL_TARGET

AC_LANG(C++)

# Checks for programs.
AC_PROG_CXX
AC_PROG_YACC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
AC_SEARCH_LIBS([sin], [m])
AC_SEARCH_LIBS([crypt], [crypt])
AC_SEARCH_LIBS([malloc], [malloc])
AC_SEARCH_LIBS([gethostbyname], [nsl nsl_s])


# Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_RESOLV
AC_HEADER_DIRENT
AC_HEADER_MAJOR
AC_CHECK_HEADERS([arpa/inet.h arpa/telnet.h execinfo.h fcntl.h float.h inttypes.h libintl.h limits.h locale.h malloc.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/ioctl.h sys/param.h sys/resource.h sys/rusage.h sys/socket.h sys/stat.h sys/time.h sys/wait.h time.h unistd.h values.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_BIGENDIAN # needed by crypt.c
AC_TYPE_INT64_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_TYPE_UINT32_T

# FIXME: remove all these.
AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([char *])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_FUNC_WAIT3

AC_CHECK_FUNCS([atexit drand48 doprnt dup2 floor getcwd gethostbyaddr gethostbyname gethostname getpagesize inet_ntoa memchr memmove memset mkdir munmap pow putenv regcomp rmdir select setlocale socket sqrt strcasecmp strchr strcspn strerror strncasecmp strpbrk strrchr strstr strtol strtoul tzset utime])

AC_CHECK_FUNCS([getrusage])

AC_SUBST([ARCH])
AC_SUBST([DRIVER_BIN])
AC_SUBST([OPTIMIZE])

AC_CONFIG_HEADERS([configure.h])
AC_CONFIG_FILES([Makefile packages/Makefile])

AC_OUTPUT
