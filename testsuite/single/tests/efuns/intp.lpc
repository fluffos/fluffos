void do_tests() {
    ASSERT(intp(5));
    ASSERT(intp(([])[0]));
    ASSERT(!intp( ({}) ));
    ASSERT(!intp(([])));
    ASSERT(!intp(this_object()));
    destruct(this_object());
    ASSERT(intp(this_object()));
}
