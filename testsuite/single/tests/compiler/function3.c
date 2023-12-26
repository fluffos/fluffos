#define FUNCTION_OBJ "/single/tests/compiler/function"

inherit FUNCTION_OBJ;

void do_tests() {
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
}