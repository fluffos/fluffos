// Test ref keyword and & syntactic sugar for pass-by-reference

void increment(int ref value) {
  value++;
}

void append_item(mixed ref *arr, mixed item) {
  arr += ({ item });
}

void do_tests() {
  // Test ref keyword in call arguments
  {
    int x = 10;
    increment(ref x);
    ASSERT_EQ(11, x);
  }

  // Test & as syntactic sugar for ref in call arguments
  {
    int x = 10;
    increment(& x);
    ASSERT_EQ(11, x);
  }

  // Test & with array ref parameter
  {
    mixed *arr = ({ 1, 2, 3 });
    append_item(& arr, 4);
    ASSERT_EQ(({ 1, 2, 3, 4 }), arr);
  }

  // Test & in foreach
  {
    int *nums = ({ 1, 2, 3 });
    foreach(int & n in nums) {
      n *= 2;
    }
    ASSERT_EQ(({ 2, 4, 6 }), nums);
  }
}
