void do_tests() {
#ifndef __NO_BUFFER_TYPE__
    int i = random(100000);
    
    ASSERT(sizeof(allocate_buffer(i)) == i);
    ASSERT(catch(allocate_buffer(-20)));
#endif
}

