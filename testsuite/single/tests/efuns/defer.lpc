object tp;
int *result;

void my_func(int num){
  result += ({ num });

  // this_player check.
  ASSERT_EQ(this_player(), tp);

  // Reverse defer: last result = max result
#ifdef __REVERSE_DEFER__
  ASSERT_EQ(max(result), num);
  // Normal defer: last result = min result
#else
  ASSERT_EQ(min(result), num);
#endif
}

void do_tests() {
  result = ({ });
  tp = this_player();

  // In normal mode, second defer is executed first.
  // In reverse mode, first defer is executed first.
  defer((: my_func, 1 :));
  defer((: my_func, 2 :));
}
