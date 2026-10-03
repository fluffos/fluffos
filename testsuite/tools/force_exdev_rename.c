/*
 * LD_PRELOAD shim used only by the `testsuite-rename-exdev-fallback` ctest
 * (see CMakeLists.txt). It replaces libc's rename(2) with a stub that
 * always fails with EXDEV, forcing do_move()'s cross-filesystem copy+unlink
 * fallback (src/packages/core/file.cc) to run even though the test
 * filesystem is actually a single device. That fallback path has no other
 * coverage: nothing in the default testsuite run ever crosses a real
 * filesystem boundary, so the fallback's correctness (or a regression in
 * it) is otherwise invisible to CI.
 *
 * Not linked into the driver or any shipped artifact -- built only as a
 * standalone MODULE library for this one ctest's LD_PRELOAD.
 */
#include <errno.h>

int rename(const char *oldpath, const char *newpath) {
  (void)oldpath;
  (void)newpath;
  errno = EXDEV;
  return -1;
}
