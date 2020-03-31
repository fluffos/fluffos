nosave int called1 = 0;
nosave int called2 = 0;

void callback_getdir(mixed res) {
  debug_message("ASYNC: callback_getdir called! \n");

  ASSERT_EQ(get_dir("/u/"), res);

  called1 = 1;
}

void callback_getdir2(mixed res) {
  debug_message("ASYNC: callback_getdir2 called! \n");

  ASSERT_EQ(get_dir("/std/"), res);

  called2 = 1;
}

void check(int index) {
  switch(index) {
    case 1:
      ASSERT_EQ(1, called1);
      break;
    case 2:
      ASSERT_EQ(1, called2);
      break;
  }
}

void do_tests() {
#ifndef __PACKAGE_ASYNC__
    write("PACKAGE_ASYNC disabled, skipping...");
    return;
#else
    async_getdir("/u/", (: callback_getdir :));

    call_out((: async_getdir, "/std/", (: callback_getdir2 :) :), 1);

    call_out((: check, 1 :), 3);
    call_out((: check, 2 :), 4);
#endif
}
