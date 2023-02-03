void do_tests() {
    ASSERT(get_config(0) == MUD_NAME);
    ASSERT(catch(get_config(-1)));

    ASSERT(get_config(256) == 4000);
}
