void do_tests() {
  mapping info = rusage();

  // These two fields are guaranteed to exists.
  ASSERT_NE(info["utime"], 0);
  ASSERT_NE(info["stime"], 0);
}
