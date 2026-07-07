void do_tests_real() {
  // non tls port
  ASSERT_NE(0, catch(sys_reload_tls(0)));

  // no support for websocket
  ASSERT_NE(0, catch(sys_reload_tls(2)));

  // valid tls
  ASSERT_EQ(0, sys_reload_tls(4));
}
void after_boot() {
  mixed err;
  write("boot finished, doing test.\n");

  err = catch(do_tests_real());

  if (err) {
    debug_message(err);
    shutdown(-1);
  }

  write("sys_reload_tls: succeed.\n");
}
void do_tests() {
  write("waiting for boot...\n");

  call_out("after_boot", 0);
}

