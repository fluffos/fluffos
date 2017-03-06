#define AAA ([ \
  "123": 1, \
  "234": 2, \
])

do_tests() {
  ASSERT_EQ(sizeof(AAA), 2);
}
