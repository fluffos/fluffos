int called;

void first_call() {
    called = 1;
}

void third_call() {
    ASSERT(!called);
}

void second_call() {
    ASSERT(!called);
    call_out( (: first_call :), 0);
    remove_call_out();
    call_out( (: third_call :), 1);
}

void do_tests() {
    int h;

    called = 0;
    h = call_out( "first_call", 0);
    ASSERT(remove_call_out(h) != -1);
    ASSERT(remove_call_out(12345) == -1);

    call_out( "first_call", 0);
    call_out( "second_call", 1);
    ASSERT(remove_call_out("first_call") != -1);
    ASSERT(remove_call_out("foo") == -1);
}
