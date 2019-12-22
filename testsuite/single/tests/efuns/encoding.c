void do_tests() {
  string tmp;

  if(!this_player()) {
    write("No this_player(), will not be effective.\n");

    // no effect
    ASSERT_EQ("utf-8", set_encoding("gbk18310"));

    // return "utf-8" even for non-interactive
    ASSERT_EQ("utf-8", query_encoding());

    // reset also has no effect
    ASSERT_EQ("utf-8", set_encoding());
    ASSERT_EQ("utf-8", query_encoding());
    return ;
  }

  // default
  ASSERT_EQ("utf-8", query_encoding());

  // Set to invalid encoding will fail
  ASSERT_NE(0, catch(set_encoding("deadbeef")));

  // Set to something else
  tmp = set_encoding("gbk");
  ASSERT_EQ(tmp, query_encoding());

  // Trigger some write to help detect transcoding bugs.
  write("黄梅时节家家雨，青草池塘处处蛙。有约不来过夜半，闲敲棋子落灯花。\n");

  // Reset
  tmp = set_encoding();
  ASSERT_EQ("utf-8", tmp);
  ASSERT_EQ("utf-8", query_encoding());
}
