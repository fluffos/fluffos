nosave int called = 0;

nosave int var = 0;

// default case
void test1() {
}

// default case
void test2(int a, int b) {
  ASSERT_EQ(a, 1);
  ASSERT_EQ(b, 2);
}

// varargs
void test3(int a, int* b ...) {
  ASSERT_EQ(a, 1);
  ASSERT_EQ(b[0], 2);
  ASSERT_EQ(b[1], 3);
  ASSERT_EQ(b[2], 4);
  ASSERT_EQ(b[3], 5);
}

// can have multiple trailing arguments with a FP for calculating default value
void test4(int a, string b: (: "str" :), int c: (: -1 :)) {
  int x = 111, y = 222; // two local variables.
  switch(a) {
    case 1: {
      ASSERT_EQ("str", b);
      ASSERT_EQ(-1, c);
      break;
    }
    case 2: {
      ASSERT_EQ("aaa", b);
      ASSERT_EQ(-1, c);
      break;
    }
    case 3: {
      ASSERT_EQ("bbb", b);
      ASSERT_EQ(1, c);
      break;
    }
  }
  ASSERT_EQ(111, x);
  ASSERT_EQ(222, y);
}

object test5(object a: (: this_object() :)) {
  return a;
}

varargs string test6(string a, string b) {
  return a + b;
}

varargs string test7(string a, string b: (: "bbb" :), string c: (: "ccc" :)) {
  return a + b + c;
}

void do_tests() {
    ASSERT_EQ(0, var);
    ASSERT_EQ(0, called);

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
    // see call_other type of tests in function2.c
    // see inherited type of tests in function3.c
    ASSERT_EQ(0, test6());
    ASSERT_EQ("str123", test6("str", "123"));
    ASSERT_EQ("0bbbccc", test7());
    ASSERT_EQ("aaabbbccc", test7("aaa"));
    ASSERT_EQ("aaaxxxccc", test7("aaa","xxx"));
    ASSERT_EQ("aaaxxxyyy", test7("aaa", "xxx", "yyy"));

    called++;
    ASSERT_EQ(1, called);
}