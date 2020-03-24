void do_tests() {
  int* x = ({1, 2, 3, 4});
  int* x_orig = ({1, 2, 3, 4});
  int count = 0;

  foreach(int i in x) {
    count++;
  }

  ASSERT_EQ(sizeof(x), count);
  ASSERT_EQ(x_orig, x);

  count = 0;
  foreach(int ref i in x) {
    i++;
    count++;
  }

  ASSERT_EQ(sizeof(x), count);
  ASSERT_EQ(({2, 3, 4, 5}), x);

  {
    mapping tmp = ([ "x": 1,
                     "y": 2,
                     "z": 3 ]);

    count = 0;
    foreach(string key, string val in tmp) {
      write(key + ":" + val + "\n");
      count++;
    }

    ASSERT_EQ(sizeof(tmp), count);
  }
}
