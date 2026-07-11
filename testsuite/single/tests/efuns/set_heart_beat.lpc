int x = 0;

void heart_beat() {
    x++;
    switch (x) {
      case 1:
        write("heartbeat test 1.\n");
        ASSERT_EQ(1, query_heart_beat(this_object()));
        set_heart_beat(2);
        ASSERT_EQ(2, query_heart_beat(this_object()));
        break;
      case 2:
        write("heartbeat test 2.\n");
        ASSERT_EQ(2, query_heart_beat(this_object()));
        set_heart_beat(1);
        ASSERT_EQ(1, query_heart_beat(this_object()));
        set_heart_beat(0);
        ASSERT_EQ(0, query_heart_beat(this_object()));
        set_heart_beat(1);
        ASSERT_EQ(1, query_heart_beat(this_object()));
        break;
      case 3:
        write("heartbeat test 3.\n");
        ASSERT_EQ(1, query_heart_beat(this_object()));
        set_heart_beat(0);
        ASSERT_EQ(0, query_heart_beat(this_object()));
        set_heart_beat(1);
        ASSERT_EQ(1, query_heart_beat(this_object()));
        set_heart_beat(0);
        ASSERT_EQ(0, query_heart_beat(this_object()));
        break;
      default:
        ASSERT(0);
    }
}

void do_tests() {
    x = 0;
    set_heart_beat(0);
    ASSERT(!query_heart_beat(this_object()));
    set_heart_beat(1);
    ASSERT_EQ(1, query_heart_beat(this_object()));
}
