void do_tests() {
  // list files in the directory
  ASSERT_EQ(({ "README" }), get_dir("/u/"));

  // List files matching pattern
  ASSERT_EQ(({ "u" }), get_dir("/u"));
  ASSERT_EQ(({ "README" }), get_dir("/READ??"));
  ASSERT_EQ(({ "README" }), get_dir("/*READ*"));

  // ending slash or . doesn't matter
  ASSERT_EQ(get_dir("/u/"), get_dir("/u/."));
}
