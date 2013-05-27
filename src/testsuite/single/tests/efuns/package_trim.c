void do_tests() {
#ifdef __PACKAGE_TRIM__
  string target = "";

  // empty string
  ASSERT_EQ(target, trim(target));
  ASSERT_EQ(target, trim(target));
  ASSERT_EQ(target, trim(target));

  target = " \t\nabc\t\n ";

  // default charset include \n \t too.
  ASSERT_EQ("abc", trim(target));
  ASSERT_EQ("abc\t\n ", ltrim(target));
  ASSERT_EQ(" \t\nabc", rtrim(target));

  target = " abc ";
  ASSERT_EQ("", ltrim(target, " abc"));
  ASSERT_EQ("", rtrim(target, "abc "));
  ASSERT_EQ("", trim(target, "abc "));

  target = "12345 abc 12345";

  // 2nd argument is charset, if provided, only trim chars
  // in the set.
  ASSERT_EQ(" abc ", trim(target, "12345"));
  ASSERT_EQ(" abc 12345", ltrim(target, "12345"));
  ASSERT_EQ("12345 abc ", rtrim(target, "12345"));

  // charset is a set, order doesn't matter.
  ASSERT_EQ(" abc ", trim(target, "5544332211"));
  ASSERT_EQ(" abc 12345", ltrim(target, "5544332211"));
  ASSERT_EQ("12345 abc ", rtrim(target, "5544332211"));

  // not found at either end.
  ASSERT_EQ(target, trim(target, "abc"));
  ASSERT_EQ(target, ltrim(target, "abc"));
  ASSERT_EQ(target, rtrim(target, "abc"));
#else
  write("PACKAGE_TRIM not defined, test skipped.");
#endif
}
