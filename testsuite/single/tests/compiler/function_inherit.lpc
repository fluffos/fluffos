#define FUNCTION_OBJ "/single/tests/compiler/function"

inherit FUNCTION_OBJ;

nosave int var1 = 1;

string test_1_1(string a, string b) {
  return a + b;
}

varargs string test_1_2(string a, string b: (: "bbb" :)) {
  return a + b;
}


void do_tests() {
    ASSERT_EQ(0, var);
    ASSERT_EQ(1, var1);

    ASSERT_EQ(0, called);
    ::do_tests();
    ASSERT_EQ(1, called);

    test1();
    test2(1, 2);
    test3(1, 2, 3, 4, 5);
    // direct call
    test4(1);
    test4(2, "aaa");
    test4(3, "bbb", 1);
    // apply
    this_object()->test4(1);
    this_object()->test4(2, "aaa");
    this_object()->test4(3, "bbb", 1);

    // making sure the default value is calculated in the caller's context
    ASSERT_EQ(this_object(), test5());
    ASSERT_EQ(this_object(), test5(this_object()));

    ASSERT_EQ("str123", test6("str", "123"));
    ASSERT_EQ("0bbbccc", test7());
    ASSERT_EQ("xxxbbbccc", test7("xxx"));
    ASSERT_EQ("xxxyyyccc", test7("xxx", "yyy"));

    // test mid layer inherit
    ASSERT_EQ("aaabbb", test_1_1("aaa", "bbb"));
    ASSERT_EQ("0bbb", test_1_2());
    ASSERT_EQ("aaabbb", test_1_2("aaa"));
    ASSERT_EQ("xxxyyy", test_1_2("xxx", "yyy"));

    called++;
    ASSERT_EQ(2, called);
}