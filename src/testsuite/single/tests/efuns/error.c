void do_tests() {
    ASSERT(catch(error("hi")) == "*hi\n");
    ASSERT(catch(error("hi\n")) == "*hi\n");
}
