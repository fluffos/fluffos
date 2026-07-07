void do_tests() {
    ASSERT(!clonep());
    ASSERT(!clonep(this_object()));
    ASSERT(clonep(new(__FILE__)));
    ASSERT(!clonep( ({ 1, 2 }) ));
    ASSERT(!clonep(1));
}
