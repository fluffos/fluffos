void do_tests() {
#ifndef __NO_WIZARDS__
    disable_wizard();
    ASSERT(!wizardp(this_object()));
    enable_wizard(); // fails; not interactive
    ASSERT(!wizardp(this_object()));

    evaluate(bind( (: disable_wizard :), this_player()));
    ASSERT(!wizardp(this_player()));
    evaluate(bind( (: enable_wizard :), this_player()));
    ASSERT(wizardp(this_player()));
#endif
}
