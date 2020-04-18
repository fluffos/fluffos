int called;
int xverb;
int utf8verb;

void do_tests() {
#ifndef __NO_ADD_ACTION__
    object tp;
    xverb = called = utf8verb = 0;
    SAVETP;
    enable_commands();
    add_action( (: called = 1 :), "foo");
    add_action( (: xverb = 1 :), "b", 1);
    add_action( (: utf8verb = 1 :), "测试测试");
    RESTORETP;
    command("foo");
    command("bar");
    command("测试测试");
    ASSERT_EQ(1, called);
    ASSERT_EQ(1, xverb);
    ASSERT_EQ(1, utf8verb);
#endif
}
