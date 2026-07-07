#define PARENT "/single/tests/compiler/function_inherit"

inherit PARENT;

nosave int var2 = 2;

void do_tests() {
  ASSERT_EQ(0, var);
  ASSERT_EQ(1, var1);
  ASSERT_EQ(2, var2);

  called = 0;
  ASSERT_EQ(0, called);
  ::do_tests();
  ASSERT_EQ(2, called);

  called++;
  ASSERT_EQ(3, called);
}

