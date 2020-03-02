void do_tests() {
  mapping info = rusage();

  // stime and utime are guaranteed to exists, however stime maybe zero.
  ASSERT_NE(info["utime"], 0);
}
