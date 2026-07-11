int func(int x, int y) {
  return x - y;
}

void do_tests() {
  mixed *tmp;
  tmp = ({ 4, 3, 2 , 1 });

  // sort with built-in sorter
  ASSERT_EQ(({ 1, 2, 3, 4 }), sort_array(tmp, 1));
  ASSERT_EQ(({ 4, 3, 2, 1 }), sort_array(tmp, -1));

  // sort with callback
  ASSERT_EQ(({ 1, 2, 3, 4 }), sort_array(tmp, "func"));
  ASSERT_EQ(({ 1, 2, 3, 4 }), sort_array(tmp, (: $1 - $2 :)));
  ASSERT_EQ(({ 4, 3, 2, 1 }), sort_array(tmp, (: $2 - $1 :)));
}
