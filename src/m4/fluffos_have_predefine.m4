AC_DEFUN([F_HAVE_PREDEFINE], [AC_COMPILE_IFELSE(
  [AC_LANG_SOURCE(
    [[
      #include "local_options"
      #ifndef $1
      #error not defined
      #endif
    ]]
  )], [$2], [$3], [])]
)
