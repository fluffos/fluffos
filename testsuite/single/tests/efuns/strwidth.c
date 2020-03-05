void do_tests() {
  // control characters doesn't have width
  ASSERT_EQ(9, strwidth("what e\nver"));

  ASSERT_EQ(10 + 2 + 10, strwidth("欲穷千里目👩‍👩‍👧‍👧更上一层楼"));
}
