#ifndef __SENSIBLE_MODIFIERS__
static
#else
protected
#endif
int stat() {
}

private int priv() {
}

void do_tests() {
    ASSERT(function_exists("stat", this_object(), 1));
    ASSERT(function_exists("priv", this_object(), 1));

    ASSERT(!function_exists("stat", this_object()));
    ASSERT(!function_exists("priv", this_object()));

    ASSERT(!function_exists("bar", this_object()));
    ASSERT(function_exists("do_tests", this_object()) + ".c" == __FILE__);
}
     
