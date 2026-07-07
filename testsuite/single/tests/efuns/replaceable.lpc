inherit "/inherit/tests";

void do_tests()
{
#ifdef __PACKAGE_CONTRIB___
    object  ob;

    ASSERT(!replaceable(this_object()));
    ASSERT(replaceable(this_object(), ({ "do_tests" })));

    ob = load_object("/clone/replaceable_1");
    ASSERT(replaceable(ob));

    ob = load_object("/clone/replaceable_2");
    ASSERT(!replaceable(ob));
    ASSERT(replaceable(ob), ({ "my_dummy" }));
#endif
}
