void lfun() {
    ASSERT(origin() == "local");
}

void ef() {
    ASSERT(origin() == "efun");
}

void do_tests() {
    ASSERT(origin() == "call_other");
    lfun();
    filter( ({ 1 }), "ef");
    ASSERT(evaluate( (: origin :)) == "function pointer");
    ASSERT(evaluate( (: origin() :)) == "functional");
}
