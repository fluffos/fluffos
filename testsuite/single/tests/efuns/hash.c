void do_tests() {
#ifndef __PACKAGE_CRYPTO__
  write("PACKAGE_CRYPTO not defined: test not run.\n");
#else
  ASSERT_EQ(hash("md5", "11111"), "b0baee9d279d34fa1dfd71aadb908c3f");
  // TODO: adding more test cases.
#endif
}
