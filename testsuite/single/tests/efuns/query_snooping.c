void do_tests() {
#ifndef __NO_SNOOP__
    ASSERT(query_snooping(this_object()) == 0);
#endif
}
