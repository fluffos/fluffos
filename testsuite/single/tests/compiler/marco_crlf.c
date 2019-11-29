#define LONG \
int real_test() \
{ \
return \
1; \
} \
\


LONG

void do_tests() {
  ASSERT_EQ(1, real_tests());
}
