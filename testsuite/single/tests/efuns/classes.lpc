class foo {
    int x;
    string y;
}

void do_tests() {
  ASSERT_EQ(({ "foo" }), classes(find_object(__FILE__)));
  ASSERT_EQ(({ ({ "foo", ({ "x", "int"}), ({ "y", "string"}) }) }), classes(find_object(__FILE__), 1));
}
