#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([FluffOS],[v3.0-alpha4])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_TARGET

AC_LANG(C++)

# Checks for programs.
AC_PROG_CXX
AC_PROG_YACC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
AC_SEARCH_LIBS([sqrt], [m])
AC_SEARCH_LIBS([crypt], [crypt])
AC_SEARCH_LIBS([malloc], [malloc])
AC_SEARCH_LIBS([gethostbyname], [nsl])

# Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_RESOLV
AC_HEADER_DIRENT
AC_HEADER_MAJOR
AC_CHECK_HEADERS([arpa/inet.h arpa/telnet.h crypt.h execinfo.h fcntl.h float.h inttypes.h libintl.h limits.h locale.h malloc.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/crypt.h sys/ioctl.h sys/param.h sys/resource.h sys/rusage.h sys/socket.h sys/stat.h sys/time.h sys/wait.h time.h unistd.h values.h])

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

AC_CHECK_FUNCS([atexit drand48 doprnt dup2 floor getcwd gethostbyaddr gethostbyname gethostname getpagesize inet_ntoa memchr memmove memset mkdir munmap pow putenv regcomp rmdir select setlocale socket strcasecmp strchr strcspn strerror strncasecmp strpbrk strrchr strstr strtol strtoul tzset utime])

AC_CHECK_FUNCS([getrusage])

# Checks local_options exist, use a grauntee exist predefine.
F_HAVE_PREDEFINE([__cplusplus], [], [AC_MSG_ERROR([[You must have local_options file, see options.h.]])])

# Checking local_options related libraries
F_HAVE_PREDEFINE([HAVE_ZLIB], [AC_SEARCH_LIBS([gzread], [z])])
F_HAVE_PREDEFINE([GCMALLOC], [AC_SEARCH_LIBS([GC_init], [gc])])
F_HAVE_PREDEFINE([PACKAGE_ASYNC], [AC_SEARCH_LIBS([pthread_create], [pthread])])
F_HAVE_PREDEFINE([PACKAGE_PCRE], [AC_SEARCH_LIBS([pcre_match], [pcre])])
F_HAVE_PREDEFINE([POSIX_TIMERS], [AC_SEARCH_LIBS([timer_create], [rt])])

# iconv has redirect on CYGWIN.
F_HAVE_PREDEFINE([USE_ICONV], [AC_SEARCH_LIBS([iconv_open], [iconv])])
F_HAVE_PREDEFINE([USE_ICONV], [AC_SEARCH_LIBS([libiconv_open], [iconv])])

F_HAVE_PREDEFINE([USE_MSQL], [AC_SEARCH_LIBS([msqlConnect], [msql])])

F_HAVE_PREDEFINE([USE_MYSQL], [AX_LIB_MYSQL()])
F_HAVE_PREDEFINE([USE_MYSQL], [F_HAVE_PREDEFINE([HAVE_MYSQL], [],
  [AC_MSG_ERROR([[Fail to find mysql header/library, try changing --with_mysql flag]])])])

F_HAVE_PREDEFINE([USE_POSTGRES], [AX_LIB_POSTGRESQL()])
F_HAVE_PREDEFINE([USE_POSTGRES], [F_HAVE_PREDEFINE([HAVE_POSTGRESQL], [],
  [AC_MSG_ERROR([[Fail to find postgresql header/library, try changing --with_postgresql flag.]])])])

F_HAVE_PREDEFINE([USE_SQLITE3], [AX_LIB_SQLITE3()])
F_HAVE_PREDEFINE([USE_SQLITE3], [F_HAVE_PREDEFINE([HAVE_SQLITE3], [],
  [AC_MSG_ERROR([[Fail to find sqlite3 header/library, try changing --with_sqlite3 flag.]])])])

F_HAVE_PREDEFINE([PACKAGE_CRYPTO], [AX_CHECK_OPENSSL([],
  [AC_MSG_ERROR([[Fail to find OPENSSL header/library, try changing --with_openssl flag.]])])])

AC_SUBST([ARCH])
AC_SUBST([DRIVER_BIN])
AC_SUBST([OPTIMIZE])

AC_CONFIG_HEADERS([configure.h])
AC_CONFIG_FILES([Makefile packages/Makefile])

AC_OUTPUT
