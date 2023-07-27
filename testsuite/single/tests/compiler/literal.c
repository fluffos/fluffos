void test_binary_literals();
void do_tests() {
  int x;
  float y;

  x = 1234567890123456;
  ASSERT_EQ(x, 0x462d53c8abac0);

  x = 0xCAFEBABE;
  ASSERT_EQ(x, 3405691582);

  x = 1234_5678_9012_3456;
  ASSERT_EQ(x, 1234567890123456);

  x = 0xCAFE_BABE;
  ASSERT_EQ(x, 0xCA_FE_BA_BE);
  ASSERT_EQ(x, 3405691582);

  y = 31415.926;
  ASSERT_EQ(y, 3_14_15.9_2_6);

  test_binary_literals();   
}

void test_binary_literals() {
    // Test assignment with binary literals
    int bin_literal1 = 0b1010;
    int bin_literal2 = 0B1101;
    int bin_literal3 = 0b01010101;

    // Test arithmetic operations with binary literals
    int sum_bin_literals = 0b101 + 0B110;
    int mul_bin_literals = 0b1010 * 0b1101;
    
    // Test comparison with binary literals
    int comp_bin_literals = 0b100 < 0B1100;

    // Expected values for the binary literals
    int expected1 = 10;
    int expected2 = 13;
    int expected3 = 85;
    int expected_sum = 11;
    int expected_mul = 130;
    int expected_comp = 1;

    // Assertions
    ASSERT_EQ(bin_literal1, expected1);
    ASSERT_EQ(bin_literal2, expected2);
    ASSERT_EQ(bin_literal3, expected3);
    ASSERT_EQ(sum_bin_literals, expected_sum);
    ASSERT_EQ(mul_bin_literals, expected_mul);
    ASSERT_EQ(comp_bin_literals, expected_comp);
}
