object tp;
#ifdef __THIS_PLAYER_IN_CALL_OUT__
#define TPIC ASSERT(this_player() == tp)
#else
#define TPIC
#endif

int called;

void no_args() {
    called++;
    TPIC;
}

void one_arg(int x) {
    called++;
    TPIC;
    ASSERT(x == 1);
}    

void two_arg(int x, int y) {
    called++;
    TPIC;
    ASSERT(x == 1);
    ASSERT(y == 2);
}

void finish() {
    ASSERT(called == 6);
}

void do_tests() {
    mixed calls, call;

    if (busy) {
	write("The call_out test is busy.  Try again later!\n");
	return;
    }
    busy = 1;

    tp = this_player();
    called = 0;
    call_out( (: no_args :), 1);
    call_out( "no_args", 2);
    call_out( (: one_arg, 1 :), 3);
    call_out( "one_arg", 4, 1);
    call_out( (: two_arg, 1 :), 5, 2);
    call_out( "two_arg", 6, 1, 2);

    // FIXME: move the recursive test out functional test. */
    /* All call_outs set up here should be called sucessfully */
    // call_out( "spin", 0);
    // call_out( "spin", 0);

    /* Only one of the recursively created call_outs should succeed */
    // call_out( "call_multiple", 0, "spin", 5);

    /* This should eventually time out */
    // call_out( "call_out_recursive_setup", 1, 0);

    // call_out( "finish", 7);
    
    calls = call_out_info();
    foreach(call in calls) {
	ASSERT(objectp(call[0]));
	ASSERT(stringp(call[1]));
	ASSERT(intp(call[2]));
    }		   
}
