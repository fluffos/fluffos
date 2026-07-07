#define OBJ1 "/single/tests/compiler/function"
#define OBJ2 "/single/tests/compiler/function_inherit"
#define OBJ3 "/single/tests/compiler/function_inherit_2"

void callother_test(object ob) {
  ob->test4(1);
  ob->test4(2, "aaa");
  ob->test4(3, "bbb", 1);
  // making sure the default value is calculated in the caller's context
  ASSERT_EQ(this_object(), ob->test5());

  ob->test4(1);
  ob->test4(2, "aaa");
  ob->test4(3, "bbb", 1);
  // making sure the default value is calculated in the caller's context
  ASSERT_EQ(this_object(), ob->test5());

  ASSERT_EQ("str123", ob->test6("str", "123"));
  ASSERT_EQ("0bbbccc", ob->test7());
  ASSERT_EQ("xxxbbbccc", ob->test7("xxx"));
  ASSERT_EQ("xxxyyyccc", ob->test7("xxx", "yyy"));

  ASSERT_EQ("str123", ob->test6("str", "123"));
  ASSERT_EQ("0bbbccc", ob->test7());
  ASSERT_EQ("xxxbbbccc", ob->test7("xxx"));
  ASSERT_EQ("xxxyyyccc", ob->test7("xxx", "yyy"));
}

void do_tests() {
  callother_test(load_object(OBJ1));
  callother_test(load_object(OBJ2));
  callother_test(load_object(OBJ3));

  // test mid layer inherit
  ASSERT_EQ(0, OBJ3->test_1_1());
  ASSERT_EQ("0bbb", OBJ3->test_1_2());
  ASSERT_EQ("aaabbb", OBJ3->test_1_2("aaa"));
  ASSERT_EQ("xxxyyy", OBJ3->test_1_2("xxx", "yyy"));
}