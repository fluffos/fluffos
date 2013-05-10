void do_tests() {
#ifdef __PACKAGE_TRIM__
  string target = "";

  ASSERT_EQ(target, trim(target));
  ASSERT_EQ(target, trim(target));
  ASSERT_EQ(target, trim(target));

  target = " \t\nabc\t\n ";

  ASSERT_EQ("abc", trim(target));
  ASSERT_EQ("abc\t\n ", ltrim(target));
  ASSERT_EQ(" \t\nabc", rtrim(target));

  target = "12345 abc 12345";

  ASSERT_EQ(" abc ", trim(target, "12345"));
  ASSERT_EQ(" abc 12345", ltrim(target, "12345"));
  ASSERT_EQ("12345 abc ", rtrim(target, "12345"));

  ASSERT_EQ("12345 abc 12345", trim(target, "xyz"));
  ASSERT_EQ("12345 abc 12345", ltrim(target, "xyz"));
  ASSERT_EQ("12345 abc 12345", rtrim(target, "xyz"));

  ASSERT_EQ(target, trim(target, "abc"));
  ASSERT_EQ(target, ltrim(target, "abc"));
  ASSERT_EQ(target, rtrim(target, "abc"));
#else
  write("PACKAGE_TRIM not defined, test skipped.");
#endif
}
