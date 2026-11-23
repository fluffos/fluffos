# Copies lexer.l to FILE_OUT with `yylmax=<YYLMAX>` appended to its
# `%option array` line, for flex >= 2.6.5 (see src/CMakeLists.txt).
#
# Since 2.6.5 flex substitutes YYLMAX into the skeleton itself, from
# `%option yylmax` (default BUFSIZ: 8192 on glibc, 1024 on macOS), so the
# `#define YYLMAX` in lexer.l's prologue no longer reaches the scanner.
# flex 2.6.4 rejects the option, so it cannot live in lexer.l. Appending
# to an existing line keeps the copy's line numbers equal to lexer.l's.
if (NOT DEFINED FILE_IN OR NOT DEFINED FILE_OUT OR NOT DEFINED YYLMAX)
  message(FATAL_ERROR "Required variables: FILE_IN, FILE_OUT, YYLMAX")
endif()

file(READ "${FILE_IN}" CONTENT)
string(REGEX MATCHALL "\n%option array\n" _matches "${CONTENT}")
list(LENGTH _matches _count)
if (NOT _count EQUAL 1)
  message(FATAL_ERROR "${FILE_IN}: expected exactly one '%option array' line, found ${_count}")
endif()
string(REPLACE "\n%option array\n" "\n%option array yylmax=${YYLMAX}\n" CONTENT "${CONTENT}")
file(WRITE "${FILE_OUT}" "${CONTENT}")
