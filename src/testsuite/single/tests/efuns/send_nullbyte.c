void do_tests() {
    if (!this_player()) {
      write("this_player() doesn't exist, test not ran.\n");
      return;
    }
    ASSERT_EQ(1, send_nullbyte(this_player()));
}
