void do_tests() {
  string err = catch(load_object("/single/helper/aborted_inner"));
  ASSERT_NE(0, err);
  object o = load_object("/single/helper/clean_inner");
  ASSERT_EQ(42, o->probe());
}
