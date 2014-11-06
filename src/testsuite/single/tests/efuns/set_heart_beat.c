int x;

void heart_beat() {
    x++;
    switch (x) {
    case 1:
	ASSERT_EQ(1, query_heart_beat(this_object()));
	set_heart_beat(2);
	ASSERT_EQ(2, query_heart_beat(this_object()));
	break;
    case 2:
	ASSERT_EQ(2, query_heart_beat(this_object()));
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
