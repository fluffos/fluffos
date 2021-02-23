void do_tests() {
  string tmp = "this is a test";
  mixed *ret;

  ret = explode(tmp, "");
  ASSERT_EQ(({ "t", "h", "i", "s", " ", "i", "s", " ", "a", " ", "t", "e", "s", "t"}), ret);
  ret = explode(tmp, " ");
  ASSERT_EQ(({ "this", "is", "a", "test" }), ret);

  ret = explode(" " + tmp, " ");
#ifndef __REVERSIBLE_EXPLODE_STRING__
  ASSERT_EQ(({ "this", "is", "a", "test" }), ret);
#else
  ASSERT_EQ(({ "", "this", "is", "a", "test" }), ret);
#endif

  ret = explode("     " + tmp, " ");
#ifndef __REVERSIBLE_EXPLODE_STRING__
#ifdef __SANE_EXPLODE_STRING__
  ASSERT_EQ(({ "", "", "", "", "this", "is", "a", "test" }), ret);
#else
  ASSERT_EQ(({ "this", "is", "a", "test" }), ret);
#endif
#else
  ASSERT_EQ(({ "", "", "", "", "", "this", "is", "a", "test" }), ret);
#endif

  tmp = "this  is  a  test  ";
  ret = explode(tmp, "  ");
#ifndef __REVERSIBLE_EXPLODE_STRING__
  ASSERT_EQ(({ "this", "is", "a", "test" }), ret);
#else
  ASSERT_EQ(({ "this", "is", "a", "test", "" }), ret);
#endif

  ret = explode("  " + tmp, "  ");
#ifndef __REVERSIBLE_EXPLODE_STRING__
  ASSERT_EQ(({ "this", "is", "a", "test" }), ret);
#else
  ASSERT_EQ(({ "", "this", "is", "a", "test", "" }), ret);
#endif

  ret = explode("      " + tmp, "  ");
#ifndef __REVERSIBLE_EXPLODE_STRING__
#ifdef __SANE_EXPLODE_STRING__
  ASSERT_EQ(({ "", "", "this", "is", "a", "test" }), ret);
#else
  ASSERT_EQ(({ "this", "is", "a", "test" }), ret);
#endif
#else
  ASSERT_EQ(({ "", "", "", "this", "is", "a", "test", "" }), ret);
#endif

}
