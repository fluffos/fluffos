// Test JSON sefun

void do_tests() {
  if(find_object("/std/json")) {
    // TODO: add more test cases

    // Decoding
    ASSERT_EQ(({0,1,2,3,4,5,0,4,0.0}), json_decode("[0,1,2,3,4,5,-0,4,0.0]"));
    ASSERT_EQ(({}), json_decode("[]"));

    // Encoding
    ASSERT_EQ("[1,2,3,4]", json_encode(({1,2,3,4})));
  }
}
