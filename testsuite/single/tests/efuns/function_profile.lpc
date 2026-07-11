void do_tests() {
#ifdef __PROFILE_FUNCTIONS__
    mixed *hmm = function_profile(this_object());
    ASSERT(sizeof(hmm) == 1);
    ASSERT(hmm[0]["calls"]);
    ASSERT(!undefinedp(hmm[0]["self"]));
    ASSERT(!undefinedp(hmm[0]["children"]));
    ASSERT(hmm[0]["name"] == "do_tests");
#endif
}

