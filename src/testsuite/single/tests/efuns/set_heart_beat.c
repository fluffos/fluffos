int x;

void heart_beat() {
    x++;
    switch (x) {
    case 1:
	ASSERT(query_heart_beat(this_object()) == 1);
	set_heart_beat(2);
	ASSERT(query_heart_beat(this_object()) == 2);
	break;
    case 1:
	ASSERT(query_heart_beat(this_object()) == 2);
	set_heart_beat(-1);
	ASSERT(query_heart_beat(this_object()) == 2);
	break;
    case 2:
	ASSERT(query_heart_beat(this_object()) == 2);
	set_heart_beat(0);
	ASSERT(query_heart_beat(this_object()) == 0);
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
    ASSERT(query_heart_beat(this_object()) == 1);
}
