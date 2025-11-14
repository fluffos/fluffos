inherit "/inherit/tests";

// Test functions
int add(int a, int b) {
  return a + b;
}

int multiply(int a, int b) {
  return a * b;
}

string concat(string a, string b) {
  return a + b;
}

int identity(int x) {
  return x;
}

// Test with arrays
int *double_array(int *arr) {
  return map(arr, (: $1 * 2 :));
}

// Global function variable
function global_func;

void test_basic_assignment() {
  function f;
  function str_f;

  // Test assigning local function to variable
  f = add;
  ASSERT_EQ(8, f(5, 3));

  f = multiply;
  ASSERT_EQ(15, f(5, 3));

  // Test with string return type
  str_f = concat;
  ASSERT_EQ("hello world", str_f("hello ", "world"));
}

void test_function_in_array() {
  function *funcs = ({ add, multiply });

  ASSERT_EQ(7, funcs[0](3, 4));
  ASSERT_EQ(12, funcs[1](3, 4));
}

void test_function_in_mapping() {
  mapping ops = ([
    "add": add,
    "multiply": multiply,
  ]);

  ASSERT_EQ(9, ops["add"](4, 5));
  ASSERT_EQ(20, ops["multiply"](4, 5));
}

void test_direct_invocation() {
  function f1 = add;
  function f2 = multiply;

  // Direct invocation should work
  ASSERT_EQ(8, f1(5, 3));
  ASSERT_EQ(15, f2(5, 3));

  // Should still work with evaluate()
  ASSERT_EQ(8, evaluate(f1, 5, 3));
  ASSERT_EQ(15, evaluate(f2, 5, 3));
}

void test_array_functions() {
  int *nums = ({ 1, 2, 3, 4, 5 });
  function f = double_array;

  int *result = f(nums);
  ASSERT_EQ(({ 2, 4, 6, 8, 10 }), result);
}

void test_global_variable() {
  // Assign to global function variable
  global_func = add;
  ASSERT_EQ(10, global_func(6, 4));

  global_func = multiply;
  ASSERT_EQ(24, global_func(6, 4));
}

void test_passing_to_higher_order() {
  int *nums = ({ 1, 2, 3, 4, 5 });
  int *doubled;

  // Pass bare function name to map
  doubled = map(nums, (: identity($1 * 2) :));
  ASSERT_EQ(({ 2, 4, 6, 8, 10 }), doubled);
}

void test_efun_pointer() {
  // Test with efuns
  function f = abs;
  ASSERT_EQ(5, f(-5));
  ASSERT_EQ(10, f(10));
}

void test_mixed_usage() {
  // Test mixing old and new syntax
  function f1 = add;                    // new syntax
  function f2 = (: multiply :);         // old syntax

  ASSERT_EQ(8, f1(5, 3));              // new invocation
  ASSERT_EQ(15, evaluate(f2, 5, 3));   // old invocation

  // Both should work with both invocation styles
  ASSERT_EQ(8, evaluate(f1, 5, 3));
  ASSERT_EQ(15, f2(5, 3));
}

int helper_function(int x) {
  return x + 2;
}

void test_forward_declaration() {
  // This tests that forward-declared functions work
  function f = helper_function;
  ASSERT_EQ(42, f(40));
}

function get_adder() {
  return add;
}

void test_return_function() {
  function f = get_adder();
  ASSERT_EQ(11, f(5, 6));
}

void test_equivalence() {
  // Verify that bare name creates same function pointer as (: name :)
  function f1 = add;
  function f2 = (: add :);

  ASSERT_EQ(evaluate(f1, 3, 4), evaluate(f2, 3, 4));
}

void test_array() {
  function *funcs = ({ add, multiply });

  ASSERT_EQ(10, funcs[0](5, 5));
  ASSERT_EQ(10, funcs[1](5, 2));
}

void test_mapping() {
  mapping funcs = ([
    "add": add,
    "multiply": multiply,
  ]);

  ASSERT_EQ(10, funcs["add"](5, 5));
  ASSERT_EQ(10, funcs["multiply"](5, 2));
}

#define FP_OWNER_DESTED 0x20
#define FIRST_CLASS_FP_VALUE "I'm a first class function!\n"
#define FIRST_CLASS_STRING_VALUE "I'm a first class string!\n"

private mixed accept_mixed_function(mixed arg) {
  int fp = functionp(arg);

  if (fp && !(fp & FP_OWNER_DESTED))
    arg = arg();

  return arg;
}

void test_shadowing() {
  ASSERT_EQ(FIRST_CLASS_FP_VALUE, accept_mixed_function((: FIRST_CLASS_FP_VALUE :)));
  ASSERT_EQ(FIRST_CLASS_STRING_VALUE, accept_mixed_function(FIRST_CLASS_STRING_VALUE));
}

void do_tests() {
  test_basic_assignment();
  test_direct_invocation();
  test_array_functions();
  test_global_variable();
  test_function_in_array();
  test_function_in_mapping();
  test_passing_to_higher_order();
  test_efun_pointer();
  test_mixed_usage();
  test_forward_declaration();
  test_return_function();
  test_equivalence();
  test_shadowing();
}
