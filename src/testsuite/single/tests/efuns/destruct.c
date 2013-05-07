void do_tests() {
    object ob = new(__FILE__);
    ASSERT(ob);
    destruct(ob);
    ASSERT(!ob);
}
