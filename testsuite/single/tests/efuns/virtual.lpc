void do_tests() {
    object ob;
    object ob1;

    ASSERT_EQ(find_object("aaa"), 0);
    ob = load_object("/test/virtual");
    ASSERT(ob != 0);
    ASSERT_EQ(1, virtualp(ob));
    ASSERT_EQ(find_object("/test/virtual"), ob);
    ASSERT_EQ(1, ob->get_called());

    ob1 = new("/test/virtual", "a", "b","c" );
    ASSERT(ob1 != 0);
    ASSERT_EQ(1, virtualp(ob1));
    ASSERT_EQ(1, clonep(ob1));
    ASSERT_EQ(1, ob1->get_called());
    write(sprintf("ob: %O, ob1: %O.\n", ob, ob1));
}


