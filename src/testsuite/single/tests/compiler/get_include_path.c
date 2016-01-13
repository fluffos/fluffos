void do_tests() {
    object ob;

    ob = load_object("/clone/mgip1");
    ASSERT(ob->doit() == 1);

    ob = load_object("/clone/mgip2");
    ASSERT(ob->doit() == 2);

    ob = load_object("/clone/mgip3");
    ASSERT(ob->doit() == 3);

    ob = load_object("/clone/mgip4");
    ASSERT(ob->doit() == 3);
}
