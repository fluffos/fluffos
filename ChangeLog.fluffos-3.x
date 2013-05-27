Wodan announced 3.x release will be maintained by
"Yucong Sun" (sunyucong@gmail.com) for now.

Please submit issue to https://github.com/fluffos/fluffos

Major changes compare to 2.x:
  switch to autoconf (alpha4 done)
  PACKAGE_TRIM (alpha4 done)
  C++11 (G++ 4.6+, CLANG 2.9+) environment is required (alpha3 done)
  Support 32bit platform, CYGWIN build. (alpha2 done)
  64bit LPC runtime on all platforms (alpha2 done)
  EFUN limit has been raised to 65535 (alpha2)
  CALLOUT_LOOP_PROTECTION, SANE_SORTING (alpha2)

TODOs:
  add boost asio for async networking.
  get rid of addr_server
  remove SQLITE2 support from package/db.c
  switch to automake
  switch to TCMalloc
  get rid of edit_source
  LPC JIT compiler

================================================================================
  Per-release ChangeLog
================================================================================
FluffOS 3.0-alpha4
  PACKAGE_TRIM: (zoilder), rtrim, ltrim, and trim for string trimming.

  FluffOS has switched to use autoconf for compatibility detection.
  This has lead to removal of almost half of the code in edit_source.c
  Note: the correct way to build is still to launch ./build.FluffOS first,
    then make.
  Other general code quality imporvements, many old crafts has been removed.
  FluffOS 3.0 will only support mondern linux distributions.

FluffOS 3.0-alpha3
  FluffOS has switched to c++ language.
  use try/catch instead of longjmp. (wodan)
  Code quality improvment.
  Fix using DEBUG without DEBUGMALLOC_EXTENSIONS cause memory corruption.

FluffOS 3.0-alpha2

General:
  Rebased to 2.27 released by wodan.
  Build will fail early when local_options is missing.
  Enforce source format using astyle.

BugFix:
  command() efun will correctly return eval cost.
  Crasher 14, crash when returning array types.

Tests:
  Tests are now executed randomly.
  debugmalloc will fill memory with magic values.

FluffOS 3.0-alpha1

New compile options:
  POSIX_TIMERS: better time preceision tracking for eval cost. (voltara@lpmuds.net)
  CALLOUT_LOOP_PROTECTION: protect call_out(0) loops. (voltara@lpmuds.net)
  SANE_SORTING: Use faster sorting implementation for "sort_array()", but requires
                LPC code to return conforming results.

General:
  Build script improvement and compile/warning fixes.
  Build under 32bit environment is now supported.
  Build under CYGWIN is now supported.
  Multiple crasher/memory leaks is fixed.
  Documentation has been moved to root directory.
  Enable Travis CI to automate test/build for each commit.
  Auto print backtrace dump when driver crash.
  Print warning on startup if core dump limit is 0.
  Compile fix for db.c without PACKAGE_ASYNC. (mactorg@lpmuds.net)
  General code quality improvements.

Packages:
  PACKAGE_CRYPTO: build fixes and enhancements. (voltara@lpmuds.net)
  PACKAGE_SHA1: Fix incorrect sha1() hash generation, verified with tests.
                (voltara@lpmuds.net)

Test:
  "make test" will launch testsuite and report any problems.
  DEBUGMALLOC, DEBUGMALLOC_EXTENSIONS and CHECK_MEMORY is working now.
  Extensive 64bit runtime tests is added.
  Switch operator tests.
  Benchmarker and auto-crasher improvments.

LPC:
  LPC runtime is strictly 64bit now and everything should conform.
  MIN_INT, MAX_FLOAT, MIN_FLOAT predefines.
  Maximum number of EFUNs has been raised to 65535 from 256.

Known Issues:
  "-MAX_INT" is not parsed correctly in LPC(old bug), see
    src/testsuite/single/tests/64bit.c for details.
  unique_mapping() EFUN will leak memory.
  crasher in testsuite needs improvements.
