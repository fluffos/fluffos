int recurse(int flag) {
  if (flag) {
    return recurse(flag);
  } else {
    return flag;
  }
}

void do_tests() {
  ASSERT_EQ(0, recurse(0));

  // we can catch error of 'too deep recursion'.
  ASSERT(catch(recurse(1)));

  ASSERT_EQ(0, recurse(0));
}
