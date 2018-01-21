#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_REVISION([$Revision 0.0$])
AC_PREREQ([2.69])

## ------------------- ##
## Internal m4-macros. ##
## ------------------- ##

## Since we already declare these directories in this file, save those
## choices, and push the results into Makefile.am to avoid declaring them in
## two places making renames error prone, and where copies might get out of
## sync.
m4_define([AC_CONFIG_AUX_DIR],
     m4_defn([AC_CONFIG_AUX_DIR])[AC_SUBST([aux_dir], [$1])])
m4_define([AC_CONFIG_MACRO_DIR],
     m4_defn([AC_CONFIG_MACRO_DIR])[AC_SUBST([macro_dir], [$1])])

## ---------------- ##
## Autotools setup. ##
## ---------------- ##
AC_INIT([FluffOS],[m4_esyscmd(${SHELL} build-aux/git-version-gen --prefix fluffos- .tarball-version)])

## remember our cmdline for reference
FLUFFOS_CONFIGURE="$0 ${@}"
AC_SUBST([FLUFFOS_CONFIGURE])

## autoconf
## ========

# This is a sanity check so we can see what version is used in bug reports.
AS_BOX([Configuring AC_PACKAGE_TARNAME AC_PACKAGE_VERSION])
AS_ECHO(["using autoconf AC_AUTOCONF_VERSION"])
AS_ECHO

AC_PREFIX_DEFAULT([/opt/games])

AC_CONFIG_SRCDIR([src/main.cc])
AC_CONFIG_AUX_DIR([build-aux])

## if changed: change Makefile.am accordingly
AC_CONFIG_MACRO_DIR([build-aux/m4])

AC_CANONICAL_TARGET
AC_ARG_PROGRAM

## autoheader
## ==========

AC_CONFIG_HEADER([src/configure.h])
AH_TOP([#ifndef _CONFIG_H_
#define _CONFIG_H_])
AH_BOTTOM([#endif   /* _CONFIG_H_ */])

## automake
## ========

AM_INIT_AUTOMAKE([1.15 foreign filename-length-max=9])
AM_SILENT_RULES([yes])

dnl Make sure config.status is regenerated when the version timestamp changes
AC_SUBST([CONFIG_STATUS_DEPENDENCIES], ['$(top_srcdir)/.version'])

############################################
## add Makefiles for additional subdirs here
############################################
AC_CONFIG_FILES([Makefile])

## -------------- ##
## Options pt. 1. ##
## -------------- ##

AS_ECHO
AS_BOX([Options pt. 1])
AS_ECHO

dnl here come all AC_ARG_ENABLE and AC_ARG_WITH
AS_ECHO([["None yet..."]])

## -------------------- ##
## Checks for programs. ##
## -------------------- ##

AC_CACHE_SAVE
AS_ECHO
AS_BOX([Checks for programs])
AS_ECHO

AS_ECHO(["=> C-compiler:"])
## for parts we need a 'plain' C-compiler
AC_LANG(C)
AC_PROG_CC
AC_USE_SYSTEM_EXTENSIONS
## we might use openmp with C-source
AC_OPENMP

AX_APPEND_COMPILE_FLAGS([-D_GNU_SOURCE])
AX_APPEND_COMPILE_FLAGS([-fno-omit-frame-pointer])
AX_CHECK_COMPILE_FLAG([-fPIE], [
    AX_APPEND_COMPILE_FLAGS([-fPIE])
    AX_APPEND_LINK_FLAGS([-fPIE -pie])
])
AX_APPEND_COMPILE_FLAGS([-pthread])
AX_APPEND_LINK_FLAGS([-rdynamic])
AX_APPEND_LINK_FLAGS([-pthread])
# lto flag must be checked for LINK.
# CYGWIN oddity: lto doesn't work (problem with gcc 6.4.0?)
case "${target}" in
    *-*-cygwin*)
        ;;
    *)
        AX_CHECK_LINK_FLAG([-flto], [AX_APPEND_COMPILE_FLAGS([-flto])])
        ;;
esac

AS_ECHO(["" "=> C++-compiler:"])

## but mostly we need a C++-compiler with some minimum requirements
## (gnu g++/clang)
AC_LANG(C++)
AC_PROG_CXX
## we might use openmp with C++-source
AC_OPENMP

AX_APPEND_COMPILE_FLAGS([-D_GNU_SOURCE])
AX_APPEND_COMPILE_FLAGS([-fno-omit-frame-pointer])
AX_CHECK_COMPILE_FLAG([-fPIE], [
    AX_APPEND_COMPILE_FLAGS([-fPIE])
    AX_APPEND_LINK_FLAGS([-fPIE -pie])
])
AX_APPEND_COMPILE_FLAGS([-pthread])
AX_APPEND_LINK_FLAGS([-rdynamic])
AX_APPEND_LINK_FLAGS([-pthread])
# lto flag must be checked for LINK.
# CYGWIN oddity: lto doesn't work
case "${target}" in
    *-*-cygwin*)
       ;;
    *)
        AX_CHECK_LINK_FLAG([-flto], [AX_APPEND_COMPILE_FLAGS([-flto])])
       ;;
esac

# Check C++11 support.
AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])
F_SUPPORT_CXX11([], [AC_MSG_ERROR([
  [You compiler version is too old! FluffOS need g++ 4.6+ or clang 3.3+.]])])

AS_ECHO(["" "=> other programs."])

## other progs
AC_PROG_CPP
AC_PROG_CXXCPP
AC_PROG_MKDIR_P
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_CHECK_PROG(BISON, bison, bison, no)
if test $BISON = no; then
    AC_MSG_ERROR("bison is required")
fi
AC_PROG_GREP
AC_PROG_SED

## -------------- ##
## Options pt. 2. ##
## -------------- ##

AC_CACHE_SAVE
AS_ECHO
AS_BOX([Options pt. 2])
AS_ECHO


source_dir=`dirname $0`/src
AC_CHECK_FILE([${source_dir}/local_options], [],
              [AC_MSG_ERROR([[You must have an local_options file!]])])
AS_ECHO([["Checking for outdated defines in local_options..."]])
${SED} 's/ \(.*\)/\\b\1\\b/' < outdated_define.txt > outdated_define.wrk
${GREP} -o -f ./outdated_define.wrk local_options 2>/dev/null
ret=$?
rm -f outdated_define.wrk
if [[ $ret == 0 ]]; then
    AC_MSG_ERROR([[Your local_options contains above listed outdated defines that have been moved to runtime config. See outdated_define.txt ]])
else
    unset ret
    AS_ECHO([["OK! None found! :)" ""]])
    AC_LANG(C)
    AX_APPEND_COMPILE_FLAGS([-iquote${source_dir}])
    AC_LANG(C++)
    AX_APPEND_COMPILE_FLAGS([-iquote${source_dir}])
fi

# JEMALLOC doesn't work on either CYGWIN or OSX (darwin)
case "${target}" in
    *-*-cygwin*|*-*-darwin*)
        echo "JEMALLOC doesn't work on CYGWIN/OSX (darwin), force disabling."
        USE_JEMALLOC=0
        ;;
    *)
        ;;
esac

## --------------------- ##
## Checks for libraries. ##
## --------------------- ##

AC_CACHE_SAVE
AS_ECHO
AS_BOX([Checks for libraries])
AS_ECHO

AC_SEARCH_LIBS([asinh], [m])
AC_SEARCH_LIBS([crypt], [crypt])
AC_SEARCH_LIBS([malloc], [malloc])
AC_SEARCH_LIBS([backtrace], [execinfo])

# FluffOS needs libevent 2.0+
AX_LIB_EVENT([2.0])
if test "$ax_have_libevent" = "no"
then
    AC_MSG_ERROR([[Failed to find libevent (2.0+) header/library, install libevent-dev or change --with_libevent]])
fi

# Checks for zlib.
AX_CHECK_ZLIB

F_HAVE_PREDEFINE([PACKAGE_ASYNC], [AC_SEARCH_LIBS([pthread_create], [pthread])])
F_HAVE_PREDEFINE([PACKAGE_EXTERNAL], [AC_SEARCH_LIBS([fork], [rt])])

# iconv has redirect on CYGWIN.
F_HAVE_PREDEFINE([USE_ICONV], [AC_SEARCH_LIBS([iconv_open], [iconv])])
F_HAVE_PREDEFINE([USE_ICONV], [AC_SEARCH_LIBS([libiconv_open], [iconv])])

# check for gcov options
# !!! this should be last library check !!!
FLUFFOS_CHECK_GCOV

## ------------------------ ##
## Checks for header files. ##
## ------------------------ ##

AC_CACHE_SAVE
AS_ECHO([])
AS_BOX([Checks for header files])
AS_ECHO

AC_HEADER_MAJOR
AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_HEADER_TIME
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h libintl.h limits.h locale.h malloc.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/param.h sys/resource.h sys/rusage.h sys/socket.h sys/time.h unistd.h])
# are those needed???
# AC_CHECK_HEADERS([arpa/telnet.h crypt.h execinfo.h float.h sys/crypt.h sys/ioctl.h sys/stat.h values.h])

## -------------------------------------------------------------- ##
## Checks for typedefs, structures, and compiler characteristics. ##
## -------------------------------------------------------------- ##

AC_CACHE_SAVE
AS_ECHO([])
AS_BOX([Checks for typedefs, structures, and compiler characteristics])
AS_ECHO

AC_C_BIGENDIAN          # needed by crypt.c
AC_C_INLINE
AC_TYPE_INT64_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_rdev])

# FIXME: remove these two, this is currently needed by base/std.h
AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([char *])

## ----------------------------- ##
## Checks for library functions. ##
## ----------------------------- ##

AC_CACHE_SAVE
AS_ECHO([])
AS_BOX([Checks for library functions])
AS_ECHO

AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_STRTOD
AC_CHECK_FUNCS([doprnt dup2 floor getcwd gethostname gettimeofday memchr memmove memset mkdir pow putenv regcomp rmdir setlocale socket sqrt strcasecmp strchr strerror strncasecmp strrchr strspn strstr strtol strtoul tzset])
#AC_CHECK_FUNCS([munmap strcspn strpbrk ])

AC_LANG(C)
AX_APPEND_COMPILE_FLAGS([-DPACKAGE_CORE -DPACKAGE_OPS])
AC_LANG(C++)
AX_APPEND_COMPILE_FLAGS([-DPACKAGE_CORE -DPACKAGE_OPS])

F_HAVE_PREDEFINE([PACKAGE_CORE], [],
    [AC_MSG_ERROR([[Fail to find PACKAGE_CORE in your local_options, the driver will not work.]])])
F_HAVE_PREDEFINE([PACKAGE_OPS], [],
    [AC_MSG_ERROR([[Fail to find PACKAGE_OPS in your local_options, the driver will not work.]])])


F_HAVE_PREDEFINE([PACKAGE_DB], [
    F_HAVE_PREDEFINE([USE_MYSQL], [
        AX_LIB_MYSQL()
        F_HAVE_PREDEFINE([HAVE_MYSQL], [], [
            AC_MSG_ERROR([[Failed to find MYSQL header/library, try changing --with_mysql flag]])
        ])
    ])
    F_HAVE_PREDEFINE([USE_POSTGRES], [
        AX_LIB_POSTGRESQL()
        AC_SEARCH_LIBS([PQconnectdb], [pq])
        F_HAVE_PREDEFINE([HAVE_POSTGRESQL], [], [
            AC_MSG_ERROR([[Failed to find POSTGRESQL header/library, try changing --with_postgresql flag.]])
        ])
    ])
    F_HAVE_PREDEFINE([USE_SQLITE3], [
        AX_LIB_SQLITE3()
        F_HAVE_PREDEFINE([HAVE_SQLITE3], [], [
            AC_MSG_ERROR([[Failed to find SQLITE3 header/library, try changing --with_sqlite3 flag.]])
        ])
    ])
])

F_HAVE_PREDEFINE([PACKAGE_CRYPTO], [AX_CHECK_OPENSSL([],
    [AC_MSG_ERROR([[Failed to find OPENSSL header/library, try changing --with_openssl flag.]])])])

F_HAVE_PREDEFINE([PACKAGE_PCRE], [AX_PATH_LIB_PCRE([],
    [AC_MSG_ERROR([[Failed to find PCRE header/library, try changing --with_pcre flag.]])])])

## ------- ##
## Done :) ##
## ------- ##

AS_ECHO([])
AS_BOX(["Done :)"])
AS_ECHO

## --------- ##
## Messages. ##
## --------- ##

AC_MSG_RESULT([========================================])
AC_MSG_RESULT([configured with:])
AC_MSG_RESULT([========================================])
AX_SHOW_CONFIG_INFO

dnl sadly jemalloc configure.ac can't be reconfigured :(
dnl if [ $USE_JEMALLOC -eq 1 ]
dnl then
dnl     AC_CONFIG_SUBDIRS([thirdparty/jemalloc])
dnl fi
dnl libtelnet needs updating first...
dnl AC_CONFIG_SUBDIRS([thirdparty/libtelnet])

AC_SUBST([ARCH])
AC_SUBST([DRIVER_BIN])
AC_SUBST([OPTIMIZE])
AC_SUBST([SOURCE_REVISION])
AC_SUBST([USE_JEMALLOC])

AC_OUTPUT
