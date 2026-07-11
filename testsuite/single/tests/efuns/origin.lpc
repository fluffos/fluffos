void lfun() {
    ASSERT(origin() == "local");
}

void co() {
    ASSERT(origin() == "internal");
}

void ef() {
    ASSERT(origin() == "efun");
}

void do_tests() {
    ASSERT(origin() == "call_other");
    lfun();
    call_out("co", 1);
    filter( ({ 1 }), "ef");
    ASSERT(evaluate( (: origin :)) == "function pointer");
    ASSERT(evaluate( (: origin() :)) == "functional");
}
