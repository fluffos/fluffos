// in LPC, there are no such thing as boolean value. LPC conditionals is really just comparing value to 0.
// Previously, only 0 (int) is false in conditionals, yet 0.0 == 0 would return 1 instead, causing confusion.
//
// This has now been fixed, so that floats with an 0 value would also evaluate to false.

void do_tests() {
  int tmp = 1;

  tmp = 1;
  if(0.0) {
    tmp = 0;
  }
  ASSERT_EQ(1, tmp);

  tmp = 1;
  if(0.0000000000000) {
    tmp = 0;
  }
  ASSERT_EQ(1, tmp);

  tmp = 1;
  if (0.0 != 0) {
    tmp = 0;
  }
  ASSERT_EQ(1, tmp);

  tmp = 1;
  tmp = 0.0 ? 0 : 1;
  ASSERT_EQ(1, tmp);

  tmp = 1;
  tmp = 0.0000000 ? 0 : 1;
  ASSERT_EQ(1, tmp);

  tmp = 1;
  do {
    tmp++;
    if(tmp >= 3) break;
  } while(0.0);
  ASSERT_EQ(2, tmp);

  tmp = 1;
  do {
    tmp++;
    if(tmp >= 3) break;
  } while(0.0000000000000);
  ASSERT_EQ(2, tmp);

  // this will never be true since they are compared by type.
  ASSERT_NE(0.0, 0);
  ASSERT_NE(0.00000000000000, 0);
}
