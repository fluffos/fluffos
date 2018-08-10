AC_DEFUN([F_HAVE_PREDEFINE], [AC_COMPILE_IFELSE(
  [AC_LANG_SOURCE(
    [[
      #include "base/internal/options_incl.h"
      #ifndef $1
      #error not defined
      #endif
    ]]
  )], [$2], [$3], [])]
)
