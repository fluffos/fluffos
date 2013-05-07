void do_tests() {
#ifndef __NO_SNOOP__
    ASSERT(query_snoop(this_object()) == 0);
#endif
}
