#define FUNCTION_OBJ "/single/tests/compiler/function"
#define FUNCTION_OBJ_3 "/single/tests/compiler/function3"

void do_tests() {
  FUNCTION_OBJ->test4(1);
  FUNCTION_OBJ->test4(2, "aaa");
  FUNCTION_OBJ->test4(3, "bbb", 1);
  // making sure the default value is calculated in the caller's context
  ASSERT_EQ(this_object(), FUNCTION_OBJ->test5());

  FUNCTION_OBJ_3->test4(1);
  FUNCTION_OBJ_3->test4(2, "aaa");
  FUNCTION_OBJ_3->test4(3, "bbb", 1);
  // making sure the default value is calculated in the caller's context
  ASSERT_EQ(this_object(), FUNCTION_OBJ_3->test5());
}