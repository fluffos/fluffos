#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.58])
AC_INIT([libtelnet], [0.23], [http://github.com/elanthis/libtelnet/tree/master])
#AC_CONFIG_AUX_DIR([auxfiles])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([libtelnet.c])
AM_INIT_AUTOMAKE([1.9 foreign -Wall -Werror subdir-objects])
AM_PROG_AR
#LT_INIT([win32-dll])
AC_LIBTOOL_WIN32_DLL

AC_ARG_ENABLE([util],
  [AS_HELP_STRING([--disable-util],
                  [disable the building of programs in util/ directory])],
  [],
  [enable_util=yes])
AM_CONDITIONAL([BUILD_UTIL], [test "$enable_util" = yes])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
AM_PROG_CC_C_O
AC_PATH_PROG([DOXYGEN], [doxygen], [no])
AM_CONDITIONAL([BUILD_DOCS], [test "$DOXYGEN" != no])

# Checks for standard headers.
AC_HEADER_STDC

# Checks for libraries.
PKG_CHECK_MODULES([zlib],
  [zlib],
  [AC_DEFINE([HAVE_ZLIB], [1], [Define to 1 if you have zlib.])],
  [AC_DEFINE([HAVE_ZLIB], [0], [Define to 1 if you have zlib.])])

# Checks for header files.

# Checks for typedefs, structures and compiler characteristics.
AC_C_INLINE

AC_CONFIG_FILES([
  Makefile
  doc/Makefile
  doc/man/Makefile
  doc/man/man1/Makefile
  doc/man/man3/Makefile
  test/Makefile
  util/Makefile

  libtelnet-uninstalled.pc
  libtelnet.pc
])
AC_OUTPUT
