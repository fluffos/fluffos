dnl Profide a configure option to enable gcov compiler flags.
AC_DEFUN([FLUFFOS_CHECK_GCOV],
[
  AC_ARG_ENABLE(gcov,
    AS_HELP_STRING([--enable-gcov],[compile with coverage profiling instrumentation (gcc only)]),
    enable_gcov=$enableval,
    enable_gcov=no)

  if test x$enable_gcov = xyes; then
    GCOV_CFLAGS="-fprofile-arcs -ftest-coverage"
    GCOV_LIBS=-lgcov
    AC_SUBST(GCOV_CFLAGS)
    AC_SUBST(GCOV_LIBS)
    GCOV=`echo $CC | sed s/gcc/gcov/g`
    AC_SUBST(GCOV)

    # Force the user to turn off optimization
    AC_MSG_NOTICE([gcov enabled, adding "-g -O0" to CFLAGS])
    AX_APPEND_COMPILE_FLAGS("-g -O0")
  fi
])
