void do_tests() {
  string tmp = "this is a test";
  mixed *ret;

  ret = explode(tmp, "");
  ASSERT_EQ(({ "t", "h", "i", "s", " ", "i", "s", " ", "a", " ", "t", "e", "s", "t"}), ret);
  ASSERT_EQ(ret, explode_reversible(tmp, ""));
  ASSERT_EQ(tmp, implode(explode_reversible(tmp, ""), ""));

  ret = explode(tmp, " ");
  ASSERT_EQ(({ "this", "is", "a", "test" }), ret);
  ASSERT_EQ(ret, explode_reversible(tmp, " "));
  ASSERT_EQ(tmp, implode(explode_reversible(tmp, " "), " "));

  ret = explode(" " + tmp, " ");
#ifndef __REVERSIBLE_EXPLODE_STRING__
  ASSERT_EQ(({ "this", "is", "a", "test" }), ret);
#else
  ASSERT_EQ(({ "", "this", "is", "a", "test" }), ret);
#endif
  ASSERT_EQ(({ "", "this", "is", "a", "test" }), explode_reversible(" " + tmp, " "));
  ASSERT_EQ(" " + tmp, implode(explode_reversible(" " + tmp, " "), " "));

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
  ASSERT_EQ(({ "", "", "", "", "", "this", "is", "a", "test" }), explode_reversible("     " + tmp, " "));
  ASSERT_EQ("     " + tmp, implode(explode_reversible("     " + tmp, " "), " "));

  tmp = "this  is  a  test  ";
  ret = explode(tmp, "  ");
#ifndef __REVERSIBLE_EXPLODE_STRING__
  ASSERT_EQ(({ "this", "is", "a", "test" }), ret);
#else
  ASSERT_EQ(({ "this", "is", "a", "test", "" }), ret);
#endif
  ASSERT_EQ(tmp, implode(explode_reversible(tmp, "  "), "  "));
  ASSERT_EQ(({ "this", "is", "a", "test", "" }), explode_reversible(tmp, "  "));


  ret = explode("  " + tmp, "  ");
#ifndef __REVERSIBLE_EXPLODE_STRING__
  ASSERT_EQ(({ "this", "is", "a", "test" }), ret);
#else
  ASSERT_EQ(({ "", "this", "is", "a", "test", "" }), ret);
#endif
  ASSERT_EQ(({ "", "this", "is", "a", "test", "" }), explode_reversible("  " + tmp, "  "));

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
  ASSERT_EQ(({ "", "", "", "this", "is", "a", "test", "" }), explode_reversible("      " + tmp, "  "));

  tmp = "..x.y..z..";
#ifndef __REVERSIBLE_EXPLODE_STRING__
#ifdef __SANE_EXPLODE_STRING__
  ASSERT_EQ(({ "", "x", "y", "", "z", "" }), explode(tmp, "."));
#else
  ASSERT_EQ(({ "x", "y", "", "z", "" }), explode(tmp, "."));
#endif
#else
  ASSERT_EQ(({ "", "", "x", "y", "", "z", "", "" }), explode(tmp, "."));
#endif
  ASSERT_EQ(({ "", "", "x", "y", "", "z", "", "" }), explode_reversible(tmp, "."));
}
