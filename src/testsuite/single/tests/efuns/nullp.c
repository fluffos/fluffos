int x;

void do_tests() {
    ASSERT(evaluate( (: nullp($1) :) ));
    ASSERT(nullp(x));
    ASSERT(!nullp(1));
}
