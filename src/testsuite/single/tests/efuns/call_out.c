object tp;
#ifdef __THIS_PLAYER_IN_CALL_OUT__
#define TPIC ASSERT(this_player() == tp)
#else
#define TPIC
#endif

int busy = 0;
mapping called;

void no_args() {
    called["basic_tests"]++;
    TPIC;
}

void one_arg(int x) {
    called["basic_tests"]++;
    TPIC;
    ASSERT(x == 1);
}    

void two_arg(int x, int y) {
    called["basic_tests"]++;
    TPIC;
    ASSERT(x == 1);
    ASSERT(y == 2);
}

void spin() {
    called["spin"]++;
    while (1) { }
}

void call_multiple(string func, int count) {
    called["call_multiple"]++;
    while (count--) {
	call_out(func, 0);
    }
}

int get_eval_cost_and_wait_for_clock_tick() {
    int cost = eval_cost();
    while (cost && cost == eval_cost()) {
    }
    return cost;
}

/* The (cost < last_cost) invariant in call_out_recursive only holds
 * after the first few recursive calls:
 *     call_out_recursive_setup(0)    // full time budget
 *     \_call_out_recursive_setup(1)  // full time budget
 *       \_call_out_recursive(...)    // leftover time budget...
 *         \_call_out_recursive(...)  // leftover time budget...
 */
void call_out_recusrive_setup(int depth) {
    int cost;
    if (depth < 1) {
	call_out("call_out_recursive_setup", 0, depth + 1);
    } else {
	call_out("call_out_recursive", 0, get_eval_cost_and_wait_for_clock_tick());
    }
}

void call_out_recursive(int last_cost) {
    int cost = get_eval_cost_and_wait_for_clock_tick();
    ASSERT(cost < last_cost);
    call_out("call_out_recursive", 0, cost, 0);
}

void finish() {
    busy = 0;
    ASSERT(called["basic_tests"] == 6);
    ASSERT(called["spin"] == 3);
    ASSERT(called["call_multiple"] == 1);
}

void do_tests() {
    mixed calls, call;


    if (busy) {
	write("The call_out test is busy.  Try again later!\n");
	return;
    }
    busy = 1;

    tp = this_player();
    called = ([ ]);
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
