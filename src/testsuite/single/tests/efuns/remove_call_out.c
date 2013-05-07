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
    called = 0;
    call_out( "first_call", 0);
    call_out( "second_call", 1);
    ASSERT(remove_call_out("first_call") != -1);
    ASSERT(remove_call_out("foo") == -1);
}
