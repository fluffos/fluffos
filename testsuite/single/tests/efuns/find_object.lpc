void do_tests() {
    object ob;
    
    ASSERT(find_object(__FILE__) == this_object());
    ob = find_object("/single/tests/efuns/unloaded");
    if (ob)
	destruct(ob);
    ASSERT(!find_object("/single/tests/efuns/unloaded"));
    ASSERT(ob = find_object("/single/tests/efuns/unloaded", 1));
    ASSERT(file_name(ob) == "/single/tests/efuns/unloaded");
    destruct(ob);
    ASSERT(ob = load_object("/single/tests/efuns/unloaded", 1));
    ASSERT(file_name(ob) == "/single/tests/efuns/unloaded");
    ASSERT(!load_object("/foo"));
}
