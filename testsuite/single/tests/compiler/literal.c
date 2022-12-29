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
}
