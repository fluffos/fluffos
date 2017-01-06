void do_tests() {
    object ob;

    ob = load_object("/clone/mgip1");
    ASSERT_EQ(1, ob->doit());

    ob = load_object("/clone/mgip2");
    ASSERT_EQ(2, ob->doit());

    ob = load_object("/clone/mgip3");
    ASSERT_EQ(3, ob->doit());

    ob = load_object("/clone/mgip4");
    ASSERT_EQ(3, ob->doit());
}
