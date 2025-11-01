inherit __DIR__"0default_args_1.c";

// Original test - single default argument
int id (string id: (: "" :)) {
  return member_array(id, query_id()) > -1;
}

// Test multiple default arguments with different types
string test_multiple(int a, string b: (: "default" :), int c: (: 42 :)) {
  return sprintf("%d,%s,%d", a, b, c);
}

// Test with float defaults
float test_float(float x, float y: (: 1.5 :), float z: (: 2.5 :)) {
  return x + y + z;
}

// Test with array defaults
int* test_array(int base, int* arr: (: ({ 1, 2, 3 }) :)) {
  return ({ base }) + arr;
}

// Test with mapping defaults
mapping test_mapping(string key, mapping m: (: ([ "default": 1 ]) :)) {
  return ([ key: 1 ]) + m;
}

// Test with object defaults - caller's context
object test_object_context(object obj: (: this_object() :)) {
  return obj;
}

// Test varargs combined with defaults
varargs string test_varargs_defaults(string a, string b: (: "B" :), string c: (: "C" :)) {
  return a + b + c;
}

// Test complex expression in default
int test_complex_default(int x, int y: (: time() % 100 :)) {
  return x + y;
}

// Test nested function calls in defaults
string test_nested(string s, string prefix: (: upper_case("pre") :)) {
  return prefix + s;
}

// Test with zero/null defaults
int test_zero_defaults(int a, int b: (: 0 :)) {
  return a + b;
}

// Test string concatenation in default
string test_concat_default(string base, string suffix: (: "_" + "default" :)) {
  return base + suffix;
}

// Test efun call in default
int test_efun_default(int x, int len: (: sizeof(({ 1, 2, 3, 4, 5 })) :)) {
  return x + len;
}

void do_tests() {
  object ob;
  int result;

  ob = new(__FILE__);

  // Original test
  ASSERT_EQ(0, ob->id());

  // Test multiple defaults - all combinations
  ASSERT_EQ("1,default,42", test_multiple(1));
  ASSERT_EQ("2,custom,42", test_multiple(2, "custom"));
  ASSERT_EQ("3,custom,99", test_multiple(3, "custom", 99));

  // Test via apply (call_other)
  ASSERT_EQ("5,default,42", this_object()->test_multiple(5));
  ASSERT_EQ("6,xyz,42", this_object()->test_multiple(6, "xyz"));
  ASSERT_EQ("7,xyz,77", this_object()->test_multiple(7, "xyz", 77));

  // Test float defaults
  ASSERT_EQ(5.0, test_float(1.0));
  ASSERT_EQ(6.5, test_float(3.0, 1.0));
  ASSERT_EQ(10.0, test_float(5.0, 2.0, 3.0));

  // Test array defaults
  ASSERT_EQ(({ 10, 1, 2, 3 }), test_array(10));
  ASSERT_EQ(({ 20, 5, 6 }), test_array(20, ({ 5, 6 })));

  // Test mapping defaults
  ASSERT_EQ(([ "key": 1, "default": 1 ]), test_mapping("key"));
  ASSERT_EQ(([ "key": 1, "custom": 2 ]), test_mapping("key", ([ "custom": 2 ])));

  // Test object context - default evaluates in caller's context
  ASSERT_EQ(this_object(), test_object_context());
  ASSERT_EQ(ob, test_object_context(ob));

  // Test varargs with defaults
  ASSERT_EQ("0BC", test_varargs_defaults());
  ASSERT_EQ("ABC", test_varargs_defaults("A"));
  ASSERT_EQ("AXC", test_varargs_defaults("A", "X"));
  ASSERT_EQ("AXY", test_varargs_defaults("A", "X", "Y"));

  // Test complex default - result depends on time() but we can verify it works
  result = test_complex_default(100);
  ASSERT(result >= 100 && result < 200);

  // Test with explicit second parameter
  ASSERT_EQ(150, test_complex_default(100, 50));

  // Test nested function call in default
  ASSERT_EQ("PREfix", test_nested("fix"));
  ASSERT_EQ("CUSTOMsuffix", test_nested("suffix", "CUSTOM"));

  // Test zero defaults
  ASSERT_EQ(10, test_zero_defaults(10));
  ASSERT_EQ(15, test_zero_defaults(10, 5));

  // Test string concatenation in default
  ASSERT_EQ("test_default", test_concat_default("test"));
  ASSERT_EQ("test_custom", test_concat_default("test", "_custom"));

  // Test efun call in default
  ASSERT_EQ(105, test_efun_default(100));  // 100 + sizeof({1,2,3,4,5}) = 105
  ASSERT_EQ(110, test_efun_default(100, 10));
}
