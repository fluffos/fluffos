// Test JSON sefun

void do_tests() {
  if(find_object("/std/json")) {
    // TODO: add more test cases

    // Decoding
    ASSERT_EQ(({1,2,3,4,5}), json_decode("[1,2,3,4,5]"));
    ASSERT_EQ(({}), json_decode("[]"));

    // Encoding
    ASSERT_EQ("[1,2,3,4]", json_encode(({1,2,3,4})));
  }
}
