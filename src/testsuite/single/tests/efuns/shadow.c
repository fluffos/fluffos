object prev;

void create() {
    prev = previous_object();
}

nomask void i_am_bad() {
}

void do_tests() {
    object ob;
#ifndef __NO_SHADOWS__
    ASSERT(catch(shadow(this_object())));
    ASSERT(catch(shadow(master())));
    new(__DIR__ "badshad", 1);

    ASSERT(shadow(this_object(), 0) == 0);
    ASSERT((object)this_object()->foo() == this_object());

    ob = new(__DIR__ "goodshad", 1);
    ASSERT(shadow(this_object(), 0) == ob);
    ASSERT((object)this_object()->foo() == ob);
    ASSERT((object)new(__FILE__)->bar() == ob);

    ASSERT(catch(shadow(ob)));
    ASSERT(catch(shadow(new(__FILE__))));

#endif
}

object foo() {
    printf("%O\n", file_name());
    return this_object();
}

object bar() {
    return (object)prev->foo();
}
